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
#include "G729EncoderIntfcCI.h"
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
// CG729EncoderIntfcCI::CG729EncoderIntfcCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CG729EncoderIntfcCI::CG729EncoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CG729EncoderIntfcCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CG729EncoderIntfcCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CG729EncoderIntfcCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CG729EncoderIntfcCI* CG729EncoderIntfcCI::NewL()
    {
	CG729EncoderIntfcCI* self = new (ELeave)CG729EncoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CG729EncoderIntfcCI::~CG729EncoderIntfcCI()
    {

    }


EXPORT_C TInt CG729EncoderIntfcCI::SetVadMode(TBool aVadMode)
    {
    iVadMode = aVadMode;
    RDebug::Print(_L("CG729EncoderIntfcCI::SetVadMode VAD mode[%d]"), iVadMode);
    return KErrNone;
    }

EXPORT_C TInt CG729EncoderIntfcCI::GetVadMode(TBool& aVadMode)
    {
    aVadMode = iVadMode;
    RDebug::Print(_L("CG729EncoderIntfcCI::GetVadMode VAD mode[%d]"), aVadMode);
    return KErrNone;
    }
