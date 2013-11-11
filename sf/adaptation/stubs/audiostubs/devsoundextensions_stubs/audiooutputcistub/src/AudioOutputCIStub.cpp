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
* Description: Audio Stubs -  AudioOutput CI Stub
*
*/

#include <e32svr.h>
//#include "AudioOutputCustomInterfaceHandler.h"
#include "AudioOutputCI.h"
//#include "AudioOutputMessageTypes.h"
#include <MAudioOutputObserver.h>
#include <AudioOutput.h>

EXPORT_C CAudioOutputCI* CAudioOutputCI::NewL(CMMFDevSound& aDevSound)
	{
	CAudioOutputCI* self = new(ELeave) CAudioOutputCI(aDevSound);
	return self;
	}

CAudioOutputCI::CAudioOutputCI(CMMFDevSound& aDevSound) :
	iDevSound(&aDevSound),
	iDefault(EPublic)
	{
	if (!iAudioOutputTimer)
		{
		iAudioOutputTimer = new (ELeave) CAudioOutputTimer(this);
		}
	iAudioOutputTimer->After(5000000);
	}

CAudioOutputCI::~CAudioOutputCI()
	{
	delete iAudioOutputTimer;
	}

CAudioOutput::TAudioOutputPreference CAudioOutputCI::AudioOutput()
	{
	return iOutput;
	}

CAudioOutput::TAudioOutputPreference CAudioOutputCI::DefaultAudioOutput()
	{
	return iDefault;
	}

void CAudioOutputCI::RegisterObserverL( MAudioOutputObserver& aObserver )
	{
	iObserver = &aObserver;
	}

TBool CAudioOutputCI::SecureOutput()
	{
#ifdef _DEBUG
	RDebug::Print(_L("CAudioOutputCI::SecureOutput"));
#endif
	return iSecureOutput;
	}

void CAudioOutputCI::SetAudioOutputL( TAudioOutputPreference aAudioOutput )
	{
#ifdef _DEBUG
	RDebug::Print(_L("CAudioOutputCI::SetAudioOutputL"));
#endif
	iOutput = aAudioOutput;
	}

void CAudioOutputCI::SetSecureOutputL( TBool aSecure )
	{
#ifdef _DEBUG
	RDebug::Print(_L("CAudioOutputCI::SetSecureOutputL"));
#endif
	iSecureOutput = aSecure;
	}

void CAudioOutputCI::UnregisterObserver( MAudioOutputObserver&/* aObserver */)
	{
	iObserver = NULL;
	}

void CAudioOutputCI::DefaultChanged()
	{

	if (iCount % 4 == 0)
		{
		iDefault = EAll;
		}
	else if (iCount % 4 == 1)
		{
		iDefault = ENoOutput;
		}
	else if ( iCount % 4 == 2)
		{
		iDefault = EPrivate;
		}
	else if (iCount % 4 == 3)
		{
		iDefault = EPublic;
		}

	if (iObserver)
		{
		iObserver->DefaultAudioOutputChanged(*this,iDefault);
		}
	iCount++;
	iAudioOutputTimer->After(5000000);

	}

EXPORT_C CAudioOutputCI* CAudioOutputCI::NewL()
	{
	CAudioOutputCI* self = new(ELeave) CAudioOutputCI();
	return self;
	}

CAudioOutputCI::CAudioOutputCI() :
	iDefault(EPublic)
	{
	if (!iAudioOutputTimer)
		{
		iAudioOutputTimer = new (ELeave) CAudioOutputTimer(this);
		}
	iAudioOutputTimer->After(5000000);
	}

CAudioOutputTimer::CAudioOutputTimer(CAudioOutputCI* aCustomInterface) :
								CTimer(0),
								iCustomInterface(aCustomInterface)
	{
    CTimer::ConstructL();
    CActiveScheduler::Add(this);
	}

CAudioOutputTimer::~CAudioOutputTimer()
	{
	Cancel();
	}

void CAudioOutputTimer::RunL()
	{
#ifdef _DEBUG
	RDebug::Print(_L("CAudioOutputTimer::RunL"));
#endif
	iCustomInterface->DefaultChanged();
	}

void CAudioOutputTimer::DoCancel()
	{
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================




// End of File
