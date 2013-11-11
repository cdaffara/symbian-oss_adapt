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
* Description: Audio Stubs -  This is the definition of the RoomLevel Custom Interface class.
*
*
*/


#include <RoomLevelBase.h>
#include <MRoomLevelObserver.h>

class CMMFDevSound;

class CRoomLevelCI : public CRoomLevel
	{
	public: // Constructor and Destructor

        /**
        * Two-phased constructor.
        */
		IMPORT_C static CRoomLevelCI* NewL(CMMFDevSound& aDevSound);

        /**
        * Destructor.
        */
		virtual ~CRoomLevelCI();

		/**
		* From CAudioEffect
        * Apply effect settings
        * @since 3.0
        */
        virtual void ApplyL();

		IMPORT_C static CRoomLevelCI* NewL();

	private:

        /**
        * C++ default constructor.
        */
        CRoomLevelCI(CMMFDevSound& aDevSound);
        CRoomLevelCI();

	private: // Data

		CMMFDevSound* iDevSound;
	};
