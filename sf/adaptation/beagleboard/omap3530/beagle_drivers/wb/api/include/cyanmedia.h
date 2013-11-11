/* Cypress West Bridge API header file (cyanmedia.h)
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

#ifndef _INCLUDED_CYANMEDIA_H_
#define _INCLUDED_CYANMEDIA_H_

#include "cyas_cplus_start.h"

/* Summary
   Specifies a specific type of media supported by West Bridge
 
   Description
   The West Bridge device supports five specific types of media as storage/IO devices
   attached to it's S-Port.  This type is used to indicate the type of 
   media being referenced in any API call.
*/
#include "cyasmedia.h"
#define CyAnMediaNand CyAsMediaNand             /* Flash NAND memory (may be SLC or MLC) */
#define CyAnMediaSDFlash CyAsMediaSDFlash       /* An SD flash memory device */
#define CyAnMediaMMCFlash CyAsMediaMMCFlash     /* An MMC flash memory device */
#define CyAnMediaCEATA CyAsMediaCEATA           /* A CE-ATA disk drive */
#define CyAnMediaSDIO CyAsMediaSDIO             /* SDIO device. */
#define CyAnMediaMaxMediaValue CyAsMediaMaxMediaValue

typedef CyAsMediaType CyAnMediaType;


#include "cyas_cplus_end.h"

#endif				/* _INCLUDED_CYANMEDIA_H_ */
