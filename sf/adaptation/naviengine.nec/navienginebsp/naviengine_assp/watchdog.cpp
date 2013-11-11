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
#include "watchdog.h"

const TInt KWatchdogTimeoutSecs = 5; // the period, in seconds, that the watchdog timer will expire

//
// Pat the watchdog to prevent the board from resetting
//
void TNaviEngineWatchdog::WatchdogTimer(TAny* aPtr)
	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf("watchdogTimer expired"));
	//Kern::Printf("watchdogTimer expired");

	TNaviEngineWatchdog *pDog = (TNaviEngineWatchdog *) aPtr;

	// Pat the Dog now to give us the full period to respond with the timer
	AsspRegister::Write32(KHwWatchdog_WDTINT, 1);

	// Now queue the timer again so that we can pat it next time around
	pDog->iWatchdogTimer.Again(NKern::TimerTicks(pDog->iWatchdogTimerPeriodMs));
	}



//
// Constructor
//
// We only really need the constructor to create the timer instance.
//
TNaviEngineWatchdog::TNaviEngineWatchdog() :
	iWatchdogTimer(WatchdogTimer,this)
	{
	}




// Destructor should never be called
TNaviEngineWatchdog::~TNaviEngineWatchdog()
	{
	__crash();
	}




/**
Perform hardware-dependent initialisation

Called by platform independent layer
*/
TInt TNaviEngineWatchdog::Create()
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("TNaviEngineWatchdog::Create"));
	//Kern::Printf("TNaviEngineWatchdog::Create");
	SetTimer(KWatchdogTimeoutSecs);
	return KErrNone;
	}


//
// Enable/disable the external watchdog timer (eWDT).
//
void TNaviEngineWatchdog::SetTimer(TUint aTimeoutInSeconds)
	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf(">TNaviEngine::SetWatchdogTimer timeout=%d seconds", aTimeoutInSeconds));
	//Kern::Printf(">TNaviEngine::SetWatchdogTimer timeout=%d seconds", aTimeoutInSeconds);

	// Disable eWDT
	AsspRegister::Write32(KHwWatchdog_WDTCNT, 0);

	// If only disabling then we're done
	if (aTimeoutInSeconds > 0)
		{
		// Set the WTD period to something very near the required number 
		// of seconds (this assumes a 66.666MHz clock)
		AsspRegister::Write32(KHwWatchdog_WDTSET, (aTimeoutInSeconds*63)<<20);
		AsspRegister::Write32(KHwWatchdog_WDTTIM, 0);

		// Enable eWDT - start counting
		AsspRegister::Write32(KHwWatchdog_WDTCNT, 1);

		// Pat the Dog now to give us the full period to respond with the timer
		AsspRegister::Write32(KHwWatchdog_WDTINT, 1);

		// We have to decide how often to pat the dog, in seconds.
		// The board will flag a warning after aTimeoutInSeconds, and reset after a further aTimeoutInSeconds.
		// So we have to fire the watchdog at least every aTimeoutInSeconds to prevent the warning
		// I've decided that we'll fire 500ms before the alarm to be sure we've patted the dog in time
		__KTRACE_OPT(KHARDWARE, Kern::Printf("TNaviEngine::SetWatchdogTimer kicking off a timer..."));
		//Kern::Printf("TNaviEngine::SetWatchdogTimer kicking off a timer...");

		iWatchdogTimerPeriodMs = ((aTimeoutInSeconds * 1000) - 500);
		iWatchdogTimer.OneShot(NKern::TimerTicks(iWatchdogTimerPeriodMs));
		}
	}


DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Watchdog: Starting timer service"));
	//Kern::Printf("Watchdog: Starting timer service");

	// create the watchdog handler
	TInt r=KErrNoMemory;
	TNaviEngineWatchdog* pH=new TNaviEngineWatchdog;
	if (pH)
		{
		r=pH->Create();
		}

	__KTRACE_OPT(KEXTENSION,Kern::Printf("Watchdog: Returns %d",r));
	//Kern::Printf("Watchdog: Returns %d",r);
	return r;
	}

