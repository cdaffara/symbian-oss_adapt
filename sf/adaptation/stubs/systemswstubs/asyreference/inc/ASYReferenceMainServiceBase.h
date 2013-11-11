/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Used as an reference implementation
*
*/


#ifndef CASYREFERENCEMAINSERVICEBASE_H
#define CASYREFERENCEMAINSERVICEBASE_H

// INCLUDES
#include <AsyMainServiceBase.h>
#include "ASYAdaptation.h"
#include "ASYmessage.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CAccConfigFileParser;
class RAccessoryControl;
class CAccConGenericID;

// CLASS DECLARATION

/**
*  Declaration of CSACAccessoryConnectionHandler class.
*/

class CASYReferenceMainServiceBase : public CASYMainServiceBase, public MASYAdaptation
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CASYReferenceMainServiceBase* NewL();

        /**
        * Destructor.
        */
        virtual ~CASYReferenceMainServiceBase();

    private:  // Constructors

        /**
        * C++ constructor.
        */
        CASYReferenceMainServiceBase();

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();

    private: // Functions from base classes

        /**
        * From CASYMainServiceBase
        * @return Error code.
        */
        TInt StartL();

    private: // Functions from base classes

        /**
        * From MASYAdaptation
        * Indicates ASY about received response message
        * @param aMessage              Reference to a received message
        * @return                      none
        */
        void ProcessResponseL( TASYMessage& aMessage );

        /**
        * From MASYAdaptation
        * Indicates ASY about received error response message
        * @param aErrorCode             Error code for current message
        * @param aMessage               Reference to a received message
        * @return none
        */
        void ProcessErrorResponseL( TInt aErrorCode, TASYMessage& aMessage );
        
        /**
        * From MASYAdaptation
        * Indicates ASY about received event message
        * @param aMessage               Reference to a received message
        * @return none
        */
        void RaiseEventL( TASYMessage& aMessage );

    public:  // Enumerations

        // internal states
        enum TState
            {
            EStateUnknown = 1,
            EStateInitWait,
            EStateInitOngoing,
            EStateInitOk,            
            };

    public:  // New functions

        /**
        * Signals that an accessory connection state has changed.
        * @param aState                 Accessory Connection state
        * @return void
        */
        void ConnectionStateChangedL( AccessoryConnection::TConnectionStateInfoStruct& aState );

        /**
        * Changes internal state.
        * @param aState                 Internal state        
        */
        void StateChangeL( TState aState );
                      
        /**
        * Maps internal accessory feature to ASY command parameter
        * @param aAccessoryFeature      Internal accessory feature
        * @param aCommandParamRecord    Accessory Server parameter
        */
        void MapAccessoryFeatureL( AccessoryFeature::TAccessoryFeature& aAccessoryFeature, 
                                   TASYCommandParamRecord& aCommandParamRecord );



    private:  // New functions

        /**
        * Signals that an accessory feature state has changed.
        * @param aState                 Feature State
        * @return void
        */
        void AccessoryFeatureStateBooleanChangedL( AccessoryFeature::TFeatureStateBooleanStruct& aState );

    private:  // Data
        
        // handle to accessory server subsession
        RAccessoryControl* iAccessoryControl;

        // accessory configuration file parser
        CAccConfigFileParser* iAccConfigFileParser;

        // internal state
        TState iState;
        
        // Generic ID
        CAccConGenericID* iConnectedAccessory;
    };

#endif // CASYREFERENCEMAINSERVICEBASE_H

// End of file
