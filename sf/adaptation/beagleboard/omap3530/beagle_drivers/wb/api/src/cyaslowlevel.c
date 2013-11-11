/* Cypress West Bridge API source file (cyaslowlevel.c)
## ===========================
##
##  Copyright Cypress Semiconductor Corporation, 2006-2009,
##  All Rights Reserved
##  UNPUBLISHED, LICENSED SOFTWARE.
##
##  CONFIDENTIAL AND PROPRIETARY INFORMATION
##  WHICH IS THE PROPERTY OF CYPRESS.
##
##  Use of this file is governed
##  by the license agreement included in the file
##
##     <install>/license/license.txt
##
##  where <install> is the Cypress software
##  installation root directory path.
##
## ===========================
*/

#include "cyashal.h"
#include "cyascast.h"
#include "cyasdevice.h"
#include "cyaslowlevel.h"
#include "cyasintr.h"
#include "cyaserr.h"
#include "cyasregs.h"

static const uint32_t CyAsLowLevelTimeoutCount = 65536 * 4 ;

/* Forward declaration */
static CyAsReturnStatus_t CyAsSendOne(CyAsDevice *dev_p, CyAsLLRequestResponse *req_p) ;

/*
* This array holds the size of the largest request we will ever recevie from
* the West Bridge device per context.  The size is in 16 bit words.  Note a size of
* 0xffff indicates that there will be no requests on this context from West Bridge.
*/
static uint16_t MaxRequestLength[CY_RQT_CONTEXT_COUNT] =
{
    8,                          /* CY_RQT_GENERAL_RQT_CONTEXT - CY_RQT_INITIALIZATION_COMPLETE */
    8,                          /* CY_RQT_RESOURCE_RQT_CONTEXT - none */
    8,                          /* CY_RQT_STORAGE_RQT_CONTEXT - CY_RQT_MEDIA_CHANGED */
    128,                        /* CY_RQT_USB_RQT_CONTEXT - CY_RQT_USB_EVENT */
    8                           /* CY_RQT_TUR_RQT_CONTEXT - CY_RQT_TURBO_CMD_FROM_HOST */
} ;

/*
* For the given context, this function removes the request node at the head of the
* queue from the context.  This is called after all processing has occurred on
* the given request and response and we are ready to remove this entry from the
* queue.
*/
static void
CyAsLLRemoveRequestQueueHead(CyAsDevice *dev_p, CyAsContext *ctxt_p)
{
    uint32_t mask, state ;
    CyAsLLRequestListNode *node_p ;

    (void)dev_p ;
    CyAsHalAssert(ctxt_p->request_queue_p != 0) ;

    mask = CyAsHalDisableInterrupts() ;
    node_p = ctxt_p->request_queue_p ;
    ctxt_p->request_queue_p = node_p->next ;
    CyAsHalEnableInterrupts(mask) ;

    node_p->callback = 0 ;
    node_p->rqt = 0 ;
    node_p->resp = 0 ;

    /*
    * Note that the caller allocates and destroys the request and response.  Generally the
    * destroy happens in the callback for async requests and after the wait returns for
    * sync.  The request and response may not actually be destroyed but may be managed in other
    * ways as well.  It is the responsibilty of the caller to deal with these in any case.  The
    * caller can do this in the request/response callback function.
    */
    state = CyAsHalDisableInterrupts() ;
    CyAsHalCBFree(node_p) ;
    CyAsHalEnableInterrupts(state) ;
}

/*
* For the context given, this function sends the next request to
* West Bridge via the mailbox register, if the next request is ready to
* be sent and has not already been sent.
*/
static void
CyAsLLSendNextRequest(CyAsDevice *dev_p, CyAsContext *ctxt_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    /*
    * ret == ret is equivalent to while (1) but eliminates compiler warnings for
    * some compilers.
    */
    while (ret == ret)
    {
        CyAsLLRequestListNode *node_p = ctxt_p->request_queue_p ;
        if (node_p == 0)
            break ;

        if (CyAsRequestGetNodeState(node_p) != CY_AS_REQUEST_LIST_STATE_QUEUED)
            break ;

        CyAsRequestSetNodeState(node_p, CY_AS_REQUEST_LIST_STATE_WAITING) ;
        ret = CyAsSendOne(dev_p, node_p->rqt) ;
        if (ret == CY_AS_ERROR_SUCCESS)
        {
            break ;
        }

        /*
        * If an error occurs in sending the request, tell the requester
        * about the error and remove the request from the queue.
        */
        CyAsRequestSetNodeState(node_p, CY_AS_REQUEST_LIST_STATE_RECEIVED) ;
        node_p->callback(dev_p, ctxt_p->number, node_p->rqt, node_p->resp, ret) ;
        CyAsLLRemoveRequestQueueHead(dev_p, ctxt_p) ;

        /*
        * This falls through to the while loop to send the next request since
        * the previous request did not get sent.
        */
    }
}

/*
* This method removes an entry from the request queue of a given context.  The entry
* is removed only if it is not in transit.
*/
CyAsRemoveRequestResult_t
CyAsLLRemoveRequest(CyAsDevice *dev_p, CyAsContext *ctxt_p, CyAsLLRequestResponse *req_p, CyBool force)
{
    uint32_t imask ;
    CyAsLLRequestListNode *node_p ;
    CyAsLLRequestListNode *tmp_p ;
    uint32_t state ;

    imask = CyAsHalDisableInterrupts() ;
    if (ctxt_p->request_queue_p != 0 && ctxt_p->request_queue_p->rqt == req_p)
    {
        node_p = ctxt_p->request_queue_p ;
        if ((CyAsRequestGetNodeState(node_p) == CY_AS_REQUEST_LIST_STATE_WAITING) && (!force))
        {
            CyAsHalEnableInterrupts(imask) ;
            return CyAsRemoveRequestInTransit ;
        }

        ctxt_p->request_queue_p = node_p->next ;
    }
    else
    {
        tmp_p = ctxt_p->request_queue_p ;
        while (tmp_p != 0 && tmp_p->next != 0 && tmp_p->next->rqt != req_p)
            tmp_p = tmp_p->next ;

        if (tmp_p == 0 || tmp_p->next == 0)
        {
            CyAsHalEnableInterrupts(imask) ;
            return CyAsRemoveRequestNotFound ;
        }

        node_p = tmp_p->next ;
        tmp_p->next = node_p->next ;
    }

    if (node_p->callback)
        node_p->callback(dev_p, ctxt_p->number, node_p->rqt, node_p->resp, CY_AS_ERROR_CANCELED) ;

    state = CyAsHalDisableInterrupts() ;
    CyAsHalCBFree(node_p) ;
    CyAsHalEnableInterrupts(state) ;

    CyAsHalEnableInterrupts(imask) ;
    return CyAsRemoveRequestSucessful ;
}

void
CyAsLLRemoveAllRequests(CyAsDevice *dev_p, CyAsContext *ctxt_p)
{
    CyAsLLRequestListNode *node = ctxt_p->request_queue_p ;

    while(node)
    {
        if(CyAsRequestGetNodeState(ctxt_p->request_queue_p) != CY_AS_REQUEST_LIST_STATE_RECEIVED)
            CyAsLLRemoveRequest(dev_p, ctxt_p, node->rqt, CyTrue) ;
        node = node->next ;
    }
}

static CyBool
CyAsLLIsInQueue(CyAsContext *ctxt_p, CyAsLLRequestResponse *req_p)
{
    uint32_t mask ;
    CyAsLLRequestListNode *node_p ;

    mask = CyAsHalDisableInterrupts() ;
    node_p = ctxt_p->request_queue_p ;
    while (node_p)
    {
        if (node_p->rqt == req_p)
        {
            CyAsHalEnableInterrupts(mask) ;
            return CyTrue ;
        }
        node_p = node_p->next ;
    }
    CyAsHalEnableInterrupts(mask) ;
    return CyFalse ;
}

/*
* This is the handler for mailbox data when we are trying to send data to the West Bridge firmware.  The
* firmware may be trying to send us data and we need to queue this data to allow the firmware to move
* forward and be in a state to receive our request.  Here we just queue the data and it is processed
* at a later time by the mailbox interrupt handler.
*/
void
CyAsLLQueueMailboxData(CyAsDevice *dev_p)
{
    CyAsContext *ctxt_p ;
    uint8_t context ;
    uint16_t data[4] ;
    int32_t i ;

    /* Read the data from mailbox 0 to determine what to do with the data */
    for(i = 3 ; i >= 0 ; i--)
        data[i] = CyAsHalReadRegister(dev_p->tag, CyCastInt2UInt16(CY_AS_MEM_P0_MAILBOX0 + i)) ;

    context = CyAsMboxGetContext(data[0]) ;
    if (context >= CY_RQT_CONTEXT_COUNT)
    {
        CyAsHalPrintMessage("Mailbox request/response received with invalid context value (%d)\n", context) ;
        return ;
    }

    ctxt_p = dev_p->context[context] ;

    /*
    * If we have queued too much data, drop future data.
    */
    CyAsHalAssert(ctxt_p->queue_index * sizeof(uint16_t) + sizeof(data) <= sizeof(ctxt_p->data_queue)) ;

    for(i = 0 ; i < 4 ; i++)
        ctxt_p->data_queue[ctxt_p->queue_index++] = data[i] ;

    CyAsHalAssert((ctxt_p->queue_index % 4) == 0) ;
    dev_p->ll_queued_data = CyTrue ;
}

void
CyAsMailBoxProcessData(CyAsDevice *dev_p, uint16_t *data)
{
    CyAsContext *ctxt_p ;
    uint8_t context ;
    uint16_t *len_p ;
    CyAsLLRequestResponse *rec_p ;
    uint8_t st ;
    uint16_t src, dest ;

    context = CyAsMboxGetContext(data[0]) ;
    if (context >= CY_RQT_CONTEXT_COUNT)
    {
        CyAsHalPrintMessage("Mailbox request/response received with invalid context value (%d)\n", context) ;
        return ;
    }

    ctxt_p = dev_p->context[context] ;

    if (CyAsMboxIsRequest(data[0]))
    {
        CyAsHalAssert(ctxt_p->req_p != 0) ;
        rec_p = ctxt_p->req_p ;
        len_p = &ctxt_p->request_length ;

    }
    else
    {
        if (ctxt_p->request_queue_p == 0 || CyAsRequestGetNodeState(ctxt_p->request_queue_p) != CY_AS_REQUEST_LIST_STATE_WAITING)
        {
            CyAsHalPrintMessage("Mailbox response received on context that was not expecting a response\n") ;
            CyAsHalPrintMessage("    Context: %d\n", context) ;
            CyAsHalPrintMessage("    Contents: 0x%04x 0x%04x 0x%04x 0x%04x\n", data[0], data[1], data[2], data[3]) ;
            if (ctxt_p->request_queue_p != 0)
                CyAsHalPrintMessage("    State: 0x%02x\n", ctxt_p->request_queue_p->state) ;
            return ;
        }

        /* Make sure the request has an associated response */
        CyAsHalAssert(ctxt_p->request_queue_p->resp != 0) ;

        rec_p = ctxt_p->request_queue_p->resp ;
        len_p = &ctxt_p->request_queue_p->length ;
    }

    if (rec_p->stored == 0)
    {
        /*
        * This is the first cycle of the response
        */
        CyAsLLRequestResponse_SetCode(rec_p, CyAsMboxGetCode(data[0])) ;
        CyAsLLRequestResponse_SetContext(rec_p, context) ;

        if (CyAsMboxIsLast(data[0]))
        {
            /* This is a single cycle response */
            *len_p = rec_p->length ;
            st = 1 ;
        }
        else
        {
            /* Ensure that enough memory has been reserved for the response. */
            CyAsHalAssert(rec_p->length >= data[1]) ;
            *len_p = (data[1] < rec_p->length) ? data[1] : rec_p->length ;
            st = 2 ;
        }
    }
    else
        st = 1 ;

    /* Trasnfer the data from the mailboxes to the response */
    while (rec_p->stored < *len_p && st < 4)
        rec_p->data[rec_p->stored++] = data[st++] ;

    if (CyAsMboxIsLast(data[0]))
    {
        /* NB: The call-back that is made below can cause the addition of more data
        in this queue, thus causing a recursive overflow of the queue. This is prevented
        by removing the request entry that is currently being passed up from the data
        queue. If this is done, the queue only needs to be as long as two request
        entries from West Bridge.
        */
        if ((ctxt_p->rqt_index > 0) && (ctxt_p->rqt_index <= ctxt_p->queue_index))
        {
            dest = 0 ;
            src  = ctxt_p->rqt_index ;

            while (src < ctxt_p->queue_index)
                ctxt_p->data_queue[dest++] = ctxt_p->data_queue[src++] ;

            ctxt_p->rqt_index = 0 ;
            ctxt_p->queue_index = dest ;
            CyAsHalAssert((ctxt_p->queue_index % 4) == 0) ;
        }

        if (ctxt_p->request_queue_p != 0 && rec_p == ctxt_p->request_queue_p->resp)
        {
            /*
            * If this is the last cycle of the response, call the callback and
            * reset for the next response.
            */
            CyAsLLRequestResponse *resp_p = ctxt_p->request_queue_p->resp ;
            resp_p->length = ctxt_p->request_queue_p->length ;
            CyAsRequestSetNodeState(ctxt_p->request_queue_p, CY_AS_REQUEST_LIST_STATE_RECEIVED) ;

            CyAsDeviceSetInCallback(dev_p) ;
            ctxt_p->request_queue_p->callback(dev_p, context, ctxt_p->request_queue_p->rqt, resp_p, CY_AS_ERROR_SUCCESS) ;
            CyAsDeviceClearInCallback(dev_p) ;

            CyAsLLRemoveRequestQueueHead(dev_p, ctxt_p) ;
            CyAsLLSendNextRequest(dev_p, ctxt_p) ;
        }
        else
        {
            /* Send the request to the appropriate module to handle */
            CyAsLLRequestResponse *request_p = ctxt_p->req_p ;
            ctxt_p->req_p = 0 ;
            if (ctxt_p->request_callback)
            {
                CyAsDeviceSetInCallback(dev_p) ;
                ctxt_p->request_callback(dev_p, context, request_p, 0, CY_AS_ERROR_SUCCESS) ;
                CyAsDeviceClearInCallback(dev_p) ;
            }
            CyAsLLInitRequest(request_p, 0, context, request_p->length) ;
            ctxt_p->req_p = request_p ;
        }
    }
}

/*
* This is the handler for processing queued mailbox data
*/
void
CyAsMailBoxQueuedDataHandler(CyAsDevice *dev_p)
{
    uint16_t i ;

    /*
     * If more data gets queued in between our entering this call and the
     * end of the iteration on all contexts; we should continue processing the
     * queued data.
     */
    while (dev_p->ll_queued_data)
    {
        dev_p->ll_queued_data = CyFalse ;
        for(i = 0 ; i < CY_RQT_CONTEXT_COUNT ; i++)
        {
            uint16_t offset ;
            CyAsContext *ctxt_p = dev_p->context[i] ;
            CyAsHalAssert((ctxt_p->queue_index % 4) == 0) ;

            offset = 0 ;
            while (offset < ctxt_p->queue_index)
            {
                ctxt_p->rqt_index = offset + 4 ;
                CyAsMailBoxProcessData(dev_p, ctxt_p->data_queue + offset) ;
                offset = ctxt_p->rqt_index ;
            }
            ctxt_p->queue_index = 0 ;
        }
    }
}

/*
* This is the handler for the mailbox interrupt.  This function reads data from the mailbox registers
* until a complete request or response is received.  When a complete request is received, the callback
* associated with requests on that context is called.  When a complete response is recevied, the callback
* associated with the request that generated the reponse is called.
*/
void
CyAsMailBoxInterruptHandler(CyAsDevice *dev_p)
{
    CyAsHalAssert(dev_p->sig == CY_AS_DEVICE_HANDLE_SIGNATURE) ;

    /*
    * Queue the mailbox data to preserve order for later processing.
    */
    CyAsLLQueueMailboxData(dev_p) ;

    /*
    * Process what was queued and anything that may be pending
    */
    CyAsMailBoxQueuedDataHandler(dev_p) ;
}

CyAsReturnStatus_t
CyAsLLStart(CyAsDevice *dev_p)
{
    uint16_t i ;

    if (CyAsDeviceIsLowLevelRunning(dev_p))
        return CY_AS_ERROR_ALREADY_RUNNING ;

    dev_p->ll_sending_rqt = CyFalse ;
    dev_p->ll_abort_curr_rqt = CyFalse ;

    for(i = 0 ; i < CY_RQT_CONTEXT_COUNT ; i++)
    {
        dev_p->context[i] = (CyAsContext *)CyAsHalAlloc(sizeof(CyAsContext)) ;
        if (dev_p->context[i] == 0)
            return CY_AS_ERROR_OUT_OF_MEMORY ;

        dev_p->context[i]->number = (uint8_t)i ;
        dev_p->context[i]->request_callback = 0 ;
        dev_p->context[i]->request_queue_p = 0 ;
        dev_p->context[i]->last_node_p = 0 ;
        dev_p->context[i]->req_p = CyAsLLCreateRequest(dev_p, 0, (uint8_t)i, MaxRequestLength[i]) ;
        dev_p->context[i]->queue_index = 0 ;

        if (!CyAsHalCreateSleepChannel(&dev_p->context[i]->channel))
            return CY_AS_ERROR_CREATE_SLEEP_CHANNEL_FAILED ;
    }

    CyAsDeviceSetLowLevelRunning(dev_p) ;
    return CY_AS_ERROR_SUCCESS ;
}

/*
* Shutdown the low level communications module.  This operation will also cancel any
* queued low level requests.
*/
CyAsReturnStatus_t
CyAsLLStop(CyAsDevice *dev_p)
{
    uint8_t i ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsContext *ctxt_p ;
    uint32_t mask ;

    for(i = 0 ; i < CY_RQT_CONTEXT_COUNT ; i++)
    {
        ctxt_p = dev_p->context[i] ;
        if (!CyAsHalDestroySleepChannel(&ctxt_p->channel))
            return CY_AS_ERROR_DESTROY_SLEEP_CHANNEL_FAILED ;

        /*
        * Now, free any queued requests and assocaited responses
        */
        while (ctxt_p->request_queue_p)
        {
            uint32_t state ;
            CyAsLLRequestListNode *node_p = ctxt_p->request_queue_p ;

            /* Mark this pair as in a cancel operation */
            CyAsRequestSetNodeState(node_p, CY_AS_REQUEST_LIST_STATE_CANCELING) ;

            /* Tell the caller that we are canceling this request */
            /* NB: The callback is responsible for destroying the request and the
            response.  We cannot count on the contents of these two after
            calling the callback.
            */
            node_p->callback(dev_p, i, node_p->rqt, node_p->resp, CY_AS_ERROR_CANCELED) ;

            /* Remove the pair from the queue */
            mask = CyAsHalDisableInterrupts() ;
            ctxt_p->request_queue_p = node_p->next ;
            CyAsHalEnableInterrupts(mask) ;

            /* Free the list node */
            state = CyAsHalDisableInterrupts() ;
            CyAsHalCBFree(node_p) ;
            CyAsHalEnableInterrupts(state) ;
        }

        CyAsLLDestroyRequest(dev_p, dev_p->context[i]->req_p) ;
        CyAsHalFree(dev_p->context[i]) ;
        dev_p->context[i] = 0 ;

    }
    CyAsDeviceSetLowLevelStopped(dev_p) ;

    return ret ;
}

void
CyAsLLInitRequest(CyAsLLRequestResponse *req_p, uint16_t code, uint16_t context, uint16_t length)
{
    uint16_t totallen = sizeof(CyAsLLRequestResponse) + (length - 1) * sizeof(uint16_t) ;

    CyAsHalMemSet(req_p, 0, totallen) ;
    req_p->length = length ;
    CyAsLLRequestResponse_SetCode(req_p, code) ;
    CyAsLLRequestResponse_SetContext(req_p, context) ;
    CyAsLLRequestResponse_SetRequest(req_p) ;
}

/*
* Create a new request.
*/
CyAsLLRequestResponse *
CyAsLLCreateRequest(CyAsDevice *dev_p, uint16_t code, uint8_t context, uint16_t length)
{
    CyAsLLRequestResponse *req_p ;
    uint32_t state ;
    uint16_t totallen = sizeof(CyAsLLRequestResponse) + (length - 1) * sizeof(uint16_t) ;

    (void)dev_p ;

    state = CyAsHalDisableInterrupts() ;
    req_p = (CyAsLLRequestResponse *)CyAsHalCBAlloc(totallen) ;
    CyAsHalEnableInterrupts(state) ;
    if(req_p)
        CyAsLLInitRequest(req_p, code, context, length) ;

    return req_p ;
}

/*
* Destroy a request.
*/
void
CyAsLLDestroyRequest(CyAsDevice *dev_p, CyAsLLRequestResponse *req_p)
{
    uint32_t state ;
    (void)dev_p ;
    (void)req_p ;

    state = CyAsHalDisableInterrupts() ;
    CyAsHalCBFree(req_p) ;
    CyAsHalEnableInterrupts(state) ;

}

void
CyAsLLInitResponse(CyAsLLRequestResponse *req_p, uint16_t length)
{
    uint16_t totallen = sizeof(CyAsLLRequestResponse) + (length - 1) * sizeof(uint16_t) ;

    CyAsHalMemSet(req_p, 0, totallen) ;
    req_p->length = length ;
    CyAsLLRequestResponse_SetResponse(req_p) ;
}

/*
* Create a new response
*/
CyAsLLRequestResponse *
CyAsLLCreateResponse(CyAsDevice *dev_p, uint16_t length)
{
    CyAsLLRequestResponse *req_p ;
    uint32_t state ;
    uint16_t totallen = sizeof(CyAsLLRequestResponse) + (length - 1) * sizeof(uint16_t) ;

    (void)dev_p ;

    state = CyAsHalDisableInterrupts() ;
    req_p = (CyAsLLRequestResponse *)CyAsHalCBAlloc(totallen) ;
    CyAsHalEnableInterrupts(state) ;
    if(req_p)
        CyAsLLInitResponse(req_p, length) ;

    return req_p ;
}

/*
* Destroy the new response
*/
void
CyAsLLDestroyResponse(CyAsDevice *dev_p, CyAsLLRequestResponse *req_p)
{
    uint32_t state ;
    (void)dev_p ;
    (void)req_p ;

    state = CyAsHalDisableInterrupts() ;
    CyAsHalCBFree(req_p) ;
    CyAsHalEnableInterrupts(state) ;
}

static uint16_t
CyAsReadIntrStatus(
                   CyAsDevice *dev_p)
{
    uint32_t mask ;
    CyBool bloop = CyTrue ;
    uint16_t v = 0, last = 0xffff;

    /*
    * Before determining if the mailboxes are ready for more data, we first check the
    * mailbox interrupt to see if we need to receive data.  This prevents a dead-lock
    * condition that can occur when both sides are trying to receive data.
    */
    while (last == last)
    {
        /*
        * Disable interrupts to be sure we don't process the mailbox here and have the
        * interrupt routine try to read this data as well.
        */
        mask = CyAsHalDisableInterrupts() ;

        /*
        * See if there is data to be read.
        */
        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_INTR_REG) ;
        if ((v & CY_AS_MEM_P0_INTR_REG_MBINT) == 0)
        {
            CyAsHalEnableInterrupts(mask) ;
            break ;
        }

        /*
        * Queue the mailbox data for later processing.  This allows the firmware to move
        * forward and service the requst from the P port.
        */
        CyAsLLQueueMailboxData(dev_p) ;

        /*
        * Enable interrupts again to service mailbox interrupts appropriately
        */
        CyAsHalEnableInterrupts(mask) ;
    }

    /*
    * Now, all data is received
    */
    last = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MB_STAT) & CY_AS_MEM_P0_MCU_MBNOTRD ;
    while (bloop)
    {
        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MB_STAT) & CY_AS_MEM_P0_MCU_MBNOTRD ;
        if (v == last)
            break ;

        last = v ;
    }

    return v ;
}

/*
* Send a single request or response using the mail box register.  This function does not deal
* with the internal queues at all, but only sends the request or response across to the firmware
*/
static CyAsReturnStatus_t
CyAsSendOne(
            CyAsDevice *dev_p,
            CyAsLLRequestResponse *req_p)
{
    int i ;
    uint16_t mb0, v ;
    int32_t loopcount ;
    uint32_t intStat ;

#ifdef _DEBUG
    if (CyAsLLRequestResponse_IsRequest(req_p))
    {
        switch(CyAsLLRequestResponse_GetContext(req_p))
        {
        case CY_RQT_GENERAL_RQT_CONTEXT:
            CyAsHalAssert(req_p->length * 2 + 2 < CY_CTX_GEN_MAX_DATA_SIZE) ;
            break ;

        case CY_RQT_RESOURCE_RQT_CONTEXT:
            CyAsHalAssert(req_p->length * 2 + 2 < CY_CTX_RES_MAX_DATA_SIZE) ;
            break ;

        case CY_RQT_STORAGE_RQT_CONTEXT:
            CyAsHalAssert(req_p->length * 2 + 2 < CY_CTX_STR_MAX_DATA_SIZE) ;
            break ;

        case CY_RQT_USB_RQT_CONTEXT:
            CyAsHalAssert(req_p->length * 2 + 2 < CY_CTX_USB_MAX_DATA_SIZE) ;
            break ;
        }
    }
#endif

    /* Write the request to the mail box registers */
    if (req_p->length > 3)
    {
        uint16_t length = req_p->length ;
        int which = 0 ;
        int st = 1 ;

        dev_p->ll_sending_rqt = CyTrue ;
        while (which < length)
        {
            loopcount = CyAsLowLevelTimeoutCount ;
            do
            {
                v = CyAsReadIntrStatus(dev_p) ;

            } while (v && loopcount-- > 0) ;

            if (v)
            {
                CyAsHalPrintMessage(">>>>>> LOW LEVEL TIMEOUT %x %x %x %x\n",
                    CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX0),
                    CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX1),
                    CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX2),
                    CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX3)) ;
                return CY_AS_ERROR_TIMEOUT ;
            }

            if (dev_p->ll_abort_curr_rqt)
            {
                dev_p->ll_sending_rqt = CyFalse ;
                dev_p->ll_abort_curr_rqt = CyFalse ;
                return CY_AS_ERROR_CANCELED ;
            }

            intStat = CyAsHalDisableInterrupts () ;

            /*
             * Check again whether the mailbox is free. It is possible that an ISR came in
             * and wrote into the mailboxes since we last checked the status.
             */
            v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MB_STAT) & CY_AS_MEM_P0_MCU_MBNOTRD ;
            if (v)
            {
                /* Go back to the original check since the mailbox is not free. */
                CyAsHalEnableInterrupts(intStat) ;
                continue ;
            }

            if (which == 0)
            {
                CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX1, length) ;
                st = 2 ;
            }
            else
            {
                st = 1;
            }

            while ((which < length) && (st < 4))
            {
                CyAsHalWriteRegister(dev_p->tag, CyCastInt2UInt16(CY_AS_MEM_MCU_MAILBOX0 + st), req_p->data[which++]) ;
                st++ ;
            }

            mb0 = req_p->box0 ;
            if (which == length)
            {
                dev_p->ll_sending_rqt = CyFalse ;
                mb0 |= CY_AS_REQUEST_RESPONSE_LAST_MASK ;
            }

            if (dev_p->ll_abort_curr_rqt)
            {
                dev_p->ll_sending_rqt = CyFalse ;
                dev_p->ll_abort_curr_rqt = CyFalse ;
                CyAsHalEnableInterrupts (intStat) ;
                return CY_AS_ERROR_CANCELED ;
            }

            CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX0, mb0) ;

            /* Wait for the MBOX interrupt to be high */
            CyAsHalSleep150() ;
            CyAsHalEnableInterrupts (intStat) ;
        }
    }
    else
    {
CheckMailboxAvailability:
        /*
        * Wait for the mailbox registers to become available.  This should be a very quick
        * wait as the firmware is designed to accept requests at interrupt time and queue
        * them for future processing.
        */
        loopcount = CyAsLowLevelTimeoutCount ;
        do
        {
            v = CyAsReadIntrStatus(dev_p) ;

        } while (v && loopcount-- > 0) ;

        if (v)
        {
            CyAsHalPrintMessage(">>>>>> LOW LEVEL TIMEOUT %x %x %x %x\n",
                CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX0),
                CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX1),
                CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX2),
                CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX3)) ;
            return CY_AS_ERROR_TIMEOUT ;
        }

        intStat = CyAsHalDisableInterrupts ();

        /*
         * Check again whether the mailbox is free. It is possible that an ISR came in
         * and wrote into the mailboxes since we last checked the status.
         */
        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_MCU_MB_STAT) & CY_AS_MEM_P0_MCU_MBNOTRD ;
        if (v)
        {
            /* Go back to the original check since the mailbox is not free. */
            CyAsHalEnableInterrupts(intStat) ;
            goto CheckMailboxAvailability ;
        }

        /* Write the data associated with the request into the mbox registers 1 - 3 */
        v = 0 ;
        for(i = req_p->length - 1 ; i >= 0 ; i--)
            CyAsHalWriteRegister(dev_p->tag, CyCastInt2UInt16(CY_AS_MEM_MCU_MAILBOX1 + i), req_p->data[i]) ;

        /* Write the mbox register 0 to trigger the interrupt */
        CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_MCU_MAILBOX0, req_p->box0 | CY_AS_REQUEST_RESPONSE_LAST_MASK) ;

        CyAsHalSleep150() ;
        CyAsHalEnableInterrupts (intStat);
    }

    return CY_AS_ERROR_SUCCESS ;
}

/*
* This function queues a single request to be sent to the firmware.
*/
extern CyAsReturnStatus_t
CyAsLLSendRequest(
                  CyAsDevice *dev_p,
                  CyAsLLRequestResponse *req,   /* The request to send */
                  CyAsLLRequestResponse *resp,  /* Storage for a reply, must be sure it is of sufficient size */
                  CyBool sync,                  /* If true, this is a synchronous request */
                  CyAsResponseCallback cb       /* Callback to call when reply is received */
                  )
{
    CyAsContext *ctxt_p ;
    uint16_t box0 = req->box0 ;
    uint8_t context ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsLLRequestListNode *node_p ;
    uint32_t mask, state ;

    CyAsHalAssert(dev_p->sig == CY_AS_DEVICE_HANDLE_SIGNATURE);

    context = CyAsMboxGetContext(box0) ;
    CyAsHalAssert(context < CY_RQT_CONTEXT_COUNT) ;
    ctxt_p = dev_p->context[context] ;

    /* Allocate the list node */
    state = CyAsHalDisableInterrupts() ;
    node_p = (CyAsLLRequestListNode *)CyAsHalCBAlloc(sizeof(CyAsLLRequestListNode)) ;
    CyAsHalEnableInterrupts(state) ;

    if (node_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* Initialize the list node */
    node_p->callback = cb ;
    node_p->length = 0 ;
    node_p->next = 0 ;
    node_p->resp = resp ;
    node_p->rqt = req ;
    node_p->state = CY_AS_REQUEST_LIST_STATE_QUEUED ;
    if (sync)
        CyAsRequestNodeSetSync(node_p) ;

    /* Put the request into the queue */
    mask = CyAsHalDisableInterrupts() ;
    if (ctxt_p->request_queue_p == 0)
    {
        /* Empty queue */
        ctxt_p->request_queue_p = node_p ;
        ctxt_p->last_node_p = node_p ;
    }
    else
    {
        ctxt_p->last_node_p->next = node_p ;
        ctxt_p->last_node_p = node_p ;
    }
    CyAsHalEnableInterrupts(mask) ;
    CyAsLLSendNextRequest(dev_p, ctxt_p) ;

    if (!CyAsDeviceIsInCallback(dev_p))
    {
        mask = CyAsHalDisableInterrupts() ;
        CyAsMailBoxQueuedDataHandler(dev_p) ;
        CyAsHalEnableInterrupts(mask) ;
    }

    return ret ;
}

static void
CyAsLLSendCallback(
                   CyAsDevice *dev_p,
                   uint8_t context,
                   CyAsLLRequestResponse *rqt,
                   CyAsLLRequestResponse *resp,
                   CyAsReturnStatus_t ret)
{
    (void)rqt ;
    (void)resp ;
    (void)ret ;


    CyAsHalAssert(dev_p->sig == CY_AS_DEVICE_HANDLE_SIGNATURE) ;

    /*
    * Storage the state to return to the caller
    */
    dev_p->ll_error = ret ;

    /*
    * Now wake the caller
    */
    CyAsHalWake(&dev_p->context[context]->channel) ;
}

CyAsReturnStatus_t
CyAsLLSendRequestWaitReply(
                           CyAsDevice *dev_p,
                           CyAsLLRequestResponse *req,                          /* The request to send */
                           CyAsLLRequestResponse *resp                          /* Storage for a reply, must be sure it is of sufficient size */
                           )
{
    CyAsReturnStatus_t ret ;
    uint8_t context ;
    uint32_t loopcount = 800 ;  /* Larger 8 sec time-out to handle the init delay for slower storage devices in USB FS. */
    CyAsContext *ctxt_p ;

    /* Get the context for the request */
    context = CyAsLLRequestResponse_GetContext(req) ;
    CyAsHalAssert(context < CY_RQT_CONTEXT_COUNT) ;
    ctxt_p = dev_p->context[context] ;

    ret = CyAsLLSendRequest(dev_p, req, resp, CyTrue, CyAsLLSendCallback) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    while (loopcount-- > 0)
    {
        /*
        * Sleep while we wait on the response.  Receiving the reply will
        * wake this thread.  We will wait, at most 2 seconds (10 ms * 200
        * tries) before we timeout.  Note if the reply arrives, we will not
        * sleep the entire 10 ms, just til the reply arrives.
        */
        CyAsHalSleepOn(&ctxt_p->channel, 10) ;

        /*
        * If the request has left the queue, it means the request has been sent
        * and the reply has been received.  This means we can return to the caller
        * and be sure the reply has been received.
        */
        if (!CyAsLLIsInQueue(ctxt_p, req))
            return dev_p->ll_error ;
    }

    /* Remove the QueueListNode for this request. */
    CyAsLLRemoveRequest(dev_p, ctxt_p, req, CyTrue) ;

    return CY_AS_ERROR_TIMEOUT ;
}

CyAsReturnStatus_t
CyAsLLRegisterRequestCallback(
                              CyAsDevice *dev_p,
                              uint8_t context,
                              CyAsResponseCallback cb)
{
    CyAsContext *ctxt_p ;
    CyAsHalAssert(context < CY_RQT_CONTEXT_COUNT) ;
    ctxt_p = dev_p->context[context] ;

    ctxt_p->request_callback = cb ;
    return CY_AS_ERROR_SUCCESS ;
}

void
CyAsLLRequestResponse_Pack(
                           CyAsLLRequestResponse *req_p,
                           uint32_t offset,
                           uint32_t length,
                           void *data_p)
{
    uint16_t dt ;
    uint8_t *dp = (uint8_t *)data_p ;

    while (length > 1)
    {
        dt = ((*dp++) << 8) ;
        dt |= (*dp++) ;
        CyAsLLRequestResponse_SetWord(req_p, offset, dt) ;
        offset++ ;
        length -= 2 ;
    }

    if (length == 1)
    {
        dt = (*dp << 8) ;
        CyAsLLRequestResponse_SetWord(req_p, offset, dt) ;
    }
}

void
CyAsLLRequestResponse_Unpack(
                             CyAsLLRequestResponse *req_p,
                             uint32_t offset,
                             uint32_t length,
                             void *data_p)
{
    uint8_t *dp = (uint8_t *)data_p ;

    while (length-- > 0)
    {
        uint16_t val = CyAsLLRequestResponse_GetWord(req_p, offset++) ;
        *dp++ = (uint8_t)((val >> 8) & 0xff) ;

        if (length)
        {
            length-- ;
            *dp++ = (uint8_t)(val & 0xff) ;
        }
    }
}

extern CyAsReturnStatus_t
CyAsLLSendStatusResponse(
                         CyAsDevice *dev_p,
                         uint8_t context,
                         uint16_t code,
                         uint8_t clear_storage)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse resp ;
    CyAsLLRequestResponse *resp_p = &resp ;

    CyAsHalMemSet(resp_p, 0, sizeof(resp)) ;
    resp_p->length = 1 ;
    CyAsLLRequestResponse_SetResponse(resp_p) ;
    CyAsLLRequestResponse_SetContext(resp_p, context) ;

    if (clear_storage)
        CyAsLLRequestResponse_SetClearStorageFlag(resp_p) ;

    CyAsLLRequestResponse_SetCode(resp_p, CY_RESP_SUCCESS_FAILURE) ;
    CyAsLLRequestResponse_SetWord(resp_p, 0, code) ;

    ret = CyAsSendOne(dev_p, resp_p) ;

    return ret ;
}

extern CyAsReturnStatus_t
CyAsLLSendDataResponse(
                       CyAsDevice *dev_p,
                       uint8_t context,
                       uint16_t code,
                       uint16_t length,
                       void *data)
{
    CyAsLLRequestResponse *resp_p ;
    uint16_t wlen ;
    uint8_t respbuf[256] ;

    if (length > 192)
        return CY_AS_ERROR_INVALID_SIZE ;

    wlen = length / 2 ;                                 /* Word length for bytes */
    if (length % 2)                                     /* If byte length odd, add one more */
        wlen++ ;
    wlen++ ;                                            /* One for the length of field */

    resp_p = (CyAsLLRequestResponse *)respbuf ;
    CyAsHalMemSet(resp_p, 0, sizeof(respbuf)) ;
    resp_p->length = wlen ;
    CyAsLLRequestResponse_SetContext(resp_p, context) ;
    CyAsLLRequestResponse_SetCode(resp_p, code) ;

    CyAsLLRequestResponse_SetWord(resp_p, 0, length) ;
    CyAsLLRequestResponse_Pack(resp_p, 1, length, data) ;

    return CyAsSendOne(dev_p, resp_p) ;
}

static CyBool
CyAsLLIsEPTransferRelatedRequest(CyAsLLRequestResponse *rqt_p, CyAsEndPointNumber_t ep)
{
    uint16_t v ;
    uint8_t  type = CyAsLLRequestResponse_GetCode(rqt_p) ;

    if (CyAsLLRequestResponse_GetContext(rqt_p) != CY_RQT_USB_RQT_CONTEXT)
        return CyFalse ;

    /*
     * When cancelling outstanding EP0 data transfers, any pending
     * Setup ACK requests also need to be cancelled.
     */
    if ((ep == 0) && (type == CY_RQT_ACK_SETUP_PACKET))
        return CyTrue ;

    if (type != CY_RQT_USB_EP_DATA)
        return CyFalse ;

    v = CyAsLLRequestResponse_GetWord(rqt_p, 0) ;
    if ((CyAsEndPointNumber_t)((v >> 13) & 1) != ep)
        return CyFalse ;

    return CyTrue ;
}

CyAsReturnStatus_t
CyAsLLRemoveEpDataRequests(CyAsDevice *dev_p, CyAsEndPointNumber_t ep)
{
    CyAsContext *ctxt_p ;
    CyAsLLRequestListNode *node_p ;
    uint32_t imask ;

    /*
    * First, remove any queued requests
    */
    ctxt_p = dev_p->context[CY_RQT_USB_RQT_CONTEXT] ;
    if (ctxt_p)
    {
        for(node_p = ctxt_p->request_queue_p ; node_p ; node_p = node_p->next)
        {
            if (CyAsLLIsEPTransferRelatedRequest(node_p->rqt, ep))
            {
                CyAsLLRemoveRequest(dev_p, ctxt_p, node_p->rqt, CyFalse) ;
                break ;
            }
        }

        /*
        * Now, deal with any request that may be in transit
        */
        imask = CyAsHalDisableInterrupts() ;

        if (ctxt_p->request_queue_p != 0 &&
            CyAsLLIsEPTransferRelatedRequest(ctxt_p->request_queue_p->rqt, ep) &&
            CyAsRequestGetNodeState(ctxt_p->request_queue_p) == CY_AS_REQUEST_LIST_STATE_WAITING)
        {
            CyAsHalPrintMessage("Need to remove an in-transit request to Antioch\n") ;

            /*
            * If the request has not been fully sent to West Bridge yet, abort sending.
            * Otherwise, terminate the request with a CANCELED status. Firmware will
            * already have terminated this transfer.
            */
            if (dev_p->ll_sending_rqt)
                dev_p->ll_abort_curr_rqt = CyTrue ;
            else
            {
                uint32_t state ;

                node_p = ctxt_p->request_queue_p ;
                if (node_p->callback)
                    node_p->callback(dev_p, ctxt_p->number, node_p->rqt, node_p->resp, CY_AS_ERROR_CANCELED) ;

                ctxt_p->request_queue_p = node_p->next ;
                state = CyAsHalDisableInterrupts() ;
                CyAsHalCBFree(node_p) ;
                CyAsHalEnableInterrupts(state) ;
            }
        }

        CyAsHalEnableInterrupts(imask) ;
    }

    return CY_AS_ERROR_SUCCESS ;
}
