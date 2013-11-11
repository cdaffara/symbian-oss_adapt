/* Cypress West Bridge API source file (cyasmisc.c)
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
#include "cyasmisc.h"
#include "cyasdma.h"
#include "cyasintr.h"
#include "cyaserr.h"
#include "cyasregs.h"
#include "cyaslowlevel.h"
#include "cyasprotocol.h"

/*
* The device list, the only global in the API
*/
static CyAsDevice *gDeviceList = 0 ;

/*
 * The current debug level
 */
#ifdef CY_AS_LOG_SUPPORT
static uint8_t DebugLevel = 0 ; //commented out as it's not

/*
 * This function sets the debug level for the API
 *
 */
void
CyAsMiscSetLogLevel(uint8_t level)
{
    DebugLevel = level ;
}

//#ifdef CY_AS_LOG_SUPPORT

/*
 * This function is a low level logger for the API.
 */
void
CyAsLogDebugMessage(int level, const char *str)
{
    if (level <= DebugLevel)
        CyAsHalPrintMessage("Log %d: %s\n", level, str) ;
}

#endif

#define CyAsCheckDeviceReady(dev_p)                                     \
{                                                                       \
    if (!(dev_p) ||((dev_p)->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))     \
        return CY_AS_ERROR_INVALID_HANDLE ;                             \
                                                                        \
    if (!CyAsDeviceIsConfigured(dev_p))                                 \
        return CY_AS_ERROR_NOT_CONFIGURED ;                             \
                                                                        \
    if (!CyAsDeviceIsFirmwareLoaded(dev_p))                             \
        return CY_AS_ERROR_NO_FIRMWARE ;                                \
}

/* Find an West Bridge device based on a TAG */
CyAsDevice *
CyAsDeviceFindFromTag(CyAsHalDeviceTag tag)
{
    CyAsDevice *dev_p ;

    for(dev_p = gDeviceList; dev_p != 0; dev_p = dev_p->next_p)
    {
        if (dev_p->tag == tag)
            return dev_p ;
    }

    return 0 ;
}

/* Map a pre-V1.2 media type to the V1.2+ bus number */
static void
CyAsBusFromMediaType(CyAsMediaType type,
                        CyAsBusNumber_t* bus)
{
    if (type == CyAsMediaNand)
    {
        *bus = 0 ;
    }
    else
    {
        *bus = 1 ;
    }
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

/*
* Create a new West Bridge device
*/
CyAsReturnStatus_t
CyAsMiscCreateDevice(CyAsDeviceHandle *handle_p, CyAsHalDeviceTag tag)
{
    CyAsDevice *dev_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsLogDebugMessage(6, "CyAsMiscCreateDevice called") ;

    dev_p = (CyAsDevice *)CyAsHalAlloc(sizeof(CyAsDevice)) ;
    if (dev_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    CyAsHalMemSet(dev_p, 0, sizeof(CyAsDevice)) ;

    /*
     * Dynamically allocating this buffer to ensure that it is
     * word aligned.
     */
    dev_p->usb_ep_data = (uint8_t *)CyAsHalAlloc(64 * sizeof(uint8_t)) ;
    if (dev_p->usb_ep_data == 0)
    {
        CyAsHalFree(dev_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    dev_p->sig = CY_AS_DEVICE_HANDLE_SIGNATURE ;
    dev_p->tag = tag ;
    dev_p->usb_max_tx_size = 0x40 ;

    dev_p->storage_write_endpoint = CY_AS_P2S_WRITE_ENDPOINT ;
    dev_p->storage_read_endpoint = CY_AS_P2S_READ_ENDPOINT ;

    dev_p->func_cbs_misc = CyAsCreateCBQueue(CYAS_FUNC_CB) ;
    if(dev_p->func_cbs_misc == 0)
        goto destroy ;

    dev_p->func_cbs_res = CyAsCreateCBQueue(CYAS_FUNC_CB) ;
    if(dev_p->func_cbs_res == 0)
        goto destroy ;

    dev_p->func_cbs_stor = CyAsCreateCBQueue(CYAS_FUNC_CB) ;
    if(dev_p->func_cbs_stor == 0)
        goto destroy ;

    dev_p->func_cbs_usb = CyAsCreateCBQueue(CYAS_FUNC_CB) ;
    if(dev_p->func_cbs_usb == 0)
        goto destroy ;

    dev_p->func_cbs_mtp = CyAsCreateCBQueue(CYAS_FUNC_CB) ;
    if(dev_p->func_cbs_mtp == 0)
            goto destroy ;

    /*
     * Allocate memory for the DMA module here. It is then marked idle, and
     * will be activated when CyAsMiscConfigureDevice is called.
     */
    ret = CyAsDmaStart(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    CyAsDeviceSetDmaStopped(dev_p) ;

    /*
     * Allocate memory for the low level module here. This module is also
     * activated only when CyAsMiscConfigureDevice is called.
     */
    ret = CyAsLLStart(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        goto destroy ;

    CyAsDeviceSetLowLevelStopped(dev_p) ;

    dev_p->next_p = gDeviceList ;
    gDeviceList = dev_p ;

    *handle_p = dev_p ;
    CyAsHalInitDevRegisters(tag, CyFalse) ;
    return CY_AS_ERROR_SUCCESS ;

destroy:
    /* Free any queues that were successfully allocated. */
    if (dev_p->func_cbs_misc) CyAsDestroyCBQueue(dev_p->func_cbs_misc) ;
    if (dev_p->func_cbs_res)  CyAsDestroyCBQueue(dev_p->func_cbs_res) ;
    if (dev_p->func_cbs_stor) CyAsDestroyCBQueue(dev_p->func_cbs_stor) ;
    if (dev_p->func_cbs_usb)  CyAsDestroyCBQueue(dev_p->func_cbs_usb) ;
    if (dev_p->func_cbs_mtp)  CyAsDestroyCBQueue(dev_p->func_cbs_mtp) ;

    CyAsHalFree(dev_p->usb_ep_data) ;
    CyAsHalFree(dev_p) ;

    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;
    else
        return CY_AS_ERROR_OUT_OF_MEMORY ;
}

/*
* Destroy an existing West Bridge device
*/
CyAsReturnStatus_t
CyAsMiscDestroyDevice(CyAsDeviceHandle handle)
{
    CyAsReturnStatus_t ret ;
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscDestroyDevice called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /*
    * If the USB stack is still running, it must be stopped first
    */
    if (dev_p->usb_count > 0)
        return CY_AS_ERROR_STILL_RUNNING ;

    /*
    * If the STORAGE stack is still running, it must be stopped first
    */
    if (dev_p->storage_count > 0)
        return CY_AS_ERROR_STILL_RUNNING ;

    if(CyAsDeviceIsIntrRunning(dev_p))
        ret = CyAsIntrStop(dev_p) ;

    ret = CyAsLLStop(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsIntrStart(dev_p, dev_p->use_int_drq) ;
        return ret ;
    }

    ret = CyAsDmaStop(dev_p) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsIntrStart(dev_p, dev_p->use_int_drq) ;
        return ret ;
    }

    /* Reset the West Bridge device. */
    CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_RST_CTRL_REG, CY_AS_MEM_RST_CTRL_REG_HARD) ;

    /*
    * Remove the device from the device list
    */
    if (gDeviceList == dev_p)
    {
        gDeviceList = dev_p->next_p ;
    }
    else
    {
        CyAsDevice *tmp_p = gDeviceList ;
        while (tmp_p && tmp_p->next_p != dev_p)
            tmp_p = tmp_p->next_p ;

        CyAsHalAssert(tmp_p != 0) ;
        tmp_p->next_p = dev_p->next_p ;
    }

    /*
    * Reset the signature so this will not be detected
    * as a valid handle
    */
    dev_p->sig = 0 ;

    CyAsDestroyCBQueue(dev_p->func_cbs_misc) ;
    CyAsDestroyCBQueue(dev_p->func_cbs_res) ;
    CyAsDestroyCBQueue(dev_p->func_cbs_stor) ;
    CyAsDestroyCBQueue(dev_p->func_cbs_usb) ;
    CyAsDestroyCBQueue(dev_p->func_cbs_mtp) ;

    /*
    * Free the memory associated with the device
    */
    CyAsHalFree(dev_p->usb_ep_data) ;
    CyAsHalFree(dev_p) ;

    return CY_AS_ERROR_SUCCESS ;
}

/*
* Determine the endian mode for the processor we are running on, then
* set the endian mode register
*/
static void
CyAsSetupEndianMode(CyAsDevice *dev_p)
{
    /*
    * BWG: In general, we always set West Bridge into the little endian mode.  This
    *      causes the data on bit 0 internally to come out on data line 0 externally
    *      and it is generally what we want regardless of the endian mode of the
    *      processor.  This capability in West Bridge should be labeled as a "SWAP" capability
    *      and can be used to swap the bytes of data in and out of West Bridge.  This is
    *      useful if there is DMA hardware that requires this for some reason I cannot
    *      imagine at this time.  Basically if the wires are connected correctly, we should
    *      never need to change the endian-ness of West Bridge.
    */
    CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_ENDIAN, CY_AS_LITTLE_ENDIAN) ;
}

/*
* Query the West Bridge device and determine if we are an standby mode
*/
CyAsReturnStatus_t
CyAsMiscInStandby(CyAsDeviceHandle handle, CyBool *standby)
{
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscInStandby called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (CyAsDeviceIsPinStandby(dev_p) || CyAsDeviceIsRegisterStandby(dev_p))
    {
        *standby = CyTrue ;
    }
    else
        *standby = CyFalse ;

    return CY_AS_ERROR_SUCCESS ;
}

static void
CyAsMiscFuncCallback(CyAsDevice *dev_p,
                        uint8_t context,
                        CyAsLLRequestResponse *rqt,
                        CyAsLLRequestResponse *resp,
                        CyAsReturnStatus_t ret) ;


static void
MyMiscCallback(CyAsDevice *dev_p, uint8_t context, CyAsLLRequestResponse *req_p, CyAsLLRequestResponse *resp_p,
               CyAsReturnStatus_t ret)
{
    (void)resp_p ;
    (void)context ;
    (void)ret ;

    switch (CyAsLLRequestResponse_GetCode(req_p))
    {
        case CY_RQT_INITIALIZATION_COMPLETE:
            {
                uint16_t v ;

                CyAsLLSendStatusResponse(dev_p, CY_RQT_GENERAL_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
                CyAsDeviceSetFirmwareLoaded(dev_p) ;

                if (CyAsDeviceIsWaking(dev_p))
                {
                    /*
                     * This is a callback from a CyAsMiscLeaveStandby() request.  In this case we call the
                     * standby callback and clear the waking state.
                     */
                    if (dev_p->misc_event_cb)
                        dev_p->misc_event_cb((CyAsDeviceHandle)dev_p, CyAsEventMiscAwake, 0) ;
                    CyAsDeviceClearWaking(dev_p) ;
                }
                else
                {
                    v = CyAsLLRequestResponse_GetWord(req_p, 3) ;

                    /*
                     * Store the media supported on each of the device buses.
                     */
                    dev_p->media_supported[0] = (uint8_t)(v & 0xFF) ;
                    dev_p->media_supported[1] = (uint8_t)((v >> 8) & 0xFF) ;

                    v = CyAsLLRequestResponse_GetWord(req_p, 4) ;
                    dev_p->is_mtp_firmware    = (CyBool)((v >> 8) & 0xFF) ;

                    if (dev_p->misc_event_cb)
                        dev_p->misc_event_cb((CyAsDeviceHandle)dev_p, CyAsEventMiscInitialized, 0) ;
                }

                v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_VM_SET) ;
                if (v & CY_AS_MEM_P0_VM_SET_CFGMODE)
                    CyAsHalPrintMessage("Initialization Message Recieved, but config bit still set\n") ;

                v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_RST_CTRL_REG) ;
                if ((v & CY_AS_MEM_RST_RSTCMPT) ==0)
                    CyAsHalPrintMessage("Initialization Message Recieved, but reset complete bit still not set\n") ;
            }
            break ;

        case CY_RQT_OUT_OF_SUSPEND:
            CyAsLLSendStatusResponse(dev_p, CY_RQT_GENERAL_RQT_CONTEXT, CY_AS_ERROR_SUCCESS, 0) ;
            CyAsDeviceClearSuspendMode(dev_p) ;

            /*
             * If the wakeup was caused by an async CyAsMiscLeaveSuspend call, we have to
             * call the corresponding callback.
             */
            if (dev_p->func_cbs_misc->count > 0)
            {
                CyAsFuncCBNode *node = (CyAsFuncCBNode*)dev_p->func_cbs_misc->head_p ;
                CyAsHalAssert(node) ;

                if (CyAsFunctCBTypeGetType(node->dataType) == CY_FUNCT_CB_MISC_LEAVESUSPEND)
                {
                    CyAsHalAssert(node->cb_p != 0) ;
                    node->cb_p((CyAsDeviceHandle)dev_p, CY_AS_ERROR_SUCCESS, node->client_data,
                            CY_FUNCT_CB_MISC_LEAVESUSPEND, 0) ;
                    CyAsRemoveCBNode(dev_p->func_cbs_misc) ;
                }
            }

            if (dev_p->misc_event_cb)
                dev_p->misc_event_cb((CyAsDeviceHandle)dev_p, CyAsEventMiscWakeup, 0) ;
            break ;

        case CY_RQT_DEBUG_MESSAGE:
            if ((req_p->data[0] == 0) && (req_p->data[1] == 0) && (req_p->data[2] == 0))
            {
                if (dev_p->misc_event_cb)
                    dev_p->misc_event_cb((CyAsDeviceHandle)dev_p, CyAsEventMiscHeartBeat, 0) ;
            }
            else
            {
                CyAsHalPrintMessage("**** Debug Message: %02x %02x %02x %02x %02x %02x\n",
                        req_p->data[0] & 0xff, (req_p->data[0] >> 8) & 0xff,
                        req_p->data[1] & 0xff, (req_p->data[1] >> 8) & 0xff,
                        req_p->data[2] & 0xff, (req_p->data[2] >> 8) & 0xff) ;
            }
            break ;

        case CY_RQT_WB_DEVICE_MISMATCH:
            {
                if (dev_p->misc_event_cb)
                    dev_p->misc_event_cb((CyAsDeviceHandle)dev_p, CyAsEventMiscDeviceMismatch, 0) ;
            }
            break ;

        case CY_RQT_BOOTLOAD_NO_FIRMWARE:
            {
                /* TODO Handle case when firmware is not found during bootloading. */
                CyAsHalPrintMessage("No firmware image found during bootload. Device not started\n") ;
            }
            break ;

        default:
            CyAsHalAssert (0) ;
    }
}

static CyBool
IsValidSiliconId(uint16_t v)
{
    CyBool idok = CyFalse ;

    /*
    * Remove the revision number from the ID value
    */
    v = v & CY_AS_MEM_CM_WB_CFG_ID_HDID_MASK ;

    /*
    * If this is West Bridge, then we are OK.
    */
    if (v == CY_AS_MEM_CM_WB_CFG_ID_HDID_ANTIOCH_VALUE ||
        v == CY_AS_MEM_CM_WB_CFG_ID_HDID_ASTORIA_FPGA_VALUE ||
        v == CY_AS_MEM_CM_WB_CFG_ID_HDID_ASTORIA_VALUE)
        idok = CyTrue ;

    return idok ;
}

/*
* Configure the West Bridge device hardware
*/
CyAsReturnStatus_t
CyAsMiscConfigureDevice(CyAsDeviceHandle handle, CyAsDeviceConfig *config_p)
{
    CyAsReturnStatus_t ret ;
    CyBool standby ;
    CyAsDevice *dev_p ;
    uint16_t v ;
    uint16_t fw_present;
    CyAsLogDebugMessage(6, "CyAsMiscConfigureDevice called") ;

    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /* Setup big endian vs little endian */
    CyAsSetupEndianMode(dev_p) ;

    /* Now, confirm that we can talk to the West Bridge device */
    dev_p->silicon_id = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_CM_WB_CFG_ID) ;
    fw_present = CyAsHalReadRegister(dev_p->tag,CY_AS_MEM_RST_CTRL_REG ) ;
    if (!(fw_present & CY_AS_MEM_RST_RSTCMPT))
    {
        if (!IsValidSiliconId(dev_p->silicon_id))
            return CY_AS_ERROR_NO_ANTIOCH ;
    }
    /* Check for standby mode */
    ret = CyAsMiscInStandby(handle, &standby) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;
    if (ret)
        return CY_AS_ERROR_IN_STANDBY ;

    /* Setup P-port interface mode (CRAM / SRAM). */
    if (CyAsDeviceIsAstoriaDev(dev_p))
    {
        if (config_p->srammode)
            v = CY_AS_MEM_P0_VM_SET_VMTYPE_SRAM ;
        else
            v = CY_AS_MEM_P0_VM_SET_VMTYPE_RAM ;
    }
    else
        v = CY_AS_MEM_P0_VM_SET_VMTYPE_RAM ;

    /* Setup synchronous versus asynchronous mode */
    if (config_p->sync)
        v |= CY_AS_MEM_P0_VM_SET_IFMODE ;
    if (config_p->dackmode == CyAsDeviceDackAck)
        v |= CY_AS_MEM_P0_VM_SET_DACKEOB ;
    if (config_p->drqpol)
        v |= CY_AS_MEM_P0_VM_SET_DRQPOL ;
    if (config_p->dackpol)
        v |= CY_AS_MEM_P0_VM_SET_DACKPOL ;
    CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_VM_SET, v) ;

    if (config_p->crystal)
        CyAsDeviceSetCrystal(dev_p) ;
    else
        CyAsDeviceSetExternalClock(dev_p) ;

    /* Register a callback to handle MISC requests from the firmware */
    CyAsLLRegisterRequestCallback(dev_p, CY_RQT_GENERAL_RQT_CONTEXT, MyMiscCallback) ;

    /* Now mark the DMA and low level modules as active. */
    CyAsDeviceSetDmaRunning(dev_p) ;
    CyAsDeviceSetLowLevelRunning(dev_p) ;

    /* Now, initialize the interrupt module */
    dev_p->use_int_drq = config_p->dmaintr ;

    ret = CyAsIntrStart(dev_p, config_p->dmaintr) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /* Mark the interface as initialized */
    CyAsDeviceSetConfigured(dev_p) ;

    return CY_AS_ERROR_SUCCESS ;
}

static void
MyDmaCallback(CyAsDevice *          dev_p,
              CyAsEndPointNumber_t  ep,
              void *                mem_p,
              uint32_t              size,
              CyAsReturnStatus_t    ret
              )
{
    CyAsDmaEndPoint *ep_p ;

    (void)size ;

    /* Get the endpoint pointer based on the endpoint number */
    ep_p = CY_AS_NUM_EP(dev_p, ep) ;

    /* Check the queue to see if is drained */
    if (ep_p->queue_p == 0)
    {
        CyAsFuncCBNode* node = (CyAsFuncCBNode*)dev_p->func_cbs_misc->head_p ;

        CyAsHalAssert(node) ;

        if (ret == CY_AS_ERROR_SUCCESS)
        {
            /*
             * Disable endpoint 2.  The storage module will enable this EP if necessary.
             */
            CyAsDmaEnableEndPoint(dev_p, CY_AS_FIRMWARE_ENDPOINT, CyFalse, CyAsDirectionIn) ;

            /*
             * Clear the reset register.  This releases the Antioch micro-controller from
             * reset and begins running the code at address zero.
             */
            CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_RST_CTRL_REG, 0x00) ;
        }

        /* Call the user Callback */
        node->cb_p((CyAsDeviceHandle)dev_p, ret, node->client_data, (CyAsFunctCBType)node->dataType, node->data) ;
        CyAsRemoveCBNode(dev_p->func_cbs_misc) ;
    }
    else
    {
        /* This is the header data that was allocated in the download firmware function,
        * and can be safely freed here. */
        uint32_t state = CyAsHalDisableInterrupts() ;
        CyAsHalCBFree(mem_p) ;
        CyAsHalEnableInterrupts(state) ;
    }
}

CyAsReturnStatus_t
CyAsMiscDownloadFirmware(CyAsDeviceHandle handle,
                           const void *mem_p,
                           uint16_t size,
                           CyAsFunctionCallback cb,
                           uint32_t client)
{
    uint8_t *header ;
    CyAsReturnStatus_t ret ;
    CyBool standby ;
    CyAsDevice *dev_p ;
    CyAsDmaCallback dmacb = 0 ;
    uint32_t state ;

    CyAsLogDebugMessage(6, "CyAsMiscDownloadFirmware called") ;

    /* Make sure we have a valid device */
    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /*
    * If the device has not been initialized, we cannot download firmware
    * to the device.
    */
    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    /*
    * Make sure West Bridge is not in standby
    */
    ret = CyAsMiscInStandby(dev_p, &standby) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (standby)
        return CY_AS_ERROR_IN_STANDBY ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /*
    * Make sure we are in configuration mode
    */
    if ((CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_VM_SET) & CY_AS_MEM_P0_VM_SET_CFGMODE) == 0)
        return CY_AS_ERROR_NOT_IN_CONFIG_MODE ;

    /* Maximum firmware size is 24k */
    if (size > CY_AS_MAXIMUM_FIRMWARE_SIZE)
        return CY_AS_ERROR_INVALID_SIZE ;

    /* Make sure the size is an even number of bytes as well */
    if (size & 0x01)
        return CY_AS_ERROR_ALIGNMENT_ERROR ;

    /*
     * Write the two word header that gives the base address and
     * size of the firmware image to download
     */
    state = CyAsHalDisableInterrupts() ;
    header = (uint8_t *)CyAsHalCBAlloc(4) ;
    CyAsHalEnableInterrupts(state) ;
    if (header == NULL)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    header[0] = 0x00 ;
    header[1] = 0x00 ;
    header[2] = (uint8_t)(size & 0xff) ;
    header[3] = (uint8_t)((size >> 8) & 0xff) ;

    /* Enable the firmware endpoint */
    ret = CyAsDmaEnableEndPoint(dev_p, CY_AS_FIRMWARE_ENDPOINT, CyTrue, CyAsDirectionIn) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /*
    * Setup DMA for 64 byte packets.  This is the requirement for downloading
    * firmware to West Bridge.
    */
    CyAsDmaSetMaxDmaSize(dev_p, CY_AS_FIRMWARE_ENDPOINT, 64) ;

    if(cb)
    {
        dmacb = MyDmaCallback ;
    }

    ret = CyAsDmaQueueRequest(dev_p, CY_AS_FIRMWARE_ENDPOINT, header, 4, CyFalse, CyFalse, dmacb) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /*
    * Write the firmware image to the West Bridge device
    */
    ret = CyAsDmaQueueRequest(dev_p, CY_AS_FIRMWARE_ENDPOINT, (void *)mem_p, size, CyFalse, CyFalse, dmacb) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if(cb)
    {
        CyAsFuncCBNode* cbnode = CyAsCreateFuncCBNodeData(cb, client, CY_FUNCT_CB_MISC_DOWNLOADFIRMWARE, 0) ;

        if(cbnode == 0)
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        else
            CyAsInsertCBNode(dev_p->func_cbs_misc, cbnode) ;

        ret = CyAsDmaKickStart(dev_p, CY_AS_FIRMWARE_ENDPOINT) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            return ret ;
    }
    else
    {

        ret = CyAsDmaDrainQueue(dev_p, CY_AS_FIRMWARE_ENDPOINT, CyTrue) ;

        /* Free the header memory that was allocated earlier. */
        CyAsHalCBFree(header) ;

        if (ret != CY_AS_ERROR_SUCCESS)
        {
			return ret ;
		}
        /*
        * Disable endpoint 2.  The storage module will enable this EP if necessary.
        */
        CyAsDmaEnableEndPoint(dev_p, CY_AS_FIRMWARE_ENDPOINT, CyFalse, CyAsDirectionIn) ;

        /*
        * Clear the reset register.  This releases the West Bridge micro-controller from
        * reset and begins running the code at address zero.
        */
        CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_RST_CTRL_REG, 0x00) ;
    }

    /*
    * The firmware is not marked as loaded until the firmware initializes West Bridge and a request
    * is sent from West Bridge to the P port processor indicating that West Bridge is ready.
    */
    return CY_AS_ERROR_SUCCESS ;
}


static CyAsReturnStatus_t
MyHandleResponseGetFirmwareVersion(CyAsDevice* dev_p,
                                   CyAsLLRequestResponse *req_p,
                                   CyAsLLRequestResponse *reply_p,
                                   CyAsGetFirmwareVersionData *data_p)
{

    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint16_t val ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_FIRMWARE_VERSION)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    data_p->major = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    data_p->minor = CyAsLLRequestResponse_GetWord(reply_p, 1) ;
    data_p->build = CyAsLLRequestResponse_GetWord(reply_p, 2) ;
    val    = CyAsLLRequestResponse_GetWord(reply_p, 3) ;
    data_p->mediaType   = (uint8_t)(((val >> 8) & 0xFF) | (val & 0xFF)) ;
    val    = CyAsLLRequestResponse_GetWord(reply_p, 4) ;
    data_p->isDebugMode = (CyBool)(val & 0xFF) ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsMiscGetFirmwareVersion(CyAsDeviceHandle handle,
                             CyAsGetFirmwareVersionData* data,
                             CyAsFunctionCallback cb,
                             uint32_t client)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyBool standby ;
    CyAsLLRequestResponse *req_p, *reply_p ;

    CyAsDevice *dev_p ;

    (void)client ;

    CyAsLogDebugMessage(6, "CyAsMiscGetFirmwareVersion called") ;

    /* Make sure we have a valid device */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /*
    * Make sure Antioch is not in standby
    */
    ret = CyAsMiscInStandby(dev_p, &standby) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;
    if (standby)
        return CY_AS_ERROR_IN_STANDBY ;

    /* Make sure the Antioch is not in suspend mode. */
    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_GET_FIRMWARE_VERSION, CY_RQT_GENERAL_RQT_CONTEXT, 0) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* Reserve space for the reply, the reply data will not exceed three words */
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

        /* Request and response are freed in MyHandleResponseGetFirmwareVersion. */
        ret = MyHandleResponseGetFirmwareVersion(dev_p, req_p, reply_p, data) ;
        return ret ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_GETFIRMWAREVERSION,
            data, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p,
            reply_p, CyAsMiscFuncCallback) ;

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
static CyAsReturnStatus_t
MyHandleResponseReadMCURegister(CyAsDevice* dev_p,
                                   CyAsLLRequestResponse *req_p,
                                   CyAsLLRequestResponse *reply_p,
                                   uint8_t *data_p)
{

    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_MCU_REGISTER_DATA)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    *data_p = (uint8_t)(CyAsLLRequestResponse_GetWord(reply_p, 0)) ;

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseGetGpioValue(CyAsDevice* dev_p,
        CyAsLLRequestResponse *req_p,
        CyAsLLRequestResponse *reply_p,
        uint8_t *data_p)
{

    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_GPIO_STATE)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
    }
    else
        *data_p = (uint8_t)(CyAsLLRequestResponse_GetWord(reply_p, 0)) ;

    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}


CyAsReturnStatus_t CyAsMiscSetSDPowerPolarity(
    CyAsDeviceHandle handle,
    CyAsMiscSignalPolarity polarity,
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


    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SDPOLARITY , CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
    {

        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }
    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)polarity) ;

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

        return (MyHandleResponseNoData(dev_p, req_p, reply_p)) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_SETSDPOLARITY,
            0, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX,
               req_p, reply_p, CyAsMiscFuncCallback) ;

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
CyAsMiscReadMCURegister(CyAsDeviceHandle handle,
                          uint16_t address,
                          uint8_t *value,
                          CyAsFunctionCallback cb,
                          uint32_t client)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsLLRequestResponse *req_p, *reply_p ;

    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscReadMCURegister called") ;

    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /* Check whether the firmware supports this command. */
    if (CyAsDeviceIsNandStorageSupported(dev_p))
        return CY_AS_ERROR_NOT_SUPPORTED ;

    /* Make sure the Antioch is not in suspend mode. */
    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_READ_MCU_REGISTER, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)address) ;

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

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_MCU_REGISTER_DATA)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        *value = (uint8_t)(CyAsLLRequestResponse_GetWord(reply_p, 0)) ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_READMCUREGISTER,
            value, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMiscFuncCallback) ;

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
CyAsMiscWriteMCURegister(CyAsDeviceHandle handle,
                           uint16_t address,
                           uint8_t mask,
                           uint8_t value,
                           CyAsFunctionCallback cb,
                           uint32_t client)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscWriteMCURegister called") ;

    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /* Check whether the firmware supports this command. */
    if (CyAsDeviceIsNandStorageSupported(dev_p))
        return CY_AS_ERROR_NOT_SUPPORTED ;

    /* Make sure the Antioch is not in suspend mode. */
    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_WRITE_MCU_REGISTER, CY_RQT_GENERAL_RQT_CONTEXT, 2) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)address) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((mask << 8) | value)) ;

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

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_WRITEMCUREGISTER,
            0, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMiscFuncCallback) ;

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
MyHandleResponseReset(CyAsDevice* dev_p,
                      CyAsLLRequestResponse *req_p,
                      CyAsLLRequestResponse *reply_p,
                      CyAsResetType type)
{
    (void)req_p ;
    (void)reply_p ;

    /*
    * If the device is in suspend mode, it needs to be woken up so that the write
    * to the reset control register succeeds. We need not however wait for the
     * wake up procedure to be complete.
     */
    if (CyAsDeviceIsInSuspendMode(dev_p))
    {
        CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_CM_WB_CFG_ID) ;
        CyAsHalSleep (1) ;
    }

    if (type == CyAsResetHard)
    {
        CyAsMiscCancelExRequests(dev_p) ;
        CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_RST_CTRL_REG, CY_AS_MEM_RST_CTRL_REG_HARD) ;
        CyAsDeviceSetUnconfigured(dev_p) ;
        CyAsDeviceSetFirmwareNotLoaded(dev_p) ;
        CyAsDeviceSetDmaStopped(dev_p) ;
        CyAsDeviceSetLowLevelStopped(dev_p) ;
        CyAsDeviceSetIntrStopped(dev_p) ;
        CyAsDeviceClearSuspendMode(dev_p) ;
        CyAsUsbCleanup(dev_p) ;
        CyAsStorageCleanup(dev_p) ;

        /*
         * Wait for a small amount of time to allow reset to be complete.
         */
        CyAsHalSleep(100) ;
    }

    CyAsDeviceClearResetPending(dev_p) ;

    return CY_AS_ERROR_SUCCESS ;
}

CyAsReturnStatus_t
CyAsMiscReset(CyAsDeviceHandle handle,
                CyAsResetType type,
                CyBool flush,
                CyAsFunctionCallback cb,
                uint32_t client)
{
    CyAsDevice *dev_p ;
    CyAsEndPointNumber_t i ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    (void)client ;
    (void)cb ;

    CyAsLogDebugMessage(6, "CyAsMiscResetEX called") ;

    /* Make sure the device is ready for the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /*
     * Soft reset is not supported until we close on the issues in the
     * firmware with what needs to happen.
     */
    if (type == CyAsResetSoft)
        return CY_AS_ERROR_NOT_YET_SUPPORTED ;

    CyAsDeviceSetResetPending(dev_p) ;

    if (flush)
    {

        /* Unable to DrainQueues in polling mode */
        if((dev_p->storage_cb || dev_p->storage_cb_ms) && CyAsHalIsPolling())
            return CY_AS_ERROR_ASYNC_PENDING ;

        /*
        * Shutdown the endpoints so no more traffic can be queued
        */
        for(i = 0; i < 15; i++)
            CyAsDmaEnableEndPoint(dev_p, i, CyFalse, CyAsDirectionDontChange) ;

        /*
         * If we are in normal mode, drain all traffic across all endpoints to be sure all traffic
         * is flushed. If the device is suspended, data will not be coming in on any endpoint and
         * all outstanding DMA operations can be canceled.
         */
        if (CyAsDeviceIsInSuspendMode(dev_p))
        {
            for(i = 0; i < 15; i++)
            {
                CyAsDmaCancel(dev_p, i, CY_AS_ERROR_CANCELED) ;
            }
        }
        else
        {
            for(i = 0; i < 15; i++)
            {
                if ((i == CY_AS_P2S_WRITE_ENDPOINT) || (i == CY_AS_P2S_READ_ENDPOINT))
                    CyAsDmaDrainQueue(dev_p, i, CyFalse) ;
                else
                    CyAsDmaDrainQueue(dev_p, i, CyTrue) ;
            }
        }
    }
    else
    {
        /* No flush was requested, so cancel any outstanding DMAs
         * so the user callbacks are called as needed
         */
        if(CyAsDeviceIsStorageAsyncPending(dev_p))
        {
            for(i = 0; i < 15; i++)
                CyAsDmaCancel(dev_p, i, CY_AS_ERROR_CANCELED) ;
        }
    }

    ret = MyHandleResponseReset(dev_p, 0, 0, type) ;

    if(cb)
        /* Even though no mailbox communication was needed, issue the callback so the
        * user does not need to special case their code. */
        cb((CyAsDeviceHandle)dev_p, ret, client, CY_FUNCT_CB_MISC_RESET, 0) ;

    /*
     * Initialize any registers that may have been changed when the device was reset.
     */
    CyAsHalInitDevRegisters(dev_p->tag, CyFalse) ;

    return ret ;
}

static CyAsReturnStatus_t
GetUnallocatedResource(CyAsDevice *dev_p, CyAsResourceType resource)
{
    uint8_t shift = 0 ;
    uint16_t v ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_NOT_ACQUIRED ;

    switch(resource)
    {
    case CyAsBusUSB:
        shift = 4 ;
        break ;
    case CyAsBus_1:
        shift = 0 ;
        break ;
    case CyAsBus_0:
        shift = 2 ;
        break ;
    default:
        CyAsHalAssert(CyFalse) ;
        break ;
    }

    /* Get the semaphore value for this resource */
    v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_RSE_ALLOCATE) ;
    v = (v >> shift) & 0x03 ;

    if (v == 0x03)
    {
        ret = CY_AS_ERROR_RESOURCE_ALREADY_OWNED ;
    }
    else if ((v & 0x01) == 0)
    {
        /* The resource is not owned by anyone, we can try to get it */
        CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_RSE_MASK, (0x03 << shift)) ;
        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_RSE_MASK) ;
        CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_RSE_ALLOCATE, (0x01 << shift)) ;
        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_RSE_MASK) ;

        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_RSE_ALLOCATE) ;
        v = (v >> shift) & 0x03 ;
        if (v == 0x03)
            ret = CY_AS_ERROR_SUCCESS ;
    }

    return ret ;
}

static CyAsReturnStatus_t
MyHandleResponseAcquireResource(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         CyAsResourceType *resource)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    if (ret == CY_AS_ERROR_SUCCESS)
    {
            ret = GetUnallocatedResource(dev_p, *resource) ;
            if (ret != CY_AS_ERROR_NOT_ACQUIRED)
                ret = CY_AS_ERROR_SUCCESS ;
    }

destroy :
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsMiscAcquireResource(CyAsDeviceHandle handle,
                          CyAsResourceType *resource,
                          CyBool force,
                          CyAsFunctionCallback cb,
                          uint32_t client)
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret ;

    CyAsDevice *dev_p ;

    (void)client ;

    CyAsLogDebugMessage(6, "CyAsMiscAcquireResource called") ;

    if (*resource != CyAsBusUSB && *resource != CyAsBus_0 && *resource != CyAsBus_1)
            return CY_AS_ERROR_INVALID_RESOURCE ;


    /* Make sure the device is ready to accept the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;


    ret = GetUnallocatedResource(dev_p, *resource) ;

    /*
     * Make sure that the callback is called if the resource is successfully
     * acquired at this point.
     */
    if ((ret == CY_AS_ERROR_SUCCESS) && (cb != 0))
        cb(handle, ret, client, CY_FUNCT_CB_MISC_ACQUIRERESOURCE, resource) ;

    if (ret != CY_AS_ERROR_NOT_ACQUIRED)
        return ret ;

    if (!force)
        return CY_AS_ERROR_NOT_ACQUIRED ;

    /* Create the request to acquire the resource */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_ACQUIRE_RESOURCE, CY_RQT_RESOURCE_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)(*resource)) ;

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
            ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_ACQUIRERESOURCE,
                resource, dev_p->func_cbs_res, CY_AS_REQUEST_RESPONSE_EX, req_p,
                reply_p, CyAsMiscFuncCallback) ;

            if (ret != CY_AS_ERROR_SUCCESS)
                    goto destroy ;

            /* The request and response are freed as part of the MiscFuncCallback */
            return ret ;
    }

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    if (ret == CY_AS_ERROR_SUCCESS)
    {
            ret = GetUnallocatedResource(dev_p, *resource) ;
            if (ret != CY_AS_ERROR_NOT_ACQUIRED)
                ret = CY_AS_ERROR_SUCCESS ;
    }

    return ret ;
}
CyAsReturnStatus_t
CyAsMiscReleaseResource(CyAsDeviceHandle handle, CyAsResourceType resource)
{
    uint8_t shift = 0 ;
    uint16_t v ;

    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscReleaseResource called") ;

    /* Make sure the device is ready for the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    if (resource != CyAsBusUSB && resource != CyAsBus_0 && resource != CyAsBus_1)
        return CY_AS_ERROR_INVALID_RESOURCE ;

    switch(resource)
    {
        case CyAsBusUSB:
            shift = 4 ;
            break ;
        case CyAsBus_1:
            shift = 0 ;
            break ;
        case CyAsBus_0:
            shift = 2 ;
            break ;
        default:
            CyAsHalAssert(CyFalse) ;
            break ;
    }

    /* Get the semaphore value for this resource */
    v = (CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_RSE_ALLOCATE) >> shift) & 0x03 ;
    if (v == 0 || v == 1 || v == 2)
        return CY_AS_ERROR_RESOURCE_NOT_OWNED ;

    CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_RSE_MASK, (0x03 << shift)) ;
    CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_RSE_ALLOCATE, (0x02 << shift)) ;
    CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_RSE_MASK, 0) ;

    return CY_AS_ERROR_SUCCESS ;
}

CyAsReturnStatus_t
CyAsMiscSetTraceLevel(CyAsDeviceHandle handle,
                        uint8_t level,
                        CyAsBusNumber_t bus,
                        uint32_t device,
                        uint32_t unit,
                        CyAsFunctionCallback cb,
                        uint32_t client)
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret ;
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscSetTraceLevel called") ;

    /* Make sure the device is ready for the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    if (bus >= CY_AS_MAX_BUSES)
        return CY_AS_ERROR_NO_SUCH_BUS ;

    if (device >= CY_AS_MAX_STORAGE_DEVICES)
        return CY_AS_ERROR_NO_SUCH_DEVICE ;

    if (unit > 255)
        return CY_AS_ERROR_NO_SUCH_UNIT ;

    if (level >= CYAS_FW_TRACE_MAX_LEVEL)
        return CY_AS_ERROR_INVALID_TRACE_LEVEL ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SET_TRACE_LEVEL, CY_RQT_GENERAL_RQT_CONTEXT, 2) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)level) ;
    CyAsLLRequestResponse_SetWord(req_p, 1, (uint16_t)((bus << 12) | (device << 8) | (unit))) ;

    /* Reserve space for the reply, the reply data will not exceed three words */
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

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
        {
            ret = CY_AS_ERROR_NOT_SUPPORTED ;
            goto destroy ;
        }

        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_SETTRACELEVEL,
            0, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMiscFuncCallback) ;

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
CyAsMiscHeartBeatControl(CyAsDeviceHandle handle,
                           CyBool enable,
                           CyAsFunctionCallback cb,
                           uint32_t client)
{
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsReturnStatus_t ret ;
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscHeartBeatControl called") ;

    /* Make sure the device is ready for the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_CONTROL_ANTIOCH_HEARTBEAT, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)enable) ;

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

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_HEARTBEATCONTROL,
            0, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMiscFuncCallback) ;

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

static CyAsReturnStatus_t
MySetSDClockFreq (
        CyAsDevice          *dev_p,
        uint8_t              cardType,
        uint8_t              setting,
        CyAsFunctionCallback cb,
        uint32_t             client)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsLLRequestResponse *req_p, *reply_p ;

    if (CyAsDeviceIsInCallback(dev_p) && (cb == 0))
        return CY_AS_ERROR_INVALID_IN_CALLBACK ;

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SET_SD_CLOCK_FREQ, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)((cardType << 8) | setting)) ;

    /* Reserve space for the reply, which will not exceed one word. */
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

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }
    else
    {
        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_SETSDFREQ, 0,
            dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p, CyAsMiscFuncCallback) ;

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
CyAsMiscSetLowSpeedSDFreq(
        CyAsDeviceHandle     handle,
        CyAsLowSpeedSDFreq   setting,
        CyAsFunctionCallback cb,
        uint32_t             client)
{
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscSetLowSpeedSDFreq called") ;

    /* Make sure the device is ready for the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    if ((setting != CY_AS_SD_DEFAULT_FREQ) && (setting != CY_AS_SD_RATED_FREQ))
        return CY_AS_ERROR_INVALID_PARAMETER ;

    return MySetSDClockFreq(dev_p, 0, (uint8_t)setting, cb, client) ;
}

CyAsReturnStatus_t
CyAsMiscSetHighSpeedSDFreq(
        CyAsDeviceHandle     handle,
        CyAsHighSpeedSDFreq  setting,
        CyAsFunctionCallback cb,
        uint32_t             client)
{
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscSetHighSpeedSDFreq called") ;

    /* Make sure the device is ready for the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    if ((setting != CY_AS_HS_SD_FREQ_24) && (setting != CY_AS_HS_SD_FREQ_48))
        return CY_AS_ERROR_INVALID_PARAMETER ;

    return MySetSDClockFreq(dev_p, 1, (uint8_t)setting, cb, client) ;
}

CyAsReturnStatus_t
CyAsMiscGetGpioValue(CyAsDeviceHandle handle,
        CyAsMiscGpio pin,
        uint8_t *value,
        CyAsFunctionCallback cb,
        uint32_t client)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsDevice *dev_p ;
    uint16_t v ;

    CyAsLogDebugMessage(6, "CyAsMiscGetGpioValue called") ;

    /* Make sure the device is ready for the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /* If the pin specified is UVALID, there is no need for firmware to be loaded. */
    if (pin == CyAsMiscGpio_UValid)
    {
        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_PMU_UPDATE) ;
        *value = (uint8_t)(v & CY_AS_MEM_PMU_UPDATE_UVALID) ;

        if (cb != 0)
            cb(dev_p, ret, client, CY_FUNCT_CB_MISC_GETGPIOVALUE, value) ;

        return ret ;
    }

    /* Check whether the firmware supports this command. */
    if (CyAsDeviceIsNandStorageSupported(dev_p))
        return CY_AS_ERROR_NOT_SUPPORTED ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /* Make sure the pin selected is valid */
    if ((pin != CyAsMiscGpio_1) && (pin != CyAsMiscGpio_0))
        return CY_AS_ERROR_INVALID_PARAMETER ;

    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_GET_GPIO_STATE, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    CyAsLLRequestResponse_SetWord(req_p, 0, ((uint8_t)pin << 8)) ;

    /* Reserve space for the reply, which will not exceed one word. */
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

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_GPIO_STATE)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        *value = (uint8_t)CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_GETGPIOVALUE,
            value, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMiscFuncCallback) ;

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
CyAsMiscSetGpioValue(CyAsDeviceHandle handle,
        CyAsMiscGpio pin,
        uint8_t value,
        CyAsFunctionCallback cb,
        uint32_t client)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyAsDevice *dev_p ;
    uint16_t v ;

    CyAsLogDebugMessage(6, "CyAsMiscSetGpioValue called") ;

    /* Make sure the device is ready for the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /* If the pin specified is UVALID, there is no need for firmware to be loaded. */
    if (pin == CyAsMiscGpio_UValid)
    {
        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_PMU_UPDATE) ;
        if (value)
            CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_PMU_UPDATE, (v | CY_AS_MEM_PMU_UPDATE_UVALID)) ;
        else
            CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_PMU_UPDATE, (v & ~CY_AS_MEM_PMU_UPDATE_UVALID)) ;

        if (cb != 0)
            cb(dev_p, ret, client, CY_FUNCT_CB_MISC_SETGPIOVALUE, 0) ;
        return ret ;
    }

    /* Check whether the firmware supports this command. */
    if (CyAsDeviceIsNandStorageSupported(dev_p))
        return CY_AS_ERROR_NOT_SUPPORTED ;

    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /* Make sure the pin selected is valid */
    if ((pin < CyAsMiscGpio_0) || (pin > CyAsMiscGpio_UValid))
        return CY_AS_ERROR_INVALID_PARAMETER ;

    /* Create and initialize the low level request to the firmware. */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_SET_GPIO_STATE, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    v = (uint16_t)(((uint8_t)pin << 8) | (value > 0)) ;
    CyAsLLRequestResponse_SetWord(req_p, 0, v) ;

    /* Reserve space for the reply, which will not exceed one word. */
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

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_SETGPIOVALUE,
            0, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMiscFuncCallback) ;

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

static CyAsReturnStatus_t
MyEnterStandby(CyAsDevice *dev_p, CyBool pin)
{
    CyAsMiscCancelExRequests(dev_p) ;

    /* Save the current values in the critical P-port registers, where necessary. */
    CyAsHalReadRegsBeforeStandby(dev_p->tag) ;

    if (pin)
    {
        if (CyAsHalSetWakeupPin(dev_p->tag, CyFalse))
            CyAsDeviceSetPinStandby(dev_p) ;
        else
            return CY_AS_ERROR_SETTING_WAKEUP_PIN ;
    }
    else
    {
        /*
         * Put antioch in the standby mode
         */
        CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_PWR_MAGT_STAT, 0x02) ;
        CyAsDeviceSetRegisterStandby(dev_p) ;
    }

    /*
     * When the Antioch comes out of standby, we have to wait until
     * the firmware initialization completes before sending other
     * requests down.
     */
    CyAsDeviceSetFirmwareNotLoaded(dev_p) ;

    /*
     * Keep West Bridge interrupt disabled until the device is being woken
     * up from standby.
     */
    dev_p->stby_int_mask = CyAsHalDisableInterrupts ();

    return CY_AS_ERROR_SUCCESS ;
}

static CyAsReturnStatus_t
MyHandleResponseEnterStandby(CyAsDevice* dev_p,
                         CyAsLLRequestResponse *req_p,
                         CyAsLLRequestResponse *reply_p,
                         CyBool pin)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
    {
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        goto destroy ;
    }

    ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;

destroy:
    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    ret = MyEnterStandby(dev_p, pin) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsMiscEnterStandby(CyAsDeviceHandle handle,
                        CyBool pin,
                        CyAsFunctionCallback cb,
                        uint32_t client)
{
    CyAsDevice *dev_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyAsLLRequestResponse *req_p, *reply_p ;
    CyBool standby ;

    CyAsLogDebugMessage(6, "CyAsMiscEnterStandby called") ;

    /* Make sure we have a valid device */
    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    /*
     * If we already are in standby, do not do it again and let the
     * user know via the error return.
     */
    ret = CyAsMiscInStandby(handle, &standby) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (standby == CyTrue)
        return CY_AS_ERROR_ALREADY_STANDBY ;

    /*
     * If the user wants to transition from suspend mode to standby mode,
     * the device needs to be woken up so that it can complete all pending
     * operations.
     */
    if (CyAsDeviceIsInSuspendMode(dev_p))
        CyAsMiscLeaveSuspend(dev_p, 0, 0) ;

    if (dev_p->usb_count)
    {
        /*
         * We do not allow West Bridge to go into standby mode when the
         * USB stack is initialized.  You must stop the USB stack in
         * order to enter standby mode.
         */
        return CY_AS_ERROR_USB_RUNNING ;
    }

    /*
     * If the storage stack is not running, the device can directly be put into
     * sleep mode. Otherwise, the firmware needs to be signaled to prepare for
     * going into sleep mode.
     */
    if (dev_p->storage_count)
    {
        /*
         * If there are async storage operations pending, make one attempt to
         * complete them.
         */
        if (CyAsDeviceIsStorageAsyncPending(dev_p))
        {
            /* DrainQueue will not work in polling mode */
            if(CyAsHalIsPolling())
                return CY_AS_ERROR_ASYNC_PENDING ;

            CyAsDmaDrainQueue(dev_p, CY_AS_P2S_READ_ENDPOINT, CyFalse) ;
            CyAsDmaDrainQueue(dev_p, CY_AS_P2S_WRITE_ENDPOINT, CyFalse) ;

            /*
             * If more storage operations were queued at this stage, return
             * an error.
             */
            if (CyAsDeviceIsStorageAsyncPending(dev_p))
                return CY_AS_ERROR_ASYNC_PENDING ;
        }

        req_p = CyAsLLCreateRequest(dev_p, CY_RQT_PREPARE_FOR_STANDBY, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
        if (req_p == 0)
            return CY_AS_ERROR_OUT_OF_MEMORY ;

        reply_p = CyAsLLCreateResponse(dev_p, 1) ;
        if (reply_p == 0)
        {
            CyAsLLDestroyRequest(dev_p, req_p) ;
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        }

        if(!cb)
        {
            ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            /* The request and response are freed in the HandleResponse */
            return MyHandleResponseEnterStandby(dev_p, req_p, reply_p, pin) ;

        }
        else
        {
            ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_ENTERSTANDBY,
                (void*)pin, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p,
                reply_p, CyAsMiscFuncCallback) ;

            if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

            /* The request and response are freed as part of the MiscFuncCallback */
            return ret ;
        }
destroy:
        CyAsLLDestroyRequest(dev_p, req_p) ;
        CyAsLLDestroyResponse(dev_p, reply_p) ;
    }
    else
    {
        ret = MyEnterStandby(dev_p, pin) ;
        if(cb)
            /* Even though no mailbox communication was needed, issue the callback so the
            * user does not need to special case their code. */
            cb((CyAsDeviceHandle)dev_p, ret, client, CY_FUNCT_CB_MISC_ENTERSTANDBY, 0) ;
    }

    return ret ;
}

CyAsReturnStatus_t
CyAsMiscEnterStandbyEXU(CyAsDeviceHandle handle,
                        CyBool pin,
                        CyBool uvalid_special,
                        CyAsFunctionCallback cb,
                        uint32_t client)
{
    CyAsDevice *dev_p ;

    dev_p = (CyAsDevice *)handle ;
    if(uvalid_special)
    {
        CyAsHalWriteRegister(dev_p->tag, 0xc5, 0x4) ;
    }

    return CyAsMiscEnterStandby(handle, pin, cb, client) ;
}

CyAsReturnStatus_t
CyAsMiscLeaveStandby(CyAsDeviceHandle handle, CyAsResourceType resource)
{
    CyAsDevice *dev_p ;
    uint16_t v ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint32_t count = 8 ;
    uint8_t  retry = 1 ;

    CyAsLogDebugMessage(6, "CyAsMiscLeaveStandby called") ;
    (void)resource ;

    /* Make sure we have a valid device */
    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (CyAsDeviceIsRegisterStandby(dev_p))
    {
        /*
         * Set a flag to indicate that the West Bridge is waking up from standby.
         */
        CyAsDeviceSetWaking(dev_p) ;

        /*
         * The initial read will not succeed, but will just wake the West Bridge
         * device from standby.  Successive reads should succeed and in that way
         * we know West Bridge is awake.
         */
        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_CM_WB_CFG_ID) ;

        do
        {
            /*
             * We have initiated the operation to leave standby, now we need to wait at least
             * N ms before trying to access the West Bridge device to insure the PLLs have locked and we
             * can talk to the device.
             */
            if (CyAsDeviceIsCrystal(dev_p))
                CyAsHalSleep(CY_AS_LEAVE_STANDBY_DELAY_CRYSTAL) ;
            else
                CyAsHalSleep(CY_AS_LEAVE_STANDBY_DELAY_CLOCK) ;
            v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_CM_WB_CFG_ID) ;

            /*
            * If the P-SPI interface mode is in use, there may be a need to re-synchronise the
            * serial clock used for Astoria access.
            */
            if (!IsValidSiliconId(v))
            {
                if (CyAsHalSyncDeviceClocks(dev_p->tag) != CyTrue)
                {
                    CyAsHalEnableInterrupts (dev_p->stby_int_mask) ;
                    return CY_AS_ERROR_TIMEOUT ;
                }
            }
        } while (!IsValidSiliconId(v) && count-- > 0) ;

        /*
         * If we tried to read the register and could not, return a timeout
         */
        if (count == 0)
        {
            CyAsHalEnableInterrupts (dev_p->stby_int_mask) ;
            return CY_AS_ERROR_TIMEOUT ;
        }

        /*
         * The standby flag is cleared here, after the action to exit standby has
         * been taken. The wait for firmware initialization, is ensured by marking
         * the firmware as not loaded until the init event is received.
         */
        CyAsDeviceClearRegisterStandby(dev_p) ;

        /*
         * Initialize any registers that may have been changed while the device was in standby mode.
         */
        CyAsHalInitDevRegisters(dev_p->tag, CyTrue) ;
    }
    else if (CyAsDeviceIsPinStandby(dev_p))
    {
        /*
         * Set a flag to indicate that the West Bridge is waking up from standby.
         */
        CyAsDeviceSetWaking(dev_p) ;

TryWakeupAgain:
        /*
        * Try to set the wakeup pin, if this fails in the HAL layer, return this
        * failure to the user.
        */
        if (!CyAsHalSetWakeupPin(dev_p->tag, CyTrue))
        {
            CyAsHalEnableInterrupts (dev_p->stby_int_mask) ;
            return CY_AS_ERROR_SETTING_WAKEUP_PIN ;
        }

        /*
        * We have initiated the operation to leave standby, now we need to wait at least
        * N ms before trying to access the West Bridge device to insure the PLLs have locked and we
        * can talk to the device.
        */
        if (CyAsDeviceIsCrystal(dev_p))
            CyAsHalSleep(CY_AS_LEAVE_STANDBY_DELAY_CRYSTAL) ;
        else
            CyAsHalSleep(CY_AS_LEAVE_STANDBY_DELAY_CLOCK) ;

        /*
         * Initialize any registers that may have been changed while the device was in standby mode.
         */
        CyAsHalInitDevRegisters(dev_p->tag, CyTrue) ;

        /*
         * The standby flag is cleared here, after the action to exit standby has
         * been taken. The wait for firmware initialization, is ensured by marking
         * the firmware as not loaded until the init event is received.
         */
        CyAsDeviceClearPinStandby(dev_p) ;
    }
    else
    {
        return CY_AS_ERROR_NOT_IN_STANDBY ;
    }

    /*
     * The West Bridge interrupt can be enabled now.
     */
    CyAsHalEnableInterrupts (dev_p->stby_int_mask) ;

    /*
     * Release the West Bridge Micro-Controller from reset, so that firmware initialization
     * can complete.
     * The attempt to release Antioch reset is made upto 8 times.
     */
    v     = 0x03 ;
    count = 0x08 ;
    while ((v & 0x03) && (count))
    {
        CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_RST_CTRL_REG, 0x00) ;
        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_RST_CTRL_REG) ;
        count-- ;
    }

    if (v & 0x03)
    {
        CyAsHalPrintMessage("Failed to clear Antioch reset\n") ;
        return CY_AS_ERROR_TIMEOUT ;
    }

    /*
     * If the wake-up pin is being used, wait here to make sure that the wake-up event
     * is received within a reasonable delay. Otherwise, toggle the wake-up pin again
     * in an attempt to start the firmware properly.
     */
    if (retry)
    {
        count = 10 ;
        while (count)
        {
            /* If the wake-up event has been received, we can return. */
            if (CyAsDeviceIsFirmwareLoaded (dev_p))
                break ;
            /* If we are in polling mode, the interrupt may not have been serviced as yet.
             * Read the interrupt status register. If a pending mailbox interrupt is seen,
             * we can assume that the wake-up event will be received soon. */
            v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_INTR_REG) ;
            if (v & CY_AS_MEM_P0_INTR_REG_MBINT)
                break ;

            CyAsHalSleep (10) ;
            count-- ;
        }

        if (!count)
        {
            retry = 0 ;
            dev_p->stby_int_mask = CyAsHalDisableInterrupts() ;
            CyAsHalSetWakeupPin(dev_p->tag, CyFalse) ;
            CyAsHalSleep (10) ;
            goto TryWakeupAgain ;
        }
    }

    return ret ;
}

CyAsReturnStatus_t
CyAsMiscRegisterCallback(
                         CyAsDeviceHandle               handle,         /* Handle to the West Bridge device */
                         CyAsMiscEventCallback  callback        /* The function to call */
                         )
{
    CyAsDevice *dev_p ;

    CyAsLogDebugMessage(6, "CyAsMiscRegisterCallback called") ;

    /* Make sure we have a valid device */
    dev_p = (CyAsDevice *)handle ;
    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    dev_p->misc_event_cb = callback ;
    return CY_AS_ERROR_SUCCESS ;
}

CyAsReturnStatus_t
CyAsMiscStorageChanged(CyAsDeviceHandle handle,
                         CyAsFunctionCallback   cb,
                         uint32_t client)
{
    CyAsDevice *dev_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyBool standby ;
    CyAsLLRequestResponse *req_p, *reply_p ;

    CyAsLogDebugMessage(6, "CyAsMiscStorageChanged called") ;

    /* Make sure the device is ready for the command. */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /*
    * Make sure Antioch is not in standby
    */
    ret = CyAsMiscInStandby(dev_p, &standby) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    if (standby)
        return CY_AS_ERROR_IN_STANDBY ;

    /*
     * Make sure Westbridge is not in suspend mode.
     */
    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_STORAGE_MEDIA_CHANGED, CY_RQT_GENERAL_RQT_CONTEXT, 0) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

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

        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
        {
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            goto destroy ;
        }

        ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }
    else
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_STORAGECHANGED,
            0, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMiscFuncCallback) ;

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
CyAsMiscEnterSuspend(
        CyAsDeviceHandle     handle,
        CyBool               usb_wakeup_en,
        CyBool               gpio_wakeup_en,
        CyAsFunctionCallback cb,
        uint32_t             client)
{
    CyAsDevice *dev_p ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyBool standby ;
    CyAsLLRequestResponse *req_p, *reply_p ;
    uint16_t value ;
    uint32_t int_state ;

    CyAsLogDebugMessage(6, "CyAsMiscEnterSuspend called") ;

    /*
     * Basic sanity checks to ensure that the device is initialised.
     */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /*
     * Make sure West Bridge is not already in standby
     */
    CyAsMiscInStandby(dev_p, &standby) ;
    if (standby)
        return CY_AS_ERROR_IN_STANDBY ;

    /*
     * Make sure that the device is not already in suspend mode.
     */
    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /*
     * Make sure there is no active USB connection.
     */
    if ((CyAsDeviceIsUsbConnected(dev_p)) && (dev_p->usb_last_event != CyAsEventUsbSuspend))
        return CY_AS_ERROR_USB_CONNECTED ;

    /*
     * Make sure that there are no async requests at this point in time.
     */
    int_state = CyAsHalDisableInterrupts() ;
    if ((dev_p->func_cbs_misc->count) || (dev_p->func_cbs_res->count) ||
            (dev_p->func_cbs_stor->count) || (dev_p->func_cbs_usb->count))
    {
        CyAsHalEnableInterrupts(int_state) ;
        return CY_AS_ERROR_ASYNC_PENDING ;
    }
    CyAsHalEnableInterrupts(int_state) ;

    /* Create the request to send to the Antioch device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_ENTER_SUSPEND_MODE, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;

    /* Reserve space for the reply, the reply data will not exceed one word */
    reply_p = CyAsLLCreateResponse(dev_p, 1) ;
    if (reply_p == 0)
    {
        CyAsLLDestroyRequest(dev_p, req_p) ;
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    }

    /* Wakeup control flags. */
    value = 0x0001 ;
    if (usb_wakeup_en)
        value |= 0x04 ;
    if (gpio_wakeup_en)
        value |= 0x02 ;
    CyAsLLRequestResponse_SetWord(req_p, 0, value) ;

    if (cb != 0)
    {

        ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_ENTERSUSPEND,
            0, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p, reply_p,
            CyAsMiscFuncCallback) ;

        if (ret != CY_AS_ERROR_SUCCESS)
                goto destroy ;

        return CY_AS_ERROR_SUCCESS ;
    }
    else
    {
        ret = CyAsLLSendRequestWaitReply(dev_p, req_p, reply_p) ;
        if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
        else
            ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }

destroy:
    if (ret == CY_AS_ERROR_SUCCESS)
        CyAsDeviceSetSuspendMode(dev_p) ;

    CyAsLLDestroyRequest(dev_p, req_p) ;
    CyAsLLDestroyResponse(dev_p, reply_p) ;

    return ret ;
}

CyAsReturnStatus_t
CyAsMiscLeaveSuspend(
        CyAsDeviceHandle     handle,
        CyAsFunctionCallback cb,
        uint32_t             client)
{
    CyAsDevice *dev_p ;
    uint16_t v, count ;
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;

    CyAsLogDebugMessage(6, "CyAsMiscLeaveSuspend called") ;

    /* Make sure we have a valid device */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /* Make sure we are in suspend mode. */
    if (CyAsDeviceIsInSuspendMode(dev_p))
    {
        if (cb)
        {
            CyAsFuncCBNode *cbnode = CyAsCreateFuncCBNodeData(cb, client, CY_FUNCT_CB_MISC_LEAVESUSPEND, 0) ;
            if (cbnode == 0)
                return CY_AS_ERROR_OUT_OF_MEMORY ;

            CyAsInsertCBNode(dev_p->func_cbs_misc, cbnode) ;
        }

        /*
         * Do a read from the ID register so that the CE assertion will wake West Bridge.
         * The read is repeated until the read comes back with valid data.
         */
        count = 8 ;

        v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_CM_WB_CFG_ID) ;

        while (!IsValidSiliconId(v) && count-- > 0)
        {
            CyAsHalSleep(CY_AS_LEAVE_STANDBY_DELAY_CLOCK) ;
            v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_CM_WB_CFG_ID) ;
        }

        /*
         * If we tried to read the register and could not, return a timeout
         */
        if (count == 0)
            return CY_AS_ERROR_TIMEOUT ;
    }
    else
        return CY_AS_ERROR_NOT_IN_SUSPEND ;

    if (cb == 0)
    {
        /*
         * Wait until the in suspend mode flag is cleared.
         */
        count = 20 ;
        while ((CyAsDeviceIsInSuspendMode(dev_p)) && (count--))
        {
            CyAsHalSleep(CY_AS_LEAVE_STANDBY_DELAY_CLOCK) ;
        }

        if (CyAsDeviceIsInSuspendMode(dev_p))
            ret = CY_AS_ERROR_TIMEOUT ;
    }

    return ret ;
}

CyAsReturnStatus_t
CyAsMiscReserveLNABootArea(CyAsDeviceHandle handle,
                           uint8_t numzones,
                           CyAsFunctionCallback cb,
                           uint32_t client)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    CyBool standby ;
    CyAsLLRequestResponse *req_p, *reply_p ;

    CyAsDevice *dev_p ;

    (void)client ;

    CyAsLogDebugMessage(6, "CyAsMiscSwitchPnandMode called") ;

    /* Make sure we have a valid device */
    dev_p = (CyAsDevice *)handle ;
    CyAsCheckDeviceReady(dev_p) ;

    /*
    * Make sure Antioch is not in standby
    */
    ret = CyAsMiscInStandby(dev_p, &standby) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;
    if (standby)
        return CY_AS_ERROR_IN_STANDBY ;

    /* Make sure the Antioch is not in suspend mode. */
    if (CyAsDeviceIsInSuspendMode(dev_p))
        return CY_AS_ERROR_IN_SUSPEND ;

    /* Create the request to send to the West Bridge device */
    req_p = CyAsLLCreateRequest(dev_p, CY_RQT_RESERVE_LNA_BOOT_AREA, CY_RQT_GENERAL_RQT_CONTEXT, 1) ;
    if (req_p == 0)
            return CY_AS_ERROR_OUT_OF_MEMORY ;
        CyAsLLRequestResponse_SetWord(req_p, 0, (uint16_t)numzones) ;

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

            if (CyAsLLRequestResponse_GetCode(reply_p) != CY_RESP_SUCCESS_FAILURE)
            {
                ret = CY_AS_ERROR_INVALID_RESPONSE ;
                goto destroy ;
            }

            ret = CyAsLLRequestResponse_GetWord(reply_p, 0) ;
    }
    else
    {

            ret = CyAsMiscSendRequest(dev_p, cb, client, CY_FUNCT_CB_MISC_RESERVELNABOOTAREA,
                0, dev_p->func_cbs_misc, CY_AS_REQUEST_RESPONSE_EX, req_p,
                reply_p, CyAsMiscFuncCallback) ;

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

CyAsFuncCBNode*
CyAsCreateFuncCBNodeData(CyAsFunctionCallback cb,
                     uint32_t client,
                     CyAsFunctCBType type,
                     void* data)
{
    uint32_t state = CyAsHalDisableInterrupts() ;
    CyAsFuncCBNode* node = (CyAsFuncCBNode*)CyAsHalCBAlloc(sizeof(CyAsFuncCBNode)) ;
    CyAsHalEnableInterrupts(state) ;
    if(node != 0)
    {
        node->nodeType = CYAS_FUNC_CB ;
        node->cb_p = cb ;
        node->client_data = client ;
        node->dataType = type ;
        if(data != 0)
            node->dataType |= CY_FUNCT_CB_DATA ;
        else
            node->dataType |= CY_FUNCT_CB_NODATA ;
        node->data = data ;
        node->next_p = 0 ;
    }
    return node ;
}

CyAsFuncCBNode*
CyAsCreateFuncCBNode(CyAsFunctionCallback cb,
                     uint32_t client)
{
    return CyAsCreateFuncCBNodeData(cb, client, CY_FUNCT_CB_NODATA, 0) ;
}

void
CyAsDestroyFuncCBNode(CyAsFuncCBNode* node)
{
    uint32_t state ;

    node->nodeType = CYAS_INVALID ;
    state = CyAsHalDisableInterrupts() ;
    CyAsHalCBFree(node) ;
    CyAsHalEnableInterrupts(state) ;
}

CyAsUsbFuncCBNode*
CyAsCreateUsbFuncCBNode(CyAsUsbFunctionCallback cb, uint32_t client)
{
    uint32_t state = CyAsHalDisableInterrupts() ;
    CyAsUsbFuncCBNode * node = (CyAsUsbFuncCBNode *)CyAsHalCBAlloc(sizeof(CyAsUsbFuncCBNode)) ;
    CyAsHalEnableInterrupts(state) ;
    if(node != 0)
    {
        node->type = CYAS_USB_FUNC_CB ;
        node->cb_p = cb ;
        node->client_data = client ;
        node->next_p = 0 ;
    }
    return node ;
}

void
CyAsDestroyUsbFuncCBNode(CyAsUsbFuncCBNode* node)
{
    uint32_t state ;

    node->type = CYAS_INVALID ;
    state = CyAsHalDisableInterrupts() ;
    CyAsHalCBFree(node) ;
    CyAsHalEnableInterrupts(state) ;
}

CyAsUsbIoCBNode*
CyAsCreateUsbIoCBNode(CyAsUsbIoCallback cb)
{
    uint32_t state = CyAsHalDisableInterrupts() ;
    CyAsUsbIoCBNode * node = (CyAsUsbIoCBNode *)CyAsHalCBAlloc(sizeof(CyAsUsbIoCBNode)) ;
    CyAsHalEnableInterrupts(state) ;
    if(node != 0)
    {
        node->type = CYAS_USB_IO_CB ;
        node->cb_p = cb ;
        node->next_p = 0 ;
    }
    return node ;
}

void
CyAsDestroyUsbIoCBNode(CyAsUsbIoCBNode* node)
{
    uint32_t state ;

    node->type = CYAS_INVALID ;

    state = CyAsHalDisableInterrupts() ;
    CyAsHalCBFree(node) ;
    CyAsHalEnableInterrupts(state) ;
}

CyAsStorageIoCBNode*
CyAsCreateStorageIoCBNode(CyAsStorageCallback cb, CyAsMediaType media, uint32_t device_index,
                          uint32_t unit, uint32_t block_addr, CyAsOperType oper,
                          CyAsLLRequestResponse* req_p, CyAsLLRequestResponse* reply_p)
{
    uint32_t state = CyAsHalDisableInterrupts() ;
    CyAsStorageIoCBNode * node = (CyAsStorageIoCBNode *)CyAsHalCBAlloc(sizeof(CyAsStorageIoCBNode)) ;
    CyAsHalEnableInterrupts(state) ;
    if(node != 0)
    {
        node->type = CYAS_STORAGE_IO_CB ;
        node->cb_p = cb ;
        node->media = media ;
        node->device_index = device_index ;
        node->unit = unit ;
        node->block_addr = block_addr ;
        node->oper = oper ;
        node->req_p = req_p ;
        node->reply_p = reply_p ;
        node->next_p = 0 ;
    }
    return node ;
}

void
CyAsDestroyStorageIoCBNode(CyAsStorageIoCBNode* node)
{
    uint32_t state ;
    node->type = CYAS_INVALID ;
    state = CyAsHalDisableInterrupts() ;
    CyAsHalCBFree(node) ;
    CyAsHalEnableInterrupts(state) ;
}

CyAsCBQueue *
CyAsCreateCBQueue(CyAsCBNodeType type)
{
    uint32_t state = CyAsHalDisableInterrupts() ;
    CyAsCBQueue * queue = (CyAsCBQueue *)CyAsHalCBAlloc(sizeof(CyAsCBQueue)) ;
    CyAsHalEnableInterrupts(state) ;
    if(queue)
    {
        queue->type = type ;
        queue->head_p = 0 ;
        queue->tail_p = 0 ;
        queue->count = 0 ;
    }

    return queue ;
}

void
CyAsDestroyCBQueue(CyAsCBQueue* queue)
{
    uint32_t state ;
    queue->type = CYAS_INVALID ;
    queue->head_p = 0 ;
    queue->tail_p = 0 ;
    queue->count = 0 ;
    state = CyAsHalDisableInterrupts() ;
    CyAsHalCBFree(queue) ;
    CyAsHalEnableInterrupts(state) ;
}

/* Inserts a CyAsCBNode into the queue, the node type must match the queue type*/
void
CyAsInsertCBNode(CyAsCBQueue * queue_p, void* cbnode)
{
    uint32_t int_state ;

    int_state = CyAsHalDisableInterrupts() ;

    CyAsHalAssert(queue_p != 0) ;

    switch (queue_p->type)
    {
        case CYAS_USB_FUNC_CB:
            {
                CyAsUsbFuncCBNode* node = (CyAsUsbFuncCBNode*)cbnode ;
                CyAsUsbFuncCBNode* tail = (CyAsUsbFuncCBNode*)queue_p->tail_p ;

                CyAsHalAssert(node->type == CYAS_USB_FUNC_CB) ;
                CyAsHalAssert(tail == 0 || tail->type == CYAS_USB_FUNC_CB) ;
                if(queue_p->head_p == 0)
                    queue_p->head_p = node ;
                else
                    tail->next_p = node ;

                queue_p->tail_p = node ;
            }
            break ;

        case CYAS_USB_IO_CB:
            {
                CyAsUsbIoCBNode* node = (CyAsUsbIoCBNode*)cbnode ;
                CyAsUsbIoCBNode* tail = (CyAsUsbIoCBNode*)queue_p->tail_p ;

                CyAsHalAssert(node->type == CYAS_USB_IO_CB) ;
                CyAsHalAssert(tail == 0 || tail->type == CYAS_USB_IO_CB) ;
                if(queue_p->head_p == 0)
                    queue_p->head_p = node ;
                else
                    tail->next_p = node ;

                queue_p->tail_p = node ;
            }
            break ;

        case CYAS_STORAGE_IO_CB:
            {
                CyAsStorageIoCBNode* node = (CyAsStorageIoCBNode*)cbnode ;
                CyAsStorageIoCBNode* tail = (CyAsStorageIoCBNode*)queue_p->tail_p ;

                CyAsHalAssert(node->type == CYAS_STORAGE_IO_CB) ;
                CyAsHalAssert(tail == 0 || tail->type == CYAS_STORAGE_IO_CB) ;
                if(queue_p->head_p == 0)
                    queue_p->head_p = node ;
                else
                    tail->next_p = node ;

                queue_p->tail_p = node ;
            }
            break ;

        case CYAS_FUNC_CB:
            {
                CyAsFuncCBNode* node = (CyAsFuncCBNode*)cbnode ;
                CyAsFuncCBNode* tail = (CyAsFuncCBNode*)queue_p->tail_p ;

                CyAsHalAssert(node->nodeType == CYAS_FUNC_CB) ;
                CyAsHalAssert(tail == 0 || tail->nodeType == CYAS_FUNC_CB) ;
                if(queue_p->head_p == 0)
                    queue_p->head_p = node ;
                else
                    tail->next_p = node ;

                queue_p->tail_p = node ;
            }
            break ;

        default:
            CyAsHalAssert(CyFalse) ;
            break ;
    }

    queue_p->count++ ;

    CyAsHalEnableInterrupts(int_state) ;
}

/* Removes the tail node from the queue and frees it */
void
CyAsRemoveCBTailNode(CyAsCBQueue *queue_p)
{
    uint32_t int_state ;

    int_state = CyAsHalDisableInterrupts () ;

    if (queue_p->count > 0)
    {
        /*
         * The worst case length of the queue should be under 10 elements, and the average
         * case should be just 1 element. So, we just employ a linear search to find the node
         * to be freed.
         */
        switch (queue_p->type)
        {
            case CYAS_FUNC_CB:
                {
                    CyAsFuncCBNode* node = (CyAsFuncCBNode*)queue_p->head_p ;
                    CyAsFuncCBNode* tail = (CyAsFuncCBNode*)queue_p->tail_p ;
                    if (node != tail)
                    {
                        while (node->next_p != tail)
                            node = node->next_p ;
                        node->next_p = 0 ;
                        queue_p->tail_p = node ;
                    }
                    CyAsDestroyFuncCBNode (tail) ;
                }
                break ;

            case CYAS_USB_FUNC_CB:
                {
                    CyAsUsbFuncCBNode* node = (CyAsUsbFuncCBNode*)queue_p->head_p ;
                    CyAsUsbFuncCBNode* tail = (CyAsUsbFuncCBNode*)queue_p->tail_p ;
                    if (node != tail)
                    {
                        while (node->next_p != tail)
                            node = node->next_p ;
                        node->next_p = 0 ;
                        queue_p->tail_p = node ;
                    }

                    CyAsDestroyUsbFuncCBNode (tail) ;
                }
                break ;

            case CYAS_USB_IO_CB:
                {
                    CyAsUsbIoCBNode* node = (CyAsUsbIoCBNode*)queue_p->head_p ;
                    CyAsUsbIoCBNode* tail = (CyAsUsbIoCBNode*)queue_p->tail_p ;
                    if (node != tail)
                    {
                        while (node->next_p != tail)
                            node = node->next_p ;
                        node->next_p = 0 ;
                        queue_p->tail_p = node ;
                    }
                    CyAsDestroyUsbIoCBNode (tail) ;
                }
                break ;

            case CYAS_STORAGE_IO_CB:
                {
                    CyAsStorageIoCBNode* node = (CyAsStorageIoCBNode*)queue_p->head_p ;
                    CyAsStorageIoCBNode* tail = (CyAsStorageIoCBNode*)queue_p->tail_p ;
                    if (node != tail)
                    {
                        while (node->next_p != tail)
                            node = node->next_p ;
                        node->next_p = 0 ;
                        queue_p->tail_p = node ;
                    }
                    CyAsDestroyStorageIoCBNode (tail) ;
                }
                break ;

            default:
                CyAsHalAssert (CyFalse) ;
        }

        queue_p->count-- ;
        if (queue_p->count == 0)
        {
            queue_p->head_p = 0 ;
            queue_p->tail_p = 0 ;
        }
    }

    CyAsHalEnableInterrupts (int_state) ;
}

/* Removes the first CyAsCBNode from the queue and frees it */
void
CyAsRemoveCBNode(CyAsCBQueue * queue_p)
{
    uint32_t int_state ;

    int_state = CyAsHalDisableInterrupts() ;

    CyAsHalAssert(queue_p->count >= 0) ;
    if(queue_p->count > 0)
    {
        if(queue_p->type == CYAS_USB_FUNC_CB)
        {
            CyAsUsbFuncCBNode* node = (CyAsUsbFuncCBNode*)queue_p->head_p ;
            queue_p->head_p = node->next_p ;
            CyAsDestroyUsbFuncCBNode(node) ;
        }
        else if(queue_p->type == CYAS_USB_IO_CB)
        {
            CyAsUsbIoCBNode* node = (CyAsUsbIoCBNode*)queue_p->head_p ;
            queue_p->head_p = node->next_p ;
            CyAsDestroyUsbIoCBNode(node) ;
        }
        else if(queue_p->type == CYAS_STORAGE_IO_CB)
        {
            CyAsStorageIoCBNode* node = (CyAsStorageIoCBNode*)queue_p->head_p ;
            queue_p->head_p = node->next_p ;
            CyAsDestroyStorageIoCBNode(node) ;
        }
        else if(queue_p->type == CYAS_FUNC_CB)
        {
            CyAsFuncCBNode* node = (CyAsFuncCBNode*)queue_p->head_p ;
            queue_p->head_p = node->next_p ;
            CyAsDestroyFuncCBNode(node) ;
        }
        else
        {
            CyAsHalAssert(CyFalse) ;
        }

        queue_p->count-- ;
        if(queue_p->count == 0)
        {
            queue_p->head_p = 0 ;
            queue_p->tail_p = 0 ;
        }
    }

    CyAsHalEnableInterrupts(int_state) ;
}

void MyPrintFuncCBNode(CyAsFuncCBNode* node)
{
    CyAsHalPrintMessage("[cd:%2u dt:%2u cb:0x%08x d:0x%08x nt:%1i]",
        node->client_data, CyAsFunctCBTypeGetType(node->dataType), (uint32_t)node->cb_p, (uint32_t)node->data, node->nodeType) ;
}

void MyPrintCBQueue(CyAsCBQueue* queue_p)
{
    uint32_t i = 0 ;

    CyAsHalPrintMessage("| Count: %u Type: ", queue_p->count) ;

    if(queue_p->type == CYAS_USB_FUNC_CB)
    {
        CyAsHalPrintMessage("USB_FUNC_CB\n") ;
    }
    else if(queue_p->type == CYAS_USB_IO_CB)
    {
        CyAsHalPrintMessage("USB_IO_CB\n") ;
    }
    else if(queue_p->type == CYAS_STORAGE_IO_CB)
    {
        CyAsHalPrintMessage("STORAGE_IO_CB\n") ;
    }
    else if(queue_p->type == CYAS_FUNC_CB)
    {
        CyAsFuncCBNode* node = (CyAsFuncCBNode*)queue_p->head_p ;
        CyAsHalPrintMessage("FUNC_CB\n") ;
        if(queue_p->count > 0)
        {
            CyAsHalPrintMessage("| Head->") ;

            for(i = 0; i < queue_p->count; i++)
            {
                if(node)
                {
                    CyAsHalPrintMessage("->") ;
                    MyPrintFuncCBNode(node) ;
                    node = node->next_p ;
                }
                else
                    CyAsHalPrintMessage("->[NULL]\n") ;
            }

            CyAsHalPrintMessage("\n| Tail->") ;
            MyPrintFuncCBNode((CyAsFuncCBNode*)queue_p->tail_p) ;
            CyAsHalPrintMessage("\n") ;
        }
    }
    else
    {
        CyAsHalPrintMessage("INVALID\n") ;
    }

    CyAsHalPrintMessage("|----------\n") ;
}


/* Removes and frees all pending callbacks */
void
CyAsClearCBQueue(CyAsCBQueue * queue_p)
{
    uint32_t int_state = CyAsHalDisableInterrupts() ;

    while(queue_p->count != 0)
        CyAsRemoveCBNode(queue_p) ;

    CyAsHalEnableInterrupts(int_state) ;
}

CyAsReturnStatus_t
CyAsMiscSendRequest(CyAsDevice* dev_p,
                      CyAsFunctionCallback cb,
                      uint32_t client,
                      CyAsFunctCBType type,
                      void* data,
                      CyAsCBQueue* queue,
                      uint16_t req_type,
                      CyAsLLRequestResponse *req_p,
                      CyAsLLRequestResponse *reply_p,
                      CyAsResponseCallback rcb)
{

    CyAsFuncCBNode* cbnode = CyAsCreateFuncCBNodeData(cb, client, type, data) ;
    CyAsReturnStatus_t ret ;

    if(cbnode == 0)
        return CY_AS_ERROR_OUT_OF_MEMORY ;
    else
        CyAsInsertCBNode(queue, cbnode) ;

    req_p->flags |= req_type ;

    ret = CyAsLLSendRequest(dev_p, req_p, reply_p, CyFalse, rcb) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsRemoveCBTailNode(queue) ;
    }

    return ret ;
}

void
CyAsMiscCancelExRequests(CyAsDevice *dev_p)
{
    int i ;
    for(i = 0; i < CY_RQT_CONTEXT_COUNT; i++)
        CyAsLLRemoveAllRequests(dev_p, dev_p->context[i]) ;
}


static void
CyAsMiscFuncCallback(CyAsDevice *dev_p,
                        uint8_t context,
                        CyAsLLRequestResponse *rqt,
                        CyAsLLRequestResponse *resp,
                        CyAsReturnStatus_t stat)
{
    CyAsFuncCBNode* node = NULL ;
    CyAsReturnStatus_t ret ;

    CyBool              exRequest = (rqt->flags & CY_AS_REQUEST_RESPONSE_EX) == CY_AS_REQUEST_RESPONSE_EX ;
    CyBool              msRequest = (rqt->flags & CY_AS_REQUEST_RESPONSE_MS) == CY_AS_REQUEST_RESPONSE_MS ;
    uint8_t             code ;
    uint32_t            type ;
    uint8_t             cntxt ;

    CyAsHalAssert(exRequest || msRequest) ;
    (void) exRequest;
    (void) msRequest;
    (void)context ;

    cntxt = CyAsLLRequestResponse_GetContext(rqt) ;
    code = CyAsLLRequestResponse_GetCode(rqt) ;

    switch(cntxt)
    {
    case CY_RQT_GENERAL_RQT_CONTEXT:
        CyAsHalAssert(dev_p->func_cbs_misc->count != 0) ;
        CyAsHalAssert(dev_p->func_cbs_misc->type == CYAS_FUNC_CB) ;
        node = (CyAsFuncCBNode*)dev_p->func_cbs_misc->head_p ;
        type = CyAsFunctCBTypeGetType(node->dataType) ;

        switch(code)
        {
        case CY_RQT_GET_FIRMWARE_VERSION:
            CyAsHalAssert(node->data != 0) ;
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_GETFIRMWAREVERSION) ;
            ret = MyHandleResponseGetFirmwareVersion(dev_p, rqt, resp, (CyAsGetFirmwareVersionData*)node->data) ;
            break ;
        case CY_RQT_READ_MCU_REGISTER:
            CyAsHalAssert(node->data != 0) ;
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_READMCUREGISTER) ;
            ret = MyHandleResponseReadMCURegister(dev_p, rqt, resp, (uint8_t*)node->data) ;
            break ;
        case CY_RQT_GET_GPIO_STATE:
            CyAsHalAssert(node->data != 0) ;
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_GETGPIOVALUE) ;
            ret = MyHandleResponseGetGpioValue(dev_p, rqt, resp, (uint8_t*)node->data) ;
            break ;
        case CY_RQT_SET_SD_CLOCK_FREQ:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_SETSDFREQ) ;
            ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
            break ;
        case CY_RQT_CONTROL_ANTIOCH_HEARTBEAT:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_HEARTBEATCONTROL) ;
            ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
            break ;
        case CY_RQT_WRITE_MCU_REGISTER:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_WRITEMCUREGISTER) ;
            ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
            break ;
        case CY_RQT_STORAGE_MEDIA_CHANGED:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_STORAGECHANGED) ;
            ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
            break ;
        case CY_RQT_SET_GPIO_STATE:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_SETGPIOVALUE) ;
            ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
            break ;
        case CY_RQT_SET_TRACE_LEVEL:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_SETTRACELEVEL) ;
            ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
            if (ret == CY_AS_ERROR_INVALID_RESPONSE)
                ret = CY_AS_ERROR_NOT_SUPPORTED ;
            break ;
        case CY_RQT_PREPARE_FOR_STANDBY:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_ENTERSTANDBY) ;
            ret = MyHandleResponseEnterStandby(dev_p, rqt, resp, (CyBool)node->data) ;
            break ;
        case CY_RQT_ENTER_SUSPEND_MODE:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_ENTERSUSPEND) ;
            ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
            if (ret == CY_AS_ERROR_SUCCESS)
            {
                CyAsDeviceSetSuspendMode(dev_p) ;
            }
            break ;
    case CY_RQT_RESERVE_LNA_BOOT_AREA:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_RESERVELNABOOTAREA) ;
            ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
            break ;
    case CY_RQT_SDPOLARITY:
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_SETSDPOLARITY) ;
            ret = MyHandleResponseNoData(dev_p, rqt, resp) ;
            break ;


    default:
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            CyAsHalAssert(CyFalse) ;
            break ;
        }
        break ;

    case CY_RQT_RESOURCE_RQT_CONTEXT:
        CyAsHalAssert(dev_p->func_cbs_res->count != 0) ;
        CyAsHalAssert(dev_p->func_cbs_res->type == CYAS_FUNC_CB) ;
        node = (CyAsFuncCBNode*)dev_p->func_cbs_res->head_p ;
        type = CyAsFunctCBTypeGetType(node->dataType) ;

        switch(code)
        {
        case CY_RQT_ACQUIRE_RESOURCE:
            /* The node->data field is actually an enum value which could be
            0, thus no assert is done */
            CyAsHalAssert(type == CY_FUNCT_CB_MISC_ACQUIRERESOURCE) ;
            ret = MyHandleResponseAcquireResource(dev_p, rqt, resp, (CyAsResourceType*)node->data) ;
            break ;
        default:
            ret = CY_AS_ERROR_INVALID_RESPONSE ;
            CyAsHalAssert(CyFalse) ;
            break ;
        }
        break ;

    default:
        ret = CY_AS_ERROR_INVALID_RESPONSE ;
        CyAsHalAssert(CyFalse) ;
        break ;
    }
    (void) type;
    /*
     * If the low level layer returns a direct error, use the corresponding error code.
     * If not, use the error code based on the response from firmware.
     */
    if (stat == CY_AS_ERROR_SUCCESS)
        stat = ret ;

    /* Call the user Callback */
    node->cb_p((CyAsDeviceHandle)dev_p, stat, node->client_data, (CyAsFunctCBType)node->dataType, node->data) ;
    if(cntxt == CY_RQT_GENERAL_RQT_CONTEXT)
        CyAsRemoveCBNode(dev_p->func_cbs_misc) ;
    else
        CyAsRemoveCBNode(dev_p->func_cbs_res) ;
}


/* This includes the implementation of the deprecated functions for backward
 * compatibility
 */
#include "cyasmisc_dep_impl.h"

/*[]*/

