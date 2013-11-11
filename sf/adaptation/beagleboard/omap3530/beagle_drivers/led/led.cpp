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
// omap3530/beagle_drivers/led/led.cpp
//
#include <beagle/beagle_gpio.h>

const TInt KBeatTimeInSeconds = 4;

class LedHeartBeat
	{
public:
	inline LedHeartBeat();
	/*inline ~LedHeartBeat() { iTimer.Cancel(); }*/ // will it ever be destroyed?
	TInt DoCreate();
	static void Beat(TAny *aPtr);
private:
	NTimer iTimer;
	TBool iIsLedOn;
	};

LedHeartBeat::LedHeartBeat() :
	iTimer(Beat, this)
	{
	}

TInt LedHeartBeat::DoCreate()
	{
	TInt r = GPIO::SetPinDirection(KGPIO_LED0, GPIO::EOutput);
	if (r == KErrNone)
		{
		GPIO::SetPinMode(KGPIO_LED0, GPIO::EEnabled);
		GPIO::SetOutputState(KGPIO_LED0, GPIO::ELow);
		iTimer.OneShot(NKern::TimerTicks(KBeatTimeInSeconds * 1000));		
		}
	else
		{
		Kern::Printf("LedHeartBeat: SetPinDirection for LED failed, r %d", r);
		}
	return r;
	}

void LedHeartBeat::Beat(TAny * aPtr)
	{
	LedHeartBeat* b = (LedHeartBeat*)aPtr;
	if(b->iIsLedOn)
		{
		GPIO::SetOutputState(KGPIO_LED0, GPIO::ELow);
		b->iIsLedOn = EFalse;
		}
	else
		{
		GPIO::SetOutputState(KGPIO_LED0, GPIO::EHigh);
		b->iIsLedOn = ETrue;
		}
	b->iTimer.Again(NKern::TimerTicks(KBeatTimeInSeconds * 1000));
	}

// the following macro is defined in led.mmp file..
#ifdef USER_BUTTON_ENTERS_CRASH_DEBUGGER
static void UserButtonIsr(TAny* aPtr)
	{
	//make sure the heartbeat led is OFF when we crash
	GPIO::SetOutputState(KGPIO_LED0, GPIO::ELow);
	Kern::Printf("User button pressed, entering crash debugger..\n");
	Kern::Fault("led.cpp", __LINE__);
	}

TInt SetupUserButton()
	{
	TInt r = GPIO::BindInterrupt(KGPIO_UserButton, UserButtonIsr, NULL);
	if(r != KErrNone)
		{
		Kern::Printf("GPIO::BindInterrupt() failed for button %d, r=%d, (is in use?)",
		             KGPIO_UserButton, r);
		return r;
		}

	r = GPIO::SetInterruptTrigger(KGPIO_UserButton, GPIO::EEdgeRising);
	if(r == KErrNone)
		{
		r = GPIO::SetPinDirection(KGPIO_UserButton, GPIO::EInput);
		if(r == KErrNone)
			{
			GPIO::SetDebounceTime(KGPIO_UserButton, 500);
			GPIO::SetPinMode(KGPIO_UserButton, GPIO::EEnabled);
			r = GPIO::EnableInterrupt(KGPIO_UserButton);
			}
		}
	return r;
	}
#endif

DECLARE_STANDARD_EXTENSION()
	{
	TInt r = KErrNoMemory;
	LedHeartBeat* beat = new LedHeartBeat;
	if(beat)
		{
		r = beat->DoCreate();
		}

#ifdef USER_BUTTON_ENTERS_CRASH_DEBUGGER
	if(r == KErrNone)
		{
		r = SetupUserButton();
		}
#endif
	return r;
	}

