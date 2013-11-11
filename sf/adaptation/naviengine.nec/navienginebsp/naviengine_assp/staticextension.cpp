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
#include <dfcs.h>

#ifdef __USE_GPIO_STATIC_EXTENSION__
// test standard extension handler number. *DO NOT USE*
#define KTestStaticExtension 0x80000000
#include <staticextension.h>
#endif

#include <gpio.h>

EXPORT_C TInt GPIO::StaticExtension(TInt /*aId*/, TInt aCmd, TAny* /*aArg1*/, TAny* /*aArg2*/)
	{
	TInt k;
	switch (aCmd)
		{
	case ETestStaticExtension:
		k = KErrNone;
		break;
	default:
		k = KErrNotSupported;
		break;
		}
	return k;
	}

