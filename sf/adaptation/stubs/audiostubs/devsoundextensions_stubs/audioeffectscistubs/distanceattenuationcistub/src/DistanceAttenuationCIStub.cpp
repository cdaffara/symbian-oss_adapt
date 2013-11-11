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
* Description: Audio Stubs -  Implementation of the DistanceAttenuation effect Custom Interface class
*
*/


#ifdef _DEBUG
#include <e32svr.h>
#endif
#include "DistanceAttenuationCI.h"


EXPORT_C CDistanceAttenuationCI* CDistanceAttenuationCI::NewL(
	CMMFDevSound& aDevSound )
	{
	CDistanceAttenuationCI* self = new(ELeave) CDistanceAttenuationCI(aDevSound);
	return self;
	}

CDistanceAttenuationCI::CDistanceAttenuationCI(
	CMMFDevSound& aDevSound )
	:	iDevSound(&aDevSound )
	{
		iDistanceAttenuationData.iRMin = 0;
		iDistanceAttenuationData.iRMax = 100;
		iDistanceAttenuationData.iMuteAfterMax = EFalse;
		iDistanceAttenuationData.iRollOffFactor = 100;
		iDistanceAttenuationData.iRoomRollOffFactor = 100;
		iDistanceAttenuationData.iRollOffFactorMax = 1000;
		iDistanceAttenuationData.iRoomRollOffFactorMax = 1000;

	}

EXPORT_C CDistanceAttenuationCI* CDistanceAttenuationCI::NewL()
	{
	CDistanceAttenuationCI* self = new(ELeave) CDistanceAttenuationCI();
	return self;
	}

CDistanceAttenuationCI::CDistanceAttenuationCI()
	{
		iDistanceAttenuationData.iRMin = 0;
		iDistanceAttenuationData.iRMax = 100;
		iDistanceAttenuationData.iMuteAfterMax = EFalse;
		iDistanceAttenuationData.iRollOffFactor = 100;
		iDistanceAttenuationData.iRoomRollOffFactor = 100;
		iDistanceAttenuationData.iRollOffFactorMax = 1000;
		iDistanceAttenuationData.iRoomRollOffFactorMax = 1000;

	}

CDistanceAttenuationCI::~CDistanceAttenuationCI()
	{
	}

void CDistanceAttenuationCI::ApplyL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CDistanceAttenuationCI::ApplyL"));
#endif

	if (iObservers.Count() > 0)
		{
		if(iDistanceAttenuationData.iEnabled)
			{
			iEnabled = iDistanceAttenuationData.iEnabled;
			iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KEnabled);
			}
		else
			{
			iEnabled = iDistanceAttenuationData.iEnabled;
			iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KDisabled);
			}

		}
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================




// End of File
