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
* base\bsp\hwip_nec_naviengine\ne1_tb_bootloader\bootloader_variant.cpp
*
*/



#define FILE_ID	0x464C5348
#include "bootldr.h"
#include "ubootldrldd.h"
#include <e32std.h>
#include <e32std_private.h>
#include <e32svr.h>
#include <e32cons.h>
#include <f32file.h>
#include <hal.h>
#include <u32hal.h>
#include "flash_nor.h"
#include <d32comm.h>

TLinAddr ConfigBlockAddress;							// Virtual address

// RAM copy of config block
SBootloaderConfig aConfigBlock;

// Invoke the flash_nor routines to read a block containing the bootloader
// configuration.
//
// If the block is found to be invalid, create a new block from defaults.
//
GLDEF_C void ReadConfig()
	{
	TLinAddr FlashAddr;
	FlashAddr = (TLinAddr) GetFlashChunk();	// Map flash

	// Config block is last block of the bootloader space before a flashed image
	ConfigBlockAddress = FlashAddr + KNORFlashMaxBootloaderSize - KFlashEraseBlockSize;

	// Make sure the region is not blank and that the first word has the
	// bootloader cfg uid on it.
	if (!BlankCheck(ConfigBlockAddress, KConfigBlockSize) &&
		(*(volatile TUint32*)ConfigBlockAddress == KUidUBootldrCfgMagic))
		{
		// Copy the config block out of flash and into ram
		volatile TUint32* j=(volatile TUint32*)ConfigBlockAddress;
		TUint32* dest = (TUint32 *)&aConfigBlock;

		for (TUint i=0 ; i<KConfigBlockSize ; ++i, ++dest, ++j)
			*dest=*j;
		}
	else
		{
		PrintToScreen(_L("Config area was blank, setting defaults\r\n"));

		aConfigBlock.iMagic = KUidUBootldrCfgMagic;
		aConfigBlock.iPortNumber = KUartPortNumber;
		aConfigBlock.iBaudRate   = EBps115200;
		aConfigBlock.iLoadDevice = ELoadDrive;

		// Write the new block to flash
		WriteConfig();
		}

	// Configure bootloader from config block
	SerialDownloadPort = aConfigBlock.iPortNumber;
	SerialBaud         = aConfigBlock.iBaudRate;
	LoadDevice = (TLoadDevice)aConfigBlock.iLoadDevice;

	// The H4 supports downloading images via the USB Mass Storage method. As
	// the method of detecting a USB-MS boot is rather awkward the task is
	// performed at a variant level.
	if (LoadDevice==ELoadUSBMS)
		{
		// Read the custom restart reason from the hal
		TInt aRestartReason;
		TInt r = HAL::Get(HAL::ECustomRestartReason, aRestartReason);
		if (r!=KErrNone)
			{
			PrintToScreen(_L("Failed to read restart reason from HAL (0x%x)"), r);
			RDebug::Printf("Failed to read restart reason from HAL (0x%x)", r);
			BOOT_FAULT();
			}

		// Decode the On H4 the restart reasons get shifted internally
		if (aRestartReason & (KtRestartCustomRestartUSBLoader >> KsRestartCustomRestartReasons))
			{
			// In the boot scenario the indicate that the bootloader should
			// check for an appropriate file before restarting the USB Mass
			// Storage application.
			LoadDevice = EBootUSBMS;
			}
		}
	}


// Write the config block back into flash
GLDEF_C void WriteConfig()
	{
	aConfigBlock.iPortNumber=SerialDownloadPort;
	aConfigBlock.iBaudRate  =SerialBaud;
	aConfigBlock.iLoadDevice=LoadDevice;
	// XXX Generate a checksum for the config block (excluding the checksum)

	TInt r = Erase(ConfigBlockAddress, KFlashEraseBlockSize);
	if (r!=KErrNone)
		{
		PrintToScreen(_L("Config block failed erase (0x%x)"), r);
		RDebug::Printf("Config block failed erase (0x%x)", r);
		BOOT_FAULT();
		}

	r=Write(ConfigBlockAddress, KFlashEraseBlockSize, (TUint32 *)&aConfigBlock);
	if (r!=KErrNone)
		{
		PrintToScreen(_L("Config block failed write (0x%x)"), r);
		RDebug::Printf("Config block failed write (0x%x)", r);
		BOOT_FAULT();
		}
	}


GLDEF_C void VariantInit()
/**
 * Can place variant code here.
 */
	{
	// Set the bootloader configuration based on values saved in Flash
	ReadConfig();

	// Other variant actions e.g. print out the board information

	return;
	}
