// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/shared/serialkeyb/serialkeyboard.mmp
// Simple serial keyboard implementation for Beagle baseport
//

#include <platform.h>
#include <e32keys.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_uart.h>
//#include <drivers/resourceman.h>

const TInt KMagicCrashValue = 15;


#define SHIFTED(x)   (0x8000|(x))
#define ISSHIFTED(x) (0x8000&(x))
#define CTRLED(x)    (0x2000|(x))
#define ISCTRL(x)  (0x2000&(x))
#define FUNCED(x)    (0x4000|(x))
#define ISFUNC(x)  (0x4000&(x))
#define STDKEY(x)    (0x1FFF&(x))

#if _UI_SOFTKEYS_ENABLED_
static const TUint16 KScanCode[] =
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
	/*41*/	EStdKeyUpArrow,
	/*42*/	EStdKeyDownArrow,
	/*43*/	EStdKeyRightArrow,
	/*44*/	EStdKeyLeftArrow,
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
	/*61*/	EStdKeyYes,
	/*62*/	'B',
	/*63*/	'C',
	/*64*/	EStdKeyNo,
	/*65*/	EStdKeyDevice1,
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
	/*71*/	EStdKeyDevice0,
	/*72*/	'R',
	/*73*/	EStdKeyApplication0,
	/*74*/	'T',
	/*75*/	'U',
	/*76*/	'V',
	/*77*/	EStdKeyDevice3,
	/*78*/	'X',
	/*79*/	'Y',
	/*7a*/	'Z',
	/*7b*/	SHIFTED(EStdKeySquareBracketLeft),
	/*7c*/	SHIFTED(EStdKeyBackSlash),
	/*7d*/	SHIFTED(EStdKeySquareBracketRight),
	/*7e*/	SHIFTED(EStdKeyHash),
	/*7f*/	EKeyDelete
	};

#else
static const TUint16 KScanCode[] =
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
	};

#endif

static const TUint16 KEscapedScanCode[] =
	{
	EStdKeyUpArrow,
	EStdKeyDownArrow,
	EStdKeyRightArrow,
	EStdKeyLeftArrow
	};

const TUint8 KEscapeChar = 0x1b;
const TUint8 KEscapeBase = 0x41;
const TUint8 KEscapeCount = sizeof(KEscapedScanCode) / sizeof(KEscapedScanCode[0]);
const TUint16 KEscapeScanCode = EStdKeyEscape;

NONSHARABLE_CLASS(TSerialKeyboard) : public DBase
	{
public:
	inline TSerialKeyboard();
	TInt Create();

private:
	static void UartIsr( TAny* aParam );
	static void AddKeyDfc( TAny* aParam );
	void AddKey( TUint aKey );
	

private:
	enum TState
		{
		ENormal,
		EEscaping1,
		EEscaping2
		};

	TDfc				iAddKeyDfc;
	Omap3530Uart::TUart	iUart;
	TUint				iPrmClientId;
	TState				iState : 8;
	TUint8				iKey;
	};

inline TSerialKeyboard::TSerialKeyboard()
:	iAddKeyDfc( AddKeyDfc, this, Kern::DfcQue0(), 1 ),
	iUart( Omap3530Assp::DebugPortNumber() ),
	iState( ENormal )
	{
	// Convert the scan rate from milliseconds to nanokernel ticks (normally 1/64 of a second)
	}

TInt TSerialKeyboard::Create()
	{
	TInt r = KErrNone;

	const Omap3530Uart::TUartNumber portNumber( Omap3530Assp::DebugPortNumber() );

	if( portNumber >= 0 )
		{
#ifdef USE_SYMBIAN_PRM
		// Register with the power resource manager
		_LIT( KName, "serkey" );
		r = PowerResourceManager::RegisterClient( iPrmClientId, KName );
		__KTRACE_OPT(KBOOT,Kern::Printf("+TSerialKeyboardl::Create:PRM client ID=%x, err=%d", iPrmClientId, r));
		if( r != KErrNone )
			{
			return r;
			}
#endif

		Prcm::SetClockState(iUart.PrcmInterfaceClk(), Prcm::EClkOn);
		Prcm::SetClockState(iUart.PrcmFunctionClk(), Prcm::EClkOn);

 		r = Interrupt::Bind( iUart.InterruptId(), UartIsr, this );
		if ( r < 0 )
 			{
 			Kern::Printf("TSerialKeyboard Bind r=%d", r);
			return r;
 			}

		Kern::Printf("+TSerialKeyboard::Create bound to interrupt" );

#ifdef USE_SYMBIAN_PRM
		// Ask power resource manager to turn on clocks to the UART
		// (this could take some time but we're not in any hurry)
		r = PowerResourceManager::ChangeResourceState( iPrmClientId, iUart.PrmFunctionClk(), Prcm::EClkOn );
		if( r == KErrNone )
			{
			r = PowerResourceManager::ChangeResourceState( iPrmClientId, iUart.PrmInterfaceClk(), Prcm::EClkOn );
			}

		if( r != KErrNone )
			{
			__KTRACE_OPT(KBOOT, Kern::Printf("+TSerialKeyboardl:PRM ChangeResourceState(clock(s)) failed, client ID=%x, err=%d", iPrmClientId, r));
			return r;
			}
#endif
		// We can assume that the debug output code has already initialized the UART, we just need to prepare it for RX
		iUart.EnableFifo( Omap3530Uart::TUart::EEnabled, Omap3530Uart::TUart::ETriggerUnchanged, Omap3530Uart::TUart::ETrigger8 );
		iUart.EnableInterrupt( Omap3530Uart::TUart::EIntRhr );

		Interrupt::Enable( iUart.InterruptId() );
		}

	return r;
	}

void TSerialKeyboard::UartIsr( TAny* aParam )
	{
	TSerialKeyboard* self = reinterpret_cast<TSerialKeyboard*>( aParam );
	
	const TUint iir = Omap3530Uart::IIR::iMem.Read( self->iUart );

	if ( 0 == (iir bitand Omap3530Uart::IIR::IT_PENDING::KMask) )
		{
		const TUint pending = iir bitand Omap3530Uart::IIR::IT_TYPE::KFieldMask;

		// Although the TI datasheet descrivwed IT_TYPE as being an enumerated priority-decoded interrupt
		// it appears to actually be a bitmask of active interrupt sources
		if ( (pending bitand Omap3530Uart::IIR::IT_TYPE::ERHR) || (pending bitand Omap3530Uart::IIR::IT_TYPE::ERxLineStatus) )
			{
			TUint byte = self->iUart.Read();
			
			if( KMagicCrashValue == byte )
				{
				Kern::Fault( "SERKEY-FORCED", 0 );
				}
			else
				{
				self->iKey = byte;
				self->iAddKeyDfc.Add();
				Interrupt::Disable( self->iUart.InterruptId() );
				}
			}
		}
	}

void TSerialKeyboard::AddKeyDfc( TAny* aParam )
	{
	TSerialKeyboard* self = reinterpret_cast<TSerialKeyboard*>( aParam );

	switch ( self->iState )
		{
	case ENormal:
		if ( self->iKey == KEscapeChar )
			{
			self->iState = EEscaping1;
			}
		else
			{
			self->AddKey( KScanCode[ self->iKey ] );
			}
		break;

	case EEscaping1:
		if ( self->iKey == KEscapeChar )
			{
			self->iState = EEscaping2;
			}
		else
			{
			self->AddKey( KEscapeScanCode );
			self->AddKey( KScanCode[ self->iKey ] );
			self->iState = ENormal;
			}
		break;

	case EEscaping2:
		{
		TInt index = self->iKey - KEscapeBase;
		
		if ( (index >= 0) && (index < KEscapeCount) )
			{
			self->AddKey( KEscapedScanCode[ index ] );
			}
		else
			{
			self->AddKey( KEscapeScanCode );
			self->AddKey( KScanCode[ self->iKey ] );
			}
		self->iState = ENormal;
		}
		break;

	default:
		self->iState = ENormal;
		break;
		};

	Interrupt::Enable( self->iUart.InterruptId() );
	}

void TSerialKeyboard::AddKey( TUint aKey )
	{
	const TBool shifted = ISSHIFTED(aKey);
	const TBool ctrl = ISCTRL(aKey);
	const TBool func = ISFUNC(aKey);
	const TUint8 stdKey = STDKEY(aKey);

	TRawEvent e;

	
	if ( func )
		{
		e.Set( TRawEvent::EKeyDown, EStdKeyRightFunc, 0 );
		Kern::AddEvent( e );
		}
	
	if ( ctrl )
		{
		e.Set( TRawEvent::EKeyDown, EStdKeyRightCtrl, 0 );
		Kern::AddEvent( e );
		}

	if ( shifted )
		{
		e.Set( TRawEvent::EKeyDown, EStdKeyRightShift, 0 );
		Kern::AddEvent( e );
		}

	e.Set( TRawEvent::EKeyDown, stdKey, 0 );
	Kern::AddEvent( e );
	e.Set( TRawEvent::EKeyUp, stdKey, 0 );
	Kern::AddEvent( e );

	if ( shifted )
		{
		e.Set( TRawEvent::EKeyUp, EStdKeyRightShift, 0 );
		Kern::AddEvent( e );
		}

	if ( ctrl )
		{
		e.Set( TRawEvent::EKeyUp, EStdKeyRightCtrl, 0 );
		Kern::AddEvent( e );
		}

	if ( func )
		{
		e.Set( TRawEvent::EKeyUp, EStdKeyRightFunc, 0 );
		Kern::AddEvent( e );
		}
	}


DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Starting serial keyboard driver"));

	TInt r = KErrNoMemory;
	TSerialKeyboard* keyboard = new TSerialKeyboard;
	if ( keyboard )
		{
		r = keyboard->Create();
		}

	__KTRACE_OPT(KEXTENSION,Kern::Printf("Returns %d",r));
	return r;
	}
