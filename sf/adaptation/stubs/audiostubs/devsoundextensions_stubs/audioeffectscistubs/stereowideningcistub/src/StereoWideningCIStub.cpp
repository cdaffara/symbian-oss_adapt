/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  Implementation of the StereoWidening effect Custom Interface class
*
*/


#ifdef _DEBUG
#include <e32svr.h>
#endif
#include "StereoWideningCI.h"


EXPORT_C CStereoWideningCI* CStereoWideningCI::NewL(
	CMMFDevSound& aDevSound )
	{
	CStereoWideningCI* self = new(ELeave) CStereoWideningCI(aDevSound);
	return self;
	}

CStereoWideningCI::CStereoWideningCI(
	CMMFDevSound& aDevSound )
	:	iDevSound(&aDevSound )
	{
	iStereoWideningData.iLevel = 20; // Initial value
	iStereoWideningData.iContinuousLevelSupported = EFalse; // Initial value
	}

EXPORT_C CStereoWideningCI* CStereoWideningCI::NewL()
	{
	CStereoWideningCI* self = new(ELeave) CStereoWideningCI();
	return self;
	}

CStereoWideningCI::CStereoWideningCI()
	{
	iStereoWideningData.iLevel = 20; // Initial value
	iStereoWideningData.iContinuousLevelSupported = EFalse; // Initial value
	}

CStereoWideningCI::~CStereoWideningCI()
	{
	}

void CStereoWideningCI::ApplyL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CStereoWideningCI::ApplyL"));
#endif

	if (iObservers.Count() > 0)
		{
		if(iStereoWideningData.iEnabled)
			{
			iEnabled = iStereoWideningData.iEnabled;
			iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KEnabled);
			}
		else
			{
			iEnabled = iStereoWideningData.iEnabled;
			iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KDisabled);
			}

		}
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================


// End of File
