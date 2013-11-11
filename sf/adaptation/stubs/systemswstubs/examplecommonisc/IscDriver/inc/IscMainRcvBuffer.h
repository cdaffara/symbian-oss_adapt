/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Declaration of DIscMainRcvBuffer class
*
*/




#ifndef DISCMAINRCVBUFFER_H
#define DISCMAINRCVBUFFER_H

//  INCLUDES
#include <kernel.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION
class DIscDevice;
class DIscQueue;

/**
*  First place to store incoming messages from Domestic OS
*
*  @lib IscDriver.LIB
*  @since 2.8
*/
class DIscMainRcvBuffer//: public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * C++ default constructor.
        */
        DIscMainRcvBuffer( DIscDevice* aDevice, TUint16 aIscMainRcvBufferQueueSize );
        
        /**
        * Destructor.
        */
        virtual ~DIscMainRcvBuffer();

    public: // New functions
        
        /**
        * Second-phase constructor. Should be called to complete construction.
        * @since 2.8
        * @return void 
        */
        void DoCreate();

        /**
        * Function that should be called to store incoming frame
        * @since 2.8
        * @param TDesC8* aData pointer to frame
        * @return void 
        */
        static void MsgReceive( TDesC8* aData );

        /**
        * Function for adding Dfc. Can be called from any context.
        * @since 2.8
        * @return void 
        */
        void AddDfc();

    private:

        /**
        * Function that goes through messages in main buffer
        * and empties those to correct channel buffers
        * @since 2.8
        * @param TAny* aPtr pointer to DIscMainRcvBuffer
        * @return static void
        */
        static void BufferCleanUp( TAny* aPtr );
 
    private:    // Data

        DIscDevice*               iDevice;
        TDfc*                     iDfc;
        static DIscMainRcvBuffer* iThisPointer;
        TUint16                   iIscMainRcvBufferQueueSize;
        TUint32**                 iMainRcvBuffer;
        DIscQueue*                iMainRcvBufferQueue;

    };

#endif      // DISCMAINRCVBUFFER_H   
            
// End of File
