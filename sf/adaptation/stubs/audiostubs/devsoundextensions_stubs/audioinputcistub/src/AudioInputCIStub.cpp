/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  AudioInput CI Stub
*
*/

#include <e32svr.h>
#include "AudioInputCI.h"


EXPORT_C CAudioInputCI* CAudioInputCI::NewL(CMMFDevSound& aDevSound)
	{
	CAudioInputCI* self = new(ELeave) CAudioInputCI(aDevSound);
	return self;
	}

EXPORT_C CAudioInputCI* CAudioInputCI::NewL()
	{
	CAudioInputCI* self = new(ELeave) CAudioInputCI();
	return self;
	}

CAudioInputCI::CAudioInputCI(CMMFDevSound& aDevSound) :
	iDevSound(&aDevSound),
	iInputArray(4)
	{
	}

CAudioInputCI::CAudioInputCI() :
	iInputArray(4)
	{
	}

CAudioInputCI::~CAudioInputCI()
	{
	}

CAudioInput::TAudioInputArray CAudioInputCI::AudioInput()
	{
#ifdef _DEBUG
	RDebug::Print(_L("CAudioInputCI::AudioInput"));
#endif
	return iInputArray.Array();
	}

void CAudioInputCI::SetAudioInputL(TAudioInputArray aAudioInputs)
	{
#ifdef _DEBUG
	RDebug::Print(_L("CAudioInputCI::SetAudioInputL"));
#endif
	iInputArray.Reset();
	TInt count = aAudioInputs.Count();


	for(int i=0; i<count; i++)
		{
		TAudioInputPreference audioInput = aAudioInputs[i];
		iInputArray.AppendL(audioInput);
		}
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================




// End of File
