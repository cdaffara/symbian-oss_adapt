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
* naviengine_assp\uart\vserialkeyb.cpp
* Serial keyboard driver
*
*/


/**
 @file
 @internalTechnology
*/

#include <e32keys.h>
#include <comm.h>
#include <assp.h>
#include "../naviengine.h"
#include <kernel/kern_priv.h>
#include "uart16550_ne.h"
#include "vserialkeyb.h"
#include "bootloader_variantconfig.h"

TInt TSerialKeyboard::GetSerialPort(TUint& aBaud)
	{
	// Get the Ymodem serial port from NOR Flash config values and set the keyboard port appropriately.
	//
	// The debug port should be the same as the Ymodem port.  The keyboard port should never be the same
	// as the Ymodem port or else the image will fail to boot.
	//
	// Don't use port 2 as the keyboard port because we may not have a port 2 on the board.
	//
	// Therefore, depending on the YmodemG download port, set the keyboard port as follows:
	//
	//    Ymodem  Keyboard
	//    0       1
	//    1       0
	//    2       1

	TInt    keyboardPort           = 1;
	aBaud = 115200;
	TUint32 ConfigBlockAddressPhys = KNORFlashMaxBootloaderSize - KFlashEraseBlockSize;

	__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: KNORFlashTargetAddr        = %x", KNORFlashTargetAddr       ));
	__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: KNORFlashTargetSize        = %x", KNORFlashTargetSize       ));
	__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: KNORFlashMaxBootloaderSize = %x", KNORFlashMaxBootloaderSize));
	__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: KNORFlashMaxImageSize      = %x", KNORFlashMaxImageSize     ));
	__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: KFlashEraseBlockSize       = %x", KFlashEraseBlockSize      ));
	__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: ConfigBlockAddressPhys     = %x", ConfigBlockAddressPhys    ));

	{
		DPlatChunkHw * chunk;

		TInt errorCode = DPlatChunkHw::New(chunk, ConfigBlockAddressPhys, KFlashEraseBlockSize, EMapAttrSupRw | EMapAttrFullyBlocking);
		
		if (errorCode == KErrNone)
		{
			TUint32 linearAddr         = (TUint)chunk->LinearAddress();

			__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: linearAddr = %x", linearAddr));

			SBootloaderConfig * configBlockPtr = (SBootloaderConfig * )linearAddr;

			//TUint32 magic = *((TUint32*)linearAddr);
			if (configBlockPtr->iMagic == KUidUBootldrCfgMagic)
				{
				// valid config block
				__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: Valid config block, magic=%x", configBlockPtr->iMagic));
				__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: ymodemPort == %d from NOR flash", configBlockPtr->iPortNumber));

				switch (configBlockPtr->iPortNumber)
					{
					default:
					case 0: keyboardPort=1; break;
					case 1: keyboardPort=0; break;
					case 2: keyboardPort=1; break;
					}

				// Set the debug port to be the same as the Ymodem download port so that error messages get logged by trgtest
				Kern::SuperPage().iDebugPort = configBlockPtr->iPortNumber;
				}
			else
				{
				// invalid config block
				__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: Invlaid config block, magic=%x", configBlockPtr->iMagic));
				}
		}
		else
		{
			__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: error: failed to allocate chunk (%d)", errorCode));
		}

		// close the chunk
		chunk->Close(chunk);
	}

	__KTRACE_OPT(KEXTENSION,Kern::Printf("ubootldrkeyb: keyboardPort == %d", keyboardPort));
	return keyboardPort;
	}

