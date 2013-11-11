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
#include <IscDefinitions.h>
#include <IscMultiplexerBase.h>
#include "IscMainRcvBuffer.h"
#include "IscQueue.h"
#include "IscDevice.h"
#include "IscChannel.h"
#include "IscChannelContainer.h"
#include "IscTrace.h"


// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS
DIscMainRcvBuffer* DIscMainRcvBuffer::iThisPointer = NULL;
const TInt KBufferCleanUpDfcPriority( 4 );

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// DIscMainRcvBuffer::DIscMainRcvBuffer
// C++ default constructor
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
DIscMainRcvBuffer::DIscMainRcvBuffer( DIscDevice *aDevice, TUint16 aIscMainRcvBufferQueueSize )
    :iDevice ( aDevice ),
     iDfc( NULL ),
     iMainRcvBufferQueue( NULL )
    {
    iIscMainRcvBufferQueueSize = aIscMainRcvBufferQueueSize;
    iMainRcvBuffer = new TUint32*[iIscMainRcvBufferQueueSize];
    ASSERT_RESET_ALWAYS( iMainRcvBuffer, "IscDriver",EIscMemoryAllocationFailure );

    for ( TInt i = 0; i < iIscMainRcvBufferQueueSize; i++ )
        {
        iMainRcvBuffer[i] = NULL;
        }
    }


// -----------------------------------------------------------------------------
// DIscMainRcvBuffer::DoCreate
// Second-phase constructor to complete construction.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscMainRcvBuffer::DoCreate()
    {
    if ( iThisPointer ) //Only one instance of this class is allowed
        {
        ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscMainRcvBufferInitialize );
        }

    iDfc = new TDfc( BufferCleanUp, this, Kern::DfcQue0(), KBufferCleanUpDfcPriority );
    ASSERT_RESET_ALWAYS( iDfc, "IscDriver",EIscMemoryAllocationFailure );
    
    iMainRcvBufferQueue = new DIscQueue( iMainRcvBuffer, iIscMainRcvBufferQueueSize );
    ASSERT_RESET_ALWAYS( iMainRcvBufferQueue, "IscDriver",EIscMemoryAllocationFailure );

    //Store pointer of this instance that can be used from static functions
    iThisPointer = this;
    }

//Destructor
DIscMainRcvBuffer::~DIscMainRcvBuffer()
    {
    delete iDfc;
    delete iMainRcvBufferQueue;
    iDfc = NULL;
    iMainRcvBufferQueue = NULL;
    iThisPointer = NULL;
    iDevice = NULL;
    }

// -----------------------------------------------------------------------------
// DIscMainRcvBuffer::MsgReceive
// Function that should be called to store incoming frame
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscMainRcvBuffer::MsgReceive( TDesC8* aData )
    {    
    E_TRACE( ( _T( "IMRB:MR(0x%x)" ), aData ) );
    //Check that instance of this class is created
    if ( !iThisPointer )
        {
        // MainRcvBuffer not initialized -> panic
        ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscMainRcvBufferInitialize );
        }
    
    TInt r = iThisPointer->iMainRcvBufferQueue->Add( aData );
    ASSERT_RESET_ALWAYS( r == KErrNone, "IscDriver",EIscMainRcvBufferOverflow );
    iThisPointer->AddDfc();
    }

// -----------------------------------------------------------------------------
// DIscMainRcvBuffer::AddDfc
// Function for adding Dfc
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscMainRcvBuffer::AddDfc()
    {    
#ifndef __WINS__
    if ( NKern::CurrentContext() == NKern::EInterrupt )
        {
        iDfc->Add();
        }
    else
        {
        iDfc->Enque();
        }
#else
    // Have to do DoEnque since Enque() & Add() cannot be called from pure 
    // win32 thread context. And this call might come directly in win32 thread
    // since the implementation of ISC Data Transmission API can do virtually 
    // anything in WINS
    iDfc->DoEnque();
#endif
    }

// -----------------------------------------------------------------------------
// DIscMainRcvBuffer::BufferCleanUp
// Function to empty messages in main buffer
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscMainRcvBuffer::BufferCleanUp( TAny* aPtr )
    {
    C_TRACE( ( _T( "DIscMainRcvBuffer::BufferCleanUp(0x%x)" ), aPtr ) );
    
    DIscMainRcvBuffer* Buffer = ( DIscMainRcvBuffer* )aPtr;
    
    
    TUint16 rcvChannelId( KIscFrameReceiverNotFound );
    TAny* channelPtr = NULL;
    DIscChannel* tempPtr = NULL;
    TBool channelFound( EFalse );

    TDesC8* ptr = NULL;
    ptr = ( TDesC8* )Buffer->iMainRcvBufferQueue->RemoveFirst();
        
    while ( ptr )
        {
        Buffer->iDevice->iIscMultiplexerInterface->GetRcvChannelId( *ptr, rcvChannelId, channelPtr );
    
        C_TRACE( ( _T( "DIscMainRcvBuffer::BufferCleanUp channelId (0x%x) channelPtr (0x%x)" ), rcvChannelId, channelPtr ) );
        if ( rcvChannelId >= KIscFirstChannel &&  rcvChannelId < KIscNumberOfUnits && channelPtr )
            {
            channelFound = EFalse;
            for ( TUint16 i( 0 ); ( i < KIscMaxNumberOfChannelSharers ) && ( !channelFound ); i++ )
                {
                tempPtr = IscChannelContainer::Channel( rcvChannelId, i );
                if ( tempPtr == channelPtr )
                    {
                    // correct channel found
                    ( ( DIscChannel* )channelPtr )->StoreFrame( ptr );
                    channelFound = ETrue;
                    }
                tempPtr = NULL;
                }
            if ( !channelFound )
                {
                TRACE_ASSERT_ALWAYS;
                // correct channel was not found -> release frame
                Buffer->iDevice->ReleaseMemoryBlock( ( TDes8* ) ptr );
                }
            }
        else if ( rcvChannelId == KIscFrameReceiverNotFound )
            {
            C_TRACE( ( _T( "DIscMainRcvBuffer::BufferCleanUp Frame Receiver not found!" ) ) );
            Buffer->iDevice->ReleaseMemoryBlock( ( TDes8* ) ptr );
            }
        // Frame going to control channel
        else if ( rcvChannelId == 0x00 )
            {
            Buffer->iDevice->iIscMultiplexerInterface->HandleControlFrame( *ptr );
            }
        else
            {
            TRACE_ASSERT_ALWAYS;
            Buffer->iDevice->ReleaseMemoryBlock( ( TDes8* ) ptr );
            }

        // get the next frame from the queue
        ptr = ( TDes8* )Buffer->iMainRcvBufferQueue->RemoveFirst();
        }

    IscChannelContainer::ChannelComplition( NULL );

    C_TRACE( ( _T( "DIscMainRcvBuffer::BufferCleanUp - return 0x%x" ) ) );

    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

//  End of File  
