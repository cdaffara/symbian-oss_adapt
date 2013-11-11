/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Reference implementation of SSY Channel
*
*/


#include <ssycallback.h>                     // MSsyCallback
#include "ssyreferencechannel.h"
#include "ssyreferencecontrol.h"             // SSY Control
#include "ssyreferencepropertyprovider.h"    // iChannelPropertyProvider
#include "ssyreferencechanneldataprovider.h" // iChannelDataProvider
#include "ssyreferencetrace.h"
#include "ssyreferencecmdhandler.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSsyReferenceChannel C++ constructor
// ---------------------------------------------------------------------------
//
CSsyReferenceChannel::CSsyReferenceChannel( CSsyReferenceControl& aSsyControl, TSensrvChannelInfo aChannelInfo ) :
    iSsyControl( aSsyControl ),
    iChannelInfo( aChannelInfo ),
    iState( ESsyReferenceChannelIdle )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::CSsyReferenceChannel()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::CSsyReferenceChannel() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// Symbian 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannel::ConstructL()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::ConstructL()" ) ) );

    // Create command handler
    iCmdHandler = CSsyReferenceCmdHandler::NewL( *this );
    
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::ConstructL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::NewL
// ---------------------------------------------------------------------------
//
CSsyReferenceChannel* CSsyReferenceChannel::NewL( CSsyReferenceControl& aSsyControl, TSensrvChannelInfo aChannelInfo )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::NewL()" ) ) );
    CSsyReferenceChannel* self = new ( ELeave ) CSsyReferenceChannel( aSsyControl, aChannelInfo );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::NewL() - return" ) ) );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CSsyReferenceChannel::~CSsyReferenceChannel()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::~CSsyReferenceChannel()" ) ) );

    // In case channel is not closed before destruction, providers are not deleted
    if ( iChannelDataProvider )
        {
        delete iChannelDataProvider;
        iChannelDataProvider = NULL;
        }
    
    if ( iChannelPropertyProvider )
        {
        delete iChannelPropertyProvider;
        iChannelPropertyProvider = NULL;
        }

    if ( iCmdHandler )
        {
        delete iCmdHandler;
        iCmdHandler = NULL;
        }

    iProperties.Reset();
    
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::~CSsyReferenceChannel() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::ChannelId
// ---------------------------------------------------------------------------
//
TInt CSsyReferenceChannel::ChannelId()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::ChannelId() - %i" ), iChannelInfo.iChannelId ) );
    return iChannelInfo.iChannelId;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::SsyControl
// ---------------------------------------------------------------------------
//
CSsyReferenceControl& CSsyReferenceChannel::SsyControl() const
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::SsyControl()" ) ) );
    return iSsyControl;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceControl::SsyCmdHandler
// ---------------------------------------------------------------------------
//
CSsyReferenceCmdHandler& CSsyReferenceChannel::CommandHandler() const
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::SsyCmdHandler()" ) ) );
    return *iCmdHandler;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::FindPropertyL
// ---------------------------------------------------------------------------
//
TSensrvProperty& CSsyReferenceChannel::FindPropertyL( 
    const TSensrvPropertyId aPropertyId, 
    TInt aItemIndex,
    TInt aArrayIndex )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::FindPropertyL()" ) ) );
    TSensrvProperty* property = NULL;
    TBool propertyFound( EFalse );

    // Search property
    for ( TInt i = 0; i < iProperties.Count() && !propertyFound; i++ )
        {
        property = static_cast<TSensrvProperty*>( &iProperties[i] );

        // Compare property IDs and array index
        if ( property->GetPropertyId() == aPropertyId )
            {
            // Compare item index if it is given
            if ( ( KErrNotFound == aItemIndex ) || ( property->PropertyItemIndex() == aItemIndex ) )
                {
                // Correct property ID is found, now check is it array type of property.
                // Either array indexes must match or propertys array index has to be array info
                if ( ( property->GetArrayIndex() == aArrayIndex ) || 
                     ( ( property->GetArrayIndex() == ESensrvArrayPropertyInfo ) && 
                       ( ESensrvSingleProperty == aArrayIndex ) ) )
                    {
                    // Correct array index found
                    propertyFound = ETrue;    
                    }
                }
            }
        }

    // Leave if not found
    if ( !propertyFound )
        {
        iSsyControl.FindPropertyL( aPropertyId, aArrayIndex, *property );
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::FindPropertyL() - return" ) ) );
    return *property;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::FindAndUpdatePropertyL
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannel::FindAndUpdatePropertyL( const TSensrvProperty& aProperty )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::FindAndUpdatePropertyL()" ) ) );
    TBool propertyFound( EFalse );

    // Search property
    for ( TInt i = 0; i < iProperties.Count() && !propertyFound; i++ )
        {
        // Compare property IDs
        if ( iProperties[i].GetPropertyId() == aProperty.GetPropertyId() )
            {
            // Compare item index if it is given
            if ( ( KErrNotFound != aProperty.PropertyItemIndex() ) && 
                 ( iProperties[i].PropertyItemIndex() == aProperty.PropertyItemIndex() ) )
                {
                // Property found -> update if possible
                if ( iProperties[i].ReadOnly() )
                    {
                    User::Leave( KErrAccessDenied );
                    }
                // If modifiable, get type and update value
                switch ( iProperties[i].PropertyType() )
                    {
                    case ESensrvIntProperty:
                        {
                        TInt value( 0 );
                        aProperty.GetValue( value );
                        iProperties[i].SetValue( value );
                        break;
                        }
                    case ESensrvRealProperty:
                        {
                        TReal value( 0 );
                        aProperty.GetValue( value );
                        iProperties[i].SetValue( (TReal) value );
                        break;
                        }
                    case ESensrvBufferProperty:
                        {
                        TBuf8<20> propValue;
                        aProperty.GetValue( propValue );
                        iProperties[i].SetValue( propValue );
                        break;
                        }
                    default:
                        {
                        break;
                        }
                    }
                propertyFound = ETrue;
                }
            }
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::FindPropertyL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::GetProperties
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannel::GetProperties( RSensrvPropertyList& aPropertyList )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::GetProperties()" ) ) );
    // Copy properties one by one to param aPropertyList
    TInt propCount( iProperties.Count() );
    RSensrvPropertyList propList( propCount );

    for ( TInt i = 0; i < propCount; i++ )
        {
        propList.Append( iProperties[i] );
        }

    aPropertyList = propList;
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::GetProperties() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::UpdateState
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannel::UpdateState( const TSsyReferenceChannelState aNewState )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::UpdateState() - %i" ), aNewState ) );
    iState = aNewState;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::ProcessResponse
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannel::ProcessResponse( TSsyReferenceMsg* aMessage )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::ProcessResponse()" ) ) );
    
    if ( aMessage ) 
        {
        switch ( aMessage->Function() )
            {
            case ESsyReferenceOpenChannelResp:
                {
                // Open channel specific handling here
                TRAPD( err, HandleOpenChannelRespL( aMessage->Error() ) );
                if ( KErrNone != err )
                    {
                    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::ProcessResponse() - Error opening channel: %i" ), err ) );
                    }
                break;
                }
            case ESsyReferenceDataItemReceived:
                {
                // Send data item to data provider
                TRAPD( err, iChannelDataProvider->ChannelDataReceivedL( aMessage ) );
                if ( KErrNone != err )
                    {
                    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::ProcessResponse() - Error receiving data: %i" ), err ) );
                    }
                break;  
                }
            case ESsyReferenceCloseChannelResp:
                {
                // Close channel specific handling here
                HandleCloseChannelResp();
                break;  
                }
            default:
                {
                // This command was not intended to process here, try Control class
                iSsyControl.ProcessResponse( aMessage );    
                COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::ProcessResponse() - Unknown function" ) ) );
                }
            }
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::ProcessResponse() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::OpenChannel
// ---------------------------------------------------------------------------
//
TInt CSsyReferenceChannel::OpenChannel()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::OpenChannel()" ) ) );

    TInt err( KErrAlreadyExists );

    // Check that this channel is not already open
    if ( ESsyReferenceChannelIdle == iState )
        {
        // Update state and issue request. Will continue in HandleOpenChannelResp
        UpdateState( ESsyReferenceChannelOpening );

        // Create message with function spesific information
        // and pass it to command handler
        err = iCmdHandler->ProcessCommand( TSsyReferenceMsg( ChannelId(), ESsyReferenceOpenChannel ) );
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::OpenChannel() - return" ) ) );
    return err;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::HandleOpenChannelResp
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannel::HandleOpenChannelRespL( const TInt aError )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::HandleOpenChannelResp()" ) ) );

    // Open channel asynhronously and complete request with MSsyCallback::ChannelOpened() when
    // channel is opened.

    // Create instance of the data provider of this channel
    iChannelDataProvider = CSsyReferenceChannelDataProvider::NewL( *this );
    // Create instance of the property provider of this channel
    iChannelPropertyProvider = CSsyReferencePropertyProvider::NewL( *this );

    TInt error( aError );

    // If channel opening succeeds, update state to Open
    if ( KErrNone == aError )
        {
        // Update state to Open
        UpdateState( ESsyReferenceChannelOpen );
        // Get channel properties
        TRAP( error, iSsyControl.SsyConfig().GetChannelPropertiesL( ChannelId(), iProperties ) );
        }
    else
        {
        // Channel opening failed, back to idle
        UpdateState( ESsyReferenceChannelIdle ); 
        }

    // Complete transaction
    iSsyControl.SsyCallback().ChannelOpened( ChannelId(),
                                error,
                                iChannelDataProvider,
                                iChannelPropertyProvider );

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::HandleOpenChannelResp() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::CloseChannel
// ---------------------------------------------------------------------------
//
TInt CSsyReferenceChannel::CloseChannel()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::CloseChannel()" ) ) );

    TInt err( KErrNotFound );

    // Check that this channel is open
    if ( ESsyReferenceChannelOpen == iState )
        {
        // Update state and issue request. Will continue in HandleCloseChannelResp
        UpdateState( ESsyReferenceChannelClosing );
        // Create message with function spesific information
        // and pass it to command handler
        err = iCmdHandler->ProcessCommand( TSsyReferenceMsg( ChannelId(), ESsyReferenceCloseChannel ) );
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::CloseChannel() - return" ) ) );
    return err;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannel::HandleCloseChannelResp
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannel::HandleCloseChannelResp()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::HandleCloseChannelResp()" ) ) );

    // Close channel and complete request with MSsyCallback::ChannelClosed() when
    // channel is closed.

    // Delete providers
    delete iChannelDataProvider;
    iChannelDataProvider = NULL;

    delete iChannelPropertyProvider;
    iChannelPropertyProvider = NULL;

    // Update state to idle
    UpdateState( ESsyReferenceChannelIdle );

    // Reset properties
    iProperties.Reset();

    // Complete transaction
    iSsyControl.SsyCallback().ChannelClosed( ChannelId() );

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannel::HandleCloseChannelResp() - return" ) ) );
    }

// End of file
