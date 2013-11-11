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
 * Description:
 * CDdcPortAccessImpl class implementation.
 *
 */

// USER INCLUDES
#include "ddc_accessimpl.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ constructor.
// ---------------------------------------------------------------------------
//
CDdcPortAccessImpl::CDdcPortAccessImpl()
    {

    }

// ---------------------------------------------------------------------------
// Symbian 2nd phase constructor.
// ---------------------------------------------------------------------------
//
void CDdcPortAccessImpl::ConstructL()
    {
        
    }

// ---------------------------------------------------------------------------
// Symbian two phased constructor.
// ---------------------------------------------------------------------------
//
CDdcPortAccessImpl* CDdcPortAccessImpl::NewL()
    {
    CDdcPortAccessImpl* self = CDdcPortAccessImpl::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Symbian two phased constructor.
// Leaves pointer in the cleanup stack.
// ---------------------------------------------------------------------------
//
CDdcPortAccessImpl* CDdcPortAccessImpl::NewLC()
    {
    CDdcPortAccessImpl* self = new ( ELeave ) CDdcPortAccessImpl;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// C++ destructor.
// ---------------------------------------------------------------------------
//
CDdcPortAccessImpl::~CDdcPortAccessImpl()
    {
        
    }

//------------------------------------------------------------------------------
// CDdcPortAccessImpl::Read
//------------------------------------------------------------------------------
//
TInt CDdcPortAccessImpl::Read( TDdcPort /*aDdcPort*/,
    TUint /*aBlockNumber*/,
    TDataBlock& /*aDataBlock*/,
    TRequestStatus& aCompletedWhenRead ) 
    {
    TRequestStatus* status = &aCompletedWhenRead;
    User::RequestComplete( status, KErrNotSupported );
    return KErrNotSupported;
    }

//------------------------------------------------------------------------------
// CDdcPortAccessImpl::CancelAll
//------------------------------------------------------------------------------
//
void CDdcPortAccessImpl::CancelAll() 
    {
    // Nothing to do here.
    }

// End of file
