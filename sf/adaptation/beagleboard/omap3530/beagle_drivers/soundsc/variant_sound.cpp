/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
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

#include "variant_sound.h"

_LIT(KSoundScPddName, "SoundSc.Beagle");


DECLARE_STANDARD_PDD()
	{
	return new DDriverBeagleSoundScPddFactory;
	}


DDriverBeagleSoundScPddFactory::DDriverBeagleSoundScPddFactory()
	{

	iUnitsMask = ((1 << KSoundScTxUnit0) | (1 << KSoundScRxUnit0));

	iVersion = RSoundSc::VersionRequired();
	}


TInt DDriverBeagleSoundScPddFactory::Install()
	{
	_LIT(KAudioDFC, "AUDIO DFC");
	// Get a pointer to the the McBSP's DFC Queue so that handling of both McBSP callbacks and requests
	// made to the LDD from user mode can be processed in the same thread, to avoid the use of semaphores
	TInt r = Kern::DfcQCreate(iDfcQ, 26, &KAudioDFC);

	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPddFactory::PDD install");

	if(r==KErrNone)
		{
		// All PDD factories must have a unique name
		TInt r = SetName(&KSoundScPddName);
		}

	return r;
	}

void DDriverBeagleSoundScPddFactory::GetCaps(TDes8& /*aDes*/) const
	{
	}


TInt DDriverBeagleSoundScPddFactory::Validate(TInt aUnit, const TDesC8* /*aInfo*/, const TVersion& aVer)
	{
	// Check that the version requested is less than or equal to the version of this PDD
	if (!Kern::QueryVersionSupported(RSoundSc::VersionRequired(), aVer))
		{
		BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPddFactory::Validate KErrNotSup1");
		return KErrNotSupported;
		}

	// Check the unit number specifies either playback or recording
	if ((aUnit != KSoundScTxUnit0) && (aUnit != KSoundScRxUnit0))
		{
		BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPddFactory::Validate KErrNotSup2");
		return KErrNotSupported;
		}

	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPddFactory::Validate KErrNone");
	return KErrNone;
	}

TInt DDriverBeagleSoundScPddFactory::Create(DBase*& aChannel, TInt aUnit, const TDesC8* /*aInfo*/, const TVersion& /*aVer*/)
	{

	DSoundScPdd* pD = NULL;

	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPddFactory::PDD create aUnit %d TxUnitId %d", aUnit, KSoundScTxUnit0);

	// Assume failure
	TInt r = KErrNoMemory;
	aChannel = NULL;

				
	DDriverBeagleSoundScPdd* pTxD = new DDriverBeagleSoundScPdd;

	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPddFactory::TxPdd %d", pTxD);
		
	if (pTxD)
		{
		pD = pTxD;

		// Save a pointer to the factory so that it is accessible by the PDD and call the PDD's
		// second stage constructor
		pTxD->iPhysicalDevice = this;
			
		pTxD->iUnitType = aUnit; // Either KSoundScTxUnit0 or KSoundScRxUnit0 (play or record)
			
		BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPddFactory::TxPdd2 %d", pTxD);
			
		r = pTxD->DoCreate();
			
		BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPddFactory::Create ret %d", r);
			
		}
	
	// If everything succeeded, save a pointer to the PDD.  This should only be done if DoCreate() succeeded,
	// as some LDDs have been known to access this pointer even if Create() returns an error!
	if (r == KErrNone)
		{
		aChannel = pD;
		BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPddFactory::TxPdd set AChannel %d", aChannel);
		}
	else
		{
		delete pD;
		}

	return r;
	}
