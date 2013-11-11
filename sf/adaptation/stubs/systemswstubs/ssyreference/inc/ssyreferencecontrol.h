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


#ifndef SSYREFERENCECONTROL_H
#define SSYREFERENCECONTROL_H

#include <ssycontrol.h>
#include "ssyreferenceconfig.h"

// FORWARD DECLARATIONS
class CSsyReferenceChannel;

/**
 *  Main control class for SSY. Controls sensor basic functionality and provides mandatory 
 *  ECOM interface specific things.
 *
 *  @lib ssyreferenceplugin.lib
 *  @since S60 5.0
 */
class CSsyReferenceControl : public CSsyControl
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 5.0
     * @param[in] aSsyCallback Reference to SSY callback instance.
     * @return CSsyReferenceControl* Pointer to created CSsyReferenceControl object
     */
    static CSsyReferenceControl* NewL( MSsyCallback& aSsyCallback );

    /**
     * Virtual destructor
     *
     * @since S60 5.0
     */
    virtual ~CSsyReferenceControl();

// from base class CSsyControl

    /**
     * From CSsyControl
     * Request for SSY to open a sensor channel asynchronously. 
     * Response to the request is delivered through MSsyCallback::ChannelOpened().
     * Initilizes SSY (and the sensor) to be ready for other control commands via
     * data and property providers. Multiple OpenChannel()-requests can be
     * active for different channels at the same time.
     *
     * @since S60 5.0
     * @param[in] aChannelID Channel that is requested to be opened
     * @return Symbian OS error code
     */
    void OpenChannelL( TSensrvChannelId aChannelID );

    /**
     * From CSsyControl
     * Request to close a sensor channel asynchronously. 
     * Response to the request is delivered through MSsyCallback::ChannelClosed().
     * Multiple CloseChannel()-requests can be active for different channels 
     * at the same time.
     *
     * @since S60 5.0
     * @param[in] aChannelID Channel that is reqeusted to be closed
     * @leave Symbian OS error code
     */  
    void CloseChannelL( TSensrvChannelId aChannelID );

    /**
     * Reference to SSY Callback instance
     */
    MSsyCallback& SsyCallback() const;

    /**
     * Reference to SSY Config file
     */
    CSsyReferenceConfig& SsyConfig() const;
    /**
     * Handles response to CSsyReferenceCmdHandler::ProcessCommand
     *
     * @since S60 5.0
     * @param[in] aMessage Contains information of the response
     */
    void ProcessResponse( TSsyReferenceMsg* aMessage );

    /**
     * Search property of given property id from the channel properties and 
     * returns reference to that. Leaves with KErrNotFound if property is not found
     *
     * @since S60 5.0
     * @param[in]  aPropertyId Property ID to locate
     * @param[in]  aArrayIndex Propertys array index
     * @param[out] aProperty Contains found property
     */
    void FindPropertyL( const TSensrvPropertyId aPropertyId, 
                        const TInt aArrayIndex,
                        TSensrvProperty& aProperty );

private:

    /**
     * C++ constructor.
     *
     * @since S60 5.0
     * @param[in] aSsyCallback Reference to SSY callback instance.
     */
    CSsyReferenceControl( MSsyCallback& aSsyCallback );

    /**
     * Symbian 2nd phase constructor.
     */
    void ConstructL();

    /**
     * Finds and returns pointer to channel
     */
    CSsyReferenceChannel* FindChannelL( TSensrvChannelId aChannelID );

private: // data

    /**
     * Reference to SSY CallBack to send responses to Sensor Server
     */
    MSsyCallback& iSsyCallback;

    /**
     * Pointer array of all channels provided by this SSY
     */
    CArrayPtrFlat<CSsyReferenceChannel>* iChannelArray;

    /**
     * Pointer to config file parser
     */
    CSsyReferenceConfig* iConfigFile;

    /**
     * Property list of general properties of this SSY
     */
    RSensrvPropertyList iProperties;

    };

#endif // SSYREFERENCECONTROL_H
