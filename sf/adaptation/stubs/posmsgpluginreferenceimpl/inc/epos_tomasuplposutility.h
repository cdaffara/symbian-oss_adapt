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
* Description:  Helper class used to obtain supl information
*
*/


#ifndef T_TOMASUPLPOSUTILITY_H
#define T_TOMASUPLPOSUTILITY_H


class COMASuplInfoRequest;


/**
* An utility class that is used by an instance of COMASuplPosSession to 
* determine POS parameters
*/
class TOmaSuplPosUtility 
    {
    
    public:
        
        /**
         * Provides the position estimates. This is method is invoked by an
         * instance of COMASuplPosSession
         */
        static void GetPositionEstimatesL( COMASuplPosition*& aPosition );
        
        /**
         * Provides the requested POS parameters. This is method is invoked by an
         * instance of COMASuplPosSession
         */        
        static void GetPosParametersL( COMASuplInfoRequestList*& aInfoRequestList );
        
    private:
    
        /**
         * Provides the requested assistance data
         */
        static void GetRequestedAsstData( COMASuplInfoRequest*& aReqElement );
        
        /**
         * Provides velocity information
         */
        static void GetVelocityL( COMASuplInfoRequest*& aReqElement );
    
        /**
         * Provides SET capabilities
         */        
        static void GetSetCapabilities( COMASuplInfoRequest*& aReqElement );
    
        /**
         * Provides the POS payload
         */        
        static void GetPosPayloadL( COMASuplInfoRequest*& aReqElement );
    
        /**
         * Provides the position estimates
         */        
        static void GetPositionL( COMASuplInfoRequest*& aReqElement );
        
    };


#endif // T_TOMASUPLPOSUTILITY_H
