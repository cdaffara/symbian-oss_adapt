/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  Custom Interface Stub implementation for RA8 decoder.
*
*/


#ifndef RA8DECODERINTFCCI_H
#define RA8DECODERINTFCCI_H

// INCLUDES
#include <e32base.h>
#include <Ra8CustomInterface.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
//class CCustomInterfaceUtility;
//class MCustomCommand;
//class CMMFDevSound;

// CLASS DECLARATION

/**
*  Custom Interface for RA8 decoder.
*  Stub implementation.
*
*  @lib Ra8DecoderIntfcCI_Stub.lib
*  @since Series 60 3.2
*/
class CRa8DecoderIntfcCI : public CRa8CustomInterface
	{
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
		IMPORT_C static CRa8DecoderIntfcCI* NewL();

        /**
        * Destructor.
        */
		IMPORT_C virtual ~CRa8DecoderIntfcCI();

    public: // New functions

    public: // Functions from base classes

        /**
        * From CRa8CustomInterface
        */
		IMPORT_C virtual TInt FrameNumber();
		IMPORT_C virtual TInt SetInitString(TDesC8& aInitString);

    protected:  // New functions

    protected:  // Functions from base classes

    private:

        /**
        * C++ default constructor.
        */
		CRa8DecoderIntfcCI();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
		void ConstructL();

    public:     // Data
    protected:  // Data
    private:    // Data
    public:     // Friend classes
    protected:  // Friend classes
    private:    // Friend classes

	};

#endif      // RA8DECODERINTFCCI_H

// End of File
