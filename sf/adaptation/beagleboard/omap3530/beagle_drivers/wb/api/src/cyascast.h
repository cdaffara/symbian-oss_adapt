/* Cypress West Bridge API header file (cyasdevice.h)
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

#ifndef _INCLUDED_CYASCAST_H_
#define _INCLUDED_CYASCAST_H_

#ifndef __doxygen__
#define CyCastInt2UInt16(v) ((uint16_t)(v))
#if 0 /*nxz*/
#ifdef _DEBUG
#define CyCastInt2UInt16(v) (CyAsHalAssert(v < 65536), (uint16_t)(v))
#else           /* _DEBUG */
#define CyCastInt2UInt16(v) ((uint16_t)(v))
#endif          /* _DEBUG */
#endif

#endif      /* __doxygen__ */
#endif          /* _INCLUDED_CYASCAST_H_ */
