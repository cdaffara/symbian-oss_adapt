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
#include "G711DecoderIntfcCI.h"
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
// CG711DecoderIntfcCI::CG711DecoderIntfcCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CG711DecoderIntfcCI::CG711DecoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CG711DecoderIntfcCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CG711DecoderIntfcCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CG711DecoderIntfcCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CG711DecoderIntfcCI* CG711DecoderIntfcCI::NewL()
    {
	CG711DecoderIntfcCI* self = new (ELeave)CG711DecoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CG711DecoderIntfcCI::~CG711DecoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CG711DecoderIntfcCI::SetDecoderMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CG711DecoderIntfcCI::SetDecoderMode(TDecodeMode aDecodeMode)
    {
    TBool myDecodeMode = aDecodeMode;
    RDebug::Print(_L("CG711DecoderIntfcCI::SetDecoderMode decodeMode[%d]"), myDecodeMode);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CG711DecoderIntfcCI::SetCng
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CG711DecoderIntfcCI::SetCng(TBool aCng)
    {
    iCng = aCng;
    RDebug::Print(_L("CG711DecoderIntfcCI::SetCng cng[%d]"), iCng);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CG711DecoderIntfcCI::GetCng
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CG711DecoderIntfcCI::GetCng(TBool& aCng)
    {
    aCng = iCng;
    RDebug::Print(_L("CG711DecoderIntfcCI::GetCng cng[%d]"), aCng);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CG711DecoderIntfcCI::SetPlc
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CG711DecoderIntfcCI::SetPlc(TBool aPlc)
    {
    TBool myPlc = aPlc;
    RDebug::Print(_L("CG711DecoderIntfcCI::SetPlc plc[%d]"), myPlc);
    return KErrNone;
    }
