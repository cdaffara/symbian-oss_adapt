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
* Description:  Implementation of DIscMultiplexerBase class
*
*/



// INCLUDE FILES
#include <IscDefinitions.h>
#include <IscDevice.h>
#include "IscMultiplexerBase.h"
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
// DIscMultiplexerBase::DIscMultiplexerBase
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C DIscMultiplexerBase::DIscMultiplexerBase()
    :iDevice( NULL )
    {
    C_TRACE( ( _T( "DIscMultiplexerBase::DIscMultiplexerBase()" ) ) );
    C_TRACE( ( _T( "DIscMultiplexerBase::DIscMultiplexerBase - return 0x%x" ) ) );
    }
    
// Destructor
EXPORT_C DIscMultiplexerBase::~DIscMultiplexerBase()
    {    
    C_TRACE( ( _T( "DIscMultiplexerBase::~DIscMultiplexerBase()" ) ) );
    C_TRACE( ( _T( "DIscMultiplexerBase::~DIscMultiplexerBase - return 0x%x" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::CustomFunction
// API extension function
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
#ifndef API_TRACE_FLAG
EXPORT_C TInt DIscMultiplexerBase::CustomFunction( 
    const TUint16,
    const TUint16 aOperation,
    TAny* ,
    const TAny* aChannelPtr )
#else 
EXPORT_C TInt DIscMultiplexerBase::CustomFunction( 
    const TUint16 aChannelId,
    const TUint16 aOperation,
    TAny* aParameters,
    const TAny* aChannelPtr )
#endif
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::CustomFunction(0x%x, 0x%x, 0x%x, 0x%x)" ), aChannelId, aOperation, aParameters, aChannelPtr ) );
    CompleteRequest( aOperation, KErrNotSupported, aChannelPtr );
    return KErrNotSupported;
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::GetChannelInfo
// Get information on specified channel
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
#ifndef API_TRACE_FLAG
EXPORT_C TInt DIscMultiplexerBase::GetChannelInfo( 
    const TUint16, 
    TDes8& )
#else 
EXPORT_C TInt DIscMultiplexerBase::GetChannelInfo( 
    const TUint16 aChannelNumber, 
    TDes8& aInfo )
#endif

    {
    A_TRACE( ( _T( "DIscMultiplexerBase::GetChannelInfo(0x%x, 0x%x)" ), aChannelNumber, &aInfo ) );
    return KErrNotSupported;
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::CompleteRequest
// Complete specified asynchronous request
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
// This method has been modified to allow channel sharing between application.
// The completion routine uses directly a pointer on a DLogicalChannel instead of a channel index
//
EXPORT_C void DIscMultiplexerBase::CompleteRequest( 
    TUint16 aReqNumber, 
    TInt aStatus, 
    const TAny* aChannelPtr )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::CompleteRequest(0x%x, 0x%x, 0x%x)" ), aReqNumber, aStatus, aChannelPtr ) );
    iDevice->CompleteRequest( aReqNumber, aStatus, aChannelPtr );
    A_TRACE( ( _T( "DIscMultiplexerBase::CompleteRequest - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::IsPending
// Check if specified request is pending in ISC Driver
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscMultiplexerBase::IsPending( 
    const TUint16 aReqNumber, 
    const TAny* aChannelPtr )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::IsPending(0x%x, 0x%x)" ), aReqNumber, aChannelPtr ) );
    return iDevice->IsPending( aReqNumber, aChannelPtr );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::QueueFrame
// Queue frame for sending
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscMultiplexerBase::QueueFrame( 
    const TUint16 aChannelId,
    const TDesC8* aFrame,
    const TAny* aChannelPtr,
    TAny* aFrameInfo )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::QueueFrame(0x%x, 0x%x, 0x%x, 0x%x)" ), aChannelId, aFrame, aChannelPtr, aFrameInfo ) );
    TInt error( KErrNone );
    if ( !aFrame )
        {
        error = KErrBadDescriptor;
        }
    else 
        {
        error = iDevice->QueueFrame( aChannelId, aFrame, aChannelPtr, aFrameInfo );
        }

    return error;
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::FlushQueue
// Start sending the frames
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscMultiplexerBase::FlushQueues()
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::FlushQueue()" ) ) );
    iDevice->FlushQueues();
    A_TRACE( ( _T( "DIscMultiplexerBase::FlushQueue - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::ULFlowControl
// Notify flow control changes to ISC Driver
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscMultiplexerBase::ULFlowControl( 
    const TInt aULFlowStatus, 
    TUint16 aChannelId,
    TAny* aChannelPtr )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::ULFlowControl(0x%x, 0x%x)" ), aULFlowStatus, aChannelPtr ) );    
    iDevice->ULFlowControl( aULFlowStatus, aChannelId, aChannelPtr );
    A_TRACE( ( _T( "DIscMultiplexerBase::ULFlowControl - return void" ) ) );
    }


// -----------------------------------------------------------------------------
// DIscMultiplexerBase::ReserveMemoryBlock
// Reserve memory block
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscMultiplexerBase::ReserveMemoryBlock( 
    TDes8*& aPtr, 
    const TUint16 aSize )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::ReserveMemoryBlock(0x%x, 0x%x)" ), aPtr, aSize ) );
    iDevice->ReserveMemoryBlock( aPtr,aSize );
    A_TRACE( ( _T( "DIscMultiplexerBase::ReserveMemoryBlock - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::ReleaseMemoryBlock
// Release the reserved memory block
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscMultiplexerBase::ReleaseMemoryBlock( 
    TDes8* aPtr )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::ReleaseMemoryBlock(0x%x)" ), aPtr ) );
    iDevice->ReleaseMemoryBlock( aPtr );
    A_TRACE( ( _T( "DIscMultiplexerBase::ReleaseMemoryBlock - return void" ) ) );
    }


// -----------------------------------------------------------------------------
// DIscMultiplexerBase::CopyFromUserBuffer
// Due to GetThreadPtr this method will be deprecated in the future.
// It is preffered to use GetThreadPtr instead. 
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscMultiplexerBase::CopyFromUserBuffer( 
    const TDesC8& aUserBuffer,
    TDes8& aKernelBuffer,
    const TAny* aChannelPtr,
    const TInt aOffset )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::CopyFromUserBuffer(0x%x, 0x%x, 0x%x, 0x%x)" ), &aUserBuffer, &aKernelBuffer, aChannelPtr, aOffset ) );
    return iDevice->CopyFromUserBuffer( aUserBuffer, aKernelBuffer, aChannelPtr, aOffset );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::CopyToUserBuffer
// Due to GetThreadPtr this method will be deprecated in the future.
// It is preffered to use GetThreadPtr instead.  
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscMultiplexerBase::CopyToUserBuffer( 
    TAny* aUserBuffer, 
    const TDesC8& aKernelBuffer, 
    const TAny* aChannelPtr,
    const TInt aOffset )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::CopyToUserBuffer(0x%x, 0x%x, 0x%x, 0x%x)" ), aUserBuffer, &aKernelBuffer, aChannelPtr, aOffset ) );
    return iDevice->CopyToUserBuffer( aUserBuffer, aKernelBuffer, aChannelPtr, aOffset );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::GetThreadPtr
// Returns user side thread pointer no owenership is given.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C DThread* DIscMultiplexerBase::GetThreadPtr( const TAny* aChannelPtr )
	{
    A_TRACE( ( _T( "DIscMultiplexerBase::DIscMultiplexerBase::GetThreadPtr(0x%x)" ), aChannelPtr ) );
	TRACE_ASSERT( aChannelPtr );
	return iDevice->GetThreadPtr( aChannelPtr );	
	}

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::CancelSending
// 
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscMultiplexerBase::CancelSending( 
    const TUint16 aChannelId, 
    const TAny* aChannelPtr )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::CancelSending(0x%x, 0x%x)" ), aChannelId, aChannelPtr ) );
    return iDevice->CancelSending( aChannelId, aChannelPtr );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::Connect
// Connect ISC Driver to ISC Multiplexer
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscMultiplexerBase::Connect( 
    DIscDevice* aDevice )
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::Connect(0x%x)" ), aDevice ) );
    ASSERT_RESET_ALWAYS( aDevice, "IscMultiplexerBase",EIscNullIscDriverPointer );
    iDevice = aDevice;
    A_TRACE( ( _T( "DIscMultiplexerBase::Connect - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::SetInitializationParameters
// Initialization string to multiplexer in case if needed before datatransmission driver has been initialized
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
#ifndef API_TRACE_FLAG
EXPORT_C void DIscMultiplexerBase::SetInitializationParameters( const TDesC8&)
#else 
EXPORT_C void DIscMultiplexerBase::SetInitializationParameters( const TDesC8& aParameters )
#endif
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::SetInitializationParameters not supported(0x%x)" ), &aParameters ) );
    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::Install
// Install from DLogicalDevice
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscMultiplexerBase::Install()
    {
    A_TRACE( ( _T( "DIscMultiplexerBase::Install()" ) ) );
    SetName( &KIscMultiplexerName );
    TInt r = Install2ndPhase();
    C_TRACE( ( _T( "DIscMultiplexerBase::Install - return 0x%x" ), r ) );
    return r;
    }

// ================= E32Dll ==============
// E32Dll
// Dll entry point.
// Returns: TInt Invariably return KErrNone.
//
#if defined( __WINSCW__ )
DECLARE_STANDARD_LDD()
    {
    return NULL;
    }
#endif
//  End of File
