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
#include "IlbcDecoderIntfcCI.h"
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
// CIlbcDecoderIntfcCI::CIlbcDecoderIntfcCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CIlbcDecoderIntfcCI::CIlbcDecoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CIlbcDecoderIntfcCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CIlbcDecoderIntfcCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CIlbcDecoderIntfcCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CIlbcDecoderIntfcCI* CIlbcDecoderIntfcCI::NewL()
    {
	CIlbcDecoderIntfcCI* self = new (ELeave)CIlbcDecoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CIlbcDecoderIntfcCI::~CIlbcDecoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CIlbcDecoderIntfcCI::SetDecoderMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIlbcDecoderIntfcCI::SetDecoderMode(TDecodeMode aDecodeMode)
    {
    TBool myDecodeMode = aDecodeMode;
    RDebug::Print(_L("CIlbcDecoderIntfcCI::SetDecoderMode decodeMode[%d]"), myDecodeMode);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CIlbcDecoderIntfcCI::SetCng
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIlbcDecoderIntfcCI::SetCng(TBool aCng)
    {
    iCng = aCng;
    RDebug::Print(_L("CIlbcDecoderIntfcCI::SetCng cng[%d]"), iCng);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CIlbcDecoderIntfcCI::GetCng
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CIlbcDecoderIntfcCI::GetCng(TBool& aCng)
    {
    aCng = iCng;
    RDebug::Print(_L("CIlbcDecoderIntfcCI::GetCng cng[%d]"), aCng);
    return KErrNone;
    }
