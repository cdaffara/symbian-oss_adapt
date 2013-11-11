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
* Description: Audio Stubs -  Implementation of the RoomLevel effect Custom Interface class
*
*/


#ifdef _DEBUG
#include <e32svr.h>
#endif
#include "RoomLevelCI.h"


EXPORT_C CRoomLevelCI* CRoomLevelCI::NewL(
	CMMFDevSound& aDevSound )
	{
	CRoomLevelCI* self = new(ELeave) CRoomLevelCI(aDevSound);
	return self;
	}

CRoomLevelCI::CRoomLevelCI(
	CMMFDevSound& aDevSound )
	:	iDevSound(&aDevSound )
	{
    iRoomLevelData.iEnvironmentalReverbId = 123456;
	iRoomLevelData.iStreamRoomLevel = 0;
	iRoomLevelData.iStreamMinRoomLevel = 0;
	iRoomLevelData.iStreamMaxRoomLevel = 1000;

	}

EXPORT_C CRoomLevelCI* CRoomLevelCI::NewL()
	{
	CRoomLevelCI* self = new(ELeave) CRoomLevelCI();
	return self;
	}

CRoomLevelCI::CRoomLevelCI()
	{
    iRoomLevelData.iEnvironmentalReverbId = 123456;
	iRoomLevelData.iStreamRoomLevel = 0;
	iRoomLevelData.iStreamMinRoomLevel = 0;
	iRoomLevelData.iStreamMaxRoomLevel = 1000;
	}

CRoomLevelCI::~CRoomLevelCI()
	{
	}

void CRoomLevelCI::ApplyL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CRoomLevelCI::ApplyL"));
#endif

	if (iObservers.Count() > 0)
		{
		iRoomLevelData.iEnabled = ETrue;
		iObservers[0]->EffectChanged(this, (TUint8)MAudioEffectObserver::KEnabled);

		}
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================



// End of File
