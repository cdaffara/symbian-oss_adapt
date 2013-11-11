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
* Description: Auth Response
*
*/

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplauthresponse.h"
#include "cstubsuplauthresponse.h"

// -----------------------------------------------------------------------------
// CStubSuplAuthResponse::NewL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplAuthResponse* CStubSuplAuthResponse::NewL()
    {
    CStubSuplAuthResponse* self = new (ELeave) CStubSuplAuthResponse;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CStubSuplAuthResponse::~CStubSuplAuthResponse
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplAuthResponse::~CStubSuplAuthResponse()
    {
    
    }
// -----------------------------------------------------------------------------
// CStubSuplAuthResponse::ConstructL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplAuthResponse::ConstructL()
    {
    COMASuplAuthResponse::ConstructL();
    //Allocate memory for internal usage....
    }
// -----------------------------------------------------------------------------
// CStubSuplAuthResponse::EncodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
HBufC8* CStubSuplAuthResponse::EncodeL(TInt& aErrorCode)
    {
    aErrorCode = 0;
    HBufC8* encodedBuffer = NULL;
    // Do actual encoding here and return encoded buffer and put appropriate error code in aErrorCode
    return encodedBuffer;
    }
// -----------------------------------------------------------------------------
// CStubSuplAuthResponse::CStubSuplAuthResponse
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
CStubSuplAuthResponse::CStubSuplAuthResponse()
    {
    }
