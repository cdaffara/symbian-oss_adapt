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
* Description:  Header file for Iscqueue
*
*/



#ifndef DISCQUEUE_H
#define DISCQUEUE_H

//  INCLUDES
#include <kernel.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Generic FIFO-type class
*
*  @lib IscDriver.LIB
*  @since 2.8
*/
class DIscQueue
    {
    public:  // Constructors and destructor
        
        DIscQueue();
        
        /**
        * C++ default constructor.
        */
        IMPORT_C DIscQueue( TUint32** aQueue, TUint16 Size );

        /**
        * Destructor.
        */
        IMPORT_C virtual ~DIscQueue();

        /**
        * Function to add element to queue
		* @since 2.8
        * @param TAny* pointer to frame
        * @return TInt KErrNone / KErrNoMemory
        */
        IMPORT_C TInt Add( TAny* );

        /**
        * Removes first element from the queue
		* @since 2.8
        * @return TAny* pointer to frame
        */
        IMPORT_C TAny* RemoveFirst();

        /**
        * Fetches first element from the queue
		* @since 2.8
        * @return TAny* pointer to frame
        */
        IMPORT_C TAny* GetFirst();

        /**
        * Deletes first element from the queue
		* @since 2.8
        * @return void 
        */
        IMPORT_C void DeleteFirst(); 

        /**
        * Checks if queue is empty
		* @since 2.8
        * @return TBool ETrue if empty, otherwise EFalse
        */
        IMPORT_C TBool Empty();

        /**
        * Gets length of next frame in queue
		* @since 2.8
        * @return TUint16 
        */
        IMPORT_C TUint16 NextBufferLength();

        /**
        * Inline function to tell the amount of elements in queue
		* @since 2.8
        * @return TUint16 amount of elements in queue
        */
        inline TUint16 Count() const { return iCount; };

        /**
        * Inline function to tell the size of queue
		* @since 2.8
        * @return TUint16 size of queue 
        */
        inline TUint16 SizeOf() const { return iSize; };

    protected:

        /**
        * Function to disable interrupts
		* @since 2.8
        * @return TInt KErrNone if succesful
        */
        TInt DisableIrqs();

        /**
        * Function to restore interrupts
		* @since 2.8
        * @param TInt aLevel level where interrupts should be restored
        * @return void 
        */
        void RestoreIrqs( TInt aLevel );
    
        volatile TUint16 iHead;
        volatile TUint16 iTail;
        volatile TUint16 iCount;
        TUint16          iSize;
        TUint32**        iQueue;
   
    };

#endif      // DISCQUEUE_H
            
// End of File
