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
* Description:Supl Response
*
*/

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplresponse.h"
#include "cstubsuplresponse.h"

// -----------------------------------------------------------------------------
// CStubSuplResponse::NewL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplResponse* CStubSuplResponse::NewL()
    {
    CStubSuplResponse* self = new (ELeave) CStubSuplResponse;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CStubSuplResponse::~CStubSuplResponse
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplResponse::~CStubSuplResponse()
    {
    
    }
// -----------------------------------------------------------------------------
// CStubSuplResponse::ConstructL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplResponse::ConstructL()
    {
    COMASuplResponse::ConstructL();
    //Allocate memory for internal usage....
    }
// -----------------------------------------------------------------------------
// CStubSuplResponse::EncodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
HBufC8* CStubSuplResponse::EncodeL(TInt& aErrorCode)
    {
    aErrorCode = KErrNotSupported;
    //Encoding of SuplResponse is not done on SET side...
    return NULL;
    }
// -----------------------------------------------------------------------------
// CStubSuplResponse::CStubSuplResponse
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
CStubSuplResponse::CStubSuplResponse()
    {
    }
