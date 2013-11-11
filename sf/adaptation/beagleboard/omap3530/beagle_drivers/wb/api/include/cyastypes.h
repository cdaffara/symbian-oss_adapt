/* Cypress West Bridge API header file (cyastypes.h)
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

#ifndef _INCLUDED_CYASTYPES_H_
#define _INCLUDED_CYASTYPES_H_

#include "cyashaldef.h"

/* Types that are not available on specific platforms.
 * These are used only in the reference HAL implementations and
 * are not required for using the API.
 */
#ifdef __unix__
typedef unsigned long DWORD;
typedef void *        LPVOID;
#define WINAPI
#define INFINITE        (0xFFFFFFFF)
#define PtrToUint(ptr)  ((unsigned int)(ptr))
#endif

/* Basic types used by the entire API */

/* Summary
   This type represents an endpoint number
*/
typedef uint8_t CyAsEndPointNumber_t ;

/* Summary
   This type is used to return status information from an API call.
*/
typedef uint16_t CyAsReturnStatus_t ;

/* Summary
   This type represents a bus number
*/
typedef uint32_t CyAsBusNumber_t ;

/* Summary
   All APIs provided with this release are marked extern through this definition.
   This definition can be changed to meet the scope changes required in the user
   build environment.

   For example, this can be changed to __declspec(exportdll) to enable exporting
   the API from a DLL.
 */
#define EXTERN          extern

#endif
