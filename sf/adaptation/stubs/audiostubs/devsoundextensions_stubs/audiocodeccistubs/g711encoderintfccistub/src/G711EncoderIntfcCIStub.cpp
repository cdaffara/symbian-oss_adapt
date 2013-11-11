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
#include "G711EncoderIntfcCI.h"
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
// CG711EncoderIntfcCI::CG711EncoderIntfcCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CG711EncoderIntfcCI::CG711EncoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CG711EncoderIntfcCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CG711EncoderIntfcCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CG711EncoderIntfcCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CG711EncoderIntfcCI* CG711EncoderIntfcCI::NewL()
    {
	CG711EncoderIntfcCI* self = new (ELeave)CG711EncoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CG711EncoderIntfcCI::~CG711EncoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CG711EncoderIntfcCI::SetEncoderMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CG711EncoderIntfcCI::SetEncoderMode(TEncodeMode aEncodeMode)
    {
    TBool myEncodeMode = aEncodeMode;
    RDebug::Print(_L("CG711EncoderIntfcCI::SetEncoderMode encodeMode[%d]"), myEncodeMode);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CG711EncoderIntfcCI::SetVadMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CG711EncoderIntfcCI::SetVadMode(TBool aVadMode)
    {
    iVadMode = aVadMode;
    RDebug::Print(_L("CG711EncoderIntfcCI::SetVadMode VAD mode[%d]"), iVadMode);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CG711EncoderIntfcCI::GetVadMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CG711EncoderIntfcCI::GetVadMode(TBool& aVadMode)
    {
    aVadMode = iVadMode;
    RDebug::Print(_L("CG711EncoderIntfcCI::GetVadMode VAD mode[%d]"), aVadMode);
    return KErrNone;
    }

