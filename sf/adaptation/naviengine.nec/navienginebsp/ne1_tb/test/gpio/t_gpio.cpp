/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


#include <e32test.h>
#include <e32ver.h>
#include <e32def_private.h>
#include "d_gpio.h"

_LIT(testName, "t_gpio");
_LIT(KLddFileName, "d_gpio.ldd");

LOCAL_D	RTest	test(testName);

GLDEF_C TInt E32Main()
	{
	test.Title();
	
	TInt r;

	test.Start(_L("Load Logical Device"));
	r = User::LoadLogicalDevice(KLddFileName);
	test(r == KErrNone || r == KErrAlreadyExists);

	__KHEAP_MARK;

	test.Next(_L("Open Device"));
	RDevice device;
	r = device.Open(RGpio::Name());
	test(r == KErrNone);

	test.Next(_L("Open Logical Channel"));
	RGpio ldd;
	r = ldd.Open();
	test(r == KErrNone);

	test.Next(_L("Call Static Extension"));
	r = ldd.CallStaticExtension();
	test(r == KErrNone);

	test.Next(_L("Close Logical Channel"));
	ldd.Close();

	__KHEAP_MARKEND;

	test.Next(_L("Unload Logical Device"));
	r = User::FreeLogicalDevice(KLddFileName);
	test(r == KErrNone);

	test.End();

	return 0;
	}

