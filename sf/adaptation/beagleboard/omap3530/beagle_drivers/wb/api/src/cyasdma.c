/* Cypress West Bridge API source file (cyasdma.c)
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
#include "cyasdma.h"
#include "cyaslowlevel.h"
#include "cyaserr.h"
#include "cyasregs.h"

/*
 * Add the DMA queue entry to the free list to be re-used later
 */
static void
CyAsDmaAddRequestToFreeQueue(CyAsDevice *dev_p, CyAsDmaQueueEntry *req_p)
{
    uint32_t imask ;
    imask = CyAsHalDisableInterrupts() ;

    req_p->next_p = dev_p->dma_freelist_p ;
    dev_p->dma_freelist_p = req_p ;

    CyAsHalEnableInterrupts(imask) ;
}

/*
 * Get a DMA queue entry from the free list.
 */
static CyAsDmaQueueEntry *
CyAsDmaGetDmaQueueEntry(CyAsDevice *dev_p)
{
    CyAsDmaQueueEntry *req_p ;
    uint32_t imask ;

    CyAsHalAssert(dev_p->dma_freelist_p != 0) ;

    imask = CyAsHalDisableInterrupts() ;
    req_p = dev_p->dma_freelist_p ;
    dev_p->dma_freelist_p = req_p->next_p ;
    CyAsHalEnableInterrupts(imask) ;

    return req_p ;
}

/*
 * Set the maximum size that the West Bridge hardware can handle in a single DMA operation.  This size
 * may change for the P <-> U endpoints as a function of the endpoint type and whether we are running
 * at full speed or high speed.
 */
CyAsReturnStatus_t
CyAsDmaSetMaxDmaSize(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, uint32_t size)
{
    /* In MTP mode, EP2 is allowed to have all max sizes. */
    if ((!dev_p->is_mtp_firmware) || (ep != 0x02))
    {
        if (size < 64 || size > 1024)
            return CY_AS_ERROR_INVALID_SIZE ;
    }

    CY_AS_NUM_EP(dev_p, ep)->maxhwdata = (uint16_t)size ;
    return CY_AS_ERROR_SUCCESS ;
}

/*
 * The callback for requests sent to West Bridge to relay endpoint data.  Endpoint
 * data for EP0 and EP1 are sent using mailbox requests.  This is the callback that
 * is called when a response to a mailbox request to send data is received.
 */
static void
CyAsDmaRequestCallback(
    CyAsDevice *dev_p,
    uint8_t context,
    CyAsLLRequestResponse *req_p,
    CyAsLLRequestResponse *resp_p,
    CyAsReturnStatus_t ret)
{
    uint16_t v ;
    uint16_t datacnt ;
    CyAsEndPointNumber_t ep ;

    (void)context ;

    CyAsLogDebugMessage(5, "CyAsDmaRequestCallback called") ;

    /*
     * Extract the return code from the firmware
     */
    if (ret == CY_AS_ERROR_SUCCESS)
    {
        if (CyAsLLRequestResponse_GetCode(resp_p) != CY_RESP_SUCCESS_FAILURE)
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        else
            ret = CyAsLLRequestResponse_GetWord(resp_p, 0) ;
    }

    /*
     * Extract the endpoint number and the transferred byte count
     * from the request.
     */
    v = CyAsLLRequestResponse_GetWord(req_p, 0) ;
    ep = (CyAsEndPointNumber_t)((v >> 13) & 0x01) ;

    if (ret == CY_AS_ERROR_SUCCESS)
    {
        /*
         * If the firmware returns success, all of the data requested was
         * transferred.  There are no partial transfers.
         */
        datacnt = v & 0x3FF ;
    }
    else
    {
        /*
         * If the firmware returned an error, no data was transferred.
         */
        datacnt = 0 ;
    }

    /*
     * Queue the request and response data structures for use with the
     * next EP0 or EP1 request.
     */
    if (ep == 0)
    {
        dev_p->usb_ep0_dma_req = req_p ;
        dev_p->usb_ep0_dma_resp = resp_p ;
    }
    else
    {
        dev_p->usb_ep1_dma_req = req_p ;
        dev_p->usb_ep1_dma_resp = resp_p ;
    }

    /*
     * Call the DMA complete function so we can signal that this portion of the
     * transfer has completed.  If the low level request was canceled, we do not
     * need to signal the completed function as the only way a cancel can happen
     * is via the DMA cancel function.
     */
    if (ret != CY_AS_ERROR_CANCELED)
        CyAsDmaCompletedCallback(dev_p->tag, ep, datacnt, ret) ;
}

/*
 * Set the DRQ mask register for the given endpoint number.  If state is
 * CyTrue, the DRQ interrupt for the given endpoint is enabled, otherwise
 * it is disabled.
 */
static void
CyAsDmaSetDrq(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, CyBool state)
{
    uint16_t mask ;
    uint16_t v ;
    uint32_t intval ;

    /*
     * There are not DRQ register bits for EP0 and EP1
     */
    if (ep == 0 || ep == 1)
        return ;

    /*
     * Disable interrupts while we do this to be sure the state of the
     * DRQ mask register is always well defined.
     */
    intval = CyAsHalDisableInterrupts() ;

    /*
     * Set the DRQ bit to the given state for the ep given
     */
    mask = (1 << ep) ;
    v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_DRQ_MASK) ;

    if (state)
        v |= mask ;
    else
        v &= ~mask ;

    CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_DRQ_MASK, v) ;
    CyAsHalEnableInterrupts(intval) ;
}

/*
* Send the next DMA request for the endpoint given
*/
static void
CyAsDmaSendNextDmaRequest(CyAsDevice *dev_p, CyAsDmaEndPoint *ep_p)
{
    uint32_t datacnt ;
    void *buf_p ;
    CyAsDmaQueueEntry *dma_p ;

    CyAsLogDebugMessage(6, "CyAsDmaSendNextDmaRequest called") ;

    /* If the queue is empty, nothing to do */
    dma_p = ep_p->queue_p ;
    if (dma_p == 0)
    {
        /*
         * There are not pending DMA requests for this endpoint.  Disable
         * the DRQ mask bits to insure no interrupts will be triggered by this
         * endpoint until someone is interested in the data.
         */
        CyAsDmaSetDrq(dev_p, ep_p->ep, CyFalse) ;
        return ;
    }

    CyAsDmaEndPointSetRunning(ep_p) ;

    /*
     * Get the number of words that still need to be xferred in
     * this request.
     */
    datacnt = dma_p->size - dma_p->offset ;
    CyAsHalAssert(datacnt >= 0) ;

    /*
     * The HAL layer should never limit the size of the transfer to
     * something less than the maxhwdata otherwise, the data will be
     * sent in packets that are not correct in size.
     */
    CyAsHalAssert(ep_p->maxhaldata == CY_AS_DMA_MAX_SIZE_HW_SIZE || ep_p->maxhaldata >= ep_p->maxhwdata) ;

    /*
     * Update the number of words that need to be xferred yet
     * based on the limits of the HAL layer.
     */
    if (ep_p->maxhaldata == CY_AS_DMA_MAX_SIZE_HW_SIZE)
    {
        if (datacnt > ep_p->maxhwdata)
            datacnt = ep_p->maxhwdata ;
    }
    else
    {
        if (datacnt > ep_p->maxhaldata)
            datacnt = ep_p->maxhaldata ;
    }

    /*
     * Find a pointer to the data that needs to be transferred
     */
    buf_p = (((char *)dma_p->buf_p) + dma_p->offset);

    /*
     * Mark a request in transit
     */
    CyAsDmaEndPointSetInTransit(ep_p) ;

    if (ep_p->ep == 0 || ep_p->ep == 1)
    {
        /*
         * If this is a WRITE request on EP0 and EP1, we write the data via an EP_DATA request
         * to West Bridge via the mailbox registers.  If this is a READ request, we do nothing and the data will
         * arrive via an EP_DATA request from West Bridge.  In the request handler for the USB context we will pass
         * the data back into the DMA module.
         */
        if (dma_p->readreq == CyFalse)
        {
            uint16_t v ;
            uint16_t len ;
            CyAsLLRequestResponse *resp_p ;
            CyAsLLRequestResponse *req_p ;
            CyAsReturnStatus_t ret ;

            len = (uint16_t)(datacnt / 2) ;
            if (datacnt % 2)
                len++ ;

            len++ ;

            if (ep_p->ep == 0)
            {
                req_p = dev_p->usb_ep0_dma_req ;
                resp_p = dev_p->usb_ep0_dma_resp ;
                dev_p->usb_ep0_dma_req = 0 ;
                dev_p->usb_ep0_dma_resp = 0 ;
            }
            else
            {
                req_p = dev_p->usb_ep1_dma_req ;
                resp_p = dev_p->usb_ep1_dma_resp ;
                dev_p->usb_ep1_dma_req = 0 ;
                dev_p->usb_ep1_dma_resp = 0 ;
            }

            CyAsHalAssert(req_p != 0) ;
            CyAsHalAssert(resp_p != 0) ;
            CyAsHalAssert(len <= 64) ;

            CyAsLLInitRequest(req_p, CY_RQT_USB_EP_DATA, CY_RQT_USB_RQT_CONTEXT, len) ;

            v = (uint16_t)(datacnt | (ep_p->ep << 13) | (1 << 14)) ;
            if (dma_p->offset == 0)
                v |= (1 << 12) ;            /* Set the first packet bit */
            if (dma_p->offset + datacnt == dma_p->size)
                v |= (1 << 11) ;            /* Set the last packet bit */

            CyAsLLRequestResponse_SetWord(req_p, 0, v) ;
            CyAsLLRequestResponse_Pack(req_p, 1, datacnt, buf_p) ;

            CyAsLLInitResponse(resp_p, 1) ;

            ret = CyAsLLSendRequest(dev_p, req_p, resp_p, CyFalse, CyAsDmaRequestCallback) ;
            if (ret == CY_AS_ERROR_SUCCESS)
                CyAsLogDebugMessage(5, "+++ Send EP 0/1 data via mailbox registers") ;
            else
                CyAsLogDebugMessage(5, "+++ Error Sending EP 0/1 data via mailbox registers - CY_AS_ERROR_TIMEOUT") ;

            if (ret != CY_AS_ERROR_SUCCESS)
                CyAsDmaCompletedCallback(dev_p->tag, ep_p->ep, 0, ret) ;
        }
    }
    else
    {
        /*
         * This is a DMA request on an endpoint that is accessible via the P port.  Ask the
         * HAL DMA capabilities to perform this.  The amount of data sent is limited by the
         * HAL max size as well as what we need to send.  If the ep_p->maxhaldata is set to
         * a value larger than the endpoint buffer size, then we will pass more than a single
         * buffer worth of data to the HAL layer and expect the HAL layer to divide the data
         * into packets.  The last parameter here (ep_p->maxhwdata) gives the packet size for
         * the data so the HAL layer knows what the packet size should be.
         */
        if (CyAsDmaEndPointIsDirectionIn(ep_p))
            CyAsHalDmaSetupWrite(dev_p->tag, ep_p->ep, buf_p, datacnt, ep_p->maxhwdata) ;
        else
            CyAsHalDmaSetupRead(dev_p->tag, ep_p->ep, buf_p, datacnt, ep_p->maxhwdata) ;

        /*
         * The DRQ interrupt for this endpoint should be enabled so that the data
         * transfer progresses at interrupt time.
         */
        CyAsDmaSetDrq(dev_p, ep_p->ep, CyTrue) ;
    }
}

/*
 * This function is called when the HAL layer has completed the last requested DMA
 * operation.  This function sends/receives the next batch of data associated with the
 * current DMA request, or it is is complete, moves to the next DMA request.
 */
void
CyAsDmaCompletedCallback(CyAsHalDeviceTag tag, CyAsEndPointNumber_t ep, uint32_t cnt, CyAsReturnStatus_t status)
{
    uint32_t mask ;
    CyAsDmaQueueEntry *req_p ;
    CyAsDmaEndPoint *ep_p ;
    CyAsDevice *dev_p = CyAsDeviceFindFromTag(tag) ;

    /* Make sure the HAL layer gave us good parameters */
    CyAsHalAssert(dev_p != 0) ;
    CyAsHalAssert(dev_p->sig == CY_AS_DEVICE_HANDLE_SIGNATURE) ;
    CyAsHalAssert(ep < 16) ;


    /* Get the endpoint ptr */
    ep_p = CY_AS_NUM_EP(dev_p, ep) ;
    CyAsHalAssert(ep_p->queue_p != 0) ;

    /* Get a pointer to the current entry in the queue */
    mask = CyAsHalDisableInterrupts() ;
    req_p = ep_p->queue_p ;

    /* Update the offset to reflect the data actually received or sent */
    req_p->offset += cnt ;

    /*
     * If we are still sending/receiving the current packet, send/receive the next chunk
     * Basically we keep going if we have not sent/received enough data, and we are not doing
     * a packet operation, and the last packet sent or received was a full sized packet.  In
     * other words, when we are NOT doing a packet operation, a less than full size packet
     * (a short packet) will terminate the operation.
     *
     * Note: If this is EP1 request and the request has timed out, it means the buffer is not free.
     * We have to resend the data.
     *
     * Note: For the MTP data transfers, the DMA transfer for the next packet can only be started
     * asynchronously, after a firmware event notifies that the device is ready.
     */
    if (((req_p->offset != req_p->size) && (req_p->packet == CyFalse) && ((cnt == ep_p->maxhaldata) ||
                    ((cnt == ep_p->maxhwdata) && ((ep != CY_AS_MTP_READ_ENDPOINT) || (cnt == dev_p->usb_max_tx_size)))))
            || ((ep == 1) && (status == CY_AS_ERROR_TIMEOUT)))
    {
        CyAsHalEnableInterrupts(mask) ;

        /*
         * And send the request again to send the next block of data. Special handling for
         * MTP transfers on EPs 2 and 6. The SendNextRequest will be processed based on the
         * event sent by the firmware.
         */
        if ((ep == CY_AS_MTP_WRITE_ENDPOINT) || (
                    (ep == CY_AS_MTP_READ_ENDPOINT) && (!CyAsDmaEndPointIsDirectionIn (ep_p))))
            CyAsDmaEndPointSetStopped(ep_p) ;
        else
            CyAsDmaSendNextDmaRequest(dev_p, ep_p) ;
    }
    else
    {
        /*
         * We get here if ...
         *    we have sent or received all of the data
         *         or
         *    we are doing a packet operation
         *         or
         *    we receive a short packet
         */

        /*
         * Remove this entry from the DMA queue for this endpoint.
         */
        CyAsDmaEndPointClearInTransit(ep_p) ;
        ep_p->queue_p = req_p->next_p ;
        if (ep_p->last_p == req_p)
        {
            /*
             * We have removed the last packet from the DMA queue, disable the
             * interrupt associated with this interrupt.
             */
            ep_p->last_p = 0 ;
            CyAsHalEnableInterrupts(mask) ;
            CyAsDmaSetDrq(dev_p, ep, CyFalse) ;
        }
        else
            CyAsHalEnableInterrupts(mask) ;

        if (req_p->cb)
        {
            /*
             * If the request has a callback associated with it, call the callback
             * to tell the interested party that this DMA request has completed.
             *
             * Note, we set the InCallback bit to insure that we cannot recursively
             * call an API function that is synchronous only from a callback.
             */
            CyAsDeviceSetInCallback(dev_p) ;
            (*req_p->cb)(dev_p, ep, req_p->buf_p, req_p->offset, status) ;
            CyAsDeviceClearInCallback(dev_p) ;
        }

        /*
         * We are done with this request, put it on the freelist to be
         * reused at a later time.
         */
        CyAsDmaAddRequestToFreeQueue(dev_p, req_p) ;

        if (ep_p->queue_p == 0)
        {
            /*
             * If the endpoint is out of DMA entries, set it the endpoint as
             * stopped.
             */
            CyAsDmaEndPointSetStopped(ep_p) ;

            /*
             * The DMA queue is empty, wake any task waiting on the QUEUE to
             * drain.
             */
            if (CyAsDmaEndPointIsSleeping(ep_p))
            {
                CyAsDmaEndPointSetWakeState(ep_p) ;
                CyAsHalWake(&ep_p->channel) ;
            }
        }
        else
        {
            /*
             * If the queued operation is a MTP transfer, wait until firmware event
             * before sending down the next DMA request.
             */
            if ((ep == CY_AS_MTP_WRITE_ENDPOINT) || (
                        (ep == CY_AS_MTP_READ_ENDPOINT) && (!CyAsDmaEndPointIsDirectionIn (ep_p))))
                CyAsDmaEndPointSetStopped(ep_p) ;
            else
                CyAsDmaSendNextDmaRequest(dev_p, ep_p) ;
        }
    }
}

/*
* This function is used to kick start DMA on a given channel.  If DMA is already running
* on the given endpoint, nothing happens.  If DMA is not running, the first entry is pulled
* from the DMA queue and sent/recevied to/from the West Bridge device.
*/
CyAsReturnStatus_t
CyAsDmaKickStart(CyAsDevice *dev_p, CyAsEndPointNumber_t ep)
{
    CyAsDmaEndPoint *ep_p ;
    CyAsHalAssert(dev_p->sig == CY_AS_DEVICE_HANDLE_SIGNATURE) ;

    ep_p = CY_AS_NUM_EP(dev_p, ep) ;

    /* We are already running */
    if (CyAsDmaEndPointIsRunning(ep_p))
        return CY_AS_ERROR_SUCCESS ;

    CyAsDmaSendNextDmaRequest(dev_p, ep_p);
    return CY_AS_ERROR_SUCCESS ;
}

/*
 * This function stops the given endpoint.  Stopping and endpoint cancels
 * any pending DMA operations and frees all resources associated with the
 * given endpoint.
 */
static CyAsReturnStatus_t
CyAsDmaStopEndPoint(CyAsDevice *dev_p, CyAsEndPointNumber_t ep)
{
    CyAsReturnStatus_t ret ;
    CyAsDmaEndPoint *ep_p = CY_AS_NUM_EP(dev_p, ep) ;

    /*
     * Cancel any pending DMA requests associated with this endpoint.  This
     * cancels any DMA requests at the HAL layer as well as dequeues any request
     * that is currently pending.
     */
    ret = CyAsDmaCancel(dev_p, ep, CY_AS_ERROR_CANCELED) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /*
     * Destroy the sleep channel
     */
    if (!CyAsHalDestroySleepChannel(&ep_p->channel) && ret == CY_AS_ERROR_SUCCESS)
        ret = CY_AS_ERROR_DESTROY_SLEEP_CHANNEL_FAILED ;

    /*
     * Free the memory associated with this endpoint
     */
    CyAsHalFree(ep_p) ;

    /*
     * Set the data structure ptr to something sane since the
     * previous pointer is now free.
     */
    dev_p->endp[ep] = 0 ;

    return ret ;
}

/*
 * This method stops the USB stack.  This is an internal function that does
 * all of the work of destroying the USB stack without the protections that
 * we provide to the API (i.e. stopping at stack that is not running).
 */
static CyAsReturnStatus_t
CyAsDmaStopInternal(CyAsDevice *dev_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsReturnStatus_t lret ;
    CyAsEndPointNumber_t i ;

    /*
     * Stop all of the endpoints.  This cancels all DMA requests, and
     * frees all resources associated with each endpoint.
     */
    for(i = 0 ; i < sizeof(dev_p->endp)/(sizeof(dev_p->endp[0])) ; i++)
    {
        lret = CyAsDmaStopEndPoint(dev_p, i) ;
        if (lret != CY_AS_ERROR_SUCCESS && ret == CY_AS_ERROR_SUCCESS)
            ret = lret ;
    }

    /*
     * Now, free the list of DMA requests structures that we use to manage
     * DMA requests.
     */
    while (dev_p->dma_freelist_p)
    {
        CyAsDmaQueueEntry *req_p ;
        uint32_t imask = CyAsHalDisableInterrupts() ;

        req_p = dev_p->dma_freelist_p ;
        dev_p->dma_freelist_p = req_p->next_p ;

        CyAsHalEnableInterrupts(imask) ;

        CyAsHalFree(req_p) ;
    }

    CyAsLLDestroyRequest(dev_p, dev_p->usb_ep0_dma_req) ;
    CyAsLLDestroyRequest(dev_p, dev_p->usb_ep1_dma_req) ;
    CyAsLLDestroyResponse(dev_p, dev_p->usb_ep0_dma_resp) ;
    CyAsLLDestroyResponse(dev_p, dev_p->usb_ep1_dma_resp) ;

    return ret ;
}


/*
 * CyAsDmaStop()
 *
 * This function shuts down the DMA module.  All resources associated with the DMA module
 * will be freed.  This routine is the API stop function.  It insures that we are stopping
 * a stack that is actually running and then calls the internal function to do the work.
 */
CyAsReturnStatus_t
CyAsDmaStop(CyAsDevice *dev_p)
{
    CyAsReturnStatus_t ret ;

    ret = CyAsDmaStopInternal(dev_p) ;
    CyAsDeviceSetDmaStopped(dev_p) ;

    return ret ;
}

/*
 * CyAsDmaStart()
 *
 * This function intializes the DMA module to insure it is up and running.
 */
CyAsReturnStatus_t
CyAsDmaStart(CyAsDevice *dev_p)
{
    CyAsEndPointNumber_t i ;
    uint16_t cnt ;

    if (CyAsDeviceIsDmaRunning(dev_p))
        return CY_AS_ERROR_ALREADY_RUNNING ;

    /*
     * Pre-allocate DMA Queue structures to be used in the interrupt context
     */
    for(cnt = 0 ; cnt < 32 ; cnt++)
    {
        CyAsDmaQueueEntry *entry_p = (CyAsDmaQueueEntry *)CyAsHalAlloc(sizeof(CyAsDmaQueueEntry)) ;
        if (entry_p == 0)
        {
            CyAsDmaStopInternal(dev_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }
        CyAsDmaAddRequestToFreeQueue(dev_p, entry_p) ;
    }

    /*
     * Pre-allocate the DMA requests for sending EP0 and EP1 data to West Bridge
     */
    dev_p->usb_ep0_dma_req = CyAsLLCreateRequest(dev_p, CY_RQT_USB_EP_DATA, CY_RQT_USB_RQT_CONTEXT, 64) ;
    dev_p->usb_ep1_dma_req = CyAsLLCreateRequest(dev_p, CY_RQT_USB_EP_DATA, CY_RQT_USB_RQT_CONTEXT, 64) ;
    if (dev_p->usb_ep0_dma_req == 0 || dev_p->usb_ep1_dma_req == 0)
    {
        CyAsDmaStopInternal(dev_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }
    dev_p->usb_ep0_dma_req_save = dev_p->usb_ep0_dma_req ;

    dev_p->usb_ep0_dma_resp = CyAsLLCreateResponse(dev_p, 1) ;
    dev_p->usb_ep1_dma_resp = CyAsLLCreateResponse(dev_p, 1) ;
    if (dev_p->usb_ep0_dma_resp == 0 || dev_p->usb_ep1_dma_resp == 0)
    {
        CyAsDmaStopInternal(dev_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }
    dev_p->usb_ep0_dma_resp_save = dev_p->usb_ep0_dma_resp ;

    /*
     * Set the dev_p->endp to all zeros to insure cleanup is possible if
     * an error occurs during initialization.
     */
    CyAsHalMemSet(dev_p->endp, 0, sizeof(dev_p->endp)) ;

    /*
     * Now, iterate through each of the endpoints and initialize each
     * one.
     */
    for(i = 0 ; i < sizeof(dev_p->endp)/sizeof(dev_p->endp[0]) ; i++)
    {
        dev_p->endp[i] = (CyAsDmaEndPoint *)CyAsHalAlloc(sizeof(CyAsDmaEndPoint)) ;
        if (dev_p->endp[i] == 0)
        {
            CyAsDmaStopInternal(dev_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }
        CyAsHalMemSet(dev_p->endp[i], 0, sizeof(CyAsDmaEndPoint)) ;

        dev_p->endp[i]->ep = i ;
        dev_p->endp[i]->queue_p = 0 ;
        dev_p->endp[i]->last_p = 0 ;

        CyAsDmaSetDrq(dev_p, i, CyFalse) ;

        if (!CyAsHalCreateSleepChannel(&dev_p->endp[i]->channel))
            return CY_AS_ERROR_CREATE_SLEEP_CHANNEL_FAILED ;
    }

    /*
     * Tell the HAL layer who to call when the HAL layer completes a DMA request
     */
    CyAsHalDmaRegisterCallback(dev_p->tag, CyAsDmaCompletedCallback) ;

    /*
     * Mark DMA as up and running on this device
     */
    CyAsDeviceSetDmaRunning(dev_p) ;

    return CY_AS_ERROR_SUCCESS ;
}

/*
* Wait for all entries in the DMA queue associated the given endpoint to be drained.  This
* function will not return until all the DMA data has been transferred.
*/
CyAsReturnStatus_t
CyAsDmaDrainQueue(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, CyBool kickstart)
{
    CyAsDmaEndPoint *ep_p ;
    int loopcount = 1000 ;
    uint32_t mask ;

    /*
    * Make sure the endpoint is valid
    */
    if (ep >= sizeof(dev_p->endp)/sizeof(dev_p->endp[0]))
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* Get the endpoint pointer based on the endpoint number */
    ep_p = CY_AS_NUM_EP(dev_p, ep) ;

    /*
    * If the endpoint is empty of traffic, we return
    * with success immediately
    */
    mask = CyAsHalDisableInterrupts() ;
    if (ep_p->queue_p == 0)
    {
        CyAsHalEnableInterrupts(mask) ;
        return CY_AS_ERROR_SUCCESS ;
    }
    else
    {
        /*
         * Add 10 seconds to the time out value for each 64 KB segment
         * of data to be transferred.
         */
        if (ep_p->queue_p->size > 0x10000)
            loopcount += ((ep_p->queue_p->size / 0x10000) * 1000) ;
    }
    CyAsHalEnableInterrupts(mask) ;

    /* If we are already sleeping on this endpoint, it is an error */
    if (CyAsDmaEndPointIsSleeping(ep_p))
        return CY_AS_ERROR_NESTED_SLEEP ;

    /*
    * We disable the endpoint while the queue drains to
    * prevent any additional requests from being queued while we are waiting
    */
    CyAsDmaEnableEndPoint(dev_p, ep, CyFalse, CyAsDirectionDontChange) ;

    if (kickstart)
    {
        /*
        * Now, kick start the DMA if necessary
        */
        CyAsDmaKickStart(dev_p, ep) ;
    }

    /*
    * Check one last time before we begin sleeping to see if the
    * queue is drained.
    */
    if (ep_p->queue_p == 0)
    {
        CyAsDmaEnableEndPoint(dev_p, ep, CyTrue, CyAsDirectionDontChange) ;
        return CY_AS_ERROR_SUCCESS ;
    }

    while (loopcount-- > 0)
    {
        /*
         * Sleep for 10 ms maximum (per loop) while waiting for the transfer
         * to complete.
         */
        CyAsDmaEndPointSetSleepState(ep_p) ;
        CyAsHalSleepOn(&ep_p->channel, 10) ;

        /* If we timed out, the sleep bit will still be set */
        CyAsDmaEndPointSetWakeState(ep_p) ;

        /* Check the queue to see if is drained */
        if (ep_p->queue_p == 0)
        {
            /*
             * Clear the endpoint running and in transit flags for the endpoint,
             * now that its DMA queue is empty.
             */
            CyAsDmaEndPointClearInTransit(ep_p) ;
            CyAsDmaEndPointSetStopped(ep_p) ;

            CyAsDmaEnableEndPoint(dev_p, ep, CyTrue, CyAsDirectionDontChange) ;
            return CY_AS_ERROR_SUCCESS ;
        }
    }

    /*
     * The DMA operation that has timed out can be cancelled, so that later
     * operations on this queue can proceed.
     */
    CyAsDmaCancel(dev_p, ep, CY_AS_ERROR_TIMEOUT) ;
    CyAsDmaEnableEndPoint(dev_p, ep, CyTrue, CyAsDirectionDontChange) ;
    return CY_AS_ERROR_TIMEOUT ;
}

/*
* This function queues a write request in the DMA queue for a given endpoint.  The direction of the
* entry will be inferred from the endpoint direction.
*/
CyAsReturnStatus_t
CyAsDmaQueueRequest(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, void *mem_p, uint32_t size, CyBool pkt, CyBool readreq, CyAsDmaCallback cb)
{
    uint32_t mask ;
    CyAsDmaQueueEntry *entry_p ;
    CyAsDmaEndPoint *ep_p ;

    /*
    * Make sure the endpoint is valid
    */
    if (ep >= sizeof(dev_p->endp)/sizeof(dev_p->endp[0]))
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* Get the endpoint pointer based on the endpoint number */
    ep_p = CY_AS_NUM_EP(dev_p, ep) ;

    if (!CyAsDmaEndPointIsEnabled(ep_p))
        return CY_AS_ERROR_ENDPOINT_DISABLED ;

    entry_p = CyAsDmaGetDmaQueueEntry(dev_p) ;

    entry_p->buf_p = mem_p ;
    entry_p->cb = cb ;
    entry_p->size = size ;
    entry_p->offset = 0 ;
    entry_p->packet = pkt ;
    entry_p->readreq = readreq ;

    mask = CyAsHalDisableInterrupts() ;
    entry_p->next_p = 0 ;
    if (ep_p->last_p)
        ep_p->last_p->next_p = entry_p ;
    ep_p->last_p = entry_p ;
    if (ep_p->queue_p == 0)
        ep_p->queue_p = entry_p ;
    CyAsHalEnableInterrupts(mask) ;

    return CY_AS_ERROR_SUCCESS ;
}

/*
* This function enables or disables and endpoint for DMA queueing.  If an endpoint is disabled, any queued
* requests continue to be processed, but no new requests can be queued.
*/
CyAsReturnStatus_t
CyAsDmaEnableEndPoint(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, CyBool enable, CyAsDmaDirection dir)
{
    CyAsDmaEndPoint *ep_p ;

    /*
    * Make sure the endpoint is valid
    */
    if (ep >= sizeof(dev_p->endp)/sizeof(dev_p->endp[0]))
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* Get the endpoint pointer based on the endpoint number */
    ep_p = CY_AS_NUM_EP(dev_p, ep) ;

    if (dir == CyAsDirectionOut)
        CyAsDmaEndPointSetDirectionOut(ep_p) ;
    else if (dir == CyAsDirectionIn)
        CyAsDmaEndPointSetDirectionIn(ep_p) ;

    /*
    * Get the maximum size of data buffer the HAL layer can accept.  This is used when
    * the DMA module is sending DMA requests to the HAL.  The DMA module will never send
    * down a request that is greater than this value.
    *
    * For EP0 and EP1, we can send no more than 64 bytes of data at one time as this is the
    * maximum size of a packet that can be sent via these endpoints.
    */
    if (ep == 0 || ep == 1)
        ep_p->maxhaldata = 64 ;
    else
        ep_p->maxhaldata = CyAsHalDmaMaxRequestSize(dev_p->tag, ep) ;

    if (enable)
        CyAsDmaEndPointEnable(ep_p) ;
    else
        CyAsDmaEndPointDisable(ep_p) ;

    return CY_AS_ERROR_SUCCESS ;
}

/*
 * This function cancels any DMA operations pending with the HAL layer as well
 * as any DMA operation queued on the endpoint.
 */
CyAsReturnStatus_t
CyAsDmaCancel(
    CyAsDevice *dev_p,
    CyAsEndPointNumber_t ep,
    CyAsReturnStatus_t err)
{
    uint32_t mask ;
    CyAsDmaEndPoint *ep_p ;
    CyAsDmaQueueEntry *entry_p ;
    CyBool epstate ;

    /*
     * Make sure the endpoint is valid
     */
    if (ep >= sizeof(dev_p->endp)/sizeof(dev_p->endp[0]))
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* Get the endpoint pointer based on the endpoint number */
    ep_p = CY_AS_NUM_EP(dev_p, ep) ;

    if (ep_p)
    {
        /* Remember the state of the endpoint */
        epstate = CyAsDmaEndPointIsEnabled(ep_p) ;

        /*
         * Disable the endpoint so no more DMA packets can be
         * queued.
         */
        CyAsDmaEnableEndPoint(dev_p, ep, CyFalse, CyAsDirectionDontChange) ;

        /*
         * Don't allow any interrupts from this endpoint while we get the
         * most current request off of the queue.
         */
        CyAsDmaSetDrq(dev_p, ep, CyFalse) ;

        /*
         * Cancel any pending request queued in the HAL layer
         */
        if (CyAsDmaEndPointInTransit(ep_p))
            CyAsHalDmaCancelRequest(dev_p->tag, ep_p->ep) ;

        /*
         * Shutdown the DMA for this endpoint so no more data is transferred
         */
        CyAsDmaEndPointSetStopped(ep_p) ;

        /*
         * Mark the endpoint as not in transit, because we are going to consume
         * any queued requests
         */
        CyAsDmaEndPointClearInTransit(ep_p) ;

        /*
         * Now, remove each entry in the queue and call the associated callback
         * stating that the request was canceled.
         */
        ep_p->last_p = 0 ;
        while (ep_p->queue_p != 0)
        {
            /* Disable interrupts to manipulate the queue */
            mask = CyAsHalDisableInterrupts() ;

            /* Remove an entry from the queue */
            entry_p = ep_p->queue_p ;
            ep_p->queue_p = entry_p->next_p ;

            /* Ok, the queue has been updated, we can turn interrupts back on */
            CyAsHalEnableInterrupts(mask) ;

            /* Call the callback indicating we have canceled the DMA */
            if (entry_p->cb)
                entry_p->cb(dev_p, ep, entry_p->buf_p, entry_p->size, err) ;

            CyAsDmaAddRequestToFreeQueue(dev_p, entry_p) ;
        }

        if (ep == 0 || ep == 1)
        {
            /*
             * If this endpoint is zero or one, we need to clear the queue of any pending
             * CY_RQT_USB_EP_DATA requests as these are pending requests to send data to
             * the West Bridge device.
             */
            CyAsLLRemoveEpDataRequests(dev_p, ep) ;
        }

        if (epstate)
        {
            /*
             * The endpoint started out enabled, so we re-enable the endpoint here.
             */
            CyAsDmaEnableEndPoint(dev_p, ep, CyTrue, CyAsDirectionDontChange) ;
        }
    }

    return CY_AS_ERROR_SUCCESS ;
}

CyAsReturnStatus_t
CyAsDmaReceivedData(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, uint32_t dsize, void *data)
{
    CyAsDmaQueueEntry *dma_p ;
    uint8_t *src_p, *dest_p ;
    CyAsDmaEndPoint *ep_p ;
    uint32_t xfersize ;

    /*
     * Make sure the endpoint is valid
     */
    if (ep != 0 && ep != 1)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* Get the endpoint pointer based on the endpoint number */
    ep_p = CY_AS_NUM_EP(dev_p, ep) ;
    dma_p = ep_p->queue_p ;
    if (dma_p == 0)
        return CY_AS_ERROR_SUCCESS ;

    /*
     * If the data received exceeds the size of the DMA buffer, clip the data to the size
     * of the buffer.  This can lead to loosing some data, but is not different than doing
     * non-packet reads on the other endpoints.
     */
    if (dsize > dma_p->size - dma_p->offset)
        dsize = dma_p->size - dma_p->offset ;

    /*
     * Copy the data from the request packet to the DMA buffer for the endpoint
     */
    src_p = (uint8_t *)data ;
    dest_p = ((uint8_t *)(dma_p->buf_p)) + dma_p->offset ;
    xfersize = dsize ;
    while (xfersize-- > 0)
        *dest_p++ = *src_p++ ;

    /* Signal the DMA module that we have received data for this EP request */
    CyAsDmaCompletedCallback(dev_p->tag, ep, dsize, CY_AS_ERROR_SUCCESS) ;

    return CY_AS_ERROR_SUCCESS ;
}
