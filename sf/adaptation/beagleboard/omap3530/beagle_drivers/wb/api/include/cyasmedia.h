/* Cypress West Bridge API header file (cyasmedia.h)
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

#ifndef _INCLUDED_CYASMEDIA_H_
#define _INCLUDED_CYASMEDIA_H_

#include "cyas_cplus_start.h"


/* Summary
   Specifies a specific type of media supported by West Bridge
 
   Description
   The West Bridge device supports five specific types of media as storage/IO devices
   attached to it's S-Port.  This type is used to indicate the type of 
   media being referenced in any API call.
*/
typedef enum CyAsMediaType
{
    CyAsMediaNand = 0x00,			/* Flash NAND memory (may be SLC or MLC) */
    CyAsMediaSDFlash = 0x01,			/* An SD flash memory device */
    CyAsMediaMMCFlash = 0x02,			/* An MMC flash memory device */
    CyAsMediaCEATA = 0x03,			/* A CE-ATA disk drive */
    CyAsMediaSDIO = 0x04,			/* SDIO device. */
    CyAsMediaMaxMediaValue = 0x05
    
} CyAsMediaType ;

#include "cyas_cplus_end.h"

#endif				/* _INCLUDED_CYASMEDIA_H_ */
