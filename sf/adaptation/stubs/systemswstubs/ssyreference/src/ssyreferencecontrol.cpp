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
* Description:  Reference implementation of SSY Control
*
*/


#include <ssycallback.h>                     // MSsyCallback
#include "ssyreferencecontrol.h"
#include "ssyreferencetrace.h"
#include "ssyreferencechannel.h"
#include "ssyreferencecmdhandler.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSsyReferenceControl C++ constructor
// ---------------------------------------------------------------------------
//
CSsyReferenceControl::CSsyReferenceControl( MSsyCallback& aSsyCallback ) :
    iSsyCallback( aSsyCallback )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::CSsyReferenceControl()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::CSsyReferenceControl() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// Symbian 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CSsyReferenceControl::ConstructL()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::ConstructL()" ) ) );

    // Create configurator and start config file parsing
    iConfigFile = CSsyReferenceConfig::NewL();
    TRAPD( err, iConfigFile->InitConfigL() ); // This will block until config is ready

    if ( KErrNone != err )
        {
        COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::ConstructL() - Init config failed: %i" ), err ) );
        }

    // ---------------------------------------------------------------

    // Store channel count for later use
    const TInt channelCount( iConfigFile->ChannelCount() );

    // Instantiate channel info list
    RSensrvChannelInfoList channelInfoList( channelCount );
    CleanupClosePushL( channelInfoList );

    // Fills channel info list with generated channel info objects
    iConfigFile->GenerateChannels( channelInfoList );

    // Register channels. Sensor Server generates unique ID for each channel
    iSsyCallback.RegisterChannelsL( channelInfoList );

    // Update channel IDs to ConfigFile
    iConfigFile->UpdateChannelIds( channelInfoList );

    // Create channels
    iChannelArray = new ( ELeave ) CArrayPtrFlat<CSsyReferenceChannel>( channelCount );
    for ( TInt i = 0; i < channelCount; i++ )
        {
        CSsyReferenceChannel* channel = CSsyReferenceChannel::NewL( *this, channelInfoList[i] );
        iChannelArray->AppendL( channel );
        }

    // Clean up
    CleanupStack::PopAndDestroy( &channelInfoList );

    // Get properties of this SSY. Leaves with KErrNotFound if not found. These properties are 
    // not mandatory, so we can ignore that leave
    TRAP_IGNORE( iConfigFile->GetSensorPropertiesL( iProperties ) );

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::ConstructL() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// CSsyReferenceControl::NewL
// ---------------------------------------------------------------------------
//
CSsyReferenceControl* CSsyReferenceControl::NewL( MSsyCallback& aSsyCallback )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::NewL()" ) ) );
    CSsyReferenceControl* self = new ( ELeave ) CSsyReferenceControl( aSsyCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::NewL() - return" ) ) );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CSsyReferenceControl::~CSsyReferenceControl()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::~CSsyReferenceControl()" ) ) );

    if ( iChannelArray )
        {
        if ( iChannelArray->Count() )
            {
            iChannelArray->ResetAndDestroy();
            }
        
        delete iChannelArray;
        }

    if ( iConfigFile ) 
        {
        delete iConfigFile;
        iConfigFile = NULL;
        }

    iProperties.Reset();

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::~CSsyReferenceControl() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceControl::SsyCallback
// ---------------------------------------------------------------------------
//
MSsyCallback& CSsyReferenceControl::SsyCallback() const
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::SsyCallback()" ) ) );
    return iSsyCallback;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceControl::SsyConfig
// ---------------------------------------------------------------------------
//
CSsyReferenceConfig& CSsyReferenceControl::SsyConfig() const
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::SsyConfig()" ) ) );
    return *iConfigFile;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceControl::FindPropertyL
// ---------------------------------------------------------------------------
//
void CSsyReferenceControl::FindPropertyL( 
    const TSensrvPropertyId aPropertyId, 
    const TInt aArrayIndex,
    TSensrvProperty& aProperty )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::FindPropertyL()" ) ) );
    TSensrvProperty* property = NULL;
    TBool propertyFound( EFalse );

    // Search property
    for ( TInt i = 0; i < iProperties.Count() && !propertyFound; i++ )
        {
        property = static_cast<TSensrvProperty*>( &iProperties[i] );

        // Compare property IDs
        if ( property->GetPropertyId() == aPropertyId )
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

    // Leave if not found
    if ( !propertyFound )
        {
        User::Leave( KErrNotFound );
        }

    aProperty = *property;

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::FindPropertyL() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// CSsyReferenceControl::FindChannel
// ---------------------------------------------------------------------------
//
CSsyReferenceChannel* CSsyReferenceControl::FindChannelL( TSensrvChannelId aChannelID )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::FindChannel()" ) ) );
    
    if ( !iChannelArray )
        {
        User::Leave( KErrNotFound );
        }

    const TInt channelCount( iChannelArray->Count() );
    CSsyReferenceChannel* channel = NULL;

    // Check that there are channels
    if ( channelCount ) 
        {
        // Loop channels until correct channel is found
        for ( TInt i = 0; i < channelCount; i++ ) 
            {
            channel = iChannelArray->At( i );
            
            // Compare channel id
            if ( channel->ChannelId() == aChannelID )
                {
                // Channel found, no need to loop rest
                i = channelCount;
                }
            }
        }

    // Leave if channel is not found
    if ( !channel )
        {
        User::Leave( KErrNotFound );
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::FindChannel() - return" ) ) );
    return channel;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceControl::OpenChannelL
// ---------------------------------------------------------------------------
//
void CSsyReferenceControl::OpenChannelL( TSensrvChannelId aChannelID )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::OpenChannelL()" ) ) );
    // Find and open channel
    User::LeaveIfError( FindChannelL( aChannelID )->OpenChannel() );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::OpenChannelL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceControl::CloseChannelL
// ---------------------------------------------------------------------------
//
void CSsyReferenceControl::CloseChannelL( TSensrvChannelId aChannelID )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::CloseChannelL()" ) ) );
    // Find and close channel
    User::LeaveIfError( FindChannelL( aChannelID )->CloseChannel() );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::CloseChannelL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceControl::ProcessResponse
// ---------------------------------------------------------------------------
//
void CSsyReferenceControl::ProcessResponse( TSsyReferenceMsg* /*aMessage*/ )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::ProcessResponse()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceControl::ProcessResponse() - return" ) ) );
    }

// End of file
