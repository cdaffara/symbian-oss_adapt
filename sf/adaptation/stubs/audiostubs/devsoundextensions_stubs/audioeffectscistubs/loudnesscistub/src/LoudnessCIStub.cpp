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
* Description: Audio Stubs -  Implementation of the Loudness effect Custom Interface class
*
*/


#ifdef _DEBUG
#include <e32svr.h>
#endif
#include "LoudnessCI.h"


EXPORT_C CLoudnessCI* CLoudnessCI::NewL(
	CMMFDevSound& aDevSound )
	{
	CLoudnessCI* self = new(ELeave) CLoudnessCI(aDevSound);
	return self;
	}

CLoudnessCI::CLoudnessCI(
	CMMFDevSound& aDevSound )
	:	iDevSound(&aDevSound )
	{
	}

EXPORT_C CLoudnessCI* CLoudnessCI::NewL()
	{
	CLoudnessCI* self = new(ELeave) CLoudnessCI();
	return self;
	}

CLoudnessCI::CLoudnessCI()
	{
	}

CLoudnessCI::~CLoudnessCI()
	{
	}

void CLoudnessCI::ApplyL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CLoudnessCI::ApplyL"));
#endif

	if (iObservers.Count() > 0)
		{
		if(iLoudnessData.iEnabled)
			{
			iEnabled = iLoudnessData.iEnabled;
			iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KEnabled);
			}
		else
			{
			iEnabled = iLoudnessData.iEnabled;
			iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KDisabled);
			}

		}
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================



// End of File
