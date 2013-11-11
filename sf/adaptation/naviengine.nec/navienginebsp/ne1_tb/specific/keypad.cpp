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
* ne1_tb\specific\keypad.cpp
* This file contains the implementation of Keypad for the NEC LCD.
*
*/



#include <naviengine.h>
#include <naviengine_priv.h>
#include <e32event.h>
#include <e32event_private.h>
#include <e32keys.h>
#include "lcdgce.h"


// =-==LCD keypad settings and registers==================
const TUint KKeybPollTime = 100;
const TUint KHWKeyRL      = KHwFPGABase + 0x0410;   // Key return line
const TUint KHWKeySR      = KHwFPGABase + 0x0418;   // Key scan register

// Scan codes for buttons: (scan_register_value | (1<<line+4))
// SW1 - is on scan line 1
const TUint KHwKeyCodeArrowUp      = 0x21; // SW1 Up
const TUint KHwKeyCodeArrowRight   = 0x22; // SW1 Right
const TUint KHwKeyCodeArrowLeft    = 0x24; // SW1 Left
const TUint KHwKeyCodeArrowDown    = 0x28; // SW1 Down
const TUint KHwKeyCodeButtonCentre = 0x30; // SW1 Centre press
const TUint KHwKeyCodeButtonLeft   = 0x81; // SW2 - is on scan line 3
const TUint KHwKeyCodeButtonRight  = 0x41; // SW3 - is on scan line 2

const TUint KHwKeyNumArrowUp       = 0;
const TUint KHwKeyNumArrowDown     = 1;
const TUint KHwKeyNumArrowLeft     = 2;
const TUint KHwKeyNumArrowRight    = 3;
const TUint KHwKeyNumButtonCentre  = 4;
const TUint KHwKeyNumButtonLeft    = 5;
const TUint KHwKeyNumButtonRight   = 6;

//#define __KTRACE(s) s
#define __KTRACE(s)

enum TKeyMode
	{
	EKeyModeS60      = 0,
	EKeyModeTechview = 1,

	// This must be the last entry in the list
	EKeyModes
	};

class DNE1Keypad
	{
public:
	DNE1Keypad();
	TInt DoCreate();
	static void GetScanCodes(TAny* aPtr);
	void ProcessKey(TUint aKeyNum, TInt aState);

private:

	inline void AddEventForKey(TUint aKeyCode, TRawEvent::TType aEventType);
	NTimer   iKeyboardTimer;
	TUint    iKeyStates;
	TKeyMode iKeyMode;
	};


/* Macro to detect is the key is shifted */
#define SHIFTED(x)   (0x8000|(x))
#define ISSHIFTED(x) (0x8000&(x))
#define FUNCED(x)    (0x4000|(x))
#define ISFUNCED(x)  (0x4000&(x))
#define CTRLED(x)    (0x2000|(x))
#define ISCTRLED(x)  (0x2000&(x))
#define STDKEY(x)    (0x1FFF&(x))

// modify this table to change the key mapping
const TUint16 convertCode[][EKeyModes] =
	{
		// S60               TechView
		{ EStdKeyUpArrow,    EStdKeyUpArrow,    }, // SW1 Up
		{ EStdKeyDownArrow,  EStdKeyDownArrow,  }, // SW1 Down
		{ EStdKeyLeftArrow,  EStdKeyLeftArrow,  }, // SW1 Left
		{ EStdKeyRightArrow, EStdKeyRightArrow, }, // SW1 Right
		{ EStdKeyDevice3,    EStdKeyEnter,      }, // SW1 Centre Press
		{ EStdKeyDevice0,    EStdKeyEscape,     }, // SW2
		{ EStdKeyDevice1,    EStdKeyMenu,       }, // SW3
	};

void DNE1Keypad::AddEventForKey(TUint aKeyCode, TRawEvent::TType aEventType)
    {
	// convert character to keycode and shift, func, ctrl status
	TUint16 code      = convertCode[aKeyCode][iKeyMode];
	TBool   isShifted = ISSHIFTED(code);
	TBool   isFunced  = ISFUNCED(code);
	TBool   isCtrled  = ISCTRLED(code);
	TUint8  stdKey    = STDKEY(code);
	TRawEvent e;

	if (aEventType == TRawEvent::EKeyDown)
		{
		// post it as a sequence of events
		if (isShifted)
			{
			e.Set(TRawEvent::EKeyDown,EStdKeyRightShift,0);
			Kern::AddEvent(e);
			}
		if (isCtrled)
			{
			e.Set(TRawEvent::EKeyDown,EStdKeyLeftCtrl,0);
			Kern::AddEvent(e);
			}
		if (isFunced)
			{
			e.Set(TRawEvent::EKeyDown,EStdKeyLeftFunc,0);
			Kern::AddEvent(e);
			}
		}

	e.Set(aEventType,stdKey,0);
	Kern::AddEvent(e);

	if (TRawEvent::EKeyUp)
		{
		if (isFunced)
			{
			e.Set(TRawEvent::EKeyUp,EStdKeyLeftFunc,0);
			Kern::AddEvent(e);
			}
		if (isCtrled)
			{
			e.Set(TRawEvent::EKeyUp,EStdKeyLeftCtrl,0);
			Kern::AddEvent(e);
			}
		if (isShifted)
			{
			e.Set(TRawEvent::EKeyUp,EStdKeyRightShift,0);
			Kern::AddEvent(e);
			}
	    }
	}

void DNE1Keypad::ProcessKey(TUint aKeyNum, TInt aState)
	{
	// if the key was pressed last time AND key is now not pressed
	if ((iKeyStates & (1<<aKeyNum)) && (aState == 0))
		{
		// send a key up event
		__KTRACE(Kern::Printf("issue KeyUp for key %d ", aKeyNum));
		AddEventForKey(aKeyNum,TRawEvent::EKeyUp);

		// mark this key as released..
		iKeyStates &= ~(1<<aKeyNum);
		}
	// if the key was NOT pressed last time AND key is now pressed
	else if (!(iKeyStates & (1<<aKeyNum)) && (aState == 1))
		{
		// send a key down event
		__KTRACE(Kern::Printf("issue KeyDown for key %d ", aKeyNum));
		AddEventForKey(aKeyNum,TRawEvent::EKeyDown);

		// mark this key as pressed down..
		iKeyStates |= (1<<aKeyNum);
		}
	}


void DNE1Keypad::GetScanCodes(TAny* aPtr)
	{
	DNE1Keypad* pD=(DNE1Keypad*)aPtr;

	TInt keyRow, key=0;
	for(keyRow = 1; keyRow < 4; ++keyRow)
		{
		AsspRegister::Write16(KHWKeySR, keyRow);        // make line active..
		TUint16 skey = AsspRegister::Read16(KHWKeyRL);  // and scan it, 1=key was pressed..

		if(skey)
			{
			key = (skey | (1<<(keyRow+4)));
			}
		}

	// process each key in turn, passing the current state
	pD->ProcessKey(KHwKeyNumArrowUp,      ((key & KHwKeyCodeArrowUp     )==KHwKeyCodeArrowUp     ));
	pD->ProcessKey(KHwKeyNumArrowRight,   ((key & KHwKeyCodeArrowRight  )==KHwKeyCodeArrowRight  ));
	pD->ProcessKey(KHwKeyNumArrowLeft,    ((key & KHwKeyCodeArrowLeft   )==KHwKeyCodeArrowLeft   ));
	pD->ProcessKey(KHwKeyNumArrowDown,    ((key & KHwKeyCodeArrowDown   )==KHwKeyCodeArrowDown   ));
	pD->ProcessKey(KHwKeyNumButtonCentre, ((key & KHwKeyCodeButtonCentre)==KHwKeyCodeButtonCentre));
	pD->ProcessKey(KHwKeyNumButtonLeft,   ((key & KHwKeyCodeButtonLeft  )==KHwKeyCodeButtonLeft  ));
	pD->ProcessKey(KHwKeyNumButtonRight,  ((key & KHwKeyCodeButtonRight )==KHwKeyCodeButtonRight ));

	pD->iKeyboardTimer.Again(KKeybPollTime);
	}

DNE1Keypad::DNE1Keypad() :
	iKeyboardTimer(GetScanCodes, this),
	iKeyStates(0)
	{
	TUint idMode = AsspRegister::Read32(KHwIDMODE);
	TUint keyMode = (idMode & KHmKeyConfigSwitch) >> KHsKeyConfigSwitch;

	switch (keyMode)
		{
		default:
		case 0: iKeyMode = EKeyModeTechview; break;
		case 1: iKeyMode = EKeyModeS60;      break;
		}
	}

TInt DNE1Keypad::DoCreate()
	{
	iKeyboardTimer.OneShot(KKeybPollTime, ETrue);
	return KErrNone;
	}


DECLARE_STANDARD_EXTENSION()
	{
	// check the display mode - and if LCD is not on
	// (any of ANALOG modes is set)- do not create/start the driver..
	TInt r = KErrNoMemory;
	TInt mode = ReadDipSwitchDisplayMode();
	if(mode == DISPLAY_MODE_ANALOG_VGA                ||
	   mode == DISPLAY_MODE_ANALOG_QVGA_LANDSCAPE_OLD ||
	   mode == DISPLAY_MODE_ANALOG_QVGA_PORTRAIT      ||
	   mode == DISPLAY_MODE_ANALOG_QVGA_LANDSCAPE)
		{
		__KTRACE(Kern::Printf("keypad.cpp: LCD panel not switched on...\nwill not start the driver"));
		r = KErrNone; //  just return..
		}
	else
		{
		DNE1Keypad* pD= new DNE1Keypad;
		if (pD)
			r = pD->DoCreate();
		}

	return r;
	}
