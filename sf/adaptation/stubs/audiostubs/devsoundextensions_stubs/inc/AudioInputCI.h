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
* Description: Audio Stubs -  AudioInput CI
*
*/

#include "AudioInput.h"

class CMMFDevSound;

class CAudioInputCI : public CAudioInput
	{
	public:
		IMPORT_C static CAudioInputCI* NewL(CMMFDevSound& aDevSound);
		IMPORT_C static CAudioInputCI* NewL();
		virtual ~CAudioInputCI();

		virtual TAudioInputArray AudioInput();
		virtual void SetAudioInputL(TAudioInputArray aAudioInputs);

	private:
		CAudioInputCI(CMMFDevSound& aDevSound);
		CAudioInputCI();

		CMMFDevSound*                         iDevSound;
		CArrayFixFlat<TAudioInputPreference>  iInputArray;
	};
