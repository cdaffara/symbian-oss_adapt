/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -
*
*/



// INCLUDE FILES
#include "SpeechEncoderConfigCI.h"
#include <e32debug.h>

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

// -----------------------------------------------------------------------------
// CSpeechEncoderConfigCI::CSpeechEncoderConfigCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSpeechEncoderConfigCI::CSpeechEncoderConfigCI()
    {

    }

// -----------------------------------------------------------------------------
// CSpeechEncoderConfigCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSpeechEncoderConfigCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CSpeechEncoderConfigCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CSpeechEncoderConfigCI* CSpeechEncoderConfigCI::NewL()
    {
	CSpeechEncoderConfigCI* self = new (ELeave)CSpeechEncoderConfigCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CSpeechEncoderConfigCI::~CSpeechEncoderConfigCI()
    {

    }

// -----------------------------------------------------------------------------
// CSpeechEncoderConfigCI::GetSupportedBitrates
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CSpeechEncoderConfigCI::GetSupportedBitrates(RArray<TUint>& aSupportedBitrates)
    {
    aSupportedBitrates.Reset();
    aSupportedBitrates.Append(8000);
    aSupportedBitrates.Append(16000);
    aSupportedBitrates.Append(44100);
    aSupportedBitrates.Append(48000);
    RDebug::Print(_L("CSpeechEncoderConfigCI::GetSupportedBitrates"));
	return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSpeechEncoderConfigCI::SetBitrate
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CSpeechEncoderConfigCI::SetBitrate(TUint aBitrate)
    {
    iBitrate = aBitrate;
    RDebug::Print(_L("CSpeechEncoderConfigCI::SetBitrate bitrate[%d]"), iBitrate);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSpeechEncoderConfigCI::GetBitrate
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CSpeechEncoderConfigCI::GetBitrate(TUint& aBitrate)
    {
    aBitrate = iBitrate;
    RDebug::Print(_L("CSpeechEncoderConfigCI::GetBitrate bitrate[%d]"), aBitrate);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSpeechEncoderConfigCI::SetVadMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CSpeechEncoderConfigCI::SetVadMode(TBool aVadMode)
    {
    iVadMode = aVadMode;
    RDebug::Print(_L("CSpeechEncoderConfigCI::SetVadMode VAD mode[%d]"), iVadMode);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSpeechEncoderConfigCI::GetVadMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CSpeechEncoderConfigCI::GetVadMode(TBool& aVadMode)
    {
    aVadMode = iVadMode;
    RDebug::Print(_L("CSpeechEncoderConfigCI::GetVadMode VAD mode[%d]"), aVadMode);
    return KErrNone;
    }


