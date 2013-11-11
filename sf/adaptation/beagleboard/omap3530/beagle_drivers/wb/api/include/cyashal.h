/* Cypress West Bridge API header file (cyashal.h)
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

#ifndef _INCLUDED_CYASHAL_H_
#define _INCLUDED_CYASHAL_H_

#if !defined(__doxygen__)

/* The possible HAL layers defined and implemented by Cypress */
/*nxz*/
#ifdef __CY_ASTORIA_BEAGLEBOARD_SPI__HAL__
#ifdef CY_HAL_DEFINED
#error Only one HAL layer can be defined
#endif

#define CY_HAL_DEFINED

#include "cyashalbeagleboard.h"
#endif

#ifdef __CY_ASTORIA_FPGA_HAL__
#ifdef CY_HAL_DEFINED
#error Only one HAL layer can be defined
#endif

#define CY_HAL_DEFINED

#include "cyashalfpga.h"
#endif

/***** SCM User space HAL  ****/
#ifdef __CY_ASTORIA_SCM_HAL__
#ifdef CY_HAL_DEFINED
#error Only one HAL layer can be defined
#endif

#define CY_HAL_DEFINED

#include "cyanhalscm.h"
#endif
/***** SCM User space HAL  ****/

/***** SCM Kernel HAL  ****/
#ifdef __CY_ASTORIA_SCM_KERNEL_HAL__
#ifdef CY_HAL_DEFINED
#error Only one HAL layer can be defined
#endif

#define CY_HAL_DEFINED

#include "cyanhalscm_kernel.h"
#endif
/***** SCM Kernel HAL  ****/

#ifdef __CY_ASTORIA_CUSTOMER_HAL__
#ifdef CY_HAL_DEFINED
#error Only one HAL layer can be defined
#endif

#define CY_HAL_DEFINED
#include "cyashal_customer.h"

#endif

#endif			/* __doxygen__ */

#endif			/* _INCLUDED_CYASHAL_H_ */
