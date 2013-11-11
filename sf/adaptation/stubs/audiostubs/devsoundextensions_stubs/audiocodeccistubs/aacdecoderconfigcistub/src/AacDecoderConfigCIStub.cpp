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
#include "AacDecoderConfigCI.h"
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
// CAacDecoderConfigCI::CAacDecoderConfigCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CAacDecoderConfigCI::CAacDecoderConfigCI()
    {

    }

// -----------------------------------------------------------------------------
// CAacDecoderConfigCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CAacDecoderConfigCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CAacDecoderConfigCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CAacDecoderConfigCI* CAacDecoderConfigCI::NewL()
    {
	CAacDecoderConfigCI* self = new (ELeave)CAacDecoderConfigCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CAacDecoderConfigCI::~CAacDecoderConfigCI()
    {

    }

// -----------------------------------------------------------------------------
// CAacDecoderConfigCI::SetAudioConfig
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CAacDecoderConfigCI::SetAudioConfig(TAudioConfig& aAudioConfig)
    {
    TAudioConfig::TAudioObjectType audioObjectType = aAudioConfig.iAudioObjectType;
    RDebug::Print(_L("CAacDecoderConfigCI::SetAudioConfig object type[%d]"), audioObjectType);
    return KErrNone;
    }
