/* Cypress West Bridge API source file (cyasmisc_dep_impl.h)
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
CyAsMiscAcquireResource_dep(CyAsDeviceHandle handle, 
                          CyAsResourceType resource, 
                          CyBool force)
{
    CyAsResourceType *resource_p, my_resource = resource;
    resource_p = &my_resource;
    return CyAsMiscAcquireResource(handle, resource_p, force, 0, 0);
}


CyAsReturnStatus_t 
CyAsMiscGetFirmwareVersion_dep(CyAsDeviceHandle handle, 
                             uint16_t *major, 
                             uint16_t *minor, 
                             uint16_t *build,
                             uint8_t *mediaType, 
                             CyBool *isDebugMode)
{
    CyAsGetFirmwareVersionData data ;
    CyAsReturnStatus_t status = CyAsMiscGetFirmwareVersion(handle, &data, 0, 0) ;
    *major = data.major ;
    *minor = data.minor ;
    *build = data.build ;
    *mediaType = data.mediaType ;
    *isDebugMode = data.isDebugMode ;
    return status ;
}

CyAsReturnStatus_t
CyAsMiscSetTraceLevel_dep(CyAsDeviceHandle handle, 
                        uint8_t level, 
                        CyAsMediaType media, 
                        uint32_t device, 
                        uint32_t unit,
                        CyAsFunctionCallback cb,
                        uint32_t client)
{
    CyAsBusNumber_t     bus ;
    if(media >= CyAsMediaMaxMediaValue) return CY_AS_ERROR_NOT_SUPPORTED;
    CyAsBusFromMediaType(media, &bus) ;
    return CyAsMiscSetTraceLevel(handle, level, bus, device, unit, cb, client) ;
}


/*[]*/
