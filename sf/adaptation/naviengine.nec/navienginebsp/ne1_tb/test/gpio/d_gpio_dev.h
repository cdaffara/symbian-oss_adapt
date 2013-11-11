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


#ifndef __D_GPIO_DEV_H__
#define __D_GPIO_DEV_H__

class DGpioFactory : public DLogicalDevice
	{
public:
	DGpioFactory();
	~DGpioFactory();
	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DLogicalChannelBase*& aChannel);
	};

class DGpioChannel : public DLogicalChannel
	{
public:
	DGpioChannel();
	virtual ~DGpioChannel();

	virtual TInt RequestUserHandle(DThread* aThread, TOwnerType aType);
	virtual TInt DoCreate(TInt aUnit, const TDesC8* anInfo, const TVersion& aVer);
	virtual void HandleMsg(TMessageBase* aMsg);
private:
	enum TPanic
		{
		ERequestAlreadyPending = 1
		};
	TInt DoControl(TInt aFunction);
private:
	DThread* iClient;
	};

#endif // __GPIO_DEV_H__

