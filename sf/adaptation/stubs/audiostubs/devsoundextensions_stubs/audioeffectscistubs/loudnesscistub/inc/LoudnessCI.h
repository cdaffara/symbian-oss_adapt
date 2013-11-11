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
* Description: Audio Stubs -  This is the definition of the Loudness Custom Interface class.
*
*
*/


#include <LoudnessBase.h>
#include <MLoudnessObserver.h>

class CMMFDevSound;

class CLoudnessCI : public CLoudness
	{
	public: // Constructor and Destructor

        /**
        * Two-phased constructor.
        */
		IMPORT_C static CLoudnessCI* NewL(CMMFDevSound& aDevSound);

        /**
        * Destructor.
        */
		virtual ~CLoudnessCI();

		/**
		* From CAudioEffect
        * Apply effect settings
        * @since 3.0
        */
        virtual void ApplyL();

		IMPORT_C static CLoudnessCI* NewL();

	private:

        /**
        * C++ default constructor.
        */
        CLoudnessCI(CMMFDevSound& aDevSound);
        CLoudnessCI();

	private: // Data

		CMMFDevSound* iDevSound;
	};
