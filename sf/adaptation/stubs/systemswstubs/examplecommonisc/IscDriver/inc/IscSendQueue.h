/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  An example implementation for ISC Driver Reference
*
*/



#ifndef DISCSENDQUEUE_H
#define DISCSENDQUEUE_H

//  INCLUDES
#include <kernel.h>
#include "IscQueue.h" // Base class

// CONSTANTS

// MACROS

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION
class DIscChannel;
class TIscSendFrameInfo;

// DATA TYPES

/**
*  Generic FIFO-type class for send messages
*
*  @lib IscDriver.LIB
*  @since 2.8
*/
class DIscSendQueue : public DIscQueue
    {
    public:  // Constructors and destructor

        /**
        * C++ default constructor.
        */
        IMPORT_C DIscSendQueue( TUint32** aQueue, 
                                TIscSendFrameInfo** aParameterQueue,
                                TUint16 aSize );

        /**
        * Destructor.
        */
        IMPORT_C ~DIscSendQueue();

        /**
        * Function to add element to queue
		* @since 2.8
        * @param anEntry pointer to frame
        * @param aId channel number
        * @param aChannelPtr DIscChannel pointer
        * @param aFrameInfo Additional information to pass information to data transmission driver
        * @return TInt KErrNone / KErrNoMemory
        */
        IMPORT_C TInt Add( TAny* anEntry, TUint16 aId, DIscChannel* aChannelPtr, TAny* aFrameInfo );

        /**
        * Function to get information about first frame in queue
		* @since 2.8
        * @return TIscSendFrameInfo* (channel id, channel pointer and frameinfo)
        */
        IMPORT_C TIscSendFrameInfo* GetFirstFrameInfo();
        

    private:

        TIscSendFrameInfo**   iParameterQueue;
        
    };

class TIscSendFrameInfo
    {
    public:
    DIscChannel* iChannelPtr;
    TAny*        iFrameInfo;
    TUint16      iChannelId;
    };

#endif      // DISCSENDQUEUE_H
            
// End of File
