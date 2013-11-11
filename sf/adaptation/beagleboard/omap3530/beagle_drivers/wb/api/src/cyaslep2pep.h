/* Cypress West Bridge API header file (cyaslep2pep.h)
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

#ifndef _INCLUDED_CYASLEP2PEP_H_
#define _INCLUDED_CYASLEP2PEP_H_

#include "cyasdevice.h"

extern CyAsReturnStatus_t
CyAsUsbMapLogical2Physical(CyAsDevice *dev_p) ;

extern CyAsReturnStatus_t
CyAsUsbSetupDma(CyAsDevice *dev_p) ;

extern CyAsReturnStatus_t
CyAsUsbSetDmaSizes(CyAsDevice *dev_p) ;

#endif
