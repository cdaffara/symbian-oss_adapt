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
* Description: SuplStart
*
*/


#ifndef CSTUBSUPLSTARTIMPLE_H_
#define CSTUBSUPLSTARTIMPLE_H_

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplstart.h"
#include "asnimplementationbase.h"

class CStubSuplStart : public COMASuplStart
    {
public:
    /**
     * Factory function that instantiates a new object of CStubSuplStart
     *
     */
    IMPORT_C   static CStubSuplStart* NewL();
    /**
     * Destructor
     */
    IMPORT_C ~CStubSuplStart();
    /**
     * Method for Encoding...
     */
    virtual HBufC8* EncodeL(TInt& aErrorCode);
    /**
     * initialize memory management stack 
     * allocate memory for structure 
     */
     
private:     
    void InitializeL();
    /**
     * Initialize SetCapabilities
     */
    void InitializeSetCapabilities();
    /**
     * Initialize LocationId
     */
    void InitializeLocationId();
    /**
     * Initialize Qop
     */
    void InitializeQop();
    /**
     * Initialize ECellId
     */
    void InitializeECellId();
    /**
     * Initialize GSM ECellId
     */
    void InitializeGSMECellId();
    /**
     * Initialize WCDMA ECellId
     */
    void InitializeWCDMAECellId();
    
    TInt FillFrequencyInfo(TOMASuplFrequencyInfo& aOMASuplFrequencyInfo);
    
    TInt FillMeasuredResultsList(TOMASuplNetworkMeasureReport aNwkMeasureReport[]);
    
    TInt FillCellMeasuredResult(TOMASuplCellMeasuredResult aCellMeasuredResult[]);
    
    TInt FillModeSpecificInfo(TOMASuplCellMeasuredResult&   aCellMeasuredResult);
    
    void FillFrequencyInfoULInfo(TInt aFddUL,TInt& aRetErrorCode);
    
    void FillFrequencyInfoDT(TInt aTddDt,TInt& aRetErrorCode);
    
private:
    /**
     * Default C++ Constructor
     */
    CStubSuplStart();
    /*
     * 2 phase construction
     */
    void ConstructL();
    };

#endif /* CSTUBSUPLSTARTIMPLE_H_ */
