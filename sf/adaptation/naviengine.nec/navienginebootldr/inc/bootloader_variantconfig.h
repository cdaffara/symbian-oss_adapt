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
* bsp\hwip_nec_naviengine\ne1_tb_bootloader\inc\bootloader_variantconfig.h
*
*/



#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <e32cmn.h>
#include "naviengine.h"
#include "flash_nor.h"
#include <d32comm.h>

#define PLATFORM_BUILD 5


//

#define VARIANT_ZIP	_L("NE1_TB.ZIP")
#define VARIANT_BIN	_L("NE1_TB.IMG")

#define __USE_VARIANT_INIT__

#define __USE_LOCAL_DRIVES__
//#define __USE_USBMS__				// NaviEngine cannot support USB Mass Storage - no hardware!
#define __SUPPORT_UNZIP__			// There is very little reason for any variant not to support zipped images?
#define __SUPPORT_FLASH_REPRO__		// NaviEngine has NOR flash support
#define __SUPPORT_FLASH_NAND__		// NaviEngine has onboard NAND flash
#define __SUPPORT_MEMORY_TEST__

#define __SUPPORT_COM0_115200__
#define __SUPPORT_COM0_230400__
#define __SUPPORT_COM1_115200__
#define __SUPPORT_COM1_230400__
#define __SUPPORT_COM2_115200__
//#define __SUPPORT_COM2_230400__ // NaviEngine cannot run COM2 at 230400
//#define __SUPPORT_COM3_115200__ // There is no COM3 on the NaviEngine
//#define __SUPPORT_COM3_230400__ // There is no COM3 on the NaviEngine

const TInt KUartPortNumber=0;
const TInt KYModemGMode=1;

// pixel bit size. included in first palette entry only.
const TUint KPaletteEntPBS				= 0x3000; // 8bpp as the palette is only written in 8bpp

// Colours used in display.cpp
// Palette entries are RGB565
const TUint KPaletteEntBlack		= 0x0000;
const TUint KPaletteEntMidBlue		= 0x0014;
const TUint KPaletteEntMidGreen		= 0x0500;
const TUint KPaletteEntMidCyan		= 0x0514;
const TUint KPaletteEntMidRed		= 0xA000;
const TUint KPaletteEntMidMagenta	= 0xA014;
const TUint KPaletteEntMidYellow	= 0xA280;
const TUint KPaletteEntDimWhite		= 0xA514;
const TUint KPaletteEntGray			= 0x528A;
const TUint KPaletteEntBlue			= 0x011F;
const TUint KPaletteEntGreen		= 0x57EA;
const TUint KPaletteEntCyan			= 0x57FF;
const TUint KPaletteEntRed			= 0xFA8A;
const TUint KPaletteEntMagenta		= 0xFA9F;
const TUint KPaletteEntYellow		= 0xFFEA;
const TUint KPaletteEntWhite		= 0xFFFF;



const TUint32 KUidUBootldrCfgMagic = 0x10273EC7;

const TUint KConfigBlockSize = (KFlashEraseBlockSize/4);	// size in words

// Structure of the config block in NOR flash
struct SBootloaderConfig
	{
	TUint32 iMagic;
	TInt32 iCheckSum;
	TInt32 iPortNumber;
	TBps   iBaudRate;
	TInt32 iLoadDevice;
	// update iReserved padding when adding members to the structure!
	TInt32 iReserved[KConfigBlockSize-4];
	};


#endif
