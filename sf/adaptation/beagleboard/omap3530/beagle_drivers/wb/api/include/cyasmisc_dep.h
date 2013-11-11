/* Cypress West Bridge API header file (cyasmisc_dep.h)
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

/* This header will contain Antioch specific declaration
 * of the APIs that are deprecated in Astoria SDK. This is
 * for maintaining backward compatibility with prior releases
 * of the Antioch SDK.
 */
#ifndef __INCLUDED_CYASMISC_DEP_H__
#define __INCLUDED_CYASMISC_DEP_H__

#ifndef __doxygen__

EXTERN CyAsReturnStatus_t 
CyAsMiscAcquireResource_dep(CyAsDeviceHandle handle, 
			  CyAsResourceType resource, 
			  CyBool force);
EXTERN CyAsReturnStatus_t 
CyAsMiscGetFirmwareVersion_dep(CyAsDeviceHandle handle, 
			     uint16_t *major, 
			     uint16_t *minor, 
			     uint16_t *build,
			     uint8_t *mediaType, 
			     CyBool *isDebugMode);
EXTERN CyAsReturnStatus_t
CyAsMiscSetTraceLevel_dep(CyAsDeviceHandle handle, 
			uint8_t level, 
			CyAsMediaType media, 
			uint32_t device, 
			uint32_t unit,
			CyAsFunctionCallback cb,
			uint32_t client);
#endif /*__doxygen*/

#endif /*__INCLUDED_CYANSTORAGE_DEP_H__*/
