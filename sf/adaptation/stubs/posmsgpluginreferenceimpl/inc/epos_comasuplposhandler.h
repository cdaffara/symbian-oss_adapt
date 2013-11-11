/*
* Copyright (c)  Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implementation of COMASuplPosHandlerBase
*
*/


#ifndef C_COMASUPLPOSHANDLER_H
#define C_COMASUPLPOSHANDLER_H


#include <lbs/epos_comasuplposhandlerbase.h>


// Forward Declaration
class COMASuplPosSession;

/**
* Reference implementation of COMASuplPosHandlerBase
*  @since Series 60 3.1u
*/
class COMASuplPosHandler : public COMASuplPosHandlerBase
    {
    
    public:  // Constructors and destructor
            
        /**
         * Factory function that instantiates a new object of COMASuplPosHandler
         *
         * @since S60 v3.0
         */
        static COMASuplPosHandlerBase* NewL();
        
        /**
         * Destructor
         *
         * @since S60 v3.0
         */
        ~COMASuplPosHandler();        
        
    public: // New functions

        /**
         * From COMASuplPosHandlerBase
         * Initializes the instance of COMASuplPosHandler
         * @since S60 v3.1u
         */
        void InitializeL( TRequestStatus &aStatus );
        
        /**
         * From COMASuplPosHandlerBase
         * Cancels an outstanding request to initialize
         * @since S60 v3.1u
         */
        void CancelInitialize();
        
        /**
         * From COMASuplPosHandlerBase
         * Factory function to create a new instance of COMASuplPosSession
         * @since S60 v3.1u
         */
        COMASuplPosSessionBase* CreateNewSessionL( 
                                        MOMASuplObserver *aSuplObserver );
        
    private:
    
        /**
         * Constructor
         *
         * @since S60 v3.1u
         */
        COMASuplPosHandler();
        
    };


#endif // C_COMASUPLPOSHANDLER_H
