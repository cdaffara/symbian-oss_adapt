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

#include "ListenerOrientationCI.h"
#include <sounddevice.h>

// -----------------------------------------------------------------------------
// CListenerOrientation::NewL
// Static function for creating an instance of the CListenerOrientationCI object.
// -----------------------------------------------------------------------------
//

EXPORT_C CListenerOrientationCI* CListenerOrientationCI::NewL(
	CMMFDevSound& aDevSound )
	{
	CListenerOrientationCI* self = new(ELeave) CListenerOrientationCI(aDevSound);
	return self;
	}

CListenerOrientationCI::CListenerOrientationCI(
	CMMFDevSound& aDevSound )
	:	iDevSound(&aDevSound)
	{
	iOrientationData.iHeading = 10;
	iOrientationData.iPitch = 10;
	iOrientationData.iRoll = 10;
	iOrientationData.iFrontX = 10;
	iOrientationData.iFrontY = 10;
	iOrientationData.iFrontZ = 10;
	iOrientationData.iAboveX = 10;
	iOrientationData.iAboveY = 10;
	iOrientationData.iAboveZ = 10;

	}

// -----------------------------------------------------------------------------
// CListenerOrientation::NewL
// Static function for creating an instance of the CListenerOrientationCI object.
// -----------------------------------------------------------------------------
//

EXPORT_C CListenerOrientationCI* CListenerOrientationCI::NewL()
	{
	CListenerOrientationCI* self = new(ELeave) CListenerOrientationCI();
	return self;
	}

CListenerOrientationCI::CListenerOrientationCI()
	{
	iOrientationData.iHeading = 10;
	iOrientationData.iPitch = 10;
	iOrientationData.iRoll = 10;
	iOrientationData.iFrontX = 10;
	iOrientationData.iFrontY = 10;
	iOrientationData.iFrontZ = 10;
	iOrientationData.iAboveX = 10;
	iOrientationData.iAboveY = 10;
	iOrientationData.iAboveZ = 10;

	}

// Destructor

CListenerOrientationCI::~CListenerOrientationCI()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CListenerOrientationCI::~CListenerOrientationCI"));
#endif
	}

// -----------------------------------------------------------------------------
// CListenerOrientationCI::ApplyL
//
// Applies the bassboost settings.
// Adaptation must check each settings and take appropriate actions since
// this method might be called after several settings have been made.
// -----------------------------------------------------------------------------
//
void CListenerOrientationCI::ApplyL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CListenerOrientationCI::ApplyL"));
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
		iObservers[0]->EffectChanged(this, (TUint8)MListenerOrientationObserver::KSpecificEffectBase);
		}
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================


// End of File
