/* Cypress West Bridge API header file (cyasintr.h)
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

#ifndef _INCLUDED_CYASINTR_H_
#define _INCLUDED_CYASINTR_H_

#include "cyasdevice.h"

#include "cyas_cplus_start.h"

/* Summary
   Initialize the interrupt manager module

   Description
   This function is called to initialize the interrupt module.  This module enables interrupts
   as well as servies West Bridge related interrupts by determining the source of the interrupt and
   calling the appropriate handler function.

   Notes
   If the dmaintr parameter is TRUE, the initialization code initializes the interrupt mask to
   have the DMA related interrupt enabled via the general purpose interrupt.  However, the interrupt
   service function assumes that the DMA interrupt is handled by the HAL layer before the interrupt
   module handler function is called.

   Returns
   * CY_AS_ERROR_SUCCESS - the interrupt module was initialized correctly
   * CY_AS_ERROR_ALREADY_RUNNING - the interrupt module was already started

   See Also
   * CyAsIntrStop
   * CyAsServiceInterrupt
*/
extern CyAsReturnStatus_t
CyAsIntrStart(
        CyAsDevice *                    dev_p,          /* Device being initialized */
        CyBool                                  dmaintr         /* If true, enable the DMA interrupt through the INT signal */
        ) ;

/* Summary
   Stop the interrupt manager module

   Description
   This function stops the interrupt module and masks all interrupts from the West Bridge device.

   Returns
   * CY_AS_ERROR_SUCCESS - the interrupt module was stopped sucessfully
   * CY_AS_ERROR_NOT_RUNNING - the interrupt module was not running

   See Also
   * CyAsIntrStart
   * CyAsServiceInterrupt
*/
extern CyAsReturnStatus_t
CyAsIntrStop(
        CyAsDevice *                    dev_p           /* Device bein stopped */
        ) ;


/* Summary
   The interrupt service routine for West Bridge

   Description
   When an interrupt is detected, this function is called to service the West Bridge interrupt.  It is safe
   and efficient for this function to be called when no West Bridge interrupt has occurred.  This function
   will determine it is not an West Bridge interrupt quickly and return.
*/
extern void CyAsIntrServiceInterrupt(
        CyAsHalDeviceTag                tag                     /* The USER supplied tag for this device */
        ) ;

#include "cyas_cplus_end.h"

#endif                  /* _INCLUDED_CYASINTR_H_ */

