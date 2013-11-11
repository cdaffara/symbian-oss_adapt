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
* Description:  Declaration of DIscBufferQueue class
*
*/



#ifndef ISCBUFFERQUEUE_H
#define ISCBUFFERQUEUE_H

//  INCLUDES
#include <kernel.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class DIscQueue;

// CLASS DECLARATION

/**
*  Queue class for buffer allocation and management
*
*  @lib IscDataTransmissionBase.LIB
*  @since 2.8
*/
class DIscBufferQueue 
    {
    public:  // Constructors and destructor
        
        /**
        * C++ default constructor.
        */
        DIscBufferQueue();
        
        /**
        * Construct a new queue
        * @since 2.8
        * @param  TUint16 aSize size of element in queue
        * @param TUint16 aCount amount of elements in queue
        * @param TUint8* &aCurrentAddress start address of queue
        * @return DIscBufferQueue* pointer to queue
        */
        static DIscBufferQueue* New( TUint16 aSize, TUint16 aCount, TUint8* &aCurrentAddress );
        
        /**
        * Destructor.
        */
        ~DIscBufferQueue();

    public: // New functions
        
        /**
        * Checks if queue is empty
        * @since 2.8
        * @return TBool ETrue if empty, otherwise EFalse
        */
        inline TBool Empty(){return iCount ? EFalse : ETrue;};

        /**
        * Reserves first element from the queue
        * @since 2.8
        * @return TDes8* pointer to element if succesful, otherwise NULL
        */
        TDes8* Reserve();
        
        /**
        * Releases element from the queue
        * @since 2.8
        * @param TDes8* aPtr pointer to element
        * @return void 
        */
        void Release( TDes8* aPtr );

    protected:  // New functions
        
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


    protected:  // Functions from base classes
        
    private:

        /**
        * 2nd phase constructor
        * @since 2.8
        * @param  TUint16 aSize size of element in queue
        * @param TUint16 aCount amount of elements in queue
        * @param TUint8* &aCurrentAddress start address of queue
        * @return TInt 
        */
        TInt Construct( TUint16 aSize, TUint16 aCount, TUint8* &aCurrentAddress );

    public:     // Data
    
    protected:  // Data

    private:    // Data

        volatile TUint32 iCount;
        TUint32**       iBuffers;
        DIscQueue*      iBuffersQueue;  

    };

#endif      // ISCBUFFERQUEUE_H   
            
// End of File
