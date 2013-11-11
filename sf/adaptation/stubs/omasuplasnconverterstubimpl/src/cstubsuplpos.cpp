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
 * Description: SuplPos
 *
 */

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplposinit.h"
#include "epos_comasuplpos.h"
#include "cstubsuplpos.h"
#include "lbs/epos_comasuplreqasstdata.h"
#include "lbs/epos_comasuplposition.h"
#include "lbs/epos_comasuplvelocity.h"
#include "lbs/epos_comasuplsetcapabilities.h"
#include "lbs/epos_comasuplpospayload.h"
#include "epos_comasuplpos.h"
#include "epos_comasuplsessionid.h"
#include "epos_comasupllocationid.h"

// -----------------------------------------------------------------------------
// CStubSuplPos::NewL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplPos* CStubSuplPos::NewL()
    {
    CStubSuplPos* self = new (ELeave) CStubSuplPos;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CStubSuplPos::~CStubSuplPos
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplPos::~CStubSuplPos()
    {

    }
// -----------------------------------------------------------------------------
// CStubSuplPos::ConstructL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplPos::ConstructL()
    {
    COMASuplPos::ConstructL();
    //Allocate memory for internal usage....
    }
// -----------------------------------------------------------------------------
// CStubSuplPos::EncodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
HBufC8* CStubSuplPos::EncodeL(TInt& aErrorCode)
    {
    aErrorCode = 0;
    HBufC8* encodedBuffer = NULL;
    InitializeL();
    // Do actual encoding here and return encoded buffer and put appropriate error code in aErrorCode
    return encodedBuffer;
    }
// -----------------------------------------------------------------------------
// CStubSuplPos::CStubSuplPos
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
CStubSuplPos::CStubSuplPos()
    {
    }
void CStubSuplPos::InitializeL()
    {
    //Initialize your library

    if(iPayload)
        {
        HBufC8* payloadData = NULL;
        COMASuplPosPayload::TOMASuplPosPayloadType payloadType;
        iPayload->GetPosPayload(payloadData,payloadType);

        if(!payloadData)  
            {
            //error   
            }           

        TPtr8 ptr = payloadData->Des();

        TInt len = payloadData->Length();

        const char* lTemp = (const char *)ptr.Ptr();

        switch(payloadType)
            {
            case COMASuplPosPayload::ETIA801:
                {
                //initialize pospayload for TIA                              
                break;
                }   
            case COMASuplPosPayload::ERRC:
                {
                //initialize pospayload for RRC
                break;                      
                }
            case COMASuplPosPayload::ERRLP:
                {
                //initialize pospayload for RRLP
                break;                      
                }
            default:
                {

                }           
            }

        }
    else
        {   
        //error

        }

    if(iOptionalMask & (1<<(0))) //means velocity has been set
        {
        TOMASuplVelocityType velocityType = iVelocity->VelType();
        COMASuplHorizVelocity* horizVelocity = iVelocity->Velocity();
        if(horizVelocity)
            {
            switch(velocityType)
                {
                case EHorizVelocity:
                    {
                    //Initialize
                    IntializeHorizVelocity(horizVelocity);
                    break;      
                    }
                case EHorizAndVertVelocity:
                    {
                    //Initialize
                    IntializeHorizAndVertVelocity(horizVelocity);
                    break;
                    }
                case EHorizUncertVelocity:
                    {
                    //Initialize
                    IntializeHorizUncertVelocity(horizVelocity);
                    break;
                    }
                case EHorizAndVertUncertVelocity:
                    {
                    //Initialize
                    IntializeHorizVertUncertVelocity(horizVelocity);
                    break;  
                    }
                default:
                    {
                    //error

                    }
                }       
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
void CStubSuplPos::IntializeHorizVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;

    aVelocity->GetHorizVel(bearing,horSpeed);

    //Initialize bearing,horSpeed
    }

void CStubSuplPos::IntializeHorizAndVertVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;
    TUint8 verDirect;
    TUint8 verSpeed;

    COMASuplHorizAndVertVelocity* horizVertVel = (COMASuplHorizAndVertVelocity*)aVelocity;

    horizVertVel->GetHorizAndVertVel(bearing,horSpeed,verDirect,verSpeed);

    //Initialize bearing,horSpeed,verDirect,verSpeed
    }

void CStubSuplPos::IntializeHorizUncertVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;
    TUint8 uncertSpeed;

    COMASuplHorizUncertVelocity* horizUncertVel = (COMASuplHorizUncertVelocity*)aVelocity;

    horizUncertVel->GetHorizUncertVel(bearing,horSpeed,uncertSpeed);
    //Initialize bearing,horSpeed,uncertSpeed
    }

void CStubSuplPos::IntializeHorizVertUncertVelocity(const COMASuplHorizVelocity* aVelocity)
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
