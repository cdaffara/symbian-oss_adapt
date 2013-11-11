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
* Description:  Command handler for SSY <--> Sensor messages
*
*/


#ifndef SSYREFERENCECMDHANDLER_H
#define SSYREFERENCECMDHANDLER_H

#include <e32base.h>
#include "ssyreferenceconfig.h"

// FORWARD DECLARATIONS
class CSsyReferenceChannel;

/**
 *  Command handler class for handling commands of one channel. Each opened 
 *  channel has its own instance of this class
 *
 *  @lib ssyreferenceplugin.lib
 *  @since S60 5.0
 */
class CSsyReferenceCmdHandler : public CActive
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 5.0
     * @param[in] aSsyChannel Reference to SSY Channel instance.
     * @return CSsyReferenceCmdHandler* Pointer to created CSsyReferenceCmdHandler object
     */
    static CSsyReferenceCmdHandler* NewL( CSsyReferenceChannel& aSsyChannel );

    /**
     * Virtual destructor
     *
     * @since S60 5.0
     */
    virtual ~CSsyReferenceCmdHandler();

// from base class CSsyControl

    /**
     * Processes command specified in param aMessage. 
     *
     * @since S60 5.0
     * @param[in] aMessage Contains command information to process
     * @return void Symbian OS error code
     */
    TInt ProcessCommand( TSsyReferenceMsg aMessage );

    /**
     * From CActive
     */
    void RunL();

    /**
     * From CActive
     */
    void DoCancel();

    /**
     * From CActive
     */
    TInt RunError( TInt aError );

    /**
     * Callback function for DataItem generation
     */
    static TInt DataItemCallback( TAny* aThis );
    
    /**
     * Handles data item generation. Called from DataItemCallback
     */
    TInt GenerateChannelDataItem();

private:

    /**
     * C++ constructor.
     *
     * @since S60 5.0
     * @param[in] aSsyChannel Reference to SSY Channel instance.
     */
    CSsyReferenceCmdHandler( CSsyReferenceChannel& aSsyChannel );

    /**
     * Symbian 2nd phase constructor.
     */
    void ConstructL();

    /**
     * Makes synchronous calls asynchronous
     */
    void IssueRequest( TInt aError = KErrNone );

    /**
     * Sends response to channel
     */
    void SendResponse( TInt aError = KErrNone );

private: // data

    /**
     * Reference to SSY Conrtol to send responses for commands
     */
    CSsyReferenceChannel& iSsyChannel;

    /**
     * Pointer to currently processing message
     */
    TSsyReferenceMsg* iMessage;

    /**
     * Data item array
     */
    TSsyRefDataItemArray iDataItemArray;

    /**
     * Pointer to next item to generate in iDataItemArray
     */
    TInt iDataItemPtr;

    /**
     * Periodic timer for generating channel data
     */
    CPeriodic* iTimer;

    };

#endif // SSYREFERENCECMDHANDLER_H

// End of file
