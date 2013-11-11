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
* Description:  Reference implementation of SSY Channel Data Provider interface
*
*/


#ifndef SSYREFERENCECHANNELDATAPROVIDER_H
#define SSYREFERENCECHANNELDATAPROVIDER_H

#include <ssychanneldataprovider.h>

class CSsyReferenceChannel;
class TSsyReferenceMsg;

/**
 *  Channel data provider implementation.
 *
 *  @lib ssyreferenceplugin.lib
 *  @since S60 5.0
 */
class CSsyReferenceChannelDataProvider : public CBase, public MSsyChannelDataProvider
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 5.0
     * @param[in] aChannel Reference to channel this provider belongs to
     * @return CSsyReferenceChannelDataProvider* Pointer to created CSsyReferenceControl object
     */
    static CSsyReferenceChannelDataProvider* NewL( CSsyReferenceChannel& aChannel );

    /**
     * Virtual destructor
     *
     * @since S60 5.0
     */
    virtual ~CSsyReferenceChannelDataProvider();

// from base class MSsyChannelDataProvider

    /**
     * From MSsyChannelDataProvider
     * Starts asynchronous data listening. Multiple OpenChannel()-requests
     * can be active for different channels at the same time.
     *
     * @since S60 5.0
     * @param[in] aBuffer Pointer to a data buffer 
     * @param[in] aCount Indicates data buffer size as a count of the data objects.
     */
    void StartChannelDataL( const TSensrvChannelId aChannelId, TUint8* aBuffer, TInt aCount );

    /**
     * From MSsyChannelDataProvider
     * Stops asynchronous data listening. The data buffer is not valid after call of 
     * this function.
     *
     * @since S60 5.0
     */
    void StopChannelDataL( const TSensrvChannelId aChannelId );

    /**
     * From MSsyChannelDataProvider
     * Forces SSY to call BufferFilled() regardless of how many data items have been
     * written to buffer. Even if no data items have yet been written, BufferFilled()
     * must be called.
     *
     * @since S60 5.0
     */
    void ForceBufferFilledL( const TSensrvChannelId aChannelId );

    /** 
    * Returns a pointer to a specified interface - to allow future extension
    * of this class without breaking binary compatibility
    *
    * @since S60 5.0
    * @param aInterfaceUid Identifier of the interface to be retrieved
    * @param aInterface A reference to a pointer that retrieves the specified interface.
    */
    void GetChannelDataProviderInterfaceL( TUid aInterfaceUid, TAny*& aInterface );

    /**
     * Channel data item received
     *
     * @param[in] aMessage Contains channel item
     */
    void ChannelDataReceivedL( TSsyReferenceMsg* aMessage );

private:

    /**
     * C++ constructor.
     * @param[in] aChannel Reference to channel this provider belongs to
     */
    CSsyReferenceChannelDataProvider( CSsyReferenceChannel& aChannel );

    /**
     * Symbian 2nd phase constructor.
     */
    void ConstructL();

    /**
     * Adds data into buffer. 
     *
     *@param[in] aData Data to add to buffer
     *@param[in] aSize Size of data
     */
    void AddDataToBuffer( TUint8* aData, const TInt aSize );

    /**
     * Sends BufferFilled notification to MSsyCallback
     */
    void SendBufferFilled();

private: // data

    /**
     * Reference to channel for which this provider belongs to
     */
    CSsyReferenceChannel& iChannel;

    /**
     * Pointer to data buffer in Sensor Server side
     */
    TUint8* iDataBuffer;

    /**
     * Maximum requested data items
     */
    TInt iMaxCount;

    /**
     * Number of items in buffer
     */
    TInt iDataCount;

    };

#endif // SSYREFERENCECHANNELDATAPROVIDER_H
