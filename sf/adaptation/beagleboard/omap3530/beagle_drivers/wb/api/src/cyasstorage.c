/* Cypress West Bridge API source file (cyasstorage.c)
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

/*
* Storage Design
*
* The storage module is fairly straight forward once the DMA and LOWLEVEL modules
* have been designed.  The storage module simple takes requests from the user, queues the
* associated DMA requests for action, and then sends the low level requests to the
* West Bridge firmware.
*
*/

#include "cyashal.h"
#include "cyasstorage.h"
#include "cyaserr.h"
#include "cyasdevice.h"
#include "cyaslowlevel.h"
#include "cyasdma.h"
#include "cyasregs.h"

/* Map a pre-V1.2 media type to the V1.2+ bus number */
CyAsReturnStatus_t
CyAnMapBusFromMediaType(CyAsDevice *dev_p, CyAsMediaType type, CyAsBusNumber_t *bus)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t code = (uint8_t)(1 << type) ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;


    if (dev_p->media_supported[0] & code)
    {
        if (dev_p->media_supported[1] & code)
        {
            /*
             * This media type could be supported on multiple buses. So, report
             * an address resolution error.
             */
            ret = CY_AS_ERROR_ADDRESS_RESOLUTION_ERROR ;
        }
        else
            *bus = 0 ;
    }
    else
    {
        if (dev_p->media_supported[1] & code)
            *bus = 1 ;
        else
            ret = CY_AS_ERROR_NO_SUCH_MEDIA ;
    }

    return ret ;
}

static uint16_t
CreateAddress(CyAsBusNumber_t bus, uint32_t device, uint8_t unit)
{
    CyAsHalAssert(bus >= 0  && bus < CY_AS_MAX_BUSES) ;
    CyAsHalAssert(device < 16) ;

    return (uint16_t)(((uint8_t)bus << 12) | (device << 8) | unit) ;
}

CyAsMediaType
CyAsStorageGetMediaFromAddress(uint16_t v)
{
    CyAsMediaType media = CyAsMediaMaxMediaValue ;

    switch(v & 0xFF)
    {
        case 0x00:
            break;
        case 0x01:
            media = CyAsMediaNand ;
            break ;
        case 0x02:
            media = CyAsMediaSDFlash ;
            break ;
        case 0x04:
            media = CyAsMediaMMCFlash ;
            break ;
        case 0x08:
            media = CyAsMediaCEATA ;
            break ;
        case 0x10:
            media = CyAsMediaSDIO ;
            break ;
        default:
            CyAsHalAssert(0) ;
            break ;
    }

    return media ;
}

CyAsBusNumber_t
CyAsStorageGetBusFromAddress(uint16_t v)
{
    CyAsBusNumber_t bus = (CyAsBusNumber_t)((v >> 12) & 0x0f) ;
    CyAsHalAssert(bus >= 0 && bus < CY_AS_MAX_BUSES) ;
    return bus ;
}

uint32_t
CyAsStorageGetDeviceFromAddress(uint16_t v)
{
    return (uint32_t)((v >> 8) & 0x0f) ;
}

static uint8_t
GetUnitFromAddress(uint16_t v)
{
    return (uint8_t)(v & 0xff) ;
}

static CyAsReturnStatus_t
CyAsMapBadAddr(uint16_t val)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_INVALID_RESPONSE ;

    switch(val)
    {
    case 0:
        ret = CY_AS_ERROR_NO_SUCH_BUS ;
        break ;
    case 1:
        ret = CY_AS_ERROR_NO_SUCH_DEVICE ;
        break ;
    case 2:
        ret = CY_AS_ERROR_NO_SUCH_UNIT ;
        break ;
    case 3:
        ret = CY_AS_ERROR_INVALID_BLOCK ;
        break ;
    }

    return ret ;
}

static void
MyStorageRequestCallback(CyAsDevice *dev_p, uint8_t context, CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *resp_p, CyAsReturnStatus_t ret)
{
    uint16_t val ;
    uint16_t addr ;
    CyAsBusNumber_t bus;
    uint32_t device;
    CyAsDeviceHandle h = (CyAsDeviceHandle)dev_p ;
    CyAsDmaEndPoint *ep_p = NULL ;

    (void)resp_p ;
    (void)context ;
    (void)ret ;

    switch(CyAsLLRequestResponse_GetCode(req_p))
    {
    case CY_RQT_MEDIA_CHANGED:
        CyAsLLSendStatusResponse(dev_p, CY_RQT_STORAGE_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;

        /* Media has either been inserted or removed */
        addr = CyAsLLRequestResponse_GetWord(req_p, 0) ;

        bus = CyAsStorageGetBusFromAddress(addr);
        device = CyAsStorageGetDeviceFromAddress(addr);

        /* Clear the entry for this device to force re-query later */
        CyAsHalMemSet(&(dev_p->storage_device_info[bus][device]), 0,
            sizeof(dev_p->storage_device_info[bus][device])) ;

        val = CyAsLLRequestResponse_GetWord(req_p, 1) ;
        if (dev_p->storage_event_cb_ms)
        {
            if (val == 1)
                dev_p->storage_event_cb_ms(h, bus, device, CyAsStorageRemoved, 0) ;
            else
                dev_p->storage_event_cb_ms(h, bus, device, CyAsStorageInserted, 0) ;
        }
        else if (dev_p->storage_event_cb)
        {
            if (val == 1)
                dev_p->storage_event_cb(h, (CyAsMediaType)bus, CyAsStorageRemoved, 0) ; /*nxz*/
            else
                dev_p->storage_event_cb(h, (CyAsMediaType)bus, CyAsStorageInserted, 0) ; /*nxz*/
        }

        break ;

    case CY_RQT_ANTIOCH_CLAIM:
        CyAsLLSendStatusResponse(dev_p, CY_RQT_STORAGE_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        if (dev_p->storage_event_cb || dev_p->storage_event_cb_ms)
        {
            val = CyAsLLRequestResponse_GetWord(req_p, 0) ;
            if (dev_p->storage_event_cb_ms)
            {
                if (val & 0x0100)
                    dev_p->storage_event_cb_ms(h, 0, 0, CyAsStorageAntioch, 0) ;
                if (val & 0x0200)
                    dev_p->storage_event_cb_ms(h, 1, 0, CyAsStorageAntioch, 0) ;
            }
            else
            {
                if (val & 0x01)
                    dev_p->storage_event_cb(h, CyAsMediaNand, CyAsStorageAntioch, 0) ;
                if (val & 0x02)
                    dev_p->storage_event_cb(h, CyAsMediaSDFlash, CyAsStorageAntioch, 0) ;
                if (val & 0x04)
                    dev_p->storage_event_cb(h, CyAsMediaMMCFlash, CyAsStorageAntioch, 0) ;
                if (val & 0x08)
                    dev_p->storage_event_cb(h, CyAsMediaCEATA, CyAsStorageAntioch, 0) ;
            }
        }
        break ;

    case CY_RQT_ANTIOCH_RELEASE:
        CyAsLLSendStatusResponse(dev_p, CY_RQT_STORAGE_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        val = CyAsLLRequestResponse_GetWord(req_p, 0) ;
        if (dev_p->storage_event_cb_ms)
        {
            if (val & 0x0100)
                dev_p->storage_event_cb_ms(h, 0, 0, CyAsStorageProcessor, 0) ;
            if (val & 0x0200)
                dev_p->storage_event_cb_ms(h, 1, 0, CyAsStorageProcessor, 0) ;
        }
        else if (dev_p->storage_event_cb)
        {
            if (val & 0x01)
                dev_p->storage_event_cb(h, CyAsMediaNand, CyAsStorageProcessor, 0) ;
            if (val & 0x02)
                dev_p->storage_event_cb(h, CyAsMediaSDFlash, CyAsStorageProcessor, 0) ;
            if (val & 0x04)
                dev_p->storage_event_cb(h, CyAsMediaMMCFlash, CyAsStorageProcessor, 0) ;
            if (val & 0x08)
                dev_p->storage_event_cb(h, CyAsMediaCEATA, CyAsStorageProcessor, 0) ;
        }
        break ;


    case CY_RQT_SDIO_INTR:
        CyAsLLSendStatusResponse(dev_p, CY_RQT_STORAGE_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        val = CyAsLLRequestResponse_GetWord(req_p, 0) ;
        if (dev_p->storage_event_cb_ms)
        {
            if (val & 0x0100)
                dev_p->storage_event_cb_ms(h, 1, 0, CyAsSdioInterrupt, 0) ;
            else
                dev_p->storage_event_cb_ms(h, 0, 0, CyAsSdioInterrupt, 0) ;

        }
        else if (dev_p->storage_event_cb)
        {
            dev_p->storage_event_cb(h, CyAsMediaSDIO, CyAsSdioInterrupt, 0) ;
        }
        break;

    case CY_RQT_P2S_DMA_START:
        /* Do the DMA setup for the waiting operation. This event is used only in
         * the MTP mode firmware. */
        CyAsLLSendStatusResponse(dev_p, CY_RQT_STORAGE_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        if (dev_p->storage_oper == CyAsOpRead)
        {
            ep_p = CY_AS_NUM_EP(dev_p, CY_AS_P2S_READ_ENDPOINT) ;
            CyAsDmaEndPointSetStopped(ep_p) ;
            CyAsDmaKickStart(dev_p, CY_AS_P2S_READ_ENDPOINT) ;
        }
        else
        {
            ep_p = CY_AS_NUM_EP(dev_p, CY_AS_P2S_WRITE_ENDPOINT) ;
            CyAsDmaEndPointSetStopped(ep_p) ;
            CyAsDmaKickStart(dev_p, CY_AS_P2S_WRITE_ENDPOINT) ;
        }
        break ;

    default:
        CyAsHalPrintMessage("Invalid request received on storage context\n") ;
        val = req_p->box0 ;
        CyAsLLSendDataResponse(dev_p, CY_RQT_STORAGE_RQT_CONTEXT, CY_RESP_INVALID_REQUEST, sizeof(val), &val) ;
        break ;
    }
}

static CyAsReturnStatus_t
IsStorageActive(CyAsDevice *dev_p)
{
    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    if (dev_p->storage_count == 0)
        return CY_AS_ERROR_NOT_RUNNING ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    return CY_AS_ERROR_SUCCESS ;
}

static void
CyAsStorageFuncCallback(CyAsDevice *dev_p,
                        uint8_t context,
                        CyAsLLRequestResponse *rqt,
                        CyAsLLRequestResponse *resp,
                        CyAsReturnStatus_t ret) ;

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
MyHandleResponseStorageStart(CyAsDevice* dev_p,
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
    if (dev_p->storage_count > 0 && ret == CY_AS_ERROR_ALREADY_RUNNING)
        ret = CY_AS_ERROR_SUCCESS ;

    ret = CyAsDmaEnableEndPoint(dev_p, CY_AS_P2S_WRITE_ENDPOINT, CyTrue, CyAsDirectionIn) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    ret = CyAsDmaSetMaxDmaSize(dev_p, CY_AS_P2S_WRITE_ENDPOINT, CY_AS_STORAGE_EP_SIZE) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    ret = CyAsDmaEnableEndPoint(dev_p, CY_AS_P2S_READ_ENDPOINT, CyTrue, CyAsDirectionOut) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    ret = CyAsDmaSetMaxDmaSize(dev_p, CY_AS_P2S_READ_ENDPOINT, CY_AS_STORAGE_EP_SIZE) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    CyAsLLRegisterRequestCallback(dev_p, CY_RQT_STORAGE_RQT_CONTEXT, MyStorageRequestCallback) ;

    /* Create the request/response used for storage reads and writes. */
    dev_p->storage_rw_req_p  = CyAsLLCreateRequest(dev_p, 0, CY_RQT_STORAGE_RQT_CONTEXT, 5) ;
    if (dev_p->storage_rw_req_p == 0)
    {
        ret = CY_AS_ERROR_OUT_OF_MEMORY;
        goto destroy;
    }

    dev_p->storage_rw_resp_p = CyAsLLCreateResponse(dev_p, 5) ;
    if (dev_p->storage_rw_resp_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, dev_p->storage_rw_req_p) ;
        ret = CY_AS_ERROR_OUT_OF_MEMORY ;
    }

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    /* Increment the storage count only if the above functionality succeeds.*/
    if (ret == CY_AS_ERROR_SUCCESS)
    {
        if (dev_p->storage_count == 0)
        {
            CyAsHalMemSet(dev_p->storage_device_info, 0, sizeof(dev_p->storage_device_info)) ;
            dev_p->is_storage_only_mode = CyFalse ;
        }

        dev_p->storage_count++ ;
    }

    CyAsDeviceClearSSSPending(dev_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageStart(CyAsDeviceHandle handle,
                   CyAsFunctionCallback cb,
                   uint32_t client)
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    if(CyAsDeviceIsSSSPending(dev_p))
        return CY_AS_ERROR_STARTSTOP_PENDING ;

    CyAsDeviceSetSSSPending(dev_p) ;

    if (dev_p->storage_count == 0)
    {
        /* Create the request to send to the West Bridge device */
        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_START_STORAGE, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
        if (req_p == 0)
        {
            CyAsDeviceClearSSSPending(dev_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        /* Reserve space for the reply, the reply data will not exceed one word */
        reply_p = CyAsLLCreateResponse(dev_p, 1) ;
        if (reply_p == 0)
        {
            CyAsDeviceClearSSSPending(dev_p) ;
            CyAsLLDestroyRequest(dev_p, req_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        if(cb == 0)
        {
            ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            return MyHandleResponseStorageStart(dev_p, req_p, reply_p, ret) ;
        }
        else
        {
            ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_START,
                0, dev_p->func_cbs_stor, CY_AS_REQUEST_RESPONSE_EX,
                req_p, reply_p, CyAsStorageFuncCallback) ;

            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            /* The request and response are freed as part of the FuncCallback */
            return ret ;
        }

destroy:
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    }
    else
    {
        dev_p->storage_count++ ;
        if (cb)
            cb(handle, ret, client, CY_FUNCT_CB_STOR_START, 0) ;
    }

    CyAsDeviceClearSSSPending(dev_p) ;

    return ret ;
}


static CyAsReturnStatus_t
MyHandleResponseStorageStop(CyAsDevice* dev_p,
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

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    if (ret == CY_AS_ERROR_SUCCESS)
    {
        CyAsLLDestroyRequest(dev_p, dev_p->storage_rw_req_p) ;
        CyAsLLDestroyResponse(dev_p, dev_p->storage_rw_resp_p) ;
        dev_p->storage_count-- ;
    }

    CyAsDeviceClearSSSPending(dev_p) ;

    return ret ;
}
CyAsReturnStatus_t
CyAsStorageStop(CyAsDeviceHandle handle,
                  CyAsFunctionCallback cb,
                  uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsStorageAsyncPending(dev_p))
        return CY_AS_ERROR_ASYNC_PENDING ;

    if(CyAsDeviceIsSSSPending(dev_p))
        return CY_AS_ERROR_STARTSTOP_PENDING ;

    CyAsDeviceSetSSSPending(dev_p) ;

    if (dev_p->storage_count == 1)
    {

        /* Create the request to send to the West Bridge device */
        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_STOP_STORAGE, CY_RQT_STORAGE_RQT_CONTEXT, 0) ;
        if (req_p == 0)
        {
            CyAsDeviceClearSSSPending(dev_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        /* Reserve space for the reply, the reply data will not exceed one word */
        reply_p = CyAsLLCreateResponse(dev_p, 1) ;
        if (reply_p == 0)
        {
            CyAsDeviceClearSSSPending(dev_p) ;
            CyAsLLDestroyRequest(dev_p, req_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        if(cb == 0)
        {
            ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            return MyHandleResponseStorageStop(dev_p, req_p, reply_p, ret) ;
        }
        else
        {
            ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_STOP,
                0, dev_p->func_cbs_stor, CY_AS_REQUEST_RESPONSE_EX,
                req_p, reply_p, CyAsStorageFuncCallback) ;

            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            /* The request and response are freed as part of the MiscFuncCallback */
            return ret ;
        }

destroy:
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    }
    else if(dev_p->storage_count > 1)
    {
        dev_p->storage_count-- ;
        if (cb)
            cb(handle, ret, client, CY_FUNCT_CB_STOR_STOP, 0) ;
    }

    CyAsDeviceClearSSSPending(dev_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageRegisterCallback(CyAsDeviceHandle handle, CyAsStorageEventCallback callback)
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

    dev_p->storage_event_cb = NULL ;
    dev_p->storage_event_cb_ms = callback ;

    return CY_AS_ERROR_SUCCESS ;
}



static CyAsReturnStatus_t
MyHandleResponseStorageClaim(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_NO_SUCH_ADDRESS)
    {
        ret = CyAsMapBadAddr(CyAsLLRequestResponse_GetWord(reply_p, 3)) ;
        goto destroy ;
    }

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_MEDIA_CLAIMED_RELEASED)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    /* The response must be about the address I am trying to claim or the firmware is broken */
    if ((CyAsStorageGetBusFromAddress(CyAsLLRequestResponse_GetWord(req_p, 0)) !=
        CyAsStorageGetBusFromAddress(CyAsLLRequestResponse_GetWord(reply_p, 0))) ||
        (CyAsStorageGetDeviceFromAddress(CyAsLLRequestResponse_GetWord(req_p, 0)) !=
        CyAsStorageGetDeviceFromAddress(CyAsLLRequestResponse_GetWord(reply_p, 0))))
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    if (CyAsLLRequestResponse_GetWord(reply_p, 1) != 1)
        ret = CY_AS_ERROR_NOT_ACQUIRED ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyStorageClaim(CyAsDevice *dev_p,
                void* data,
                CyAsBusNumber_t bus,
                uint32_t device,
                uint16_t req_flags,
                CyAsFunctionCallback cb,
                uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if(dev_p->mtp_count > 0)
        return CY_AS_ERROR_NOT_VALID_IN_MTP ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_CLAIM_STORAGE, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, device, 0)) ;

    /* Reserve space for the reply, the reply data will not exceed one word */
    reply_p = CyAsLLCreateResponse(dev_p, 4) ;
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

        return MyHandleResponseStorageClaim(dev_p, req_p, reply_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_CLAIM,
            data, dev_p->func_cbs_stor, req_flags,
            req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the MiscFuncCallback */
        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageClaim(CyAsDeviceHandle handle,
                   CyAsBusNumber_t bus,
                   uint32_t device,
                   CyAsFunctionCallback cb,
                   uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (bus >= CY_AS_MAX_BUSES)
        return CY_AS_ERROR_NO_SUCH_BUS ;

    return MyStorageClaim(dev_p, NULL, bus, device, CY_AS_REQUEST_RESPONSE_MS, cb, client) ;
}

static CyAsReturnStatus_t
MyHandleResponseStorageRelease(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_NO_SUCH_ADDRESS)
    {
        ret = CyAsMapBadAddr(CyAsLLRequestResponse_GetWord(reply_p, 3)) ;
        goto destroy ;
    }

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_MEDIA_CLAIMED_RELEASED)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    /* The response must be about the address I am trying to release or the firmware is broken */
    if ((CyAsStorageGetBusFromAddress(CyAsLLRequestResponse_GetWord(req_p, 0)) !=
        CyAsStorageGetBusFromAddress(CyAsLLRequestResponse_GetWord(reply_p, 0))) ||
        (CyAsStorageGetDeviceFromAddress(CyAsLLRequestResponse_GetWord(req_p, 0)) !=
        CyAsStorageGetDeviceFromAddress(CyAsLLRequestResponse_GetWord(reply_p, 0))))
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }


    if (CyAsLLRequestResponse_GetWord(reply_p, 1) != 0)
        ret = CY_AS_ERROR_NOT_RELEASED ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyStorageRelease(CyAsDevice* dev_p,
                    void* data,
                    CyAsBusNumber_t bus,
                    uint32_t device,
                    uint16_t req_flags,
                    CyAsFunctionCallback cb,
                    uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if(dev_p->mtp_count > 0)
        return CY_AS_ERROR_NOT_VALID_IN_MTP ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_RELEASE_STORAGE, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, device, 0)) ;

    /* Reserve space for the reply, the reply data will not exceed one word */
    reply_p = CyAsLLCreateResponse(dev_p, 4) ;
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

        return MyHandleResponseStorageRelease(dev_p, req_p, reply_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_RELEASE,
            data, dev_p->func_cbs_stor, req_flags,
            req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the MiscFuncCallback */
        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageRelease(CyAsDeviceHandle handle,
                   CyAsBusNumber_t bus,
                   uint32_t device,
                   CyAsFunctionCallback cb,
                   uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (bus >= CY_AS_MAX_BUSES)
        return CY_AS_ERROR_NO_SUCH_BUS ;

    return MyStorageRelease(dev_p, NULL, bus, device, CY_AS_REQUEST_RESPONSE_MS, cb, client) ;
}

static CyAsReturnStatus_t
MyHandleResponseStorageQueryBus(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         uint32_t* count)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t code = CyAsLLRequestResponse_GetCode(reply_p) ;
    uint16_t v ;

    if (code == CY_RESP_NO_SUCH_ADDRESS)
    {
        ret = CY_AS_ERROR_NO_SUCH_BUS ;
        goto destroy ;
    }

    if (code != CY_RESP_BUS_DESCRIPTOR)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    /*
     * Verify that the response corresponds to the bus that was queried.
     */
    if (CyAsStorageGetBusFromAddress(CyAsLLRequestResponse_GetWord(req_p, 0)) !=
                CyAsStorageGetBusFromAddress(CyAsLLRequestResponse_GetWord(reply_p, 0)))
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    v = CyAsLLRequestResponse_GetWord(reply_p, 1) ;
    if (req_p->flags & CY_AS_REQUEST_RESPONSE_MS)
    {
        /*
         * This request is only for the count of devices on the bus. There
         * is no need to check the media type.
         */
        if (v)
            *count = 1 ;
        else
            *count = 0 ;
    }
    else
    {
        /*
         * This request is for the count of devices of a particular type. We need
         * to check whether the media type found matches the queried type.
         */
        CyAsMediaType queried = (CyAsMediaType)CyAsLLRequestResponse_GetWord(req_p, 1) ;
        CyAsMediaType found   = CyAsStorageGetMediaFromAddress(v) ;

        if (queried == found)
            *count = 1 ;
        else
            *count = 0 ;
    }

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
MyStorageQueryBus(CyAsDevice *dev_p,
                        CyAsBusNumber_t bus,
                        CyAsMediaType   type,
                        uint16_t req_flags,
                        uint32_t *count,
                        CyAsFunctionCallback cb,
                        uint32_t client)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsFunctCBType cbType = CY_FUNCT_CB_STOR_QUERYBUS ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /* Create the request to send to the Antioch device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_QUERY_BUS, CY_RQT_STORAGE_RQT_CONTEXT, 2) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, 0, 0)) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)type) ;

    /* Reserve space for the reply, the reply data will not exceed two words. */
    reply_p = CyAsLLCreateResponse(dev_p, 2) ;
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

        req_p->flags |= req_flags;
        return MyHandleResponseStorageQueryBus(dev_p, req_p, reply_p, count) ;
    }
    else
    {
        if (req_flags == CY_AS_REQUEST_RESPONSE_EX)
            cbType = CY_FUNCT_CB_STOR_QUERYMEDIA ;

        ret = CyAsMiscSendRequest(dev_p, cb, client, cbType,
            count, dev_p->func_cbs_stor, req_flags,
            req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the MiscFuncCallback */
        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageQueryBus(CyAsDeviceHandle handle,
                        CyAsBusNumber_t bus,
                        uint32_t *count,
                        CyAsFunctionCallback cb,
                        uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    return MyStorageQueryBus(dev_p, bus, CyAsMediaMaxMediaValue, CY_AS_REQUEST_RESPONSE_MS,
            count, cb, client) ;
}

CyAsReturnStatus_t
CyAsStorageQueryMedia(CyAsDeviceHandle handle,
                        CyAsMediaType type,
                        uint32_t *count,
                        CyAsFunctionCallback cb,
                        uint32_t client)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsBusNumber_t bus ;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    ret = CyAnMapBusFromMediaType(dev_p, type, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return MyStorageQueryBus(dev_p, bus, type, CY_AS_REQUEST_RESPONSE_EX,
            count, cb, client) ;
}

static CyAsReturnStatus_t
MyHandleResponseStorageQueryDevice(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         void* data_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint16_t v ;
    CyAsBusNumber_t bus ;
    CyAsMediaType type ;
    uint32_t device ;
    CyBool removable ;
    CyBool writeable ;
    CyBool locked ;
    uint16_t block_size ;
    uint32_t number_units ;
    uint32_t number_eus ;

    if (CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_NO_SUCH_ADDRESS)
    {
        ret = CyAsMapBadAddr(CyAsLLRequestResponse_GetWord(reply_p, 3)) ;
        goto destroy ;
    }

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_DEVICE_DESCRIPTOR)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    /* Unpack the response */
    v = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    type = CyAsStorageGetMediaFromAddress(v) ;
    bus  = CyAsStorageGetBusFromAddress(v) ;
    device = CyAsStorageGetDeviceFromAddress(v) ;

    block_size = CyAsLLRequestResponse_GetWord(reply_p, 1) ;

    v = CyAsLLRequestResponse_GetWord(reply_p, 2) ;
    removable = (v & 0x8000) ? CyTrue : CyFalse ;
    writeable = (v & 0x0100) ? CyTrue : CyFalse ;
    locked = (v & 0x0200) ? CyTrue : CyFalse ;
    number_units = (v & 0xff) ;

    number_eus  = (CyAsLLRequestResponse_GetWord(reply_p, 3) << 16) | CyAsLLRequestResponse_GetWord(reply_p, 4) ;

    /* Store the results based on the version of originating function */
    if (req_p->flags & CY_AS_REQUEST_RESPONSE_MS)
    {
        CyAsStorageQueryDeviceData  *store_p = (CyAsStorageQueryDeviceData*)data_p ;

        /* Make sure the response is about the address we asked about - if not, firmware error */
        if ((bus != store_p->bus) || (device != store_p->device))
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        store_p->desc_p.type = type ;
        store_p->desc_p.removable = removable ;
        store_p->desc_p.writeable = writeable ;
        store_p->desc_p.block_size = block_size ;
        store_p->desc_p.number_units = number_units ;
        store_p->desc_p.locked = locked ;
        store_p->desc_p.erase_unit_size = number_eus ;
        dev_p->storage_device_info[bus][device] = store_p->desc_p ;
    }
    else
    {
        CyAsStorageQueryDeviceData_dep    *store_p = (CyAsStorageQueryDeviceData_dep*)data_p ;

        /* Make sure the response is about the address we asked about - if not, firmware error */
        if ((type != store_p->type) || (device != store_p->device))
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        store_p->desc_p.type = type ;
        store_p->desc_p.removable = removable ;
        store_p->desc_p.writeable = writeable ;
        store_p->desc_p.block_size = block_size ;
        store_p->desc_p.number_units = number_units ;
        store_p->desc_p.locked = locked ;
        store_p->desc_p.erase_unit_size = number_eus ;
        dev_p->storage_device_info[bus][device] = store_p->desc_p ;
    }

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyStorageQueryDevice(CyAsDevice *dev_p,
                        void* data_p,
                        uint16_t req_flags,
                        CyAsBusNumber_t bus,
                        uint32_t device,
                        CyAsFunctionCallback cb,
                        uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /* Create the request to send to the Antioch device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_QUERY_DEVICE, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, device, 0)) ;

    /* Reserve space for the reply, the reply data will not exceed five words. */
    reply_p = CyAsLLCreateResponse(dev_p, 5) ;
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

        req_p->flags |= req_flags;
        return MyHandleResponseStorageQueryDevice(dev_p, req_p, reply_p, data_p) ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_QUERYDEVICE,
            data_p, dev_p->func_cbs_stor, req_flags,
            req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the MiscFuncCallback */
        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageQueryDevice(CyAsDeviceHandle handle,
                         CyAsStorageQueryDeviceData* data_p,
                         CyAsFunctionCallback cb,
                         uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    return MyStorageQueryDevice(dev_p, data_p, CY_AS_REQUEST_RESPONSE_MS, data_p->bus,
            data_p->device, cb, client) ;
}

static CyAsReturnStatus_t
MyHandleResponseStorageQueryUnit(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         void* data_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsBusNumber_t bus ;
    uint32_t device ;
    uint32_t unit ;
    CyAsMediaType type ;
    uint16_t block_size ;
    uint32_t start_block ;
    uint32_t unit_size ;
    uint16_t v ;

    if (CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_NO_SUCH_ADDRESS)
    {
        ret = CyAsMapBadAddr(CyAsLLRequestResponse_GetWord(reply_p, 3)) ;
        goto destroy ;
    }

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_UNIT_DESCRIPTOR)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    /* Unpack the response */
    v      = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    bus    = CyAsStorageGetBusFromAddress(v) ;
    device = CyAsStorageGetDeviceFromAddress(v) ;
    unit   = GetUnitFromAddress(v) ;

    type   = CyAsStorageGetMediaFromAddress(CyAsLLRequestResponse_GetWord(reply_p, 1));

    block_size = CyAsLLRequestResponse_GetWord(reply_p, 2) ;
    start_block = CyAsLLRequestResponse_GetWord(reply_p, 3) | (CyAsLLRequestResponse_GetWord(reply_p, 4) << 16) ;
    unit_size = CyAsLLRequestResponse_GetWord(reply_p, 5) | (CyAsLLRequestResponse_GetWord(reply_p, 6) << 16) ;

    /* Store the results based on the version of originating function */
    if (req_p->flags & CY_AS_REQUEST_RESPONSE_MS)
    {
        CyAsStorageQueryUnitData  *store_p = (CyAsStorageQueryUnitData*)data_p ;

        /* Make sure the response is about the address we asked about - if not, firmware error */
        if (bus != store_p->bus || device != store_p->device || unit != store_p->unit)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        store_p->desc_p.type = type ;
        store_p->desc_p.block_size = block_size ;
        store_p->desc_p.start_block = start_block ;
        store_p->desc_p.unit_size = unit_size ;
    }
    else
    {
        CyAsStorageQueryUnitData_dep    *store_p = (CyAsStorageQueryUnitData_dep*)data_p ;

        /* Make sure the response is about the media type we asked about - if not, firmware error */
        if ((type != store_p->type) || (device != store_p->device) || (unit != store_p->unit) ) {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        store_p->desc_p.type = type ;
        store_p->desc_p.block_size = block_size ;
        store_p->desc_p.start_block = start_block ;
        store_p->desc_p.unit_size = unit_size ;
    }

    dev_p->storage_device_info[bus][device].type = type ;
    dev_p->storage_device_info[bus][device].block_size = block_size ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyStorageQueryUnit(CyAsDevice *dev_p,
                    void* data_p,
                    uint16_t req_flags,
                    CyAsBusNumber_t bus,
                    uint32_t device,
                    uint32_t unit,
                    CyAsFunctionCallback cb,
                    uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_QUERY_UNIT, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    if (device > 255)
        return CY_AS_ERROR_NO_SUCH_DEVICE ;

    if (unit > 255)
        return CY_AS_ERROR_NO_SUCH_UNIT ;

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, device, (uint8_t)unit)) ;

    /* Reserve space for the reply, the reply data will be of seven words. */
    reply_p = CyAsLLCreateResponse(dev_p, 7) ;
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

        req_p->flags |= req_flags ;
        return MyHandleResponseStorageQueryUnit(dev_p, req_p, reply_p, data_p) ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_QUERYUNIT,
            data_p, dev_p->func_cbs_stor, req_flags,
            req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the MiscFuncCallback */
        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageQueryUnit(CyAsDeviceHandle handle,
                       CyAsStorageQueryUnitData* data_p,
                       CyAsFunctionCallback cb,
                       uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    return MyStorageQueryUnit(dev_p, data_p, CY_AS_REQUEST_RESPONSE_MS, data_p->bus,
            data_p->device, data_p->unit, cb, client) ;
}


static CyAsReturnStatus_t
CyAsGetBlockSize(CyAsDevice *dev_p,
                    CyAsBusNumber_t bus,
                    uint32_t device,
                    CyAsFunctionCallback cb)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_QUERY_DEVICE, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, device, 0)) ;

    reply_p = CyAsLLCreateResponse(dev_p, 4) ;
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

        if (CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_NO_SUCH_ADDRESS)
        {
            ret = CY_AS_ERROR_NO_SUCH_BUS ;
            goto destroy ;
        }

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_DEVICE_DESCRIPTOR)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        /* Make sure the response is about the media type we asked about - if not, firmware error */
        if ((CyAsStorageGetBusFromAddress(CyAsLLRequestResponse_GetWord(reply_p, 0)) != bus) ||
            (CyAsStorageGetDeviceFromAddress(CyAsLLRequestResponse_GetWord(reply_p, 0)) != device) )
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }


        dev_p->storage_device_info[bus][device].block_size = CyAsLLRequestResponse_GetWord(reply_p, 1) ;
    }
    else
        ret = CY_AS_ERROR_INVALID_REQUEST ;

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
MyStorageDeviceControl(
        CyAsDevice          *dev_p,
        CyAsBusNumber_t      bus,
        uint32_t             device,
        CyBool               card_detect_en,
        CyBool               write_prot_en,
                CyAsStorageCardDetect config_detect,
        CyAsFunctionCallback cb,
        uint32_t             client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret ;
    CyBool use_gpio = CyFalse ;

    (void)device ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    if (bus >= CY_AS_MAX_BUSES)
        return CY_AS_ERROR_NO_SUCH_BUS ;

    if (device >= CY_AS_MAX_STORAGE_DEVICES)
        return CY_AS_ERROR_NO_SUCH_DEVICE ;

    /* If SD is not supported on the specified bus, then return ERROR */
    if((dev_p->media_supported[bus] ==0 ) || (dev_p->media_supported[bus] &(1<<CyAsMediaNand)))
        return CY_AS_ERROR_NOT_SUPPORTED;

    if(config_detect == CyAsStorageDetect_GPIO)
        use_gpio = CyTrue ;
    else if(config_detect == CyAsStorageDetect_SDAT_3)
        use_gpio = CyFalse ;
    else
        return CY_AS_ERROR_INVALID_PARAMETER ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SD_INTERFACE_CONTROL, CY_RQT_STORAGE_RQT_CONTEXT, 2) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, device, 0)) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (((uint16_t)card_detect_en << 8) | ((uint16_t)use_gpio << 1) | (uint16_t)write_prot_en)) ;

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

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_DEVICECONTROL,
            0, dev_p->func_cbs_stor, CY_AS_REQUEST_RESPONSE_EX,
            req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the MiscFuncCallback */
        return ret ;
    }
destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageDeviceControl(CyAsDeviceHandle handle,
                            CyAsBusNumber_t bus,
                            uint32_t device,
                            CyBool card_detect_en,
                            CyBool write_prot_en,
                            CyAsStorageCardDetect config_detect,
                            CyAsFunctionCallback cb,
                            uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    return MyStorageDeviceControl(dev_p, bus, device, card_detect_en, write_prot_en, config_detect, cb, client);
}

static void
CyAsAsyncStorageCallback(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, void *buf_p, uint32_t size, CyAsReturnStatus_t ret)
{
    CyAsStorageCallback_dep cb ;
    CyAsStorageCallback cb_ms ;

    (void)size ;
    (void)buf_p ;
    (void)ep ;

    CyAsDeviceClearStorageAsyncPending(dev_p) ;

    /*
    * If the LL request callback has already been called, the user
    * callback has to be called from here.
    */
    if (!dev_p->storage_wait)
    {
            CyAsHalAssert(dev_p->storage_cb != NULL || dev_p->storage_cb_ms != NULL) ;
            cb = dev_p->storage_cb ;
            cb_ms = dev_p->storage_cb_ms ;

            dev_p->storage_cb = 0 ;
            dev_p->storage_cb_ms = 0 ;

            if (ret == CY_AS_ERROR_SUCCESS)
                ret = dev_p->storage_error ;

        if (cb_ms)
        {
            cb_ms((CyAsDeviceHandle)dev_p, dev_p->storage_bus_index, dev_p->storage_device_index,
                dev_p->storage_unit, dev_p->storage_block_addr, dev_p->storage_oper, ret) ;
        }
        else
        {
            cb((CyAsDeviceHandle)dev_p,
                dev_p->storage_device_info[dev_p->storage_bus_index][dev_p->storage_device_index].type,
                dev_p->storage_device_index, dev_p->storage_unit, dev_p->storage_block_addr, dev_p->storage_oper, ret) ;
        }
    }
    else
        dev_p->storage_error = ret ;
}

static void
CyAsAsyncStorageReplyCallback(
                              CyAsDevice *dev_p,
                              uint8_t context,
                              CyAsLLRequestResponse *rqt,
                              CyAsLLRequestResponse *resp,
                              CyAsReturnStatus_t ret)
{
    CyAsStorageCallback_dep cb ;
    CyAsStorageCallback cb_ms ;
    uint8_t reqtype ;
    (void)rqt ;
    (void)context ;

    reqtype = CyAsLLRequestResponse_GetCode(rqt) ;

    if (ret == CY_AS_ERROR_SUCCESS)
    {
        if (CyAsLLRequestResponse_GetCode(resp) == CY_RESP_ANTIOCH_DEFERRED_ERROR)
        {
            ret = CyAsLLRequestResponse_GetWord(resp, 0) & 0x00FF ;
        }
        else if (CyAsLLRequestResponse_GetCode(resp) != CY_RESP_SUCCESS_FAILURE)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        }
    }

    if (ret != CY_AS_ERROR_SUCCESS)
    {
        if(reqtype == CY_RQT_READ_BLOCK)
            CyAsDmaCancel(dev_p, dev_p->storage_read_endpoint, ret) ;
        else
            CyAsDmaCancel(dev_p, dev_p->storage_write_endpoint, ret) ;
    }

    dev_p->storage_wait = CyFalse ;

    /*
    * If the DMA callback has already been called, the user callback
    * has to be called from here.
    */
    if (!CyAsDeviceIsStorageAsyncPending(dev_p))
    {
            CyAsHalAssert(dev_p->storage_cb != NULL || dev_p->storage_cb_ms != NULL) ;
        cb = dev_p->storage_cb ;
        cb_ms = dev_p->storage_cb_ms ;

            dev_p->storage_cb = 0 ;
            dev_p->storage_cb_ms = 0 ;

            if (ret == CY_AS_ERROR_SUCCESS)
                ret = dev_p->storage_error ;

        if (cb_ms)
        {
            cb_ms((CyAsDeviceHandle)dev_p, dev_p->storage_bus_index, dev_p->storage_device_index,
                dev_p->storage_unit, dev_p->storage_block_addr, dev_p->storage_oper, ret) ;
        }
        else
        {
            cb((CyAsDeviceHandle)dev_p,
                dev_p->storage_device_info[dev_p->storage_bus_index][dev_p->storage_device_index].type,
                dev_p->storage_device_index, dev_p->storage_unit, dev_p->storage_block_addr, dev_p->storage_oper, ret) ;
        }
    }
    else
        dev_p->storage_error = ret ;
}

static CyAsReturnStatus_t
CyAsStorageAsyncOper(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, uint8_t reqtype, uint16_t req_flags,
                     CyAsBusNumber_t bus, uint32_t device, uint32_t unit,
                     uint32_t block, void *data_p, uint16_t num_blocks,
                     CyAsStorageCallback_dep callback, CyAsStorageCallback callback_ms)
{
    uint32_t mask ;
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (bus >= CY_AS_MAX_BUSES)
        return CY_AS_ERROR_NO_SUCH_BUS ;

    if (device >= CY_AS_MAX_STORAGE_DEVICES)
        return CY_AS_ERROR_NO_SUCH_DEVICE ;

    if (unit > 255)
        return CY_AS_ERROR_NO_SUCH_UNIT ;

    /* We are supposed to return sucess if the number of
    * blocks is zero
    */
    if (num_blocks == 0)
    {
        if (callback_ms)
            callback_ms((CyAsDeviceHandle)dev_p, bus, device, unit, block,
                ((reqtype == CY_RQT_WRITE_BLOCK) ? CyAsOpWrite : CyAsOpRead),
                CY_AS_ERROR_SUCCESS) ;
        else
            callback((CyAsDeviceHandle)dev_p, dev_p->storage_device_info[bus][device].type,
                device, unit, block, ((reqtype == CY_RQT_WRITE_BLOCK) ? CyAsOpWrite : CyAsOpRead),
                CY_AS_ERROR_SUCCESS) ;

        return CY_AS_ERROR_SUCCESS ;
    }

    if (dev_p->storage_device_info[bus][device].block_size == 0)
            return CY_AS_ERROR_QUERY_DEVICE_NEEDED ;

    /*
    * Since async operations can be triggered by interrupt code, we must
    * insure that we do not get multiple async operations going at one time and
    * protect this test and set operation from interrupts.
    * Also need to check for pending Async MTP writes
    */
    mask = CyAsHalDisableInterrupts() ;
    if ((CyAsDeviceIsStorageAsyncPending(dev_p)) || (dev_p->storage_wait) || (CyAsDeviceIsUsbAsyncPending(dev_p, 6)))
    {
        CyAsHalEnableInterrupts(mask) ;
        return CY_AS_ERROR_ASYNC_PENDING ;
    }

    CyAsDeviceSetStorageAsyncPending(dev_p) ;
    CyAsHalEnableInterrupts(mask) ;

    /*
    * Storage information about the currently outstanding request
    */
    dev_p->storage_cb = callback ;
    dev_p->storage_cb_ms = callback_ms ;
    dev_p->storage_bus_index = bus ;
    dev_p->storage_device_index = device ;
    dev_p->storage_unit = unit ;
    dev_p->storage_block_addr = block ;

    /* Initialise the request to send to the West Bridge. */
    req_p = dev_p->storage_rw_req_p ;
    CyAsLLInitRequest(req_p, reqtype, CY_RQT_STORAGE_RQT_CONTEXT, 5) ;

    /* Initialise the space for reply from the West Bridge. */
    reply_p = dev_p->storage_rw_resp_p ;
    CyAsLLInitResponse(reply_p, 5) ;

    /* Remember which version of the API originated the request */
    req_p->flags |= req_flags ;

    /* Setup the DMA request and adjust the storage operation if we are reading */
    if (reqtype == CY_RQT_READ_BLOCK)
    {
        ret = CyAsDmaQueueRequest(dev_p, ep, data_p, dev_p->storage_device_info[bus][device].block_size * num_blocks,
            CyFalse, CyTrue, CyAsAsyncStorageCallback) ;
        dev_p->storage_oper = CyAsOpRead ;
    }
    else if (reqtype == CY_RQT_WRITE_BLOCK)
    {
        ret = CyAsDmaQueueRequest(dev_p, ep, data_p, dev_p->storage_device_info[bus][device].block_size * num_blocks,
            CyFalse, CyFalse, CyAsAsyncStorageCallback) ;
        dev_p->storage_oper = CyAsOpWrite ;
    }
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsDeviceClearStorageAsyncPending(dev_p) ;
        return ret ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, (uint8_t)unit)) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((block >> 16) & 0xffff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 2, (uint16_t)(block & 0xffff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 3, (uint16_t)((num_blocks >> 8) & 0x00ff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 4, (uint16_t)((num_blocks << 8) & 0xff00)) ;

    /* Set the burst mode flag. */
    if (dev_p->is_storage_only_mode)
        req_p->data[4] |= 0x0001;

    /* Send the request and wait for completion of storage request */
    dev_p->storage_wait = CyTrue ;
    ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyTrue, CyAsAsyncStorageReplyCallback) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsDmaCancel(dev_p, ep, CY_AS_ERROR_CANCELED) ;
        CyAsDeviceClearStorageAsyncPending(dev_p) ;
    }

    return ret ;
}

static void
CyAsSyncStorageCallback(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, void *buf_p, uint32_t size, CyAsReturnStatus_t err)
{
    (void)ep ;
    (void)buf_p ;
    (void)size ;

    dev_p->storage_error = err ;
}

static void
CyAsSyncStorageReplyCallback(
                             CyAsDevice *dev_p,
                             uint8_t context,
                             CyAsLLRequestResponse *rqt,
                             CyAsLLRequestResponse *resp,
                             CyAsReturnStatus_t ret)
{
    uint8_t reqtype ;
    (void)rqt ;

    reqtype = CyAsLLRequestResponse_GetCode(rqt) ;

    if (CyAsLLRequestResponse_GetCode(resp) == CY_RESP_ANTIOCH_DEFERRED_ERROR)
    {
        ret = CyAsLLRequestResponse_GetWord(resp, 0) & 0x00FF ;

        if (ret != CY_AS_ERROR_SUCCESS)
        {
            if(reqtype == CY_RQT_READ_BLOCK)
                CyAsDmaCancel(dev_p, dev_p->storage_read_endpoint, ret) ;
            else
                CyAsDmaCancel(dev_p, dev_p->storage_write_endpoint, ret) ;
        }
    }
    else if (CyAsLLRequestResponse_GetCode(resp) != CY_RESP_SUCCESS_FAILURE)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }

    dev_p->storage_wait = CyFalse ;
    dev_p->storage_error = ret ;

    /* Wake any threads/processes that are waiting on the read/write completion. */
    CyAsHalWake (&dev_p->context[context]->channel) ;
}

static CyAsReturnStatus_t
CyAsStorageSyncOper(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, uint8_t reqtype, CyAsBusNumber_t bus, uint32_t device,
                    uint32_t unit, uint32_t block, void *data_p, uint16_t num_blocks)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsContext *ctxt_p ;
    uint32_t loopcount = 200 ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (bus >= CY_AS_MAX_BUSES)
        return CY_AS_ERROR_NO_SUCH_BUS ;

    if (device >= CY_AS_MAX_STORAGE_DEVICES)
        return CY_AS_ERROR_NO_SUCH_DEVICE ;

    if (unit > 255)
        return CY_AS_ERROR_NO_SUCH_UNIT ;

    if ((CyAsDeviceIsStorageAsyncPending(dev_p)) || (dev_p->storage_wait))
        return CY_AS_ERROR_ASYNC_PENDING ;

    /* Also need to check for pending Async MTP writes */
    if(CyAsDeviceIsUsbAsyncPending(dev_p, 6))
        return CY_AS_ERROR_ASYNC_PENDING ;

    /* We are supposed to return sucess if the number of
    * blocks is zero
    */
    if (num_blocks == 0)
        return CY_AS_ERROR_SUCCESS ;

    if (dev_p->storage_device_info[bus][device].block_size == 0)
    {
        /*
        * Normally, a given device has been queried via the query device call before a
        * read request is issued.  Therefore, this normally will not be run.
        */
        ret = CyAsGetBlockSize(dev_p, bus, device, 0) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            return ret ;
    }

    /* Initialise the request to send to the West Bridge. */
    req_p = dev_p->storage_rw_req_p ;
    CyAsLLInitRequest(req_p, reqtype, CY_RQT_STORAGE_RQT_CONTEXT, 5) ;

    /* Initialise the space for reply from the West Bridge. */
    reply_p = dev_p->storage_rw_resp_p ;
    CyAsLLInitResponse(reply_p, 5) ;

    /* Setup the DMA request */
    if (reqtype == CY_RQT_READ_BLOCK)
    {
        ret = CyAsDmaQueueRequest(dev_p, ep, data_p, dev_p->storage_device_info[bus][device].block_size * num_blocks, CyFalse,
            CyTrue, CyAsSyncStorageCallback) ;
        dev_p->storage_oper = CyAsOpRead ;
    }
    else if (reqtype == CY_RQT_WRITE_BLOCK)
    {
        ret = CyAsDmaQueueRequest(dev_p, ep, data_p, dev_p->storage_device_info[bus][device].block_size * num_blocks, CyFalse,
            CyFalse, CyAsSyncStorageCallback) ;
        dev_p->storage_oper = CyAsOpWrite ;
    }
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        return ret ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, (uint8_t)unit)) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((block >> 16) & 0xffff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 2, (uint16_t)(block & 0xffff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 3, (uint16_t)((num_blocks >> 8) & 0x00ff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 4, (uint16_t)((num_blocks << 8) & 0xff00)) ;

    /* Set the burst mode flag. */
    if (dev_p->is_storage_only_mode)
        req_p->data[4] |= 0x0001;

    /* Send the request and wait for completion of storage request */
    dev_p->storage_wait = CyTrue ;
    ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyTrue, CyAsSyncStorageReplyCallback) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsDmaCancel(dev_p, ep, CY_AS_ERROR_CANCELED) ;
    }
    else
    {
        /* Setup the DMA request */
        ctxt_p = dev_p->context[CY_RQT_STORAGE_RQT_CONTEXT] ;
        ret = CyAsDmaDrainQueue(dev_p, ep, CyFalse) ;

        while (loopcount-- > 0)
        {
            if (dev_p->storage_wait == CyFalse)
                break ;
            CyAsHalSleepOn(&ctxt_p->channel, 10) ;
        }

        if (dev_p->storage_wait == CyTrue)
        {
            dev_p->storage_wait = CyFalse ;
            CyAsLLRemoveRequest(dev_p, ctxt_p, req_p, CyTrue) ;
            ret = CY_AS_ERROR_TIMEOUT ;
        }

        if (ret == CY_AS_ERROR_SUCCESS)
            ret = dev_p->storage_error ;
    }

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageRead(CyAsDeviceHandle handle, CyAsBusNumber_t bus, uint32_t device, uint32_t unit, uint32_t block,
                void *data_p, uint16_t num_blocks)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    return CyAsStorageSyncOper(dev_p, dev_p->storage_read_endpoint, CY_RQT_READ_BLOCK, bus, device,
        unit, block, data_p, num_blocks) ;
}

CyAsReturnStatus_t
CyAsStorageWrite(CyAsDeviceHandle handle, CyAsBusNumber_t bus, uint32_t device, uint32_t unit, uint32_t block,
                 void *data_p, uint16_t num_blocks)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if(dev_p->mtp_turbo_active)
        return CY_AS_ERROR_NOT_VALID_DURING_MTP ;

    return CyAsStorageSyncOper(dev_p, dev_p->storage_write_endpoint, CY_RQT_WRITE_BLOCK, bus, device,
        unit, block, data_p, num_blocks) ;
}


CyAsReturnStatus_t
CyAsStorageReadAsync(CyAsDeviceHandle handle, CyAsBusNumber_t bus, uint32_t device, uint32_t unit,
                     uint32_t block, void *data_p, uint16_t num_blocks, CyAsStorageCallback callback)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (callback == 0)
        return CY_AS_ERROR_NULL_CALLBACK ;

    return CyAsStorageAsyncOper(dev_p, dev_p->storage_read_endpoint, CY_RQT_READ_BLOCK,
        CY_AS_REQUEST_RESPONSE_MS, bus, device, unit, block, data_p, num_blocks, NULL, callback);
}

CyAsReturnStatus_t
CyAsStorageWriteAsync(CyAsDeviceHandle handle, CyAsBusNumber_t bus, uint32_t device, uint32_t unit,
                      uint32_t block, void *data_p, uint16_t num_blocks, CyAsStorageCallback callback)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (callback == 0)
        return CY_AS_ERROR_NULL_CALLBACK ;

    if(dev_p->mtp_turbo_active)
        return CY_AS_ERROR_NOT_VALID_DURING_MTP ;

    return CyAsStorageAsyncOper(dev_p, dev_p->storage_write_endpoint, CY_RQT_WRITE_BLOCK,
        CY_AS_REQUEST_RESPONSE_MS, bus, device, unit, block, data_p, num_blocks, NULL, callback);
}


static void
MyStorageCancelCallback (
        CyAsDevice *dev_p,
        uint8_t context,
        CyAsLLRequestResponse *rqt,
        CyAsLLRequestResponse *resp,
        CyAsReturnStatus_t stat)
{
    (void)context ;
    (void)stat ;

    /* Nothing to do here, except free up the request and response structures. */
    CyAsLLDestroyResponse(dev_p, resp) ;
    CyAsLLDestroyRequest(dev_p, rqt) ;
}


CyAsReturnStatus_t
CyAsStorageCancelAsync(CyAsDeviceHandle handle)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p , *reply_p ;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (!CyAsDeviceIsStorageAsyncPending(dev_p))
        return CY_AS_ERROR_ASYNC_NOT_PENDING ;

    /*
     * Create and send a mailbox request to firmware asking it to abort processing
     * of the current P2S operation. The rest of the cancel processing will be
     * driven through the callbacks for the read/write call.
     */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_ABORT_P2S_XFER, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyFalse, MyStorageCancelCallback) ;
    if (ret)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    }

    return CY_AS_ERROR_SUCCESS ;
}

/*
 * This function does all the API side clean-up associated with
 * CyAsStorageStop, without any communication with the firmware.
 */
void CyAsStorageCleanup(CyAsDevice *dev_p)
{
    if (dev_p->storage_count)
    {
        CyAsLLDestroyRequest(dev_p, dev_p->storage_rw_req_p) ;
        CyAsLLDestroyResponse(dev_p, dev_p->storage_rw_resp_p) ;
        dev_p->storage_count = 0 ;
        CyAsDeviceClearScsiMessages(dev_p) ;
        CyAsHalMemSet(dev_p->storage_device_info, 0, sizeof(dev_p->storage_device_info)) ;

        CyAsDeviceClearStorageAsyncPending(dev_p) ;
        dev_p->storage_cb = 0 ;
        dev_p->storage_cb_ms = 0 ;
        dev_p->storage_wait = CyFalse ;
    }
}

static CyAsReturnStatus_t
MyHandleResponseSDRegRead(
        CyAsDevice               *dev_p,
        CyAsLLRequestResponse    *req_p,
        CyAsLLRequestResponse    *reply_p,
        CyAsStorageSDRegReadData *info)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t  resp_type, i ;
    uint16_t resp_len ;
    uint8_t  length = info->length ;
    uint8_t *data_p = info->buf_p ;

    resp_type = CyAsLLRequestResponse_GetCode(reply_p) ;
    if (resp_type == CY_RESP_SD_REGISTER_DATA)
    {
        uint16_t *resp_p = reply_p->data + 1 ;
        uint16_t temp ;

        resp_len = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        CyAsHalAssert(resp_len >= length) ;

        /*
         * Copy the values into the output buffer after doing the
         * necessary bit shifting. The bit shifting is required because
         * the data comes out of the West Bridge with a 6 bit offset.
         */
        i = 0 ;
        while (length)
        {
            temp = ((resp_p[i] << 6) | (resp_p[i + 1] >> 10)) ;
            i++ ;

            *data_p++ = (uint8_t)(temp >> 8) ;
            length-- ;

            if (length)
            {
                *data_p++ = (uint8_t)(temp & 0xFF) ;
                length-- ;
            }
        }
    }
    else
    {
        if (resp_type == CY_RESP_SUCCESS_FAILURE)
            ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        else
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }

    CyAsLLDestroyResponse(dev_p, reply_p) ;
    CyAsLLDestroyRequest(dev_p, req_p) ;
    (void)resp_len;
    return ret ;
}

CyAsReturnStatus_t
CyAsStorageSDRegisterRead(
        CyAsDeviceHandle          handle,
        CyAsBusNumber_t           bus,
        uint8_t                   device,
        CyAsSDCardRegType         regType,
        CyAsStorageSDRegReadData *data_p,
        CyAsFunctionCallback      cb,
        uint32_t                  client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t  length ;

    /*
     * Sanity checks required before sending the request to the
     * firmware.
     */
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (device >= CY_AS_MAX_STORAGE_DEVICES)
        return CY_AS_ERROR_NO_SUCH_DEVICE ;

    if (regType > CyAsSDReg_CSD)
        return CY_AS_ERROR_INVALID_PARAMETER ;

    /* If SD/MMC media is not supported on the addressed bus, return error. */
    if ((dev_p->media_supported[bus] & (1 << CyAsMediaSDFlash)) == 0)
        return CY_AS_ERROR_INVALID_PARAMETER ;

    /*
     * Find the amount of data to be returned. This will be the minimum of
     * the actual data length, and the length requested.
     */
    switch (regType)
    {
        case CyAsSDReg_OCR:
            length = CY_AS_SD_REG_OCR_LENGTH ;
            break ;

        case CyAsSDReg_CID:
            length = CY_AS_SD_REG_CID_LENGTH ;
            break ;

        case CyAsSDReg_CSD:
            length = CY_AS_SD_REG_CSD_LENGTH ;
            break ;

        default:
            length = 0 ;
            CyAsHalAssert(0) ;
    }

    if (length < data_p->length)
        data_p->length = length ;
    length = data_p->length ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SD_REGISTER_READ, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, (CreateAddress(bus, device, 0) | (uint16_t)regType)) ;

    reply_p = CyAsLLCreateResponse(dev_p, CY_AS_SD_REG_MAX_RESP_LENGTH) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    if (cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return MyHandleResponseSDRegRead(dev_p, req_p, reply_p, data_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_SDREGISTERREAD,
            data_p, dev_p->func_cbs_stor, CY_AS_REQUEST_RESPONSE_EX,
            req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the MiscFuncCallback */
        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageCreatePPartition(
        CyAsDeviceHandle        handle,                 /* Handle to the device of interest */
        CyAsBusNumber_t         bus,
        uint32_t                device,
        uint32_t                size, /* of P-port only partition in blocks */
        CyAsFunctionCallback    cb,
        uint32_t                client)
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /* Partitions cannot be created or deleted while the USB stack is active. */
    if (dev_p->usb_count)
        return CY_AS_ERROR_USB_RUNNING ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_PARTITION_STORAGE, CY_RQT_STORAGE_RQT_CONTEXT, 3) ;
    if (req_p == 0)
    {
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    /* Reserve space for the reply, the reply data will not exceed one word */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }
    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, 0x00) );
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((size >> 16) & 0xffff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 2, (uint16_t)(size & 0xffff)) ;

    if(cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return MyHandleResponseNoData(dev_p, req_p, reply_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_PARTITION,
            0, dev_p->func_cbs_stor, CY_AS_REQUEST_RESPONSE_EX,
                req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the FuncCallback */
        return ret ;

    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageRemovePPartition(
        CyAsDeviceHandle        handle,
        CyAsBusNumber_t         bus,
        uint32_t                device,
        CyAsFunctionCallback    cb,
        uint32_t                client)
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /* Partitions cannot be created or deleted while the USB stack is active. */
    if (dev_p->usb_count)
        return CY_AS_ERROR_USB_RUNNING ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_PARTITION_ERASE, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
    {
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    /* Reserve space for the reply, the reply data will not exceed one word */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, 0x00) );

    if(cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return MyHandleResponseNoData(dev_p, req_p, reply_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_NODATA,
            0, dev_p->func_cbs_stor, CY_AS_REQUEST_RESPONSE_EX,
                req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the FuncCallback */
        return ret ;

    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseGetTransferAmount(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         CyAsMSCProgressData *data)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t code = CyAsLLRequestResponse_GetCode(reply_p) ;
    uint16_t v1, v2 ;

    if (code != CY_RESP_TRANSFER_COUNT)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    v1 = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    v2 = CyAsLLRequestResponse_GetWord(reply_p, 1) ;
    data->wr_count = (uint32_t)((v1 << 16) | v2) ;

    v1 = CyAsLLRequestResponse_GetWord(reply_p, 2) ;
    v2 = CyAsLLRequestResponse_GetWord(reply_p, 3) ;
    data->rd_count = (uint32_t)((v1 << 16) | v2) ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsStorageGetTransferAmount(
        CyAsDeviceHandle handle,
        CyAsBusNumber_t  bus,
        uint32_t device,
        CyAsMSCProgressData *data_p,
        CyAsFunctionCallback cb,
        uint32_t client
    )
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /* Check if the firmware image supports this feature. */
    if ((dev_p->media_supported[0]) && (dev_p->media_supported[0] == (1 << CyAsMediaNand)))
        return CY_AS_ERROR_NOT_SUPPORTED ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_GET_TRANSFER_AMOUNT, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
    {
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    /* Reserve space for the reply, the reply data will not exceed four words. */
    reply_p = CyAsLLCreateResponse(dev_p, 4) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, 0x00));

    if(cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return MyHandleResponseGetTransferAmount(dev_p, req_p, reply_p, data_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_GETTRANSFERAMOUNT,
            (void *)data_p, dev_p->func_cbs_stor, CY_AS_REQUEST_RESPONSE_EX,
                req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the FuncCallback */
        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;

}

CyAsReturnStatus_t
CyAsStorageErase(
        CyAsDeviceHandle        handle,
        CyAsBusNumber_t         bus,
        uint32_t                device,
        uint32_t                erase_unit,
        uint16_t                num_erase_units,
        CyAsFunctionCallback    cb,
        uint32_t                client
        )
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsStorageActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (bus >= CY_AS_MAX_BUSES)
        return CY_AS_ERROR_NO_SUCH_BUS ;

    if (device >= CY_AS_MAX_STORAGE_DEVICES)
        return CY_AS_ERROR_NO_SUCH_DEVICE ;

    if (dev_p->storage_device_info[bus][device].block_size == 0)
        return CY_AS_ERROR_QUERY_DEVICE_NEEDED ;

    /* If SD is not supported on the specified bus, then return ERROR */
    if(dev_p->storage_device_info[bus][device].type != CyAsMediaSDFlash)
        return CY_AS_ERROR_NOT_SUPPORTED;

    if (num_erase_units == 0)
        return CY_AS_ERROR_SUCCESS ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_ERASE, CY_RQT_STORAGE_RQT_CONTEXT, 5) ;
    if (req_p == 0)
    {
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    /* Reserve space for the reply, the reply data will not exceed four words. */
    reply_p = CyAsLLCreateResponse(dev_p, 4) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, 0x00));
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((erase_unit >> 16) & 0xffff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 2, (uint16_t)(erase_unit & 0xffff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 3, (uint16_t)((num_erase_units >> 8) & 0x00ff)) ;
    CyAsLLRequestResponse_SetWord(req_p, 4, (uint16_t)((num_erase_units << 8) & 0xff00)) ;

    if(cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        ret = MyHandleResponseNoData(dev_p, req_p, reply_p) ;

        /* If error = "invalid response", this (very likely) means that we are not using the SD-only firmware module
        which is the only one supporting StorageErase. In this case force a "non supported" error code */
        if (ret == CY_AS_ERROR_INVALID_RESPONSE)
            ret = CY_AS_ERROR_NOT_SUPPORTED;

        return ret ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_STOR_ERASE,
            0, dev_p->func_cbs_stor, CY_AS_REQUEST_RESPONSE_EX,
                req_p, reply_p, CyAsStorageFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        /* The request and response are freed as part of the FuncCallback */
        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static void
CyAsStorageFuncCallback(CyAsDevice *dev_p,
                        uint8_t context,
                        CyAsLLRequestResponse *rqt,
                        CyAsLLRequestResponse *resp,
                        CyAsReturnStatus_t stat)
{
    CyAsFuncCBNode* node = (CyAsFuncCBNode*)dev_p->func_cbs_stor->head_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyBool              exRequest = (rqt->flags & CY_AS_REQUEST_RESPONSE_EX) == CY_AS_REQUEST_RESPONSE_EX ;
    CyBool              msRequest = (rqt->flags & CY_AS_REQUEST_RESPONSE_MS) == CY_AS_REQUEST_RESPONSE_MS ;
    uint8_t             code ;
    uint8_t             cntxt ;

    CyAsHalAssert(exRequest || msRequest) ;
    CyAsHalAssert(dev_p->func_cbs_stor->count != 0) ;
    CyAsHalAssert(dev_p->func_cbs_stor->type == CYAS_FUNC_CB) ;
    (void) exRequest;
    (void) msRequest;
    (void)cntxt;
    (void)context ;

    cntxt = CyAsLLRequestResponse_GetContext(rqt) ;
    CyAsHalAssert(cntxt == CY_RQT_STORAGE_RQT_CONTEXT) ;

    code = CyAsLLRequestResponse_GetCode(rqt) ;
    switch(code)
    {
    case CY_RQT_START_STORAGE:
        ret = MyHandleResponseStorageStart(dev_p, rqt, resp, stat) ;
        break ;
    case CY_RQT_STOP_STORAGE:
        ret = MyHandleResponseStorageStop(dev_p, rqt, resp, stat) ;
        break ;
    case CY_RQT_CLAIM_STORAGE:
        ret = MyHandleResponseStorageClaim(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_RELEASE_STORAGE:
        ret = MyHandleResponseStorageRelease(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_QUERY_MEDIA:
        CyAsHalAssert(CyFalse) ;        /* Not used any more. */
        break ;
    case CY_RQT_QUERY_BUS:
        CyAsHalAssert(node->data != 0) ;
        ret = MyHandleResponseStorageQueryBus(dev_p, rqt, resp, (uint32_t*)node->data) ;
        break ;
    case CY_RQT_QUERY_DEVICE:
        CyAsHalAssert(node->data != 0) ;
        ret = MyHandleResponseStorageQueryDevice(dev_p, rqt, resp, node->data) ;
        break ;
    case CY_RQT_QUERY_UNIT:
        CyAsHalAssert(node->data != 0) ;
        ret = MyHandleResponseStorageQueryUnit(dev_p, rqt, resp, node->data) ;
        break ;
    case CY_RQT_SD_INTERFACE_CONTROL:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_SD_REGISTER_READ:
        CyAsHalAssert(node->data != 0) ;
        ret = MyHandleResponseSDRegRead(dev_p, rqt, resp, (CyAsStorageSDRegReadData *)node->data) ;
        break ;
    case CY_RQT_PARTITION_STORAGE:
        ret = MyHandleResponseNoData(dev_p, rqt, resp);
        break ;
    case CY_RQT_PARTITION_ERASE:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_GET_TRANSFER_AMOUNT:
        CyAsHalAssert(node->data != 0) ;
        ret = MyHandleResponseGetTransferAmount(dev_p, rqt, resp, (CyAsMSCProgressData *)node->data) ;
        break ;
    case CY_RQT_ERASE:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;

        /* If error = "invalid response", this (very likely) means that we are not using the SD-only firmware module
        which is the only one supporting StorageErase. In this case force a "non supported" error code */
        if (ret == CY_AS_ERROR_INVALID_RESPONSE)
            ret = CY_AS_ERROR_NOT_SUPPORTED;

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

    /* Call the user callback, if there is one */
    if (node->cb_p)
        node->cb_p((CyAsDeviceHandle)dev_p, stat, node->client_data, (CyAsFunctCBType)node->dataType, node->data) ;
    CyAsRemoveCBNode(dev_p->func_cbs_stor) ;
}


static void
CyAsSdioSyncReplyCallback(
        CyAsDevice *dev_p,
        uint8_t context,
        CyAsLLRequestResponse *rqt,
        CyAsLLRequestResponse *resp,
        CyAsReturnStatus_t ret)
{
    (void)rqt ;

    if ((CyAsLLRequestResponse_GetCode(resp) == CY_RESP_SDIO_GET_TUPLE )||
            (CyAsLLRequestResponse_GetCode(resp) == CY_RESP_SDIO_EXT))
    {
        ret = CyAsLLRequestResponse_GetWord(resp, 0) ;
        if ((ret & 0x00FF) != CY_AS_ERROR_SUCCESS)
        {
            if(CyAsLLRequestResponse_GetCode(rqt) == CY_RQT_SDIO_READ_EXTENDED)
                CyAsDmaCancel(dev_p, dev_p->storage_read_endpoint, ret) ;
            else
                CyAsDmaCancel(dev_p, dev_p->storage_write_endpoint, ret) ;
        }
    }
    else
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }

    dev_p->storage_rw_resp_p=resp;
    dev_p->storage_wait = CyFalse ;
    if(((ret & 0x00FF) == CY_AS_ERROR_IO_ABORTED) || ((ret & 0x00FF) == CY_AS_ERROR_IO_SUSPENDED))
        dev_p->storage_error =  (ret & 0x00FF);
    else
        dev_p->storage_error = (ret & 0x00FF)? CY_AS_ERROR_INVALID_RESPONSE : CY_AS_ERROR_SUCCESS ;

    /* Wake any threads/processes that are waiting on the read/write completion. */
    CyAsHalWake (&dev_p->context[context]->channel);
}

CyAsReturnStatus_t
CyAsSdioDeviceCheck(
        CyAsDevice*         dev_p,
        CyAsBusNumber_t     bus,
        uint32_t            device)
{
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (bus >= CY_AS_MAX_BUSES)
        return CY_AS_ERROR_NO_SUCH_BUS ;

    if (device >= CY_AS_MAX_STORAGE_DEVICES)
        return CY_AS_ERROR_NO_SUCH_DEVICE ;

    if (!CyAsDeviceIsAstoriaDev(dev_p))
        return CY_AS_ERROR_NOT_SUPPORTED ;

    return  (IsStorageActive(dev_p)) ;
}

CyAsReturnStatus_t
CyAsSdioDirectIo(
        CyAsDeviceHandle    handle,
        CyAsBusNumber_t     bus,
        uint32_t            device,
        uint8_t             nFunctionNo,
        uint32_t            address,
        uint8_t             miscBuf,
        uint16_t            argument,
        uint8_t             isWrite,
        uint8_t *           data_p )
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint16_t resp_data;

    /*
     * Sanity checks required before sending the request to the
     * firmware.
     */
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;


    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_INVALID_FUNCTION;
    if(CyAsSdioCheckFunctionSuspended(handle,bus,nFunctionNo))
        return CY_AS_ERROR_FUNCTION_SUSPENDED;

    req_p = CyAsLLCreateRequest(dev_p, (isWrite==CyTrue)?CY_RQT_SDIO_WRITE_DIRECT:CY_RQT_SDIO_READ_DIRECT,
            CY_RQT_STORAGE_RQT_CONTEXT, 3) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /*Setting up request*/

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, nFunctionNo) ) ;
    /* D1 */
    if(isWrite==CyTrue)
    {
        CyAsLLRequestResponse_SetWord(req_p, 1,  ((argument<<8)|0x0080|(nFunctionNo<<4)|
                    ((miscBuf&CY_SDIO_RAW)<<3)|((miscBuf&CY_SDIO_REARM_INT)>>5)|(uint16_t)(address>>15)));
    }
    else
    {
        CyAsLLRequestResponse_SetWord(req_p, 1,  (nFunctionNo<<4)|((miscBuf&CY_SDIO_REARM_INT)>>5)|
                (uint16_t)(address>>15));
    }
    /* D2 */
    CyAsLLRequestResponse_SetWord(req_p, 2,  ((uint16_t)((address&0x00007fff)<<1))) ;

    /*Create response*/
    reply_p = CyAsLLCreateResponse(dev_p, 2) ;

    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    /*Sending the request*/
    ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    /*Check reply type*/
    if ( CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_SDIO_DIRECT)
    {
        resp_data = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        if(resp_data>>8)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        }
        else if (data_p!=0)
            *(uint8_t*)(data_p)=(uint8_t)(resp_data&0x00ff);
    }
    else
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }

destroy:
    if(req_p!=0)
        CyAsLLDestroyRequest(dev_p, req_p) ;
    if(reply_p!=0)
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    return ret ;
}


CyAsReturnStatus_t
CyAsSdioDirectRead(
        CyAsDeviceHandle        handle,
        CyAsBusNumber_t         bus,
        uint32_t                device,
        uint8_t                 nFunctionNo,
        uint32_t                address,
        uint8_t                 miscBuf,
        uint8_t *               data_p)
{
    return (CyAsSdioDirectIo(handle,bus,device, nFunctionNo, address, miscBuf, 0x00, CyFalse, data_p));
}

CyAsReturnStatus_t
CyAsSdioDirectWrite(
        CyAsDeviceHandle        handle,
        CyAsBusNumber_t         bus,
        uint32_t                device,
        uint8_t                 nFunctionNo,
        uint32_t                address,
        uint8_t                 miscBuf,
        uint16_t                argument,
        uint8_t *               data_p)
{
    return (CyAsSdioDirectIo(handle,bus,device, nFunctionNo, address, miscBuf, argument, CyTrue, data_p));
}

/*Cmd53 IO*/
CyAsReturnStatus_t
CyAsSdioExtendedIO(
        CyAsDeviceHandle        handle,
        CyAsBusNumber_t         bus,
        uint32_t                device,
        uint8_t                 nFunctionNo,
        uint32_t                address,
        uint8_t                 miscBuf,
        uint16_t                argument,
        uint8_t                 isWrite,
        uint8_t *               data_p ,
        uint8_t                 isResume)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t resp_type;
    uint8_t reqtype;
    uint16_t resp_data;
    CyAsContext *ctxt_p ;
    uint32_t  dmasize,loopcount = 200;
    CyAsEndPointNumber_t ep;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;


    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_INVALID_FUNCTION;
    if(CyAsSdioCheckFunctionSuspended(handle,bus,nFunctionNo))
        return CY_AS_ERROR_FUNCTION_SUSPENDED;


    if ((CyAsDeviceIsStorageAsyncPending(dev_p)) || (dev_p->storage_wait))
        return CY_AS_ERROR_ASYNC_PENDING ;

    /* Request for 0 bytes of blocks is returned as a success*/
    if(argument == 0)
        return CY_AS_ERROR_SUCCESS;

    /* Initialise the request to send to the West Bridge device. */
    if(isWrite == CyTrue)
    {
        reqtype =CY_RQT_SDIO_WRITE_EXTENDED;
        ep=dev_p->storage_write_endpoint;
    }
    else
    {
        reqtype=CY_RQT_SDIO_READ_EXTENDED;
        ep=dev_p->storage_read_endpoint;
    }

    req_p = dev_p->storage_rw_req_p ;
    CyAsLLInitRequest(req_p, reqtype, CY_RQT_STORAGE_RQT_CONTEXT, 3) ;

    /* Initialise the space for reply from the Antioch. */
    reply_p = dev_p->storage_rw_resp_p ;
    CyAsLLInitResponse(reply_p, 2) ;

    /* Setup the DMA request */
    if(!(miscBuf&CY_SDIO_BLOCKMODE))
    {
        if(argument > dev_p->sdiocard[bus].function[nFunctionNo-1].blocksize)
            return CY_AS_ERROR_INVALID_BLOCKSIZE;

    }
    else
    {
        if( argument > 511)
        {
            return CY_AS_ERROR_INVALID_BLOCKSIZE;
        }
    }

    if(argument == 512)
        argument =0;

    dmasize=((miscBuf&CY_SDIO_BLOCKMODE) !=0)? dev_p->sdiocard[bus].function[nFunctionNo-1].blocksize*argument:argument;

    ret = CyAsDmaQueueRequest(dev_p, ep, (void*)(data_p), dmasize, CyFalse,
            (isWrite & CyTrue)?CyFalse:CyTrue, CyAsSyncStorageCallback) ;

    if (ret != CY_AS_ERROR_SUCCESS)
    {
        return ret ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, nFunctionNo|((isResume)?0x80:0x00) )) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, ((uint16_t)nFunctionNo)<<12|
            ((uint16_t)(miscBuf&(CY_SDIO_BLOCKMODE|CY_SDIO_OP_INCR)))<<9|
            (uint16_t)(address>>7)|((isWrite==CyTrue)?0x8000:0x0000 )) ;
    CyAsLLRequestResponse_SetWord(req_p, 2, ((uint16_t)(address&0x0000ffff)<<9) |  argument) ;


    /* Send the request and wait for completion of storage request */
    dev_p->storage_wait = CyTrue ;
    ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyTrue, CyAsSdioSyncReplyCallback) ;

    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsDmaCancel(dev_p, ep, CY_AS_ERROR_CANCELED) ;
    }
    else
    {
        /* Setup the DMA request */
        ctxt_p = dev_p->context[CY_RQT_STORAGE_RQT_CONTEXT] ;
        ret = CyAsDmaDrainQueue(dev_p, ep, CyTrue) ;

        while (loopcount-- > 0)
        {
            if (dev_p->storage_wait == CyFalse)
                break;
            CyAsHalSleepOn(&ctxt_p->channel, 10) ;
        }
        if (dev_p->storage_wait == CyTrue)
        {
            dev_p->storage_wait = CyFalse ;
            CyAsLLRemoveRequest(dev_p, ctxt_p, req_p, CyTrue) ;
            dev_p->storage_error = CY_AS_ERROR_TIMEOUT ;
        }

        ret=dev_p->storage_error;

        if (ret != CY_AS_ERROR_SUCCESS)
        {
            return ret ;
        }


        resp_type = CyAsLLRequestResponse_GetCode(dev_p->storage_rw_resp_p) ;
        if (resp_type == CY_RESP_SDIO_EXT)
        {
            resp_data = CyAsLLRequestResponse_GetWord(reply_p, 0)&0x00ff ;
            if(resp_data)
            {
                ret = CY_AS_ERROR_INVALID_REQUEST ;
            }

        }
        else
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        }
    }
    return ret;

}

static void
CyAsSdioAsyncReplyCallback(
        CyAsDevice*             dev_p,
        uint8_t                 context,
        CyAsLLRequestResponse*  rqt,
        CyAsLLRequestResponse*  resp,
        CyAsReturnStatus_t      ret )
{
    CyAsStorageCallback cb_ms ;
    uint8_t reqtype ;
    uint32_t pendingblocks;
    (void)rqt ;
    (void)context ;

    pendingblocks = 0;
    reqtype = CyAsLLRequestResponse_GetCode(rqt) ;
    if (ret == CY_AS_ERROR_SUCCESS)
    {
        if ((CyAsLLRequestResponse_GetCode(resp) == CY_RESP_SUCCESS_FAILURE) ||
                (CyAsLLRequestResponse_GetCode(resp) == CY_RESP_SDIO_EXT))
        {
            ret = CyAsLLRequestResponse_GetWord(resp, 0) ;
            ret &= 0x00FF ;
        }
        else
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        }
    }

    if (ret != CY_AS_ERROR_SUCCESS)
    {
        if(reqtype == CY_RQT_SDIO_READ_EXTENDED)
            CyAsDmaCancel(dev_p, dev_p->storage_read_endpoint, ret) ;
        else
            CyAsDmaCancel(dev_p, dev_p->storage_write_endpoint, ret) ;

        dev_p->storage_error = ret;
    }



    dev_p->storage_wait = CyFalse ;

    /*
     * If the DMA callback has already been called, the user callback
     * has to be called from here.
     */
    if (!CyAsDeviceIsStorageAsyncPending(dev_p))
    {
        CyAsHalAssert(dev_p->storage_cb_ms != NULL) ;
        cb_ms = dev_p->storage_cb_ms ;

        dev_p->storage_cb = 0 ;
        dev_p->storage_cb_ms = 0 ;

        if ((ret == CY_AS_ERROR_SUCCESS) || (ret == CY_AS_ERROR_IO_ABORTED) || (ret == CY_AS_ERROR_IO_SUSPENDED))
        {
            ret = dev_p->storage_error ;
            pendingblocks = ((uint32_t)CyAsLLRequestResponse_GetWord(resp, 1))<<16;
        }
        else
            ret = CY_AS_ERROR_INVALID_RESPONSE;

        cb_ms((CyAsDeviceHandle)dev_p, dev_p->storage_bus_index, dev_p->storage_device_index,
                (dev_p->storage_unit | pendingblocks), dev_p->storage_block_addr, dev_p->storage_oper, ret) ;
    }
    else
        dev_p->storage_error = ret ;
}


CyAsReturnStatus_t
CyAsSdioExtendedIOAsync(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        uint8_t                     nFunctionNo,
        uint32_t                    address,
        uint8_t                     miscBuf,
        uint16_t                    argument,
        uint8_t                     isWrite,
        uint8_t *                   data_p,
        CyAsStorageCallback       callback )
{

    uint32_t mask ;
    uint32_t dmasize;
    CyAsLLRequestResponse *req_p , *reply_p ;
    uint8_t reqtype;
    CyAsEndPointNumber_t ep;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_INVALID_FUNCTION;
    if(CyAsSdioCheckFunctionSuspended(handle,bus,nFunctionNo))
        return CY_AS_ERROR_FUNCTION_SUSPENDED;

    if (callback == 0)
        return CY_AS_ERROR_NULL_CALLBACK ;

    /* We are supposed to return sucess if the number of
     * blocks is zero
     */
    if(((miscBuf&CY_SDIO_BLOCKMODE)!=0)&&(argument==0))
    {
        callback(handle, bus, device,nFunctionNo,address,((isWrite) ? CyAsOpWrite : CyAsOpRead), CY_AS_ERROR_SUCCESS) ;
        return CY_AS_ERROR_SUCCESS ;
    }


    /*
     * Since async operations can be triggered by interrupt code, we must
     * insure that we do not get multiple async operations going at one time and
     * protect this test and set operation from interrupts.
     */
    mask = CyAsHalDisableInterrupts() ;
    if ((CyAsDeviceIsStorageAsyncPending(dev_p)) || (dev_p->storage_wait))
    {
        CyAsHalEnableInterrupts(mask) ;
        return CY_AS_ERROR_ASYNC_PENDING ;
    }
    CyAsDeviceSetStorageAsyncPending(dev_p) ;
    CyAsHalEnableInterrupts(mask) ;


    /*
     * Storage information about the currently outstanding request
     */
    dev_p->storage_cb_ms = callback ;
    dev_p->storage_bus_index = bus ;
    dev_p->storage_device_index = device ;
    dev_p->storage_unit = nFunctionNo ;
    dev_p->storage_block_addr = address ;

    if(isWrite == CyTrue)
    {
        reqtype =CY_RQT_SDIO_WRITE_EXTENDED;
        ep=dev_p->storage_write_endpoint;
    }
    else
    {
        reqtype=CY_RQT_SDIO_READ_EXTENDED;
        ep=dev_p->storage_read_endpoint;
    }

    /* Initialise the request to send to the West Bridge. */
    req_p = dev_p->storage_rw_req_p ;
    CyAsLLInitRequest(req_p, reqtype, CY_RQT_STORAGE_RQT_CONTEXT, 3) ;

    /* Initialise the space for reply from the West Bridge. */
    reply_p = dev_p->storage_rw_resp_p ;
    CyAsLLInitResponse(reply_p, 2) ;

    if(!(miscBuf&CY_SDIO_BLOCKMODE))
    {
        if(argument > dev_p->sdiocard[bus].function[nFunctionNo-1].blocksize)
            return CY_AS_ERROR_INVALID_BLOCKSIZE;

    }
    else
    {
        if( argument > 511)
        {
            return CY_AS_ERROR_INVALID_BLOCKSIZE;
        }
    }

    if(argument == 512)
        argument =0;
    dmasize=((miscBuf&CY_SDIO_BLOCKMODE) !=0)? dev_p->sdiocard[bus].function[nFunctionNo-1].blocksize*argument:argument;

    /* Setup the DMA request and adjust the storage operation if we are reading */
    if (reqtype == CY_RQT_SDIO_READ_EXTENDED)
    {
        ret = CyAsDmaQueueRequest(dev_p, ep, (void*)data_p,dmasize , CyFalse, CyTrue,CyAsAsyncStorageCallback) ;
        dev_p->storage_oper = CyAsOpRead ;
    }
    else if (reqtype == CY_RQT_SDIO_WRITE_EXTENDED)
    {
        ret = CyAsDmaQueueRequest(dev_p, ep, (void*)data_p, dmasize, CyFalse, CyFalse,CyAsAsyncStorageCallback) ;
        dev_p->storage_oper = CyAsOpWrite ;
    }
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsDeviceClearStorageAsyncPending(dev_p) ;
        return ret ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, nFunctionNo )) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, ((uint16_t)nFunctionNo)<<12|
            ((uint16_t)(miscBuf&(CY_SDIO_BLOCKMODE|CY_SDIO_OP_INCR)))<<9|
            (uint16_t)(address>>7)|((isWrite==CyTrue)?0x8000:0x0000 )) ;
    CyAsLLRequestResponse_SetWord(req_p, 2, ((uint16_t)(address&0x0000ffff)<<9) |  argument) ;


    /* Send the request and wait for completion of storage request */
    dev_p->storage_wait = CyTrue ;
    ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyTrue, CyAsSdioAsyncReplyCallback) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsDmaCancel(dev_p, ep, CY_AS_ERROR_CANCELED) ;
        CyAsDeviceClearStorageAsyncPending(dev_p) ;
    }
    else
    {
        CyAsDmaKickStart(dev_p, ep) ;
    }

    return ret ;
}

/* CMD53 Extended Read*/
CyAsReturnStatus_t
CyAsSdioExtendedRead(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        uint8_t                     nFunctionNo,
        uint32_t                    address,
        uint8_t                     miscBuf,
        uint16_t                    argument,
        uint8_t *                   data_p,
        CyAsSdioCallback            callback )
{
    if (callback==0)
        return CyAsSdioExtendedIO(handle,bus,device,nFunctionNo,address,miscBuf,argument,CyFalse,data_p,0);

    return CyAsSdioExtendedIOAsync(handle,bus,device,nFunctionNo,address,miscBuf,argument,CyFalse,data_p,callback);
}

/* CMD53 Extended Write*/
CyAsReturnStatus_t
CyAsSdioExtendedWrite(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        uint8_t                     nFunctionNo,
        uint32_t                    address,
        uint8_t                     miscBuf,
        uint16_t                    argument,
        uint8_t *                   data_p,
        CyAsSdioCallback            callback )
{
    if (callback==0)
        return CyAsSdioExtendedIO(handle,bus,device,nFunctionNo,address,miscBuf,argument,CyTrue,data_p,0);

    return CyAsSdioExtendedIOAsync(handle,bus,device,nFunctionNo,address,miscBuf,argument,CyTrue,data_p,callback);
}


/* Read the CIS info tuples for the given function and Tuple ID*/
CyAsReturnStatus_t
CyAsSdioGetCISInfo(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        uint8_t                     nFunctionNo,
        uint16_t                    tupleId,
        uint8_t *                   data_p )
{

    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint16_t resp_data;
    CyAsContext *ctxt_p ;
    uint32_t loopcount = 200;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(!( CyAsSdioCheckFunctionInitialized(handle,bus,0) ))
        return CY_AS_ERROR_INVALID_FUNCTION;

    if ((CyAsDeviceIsStorageAsyncPending(dev_p)) || (dev_p->storage_wait))
        return CY_AS_ERROR_ASYNC_PENDING ;


    /* Initialise the request to send to the Antioch. */
    req_p = dev_p->storage_rw_req_p ;
    CyAsLLInitRequest(req_p, CY_RQT_SDIO_GET_TUPLE, CY_RQT_STORAGE_RQT_CONTEXT, 2) ;

    /* Initialise the space for reply from the Antioch. */
    reply_p = dev_p->storage_rw_resp_p ;
    CyAsLLInitResponse(reply_p, 3) ;

    /* Setup the DMA request */
    ret = CyAsDmaQueueRequest(dev_p, dev_p->storage_read_endpoint, data_p+1, 255, CyFalse, CyTrue,
            CyAsSyncStorageCallback) ;

    if (ret != CY_AS_ERROR_SUCCESS)
    {
        return ret ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, nFunctionNo )) ;

    /* Set tuple id to fetch. */
    CyAsLLRequestResponse_SetWord(req_p, 1, tupleId<<8) ;

    /* Send the request and wait for completion of storage request */
    dev_p->storage_wait = CyTrue ;
    ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyTrue, CyAsSdioSyncReplyCallback) ;


    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsDmaCancel(dev_p, dev_p->storage_read_endpoint, CY_AS_ERROR_CANCELED) ;
    }
    else
    {
        /* Setup the DMA request */
        ctxt_p = dev_p->context[CY_RQT_STORAGE_RQT_CONTEXT] ;
        ret = CyAsDmaDrainQueue(dev_p, dev_p->storage_read_endpoint, CyTrue) ;

        while (loopcount-- > 0)
        {
            if (dev_p->storage_wait == CyFalse)
                break;
            CyAsHalSleepOn(&ctxt_p->channel, 10) ;
        }

        if (dev_p->storage_wait == CyTrue)
        {
            dev_p->storage_wait = CyFalse ;
            CyAsLLRemoveRequest(dev_p, ctxt_p, req_p, CyTrue) ;
            return CY_AS_ERROR_TIMEOUT ;
        }
        ret = dev_p->storage_error ;

        if (ret != CY_AS_ERROR_SUCCESS)
        {
            return ret ;
        }

        if ( CyAsLLRequestResponse_GetCode(dev_p->storage_rw_resp_p) == CY_RESP_SDIO_GET_TUPLE)
        {
            resp_data = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
            if(resp_data)
            {
                ret = CY_AS_ERROR_INVALID_REQUEST ;
            }
            else if (data_p!=0)
                *(uint8_t*)data_p=(uint8_t)(CyAsLLRequestResponse_GetWord(reply_p, 0)&0x00ff);
        }
        else
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        }
    }
    return ret;
}

/*Query Device*/
CyAsReturnStatus_t
CyAsSdioQueryCard(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        CyAsSDIOCard*               data_p )
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    uint8_t resp_type;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    /* Allocating memory to the SDIO device structure in dev_p */

    CyAsHalMemSet(& dev_p->sdiocard[bus],0,sizeof(CyAsSDIODevice));

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SDIO_QUERY_CARD, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, 0 )) ;

    reply_p = CyAsLLCreateResponse(dev_p, 5) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;

    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    resp_type = CyAsLLRequestResponse_GetCode(reply_p);
    if ( resp_type == CY_RESP_SDIO_QUERY_CARD)
    {
        dev_p->sdiocard[bus].card.num_functions     = (uint8_t)((reply_p->data[0]&0xff00)>>8);
        dev_p->sdiocard[bus].card.memory_present    = (uint8_t)reply_p->data[0]&0x0001;
        dev_p->sdiocard[bus].card.manufacturer_Id   = reply_p->data[1];
        dev_p->sdiocard[bus].card.manufacturer_info = reply_p->data[2];
        dev_p->sdiocard[bus].card.blocksize         = reply_p->data[3];
        dev_p->sdiocard[bus].card.maxblocksize      = reply_p->data[3];
        dev_p->sdiocard[bus].card.card_capability   = (uint8_t)((reply_p->data[4]&0xff00)>>8);
        dev_p->sdiocard[bus].card.sdio_version      = (uint8_t)(reply_p->data[4]&0x00ff);
        dev_p->sdiocard[bus].function_init_map      = 0x01;
        data_p->num_functions       = dev_p->sdiocard[bus].card.num_functions;
        data_p->memory_present      = dev_p->sdiocard[bus].card.memory_present;
        data_p->manufacturer_Id     = dev_p->sdiocard[bus].card.manufacturer_Id;
        data_p->manufacturer_info   = dev_p->sdiocard[bus].card.manufacturer_info;
        data_p->blocksize           = dev_p->sdiocard[bus].card.blocksize;
        data_p->maxblocksize        = dev_p->sdiocard[bus].card.maxblocksize;
        data_p->card_capability     = dev_p->sdiocard[bus].card.card_capability;
        data_p->sdio_version        = dev_p->sdiocard[bus].card.sdio_version;
    }
    else
    {
        if (resp_type == CY_RESP_SUCCESS_FAILURE)
            ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        else
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }
destroy:
    if(req_p!=0)
        CyAsLLDestroyRequest(dev_p, req_p) ;
    if(reply_p!=0)
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    return ret ;
}

/*Reset SDIO card. */
CyAsReturnStatus_t
CyAsSdioResetCard(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t         bus,
        uint32_t                device )
{

    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t resp_type;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    ret = CyAsSdioDeviceCheck(dev_p,bus,device);

    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(dev_p->sdiocard != 0)
    {
        dev_p->sdiocard[bus].function_init_map=0;
        dev_p->sdiocard[bus].function_suspended_map = 0;
    }


    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SDIO_RESET_DEV, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;

    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /*Setup mailbox */
    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, 0) ) ;

    reply_p = CyAsLLCreateResponse(dev_p, 2) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;


    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;


    resp_type = CyAsLLRequestResponse_GetCode(reply_p) ;


    if (resp_type == CY_RESP_SUCCESS_FAILURE)
    {
        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        if(ret == CY_AS_ERROR_SUCCESS)
        {
            ret = CyAsSdioQueryCard(handle,bus,device,0);
        }
    }
    else
        ret = CY_AS_ERROR_INVALID_RESPONSE ;

destroy:
    if(req_p!=0)
        CyAsLLDestroyRequest(dev_p, req_p) ;
    if(reply_p!=0)
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    return ret ;
}

/* Initialise an IO function*/
CyAsReturnStatus_t
CyAsSdioInitFunction(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        uint8_t                     nFunctionNo,
        uint8_t                     miscBuf     )
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t resp_type;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    ret = CyAsSdioDeviceCheck(dev_p,bus,device);

    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,0)))
        return CY_AS_ERROR_NOT_RUNNING;

    if((CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
    {
        if(miscBuf&CY_SDIO_FORCE_INIT)
            dev_p->sdiocard[bus].function_init_map&=(~(1<<nFunctionNo));
        else
            return CY_AS_ERROR_ALREADY_RUNNING;
    }

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SDIO_INIT_FUNCTION, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, nFunctionNo )) ;

    reply_p = CyAsLLCreateResponse(dev_p, 5) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;

    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    resp_type = CyAsLLRequestResponse_GetCode(reply_p) ;

    if (resp_type == CY_RESP_SDIO_INIT_FUNCTION)
    {

        dev_p->sdiocard[bus].function[nFunctionNo-1].function_code      = (uint8_t)((reply_p->data[0]&0xff00)>>8);
        dev_p->sdiocard[bus].function[nFunctionNo-1].extended_func_code = (uint8_t)reply_p->data[0]&0x00ff;
        dev_p->sdiocard[bus].function[nFunctionNo-1].blocksize          =  reply_p->data[1];
        dev_p->sdiocard[bus].function[nFunctionNo-1].maxblocksize       =  reply_p->data[1];
        dev_p->sdiocard[bus].function[nFunctionNo-1].card_psn           = (uint32_t)(reply_p->data[2])<<16;
        dev_p->sdiocard[bus].function[nFunctionNo-1].card_psn           |=(uint32_t)(reply_p->data[3]);
        dev_p->sdiocard[bus].function[nFunctionNo-1].csa_bits           = (uint8_t)((reply_p->data[4]&0xff00)>>8);
        dev_p->sdiocard[bus].function[nFunctionNo-1].wakeup_support     = (uint8_t)(reply_p->data[4]&0x0001);
        dev_p->sdiocard[bus].function_init_map                          |= (1<<nFunctionNo);
        CyAsSdioClearFunctionSuspended(handle,bus,nFunctionNo);

    }
    else
    {
        if (resp_type == CY_RESP_SUCCESS_FAILURE)
            ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        else
            ret = CY_AS_ERROR_INVALID_FUNCTION ;
    }

destroy:
    if(req_p!=0)
        CyAsLLDestroyRequest(dev_p, req_p) ;
    if(reply_p!=0)
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    return ret ;
}

/*Query individual functions. */
CyAsReturnStatus_t
CyAsSdioQueryFunction(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        uint8_t                     nFunctionNo,
        CyAsSDIOFunc*               data_p )
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsReturnStatus_t ret;

    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_INVALID_FUNCTION;

    data_p->blocksize           =   dev_p->sdiocard[bus].function[nFunctionNo-1].blocksize;
    data_p->card_psn            =   dev_p->sdiocard[bus].function[nFunctionNo-1].card_psn;
    data_p->csa_bits            =   dev_p->sdiocard[bus].function[nFunctionNo-1].csa_bits;
    data_p->extended_func_code  =   dev_p->sdiocard[bus].function[nFunctionNo-1].extended_func_code;
    data_p->function_code       =   dev_p->sdiocard[bus].function[nFunctionNo-1].function_code;
    data_p->maxblocksize        =   dev_p->sdiocard[bus].function[nFunctionNo-1].maxblocksize;
    data_p->wakeup_support      =   dev_p->sdiocard[bus].function[nFunctionNo-1].wakeup_support;


    return CY_AS_ERROR_SUCCESS;
}

/* Abort the Current Extended IO Operation*/
CyAsReturnStatus_t
CyAsSdioAbortFunction(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        uint8_t                     nFunctionNo)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t resp_type;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_INVALID_FUNCTION;

    if ((CyAsDeviceIsStorageAsyncPending(dev_p)) || (dev_p->storage_wait))
    {
        if(!(CyAsSdioGetCardCapability(handle,bus) & CY_SDIO_SDC ))
        {
            return CY_AS_ERROR_INVALID_COMMAND;
        }
    }

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SDIO_ABORT_IO, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;

    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /*Setup mailbox */
    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, nFunctionNo) ) ;

    reply_p = CyAsLLCreateResponse(dev_p, 2) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    resp_type = CyAsLLRequestResponse_GetCode(reply_p) ;

    if (resp_type == CY_RESP_SUCCESS_FAILURE)
        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    else
        ret = CY_AS_ERROR_INVALID_RESPONSE ;


destroy:
    if(req_p!=0)
        CyAsLLDestroyRequest(dev_p, req_p) ;
    if(reply_p!=0)
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    return ret ;
}

/* Suspend IO to current function*/
CyAsReturnStatus_t
CyAsSdioSuspend(
        CyAsDeviceHandle        handle,
        CyAsBusNumber_t         bus,
        uint32_t                device,
        uint8_t                 nFunctionNo)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_INVALID_FUNCTION;
    if(!(CyAsSdioCheckSupportBusSuspend(handle,bus)))
        return CY_AS_ERROR_INVALID_FUNCTION;
    if(!(CyAsSdioGetCardCapability(handle,bus) & CY_SDIO_SDC))
        return CY_AS_ERROR_INVALID_FUNCTION;
    if(CyAsSdioCheckFunctionSuspended(handle,bus,nFunctionNo))
        return CY_AS_ERROR_FUNCTION_SUSPENDED;

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SDIO_SUSPEND, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /*Setup mailbox */
    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, nFunctionNo) ) ;

    reply_p = CyAsLLCreateResponse(dev_p, 2) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }
    ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;

    if (ret == CY_AS_ERROR_SUCCESS)
    {
        ret = CyAsLLRequestResponse_GetCode(reply_p) ;
        CyAsSdioSetFunctionSuspended(handle,bus,nFunctionNo);
    }

    if(req_p!=0)
        CyAsLLDestroyRequest(dev_p, req_p) ;
    if(reply_p!=0)
        CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

/*Resume suspended function*/
CyAsReturnStatus_t
CyAsSdioResume(
        CyAsDeviceHandle        handle,
        CyAsBusNumber_t         bus,
        uint32_t                device,
        uint8_t                 nFunctionNo,
        CyAsOperType            op,
        uint8_t                 miscBuf,
        uint16_t                pendingblockcount,
        uint8_t                 *data_p
        )
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t resp_data, ret = CY_AS_ERROR_SUCCESS ;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_INVALID_FUNCTION;

    /* If suspend resume is not supported return */
    if(!(CyAsSdioCheckSupportBusSuspend(handle,bus)))
        return CY_AS_ERROR_INVALID_FUNCTION;

    /* if the function is not suspended return. */
    if(!(CyAsSdioCheckFunctionSuspended(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_INVALID_FUNCTION;

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SDIO_RESUME, CY_RQT_STORAGE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /*Setup mailbox */
    CyAsLLRequestResponse_SetWord(req_p, 0, CreateAddress(bus, (uint8_t)device, nFunctionNo) ) ;

    reply_p = CyAsLLCreateResponse(dev_p, 2) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }
    ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;

    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;


    if ( CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_SDIO_RESUME)
    {
        resp_data = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        if(resp_data & 0x00ff)
        {
            /* Send extended read request to resume the read. */
            if(op == CyAsOpRead)
            {
                ret = CyAsSdioExtendedIO(handle,bus,device,nFunctionNo,0,miscBuf,pendingblockcount,CyFalse,data_p,1);
            }
            else
            {
                ret = CyAsSdioExtendedIO(handle,bus,device,nFunctionNo,0,miscBuf,pendingblockcount,CyTrue,data_p,1);
            }
        }
        else
        {
            ret= CY_AS_ERROR_SUCCESS;
        }

    }
    else
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }

destroy:
    CyAsSdioClearFunctionSuspended(handle,bus,nFunctionNo);
    if(req_p!=0)
        CyAsLLDestroyRequest(dev_p, req_p) ;
    if(reply_p!=0)
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    return ret ;

}

/*Set function blocksize. Size cannot exceed max block size for the function*/
CyAsReturnStatus_t
CyAsSdioSetBlocksize(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        uint8_t                     nFunctionNo,
        uint16_t                    blocksize)
{
    CyAsReturnStatus_t ret;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    ret = CyAsSdioDeviceCheck(dev_p,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_INVALID_FUNCTION;
    if( nFunctionNo == 0)
    {
        if ( blocksize > CyAsSdioGetCardMaxBlocksize(handle,bus))
            return CY_AS_ERROR_INVALID_BLOCKSIZE;
        else if (blocksize == CyAsSdioGetCardBlocksize(handle,bus))
            return CY_AS_ERROR_SUCCESS;
    }
    else
    {
        if ( blocksize > CyAsSdioGetFunctionMaxBlocksize(handle,bus,nFunctionNo))
            return CY_AS_ERROR_INVALID_BLOCKSIZE;
        else if (blocksize == CyAsSdioGetFunctionBlocksize(handle,bus,nFunctionNo))
            return CY_AS_ERROR_SUCCESS;
    }

    ret = CyAsSdioDirectWrite(handle,bus,device,0,(uint16_t)(nFunctionNo<<8)|0x10,0,blocksize&0x00ff,0);
    if(ret != CY_AS_ERROR_SUCCESS )
        return ret;
    ret = CyAsSdioDirectWrite(handle,bus,device,0,(uint16_t)(nFunctionNo<<8)|0x11,0,(blocksize&0xff00)>>8,0);
    if (ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(nFunctionNo ==0)
        CyAsSdioSetCardBlockSize(handle,bus,blocksize);
    else
        CyAsSdioSetFunctionBlockSize(handle,bus,nFunctionNo,blocksize);
    return ret;
}

/* Deinitialize an SDIO function*/
CyAsReturnStatus_t
CyAsSdioDeInitFunction(
        CyAsDeviceHandle            handle,
        CyAsBusNumber_t             bus,
        uint32_t                    device,
        uint8_t                     nFunctionNo)
{
    CyAsReturnStatus_t ret;
    uint8_t temp;

    if(nFunctionNo == 0)
        return CY_AS_ERROR_INVALID_FUNCTION;

    ret = CyAsSdioDeviceCheck((CyAsDevice*)handle,bus,device);
    if( ret != CY_AS_ERROR_SUCCESS )
        return ret;

    if(!(CyAsSdioCheckFunctionInitialized(handle,bus,nFunctionNo)))
        return CY_AS_ERROR_SUCCESS;

    temp =(uint8_t)(((CyAsDevice*)handle)->sdiocard[bus].function_init_map & (~(1<<nFunctionNo)));
    CyAsSdioDirectWrite(handle,bus,device,0,0x02,0,temp,0);
    ((CyAsDevice*)handle)->sdiocard[bus].function_init_map &= (~(1<<nFunctionNo));

    return CY_AS_ERROR_SUCCESS;
}

/* This includes the implementation of the deprecated functions for backward
 * compatibility
 */
#include "cyasstorage_dep_impl.h"

/*[]*/
