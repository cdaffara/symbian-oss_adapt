/* Cypress West Bridge API header file (cyashalcb.h)
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

#ifndef _INCLUDED_CYASHALCB_H_
#define _INCLUDED_CYASHALCB_H_

/* Summary
   This type defines a callback function type called when a DMA operation has completed.

   Description

   See Also
   * CyAsHalDmaRegisterCallback
   * CyAsHalDmaSetupWrite
   * CyAsHalDmaSetupRead
*/
typedef void (*CyAsHalDmaCompleteCallback)(
	CyAsHalDeviceTag tag,
	CyAsEndPointNumber_t ep, 
	uint32_t cnt,
	CyAsReturnStatus_t ret) ;

typedef CyAsHalDmaCompleteCallback CyAnHalDmaCompleteCallback;
#endif
