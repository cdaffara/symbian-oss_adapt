/* Cypress West Bridge API source file (cyasstorage_dep_impl.h)
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
CyAsReturnStatus_t
CyAsStorageRegisterCallback_dep(CyAsDeviceHandle handle, CyAsStorageEventCallback_dep callback)
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

    dev_p->storage_event_cb = callback ;
    dev_p->storage_event_cb_ms = NULL ;

    return CY_AS_ERROR_SUCCESS ;
}


CyAsReturnStatus_t
CyAsStorageClaim_dep_EX(CyAsDeviceHandle handle,
                   CyAsMediaType* type,
                   CyAsFunctionCallback cb,
                   uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsReturnStatus_t ret ;
    CyAsBusNumber_t bus ;

    if (*type == CyAsMediaSDIO)
        return CY_AS_ERROR_SUCCESS;

    if (*type != CyAsMediaNand && *type != CyAsMediaSDFlash && *type != CyAsMediaMMCFlash && *type != CyAsMediaCEATA)
        return CY_AS_ERROR_NO_SUCH_MEDIA ;

    ret = CyAnMapBusFromMediaType(dev_p, *type, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return MyStorageClaim(dev_p, type, bus, 0, CY_AS_REQUEST_RESPONSE_EX, cb, client) ;
}


CyAsReturnStatus_t
CyAsStorageRelease_dep_EX(CyAsDeviceHandle handle,
                     CyAsMediaType* type,
                     CyAsFunctionCallback cb,
                     uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsBusNumber_t bus ;
    CyAsReturnStatus_t ret ;

    if (*type != CyAsMediaNand && *type != CyAsMediaSDFlash && *type != CyAsMediaMMCFlash && *type != CyAsMediaCEATA)
        return CY_AS_ERROR_NO_SUCH_MEDIA ;

    ret = CyAnMapBusFromMediaType(dev_p, *type, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return MyStorageRelease(dev_p, type, bus, 0, CY_AS_REQUEST_RESPONSE_EX, cb, client) ;
}


CyAsReturnStatus_t
CyAsStorageQueryDevice_dep_EX(CyAsDeviceHandle handle,
                         CyAsStorageQueryDeviceData_dep* data_p,
                         CyAsFunctionCallback cb,
                         uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsBusNumber_t bus ;
    CyAsReturnStatus_t ret ;

    ret = CyAnMapBusFromMediaType(dev_p, data_p->type, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return MyStorageQueryDevice(dev_p, data_p, CY_AS_REQUEST_RESPONSE_EX, bus, data_p->device, cb, client) ;
}

CyAsReturnStatus_t
CyAsStorageQueryUnit_dep_EX(CyAsDeviceHandle handle,
                       CyAsStorageQueryUnitData_dep* data_p,
                       CyAsFunctionCallback cb,
                       uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsBusNumber_t bus;
    CyAsReturnStatus_t ret ;

    ret = CyAnMapBusFromMediaType(dev_p, data_p->type, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return MyStorageQueryUnit(dev_p, data_p, CY_AS_REQUEST_RESPONSE_EX, bus,
            data_p->device, data_p->unit, cb, client) ;
}


CyAsReturnStatus_t
CyAsStorageRead_dep(CyAsDeviceHandle handle, CyAsMediaType media, uint32_t device, uint32_t unit, uint32_t block,
                void *data_p, uint16_t num_blocks)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsBusNumber_t bus ;
    CyAsReturnStatus_t ret ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (media != CyAsMediaNand && media != CyAsMediaSDFlash && media != CyAsMediaMMCFlash && media != CyAsMediaCEATA)
        return CY_AS_ERROR_NO_SUCH_MEDIA ;

    ret = CyAnMapBusFromMediaType(dev_p, media, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return CyAsStorageSyncOper(dev_p, dev_p->storage_read_endpoint, CY_RQT_READ_BLOCK, bus, device,
        unit, block, data_p, num_blocks) ;
}

CyAsReturnStatus_t
CyAsStorageWrite_dep(CyAsDeviceHandle handle, CyAsMediaType media, uint32_t device, uint32_t unit, uint32_t block,
                 void *data_p, uint16_t num_blocks)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsBusNumber_t bus ;
    CyAsReturnStatus_t ret ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (media != CyAsMediaNand && media != CyAsMediaSDFlash && media != CyAsMediaMMCFlash && media != CyAsMediaCEATA)
        return CY_AS_ERROR_NO_SUCH_MEDIA ;

    ret = CyAnMapBusFromMediaType(dev_p, media, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return CyAsStorageSyncOper(dev_p, dev_p->storage_write_endpoint, CY_RQT_WRITE_BLOCK, bus, device,
        unit, block, data_p, num_blocks) ;
}

CyAsReturnStatus_t
CyAsStorageReadAsync_dep(CyAsDeviceHandle handle, CyAsMediaType media, uint32_t device, uint32_t unit,
                     uint32_t block, void *data_p, uint16_t num_blocks, CyAsStorageCallback_dep callback)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsBusNumber_t bus ;
    CyAsReturnStatus_t ret ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (media != CyAsMediaNand && media != CyAsMediaSDFlash && media != CyAsMediaMMCFlash && media != CyAsMediaCEATA)
        return CY_AS_ERROR_NO_SUCH_MEDIA ;

    if (callback == 0)
        return CY_AS_ERROR_NULL_CALLBACK ;

    ret = CyAnMapBusFromMediaType(dev_p, media, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return CyAsStorageAsyncOper(dev_p, dev_p->storage_read_endpoint, CY_RQT_READ_BLOCK,
        CY_AS_REQUEST_RESPONSE_EX, bus, device, unit, block, data_p, num_blocks, callback, NULL);
}

CyAsReturnStatus_t
CyAsStorageWriteAsync_dep(CyAsDeviceHandle handle, CyAsMediaType media, uint32_t device, uint32_t unit,
                      uint32_t block, void *data_p, uint16_t num_blocks, CyAsStorageCallback_dep callback)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsBusNumber_t bus ;
    CyAsReturnStatus_t ret ;

    if (!dev_p ||(dev_p->sig != CY_AS_DEVICE_HANDLE_SIGNATURE))
        return CY_AS_ERROR_INVALID_HANDLE ;

    if (media != CyAsMediaNand && media != CyAsMediaSDFlash && media != CyAsMediaMMCFlash && media != CyAsMediaCEATA)
        return CY_AS_ERROR_NO_SUCH_MEDIA ;

    if (callback == 0)
        return CY_AS_ERROR_NULL_CALLBACK ;

    ret = CyAnMapBusFromMediaType(dev_p, media, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return CyAsStorageAsyncOper(dev_p, dev_p->storage_write_endpoint, CY_RQT_WRITE_BLOCK,
        CY_AS_REQUEST_RESPONSE_EX, bus, device, unit, block, data_p, num_blocks, callback, NULL);
}

CyAsReturnStatus_t
CyAsStorageSDRegisterRead_dep_EX(
        CyAsDeviceHandle          handle,
        CyAsMediaType             type,
        uint8_t                   device,
        CyAsSDCardRegType         regType,
        CyAsStorageSDRegReadData *data_p,
        CyAsFunctionCallback      cb,
        uint32_t                  client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsBusNumber_t bus;
    CyAsReturnStatus_t ret ;

    if (type != CyAsMediaSDFlash && type != CyAsMediaMMCFlash)
        return CY_AS_ERROR_INVALID_PARAMETER ;

    ret = CyAnMapBusFromMediaType(dev_p, type, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return CyAsStorageSDRegisterRead(handle, bus, device, regType, data_p, cb, client);
}


CyAsReturnStatus_t
CyAsStorageSDRegisterRead_dep(
        CyAsDeviceHandle        handle,
        CyAsMediaType           type,
        uint8_t                 device,
        CyAsSDCardRegType       regType,
        uint8_t                 readLen,
        uint8_t                 *data_p)
{
    CyAsStorageSDRegReadData info ;
    CyAsReturnStatus_t ret ;

    info.buf_p = data_p ;
    info.length = readLen ;

    ret = CyAsStorageSDRegisterRead_dep_EX(handle, type, device, regType, &info, 0, 0) ;
    return ret ;
}

CyAsReturnStatus_t
CyAsStorageRelease_dep(CyAsDeviceHandle handle,
                     CyAsMediaType type)
{
    CyAsMediaType* t = &type ;
    return CyAsStorageRelease_dep_EX(handle, t, 0, 0) ;
}

CyAsReturnStatus_t
CyAsStorageQueryUnit_dep(CyAsDeviceHandle handle,
                       CyAsMediaType media,
                       uint32_t device,
                       uint32_t unit,
                       CyAsUnitDesc *desc_p)
{
    CyAsReturnStatus_t status ;
    CyAsStorageQueryUnitData_dep data ;
    data.device = device ;
    data.type = media ;
    data.unit = unit ;

    status = CyAsStorageQueryUnit_dep_EX(handle, &data, 0, 0) ;

    desc_p->block_size = data.desc_p.block_size ;
    desc_p->start_block = data.desc_p.start_block ;
    desc_p->type = data.desc_p.type ;
    desc_p->unit_size = data.desc_p.unit_size ;

    return status ;
}

CyAsReturnStatus_t
CyAsStorageQueryDevice_dep(
                        CyAsDeviceHandle handle,
                         CyAsMediaType media,
                         uint32_t device,
                         CyAsDeviceDesc *desc_p)
{
    CyAsReturnStatus_t status ;
    CyAsStorageQueryDeviceData_dep data ;
    data.device = device ;
    data.type = media ;

    status = CyAsStorageQueryDevice_dep_EX(handle, &data, 0, 0) ;

    desc_p->block_size = data.desc_p.block_size ;
    desc_p->number_units = data.desc_p.number_units ;
    desc_p->removable = data.desc_p.removable ;
    desc_p->type = data.desc_p.type ;
    desc_p->writeable = data.desc_p.writeable ;
    desc_p->locked = data.desc_p.locked ;
    desc_p->erase_unit_size = data.desc_p.erase_unit_size ;

    return status ;
}

CyAsReturnStatus_t
CyAsStorageClaim_dep(CyAsDeviceHandle handle,
                   CyAsMediaType type)
{
    CyAsMediaType* t = &type ;

    return CyAsStorageClaim_dep_EX(handle, t, 0, 0) ;
}

CyAsReturnStatus_t
CyAsStorageDeviceControl_dep(CyAsDeviceHandle handle,
                           CyBool card_detect_en,
                           CyBool write_prot_en,
                           CyAsFunctionCallback cb,
                           uint32_t client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    return MyStorageDeviceControl(dev_p, CyAsBus_1, 0, card_detect_en, write_prot_en, CyAsStorageDetect_GPIO, cb, client);
}

CyAsReturnStatus_t
CyAsStorageCreatePPartition_dep(
        CyAsDeviceHandle     handle,
        CyAsMediaType        media,
        uint32_t             device,
        uint32_t             size,
        CyAsFunctionCallback cb,
        uint32_t             client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsReturnStatus_t ret ;
    CyAsBusNumber_t bus ;

    if ((media == CyAsMediaSDIO) || (media == CyAsMediaCEATA))
        return CY_AS_ERROR_NOT_SUPPORTED ;

    if ((media != CyAsMediaNand) && (media != CyAsMediaSDFlash) && (media != CyAsMediaMMCFlash))
        return CY_AS_ERROR_NO_SUCH_MEDIA ;

    ret = CyAnMapBusFromMediaType(dev_p, media, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return CyAsStorageCreatePPartition(handle, bus, device, size, cb, client) ;
}

CyAsReturnStatus_t
CyAsStorageRemovePPartition_dep(
        CyAsDeviceHandle        handle,
        CyAsMediaType           media,
        uint32_t                device,
        CyAsFunctionCallback    cb,
        uint32_t                client)
{
    CyAsDevice *dev_p = (CyAsDevice *)handle ;
    CyAsReturnStatus_t ret ;
    CyAsBusNumber_t bus ;

    if ((media == CyAsMediaSDIO) || (media == CyAsMediaCEATA))
        return CY_AS_ERROR_NOT_SUPPORTED ;

    if ((media != CyAsMediaNand) && (media != CyAsMediaSDFlash) && (media != CyAsMediaMMCFlash))
        return CY_AS_ERROR_NO_SUCH_MEDIA ;

    ret = CyAnMapBusFromMediaType(dev_p, media, &bus) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    return CyAsStorageRemovePPartition(handle, bus, device, cb, client) ;
}

/*[]*/

