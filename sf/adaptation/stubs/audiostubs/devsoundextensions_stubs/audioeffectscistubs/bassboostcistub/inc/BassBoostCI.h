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
* Description: Audio Stubs -  This is the definition of the bassboost Custom Interface class.
*				 This serves as an example of how adaptation might define the
*				 custom interface class.
*				 The CBassBoostCI object is typically instantiated by a DevSound
*                instance and return to the client when the CustomInterface()
*				 method is invoked.
*
*
*/


#include <BassBoostBase.h>
#include <MBassBoostObserver.h>

class CMMFDevSound;

class CBassBoostCI : public CBassBoost
	{
	public: // Constructor and Destructor

        /**
        * Two-phased constructor.
        */
		IMPORT_C static CBassBoostCI* NewL(CMMFDevSound& aDevSound);

        /**
        * Destructor.
        */
		virtual ~CBassBoostCI();

 	public: // From Base Class

		/**
		* From CAudioEffect
        * Apply effect settings
        * @since 3.0
        */
        virtual void ApplyL();

   		/**
        * Two-phased constructor.
        */
		IMPORT_C static CBassBoostCI* NewL();

	private:

        /**
        * C++ default constructor.
        */
        CBassBoostCI(CMMFDevSound& aDevSound);
        CBassBoostCI();

	private: // Data

		CMMFDevSound* iDevSound;
	};
