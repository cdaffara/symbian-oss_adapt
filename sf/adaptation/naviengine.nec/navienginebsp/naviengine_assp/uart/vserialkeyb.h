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
* naviengine_assp\uart\vserialkeyb.h
* Header for serial keyboard driver
*
*/



#ifndef __VSERIALKEYB_H__
#define __VSERIALKEYB_H__

const TUint maxSeq = 5;


class TSerialKeyboard
{
public:
	TSerialKeyboard();
	TInt Create();
	static void Isr(TAny* aPtr);

private:
	static void KeyDfcFn(TAny* aPtr);
	inline void KeyDfc();
	void AddConvertedEvent(TUint aKey);
	void AddUnconvertedEvent(TUint aKey);
	TInt GetSerialPort(TUint& aBaud);

	enum TState
		{
		ENormal,
		EEscapingStart = 0x1b,
		EEscapingType1 = 0x4f,
		EEscapingType2 = 0x5b,
		};
private:
	TDfc    iKeyDfc;
	TInt    iKeyboardPort;

	TUint   iSeqNum;
	TUint   iCode[maxSeq];

	TInt    iInterrupt;
	T16550Uart* iUart;
};


#endif /* __VSERIALKEYB_H__ */
