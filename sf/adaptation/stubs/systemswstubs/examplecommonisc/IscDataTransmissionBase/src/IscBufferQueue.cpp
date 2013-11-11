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
* Description:  Implementation of DIscBufferQueue class
*
*/



// INCLUDE FILES

#include <IscDefinitions.h>
#include "IscBufferQueue.h"
#include "IscQueue.h"
#include "IscTrace.h"

#ifdef __WINS__
#include <windows.h>
#endif

// EXTERNAL DATA STRUCTURES
#ifdef __WINS__
extern CRITICAL_SECTION g_IscDTBCriticalSection;
#endif

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS
const TInt KIscInterruptLevelTwo( 2 );

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// DIscBufferQueue::DIscBufferQueue
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
DIscBufferQueue::DIscBufferQueue()
    :   iCount( NULL ),
        iBuffers( NULL ),
        iBuffersQueue( NULL )
    {
    }
    

// -----------------------------------------------------------------------------
// DIscBufferQueue::Construct
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
TInt DIscBufferQueue::Construct( 
    TUint16 aSize, 
    TUint16 aCount, 
    TUint8* &aCurrentAddress )
    {
    C_TRACE( ( _T( "DIscBufferQueue::Construct(0x%x, 0x%x, 0x%x) 0x%x, 0x%x" ), aSize, aCount, &aCurrentAddress, iBuffers, iBuffersQueue  ) );
    
    TInt r = KErrNone;
    if ( aSize == 0 || aCount == 0 )
        return KErrArgument;
    
    iCount = aCount;
    iBuffers = new TUint32*[aCount];
    ASSERT_RESET_ALWAYS( iBuffers, "ISCDataTransmissionBase",EIscMemoryAllocationFailure );
    iBuffersQueue = new DIscQueue( iBuffers, aCount );
    ASSERT_RESET_ALWAYS( iBuffersQueue, "ISCDataTransmissionBase",EIscMemoryAllocationFailure );
    
    for ( TInt i = 0; i < aCount; i++ )
        {   
        TPtr8*  pTmp;        
#ifndef __WINS__
        pTmp = new TPtr8 ( ( TUint8* )aCurrentAddress, aSize );
        ASSERT_RESET_ALWAYS( pTmp, "ISCDataTransmissionBase",EIscMemoryAllocationFailure );
#else
        TAny* ptr = Kern::Alloc( aSize );
        ASSERT_RESET_ALWAYS( ptr, "ISCDataTransmissionBase",EIscMemoryAllocationFailure );
        pTmp = new TPtr8( ( TUint8* )ptr, 0, aSize );
        ASSERT_RESET_ALWAYS( pTmp, "ISCDataTransmissionBase",EIscMemoryAllocationFailure );
#endif // __WINS__

        // Put the buffer into RX resource queue.        
        r = iBuffersQueue->Add( pTmp );
        
        aCurrentAddress += aSize;
        }
    
    C_TRACE( ( _T( "DIscBufferQueue::Construct - return 0x%x" ),r ) );

    return r;
    }


// -----------------------------------------------------------------------------
// DIscBufferQueue::New
// Construct a new queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
DIscBufferQueue* DIscBufferQueue::New( 
    TUint16 aSize, 
    TUint16 aCount, 
    TUint8*& aCurrentAddress )
    {
    C_TRACE( ( _T( "DIscBufferQueue::New(0x%x, 0x%x, 0x%x)" ), aSize, aCount, aCurrentAddress) );

    DIscBufferQueue* self = new DIscBufferQueue();
    ASSERT_RESET_ALWAYS( self, "ISCDataTransmissionBase",EIscMemoryAllocationFailure );
    if ( self->Construct( aSize, aCount, aCurrentAddress ) != KErrNone )
        {
        delete self;
        self = NULL;
        }
    else
        {
        }

    C_TRACE( ( _T( "DIscBufferQueue::New - return 0x%x" ),self ) );
    return self;
    }

    
// -----------------------------------------------------------------------------
// DIscBufferQueue::~DIscBufferQueue
// Destructor
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
DIscBufferQueue::~DIscBufferQueue()
    {
    C_TRACE( ( _T( "DIscBufferQueue::~DIscBufferQueue()" ) ) );
    if ( iBuffersQueue )
        {
        /* release all buffers */
        for ( TInt i = 0; i < iBuffersQueue->Count(); i++ )
            {
            // destroy allocated buffer
            TPtr8* delBuf = ( TPtr8* )iBuffersQueue->RemoveFirst();
            
            if ( delBuf )
                {
#ifdef __WINS__
                Kern::Free( ( TAny* )delBuf->Ptr() );
                delete delBuf;
#else
                delete delBuf;
#endif
                }
            else
                {
                }
            }
        delete iBuffersQueue;
        delete []iBuffers;
        }
    C_TRACE( ( _T( "DIscBufferQueue::~DIscBufferQueue - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscBufferQueue::Reserve
// Reserves first element from the queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TDes8* DIscBufferQueue::Reserve()
    {

    if ( iBuffersQueue )
        {
        TDes8* temp =  ( TDes8* )iBuffersQueue->RemoveFirst();
        if ( temp )
            {
            TInt irqLevel = DisableIrqs();
            iCount--;
            RestoreIrqs( irqLevel );
            }
        return temp;
        }
    else
        {
        return NULL;
        }

    }

// -----------------------------------------------------------------------------
// DIscBufferQueue::Release
// Releases element from the queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscBufferQueue::Release( 
    TDes8* aPtr )
    {
     
    if ( iBuffersQueue )
        {
        aPtr->Zero();
        TInt err = iBuffersQueue->Add( aPtr );
        ASSERT_RESET_ALWAYS( err == KErrNone, "ISCDataTransmissionBase",EIscBufferAllocationFailure );
        TInt irqLevel = DisableIrqs();
        iCount++;
        RestoreIrqs( irqLevel );
        }

    }

// -----------------------------------------------------------------------------
// DIscBufferQueue::DisableIrqs
// Function to disable interrupts
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscBufferQueue::DisableIrqs()
    {
#ifndef __WINS__
    return NKern::DisableInterrupts( KIscInterruptLevelTwo );    
#else //__WINS__
    EnterCriticalSection( &g_IscDTBCriticalSection );
    return KErrNone;
#endif//__WINS__
    }

// -----------------------------------------------------------------------------
// DIscBufferQueue::RestoreIrqs
// Function to restore interrupts
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//

#ifndef __WINS__
void DIscBufferQueue::RestoreIrqs( 
    TInt aLevel )
    {
    NKern::RestoreInterrupts( aLevel );
#else //__WINS__
void DIscBufferQueue::RestoreIrqs( 
    TInt )
    {
    LeaveCriticalSection( &g_IscDTBCriticalSection );
#endif//__WINS__
    }
//  End of File  
