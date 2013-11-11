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
* Description:  Declaration of MSAStubEventSender interface class.
*
*/


#ifndef __SASTUBEVENTSENDER_H__
#define __SASTUBEVENTSENDER_H__

#include <e32def.h>

/**
 *  Interface representing object capable of sending Startup Adaptation events.
 *
 *  @lib None
 *  @since S60 4.0
 */
class MSAStubEventSender
    {

public:

    /**
     * Send an event.
     *
     * @since S60 4.0
     * @param aEvent Identifies the event to send.
     * @param aArg Optional paraneter for the event.
     */
    virtual void SendEvent( const TInt aEvent, const TInt aArg ) = 0;

    };

#endif // __SASTUBEVENTSENDER_H__
