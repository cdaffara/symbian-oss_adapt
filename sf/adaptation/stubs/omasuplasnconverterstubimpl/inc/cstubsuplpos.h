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
* Description:SuplPos
*
*/


#ifndef CSTUBSUPLPOS_H_
#define CSTUBSUPLPOS_H_

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplpos.h"
#include "epos_comasuplposinit.h"
#include "asnimplementationbase.h"

class CStubSuplPos : public COMASuplPos
    {
public:
    /**
     * Factory function that instantiates a new object of CStubSuplPos
     *
     */
    IMPORT_C   static CStubSuplPos* NewL();
    /**
     * Destructor
     */
    IMPORT_C ~CStubSuplPos();
    /**
     * Method for Encoding...
     */
    virtual HBufC8* EncodeL(TInt& aErrorCode);
    
private:    
    void InitializeL();
    
    void IntializeHorizVelocity(const COMASuplHorizVelocity* aVelocity);
    
    void IntializeHorizAndVertVelocity(const COMASuplHorizVelocity* aVelocity);
    
    void IntializeHorizUncertVelocity(const COMASuplHorizVelocity* aVelocity);
    
    void IntializeHorizVertUncertVelocity(const COMASuplHorizVelocity* aVelocity);
    
private:
    /**
     * Default C++ Constructor
     */
    CStubSuplPos();
    
    void ConstructL();
    };

#endif /* CSTUBSUPLSTARTIMPLE_H_ */
