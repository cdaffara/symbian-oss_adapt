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
* Description: Audio Stubs -  Implementation of the EnvironmentalReverb effect Custom Interface class
*
*/


#ifdef _DEBUG
#include <e32svr.h>
#endif
#include "EnvironmentalReverbCI.h"


EXPORT_C CEnvironmentalReverbCI* CEnvironmentalReverbCI::NewL(
	CMMFDevSound& aDevSound )
	{
	CEnvironmentalReverbCI* self = new(ELeave) CEnvironmentalReverbCI(aDevSound);
	return self;
	}

CEnvironmentalReverbCI::CEnvironmentalReverbCI(
	CMMFDevSound& aDevSound )
	:	iDevSound(&aDevSound )
	{
	iReverbData.iDecayHFRatio = 0;
	iReverbData.iDecayHFRatioMin = 0;
	iReverbData.iDecayHFRatioMax = 10000;
	iReverbData.iDecayTime = 0;
	iReverbData.iDecayTimeMin = 0;
	iReverbData.iDecayTimeMax = 100000;
	iReverbData.iDensity = 0;
	iReverbData.iDiffusion = 0;
	iReverbData.iReflectionsLevel = 0;
	iReverbData.iReflectionLevelMin = 0;
	iReverbData.iReflectionLevelMax = 1000;
	iReverbData.iReflectionsDelay = 0;
	iReverbData.iReflectionsDelayMax = 10000;
	iReverbData.iReverbDelay = 0;
	iReverbData.iReverbDelayMax = 100000;
	iReverbData.iReverbLevel = 0;
	iReverbData.iReverbLevelMin = 0;
	iReverbData.iReverbLevelMax = 1000;
	iReverbData.iRoomHFLevel = 0;
	iReverbData.iRoomHFLevelMin = 0;
	iReverbData.iRoomHFLevelMax = 1000;
	iReverbData.iRoomLevel = 0;
	iReverbData.iRoomLevelMin = 0;
	iReverbData.iRoomLevelMax = 1000;
	iReverbData.iDelayMax = 110000;
	iReverbData.iEnvironmentalReverbId = 54;
	}

EXPORT_C CEnvironmentalReverbCI* CEnvironmentalReverbCI::NewL()
	{
	CEnvironmentalReverbCI* self = new(ELeave) CEnvironmentalReverbCI();
	return self;
	}

CEnvironmentalReverbCI::CEnvironmentalReverbCI()
	{
	iReverbData.iDecayHFRatio = 0;
	iReverbData.iDecayHFRatioMin = 0;
	iReverbData.iDecayHFRatioMax = 10000;
	iReverbData.iDecayTime = 0;
	iReverbData.iDecayTimeMin = 0;
	iReverbData.iDecayTimeMax = 100000;
	iReverbData.iDensity = 0;
	iReverbData.iDiffusion = 0;
	iReverbData.iReflectionsLevel = 0;
	iReverbData.iReflectionLevelMin = 0;
	iReverbData.iReflectionLevelMax = 1000;
	iReverbData.iReflectionsDelay = 0;
	iReverbData.iReflectionsDelayMax = 10000;
	iReverbData.iReverbDelay = 0;
	iReverbData.iReverbDelayMax = 100000;
	iReverbData.iReverbLevel = 0;
	iReverbData.iReverbLevelMin = 0;
	iReverbData.iReverbLevelMax = 1000;
	iReverbData.iRoomHFLevel = 0;
	iReverbData.iRoomHFLevelMin = 0;
	iReverbData.iRoomHFLevelMax = 1000;
	iReverbData.iRoomLevel = 0;
	iReverbData.iRoomLevelMin = 0;
	iReverbData.iRoomLevelMax = 1000;
	iReverbData.iDelayMax = 110000;
	iReverbData.iEnvironmentalReverbId = 54;
	}

CEnvironmentalReverbCI::~CEnvironmentalReverbCI()
	{
	}

void CEnvironmentalReverbCI::ApplyL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CEnvironmentalReverbCI::ApplyL"));
#endif

	if (iObservers.Count() > 0)
		{
		iReverbData.iEnabled = ETrue;
		iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KEnabled);

		}
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================



// End of File
