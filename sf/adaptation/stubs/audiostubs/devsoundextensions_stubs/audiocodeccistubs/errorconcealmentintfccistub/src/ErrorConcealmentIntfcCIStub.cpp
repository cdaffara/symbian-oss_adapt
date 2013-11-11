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
#include "ErrorConcealmentIntfcCI.h"
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
// CErrorConcealmentIntfcCI::CErrorConcealmentIntfcCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CErrorConcealmentIntfcCI::CErrorConcealmentIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CErrorConcealmentIntfcCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CErrorConcealmentIntfcCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CErrorConcealmentIntfcCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CErrorConcealmentIntfcCI* CErrorConcealmentIntfcCI::NewL()
    {
	CErrorConcealmentIntfcCI* self = new (ELeave)CErrorConcealmentIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CErrorConcealmentIntfcCI::~CErrorConcealmentIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CErrorConcealmentIntfcCI::ConcealErrorForNextBuffer
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CErrorConcealmentIntfcCI::ConcealErrorForNextBuffer()
    {
    RDebug::Print(_L("CErrorConcealmentIntfcCI::ConcealErrorForNextBuffer"));
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CErrorConcealmentIntfcCI::SetFrameMode
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CErrorConcealmentIntfcCI::SetFrameMode(TBool aFrameMode)
    {
    TBool myFrameMode = aFrameMode;
    RDebug::Print(_L("CErrorConcealmentIntfcCI::SetFrameMode frameMode[%d]"), myFrameMode);
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CErrorConcealmentIntfcCI::FrameModeRqrdForEC
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CErrorConcealmentIntfcCI::FrameModeRqrdForEC(TBool& aFrameModeRqrd)
    {
    iFrameModeRqrd = !iFrameModeRqrd;
    aFrameModeRqrd = iFrameModeRqrd;
    RDebug::Print(_L("CErrorConcealmentIntfcCI::FrameModeRqrdForEC frameModeRqrd[%d]"), aFrameModeRqrd);
    return KErrNone;
    }
