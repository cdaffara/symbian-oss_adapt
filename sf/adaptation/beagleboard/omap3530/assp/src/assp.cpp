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
// omap3530/assp/src/assp.cpp
//

#include <kernel.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_timer.h>
#include <assp/omap3530_assp/omap3530_prcm.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>
#include <assp/omap3530_assp/omap3530_hardware_base.h>
#include <assp/omap3530_assp/omap3530_uart.h>
#include <assp/omap3530_shared/omap3_mstick.h>

#define PS_PER_SECOND 1000000000000
#define US_PER_SECOND 1000000


Omap3530Assp* Omap3530Assp::Variant;


DECLARE_STANDARD_ASSP()

EXPORT_C Omap3530Assp::Omap3530Assp()
	{
	Kern::Printf("%s::%s",__FUNCTION__,__FUNCTION__);
	
	}

EXPORT_C TMachineStartupType Omap3530Assp::StartupReason()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Assp::StartupReason"));
#ifdef _DEBUG															// REMOVE THIS
	TUint s = Kern::SuperPage().iHwStartupReason;
	__KTRACE_OPT(KBOOT,Kern::Printf("CPU page value %08x", s));
#endif																	// REMOVE THIS
	//
	// TO DO: (mandatory)
	//
	// Map the startup reason read from the Super Page to one of TMachineStartupType enumerated values
	// and return this
	//
	return EStartupCold;   // EXAMPLE ONLY
	}

EXPORT_C void Omap3530Assp::Init1()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Assp::Init1()"));
	
	Variant = (Omap3530Assp*)Arch::TheAsic();
	__ASSERT_ALWAYS( Variant != NULL, Kern::Fault( "assp Init1: no variant", 0 ) );
	
	Omap3530Interrupt::Init1();			// initialise the ASSP interrupt controller
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Assp::Init1() OUT"));
	}





EXPORT_C TUint64  Omap3530Assp::GetXtalFrequency()
	{
	return Prcm::ClockFrequency( Prcm::EClkSysClk );
	}


EXPORT_C void Omap3530Assp::Init3()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Assp::Init3() >"));

	Prcm::Init3();
	
	TArmCpuId id={0};
	ArmCpuVersionId(id);
	Omap3530Assp::NanoWaitCalibration();
	TUint64 hz = Prcm::ClockFrequency( Prcm::EClkMpu );
	Kern::Printf("CPU at %d MHz",(TInt)hz/1000000);
	Kern::SetNanoWaitHandler(NanoWait);
	Omap3::MsTick::Start();
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Assp::Init3() <"));
	}


/*
 * Returns the number of microseconds for the system millisecond tick;
 * */
EXPORT_C TInt Omap3530Assp::MsTickPeriod()
	{
	return 1000; 
	}




EXPORT_C TUint32 Omap3530Assp::NanoWaitCalibration()
	{
	// Return the minimum time in nano-seconds that it takes to execute the following code:
	//	 nanowait_loop:
	//	 		  subs r0, r0, r1
	//	 		  bhi nanowait_loop
	//THE cpu frequency can change so this calibration must be achieved each time the power management modifies the cpu freq.
  
	TInt cycles_ns =  Prcm::ClockFrequency( Prcm::EClkMpu ) /1000000000 ;
	      
	    
	return 2 * cycles_ns;  // 2 cycles approx 3.333ns at 600MHz
	}



EXPORT_C void Omap3530Assp::ArmCpuVersionId(TArmCpuId &id)
//
// Read and return the the CPU ID
//
	{
		TUint armString = Kern::SuperPage().iCpuId; 
		
		id.Implementor    = (armString & 0xFF000000) >>24;	
		id.Variant	  = (armString &     0x00F00000)  >>20;
		id.Architecture = (armString &   0x000F0000)  >>16; 		
		id.Primary	  = (armString &     0x0000FFF0)>> 4; 			
		id.Revision	  = (armString &     0x0000000F);
		
		Kern::Printf("%s  %s  Rev:%d",__FUNCTION__,(id.Primary ==0xc08)?"CORTEX_A8":"UNKNOWN",id.Revision);	
	}

EXPORT_C Omap3530Uart::TUartNumber Omap3530Assp::DebugPortNumber()
//
// Return index of the debug UART Omap3530Uart::EUart([0-2]|None)
//
	{
	Omap3530Uart::TUartNumber debugPort;
	switch (Kern::SuperPage().iDebugPort)
		{
		case 0:
			debugPort = Omap3530Uart::EUart0;
			break;
		case 1:
			debugPort = Omap3530Uart::EUart1;
			break;
		case 2:
			debugPort = Omap3530Uart::EUart2;
			break;

		default:
			debugPort = Omap3530Uart::EUartNone;
			break;
		}
	return debugPort;
	}



EXPORT_C TUint Omap3530Assp::ProcessorPeriodInPs()
//
// Return CPU clock period in picoseconds
//
	{
	TUint64 cycleTime =  (Prcm::ClockFrequency( Prcm::EClkMpu ) * US_PER_SECOND) /  PS_PER_SECOND ;
	return (TUint) cycleTime;
	}

