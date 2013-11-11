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
* Description:  Used as an reference implementation
*
*/



// INCLUDE FILES
#include <AccessoryControl.h>
#include <AccessoryServer.h>
#include <AccConGenericID.h>
#include <AccConfigFileParser.h>
#include <AccPolProprietaryNameValuePairs.h>
#include "ASYReferenceMainServiceBase.h"
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

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// 
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CASYReferenceMainServiceBase::CASYReferenceMainServiceBase()    
    : iAccessoryControl()
    , iAccConfigFileParser( NULL )
    , iState( EStateUnknown )
    , iConnectedAccessory( NULL )
    {

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::CASYReferenceMainServiceBase() - Enter" );
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::CASYReferenceMainServiceBase() - Return" );
    }

// -----------------------------------------------------------------------------
// 
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
void CASYReferenceMainServiceBase::ConstructL()
    {    
    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::ConstructL() - Enter" );
    
    // ConfigFileParser instance is created for GID parsing.
    iAccConfigFileParser = CAccConfigFileParser::NewL( KNullDesC );    
    
    // AccessoryControl instance is created for Accessory Connection / Disconnection handling.
    iAccessoryControl = new ( ELeave ) RAccessoryControl();    

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::ConstructL() - Return" );
    }

// -----------------------------------------------------------------------------
// 
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CASYReferenceMainServiceBase* CASYReferenceMainServiceBase::NewL()
    {

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::NewL() - Enter" );

    CASYReferenceMainServiceBase * self = new ( ELeave ) CASYReferenceMainServiceBase();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::NewL() - Return" );

    return self;
    }

// -----------------------------------------------------------------------------
// 
// destructor.
// -----------------------------------------------------------------------------
//
CASYReferenceMainServiceBase::~CASYReferenceMainServiceBase()
    {

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::~CASYReferenceMainServiceBase() - Enter" );

    if( iAccessoryControl )
        {
        iAccessoryControl->CloseSubSession();
        }
    
    delete iConnectedAccessory;
    iConnectedAccessory = NULL;

    delete iAccessoryControl;
    iAccessoryControl = NULL;

    delete iAccConfigFileParser;
    iAccConfigFileParser = NULL;
    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::~CASYReferenceMainServiceBase() - Return" );
    
    }

// ----------------------------------------------------------------------------------
// CASYReferenceMainServiceBase::StartL
// ----------------------------------------------------------------------------------
//
TInt CASYReferenceMainServiceBase::StartL()
    {
    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::StartL() - Enter" );

    //
    //CASYReferenceMainServiceBase internal state is change to InitWait
    //
  StateChangeL( EStateInitWait );

    TInt err ( KErrNone );
   
   
   	//
   	// Following syntax is used from now on to illustrate ASY sequences.
   	//
   	   	
    //
    // Comments of initialization sequence during terminal boot are marked as [A.n].
    //

    //
    // Comments of accessory connection sequence are marked as [B.n]
    //
    
    //
    // Comments of accessory disconnection sequence are marked as [C.n]
    //    

    //
    // Comments of internal state change of accessory feature are marked as [D.n]
    // Example external headphones are connected to headset control unit.

    //
    // Comments of getting/setting accessory internal state are marked as [E.n]
    //

    // [A.1] Signals Accessory Server that ASY implementation is ready to receive commands.     
    //       Process command by init parameter is received after signal method call 
    //       (Receiver in this example is CASYReferenceCmdHandlerBase class).
    Signal();
    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::StartL() - Return" );

    return err;
    }

// -----------------------------------------------------------------------------
// CASYReferenceMainServiceBase::ProcessResponseL
// -----------------------------------------------------------------------------
//
void CASYReferenceMainServiceBase::ProcessResponseL( TASYMessage& /*aMessage*/ )
    {    
    
    }

// -----------------------------------------------------------------------------
// CASYReferenceMainServiceBase::ProcessErrorResponseL
// -----------------------------------------------------------------------------
//
void CASYReferenceMainServiceBase::ProcessErrorResponseL(
    TInt aErrorCode,
    TASYMessage& aMessage )
    {
    
    (void) aMessage;
    (void) aErrorCode;
    
    }

// -----------------------------------------------------------------------------
// CASYReferenceMainServiceBase::RaiseEventL
// -----------------------------------------------------------------------------
//
void CASYReferenceMainServiceBase::RaiseEventL( TASYMessage& aMessage )
    {
    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::RaiseEventL() - Enter" );
    
    //
    // [B.1], [C.1], [D.1] Accessory events are received from Accessory Driver/Domestic OS.
    //
    if( aMessage.GroupId() == KASYMessageGroupAccessoryEvent )
        {
        // Event processing according the accessory event.
        switch( aMessage.MessageId() )
            {
            case KASYMsgAccessoryConnectionStateChangedEvent:
                {
    
                // Accessory connection state is changed.
                TASYMsgAccessoryConnectionStateChangedEventBuf package;
                package.Copy( aMessage.Data() );
                         
                ConnectionStateChangedL( package() );
                }
                break;
            case KASYMsgAccessoryFeatureStateChangedEvent:
                {
                
                // State of accessory feature is changed.
                TASYMsgAccessoryFeatureStateChangedEventBuf package;
                package.Copy( aMessage.Data() );

                AccessoryFeatureStateBooleanChangedL( package() );
                }
                break;
            default:                
                User::Leave( KErrGeneral );
                break;
            };
        }
    else
        {        
        User::Leave( KErrGeneral );
        }
    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::RaiseEventL() - Return" );
    }


// -----------------------------------------------------------------------------
// CASYReferenceMainServiceBase::StateChangeL
// -----------------------------------------------------------------------------
//
void CASYReferenceMainServiceBase::StateChangeL( TState aState )
    {
    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::StateChangeL() - Enter" );

    TInt err ( KErrGeneral );

    switch ( aState )
        {
        case EStateInitWait:
            {
   
            if ( iState != EStateUnknown )
                {
                User::Leave( KErrGeneral );
                }
            else
                {                
                //
                // initialization of event subscribe(s) can be done here.
                //
                }
            }
            break;
        case EStateInitOngoing:
            {
            
            if ( iState != EStateInitWait )
                {            
                User::Leave( KErrGeneral );
                }
                //
                // Current state extra handling can be added here, if needed.
                //
            }
            break;
        case EStateInitOk:
            {            
            if ( iState != EStateInitOngoing )
                {             
                User::Leave( KErrGeneral );
                }
            else
                { 
                //
                // Initialized sequence is ready, and session can be created to accessory control.
                //                               
                err = iAccessoryControl->CreateSubSession( CASYMainServiceBase::Session() );                

                if ( err != KErrNone )
                    {                    
                    User::Leave( err );
                    }
                }
            }
            break;
        
        default:
            {            
            User::Leave( KErrGeneral );
            }
            break;
        }

    //
    // Example class state change.
    //
    iState = aState;

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::StateChangeL() - Return" );

    }
    

// -----------------------------------------------------------------------------
// CASYReferenceMainServiceBase::ConnectionStateChangedL
// -----------------------------------------------------------------------------
//
void CASYReferenceMainServiceBase::ConnectionStateChangedL( AccessoryConnection::TConnectionStateInfoStruct& aState )
    {

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::ConnectionStateChangedL() - Enter" );
    //
    // Accessory Connection handling example.
    //    
    TRequestStatus status;

    switch( aState.iState )
        {
        case AccessoryConnection::EStateConnected:
            {

            //
            // [B.2] Event of an accessory connection is raised to ASY from a Accessory Driver/Domestic OS.
            //

            // Delete old Generic ID
            if( iConnectedAccessory )
                {
               
                delete iConnectedAccessory;
                iConnectedAccessory = NULL;
                }
                
            TUint64 accessoryId( aState.iAccessoryId );
            

            iConnectedAccessory = CAccConGenericID::NewL();

            // [B.3] Parse a new Generic ID according to the accessory spesific ID.
            iAccConfigFileParser->FindL( iConnectedAccessory, accessoryId, KNullDesC );
   
            // Check if accessory is not supported
            if( ( iConnectedAccessory->GenericID().UniqueID() ) < 0 )
                {
                // Accessory is not supported. Debug purpose.
                }
            else
                {
                // Accessory is supported. Debug purpose.
                }
            
            // [B.4] Send connect accessory request to Accessory Server
            iAccessoryControl->ConnectAccessory( status, iConnectedAccessory, EFalse );
            
            // the WaitForRequest method can be used because ConnectAccessory is immediately complete by
            // Accessory Server.
            User::WaitForRequest( status );
            
            }
            break;
        case AccessoryConnection::EStateNotConnected:
            {

            //
            // [C.2] Event of an accessory disconnection is raised to ASY from a Accessory Driver/Domestic OS.
            //

            // [C.3] Get current Generic ID.
            TAccPolGenericID genericId = iConnectedAccessory->GenericID();

            // [C.4] Send disconnect request to Accessory Server.
            iAccessoryControl->DisconnectAccessory( status, genericId );

            // the WaitForRequest can be used because DisconnectAccessory is immediately complete by
            // Accessory Server.
            User::WaitForRequest( status );
           
            delete iConnectedAccessory;
            iConnectedAccessory = NULL;
            }
            break;
        default:
            {
            User::Leave( KErrGeneral );
            }
            break;
        }

    if ( status.Int() != KErrNone )
        {

        }

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::ConnectionStateChangedL() - Return" );
    }


// -----------------------------------------------------------------------------
// CASYReferenceMainServiceBase::AccessoryFeatureStateBooleanChangedL
// -----------------------------------------------------------------------------
void CASYReferenceMainServiceBase::AccessoryFeatureStateBooleanChangedL( AccessoryFeature::TFeatureStateBooleanStruct& aState )
    {

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::AccessoryFeatureStateBooleanChangedL() - Enter" );

    // [D.2] the accessory internal state is change.
    
    TASYCommandParamRecord record;

    // See accessory feature example --> MapAccessoryFeatureL method. 
    MapAccessoryFeatureL( aState.iFeature, record );
    
    TAccValueTypeTBool state;
    state.iValue = aState.iState;

    //Current Generic ID
    TAccPolGenericID genericId = iConnectedAccessory->GenericID();    

    // [D.4] Accessory internal state change is indicated to Accessory Server.  
    iAccessoryControl->AccessoryValueChangedNotifyL( genericId, record.iNameRecord, state );

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::AccessoryFeatureStateBooleanChangedL() - Return" );
    
    }
    
// -----------------------------------------------------------------------------
// CASYReferenceMainServiceBase::MapAccessoryFeatureL
// -----------------------------------------------------------------------------
void CASYReferenceMainServiceBase::MapAccessoryFeatureL( AccessoryFeature::TAccessoryFeature& aAccessoryFeature, 
                                                         TASYCommandParamRecord& aCommandParamRecord )
    {
        
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::AccessoryFeatureStateBooleanChangedL() - Enter" );

    // Couple feature examples
    switch( aAccessoryFeature )
        {
        
        // [D.3]
        //A headphone is not connected to the sound adapter which means that the accessory is not detected as a control connection (the headset icon is not shown on the phone display).
        //Audio Out Connector capability value is updated by ASY when the headphone is connected to the sound adapter, which means that the accessory connection is updated as a control connection.
         
        case AccessoryFeature::EHeadphonesConnected:
            {            
            aCommandParamRecord.iNameRecord.SetNameL( KAccAudioOutConnector );
            }
            break;
        case AccessoryFeature::EExternalPowerSourceConnected:
            {            
            aCommandParamRecord.iNameRecord.SetNameL( KAccExtPowerSupply );
            }
            break;
        
        default:
            {            
            User::Leave( KErrGeneral );
            }
            break;
        }    

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceMainServiceBase::AccessoryFeatureStateBooleanChangedL() - Return" );

    }

// End of file

