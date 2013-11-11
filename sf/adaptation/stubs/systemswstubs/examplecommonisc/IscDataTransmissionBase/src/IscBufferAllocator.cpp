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
* Description:  Implementation of DIscBufferAllocator class
*
*/



// INCLUDE FILES
#include <kernel.h>
#include <platform.h>
#include <kern_priv.h>

#include "IscBufferAllocator.h"
#include "IscBufferQueue.h"
#include "IscTrace.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// DIscBufferAllocator::DIscBufferAllocator
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
DIscBufferAllocator::DIscBufferAllocator( TIscBufferEntry* aBufferConfig )
    : iBuffers( NULL )
#ifndef __WINS__
    , iBufferChunk( NULL )
    , iCurrentAddress( NULL )
#endif
    {
    for ( TUint16 i = 0; i < KIscBufferAmount; i++ )
        {
        iBufferConfig[i].size  = aBufferConfig[i].size;
        iBufferConfig[i].count = aBufferConfig[i].count;
        }
    
    C_TRACE( ( _T( "DIscBufferAllocator::DIscBufferAllocator" ) ) );
    }

// Destructor
DIscBufferAllocator::~DIscBufferAllocator()
    {
    // delete iBuffers & chunk
    for ( TUint8 i = 0; i < KIscBufferAmount; i++ )
        {
        delete iBuffers[i];
        iBuffers[i] = NULL;
        }
    delete [] iBuffers;
    iBuffers=NULL;

#ifndef __WINS__
    delete iBufferChunk;
    iBufferChunk = NULL;
    // free ram pages..
#endif
    }

// -----------------------------------------------------------------------------
// DIscBufferAllocator::AllocBuffers
// Allocate buffers according the multiplexer
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscBufferAllocator::AllocBuffers()
    {
    C_TRACE( ( _T( "DIscBufferAllocator::AllocBuffers()" ) ) );
    TUint8 i = 0;
#ifndef __WINS__
    TInt memoryNeeded = 0;
    for ( i = 0; i < KIscBufferAmount; i++ )
        {
        if ( iBufferConfig[i].size > 0 && iBufferConfig[i].count> 0 )
            {
            memoryNeeded += ( iBufferConfig[i].size * iBufferConfig[i].count );
            }        
        }    
    C_TRACE( ( _T( "DIscBufferAllocator::AllocBuffers memory needed %d" ), memoryNeeded ) );

    // Check if buffers are allocated from ISC
    if ( memoryNeeded > 0 )
        {
        AllocPhMemory( memoryNeeded );
        }
    else
        {
        return KErrNone;
        }

    TUint8* address = ( TUint8* )iCurrentAddress;
#else
    TUint8* address = NULL;
#endif //WINS

   
        
    iBuffers = new DIscBufferQueue*[ KIscBufferAmount ];
    ASSERT_RESET_ALWAYS( iBuffers, "ISCDataTransmissionBase",EIscMemoryAllocationFailure );
    
    for ( i = 0; i < KIscBufferAmount; i++ )
        {
        if ( iBufferConfig[i].size > 0 && iBufferConfig[i].count > 0 )
            {
            iBuffers[i] = DIscBufferQueue::New( iBufferConfig[i].size, iBufferConfig[i].count, address );
            }
        }

    return KErrNone;

    }

#ifndef __WINS__
// -----------------------------------------------------------------------------
// DIscBufferAllocator::AllocPhMemory
// Physical buffer allocations
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscBufferAllocator::AllocPhMemory( TInt aMemoryNeeded )
    {
    C_TRACE( ( _T( "DIscBufferAllocator::AllocPhMemory(0x%x)" ), aMemoryNeeded ) );

    TPhysAddr physAddr = 0;
    NKern::ThreadEnterCS();
    TInt r = Epoc::AllocPhysicalRam( aMemoryNeeded, physAddr );
    ASSERT_RESET_ALWAYS( r == KErrNone, "IscDataTransmissionBase",EIscMemoryAllocationFailure );
    r = DPlatChunkHw::New( iBufferChunk, physAddr, aMemoryNeeded, EMapAttrSupRw );
    NKern::ThreadLeaveCS();
    ASSERT_RESET_ALWAYS( r == KErrNone, "IscDataTransmissionBase",EIscMemoryAllocationFailure );
    iCurrentAddress = iBufferChunk->LinearAddress();

    }
#endif //WINS

// -----------------------------------------------------------------------------
// DIscBufferAllocator::ReserveMemoryBlock
// Reserves pre-allocated memory block
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscBufferAllocator::ReserveMemoryBlock( 
    TDes8*& aPtr, 
    TUint16 aSize )
    {
    E_TRACE( ( _T( "IBA:Res (0x%x)" ), aSize ) );
    
    if ( iBuffers )
        {
        for ( TUint8 i = 0; i < KIscBufferAmount;i++ )
            {
            if ( aSize <= iBufferConfig[i].size && iBufferConfig[i].count > 0 )
                {
                if ( iBuffers[i] )
                    {
                    if ( !iBuffers[i]->Empty() )
                        {
                        aPtr = ( TPtr8* )iBuffers[i]->Reserve();
                        if ( aPtr )
                            {
                            aPtr->SetLength( 0 );
                            E_TRACE( ( _T( "IBA:Res 0x%x %d" ), aPtr, i ) );
                            return;
                            }
                        }
                    }
                }
            }
        }
    // If Memory block is not reserved, set NULL to aPtr    
    aPtr = NULL;
    return;    
    }

// -----------------------------------------------------------------------------
// DIscBufferAllocator::ReleaseMemoryBlock
// Releases memory block allocated with ReserveMemoryBlock
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscBufferAllocator::ReleaseMemoryBlock( 
    TDes8* aPtr )
    {
    E_TRACE( ( _T( "IBA:Rel 0x%x" ), aPtr ) );
    if ( !aPtr )
        {
        TRACE_ASSERT_ALWAYS;
        }
    else if ( KIscBufferAmount != 0 && iBuffers )
        {
        for ( TUint8 i = 0; i < KIscBufferAmount; i++ )
            {
            if ( iBufferConfig[i].size == ( ( TDes8* )aPtr )->MaxLength() )
                {
                iBuffers[i]->Release( aPtr );
                }
            }
        }   
    }

//  End of File  
