/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Reference implementation of SSY Property Provider interface
*
*/


#include "ssyreferencepropertyprovider.h"
#include "ssyreferencetrace.h"
#include "ssyreferencechannel.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSsyReferencePropertyProvider C++ constructor
// ---------------------------------------------------------------------------
//
CSsyReferencePropertyProvider::CSsyReferencePropertyProvider( CSsyReferenceChannel& aChannel ) :
    iChannel( aChannel )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::CSsyReferencePropertyProvider()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::CSsyReferencePropertyProvider() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// Symbian 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CSsyReferencePropertyProvider::ConstructL()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::ConstructL()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::ConstructL() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// CSsyReferencePropertyProvider::NewL
// ---------------------------------------------------------------------------
//
CSsyReferencePropertyProvider* CSsyReferencePropertyProvider::NewL( CSsyReferenceChannel& aChannel )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::NewL()" ) ) );
    CSsyReferencePropertyProvider* self = new ( ELeave ) CSsyReferencePropertyProvider( aChannel );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::NewL() - return" ) ) );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CSsyReferencePropertyProvider::~CSsyReferencePropertyProvider()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::~CSsyReferencePropertyProvider()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::~CSsyReferencePropertyProvider() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferencePropertyProvider::CheckPropertyDependenciesL
// ---------------------------------------------------------------------------
//
void CSsyReferencePropertyProvider::CheckPropertyDependenciesL( 
    const TSensrvChannelId /*aChannelId*/,
    const TSensrvProperty& /*aProperty*/,
    RSensrvChannelList& /*aAffectedChannels*/ )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::CheckPropertyDependenciesL()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::CheckPropertyDependenciesL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferencePropertyProvider::SetPropertyL
// ---------------------------------------------------------------------------
//
void CSsyReferencePropertyProvider::SetPropertyL( 
    const TSensrvChannelId aChannelId,
    const TSensrvProperty& aProperty )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::SetPropertyL()" ) ) );

    if ( iChannel.ChannelId() != aChannelId )
        {
        User::Leave( KErrArgument );
        }

    // Search property. Leaves with KErrNotFound if property is not found. 
    // Leaves with KErrAccessDenied if found property is Read only
    iChannel.FindAndUpdatePropertyL( aProperty );
    
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::SetPropertyL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferencePropertyProvider::GetPropertyL
// ---------------------------------------------------------------------------
//
void CSsyReferencePropertyProvider::GetPropertyL( 
    const TSensrvChannelId aChannelId,
    TSensrvProperty& aProperty )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::GetPropertyL()" ) ) );
    
    if ( iChannel.ChannelId() != aChannelId && aChannelId != 0 )
        {
        User::Leave( KErrArgument );
        }
    else
        {
        // Search property. Leaves with KErrNotFound if property is not found
        aProperty = iChannel.FindPropertyL( 
                        aProperty.GetPropertyId(), 
                        aProperty.PropertyItemIndex(),
                        aProperty.GetArrayIndex() );
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::GetPropertyL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferencePropertyProvider::GetAllPropertiesL
// ---------------------------------------------------------------------------
//
void CSsyReferencePropertyProvider::GetAllPropertiesL( 
    const TSensrvChannelId aChannelId,
    RSensrvPropertyList& aChannelPropertyList )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::GetAllPropertiesL()" ) ) );
    
    if ( iChannel.ChannelId() != aChannelId )
        {
        User::Leave( KErrArgument );
        }

    iChannel.GetProperties( aChannelPropertyList );

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferencePropertyProvider::GetAllPropertiesL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferencePropertyProvider::GetPropertyProviderInterfaceL
// ---------------------------------------------------------------------------
//
void CSsyReferencePropertyProvider::GetPropertyProviderInterfaceL( TUid aInterfaceUid, 
	                                        TAny*& aInterface )
    {
    aInterface = NULL;
    
	if ( aInterfaceUid.iUid == KSsyPropertyProviderInterface1.iUid )
		{
		aInterface = reinterpret_cast<TAny*>(
			static_cast<MSsyPropertyProvider*>( this ) );
		}
    }
  
// End of file
