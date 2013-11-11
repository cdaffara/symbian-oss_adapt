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
#include <kern_priv.h>
#include <IscDataTransmissionBase.h>
#include <IscMultiplexerBase.h>

#include "IscMainRcvBuffer.h"
#include "IscQueue.h"
#include "IscSendQueue.h"
#include "IscDevice.h"
#include "IscChannel.h"
#include "IscChannelContainer.h"
#include "IscTrace.h"

#ifdef __WINS__
#include <windows.h>
#endif


// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS
_LIT( KIscDriverName, "IscDriver" );

// MACROS

// LOCAL CONSTANTS AND MACROS
const TInt KSendDfcPriority( 4 );
const TInt KNotifyDfcPriority( 5 );
const TInt KIscInterruptLevelTwo( 2 );

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


DIscDataTransmissionBase* DIscDevice::iIscDataTransmissionInterface = NULL;
DIscMultiplexerBase* DIscDevice::iIscMultiplexerInterface = NULL;
DIscSendQueue* DIscDevice::iSendQueue=NULL;
DIscSendQueue* DIscDevice::iTempQueue=NULL;
DIscSendQueue* DIscDevice::iControlSendQueue=NULL;
TDfc* DIscDevice::iSendDfc = NULL;
TDfc* DIscDevice::iNotifyDfc = NULL;
TInt DIscDevice::iConnectionStatus = EIscConnectionNotOk;

#ifdef __WINS__
CRITICAL_SECTION g_IscDTBCriticalSection;
#endif

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// DIscDevice::DIscDevice
// C++ default constructor
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C DIscDevice::DIscDevice()
    :DLogicalDevice(),
    iSend( NULL ),
    iTempSend( NULL ),
    iControlSend( NULL ),
    iSendFrameParameters( NULL ),
    iTempFrameParameters( NULL ),
    iControlFrameParameters( NULL ),
    iIscMainRcvBuffer( NULL )     
    {
    C_TRACE( ( _T( "DIscDevice::DIscDevice()" ) ) );

    iVersion = TVersion( KMajorVersionNumber, KMinorVersionNumber,
    KBuildVersionNumber );
    iParseMask |= KDeviceAllowUnit;
    iParseMask |= KDeviceAllowInfo;

#ifdef __WINS__
     InitializeCriticalSection( &g_IscDTBCriticalSection );
#endif 
    }

// Destructor
EXPORT_C DIscDevice::~DIscDevice()
    {
    C_TRACE( ( _T( "DIscDevice::~DIscDevice()" ) ) );

    IscChannelContainer::DeActivate();
    
    if ( iIscMainRcvBuffer )
        {
        delete iIscMainRcvBuffer;
        iIscMainRcvBuffer = NULL;
        }


    iIscDataTransmissionInterface = NULL;
    iIscMultiplexerInterface = NULL;

    delete iTempSend;
    delete iControlSend;
    delete iSendQueue;
    delete iTempQueue;
    delete iControlSendQueue;
    delete iSendDfc;
    delete iNotifyDfc;

#ifdef __WINS__
     DeleteCriticalSection( &g_IscDTBCriticalSection );
#endif
        
    }


// -----------------------------------------------------------------------------
// DIscDevice::Install
// Complete the installation of driver
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscDevice::Install()
    {
    C_TRACE( ( _T( "DIscDevice::Install()" ) ) );

    // Dfc for sending frames
    iSendDfc = new TDfc( Flush, this, Kern::DfcQue0(), KSendDfcPriority );
    iNotifyDfc = new TDfc( NotifyConnection, this, Kern::DfcQue0(), KNotifyDfcPriority );        
    ASSERT_RESET_ALWAYS( iSendDfc, "IscDriver",EIscMemoryAllocationFailure );

    //Initialize IscChannelContainer
    IscChannelContainer::Initialize();
    
    // connect to multiplexer and data transmission driver
    TInt r = InitializeLdd2LddInterface();
    if ( r != KErrNone )
        {
        TRACE_ASSERT_ALWAYS;
        return r;
        }

    return ( SetName( &KIscDriverName ) );
    }

// -----------------------------------------------------------------------------
// DIscDevice::Initialize
// Complete the initialization of driver
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscDevice::Initialize()
    {
    C_TRACE( ( _T( "DIscDevice::Initialize()" ) ) );

    TInt i( 0 );

    // Get buffer configuration data from multiplexer
    TIscConfiguration configData;
    iIscMultiplexerInterface->GetConfiguration( configData );

    // Create main buffer  
    iIscMainRcvBuffer = new DIscMainRcvBuffer( this, configData.mainRcvQueueSize );
    ASSERT_RESET_ALWAYS( iIscMainRcvBuffer, "IscDriver",EIscMemoryAllocationFailure );
    
    // Do second phase installation    
    iIscMainRcvBuffer->DoCreate();
    
    // Create queue for sending frames
    iSend = new TUint32*[configData.channelSendQueueSize];
    ASSERT_RESET_ALWAYS( iSend, "IscDriver",EIscMemoryAllocationFailure );
    
    iSendFrameParameters = new TIscSendFrameInfo*[configData.channelSendQueueSize];
    for ( i = 0; i < configData.channelSendQueueSize; i++ )
        {
        iSendFrameParameters[i] = new TIscSendFrameInfo;
        }
    ASSERT_RESET_ALWAYS( iSendFrameParameters, "IscDriver",EIscMemoryAllocationFailure );

    iSendQueue = new DIscSendQueue( iSend, iSendFrameParameters, configData.channelSendQueueSize );
    ASSERT_RESET_ALWAYS( iSendQueue, "IscDriver",EIscMemoryAllocationFailure );


    // create temporary queue
    iTempSend = new TUint32*[configData.channelSendQueueSize];
    ASSERT_RESET_ALWAYS( iTempSend, "IscDriver",EIscMemoryAllocationFailure );

    iTempFrameParameters = new TIscSendFrameInfo*[configData.channelSendQueueSize];
    for ( i =0; i < configData.channelSendQueueSize; i++ )
        {
        iTempFrameParameters[i] = new TIscSendFrameInfo;
        }
    ASSERT_RESET_ALWAYS( iTempFrameParameters, "IscDriver",EIscMemoryAllocationFailure );

    iTempQueue = new DIscSendQueue( iTempSend, iTempFrameParameters, configData.channelSendQueueSize );
    ASSERT_RESET_ALWAYS( iTempQueue, "IscDriver",EIscMemoryAllocationFailure );


    // Create send queue for control channel
    iControlSend = new TUint32*[configData.channelSendQueueSize];
    ASSERT_RESET_ALWAYS( iControlSend, "IscDriver",EIscMemoryAllocationFailure );

    iControlFrameParameters = new TIscSendFrameInfo*[configData.channelSendQueueSize];
    for ( i = 0; i < configData.channelSendQueueSize; i++ )
        {
        iControlFrameParameters[i] = new TIscSendFrameInfo;
        }
    ASSERT_RESET_ALWAYS( iControlFrameParameters, "IscDriver",EIscMemoryAllocationFailure );

    iControlSendQueue = new DIscSendQueue( iControlSend, iControlFrameParameters, configData.channelSendQueueSize );
    ASSERT_RESET_ALWAYS( iControlSendQueue, "IscDriver",EIscMemoryAllocationFailure );
    
    iIscDataTransmissionInterface->AllocBuffers( configData.bufferConfig );
    
    iConnectionStatus = iIscDataTransmissionInterface->ConnectionStatus();

    iIscMultiplexerInterface->NotifyConnectionStatus( iConnectionStatus );
    C_TRACE( ( _T( "DIscDevice::Initialize - return void" ) ) );
    }
// -----------------------------------------------------------------------------
// DIscDevice::GetCaps
// 
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDevice::GetCaps( 
    TDes8& /*aDes*/ ) const
    {
    // GetCaps implemented to keep compiler happy
    }

// -----------------------------------------------------------------------------
// DEcsDevice::Create
// From DLogicalDevice
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscDevice::Create( 
    DLogicalChannelBase*& aChannel )
    {
    aChannel=new DIscChannel( this );
    return aChannel?KErrNone:KErrNoMemory;
    }

// -----------------------------------------------------------------------------
// DIscDevice::Receive
// Frames coming from Domestic OS
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDevice::Receive( 
    TDesC8* aData ) const
    {
    DIscMainRcvBuffer::MsgReceive( aData );
    }


// -----------------------------------------------------------------------------
// DIscDevice::ReserveMemoryBlock
// Get message block from buffers allocated in IscDataTransmissionBase.dll
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDevice::ReserveMemoryBlock( TDes8*& aPtr, TUint16 aSize )
    {
    iIscDataTransmissionInterface->ReserveMemoryBlock( aPtr, aSize );
    }

// -----------------------------------------------------------------------------
// DIscDevice::ReleaseMemoryBlock
// Release memory block allocated with ReserveMemoryBlock
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDevice::ReleaseMemoryBlock( 
    TDes8* aPtr )
    {
    C_TRACE( ( _T( "DIscDevice::ReleaseMemoryBlock(0x%x)" ), aPtr ) );

    if ( iIscDataTransmissionInterface )
        {
        iIscDataTransmissionInterface->ReleaseMemoryBlock( aPtr );
        }
    }

// -----------------------------------------------------------------------------
// DIscDevice::NotifyConnectionStatus
// Connection status change function
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDevice::NotifyConnectionStatus( const TInt aStatus )
    {
    C_TRACE( ( _T( "DIscDevice::NotifyConnectionStatus(0x%x)" ), aStatus ) );
    if ( iConnectionStatus != aStatus )
        {
        iConnectionStatus = aStatus;
		if ( NKern::CurrentContext() == NKern::EInterrupt )
			{
			iNotifyDfc->Add();
			}
		else
			{
		    iNotifyDfc->Enque();
		    }
        }
        
    C_TRACE( ( _T( "DIscDevice::NotifyConnectionStatus() return" ) ) );    
    }

// -----------------------------------------------------------------------------
// DIscDevice::NotifyConnection
// Connection status change DFC function.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscDevice::NotifyConnection( TAny* )
    {
    C_TRACE( ( _T( "DIscDevice::NotifyConnection(0x%x)" ), iConnectionStatus ) );
    
    iIscMultiplexerInterface->NotifyConnectionStatus( iConnectionStatus );
    DIscChannel* tempPtr = NULL;
    for ( TUint16 i = KIscFirstChannel; i < KIscNumberOfUnits; i++ )
        {
        for ( TUint16 ii( 0 ); ii < KIscMaxNumberOfChannelSharers; ii++ )
            {
            tempPtr = IscChannelContainer::Channel( i, ii );
            if ( tempPtr )
                { 
                tempPtr->NotifyConnectionStatus( iConnectionStatus );
                tempPtr = NULL;
                }
            }
        }
    C_TRACE( ( _T( "DIscDevice::NotifyConnection() return" ) ) );

    }

// -----------------------------------------------------------------------------
// DIscDevice::ULFlowControl
// Function to notify client about uplink flow control status
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDevice::ULFlowControl( 
    const TInt aULFlowStatus,
    const TUint16 aChannelId,
    const TAny* aChannelPtr )
    {
    C_TRACE( ( _T( "DIscDevice::ULFlowControl(0x%x, 0x%x, 0x%x)" ), aULFlowStatus, aChannelId, aChannelPtr ) );

    DIscChannel* tempPtr = NULL;
    TBool channelFound(EFalse);

    if ( !aChannelPtr )
        {
        // All channels.
        for ( TUint16 i(0); i < KIscMaxNumberOfChannelSharers; i++ )
            {
            tempPtr = IscChannelContainer::Channel( aChannelId, i );
            if ( tempPtr )
                {
                tempPtr->NotifyFlowControl( aULFlowStatus );
                tempPtr = NULL;
                channelFound = ETrue;
                }
            else
            	{
            	//Do nothing	
            	}
            }
        }
    else
        {
        // Single channel.
        for ( TUint16 i(0); i < KIscMaxNumberOfChannelSharers; i++ )
            {
            tempPtr = IscChannelContainer::Channel( aChannelId, i );
            if ( tempPtr == ( DIscChannel* )aChannelPtr )
                {
                tempPtr->NotifyFlowControl( aULFlowStatus );
                tempPtr = NULL;
                channelFound = ETrue;
                break;
                }
            else
            	{
            	//Do nothing	
            	}                
            }            
        }

    if ( channelFound == EFalse )
        TRACE_ASSERT_ALWAYS;
    
    }

// -----------------------------------------------------------------------------
// DIscDevice::IsPending
// Function to check if asycnhronous request is active
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscDevice::IsPending( 
    const TUint16 aReqNumber, 
    const TAny* aChannelPtr )
    {
    C_TRACE( ( _T( "DIscDevice::IsPending(0x%x, 0x%x)" ), aReqNumber, aChannelPtr ) );

    DIscChannel* tempPtr = ( DIscChannel* )aChannelPtr;
    TInt error = IscChannelContainer::ValidateChannel( tempPtr );
    if( error == KErrNone )
    	{
    	error = tempPtr->IsPending( aReqNumber );
    	}
    C_TRACE( ( _T( "DIscDevice::IsPending - return %d" ), error ) );	
   	return error;

    }

// -----------------------------------------------------------------------------
// DIscDevice::DLFlowControl
// Notify multiplexer about down link flow control
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscDevice::DLFlowControlNotify( 
    const TInt aDLFlowStatus,
    const TUint16 aChannel,
    const TAny* aChannelPtr )
    {
    C_TRACE( ( _T( "DIscDevice::DLFlowControlNotify(0x%x, 0x%x)" ), aDLFlowStatus, aChannel ) );
    if ( aChannel >= KIscFirstChannel 
        && aChannel < KIscNumberOfUnits )
        {        
        if ( aDLFlowStatus == EIscFlowControlOn )
            {
            iIscMultiplexerInterface->DLFlowControl( EIscFlowControlOn, aChannel, aChannelPtr );
            }        
        else
            {
            iIscMultiplexerInterface->DLFlowControl( EIscFlowControlOff, aChannel, aChannelPtr );            
            }
        }
    else
        {
        
        if ( aChannel == 0x00 )
            {
            // control channel, no flow control used
            }
        else
            {
            // should never came here
            TRACE_ASSERT_ALWAYS;
            }
        }
    }

// -----------------------------------------------------------------------------
// DIscDevice::InitializeLdd2LddInterface
// Function to connect to DataTransmission and Multiplexer ldds
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscDevice::InitializeLdd2LddInterface()
    {
    C_TRACE( ( _T( "DIscDevice::InitializeLdd2LddInterface()" ) ) );
    
    // Find pointer to second level LDD.
    DObjectCon* lDevices = Kern::Containers()[ ELogicalDevice ];
    TKName driverName;
    ASSERT_RESET_ALWAYS( lDevices, "IscDriver", EIscLogicalDevicesNotFound );

    TInt err( KErrNone );
    //TInt driverHandle( KErrNone ); // API change in SOS9.2 WK08
	TFindHandle driverHandle;
    // Find pointer to ISC Multiplexer.
    err = lDevices->FindByName( driverHandle, KIscMultiplexerName, driverName );
    if( KErrNone != err )
        {
        C_TRACE( ( _T( "DIscDevice::InitializeLdd2LddInterface() ISC Multiplexer Not Found!" ) ) );
        ASSERT_RESET_ALWAYS( 0, "IscDriver" ,EIscMultiplexerNotFound );
        }
	
    iIscMultiplexerInterface = static_cast<DIscMultiplexerBase*>( lDevices->At( driverHandle ) );
    ASSERT_RESET_ALWAYS( iIscMultiplexerInterface, "IscDriver", EIscMultiplexerNotFound );

    //TInt secondDriverHandle( KErrNone );  // API change in SOS9.2 WK08
	TFindHandle secondDriverHandle;
    // Find pointer to Data Transmission Plugin.
    err = lDevices->FindByName( secondDriverHandle, KIscDataTransmissionDriverName, driverName );
    if( KErrNone != err )
        {
        C_TRACE( ( _T( "DIscDevice::InitializeLdd2LddInterface() Data Transmission Plug-In Not Found!" ) ) );
        ASSERT_RESET_ALWAYS( 0, "IscDriver", EIscDataTransmissionDriverNotFound );
        }

    iIscDataTransmissionInterface = static_cast<DIscDataTransmissionBase*>( lDevices->At( secondDriverHandle ) );
    ASSERT_RESET_ALWAYS( iIscDataTransmissionInterface, "IscDriver", EIscDataTransmissionDriverNotFound );
    
    iIscDataTransmissionInterface->Connect( this );
    iIscMultiplexerInterface->Connect( this );

    C_TRACE( ( _T( "DIscDevice::InitializeLdd2LddInterface - return 0x%x" ), err ) );
    return err;
    
    }

// -----------------------------------------------------------------------------
// DIscDevice::QueueFrame
// Queue frames that will be sent to Domestic OS
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscDevice::QueueFrame( 
    const TUint16 aChannelId,
    const TDesC8* aFrame,
    const TAny* aChannelPtr,
    TAny* aFrameInfo )
    {
    C_TRACE( ( _T( "DIscDevice::QueueFrame(0x%x, 0x%x, 0x%x, 0x%x)" ), aChannelId, aFrame, aChannelPtr, aFrameInfo ) );
    
    TInt error = KErrNone;
    
    // control channel frame ( highest priority )
    if ( aChannelId == KIscControlChannel )
        {
        C_TRACE( ( _T( "DIscDevice::QueueFrame control frame queue" ) ) );
        //add to control frame queue
        error = iControlSendQueue->Add( ( TDes8* )aFrame, aChannelId, ( DIscChannel* )aChannelPtr, aFrameInfo );
        }
    else
        {        
        C_TRACE( ( _T( "DIscDevice::QueueFrame send queue" ) ) );
        // add to send queue
        error = iSendQueue->Add( ( TDes8* )aFrame, aChannelId, ( DIscChannel* )aChannelPtr, aFrameInfo );
        }
    
    C_TRACE( ( _T( "DIscDevice::QueueFrame - return 0x%x" ), error ) );
    return error;
    }


// -----------------------------------------------------------------------------
// DIscDevice::CancelSending
// Cancels sending of frames to Domestic OS
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscDevice::CancelSending( const TUint16 aChannelId, const TAny* aChannelPtr )
    {
    C_TRACE( ( _T( "DIscDevice::CancelSending - Caller is channel: %d (0x%x)" ), aChannelId, aChannelPtr ) );
    
    TInt error( KErrNotFound );
    TInt irqLevel( 0 );

    TInt counterA( 0 );
    TInt counterB( 0 );

    TIscSendFrameInfo* temp = NULL;
    TDes8* frame = NULL;

    irqLevel = DisableIrqs();

    if ( KIscControlChannel == aChannelId )
        {
        // empty control send queue
        while ( !iControlSendQueue->Empty() )
            {
            temp = iControlSendQueue->GetFirstFrameInfo();
            frame = ( TDes8* )iControlSendQueue->RemoveFirst();
            if ( temp && frame )
                {
                if ( temp->iChannelId == aChannelId && 
                     temp->iChannelPtr == ( DIscChannel* )aChannelPtr )
                    {
                    // sender found, no need to store the frame
                    counterB++;
                    }
                else
                    {
                    iTempQueue->Add( frame, temp->iChannelId, temp->iChannelPtr, temp->iFrameInfo );
                    }
                }
            else
                {
                // should never came here
                TRACE_ASSERT_ALWAYS;
                }
            counterA++;
            temp = NULL;
            frame = NULL;
            }  
        
        while ( !iTempQueue->Empty() )
            {
            temp = iTempQueue->GetFirstFrameInfo();
            frame = ( TDes8* )iTempQueue->RemoveFirst();
            if ( temp && frame )
                {
                iControlSendQueue->Add( frame, temp->iChannelId, temp->iChannelPtr, temp->iFrameInfo );
                }
            else
                {
                // should never came here
                TRACE_ASSERT_ALWAYS;
                }
            temp = NULL;
            frame = NULL;
            }
        }
    else
        {
        // empty normal send queue
        while ( !iSendQueue->Empty() )
            {
            temp = iSendQueue->GetFirstFrameInfo();
            frame = ( TDes8* )iSendQueue->RemoveFirst();
            if ( temp && frame )
                {
                if ( temp->iChannelId == aChannelId  && 
                     temp->iChannelPtr == ( DIscChannel* )aChannelPtr )            
                    {
                    // sender found, no need to store frame
                    counterB++;                
                    }
                else
                    {
                    iTempQueue->Add( frame, temp->iChannelId, temp->iChannelPtr, temp->iFrameInfo );
                    }
                }
            else
                {
                // should never came here
                TRACE_ASSERT_ALWAYS;
                }
            counterA++;
            temp = NULL;
            frame = NULL;
            }  

        while ( !iTempQueue->Empty() )
            {
            temp = iTempQueue->GetFirstFrameInfo();
            frame = ( TDes8* )iTempQueue->RemoveFirst();
            if ( temp && frame )
                {
                iSendQueue->Add( frame, temp->iChannelId, temp->iChannelPtr, temp->iFrameInfo );
                }
            else
                {
                // should never came here
                TRACE_ASSERT_ALWAYS;
                }

            temp = NULL;
            frame = NULL;
            }
        }
        
    RestoreIrqs( irqLevel );

    C_TRACE( ( _T( "DIscDevice::CancelSending() - Frames in queue: Before: %d, After: %d" ), counterA, ( counterA-counterB ) ) );
    C_TRACE( ( _T( "DIscDevice::CancelSending() - So channel 0x%x 0x%x had %d pending messages!" ), aChannelId, aChannelPtr, counterB ) );

    // if there weren't any frames that were cancelled return KErrNotFound, otherwise return KErrNone
    if ( counterB > 0 )
        {        
        error = KErrNone;
        }
    
    return error;

    }
 
// -----------------------------------------------------------------------------
// DIscDevice::FlushQueues
// Adds Dfc to empty queues
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void DIscDevice::FlushQueues()
    {
    C_TRACE( ( _T( "DIscDevice::FlushQueues()" ) ) );
 
    if ( NKern::CurrentContext() == NKern::EInterrupt )
		{
		iSendDfc->Add();
		}
	else
		{
	    iSendDfc->Enque();
	    }

    }

// -----------------------------------------------------------------------------
// DIscDevice::CompleteRequest
// Function to complete user side asynchronous request
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
// This method has been modified to allow channel sharing between application.
// The completion routine uses directly a pointer on a DLogicalChannel instead of a channel index
//
EXPORT_C void DIscDevice::CompleteRequest( 
    TUint16 aOperation, 
    TInt aCompleteStatus,
    const TAny* aChannelPtr )
    {
    C_TRACE( ( _T( "DIscDevice::CompleteRequest(0x%x, 0x%x, 0x%x)" ), aOperation, aCompleteStatus, aChannelPtr ) ); 

    DIscChannel* tempPtr = ( DIscChannel* )aChannelPtr;
    TInt error = IscChannelContainer::ValidateChannel( tempPtr );
    if( error == KErrNone )
    	{
    	tempPtr->CompleteRequest( aOperation, aCompleteStatus );
    	}	

    C_TRACE( ( _T( "DIscDevice::CompleteRequest - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscDevice::CopyFromUserBuffer
// 
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscDevice::CopyFromUserBuffer( 
    const TDesC8& aUserBuffer, 
    TDes8& aKernelBuffer, 
    const TAny* aChannelPtr,
    const TInt aOffset )
    {
    C_TRACE( ( _T( "DIscDevice::CopyFromUserBuffer(0x%x, 0x%x, 0x%x, 0x%x)" ), &aUserBuffer, &aKernelBuffer, aChannelPtr, aOffset ) );

    // Check if channel pointer is valid.
    DIscChannel* tempPtr = ( DIscChannel* )aChannelPtr;
    TInt error = IscChannelContainer::ValidateChannel( tempPtr );
    if( error == KErrNone )
    	{
    	error = tempPtr->CopyFromUserBuffer( aUserBuffer, aKernelBuffer, aOffset );	
    	}
    C_TRACE( ( _T( "DIscDevice::CopyFromUserBuffer - return %d" ), error ) );	
   	return error;
    
    }

// -----------------------------------------------------------------------------
// DIscDevice::CopyToUserBuffer
// 
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt DIscDevice::CopyToUserBuffer( 
    TAny* aUserBuffer, 
    const TDesC8& aKernelBuffer, 
    const TAny* aChannelPtr,
    const TInt aOffset )
    {
    C_TRACE( ( _T( "DIscDevice::CopyToUserBuffer(0x%x, 0x%x, 0x%x)" ), aUserBuffer, &aKernelBuffer, aChannelPtr ) );
    
    DIscChannel* tempPtr = ( DIscChannel* )aChannelPtr;
    TInt error = IscChannelContainer::ValidateChannel( tempPtr );
    if( KErrNone == error )
    	{
    	error = tempPtr->ThreadWrite( aUserBuffer, &aKernelBuffer, aOffset );
    	}
    C_TRACE( ( _T( "DIscDevice::CopyToUserBuffer - return %d" ), error ) );	
   	return error;

    }

// -----------------------------------------------------------------------------
// DIscMultiplexerBase::GetThreadPtr
// Returns user side thread. Ownership is not given.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C DThread* DIscDevice::GetThreadPtr( const TAny* aChannelPtr )
	{
	C_TRACE( ( _T( "DIscMultiplexerBase::GetThreadPtr(0x%x)" ), aChannelPtr ) );
    DIscChannel* tempPtr = ( DIscChannel* )aChannelPtr;
    TInt error = IscChannelContainer::ValidateChannel( tempPtr );
	DThread* tmp = NULL;
	if( KErrNone == error )
		{
		tmp = tempPtr->GetDThread();	
		}
	else
		{
		C_TRACE( ( _T( "DIscMultiplexerBase::GetThreadPtr failed return NULL" ) ) );	    				
		}
	C_TRACE( ( _T( "DIscMultiplexerBase::GetThreadPtr return 0x%x" ), tmp ) );	    	
   	return tmp;
   		
	}
    
// -----------------------------------------------------------------------------
// DIscDevice::Flush
// Dfc to empty control channel and other send queues
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscDevice::Flush( TAny* aPtr )
    {
    C_TRACE( ( _T( "DIscDevice::Flush(0x%x)" ), aPtr ) );
    DIscDevice* device = ( DIscDevice* )aPtr;

    TDes8* frame = NULL;
    TIscSendFrameInfo* temp = NULL;

    TInt irqLevel(0);

    // If transmission is asynchronous and there can't be
    // several requests at the same time
    if ( !iIscDataTransmissionInterface->IsWritePending() )
        {
        irqLevel = DisableIrqs();
        if ( !iControlSendQueue->Empty() )
            {
            temp = iControlSendQueue->GetFirstFrameInfo();
            frame = ( TDes8* )iControlSendQueue->RemoveFirst();
            }
        else
            {
            temp = iSendQueue->GetFirstFrameInfo();
            frame = ( TDes8* )iSendQueue->RemoveFirst();
            }
        RestoreIrqs( irqLevel );
        C_TRACE( ( _T( "DIscDevice::Flush after RESTOREIRQS" ) ) );    
        if ( frame )    
            iIscDataTransmissionInterface->SendFrame( *frame, device->iSendDfc, temp->iFrameInfo );
        }
    C_TRACE( ( _T( "DIscDevice::Flush - return 0x0" ) ) );

    }


// -----------------------------------------------------------------------------
// DIscDevice::ConnectionStatus
// Function to tell current status of connection to Domestic OS
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscDevice::ConnectionStatus()
    {
    return iConnectionStatus;
    }

// -----------------------------------------------------------------------------
// DIscDevice::DisableIrqs
// Function to disable interrupts
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscDevice::DisableIrqs()
    {
#ifndef __WINS__
    return NKern::DisableInterrupts( KIscInterruptLevelTwo );
#else //__WINS__
    EnterCriticalSection( &g_IscDTBCriticalSection );
    return KErrNone;
#endif//__WINS__
    }

// -----------------------------------------------------------------------------
// DIscDevice::RestoreIrqs
// Function to restore interrupts
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
#ifndef __WINS__
void DIscDevice::RestoreIrqs( 
    TInt aLevel )
    {
    NKern::RestoreInterrupts( aLevel );
    
#else //__WINS__
void DIscDevice::RestoreIrqs( 
    TInt )
	{
    LeaveCriticalSection( &g_IscDTBCriticalSection );
#endif//__WINS__
    }


// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// E32Dll
// Epoc Kernel Architecture 2 style entry point
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
DECLARE_STANDARD_LDD()
    {
    DLogicalDevice* device = new DIscDevice;
    if ( !device )
        {
        ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscPanicCreateLogicalDevice );
        }
    return device;
    }

//  End of File  
