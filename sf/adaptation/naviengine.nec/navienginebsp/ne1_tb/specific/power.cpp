/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* ne1_tb\specific\power.cpp*
*/



#include "ne1_tb_power.h"

DNE1_TBPowerController* TNE1_TBPowerController::iPowerController = NULL;

#ifdef __SMP__
#ifndef __NO_IDLE_HANDLER_PIL__
DNE1_SMPIdleHandler* TNE1_TBPowerController::iIdleHandler = NULL;
#endif
const TUint32 DNE1_TBPowerController::KCyclesPerTick 	= 66666;
const TInt DNE1_TBPowerController::KMaxSleepTicks 	= TInt(0xffffff00u/DNE1_TBPowerController::KCyclesPerTick)-1;
const TUint32 DNE1_TBPowerController::KWakeUpBeforeTick = 24000;
const TUint32 DNE1_TBPowerController::KTooCloseToTick	= 6666;
const TUint32 DNE1_TBPowerController::KMinTimeToTick	= 2000;
const TInt DNE1_TBPowerController::KMinIdleTicks		= 2;
#if defined(SIMULATE_RETIREMENT) && !defined(__NO_IDLE_HANDLER_PIL__)
volatile TUint32 DNE1_SMPIdleHandler::iRetiredCores = 0;
#endif
#endif // __SMP__

inline TUint32 abs_u32diff(TUint32 aA, TUint32 aB) 
    {
    return (aA > aB) ? aA - aB : aB - aA; 
    }


//-/-/-/-/-/-/-/-/-/ class DNE1_SMPIdleHandler /-/-/-/-/-/-/-/-/-/

#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__)

DNE1_SMPIdleHandler::DNE1_SMPIdleHandler(DNE1_TBPowerController* aController)
    :DSMPIdleHandler(),iController(aController)
    {
    }

TInt DNE1_SMPIdleHandler::Initialise()
    {
    TInt r = KErrNone;
    DSMPIdleHandler::Initialise(KHwBaseGlobalIntDist,KHwBaseIntIf);
#ifdef SIMULATE_RETIREMENT
    // create as many antiIdle threads as there are cored
    TInt nc = NKern::NumberOfCpus();
    iIdleStealers = new TDfcQue*[nc];
    __PM_ASSERT_ALWAYS(iIdleStealers);
    iIdleStealDfcs = new TDfc*[nc];
    __PM_ASSERT_ALWAYS(iIdleStealDfcs);
    for (TInt i = 0; i < nc; i++)
        {
        TName name = _L("IDLESTEALER");
        name.AppendNum(i);
        r = Kern::DfcQCreate(iIdleStealers[i],1,&name);
        __PM_ASSERT_ALWAYS(KErrNone==r);
        iIdleStealDfcs[i] = new TDfc(IdleSteal,(TAny*) i,iIdleStealers[i],0);
        __PM_ASSERT_ALWAYS(iIdleStealDfcs[i]);
        NKern::ThreadSetCpuAffinity((NThread*)iIdleStealers[i]->iThread,i);
        }
    TName name = _L("RETIREENAGE");
    r = Kern::DfcQCreate(iRetireEngageQue,27,&name);
#endif
    return r;
    }

TBool DNE1_SMPIdleHandler::DoEnterIdle(TInt aCpuMask, TInt aStage, volatile TAny* /*aU*/)
    {
    if (aStage & SCpuIdleHandler::EPostamble)
        {
        iController->IdleTickSuppresionRestore();
        return EFalse;
        }
#ifdef SIMULATE_RETIREMENT
    // are we retiring? 
    if (iRetiredCores&aCpuMask) 
        {
        // this should be safe as no cores can be using sync points yet 
        // as DoEnterIdle is called before all cores are in idle
        // and the last core has not arrived yet
        // theorically would not return from here 
        // DoRetireCore would call TIdleSupport::MarkCoreRetired as last 
        // thing once core is guaranteed not to enter idle handler again 
        // until it is enaged once more
        DoRetireCore(__e32_find_ms1_32(aCpuMask),0); 
        return EFalse;
        }
#endif
    return ETrue;
    }


TBool DNE1_SMPIdleHandler::GetLowPowerMode(TInt aIdleTime, TInt &aLowPowerMode)
    {
    
    aLowPowerMode = 0;
    if (aIdleTime < DNE1_TBPowerController::KMinIdleTicks) return EFalse;
    iController->IdleTickSuppresionEntry(DNE1_TBPowerController::KWakeUpBeforeTick,aIdleTime);  
    return ETrue;
    }

TBool DNE1_SMPIdleHandler::EnterLowPowerMode(TInt aMode, TInt aCpuMask, TBool aLastCpu)
    {
    TIdleSupport::DoWFI();	// maybe we will wake up, or maybe another CPU will wake us up
    return ETrue;
    }


#ifdef SIMULATE_RETIREMENT

void DNE1_SMPIdleHandler::IdleSteal(TAny* aPtr)
    {
    TInt cpu = (TInt) aPtr;
    PMBTRACE4(KRetireCore,KRetireCoreEntry,cpu);
    TUint32 cpuMask = 0x1 << cpu;

    while (cpuMask&iRetiredCores);
    PMBTRACE4(KRetireCore,KRetireCoreeXit,cpu);
    }

void DNE1_SMPIdleHandler::DoRetireCore(TInt aCpu, TLinAddr /*aReturnPoint*/)
    {
    iIdleStealDfcs[aCpu]->RawAdd();
    TIdleSupport::MarkCoreRetired(0x1<<aCpu);
    }

#endif
#endif


//-/-/-/-/-/-/-/-/-/ class DNE1_TBPowerController /-/-/-/-/-/-/-/-/-/

DNE1_TBPowerController::DNE1_TBPowerController()
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__)
    :iIdleHandler(this)
#endif
    {
    Register();			// register Power Controller with Power Manager
	TNE1_TBPowerController::RegisterPowerController(this);
	
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__)
    //	Register idle handler
    if ((AsspRegister::Read32(KHwRoGpio_Port_Value) & (0x1<<27)))
        {
        __PM_ASSERT_ALWAYS(KErrNone==iIdleHandler.Initialise());
        }
    else 
        {
        // press and hold User Switch 0 / INT0 (SW3) on boot
        // to disable idle tick suppression
        Kern::Printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        Kern::Printf("!!!!!!!!!!!! NOT DOING ITS !!!!!!!!!!!");
        Kern::Printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        }
#endif
	}

void DNE1_TBPowerController::CpuIdle()
	{
	Arch::TheAsic()->Idle();
#ifndef __SMP__
    iIdleCount++;
#endif
	}

void DNE1_TBPowerController::EnableWakeupEvents()
	{
	//
	// TO DO: (mandatory)
	//
	// Enable tracking of wake-up events directly in hardware. If the hardware is controlled by a Driver
	// or Extension, may need to disable interrupts and preemption around the code that accesses the hardware
	// and set up a flag which the Driver/Extension code need to read before modifying the state of that piece
	// of hardware. Note in that case the Driver/Extension may need to link to this Library.
	//

	//
	// EXAMPLE ONLY
	// In this example we simply assume that the driver will call the Power Controller every time a 
	// wakeup event occurr. It is up to the Power Controller to know if it is tracking them or not.
	// We also assume that if a wakeup event occurrs when the CPU is in Standby, this will automatically
	// bring it back from that state.
	iWakeupEventsOn = ETrue;	// start tracking wakeup events
	}

void DNE1_TBPowerController::DisableWakeupEvents()
	{
	//
	// TO DO: (mandatory)
	//
	// Disable tracking of wake-up events directly in hardware or if the hardware is controlled by a Driver or
	// Extension need to set up a flag which the Driver/Extension reads whenever the event occurs, in order to
	// find out if it needs to deliver notification to the Power Controller
	//
	iWakeupEventsOn = EFalse;	// stop tracking wakeup events
	}

void DNE1_TBPowerController::AbsoluteTimerExpired()
	{
	if (iTargetState == EPwStandby && iWakeupEventsOn)
		{
		iWakeupEventsOn = EFalse;		// one occurred, no longer track wakeup events
		WakeupEvent();
		}
	}

void DNE1_TBPowerController::PowerDown(TTimeK aWakeupST)	
	{
	if (iTargetState == EPwStandby)
		{
		//
		// TO DO: (mandatory)
		//
		// Converts between the Wakeup time in System Time units as passed in to this function and a Wakeup
		// time in RTC units. The following code is given as an example how to convert between System time units
		// RTC time units on a system with a 32 bit RTC timer and which is incremented on a second interval:
		//
		// TUint32 wakeupRTC;
		if (aWakeupST)
			{
			TUint32 nowRTC = TNaviEngine::RtcData();
			TTimeK nowST = Kern::SystemTime();
			__KTRACE_OPT(KPOWER,Kern::Printf("system time: now = 0x%lx(us) wakeup = 0x%lx(us)", nowST, aWakeupST));
			if (aWakeupST < nowST)
				return;
			Int64 deltaSecs = (aWakeupST - nowST) / 1000000;
			if (deltaSecs <= 0)
				return;
			if (deltaSecs + (Int64)nowRTC > (Int64)(KMaxTInt - 2))
				{
				//wakeupRTC = (KMaxTInt - 2); // RTC can't wrap around during standby
				__KTRACE_OPT(KPOWER,Kern::Printf("RTC: now = %d(s) wakeup = %d(s)", nowRTC, KMaxTInt - 2));
				}
			else
				{
				//wakeupRTC = nowRTC + deltaSecs;
				__KTRACE_OPT(KPOWER,Kern::Printf("RTC: now = %d(s) wakeup = %d(s)", nowRTC, nowRTC + deltaSecs));
				}
			}
//		else
//			wakeupRTC = 0;
		//
		// TO DO: (optional)
		//
		// It then uses the calculated value to program the RTC to wakeup the System at the Wakeup
		// time ans sets the CPU and remaining hardware to go to the correponding low power mode. When the 
		// state of the Core and Core Peripherals is not preserved in this mode the following is usually 
		// required:
		//	- save current Core state (current Mode, banked registers for each Mode and Stack Pointer for 
		//	  both current and User Modes
		//	- save MMU state: Control Register, TTB and Domain Access Control
		//	- Flush Dta Cache and drain Write Buffer
		//	- save Core Peripherals state: Interrupt Controller, Pin Function, Bus State and Clock settings
		// SDRAM should be put in self refresh mode. Peripheral devices involved in detection of Wakeup events
		// should be left powered.
		// The Tick timer should be disabled and the current count of this and other System timers shall be
		// saved.
		// On wakeing up the state should be restored from the save state as above. SDRAM shall be brought back
		// under CPU control, The Tick count shall be restored and timers re-enabled.

		// We assume that if a wakeup event occurrs when the CPU is in Standby, this will automatically
		// bring it back from that state. Therefore we stop tracking wakeup events as the Power Manager will
		// complete any pending notifications anyway. When the driver delivers its notification, we just ignore
		// it.
		iWakeupEventsOn = EFalse;		// tracking of wakeup events is now done in hardware
		}
	else
		{
		Kern::Restart(0x80000000);
		}
	}

void DNE1_TBPowerController::IdleTickSuppresionRestore()
    {
#ifdef __SMP__	
    // only one CPU can enter this function
    NETimer& NET = NETimer::Timer(0);
    TUint32 timerWrapped = NET.iGTInterrupt&KNETimerGTIInt_TCI;
    __e32_io_completion_barrier();
    TUint32  timeIn = NET.iTimerCount;
    TUint32 timeSlept = timeIn;

    __PM_ASSERT_DEBUG(NET.iTimerReset == iNextInterrupt);
    
    if (timerWrapped) 
        {
        // We woke up due to a the main timer. If this is case unless we clear the interrupt
        // this result in an extra tick being advanced. We are reprogramming the ISR for a latter
        // activation aligned with the correct phase. For timer based wakeups we wake up a bit early
        // early enough to allow the time needed to repogram the timer for the next edge
        timeSlept+=((iNextInterrupt+KCyclesPerTick)-iOriginalTimerExpire);  // timer wrapped if interrupt is pending
        ClearTimerInterrupt();
        }
	else if	(timeIn >= iOriginalTimerExpire)
		{
		//We woke up after one or more ticks
		timeSlept+=(KCyclesPerTick-iOriginalTimerExpire);		
		}
		
    TUint32 ticksSlept = timeSlept/KCyclesPerTick;
    TUint32 timeToNextInterruptDelta = (ticksSlept+1)*KCyclesPerTick-timeSlept;
	
	if	(timerWrapped==0 && timeIn<iOriginalTimerExpire)
		{
		//We woke up before first tick expired
		ticksSlept=0;
		timeToNextInterruptDelta=iOriginalTimerExpire-timeIn;
		}
	
	
    if (timeToNextInterruptDelta < KMinTimeToTick) 
        {        
        // This should not happen on normal timer expiries as we should be be programmed to wake 
        // well before the next timer expiry which therefore means that ie we need to make sure that
        // wake up times are always larger than this KMinTimeToTick
		// However a WakeupEvent could have resulted in us waking close potential tick		
        // skip a tick
        ticksSlept++;
        timeToNextInterruptDelta +=KCyclesPerTick;
        }
    TUint32 timeToNextInterrupt = timeIn+timeToNextInterruptDelta;

//while(timeToNextInterrupt==KWakeUpBeforeTick);


    NET.iTimerReset = timeToNextInterrupt;
	__e32_io_completion_barrier();
    NTimerQ::Advance(ticksSlept);
    // restart stopped timers used in NKern::Timestamp, in hardware that will be used for
    // product timers of this type would stop when entering low power mode
    NETimer& T1 = NETimer::Timer(1);
	NETimer& T2 = NETimer::Timer(2);

    TUint32 t2 = T2.iTimerCount;
    TUint32 t1 = T1.iTimerCount;

    // because timers at started one after the other
    // there a certain amount of error accumulated in the diffence between them
    // we need to take into account this error level when restarting them
    // so that we can ensure the error does not grow
	// note sleep time cannot exceed 0xffffff00
    TUint32 error = (t1-t2)&0xff;
    TUint32 remainder = 0xffffff00-t1;
    if (remainder > timeSlept) t1+=timeSlept;
    else t1 = timeSlept - remainder;
    T1.iTimerCount = t1;
    T2.iTimerCount += timeSlept + error;
	__e32_io_completion_barrier();
	T1.iTimerCtrl |=  KNETimerCtrl_CAE;	// start   timer 1 first
	__e32_io_completion_barrier();
    T2.iTimerCtrl |=  KNETimerCtrl_CAE;	// start   timer 2
	__e32_io_completion_barrier();

#ifndef __NO_IDLE_HANDLER_PIL__
    PMBTRACE8(KIdleTickSupression,KTimeSleptTimeNextInt,timeSlept,timeToNextInterrupt);
    PMBTRACE8(KIdleTickSupression,KTIcksSlept,ticksSlept,timeToNextInterruptDelta);
    PMBTRACE8(KMisc,0x20,timeIn,timerWrapped);
#endif
#endif	
    }

void DNE1_TBPowerController::IdleTickSuppresionEntry(TUint32 aWakeDelay, TInt aNextTimer)
    {
#ifdef __SMP__		
	NETimer& NET = NETimer::Timer(0);
    	
    TUint32 cyclesInTick = NET.iTimerCount;
	TUint32 cyclesFullTick= NET.iTimerReset;
	__e32_io_completion_barrier();
     
    if (abs_u32diff(cyclesFullTick,cyclesInTick) < KTooCloseToTick || (NET.iGTInterrupt&KNETimerGTIInt_TCI)) 
		return; // to close to edge of tick so we skip this one or even past it
		
    if (aNextTimer > KMaxSleepTicks) aNextTimer = KMaxSleepTicks;
    iNextInterrupt = (aNextTimer)*KCyclesPerTick;//max time we can sleep	for
	if	(iNextInterrupt > (KMaxTUint32-cyclesFullTick))
		return;
	// We need to wakeup just before the next timer expire is due	
	iOriginalTimerExpire=cyclesFullTick;//this is where the current tick would have expired
	iNextInterrupt+=(cyclesFullTick -aWakeDelay);//adjust next interrupt time from where we are now
    
    NET.iTimerReset = iNextInterrupt;
    __e32_io_completion_barrier();	
    NET.iGTInterrupt = KNETimerGTIInt_All;	// clear any pending interrupts
	__e32_io_completion_barrier();
#ifndef __NO_IDLE_HANDLER_PIL__
    PMBTRACE8(KIdleTickSupression,KCyclesInTickCyclesFullTick,cyclesInTick,cyclesFullTick);
    PMBTRACE4(KIdleTickSupression,KNextInterrupt,iNextInterrupt);
#endif    
//TO DO: Review method of setting iPostambleReuired flag
	SCpuIdleHandler* pS = NKern::CpuIdleHandler();
	pS->iPostambleRequired = ETrue;
    // stop timers used in NKern::Timestamp, in hardware that will be used for
    // product timers of this type would stop when entering low power mode
    NETimer& T1 = NETimer::Timer(1);
	NETimer& T2 = NETimer::Timer(2);
    T2.iTimerCtrl &= ~ KNETimerCtrl_CAE;	// clear timer CAE to hold timer value
	__e32_io_completion_barrier();
	T1.iTimerCtrl &= ~ KNETimerCtrl_CAE; // stop timer 1 last (lets error increase
	__e32_io_completion_barrier();       // but this should be ok as it shouldn't exceed 0xff
    iIdleCount++;
#endif	
    }

//-/-/-/-/-/-/-/-/-/ class TNE1_TBPowerController /-/-/-/-/-/-/-/-/-/


EXPORT_C void TNE1_TBPowerController::WakeupEvent()
	{
	if(!iPowerController)
		__PM_PANIC("Power Controller not present");
	else if(iPowerController->iWakeupEventsOn)
		{
		iPowerController->iWakeupEventsOn=EFalse;		// one occurred, no longer track wakeup events
		iPowerController->WakeupEvent();
		}
	}

// NOTE: these are just enabler functions to simulate core retirement
//       they would not stand to any scrutiny as the basis for a proper solution
//       they are just here to allow to test the idle handler for robustness against retirement
//       whilst we wait for a kernel solution
// @pre thread context, interrupt on, kernel unlocked, no fast mutex held
EXPORT_C void TNE1_TBPowerController::RetireCore(TInt aCpu,TRetireEngageCb& aCb)
    {
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__) && defined(SIMULATE_RETIREMENT)
    SRetireCall* call = new SRetireCall(aCpu,aCb);
    if (call && aCpu < NKern::NumberOfCpus()) 
        {
        call->Call();
        }
    else
        {
        
        if (!call) aCb.iResult = KErrNoMemory;
        else 
            {
            aCb.iResult = KErrArgument;
            delete call;
            }
        aCb.iDfc.Enque();
        }
#endif    
    }

// can be called from any core to engage any other core but caller must be outside idle thread
// @pre thread context interrupt on no fast mutex held
EXPORT_C void TNE1_TBPowerController::EngageCore(TInt aCpu, TRetireEngageCb& aCb)
    {
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__) && defined(SIMULATE_RETIREMENT)
    SEngageCall* call = new SEngageCall(aCpu,aCb);
    if (call && aCpu < NKern::NumberOfCpus()) 
        {
        call->Call();
        }
    else
        {
        if (!call) aCb.iResult = KErrNoMemory;
        else
            {
            aCb.iResult = KErrArgument;
            delete call;
            }
        aCb.iDfc.Enque();
        }
#endif
    }


/**
   Idle count is incremented everytime ITS takes place
   @return idle count
*/ 
EXPORT_C TUint TNE1_TBPowerController::IdleCount()
    {
    return iPowerController->iIdleCount;
    }


//-/-/-/-/-/-/-/-/-/ class SRetireCall /-/-/-/-/-/-/-/-/-/

SRetireCall::SRetireCall(TInt aCpu,TRetireEngageCb& aCb)
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__) && defined(SIMULATE_RETIREMENT)
    :iTimer(RetireCoreDfcFn,(TAny*)this,
            TNE1_TBPowerController::iIdleHandler->iRetireEngageQue,0),
     iCpu(aCpu),iCb(aCb),iAllCpusMask(TIdleSupport::AllCpusMask())
#endif
    {};

#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__) && defined(SIMULATE_RETIREMENT)
void SRetireCall::RetireCoreDfcFn(TAny* aParam)
    {
    
    SRetireCall* pC = (SRetireCall*) aParam;
    
    TUint32 cMask = 0x1<<pC->iCpu;
    TUint32 toBeRetired = DNE1_SMPIdleHandler::iRetiredCores|cMask;
    PMBTRACE4(KRetireCore,0x10,toBeRetired);
    if (toBeRetired==pC->iAllCpusMask || (DNE1_SMPIdleHandler::iRetiredCores&cMask) )
        {
        //Make sure we don't retire all cores! at least one should run. Core might also already be retired
        pC->iCb.iResult = KErrArgument;
        pC->iCb.iDfc.Enque();
        delete pC;
        return;
        }
    //Ensure that timer interrupt only hits a cpu that is still active
    // this won't be need when core retiring support is complete in the kernel
    // as it will migrate all interrupts to remaining enaged cores
    // also just for added realism make sure this thread can only run in cores that are still enaged
    TUint32 enaged = (~toBeRetired)&pC->iAllCpusMask;
    TInt targetCpu =  __e32_find_ls1_32(enaged);
    TUint32 targetCpuMask = 0x1<<targetCpu;
    PMBTRACE4(KRetireCore,0x11,targetCpu);
    targetCpuMask <<= ((KIntIdOstMatchMsTimer %4)<<3);
    TUint32 clear = ~(0xff << ((KHwBaseGlobalIntDist%4)<<3));
    GicDistributor* GIC = (GicDistributor*) KHwBaseGlobalIntDist;
    GIC->iTarget[KIntIdOstMatchMsTimer>>2]&=clear;
    __e32_io_completion_barrier();
    GIC->iTarget[KIntIdOstMatchMsTimer>>2]|=targetCpuMask;
    __e32_io_completion_barrier();
    NKern::ThreadSetCpuAffinity(NKern::CurrentThread(),targetCpu);
    TNE1_TBPowerController::iIdleHandler->ResetSyncPoints(); 
    DNE1_SMPIdleHandler::iRetiredCores=toBeRetired;
    PMBTRACE4(KRetireCore,0x12,DNE1_SMPIdleHandler::iRetiredCores);//,DNE1_TBPowerController::iEngagingCores);
    // queue callback
    pC->iCb.iResult = KErrNone;
    pC->iCb.iDfc.Enque();
    delete pC;
    }
#endif

//-/-/-/-/-/-/-/-/-/ class SEngageCall /-/-/-/-/-/-/-/-/-/

SEngageCall::SEngageCall(TInt aCpu,TRetireEngageCb& aCb)
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__) && defined(SIMULATE_RETIREMENT)
    :iDfc(EngageCoreDfcFn,(TAny*)this,
          TNE1_TBPowerController::iIdleHandler->iRetireEngageQue,0),iCpu(aCpu),iCb(aCb)
#endif
    {};

#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__) && defined(SIMULATE_RETIREMENT)
void SEngageCall::EngageCoreDfcFn(TAny* aParam)
    {
    SEngageCall* pC = (SEngageCall*) aParam;
    TUint32 cMask = 0x1<<pC->iCpu;
    PMBTRACE4(KEngageCore,0x10,cMask);
    if ((~DNE1_SMPIdleHandler::iRetiredCores)&cMask )
        {
        //core is already engaged
        pC->iCb.iResult = KErrArgument;
        pC->iCb.iDfc.Enque();
        delete pC;
        return;
        }

    TNE1_TBPowerController::iIdleHandler->ResetSyncPoints();  
    TIdleSupport::MarkCoreEngaged(cMask);
    DNE1_SMPIdleHandler::iRetiredCores&=~cMask;                // This will free calling CPU
    PMBTRACE4(KEngageCore,0x11,DNE1_SMPIdleHandler::iRetiredCores);
    pC->iCb.iResult = KErrNone;
    pC->iCb.iDfc.Enque();
    delete pC;
    }
#endif



TInt BinaryPowerInit();		// the Symbian example Battery Monitor and Power HAL handling

GLDEF_C TInt KernelModuleEntry(TInt aReason)
	{
	if(aReason==KModuleEntryReasonVariantInit0)
		{
		//
		//
		//
		__KTRACE_OPT(KPOWER, Kern::Printf("Starting NE1_TBVariant Resource controller"));
		return KErrNone;
		}
	else if(aReason==KModuleEntryReasonExtensionInit0)
		{
		__KTRACE_OPT(KPOWER, Kern::Printf("Starting NE1_TBVariant power controller"));
		//
		// TO DO: (optional)
		//
		// Start the Kernel-side Battery Monitor and hook a Power HAL handling function.
		// Symbian provides example code for both of the above in \e32\include\driver\binpower.h
		// You may want to write your own versions.
		// The call below starts the example Battery Monitor and hooks the example Power HAL handling function
		// At the end we return an error to make sure that the entry point is not called again with
		// KModuleEntryReasonExtensionInit1 (which would call the constructor of TheResourceManager again)
		//
		TInt r = BinaryPowerInit();
		if (r!= KErrNone)
			__PM_PANIC("Can't initialise Binary Power model");
		DNE1_TBPowerController* c = new DNE1_TBPowerController();
		if(c)
			return KErrGeneral;
		else
			__PM_PANIC("Can't create Power Controller");
		}
	else if(aReason==KModuleEntryReasonExtensionInit1)
		{
        // doesn't get called
		}
	return KErrArgument;
	}

