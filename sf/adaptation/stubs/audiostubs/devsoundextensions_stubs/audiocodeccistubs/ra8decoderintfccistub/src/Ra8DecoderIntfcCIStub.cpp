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
* Description: Audio Stubs -  Custom Interface stub object for configuring the RA8 decoder.
*
*/


// INCLUDE FILES
#include <e32debug.h>
#include "Ra8CustomInterfaceCI.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

/**
 * CRa8DecoderIntfcCI::CRa8DecoderIntfcCI
 * C++ default constructor.
 */
CRa8DecoderIntfcCI::CRa8DecoderIntfcCI()
    {
    }

/**
 * CRa8DecoderIntfcCI::ConstructL
 * Symbian 2nd phase constructor.
 */
void CRa8DecoderIntfcCI::ConstructL()
    {
    }

/**
 * CRa8DecoderIntfcCI::NewL
 * Two-phased constructor.
 */
EXPORT_C CRa8DecoderIntfcCI* CRa8DecoderIntfcCI::NewL()
    {
	CRa8DecoderIntfcCI* self = new (ELeave)CRa8DecoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

/**
 * Destructor
 */
EXPORT_C CRa8DecoderIntfcCI::~CRa8DecoderIntfcCI()
    {
    }

/**
 * CRa8DecoderIntfcCI::FrameNumber
 * Returns frame number from the codec.
 * (other items were commented in a header).
 */
EXPORT_C TInt CRa8DecoderIntfcCI::FrameNumber()
    {
    TInt frameNum(125); //any number is good

#ifdef _DEBUG
    RDebug::Print(_L("CRa8DecoderIntfcCI::FrameNumber [%d]"), frameNum);
#endif
	return frameNum;
    }

/*
 * CRa8DecoderIntfcCI::SetInitString
 * Processes received init string.
 * (other items were commented in a header).
 */
EXPORT_C TInt CRa8DecoderIntfcCI::SetInitString(TDesC8& aInitString)
    {
#ifdef _DEBUG
    RDebug::Print(_L("CRa8DecoderIntfcCI::SetInitString"));
#endif

    TBuf<64> buf;
    buf.Copy(aInitString);

#ifdef _DEBUG
    _LIT(KTest, "RECEIVED STRING: [%S]\n");
    RDebug::Print(KTest, &buf);
#endif

    return KErrNone;
    }

// End of File
