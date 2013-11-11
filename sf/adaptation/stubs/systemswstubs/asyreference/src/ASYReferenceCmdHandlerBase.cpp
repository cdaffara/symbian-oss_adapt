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

#include "ASYReferenceCmdHandlerBase.h"
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
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CASYReferenceCmdHandlerBase* CASYReferenceCmdHandlerBase::NewL()
    { 
     
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::NewL() - Enter" );

    CASYReferenceCmdHandlerBase * self = new(ELeave) CASYReferenceCmdHandlerBase( );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::NewL() - Return" );

    return self;
    }

// -----------------------------------------------------------------------------
// destructor.
// -----------------------------------------------------------------------------
//
CASYReferenceCmdHandlerBase::~CASYReferenceCmdHandlerBase()
    {        
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::~CASYReferenceCmdHandlerBase() - Enter" );
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::~CASYReferenceCmdHandlerBase() - Return" );
    }

// -----------------------------------------------------------------------------
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CASYReferenceCmdHandlerBase::CASYReferenceCmdHandlerBase()        
    {    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::CASYReferenceCmdHandlerBase() - Enter" );
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::CASYReferenceCmdHandlerBase() - Return" );
    }

// -----------------------------------------------------------------------------
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
void CASYReferenceCmdHandlerBase::ConstructL()
    {
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::CASYReferenceCmdHandlerBase() - Enter" );
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::CASYReferenceCmdHandlerBase() - Return" );
    }

// -----------------------------------------------------------------------------
// CASYReferenceCmdHandlerBase::ProcessCommandL
// -----------------------------------------------------------------------------
//
void CASYReferenceCmdHandlerBase::ProcessCommandL(
     const TProcessCmdId aCommand,
     const TASYCmdParams& aCmdParams )
    {

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::ProcessCommandL() - Enter" );

    CASYReferenceMainServiceBase* connectionHandler = (CASYReferenceMainServiceBase *) ASYMainServiceBase();

    if( !connectionHandler )
        {        
        User::Leave( KErrGeneral );
        }

    switch( aCommand )
        {
        case ECmdProcessCommandInit:
            {

            // Command received from Accessory Server.

            // Connection handler state is set ongoing.
            connectionHandler->StateChangeL( CASYReferenceMainServiceBase::EStateInitOngoing );                        
            
            // [A.2] Process command init is received from Accessory Server
            //       At this point it is possible to request from an Accessory Driver/Domestic OS if the accessory connection exists.                                     
            //       In this example the Process response is send immediately to Accessory Server.
            TASYMsgAccessoryConnectionHandlerInitRetBuf package;

            // Accessory is not connected to terminal.
            package().iStateInfo.iState = AccessoryConnection::EStateNotConnected;
            package().iStateInfo.iAccessoryId = 0xffffff;
            TASYMessage message( KASYMessageGroupAccessoryMessage, KASYMsgAccessoryConnectionHandlerInitRet, package );            

            // Call straight ProcessResponse.
            ProcessResponseL( message );

            }
            break;

        case ECmdGetValueBool:
            {
                       
            // [E.1] Command received from Accessory Server

            TASYMsgAccessoryGetValueBooleanCmdBuf cmdPackage;                                 

            // Map and set feature
            MapCommandToAccessoryFeatureL( aCmdParams(), cmdPackage().iFeature );

            // Set Device ID
            TInt32 deviceId = ( TInt32 ) aCmdParams().iGenericID.HWDeviceID();            
            cmdPackage().iAccessoryId = deviceId;
            
            //
            // [E.2] Send request to an Accessory Driver/Domestic OS.
            //

            }
            break;
       

        case ECmdSetValueBool:
            {

            // [E.1] Command received from Accessory Server

            TASYMsgAccessorySetValueBooleanCmdBuf cmdPackage;            

            // Map and set feature
            MapCommandToAccessoryFeatureL( aCmdParams(), cmdPackage().iFeature );

            // Set Device ID
            TInt32 deviceId = ( TInt32 ) aCmdParams().iGenericID.HWDeviceID();            
            cmdPackage().iAccessoryId = deviceId;

            if( aCmdParams().iCmdValue )
                {            
                cmdPackage().iState = ETrue;
                }
            else
                {                
                cmdPackage().iState = EFalse;
                }
                        
            //
            // [E.2] Send request to an Accessory Driver/Domestic OS
            //            

            }
            break;

        default:

            User::Leave( KErrNotSupported );
            break;
        }
    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::ProcessCommandL() - Enter" );
    }

// -----------------------------------------------------------------------------
// CASYReferenceCmdHandlerBase::ProcessResponseL
// -----------------------------------------------------------------------------
//
void CASYReferenceCmdHandlerBase::ProcessResponseL( TASYMessage& aMessage )
    {
   
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::ProcessResponseL() - Enter" );
    // Response for previous made process command, received from an Accessory Driver/Domestic OS.

    CASYReferenceMainServiceBase* connectionHandler = (CASYReferenceMainServiceBase *) ASYMainServiceBase();  

    if( !connectionHandler )
        {        
        User::Leave( KErrGeneral );
        }

    switch ( aMessage.GroupId() )
        {
        case KASYMessageGroupAccessoryMessage:

            switch ( aMessage.MessageId() )
                {
                
                case KASYMsgAccessoryConnectionHandlerInitRet:
                    {
                                        
                    connectionHandler->StateChangeL( CASYReferenceMainServiceBase::EStateInitOk );

                    TASYMsgAccessoryConnectionHandlerInitRetBuf package;
                    package.Copy( aMessage.Data() );
                    
                    if( package().iStateInfo.iState == AccessoryConnection::EStateConnected )
                        {
                        //
                        //  Accessory connection exists in Accessory Driver/Domestic OS, inform accessory connection handler.
                        //
                        connectionHandler->ConnectionStateChangedL( package().iStateInfo );
                        }

                    TAccValueTypeTBool state;
                    state.iValue = ETrue;
                    
                     // [A.3] After process responce for the previous made process command with init parameter
                     //       Server is aware wheather accessory was connected or not ( to Accessory Server ). 
                   
                    CASYCommandHandlerBase::ProcessResponseL( state, KErrNone );                    
       
                    }
                    break;
                case KASYMsgAccessoryGetValueBooleanRet:
                    {
                                                       
                    //
                    // [E.3] Send response previous made process command ( to Accessory Server ).
                    //
                    TASYMsgAccessoryGetValueBooleanRetBuf response;
                    response.Copy( aMessage.Data() );

                    TAccValueTypeTBool state;
                    state.iValue = response().iFeatureState.iState;                    
                    CASYCommandHandlerBase::ProcessResponseL( state, KErrNone );

                    }
                    break;
               
                case KASYMsgAccessorySetValueBooleanRet:
                    {
                                        
                    //
                    // [E.3] Send response previous made process command ( to Accessory Server ).
                    //
                   
                    TASYMsgAccessorySetValueBooleanRetBuf response;
                    response.Copy( aMessage.Data() );

                    TAccValueTypeTBool state;
                    TInt errorCode = response().iErrorCode;
                    state.iValue = response().iFeatureState.iState;                    
                    CASYCommandHandlerBase::ProcessResponseL( state, errorCode );
                    
                    }
                    break;

                default:
                    {
                    
                    User::Leave( KErrGeneral );
                    }
                    break;
                }

            break;

        default:
            {
            
            User::Leave( KErrGeneral );
            }
            break;
        }

    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::ProcessResponseL() - Return" );
    }

// -----------------------------------------------------------------------------
// CSACAccessoryCommandHandler::ProcessErrorResponseL
// -----------------------------------------------------------------------------
//
void CASYReferenceCmdHandlerBase::ProcessErrorResponseL( TInt /*aErrorCode*/, 
                                                         TASYMessage& /*aMessage*/ )
    {

    }


// -----------------------------------------------------------------------------
// 
// -----------------------------------------------------------------------------
void CASYReferenceCmdHandlerBase::MapCommandToAccessoryFeatureL( const TASYCommandParamRecord& aCommandParamRecord, 
                                                                 AccessoryFeature::TAccessoryFeature& aAccessoryFeature )
    {
    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::MapCommandToAccessoryFeatureL() - Enter" );    
    TUint32 param;

    //Couple of accessory feature examples which can be requested from an Accessory Driver/Domestic OS.    
    aCommandParamRecord.iNameRecord.GetName( param );
        
    if( param == KAccAudioOutConnector )
        {        
        aAccessoryFeature = AccessoryFeature::EHeadphonesConnected;
        }
    else if( param == KAccExtPowerSupply )
        {
        
        aAccessoryFeature = AccessoryFeature::EExternalPowerSourceConnected;
        }    
    else
        {
        
        User::Leave( KErrArgument );
        }    
    COM_TRACE_( "[AccFW:AsyReference] CASYReferenceCmdHandlerBase::MapCommandToAccessoryFeatureL() - Return" );    
    }
// -----------------------------------------------------------------------------
// 
// -----------------------------------------------------------------------------
//
void CASYReferenceCmdHandlerBase::RaiseEventL( TASYMessage& aMessage )
    {
   
    (void) aMessage;
    }

// End of file

