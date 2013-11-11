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
* naviengine_assp\assp.cpp
*
*/



#include <naviengine_priv.h>
#include <upd35001_timer.h>

NaviEngineAssp* NaviEngineAssp::Variant=NULL;
TPhysAddr NaviEngineAssp::VideoRamPhys;


DECLARE_STANDARD_ASSP()

EXPORT_C NaviEngineAssp::NaviEngineAssp()
	{
	NaviEngineAssp::Variant=this;
	iDebugInitialised = EFalse;

	/* Initialize timers 1 and 2 to generate the system timestamp counter */
	NETimer& T1 = NETimer::Timer(1);
	NETimer& T2 = NETimer::Timer(2);

	T1.iTimerCtrl = 0;						// stop and reset timer 1
	T1.iGTICtrl = 0;						// disable timer 1 capture modes
	T2.iTimerCtrl = 0;						// stop and reset timer 2
	T2.iGTICtrl = 0;						// disable timer 2 capture modes
	__e32_io_completion_barrier();
#ifdef __SMP__
	T1.iPrescaler = KNETimerPrescaleBy1;	// Timer 1 prescaled by 1 (=66.667MHz)
	T2.iPrescaler = KNETimerPrescaleBy1;	// Timer 2 prescaled by 1 (=66.667MHz)
#else
	T1.iPrescaler = KNETimerPrescaleBy32;	// Timer 1 prescaled by 32 (=2.0833MHz)
	T2.iPrescaler = KNETimerPrescaleBy32;	// Timer 2 prescaled by 32 (=2.0833MHz)
#endif
	__e32_io_completion_barrier();
	T1.iGTInterruptEnable = 0;
	T2.iGTInterruptEnable = 0;
	__e32_io_completion_barrier();
	T1.iGTInterrupt = KNETimerGTIInt_All;
	T2.iGTInterrupt = KNETimerGTIInt_All;
	__e32_io_completion_barrier();
	T1.iTimerCtrl = KNETimerCtrl_CE;		// deassert reset for timer 1, count still stopped
	T2.iTimerCtrl = KNETimerCtrl_CE;		// deassert reset for timer 2, count still stopped
	__e32_io_completion_barrier();
	T1.iTimerReset = 0xfffffeffu;			// timer 1 wraps after 2^32-256 counts
	T2.iTimerReset = 0xffffffffu;			// timer 2 wraps after 2^32 counts
	__e32_io_completion_barrier();
	T1.iTimerCtrl = KNETimerCtrl_CE | KNETimerCtrl_CAE;	// start timer 1
	__e32_io_completion_barrier();			// make sure timer 1 started before timer 2
	T2.iTimerCtrl = KNETimerCtrl_CE | KNETimerCtrl_CAE;	// start timer 2
	__e32_io_completion_barrier();

	// Each time T1 wraps, (T1-T2) increases by 256 after starting at 0
	// t1=T1; t2=T2; n=(t1-t2)>>8; time = t1 + n * (2^32-256)

	}

extern void MsTimerTick(TAny* aPtr);


EXPORT_C TMachineStartupType NaviEngineAssp::StartupReason()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("NaviEngineAssp::StartupReason"));
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

EXPORT_C void NaviEngineAssp::Init1()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("NaviEngineAssp::Init1()"));
	//
	// TO DO: (optional)
	//
	NaviEngineInterrupt::Init1();			// initialise the ASSP interrupt controller

	//
	// TO DO: (optional)
	//
	// Initialises any hardware blocks which require early initialisation, e.g. enable and power the LCD, set up
	// RTC clocks, disable DMA controllers. etc.
	//
	TNaviEngine::Init1();
	}


EXPORT_C void NaviEngineAssp::Init3()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("NaviEngineAssp::Init3()"));

	TNaviEngine::Init3();

#ifdef TOGLE_UART_DTR_LINE
     AsspRegister::Write32(KHwRwGpio_Port_Control_Enable, 1<<9);   
#endif


	NTimerQ& m=*(NTimerQ*)NTimerQ::TimerAddress();
	iTimerQ=&m;

	// Initialize timer 0 to generate the 1ms periodic system tick
	NETimer& NET = NETimer::Timer(0);
	NET.iTimerCtrl = 0;						// reset counter
	NET.iGTICtrl = 0;						// disable input capture
	__e32_io_completion_barrier();
	NET.iPrescaler = KNETimerPrescaleBy1;	// prescaler divides by 1
	__e32_io_completion_barrier();
	NET.iTimerCtrl = KNETimerCtrl_CE;		// take timer out of reset
	__e32_io_completion_barrier();
	NET.iTimerReset = 66666;				// clocks before timer reset (66.666MHz clock frequency)
	__e32_io_completion_barrier();
	NET.iGTInterrupt = KNETimerGTIInt_All;	// clear any pending interrupts
	__e32_io_completion_barrier();
	NET.iGTInterruptEnable = KNETimerGTIIntE_TCE;	// enable counter reset interrupt
	__e32_io_completion_barrier();
	NET.iTimerCtrl = KNETimerCtrl_CE | KNETimerCtrl_CAE;	// start counter
	__e32_io_completion_barrier();
    
	//
	// TO DO: (mandatory)
	//
	// If Hardware Timer used for System Ticks cannot give exactly the period required store the initial rounding value
	// here which is updated every time a match occurrs. Note this leads to "wobbly" timers whose exact period change
	// but averages exactly the required value
	// e.g.
	// m.iRounding=-5;
	//
	
	TInt r=Interrupt::Bind(KIntIdOstMatchMsTimer,MsTimerTick,&m);	// bind the System Tick interrupt
	if (r<0)
		Kern::Fault("BindMsTick",r);

	// 
	// TO DO: (mandatory)
	//
	// Clear any pending OST interrupts and enable any OST match registers.
	// If possible may reset the OST here (to start counting from a full period). Set the harwdare to produce an 
	// interrupt on full count
	//

	r=Interrupt::Enable(r);	// enable the System Tick interrupt
	if (r!=KErrNone)
		Kern::Fault("EnbMsTick",r);

	// Allocate physical RAM for video buffer.
	TInt vSize=VideoRamSize();
	r=Epoc::AllocPhysicalRam(2*vSize,NaviEngineAssp::VideoRamPhys); //Alloc memory for both secure and non-secure display.
	if (r!=KErrNone)
		Kern::Fault("AllocVRam",r);
	}

EXPORT_C TInt NaviEngineAssp::MsTickPeriod()
	{
	// Return the OST tick period (System Tick)
	return 1000;
	}

EXPORT_C TInt NaviEngineAssp::SystemTimeInSecondsFrom2000(TInt& aTime)
	{
	aTime=(TInt)TNaviEngine::RtcData();
	__KTRACE_OPT(KHARDWARE,Kern::Printf("RTC READ: %d",aTime));
	return KErrNone;
	}

EXPORT_C TInt NaviEngineAssp::SetSystemTimeInSecondsFrom2000(TInt aTime)
	{
	//
	// TO DO: (optional)
	//
	// Check if the RTC is running and is stable
	//
	__KTRACE_OPT(KHARDWARE,Kern::Printf("Set RTC: %d",aTime));
	TNaviEngine::SetRtcData(aTime);
	__KTRACE_OPT(KHARDWARE,Kern::Printf("RTC: %d",TNaviEngine::RtcData()));
	return KErrNone;
	}

EXPORT_C TUint32 NaviEngineAssp::NanoWaitCalibration()
	{
	// 
	// TO DO: (mandatory)
	//
	// Return the minimum time in nano-seconds that it takes to execute the following code:
	//	 nanowait_loop:
	//	 		  subs r0, r0, r1
	//	 		  bhi nanowait_loop
	//
	// If accurate timings are required by the Base Port, then it should provide it's own implementation 
	// of NanoWait which uses a hardware counter. (See Kern::SetNanoWaitHandler)
	//
	
	return 0;   // EXAMPLE ONLY
	}

EXPORT_C void NaviEngineAssp::DebugOutput(TUint aLetter)
//
// Output a character to the debug port
//
    {
	if (!iDebugInitialised)
		{
		TNaviEngine::InitDebugOutput();
		iDebugInitialised = ETrue;
		}
	TNaviEngine::DoDebugOutput(aLetter);
    }

