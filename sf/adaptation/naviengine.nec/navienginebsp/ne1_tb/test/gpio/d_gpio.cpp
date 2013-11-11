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


#include <naviengine_priv.h>
#include <gpio.h>
#include "d_gpio.h"
#include "d_gpio_dev.h"

#define ETestStaticExtension 0x80000000

_LIT(KGpioDfscQueName, "D_GPIO_DFC");

DECLARE_STANDARD_LDD()
	{
	return new DGpioFactory;
	}

DGpioFactory::DGpioFactory()
	{
	// Set version number for this device
	iVersion=RGpio::VersionRequired();
	iParseMask=0;
	}

TInt DGpioFactory::Install()
	{
	return SetName(&RGpio::Name());
	}

DGpioFactory::~DGpioFactory()
   	{
   	}

void DGpioFactory::GetCaps(TDes8& aDes) const
	{
	// Create a capabilities object
	RGpio::TCaps caps;
	caps.iVersion = iVersion;
	// Write it back to user memory
	Kern::InfoCopy(aDes,(TUint8*)&caps,sizeof(caps));
	}

TInt DGpioFactory::Create(DLogicalChannelBase*& aChannel)
	{
	aChannel=new DGpioChannel;
	if(!aChannel)
		return KErrNoMemory;

	return KErrNone;
	}

DGpioChannel::DGpioChannel()
	{
	iClient=&Kern::CurrentThread();
	iClient->Open();
	}

TInt DGpioChannel::DoCreate(TInt /*aUnit*/, const TDesC8* /*aInfo*/, const TVersion& aVer)
	{
	TDynamicDfcQue* dfcq = NULL; 
	TInt r = Kern::DynamicDfcQCreate(dfcq, 27, KGpioDfscQueName); 

	if(r == KErrNone) 
		{ 
		iDfcQ = dfcq; 
		SetDfcQ(iDfcQ); 
		iMsgQ.Receive(); 
		} 
	return r;
	}

DGpioChannel::~DGpioChannel()
	{
	((TDynamicDfcQue*)iDfcQ)->Destroy();
	Kern::SafeClose((DObject*&)iClient,NULL);
	}

TInt DGpioChannel::RequestUserHandle(DThread* aThread, TOwnerType aType)
	{
	// Make sure that only our client can get a handle
	if (aType!=EOwnerThread || aThread!=iClient)
		return KErrAccessDenied;
	return KErrNone;
	}

void DGpioChannel::HandleMsg(TMessageBase* aMsg)
	{
	TThreadMessage& m=*(TThreadMessage*)aMsg;

	// Get message type
	TInt id=m.iValue;

	// Decode the message type and dispatch it to the relevent handler function...

	if (id==(TInt)ECloseMsg)
		{
		m.Complete(KErrNone, EFalse);
		return;
		}

	if (id==KMaxTInt)
		{
		m.Complete(KErrNone,ETrue);
		return;
		}

	if (id<0)
		{
		TInt r=DoControl(id);
		m.Complete(r, KErrNotSupported);
		}
	else
		{
		// DoControl
		TInt r=DoControl(id);
		m.Complete(r,ETrue);
		}
	}

/**
  Process synchronous 'control' requests
*/
TInt DGpioChannel::DoControl(TInt aFunction)
	{
	TInt r=KErrNone;

	switch (aFunction)
		{
	case RGpio::ECallStaticExtension:
		r = GPIO::StaticExtension(0, ETestStaticExtension, NULL, NULL);
		break;

	default:
		r = KErrNotSupported;
		break;
		}

	return r;
	}

