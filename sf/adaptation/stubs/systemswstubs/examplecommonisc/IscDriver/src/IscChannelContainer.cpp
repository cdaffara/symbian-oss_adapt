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
#include <IscDefinitions.h>
#include "IscChannel.h"
#include "IscChannelContainer.h"
#include "IscTrace.h"

#ifdef __WINS__
#include <windows.h>
#endif

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS
const TInt KChannelComplitionDfcPriority( 2 );

// MACROS

// LOCAL CONSTANTS AND MACROS
DIscChannel* IscChannelContainer::iIscChannels[KIscNumberOfUnits][KIscMaxNumberOfChannelSharers];
TDfc* IscChannelContainer::iDfc = NULL;

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
#ifdef __WINS__
CRITICAL_SECTION g_IscCriticalSection;
#endif

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// IscChannelContainer::SetChannel
// Set pointer to channel
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt IscChannelContainer::SetChannel( 
    DIscChannel* aChannel,
    const TUint16 aUnit )
    {
    C_TRACE( ( _T( "IscChannelContainer::SetChannel(0x%x, 0x%x)" ), aChannel, aUnit ) );
    TInt error( KErrNoMemory );

    //check values
    if ( aUnit < KIscNumberOfUnits )
        {
        for ( TInt i( 0 ); i < KIscMaxNumberOfChannelSharers; i++ )
            {
            if ( !iIscChannels[aUnit][i] )
                {
                iIscChannels[aUnit][i] = aChannel;
                error = KErrNone;
                break;
                }
            }
        }
    else
        {
        ASSERT_RESET_ALWAYS( 0, "IscDriver",EIscPanicSetChannel );
        }
    
    C_TRACE( ( _T( "IscChannelContainer::SetChannel - return 0x%x" ), error ) );
    return error;    
    }

// -----------------------------------------------------------------------------
// IscChannelContainer::Channel
// Return pointer to channel
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
DIscChannel* IscChannelContainer::Channel( 
    const TUint16 aChannel, 
    const TUint16 aChannelSharingIndex )    
    {
    TRACE_ASSERT( aChannel < KIscNumberOfUnits );
    TRACE_ASSERT( aChannelSharingIndex < KIscMaxNumberOfChannelSharers );
    return( iIscChannels[ aChannel ][ aChannelSharingIndex ] );    
    }

// -----------------------------------------------------------------------------
// IscChannelContainer::RemoveChannel
// 
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt IscChannelContainer::RemoveChannel
		( 
    	DIscChannel* aChannel
    	)
    {
    
    TRACE_ASSERT( aChannel );
    C_TRACE( ( _T( "IscChannelContainer::RemoveChannel(0x%x)" ), aChannel ) );
    TUint16 channelNumber = aChannel->GetChannelNumber();

    TInt error( KErrNotFound );
    for ( TInt i( KErrNone ); ( i < KIscMaxNumberOfChannelSharers ) && ( KErrNone != error ); i++ )
        {
        if ( iIscChannels[channelNumber][i] == aChannel )
            {
            iIscChannels[channelNumber][i] = NULL;
            error = KErrNone;
            }
        }

    C_TRACE( ( _T( "IscChannelContainer::RemoveChannel - return %d" ), error ) );
    return error;
    
    }

// -----------------------------------------------------------------------------
// IscChannelContainer::ChannelComplition
// Goes through channels and open ones are emptied
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void IscChannelContainer::ChannelComplition( 
    TAny* /*a*/ )
    {
    C_TRACE( ( _T( "IscChannelContainer::ChannelComplition()" ) ) );
#ifdef __WINS__
    EnterCriticalSection( &g_IscCriticalSection );
#endif
    
    DIscChannel* channelPtr = NULL;

    for ( TUint16 channel( KIscFirstChannel ) ; channel < KIscNumberOfUnits; channel++ )
        {
        for ( TUint16 i( 0 ); i < KIscMaxNumberOfChannelSharers; i++ )
            {
            channelPtr = iIscChannels[channel][i];
            if ( channelPtr )
                {
                channelPtr->EmptyBuffers();
                }
            }
        }
#ifdef __WINS__
    LeaveCriticalSection( &g_IscCriticalSection );
#endif

    C_TRACE( ( _T( "IscChannelContainer::ChannelComplition - return 0x%x" ), KErrNone ) );
    
    }

// -----------------------------------------------------------------------------
// IscChannelContainer::AddDfc
// Adds ChannelComplition Dfc
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void IscChannelContainer::AddDfc()
    {
    C_TRACE( ( _T( "IscChannelContainer::AddDfc()" ) ) );

    if ( NKern::CurrentContext() == NKern::EInterrupt )
        {
        iDfc->Add();
        }
    else
        {
        iDfc->Enque();
        }    
    
    }

// -----------------------------------------------------------------------------
// IscChannelContainer::Initialize
// Allocates Dfc function dynamically and formats data tables.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void IscChannelContainer::Initialize()
    {    
    C_TRACE( ( _T( "IscChannelContainer::Initialize()" ) ) );

    if ( !iDfc )
        {
        iDfc = new TDfc( IscChannelContainer::ChannelComplition, NULL, Kern::DfcQue0(),
                         KChannelComplitionDfcPriority );
        ASSERT_RESET_ALWAYS( iDfc, "IscDriver",EIscMemoryAllocationFailure );
        }
    
    for ( TUint16 channel( 0 ); channel < KIscNumberOfUnits; channel++ )
        {
        for ( TUint16 i( 0 ); i < KIscMaxNumberOfChannelSharers; i++ )
            {
            iIscChannels[channel][i] = NULL;
            }
        }

#ifdef __WINS__
    InitializeCriticalSection( &g_IscCriticalSection );
#endif
    }

// -----------------------------------------------------------------------------
// IscChannelContainer::DeActivate
// Deallocates Dfc function
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void IscChannelContainer::DeActivate()
    {    
    if ( iDfc )
        {
        delete iDfc;
        iDfc = NULL;
        }
#ifdef __WINS__
    DeleteCriticalSection( &g_IscCriticalSection );
#endif
    
    }

// -----------------------------------------------------------------------------
// IscChannelContainer::ValidateChannel
// 
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt IscChannelContainer::ValidateChannel( 
    DIscChannel* aChannelPtr )
    {
    C_TRACE( ( _T( "IscChannelContainer::ValidateChannel(0x%x)" ), aChannelPtr ) );
    TInt error( KErrNotFound );
    if( !aChannelPtr )
    	{
    	C_TRACE( ( _T( "IscChannelContainer::ValidateChannel aChannelPtr was NULL" ) ) );
    	error = KErrArgument;	
    	}
    else
    	{
	    for ( TUint16 channelNumber( 0 ); channelNumber < KIscNumberOfUnits ; channelNumber++ )
	        {
	        for ( TInt i( KErrNone ); i < KIscMaxNumberOfChannelSharers ; i++ )
	            {
	            if ( iIscChannels[channelNumber][i] == aChannelPtr )
	                {
	                error = KErrNone;
	                break;
	                }
	            }
	        }    		
    	}

    C_TRACE( ( _T( "IscChannelContainer::ValidateChannel - return %d" ), error ) );
    return error;
    
    }

//  End of File  
