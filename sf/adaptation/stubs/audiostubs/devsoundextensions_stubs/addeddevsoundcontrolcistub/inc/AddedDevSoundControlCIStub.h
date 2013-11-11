/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  Custom Interface for AddedDevSoundControl.
*
*/


#ifndef ADDEDDEVSOUNDCONTROLCI_H
#define ADDEDDEVSOUNDCONTROLCI_H

// INCLUDES
#include <e32base.h>
#include <AddedDevSoundControlCI.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Custom Interface for AddedDevSoundControl.
*
*  @lib AddedDevSoundControlCIStub.lib
*  @since S60 3.2
*/
class CAddedDevSoundControlCI : public CBase,
                                public MAddedDevSoundControl
	{
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CAddedDevSoundControlCI* NewL();

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CAddedDevSoundControlCI();

    public: // New functions

    public: // Functions from MAddedDevSoundControl

        /**
        * Sets HW awareness for Pause.
        *
        * @since S60 3.2
        * @param TBool - Toggles HW awareness
        * @return TInt -
        */
        IMPORT_C virtual TInt SetHwAwareness(TBool aHwAware);

        /**
        * Pauses audio playback and flushes buffers.
        *
        * @since S60 3.2
        * @return TInt - Status
        */
        IMPORT_C virtual TInt PauseAndFlush();


    protected:  // New functions

    protected:  // Functions from base classes

	private:

        /**
        * C++ default constructor.
        */
	    CAddedDevSoundControlCI();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

    public:     // Data
    protected:  // Data
    private:    // Data

        TBool iHwAware;

	};

#endif      // ADDEDDEVSOUNDCONTROLCI_H

// End of File
