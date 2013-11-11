/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  This is the definition of the RestrictedAudioOutput Custom Interface class.
*				 This serves as an example of how adaptation might define the
*				 custom interface class.
*				 The CRestrictedAudioOutputCI object is typically instantiated by a DevSound
*                instance and return to the client when the CustomInterface()
*				 method is invoked.
*
*
*/

#ifndef RESTRICTEDAUDIOOUTPUTCI_H
#define RESTRICTEDAUDIOOUTPUTCI_H

#include <RestrictedAudioOutput.h>
#include <RestrictedAudioOutputMessageTypes.h>
#
class CMMFDevSound;

class CRestrictedAudioOutputCI : public CRestrictedAudioOutput

	{
	public: // Constructor and Destructor

        /**
        * Two-phased constructor.
        */
		IMPORT_C static CRestrictedAudioOutputCI* NewL();

        /**
        * Destructor.
        */
		virtual ~CRestrictedAudioOutputCI();

 	public: // From Base Class

		/**
		*
        *
        * @since 3.2
        */
        virtual TInt Commit();

	private:

        /**
        * By default Symbian 2nd phase constructor is private.
        */
      //  static void ConstructL();
        /**
        * C++ default constructor.
        */

        CRestrictedAudioOutputCI();

	private: // Data


	};
#endif //RESTRICTEDAUDIOOUTPUTCI_H
