/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
*
*/



#ifndef __VARIANT_NAND_PLAT_H__
#define __VARIANT_NAND_PLAT_H__

#include <nand_fbr_offset.h>
#include <nanddevice.h>


//
// Const declarations
//
const TNandDeviceInfo gDeviceTable[] = {
/********************************************************************************************************/
//        iManufacturerCode;
//        |                   iDeviceCode;
//        |	                  |       iNumBlocks;
//        |	                  |       |     iNumSectorsPerBlock;
//        |                   |       |     |    iNumBytesMain;
//        |                   |       |     |    |    iNumBytesSpare;
//        |                   |       |     |    |    |   iSectorShift;
//        |                   |       |     |    |    |   |  iBlockShift;
//        |                   |       |     |    |    |   |  |   iBlksInRsv;
//        |                   |       |     |    |    |   |  |   |   iBadPos;
//        |                   |       |     |    |    |   |  |   |   |  iLsnPos;
//        |                   |       |     |    |    |   |  |   |   |  |  iECCPos;
//        |                   |       |     |    |    |   |  |   |   |  |  |  iFlags;
//        |                   |       |     |    |    |   |  |   |   |  |  |  |
//        V                   V       V     V    V    V   V  V   V   V  V  V  V
/********************************************************************************************************/
        { ESamsungId,         0xDAu,  2048, 256, 512, 16, 9, 14, 40, 0, 2, 6, TDeviceFlags(ELargeBlock|EDataIoWidth16)},	//  64 MB (512 MBit)
        // Last entry
        { TManufacturerId(0),     0,     0,   0,   0,  0, 0,  0,  0, 0, 0, 0, TDeviceFlags(0) }
	};
	
	
	
/**
 @internalTechnology
*/
const TUint32 KCoreldrStackAddr	= 0x8C0003FCu;		// Top of coreloader stack

/**
 * The following constants support relocation of the NAND FBR for this platform
 * KNandFbrSectorOffset is the third of three constants required - it is defined
 * in nand_fbr_offset.h
 */
#ifndef MINIBOOT
const TBool KNandMinibootUsed		=	EFalse;
const TBool KNandCoreldrRelocatable	=	ETrue;
#else
const TBool KNandMinibootUsed		=	ETrue;
const TBool KNandCoreldrRelocatable	=	EFalse;
#endif /* MINIBOOT */

#endif // __VARIANT_NAND_PLAT_H__


