/* Cypress West Bridge API header file (cyashaldef.h)
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

#ifndef _INCLUDED_CYASHALDEF_H_
#define _INCLUDED_CYASHALDEF_H_

/* Summary
   If set to TRUE, the basic numeric types are defined by the West Bridge API code

   Description
   The West Bridge API relies on some basic integral types to be defined.  These types include
   uint8_t, int8_t, uint16_t, int16_t, uint32_t, and int32_t.  If this macro is defined the
   West Bridge API will define these types based on some basic assumptions.  If this value is
   set and the West Bridge API is used to set these types, the definition of these types must
   be examined to insure that they are appropriate for the given target architecture and
   compiler.

   Notes
   It is preferred that if the basic platform development environment defines these types
   that the CY_DEFINE_BASIC_TYPES macro be undefined and the appropriate target system header
   file be added to the file cyashaldef.h.
*/
#ifndef CY_DEFINE_BASIC_TYPES

/*
 * Include your target system header file that defines the basic types here if at all
 * possible.
 */
#ifdef __CY_ASTORIA_SCM_KERNEL_HAL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#else

/*
 * Define the basic types.  If these are not defined by your system, define these
 * here by defining the symbol CY_DEFINE_BASIC_TYPES
 */
typedef signed char int8_t ;
typedef signed short int16_t ;
typedef signed int int32_t ;
typedef unsigned char uint8_t ;
typedef unsigned short uint16_t ;
typedef unsigned int uint32_t ;

/*nxz*/
#include <e32def.h>


#endif

#if !defined(__doxygen__)
typedef int CyBool ;
#define CyTrue				(1)
#define CyFalse				(0)
#endif

#endif			/* _INCLUDED_CYASHALDEF_H_ */
