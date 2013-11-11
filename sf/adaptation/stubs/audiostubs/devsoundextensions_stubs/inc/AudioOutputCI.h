/*
* Copyright (c) 2005-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  AudioOutput CI
*
*/

#ifndef _AUDIOOUTPUTCI_H__
#define _AUDIOOUTPUTCI_H__

#include <mmfcontrollerframework.h>
#include "AudioOutput.h"

class CMMFDevSound;
class CAudioOutputCustomInterfaceHandler;
class CAudioOutputTimer;

class CAudioOutputCI : public CAudioOutput
	{
	public:
		IMPORT_C static CAudioOutputCI* NewL(CMMFDevSound& aDevSound);
		virtual ~CAudioOutputCI();


		virtual TAudioOutputPreference AudioOutput();
		virtual TAudioOutputPreference DefaultAudioOutput();
		virtual void RegisterObserverL( MAudioOutputObserver& aObserver );
		virtual TBool SecureOutput();
		virtual void SetAudioOutputL( TAudioOutputPreference aAudioOutput = ENoPreference );
		virtual void SetSecureOutputL( TBool aSecure = EFalse );
		virtual void UnregisterObserver( MAudioOutputObserver& aObserver );
		void DefaultChanged();

        IMPORT_C static CAudioOutputCI* NewL();

	private:
		CAudioOutputCI(CMMFDevSound& aDevSound);
        CAudioOutputCI();

	private:
		//CMMFDevSound&							iDevSound;
        CMMFDevSound*							iDevSound;
		TAudioOutputPreference					iOutput;
		TBool									iSecureOutput;
		TAudioOutputPreference					iDefault;
		MAudioOutputObserver*					iObserver;
		CAudioOutputTimer*						iAudioOutputTimer;
		TInt									iCount;

	};


class CAudioOutputTimer : public CTimer
	{
	public:
	CAudioOutputTimer(CAudioOutputCI* aCustomInterface);
	~CAudioOutputTimer();
	void RunL();
	void DoCancel();

	private:
		CAudioOutputCI*				iCustomInterface;
	};

#endif