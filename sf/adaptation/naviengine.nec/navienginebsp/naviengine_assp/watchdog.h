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
* naviengine_assp\naviengine.h
* Definitions for NE1_TBVariant ASSP
*
*/





#ifndef __A32WATCHDOG_H__
#define __A32WATCHDOG_H__
#include <e32const.h>
#include <platform.h>
#include <e32hal.h>
#include <assp.h>
#include <kernel/kern_priv.h>



class TNaviEngineWatchdog
	{
public:
	TNaviEngineWatchdog();
	virtual ~TNaviEngineWatchdog(); // To shut up gcc98r2
	virtual TInt Create();

	/**
	 * Enable/disable external watchdog timer (eWDT)
	 */
	void        SetTimer      (TUint aTimeoutInSeconds);
	static void WatchdogTimer (TAny* aPtr);
private:
	NTimer iWatchdogTimer;         // Timer the kernel will fire to pat the dog
	TUint  iWatchdogTimerPeriodMs; // the period of the timer that pats the dog, note this should be LESS than the actual watchdog timeout
	};

#endif // __A32WATCHDOG_H__
