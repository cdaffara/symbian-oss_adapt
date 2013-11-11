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
#include "../naviengine.h"
#include <kernel/kern_priv.h>
#include "uart16550_ne.h"
#include "vserialkeyb.h"

TInt TSerialKeyboard::GetSerialPort(TUint& aBaud)
	{
	// For non-BootLoader images, set the debug port and serial port to be the same port
	aBaud = 115200;
	TInt r = Kern::SuperPage().iDebugPort;
	if (r==0x100 || r==0x101)
		r &= 0xff, aBaud = 230400;
	return r;
	}

