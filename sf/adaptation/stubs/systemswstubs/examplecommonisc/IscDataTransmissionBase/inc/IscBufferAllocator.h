/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Reference implementation for ISC buffer allocator.
*
*/




#ifndef ISCBUFFERALLOCATOR_H
#define ISCBUFFERALLOCATOR_H

//  INCLUDES
#include <kernel.h>

#include "IscDataTransmissionBase.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class DIscDevice;
class IscBufferQueue;
class DPlatChunkHw;
class DIscBufferQueue;

// CLASS DECLARATION


/**
*  Manages pre-allocated memory blocks
*
*  @lib IscDataTransmissionBase.LIB
*  @since 2.8
*/
class DIscBufferAllocator //: public DLogicalDevice
    {
    public:  // Constructors and destructor
        
        DIscBufferAllocator( TIscBufferEntry* aBufferConfig );
        virtual ~DIscBufferAllocator();

    public: // New functions
        
        /**
        * This method reserves pre-allocated memory block for sending/receiving frames
        * @since 2.8
        * @param TDes8*& aPtr a reference to pointer of descriptor
        * @param TUint16 aSize The minimum size of needed memory block
        * @return void 
        */
        IMPORT_C void ReserveMemoryBlock( TDes8*& aPtr, TUint16 aSize );
        
        /**
        * This method releases memory block allocated with ReserveMemoryBlock
        * @since 2.8
        * @param TDes8* aPtr a pointer to descriptor that is to be released
        * @return void 
        */
        IMPORT_C void ReleaseMemoryBlock( TDes8* aPtr );

        /**
        * This method makes the buffer allocations according to the 
        * configuration specified by the ISC Multiplexer
        * @since 2.8
        * @return TInt KErrNone if succesful
        */
        IMPORT_C TInt AllocBuffers();


    private:

        /**
        * This method makes the physical buffer allocations 
        * @since 2.8
        * @param TInt aPages amount of needed pages to allocate
        * @return void 
        */
        void AllocPhMemory( TInt aPages );

    private:    // Data

        TIscBufferEntry    iBufferConfig[KIscBufferAmount];
        DIscBufferQueue**   iBuffers;
        
#ifndef __WINS__
        DPlatChunkHw* iBufferChunk;
        TLinAddr    iCurrentAddress;
#endif

    };

#endif      // ISCBUFFERALLOCATOR
            
// End of File
