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
* naviengine_assp\uart\vserialkeyb.cpp
* Serial keyboard driver
*
*/



/**
 @file
 @internalTechnology
*/

#include <e32keys.h>
#include <comm.h>
#include <assp.h>
#include "../naviengine_priv.h"
#include "../naviengine.h"
#include "uart16550_ne.h"
#include "vserialkeyb.h"


#define SHIFTED(x)   (0x8000|(x))
#define ISSHIFTED(x) (0x8000&(x))
#define FUNCED(x)    (0x4000|(x))
#define ISFUNCED(x)  (0x4000&(x))
#define CTRLED(x)    (0x2000|(x))
#define ISCTRLED(x)  (0x2000&(x))
#define STDKEY(x)    (0x1FFF&(x))

/*
 *  Definition of the converting table for the receive char
 *  through the serial port.
 *
 */
const TUint16 convertCodeSerial[] =
{
/*00*/	EStdKeyNull,
/*01*/	EStdKeyHome,				// ^A
/*02*/	EStdKeyLeftArrow,			// ^B
/*03*/	EStdKeyEscape,				// ^C
/*04*/	SHIFTED(EStdKeyDownArrow),	// ^D - move window down
/*05*/	EStdKeyEnd,					// ^E
/*06*/	EStdKeyRightArrow,			// ^F
/*07*/	EStdKeyNull,
/*08*/	EStdKeyBackspace,			// ^H - Reserved!
/*09*/	EStdKeyTab,					// ^I - Reserved!
/*0a*/	EStdKeyNull,
/*0b*/	EStdKeyIncContrast,			// ^K
/*0c*/	EStdKeyDecContrast,			// ^L
/*0d*/	EStdKeyEnter,				// ^M - Reserved!
/*0e*/	EStdKeyDownArrow,			// ^N
/*0f*/	EStdKeyNull,				// ^O = instant death
/*10*/	EStdKeyUpArrow,				// ^P
/*11*/	CTRLED(EStdKeyLeftArrow),	// ^Q - make window narrower
/*12*/	CTRLED(FUNCED('5')),		// ^R - rotate windows in text window server
/*13*/	EStdKeyNull,
/*14*/	CTRLED(EStdKeyDownArrow),	// ^T - make window taller
/*15*/	SHIFTED(EStdKeyUpArrow),	// ^U - move window up
/*16*/	EStdKeyNull,
/*17*/	CTRLED(EStdKeyRightArrow),	// ^W - make window wider
/*18*/	SHIFTED(EStdKeyRightArrow),	// ^X - move window right
/*19*/	CTRLED(EStdKeyUpArrow),		// ^Y - make window shorter
/*1a*/	SHIFTED(EStdKeyLeftArrow),	// ^Z - move window left
/*1b*/	EStdKeyEscape,				// ^[ - Reserved!
/*1c*/	EStdKeyNull,
/*1d*/	EStdKeyNull,
/*1e*/	EStdKeyNull,
/*1f*/	EStdKeyNull,
/*20*/	EStdKeySpace,
/*21*/	SHIFTED('1'),		// !
/*22*/	SHIFTED('2'),		// "
/*23*/	EStdKeyHash,		// #
/*24*/	SHIFTED('4'),		// $
/*25*/	SHIFTED('5'),		// %
/*26*/	SHIFTED('7'),		// &
/*27*/	EStdKeySingleQuote,
/*28*/	SHIFTED('9'),		// (
/*29*/	SHIFTED('0'),		// )
/*2a*/	SHIFTED('8'),		// *
/*2b*/	SHIFTED(EStdKeyEquals),	// +
/*2c*/	EStdKeyComma,
/*2d*/	EStdKeyMinus,
/*2e*/	EStdKeyFullStop,
/*2f*/	EStdKeyForwardSlash,
/*30*/	'0',
/*31*/	'1',
/*32*/	'2',
/*33*/	'3',
/*34*/	'4',
/*35*/	'5',
/*36*/	'6',
/*37*/	'7',
/*38*/	'8',
/*39*/	'9',
/*3a*/	SHIFTED(EStdKeySemiColon),	// :
/*3b*/	EStdKeySemiColon,
/*3c*/	SHIFTED(EStdKeyComma),		// <
/*3d*/	EStdKeyEquals,
/*3e*/	SHIFTED(EStdKeyFullStop),	// >
/*3f*/	SHIFTED(EStdKeyForwardSlash),	// ?
/*40*/	SHIFTED(EStdKeySingleQuote),	// @
/*41*/	SHIFTED('A'),
/*42*/	SHIFTED('B'),
/*43*/	SHIFTED('C'),
/*44*/	SHIFTED('D'),
/*45*/	SHIFTED('E'),
/*46*/	SHIFTED('F'),
/*47*/	SHIFTED('G'),
/*48*/	SHIFTED('H'),
/*49*/	SHIFTED('I'),
/*4a*/	SHIFTED('J'),
/*4b*/	SHIFTED('K'),
/*4c*/	SHIFTED('L'),
/*4d*/	SHIFTED('M'),
/*4e*/	SHIFTED('N'),
/*4f*/	SHIFTED('O'),
/*50*/	SHIFTED('P'),
/*51*/	SHIFTED('Q'),
/*52*/	SHIFTED('R'),
/*53*/	SHIFTED('S'),
/*54*/	SHIFTED('T'),
/*55*/	SHIFTED('U'),
/*56*/	SHIFTED('V'),
/*57*/	SHIFTED('W'),
/*58*/	SHIFTED('X'),
/*59*/	SHIFTED('Y'),
/*5a*/	SHIFTED('Z'),
/*5b*/	EStdKeySquareBracketLeft,
/*5c*/	EStdKeyBackSlash,
/*5d*/	EStdKeySquareBracketRight,
/*5e*/	SHIFTED('6'),			// ^
/*5f*/	SHIFTED(EStdKeyMinus),	// _
/*60*/	EStdKeyBacklightToggle,	// Actually `
/*61*/	'A',
/*62*/	'B',
/*63*/	'C',
/*64*/	'D',
/*65*/	'E',
/*66*/	'F',
/*67*/	'G',
/*68*/	'H',
/*69*/	'I',
/*6a*/	'J',
/*6b*/	'K',
/*6c*/	'L',
/*6d*/	'M',
/*6e*/	'N',
/*6f*/	'O',
/*70*/	'P',
/*71*/	'Q',
/*72*/	'R',
/*73*/	'S',
/*74*/	'T',
/*75*/	'U',
/*76*/	'V',
/*77*/	'W',
/*78*/	'X',
/*79*/	'Y',
/*7a*/	'Z',
/*7b*/	SHIFTED(EStdKeySquareBracketLeft),
/*7c*/	SHIFTED(EStdKeyBackSlash),
/*7d*/	SHIFTED(EStdKeySquareBracketRight),
/*7e*/	SHIFTED(EStdKeyHash),
/*7f*/	EKeyDelete
}; /* end of array - convertCodeSerial - */

typedef struct
	{
	TUint8 seq[maxSeq+1];
	TUint32 convertedCode;
	} keyType;

const keyType escapeCodes[] =
{
	// Three escape codes in a row that completes a sequence
	{ {0x1b, 0x4f, 0x50, 0,    0,    0,}, EStdKeyMenu,                       }, // F1 gives menu key
	{ {0x1b, 0x4f, 0x51, 0,    0,    0,}, EStdKeyF2,                         }, // F2
	{ {0x1b, 0x4f, 0x52, 0,    0,    0,}, EStdKeyF3,                         }, // F3
	{ {0x1b, 0x4f, 0x53, 0,    0,    0,}, EStdKeyF4,                         }, // F4
	{ {0x1b, 0x4f, 0x74, 0,    0,    0,}, EStdKeyDevice3,                    }, // F5 gives S60 centre press
	{ {0x1b, 0x4f, 0x75, 0,    0,    0,}, EStdKeyDevice0,                    }, // F6 gives S60 left softkey
	{ {0x1b, 0x4f, 0x76, 0,    0,    0,}, EStdKeyDevice1,                    }, // F7 gives S60 right softkey
	{ {0x1b, 0x4f, 0x6c, 0,    0,    0,}, EStdKeyApplication0,               }, // F8 gives S60 application softkey
	{ {0x1b, 0x4f, 0x77, 0,    0,    0,}, EStdKeyF9,                         }, // F9
	{ {0x1b, 0x4f, 0x78, 0,    0,    0,}, EStdKeyF10,                        }, // F10

	{ {0x1b, 0x5b, 0x41, 0,    0,    0,}, EStdKeyUpArrow,                    }, // Arrow keys
	{ {0x1b, 0x5b, 0x42, 0,    0,    0,}, EStdKeyDownArrow,                  }, // Arrow keys
	{ {0x1b, 0x5b, 0x43, 0,    0,    0,}, EStdKeyRightArrow,                 }, // Arrow keys
	{ {0x1b, 0x5b, 0x44, 0,    0,    0,}, EStdKeyLeftArrow,                  }, // Arrow keys

	// Four escape codes in a row that completes a sequence
	{ {0x1b, 0x5b, 0x31, 0x7e, 0,    0,}, EStdKeyHome,                       }, // Home key
	{ {0x1b, 0x5b, 0x34, 0x7e, 0,    0,}, EStdKeyEnd,                        }, // End key
	{ {0x1b, 0x5b, 0x35, 0x7e, 0,    0,}, EStdKeyPageUp,                     }, // Page Up key
	{ {0x1b, 0x5b, 0x36, 0x7e, 0,    0,}, EStdKeyPageDown,                   }, // Page down key
	{ {0x1b, 0x5b, 0x32, 0x7e, 0,    0,}, EStdKeyInsert,                     }, // Page down key

	// Five escape codes in a row that completes a sequence
	{ {0x1b, 0x5b, 0x32, 0x33, 0x7e, 0,}, EStdKeyF11,                        }, // F11
	{ {0x1b, 0x5b, 0x32, 0x34, 0x7e, 0,}, EStdKeyF12,                        }, // F12

	// end of table - should be all zeros
    { {0,    0,    0,    0,    0,    0,}, 0,                                 }
};

TSerialKeyboard::TSerialKeyboard()
				: iKeyDfc( KeyDfcFn, this, Kern::DfcQue0(), 1 ), iSeqNum(0)
	{
    for (TUint i=0; i<maxSeq; i++)
        {
        iCode[i]=0;
        }
	}
 
TInt TSerialKeyboard::Create()
	{
//	__KTRACE_OPT(KBOOT,Kern::Printf(_L("+TKeyboardSerial::Init"))) ;
	TUint base;
	TUint baud;

	iKeyboardPort = GetSerialPort(baud);

	switch(iKeyboardPort)
		{
	case 1:		base=KHwBaseUart1; break;
	case 2:		base=KHwBaseUart2; break;
	default:	base=KHwBaseUart0;
				iKeyboardPort = 0; //JTAG debug port, use port #0
		}

	__KTRACE_OPT(KBOOT,Kern::Printf("SERIAL KEYBOARD DRIVER: keyboard port=%d", iKeyboardPort ));

	iUart = new T16550Uart;
	if (iUart)
		{
		iUart->iBase = (TUint8*)base;
		iUart->SetIER(0);
		iUart->SetLCR(0);
		iUart->SetFCR(0);
		iUart->SetMCR(0);
		}
	else
		return KErrNoMemory;
	
 	iInterrupt=Interrupt::Bind(KIntIdUart0+iKeyboardPort,Isr,this);
	if (iInterrupt<0)
 	{
		delete iUart;
		return iInterrupt;
 	}
   
	iUart->SetLCR(K16550LCR_Data8|K16550LCR_DLAB);
	iUart->SetBaudRateDivisor((baud==230400) ? KBaudRateDiv_230400 : KBaudRateDiv_default);
	iUart->SetLCR(K16550LCR_Data8);
	iUart->SetFCR(K16550FCR_Enable|K16550FCR_RxReset|K16550FCR_TxReset|K16550FCR_RxTrig8);
	iUart->SetIER(K16550IER_RDAI);// enable receiving data

	Interrupt::Enable(iInterrupt);
    
	return KErrNone;
	}


void TSerialKeyboard::Isr( TAny* aPtr )
	//
	// Interrupt service routine. Called when we receive an interrupt
	// on the IRQ line it is bound to. If it's a receive, queues DFC
	// to post the event into the event queue.
	//
	{
	TSerialKeyboard* self = (TSerialKeyboard*)aPtr;
	T16550Uart& u=*(self->iUart);

	TUint isr=u.ISR();
	if (isr & K16550ISR_NotPending)
		return;
	isr&=K16550ISR_IntIdMask;
 
	if (isr==K16550ISR_RDAI || isr==K16550ISR_RLSI)
		{
		__KTRACE_OPT(KEXTENSION,Kern::Printf("TSerialKeyboard::Isr:RHR"));
		TUint ch=u.RxData();
		if (ch==31)
			__crash();	// CTRL-? = instant death
		if (self->iSeqNum < maxSeq)
			{
			self->iCode[self->iSeqNum] = ch;
			self->iSeqNum++;
			self->iKeyDfc.Add();
			Interrupt::Disable(self->iInterrupt); // Can only handle one char at a time
			}
		}
	}
	
void TSerialKeyboard::KeyDfcFn( TAny* aPtr )
	//
	// DFC function. Just calls inline function KeyDfc()
	//
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("TSerialKeyboard::KeyDfcFn"));
	((TSerialKeyboard*)aPtr)->KeyDfc();
	}

void TSerialKeyboard::AddConvertedEvent(TUint aKey)
	{
	// split aKey into keycode and shift, func, ctrl status
	TBool isShifted = ISSHIFTED(aKey);
	TBool isFunced = ISFUNCED(aKey);
	TBool isCtrled = ISCTRLED(aKey);
	TUint8 stdKey = STDKEY(aKey);

	// post it as a sequence of events
	TRawEvent e;
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

	e.Set(TRawEvent::EKeyDown,stdKey,0);
	Kern::AddEvent(e);

	e.Set(TRawEvent::EKeyUp,stdKey,0);
	Kern::AddEvent(e);

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
	
void TSerialKeyboard::AddUnconvertedEvent(TUint aKey)
	{
	TUint16 convertedKey = convertCodeSerial[aKey];
	AddConvertedEvent(convertedKey);
	}
	
inline void TSerialKeyboard::KeyDfc()
	//
	// Processes received characters
	//
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("KEY: iSeqNum=%x, key=%x", iSeqNum, iCode[iSeqNum-1]));

	switch (iSeqNum)
		{
	case 1:
		if (iCode[0] != EEscapingStart)
			{
			// Unknown escape sequence - just pass chars as normal events
			AddUnconvertedEvent(iCode[0]);
			iSeqNum = 0;
			}
		break;
	case 2:
		if ((iCode[1] != EEscapingType1) && iCode[1] != EEscapingType2)
			{
			// Unknown escape sequence - just pass chars as normal events
			AddUnconvertedEvent(iCode[0]);
			AddUnconvertedEvent(iCode[1]);
			iSeqNum = 0;
			}
		break;
	case 3:
	case 4:
	case 5:
		{
		TUint escCodeindex = 0; // index into the escape code list
		TUint seqIndex     = 0; // index into the current code sequence
        TUint bestMatch    = 0; // keep a track of the best number of matches so far
		while (escapeCodes[escCodeindex].seq[0] != 0)
			{
			for (seqIndex = 0; seqIndex<iSeqNum; seqIndex++)
				{
				if (iCode[seqIndex] != escapeCodes[escCodeindex].seq[seqIndex])
					{
					break; // out of for loop
					}
				}
			if (seqIndex > bestMatch)
				{
				bestMatch = seqIndex;
				}
			if (escapeCodes[escCodeindex].seq[seqIndex] == 0)
				{
				AddConvertedEvent(escapeCodes[escCodeindex].convertedCode);
				iSeqNum = 0;
				break; // out of while loop
				}
			escCodeindex++;
			}

		if (  (bestMatch < iSeqNum) // if we couldn't match all numbers in the sequence so far, this must not be a valid sequence
		   || (iSeqNum == maxSeq)   // if we reached the max number of codes in a sequence, this must also not be a valid sequence
		   )
			{
			if (escapeCodes[escCodeindex].seq[0] == 0)
				{
				// Unknown escape sequence - just pass chars as normal events
				for (TUint i=0; i < iSeqNum; i++)
					{
					AddUnconvertedEvent(iCode[i]);
					}
				}
			iSeqNum = 0;
			}
		}
		break;
	default:
		// Should never reach here!
		iSeqNum = 0;
		break;
		};
	Interrupt::Enable(iInterrupt); // Can handle new chars now
	} /* end of function - KeyDfc - */

//
// Kernel Extension entry point
//
DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Starting keyboard driver"));

	// create keyboard driver
	TInt r=KErrNoMemory;
	TSerialKeyboard* keyboard = new TSerialKeyboard;
	if ( keyboard )
		{
		r=keyboard->Create();
		}

	__KTRACE_OPT(KEXTENSION,Kern::Printf("Returns %d",r));
	return r;
	}
