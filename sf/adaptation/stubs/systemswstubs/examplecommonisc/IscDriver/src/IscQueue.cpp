/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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



// INCLUDE FILES

#include <kernel.h>
#ifdef __WINS__
#include <windows.h>
#endif

#include <IscDefinitions.h>
#include "IscQueue.h"
#include "IscTrace.h"


// EXTERNAL DATA STRUCTURES
#ifdef __WINS__
extern CRITICAL_SECTION g_IscCriticalSection;
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
// DIscQueue::DIscQueue
// C++ default constructor 
// -----------------------------------------------------------------------------
//
DIscQueue::DIscQueue()
    :iHead( 0 ),iTail( 0 ),iCount( 0 ),iSize( 0 ),iQueue( NULL )
    {
    }

// -----------------------------------------------------------------------------
// DIscQueue::DIscQueue
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C DIscQueue::DIscQueue( TUint32** aQueue, TUint16 Size )
        :iHead( 0 ),iTail( 0 ),iCount( 0 ),iSize( Size ),iQueue( aQueue )
    {
    E_TRACE( ( _T( "IQ:IQ %d 0x%x" ), iSize, iQueue ) );
    }
    
// Destructor
EXPORT_C DIscQueue::~DIscQueue()
    {
    iHead   = 0;
    iTail   = 0;
    iCount  = 0;
    iQueue  = NULL;
    iSize   = 0;
    }


// -----------------------------------------------------------------------------
// DIscQueue::Add
// Function to add element to queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscQueue::Add( TAny* anEntry )
    {
    E_TRACE( ( _T( "IQ:A %d 0x%x" ), iCount, iQueue ) );
    
    TInt irqLevel = DisableIrqs();

    if ( iCount == iSize || iSize == 0 )
        {
        RestoreIrqs( irqLevel );
        return KErrNoMemory;//EFalse;
        }

    /* place the buffer into the queue */
    iQueue[ iTail ] = ( TUint32* )( anEntry );

    if ( iSize > 0 )
        {
        /* adjust tail pointer */
        iTail = ++iTail % iSize;

        /* remember the amount of the requests in the queue */
        iCount++;
        }
    else
        {
        ASSERT_RESET_ALWAYS( 0, "IscDriver", EIscBufferAllocationFailure )
        }

    RestoreIrqs( irqLevel );

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// DIscQueue::Remove
// Removes first element from the queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TAny* DIscQueue::RemoveFirst()
    {
    E_TRACE( ( _T( "IQ:R %d %d" ), iCount, iSize ) );
    
    TAny* result = NULL;
    TInt irqLevel = DisableIrqs();

    if ( iCount == 0 || iSize == 0 )
        {
        RestoreIrqs( irqLevel );
        return NULL;
        }
    // Get an element from the queue.
    result = ( TAny* )iQueue[ iHead ];

    iQueue[ iHead ] = NULL;

    // Adjust the head of the queue.
    iHead = ++iHead % iSize;
    // Decrease counter.
    iCount--;

    RestoreIrqs( irqLevel );

    return result;

    }

// -----------------------------------------------------------------------------
// DIscQueue::GetFirst
// Fetches first element from the queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TAny* DIscQueue::GetFirst()
    {
    E_TRACE( ( _T( "IQ:G %d %d" ), iCount, iSize ) );
    
    TAny* result;
    TInt irqLevel = DisableIrqs();

    if ( iCount == 0 || iSize == 0 )
        {
        RestoreIrqs( irqLevel );
        return NULL;
        }
    // Get an element from the queue.
    result = ( TAny* )iQueue[ iHead ];
    
    RestoreIrqs( irqLevel );
    
    return result;

    }

// -----------------------------------------------------------------------------
// DIscQueue::DeleteFirst
// Deletes first element from the queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscQueue::DeleteFirst()
    {
    E_TRACE( ( _T( "IQ:D %d %d" ), iCount, iSize ) );
    
    TInt irqLevel = DisableIrqs();

    iQueue[ iHead ] = NULL;

    if ( iSize > 0 )
        {
        // Adjust the head of the queue.
        iHead = ++iHead % iSize;
        // decrease counter.
        iCount--;
        }
    else
        {
        ASSERT_RESET_ALWAYS( 0, "IscDriver", EIscBufferAllocationFailure )
        }

    RestoreIrqs( irqLevel );

    }

// -----------------------------------------------------------------------------
// DIscQueue::Empty
// Checks if queue is empty
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TBool DIscQueue::Empty()
    {
    TInt result;
    TInt irqLevel = DisableIrqs();
    result = iCount == 0 ? ETrue : EFalse;
    RestoreIrqs( irqLevel );

    return result;
    }

// -----------------------------------------------------------------------------
// DIscQueue::NextBufferLenth
// Gets length of next frame in queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TUint16 DIscQueue::NextBufferLength()
    {
    TUint16 length;

    TInt irqLevel = DisableIrqs();

    if ( iCount == 0 )
        {
        RestoreIrqs( irqLevel );
        return 0;
        }    

    length = ( TUint16 )( ( ( TDes8* )iQueue[ iHead ] )->Length() );    


    RestoreIrqs( irqLevel );

    return length;
    }

// -----------------------------------------------------------------------------
// DIscQueue::DisableIrqs
// Function to disable interrupts
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscQueue::DisableIrqs()
    {
#ifndef __WINS__
    return NKern::DisableInterrupts( KIscInterruptLevelTwo );
#else //__WINS__
    EnterCriticalSection( &g_IscCriticalSection );
    return KErrNone;
#endif//__WINS__
    }

// -----------------------------------------------------------------------------
// DIscQueue::RestoreIrqs
// Function to restore interrupts
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
#ifndef __WINS__
void DIscQueue::RestoreIrqs( 
    TInt aLevel )
    {
    NKern::RestoreInterrupts( aLevel );
#else //__WINS__
void DIscQueue::RestoreIrqs( 
    TInt )
{
    LeaveCriticalSection( &g_IscCriticalSection );
#endif//__WINS__
    }

//  End of File  
