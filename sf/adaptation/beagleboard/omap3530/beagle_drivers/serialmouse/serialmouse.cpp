// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/beagle_drivers/serialmouse/serialmouse.cpp
// Serial mouse driver
//



/**
 @file
 @internalTechnology
*/

#include <e32keys.h>
#include <arm.h>
#include <comm.h>
#include <assp.h>

#include <assp/omap3530_assp/omap3530_uart.h>
//#include <assp/omap3530_assp/omap3530_prm.h>
#include <assp/omap3530_assp/omap3530_prcm.h>
//#include <resourceman.h>

#include "serialmouse.h"

#ifdef USE_SYMBIAN_PRM
_LIT( KName, "SERMOUSE" );
#endif

#ifdef _FRAME_BUFFER_CURSOR_
#	define CURSOR_SIZE 5
#endif

// AndyS Add support for sending keyboard events
#define STDKEY(x)    (0x1FFF&(x))

LOCAL_C TInt halFunction(TAny* aPtr, TInt aFunction, TAny* a1, TAny* a2)
	{
	TSerialMouse* pH=(TSerialMouse*)aPtr;
	return pH->HalFunction(aFunction,a1,a2);
	}

TSerialMouse::TSerialMouse()
				: iKeyDfc( KeyDfcFn, this, Kern::DfcQue0(), 1 ), iState(ENormal),
				iUart( Omap3530Uart::EUart2 )
	{
	Kern::Printf("+TSerialMouse::TSerialMouse");

#	ifdef _FRAME_BUFFER_CURSOR_
	iCursorBuffer = new TUint16[CURSOR_SIZE*CURSOR_SIZE];
#	endif
	
	}

TSerialMouse::~TSerialMouse()
	{
#	ifdef _FRAME_BUFFER_CURSOR_
	delete[] iCursorBuffer;
#	endif
	}
 
TInt TSerialMouse::Create()
	{
	
	GetScreenInfo();
	
	TInt r=Kern::AddHalEntry(EHalGroupMouse, halFunction, this);
	if (r!=KErrNone)
		return r;
	
	Kern::Printf("TSerialMouse initialising");
	
	__KTRACE_OPT(KBOOT,Kern::Printf("+TSerialMouse::Init")) ;
/*
	iDebugPort = Kern::SuperPage().iDebugPort; // Get the debug port number
	Kern::Printf("Debugport=%d", iDebugPort);
	
	if( Arm::EDebugPortJTAG == iDebugPort )
		{
		__KTRACE_OPT(KBOOT,Kern::Printf("-TSerialMouse::Init: JTAG not supported"));
		// We don't want to return an error here, just don't bother to initialize
		return KErrNone;
		}
	else if( 2 != iDebugPort )
		{
		__KTRACE_OPT(KBOOT,Kern::Printf("-TSerialMouse::Init: Only UART3 supported"));
		// We don't want to return an error here, just don't bother to initialize
		return KErrNone;
		}
*/

#ifdef USE_SYMBIAN_PRM
	// Register with the power resource manager
	r = PowerResourceManager::RegisterClient( iPrmClientId, KName );
	__KTRACE_OPT(KBOOT,Kern::Printf("+TSerialMouse::Init:PRM client ID=%x, r=%d", iPrmClientId, r )) ;
	if( r != KErrNone )
		{
		return r;
		}
#endif

	Prcm::SetClockState(iUart.PrcmInterfaceClk(), Prcm::EClkOn);
	Prcm::SetClockState(iUart.PrcmFunctionClk(), Prcm::EClkOn);
	
 	r =Interrupt::Bind( iUart.InterruptId(),Isr,this);
	if ( r < 0 )
 		{
		Kern::Printf("TSerialMouse::Create Cant Bind to Interrupt %d ret %d",iUart.InterruptId(), r );
		return r;
 		}

#ifdef USE_SYMBIAN_PRM
	// Ask power resource manager to turn on clocks to the UART
	// (this could take some time but we're not in any hurry)
	r = PowerResourceManager::ChangeResourceState( iPrmClientId, iUart.PrmFunctionClk(), Prcm::EClkAuto );
	if( r = KErrNone )
		{
		r = PowerResourceManager::ChangeResourceState( iPrmClientId, iUart.PrmInterfaceClk(), Prcm::EClkAuto );
		}

	if( r != KErrNone )
		{
		__KTRACE_OPT(KBOOT, Kern::Printf("+TSerialMouse:PRM ChangeResourceState(clock(s)) failed, client ID=%x, err=%d", iPrmClientId, r));
		return r;
		}
#endif

	iUart.Init();
	iUart.DefineMode( Omap3530Uart::TUart::EUart );
	iUart.SetBaud( Omap3530Uart::TUart::E1200 );
	iUart.SetDataFormat( Omap3530Uart::TUart::E7Data, Omap3530Uart::TUart::E1Stop, Omap3530Uart::TUart::ENone );
	iUart.EnableFifo( Omap3530Uart::TUart::EEnabled, Omap3530Uart::TUart::ETriggerUnchanged, Omap3530Uart::TUart::ETrigger8 );
	iUart.EnableInterrupt( Omap3530Uart::TUart::EIntRhr );
	iUart.Enable();

	Interrupt::Enable(iUart.InterruptId());

	Kern::Printf("TSerialMouse initialised ID=%d", iUart.InterruptId());
	
	return KErrNone;
	}


void TSerialMouse::Isr( TAny* aPtr )
	//
	// Interrupt service routine. Called when we receive an interrupt
	// on the IRQ line it is bound to. If it's a receive, queues DFC
	// to post the event into the event queue.
	//
	{
	TSerialMouse* self = (TSerialMouse*)aPtr;
	
	const TUint iir = Omap3530Uart::IIR::iMem.Read( self->iUart );

	if ( 0 == (iir bitand Omap3530Uart::IIR::IT_PENDING::KMask) )
		{
		const TUint pending = iir bitand Omap3530Uart::IIR::IT_TYPE::KFieldMask;

		// Although the TI datasheet descrivwed IT_TYPE as being an enumerated priority-decoded interrupt
		// it appears to actually be a bitmask of active interrupt sources
		if ( (pending bitand Omap3530Uart::IIR::IT_TYPE::ERHR) || (pending bitand Omap3530Uart::IIR::IT_TYPE::ERxLineStatus) )
			{
			TUint byte = self->iUart.Read();
			
			self->iKey = byte;
			self->iKeyDfc.Add();
			Interrupt::Disable( self->iUart.InterruptId() );
			}
		}
	}
	
void TSerialMouse::KeyDfcFn( TAny* aPtr )
	//
	// DFC function. Just calls inline function KeyDfc()
	//
	{
	((TSerialMouse*)aPtr)->KeyDfc();
	}
	
inline void TSerialMouse::KeyDfc()
	//
	// Processes received characters
	//
	{	
	
	const TUint8 b = iKey;
	if ( b & 1<<6 )
		{
		// Beginning of a new frame
		iByteIndex = 0;
		iB0 = b;
		}
	else if ( iByteIndex == 0 )
		{
		iByteIndex = 1;
		iB1 = b;
		}
	else if ( iByteIndex == 1 )
		{
		iByteIndex = -1;
		iB2 = b;
		
		const TInt8 x_increment = (iB0 & 0x3)<<6 | (iB1 & 0x3f);
		const TInt8 y_increment = (iB0 & 0xC)<<4 | (iB2 & 0x3f);
		const TBool isLeftButtonDown	= iB0& 1<<5;
		const TBool isRightButtonDown	= iB0& 1<<4;
		
		Kern::Printf("Mouse dx=%d  dy=%d lmb=%d", x_increment, y_increment, isLeftButtonDown);
		
#		ifdef _FRAME_BUFFER_CURSOR_
		iLastX = iX;
		iLastY = iY;
#		endif
		
		iX += x_increment; //Scale ( x_increment );
		iY += y_increment; //Scale ( y_increment );
		
		Clip( iX, iY );
		
		//DBG_PRINT4(_L("\nx:%d y:%d (%d,%d)"), x_increment, y_increment, iX, iY );
		
		TBool rightButtonEvent = (isRightButtonDown != iLastRightButtonDown);
		TBool leftButtonEvent = (isLeftButtonDown != iLastLeftButtonDown);
		iLastLeftButtonDown	= isLeftButtonDown;
		iLastRightButtonDown	= isRightButtonDown;
		
		TRawEvent e;
		
		if ( rightButtonEvent )
			{
			if(isRightButtonDown)
				{
				e.Set( isRightButtonDown ? TRawEvent::EButton2Down : TRawEvent::EButton2Up, iX, iY );
				Kern::AddEvent(e);					
				}
			else
				{
				AddKey(EStdKeyApplication0);
				}
			//DBG_PRINT1(_L(" right:%S"), isRightButtonDown?&_L("down"):&_L("up") );
			}
		
		if ( leftButtonEvent )
			{
			e.Set( isLeftButtonDown ? TRawEvent::EButton1Down : TRawEvent::EButton1Up, iX, iY );
			Kern::AddEvent(e);
			//DBG_PRINT1(_L(" left:%S"), isLeftButtonDown?&_L("down"):&_L("up") );
			}
		
		if ( !rightButtonEvent && !leftButtonEvent )
			{
#			ifdef _TRACK_COORDINATES_
			// Reoprt the exact coordinate to the windowserver
			e.Set(TRawEvent::EPointerMove,iX,iY);
#			else
			// Report the change in coordinates to the windowserver
			e.Set(TRawEvent::EPointerMove,x_increment,y_increment);
#			endif
			Kern::AddEvent(e);
			}
		
#		ifdef _FRAME_BUFFER_CURSOR_
		DrawCursor(iX,iY);
#		endif
		
		}
	
	Interrupt::Enable(iUart.InterruptId()); // Can handle new chars now
	} /* end of function - KeyDfc - */


/*
Perform 2:1 scaling on a mosue movement.
*/
TInt TSerialMouse::Scale(const TInt& aVal)
	{
	switch (aVal)
		{
		case 0: return 0;
		case 1:
		case 2: return 1;
		case 3: return 3;
		case 4: return 6;
		case 5: return 9;
		case -1:
		case -2: return -1;
		case -3: return -3;
		case -4: return -6;
		case -5: return -9;
		default: return 2*aVal;
		}
	}

/*
Clip the mouse coordinates into the dimensions of the screen
*/
void TSerialMouse::Clip(TInt& aX, TInt& aY)
	{
	if ( aX < 0 )	aX = 0;
	if ( aX >= iScreenWidth )	aX = iScreenWidth - 1;
	if ( aY < 0 )	aY = 0;
	if ( aY >= iScreenHeight )	aY = iScreenHeight - 1;
	}

void TSerialMouse::GetScreenInfo()
	{
	TScreenInfoV01 screenInfo;
	TPckg<TScreenInfoV01> sI(screenInfo);
	Kern::HalFunction(EHalGroupDisplay,EDisplayHalScreenInfo,(TAny*)&sI,NULL);
	iScreenWidth = screenInfo.iScreenSize.iWidth;
	iScreenHeight = screenInfo.iScreenSize.iHeight;
#	ifdef _FRAME_BUFFER_CURSOR_
	iVideoAddress = (TUint8*)screenInfo.iScreenAddress;
#	endif
	}

#ifdef _FRAME_BUFFER_CURSOR_
void TSerialMouse::Blit(TInt x, TInt y, TUint16 aColour)
	{
	
	TUint16* fb = (TUint16*)iVideoAddress;
	fb += (iLastY * iScreenWidth) + iLastX;
	for (TInt i = 0; i<CURSOR_SIZE; ++i) //row
		{
		
		if ( (iLastY + i) >= iScreenHeight) break;
		
		for (TInt j = 0; j<CURSOR_SIZE; ++j) //column
			{
			
			if ((iLastX+j) < iScreenWidth)
				{
				*fb = iCursorBuffer[i*CURSOR_SIZE + j];
				}
				
			++fb;
			}
		fb += iScreenWidth - CURSOR_SIZE;
		}
	
	
	fb = (TUint16*)iVideoAddress;
	fb += (y * iScreenWidth) + x;
	for (TInt i = 0; i<CURSOR_SIZE; ++i) //row
		{
		
		if ( (y + i) >= iScreenHeight) break;
		
		for (TInt j = 0; j<CURSOR_SIZE; ++j) //column
			{
			
			if ((x+j) < iScreenWidth)
				{
				iCursorBuffer[i*CURSOR_SIZE + j] = *fb;
				*fb = aColour;
				}
				
			++fb;
			}
		fb += iScreenWidth - CURSOR_SIZE;
		}
	
	/*TUint8* fb = iVideoAddress;
	fb += ( (y * iScreenWidth) + x ) * 2;	
	
	TBuf8<256> cur;
	cur.FillZ();
	TUint8* cursor = &cur[0];
	
	for (TInt i = 0; i<cursorSize; ++i)
		{
		
		if ( (y + i) >= iScreenHeight) break;
		
		TInt bytes_to_copy = Min( cursorSize, iScreenWidth*2 - x*2 );
		
		Mem::Copy(fb, cursor, bytes_to_copy );
		
		cursor += cursorSize;
		
		}*/
	
	}

void TSerialMouse::DrawCursor(TInt x, TInt y)
	{	
	TUint16 cursorColour = iLastLeftButtonDown ? 0x1F : 0x00;
	Blit(x, y, cursorColour);
	}
#endif

 
TInt TSerialMouse::HalFunction(TInt aFunction, TAny* a1, TAny* /*a2*/)
	{
	TInt r=KErrNone;
	switch(aFunction)
		{
		case EMouseHalMouseInfo:
			{
			TPckgBuf<TMouseInfoV01> vPckg;
			TMouseInfoV01& xyinfo=vPckg();
			xyinfo.iMouseButtons=2;
			xyinfo.iMouseAreaSize.iWidth=iScreenWidth;
			xyinfo.iMouseAreaSize.iHeight=iScreenHeight;
			xyinfo.iOffsetToDisplay.iX=0;
			xyinfo.iOffsetToDisplay.iY=0;
			Kern::InfoCopy(*(TDes8*)a1,vPckg);
			break;
			}
		default:
			r=KErrNotSupported;
			break;
		}
	return r;
	}

void DoCreate( TAny* aParam )
	{
	TInt r = reinterpret_cast< TSerialMouse* >( aParam )->Create();
	__ASSERT_ALWAYS( r == KErrNone, Kern::Fault( "SERKEY-Cr", r ) );	
	}

// AndyS support for sending keypresses
void TSerialMouse::AddKey( TUint aKey )
	{
	const TUint8 stdKey = STDKEY(aKey);
	
	TRawEvent e;
	
	Kern::Printf("AddKey %d", stdKey);
	
	e.Set( TRawEvent::EKeyDown, stdKey, 0 );
	Kern::AddEvent( e );
	e.Set( TRawEvent::EKeyUp, stdKey, 0 );
	Kern::AddEvent( e );
}


//
// Kernel Extension entry point
//
DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("Starting mouse driver"));
	
	// create mouse driver
	TInt r=KErrNoMemory;
	TSerialMouse* mouse = new TSerialMouse;
	if ( mouse )
		{
		// Because the Power Resource Manager doesn't finish initializing until after it
		// has run a DFC on SvMsgQue, we need to defer our initialization until after that
		// so we know that the PRM is ready for us to use it
		static TDfc createDfc( &DoCreate, NULL );
		new( &createDfc ) TDfc( &DoCreate, mouse, Kern::SvMsgQue(), KMaxDfcPriority-2 );
		createDfc.Enque();
		r = KErrNone;
		}
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Returns %d",r));
	return r;
	}
