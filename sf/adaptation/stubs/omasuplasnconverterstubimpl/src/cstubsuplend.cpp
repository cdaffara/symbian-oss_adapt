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
 * Description: Supl End
 *
 */

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplend.h"
#include "cstubsuplend.h"
#include "lbs/epos_comasuplreqasstdata.h"
#include "lbs/epos_comasuplposition.h"
#include "lbs/epos_comasuplvelocity.h"
#include "lbs/epos_comasuplsetcapabilities.h"
#include "lbs/epos_comasuplpospayload.h"
#include "epos_comasuplpos.h"
#include "epos_comasuplsessionid.h"
#include "epos_comasupllocationid.h"
// -----------------------------------------------------------------------------
// CStubSuplEnd::NewL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplEnd* CStubSuplEnd::NewL()
    {
    CStubSuplEnd* self = new (ELeave) CStubSuplEnd;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CStubSuplEnd::ConstructL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplEnd::ConstructL()
    {
    COMASuplEnd::ConstructL();
    //Allocate memory for internal usage....
    }
// -----------------------------------------------------------------------------
// CStubSuplEnd::~CStubSuplEnd
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplEnd::~CStubSuplEnd()
    {

    }
// -----------------------------------------------------------------------------
// CStubSuplEnd::EncodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
HBufC8* CStubSuplEnd::EncodeL(TInt& aErrorCode)
    {
    aErrorCode = 0;
    HBufC8* encodedBuffer = NULL ;
    InitializeL();
    // Do actual encoding here and return encoded buffer and put appropriate error code in aErrorCode
    return encodedBuffer;
    }
// -----------------------------------------------------------------------------
// CStubSuplEnd::CStubSuplEnd
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
CStubSuplEnd::CStubSuplEnd()
    {
    }
void CStubSuplEnd::InitializeL()
    {
    if (iPosition)   //This parameter is optional in asn
        {
    InitializePosition();
        }
    }
void CStubSuplEnd::InitializePosition()
    {
    // Retrieve mandatory parameters of Position
    // Retrieve mandatory parameters of Position
    TOMASuplUtcTime utcTime;
    TOMASuplPositionEstimate posEstimate;

    iPosition->GetPosition(utcTime,posEstimate );

    TInt error = 0;

    //Initialize Time
    InitializeTime(utcTime);

    InitializePositionEstimate(posEstimate);

    if(error)
        {
        return;
        }
    else
        {
        COMASuplVelocity* velocity = NULL;
        error = iPosition->GetVelocity(velocity);
        if(error)
            {
            //return error
            }
        else
            {

            TOMASuplVelocityType velocityType = velocity->VelType();
            COMASuplHorizVelocity* horizVelocity = velocity->Velocity();
            if(horizVelocity)
                {
                switch(velocityType)
                    {
                    case EHorizVelocity:
                        {
                        //Initialize Horizontal velocity
                        IntializeHorizVelocity(horizVelocity);
                        break;      
                        }
                    case EHorizAndVertVelocity:
                        {
                        //Initialize Horizontal and Vertical velocity
                        IntializeHorizAndVertVelocity(horizVelocity);
                        break;
                        }
                    case EHorizUncertVelocity:
                        {
                        //Initialize Horizontal Uncert velocity
                        IntializeHorizUncertVelocity(horizVelocity);
                        break;
                        }
                    case EHorizAndVertUncertVelocity:
                        {
                        //Initialize Horizontal Vertical Uncert velocity
                        IntializeHorizVertUncertVelocity(horizVelocity);
                        break;  
                        }
                    default:
                        {
                        //error
                        return;         
                        }
                    }   
                }
            else
                {
                //error
                return; 
                }
            }
        }
    }

void CStubSuplEnd::IntializeHorizVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;

    aVelocity->GetHorizVel(bearing,horSpeed);

    //Initialize bearing,horSpeed
    }

void CStubSuplEnd::IntializeHorizAndVertVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;
    TUint8 verDirect;
    TUint8 verSpeed;

    COMASuplHorizAndVertVelocity* horizVertVel = (COMASuplHorizAndVertVelocity*)aVelocity;

    horizVertVel->GetHorizAndVertVel(bearing,horSpeed,verDirect,verSpeed);

    //Initialize bearing,horSpeed,verDirect,verSpeed
    }

void CStubSuplEnd::IntializeHorizUncertVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;
    TUint8 uncertSpeed;

    COMASuplHorizUncertVelocity* horizUncertVel = (COMASuplHorizUncertVelocity*)aVelocity;

    horizUncertVel->GetHorizUncertVel(bearing,horSpeed,uncertSpeed);
    //Initialize bearing,horSpeed,uncertSpeed
    }

void CStubSuplEnd::IntializeHorizVertUncertVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;
    TUint8  verDirect;
    TUint8  verSpeed;
    TUint8  horizUncertSpeed;
    TUint8  vertUncertSpeed;

    COMASuplHorizAndVertUncertVelocity* horizVertUncertVel = (COMASuplHorizAndVertUncertVelocity*)aVelocity;

    horizVertUncertVel->GetHorizVertUncertVel(bearing,horSpeed,verDirect,verSpeed,
            horizUncertSpeed,vertUncertSpeed);
    //Initialize bearing,horSpeed,verDirect,verSpeed,horizUncertSpeed,vertUncertSpeed
    }

void CStubSuplEnd::InitializePositionEstimate(TOMASuplPositionEstimate& aPosEstimate)
    {
    TInt error;

    // Retrieve mandatory parametrs of Position Estimate
    TOMASuplPositionEstimate::TOMASuplLatitudeSign  latSign;
    TInt               latitude;
    TInt               longitude;
    aPosEstimate.GetPositionEstimate(latSign,latitude,longitude);   

    // Set Position Estimate
    if(KLatitudeMin <= latitude && latitude <= KLatitudeMax)
        {
        //Initialize
        }
    else
        {
        //error
        }

    if(KLongitudeMin <= longitude && longitude <= KLongitudeMax)
        {
        //Initialize
        }
    else
        {
        //error
        }

    //Initialize

    // Retrieve optional parametrs of Position Estimate
    // Retrieve Uncertainity
    TOMASuplUncertainty uncertainty;
    error = aPosEstimate.GetUncertainty(uncertainty);
    if(!error)
        {    
        TInt uncertSemiMajor;
        TInt uncertSemiMinor;
        TInt orientMajorAxis;
        uncertainty.GetUncertainty(uncertSemiMajor,uncertSemiMinor,orientMajorAxis);
        if((KUncertSemiMajorMin <= uncertSemiMajor && uncertSemiMajor<= KUncertSemiMajorMax)
                && (KUncertSemiMinorMin <= uncertSemiMinor && uncertSemiMinor<= KUncertSemiMinorMax)
                && (KOrientMajorAxisMin <= orientMajorAxis && orientMajorAxis<= KOrientMajorAxisMax))
            {
            //Initialize
            }
        else
            {
            //error
            }
        }
    else
        {
        //return error
        }

    // Retrieve Confidence
    TInt confidence;
    error = aPosEstimate.GetConfidence(confidence);
    if(!error)
        {

        if(KConfidenceMin <= confidence && confidence <= KConfidenceMax)
            {
            //Initialize       
            }
        else
            {
            //error 
            }
        }
    else
        {
        //return error
        }

    // Retrieve AltitudeInfo
    TOMASuplAltitudeInfo altitudeInfo;
    error = aPosEstimate.GetAltitudeInfo(altitudeInfo);
    if(!error)
        {

        TOMASuplAltitudeInfo::TOMASuplAltitudeDirection altDirection;
        TInt altitude;
        TInt altUncert;
        altitudeInfo.GetAltitudeInfo(altDirection,altitude,altUncert);
        if((KAltitudeMin <= altitude && altitude<= KAltitudeMax) && (KAltitudeUncertMin <= altUncert && altUncert<= KAltitudeUncertMax))
            {
            //Initialize
            }
        else
            {
            //error
            }
        }
    else
        {
        //return error
        }

    }

void CStubSuplEnd::InitializeTime(const TOMASuplUtcTime aUtcTime)
    {

    TDateTime lDateTime;
    TInt      zoneCode;
    TInt      zone;

    aUtcTime.GetUtcTime(lDateTime,zoneCode,zone);

    TInt   year     = lDateTime.Year();
    TMonth month    = lDateTime.Month(); 
    TInt   day      = lDateTime.Day();
    TInt   hour     = lDateTime.Hour();
    TInt   minute   = lDateTime.Minute();
    TInt   second   = lDateTime.Second();
    TInt   microsec = lDateTime.MicroSecond();

    if(0 == zoneCode || 1 == zoneCode)
        {
        //Initialize 
        }
    else
        {
        //error
        }


    }
