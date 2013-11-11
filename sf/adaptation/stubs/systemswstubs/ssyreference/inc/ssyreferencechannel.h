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


#ifndef SSYREFERENCECHANNEL_H
#define SSYREFERENCECHANNEL_H

#include <e32base.h>
#include <sensrvtypes.h>
#include "ssyreferenceconfig.h"

class CSsyReferenceControl;
class CSsyReferenceChannelDataProvider;
class CSsyReferencePropertyProvider;
class CSsyReferenceCmdHandler;

/**
 *  Main control class for SSY. Controls sensor basic functionality and provides mandatory 
 *  ECOM interface specific things.
 *
 *  @lib ssyreferenceplugin.lib
 *  @since S60 5.0
 */
class CSsyReferenceChannel : public CBase
    {

public:

    /**
     * Enumeration of the state of this channel
     */
    enum TSsyReferenceChannelState
        {
        ESsyReferenceChannelIdle,      // Channel created, not opened
        ESsyReferenceChannelOpening,   // Processing channel opening
        ESsyReferenceChannelOpen,      // Channel is open
        ESsyReferenceChannelReceiving, // Channel is receiving data
        ESsyReferenceChannelClosing    // Processing channel closing. After this state is idle
        };

public:

    /**
     * Two-phase constructor
     *
     * @since S60 5.0
     * @param[in] aSsyControl Reference to SSY control instance.
     * @param[in] aChannelInfo Information of this channel
     * @return CSsyReferenceControl* Pointer to created CSsyReferenceControl object
     */
    static CSsyReferenceChannel* NewL( CSsyReferenceControl& aSsyControl, TSensrvChannelInfo aChannelInfo );

    /**
     * Virtual destructor
     *
     * @since S60 5.0
     */
    virtual ~CSsyReferenceChannel();

    /**
     * Request for SSY to open a sensor channel asynchronously. 
     * Response to the request is delivered through MSsyCallback::ChannelOpened().
     * Initilizes SSY (and the sensor) to be ready for other control commands via
     * data and property providers. Multiple OpenChannel()-requests can be
     * active for different channels at the same time.
     *
     * @since S60 5.0
     * @return void Symbian OS error code
     */
    TInt OpenChannel();

    /**
     * Request to close a sensor channel asynchronously. 
     * Response to the request is delivered through MSsyCallback::ChannelClosed().
     * Multiple CloseChannel()-requests can be active for different channels 
     * at the same time.
     *
     * @since S60 5.0
     * @return void Symbian OS error code
     */  
    TInt CloseChannel();

    /**
     * Returns ID of this channel
     */
    TInt ChannelId();

    /**
     * Handles response directed to this channel
     *
     * @since S60 5.0
     * @param[in] aMessage Contains information of the response
     */
    void ProcessResponse( TSsyReferenceMsg* aMessage );

    /**
     * Updates the state of this channel
     * 
     * @since S60 5.0
     * @param[in] aNewState State to update this channel
     */
    void UpdateState( const TSsyReferenceChannelState aNewState );

    /**
     * Reference to SsyControl
     */
    CSsyReferenceControl& SsyControl() const;

    /**
     * Reference to command handler
     */
    CSsyReferenceCmdHandler& CommandHandler() const;

    /**
     * Search property of given property id from the channel properties and 
     * returns reference to that. Leaves with KErrNotFound if property is not found
     *
     * @since S60 5.0
     * @param[in] aPropertyId Property ID to locate
     * @param[in] aItemIndex Item index if this search conserns specific property
     * @param[in] aArrayIndex Indicates array index of property
     */
    TSensrvProperty& FindPropertyL( const TSensrvPropertyId aPropertyId,
                                    TInt aItemIndex = KErrNotFound,
                                    TInt aArrayIndex = ESensrvSingleProperty );

    /**
     * Search property of given property id from the channel properties and 
     * update property values, if not read only
     *
     * @since S60 5.0
     * @param[in] aProperty Property to find and update
     */
    void FindAndUpdatePropertyL( const TSensrvProperty& aProperty );
    

    /**
     * Copies properties to param PropertyList
     *
     * @since S60 5.0
     * @param[in, out] aPropertyList List where to copy properties
     */
    void GetProperties( RSensrvPropertyList& aPropertyList );

private:

    /**
     * C++ constructor.
     *
     * @since S60 5.0
     * @param[in] aSsyControl Reference to SSY Control instance.
     * @param[in] aChannelInfo Information of this channel
     */
    CSsyReferenceChannel( CSsyReferenceControl& aSsyControl, TSensrvChannelInfo aChannelInfo );

    /**
     * Symbian 2nd phase constructor.
     */
    void ConstructL();

    /**
     * Handles response for OpenChannel and completes transaction by calling 
     * MSsyCallback::ChannelOpened
     *
     * @since S60 5.0
     * @param[in] aError Error code indicating the channel opening status
     */
    void HandleOpenChannelRespL( const TInt aError );

    /**
     * Handles response for CloseChannel and completes transaction by calling 
     * MSsyCallback::ChannelClosed
     *
     * @since S60 5.0
     */
    void HandleCloseChannelResp();

    /**
     * Handles response for StartChannelData. Loops all channel data items and sends 
     * each item to ChannelDataProvider
     *
     * @since S60 5.0
     */
    void HandleDataReceivedResp();

private: // data

    /**
     * Reference to SSY Control to send responses to Sensor Server
     */
    CSsyReferenceControl& iSsyControl;

    /**
     * Pointer to CSsyReferenceChannelDataProvider owned by this channel
     */
    CSsyReferenceChannelDataProvider* iChannelDataProvider;

    /**
     * Pointer to CSsyReferencePropertyProvider owned by this channel
     */
    CSsyReferencePropertyProvider* iChannelPropertyProvider;

    /**
     * Pointer to command handler
     */
    CSsyReferenceCmdHandler* iCmdHandler;

    /**
     * Information of this channel
     */
    TSensrvChannelInfo iChannelInfo;

    /**
     * State of this channel. See CSsyReferenceChannel::TSsyReferenceChannelState
     */
    TInt iState;

    /**
     * Property list of this channel
     */
    RSensrvPropertyList iProperties;
    };

#endif // SSYREFERENCECHANNEL_H
