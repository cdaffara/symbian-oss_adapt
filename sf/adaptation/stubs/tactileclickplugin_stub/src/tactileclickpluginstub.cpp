/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The click maker plugin, which
*                handles key events at window server process.
* Part of:      Tactile Feedback.
*
*/



#include <e32std.h>
#include "tactileclickpluginstub.h"


/**
 *  These are used as opcodes (function number) in IPC communication 
 *  between Tactile Feedback Client and the server hosting
 *  Tactile Area Registry (currently window server)
 */
enum TTactileFeedbackOpCodes
    {
    ETactileOpCodeConnect = 200,
    ETactileOpCodeDisconnect = 201,
    ETactileOpCodeImmediateFeedback = 202
    };



// ======== MEMBER FUNCTIONS ========


CTactileClickPlugin::CTactileClickPlugin()
    {
    }


void CTactileClickPlugin::ConstructL()
    {
    // We need to store ourselves to thead local storage, so that
    // Anim Dll plugins can access the feedback functionality
    Dll::SetTls( this );
    }


// ---------------------------------------------------------------------------
// We really have to trap ConstructL, because construction of click maker 
// plug-in must not fail (otherwise WSERV will panic, resulting in KERN 4 and
// re-boot of whole device).
// ---------------------------------------------------------------------------
//
CTactileClickPlugin* CTactileClickPlugin::NewL()
    {
    CTactileClickPlugin* self = new( ELeave ) CTactileClickPlugin;
    TRAP_IGNORE( self->ConstructL() );
    return self;
    }


CTactileClickPlugin::~CTactileClickPlugin()
    {
    }


// ---------------------------------------------------------------------------
// From class CClickMaker.
// No implementation needed
// ---------------------------------------------------------------------------
//
void CTactileClickPlugin::KeyEvent( 
    TEventCode /*aType*/, const TKeyEvent& /*aEvent*/ )
    {    
    }
    
// ---------------------------------------------------------------------------
// From class CClickMaker.
// No implementation needed
// ---------------------------------------------------------------------------
//
void CTactileClickPlugin::PointerEvent( const TPointerEvent& /*aEvent*/ )
    {
    }

// ---------------------------------------------------------------------------
// From class CClickMaker.
// ---------------------------------------------------------------------------
//
void CTactileClickPlugin::OtherEvent( TInt /*aType*/, TAny* /*aParam*/ )
    {
    }


// ---------------------------------------------------------------------------
// From class CClickMaker.
// This is run when client calls RSoundPlugin::CommandReply -function.
//
// Handling of individual commands has been moved to dedicated functions,
// because data handling requires some effort and relively complex code.
// This is due to the mistake in CClickMaker API design, as the data is
// not delivered in the same format as it was passed to RSoundPlugin on
// client side (client gives data in descriptor, but here the descriptor
// needs to be re-built from TAny* pointer, and in addition the lenght
// of data needs to be known based on command id).
// ---------------------------------------------------------------------------
//
TInt CTactileClickPlugin::CommandReplyL( TInt aOpcode, TAny* aArgs )
    {    
    TInt errCode = KErrNone;
    
    switch ( aOpcode )
        {
        case ETactileOpCodeConnect:
            errCode = HandleConnectL( aArgs );
            break;
        case ETactileOpCodeDisconnect:
            HandleDisconnectL( aArgs );
            break;
        case ETactileOpCodeImmediateFeedback:
            errCode = HandleImmediateFeedbackL( aArgs );
            break;
        default:
            break;
        }
    
    return errCode;    
    };


// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CTactileClickPlugin::InstantFeedback( TTouchLogicalFeedback /*aType*/ )
    {
    }


// ---------------------------------------------------------------------------
// This is not real plugin --> Always return KErrNotSupported
// ---------------------------------------------------------------------------
//
TInt CTactileClickPlugin::HandleConnectL( TAny* /*aArgs*/ )
    {
    return KErrNotSupported;
    }
    
    
// ---------------------------------------------------------------------------
// No need to do anything here
// ---------------------------------------------------------------------------
//
void CTactileClickPlugin::HandleDisconnectL( TAny* /*aArgs*/ )
    {  
    }


// ---------------------------------------------------------------------------
// No need to do anything here
// ---------------------------------------------------------------------------
//
TInt CTactileClickPlugin::HandleImmediateFeedbackL( TAny* /*aArgs*/ )
    {
    return KErrNone;
    }


// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Function with this signature needs to be the first exported function
// in click maker plugin DLLs.
//
// Constructs and returns an instance to tactile click maker plugin.
// ---------------------------------------------------------------------------
//
EXPORT_C CClickMaker* CreateClickMakerL()
    {
    CTactileClickPlugin* clickMaker = CTactileClickPlugin::NewL();
    return clickMaker;
    }
