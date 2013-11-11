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
#include "G729DecoderIntfcCI.h"
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
// CG729DecoderIntfcCI::CG729DecoderIntfcCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CG729DecoderIntfcCI::CG729DecoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CG729DecoderIntfcCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CG729DecoderIntfcCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CG729DecoderIntfcCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CG729DecoderIntfcCI* CG729DecoderIntfcCI::NewL()
    {
	CG729DecoderIntfcCI* self = new (ELeave)CG729DecoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CG729DecoderIntfcCI::~CG729DecoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CG729DecoderIntfcCI::BadLsfNextBuffer
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CG729DecoderIntfcCI::BadLsfNextBuffer()
    {
    RDebug::Print(_L("CG729DecoderIntfcCI::BadLsfNextBuffer"));
    return KErrNone;
    }

