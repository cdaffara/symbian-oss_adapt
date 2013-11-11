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
* Description:  authrequest
*
*/


#ifndef CSTUBSUPLAUTHREQUEST_H_
#define CSTUBSUPLAUTHREQUEST_H_

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplauthrequest.h"
#include "asnimplementationbase.h"

class CStubSuplAuthRequest : public COMASuplAuthRequest
    {
public:
    /**
     * Factory function that instantiates a new object of CStubSuplAuthRequest
     *
     */
    IMPORT_C   static CStubSuplAuthRequest* NewL();
    /**
     * Destructor
     */
    IMPORT_C ~CStubSuplAuthRequest();
    /**
     * Method for Encoding...
     */
    virtual HBufC8* EncodeL(TInt& aErrorCode);
private:
    /**
     * Default C++ Constructor
     */
    CStubSuplAuthRequest();
    
    void ConstructL();
    };

#endif /* CSTUBSUPLSTARTIMPLE_H_ */
