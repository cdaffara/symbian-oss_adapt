/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Provides the reference implementation for the POS message 
*                plugin
*
*/



#include <ecom/ecom.h>
#include "epos_comasuplposhandler.h"
#include "epos_comasuplpossession.h"


// -----------------------------------------------------------------------------
// COMASuplPosHandler::NewL
// Factory function to instantiate an object of COMASuplPosHandler
// -----------------------------------------------------------------------------
//
COMASuplPosHandlerBase* COMASuplPosHandler::NewL()
    {
    return new ( ELeave ) COMASuplPosHandler;
    }
    

// -----------------------------------------------------------------------------
// COMASuplPosHandler::~COMASuplPosHandler
// Destructor
// -----------------------------------------------------------------------------
//
COMASuplPosHandler::~COMASuplPosHandler()
    {
    }
    

// -----------------------------------------------------------------------------
// COMASuplPosHandler::InitializeL
// Initializes the instance of COMASuplPosHandler
// -----------------------------------------------------------------------------
//
void COMASuplPosHandler::InitializeL( TRequestStatus& aStatus )
    {
    // Initialization operations to be done
    TRequestStatus* status = &aStatus;
    User::RequestComplete( status, KErrNone );
    }
    

// -----------------------------------------------------------------------------
// COMASuplPosHandler::CancelInitialize
// Cancels an outstanding request to initialize
// -----------------------------------------------------------------------------
//
void COMASuplPosHandler::CancelInitialize()
    {
    
    }
    

// -----------------------------------------------------------------------------
// COMASuplPosHandler::CreateNewSessionL
// Factory function to create a new instance of COMASuplPosSession
// -----------------------------------------------------------------------------
//
COMASuplPosSessionBase* COMASuplPosHandler::CreateNewSessionL( 
                                MOMASuplObserver* aSuplObserver )
    {
    return COMASuplPosSession::NewL( aSuplObserver );
    }
    

// -----------------------------------------------------------------------------
// COMASuplPosHandler::COMASuplPosHandler
// Constructor
// -----------------------------------------------------------------------------
//
COMASuplPosHandler::COMASuplPosHandler():COMASuplPosHandlerBase()
    {

    }
