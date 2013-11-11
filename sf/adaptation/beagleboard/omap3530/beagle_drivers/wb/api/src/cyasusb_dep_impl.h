/* Cypress West Bridge API source file (cyasusb_dep_impl.h)
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

/* This c file will contain Antioch specific implementation
 * of the APIs that are deprecated in Astoria SDK. This is
 * for maintaining backward compatibility
 */

#include "cyasusb.h"
/*
* This function registers a callback to be called when USB events are processed
*/
CyAsReturnStatus_t
CyAsUsbRegisterCallback_dep(CyAsDeviceHandle handle, CyAsUsbEventCallback_dep callback)
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

    dev_p->usb_event_cb = callback ;
    dev_p->usb_event_cb_ms = NULL ;
    return CY_AS_ERROR_SUCCESS ;
}

/*
* This method sets how the USB is enumerated and should be called before the
* CyAsUsbConnect() is called.
*/
CyAsReturnStatus_t 
CyAsUsbSetEnumConfig_dep(CyAsDeviceHandle handle, 
                       CyAsUsbEnumControl_dep *config_p,
                       CyAsFunctionCallback cb,
                       uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    uint8_t enum_bus = 0 ;
    uint8_t bus ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (!CyAsDeviceIsConfigured(dev_p))
        return CY_AS_ERROR_NOT_CONFIGURED ;

    if (!CyAsDeviceIsFirmwareLoaded(dev_p))
        return CY_AS_ERROR_NO_FIRMWARE ;

    /*
     * Map the storage devices to be enumerated to the storage
     * buses to be enumerated.
     */
    for (bus = 0; bus < CY_AS_MAX_BUSES; bus++)
    {
        if (config_p->enum_mass_storage & dev_p->media_supported[bus])
            enum_bus |= (0x01 << bus) ;
    }

    return MyUsbSetEnumConfig(dev_p, enum_bus,
             config_p->enum_mass_storage,
             config_p->antioch_enumeration,
            config_p->mass_storage_interface,
            0,
            config_p->mass_storage_callbacks,
            cb,
            client
        ) ;
}

/*
* This method returns the enumerateion configuration information from the Antioch device.  Generally this is not
* used by client software but is provided mostly for debug information.  We want a method to read all state information
* from the Antioch device.
*/
CyAsReturnStatus_t
CyAsUsbGetEnumConfig_dep(CyAsDeviceHandle handle, 
                       CyAsUsbEnumControl_dep *config_p,
                       CyAsFunctionCallback cb,
                       uint32_t client)
{
    return MyUsbGetEnumConfig(handle, CY_AS_REQUEST_RESPONSE_EX, config_p, cb, client);
}


CyAsReturnStatus_t
CyAsUsbGetDescriptor_dep(CyAsDeviceHandle handle, 
                       CyAsUsbDescType type, 
                       uint8_t index, 
                       void *desc_p, 
                       uint32_t *length_p)
{
    CyAsGetDescriptorData data ;
    CyAsReturnStatus_t status;

    data.desc_p = desc_p;
    data.length = *length_p;
    status = CyAsUsbGetDescriptor(handle, type, index, &data, 0, 0) ;
    *length_p = data.length ;

    return status ;
}


CyAsReturnStatus_t
CyAsUsbSetNak_dep(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, CyAsUsbFunctionCallback cb, uint32_t client)
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
    return CyAsUsbNakStallRequest(handle, ep, CY_RQT_ENDPOINT_SET_NAK, CyTrue, cb, 0, client) ;
}

CyAsReturnStatus_t 
CyAsUsbClearNak_dep(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, CyAsUsbFunctionCallback cb, uint32_t client)
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
    return CyAsUsbNakStallRequest(handle, ep, CY_RQT_ENDPOINT_SET_NAK, CyFalse, cb, 0, client) ;
}

CyAsReturnStatus_t
CyAsUsbSetStall_dep(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, CyAsUsbFunctionCallback cb, uint32_t client)
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

    return CyAsUsbNakStallRequest(handle, ep, CY_RQT_STALL_ENDPOINT, CyTrue, cb, 0, client) ;
}

CyAsReturnStatus_t 
CyAsUsbClearStall_dep(CyAsDeviceHandle handle, CyAsEndPointNumber_t ep, CyAsUsbFunctionCallback cb, uint32_t client)
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

    return CyAsUsbNakStallRequest(handle, ep, CY_RQT_STALL_ENDPOINT, CyFalse, cb, 0, client) ;
}

extern CyAsReturnStatus_t
CyAnMapBusFromMediaType (
        CyAsDevice      *dev_p,
        CyAsMediaType    type,
        CyAsBusNumber_t *bus) ;

CyAsReturnStatus_t
CyAsUsbSelectMSPartitions_dep (
        CyAsDeviceHandle        handle,
        CyAsMediaType           media,
        uint32_t                device,
        CyAsUsbMSType_t         type,
        CyAsFunctionCallback    cb,
        uint32_t                client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsBusNumber_t bus ;
    CyAsReturnStatus_t ret ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    ret = CyAnMapBusFromMediaType(dev_p, media, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return CyAsUsbSelectMSPartitions(handle, bus, device, type, cb, client) ;
}

/*[]*/
