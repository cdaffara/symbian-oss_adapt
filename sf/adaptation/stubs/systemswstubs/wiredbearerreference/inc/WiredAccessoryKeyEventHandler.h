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
* Description:  Used as an reference implementation.
*
*/



#ifndef WIREDACCESSORYKEYEVENTHANDLER_H
#define WIREDACCESSORYKEYEVENTHANDLER_H

//  INCLUDES
#include <remcon/remconbearerinterface.h>
#include <remcon/remconbearerplugin.h>
#include <remcon/remconbearerobserver.h>
#include <remcon/messagetype.h> // from RemCon
#include <RemConKeyEventData.h>
#include "MAdaptation.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class TBearerParams;
class TAdaptationMessage;

// CLASS DECLARATION

/**
*  Declaration of CWiredAccessoryKeyEventHandler class.
*/
class CWiredAccessoryKeyEventHandler : public CRemConBearerPlugin, public MAdaptation, public MRemConBearerInterface
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aBearerParams
        */
        static CWiredAccessoryKeyEventHandler* NewL( TBearerParams& aBearerParams );

        /**
        * Destructor.
        */
        virtual ~CWiredAccessoryKeyEventHandler();

    private: // Constructors

        /**
        * C++ constructor.
        * @param aBearerParams
        */
        CWiredAccessoryKeyEventHandler( TBearerParams& aBearerParams );

        /**
        * Symbian 2nd phase constructor.
        * @return void
        */
        void ConstructL();

    private: // Functions from base classes

        /**
        * From CRemConBearerPlugin
        *
        * Called by RemCon server to get a pointer to
        * an object which implements the bearer API
        * @param aUid
        * @return TAny*
        */
        TAny* GetInterface( TUid aUid );

        /**
        * From MRemConBearerInterface
        *
        * Called by RemCon to retrieve a response on a
        * connection. Must only be  called as a result
        * of a NewResponse up-call.
        * @param aInterfaceUid
        * @param aTransactionId
        * @param aOperationId
        * @param aData, On success, ownership is returned.
        * @param aAddr
        * @return Error code.
        */
        TInt GetResponse( TUid& aInterfaceUid,
                          TUint& aTransactionId,
                          TUint& aOperationId,
                          RBuf8& aData,
                          TRemConAddress& aAddr );

        /**
        * From MRemConBearerInterface
        *
        * Called by RemCon to retrieve a command on a
        * connection. Must only be called as a result of
        * a NewCommand up-call.
        * @param aInterfaceUid
        * @param aTransactionId
        * @param aOperationId
        * @param aCommandData, On success, ownership is returned.
        * @param aAddr
        * @return Error code.
        */
        TInt GetCommand( TUid& aInterfaceUid,
                         TUint& aTransactionId,
                         TUint& aOperationId,
                         RBuf8& aCommandData,
                         TRemConAddress& aAddr );
                         
        /**
        * From MRemConBearerInterface
        *
        * Called by RemCon to send a command on a connection.
        * The connection is not assumed to exist- the bearer
        * is responsible for bringing up the requested.
        * connection if necessary.
        * @param aInterfaceUid
        * @param aOperationId
        * @param aTransactionId
        * @param aData, On success, ownership is passed.
        * @param aAddr
        * @return Error code.
        */
        TInt SendCommand( TUid aInterfaceUid,
                          TUint aOperationId,
                          TUint aTransactionId,
                          RBuf8& aData,
                          const TRemConAddress& aAddr );

        /**
        * From MRemConBearerInterface
        *
        * Called by RemCon to send a response on a connection.
        * The connection is not assumed to exist- the bearer is
        * responsible for bringing up the requested connection if necessary.
        * @param aInterfaceUid
        * @param aOperationId
        * @param aTransactionId
        * @param aData, On success, ownership is passed.
        * @param aAddr
        * @return Error code.
        */
        TInt SendResponse( TUid aInterfaceUid,
                           TUint aOperationId,
                           TUint aTransactionId,
                           RBuf8& aData,
                           const TRemConAddress& aAddr );

        /**
        * From MRemConBearerInterface
        *
        * Called by RemCon to establish a bearer-level
        * connection to another party. Completion is signalled
        * back in ConnectConfirm.
        * @param aAddr
        * @return void
        */
        void ConnectRequest( const TRemConAddress& aAddr );

        /**
        * From MRemConBearerInterface
        *
        * Called by RemCon to destroy a bearer-level connection to another party. 
        * Completion is signalled back in DisconnectConfirm.
        * @param aAddr
        * @return void
        */
        void DisconnectRequest( const TRemConAddress& aAddr );

        /**
        * From MRemConBearerInterface
        *
        * Called by RemCon when either (a) the number of controller clients changes 
        * from 0 to 1 or from 1 to 0, or (b) the number of target clients changes 
        * from 0 to 1 or from 1 to 0.
        * @param aControllerPresent
        * @param aTargetPresent
        */
        void ClientStatus( TBool aControllerPresent, TBool aTargetPresent );

        /**
        * From MRemConBearerInterface
        *
        * Called by RemCon to get the capabilities required to make/destroy
        * connections over the bearer, and to send and receive messages over
        * the bearer.
        * @return TSecurityPolicy
        */
        TSecurityPolicy SecurityPolicy() const;

    private: // Enumerations

        // internal states
        enum TAccKeyEventHandlerState
            {
            EAccKeyEventHandlerStateUnknown = -1,
            EAccKeyEventHandlerStateReady,
            EAccKeyEventHandlerStateNewCommand
            };

    private: // Functions from base classes

        /**
        * From MAdaptation
        */
        
        void ProcessResponseL( TAdaptationMessage& aMessage );
       
        void ProcessErrorResponseL( TInt aErrorCode, TAdaptationMessage& aMessage );
        
        void RaiseEventL( TAdaptationMessage& aMessage );

    private:    // Data
        
        // internal state
        TAccKeyEventHandlerState iState;
        
        // event data
        TUid iInterfaceUid;
        TUint iOperationId;
        TUint iTransactionId;
        TBuf8<64> iData;

    };

#endif      // WIREDACCESSORYKEYEVENTHANDLER_H

// End of File
