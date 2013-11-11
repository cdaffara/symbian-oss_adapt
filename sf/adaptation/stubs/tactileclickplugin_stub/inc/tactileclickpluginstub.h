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


#ifndef C_TACTILECLICKPLUGIN_H
#define C_TACTILECLICKPLUGIN_H



#include <w32click.h>

#include "tactilefeedbackserver.h"



/**
 *  The click maker plugin to be used as stub when the real plugin is
 *  not available.
 *
 *  All connections fail with KErrNotSupported, which will essentially
 *  disable feedback support on client side.
 *
 *  @lib tactilefeedbackresolver.dll
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CTactileClickPlugin ): 
    public CClickMaker,
    public MTactileFeedbackServer
    {
public:

    static CTactileClickPlugin* NewL();

    virtual ~CTactileClickPlugin();

// from base class CClickMaker

    /**
     * From CClickMaker.
     * This function is called by the window server whenever there is a 
     * key event, to generate the sound.
     *
     * The key events are not used by this plugin actually, because this
     * plug-in's purpose is to handle only pointer events.
     *
     * @since S60 v5.0
     * @param aType  - The type of the key event.
     * @param aEvent - The key event details. 
     */
    void KeyEvent( TEventCode aType,const TKeyEvent& aEvent );

    /**
     * From CClickMaker.
     * This function is called by the window server whenever there is a 
     * pointer event, to generate the sound.
     *
     * Not used by this plug-in because there is not enough information in
     * the pointer event for resolving the actual area and window where
     * pointer event was hit. Intead pointer events are handled in 
     * OtherEvent -function.
     *
     * @since S60 v5.0
     * @param aEvent - The pointer event details.
     */
    void PointerEvent( const TPointerEvent& aEvent );

    /**
     * From CClickMaker.
     * This function is a "future expansion" or the original click maker API,
     * and it has been taken into use by this plugin for getting more 
     * information about the pointer event than what is possible to pass as
     * parameter to PointerEvent -function.
     *
     * This stub ignores all events.
     *
     * @since S60 v5.0
     * @param aType  - The type of the event.
     * @param aParam - Pointer to data package containing the pointer event,
     *                 window identifier where pointer event hit, and window
     *                 group identifier of the window group where the window
     *                 belongs to.
     */
    void OtherEvent( TInt aType, TAny* aParam=NULL );

    /**
     * From CClickMaker.
     * Implemented to enable this plug-in to communicate with the window
     * server client.
     *
     * Area registry management communication (such as creating or closing
     * an application) is communicated through this function.
     *
     * In practice communication is done between this plug-in, and 
     * Tactile Feedback Client API implementation.
     *
     * This plugin completes connect requests with KErrNotSupported, and all
     * other requests with KErrNone without actually doing any actions.
     *
     * @since S60 v5.0
     * @param aOpcode - Opcode of the command sent by Client API
     * @param aArgs   - Pointer to the data sent by client.
     */
    TInt CommandReplyL( TInt aOpcode, TAny* aArgs );


// from base class MTactileFeedbackServer

    /**
     * From MTactileFeedbackServer.
     * Direct feedback service for Anim Dll plugins.
     *
     * This stub ignores all instant feedback requests.
     *
     * @since S60 v5.0
     * @param aType - Logical feedback type to play.
     */
    void InstantFeedback( TTouchLogicalFeedback aType );

private:

    CTactileClickPlugin();

    void ConstructL();
    
    /**
     * Handles data parsing when new client connects to this plugin.
     *
     * No real functionality in this stub.
     *
     * @param aArgs - Pointer to the data sent by client.
     * @return Always KErrNotSupported in this stub.
     */
    TInt HandleConnectL( TAny* aArgs );

    /**
     * Handles data parsing when new client connects to this plugin.
     *
     * No real functionality in this stub.
     *
     * @param aArgs - Pointer to the data sent by client.
     */
    void HandleDisconnectL( TAny* aArgs );

    /**
     * Handles direct feedback request from Tactile Client API.
     *
     * No real functionality in this stub.
     *
     * @param aArgs - Pointer to the data sent by client.
     * @return KErrNone if operation succeeded, otherwise one of standand
     *         Symbian OS error codes.
     */
    TInt HandleImmediateFeedbackL( TAny* aArgs );
    };


#endif // C_TACTILECLICKPLUGIN_H
