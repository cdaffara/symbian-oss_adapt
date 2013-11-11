/* Cypress West Bridge API source file (cyasusb.c)
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
#include "cyasusb.h"
#include "cyaserr.h"
#include "cyasdma.h"
#include "cyaslowlevel.h"
#include "cyaslep2pep.h"
#include "cyasregs.h"
#include "cyasstorage.h"

extern CyAsMediaType
CyAsStorageGetMediaFromAddress(uint16_t v) ;

extern CyAsBusNumber_t
CyAsStorageGetBusFromAddress(uint16_t v) ;

extern uint32_t
CyAsStorageGetDeviceFromAddress(uint16_t v) ;

static CyAsReturnStatus_t
CyAsUsbAckSetupPacket(
                      CyAsDeviceHandle                  handle,         /* Handle to the West Bridge device */
                      CyAsFunctionCallback              cb,             /* The callback if async call */
                      uint32_t                          client          /* Client supplied data */
                      ) ;

static void
CyAsUsbFuncCallback(
                    CyAsDevice *dev_p,
                    uint8_t context,
                    CyAsLLRequestResponse *rqt,
                    CyAsLLRequestResponse *resp,
                    CyAsReturnStatus_t ret) ;
/*
* Reset the USB EP0 state
*/
static void
CyAsUsbResetEP0State(CyAsDevice *dev_p)
{
    CyAsLogDebugMessage(6, "CyAsUsbResetEP0State called") ;

    CyAsDeviceClearAckDelayed(dev_p) ;
    CyAsDeviceClearSetupPacket(dev_p) ;
    if (CyAsDeviceIsUsbAsyncPending(dev_p, 0))
        CyAsUsbCancelAsync((CyAsDeviceHandle)dev_p, 0) ;

    dev_p->usb_pending_buffer = 0 ;
}

/*
* External function to map logical endpoints to physical endpoints
*/
static CyAsReturnStatus_t
IsUsbActive(CyAsDevice *dev_p)
{
    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    if (dev_p->usb_count == 0)
        return CY_AS_ERROR_NOT_RUNNING ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    return CY_AS_ERROR_SUCCESS ;
}

static void
UsbAckCallback(CyAsDeviceHandle h,
               CyAsReturnStatus_t status,
               uint32_t client,
               CyAsFunctCBType  type,
               void* data)
{
    CyAsDevice *dev_p = (CyAsDevice *)h ;

    (void)client ;
    (void)status ;
    (void)data ;

    CyAsHalAssert(type == CY_FUNCT_CB_NODATA) ;

    if (dev_p->usb_pending_buffer)
    {
        CyAsUsbIoCallback cb ;

        cb = dev_p->usb_cb[0] ;
        dev_p->usb_cb[0] = 0 ;
        CyAsDeviceClearUsbAsyncPending(dev_p, 0) ;
        if (cb)
            cb(h, 0, dev_p->usb_pending_size, dev_p->usb_pending_buffer, dev_p->usb_error) ;

        dev_p->usb_pending_buffer = 0 ;
    }

    CyAsDeviceClearSetupPacket(dev_p) ;
}

static void
MyUsbRequestCallbackUsbEvent(CyAsDevice *dev_p, CyAsLLRequestResponse *req_p)
{
    uint16_t ev ;
    uint16_t val ;
    CyAsDeviceHandle h = (CyAsDeviceHandle)dev_p ;

    ev = CyAsLLRequestResponse_GetWord(req_p, 0) ;
    switch(ev)
    {
    case 0:             /* Reserved */
        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_INVALID_REQUEST, 0) ;
        break ;

    case 1:             /* Reserved */
        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_INVALID_REQUEST, 0) ;
        break ;

    case 2:             /* USB Suspend */
        dev_p->usb_last_event = CyAsEventUsbSuspend ;
        if (dev_p->usb_event_cb_ms)
            dev_p->usb_event_cb_ms(h, CyAsEventUsbSuspend, 0) ;
        else if (dev_p->usb_event_cb)
            dev_p->usb_event_cb(h, CyAsEventUsbSuspend, 0) ;
        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        break;

    case 3:             /* USB Resume */
        dev_p->usb_last_event = CyAsEventUsbResume ;
        if (dev_p->usb_event_cb_ms)
            dev_p->usb_event_cb_ms(h, CyAsEventUsbResume, 0) ;
        else if (dev_p->usb_event_cb)
            dev_p->usb_event_cb(h, CyAsEventUsbResume, 0) ;
        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        break ;

    case 4:             /* USB Reset */
        /*
        * If we get a USB reset, the USB host did not understand our response
        * or we timed out for some reason.  Reset our internal state to be ready for
        * another set of enumeration based requests.
        */
        if (CyAsDeviceIsAckDelayed(dev_p))
        {
            CyAsUsbResetEP0State(dev_p) ;
        }

        dev_p->usb_last_event = CyAsEventUsbReset ;
        if (dev_p->usb_event_cb_ms)
            dev_p->usb_event_cb_ms(h, CyAsEventUsbReset, 0) ;
        else if (dev_p->usb_event_cb)
            dev_p->usb_event_cb(h, CyAsEventUsbReset, 0) ;

        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        CyAsDeviceClearUsbHighSpeed(dev_p) ;
        CyAsUsbSetDmaSizes(dev_p) ;
        dev_p->usb_max_tx_size = 0x40 ;
        CyAsDmaSetMaxDmaSize(dev_p, 0x06, 0x40) ;
        break ;

    case 5:             /* USB Set Configuration */
        val = CyAsLLRequestResponse_GetWord(req_p, 1) ;                 /* The configuration to set */
        dev_p->usb_last_event = CyAsEventUsbSetConfig ;
        if (dev_p->usb_event_cb_ms)
            dev_p->usb_event_cb_ms(h, CyAsEventUsbSetConfig, &val) ;
        else if (dev_p->usb_event_cb)
            dev_p->usb_event_cb(h, CyAsEventUsbSetConfig, &val) ;
        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        break ;

    case 6:             /* USB Speed change */
        val = CyAsLLRequestResponse_GetWord(req_p, 1) ;                 /* Connect speed */
        dev_p->usb_last_event = CyAsEventUsbSpeedChange ;
        if (dev_p->usb_event_cb_ms)
            dev_p->usb_event_cb_ms(h, CyAsEventUsbSpeedChange, &val) ;
        else if (dev_p->usb_event_cb)
            dev_p->usb_event_cb(h, CyAsEventUsbSpeedChange, &val) ;
        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        CyAsDeviceSetUsbHighSpeed(dev_p) ;
        CyAsUsbSetDmaSizes(dev_p) ;
        dev_p->usb_max_tx_size = 0x200 ;
        CyAsDmaSetMaxDmaSize(dev_p, 0x06, 0x200) ;
        break ;

    case 7:             /* USB Clear Feature */
        val = CyAsLLRequestResponse_GetWord(req_p, 1) ;                 /* EP Number */
        if (dev_p->usb_event_cb_ms)
            dev_p->usb_event_cb_ms(h, CyAsEventUsbClearFeature, &val) ;
        if (dev_p->usb_event_cb)
            dev_p->usb_event_cb(h, CyAsEventUsbClearFeature, &val) ;
        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        break ;

    default:
        CyAsHalPrintMessage("Invalid event type\n") ;
        CyAsLLSendDataResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_RESP_USB_INVALID_EVENT, sizeof(ev), &ev) ;
        break ;
    }
}

static void
MyUsbRequestCallbackUsbData(CyAsDevice *dev_p, CyAsLLRequestResponse *req_p)
{
    CyAsEndPointNumber_t ep ;
    uint8_t type ;
    uint16_t len ;
    uint16_t val ;
    CyAsDeviceHandle h = (CyAsDeviceHandle)dev_p ;

    val = CyAsLLRequestResponse_GetWord(req_p, 0) ;
    ep = (CyAsEndPointNumber_t)((val >> 13) & 0x01) ;
    len = (val & 0x1ff) ;

    CyAsHalAssert(len <= 64) ;
    CyAsLLRequestResponse_Unpack(req_p, 1, len, dev_p->usb_ep_data) ;

    type = (uint8_t)((val >> 14) & 0x03) ;
    if (type == 0)
    {
        if (CyAsDeviceIsAckDelayed(dev_p))
        {
            /*
            * A setup packet has arrived while we are processing a previous setup packet.
            * Reset our state with respect to EP0 to be ready to process the new packet.
            */
            CyAsUsbResetEP0State(dev_p) ;
        }

        if (len != 8)
            CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_INVALID_REQUEST, 0) ;
        else
        {
            CyAsDeviceClearEp0Stalled(dev_p) ;
            CyAsDeviceSetSetupPacket(dev_p) ;
            CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
            if (dev_p->usb_event_cb_ms)
                dev_p->usb_event_cb_ms(h, CyAsEventUsbSetupPacket, dev_p->usb_ep_data) ;
            else
                dev_p->usb_event_cb(h, CyAsEventUsbSetupPacket, dev_p->usb_ep_data) ;

            if ((!CyAsDeviceIsAckDelayed(dev_p)) && (!CyAsDeviceIsEp0Stalled(dev_p)))
            {
                CyAsUsbAckSetupPacket(h, UsbAckCallback, 0) ;
            }
        }
    }
    else if (type == 2)
    {
        if (len != 0)
            CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_INVALID_REQUEST, 0) ;
        else
        {
            if (dev_p->usb_event_cb_ms)
                dev_p->usb_event_cb_ms(h, CyAsEventUsbStatusPacket, 0) ;
            else
                dev_p->usb_event_cb(h, CyAsEventUsbStatusPacket, 0) ;

            CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
        }
    }
    else if (type == 1)
    {
        /*
        * We need to hand the data associated with these endpoints to the DMA
        * module.
        */
        CyAsDmaReceivedData(dev_p, ep, len, dev_p->usb_ep_data) ;
        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
    }
}

static void
MyUsbRequestCallbackInquiry(CyAsDevice *dev_p, CyAsLLRequestResponse *req_p)
{
    CyAsUsbInquiryData_dep cbdata ;
    CyAsUsbInquiryData cbdata_ms ;
    void *data ;
    uint16_t val ;
    CyAsDeviceHandle h = (CyAsDeviceHandle)dev_p ;
    uint8_t def_inq_data[64] ;
    uint8_t evpd ;
    uint8_t codepage ;
    CyBool updated ;
    uint16_t length ;

    CyAsBusNumber_t bus ;
    uint32_t        device ;
    CyAsMediaType   media ;

    val    = CyAsLLRequestResponse_GetWord(req_p, 0) ;
    bus    = CyAsStorageGetBusFromAddress(val) ;
    device = CyAsStorageGetDeviceFromAddress(val) ;
    media  = CyAsStorageGetMediaFromAddress(val) ;

    val      = CyAsLLRequestResponse_GetWord(req_p, 1) ;
    evpd     = (uint8_t)((val >> 8) & 0x01) ;
    codepage = (uint8_t)(val & 0xff) ;

    length = CyAsLLRequestResponse_GetWord(req_p, 2) ;
    data   = (void *)def_inq_data ;

    updated = CyFalse ;

    if (dev_p->usb_event_cb_ms)
    {
        cbdata_ms.bus = bus ;
        cbdata_ms.device = device ;
        cbdata_ms.updated = updated ;
        cbdata_ms.evpd = evpd ;
        cbdata_ms.codepage = codepage ;
        cbdata_ms.length = length ;
        cbdata_ms.data = data ;

        CyAsHalAssert(cbdata_ms.length <= sizeof(def_inq_data)) ;
        CyAsLLRequestResponse_Unpack(req_p, 3, cbdata_ms.length, cbdata_ms.data) ;

        dev_p->usb_event_cb_ms(h, CyAsEventUsbInquiryBefore, &cbdata_ms) ;

        updated = cbdata_ms.updated;
        data    = cbdata_ms.data ;
        length  = cbdata_ms.length ;
    }
    else if (dev_p->usb_event_cb)
    {
        cbdata.media = media ;
        cbdata.updated = updated ;
        cbdata.evpd = evpd ;
        cbdata.codepage = codepage ;
        cbdata.length = length ;
        cbdata.data = data ;

        CyAsHalAssert(cbdata.length <= sizeof(def_inq_data)) ;
        CyAsLLRequestResponse_Unpack(req_p, 3, cbdata.length, cbdata.data) ;

        dev_p->usb_event_cb(h, CyAsEventUsbInquiryBefore, &cbdata) ;

        updated = cbdata.updated ;
        data    = cbdata.data ;
        length  = cbdata.length ;
    }

    if (updated && length > 192)
        CyAsHalPrintMessage("An inquiry result from a CyAsEventUsbInquiryBefore event was greater than 192 bytes.") ;

    /* Now send the reply with the data back to the West Bridge device */
    if (updated && length <= 192)
    {
        /*
        * The callback function modified the inquiry data, ship the data back
        * to the West Bridge firmware.
        */
        CyAsLLSendDataResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_RESP_INQUIRY_DATA, length, data) ;
    }
    else
    {
        /*
        * The callback did not modify the data, just acknowledge that we
        * processed the request
        */
        CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 1) ;
    }

    if (dev_p->usb_event_cb_ms)
        dev_p->usb_event_cb_ms(h, CyAsEventUsbInquiryAfter, &cbdata_ms) ;
    else if (dev_p->usb_event_cb)
        dev_p->usb_event_cb(h, CyAsEventUsbInquiryAfter, &cbdata) ;
}

static void
MyUsbRequestCallbackStartStop(CyAsDevice *dev_p, CyAsLLRequestResponse *req_p)
{
    CyAsBusNumber_t bus ;
    CyAsMediaType media ;
    uint32_t device ;
    uint16_t val ;

    if (dev_p->usb_event_cb_ms || dev_p->usb_event_cb)
    {
        CyBool loej ;
        CyBool start ;
        CyAsDeviceHandle h = (CyAsDeviceHandle)dev_p ;

        val = CyAsLLRequestResponse_GetWord(req_p, 0) ;
        bus = CyAsStorageGetBusFromAddress(val) ;
        device = CyAsStorageGetDeviceFromAddress(val) ;
        media = CyAsStorageGetMediaFromAddress(val) ;

        val = CyAsLLRequestResponse_GetWord(req_p, 1) ;
        loej = (val & 0x02) ? CyTrue : CyFalse ;
        start = (val & 0x01) ? CyTrue: CyFalse ;

        if (dev_p->usb_event_cb_ms)
        {
            CyAsUsbStartStopData cbdata_ms ;

            cbdata_ms.bus = bus ;
            cbdata_ms.device = device ;
            cbdata_ms.loej = loej ;
            cbdata_ms.start = start ;
            dev_p->usb_event_cb_ms(h, CyAsEventUsbStartStop, &cbdata_ms) ;
        }
        else if (dev_p->usb_event_cb)
        {
            CyAsUsbStartStopData_dep cbdata ;

            cbdata.media = media ;
            cbdata.loej = loej ;
            cbdata.start = start ;
            dev_p->usb_event_cb(h, CyAsEventUsbStartStop, &cbdata) ;
        }
    }
    CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 1) ;
}

static void
MyUsbRequestCallbackUknownCBW(CyAsDevice *dev_p, CyAsLLRequestResponse *req_p)
{
    uint16_t val ;
    CyAsDeviceHandle h = (CyAsDeviceHandle)dev_p ;
    uint8_t buf[16] ;

    uint8_t response[4] ;
    uint16_t reqlen ;
    void *request ;
    uint8_t status ;
    uint8_t key ;
    uint8_t asc ;
    uint8_t ascq ;

    val = CyAsLLRequestResponse_GetWord(req_p, 0) ;
    status = 1 ;                                            /* Failed by default */
    key = 0x05 ;                                            /* Invalid command */
    asc = 0x20 ;                                            /* Invalid command */
    ascq = 0x00 ;                                   /* Invalid command */
    reqlen = CyAsLLRequestResponse_GetWord(req_p, 1) ;
    request = buf ;

    CyAsHalAssert(reqlen <= sizeof(buf)) ;
    CyAsLLRequestResponse_Unpack(req_p, 2, reqlen, request) ;

    if (dev_p->usb_event_cb_ms)
    {
        CyAsUsbUnknownCommandData cbdata_ms ;
        cbdata_ms.bus = CyAsStorageGetBusFromAddress(val) ;
        cbdata_ms.device = CyAsStorageGetDeviceFromAddress(val) ;
        cbdata_ms.reqlen = reqlen ;
        cbdata_ms.request = request ;
        cbdata_ms.status = status ;
        cbdata_ms.key = key ;
        cbdata_ms.asc = asc ;
        cbdata_ms.ascq = ascq ;

        dev_p->usb_event_cb_ms(h, CyAsEventUsbUnknownStorage, &cbdata_ms) ;
        status = cbdata_ms.status ;
        key = cbdata_ms.key ;
        asc = cbdata_ms.asc ;
        ascq = cbdata_ms.ascq ;
    }
    else if (dev_p->usb_event_cb)
    {
        CyAsUsbUnknownCommandData_dep cbdata ;
        cbdata.media = CyAsStorageGetMediaFromAddress(val) ;
        cbdata.reqlen = reqlen ;
        cbdata.request = request ;
        cbdata.status = status ;
        cbdata.key = key ;
        cbdata.asc = asc ;
        cbdata.ascq = ascq ;

        dev_p->usb_event_cb(h, CyAsEventUsbUnknownStorage, &cbdata) ;
        status = cbdata.status ;
        key = cbdata.key ;
        asc = cbdata.asc ;
        ascq = cbdata.ascq ;
    }

    response[0] = status ;
    response[1] = key ;
    response[2] = asc ;
    response[3] = ascq ;
    CyAsLLSendDataResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_RESP_UNKNOWN_SCSI_COMMAND, sizeof(response), response) ;
}

static void
MyUsbRequestCallbackMSCProgress(CyAsDevice *dev_p, CyAsLLRequestResponse *req_p)
{
    uint16_t val1, val2 ;
    CyAsDeviceHandle h = (CyAsDeviceHandle)dev_p ;

    if ((dev_p->usb_event_cb) || (dev_p->usb_event_cb_ms))
    {
        CyAsMSCProgressData cbdata ;

        val1 = CyAsLLRequestResponse_GetWord(req_p, 0) ;
        val2 = CyAsLLRequestResponse_GetWord(req_p, 1) ;
        cbdata.wr_count = (uint32_t)((val1 << 16) | val2) ;

        val1 = CyAsLLRequestResponse_GetWord(req_p, 2) ;
        val2 = CyAsLLRequestResponse_GetWord(req_p, 3) ;
        cbdata.rd_count = (uint32_t)((val1 << 16) | val2) ;

        if (dev_p->usb_event_cb)
            dev_p->usb_event_cb(h, CyAsEventUsbMSCProgress, &cbdata) ;
        else
            dev_p->usb_event_cb_ms(h, CyAsEventUsbMSCProgress, &cbdata) ;
    }

    CyAsLLSendStatusResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
}

/*
* This function processes the requests delivered from the firmware within the West Bridge
* device that are delivered in the USB context.  These requests generally are EP0 and
* EP1 related requests or USB events.
*/
static void
MyUsbRequestCallback(CyAsDevice *dev_p, uint8_t context, CyAsLLRequestResponse *req_p, CyAsLLRequestResponse *resp_p,
                     CyAsReturnStatus_t ret)
{
    uint16_t val ;
    uint8_t code = CyAsLLRequestResponse_GetCode(req_p) ;

    (void)resp_p ;
    (void)context ;
    (void)ret ;

    switch(code)
    {
    case CY_RQT_USB_EVENT:
        MyUsbRequestCallbackUsbEvent(dev_p, req_p) ;
        break ;

    case CY_RQT_USB_EP_DATA:
        dev_p->usb_last_event = CyAsEventUsbSetupPacket ;
        MyUsbRequestCallbackUsbData(dev_p, req_p) ;
        break ;

    case CY_RQT_SCSI_INQUIRY_COMMAND:
        dev_p->usb_last_event = CyAsEventUsbInquiryAfter ;
        MyUsbRequestCallbackInquiry(dev_p, req_p) ;
        break ;

    case CY_RQT_SCSI_START_STOP_COMMAND:
        dev_p->usb_last_event = CyAsEventUsbStartStop ;
        MyUsbRequestCallbackStartStop(dev_p, req_p) ;
        break ;

    case CY_RQT_SCSI_UNKNOWN_COMMAND:
        dev_p->usb_last_event = CyAsEventUsbUnknownStorage ;
        MyUsbRequestCallbackUknownCBW(dev_p, req_p) ;
        break ;

    case CY_RQT_USB_ACTIVITY_UPDATE:
        dev_p->usb_last_event = CyAsEventUsbMSCProgress ;
        MyUsbRequestCallbackMSCProgress(dev_p, req_p) ;
        break ;

    default:
        CyAsHalPrintMessage("Invalid request received on USB context\n") ;
        val = req_p->box0 ;
        CyAsLLSendDataResponse(dev_p, CY_RQT_USB_RQT_CONTEXT, CY_RESP_INVALID_REQUEST, sizeof(val), &val) ;
        break ;
    }
}

static CyAsReturnStatus_t
MyHandleResponseUsbStart(CyAsDevice* dev_p,
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
    * Mark EP 0 and EP1 as 64 byte endpoints
    */
    CyAsDmaSetMaxDmaSize(dev_p, 0, 64) ;
    CyAsDmaSetMaxDmaSize(dev_p, 1, 64) ;

    dev_p->usb_count++ ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsDestroyCBQueue(dev_p->usb_func_cbs) ;
        CyAsLLRegisterRequestCallback(dev_p, CY_RQT_USB_RQT_CONTEXT, 0) ;
    }

    CyAsDeviceClearUSSPending(dev_p) ;

    return ret ;

}

/*
* This function starts the USB stack.  The stack is reference counted so if the stack is already
* started, this function just increments the count.  If the stack has not been started, a start
* request is sent to the West Bridge device.
*
* Note: Starting the USB stack does not cause the USB signals to be connected to the USB pins.  To do
*       this and therefore initiate enumeration, CyAsUsbConnect() must be called.
*/
CyAsReturnStatus_t
CyAsUsbStart(CyAsDeviceHandle handle,
               CyAsFunctionCallback cb,
               uint32_t client)
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbStart called") ;

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

    if(CyAsDeviceIsUSSPending(dev_p))
        return CY_AS_ERROR_STARTSTOP_PENDING ;

    CyAsDeviceSetUSSPending(dev_p) ;

    if (dev_p->usb_count == 0)
    {
        /*
        * Since we are just starting the stack, mark USB as not connected to the
        * remote host
        */
        CyAsDeviceClearUsbConnected(dev_p) ;
        dev_p->usb_phy_config = 0 ;

        /* Queue for 1.0 Async Requests, kept for backwards compatibility */
        dev_p->usb_func_cbs = CyAsCreateCBQueue(CYAS_USB_FUNC_CB) ;
        if(dev_p->usb_func_cbs == 0)
        {
            CyAsDeviceClearUSSPending(dev_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        /* Reset the EP0 state */
        CyAsUsbResetEP0State(dev_p) ;

        /*
        * We register here becuase the start request may cause events to occur before the
        * response to the start request.
        */
        CyAsLLRegisterRequestCallback(dev_p, CY_RQT_USB_RQT_CONTEXT, MyUsbRequestCallback) ;

        /* Create the request to send to the West Bridge device */
        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_START_USB, CY_RQT_USB_RQT_CONTEXT, 0) ;
        if (req_p == 0)
        {
            CyAsDestroyCBQueue(dev_p->usb_func_cbs) ;
            dev_p->usb_func_cbs = 0 ;
            CyAsDeviceClearUSSPending(dev_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        /* Reserve space for the reply, the reply data will not exceed one word */
        reply_p = CyAsLLCreateResponse(dev_p, 1) ;
        if (reply_p == 0)
        {
            CyAsDestroyCBQueue(dev_p->usb_func_cbs) ;
            dev_p->usb_func_cbs = 0 ;
            CyAsLLDestroyRequest(dev_p, req_p) ;
            CyAsDeviceClearUSSPending(dev_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        if(cb == 0)
        {
            ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            return MyHandleResponseUsbStart(dev_p, req_p, reply_p, ret) ;
        }
        else
        {
            ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_START,
                0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
                CyAsUsbFuncCallback) ;

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
        dev_p->usb_count++ ;
        if (cb)
            cb(handle, ret, client, CY_FUNCT_CB_USB_START, 0) ;
    }

    CyAsDeviceClearUSSPending(dev_p) ;

    return ret ;
}

void
CyAsUsbReset(CyAsDevice *dev_p)
{
    int i ;

    CyAsDeviceClearUsbConnected(dev_p) ;

    for(i = 0 ; i < sizeof(dev_p->usb_config)/sizeof(dev_p->usb_config[0]) ; i++)
    {
        /*
         * Cancel all pending USB read/write operations, as it is possible
         * that the USB stack comes up in a different configuration with a
         * different set of endpoints.
         */
        if (CyAsDeviceIsUsbAsyncPending(dev_p, i))
            CyAsUsbCancelAsync(dev_p, (CyAsEndPointNumber_t)i) ;

        dev_p->usb_cb[i] = 0 ;
        dev_p->usb_config[i].enabled = CyFalse ;
    }

    dev_p->usb_phy_config = 0 ;
}

/*
 * This function does all the API side clean-up associated with CyAsUsbStop,
 * without any communication with firmware. This needs to be done when the
 * device is being reset while the USB stack is active.
 */
void
CyAsUsbCleanup(CyAsDevice *dev_p)
{
    if (dev_p->usb_count)
    {
        CyAsUsbResetEP0State(dev_p) ;
        CyAsUsbReset(dev_p) ;
        CyAsHalMemSet(dev_p->usb_config, 0, sizeof(dev_p->usb_config)) ;
        CyAsDestroyCBQueue(dev_p->usb_func_cbs) ;

        dev_p->usb_count = 0 ;
    }
}

static CyAsReturnStatus_t
MyHandleResponseUsbStop(CyAsDevice* dev_p,
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
    CyAsUsbCleanup(dev_p) ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    if (ret != CY_AS_ERROR_SUCCESS)
        CyAsLLRegisterRequestCallback(dev_p, CY_RQT_USB_RQT_CONTEXT, 0) ;

    CyAsDeviceClearUSSPending(dev_p) ;

    return ret ;
}

/*
* This function stops the USB stack.  The USB stack is reference counted so first is reference count is
* decremented.  If the reference count is then zero, a request is sent to the West Bridge device to stop the USB
* stack on the West Bridge device.
*/
CyAsReturnStatus_t
CyAsUsbStop(CyAsDeviceHandle handle,
              CyAsFunctionCallback cb,
              uint32_t client)
{
    CyAsLLRequestResponse *req_p = 0, *reply_p = 0 ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbStop called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsUsbConnected(dev_p))
        return CY_AS_ERROR_USB_CONNECTED ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    if(CyAsDeviceIsUSSPending(dev_p))
        return CY_AS_ERROR_STARTSTOP_PENDING ;

    CyAsDeviceSetUSSPending(dev_p) ;

    if (dev_p->usb_count == 1)
    {
        /* Create the request to send to the West Bridge device */
        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_STOP_USB, CY_RQT_USB_RQT_CONTEXT, 0) ;
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

            return MyHandleResponseUsbStop(dev_p, req_p, reply_p, ret) ;
        }
        else
        {
            ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_STOP,
                0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
                CyAsUsbFuncCallback) ;

            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            return ret ;
        }

destroy:
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    }
    else if (dev_p->usb_count > 1)
    {
        /*
         * Reset all LEPs to inactive state, after cleaning up any
         * pending async read/write calls.
         */
        CyAsUsbReset(dev_p) ;
        dev_p->usb_count-- ;

        if (cb)
            cb(handle, ret, client, CY_FUNCT_CB_USB_STOP, 0) ;
    }

    CyAsDeviceClearUSSPending(dev_p) ;

    return ret ;
}

/*
* This function registers a callback to be called when USB events are processed
*/
CyAsReturnStatus_t
CyAsUsbRegisterCallback(CyAsDeviceHandle handle, CyAsUsbEventCallback callback)
{
    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbRegisterCallback called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    dev_p->usb_event_cb = NULL ;
    dev_p->usb_event_cb_ms = callback ;
    return CY_AS_ERROR_SUCCESS ;
}


static CyAsReturnStatus_t
MyHandleResponseNoData(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
    else
        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;

    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseConnect(CyAsDevice* dev_p,
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
    if (ret == CY_AS_ERROR_SUCCESS)
        CyAsDeviceSetUsbConnected(dev_p) ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}


/*
* This method asks the West Bridge device to connect the internal USB D+ and D- signals to the USB
* pins, thus starting the enumeration processes if the external pins are connnected to a USB host.
* If the external pins are not connect to a USB host, enumeration will begin as soon as the USB
* pins are connected to a host.
*/
CyAsReturnStatus_t
CyAsUsbConnect(CyAsDeviceHandle handle,
                 CyAsFunctionCallback cb,
                 uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbConnect called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SET_CONNECT_STATE, CY_RQT_USB_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* 1 = Connect request */
    CyAsLLRequestResponse_SetWord(req_p, 0, 1) ;

    /* Reserve space for the reply, the reply data will not exceed one word */
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

        return MyHandleResponseConnect(dev_p, req_p, reply_p, ret) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_CONNECT,
            0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseDisconnect(CyAsDevice* dev_p,
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
    if (ret == CY_AS_ERROR_SUCCESS)
        CyAsDeviceClearUsbConnected(dev_p) ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}
/*
* This method forces a disconnect of the D+ and D- pins external to the West Bridge device from the
* D+ and D- signals internally, effectively disconnecting the West Bridge device from any connectec
* USB host.
*/
CyAsReturnStatus_t
CyAsUsbDisconnect(CyAsDeviceHandle handle,
                    CyAsFunctionCallback cb,
                    uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbDisconnect called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    if (!CyAsDeviceIsUsbConnected(dev_p))
        return CY_AS_ERROR_USB_NOT_CONNECTED ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SET_CONNECT_STATE, CY_RQT_USB_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, 0) ;

    /* Reserve space for the reply, the reply data will not exceed two bytes */
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

        return MyHandleResponseDisconnect(dev_p, req_p, reply_p, ret) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_DISCONNECT,
            0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }
destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseSetEnumConfig(CyAsDevice* dev_p,
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

    if (ret == CY_AS_ERROR_SUCCESS)
    {
        /*
        * We configured the West Bridge device and enumeration is going to happen
        * on the P port processor.  Now we must enable endpoint zero
        */
        CyAsUsbEndPointConfig config ;

        config.dir = CyAsUsbInOut ;
        config.type = CyAsUsbControl ;
        config.enabled = CyTrue ;

        ret = CyAsUsbSetEndPointConfig((CyAsDeviceHandle*)dev_p, 0, &config) ;
    }

destroy :
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;

        return ret ;
}

/*
* This method sets how the USB is enumerated and should be called before the
* CyAsUsbConnect() is called.
*/
static CyAsReturnStatus_t
MyUsbSetEnumConfig(CyAsDevice *dev_p,
                    uint8_t bus_mask,
                    uint8_t media_mask,
                    CyBool use_antioch_enumeration,
                    uint8_t mass_storage_interface,
                    uint8_t mtp_interface,
                    CyBool mass_storage_callbacks,
                    CyAsFunctionCallback cb,
                    uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsLogDebugMessage(6, "CyAsUsbSetEnumConfig called") ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsUsbConnected(dev_p))
        return CY_AS_ERROR_USB_CONNECTED ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    /* if we are using MTP firmware:  */
    if (dev_p->is_mtp_firmware == 1)
    {
        if (mass_storage_interface != 0) /* we can not enemurate MSC */
            return CY_AS_ERROR_INVALID_CONFIGURATION ;

        if (bus_mask == 0)
        {
            if (mtp_interface != 0)
                return CY_AS_ERROR_INVALID_CONFIGURATION ;
        }
        else if (bus_mask == 2)
        {
            /* enable EP 1 as it will be used */
            CyAsDmaEnableEndPoint(dev_p, 1, CyTrue, CyAsDirectionIn) ;
            dev_p->usb_config[1].enabled = CyTrue ;
            dev_p->usb_config[1].dir = CyAsUsbIn ;
            dev_p->usb_config[1].type = CyAsUsbInt ;
        }
        else
        {
            return CY_AS_ERROR_INVALID_CONFIGURATION ;
        }
    }
    else if (mtp_interface != 0) /* if we are not using MTP firmware, we can not enumerate MTP */
        return CY_AS_ERROR_INVALID_CONFIGURATION ;

    /*
    * If we are not enumerating mass storage, we should not be providing an interface
    * number.
    */
    if (bus_mask == 0 && mass_storage_interface != 0)
        return CY_AS_ERROR_INVALID_CONFIGURATION ;

    /*
    * If we are going to use mtp_interface, bus mask must be 2.
    */
    if (mtp_interface != 0 && bus_mask != 2)
        return CY_AS_ERROR_INVALID_CONFIGURATION ;


    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SET_USB_CONFIG, CY_RQT_USB_RQT_CONTEXT, 4) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* Marshal the structure */
    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)((media_mask << 8) | bus_mask)) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)use_antioch_enumeration) ;
    CyAsLLRequestResponse_SetWord(req_p, 2, dev_p->is_mtp_firmware ? mtp_interface : mass_storage_interface) ;
    CyAsLLRequestResponse_SetWord(req_p, 3, (uint16_t)mass_storage_callbacks) ;

    /* Reserve space for the reply, the reply data will not exceed one word */
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

        return MyHandleResponseSetEnumConfig(dev_p, req_p, reply_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_SETENUMCONFIG,
            0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

/*
 * This method sets how the USB is enumerated and should be called before the
 * CyAsUsbConnect() is called.
 */
CyAsReturnStatus_t
CyAsUsbSetEnumConfig(CyAsDeviceHandle handle,
                       CyAsUsbEnumControl *config_p,
                       CyAsFunctionCallback cb,
                       uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    uint8_t bus_mask, media_mask ;
    uint32_t bus, device ;
    CyAsReturnStatus_t ret ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if ((CyAsDeviceIsInCallback(dev_p))  && (cb != 0))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    /* Since we are mapping the media types to bus with NAND to 0 and the rest
     * to 1, and we are only allowing for enumerating all the devices on a bus
     * we just scan the array for any positions where there a device is enabled
     * and mark the bus to be enumerated.
     */
    bus_mask   = 0 ;
    media_mask = 0 ;
    media_mask = 0 ;
    for( bus = 0; bus < CY_AS_MAX_BUSES; bus++)
    {
        for( device = 0; device < CY_AS_MAX_STORAGE_DEVICES; device++)
        {
            if (config_p->devices_to_enumerate[bus][device] == CyTrue)
            {
                bus_mask   |= (0x01 << bus) ;
                media_mask |= dev_p->media_supported[bus] ;
                media_mask |= dev_p->media_supported[bus] ;
            }
        }
    }

    return MyUsbSetEnumConfig(dev_p, bus_mask,media_mask,
            config_p->antioch_enumeration,
            config_p->mass_storage_interface,
            config_p->mtp_interface,
            config_p->mass_storage_callbacks,
            cb,
            client
        ) ;
}


static CyAsReturnStatus_t
MyHandleResponseGetEnumConfig(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         void* config_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint16_t val ;
    uint8_t bus_mask ;
    uint32_t bus ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_USB_CONFIG)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    /* Marshal the reply */
    if (req_p->flags & CY_AS_REQUEST_RESPONSE_MS)
    {
        uint32_t device ;
        CyBool state;
        CyAsUsbEnumControl *ms_config_p = (CyAsUsbEnumControl*)config_p;

        bus_mask = (uint8_t)(CyAsLLRequestResponse_GetWord(reply_p, 0) & 0xFF) ;
        for( bus = 0; bus < CY_AS_MAX_BUSES; bus++)
        {
            if (bus_mask & (1 << bus))
                state = CyTrue ;
            else
                state = CyFalse ;

            for( device = 0; device < CY_AS_MAX_STORAGE_DEVICES; device++)
            {
                ms_config_p->devices_to_enumerate[bus][device] = state ;
            }
        }

        ms_config_p->antioch_enumeration = (CyBool)CyAsLLRequestResponse_GetWord(reply_p, 1) ;

        val = CyAsLLRequestResponse_GetWord(reply_p, 2) ;
        if (dev_p->is_mtp_firmware)
        {
            ms_config_p->mass_storage_interface = 0 ;
            ms_config_p->mtp_interface          = (uint8_t)(val & 0xFF) ;
        }
        else
        {
            ms_config_p->mass_storage_interface = (uint8_t)(val & 0xFF) ;
            ms_config_p->mtp_interface          = 0 ;
        }
        ms_config_p->mass_storage_callbacks = (CyBool)(val >> 8) ;

        /*
        * Firmware returns an invalid interface number for mass storage,
        * if mass storage is not enabled. This needs to be converted to zero
        * to match the input configuration.
        */
        if (bus_mask == 0)
        {
            if (dev_p->is_mtp_firmware)
                ms_config_p->mtp_interface = 0 ;
            else
                ms_config_p->mass_storage_interface = 0 ;
        }
    }
    else
    {
        CyAsUsbEnumControl_dep *ex_config_p = (CyAsUsbEnumControl_dep*)config_p;

        ex_config_p->enum_mass_storage = (uint8_t)((CyAsLLRequestResponse_GetWord(reply_p, 0) >> 8) & 0xFF) ;
        ex_config_p->antioch_enumeration = (CyBool)CyAsLLRequestResponse_GetWord(reply_p, 1) ;

        val = CyAsLLRequestResponse_GetWord(reply_p, 2) ;
        ex_config_p->mass_storage_interface = (uint8_t)(val & 0xFF) ;
        ex_config_p->mass_storage_callbacks = (CyBool)(val >> 8) ;

        /*
        * Firmware returns an invalid interface number for mass storage,
        * if mass storage is not enabled. This needs to be converted to zero
        * to match the input configuration.
        */
        if (ex_config_p->enum_mass_storage == 0)
            ex_config_p->mass_storage_interface = 0 ;
    }

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

/*
* This sets up the request for the enumerateion configuration information, based on if the request is from
* the old pre-1.2 functions.
*/
static CyAsReturnStatus_t
MyUsbGetEnumConfig(CyAsDeviceHandle handle,
                    uint16_t req_flags,
                    void* config_p,
                    CyAsFunctionCallback cb,
                    uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbGetEnumConfig called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_GET_USB_CONFIG, CY_RQT_USB_RQT_CONTEXT, 0) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* Reserve space for the reply, the reply data will not exceed two bytes */
    reply_p = CyAsLLCreateResponse(dev_p, 3) ;
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

        /* we need to know the type of request to know how to manage the data */
        req_p->flags |= req_flags;
        return MyHandleResponseGetEnumConfig(dev_p, req_p, reply_p, config_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_GETENUMCONFIG,
            config_p, dev_p->func_cbs_usb, req_flags, req_p, reply_p,
            CyAsUsbFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

/*
 * This method returns the enumerateion configuration information from the West Bridge device.
 * Generally this is not used by client software but is provided mostly for debug information.
 * We want a method to read all state information from the device.
 */
CyAsReturnStatus_t
CyAsUsbGetEnumConfig(CyAsDeviceHandle handle,
                       CyAsUsbEnumControl *config_p,
                       CyAsFunctionCallback cb,
                       uint32_t client)
{
    return MyUsbGetEnumConfig(handle, CY_AS_REQUEST_RESPONSE_MS, config_p, cb, client);
}


/*
* This method sets the USB descriptor for a given entity.
*/
CyAsReturnStatus_t
CyAsUsbSetDescriptor(CyAsDeviceHandle handle,
                       CyAsUsbDescType type,
                       uint8_t index,
                       void *desc_p,
                       uint16_t length,
                       CyAsFunctionCallback cb,
                       uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint16_t pktlen ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbSetDescriptor called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    if (length > CY_AS_MAX_USB_DESCRIPTOR_SIZE)
        return CY_AS_ERROR_INVALID_DESCRIPTOR ;

    pktlen = (uint16_t)length / 2 ;
    if (length % 2)
        pktlen++ ;
    pktlen += 2 ;                       /* 1 for type, 1 for length */

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SET_DESCRIPTOR, CY_RQT_USB_RQT_CONTEXT, (uint16_t)pktlen) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)((uint8_t)type | (index << 8))) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)length) ;
    CyAsLLRequestResponse_Pack(req_p, 2, length, desc_p) ;

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

        return MyHandleResponseNoData(dev_p, req_p, reply_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_SETDESCRIPTOR,
            0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

/*
 * This method clears all descriptors that were previously stored on the West Bridge
 * through CyAsUsbSetDescriptor calls.
 */
CyAsReturnStatus_t
CyAsUsbClearDescriptors(CyAsDeviceHandle handle,
                       CyAsFunctionCallback cb,
                       uint32_t client)
{
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbClearDescriptors called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if ((CyAsDeviceIsInCallback(dev_p)) && (cb == 0))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_CLEAR_DESCRIPTORS, CY_RQT_USB_RQT_CONTEXT, 1) ;
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

        return MyHandleResponseNoData(dev_p, req_p, reply_p) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_CLEARDESCRIPTORS,
            0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseGetDescriptor(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         CyAsGetDescriptorData* data)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
     uint32_t retlen ;

    if (CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_SUCCESS_FAILURE)
    {
        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        goto destroy ;
    }
    else if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_USB_DESCRIPTOR)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    retlen = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    if (retlen > data->length)
    {
        ret = CY_AS_ERROR_INVALID_SIZE ;
        goto destroy ;
    }

    ret = CY_AS_ERROR_SUCCESS ;
    CyAsLLRequestResponse_Unpack(reply_p, 1, retlen, data->desc_p) ;


destroy :
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;

        return ret ;
}

/*
* This method retreives the USB descriptor for a given type.
*/
CyAsReturnStatus_t
CyAsUsbGetDescriptor(CyAsDeviceHandle handle,
                       CyAsUsbDescType type,
                       uint8_t index,
                       CyAsGetDescriptorData* data,
                       CyAsFunctionCallback cb,
                       uint32_t client)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p , *reply_p ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbGetDescriptor called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_GET_DESCRIPTOR, CY_RQT_USB_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)((uint8_t)type | (index << 8))) ;

    /* Add one for the length field */
    reply_p = CyAsLLCreateResponse(dev_p, CY_AS_MAX_USB_DESCRIPTOR_SIZE + 1) ;
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

        return MyHandleResponseGetDescriptor(dev_p, req_p, reply_p, data) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_GETDESCRIPTOR,
            data, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

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
CyAsUsbSetPhysicalConfiguration(CyAsDeviceHandle handle, uint8_t config)
{
    CyAsReturnStatus_t ret ;
    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbSetPhysicalConfiguration called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsUsbConnected(dev_p))
        return CY_AS_ERROR_USB_CONNECTED ;

    if (config < 1 || config > 12)
        return CY_AS_ERROR_INVALID_CONFIGURATION ;

    dev_p->usb_phy_config = config ;

    return CY_AS_ERROR_SUCCESS ;
}

static CyBool
IsPhysicalValid(uint8_t config, CyAsEndPointNumber_t ep)
{
    static uint8_t validmask[12] =
    {
        0x0f,       /* Config  1 - 1, 2, 3, 4 */
        0x07,       /* Config  2 - 1, 2, 3 */
        0x07,       /* Config  3 - 1, 2, 3 */
        0x0d,       /* Config  4 - 1, 3, 4 */
        0x05,       /* Config  5 - 1, 3 */
        0x05,       /* Config  6 - 1, 3 */
        0x0d,       /* Config  7 - 1, 3, 4 */
        0x05,       /* Config  8 - 1, 3 */
        0x05,       /* Config  9 - 1, 3 */
        0x0d,       /* Config 10 - 1, 3, 4 */
        0x09,       /* Config 11 - 1, 4 */
        0x01        /* Config 12 - 1 */
    } ;

    return (validmask[config - 1] & (1 << (ep - 1))) ? CyTrue : CyFalse ;
}

/*
* This method sets the configuration for an endpoint
*/
CyAsReturnStatus_t
CyAsUsbSetEndPointConfig(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, CyAsUsbEndPointConfig *config_p)
{
    CyAsReturnStatus_t ret ;
    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbSetEndPointConfig called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsUsbConnected(dev_p))
        return CY_AS_ERROR_USB_CONNECTED ;

    if (ep >= 16 || ep == 2 || ep == 4 || ep == 6 || ep == 8)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    if (ep == 0)
    {
        /* Endpoint 0 must be 64 byte, dir IN/OUT, and control type */
        if (config_p->dir != CyAsUsbInOut || config_p->type != CyAsUsbControl)
            return CY_AS_ERROR_INVALID_CONFIGURATION ;
    }
    else if (ep == 1)
    {
        if ((dev_p->is_mtp_firmware == 1) && (dev_p->usb_config[1].enabled == CyTrue))
        {
            return CY_AS_ERROR_INVALID_ENDPOINT ;
        }

        /*
         * EP1 can only be used either as an OUT ep, or as an IN ep.
         */
        if ((config_p->type == CyAsUsbControl) || (config_p->type == CyAsUsbIso) || (config_p->dir == CyAsUsbInOut))
            return CY_AS_ERROR_INVALID_CONFIGURATION ;
    }
    else
    {
        if (config_p->dir == CyAsUsbInOut || config_p->type == CyAsUsbControl)
            return CY_AS_ERROR_INVALID_CONFIGURATION ;

        if (!IsPhysicalValid(dev_p->usb_phy_config, config_p->physical))
            return CY_AS_ERROR_INVALID_PHYSICAL_ENDPOINT ;

        /*
        * ISO endpoints must be on EPs 3, 5, 7 or 9 as they need to align directly
        * with the underlying physical endpoint.
        */
        if (config_p->type == CyAsUsbIso)
        {
            if (ep != 3 && ep != 5 && ep != 7 && ep != 9)
                return CY_AS_ERROR_INVALID_CONFIGURATION ;

            if (ep == 3 && config_p->physical != 1)
                return CY_AS_ERROR_INVALID_CONFIGURATION ;

            if (ep == 5 && config_p->physical != 2)
                return CY_AS_ERROR_INVALID_CONFIGURATION ;

            if (ep == 7 && config_p->physical != 3)
                return CY_AS_ERROR_INVALID_CONFIGURATION ;

            if (ep == 9 && config_p->physical != 4)
                return CY_AS_ERROR_INVALID_CONFIGURATION ;
        }
    }

    /* Store the configuration information until a CyAsUsbCommitConfig is done */
    dev_p->usb_config[ep] = *config_p ;

    /* If the endpoint is enabled, enable DMA associated with the endpoint */
    /*
    * We make some assumptions that we check here.  We assume that the direction fields for the
    * DMA module are the same values as the direction values for the USB module.  At some point these should
    * be consolidated into a single enumerated type. - BWG
    */
    CyAsHalAssert((int)CyAsUsbIn == (int)CyAsDirectionIn);
    CyAsHalAssert((int)CyAsUsbOut == (int)CyAsDirectionOut);
    CyAsHalAssert((int)CyAsUsbInOut == (int)CyAsDirectionInOut) ;

    return CyAsDmaEnableEndPoint(dev_p, ep, config_p->enabled, (CyAsDmaDirection)config_p->dir) ;
}

CyAsReturnStatus_t
CyAsUsbGetEndPointConfig(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, CyAsUsbEndPointConfig *config_p)
{
    CyAsReturnStatus_t ret ;

    CyAsDevice*dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbGetEndPointConfig called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (ep >= 16 || ep == 2 || ep == 4 || ep == 6 || ep == 8)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    *config_p = dev_p->usb_config[ep] ;

    return CY_AS_ERROR_SUCCESS ;
}

/*
* Commit the configuration of the various endpoints to the hardware.
*/
CyAsReturnStatus_t
CyAsUsbCommitConfig(CyAsDeviceHandle handle,
                      CyAsFunctionCallback cb,
                      uint32_t client)
{
    uint32_t i ;
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsDevice*dev_p ;
    uint16_t data ;

    CyAsLogDebugMessage(6, "CyAsUsbCommitConfig called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsUsbConnected(dev_p))
        return CY_AS_ERROR_USB_CONNECTED ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    /*
    * This performs the mapping based on informatation that was previously
    * stored on the device about the various endpoints and how they are configured.  The
    * output of this mapping is setting the the 14 register values contained in usb_lepcfg
    * and usb_pepcfg
    */
    ret = CyAsUsbMapLogical2Physical(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /*
    * Now, package the information about the various logical and physical
    * endpoint configuration registers and send it across to the West Bridge
    * device.
    */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SET_USB_CONFIG_REGISTERS, CY_RQT_USB_RQT_CONTEXT, 8) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsHalPrintMessage("USB Configuration: %d\n", dev_p->usb_phy_config) ;
    CyAsHalPrintMessage("EP1OUT: 0x%02x EP1IN: 0x%02x\n", dev_p->usb_ep1cfg[0], dev_p->usb_ep1cfg[1]) ;
    CyAsHalPrintMessage("PEP Registers: 0x%02x 0x%02x 0x%02x 0x%02x\n", dev_p->usb_pepcfg[0],
        dev_p->usb_pepcfg[1], dev_p->usb_pepcfg[2], dev_p->usb_pepcfg[3]) ;

    CyAsHalPrintMessage("LEP Registers: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
        dev_p->usb_lepcfg[0],dev_p->usb_lepcfg[1], dev_p->usb_lepcfg[2], dev_p->usb_lepcfg[3],
        dev_p->usb_lepcfg[4],dev_p->usb_lepcfg[5], dev_p->usb_lepcfg[6], dev_p->usb_lepcfg[7],
        dev_p->usb_lepcfg[8],dev_p->usb_lepcfg[9]) ;

    /* Write the EP1OUTCFG and EP1INCFG data in the first word. */
    data = (uint16_t)((dev_p->usb_ep1cfg[0] << 8) | dev_p->usb_ep1cfg[1]) ;
    CyAsLLRequestResponse_SetWord(req_p, 0, data) ;

    /* Write the PEP CFG data in the next 2 words */
    for(i = 0 ; i < 4 ; i += 2)
    {
        data = (uint16_t)((dev_p->usb_pepcfg[i] << 8) | dev_p->usb_pepcfg[i + 1]) ;
        CyAsLLRequestResponse_SetWord(req_p, 1 + i / 2, data) ;
    }

    /* Write the LEP CFG data in the next 5 words */
    for(i = 0 ; i < 10 ; i += 2)
    {
        data = (uint16_t)((dev_p->usb_lepcfg[i] << 8) | dev_p->usb_lepcfg[i + 1]) ;
        CyAsLLRequestResponse_SetWord(req_p, 3 + i / 2, data) ;
    }

    /* A single status word response type */
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
            ret = CyAsUsbSetupDma(dev_p) ;

        return ret ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_COMMITCONFIG,
            0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static void
SyncRequestCallback(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, void *buf_p, uint32_t size, CyAsReturnStatus_t err)
{
    (void)ep ;
    (void)buf_p ;

    dev_p->usb_error = err ;
    dev_p->usb_actual_cnt = size ;
}

static void
AsyncReadRequestCallback(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, void *buf_p, uint32_t size, CyAsReturnStatus_t err)
{
    CyAsDeviceHandle h ;

    CyAsLogDebugMessage(6, "AsyncReadRequestCallback called") ;

    h = (CyAsDeviceHandle)dev_p ;

    if (ep == 0 && CyAsDeviceIsAckDelayed(dev_p))
    {
        dev_p->usb_pending_buffer = buf_p ;
        dev_p->usb_pending_size = size ;
        dev_p->usb_error = err ;
        CyAsUsbAckSetupPacket(h, UsbAckCallback, 0) ;
    }
    else
    {
        CyAsUsbIoCallback cb ;

        cb = dev_p->usb_cb[ep] ;
        dev_p->usb_cb[ep] = 0 ;
        CyAsDeviceClearUsbAsyncPending(dev_p, ep) ;
        if (cb)
            cb(h, ep, size, buf_p, err) ;
    }
}

static void
AsyncWriteRequestCallback(CyAsDevice *dev_p, CyAsEndPointNumber_t ep, void *buf_p, uint32_t size, CyAsReturnStatus_t err)
{
    CyAsDeviceHandle h ;

    CyAsLogDebugMessage(6, "AsyncWriteRequestCallback called") ;

    h = (CyAsDeviceHandle)dev_p ;

    if (ep == 0 && CyAsDeviceIsAckDelayed(dev_p))
    {
        dev_p->usb_pending_buffer = buf_p ;
        dev_p->usb_pending_size = size ;
        dev_p->usb_error = err ;

        /* The west bridge protocol generates ZLPs as required. */
        CyAsUsbAckSetupPacket(h, UsbAckCallback, 0) ;
    }
    else
    {
        CyAsUsbIoCallback cb ;

        cb = dev_p->usb_cb[ep] ;
        dev_p->usb_cb[ep] = 0 ;

        CyAsDeviceClearUsbAsyncPending(dev_p, ep) ;
        if (cb)
            cb(h, ep, size, buf_p, err) ;
    }
}

static void
MyTurboRqtCallback(CyAsDevice *dev_p,
                    uint8_t context,
                    CyAsLLRequestResponse *rqt,
                    CyAsLLRequestResponse *resp,
                    CyAsReturnStatus_t stat)
{
    uint8_t code ;

    (void)context ;
    (void)stat ;

    /* The Handlers are responsible for Deleting the rqt and resp when
     * they are finished
     */
    code = CyAsLLRequestResponse_GetCode(rqt) ;
    switch(code)
    {
    case CY_RQT_TURBO_SWITCH_ENDPOINT:
        CyAsHalAssert(stat == CY_AS_ERROR_SUCCESS) ;
        CyAsLLDestroyRequest(dev_p, rqt) ;
        CyAsLLDestroyResponse(dev_p, resp) ;
        break;
    default:
        CyAsHalAssert(CyFalse) ;
        break ;
    }
}

/* Send a mailbox request to prepare the endpoint for switching */
static CyAsReturnStatus_t
MySendTurboSwitch(CyAsDevice* dev_p, uint32_t size, CyBool pktread)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p , *reply_p ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_TURBO_SWITCH_ENDPOINT, CY_RQT_TUR_RQT_CONTEXT, 3) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* Reserve space for the reply, the reply data will not exceed one word */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)pktread) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((size >> 16) & 0xFFFF)) ;
    CyAsLLRequestResponse_SetWord(req_p, 2, (uint16_t)(size & 0xFFFF)) ;

    ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyFalse, MyTurboRqtCallback) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyRequest(dev_p, reply_p) ;
        return ret ;
    }

    return CY_AS_ERROR_SUCCESS ;
}

CyAsReturnStatus_t
CyAsUsbReadData(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, CyBool pktread, uint32_t dsize, uint32_t *dataread, void *data)
{
    CyAsReturnStatus_t ret ;
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbReadData called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    if (ep >= 16 || ep == 4 || ep == 6 || ep == 8)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* EP2 is available for reading when MTP is active */
    if(dev_p->mtp_count == 0 && ep == CY_AS_MTP_READ_ENDPOINT)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* If the endpoint is disabled, we cannot write data to the endpoint */
    if (!dev_p->usb_config[ep].enabled)
        return CY_AS_ERROR_ENDPOINT_DISABLED ;

    if (dev_p->usb_config[ep].dir != CyAsUsbOut)
        return CY_AS_ERROR_USB_BAD_DIRECTION ;

    ret = CyAsDmaQueueRequest(dev_p, ep, data, dsize, pktread, CyTrue, SyncRequestCallback) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (ep == CY_AS_MTP_READ_ENDPOINT )
    {
        ret = MySendTurboSwitch(dev_p, dsize, pktread) ;
        if (ret != CY_AS_ERROR_SUCCESS)
        {
            CyAsDmaCancel(dev_p, ep, ret) ;
            return ret ;
        }

        ret = CyAsDmaDrainQueue(dev_p, ep, CyFalse) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            return ret ;
    }
    else
    {
        ret = CyAsDmaDrainQueue(dev_p, ep, CyTrue) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            return ret ;
    }

    ret = dev_p->usb_error ;
    *dataread = dev_p->usb_actual_cnt ;

    return ret ;
}

CyAsReturnStatus_t
CyAsUsbReadDataAsync(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, CyBool pktread, uint32_t dsize, void *data, CyAsUsbIoCallback cb)
{
    CyAsReturnStatus_t ret ;
    uint32_t mask ;
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbReadDataAsync called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (ep >= 16 || ep == 4 || ep == 6 || ep == 8)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* EP2 is available for reading when MTP is active */
    if(dev_p->mtp_count == 0 && ep == CY_AS_MTP_READ_ENDPOINT)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* If the endpoint is disabled, we cannot write data to the endpoint */
    if (!dev_p->usb_config[ep].enabled)
        return CY_AS_ERROR_ENDPOINT_DISABLED ;

    if (dev_p->usb_config[ep].dir != CyAsUsbOut && dev_p->usb_config[ep].dir != CyAsUsbInOut)
        return CY_AS_ERROR_USB_BAD_DIRECTION ;

    /*
    * Since async operations can be triggered by interrupt code, we must
    * insure that we do not get multiple async operations going at one time and
    * protect this test and set operation from interrupts.
    */
    mask = CyAsHalDisableInterrupts() ;
    if (CyAsDeviceIsUsbAsyncPending(dev_p, ep))
    {
        CyAsHalEnableInterrupts(mask) ;
        return CY_AS_ERROR_ASYNC_PENDING ;
    }
    CyAsDeviceSetUsbAsyncPending(dev_p, ep) ;

    /*
    * If this is for EP0, we set this bit to delay the ACK response
    * until after this read has completed.
    */
    if (ep == 0)
        CyAsDeviceSetAckDelayed(dev_p) ;

    CyAsHalEnableInterrupts(mask) ;

    CyAsHalAssert(dev_p->usb_cb[ep] == 0) ;
    dev_p->usb_cb[ep] = cb ;

    ret = CyAsDmaQueueRequest(dev_p, ep, data, dsize, pktread, CyTrue, AsyncReadRequestCallback) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (ep == CY_AS_MTP_READ_ENDPOINT)
    {
        ret = MySendTurboSwitch(dev_p, dsize, pktread) ;
        if (ret != CY_AS_ERROR_SUCCESS)
        {
            CyAsDmaCancel(dev_p, ep, ret) ;
            return ret ;
        }
    }
    else
    {
        /* Kick start the queue if it is not running */
        CyAsDmaKickStart(dev_p, ep) ;
    }
    return ret ;
}

CyAsReturnStatus_t
CyAsUsbWriteData(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, uint32_t dsize, void *data)
{
    CyAsReturnStatus_t ret ;
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbWriteData called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    if (ep >= 16 || ep == 2 || ep == 4 || ep == 8)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* EP6 is available for writing when MTP is active */
    if(dev_p->mtp_count == 0 && ep == CY_AS_MTP_WRITE_ENDPOINT)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* If the endpoint is disabled, we cannot write data to the endpoint */
    if (!dev_p->usb_config[ep].enabled)
        return CY_AS_ERROR_ENDPOINT_DISABLED ;

    if (dev_p->usb_config[ep].dir != CyAsUsbIn && dev_p->usb_config[ep].dir != CyAsUsbInOut)
        return CY_AS_ERROR_USB_BAD_DIRECTION ;

    /* Write on Turbo endpoint */
    if (ep == CY_AS_MTP_WRITE_ENDPOINT)
    {
        CyAsLLRequestResponse *req_p, *reply_p ;

        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_TURBO_SEND_RESP_DATA_TO_HOST, CY_RQT_TUR_RQT_CONTEXT, 3) ;
        if (req_p == 0)
            return CY_AS_ERROR_OUT_OF_MEMORY ;

        CyAsLLRequestResponse_SetWord(req_p, 0, 0x0006) ; /* EP number to use. */
        CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((dsize >> 16) & 0xFFFF)) ;
        CyAsLLRequestResponse_SetWord(req_p, 2, (uint16_t)(dsize & 0xFFFF)) ;

        /* Reserve space for the reply, the reply data will not exceed one word */
        reply_p = CyAsLLCreateResponse(dev_p, 1) ;
        if (reply_p == 0)
        {
            CyAsLLDestroyRequest(dev_p, req_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        if (dsize)
        {
            ret = CyAsDmaQueueRequest(dev_p, ep, data, dsize, CyFalse, CyFalse, SyncRequestCallback) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                return ret ;
        }

        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret == CY_AS_ERROR_SUCCESS)
        {
            if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
                ret = CY_AS_ERROR_INVALID_RESPONSE ;
            else
                ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        }

        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;

        if (ret != CY_AS_ERROR_SUCCESS)
        {
            if (dsize)
                CyAsDmaCancel(dev_p, ep, ret) ;
            return ret ;
        }

        /* If this is a zero-byte write, firmware will handle it.
         * There is no need to do any work here.
         */
        if (!dsize)
            return CY_AS_ERROR_SUCCESS ;
    }
    else
    {
        ret = CyAsDmaQueueRequest(dev_p, ep, data, dsize, CyFalse, CyFalse, SyncRequestCallback) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            return ret ;
    }

    if (ep != CY_AS_MTP_WRITE_ENDPOINT)
        ret = CyAsDmaDrainQueue(dev_p, ep, CyTrue) ;
    else
        ret = CyAsDmaDrainQueue(dev_p, ep, CyFalse) ;

    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    ret = dev_p->usb_error ;
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
    CyAsUsbIoCallback cb ;
    CyAsDeviceHandle h = (CyAsDeviceHandle)dev_p ;

    CyAsHalAssert(context == CY_RQT_TUR_RQT_CONTEXT) ;

    if (ret == CY_AS_ERROR_SUCCESS)
    {
        if (CyAsLLRequestResponse_GetCode(resp) != CY_RESP_SUCCESS_FAILURE)
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        else
            ret = CyAsLLRequestResponse_GetWord(resp, 0) ;
    }

    /* If this was a zero byte transfer request, we can call the callback from
     * here. */
    if ((CyAsLLRequestResponse_GetWord(rqt, 1) == 0) &&
            (CyAsLLRequestResponse_GetWord(rqt, 2) == 0))
    {
        cb = dev_p->usb_cb[CY_AS_MTP_WRITE_ENDPOINT] ;
        dev_p->usb_cb[CY_AS_MTP_WRITE_ENDPOINT] = 0 ;
        CyAsDeviceClearUsbAsyncPending(dev_p, CY_AS_MTP_WRITE_ENDPOINT) ;
        if (cb)
            cb(h, CY_AS_MTP_WRITE_ENDPOINT, 0, 0, ret) ;

        goto destroy ;
    }

    if (ret != CY_AS_ERROR_SUCCESS)
    {
        /* Firmware failed the request. Cancel the DMA transfer. */
        CyAsDmaCancel(dev_p, 0x06, CY_AS_ERROR_CANCELED) ;
        dev_p->usb_cb[0x06] = 0 ;
        CyAsDeviceClearUsbAsyncPending(dev_p, 0x06) ;
    }

destroy:
    CyAsLLDestroyResponse(dev_p, resp) ;
    CyAsLLDestroyRequest(dev_p, rqt) ;
}

CyAsReturnStatus_t
CyAsUsbWriteDataAsync(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, uint32_t dsize, void *data, CyBool spacket, CyAsUsbIoCallback cb)
{
    uint32_t mask ;
    CyAsReturnStatus_t ret ;
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsUsbWriteDataAsync called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (ep >= 16 || ep == 2 || ep == 4 || ep == 8)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

     /* EP6 is available for writing when MTP is active */
    if(dev_p->mtp_count == 0 && ep == CY_AS_MTP_WRITE_ENDPOINT)
        return CY_AS_ERROR_INVALID_ENDPOINT ;

    /* If the endpoint is disabled, we cannot write data to the endpoint */
    if (!dev_p->usb_config[ep].enabled)
        return CY_AS_ERROR_ENDPOINT_DISABLED ;

    if (dev_p->usb_config[ep].dir != CyAsUsbIn && dev_p->usb_config[ep].dir != CyAsUsbInOut)
        return CY_AS_ERROR_USB_BAD_DIRECTION ;

    /*
    * Since async operations can be triggered by interrupt code, we must
    * insure that we do not get multiple async operations going at one time and
    * protect this test and set operation from interrupts.
    */
    mask = CyAsHalDisableInterrupts() ;
    if (CyAsDeviceIsUsbAsyncPending(dev_p, ep))
    {
        CyAsHalEnableInterrupts(mask) ;
        return CY_AS_ERROR_ASYNC_PENDING ;
    }

    CyAsDeviceSetUsbAsyncPending(dev_p, ep) ;

    if (ep == 0)
        CyAsDeviceSetAckDelayed(dev_p) ;

    CyAsHalEnableInterrupts(mask) ;

    CyAsHalAssert(dev_p->usb_cb[ep] == 0) ;
    dev_p->usb_cb[ep] = cb ;
    dev_p->usb_spacket[ep] = spacket ;

    /* Write on Turbo endpoint */
    if (ep == CY_AS_MTP_WRITE_ENDPOINT)
    {
        CyAsLLRequestResponse *req_p, *reply_p ;

        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_TURBO_SEND_RESP_DATA_TO_HOST, CY_RQT_TUR_RQT_CONTEXT, 3) ;
        if (req_p == 0)
            return CY_AS_ERROR_OUT_OF_MEMORY ;

        CyAsLLRequestResponse_SetWord(req_p, 0, 0x0006) ; /* EP number to use. */
        CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((dsize >> 16) & 0xFFFF)) ;
        CyAsLLRequestResponse_SetWord(req_p, 2, (uint16_t)(dsize & 0xFFFF)) ;

        /* Reserve space for the reply, the reply data will not exceed one word */
        reply_p = CyAsLLCreateResponse(dev_p, 1) ;
        if (reply_p == 0)
        {
            CyAsLLDestroyRequest(dev_p, req_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        if (dsize)
        {
            ret = CyAsDmaQueueRequest(dev_p, ep, data, dsize, CyFalse, CyFalse, AsyncWriteRequestCallback) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                return ret ;
        }

        ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyFalse, MtpWriteCallback) ;
        if (ret != CY_AS_ERROR_SUCCESS)
        {
            if (dsize)
                CyAsDmaCancel(dev_p, ep, ret) ;
            return ret ;
        }

        /* Firmware will handle a zero byte transfer without any DMA transfers. */
        if (!dsize)
            return CY_AS_ERROR_SUCCESS ;
    }
    else
    {
        ret = CyAsDmaQueueRequest(dev_p, ep, data, dsize, CyFalse, CyFalse, AsyncWriteRequestCallback) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            return ret ;
    }

    /* Kick start the queue if it is not running */
    if (ep != CY_AS_MTP_WRITE_ENDPOINT)
    {
        CyAsDmaKickStart(dev_p, ep) ;
    }

    return CY_AS_ERROR_SUCCESS ;
}

static void
MyUsbCancelAsyncCallback(
                   CyAsDevice *dev_p,
                   uint8_t context,
                   CyAsLLRequestResponse *rqt,
                   CyAsLLRequestResponse *resp,
                   CyAsReturnStatus_t ret)
{
    uint8_t ep ;
    (void)context ;

    ep = (uint8_t)CyAsLLRequestResponse_GetWord(rqt, 0) ;
    if (ret == CY_AS_ERROR_SUCCESS)
    {
        if (CyAsLLRequestResponse_GetCode(resp) != CY_RESP_SUCCESS_FAILURE)
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        else
            ret = CyAsLLRequestResponse_GetWord(resp, 0) ;
    }

    CyAsLLDestroyRequest(dev_p, rqt) ;
    CyAsLLDestroyResponse(dev_p, resp) ;

    if (ret == CY_AS_ERROR_SUCCESS)
    {
        CyAsDmaCancel(dev_p, ep, CY_AS_ERROR_CANCELED) ;
        dev_p->usb_cb[ep] = 0 ;
        CyAsDeviceClearUsbAsyncPending(dev_p, ep) ;
    }
}

CyAsReturnStatus_t
CyAsUsbCancelAsync(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p, *reply_p ;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ep &= 0x7F;         /* Remove the direction bit. */
    if (!CyAsDeviceIsUsbAsyncPending(dev_p, ep))
        return CY_AS_ERROR_ASYNC_NOT_PENDING;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    if ((ep == CY_AS_MTP_WRITE_ENDPOINT) || (ep == CY_AS_MTP_READ_ENDPOINT))
    {
        /* Need firmware support for the cancel operation. */
        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_CANCEL_ASYNC_TRANSFER, CY_RQT_TUR_RQT_CONTEXT, 1) ;
        if (req_p == 0)
            return CY_AS_ERROR_OUT_OF_MEMORY ;

        reply_p = CyAsLLCreateResponse(dev_p, 1) ;
        if (reply_p == 0)
        {
            CyAsLLDestroyRequest(dev_p, req_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)ep) ;
        ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyFalse, MyUsbCancelAsyncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
        {
            CyAsLLDestroyRequest(dev_p, req_p) ;
            CyAsLLDestroyResponse(dev_p, reply_p) ;
            return ret ;
        }
    }
    else
    {
        ret = CyAsDmaCancel(dev_p, ep, CY_AS_ERROR_CANCELED) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            return ret ;

        dev_p->usb_cb[ep] = 0 ;
        CyAsDeviceClearUsbAsyncPending(dev_p, ep) ;
    }

    return CY_AS_ERROR_SUCCESS ;
}

static void
CyAsUsbAckCallback(
                   CyAsDevice *dev_p,
                   uint8_t context,
                   CyAsLLRequestResponse *rqt,
                   CyAsLLRequestResponse *resp,
                   CyAsReturnStatus_t ret)
{
    CyAsFuncCBNode* node  = (CyAsFuncCBNode*)dev_p->func_cbs_usb->head_p ;

    (void)context ;

    if (ret == CY_AS_ERROR_SUCCESS)
    {
        if (CyAsLLRequestResponse_GetCode(resp) != CY_RESP_SUCCESS_FAILURE)
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        else
            ret = CyAsLLRequestResponse_GetWord(resp, 0) ;
    }

    node->cb_p((CyAsDeviceHandle)dev_p, ret, node->client_data, (CyAsFunctCBType)node->dataType, node->data) ;
    CyAsRemoveCBNode(dev_p->func_cbs_usb) ;

    CyAsLLDestroyRequest(dev_p, rqt) ;
    CyAsLLDestroyResponse(dev_p, resp) ;
    CyAsDeviceClearAckDelayed(dev_p) ;
}

static CyAsReturnStatus_t
CyAsUsbAckSetupPacket(CyAsDeviceHandle handle,
                      CyAsFunctionCallback      cb,
                      uint32_t client)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p ;
    CyAsLLRequestResponse *reply_p ;
    CyAsFuncCBNode* cbnode ;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p) && cb == 0)
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    CyAsHalAssert(cb != 0) ;

    cbnode = CyAsCreateFuncCBNode(cb, client);
    if( cbnode == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    req_p = CyAsLLCreateRequest(dev_p, 0, CY_RQT_USB_RQT_CONTEXT, 2) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if(reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    CyAsLLInitRequest(req_p, CY_RQT_ACK_SETUP_PACKET, CY_RQT_USB_RQT_CONTEXT, 1) ;
    CyAsLLInitResponse(reply_p, 1) ;

    req_p->flags |= CY_AS_REQUEST_RESPONSE_EX ;

    CyAsInsertCBNode(dev_p->func_cbs_usb, cbnode) ;

    ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyFalse, CyAsUsbAckCallback) ;

    return ret ;
}

/*
 * Flush all data in logical EP that is being NAK-ed or Stall-ed,
 * so that this does not continue to block data on other LEPs that
 * use the same physical EP.
 */
static void
CyAsUsbFlushLogicalEP(
        CyAsDevice *dev_p,
        uint16_t    ep)
{
    uint16_t addr, val, count ;

    addr = CY_AS_MEM_P0_EP2_DMA_REG + ep - 2 ;
    val  = CyAsHalReadRegister(dev_p->tag, addr) ;

    while (val)
    {
        count = ((val & 0xFFF) + 1) / 2 ;
        while (count--)
        {
            val = CyAsHalReadRegister(dev_p->tag, ep) ;
        }

        CyAsHalWriteRegister(dev_p->tag, addr, 0) ;
        val = CyAsHalReadRegister(dev_p->tag, addr) ;
    }
}

static CyAsReturnStatus_t
CyAsUsbNakStallRequest(CyAsDeviceHandle handle,
                       CyAsEndPointNumber_t ep,
                       uint16_t request,
                       CyBool state,
                       CyAsUsbFunctionCallback cb,
                       CyAsFunctionCallback fcb,
                       uint32_t client)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p , *reply_p ;
    uint16_t data ;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if(cb)
        CyAsHalAssert(fcb == 0) ;
    if(fcb)
        CyAsHalAssert(cb == 0) ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p) && cb == 0 && fcb == 0)
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    req_p = CyAsLLCreateRequest(dev_p, request, CY_RQT_USB_RQT_CONTEXT, 2) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* A single status word response type */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    /* Set the endpoint */
    data = (uint8_t)ep ;
    CyAsLLRequestResponse_SetWord(req_p, 0, data) ;

    /* Set stall state to stalled */
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint8_t)state) ;

    if (cb || fcb)
    {
        void * cbnode ;
        CyAsCBQueue* queue ;
        if(cb)
        {
            cbnode = CyAsCreateUsbFuncCBNode(cb, client) ;
            queue = dev_p->usb_func_cbs ;
        }
        else
        {
            cbnode = CyAsCreateFuncCBNode(fcb, client) ;
            queue = dev_p->func_cbs_usb ;
            req_p->flags |= CY_AS_REQUEST_RESPONSE_EX ;
        }

        if(cbnode == 0)
        {
            ret = CY_AS_ERROR_OUT_OF_MEMORY ;
            goto destroy ;
        }
        else
            CyAsInsertCBNode(queue, cbnode) ;


        if (CyAsDeviceIsSetupPacket(dev_p))
        {
            /* No Ack is needed on a stall request on EP0 */
            if ((state == CyTrue) && (ep == 0))
            {
                CyAsDeviceSetEp0Stalled(dev_p) ;
            }
            else
            {
                CyAsDeviceSetAckDelayed(dev_p) ;
                req_p->flags |= CY_AS_REQUEST_RESPONSE_DELAY_ACK ;
            }
        }

        ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyFalse, CyAsUsbFuncCallback) ;
        if (ret != CY_AS_ERROR_SUCCESS)
        {
            if (req_p->flags & CY_AS_REQUEST_RESPONSE_DELAY_ACK)
                CyAsDeviceRemAckDelayed(dev_p) ;
            CyAsRemoveCBTailNode(queue) ;

            goto destroy ;
        }
    }
    else
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

        if ((ret == CY_AS_ERROR_SUCCESS) && (request == CY_RQT_STALL_ENDPOINT))
        {
            if ((ep > 1) && (state != 0) && (dev_p->usb_config[ep].dir == CyAsUsbOut))
                CyAsUsbFlushLogicalEP(dev_p, ep) ;
        }

destroy:
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    }

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseGetStall(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         CyBool *state_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t code = CyAsLLRequestResponse_GetCode(reply_p) ;

    if (code == CY_RESP_SUCCESS_FAILURE)
    {
        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        goto destroy ;
    }
    else if (code != CY_RESP_ENDPOINT_STALL)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    *state_p = (CyBool)CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    ret = CY_AS_ERROR_SUCCESS ;


destroy :
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;

        return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseGetNak(CyAsDevice* dev_p,
                       CyAsLLRequestResponse *req_p,
                       CyAsLLRequestResponse *reply_p,
                       CyBool *state_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint8_t code = CyAsLLRequestResponse_GetCode(reply_p) ;

    if (code == CY_RESP_SUCCESS_FAILURE)
    {
        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        goto destroy ;
    }
    else if (code != CY_RESP_ENDPOINT_NAK)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    *state_p = (CyBool)CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    ret = CY_AS_ERROR_SUCCESS ;


destroy :
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;

        return ret ;
}

static CyAsReturnStatus_t
CyAsUsbGetNakStall(CyAsDeviceHandle handle,
                   CyAsEndPointNumber_t ep,
                   uint16_t request,
                   uint16_t response,
                   CyBool *state_p,
                   CyAsFunctionCallback cb,
                   uint32_t client)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p , *reply_p ;
    CyAsDevice *dev_p = (CyAsDevice *)handle ;

    (void)response ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p) && !cb)
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    req_p = CyAsLLCreateRequest(dev_p, request, CY_RQT_USB_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* Set the endpoint */
    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)ep) ;

    /* A single status word response type */
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

        if(request == CY_RQT_GET_STALL)
            return MyHandleResponseGetStall(dev_p, req_p, reply_p, state_p) ;
        else
            return MyHandleResponseGetNak(dev_p, req_p, reply_p, state_p) ;

    }
    else
    {
        CyAsFunctCBType type ;

        if(request == CY_RQT_GET_STALL)
            type = CY_FUNCT_CB_USB_GETSTALL ;
        else
            type = CY_FUNCT_CB_USB_GETNAK ;

        ret = CyAsMiscSendRequest(dev_p, cb, client, type,
            state_p, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

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
CyAsUsbSetNak(CyAsDeviceHandle handle,
                CyAsEndPointNumber_t ep,
                CyAsFunctionCallback cb,
                uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /*
    * We send the firmware the EP# with the appropriate direction bit, regardless
    * of what the user gave us.
    */
    ep &= 0x0f ;
    if (dev_p->usb_config[ep].dir == CyAsUsbIn)
        ep |= 0x80 ;

        if(dev_p->mtp_count > 0)
                return CY_AS_ERROR_NOT_VALID_IN_MTP ;

    return CyAsUsbNakStallRequest(handle, ep, CY_RQT_ENDPOINT_SET_NAK, CyTrue, 0, cb, client) ;
}


CyAsReturnStatus_t
CyAsUsbClearNak(CyAsDeviceHandle handle,
                  CyAsEndPointNumber_t ep,
                  CyAsFunctionCallback cb,
                  uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /*
    * We send the firmware the EP# with the appropriate direction bit, regardless
    * of what the user gave us.
    */
    ep &= 0x0f ;
    if (dev_p->usb_config[ep].dir == CyAsUsbIn)
        ep |= 0x80 ;

        if(dev_p->mtp_count > 0)
                return CY_AS_ERROR_NOT_VALID_IN_MTP ;

    return CyAsUsbNakStallRequest(handle, ep, CY_RQT_ENDPOINT_SET_NAK, CyFalse, 0, cb, client) ;
}

CyAsReturnStatus_t
CyAsUsbGetNak(CyAsDeviceHandle handle,
                CyAsEndPointNumber_t ep,
                CyBool *nak_p,
                CyAsFunctionCallback cb,
                uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /*
    * We send the firmware the EP# with the appropriate direction bit, regardless
    * of what the user gave us.
    */
    ep &= 0x0f ;
    if (dev_p->usb_config[ep].dir == CyAsUsbIn)
        ep |= 0x80 ;

        if(dev_p->mtp_count > 0)
                return CY_AS_ERROR_NOT_VALID_IN_MTP ;

    return CyAsUsbGetNakStall(handle, ep, CY_RQT_GET_ENDPOINT_NAK, CY_RESP_ENDPOINT_NAK, nak_p, cb, client ) ;
}


CyAsReturnStatus_t
CyAsUsbSetStall(CyAsDeviceHandle handle,
                  CyAsEndPointNumber_t ep,
                  CyAsFunctionCallback cb,
                  uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /*
    * We send the firmware the EP# with the appropriate direction bit, regardless
    * of what the user gave us.
    */
    ep &= 0x0f ;
    if (dev_p->usb_config[ep].dir == CyAsUsbIn)
        ep |= 0x80 ;

    if(dev_p->mtp_turbo_active)
        return CY_AS_ERROR_NOT_VALID_DURING_MTP ;

    return CyAsUsbNakStallRequest(handle, ep, CY_RQT_STALL_ENDPOINT, CyTrue, 0, cb, client) ;
}

CyAsReturnStatus_t
CyAsUsbClearStall(CyAsDeviceHandle handle,
                    CyAsEndPointNumber_t ep,
                    CyAsFunctionCallback cb,
                    uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /*
    * We send the firmware the EP# with the appropriate direction bit, regardless
    * of what the user gave us.
    */
    ep &= 0x0f ;
    if (dev_p->usb_config[ep].dir == CyAsUsbIn)
        ep |= 0x80 ;

    if(dev_p->mtp_turbo_active)
        return CY_AS_ERROR_NOT_VALID_DURING_MTP ;

    return CyAsUsbNakStallRequest(handle, ep, CY_RQT_STALL_ENDPOINT, CyFalse, 0, cb, client) ;
}

CyAsReturnStatus_t
CyAsUsbGetStall(CyAsDeviceHandle handle,
                  CyAsEndPointNumber_t ep,
                  CyBool *stall_p,
                  CyAsFunctionCallback cb,
                  uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /*
    * We send the firmware the EP# with the appropriate direction bit, regardless
    * of what the user gave us.
    */
    ep &= 0x0f ;
    if (dev_p->usb_config[ep].dir == CyAsUsbIn)
        ep |= 0x80 ;

    if(dev_p->mtp_turbo_active)
        return CY_AS_ERROR_NOT_VALID_DURING_MTP ;

    return CyAsUsbGetNakStall(handle, ep, CY_RQT_GET_STALL, CY_RESP_ENDPOINT_STALL, stall_p, cb, client) ;
}

CyAsReturnStatus_t
CyAsUsbSignalRemoteWakeup(CyAsDeviceHandle handle,
        CyAsFunctionCallback cb,
        uint32_t client)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p , *reply_p ;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (CyAsDeviceIsInCallback(dev_p))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    if (dev_p->usb_last_event != CyAsEventUsbSuspend)
        return CY_AS_ERROR_NOT_IN_SUSPEND ;

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_USB_REMOTE_WAKEUP, CY_RQT_USB_RQT_CONTEXT, 0) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* A single status word response type */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
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

        if (CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_SUCCESS_FAILURE)
            ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        else
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_SIGNALREMOTEWAKEUP,
            0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

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
CyAsUsbSetMSReportThreshold(CyAsDeviceHandle handle,
        uint32_t wr_sectors,
        uint32_t rd_sectors,
        CyAsFunctionCallback cb,
        uint32_t client)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p , *reply_p ;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if ((cb == 0) && (CyAsDeviceIsInCallback(dev_p)))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    /* Check if the firmware version supports this feature. */
    if ((dev_p->media_supported[0]) && (dev_p->media_supported[0] == (1 << CyAsMediaNand)))
        return CY_AS_ERROR_NOT_SUPPORTED ;

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_USB_STORAGE_MONITOR, CY_RQT_USB_RQT_CONTEXT, 4) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* A single status word response type */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    /* Set the read and write count parameters into the request structure. */
    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)((wr_sectors >> 16) & 0xFFFF)) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)(wr_sectors & 0xFFFF)) ;
    CyAsLLRequestResponse_SetWord(req_p, 2, (uint16_t)((rd_sectors >> 16) & 0xFFFF)) ;
    CyAsLLRequestResponse_SetWord(req_p, 3, (uint16_t)(rd_sectors & 0xFFFF)) ;

    if (cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        if (CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_SUCCESS_FAILURE)
            ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        else
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_USB_SET_MSREPORT_THRESHOLD,
            0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

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
CyAsUsbSelectMSPartitions (
        CyAsDeviceHandle        handle,
        CyAsBusNumber_t         bus,
        uint32_t                device,
        CyAsUsbMSType_t         type,
        CyAsFunctionCallback    cb,
        uint32_t                client)
{
    CyAsReturnStatus_t ret ;
    CyAsLLRequestResponse *req_p , *reply_p ;
    uint16_t val ;

    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = IsUsbActive(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /* This API has to be made before SetEnumConfig is called. */
    if (dev_p->usb_config[0].enabled)
        return CY_AS_ERROR_INVALID_CALL_SEQUENCE ;

    if ((cb == 0) && (CyAsDeviceIsInCallback(dev_p)))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_MS_PARTITION_SELECT, CY_RQT_USB_RQT_CONTEXT, 2) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* A single status word response type */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    /* Set the read and write count parameters into the request structure. */
    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)((bus << 8) | device)) ;

    val = 0 ;
    if ((type == CyAsUsbMSUnit0) || (type == CyAsUsbMSBoth))
        val |= 1 ;
    if ((type == CyAsUsbMSUnit1) || (type == CyAsUsbMSBoth))
        val |= (1 << 8) ;

    CyAsLLRequestResponse_SetWord(req_p, 1, val) ;

    if (cb == 0)
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;

        if (CyAsLLRequestResponse_GetCode(reply_p) == CY_RESP_SUCCESS_FAILURE)
            ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
        else
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_NODATA,
            0, dev_p->func_cbs_usb, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsUsbFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
            goto destroy ;
        return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static void
CyAsUsbFuncCallback(
                    CyAsDevice *dev_p,
                    uint8_t context,
                    CyAsLLRequestResponse *rqt,
                    CyAsLLRequestResponse *resp,
                    CyAsReturnStatus_t stat)
{
    CyAsUsbFuncCBNode*  node = (CyAsUsbFuncCBNode*)dev_p->usb_func_cbs->head_p ;
    CyAsFuncCBNode*     fnode = (CyAsFuncCBNode*)dev_p->func_cbs_usb->head_p ;
    CyAsReturnStatus_t  ret = CY_AS_ERROR_SUCCESS ;

    CyAsDeviceHandle    h = (CyAsDeviceHandle)dev_p ;
    CyBool              delayed_ack = (rqt->flags & CY_AS_REQUEST_RESPONSE_DELAY_ACK) == CY_AS_REQUEST_RESPONSE_DELAY_ACK;
    CyBool              exRequest = (rqt->flags & CY_AS_REQUEST_RESPONSE_EX) == CY_AS_REQUEST_RESPONSE_EX ;
    CyBool              msRequest = (rqt->flags & CY_AS_REQUEST_RESPONSE_MS) == CY_AS_REQUEST_RESPONSE_MS ;
    uint8_t             code ;
    uint8_t             ep, state ;

    if(!exRequest && !msRequest)
    {
        CyAsHalAssert(dev_p->usb_func_cbs->count != 0) ;
        CyAsHalAssert(dev_p->usb_func_cbs->type == CYAS_USB_FUNC_CB) ;
    }
    else
    {
        CyAsHalAssert(dev_p->func_cbs_usb->count != 0) ;
        CyAsHalAssert(dev_p->func_cbs_usb->type == CYAS_FUNC_CB) ;
    }

    (void)context ;

    /* The Handlers are responsible for Deleting the rqt and resp when
     * they are finished
     */
    code = CyAsLLRequestResponse_GetCode(rqt) ;
    switch(code)
    {
    case CY_RQT_START_USB:
        ret = MyHandleResponseUsbStart(dev_p, rqt, resp, stat) ;
        break ;
    case CY_RQT_STOP_USB:
        ret = MyHandleResponseUsbStop(dev_p, rqt, resp, stat) ;
        break ;
    case CY_RQT_SET_CONNECT_STATE:
        if(!CyAsLLRequestResponse_GetWord(rqt, 0))
            ret = MyHandleResponseDisconnect(dev_p, rqt, resp, stat) ;
        else
            ret = MyHandleResponseConnect(dev_p, rqt, resp, stat) ;
        break ;
    case CY_RQT_GET_CONNECT_STATE:
        break ;
    case CY_RQT_SET_USB_CONFIG:
        ret = MyHandleResponseSetEnumConfig(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_GET_USB_CONFIG:
        CyAsHalAssert(fnode->data != 0) ;
        ret = MyHandleResponseGetEnumConfig(dev_p, rqt, resp, fnode->data) ;
        break ;
    case CY_RQT_STALL_ENDPOINT:
        ep    = (uint8_t)CyAsLLRequestResponse_GetWord(rqt, 0) ;
        state = (uint8_t)CyAsLLRequestResponse_GetWord(rqt, 1) ;
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        if ((ret == CY_AS_ERROR_SUCCESS) && (ep > 1) && (state != 0) && (dev_p->usb_config[ep].dir == CyAsUsbOut))
            CyAsUsbFlushLogicalEP(dev_p, ep) ;
        break ;
    case CY_RQT_GET_STALL:
        CyAsHalAssert(fnode->data != 0) ;
        ret = MyHandleResponseGetStall(dev_p, rqt, resp, (CyBool*)fnode->data) ;
        break ;
    case CY_RQT_SET_DESCRIPTOR:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_GET_DESCRIPTOR:
        CyAsHalAssert(fnode->data != 0) ;
        ret = MyHandleResponseGetDescriptor(dev_p, rqt, resp, (CyAsGetDescriptorData*)fnode->data) ;
        break;
    case CY_RQT_SET_USB_CONFIG_REGISTERS:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        if (ret == CY_AS_ERROR_SUCCESS)
            ret = CyAsUsbSetupDma(dev_p) ;
        break ;
    case CY_RQT_ENDPOINT_SET_NAK:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_GET_ENDPOINT_NAK:
        CyAsHalAssert(fnode->data != 0) ;
        ret = MyHandleResponseGetNak(dev_p, rqt, resp, (CyBool*)fnode->data) ;
        break ;
    case CY_RQT_ACK_SETUP_PACKET:
        break ;
    case CY_RQT_USB_REMOTE_WAKEUP:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_CLEAR_DESCRIPTORS:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_USB_STORAGE_MONITOR:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
        break ;
    case CY_RQT_MS_PARTITION_SELECT:
        ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
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

    if(exRequest || msRequest)
    {
        fnode->cb_p((CyAsDeviceHandle)dev_p, stat, fnode->client_data, (CyAsFunctCBType)fnode->dataType, fnode->data) ;
        CyAsRemoveCBNode(dev_p->func_cbs_usb) ;
    }
    else
    {
        node->cb_p((CyAsDeviceHandle)dev_p, stat, node->client_data) ;
        CyAsRemoveCBNode(dev_p->usb_func_cbs) ;
    }

    if(delayed_ack)
    {
        CyAsHalAssert(CyAsDeviceIsAckDelayed(dev_p)) ;
        CyAsDeviceRemAckDelayed(dev_p) ;

        /*
         * Send the ACK if required.
         */
        if (!CyAsDeviceIsAckDelayed(dev_p))
            CyAsUsbAckSetupPacket(h, UsbAckCallback, 0) ;
    }
}

/* This includes the implementation of the deprecated functions for backward
 * compatibility
 */
#include "cyasusb_dep_impl.h"

/*[]*/
