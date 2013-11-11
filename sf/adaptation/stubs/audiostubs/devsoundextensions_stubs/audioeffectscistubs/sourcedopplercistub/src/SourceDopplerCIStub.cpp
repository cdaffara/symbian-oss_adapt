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
* Description: Audio Stubs -  Implementation of the Source Doppler Effect Custom Interface class
*
*/


#ifdef _DEBUG
#include <e32svr.h> // Needed for RDebug Prints
#endif

#include "SourceDopplerCI.h"
#include <sounddevice.h>

// -----------------------------------------------------------------------------
// CSourceDoppler::NewL
// Static function for creating an instance of the CSourceDopplerCI object.
// -----------------------------------------------------------------------------
//

EXPORT_C CSourceDopplerCI* CSourceDopplerCI::NewL(
	CMMFDevSound& aDevSound )
	{
	CSourceDopplerCI* self = new(ELeave) CSourceDopplerCI(aDevSound);
	return self;
	}

CSourceDopplerCI::CSourceDopplerCI(
	CMMFDevSound& aDevSound )
	:	iDevSound(&aDevSound)
	{
	iDopplerData.iVelocityX = 10;
	iDopplerData.iVelocityY = 10;
	iDopplerData.iVelocityZ = 10;
	iDopplerData.iAzimuth = 20;
	iDopplerData.iElevation = 20;
	iDopplerData.iRadius = 20;
	iDopplerData.iFactor = 5;
	iDopplerData.iMaxFactor = 10;
	}

// -----------------------------------------------------------------------------
// CSourceDoppler::NewL
// Static function for creating an instance of the CSourceDopplerCI object.
// -----------------------------------------------------------------------------
//

EXPORT_C CSourceDopplerCI* CSourceDopplerCI::NewL()
	{
	CSourceDopplerCI* self = new(ELeave) CSourceDopplerCI();
	return self;
	}

CSourceDopplerCI::CSourceDopplerCI()
	{
	iDopplerData.iVelocityX = 10;
	iDopplerData.iVelocityY = 10;
	iDopplerData.iVelocityZ = 10;
	iDopplerData.iAzimuth = 20;
	iDopplerData.iElevation = 20;
	iDopplerData.iRadius = 20;
	iDopplerData.iFactor = 5;
	iDopplerData.iMaxFactor = 10;
	}

// Destructor

CSourceDopplerCI::~CSourceDopplerCI()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CSourceDopplerCI::~CSourceDopplerCI"));
#endif
	}

// -----------------------------------------------------------------------------
// CSourceDopplerCI::ApplyL
//
// Applies the SourceDoppler settings.
// Adaptation must check each settings and take appropriate actions since
// this method might be called after several settings have been made.
// -----------------------------------------------------------------------------
//
void CSourceDopplerCI::ApplyL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CSourceDopplerCI::ApplyL"));
#endif

	if ( !HaveUpdateRights() )
		{
		User::Leave(KErrAccessDenied);
		}

	if (iObservers.Count() > 0)
		{
		if(iDopplerData.iEnabled)
			{
			iEnabled = iDopplerData.iEnabled;
			iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KEnabled);
			}
		else
			{
			iEnabled = iDopplerData.iEnabled;
			iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KDisabled);
			}

		}

	}


// ========================== OTHER EXPORTED FUNCTIONS =========================



// End of File
