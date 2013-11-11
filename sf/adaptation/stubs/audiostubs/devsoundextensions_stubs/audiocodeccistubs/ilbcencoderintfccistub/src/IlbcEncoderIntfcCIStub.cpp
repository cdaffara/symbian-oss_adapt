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
#include "IlbcEncoderIntfcCI.h"
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
// CIlbcEncoderIntfcCI::CIlbcEncoderIntfcCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CIlbcEncoderIntfcCI::CIlbcEncoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CIlbcEncoderIntfcCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CIlbcEncoderIntfcCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CIlbcEncoderIntfcCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CIlbcEncoderIntfcCI* CIlbcEncoderIntfcCI::NewL()
    {
	CIlbcEncoderIntfcCI* self = new (ELeave)CIlbcEncoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CIlbcEncoderIntfcCI::~CIlbcEncoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CIlbcEncoderIntfcCI::SetEncoderMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIlbcEncoderIntfcCI::SetEncoderMode(TEncodeMode aEncodeMode)
    {
    TBool myEncodeMode = aEncodeMode;
    RDebug::Print(_L("CIlbcEncoderIntfcCI::SetEncoderMode encodeMode[%d]"), myEncodeMode);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CIlbcEncoderIntfcCI::SetVadMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIlbcEncoderIntfcCI::SetVadMode(TBool aVadMode)
    {
    iVadMode = aVadMode;
    RDebug::Print(_L("CIlbcEncoderIntfcCI::SetVadMode VAD mode[%d]"), iVadMode);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CIlbcEncoderIntfcCI::GetVadMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIlbcEncoderIntfcCI::GetVadMode(TBool& aVadMode)
    {
    aVadMode = iVadMode;
    RDebug::Print(_L("CIlbcEncoderIntfcCI::GetVadMode VAD mode[%d]"), aVadMode);
    return KErrNone;
    }
