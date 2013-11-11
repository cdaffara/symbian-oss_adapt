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
* Description:  Provides the reference implementation for POS message plugin
*
*/



#include <ecom/ecom.h>
#include <lbs/epos_eomasuplposerrors.h>
#include <lbs/epos_comasuplpospayload.h>
#include <lbs/epos_momasuplobserver.h>
#include "epos_comasuplpossession.h"
#include "epos_tomasuplposutility.h"


// -----------------------------------------------------------------------------
// COMASuplPosSession::NewL
// Factory function to instantiate an object of COMASuplPosSession
// -----------------------------------------------------------------------------
//
COMASuplPosSessionBase* COMASuplPosSession::NewL(
                            MOMASuplObserver* aSuplObserver )
    {
    return new ( ELeave ) COMASuplPosSession( aSuplObserver );
    }


// -----------------------------------------------------------------------------
// COMASuplPosSession::~COMASuplPosSession
// Destructor. Removes the object from the active scheduler's list
// -----------------------------------------------------------------------------
//
COMASuplPosSession::~COMASuplPosSession()
    {
    delete iPosPayload;
    Deque();
    }


// -----------------------------------------------------------------------------
// COMASuplPosSession::COMASuplPosSession
// Constructor
// -----------------------------------------------------------------------------
//
COMASuplPosSession::COMASuplPosSession( MOMASuplObserver* aSuplObserver ):
                        CActive( EPriorityStandard )
                        , COMASuplPosSessionBase( aSuplObserver )
    {
    iIsInitialized = EFalse;
    iRequestID = 0;
    CActiveScheduler::Add( this );
    }


// -----------------------------------------------------------------------------
// COMASuplPosSession::InitializeL
// Initializes the instance of COMASuplPosSession
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::InitializeL( const TInt aRequestID, 
                        TRequestStatus& aStatus )
    {
    aStatus = KRequestPending;
    iRequestID = aRequestID;
    iClientStatus = &aStatus;
    iState = EInitialize;
    if ( iPosPayload )
        {
        delete iPosPayload;
        iPosPayload = NULL;
        }
    IdleComplete();
    }
    

// -----------------------------------------------------------------------------
// COMASuplPosSession::CancelInitialize
// Cancels an outstanding request to initialize
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::CancelInitialize()
    {
    iIsInitialized = EFalse;
    CancelRequest();
    }


// -----------------------------------------------------------------------------
// COMASuplPosSession::HandleSuplPosMessageL
// Handles SUPL POS payload 
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::HandleSuplPosMessageL( 
                        const COMASuplPosPayload* aPosPayload )
    {
    __ASSERT_ALWAYS( iIsInitialized, User::Leave( KErrOMASuplPosInActive ) );
    
    // Delete any existing memory
    if ( iPosPayload )
        {
        delete iPosPayload;
        iPosPayload = NULL;
        }

    iPosPayload = static_cast<COMASuplPosPayload*>( aPosPayload->CloneL() );
    }
        

// -----------------------------------------------------------------------------
// COMASuplPosSession::GetSuplInfoL
// Provides different POS parameters to the SUPL FW
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::GetSuplInfoL( 
                        COMASuplInfoRequestList* aInfoRequestList,
                        TRequestStatus& aStatus )
    {
    TRequestStatus* status = &aStatus;
    if(!iIsInitialized)
    	{
    	User::RequestComplete(status, KErrOMASuplPosInActive );
    	return;
    	}
    aStatus = KRequestPending;
    iClientStatus = &aStatus;
    iSuplInfoReqList = aInfoRequestList;
    iState = EGetSuplInfo;
    IdleComplete();
    }
    						  

// -----------------------------------------------------------------------------
// COMASuplPosSession::CancelSuplInfoRequest
// Cancels an outstanding request to GetSuplInfoL
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::CancelSuplInfoRequest()
    {
    CancelRequest();
    }
        

// -----------------------------------------------------------------------------
// COMASuplPosSession::GetPositionL
// Provides position estimates to the SUPL FW
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::GetPositionL( TRequestStatus& aStatus, 
                        COMASuplPosition* aPosition )
    {
    TRequestStatus* status = &aStatus;
    if(!iIsInitialized)
    	{
    	User::RequestComplete(status, KErrOMASuplPosInActive );
    	return;
    	}
    aStatus = KRequestPending;
    iClientStatus = &aStatus;
    iPosition = aPosition;
    iState = EGetPosition;
    IdleComplete();
    }
        

// -----------------------------------------------------------------------------
// COMASuplPosSession::CancelGetPosition
// Cancels an outstanding request to GetPositionL
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::CancelGetPosition()
    {
    CancelRequest();
    }
        

// -----------------------------------------------------------------------------
// COMASuplPosSession::SessionEnd
// Terminates a SUPL POS Session
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::SessionEnd()
    {
    CancelRequest();
    iIsInitialized = EFalse;
    }


// -----------------------------------------------------------------------------
// COMASuplPosSession::RunL
// Inherited from CActive - called when object is active.
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::RunL()
    {
    switch( iState )
        {
        case EInitialize:
            iIsInitialized = ETrue;
            User::RequestComplete( iClientStatus, KErrNone );
            // CActiveScheduler::Stop();
            break;
            
        case EGetSuplInfo:
            TOmaSuplPosUtility::GetPosParametersL( iSuplInfoReqList );
            User::RequestComplete( iClientStatus, KErrNone );
            // CActiveScheduler::Stop();
            break;
            
        case EGetPosition:
            TOmaSuplPosUtility::GetPositionEstimatesL( iPosition );
            User::RequestComplete( iClientStatus, KErrNone );
            // CActiveScheduler::Stop();
            break;
            
        default:
            break;
        }
    }
    

// -----------------------------------------------------------------------------
// COMASuplPosSession::DoCancel
// Inherited from CActive - called to cancel outanding requests
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::DoCancel()
    {
    // Cancel any outstanding requests to asynchronous service providers
    TBool isActive = IsActive();
    if ( isActive )
        {
        User::RequestComplete( iClientStatus, KErrCancel );
        }
    }

TInt COMASuplPosSession::RunError(TInt /*aError*/)
	{
	return KErrNone;
	}

// -----------------------------------------------------------------------------
// COMASuplPosSession::IdleComplete
// Completes an asynchronous request with KErrNone
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::IdleComplete()
    {
    TRequestStatus* pS = &iStatus;
    User::RequestComplete( pS, KErrNone );
    if ( !IsActive() )
        {
        SetActive();
        }
    }

    
// -----------------------------------------------------------------------------
// COMASuplPosSession::CancelRequest
// Cancels an outstanding request if any with KErrCancel
// -----------------------------------------------------------------------------
//
void COMASuplPosSession::CancelRequest()
    {
    Cancel();
    }
