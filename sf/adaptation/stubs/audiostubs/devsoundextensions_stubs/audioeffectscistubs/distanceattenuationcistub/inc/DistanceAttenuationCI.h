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
* Description: Audio Stubs -  This is the definition of the DistanceAttenuation Custom Interface class.
*
*
*/


#include <DistanceAttenuationBase.h>
#include <MDistanceAttenuationObserver.h>

class CMMFDevSound;

class CDistanceAttenuationCI : public CDistanceAttenuation
	{
	public: // Constructor and Destructor

        /**
        * Two-phased constructor.
        */
		IMPORT_C static CDistanceAttenuationCI* NewL(CMMFDevSound& aDevSound);

        /**
        * Destructor.
        */
		virtual ~CDistanceAttenuationCI();

		/**
		* From CAudioEffect
        * Apply effect settings
        * @since 3.0
        */
        virtual void ApplyL();

		IMPORT_C static CDistanceAttenuationCI* NewL();

	private:

        /**
        * C++ default constructor.
        */
        CDistanceAttenuationCI(CMMFDevSound& aDevSound);
        CDistanceAttenuationCI();

	private: // Data

		CMMFDevSound* iDevSound;
	};
