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
* omap_hrp\h4\hal.cpp
*
*/



#include <kernel/hal_int.h>
#include <u32std.h>

//
TInt GetDisplayNumberOfScreens(TInt /*aDeviceNumber*/, TInt /*aAttrib*/, TBool /*aSet*/, TAny* aInOut)
	{
	TInt err = KErrNone;
	TInt numberOfScreens = 0;
	err = UserSvr::HalFunction(EHalGroupVariant, EVariantHalCurrentNumberOfScreens, &numberOfScreens, NULL);
	if (err == KErrNone)
		{
		*(TInt*)aInOut=numberOfScreens;
		}
	return err;

	}

TInt GetSerialNumber(TInt /*aDeviceNumber*/, TInt /*aAttrib*/, TBool /*aSet*/, TAny*aInOut)
	{
	TInt err = KErrNone;
	TInt serialNumber = 0;
	err = UserSvr::HalFunction(EHalGroupVariant, EVariantHalSerialNumber, &serialNumber, NULL);
	if (err == KErrNone)
		{
		*(TInt*)aInOut=serialNumber;
		}
	return err;

	}

TInt GetInterruptNumber(TInt /*aDeviceNumber*/, TInt /*aAttrib*/, TBool /*aSet*/, TAny*aInOut)
	{
  	TInt err = KErrNone;
	TInt interruptNumber = 0;
	err = UserSvr::HalFunction(EHalGroupVariant, EVariantHalProfilingDefaultInterruptBase, &interruptNumber, NULL);
	if (err == KErrNone)
		{
		*(TInt*)aInOut=interruptNumber;
		}
	return err;

	}

