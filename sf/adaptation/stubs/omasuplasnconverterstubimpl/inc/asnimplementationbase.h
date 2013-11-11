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
* Description: Asn ImplementationBase
*
*/


#ifndef ASNIMPLEMENTATIONBASE_H_
#define ASNIMPLEMENTATIONBASE_H_

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"


class COMASuplAsnMessageBase;
class COMASuplAsnHandlerBase;
class CStubSuplStart;
class CStubSuplResponse;
class CStubSuplPosInit;
class CStubSuplPos;
class CStubSuplInit;
class CStubSuplEnd;
class CStubSuplAuthRequest;
class CStubSuplAuthResponse;

class CAsnImplementationBase : public COMASuplAsnHandlerBase
    {

public: 
    /**
     * Instanitates an object..
     */
    IMPORT_C static CAsnImplementationBase* NewL();
        
  virtual HBufC8* EncodeL(COMASuplAsnMessageBase*, TInt& aErrorCode);
  
  virtual COMASuplAsnMessageBase* DecodeL(const TPtrC8& aPacket,TInt& aErrorCode,TInt& aUnConsumedBytes );
  
  virtual COMASuplAsnMessageBase* CreateOMASuplMessageL(COMASuplAsnMessageBase::TSuplMessageType aOMASuplMessageType);

	
  virtual TOMASuplVersion GetSuplMessageVersionL(TInt& aErrorCode,
                const TPtrC8& aSuplAsnMessage);
    /**
     * Destructor
     */
 IMPORT_C ~CAsnImplementationBase();

protected:   
    /**
     * Constructor
     */
    CAsnImplementationBase();
    /**
     * Two phase construction
     */
    void ConstructL(); 

    
private:

    /**
     *  By default, prohibit copy constructor
     */
         
    CAsnImplementationBase( const CAsnImplementationBase& );
    
    /**
     *  By default, Prohibit assigment operator
     */

    CAsnImplementationBase& operator= ( const CAsnImplementationBase& );
    
    
    };

#endif /* ASNIMPLEMENTATIONBASE_H_ */
