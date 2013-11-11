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
* ne1_tb\specific\monitor.cpp
* Kernel crash debugger - NE1_TBVariant specific
*
*/



#include <kernel/monitor.h>
#include "variant.h"
#include "uart/uart16550_ne.h"

// A copy of TNaviEngine::DoDebugOutput
static TUint DebugPortAddr()
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


//
// UART code
//
void CrashDebugger::InitUart()
	{
	// Initialise the UART for standard debug output
	TUint32 baseAddr = DebugPortAddr();
	if (baseAddr) // baseAddr is NULL in case of jtag logging
		{
		TUint32 dp = Kern::SuperPage().iDebugPort;
		TBool highSpeed = (dp==0x100 || dp==0x101);

		// Baud Rate = 115200; 8 bits, no parity, 1 stop bit
		*((volatile TInt*) (baseAddr+K16550LCROffset))  = 0x83;
		*((volatile TInt*) (baseAddr+K16550BDLoOffset)) = highSpeed ? KBaudRateDiv_230400 : KBaudRateDiv_default;
		*((volatile TInt*) (baseAddr+K16550BDHiOffset)) = 0;
		*((volatile TInt*) (baseAddr+K16550LCROffset))  = 3;

		// Set the FIFO control register (FCR) to polled mode & don't use interrupts
		*((volatile TInt*) (baseAddr+K16550FCROffset)) = 0;
		*((volatile TInt*) (baseAddr+K16550FCROffset)) = 1;
		*((volatile TInt*) (baseAddr+K16550IEROffset)) = 0;
		}
	}

void CrashDebugger::UartOut(TUint aChar)
	{
	TUint debugPort = DebugPortAddr();
	volatile TUint32& LSR = *(volatile TUint32*)(debugPort + 0x14);
	volatile TUint32& TXHR = *(volatile TUint32*)(debugPort + 0x00);
	volatile TUint32& RXHR = *(volatile TUint32*)(debugPort + 0x00);
	while (LSR & 1)
		{ 
		if (CheckPower())
			return;
		TUint32 c = RXHR;
		if (c==19)            // XOFF
			{
			FOREVER
				{
				// wait for XON
				while (!(LSR & 1))
					{
					if (CheckPower())
						return;
					}
				c = RXHR;
				if (c==17)    // XON
					break;
				else if (c==3)		// Ctrl C
					Leave(KErrCancel);
				}
			}
		else if (c==3)		// Ctrl C
			Leave(KErrCancel);
		}
	while (!(LSR & 32))
		CheckPower();
	TXHR = aChar;
	}

TUint8 CrashDebugger::UartIn()
	{
	TUint debugPort = DebugPortAddr();
	volatile TUint32& LSR = *(volatile TUint32*)(debugPort + 0x14);
	volatile TUint32& RXHR = *(volatile TUint32*)(debugPort + 0x00);
	while (!(LSR & 1))
		{
		if (CheckPower())
			return (TUint8)0x0d;
		}
	return (TUint8)RXHR;
	}

TBool CrashDebugger::CheckPower()
	{
	//
	// TO DO: (mandatory)
	//
	// Check if power supply is stable and return ETrue if not
	//
	return EFalse;	// EXAMPLE ONLY
	}

