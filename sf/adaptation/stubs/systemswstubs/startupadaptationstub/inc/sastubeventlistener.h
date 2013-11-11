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
* Description:  Declaration of CSAStubEventListener class.
*
*/


#ifndef __SASTUBEVENTLISTENER_H__
#define __SASTUBEVENTLISTENER_H__

#include <e32base.h>
#include <e32property.h>

class MSAStubEventSender;

/**
 *  Observer for Starter module test events.
 *
 *  @lib None
 *  @since S60 4.0
 */
class CSAStubEventListener : public CActive
    {

public:

    /**
     * Second phase constructor.
     *
     * @param aSender Object that can send an event in response to an
     * indication received from P&S.
     * @since S60 4.0
     */
    static CSAStubEventListener* NewL( MSAStubEventSender& aSender );

    virtual ~CSAStubEventListener();

protected:

    /**
     * From CActive.
     * Implements cancellation of an outstanding request.
     *
     * @since S60 4.0
     */
    virtual void DoCancel();

    /**
     * From CActive.
     * Handles an active object's request completion event.
     * Never leaves.
     *
     * @since S60 4.0
     */
    virtual void RunL();

private:

    /**
     * First phase constructor.
     *
     * @param aSender Object that can send an event in response to an
     * indication received from P&S.
     * @since S60 4.0
     */
    CSAStubEventListener( MSAStubEventSender& aSender );

    void ConstructL();

private: // data

    /**
     * Handle to Publish&Subscribe key to observe.
     */
    RProperty iProperty;

    /**
     * Object that can send an event in response to an indication received from
     * P&S.
     */
    MSAStubEventSender& iSender;

    };

#endif // __SASTUBEVENTLISTENER_H__
