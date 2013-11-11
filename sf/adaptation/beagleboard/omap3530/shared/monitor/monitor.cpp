// Copyright (c) 1994-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// omap3530/shared/monitor/monitor.cpp
// 
//

#include <monitor.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_uart.h>
#include <assp/omap3530_assp/omap3530_prcm.h>

void CrashDebugger::InitUart()
	{
	const Omap3530Uart::TUartNumber portNumber( Omap3530Assp::DebugPortNumber() );

	if( portNumber >= 0 )
		{
		Omap3530Uart::TUart uart( portNumber );

		// Ensure UART clocks are running
		Prcm::SetClockState( uart.PrcmInterfaceClk(),Prcm::EClkOn );
		Prcm::SetClockState( uart.PrcmFunctionClk(), Prcm::EClkOn );
	
		// Add new line and wait for uart to fihish any transmission (i.e. crash info from fifo)
		uart.Write('\r');
		uart.Write('\n');
		while(!uart.TxFifoEmpty());
		
		// We don't know what state the UART is in, so reinitialize it
		uart.Init();
		uart.DefineMode( Omap3530Uart::TUart::EUart );
		uart.SetBaud( Omap3530Uart::TUart::E115200 );
		uart.SetDataFormat( Omap3530Uart::TUart::E8Data, Omap3530Uart::TUart::E1Stop, Omap3530Uart::TUart::ENone );
		uart.Enable();
		}
	}

void CrashDebugger::UartOut(TUint aChar)
	{
	const Omap3530Uart::TUartNumber portNumber( Omap3530Assp::DebugPortNumber() );

	if( portNumber >= 0 )
		{
		Omap3530Uart::TUart uart( portNumber );

		TUint c=0;
			
		while ( !uart.RxFifoEmpty() )
			{ 
			if ( CheckPower() )
				{
				return;
				}

			c = uart.Read();
			
			if ( c == 19 )            // XOFF
				{
				FOREVER
					{
					while( uart.RxFifoEmpty() )
						{
						if ( CheckPower() )
							{
							return;
							}
						}

					c = uart.Read();
					
					if ( c == 17 )    // XON
						{
						break;
						}
					else if ( c == 3 )		// Ctrl C
						{
						Leave(KErrCancel);
						}
					}
				}
			else if ( c == 3 )		// Ctrl C
				{
				Leave(KErrCancel);
				}
			}

		while ( uart.TxFifoFull() )
			{
			CheckPower();
			}

		uart.Write( aChar );
		}
	}

TUint8 CrashDebugger::UartIn()
	{
	const Omap3530Uart::TUartNumber portNumber( Omap3530Assp::DebugPortNumber() );

	if( portNumber >= 0 )
		{
		Omap3530Uart::TUart uart( portNumber );

		while ( uart.RxFifoEmpty() )
			{
			if ( CheckPower() )
				{
				return 0x0d;
				}
			}
		return uart.Read();
		}
	
	return 0;
	}

TBool CrashDebugger::CheckPower()
	{
	//
	// Check if power supply is stable and return ETrue if not
	//
	return EFalse;	// EXAMPLE ONLY
	}

