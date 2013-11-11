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
* Description:  Implementation of COMASuplPosSessionBase
*
*/


#ifndef C_COMASUPLPOSSESSION_H
#define C_COMASUPLPOSSESSION_H


#include <lbs/epos_comasuplpossessionbase.h>


/**
* Reference implementation of COMASuplPosSessionBase
* @since S60 3.1u
*/
class COMASuplPosSession : public COMASuplPosSessionBase
							, public virtual CActive
    {
    
    public:  // Constructors and destructor
            
        /**
         * Factory function that instantiates a new object of COMASuplPosSession
         *
         * @since S60 v3.1u
         */
        static COMASuplPosSessionBase* NewL( MOMASuplObserver* aSuplObserver );
        
        /**
         * Destructor
         *
         * @since S60 v3.0
         */
        ~COMASuplPosSession();        
        
    public: 

        /**
         * From COMASuplPosSessionBase
         * Initializes the instance of COMASuplPosSession
         * @since S60 v3.1u
         */
        void InitializeL( const TInt aRequestID, TRequestStatus &aStatus );
        
        /**
         * From COMASuplPosSessionBase
         * Cancels an outstanding request to initialize
         * @since S60 v3.1u
         */
        void CancelInitialize();
        
        /**
         * From COMASuplPosSessionBase
         * Handles SUPL POS payload
         * @since S60 v3.1u
         */
        void HandleSuplPosMessageL( const COMASuplPosPayload *aPosPayload );
        
        /**
         * From COMASuplPosSessionBase
         * Provides different POS parameters to the SUPL FW
         * @since S60 v3.1u
         */
        void GetSuplInfoL( COMASuplInfoRequestList *aInfoRequestList,
    						    TRequestStatus &aStatus );
    						  
        /**
         * From COMASuplPosSessionBase
         * Cancels an outstanding request to GetSuplInfoL
         * @since S60 v3.1u
         */
        void CancelSuplInfoRequest();
        
        /**
         * From COMASuplPosSessionBase
         * Provides position estimates to the SUPL FW
         * @since S60 v3.1u
         */
        void GetPositionL( TRequestStatus &aStatus, 
                                COMASuplPosition* aPosition );
        
        /**
         * From COMASuplPosSessionBase
         * Cancels an outstanding request to GetPositionL
         * @since S60 v3.1u
         */
        void CancelGetPosition();
        
        /**
         * From COMASuplPosSessionBase
         * Terminates a SUPL POS Session
         * @since S60 v3.1u
         */
        void SessionEnd();
        
        
    private:
    
        /**
         * Creates an IPC connection with the SUPL gateway, forwards 
         *				   the request and terminates the connection
         * @since S60 v3.1u
         */
        COMASuplPosSession( MOMASuplObserver* aSuplObserver );
        
        /**
         * From CActive
         * Request completion event handler
         * @since S60 v3.1u
         */
        void RunL();
        
        /**
         * From CActive
         * Called to cancel outanding requests
         * @since S60 v3.1u
         */
        void DoCancel();
        
        /**
         * From CActive
         * Called to handle leave in RunL
         * @since S60 v3.1u
         */
         TInt RunError(TInt aError);
        /**
         * Completes an asynchronous request with KErrNone
         * @since S60 v3.1u
         */
        void IdleComplete();
        
        /**
         * Cancels an outstanding request
         * @since S60 v3.1u
         */
        void CancelRequest();
    
    private:
    
        /**
         * State information of an instance of COMASuplPosSession
         */
        enum TPosSessionState 
            {
            EInitialize, 
            EGetSuplInfo, 
            EGetPosition
            };
        
        /**
         * Contains the client request status
         */        
        TRequestStatus*             iClientStatus;
        
        /**
         * Contains the request ID 
         */
        TInt                        iRequestID;
        
        /**
         * A flag field to determine if the instance of COMASuplPosSession
         * was initialized
         */        
        TBool                       iIsInitialized;
        
        /**
         * Data field to hold the state information
         */
        TPosSessionState            iState;
        
        /**
         * Data field to hold the position estimates. The ownership of this 
         * object is not taken.
         */       
        COMASuplPosition*           iPosition;
        
        /**
         * Data field to hold the SUPL Info Request List. The ownership 
         * of this object is not taken.
         */       
        COMASuplInfoRequestList*    iSuplInfoReqList;
        
        /**
         * Data field to hold the payload information. The instance of 
         * COMASuplPosSession instantiates this object and also takes 
         * ownership of the same.
         */       
        COMASuplPosPayload*         iPosPayload;

    };


#endif // C_COMASUPLPOSSESSION_H
