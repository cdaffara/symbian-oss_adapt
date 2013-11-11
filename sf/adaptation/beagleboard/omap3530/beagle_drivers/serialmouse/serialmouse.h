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
// omap3530\beagle_variant\inc\serialmouse.h
// Header for serial mouse driver
//

#ifndef __VSERIALMOUSE_H__
#define __VSERIALMOUSE_H__

#include <e32cmn.h>
#include <assp/omap3530_assp/omap3530_uart.h>


class TSerialMouse
{
public:
	TSerialMouse();
	~TSerialMouse();
	TInt Create();
	static void Isr(TAny* aPtr);
	TInt HalFunction(TInt aFunction, TAny* a1, TAny* a2);

private:
	static void KeyDfcFn(TAny* aPtr);
	inline void KeyDfc();
	
	TInt Scale(const TInt& aVal);
	void Clip(TInt& aX, TInt& aY);
	void GetScreenInfo();

	// AndyS support for sending keyboard events
	void AddKey( TUint aKey );
	
	enum TState	{ ENormal, EEscapingStart, EEscapingType1, EEscapingType2 };
private:
	TDfc    iKeyDfc;
	TUint   iKey;
	TInt    iDebugPort;
	TState  iState;
	Omap3530Uart::TUart	iUart;
	TUint	iPrmClientId;

private:
	TInt iScreenWidth;
	TInt iScreenHeight;
	TInt iByteIndex;
	TUint8 iB0;
	TUint8 iB1;
	TUint8 iB2;
	TInt iX;
	TInt iY;
	TInt iLastX;
	TInt iLastY;
	TBool iLastLeftButtonDown;
	TBool iLastRightButtonDown;

#ifdef _FRAME_BUFFER_CURSOR_
	void DrawCursor(TInt x, TInt y);
	void Blit(TInt x, TInt y, TUint16 aColour);
	TUint8* iVideoAddress;
	TUint16* iCursorBuffer;
#endif	

}; 


#endif /* __VSERIALMOUSE_H__ */
