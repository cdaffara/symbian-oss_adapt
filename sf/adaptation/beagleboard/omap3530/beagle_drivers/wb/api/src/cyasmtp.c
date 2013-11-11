/* Cypress West Bridge API header file (cyasmtp.h)
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
#include "cyasmtp.h"
#include "cyaserr.h"
#include "cyasdma.h"
#include "cyaslowlevel.h"

static void
CyAsMTPFuncCallback(CyAsDevice *dev_p,
                    uint8_t context,
                    CyAsLLRequestResponse *rqt,
                    CyAsLLRequestResponse *resp,
                    CyAsReturnStatus_t stat) ;

static CyAsReturnStatus_t
IsMTPActive(CyAsDevice *dev_p)
{
    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    if (dev_p->mtp_count == 0)
        return CY_AS_ERROR_NOT_RUNNING ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    return CY_AS_ERROR_SUCCESS ;
}

static void
MyMtpRequestCallback(CyAsDevice *dev_p,
                     uint8_t context,
                     CyAsLLRequestResponse *req_p,
                     CyAsLLRequestResponse *resp_p,
                     CyAsReturnStatus_t ret)
{
    uint16_t val, ev, status ;
    uint16_t mtp_datalen = 0 ;
    uint32_t bytecount_l, bytecount_h ;
    CyAsMTPSendObjectCompleteData sendObjData ;
    CyAsMTPGetObjectCompleteData  getObjData ;
    CyAsDmaEndPoint *ep_p ;

    uint8_t code = CyAsLLRequestResponse_GetCode(req_p) ;

    (void)resp_p ;
    (void)context ;
    (void)ret ;

    switch(code)
    {
    case CY_RQT_MTP_EVENT:
        val = CyAsLLRequestResponse_GetWord(req_p, 0) ;
        status = (val >> 8) & 0xFF ; /* MSB indicates status of read/write */
        ev =   val & 0xFF ; /* event type */
        switch(ev)
        {
        case 0: /* SendObject Complete */
            {
                bytecount_l = CyAsLLRequestResponse_GetWord(req_p, 1) ;
                bytecount_h = CyAsLLRequestResponse_GetWord(req_p, 2) ;
                sendObjData.byte_count = (bytecount_h << 16) | bytecount_l ;

                sendObjData.status = status ;

                /* use the byte count again */
                bytecount_l = CyAsLLRequestResponse_GetWord(req_p, 3) ;
                bytecount_h = CyAsLLRequestResponse_GetWord(req_p, 4) ;
                sendObjData.transaction_id = (bytecount_h << 16) | bytecount_l ;

                dev_p->mtp_turbo_active = CyFalse ;

                if (dev_p->mtp_event_cb)
                    dev_p->mtp_event_cb((CyAsDeviceHandle)dev_p,  CyAsMTPSendObjectComplete,
                                        &sendObjData) ;
            }
            break ;

        case 1: /* GetObject Complete */
            {
                bytecount_l = CyAsLLRequestResponse_GetWord(req_p, 1) ;
                bytecount_h = CyAsLLRequestResponse_GetWord(req_p, 2) ;

                getObjData.byte_count = (bytecount_h << 16) | bytecount_l ;
                getObjData.status = status ;

                dev_p->mtp_turbo_active = CyFalse ;

                if (dev_p->mtp_event_cb)
                    dev_p->mtp_event_cb((CyAsDeviceHandle)dev_p,  CyAsMTPGetObjectComplete,
                                        &getObjData);
            }
            break ;

        case 2: /* BlockTable Needed */
            {
                if (dev_p->mtp_event_cb)
                    dev_p->mtp_event_cb((CyAsDeviceHandle)dev_p,  CyAsMTPBlockTableNeeded, 0);
            }
            break ;
        default:
            CyAsHalPrintMessage("Invalid event type\n") ;
            CyAsLLSendDataResponse(dev_p, CY_RQT_TUR_RQT_CONTEXT, CY_RESP_MTP_INVALID_EVENT, sizeof(ev), &ev) ;
            break ;
        }
        break ;

    case CY_RQT_TURBO_CMD_FROM_HOST:
        {
            mtp_datalen = CyAsLLRequestResponse_GetWord(req_p, 1) ;

            /* Get the endpoint pointer based on the endpoint number */
            ep_p = CY_AS_NUM_EP(dev_p, CY_AS_MTP_READ_ENDPOINT) ;

            /* The event should arrive only after the DMA operation has been queued. */
            CyAsHalAssert(ep_p->queue_p != 0) ;

            /* Put the len in ep data information in dmaqueue and kick start the queue */
            CyAsHalAssert(ep_p->queue_p->size >= mtp_datalen) ;

            if (mtp_datalen == 0)
            {
                CyAsDmaCompletedCallback(dev_p->tag, CY_AS_MTP_READ_ENDPOINT, 0, CY_AS_ERROR_SUCCESS) ;
            }
            else
            {
                ep_p->maxhwdata = mtp_datalen ;

                /*
                 * Make sure that the DMA status for this EP is not running, so that
                 * the call to CyAsDmaKickStart gets this transfer going.
                 * Note: In MTP mode, we never leave a DMA transfer of greater than one
                 * packet running. So, it is okay to override the status here and start
                 * the next packet transfer.
                 */
                CyAsDmaEndPointSetStopped(ep_p) ;

                /* Kick start the queue if it is not running */
                CyAsDmaKickStart(dev_p, CY_AS_MTP_READ_ENDPOINT) ;
            }
        }
        break ;

    case CY_RQT_TURBO_START_WRITE_DMA:
        {
            /*
             * Now that the firmware is ready to receive the next packet of data, start
             * the corresponding DMA transfer.  First, ensure that a DMA operation is still
             * pending in the queue for the write endpoint.
             */
            CyAsLLSendStatusResponse(dev_p, CY_RQT_TUR_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;

            ep_p = CY_AS_NUM_EP(dev_p, CY_AS_MTP_WRITE_ENDPOINT) ;
            CyAsHalAssert (ep_p->queue_p != 0) ;

            CyAsDmaEndPointSetStopped(ep_p) ;
            CyAsDmaKickStart(dev_p, CY_AS_MTP_WRITE_ENDPOINT) ;
        }
        break ;

    default:
        CyAsHalPrintMessage("Invalid request received on TUR context\n") ;
        val = req_p->box0 ;
        CyAsLLSendDataResponse(dev_p, CY_RQT_TUR_RQT_CONTEXT, CY_RESP_INVALID_REQUEST, sizeof(val), &val) ;
        break ;
    }
}

static CyAsReturnStatus_t
MyHandleResponseNoData(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseMTPStart(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         CyAsReturnStatus_t ret)
{
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    dev_p->mtp_count++ ;

    CyAsDmaEnableEndPoint(dev_p, CY_AS_MTP_READ_ENDPOINT, CyTrue, CyAsDirectionOut) ;
    dev_p->usb_config[CY_AS_MTP_READ_ENDPOINT].enabled = CyTrue ;
    dev_p->usb_config[CY_AS_MTP_READ_ENDPOINT].dir = CyAsUsbOut ;
    dev_p->usb_config[CY_AS_MTP_READ_ENDPOINT].type = CyAsUsbBulk ;

    CyAsDmaEnableEndPoint(dev_p, CY_AS_MTP_WRITE_ENDPOINT, CyTrue, CyAsDirectionIn) ;
    dev_p->usb_config[CY_AS_MTP_WRITE_ENDPOINT].enabled = CyTrue ;
    dev_p->usb_config[CY_AS_MTP_WRITE_ENDPOINT].dir = CyAsUsbIn ;
    dev_p->usb_config[CY_AS_MTP_WRITE_ENDPOINT].type = CyAsUsbBulk ;

    CyAsDmaSetMaxDmaSize(dev_p, 0x02, 0x0200) ; /* Packet size is 512 bytes */
    CyAsDmaSetMaxDmaSize(dev_p, 0x06, 0x40) ;   /* Packet size is 64 bytes until a switch to high speed happens. */

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    if (ret != CY_AS_ERROR_SUCCESS)
        CyAsLLRegisterRequestCallback(dev_p, CY_RQT_TUR_RQT_CONTEXT, 0) ;

    CyAsDeviceClearMSSPending(dev_p) ;

    return ret ;
}


CyAsReturnStatus_t
CyAsMTPStart(CyAsDeviceHandle handle,
             CyAsMTPEventCallback eventCB,
             CyAsFunctionCallback cb,
             uint32_t client
             )
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice* dev_p ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    if(CyAsDeviceIsMSSPending(dev_p))
        return CY_AS_ERROR_STARTSTOP_PENDING ;

    if (dev_p->storage_count == 0)
        return CY_AS_ERROR_NOT_RUNNING ;

    if (dev_p->usb_count == 0)
        return CY_AS_ERROR_NOT_RUNNING ;

    if (dev_p->is_mtp_firmware == 0)
        return CY_AS_ERROR_NOT_SUPPORTED ;

    CyAsDeviceSetMSSPending(dev_p) ;

    if (dev_p->mtp_count == 0)
    {

        dev_p->mtp_event_cb = eventCB ;
        /*
        * We register here becuase the start request may cause events to occur before the
        * response to the start request.
        */
        CyAsLLRegisterRequestCallback(dev_p, CY_RQT_TUR_RQT_CONTEXT, MyMtpRequestCallback) ;

        /* Create the request to send to the West Bridge device */
        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_START_MTP, CY_RQT_TUR_RQT_CONTEXT, 0) ;
        if (req_p == 0)
        {
            CyAsDeviceClearMSSPending(dev_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        /* Reserve space for the reply, the reply data will not exceed one word */
        reply_p = CyAsLLCreateResponse(dev_p, 1) ;
        if (reply_p == 0)
        {
            CyAsLLDestroyRequest(dev_p, req_p) ;
            CyAsDeviceClearMSSPending(dev_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        if(cb == 0)
        {
            ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            return MyHandleResponseMTPStart(dev_p, req_p, reply_p, ret) ;
        }
        else
        {
            ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MTP_START,
                0, dev_p->func_cbs_mtp, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
                CyAsMTPFuncCallback) ;

            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            return ret ;
        }

destroy:
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    }
    else
    {
        dev_p->mtp_count++ ;
        if (cb)
            cb(handle, ret, client, CY_FUNCT_CB_MTP_START, 0) ;
    }

    CyAsDeviceClearMSSPending(dev_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseMTPStop(CyAsDevice* dev_p,
                        CyAsLLRequestResponse *req_p,
                        CyAsLLRequestResponse *reply_p,
                        CyAsReturnStatus_t ret)
{
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    /*
    * We sucessfully shutdown the stack, so decrement to make the count
    * zero.
    */
    dev_p->mtp_count-- ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    if (ret != CY_AS_ERROR_SUCCESS)
        CyAsLLRegisterRequestCallback(dev_p, CY_RQT_TUR_RQT_CONTEXT, 0) ;

    CyAsDeviceClearMSSPending(dev_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsMTPStop(CyAsDeviceHandle handle,
            CyAsFunctionCallback cb,
            uint32_t client
            )
{
    CyAsLLRequestResponse *req_p = 0, *reply_p = 0 ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsMTPStop called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsMTPActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    if(CyAsDeviceIsMSSPending(dev_p))
        return CY_AS_ERROR_STARTSTOP_PENDING ;

    CyAsDeviceSetMSSPending(dev_p) ;

    if (dev_p->mtp_count == 1)
    {
        /* Create the request to send to the West Bridge device */
        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_STOP_MTP, CY_RQT_TUR_RQT_CONTEXT, 0) ;
        if (req_p == 0)
        {
            ret = CY_AS_ERROR_OUT_OF_MEMORY ;
            goto destroy ;
        }

        /* Reserve space for the reply, the reply data will not exceed one word */
        reply_p = CyAsLLCreateResponse(dev_p, 1) ;
        if (reply_p == 0)
        {
            ret = CY_AS_ERROR_OUT_OF_MEMORY ;
            goto destroy ;
        }

        if(cb == 0)
        {
            ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            return MyHandleResponseMTPStop(dev_p, req_p, reply_p, ret) ;
        }
        else
        {
            ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MTP_STOP,
                0, dev_p->func_cbs_mtp, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
                CyAsMTPFuncCallback) ;

            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            return ret ;
        }

destroy:
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    }
    else if (dev_p->mtp_count > 1)
    {

        dev_p->mtp_count-- ;

        if (cb)
            cb(handle, ret, client, CY_FUNCT_CB_MTP_STOP, 0) ;
    }

    CyAsDeviceClearMSSPending(dev_p) ;

    return ret ;
}

static void
MtpWriteCallback(
        CyAsDevice *dev_p,
        uint8_t context,
        CyAsLLRequestResponse *rqt,
        CyAsLLRequestResponse *resp,
        CyAsReturnStatus_t ret)
{
    CyAsHalAssert(context == CY_RQT_TUR_RQT_CONTEXT) ;

    if (ret == CY_AS_ERROR_SUCCESS)
    {
        if (CyAsLLRequestResponse_GetCode(resp) != CY_RESP_SUCCESS_FAILURE)
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        else
            ret = CyAsLLRequestResponse_GetWord(resp, 0) ;
    }

    if (ret != CY_AS_ERROR_SUCCESS)
    {
        /* Firmware failed the request. Cancel the DMA transfer. */
        CyAsDmaCancel(dev_p, 0x04, CY_AS_ERROR_CANCELED) ;
        CyAsDeviceClearStorageAsyncPending(dev_p) ;
    }

    CyAsLLDestroyResponse(dev_p, resp) ;
    CyAsLLDestroyRequest(dev_p, rqt) ;
}

static void
AsyncWriteRequestCallback(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, void *buf_p, uint32_t size, CyAsReturnStatus_t err)
{
    CyAsDeviceHandle h ;
    CyAsFunctionCallback cb ;

    (void)size ;
    (void)buf_p ;
    (void)ep ;


    CyAsLogDebugMessage(6, "AsyncWriteRequestCallback called") ;

    h = (CyAsDeviceHandle)dev_p ;

    cb = dev_p->mtp_cb ;
    dev_p->mtp_cb = 0 ;

    CyAsDeviceClearStorageAsyncPending(dev_p) ;

    if (cb)
        cb(h, err, dev_p->mtp_client, dev_p->mtp_op, 0) ;

}

static void
SyncMTPCallback(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, void *buf_p, uint32_t size, CyAsReturnStatus_t err)
{
    (void)ep ;
    (void)buf_p ;
    (void)size ;

    dev_p->mtp_error = err ;
}

static CyAsReturnStatus_t
CyAsMTPOperation(CyAsDevice *dev_p,
                 CyAsMTPBlockTable* blk_table,
                 uint32_t num_bytes,
                 uint32_t transaction_id,
                 CyAsFunctionCallback cb,
                 uint32_t client,
                 uint8_t rqttype
                 )
{
    CyAsLLRequestResponse *req_p = 0, *reply_p = 0 ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint32_t mask = 0 ;
    CyAsFunctCBType mtp_cb_op = (CyAsFunctCBType)0 ;
    uint16_t size = 2 ;

        if(dev_p->mtp_count == 0)
                return CY_AS_ERROR_NOT_RUNNING ;

    if (rqttype == CY_RQT_INIT_SEND_OBJECT)
    {
        mtp_cb_op = CY_FUNCT_CB_MTP_INIT_SEND_OBJECT ;
        dev_p->mtp_turbo_active = CyTrue ;
    }
    else if (rqttype == CY_RQT_INIT_GET_OBJECT)
    {
        mtp_cb_op = CY_FUNCT_CB_MTP_INIT_GET_OBJECT ;
        dev_p->mtp_turbo_active = CyTrue ;
    }
    else
        mtp_cb_op = CY_FUNCT_CB_MTP_SEND_BLOCK_TABLE ;

    ret = IsMTPActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CY_RQT_INIT_GET_OBJECT == rqttype)
    {
        size = 4 ;
    }
    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, rqttype, CY_RQT_TUR_RQT_CONTEXT, size) ;
    if (req_p == 0)
    {
        ret = CY_AS_ERROR_OUT_OF_MEMORY ;
        goto destroy ;
    }

    /* Reserve space for the reply, the reply data will not exceed one word */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        ret = CY_AS_ERROR_OUT_OF_MEMORY ;
        goto destroy ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)(num_bytes & 0xFFFF)) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((num_bytes >> 16) & 0xFFFF)) ;

    /* If it is GET_OBJECT, send transaction id as well */
    if (CY_RQT_INIT_GET_OBJECT == rqttype)
    {
        CyAsLLRequestResponse_SetWord(req_p, 2, (uint16_t)(transaction_id & 0xFFFF)) ;
        CyAsLLRequestResponse_SetWord(req_p, 3, (uint16_t)((transaction_id >> 16) & 0xFFFF)) ;
    }

    if(cb == 0)
    {
        /* Queue the DMA request for block table write */
        ret = CyAsDmaQueueRequest(dev_p, 4, blk_table, sizeof(CyAsMTPBlockTable), CyFalse, CyFalse, SyncMTPCallback) ;

        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
        {
            CyAsDmaCancel(dev_p, 4, CY_AS_ERROR_CANCELED) ;
            CyAsDeviceClearStorageAsyncPending(dev_p) ;

            goto destroy ;
        }

        ret = CyAsDmaDrainQueue(dev_p, 4, CyTrue) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        ret = dev_p->mtp_error ;
        goto destroy ;
    }
    else
    {
#if 0
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MTP_INIT_SEND_OBJECT,
            0, dev_p->func_cbs_mtp, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMTPFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;
#endif

        /* Protection from interrupt driven code */
        /* since we are using storage EP4 check if any storage activity is pending */
        mask = CyAsHalDisableInterrupts() ;
        if ((CyAsDeviceIsStorageAsyncPending(dev_p)) || (dev_p->storage_wait))
        {
            CyAsHalEnableInterrupts(mask) ;
            return CY_AS_ERROR_ASYNC_PENDING ;
        }
        CyAsDeviceSetStorageAsyncPending(dev_p) ;
        CyAsHalEnableInterrupts(mask) ;

        dev_p->mtp_cb     = cb ;
        dev_p->mtp_client = client ;
        dev_p->mtp_op     = mtp_cb_op ;

        ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyFalse, MtpWriteCallback) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        ret = CyAsDmaQueueRequest(dev_p, 4, blk_table, sizeof(CyAsMTPBlockTable), CyFalse, CyFalse, AsyncWriteRequestCallback) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            return ret ;

        /* Kick start the queue if it is not running */
        CyAsDmaKickStart(dev_p, 4) ;

        return CY_AS_ERROR_SUCCESS ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsMTPInitSendObject(CyAsDeviceHandle handle,
                      CyAsMTPBlockTable* blk_table,
                      uint32_t num_bytes,
                      CyAsFunctionCallback cb,
                      uint32_t client
                      )
{
    CyAsDevice* dev_p ;
    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    return CyAsMTPOperation(dev_p, blk_table, num_bytes, 0, cb, client, CY_RQT_INIT_SEND_OBJECT) ;

}

CyAsReturnStatus_t
CyAsMTPInitGetObject(CyAsDeviceHandle handle,
                     CyAsMTPBlockTable* blk_table,
                     uint32_t num_bytes,
                     uint32_t transaction_id,
                     CyAsFunctionCallback cb,
                     uint32_t client
                     )
{
    CyAsDevice* dev_p ;
    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    return CyAsMTPOperation(dev_p, blk_table, num_bytes, transaction_id, cb, client, CY_RQT_INIT_GET_OBJECT) ;

}

static CyAsReturnStatus_t
MyHandleResponseCancelSendObject(CyAsDevice* dev_p,
                                 CyAsLLRequestResponse *req_p,
                                 CyAsLLRequestResponse *reply_p,
                                 CyAsReturnStatus_t ret)
{
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;


destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsMTPCancelSendObject(CyAsDeviceHandle handle,
                        CyAsFunctionCallback cb,
                        uint32_t client
                        )
{
    CyAsLLRequestResponse *req_p = 0, *reply_p = 0 ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice*dev_p ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

        if(dev_p->mtp_count == 0)
                return CY_AS_ERROR_NOT_RUNNING ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_CANCEL_SEND_OBJECT, CY_RQT_TUR_RQT_CONTEXT, 0) ;
    if (req_p == 0)
    {
        ret = CY_AS_ERROR_OUT_OF_MEMORY ;
        goto destroy ;
    }

    /* Reserve space for the reply, the reply data will not exceed one word */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        ret = CY_AS_ERROR_OUT_OF_MEMORY ;
        goto destroy ;
    }

    if(cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return MyHandleResponseCancelSendObject(dev_p, req_p, reply_p, ret) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MTP_CANCEL_SEND_OBJECT,
            0, dev_p->func_cbs_mtp, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMTPFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseCancelGetObject(CyAsDevice* dev_p,
                                CyAsLLRequestResponse *req_p,
                                CyAsLLRequestResponse *reply_p,
                                CyAsReturnStatus_t ret)
{
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;


destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsMTPCancelGetObject(CyAsDeviceHandle handle,
                       CyAsFunctionCallback cb,
                       uint32_t client
                       )
{
    CyAsLLRequestResponse *req_p = 0, *reply_p = 0 ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice*dev_p ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

        if(dev_p->mtp_count == 0)
                return CY_AS_ERROR_NOT_RUNNING ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_CANCEL_GET_OBJECT, CY_RQT_TUR_RQT_CONTEXT, 0) ;
    if (req_p == 0)
    {
        ret = CY_AS_ERROR_OUT_OF_MEMORY ;
        goto destroy ;
    }

    /* Reserve space for the reply, the reply data will not exceed one word */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        ret = CY_AS_ERROR_OUT_OF_MEMORY ;
        goto destroy ;
    }

    if(cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return MyHandleResponseCancelGetObject(dev_p, req_p, reply_p, ret) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MTP_CANCEL_GET_OBJECT,
            0, dev_p->func_cbs_mtp, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMTPFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsMTPSendBlockTable(CyAsDeviceHandle handle,
                      CyAsMTPBlockTable* blk_table,
                      CyAsFunctionCallback cb,
                      uint32_t client
                      )
{
    CyAsDevice* dev_p ;
    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    return CyAsMTPOperation(dev_p, blk_table, 0, 0, cb, client, CY_RQT_SEND_BLOCK_TABLE) ;
}

static void
CyAsMTPFuncCallback(CyAsDevice *dev_p,
                    uint8_t context,
                    CyAsLLRequestResponse *rqt,
                    CyAsLLRequestResponse *resp,
                    CyAsReturnStatus_t stat)
{
    CyAsFuncCBNode*     node = (CyAsFuncCBNode*)dev_p->func_cbs_mtp->head_p ;
    CyAsReturnStatus_t  ret = CY_AS_ERROR_SUCCESS ;
    uint8_t             code ;
    CyBool delay_callback = CyFalse ;

    CyAsHalAssert(dev_p->func_cbs_mtp->count != 0) ;
    CyAsHalAssert(dev_p->func_cbs_mtp->type == CYAS_FUNC_CB) ;

    (void)context ;

    /* The Handlers are responsible for Deleting the rqt and resp when
    * they are finished
    */
    code = CyAsLLRequestResponse_GetCode(rqt) ;
    switch(code)
    {
    case CY_RQT_START_MTP:
        ret = MyHandleResponseMTPStart(dev_p, rqt, resp, stat) ;
        break ;
    case CY_RQT_STOP_MTP:
        ret = MyHandleResponseMTPStop(dev_p, rqt, resp, stat) ;
        break ;
#if 0
    case CY_RQT_INIT_SEND_OBJECT:
        ret = MyHandleResponseInitSendObject(dev_p, rqt, resp, stat, CyTrue) ;
        delay_callback = CyTrue ;
        break ;
#endif
    case CY_RQT_CANCEL_SEND_OBJECT:
        ret = MyHandleResponseCancelSendObject(dev_p, rqt, resp, stat) ;
        break ;
#if 0
    case CY_RQT_INIT_GET_OBJECT:
        ret = MyHandleResponseInitGetObject(dev_p, rqt, resp, stat, CyTrue) ;
        delay_callback = CyTrue ;
        break ;
#endif
    case CY_RQT_CANCEL_GET_OBJECT:
        ret = MyHandleResponseCancelGetObject(dev_p, rqt, resp, stat) ;
        break ;
#if 0
    case CY_RQT_SEND_BLOCK_TABLE:
        ret = MyHandleResponseSendBlockTable(dev_p, rqt, resp, stat, CyTrue) ;
        delay_callback = CyTrue ;
        break ;
#endif
    case CY_RQT_ENABLE_USB_PATH:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        if (ret == CY_AS_ERROR_SUCCESS)
            dev_p->is_storage_only_mode = CyFalse ;
        break ;
    default:
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        CyAsHalAssert(CyFalse) ;
        break ;
    }

    /*
    * If the low level layer returns a direct error, use the corresponding error code.
    * If not, use the error code based on the response from firmware.
    */
    if (stat == CY_AS_ERROR_SUCCESS)
        stat = ret ;

    if(!delay_callback)
    {
        node->cb_p((CyAsDeviceHandle)dev_p, stat, node->client_data, (CyAsFunctCBType)node->dataType, node->data) ;
        CyAsRemoveCBNode(dev_p->func_cbs_mtp) ;
    }
}

CyAsReturnStatus_t
CyAsMTPStorageOnlyStart(CyAsDeviceHandle handle)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    if (dev_p->storage_count == 0)
        return CY_AS_ERROR_NOT_RUNNING ;

    dev_p->is_storage_only_mode = CyTrue ;
    return CY_AS_ERROR_SUCCESS ;
}

CyAsReturnStatus_t
CyAsMTPStorageOnlyStop(CyAsDeviceHandle handle,
                        CyAsFunctionCallback cb,
                        uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    if (dev_p->storage_count == 0)
        return CY_AS_ERROR_NOT_RUNNING ;

    if (dev_p->is_storage_only_mode == CyFalse)
        return CY_AS_ERROR_SUCCESS ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_ENABLE_USB_PATH, CY_RQT_TUR_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    if(cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        ret = MyHandleResponseNoData(dev_p, req_p, reply_p) ;
        if (ret == CY_AS_ERROR_SUCCESS)
            dev_p->is_storage_only_mode = CyFalse ;
        return ret ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MTP_STOP_STORAGE_ONLY,
            0, dev_p->func_cbs_mtp, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMTPFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}
