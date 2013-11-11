/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __D_GPIO_H__
#define __D_GPIO_H__

#include <e32def.h>
#include <e32cmn.h>
#include <e32ver.h>

#ifndef __KERNEL_MODE__
#include <e32std.h>
#endif

class RGpio : public RBusLogicalChannel
	{
public:
	class TCaps
		{
	public:
		TVersion iVersion;
		};

public:
	TInt Open();
	TInt CallStaticExtension();
	inline static const TDesC& Name();
	inline static TVersion VersionRequired();
	enum TControl
		{
		ECallStaticExtension = 0x8fff
		};

	// Kernel side LDD channel is a friend
	//friend class DGpioChannel;
	};

inline const TDesC& RGpio::Name()
	{
	_LIT(KGpioName,"GPIO");
	return KGpioName;
	}

inline TVersion RGpio::VersionRequired()
	{
	const TInt KMajorVersionNumber=1;
	const TInt KMinorVersionNumber=0;
	const TInt KBuildVersionNumber=KE32BuildVersionNumber;
	return TVersion(KMajorVersionNumber,KMinorVersionNumber,KBuildVersionNumber);
	}

#ifndef __KERNEL_MODE__

TInt RGpio::Open()
	{
	return DoCreate(Name(),VersionRequired(),KNullUnit,NULL,NULL,EOwnerThread);
	}

TInt RGpio::CallStaticExtension()
	{
	return DoControl(ECallStaticExtension, NULL);
	}

#endif  // !__KERNEL_MODE__

#endif // __GPIO_H__

