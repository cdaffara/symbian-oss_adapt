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
#include "IscSendQueue.h"
#include "IscTrace.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS
const TInt KImpossibleChannelId( 255 );

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// DIscSendQueue::DIscSendQueue
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C DIscSendQueue::DIscSendQueue( TUint32** aQueue, TIscSendFrameInfo** aParameterQueue, TUint16 aSize )
         :iParameterQueue( aParameterQueue )
    {
    iHead  = 0;
    iTail  = 0;
    iCount = 0;
    iQueue = aQueue;
    iSize  = aSize;
    for ( TInt i( 0 ); i < aSize; i++ )
        {
        iParameterQueue[i]->iChannelId = KImpossibleChannelId;
        iParameterQueue[i]->iChannelPtr = NULL;
        iParameterQueue[i]->iFrameInfo = NULL;
        }
    E_TRACE( ( _T( "ISQ:ISQ %d 0x%x" ), iSize, iQueue ) );
    }
    
// Destructor
EXPORT_C DIscSendQueue::~DIscSendQueue()
    {
    iHead       = 0;
    iTail       = 0;
    iCount      = 0;
    iQueue      = NULL;
    iParameterQueue = NULL;
    iSize       = 0;
    }


// -----------------------------------------------------------------------------
// DIscSendQueue::Add
// Function to add element to queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscSendQueue::Add( TAny* anEntry, TUint16 aId, DIscChannel* aChannelPtr, TAny* aFrameInfo )
    {
    E_TRACE( ( _T( "ISQ:A %d, 0x%x" ), iCount, iQueue ) );

    TInt irqLevel = DisableIrqs();

    if ( iCount == iSize || iSize == 0 )
        {
        RestoreIrqs( irqLevel );
        return KErrNoMemory;//EFalse;
        }

    /* place the buffer into the queue */
    iQueue[ iTail ] = ( TUint32* )( anEntry );

    TIscSendFrameInfo* tmp = iParameterQueue[ iTail ];
    // Set additional info for send frame
    tmp->iChannelId = aId;
    
    tmp->iChannelPtr = aChannelPtr;

    tmp->iFrameInfo = aFrameInfo;

    /* adjust tail pointer */
    iTail = ++iTail % iSize;

    /* remember the amount of the requests in the queue */
    iCount++;

    RestoreIrqs( irqLevel );

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// DIscSendQueue::GetFirstFrameInfo
// Returns a frist frame info from list.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TIscSendFrameInfo* DIscSendQueue::GetFirstFrameInfo()
    {
    E_TRACE( ( _T( "ISQ:G(%d, 0x%x)" ), iCount, iQueue ) );

    TIscSendFrameInfo* result;

    TInt irqLevel = DisableIrqs();

    if ( iCount == 0 || iSize == 0 )
        {
        RestoreIrqs( irqLevel );
        return NULL;
        }

    result = iParameterQueue[ iHead ];
    
    RestoreIrqs( irqLevel );

    return result;
    }

