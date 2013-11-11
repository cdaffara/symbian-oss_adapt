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


#ifndef CSTUBSUPLEND_H_
#define CSTUBSUPLEND_H_

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplend.h"
#include "asnimplementationbase.h"

class CStubSuplEnd : public COMASuplEnd
    {
public:
    /**
     * Factory function that instantiates a new object of CStubSuplEnd
     *
     */
    IMPORT_C   static CStubSuplEnd* NewL();
    /**
     * Destructor
     */
    IMPORT_C ~CStubSuplEnd();
    /**
     * Method for Encoding...
     */
    virtual HBufC8* EncodeL(TInt& aErrorCode);

private:    
    void InitializeL();
    
    void InitializePosition();
    
    void InitializePositionEstimate(TOMASuplPositionEstimate& posEstimate);
    
    void InitializeTime(const TOMASuplUtcTime aUtcTime);
    
    void IntializeHorizVelocity(const COMASuplHorizVelocity* aVelocity);

    void IntializeHorizAndVertVelocity(const COMASuplHorizVelocity* aVelocity);

    void IntializeHorizUncertVelocity(const COMASuplHorizVelocity* aVelocity);

    void IntializeHorizVertUncertVelocity(const COMASuplHorizVelocity* aVelocity);
private:
    /**
     * Default C++ Constructor
     */
    CStubSuplEnd();
    
    void ConstructL();
    };

#endif /* CSTUBSUPLSTARTIMPLE_H_ */
