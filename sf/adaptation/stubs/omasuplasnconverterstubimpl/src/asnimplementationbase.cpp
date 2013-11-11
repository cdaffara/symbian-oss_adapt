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
* Description: Implementation of ASN plugin
*
*/

#include "asnimplementationbase.h"
#include <e32base.h>
#include "epos_comasuplstart.h"
#include "epos_comasuplresponse.h"
#include "epos_comasuplposinit.h"
#include "epos_comasuplpos.h"
#include "epos_comasuplinit.h"
#include "epos_comasuplend.h"
#include "epos_comasuplauthrequest.h"
#include "epos_comasuplauthresponse.h"
#include "cstubsuplstart.h"
#include "cstubsuplresponse.h"
#include "cstubsuplposinit.h"
#include "cstubsuplpos.h"
#include "cstubsuplinit.h"
#include "cstubsuplend.h"
#include "cstubsuplauthrequest.h"
#include "cstubsuplauthresponse.h"

CAsnImplementationBase::CAsnImplementationBase()
    {
    
    }
// -----------------------------------------------------------------------------
// CAsnImplementationBase::NewL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CAsnImplementationBase* CAsnImplementationBase::NewL()
    {
    CAsnImplementationBase* self = new (ELeave) CAsnImplementationBase;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CAsnImplementationBase::EncodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
HBufC8* CAsnImplementationBase::EncodeL(COMASuplAsnMessageBase* asnMessage, TInt& aErrorCode)
    {
    //Initialize your library for encoding...
    
    //Encoding operations to be done here!!
    HBufC8* encodedBuffer;
    if ( asnMessage )
        encodedBuffer = asnMessage->EncodeL(aErrorCode);
    else 
        encodedBuffer = NULL;
        
    return encodedBuffer;
    }
// -----------------------------------------------------------------------------
// CAsnImplementationBase::DecodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
COMASuplAsnMessageBase* CAsnImplementationBase::DecodeL(const TPtrC8& /*aPacket*/,TInt& aErrorCode,TInt& aUnConsumedBytes )
    {
    COMASuplAsnMessageBase* self = NULL;
    aErrorCode = 0;
    aUnConsumedBytes = 0;
    
		//Decoding operations to be done here!!!
		//Here you will have to decode the buffer using your own ASN library and return the object of appropriate SUPL message class.
		//e.g. if you finds that decoded message is SUPL_RESPONSE,create object of SUPL_RESPONSE class and call appropriate set methods to fill its all mandatory 
		//and optional methods. For demostration purpose, here are some set methods for SUPL_RESPONSE
		//
		//CStubSuplResponse* lOMASuplResponse = CStubSuplResponse::NewL();
		//CleanupStack::PushL(lOMASuplResponse);			
		//TOMASuplVersion lVersion;
		//TInt lMajor = 1;
		//TInt lMinor = 0;
		//TInt lServInd; = 0;
		//lVersion.SetSuplVersion(lMajor,lMinor,lServInd);
		//COMASuplSessionID* lSessionId = COMASuplSessionID::NewL();//Set all parameters needed for session id.
		//lOMASuplResponse->SetMessageBase(lVersion, lSessionId);
		//lOMASuplResponse->SetMessageLength(20);
		//TInt lPosMethod = 1;
		//lOMASuplResponse->SetSuplResponse((TOMASuplPosMethod)lPosMethod);
		//CleanupStack::Pop();
		//aErrorCode = KErrNone;
		//aUnConsumedBytes  = 0;
		// return lOMASuplResponse;

    return self;
    }
// -----------------------------------------------------------------------------
// CAsnImplementationBase::CreateOMASuplMessageL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
COMASuplAsnMessageBase* CAsnImplementationBase::CreateOMASuplMessageL(COMASuplAsnMessageBase::TSuplMessageType aOMASuplMessageType)
    {
    COMASuplAsnMessageBase* suplMsg = NULL;
    // Creating operations!!
    switch ( aOMASuplMessageType )
        {
        case COMASuplAsnMessageBase::ESUPL_START :
            {
            suplMsg = CStubSuplStart::NewL();
            break;
            }
        case COMASuplAsnMessageBase::ESUPL_RESPONSE :
            {
            suplMsg = CStubSuplResponse::NewL();
            break;
            }
        case COMASuplAsnMessageBase::ESUPL_POSINIT :
            {
            suplMsg = CStubSuplPosInit::NewL();
            break;
            }
        case COMASuplAsnMessageBase::ESUPL_POS :
            {
            suplMsg = CStubSuplPos::NewL();
            break;
            }
        case COMASuplAsnMessageBase::ESUPL_INIT : 
            {
            suplMsg = CStubSuplInit::NewL();
            break;
            }
        case COMASuplAsnMessageBase::ESUPL_END :
            {
            suplMsg = CStubSuplEnd::NewL();
            break;
            }
        case COMASuplAsnMessageBase::ESUPL_AUTH_REQ :
            {
            suplMsg = CStubSuplAuthRequest::NewL();
            break;
            }
        case COMASuplAsnMessageBase::ESUPL_AUTH_RESP :
            {
            suplMsg = CStubSuplAuthResponse::NewL();
            break;
            }
        default : break ;
        }
    return suplMsg;
    }
// -----------------------------------------------------------------------------
// CAsnImplementationBase::~CAsnImplementationBase
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
CAsnImplementationBase::~CAsnImplementationBase()
    {
    
    }
// -----------------------------------------------------------------------------
// CAsnImplementationBase::ConstructL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CAsnImplementationBase::ConstructL()
    {
    }

TOMASuplVersion CAsnImplementationBase::GetSuplMessageVersionL(TInt& aErrorCode,const TPtrC8& aSuplAsnMessage)
	{
	TOMASuplVersion version;
	return version;
	}