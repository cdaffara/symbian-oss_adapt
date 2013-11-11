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
* Description: Auth request
*
*/

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplauthrequest.h"
#include "cstubsuplauthrequest.h"

// -----------------------------------------------------------------------------
// CStubSuplAuthRequest::NewL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplAuthRequest* CStubSuplAuthRequest::NewL()
    {
    CStubSuplAuthRequest* self = new (ELeave) CStubSuplAuthRequest;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CStubSuplAuthRequest::~CStubSuplAuthRequest
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplAuthRequest::~CStubSuplAuthRequest()
    {
    
    }
// -----------------------------------------------------------------------------
// CStubSuplAuthRequest::ConstructL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplAuthRequest::ConstructL()
    {
    COMASuplAuthRequest::ConstructL();
    //Allocate memory for internal usage....
    }
// -----------------------------------------------------------------------------
// CStubSuplAuthRequest::EncodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
HBufC8* CStubSuplAuthRequest::EncodeL(TInt& aErrorCode)
    {
    aErrorCode = 0;
    HBufC8* encodedBuffer = NULL;
    // Do actual encoding here and return encoded buffer and put appropriate error code in aErrorCode
    return encodedBuffer;
    }
// -----------------------------------------------------------------------------
// CStubSuplAuthRequest::CStubSuplAuthRequest
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
CStubSuplAuthRequest::CStubSuplAuthRequest()
    {
    }
