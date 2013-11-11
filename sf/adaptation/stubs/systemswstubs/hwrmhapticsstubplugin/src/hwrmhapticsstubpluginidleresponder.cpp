/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Haptics test (adaptation) plugin idle responder 
*                implementation.
*
*/


#include "hwrmhapticsstubplugin.h"
#include "hwrmhapticsstubpluginidleresponder.h"

// ---------------------------------------------------------------------------
// Static instantiation method.
// ---------------------------------------------------------------------------
//
CHWRMHapticsStubPluginIdleResponder* 
    CHWRMHapticsStubPluginIdleResponder::NewL(
        CHWRMHapticsStubPlugin* aPlugin,
        TUint8 aTransId,
        TUint8* aDataPacket)
    {
    CHWRMHapticsStubPluginIdleResponder* self = 
        new ( ELeave ) CHWRMHapticsStubPluginIdleResponder( aPlugin,
                                                            aTransId,
                                                            aDataPacket );
                
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CHWRMHapticsStubPluginIdleResponder::~CHWRMHapticsStubPluginIdleResponder()
    {
    if ( iIdle )
        {
        iIdle->Cancel();
        delete iIdle;
        }
    }

// ---------------------------------------------------------------------------
// Method for starting response generation from CIdle callback
// This is used as TCallBack object in CIdle AO. 
// ---------------------------------------------------------------------------
//
TInt CHWRMHapticsStubPluginIdleResponder::GenerateResponse( TAny* aSelf )
    {
    CHWRMHapticsStubPluginIdleResponder* self = 
        reinterpret_cast<CHWRMHapticsStubPluginIdleResponder*>( aSelf );
    if ( self )
        {
        TRAP_IGNORE( self->GenerateResponseL() );
        }
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Method that does the actual response generation towards the issuer of 
// command i.e., the HapticsPluginManager
// ---------------------------------------------------------------------------
//
void CHWRMHapticsStubPluginIdleResponder::GenerateResponseL()
    {
    if ( iPlugin )
        {
        iPlugin->GenerateResponseL( iTransId, iDataPacket );
        }
    }

// ---------------------------------------------------------------------------
// Getter for iTransId
// ---------------------------------------------------------------------------
// 
TUint8 CHWRMHapticsStubPluginIdleResponder::TransId() const
    {
    return iTransId;
    }

// ---------------------------------------------------------------------------
// Method for enquiring whether the contained CIdle AO is active or not
// ---------------------------------------------------------------------------
//
TBool CHWRMHapticsStubPluginIdleResponder::Active() const
    {
    return ( iIdle && iIdle->IsActive() );
    }

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CHWRMHapticsStubPluginIdleResponder::CHWRMHapticsStubPluginIdleResponder(
        CHWRMHapticsStubPlugin* aPlugin,
        TUint8 aTransId,
        TUint8* aDataPacket )
    : iPlugin( aPlugin ), 
      iTransId ( aTransId ), 
      iDataPacket ( aDataPacket )       
    {
    // empty
    }

// ---------------------------------------------------------------------------
// Two-phase construction ConstructL
// ---------------------------------------------------------------------------
//
void CHWRMHapticsStubPluginIdleResponder::ConstructL()     
    {
    // Create and start the CIdle AO
    iIdle = CIdle::NewL( CActive::EPriorityLow );
    iIdle->Start( TCallBack( GenerateResponse, this ) );
    }
    
// end of file

