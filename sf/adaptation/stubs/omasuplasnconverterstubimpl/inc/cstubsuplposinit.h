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
* Description:SuplPosInit
*
*/


#ifndef CSTUBSUPLPOSINIT_H_
#define CSTUBSUPLPOSINIT_H_

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplposinit.h"
#include "asnimplementationbase.h"


class CStubSuplPosInit : public COMASuplPosInit
    {
public:
    /**
     * Factory function that instantiates a new object of CStubSuplPosInit
     *
     */
    IMPORT_C   static CStubSuplPosInit* NewL();
    /**
     * Destructor
     */
    IMPORT_C ~CStubSuplPosInit();
    /**
     * Method for Encoding...
     */
    virtual HBufC8* EncodeL(TInt& aErrorCode);

private:    
    
    void InitializeL();
    
    void InitializeRequestData();
    
    void InitializePosition();
    
    void InitializePositionEstimate(TOMASuplPositionEstimate& posEstimate);
    
    void InitializeTime(const TOMASuplUtcTime aUtcTime);
    
    void InitializePosPayLoad();
    
    void IntializeHorizVelocity(const COMASuplHorizVelocity* aVelocity);

    void IntializeHorizAndVertVelocity(const COMASuplHorizVelocity* aVelocity);

    void IntializeHorizUncertVelocity(const COMASuplHorizVelocity* aVelocity);

    void IntializeHorizVertUncertVelocity(const COMASuplHorizVelocity* aVelocity);

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
    CStubSuplPosInit();
    
    void ConstructL();
    
    
    };

#endif /* CSTUBSUPLSTARTIMPLE_H_ */
