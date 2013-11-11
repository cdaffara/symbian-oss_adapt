/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: Supl Init
*
*/

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplinit.h"
#include "cstubsuplinit.h"

// -----------------------------------------------------------------------------
// CStubSuplInit::NewL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplInit* CStubSuplInit::NewL()
    {
    CStubSuplInit* self = new (ELeave) CStubSuplInit;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CStubSuplInit::~CStubSuplInit
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplInit::~CStubSuplInit()
    {
    
    }
// -----------------------------------------------------------------------------
// CStubSuplInit::ConstructL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplInit::ConstructL()
    {
    COMASuplInit::ConstructL();
    //Allocate memory for internal usage....
    }
// -----------------------------------------------------------------------------
// CStubSuplInit::EncodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
HBufC8* CStubSuplInit::EncodeL(TInt& aErrorCode)
    {
    aErrorCode = KErrNotSupported;
    //Encoding of SuplInit is not handled by client..
    return NULL;
    }
// -----------------------------------------------------------------------------
// CStubSuplInit::CStubSuplInit
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
CStubSuplInit::CStubSuplInit()
    {
    }
