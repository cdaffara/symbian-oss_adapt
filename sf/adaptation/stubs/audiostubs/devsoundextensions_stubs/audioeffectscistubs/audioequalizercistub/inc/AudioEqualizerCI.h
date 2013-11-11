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
* Description: Audio Stubs -  This is the definition of the AudioEqualizer Custom Interface class.
*
*
*/


#include <AudioEqualizerBase.h>
#include <MAudioEqualizerObserver.h>

class CMMFDevSound;

class CAudioEqualizerCI : public CAudioEqualizer
	{
	public: // Constructor and Destructor

        /**
        * Two-phased constructor.
        */
		IMPORT_C static CAudioEqualizerCI* NewL(CMMFDevSound& aDevSound);

        /**
        * Destructor.
        */
		virtual ~CAudioEqualizerCI();

		/**
		* From CAudioEffect
        * Apply effect settings
        * @since 3.0
        */
        virtual void ApplyL();

		IMPORT_C static CAudioEqualizerCI* NewL();

	private:

        /**
        * C++ default constructor.
        */
        CAudioEqualizerCI(CMMFDevSound& aDevSound);
        CAudioEqualizerCI();

	private: // Data

		CMMFDevSound* iDevSound;
	};
