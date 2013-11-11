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
* ne1_tb\inc\ne1_tb_power.h
* NE1_TBVariant Power Management Header
* (see also assp.cpp for a discussion on Sleep modes and xyin.cpp for example
* of usage of Resource Manager and Peripheral self power down and interaction
* with Power Controller for Wakeup Events)
*
*/



#ifndef __PM_STD_H__
#define __PM_STD_H__
#include <kernel/kpower.h>
#include "variant.h"
#include <e32btrace.h>
#ifdef __SMP__
#include <arm_gic.h>
#endif
#include <upd35001_timer.h>
#if defined (__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__)
#include <smppower/idlehelper.h>
#include "smpidlehandler.h"
#endif

#define __PM_ASSERT_ALWAYS(aCond) \
	__ASSERT_ALWAYS( (aCond), \
		( \
			Kern::Printf("Assertion '" #aCond "' failed;\nFile: '" __FILE__ "' Line: %d\n", __LINE__), \
			Kern::Fault("Power Management", 1) \
		) )

#define __PM_ASSERT_DEBUG(aCond) \
	__ASSERT_DEBUG( (aCond), \
		( \
			Kern::Printf("Assertion '" #aCond "' failed;\nFile: '" __FILE__ "' Line: %d\n", __LINE__), \
			Kern::Fault("Power Management", 1) \
		) )

class TNE1_TBPowerController;
class DNE1_TBPowerController;

struct TRetireEngageCb
    {
    TRetireEngageCb(TDfcFn aFunction, TAny* aPtr, TInt aPriority)
        :iDfc(aFunction,aPtr,aPriority)
            {};
    TRetireEngageCb(TDfcFn aFunction, TAny* aPtr, TDfcQue* aDfcQ, TInt aPriority)
        :iDfc(aFunction,aPtr,aDfcQ,aPriority)
            {};
    TDfc iDfc;
    TInt iResult;
    TAny* iParam;
    };    



struct SRetireCall
    {
    SRetireCall(TInt aCpu,TRetireEngageCb& aCb);
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__) && defined(SIMULATE_RETIREMENT)
    static void RetireCoreDfcFn(TAny*);
    void Call() { iTimer.OneShot(0);} // use a timer instead of Dfc as call can race with timer ISR
    NTimer iTimer;                    // by using a timer we are kinda sure we will run between ticks
    TInt iCpu;
    TRetireEngageCb& iCb;
    const TUint32 iAllCpusMask;
#endif
    };


struct SEngageCall
    {
    SEngageCall(TInt aCpu,TRetireEngageCb& aCb);
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__) && defined(SIMULATE_RETIREMENT)
    static void EngageCoreDfcFn(TAny*);
    void Call() { iDfc.Enque();}
    TDfc iDfc;
    TInt iCpu;
    TRetireEngageCb& iCb;
#endif
    };

#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__)

class DNE1_TBPowerController;

NONSHARABLE_CLASS(DNE1_SMPIdleHandler) : public DSMPIdleHandler
    {
friend class SRetireCall;
friend class SEngageCall;

public:
    DNE1_SMPIdleHandler(DNE1_TBPowerController* aController);
    TInt Initialise();
    TBool DoEnterIdle(TInt aCpuMask, TInt aStage, volatile TAny* aU);
    TBool GetLowPowerMode(TInt aIdleTime, TInt &aLowPowerMode);
    TBool EnterLowPowerMode(TInt aMode, TInt aCpuMask, TBool aLastCpu);
private:
    void DoRetireCore(TInt aCpu, TLinAddr aReturnPoint);
#ifdef SIMULATE_RETIREMENT
    static void IdleSteal(TAny*);
#endif

private:
#ifdef SIMULATE_RETIREMENT
    static volatile TUint32 iRetiredCores;
    //    static volatile TUint32 iEngagingCores;
    TDfcQue** iIdleStealers;
    TDfc**    iIdleStealDfcs;
    TDfcQue*  iRetireEngageQue;
#endif
    DNE1_TBPowerController* iController;
    };

#endif

//
// TO DO: (mandatory)
//
// Definition of the DPowerController derived class
//
NONSHARABLE_CLASS(DNE1_TBPowerController) : public DPowerController
	{
friend class TNE1_TBPowerController;

public: // from DPowerController
	void CpuIdle();
	void EnableWakeupEvents();
	void AbsoluteTimerExpired();
	void DisableWakeupEvents();
	void PowerDown(TTimeK aWakeupTime);
    void IdleTickSuppresionEntry(TUint32 aWakeDelay, TInt aNextTimer);
    void IdleTickSuppresionRestore();
public:
	DNE1_TBPowerController();
private:
    static TInt ClearTimerInterrupt();      // return pending interrupt or 1023 if non pending
	void DoStandby(TBool aTimed, TUint32 aWakeupRTC);
public:
    static const TUint32 KCyclesPerTick;	// clock cycles per ms tick
    static const TInt KMaxSleepTicks;	// max number of ticks that can be slept
	static const TUint32 KWakeUpBeforeTick;	// The time (in cycles) that we required to before next tick is due to expire (i.e. after idle tick suppression)
	static const TUint32 KTooCloseToTick;	// In cycles abort idle tick suppresion if we very close current tick edge
	static const TUint32 KMinTimeToTick;	// In cyles, if time to next interrupt is less than this value in wakeup
											// ... we need to update tick in idle tick restore
	static const TInt KMinIdleTicks;		// Minimum amount of ticks we wish to enter power saving mode				

private:
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__)
    DNE1_SMPIdleHandler iIdleHandler;
#endif
//     const TUint32 iNumCpus;
// Idle tick supression related variables
	TBool iWakeupEventsOn;
    TUint32 iNextInterrupt;
	TUint32 iOriginalTimerExpire;
    TUint iIdleCount;                     // in SMP increased every time we do ITS
                                            // in unicore, updated on every entry to CpuIdle
    };


//
// If you need to access to the Power Controller from Drivers/Extensions/Variant 
// or access to Resource Manager then define an accessor class as below
//
class TNE1_TBPowerController
	{
friend class SRetireCall;
friend class SEngageCall;
public:
	// to allow Variant/Drivers/Extensions access to Resource Manager
	// used by drivers/extensions to signal a wakeup event to Power Controller
	IMPORT_C static void WakeupEvent();
    IMPORT_C static void RetireCore(TInt aCpu, TRetireEngageCb& aCb);
    IMPORT_C static void EngageCore(TInt aCpu, TRetireEngageCb& aCb);
    IMPORT_C static TUint IdleCount();

	inline static void RegisterPowerController(DNE1_TBPowerController* aPowerController);
private:
	static DNE1_TBPowerController* iPowerController;
#if defined(__SMP__) && !defined(__NO_IDLE_HANDLER_PIL__)
    static DNE1_SMPIdleHandler* iIdleHandler;
#endif
    };



#include "ne1_tb_power.inl"

#endif

