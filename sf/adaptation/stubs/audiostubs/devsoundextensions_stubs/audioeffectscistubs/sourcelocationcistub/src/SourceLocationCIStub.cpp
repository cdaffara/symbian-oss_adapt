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
* Description: Audio Stubs -  Implementation of the bassboost effect Custom Interface class
*
*/


#ifdef _DEBUG
#include <e32svr.h> // Needed for RDebug Prints
#endif

#include "SourceLocationCI.h"
#include <sounddevice.h>

// -----------------------------------------------------------------------------
// CSourceLocation::NewL
// Static function for creating an instance of the CSourceLocationCI object.
// -----------------------------------------------------------------------------
//

EXPORT_C CSourceLocationCI* CSourceLocationCI::NewL(
	CMMFDevSound& aDevSound )
	{
	CSourceLocationCI* self = new(ELeave) CSourceLocationCI(aDevSound);
	return self;
	}

CSourceLocationCI::CSourceLocationCI(
	CMMFDevSound& aDevSound )
	:	iDevSound(&aDevSound)
	{
	iLocationData.iXCoordinate = 10;
	iLocationData.iYCoordinate = 10;
	iLocationData.iZCoordinate = 10;
	iLocationData.iAzimuth = 10;
	iLocationData.iElevation = 10;
	iLocationData.iRadius = 10;
	}

// -----------------------------------------------------------------------------
// CSourceLocation::NewL
// Static function for creating an instance of the CSourceLocationCI object.
// -----------------------------------------------------------------------------
//

EXPORT_C CSourceLocationCI* CSourceLocationCI::NewL()
	{
	CSourceLocationCI* self = new(ELeave) CSourceLocationCI();
	return self;
	}

CSourceLocationCI::CSourceLocationCI()
	{
	iLocationData.iXCoordinate = 10;
	iLocationData.iYCoordinate = 10;
	iLocationData.iZCoordinate = 10;
	iLocationData.iAzimuth = 10;
	iLocationData.iElevation = 10;
	iLocationData.iRadius = 10;
	}

// Destructor

CSourceLocationCI::~CSourceLocationCI()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CSourceLocationCI::~CSourceLocationCI"));
#endif
	}

// -----------------------------------------------------------------------------
// CSourceLocationCI::ApplyL
//
// Applies the bassboost settings.
// Adaptation must check each settings and take appropriate actions since
// this method might be called after several settings have been made.
// -----------------------------------------------------------------------------
//
void CSourceLocationCI::ApplyL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CSourceLocationCI::ApplyL"));
#endif

	if ( !HaveUpdateRights() )
		{
		User::Leave(KErrAccessDenied);
		}

	if ( IsEnabled() )
		{
		}

	// The effect change event is simulated by changing the bassboost and
	// sending the observer a message indicating bassboost has changed.
	// This is done for testing only.

	// The intention of this callback is to notify the observer when the bassboost
	// object changes spontaneously. ie the user did not change the settings but
	// event somewhere in the system causes the bassboost object to change state.

	if (iObservers.Count() > 0)
		{
		iObservers[0]->EffectChanged(this, (TUint8)MSourceLocationObserver::KSpecificEffectBase);
		}
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================




// End of File
