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
* naviengine_assp\naviengine.cpp
*
*/



#include <naviengine_priv.h>
#include "uart/uart16550_ne.h"

//----------------------------------------------------------------------------
// Initialisation

void TNaviEngine::Init1()
//
// Phase 1 initialisation
//
    {
	// Use assp-specific nano wait implementation
	Kern::SetNanoWaitHandler(&NanoWait);
	}

void TNaviEngine::Init3()
//
// Phase 3 initialisation
//
    {
	//
	// TO DO: (optional)
	//
	// Initialise any TNaviEngine class data members here
	//
	}

EXPORT_C TMachineStartupType TNaviEngine::StartupReason()
//
// Read and return the Startup reason of the Hardware
//
	{
	//
	// TO DO: (optional)
	//
	// Read the Reset reason from the hardware register map it to one of TMachineStartupType enumerated values
	// and return this
	//
	return EStartupCold;   // EXAMPLE ONLY
	}

EXPORT_C TInt TNaviEngine::CpuVersionId()
//
// Read and return the the CPU ID
//
	{
	//
	// TO DO: (optional)
	//
	// Read the CPU identification register (if one exists) mask off redundant bits and return this
	//
	return 0;   // EXAMPLE ONLY
	}

EXPORT_C TUint TNaviEngine::DebugPortAddr()
//
// Return Linear base address of debug UART (as selected in obey file or with eshell debugport command).
//	
	{
	TUint debugPort;
	switch (Kern::SuperPage().iDebugPort)
		{
		case Arm::EDebugPortJTAG:	debugPort = 0; break; //indicates JTAG debugging
		case 0x101:
		case 1:						debugPort=KHwBaseUart1;	break;
		case 2:						debugPort=KHwBaseUart2;	break;
		default:					debugPort=KHwBaseUart0;	break;
		}
	return debugPort;
	}

EXPORT_C TUint TNaviEngine::ProcessorPeriodInPs()
//
// Return CPU clock period in picoseconds
//
	{
	//
	// TO DO: (optional)
	//
	// Read the CPU clock speed and return its period in picoseconds. If only a limited range of speeds is possible
	// it is preferable to use the masked speed reading as an index into a look up table containing the corresponding
	// period
	//
	return 0;	// EXAMPLE ONLY
	}

EXPORT_C TUint TNaviEngine::RtcData()
//
// Return the current time of the RTC
//
	{
	//
	// TO DO: (optional)
	//
	// Read the RTC current time register and return this time
	//
	return 0;	// EXAMPLE ONLY
	}

EXPORT_C void TNaviEngine::SetRtcData(TUint aValue)
//
// Set the RTC time
//
	{
	//
	// TO DO: (optional)
	//
	// Set the RTC current time with aValue (may need formatting appropriately)
	//
	}

EXPORT_C TPhysAddr TNaviEngine::VideoRamPhys()
//
// Return the physical address of the video RAM
//
	{
	return NaviEngineAssp::VideoRamPhys;
	}



void TNaviEngine::InitDebugOutput()
	{
	TUint baseAddr = DebugPortAddr();
	if (baseAddr) //baseAddr is NULL in case og jtag logging
		{
		TUint32 dp = Kern::SuperPage().iDebugPort;
		TBool highSpeed = (dp==0x100 || dp==0x101);

		// Baud Rate = 115200 or 230400 (highspeed); 8 bits, no parity, 1 stop bit
		*((volatile TInt*) (baseAddr+K16550LCROffset))  = 0x83;
		*((volatile TInt*) (baseAddr+K16550BDLoOffset)) = highSpeed ? KBaudRateDiv_230400 : KBaudRateDiv_default;
		*((volatile TInt*) (baseAddr+K16550BDHiOffset)) = 0;
		*((volatile TInt*) (baseAddr+K16550LCROffset))  = 3;
		}
	}

void TNaviEngine::DoDebugOutput(TUint aLetter)
	{
	TUint baseAddr = DebugPortAddr();
	if (baseAddr)
		{//serial port logging
		volatile TInt* status = (volatile TInt*) (baseAddr+K16550LSROffset);
		while ((*status & K16550LSR_TXHREmpty) == 0); //wait till TXR is empty
		*((volatile TInt*) (baseAddr+K16550TXHROffset)) = aLetter;
		}
	else
		Arm::DebugOutJTAG((TUint8)aLetter); // jtag logging
	}



