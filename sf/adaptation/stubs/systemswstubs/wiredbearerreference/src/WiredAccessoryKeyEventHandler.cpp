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


// INCLUDE FILES
#include <remcon/remconconverterplugin.h>
#include <remcon/bearerparams.h>
#include <remconaddress.h>
#include <RemConKeyEventData.h>
#include <remconcoreapi.h>
#include "WiredAccessoryKeyEventHandler.h"
#include "AdaptationMessage.h"

#include "AdaptationMessageDefinitions.h"
#include "acc_debug.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::CWiredAccessoryKeyEventHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWiredAccessoryKeyEventHandler::CWiredAccessoryKeyEventHandler( TBearerParams& aBearerParams )
    : CRemConBearerPlugin( aBearerParams )    
    , iState( EAccKeyEventHandlerStateUnknown )
    , iOperationId( 0 )
    , iTransactionId( 0 )
    {
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::CWiredAccessoryKeyEventHandler() - Enter" );
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::CWiredAccessoryKeyEventHandler() - Return" );
    
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWiredAccessoryKeyEventHandler::ConstructL()
    {
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::ConstructL() - Enter" );	   
    //
    // initialization of event subscribe(s) can be done here.
    //
    iState = EAccKeyEventHandlerStateReady;
    
    
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::ConstructL() - Return" );
    
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWiredAccessoryKeyEventHandler* CWiredAccessoryKeyEventHandler::NewL( TBearerParams& aBearerParams )
    {    
    CWiredAccessoryKeyEventHandler* self = new (ELeave) CWiredAccessoryKeyEventHandler( aBearerParams );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// Destructor
CWiredAccessoryKeyEventHandler::~CWiredAccessoryKeyEventHandler()
    {
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::~CWiredAccessoryKeyEventHandler() - Enter" );
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::~CWiredAccessoryKeyEventHandler() - Return" );    
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::GetInterface
// -----------------------------------------------------------------------------
//
TAny* CWiredAccessoryKeyEventHandler::GetInterface( TUid aUid )
    {

    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::GetInterface() - Enter" );
    
    
    TAny* bearer = NULL;

    if ( aUid == TUid::Uid( KRemConBearerInterface1 ) )
        {
        bearer = reinterpret_cast<TAny*>( static_cast<MRemConBearerInterface*>( this ) );
        }
    else
        {
        // Interface is not supported!
        }
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::GetInterface() - Return" );

    return bearer;
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::GetResponse
// -----------------------------------------------------------------------------
//
TInt CWiredAccessoryKeyEventHandler::GetResponse( TUid& aInterfaceUid,
                                                  TUint& aTransactionId,
                                                  TUint& aOperationId,
                                                  RBuf8& aCommandData,
                                                  TRemConAddress& aAddr )
    {

    // Only events are forwarded, thus no need to implement get response
    TInt err ( KErrNotSupported );
                              
    ( void ) aTransactionId;
    ( void ) aOperationId;
    ( void ) aCommandData;
    ( void ) aAddr;
    ( void ) aInterfaceUid;

    return err;
    }

// -----------------------------------------------------------------------------
// CSACAccessoryKeyEventHandler::GetCommand
// -----------------------------------------------------------------------------
//
TInt CWiredAccessoryKeyEventHandler::GetCommand( TUid& aInterfaceUid,
                                                 TUint& aTransactionId,
                                                 TUint& aOperationId,
                                                 RBuf8& aCommandData,
                                                 TRemConAddress& aAddr )
    { 
    
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::GetCommand() - Enter" );
    TInt err ( KErrNone );
    
    if ( iState != EAccKeyEventHandlerStateNewCommand )
        {        
        err = KErrNotReady;
        }
    else
        {
        aInterfaceUid  = iInterfaceUid;
        aTransactionId = iTransactionId++;
        aOperationId   = iOperationId;
        
        aAddr.BearerUid() = Uid();
        aAddr.Addr()      = KNullDesC8();
        
        iState = EAccKeyEventHandlerStateReady;
        
        TRAP( err, aCommandData.CreateL( iData ) );
        
        if ( err != KErrNone )
            {
            //GetCommand - failed to create data
            }
        }

    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::GetCommand() - Return" );
    return err;
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::SendCommand
// -----------------------------------------------------------------------------
//
TInt CWiredAccessoryKeyEventHandler::SendCommand( TUid aInterfaceUid,
                                                  TUint aOperationId,
                                                  TUint aTransactionId,
                                                  RBuf8& aData,
                                                  const TRemConAddress& aAddr )
    {

    // Only events are forwarded, thus no need to implement send command
    TInt err ( KErrNotSupported );

    ( void ) aTransactionId;
    ( void ) aOperationId;
    ( void ) aAddr;
    ( void ) aInterfaceUid;
    ( void ) aData;
                    
    return err;
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::SendResponse
// -----------------------------------------------------------------------------
//
TInt CWiredAccessoryKeyEventHandler::SendResponse( TUid aInterfaceUid,
                                                   TUint aOperationId,
                                                   TUint aTransactionId,
                                                   RBuf8& aData,
                                                   const TRemConAddress& aAddr )
    {
    
    // Only events are forwarded, thus no need to implement send response
    TInt err ( KErrNotSupported );

    ( void ) aTransactionId;
    ( void ) aOperationId;
    ( void ) aAddr;
    ( void ) aInterfaceUid;
    ( void ) aData;
    
    return err;
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::ConnectRequest
// -----------------------------------------------------------------------------
//
void CWiredAccessoryKeyEventHandler::ConnectRequest( const TRemConAddress& aAddr )
    {

    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::ConnectRequest() - Enter" );
    
            
    TInt err = Observer().ConnectConfirm( aAddr, KErrNone );    

    if ( err != KErrNone )
        {
        // ConnectConfirm failed
        }        
        
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::ConnectRequest() - Return" );
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::DisconnectRequest
// -----------------------------------------------------------------------------
//
void CWiredAccessoryKeyEventHandler::DisconnectRequest( const TRemConAddress& aAddr )
    {            
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::DisconnectRequest() - Enter" );
         
    Observer().DisconnectConfirm( aAddr, KErrNone );    
    
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::DisconnectRequest() - Return" );
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::ClientStatus
// -----------------------------------------------------------------------------
//
void CWiredAccessoryKeyEventHandler::ClientStatus( TBool aControllerPresent, TBool aTargetPresent )
    {    
    ( void ) aTargetPresent;
    ( void ) aControllerPresent;           
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::SecurityPolicy
// -----------------------------------------------------------------------------
//
TSecurityPolicy CWiredAccessoryKeyEventHandler::SecurityPolicy() const
    {
    
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::SecurityPolicy() - Enter" );
    TSecurityPolicy securityPolicy ( ECapabilitySwEvent );
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::SecurityPolicy() - Return" );
    return securityPolicy;
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::ProcessResponseL
// -----------------------------------------------------------------------------
//
void CWiredAccessoryKeyEventHandler::ProcessResponseL( TAdaptationMessage& aMessage )
    {
    ( void ) aMessage;
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::ProcessErrorResponseL
// -----------------------------------------------------------------------------
//
void CWiredAccessoryKeyEventHandler::ProcessErrorResponseL( TInt aErrorCode,
                                                          TAdaptationMessage& aMessage )
    {    
    ( void ) aErrorCode;
    ( void ) aMessage;
    }

// -----------------------------------------------------------------------------
// CWiredAccessoryKeyEventHandler::RaiseEventL
// -----------------------------------------------------------------------------
//
void CWiredAccessoryKeyEventHandler::RaiseEventL( TAdaptationMessage& aMessage )
    {
    
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::RaiseEventL() - Enter" );
    //Accessory key events are received from Accessory Driver/Domestic OS.
    
    if ( iState != EAccKeyEventHandlerStateReady )
        {
        User::Leave( KErrNotReady );
        }

    // only key events are supported
    if ( aMessage.GroupId() != KBearerMessageGroupAccessoryEvent && aMessage.MessageId() != KBearerMessageGroupAccessoryKeyEvent )
        {        
        User::Leave( KErrArgument );
        }

    TRemConMessageType type( ERemConCommand );
    
    // Note!
    // following uid 0x102069AA should be used if platform key event converter is used, 
    // uid is hard coded to platfrom converter which is returned to remotecontrol.
    // Data should give as format defined in S60_Accessory_Key_Event_Handling_API_Specification_C.doc.
    const TInt KWireBearerUid = 0x102069AA;

    TInt err = Observer().BearerToInterface( TUid::Uid( KWireBearerUid ), 
                                             aMessage.Data(), 
                                             aMessage.Data(), 
                                             iInterfaceUid, 
                                             iOperationId, 
                                             type, 
                                             iData );     
    if ( err == KErrNotSupported )
        {
    
        }
    else if ( err == KErrNone )
        {
        iState = EAccKeyEventHandlerStateNewCommand;

        TRemConAddress addr;
        addr.BearerUid() = Uid();
        addr.Addr() = KNullDesC8();
        
        err = Observer().NewCommand( addr );
        
        if ( err != KErrNone )
            {
            User::Leave( err );
            }
        }
    else
        // leave if converting fails
        {       
        User::Leave( err );
        }    
        
    COM_TRACE_( "[AccFW:WiredBearerReference] CWiredAccessoryKeyEventHandler::RaiseEventL() - Return" );    
    }

//  End of File
