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
* Description:  Implementation of DIscDataTransmissionBase class
*
*/



// INCLUDE FILES
#include <IscDevice.h>
#include "IscTrace.h"
#include "IscDataTransmissionBase.h"
#include "IscBufferAllocator.h"

#ifdef __WINS__
#include <windows.h>
#endif

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
#ifdef __WINS__
CRITICAL_SECTION g_IscDTBCriticalSection;
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// DIscDataTransmissionBase::DIscDataTransmissionBase
// C++ default constructor
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C DIscDataTransmissionBase::DIscDataTransmissionBase()
    :iDevice( NULL ),
    iIscBufferAllocator( NULL )
    {
    A_TRACE( ( _T( "DIscDataTransmissionBase::DIscDataTransmissionBase()" ) ) );
#ifdef __WINS__
     InitializeCriticalSection( &g_IscDTBCriticalSection );
#endif
    }

    
// Destructor
EXPORT_C DIscDataTransmissionBase::~DIscDataTransmissionBase()
    {
    A_TRACE( ( _T( "DIscDataTransmissionBase::~DIscDataTransmissionBase()" ) ) );
    if ( iIscBufferAllocator )
        {
        delete iIscBufferAllocator;
        iIscBufferAllocator=NULL;
        }
#ifdef __WINS__
    DeleteCriticalSection( &g_IscDTBCriticalSection );
#endif
    A_TRACE( ( _T( "DIscDataTransmissionBase::~DIscDataTransmissionBase - return void" ) ) );
    }


// -----------------------------------------------------------------------------
// DIscDataTransmissionBase::AllocBuffers
// buffer allocations according to the ISC Multiplexer
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDataTransmissionBase::AllocBuffers( TIscBufferEntry* aBufferConfig )
    {
    A_TRACE( ( _T( "DIscDataTransmissionBase::AllocBuffers(0x%x)" ), aBufferConfig ) );
    // Allocate buffers
    if ( !iIscBufferAllocator )
        {
        iIscBufferAllocator = new DIscBufferAllocator( aBufferConfig );        
        ASSERT_RESET_ALWAYS( iIscBufferAllocator, "ISCDataTransmissionBase",EIscMemoryAllocationFailure );
        iIscBufferAllocator->AllocBuffers();
        }
    A_TRACE( ( _T( "DIscDataTransmissionBase::AllocBuffers - return void" ) ) );
    }
// -----------------------------------------------------------------------------
// DIscDataTransmissionBase::ReceiveFrame
// Transfer incoming frame to ISC Driver
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDataTransmissionBase::ReceiveFrame( 
    TDesC8& aFrame )
    {
        
    if ( iDevice )
        {
        iDevice->Receive( &aFrame );
        }
    else
        {
        TRACE_ASSERT_ALWAYS;
        }

    }

// -----------------------------------------------------------------------------
// DIscDataTransmissionBase::NotifyConnectionStatus
// Notifies any connection status changes to ISC Driver
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDataTransmissionBase::NotifyConnectionStatus( 
    TInt aStatus )
    {
    A_TRACE( ( _T( "DIscDataTransmissionBase::NotifyConnectionStatus(0x%x)" ), aStatus ) );

    if ( iDevice )
        {
        iDevice->NotifyConnectionStatus( aStatus );
        }

    A_TRACE( ( _T( "DIscDataTransmissionBase::NotifyConnectionStatus - return void" ) ) );
    }


// -----------------------------------------------------------------------------
// DIscDataTransmissionBase::ReserveMemoryBlock
// Reserve memory block
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDataTransmissionBase::ReserveMemoryBlock( 
    TDes8*& aPtr, 
    TUint16 aSize )
    {    
    E_TRACE( ( _T( "IDTB:Res(0x%x)" ), aSize ) );
      
    iIscBufferAllocator->ReserveMemoryBlock( aPtr, aSize );
    }


// -----------------------------------------------------------------------------
// DIscDataTransmissionBase::ReleaseMemoryBlock
// Release the reserved memory block
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDataTransmissionBase::ReleaseMemoryBlock( 
    TDes8* aPtr )
    {

    E_TRACE( ( _T( "IDTB:Rel (0x%x)" ), aPtr ) );    
    iIscBufferAllocator->ReleaseMemoryBlock( aPtr );

    }


// -----------------------------------------------------------------------------
// DIscDataTransmissionBase::Connect
// Connects ISC Driver to ISC Data Transmission Driver
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDataTransmissionBase::Connect( 
    DIscDevice* aDevice )
    {
    A_TRACE( ( _T( "DIscDataTransmissionBase::Connect(0x%x)" ), aDevice ) );
    ASSERT_RESET_ALWAYS( aDevice, "ISCDataTransmissionBase", EIscNullIscDriverPointer );
    iDevice = aDevice;
    A_TRACE( ( _T( "DIscDataTransmissionBase::Connect - return void" ) ) );
    }


// -----------------------------------------------------------------------------
// DIscDataTransmissionBase::Install
// Implementation of pure-virtual Install from DLogicalDevice
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscDataTransmissionBase::Install()
    {
    A_TRACE( ( _T( "DIscDataTransmissionBase::Install()" ) ) );
    SetName( &KIscDataTransmissionDriverName );
    
    // call second phase install ( located in the driver realizing this API )
    TInt r = Install2ndPhase();
    
    A_TRACE( ( _T( "DIscDataTransmissionBase::Install - return 0x%x" ), r ) );
    return r;
    }

#if defined( __WINSCW__ )
DECLARE_STANDARD_LDD()
    {
    return NULL;
    }
#endif

//  End of File  
