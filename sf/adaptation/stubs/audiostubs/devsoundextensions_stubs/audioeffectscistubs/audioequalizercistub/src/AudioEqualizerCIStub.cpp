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
* Description: Audio Stubs -  Implementation of the AudioEqualizer effect Custom Interface class
*
*/


#ifdef _DEBUG
#include <e32svr.h>
#endif
#include "AudioEqualizerCI.h"


EXPORT_C CAudioEqualizerCI* CAudioEqualizerCI::NewL(
	CMMFDevSound& aDevSound )
	{
	CAudioEqualizerCI* self = new(ELeave) CAudioEqualizerCI(aDevSound);
	return self;
	}

CAudioEqualizerCI::CAudioEqualizerCI(
	CMMFDevSound& aDevSound )
	:	iDevSound(&aDevSound )
	{
	iAudioEqualizerData.iMindB = -20; // Initial value
	iAudioEqualizerData.iMaxdB = 20; // Initial value

	TEfAudioEqualizerBand band;

//	for (TInt i=1; i<=8; i++)

        for (TInt i=1; i<=5; i++)		{
		band.iBandId = i;
		band.iBandLevel = 1+i;
		band.iBandWidth = 10+i;
		band.iCenterFrequency = 20+i;
		band.iCrossoverFrequency = 30+i;

		iBandsData.Append(band);
		}

	}

EXPORT_C CAudioEqualizerCI* CAudioEqualizerCI::NewL()
	{
	CAudioEqualizerCI* self = new(ELeave) CAudioEqualizerCI();
	return self;
	}

CAudioEqualizerCI::CAudioEqualizerCI()
	{
	iAudioEqualizerData.iMindB = -20; // Initial value
	iAudioEqualizerData.iMaxdB = 20; // Initial value

	TEfAudioEqualizerBand band;

//	for (TInt i=1; i<=8; i++)
        for (TInt i=1; i<=5; i++)
		{
		band.iBandId = i;
		band.iBandLevel = 1+i;
		band.iBandWidth = 10+i;
		band.iCenterFrequency = 20+i;
		band.iCrossoverFrequency = 30+i;

		iBandsData.Append(band);
		}

	}


CAudioEqualizerCI::~CAudioEqualizerCI()
	{
	}

void CAudioEqualizerCI::ApplyL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CAudioEqualizerCI::ApplyL"));
#endif

	if (iObservers.Count() > 0)
		{
		iAudioEqualizerData.iEnabled = ETrue;

		iBandsData[4].iBandLevel = 99;
		iBandsData[4].iBandWidth = 99;
		iBandsData[4].iCenterFrequency = 99;
		iBandsData[4].iCrossoverFrequency = 99;

		iObservers[0]->EffectChanged(this, (TUint8)MAudioEqualizerObserver::KBand5Changed);

		}
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================


// End of File
