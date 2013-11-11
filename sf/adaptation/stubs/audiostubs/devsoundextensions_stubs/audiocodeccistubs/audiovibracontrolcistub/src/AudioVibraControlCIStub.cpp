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
#include "AudioVibraControlCI.h"
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
// CAudioVibraControlCI::CAudioVibraControlCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CAudioVibraControlCI::CAudioVibraControlCI()
    {

    }

// -----------------------------------------------------------------------------
// CAudioVibraControlCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CAudioVibraControlCI::ConstructL()
    {

    }

// -----------------------------------------------------------------------------
// CAudioVibraControlCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CAudioVibraControlCI* CAudioVibraControlCI::NewL()
    {
	CAudioVibraControlCI* self = new (ELeave)CAudioVibraControlCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CAudioVibraControlCI::~CAudioVibraControlCI()
    {

    }

// -----------------------------------------------------------------------------
// CAudioVibraControlCI::StartVibra
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CAudioVibraControlCI::StartVibra()
    {
    RDebug::Print(_L("CAudioVibraControlCI::StartVibra"));
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CAudioVibraControlCI::StopVibra
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CAudioVibraControlCI::StopVibra()
    {
    RDebug::Print(_L("CAudioVibraControlCI::StopVibra"));
    return KErrNone;
    }

