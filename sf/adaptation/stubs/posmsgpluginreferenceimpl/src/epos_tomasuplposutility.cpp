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
* Description:  Helper class that provides SUPL information
*
*/



#include <lbs/epos_comasuplinforequestlist.h>
#include <lbs/epos_comasuplpospayload.h>
#include <lbs/epos_comasuplposition.h>
#include <lbs/epos_comasuplreqasstdata.h>
#include <lbs/epos_comasuplvelocity.h>
#include <lbs/epos_comasuplsetcapabilities.h>
#include <lbs/epos_eomasuplposerrors.h>
#include "epos_tomasuplposutility.h"


// -----------------------------------------------------------------------------
// TOmaSuplPosUtility::GetPositionEstimatesL
// Determines the position estimates
// -----------------------------------------------------------------------------
//
void TOmaSuplPosUtility::GetPositionEstimatesL( COMASuplPosition*& aPosition )
    {
    TDateTime dateTime( 2006, EJanuary, 5, 00, 00, 00, 000000);
    TOMASuplUtcTime utcTime;
    TInt zoneCode = 0;
    TInt zone = 23;

    utcTime.SetUtcTime( dateTime, zoneCode, zone);

    TOMASuplPositionEstimate posEstimate;

    TInt latitude = 100;
    TInt longitude = 100;
    posEstimate.SetPositionEstimate( TOMASuplPositionEstimate::ENorth, latitude, 
                                longitude);
    aPosition->SetPosition( utcTime, posEstimate );
    COMASuplInfoRequest* req = COMASuplVelocity::NewL();
    GetVelocityL(req);
    aPosition->SetSuplVelocity(STATIC_CAST(COMASuplVelocity*,req));
    }


// -----------------------------------------------------------------------------
// TOmaSuplPosUtility::GetPosParametersL
// Determines the POS parameters
// -----------------------------------------------------------------------------
//
void TOmaSuplPosUtility::GetPosParametersL( COMASuplInfoRequestList*& aInfoRequestList )
    {
    TInt count = aInfoRequestList->GetCount();
    for ( TInt index = 0; index < count; index ++ )
        {
        // Determine the type of the element
        COMASuplInfoRequest *reqElement = aInfoRequestList->GetElement( index );
       	COMASuplInfoRequest::TOMASuplInfoType infoReqType = reqElement->Type();

        // Fill up the element with appropriate values
        switch( infoReqType )
            {
            case COMASuplInfoRequest::EOMASuplReqAsstData:
                GetRequestedAsstData( reqElement );
                break;
                
            case COMASuplInfoRequest::EOMASuplVelocity:
                TRAPD( err, GetVelocityL( reqElement ) );
                if ( KErrNone != err )
                    {
                    reqElement->SetStatus( KErrOMASuplParamNotSet );
                    }
                break;
                
            case COMASuplInfoRequest::EOMASuplSETCapabilities:
                GetSetCapabilities( reqElement );
                break;
                
            case COMASuplInfoRequest::EOMASuplPosPayload:
                TRAP( err, GetPosPayloadL( reqElement ) );
                if ( KErrNone != err )
                    {
                    reqElement->SetStatus( KErrOMASuplParamNotSet );
                    }
                else
                    {
                    reqElement->SetStatus( KErrOMASuplLastPosMsg );
                    }
                break;
                
            case COMASuplInfoRequest::EOMASuplPosition:
                GetPositionL( reqElement );
                break;
                
            default:
                reqElement->SetStatus( KErrOMASuplParamNotSet );
                break;
            }
        }
    }


// -----------------------------------------------------------------------------
// TOmaSuplPosUtility::GetRequestedAsstData
// Determines the requested assistance data
// -----------------------------------------------------------------------------
//
void TOmaSuplPosUtility::GetRequestedAsstData( COMASuplInfoRequest*& aReqElement )
    {
	COMASuplReqAsstData* reqAsstData = static_cast<COMASuplReqAsstData*>(aReqElement);

    reqAsstData->SetReqAsstData( ETrue, ETrue, ETrue, ETrue, ETrue, ETrue, 
                                        ETrue, ETrue );
    }
    

// -----------------------------------------------------------------------------
// TOmaSuplPosUtility::GetVelocity
// Determines the velocity
// -----------------------------------------------------------------------------
//
void TOmaSuplPosUtility::GetVelocityL( COMASuplInfoRequest*& aReqElement )
    {
    COMASuplVelocity* velocity = static_cast<COMASuplVelocity*>( aReqElement );
    COMASuplHorizVelocity* horizVel = COMASuplHorizVelocity::NewL();
    horizVel->SetHorizVel(1,2);
    velocity->SetVelocity( horizVel );
    }
    

// -----------------------------------------------------------------------------
// TOmaSuplPosUtility::GetSetCapabilities
// Determines the SET capabilities
// -----------------------------------------------------------------------------
//
void TOmaSuplPosUtility::GetSetCapabilities( COMASuplInfoRequest*& aReqElement )
    {
    COMASuplSETCapabilities* setCap = static_cast<COMASuplSETCapabilities*>( aReqElement );
    TOMASuplPosTechnology posTech;
    posTech.SetPosTechnology( ETrue, ETrue, ETrue, ETrue, ETrue, ETrue );
    TOMASuplPosProtocol suplProtocol;
    suplProtocol.SetPosProtocol( ETrue, ETrue, ETrue ); 
    setCap->SetSETCapabilities( 
                posTech, COMASuplSETCapabilities::EOMANoPreferred, suplProtocol );
    }
    

// -----------------------------------------------------------------------------
// TOmaSuplPosUtility::GetPosPayload
// Determines the POS payload
// -----------------------------------------------------------------------------
//
void TOmaSuplPosUtility::GetPosPayloadL( COMASuplInfoRequest*& aReqElement )
    {
    COMASuplPosPayload* payload = static_cast<COMASuplPosPayload*>( aReqElement );
    HBufC8* payloadData = HBufC8::NewL(20);
    payloadData->Des().Copy( _L8( "PayLoadData" ) );
    payload->SetPosPayload( payloadData, COMASuplPosPayload::ETIA801 );
    }
    

// -----------------------------------------------------------------------------
// TOmaSuplPosUtility::GetPositionL
// Determines the position estimates
// -----------------------------------------------------------------------------
//
void TOmaSuplPosUtility::GetPositionL( COMASuplInfoRequest*& aReqElement )
    {
    COMASuplPosition* position = static_cast<COMASuplPosition*>( aReqElement );
    GetPositionEstimatesL( position );
    }
