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
#include <kern_priv.h>
#include <IscDataTransmissionBase.h>
#include "IscChannel.h"
#include "IscDevice.h"
#include "IscChannelContainer.h"
#include "IscQueue.h"
#include "IscTrace.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS
_LIT( KIscDriver, "IscDriver" );

// MACROS

// LOCAL CONSTANTS AND MACROS
const TInt KIscInitializeDfcPriority( 1 );
const TInt KOneParam( 1 );
const TInt KTwoParams( 2 );
const TInt KThreeParams( 3 );
const TInt KFirstParam( 0 );
const TInt KSecondParam( 1 );
const TInt KThirdParam( 2 );
const TInt KMultiplyByOne( KSecondParam );
const TInt KMultiplyByThree( KThreeParams );
const TInt KDivideByFour( 4 );

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// DIscChannel::DIscChannel
// C++ default constructor.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C DIscChannel::DIscChannel( DLogicalDevice* aDevice ) 
    : iInitializeDfc( NULL ),
    iDataTransmissionIniData( NULL ),
    iMultiplexerIniData( NULL ),
    iMultiplexerBuffer( NULL ),
    iDataTransmissionBuffer( NULL ),
    iIscDevice( NULL ),
    iIscConnectionStatusPtr( NULL ),
    iIscFlowControlStatusPtr( NULL ),
    iReceiveBufPtr( NULL ),
    iDataReceiveBufPtr( NULL ),
    iNeededBufLen( NULL ),
    iNeededDataBufLen( NULL ),
    iChannelNumber( 0 ),
    iChannelOpen( EFalse ),
    iFrameRx( NULL ),
    iFrameRxQueue( NULL ),    
    iDataFrameRx( NULL ),
    iDataFrameRxQueue( NULL ),
    iULFlowControlStatus( EIscFlowControlOff ),
    iDLFlowControlStatus( EIscFlowControlOff ),
    iLastNotifiedULFlowstatus( EIscFlowControlOff ),
    iIscChannelHighWaterMark( 0 ),
    iIscChannelLowWaterMark( 0 ),
    iOverFlow( EFalse ),
    iClientPanic( EFalse ),
    iDataTransmissionErrorCode( KErrNone )
    {
    
    iIscDevice = ( DIscDevice * )aDevice;
    for ( TInt i( KErrNone ); i < EIscAsyncLast; i++ )
        {
        iIscRequests[i] = NULL;
        }
    iThread = &Kern::CurrentThread();   
    TInt r = iThread->Open();
    TRACE_ASSERT( r == KErrNone );

    }


// -----------------------------------------------------------------------------
// DIscChannel::~DIscChannel
// Destructor
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
DIscChannel::~DIscChannel()
    {
    C_TRACE( ( _T( "DIscChannel::~DIscChannel()" ) ) );
    
    if ( iChannelNumber < KIscNumberOfUnits )
        {
        IscChannelContainer::RemoveChannel( this );
        }
    else 
        {
        C_TRACE( ( _T( "DIscChannel::~DIscChannel() re-open" ) ) );
        }

    ChannelDestruction();    
    Kern::SafeClose( ( DObject*& )iThread, NULL );
    C_TRACE( ( _T( "DIscChannel::~DIscChannel() SafeClose called" ) ) );

    }

// -----------------------------------------------------------------------------
// DIscChannel::ChannelDestruction
// Destructor
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::ChannelDestruction()
    {
    C_TRACE( ( _T( "DIscChannel::ChannelDestruction() iChannelNumber (0x%x)" ),iChannelNumber ) );

    // call DIscMultiplexerBase::CloseDLC and DLFlowControlNotify in case the channel has not been
    // properly closed ( e.g. client thread panic etc. )
    if ( iChannelOpen )
        {
        iIscDevice->CancelSending( iChannelNumber, this );// Delete pending send frames
        iIscDevice->iIscMultiplexerInterface->CloseDLC( iChannelNumber, this );
        iIscDevice->DLFlowControlNotify( EIscFlowControlOff, iChannelNumber, this );
        }

    if ( iFrameRxQueue )
        {
        while ( !iFrameRxQueue->Empty() )
            {
            TDes8* tempPtr = ( TDes8* ) iFrameRxQueue->GetFirst();
            iFrameRxQueue->DeleteFirst();
            iIscDevice->ReleaseMemoryBlock( tempPtr );
            }
        delete iFrameRxQueue;
        iFrameRxQueue = NULL;
        }

    if ( iFrameRx )
            {
            delete [] iFrameRx;
            iFrameRx = NULL;
            }

    if ( iDataFrameRxQueue )
        {
        while ( !iDataFrameRxQueue->Empty() )
            {
            TDes8* tempPtr = ( TDes8* ) iDataFrameRxQueue->GetFirst();
            iDataFrameRxQueue->DeleteFirst();
            iIscDevice->ReleaseMemoryBlock( tempPtr );        
            }
        delete iDataFrameRxQueue;
        iDataFrameRxQueue = NULL;        
        }

    if ( iDataFrameRx )
            {
            delete [] iDataFrameRx;
            iDataFrameRx = NULL;
            }
    
    if ( iInitializeDfc )
        {        
        delete iInitializeDfc;
        iInitializeDfc = NULL;
        }    

    if ( iDataTransmissionIniData )
        {
        Kern::Free( iDataTransmissionBuffer );
        delete iDataTransmissionIniData;
        iDataTransmissionIniData = NULL;
        }

    if ( iMultiplexerIniData )
        {
        Kern::Free( iMultiplexerBuffer );
        delete iMultiplexerIniData;
        iMultiplexerIniData = NULL;
        }

    C_TRACE( ( _T( "DIscChannel::ChannelDestruction - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscChannel::HandleMsg
// Message handling ( kernel server context ).
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::HandleMsg( 
    TMessageBase* aMsg )
    {
    C_TRACE( ( _T( "DIscChannel::HandleMsg(0x%x)" ), aMsg ) );
    TThreadMessage& m=*( TThreadMessage* )aMsg;
    TInt id( m.iValue );

    if ( id==( TInt )ECloseMsg )
        {
        C_TRACE( ( _T( "DIscChannel::HandleMsg ECloseMsg" ) ) );
        m.Complete( KErrNone,EFalse );
        return;
        }

    else if ( id==KMaxTInt )
        {
        // DoCancel
        // Should not come here ever
        ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscNotAllowedCallToDoCancel );
        }

    else if ( id<0 )
        {
        // DoRequest
        // should not come here ever
        ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscNotAllowedCallToDoRequest );
        }
    
    else
        {
        // DoControl
        TUint32 a1[ KThreeParams ];
        TInt r( KErrNone );
        if ( id != EIscSyncClose )
            {
            TInt amountOfParams( KErrNone );
            switch( id )
                {
                case EIscAsyncInitializeModemInterface:
                case EIscAsyncOpen:
                    amountOfParams = KThreeParams;
                    break;
                default:
                    ASSERT_RESET_ALWAYS( 0, "NokiaISCDriver", EIscUnknownCommand );
                break;
                }
            r = Kern::ThreadRawRead( iThread, ( TAny* )m.Ptr0(), a1,
                                     amountOfParams * sizeof( TAny* ) );
            TRACE_ASSERT( r == KErrNone );
            }
        if( r == KErrNone )
            {
            r=HandleRequest( id,a1,m.Ptr1() );
            }

        m.Complete( r,ETrue );
        }
    }

// -----------------------------------------------------------------------------
// DIscChannel::Request
// Message handling ( user thread context ).
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
#ifndef COMPONENT_TRACE_FLAG
TInt DIscChannel::Request( TInt aReqNo, TAny* a1, TAny* )
#else 
TInt DIscChannel::Request( TInt aReqNo, TAny* a1, TAny* a2)
#endif
    {
    C_TRACE( ( _T( "DIscChannel::Request(0x%x, 0x%x, 0x%x)" ), aReqNo, a1, a2 ) );

    TInt r( KErrNotFound );

    if ( aReqNo<( TInt )EMinRequestId )
        C_TRACE( ( _T( "DIscChannel::Request ERROR: False aReqNo %d" ), aReqNo ) );
    
    if ( aReqNo >= 0 && aReqNo < EIscAsyncLastKernelServerContext ||
         aReqNo >= EIscAsyncLast && aReqNo < EIscSyncLastKernelServerContext )
        {
        TThreadMessage& m=Kern::Message();
        m.iValue=aReqNo;
        m.iArg[ KFirstParam ]=a1;
        m.iArg[ KSecondParam ]=NULL;
        r = m.SendReceive( &iMsgQ );
        }
    else
        {
        TInt ulen( KErrNotFound );
        switch ( aReqNo )
            {
            case EIscCancelAsyncInitialize:
            case EIscCancelAsyncReceive:
            case EIscCancelAsyncDataReceive:
            case EIscSyncGetConnectionStatus:
            case EIscCancelAsyncNotifyConnection:
            case EIscSyncGetFlowControlStatus:
            case EIscCancelAsyncNotifyFlowControl:
            case EIscSyncGetMaximunDataSize:
            case EIscCancelAsyncCustomOperation1:
            case EIscCancelAsyncCustomOperation2:
            case EIscCancelAsyncCustomOperation3:
            case EIscCancelAsyncCustomOperation4:
            case EIscCancelAsyncCustomOperation5:
            case EIscCancelAsyncOpen:
            case EIscSyncResetBuffers:
            case EIscCancelAsyncSend:
            case EIscCancelAsyncDataSend:
                {
                ulen = KErrNone;
                break;
                }
            case EIscSyncSend:
            case EIscSyncDataSend:
            case EIscSyncCustomOperation1:
            case EIscSyncCustomOperation2:
            case EIscSyncCustomOperation3:
            case EIscSyncCustomOperation4:
            case EIscSyncCustomOperation5:
            case EIscAsyncClose:
                {
                ulen = KOneParam;
                break;
                }
            case EIscAsyncSend:
            case EIscAsyncDataSend:
            case EIscSyncGetChannelInfo:
            case EIscAsyncNotifyConnectionStatus:
            case EIscAsyncNotifyFlowControlStatus:
            case EIscAsyncCustomOperation1:
            case EIscAsyncCustomOperation2:
            case EIscAsyncCustomOperation3:
            case EIscAsyncCustomOperation4:
            case EIscAsyncCustomOperation5:
                {
                ulen = KTwoParams;
                break;
                }
            case EIscAsyncReceive:
            case EIscAsyncDataReceive:
                {
                ulen = KThreeParams;
                break;
                }
            default:
                {
                TRACE_ASSERT_ALWAYS;
                }
            }
        ASSERT_RESET_ALWAYS( KErrNotFound != ulen, "ISCDriver", EIscUnknownCommand );
        // Maximum number of elements is three!!!
        TAny* kptr[ KThreeParams ] = { KErrNone }; 
        if( ulen > KErrNone )
            {
            C_TRACE( ( _T( "DIscChannel::Request ISC kumemget32" ) ) );
            kumemget32( kptr , a1, ( sizeof( TAny* ) ) * ulen ); 
            }
        r = HandleRequest( aReqNo, kptr, NULL );
        }
    C_TRACE( ( _T( "DIscChannel::Request ISC return %d CH 0x%x" ), r, iChannelNumber ) );
    return r;        

    }

// -----------------------------------------------------------------------------
// DIscChannel::DoControl
// Handles requests.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscChannel::HandleRequest( 
    TInt aFunction, 
    TAny* a1, 
    TAny* /*a2*/ )
    {
    C_TRACE( ( _T( "DIscChannel::DoControl(0x%x, 0x%x) iChannelNumber 0x%x channelPtr 0x%x" ), aFunction, a1, iChannelNumber, this ) );

    TInt error( KErrNone );
    
#ifdef _DEBUG
    // Check if control frame buffer overflow -> panic
    if ( iClientPanic )
        {
        C_TRACE( ( _T( "DIscChannel::DoControl() BUFFER OVERFLOW: PANIC CLIENT 0x%x" ), iChannelNumber ) );
        TRACE_ASSERT_ALWAYS;
        // This panic the user thread only.
		Kern::ThreadKill( iThread, EExitPanic, EIscControlBufferOverflow, KIscDriver );
        }
#endif // _DEBUG

    // Handle asynchronous requests
    if ( aFunction >= EIscAsyncInitializeModemInterface &&
         aFunction < EIscAsyncLast )
        {
        // if request is already active
        if ( iIscRequests[aFunction] )
            {
            TUint32* tablePtr = ( TUint32* )a1;
            TRequestStatus* requestStatus = ( TRequestStatus* )( tablePtr[ KFirstParam ] );
            Kern::RequestComplete( iThread, requestStatus, KErrAlreadyExists );
            }
        else
            {
            HandleAsyncRequest( aFunction, a1 );
            }
        }
    // Handle synchronous requests.
    else if ( aFunction >= EIscAsyncLast &&
              aFunction < EIscSyncLast )
        {
        error = HandleSyncRequest( aFunction, a1 );
        }
    // Handle cancellation requests.
    else if ( aFunction >= EIscSyncLast &&
              aFunction < EIscCancelLast )
        {
        error = HandleCancelRequest( aFunction, a1 );
        }
    // Undefined request, panic current thread. 
    else
        {
        // This panic the user thread only.
		Kern::ThreadKill( iThread, EExitPanic, EIscControlBufferOverflow, KIscDriver );
        }
        
    C_TRACE( ( _T( "DIscChannel::DoControl - return %d" ), error ) );

    return error;
    }

   
// -----------------------------------------------------------------------------
// DIscChannel::HandleAsyncRequests
// Handles asynchronous client requests
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::HandleAsyncRequest( 
    TInt aFunction, 
    TAny* a1 )
    {
    C_TRACE( ( _T( "DIscChannel::HandleAsyncRequest(0x%x, 0x%x) channelPtr 0x%x" ), aFunction, a1, this ) );

    TUint32* tablePtr = ( TUint32* )a1;

    switch ( aFunction )
        {
        case EIscAsyncInitializeModemInterface:
            {    
            TInt r = KErrNotFound;
            iIscRequests[EIscAsyncInitializeModemInterface] = ( TRequestStatus* )( tablePtr[ KFirstParam ] );
           
            iInitializeDfc = new TDfc( InitializeComplete, this, Kern::DfcQue0(), KIscInitializeDfcPriority );            
            ASSERT_RESET_ALWAYS( iInitializeDfc, "IscDriver",EIscMemoryAllocationFailure );

            // Get Data Transmission Driver initialization string
            TDesC8* tempPtr = ( TDesC8* )tablePtr[ KThirdParam ];
            iDataTransmissionBuffer = ( TUint8* )Kern::Alloc( KIscIniLineLength );
            ASSERT_RESET_ALWAYS( iDataTransmissionBuffer, "IscDriver",EIscMemoryAllocationFailure );

            iDataTransmissionIniData = new ( TPtr8 )( iDataTransmissionBuffer, KIscIniLineLength );
            ASSERT_RESET_ALWAYS( iDataTransmissionIniData, "IscDriver",EIscMemoryAllocationFailure );

            r = Kern::ThreadDesRead( iThread, tempPtr, *iDataTransmissionIniData, 0, KChunkShiftBy0 );
            ASSERT_RESET_ALWAYS( r == KErrNone, "IscDriver",EIscMemoryAllocationFailure );

            // Get Multiplexer initialization string
            tempPtr = ( TDesC8* )tablePtr[1];
            iMultiplexerBuffer = ( TUint8* )Kern::Alloc( KIscIniLineLength );
            ASSERT_RESET_ALWAYS( iMultiplexerBuffer, "IscDriver",EIscMemoryAllocationFailure );

            iMultiplexerIniData = new ( TPtr8 )( iMultiplexerBuffer, KIscIniLineLength );
            ASSERT_RESET_ALWAYS( iDataTransmissionIniData, "IscDriver",EIscMemoryAllocationFailure );

            r = Kern::ThreadDesRead( iThread, tempPtr, *iMultiplexerIniData, 0, KChunkShiftBy0 );
            ASSERT_RESET_ALWAYS( r == KErrNone, "IscDriver",EIscMemoryAllocationFailure );

            // If buffer configuration is given from isc_config.ini as multiplexer configuration string, multiplexer 
            // needs configuration before datatransmissin driver is initialized
            iIscDevice->iIscMultiplexerInterface->SetInitializationParameters( *iMultiplexerIniData );

            // Allocate buffers and receive queues
            iIscDevice->Initialize();

            iIscDevice->iIscDataTransmissionInterface->InitializeDataTransmission( *iDataTransmissionIniData, 
                                                                                   iInitializeDfc,
                                                                                   iDataTransmissionErrorCode );            
            break;
            }
            
        case EIscAsyncOpen:
            {
            iIscRequests[EIscAsyncOpen] = ( TRequestStatus* )( tablePtr[ KFirstParam ] );
            if ( iIscDevice->ConnectionStatus() == EIscConnectionOk )
                {
                TDesC8* openInfo = ( TDesC8* )tablePtr[ KThirdParam ];
                // Channel info parameter has to be copied from user side in Epoc Kernel Architecture 2
                TInt length = Kern::ThreadGetDesLength( iThread, ( TDesC8* )tablePtr[ KThirdParam ] );
                TUint8* buffer = NULL;
                TPtr8* bufferPtr = NULL;
                if ( length > KErrNone )
                    {
                    C_TRACE( ( _T( "DIscChannel::HandleAsyncRequest EIscAsyncOpen channel info got" ) ) );
                    buffer = ( TUint8* )Kern::Alloc( length );
                    bufferPtr = new ( TPtr8 )( buffer, length );
                    TInt r = Kern::ThreadDesRead( iThread, openInfo, *bufferPtr, 0, KChunkShiftBy0 );
                    ASSERT_RESET_ALWAYS( r == KErrNone, "IscDriver",EIscMemoryAllocationFailure );
                    openInfo = ( TDesC8* )bufferPtr;
                    }
                iIscDevice->iIscMultiplexerInterface->OpenDLC( ( TUint16 )tablePtr[ KSecondParam ], 
                                                               openInfo,
                                                               this );

                if ( buffer )
                    Kern::Free( buffer );
                if ( bufferPtr )
                    {
                    delete bufferPtr;
                    bufferPtr = NULL;
                    }
                }
            else
                {
                CompleteRequest( aFunction, KErrNotReady );
                }
            break;
            }
        case EIscAsyncSend:
            {
            iIscRequests[EIscAsyncSend] = ( TRequestStatus* )( tablePtr[ KFirstParam ] );
            if ( iIscDevice->ConnectionStatus() == EIscConnectionOk
                && iULFlowControlStatus == EIscFlowControlOff )
                {
                TDesC8* ptr = ( TDesC8* ) tablePtr[ KSecondParam ];

                // No return values check needed. Request completed with error value by multiplexer
                iIscDevice->iIscMultiplexerInterface->Send( ( TUint16 )aFunction,
                                                            iChannelNumber, 
                                                            *ptr,
                                                            this );
                }
            else
                {
                if ( iULFlowControlStatus != EIscFlowControlOff )
                    {
                    CompleteRequest( aFunction, KErrOverflow );
                    }
                else
                    {
                    CompleteRequest( aFunction, KErrNotReady );
                    }
                }
            break;
            }
        case EIscAsyncReceive:
            {    
            TRACE_ASSERT( tablePtr[ KFirstParam ] );
            TRACE_ASSERT( tablePtr[ KSecondParam ] );
            TRACE_ASSERT( tablePtr[ KThirdParam ] );
            
            // check for descriptor validity
            TRACE_ASSERT( Kern::ThreadGetDesMaxLength( iThread, (TPtr8* )tablePtr[KSecondParam] ) > 0 ); 
            
            //Store needed length ptr
            iNeededBufLen = ( TPtr8* )tablePtr[ KThirdParam ];
            
            //Store msg data ptr
            iReceiveBufPtr = ( TPtr8* )tablePtr[ KSecondParam ];
            
            iIscRequests[EIscAsyncReceive] = ( TRequestStatus* )( tablePtr[ KFirstParam ] );
                
            break;
            }
        case EIscAsyncDataSend:
            {
            iIscRequests[EIscAsyncDataSend] = ( TRequestStatus* )( tablePtr[ KFirstParam ] );
                
            if ( iIscDevice->ConnectionStatus() == EIscConnectionOk
                && iULFlowControlStatus == EIscFlowControlOff )
                {
                TDesC8* ptr = ( TDesC8* )tablePtr[ KSecondParam ];
                iIscDevice->iIscMultiplexerInterface->DataSend( ( TUint16 )aFunction,
                                                                iChannelNumber,
                                                                *ptr,
                                                                this );
                }
            else
                {
                if ( iULFlowControlStatus != EIscFlowControlOff )
                    {
                    CompleteRequest( aFunction, KErrOverflow );
                    }
                else
                    {
                    CompleteRequest( aFunction, KErrNotReady );
                    }
                }
            break;
            }
        case EIscAsyncDataReceive:
            {
            TRACE_ASSERT( tablePtr[ KFirstParam ] );
            TRACE_ASSERT( tablePtr[ KSecondParam ] );
            TRACE_ASSERT( tablePtr[ KThirdParam ] );

            // check for descriptor validity
            TRACE_ASSERT( Kern::ThreadGetDesMaxLength( iThread, (TPtr8* )tablePtr[KSecondParam] ) > 0 ); 
            
            //Store needed length ptr
            iNeededDataBufLen = ( TPtr8* )tablePtr[ KThirdParam ];
            
            //Store msg data ptr
            iDataReceiveBufPtr = ( TPtr8* )tablePtr[ KSecondParam ];
            
            iIscRequests[EIscAsyncDataReceive] = ( TRequestStatus* )( tablePtr[ KFirstParam ] );
                
            break;
            }
        case EIscAsyncNotifyConnectionStatus:
            {
            iIscConnectionStatusPtr = ( TPtr8* )tablePtr[ KSecondParam ];
            iIscRequests[EIscAsyncNotifyConnectionStatus] = ( TRequestStatus* )( tablePtr[ KFirstParam ] );
            break;
            }
        case EIscAsyncNotifyFlowControlStatus:
            {
            iIscFlowControlStatusPtr = reinterpret_cast<TPtr8*>( tablePtr[ KSecondParam ] );
            iIscRequests[ EIscAsyncNotifyFlowControlStatus ] = reinterpret_cast<TRequestStatus*>( tablePtr[ KFirstParam ] );
            C_TRACE( ( _T( "DIscChannel::NotifyFlowControl iLastNotifiedULFlowstatus = %d iULFlowControlStatus = %d" ), iLastNotifiedULFlowstatus, iULFlowControlStatus ) );
            
            if( iULFlowControlStatus != iLastNotifiedULFlowstatus )
                {
                // Complete immediately.
                C_TRACE( ( _T( "DIscChannel::HandleAsyncRequest iULFlowControlStatus != iLastNotifiedULFlowstatus" ) ) );
                NotifyFlowControl( iULFlowControlStatus );    
                }
            else
                {
                // None
                }
            break;
            }
        case EIscAsyncCustomOperation1:
        case EIscAsyncCustomOperation2:
        case EIscAsyncCustomOperation3:
        case EIscAsyncCustomOperation4:
        case EIscAsyncCustomOperation5:
            {
            iIscRequests[aFunction] = ( TRequestStatus* )( tablePtr[ KFirstParam ] );
            TAny* tempPtr = ( TAny* )tablePtr[ KSecondParam ];
            iIscDevice->iIscMultiplexerInterface->CustomFunction( iChannelNumber, 
                                                             ( TUint16 )aFunction, 
                                                             tempPtr,
                                                             this );
            break;
            }
        case EIscAsyncClose:
            {
            iIscRequests[aFunction] = ( TRequestStatus* )( tablePtr[ KFirstParam ] );

            ResetBuffers();
            
            // Cancel all active requests except asynchronous close
            for ( TInt i( KErrNone ); i < EIscAsyncLast; i++ )
                {
                // if request is active complete it with KErrCancel
                if ( iIscRequests[i] && i != EIscAsyncClose )
                    {
                    iIscDevice->iIscMultiplexerInterface->CancelNotify( iChannelNumber, i, this );
                    CompleteRequest( i, KErrCancel );
                    }
                }            

            iChannelOpen = EFalse;

            iIscDevice->iIscMultiplexerInterface->CloseDLC( iChannelNumber, this );
            
            break;
            }
        default:
            {
            ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscUnknownCommand );
            break;
            }
        }

    IscChannelContainer::AddDfc();
    
    }


// -----------------------------------------------------------------------------
// DIscChannel::HandleSyncRequest
// Handles synchronous client requests
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscChannel::HandleSyncRequest( 
        TInt aFunction, 
        TAny* a1 )
    {
    C_TRACE( ( _T( "DIscChannel::HandleSyncRequest(0x%x, 0x%x) channelPtr 0x%x" ), aFunction, a1, this ) );

    TInt error( KErrNone );
    TUint32* tablePtr = ( TUint32* )a1;

    switch ( aFunction )
        {
        case EIscSyncClose:
            {
            ResetBuffers();
            
            iIscDevice->iIscMultiplexerInterface->CloseDLC( iChannelNumber, this );
            
            // Cancel all active requests 
            for ( TInt i( KErrNone ); i < EIscAsyncLast; i++ )
                {
                // if request is active complete it with KErrCancel
                if ( iIscRequests[i] )
                    {
                    iIscDevice->iIscMultiplexerInterface->CancelNotify( iChannelNumber, i, this );
                    CompleteRequest( i, KErrCancel );
                    }
                }

            error = KErrNone;
            iChannelOpen = EFalse;
            break;
            }
        case EIscSyncSend:
            {
            if ( iIscDevice->ConnectionStatus() == EIscConnectionOk 
                && iULFlowControlStatus == EIscFlowControlOff ) 
                {
                
                TDesC8* ptr = ( TDesC8* ) tablePtr[ KFirstParam ];
                error = iIscDevice->iIscMultiplexerInterface->Send( ( TUint16 )aFunction,
                                                            iChannelNumber, 
                                                            *ptr,
                                                            this );
                }
            else
                {
                if ( iULFlowControlStatus != EIscFlowControlOff )
                    {
                    error = KErrOverflow;
                    }
                else
                    {
                    error = KErrNotReady;
                    }
                }
            break;
            }
        case EIscSyncDataSend:
            {
            if ( iIscDevice->ConnectionStatus() == EIscConnectionOk 
                && iULFlowControlStatus == EIscFlowControlOff ) 
                {
                TDesC8* ptr = ( TDesC8* ) tablePtr[ KFirstParam ];
                error = iIscDevice->iIscMultiplexerInterface->DataSend( ( TUint16 )aFunction,
                                                                iChannelNumber,
                                                                *ptr,
                                                                this );
                }
            else
                {
                if ( iULFlowControlStatus != EIscFlowControlOff )
                    {
                    error = KErrOverflow;
                    }
                else
                    {
                    error = KErrNotReady;
                    }
                }
            break;
            }
        case EIscSyncGetConnectionStatus:
            {
            error = iIscDevice->ConnectionStatus();
            break;
            }
        case EIscSyncGetFlowControlStatus:
            {
            error = iULFlowControlStatus;
            break;
            }
        case EIscSyncGetChannelInfo:
            {
            TDes8* tempPtr = ( TDes8* ) tablePtr[1];
            error = iIscDevice->iIscMultiplexerInterface->GetChannelInfo( ( TUint16 )tablePtr[ KFirstParam ],
                                                                  *tempPtr );
            break;
            }
        case EIscSyncGetMaximunDataSize:
            {
            error = iIscDevice->iIscMultiplexerInterface->MaximumDataSize( iChannelNumber );
            break;
            }
        case EIscSyncCustomOperation1:
        case EIscSyncCustomOperation2:
        case EIscSyncCustomOperation3:
        case EIscSyncCustomOperation4:
        case EIscSyncCustomOperation5:
            {
            TAny* tempPtr = ( TAny* )tablePtr[ KFirstParam ];
            error = iIscDevice->iIscMultiplexerInterface->CustomFunction( iChannelNumber,
                                                                     ( TUint16 )aFunction, 
                                                                     tempPtr,
                                                                     this );
            break;
            }
        case EIscSyncResetBuffers:
            {
            ResetBuffers();
            error = KErrNone;
            break;
            }
        default:
            {
            ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscUnknownCommand );
            break;
            }
        }
    return error;
    }

// -----------------------------------------------------------------------------
// DIscChannel::HandleCancelRequest
// Cancels active request
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscChannel::HandleCancelRequest( 
    TInt aFunction, 
    TAny* /*a1*/ )
    {
    C_TRACE( ( _T( "DIscChannel::HandleCancelRequest(0x%x)" ), aFunction ) );

    TUint16 operationToCancel( 0 );
    switch ( aFunction )
        {
        
        case EIscCancelAsyncInitialize:
            {
            if ( iDataTransmissionIniData )
                {
                Kern::Free( iDataTransmissionBuffer );
                delete iDataTransmissionIniData;
                iDataTransmissionIniData = NULL;
                }
            if ( iMultiplexerIniData )
                {
                Kern::Free( iMultiplexerBuffer );
                delete iMultiplexerIniData;
                iMultiplexerIniData = NULL; 
                }

            operationToCancel = EIscAsyncInitializeModemInterface;
            break;
            }
        case EIscCancelAsyncOpen:
            {
            if ( KRequestPending == IsPending( EIscAsyncOpen ) )
                {
                iChannelOpen = EFalse;
                }            
            operationToCancel = EIscAsyncOpen;
            break;
            }
        case EIscCancelAsyncSend:
            {
            operationToCancel = EIscAsyncSend;
            // Cancel sending / empty send queues
            iIscDevice->CancelSending( iChannelNumber, this );
            break;
            }
        case EIscCancelAsyncDataSend:
            {
            operationToCancel = EIscAsyncDataSend;
            // Cancel sending / empty send queues
            iIscDevice->CancelSending( iChannelNumber, this );
            break;
            }
        
        case EIscCancelAsyncReceive:
            {
            iReceiveBufPtr = NULL;
            iNeededBufLen = NULL;
            operationToCancel = EIscAsyncReceive;
            break;
            }
        
        case EIscCancelAsyncDataReceive:
            {
            iDataReceiveBufPtr = NULL;
            iNeededDataBufLen = NULL;
            operationToCancel = EIscAsyncDataReceive;
            break;
            }
        
        case EIscCancelAsyncNotifyConnection:
            {
            iIscConnectionStatusPtr = NULL;
            operationToCancel = EIscAsyncNotifyConnectionStatus;
            break;
            }
        
        case EIscCancelAsyncNotifyFlowControl:
            {
            iIscFlowControlStatusPtr = NULL;
            operationToCancel = EIscAsyncNotifyFlowControlStatus;
            break;
            }
        case EIscCancelAsyncCustomOperation1:
            {
            operationToCancel = EIscAsyncCustomOperation1;
            break;
            }
        case EIscCancelAsyncCustomOperation2:
            {
            operationToCancel = EIscAsyncCustomOperation2;
            break;
            }
        case EIscCancelAsyncCustomOperation3:
            {
            operationToCancel = EIscAsyncCustomOperation3;
            break;
            }
        case EIscCancelAsyncCustomOperation4:
            {
            operationToCancel = EIscAsyncCustomOperation4;
            break;
            }
        case EIscCancelAsyncCustomOperation5:
            {
            operationToCancel = EIscAsyncCustomOperation5;
            break;
            }
        case EIscCancelAsyncClose:
            {
            TRACE_ASSERT_ALWAYS;
            operationToCancel = EIscAsyncClose;
            break;
            }
        default:
            {
            ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscUnknownCommand );
            break;
            }
        }

    iIscDevice->iIscMultiplexerInterface->CancelNotify( iChannelNumber, ( TUint16 )operationToCancel, this );
    CompleteRequest( operationToCancel, KErrCancel );

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// DIscChannel::DoCreate
// Secondary initialization of channel
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscChannel::DoCreate( 
    TInt aUnit, 
    const TDesC8* anInfo, 
    const TVersion& /*aVer*/ )
    {
    C_TRACE( ( _T( "DIscChannel::DoCreate(0x%x, 0x%x, 0x%x)" ), aUnit, anInfo, this ) );
    if ( !Kern::CurrentThreadHasCapability( ECapabilityCommDD,__PLATSEC_DIAGNOSTIC_STRING
        ( "Checked by ISCDRIVER.LDD ( Inter-System Communication Driver )" ) ) )
        {
        return KErrPermissionDenied;
        }    
    TUint16 channelNumber = ( TUint16 )aUnit;
    if ( anInfo )
        {
        // check for channel number inside anInfo
        TUint8 channel = ( TUint8 )( *anInfo )[0];
        if ( channel >= KIscMaxChannelsInLdd )
            {
            channelNumber += KIscMaxChannelsInLdd;
            }
        C_TRACE( ( _T( "DIscChannel::DoCreate channel=0x%x" ), channelNumber ) );
        }

    if ( channelNumber != KIscControlChannel )
        {
        TIscConfiguration config;
        iIscDevice->iIscMultiplexerInterface->GetConfiguration( config );
        TInt i( KErrNone );
        iFrameRx = new TUint32*[config.channelRcvQueueSize];
        ASSERT_RESET_ALWAYS( iFrameRx, "IscDriver",EIscMemoryAllocationFailure );
        for ( i = KErrNone; i < config.channelRcvQueueSize; i++ )
            {
            iFrameRx[i] = 0;
            }

        iDataFrameRx = new TUint32*[config.channelDataRcvQueueSize];
        ASSERT_RESET_ALWAYS( iDataFrameRx, "IscDriver",EIscMemoryAllocationFailure );
        for ( i = KErrNone; i < config.channelDataRcvQueueSize; i++ )
            {
            iDataFrameRx[i] = 0;
            }

        // creating frame queue for incoming frames
        iFrameRxQueue = new  DIscQueue( iFrameRx, config.channelRcvQueueSize );
        ASSERT_RESET_ALWAYS( iFrameRxQueue, "IscDriver",EIscMemoryAllocationFailure );

        // creating frame queue for incoming data frames
        iDataFrameRxQueue = new  DIscQueue( iDataFrameRx, config.channelDataRcvQueueSize );
        ASSERT_RESET_ALWAYS( iDataFrameRxQueue, "IscDriver",EIscMemoryAllocationFailure );
    
        // Flowcontrol marks for data frames
        iIscChannelHighWaterMark = ( TUint16 )( ( ( ( TUint16 )config.channelDataRcvQueueSize ) * KMultiplyByThree ) / KDivideByFour );// 75% = multiply with 3, divide by 4 
        iIscChannelLowWaterMark =  ( TUint16 )( ( ( ( TUint16 )config.channelDataRcvQueueSize ) * KMultiplyByOne ) / KDivideByFour );// 25% = multiply with 1, divide by 4 
    
        TRACE_ASSERT( iIscChannelHighWaterMark != 0 );
        }

#ifndef ISC_CHANNEL_SHARING_IN_USE
    // Remove checking if channel already set to enable channel sharing
    //Check if channel already set
    if ( IscChannelContainer::Channel( channelNumber, 0 ) )
        {
        C_TRACE( ( _T( "DIscChannel::DoCreate channel 0x%x already set!!!!" ), channelNumber ) );
        return KErrAlreadyExists;
        }
#endif //ISC_CHANNEL_SHARING_IN_USE

    //Add itself to channel table.
    TInt error = IscChannelContainer::SetChannel( ( DIscChannel* )this, channelNumber );
    if ( KErrNone != error )
        {
        return error;
        }

    iChannelOpen = ETrue;
    
    // Store channel number.
    iChannelNumber = channelNumber;    
    SetDfcQ( Kern::DfcQue0() );
    iMsgQ.Receive();
    
    return KErrNone;

    }


// -----------------------------------------------------------------------------
// DIscChannel::NotifyFlowControl
// Notify user side client that uplink flow control is on/off
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::NotifyFlowControl
        ( 
        const TInt aFlowControlStatus
        )
    {
    C_TRACE( ( _T( "DIscChannel::NotifyFlowControl(0x%x) iChannelNumber 0x%x channelPtr 0x%x" ), aFlowControlStatus, iChannelNumber, this ) );

    iULFlowControlStatus = aFlowControlStatus;
    if( iIscRequests[ EIscAsyncNotifyFlowControlStatus ]
        && iIscFlowControlStatusPtr )
        {
        TPtr8 tempDes( reinterpret_cast<TUint8*>( &iULFlowControlStatus ), sizeof ( TInt ), sizeof ( TInt ) );
        iLastNotifiedULFlowstatus = aFlowControlStatus;
        TInt r = ThreadWrite( static_cast<TAny*>( iIscFlowControlStatusPtr ), &tempDes, 0 );
        TRACE_ASSERT( r == KErrNone ); 
        CompleteRequest( EIscAsyncNotifyFlowControlStatus, r );
        }
    else
        {
        C_TRACE( ( _T( "DIscChannel::NotifyFlowControl No request pending!" ) ) );        
        }
    }

// -----------------------------------------------------------------------------
// DIscChannel::NotifyConnectionStatus
// Notify user side client that connection status has changed
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::NotifyConnectionStatus( 
    const TInt aConnectionStatus ) 
    {
    C_TRACE( ( _T( "DIscChannel::NotifyConnectionStatus(0x%x) channelPtr 0x%x" ), aConnectionStatus, this ) );

    if ( iIscRequests[EIscAsyncNotifyConnectionStatus] 
         && iIscConnectionStatusPtr )
        {
        TInt temp = aConnectionStatus;
        TPtr8 tempDes( ( TUint8* )&temp,sizeof ( TInt ),sizeof ( TInt ) );
        TInt r = ThreadWrite( ( TAny* )iIscConnectionStatusPtr, &tempDes, 0 );
        TRACE_ASSERT( r == KErrNone );

        CompleteRequest( EIscAsyncNotifyConnectionStatus, r );
        }
    }

// -----------------------------------------------------------------------------
// DIscChannel::IsPending
// Check if asynchronous request is active
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscChannel::IsPending( 
    const TUint16 aReqNumber )
    {
    TInt error( KErrNone );
    if ( iIscRequests[aReqNumber] )
        {
        error = KRequestPending;
        }
    else
        {
        // error is KErrNone
        }
    return error;
    }
// -----------------------------------------------------------------------------
// DIscChannel::CompleteRequest
// Function to complete clients pending asynchronous request
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::CompleteRequest( 
    TInt aOperation, 
    TInt aCompleteStatus )
    {
    C_TRACE( ( _T( "DIscChannel::CompleteRequest(0x%x, 0x%x) iChannelNumber 0x%x channelPtr 0x%x" ), aOperation, aCompleteStatus, iChannelNumber, this ) );
  
    if ( aOperation < EIscAsyncLast )
        {
        
        TRequestStatus* requestStatus = iIscRequests[aOperation];
        if ( requestStatus )
            {        
            // In case of higher priority thread, set request to NULL from the request table before actual completing
            iIscRequests[aOperation] = NULL;
            Kern::RequestComplete( iThread, requestStatus, aCompleteStatus );
            }
        }
    else
        {
        // Do nothing
        }

    C_TRACE( ( _T( "DIscChannel::CompleteRequest - return void" ) ) );

    }

// -----------------------------------------------------------------------------
// DIscChannel::InitializeComplete
// Initialization complete dfc
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::InitializeComplete( 
    TAny* aPtr )
    {
    C_TRACE( ( _T( "DIscChannel::InitializeComplete(0x%x)" ), aPtr ) );

    DIscChannel* ThisPtr = ( DIscChannel* )aPtr;
    if ( KErrNone == ThisPtr->iDataTransmissionErrorCode )
        {        
        ThisPtr->DoMultiplexerInitialize();
        }
    else
        {
        ThisPtr->CompleteRequest( EIscAsyncInitializeModemInterface, ThisPtr->iDataTransmissionErrorCode );
        }
    C_TRACE( ( _T( "DIscChannel::InitializeComplete - return 0x%x" ) ) );

    }

// -----------------------------------------------------------------------------
// DIscChannel::DoMultiplexerInitialize
// Completes the multiplexer initialization
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::DoMultiplexerInitialize()
    {
    C_TRACE( ( _T( "DIscChannel::DoMultiplexerInitialize() channelPtr 0x%x" ), this ) );
    
    if ( iIscRequests[EIscAsyncInitializeModemInterface] )
        {
        
        iIscDevice->iIscMultiplexerInterface->InitializeMultiplexer( 
                EIscAsyncInitializeModemInterface, 
                *iMultiplexerIniData,
                this );
        }

    C_TRACE( ( _T( "DIscChannel::DoMultiplexerInitialize - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscChannel::StoreFrame
// Stores the incoming frame to channels receive queue
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::StoreFrame( TDesC8* aData )
    {
    C_TRACE( ( _T( "DIscChannel::StoreFrame(0x%x) channelId 0x%x channelPtr 0x%x" ), aData, iChannelNumber, this ) );
    
    TInt error( KErrNone );

    TIscFrameInfo frameInfo;
    iIscDevice->iIscMultiplexerInterface->GetFrameInfo( *aData, frameInfo );

    if ( frameInfo.frameType == EIscDataFrame )
        {
        C_TRACE( ( _T( "DIscChannel::StoreFrame dataFrame" ) ) );
        error = iDataFrameRxQueue->Add( ( TAny* )aData );
        if ( error == KErrNone )
            {
            if ( iDataFrameRxQueue->Count() >= iIscChannelHighWaterMark 
                && iDLFlowControlStatus == EIscFlowControlOff )
                {
                iIscDevice->DLFlowControlNotify( EIscFlowControlOn, iChannelNumber, this );
                iDLFlowControlStatus = EIscFlowControlOn;
                }
            else if ( iDataFrameRxQueue->Count() <= iIscChannelLowWaterMark 
                     && iDLFlowControlStatus != EIscFlowControlOff )
                {
                iIscDevice->DLFlowControlNotify( EIscFlowControlOff, iChannelNumber, this );
                iDLFlowControlStatus = EIscFlowControlOff;
                }
            else
                {
                // Do nothing
                }
            }
        else
            {
            // Set overflow flag on. Complete next DataReceive with KErrOverFlow
            TRACE_ASSERT_ALWAYS;
            iOverFlow = ETrue;
            iIscDevice->ReleaseMemoryBlock( ( TDes8* )aData );
            }
        }

    else
        {
        C_TRACE( ( _T( "DIscChannel::StoreFrame controlFrame" ) ) );
        error = iFrameRxQueue->Add( ( TAny* )aData );
        if ( error != KErrNone )
            {
            C_TRACE( ( _T( "DIscChannel::StoreFrame() CONTROL FRAME OVERFLOW channel %d" ), iChannelNumber ) );
            TRACE_ASSERT_ALWAYS;
            iClientPanic = ETrue;
            iIscDevice->ReleaseMemoryBlock( ( TDes8* )aData );
            }
        }

    C_TRACE( ( _T( "DIscChannel::StoreFrame - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// DIscChannel::EmptyBuffers
// Goes through channel's queue and delivers possible frame( s ) to client
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::EmptyBuffers()
    {    
    C_TRACE( ( _T( "DIscChannel::EmptyBuffers() channelId 0x%x channelPtr 0x%x" ),iChannelNumber, this ) );

    if ( iDataFrameRxQueue->NextBufferLength() > KErrNone )
        {
        TDes8* tempPtr = ( TDes8* ) iDataFrameRxQueue->GetFirst();
        TIscFrameInfo frameInfo;
        TInt desMaxLen( KErrNone );
 
        iIscDevice->iIscMultiplexerInterface->GetFrameInfo( *tempPtr, frameInfo );
        
        // frame incoming, and datareceive request active
        if ( iIscRequests[EIscAsyncDataReceive] && frameInfo.frameType == EIscDataFrame )             
            {
            if ( frameInfo.concatenation == EIscNoConcatenation )
                {
                desMaxLen = Kern::ThreadGetDesMaxLength( iThread, iDataReceiveBufPtr );                    

                C_TRACE( ( _T( "DIscChannel::EmptyBuffers() data desMaxLen %d" ),desMaxLen ) );

                TRACE_ASSERT( desMaxLen > KErrNone );
                
                // check that client's memory block is big enough
                if ( desMaxLen >= frameInfo.writeLength )
                    {
                    // create a temporary descriptor for writing since we're
                    // necessary not writing the whole contents of the 
                    // source descriptor, only the part that ISC Multiplexer
                    // wants
                    TPtr8 writePtr( ( TUint8* )( tempPtr->Ptr() + frameInfo.writeStartIndex ),
                                    frameInfo.writeLength,
                                    frameInfo.writeLength );

                    TInt r = ThreadWrite( iDataReceiveBufPtr, &writePtr, 0 );

                    TRACE_ASSERT( r == KErrNone );
                    
                    // remove the pointer from queue and release the memory block 
                    // but only if the ThreadWrite was successfull
                    if  ( r == KErrNone )
                        {
                        iDataFrameRxQueue->DeleteFirst();
                        iIscDevice->ReleaseMemoryBlock( tempPtr );
                        if ( iOverFlow )
                            {
                            iOverFlow = EFalse;
                            CompleteRequest( EIscAsyncDataReceive, KErrOverflow );
                            }
                        else
                            {
                            CompleteRequest( EIscAsyncDataReceive, KErrNone );
                            }
                        }
                    else
                        {
                        C_TRACE( ( _T( "DIscChannel::EmptyBuffers() data ThreadWrite %d" ), r ) );
                        CompleteRequest( EIscAsyncDataReceive, r );
                        }
                    }
                // client buffer too small
                else
                    {
                    TUint16 tempLen( frameInfo.writeLength );
                    TPtr8 tempLenDes( ( TUint8* )&tempLen, sizeof ( TUint16 ), sizeof ( TUint16 ) );
                    
                    TInt r = ThreadWrite( ( TAny* )iNeededDataBufLen, &tempLenDes, 0 );
                    TRACE_ASSERT( r == KErrNone );
                    if ( r != KErrNone ) 
                        {
                        C_TRACE( ( _T( "DIscChannel::EmptyBuffers() data ThreadWrite %d" ), r ) );
                        }
                    
                    CompleteRequest( EIscAsyncDataReceive, KErrNoMemory );
                    }
                }
            else
                {
                HandleConcatenatedDataFrame( tempPtr, frameInfo );
                }
            }
        }
        // no frames in data queue
    else
        {
        C_TRACE( ( _T( "DIscChannel::EmptyBuffers() channelId 0x%x channelPtr No frames in data queue" ), iChannelNumber, this ) );
        }

    // Check if there is frame in queue
    if ( iFrameRxQueue->NextBufferLength() > KErrNone )
        {
        TDes8* tempPtr = ( TDes8* ) iFrameRxQueue->GetFirst();
        TIscFrameInfo frameInfo;
        TInt desMaxLen( KErrNone );
 
        iIscDevice->iIscMultiplexerInterface->GetFrameInfo( *tempPtr, frameInfo );
    
        // frame incoming and normal receive request active
        if ( iIscRequests[EIscAsyncReceive] && frameInfo.frameType == EIscNonDataFrame )
            {
            if ( frameInfo.concatenation == EIscNoConcatenation )
                {
                desMaxLen = Kern::ThreadGetDesMaxLength( iThread, iReceiveBufPtr );
                
                C_TRACE( ( _T( "DIscChannel::EmptyBuffers() desMaxLen %d" ),desMaxLen ) );

                TRACE_ASSERT( desMaxLen > KErrNone );

                // check that client's memory block is big enough
                if ( desMaxLen >= frameInfo.writeLength )
                    {
                    // create a temporary descriptor for writing since we're
                    // necessary not writing the whole contents of the 
                    // source descriptor, only the part that ISC Multiplexer
                    // wants
                    TPtr8 writePtr( ( TUint8* )( tempPtr->Ptr() + frameInfo.writeStartIndex ),
                                    frameInfo.writeLength,
                                    frameInfo.writeLength );
                    
                    TInt r = ThreadWrite( iReceiveBufPtr, &writePtr, 0 );

                    TRACE_ASSERT( r == KErrNone );
                    
                    // remove the pointer from queue and release the memory block 
                    // but only if the ThreadWrite was successfull
                    if ( r == KErrNone )
                        {
                        iFrameRxQueue->DeleteFirst();
                        iIscDevice->ReleaseMemoryBlock( tempPtr );
                        if ( iClientPanic )
                            {
                            iClientPanic = EFalse;
                            CompleteRequest( EIscAsyncReceive, KErrOverflow );                            
                            }
                        else
                            {
                            CompleteRequest( EIscAsyncReceive, KErrNone );
                            }
                        }
                    else
                        {
                        C_TRACE( ( _T( "DIscChannel::EmptyBuffers() ThreadWrite %d" ), r ) );
                        CompleteRequest( EIscAsyncReceive, r );
                        }
                    }
                // client buffer too small
                else
                    {
                    TUint16 tempLen = frameInfo.writeLength;
                    TPtr8 tempLenDes( ( TUint8* )&tempLen, sizeof ( TUint16 ), sizeof ( TUint16 ) );
                    
                    TInt r = ThreadWrite ( ( TAny* )iNeededBufLen, &tempLenDes, 0 );
                    TRACE_ASSERT( r == KErrNone );
                    if ( r != KErrNone ) 
                        {
                        C_TRACE( ( _T( "DIscChannel::EmptyBuffers() ThreadWrite %d" ), r ) );
                        }
                    
                    CompleteRequest( EIscAsyncReceive, KErrNoMemory );
                    }
                }
            else
                {
                HandleConcatenatedFrame( tempPtr, frameInfo );
                }
            }
        }
    // no frames in queue
    else
        {
        C_TRACE( ( _T( "DIscChannel::EmptyBuffers() channelId 0x%x channelPtr 0x%x No frames in queue" ), iChannelNumber, this ) );
        }

    // If possible, set flow control off from data frame receiving
    if ( iDataFrameRxQueue->Count() <= iIscChannelLowWaterMark 
         && iDLFlowControlStatus != EIscFlowControlOff )
        {
        iIscDevice->DLFlowControlNotify( EIscFlowControlOff, iChannelNumber, this );
        iDLFlowControlStatus = EIscFlowControlOff;
        }
        

    C_TRACE( ( _T( "DIscChannel::EmptyBuffers - return void" ) ) );

    }

// -----------------------------------------------------------------------------
// DIscChannel::HandleConcatenatedDataFrame
// Copies several data frames to clients buffer if needed before compliting receive request
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::HandleConcatenatedDataFrame( TDes8* aPtr, TIscFrameInfo& aInfo )
    {
    C_TRACE( ( _T( "DIscChannel::HandleConcatenatedDataFrame(0x%x, 0x%x) channelPtr 0x%x" ), aPtr, &aInfo, this ) );
    TInt desMaxLen( KErrNone );
    TInt desLen( KErrNone );
    TUint16 totalLength( 0 );

    desMaxLen = Kern::ThreadGetDesMaxLength( iThread, iDataReceiveBufPtr );
    desLen = Kern::ThreadGetDesLength( iThread, iDataReceiveBufPtr );
    
    if ( aInfo.totalLength > KErrNone )
        {
        totalLength = aInfo.totalLength;
        }
    else
        { 
        totalLength = desMaxLen;
        }
    switch ( aInfo.concatenation )
        {
        // first frame of a larger data chunk
        case EIscConcatenationDataStart:
            {
            // check whether the whole data amount will fit into the user buffer
            if ( desMaxLen >= ( desLen + aInfo.writeLength ) && desMaxLen >= totalLength )
                {
                // create a temporary descriptor for writing since we're
                // necessary not writing the whole contents of the 
                // source descriptor, only the part that ISC Multiplexer
                // wants
                TPtr8 writePtr( ( TUint8* )( aPtr->Ptr() + aInfo.writeStartIndex ),
                                aInfo.writeLength,
                                aInfo.writeLength );
                
                // start writing the data at offset 0 since this is the first frame 
                TInt r = ThreadWrite( iDataReceiveBufPtr, &writePtr, 0 );
                    
                // remove the pointer from queue and release the memory block 
                // but only if the ThreadWrite was successfull
                // we do not complete the user request until EIscConcatenationDataEnd
                if ( r == KErrNone )
                    {
                    iDataFrameRxQueue->DeleteFirst();
                    iIscDevice->ReleaseMemoryBlock( aPtr );
                    }
                else
                    {
                    C_TRACE( ( _T( "DIscChannel::HandleConcatenatedDataFrame() ThreadWrite %d" ), r ) );
                    TRACE_ASSERT_ALWAYS;
                    CompleteRequest( EIscAsyncDataReceive, KErrWrite );
                    }
                }            
            else // buffer too small
                {
                TRACE_ASSERT( totalLength >= ( desLen + aInfo.writeLength ) );
                TPtr8 tempLenDes( ( TUint8* )&totalLength, sizeof ( TUint16 ), sizeof ( TUint16 ) );
                TInt r = ThreadWrite ( ( TAny* )iNeededDataBufLen, &tempLenDes, 0 );
                TRACE_ASSERT( r == KErrNone );
                if ( r != KErrNone ) 
                    {
                    C_TRACE( ( _T( "DIscChannel::HandleConcatenatedDataFrame() data start KErrNoMemory ThreadWrite %d" ), r ) );
                    }
                
                CompleteRequest( EIscAsyncDataReceive, KErrNoMemory );
                }
            break;
            }
        case EIscConcatenationData:
        case EIscConcatenationDataEnd:    
            {
            // check whether the next frame fits to the remaining buffer
            if ( ( desMaxLen - desLen ) >= aInfo.writeLength ) 
                {
                // create a temporary descriptor for writing since we're
                // necessary not writing the whole contents of the 
                // source descriptor, only the part that ISC Multiplexer
                // wants
                TPtr8 writePtr( ( TUint8* )( aPtr->Ptr() + aInfo.writeStartIndex ),
                                aInfo.writeLength,
                                aInfo.writeLength );
                
                // start writing the data at offset desLen
                TInt r = ThreadWrite( iDataReceiveBufPtr, &writePtr, desLen );
                TRACE_ASSERT( r == KErrNone );
                // remove the pointer from queue and release the memory block 
                // but only if the ThreadWrite was successfull
                if ( r == KErrNone ) 
                    {
                    iDataFrameRxQueue->DeleteFirst();
                    iIscDevice->ReleaseMemoryBlock( aPtr );
                    }                
                else
                    {                    
                    C_TRACE( ( _T( "DIscChannel::HandleConcatenatedDataFrame() data ThreadWrite %d" ), r ) );
                    }

                // complete client request if the frame was the last one
                if ( aInfo.concatenation == EIscConcatenationDataEnd )
                    {
                    if ( r == KErrNone )
                        {
                        CompleteRequest( EIscAsyncDataReceive, KErrNone );
                         }
                    else
                        {
                        CompleteRequest( EIscAsyncDataReceive, KErrWrite );
                        }
                    }
                }
            else
                {
                CompleteRequest( EIscAsyncDataReceive, KErrUnderflow );
                }
            break;
            }
        default:
            {
            ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscNotAllowedCallToDoCancel );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// DIscChannel::HandleConcatenatedFrame
// Copies several frames to clients buffer if needed before compliting receive request
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::HandleConcatenatedFrame( TDes8* aPtr, TIscFrameInfo& aInfo )
    {
    C_TRACE( ( _T( "DIscChannel::HandleConcatenatedFrame(0x%x, 0x%x) channelPtr 0x%x" ), aPtr, &aInfo, this ) );
    TInt desMaxLen( 0 );
    TInt desLen( 0 );
    TUint16 totalLength( 0 );

    desMaxLen = Kern::ThreadGetDesMaxLength( iThread, iReceiveBufPtr );
    desLen = Kern::ThreadGetDesLength( iThread, iReceiveBufPtr );

    if ( aInfo.totalLength > 0 )
        {
        totalLength = aInfo.totalLength;
        }
    else
        { 
        totalLength = desMaxLen;
        }
    switch ( aInfo.concatenation )
        {
        // first frame of a larger data chunk
        case EIscConcatenationDataStart:
            {
            // check whether the whole data amount will fit into the user buffer
            if ( desMaxLen >= ( desLen + aInfo.writeLength ) && desMaxLen >= totalLength )
                {
                // create a temporary descriptor for writing since we're
                // necessary not writing the whole contents of the 
                // source descriptor, only the part that ISC Multiplexer
                // wants
                TPtr8 writePtr( ( TUint8* )( aPtr->Ptr() + aInfo.writeStartIndex ),
                                aInfo.writeLength,
                                aInfo.writeLength );
                
                // start writing the data at offset 0 since this is the first frame 
                TInt r = ThreadWrite( iReceiveBufPtr, &writePtr, 0 );
                TRACE_ASSERT( r == KErrNone );
                    
                // remove the pointer from queue and release the memory block 
                // but only if the ThreadWrite was successfull
                // we do not complete the user request until EIscConcatenationDataEnd
                if ( r == KErrNone )
                    {
                    iFrameRxQueue->DeleteFirst();
                    iIscDevice->ReleaseMemoryBlock( aPtr );
                    }
                else 
                    {
                    C_TRACE( ( _T( "DIscChannel::HandleConcatenatedFrame() ThreadWrite %d" ), r ) );
                    CompleteRequest( EIscAsyncReceive, KErrWrite );
                    }
                }
            else // buffer too small
                {
                TRACE_ASSERT( totalLength >= ( desLen + aInfo.writeLength ) );
                TPtr8 tempLenDes( ( TUint8* )&totalLength, sizeof ( TUint16 ), sizeof ( TUint16 ) );
                TInt r = ThreadWrite ( ( TAny* )iNeededBufLen, &tempLenDes, 0 );
                TRACE_ASSERT( r == KErrNone );
                if ( r != KErrNone ) 
                    {
                    C_TRACE( ( _T( "DIscChannel::HandleConcatenatedFrame() ThreadWrite %d" ), r ) );
                    }
                
                CompleteRequest( EIscAsyncReceive, KErrNoMemory );
                }
            break;
            }
        case EIscConcatenationData:
        case EIscConcatenationDataEnd:    
            {
            // check whether the next frame fits to the remaining buffer
            if ( ( desMaxLen - desLen ) >= aInfo.writeLength ) 
                {
                // create a temporary descriptor for writing since we're
                // necessary not writing the whole contents of the 
                // source descriptor, only the part that ISC Multiplexer
                // wants
                TPtr8 writePtr( ( TUint8* )( aPtr->Ptr() + aInfo.writeStartIndex ),
                                aInfo.writeLength,
                                aInfo.writeLength );
                
                // start writing the data at offset desLen
                TInt r = ThreadWrite( iReceiveBufPtr, &writePtr, desLen );
                
                if ( r != KErrNone ) 
                    {
                    C_TRACE( ( _T( "DIscChannel::HandleConcatenatedFrame() ThreadWrite %d" ), r ) );
                    }
                // remove the pointer from queue and release the memory block 
                // but only if the ThreadWrite was successfull
                if ( r == KErrNone )
                    {
                    iFrameRxQueue->DeleteFirst();
                    iIscDevice->ReleaseMemoryBlock( aPtr );
                    }

                // complete client request if the frame was the last one
                if ( aInfo.concatenation == EIscConcatenationDataEnd )
                    {
                    if ( r == KErrNone )
                        {
                        CompleteRequest( EIscAsyncReceive, KErrNone );
                         }
                    else
                        {
                        CompleteRequest( EIscAsyncReceive, KErrWrite );
                        }
                    }
                }
            else
                {
                CompleteRequest( EIscAsyncDataReceive, KErrUnderflow );
                }
            break;
            }
        default:
            {
            ASSERT_RESET_ALWAYS( 0,"IscDriver",EIscUnknownCommand );
            break;
            }
        }

    }

// -----------------------------------------------------------------------------
// DIscChannel::CopyFromUserBuffer
// Copy data from user-thread memory space.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscChannel::CopyFromUserBuffer( 
    const TDesC8& aUserBuffer,
    TDes8& aKernelBuffer,
    const TInt aOffset )
    {
    C_TRACE( ( _T( "DIscChannel::CopyFromUserBuffer(0x%x, 0x%x, 0x%x) channelPtr 0x%x" ), &aUserBuffer, &aKernelBuffer, aOffset, this ) );
    return Kern::ThreadDesRead( iThread, ( TAny* )&aUserBuffer, aKernelBuffer, aOffset, KChunkShiftBy0 );

    }

// -----------------------------------------------------------------------------
// DIscChannel::ThreadWrite
// Writes data/frames to clients buffer
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscChannel::ThreadWrite( 
    TAny* dest,
    const TDesC8* src,
    const TInt aOffset )
    {
    C_TRACE( ( _T( "DIscChannel::ThreadWrite(0x%x, 0x%x, 0x%x)" ), dest, src, aOffset ) );
    C_TRACE( ( _T( "DIscChannel::ThreadWrite writeLen 0x%x" ), src->Length() ) );
    
    return Kern::ThreadDesWrite( iThread, dest, *src, aOffset, iThread ); 

    }

// -----------------------------------------------------------------------------
// DIscChannel::ResetBuffers
// Resets buffers
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void DIscChannel::ResetBuffers()
    {
    // Delete pending send frames
    iIscDevice->CancelSending( iChannelNumber, this );

    // Empty receive queue
    if ( iFrameRxQueue )
        {
        while ( !iFrameRxQueue->Empty() )
            {
            TDes8* tempPtr = ( TDes8* ) iFrameRxQueue->GetFirst();
            iFrameRxQueue->DeleteFirst();
            iIscDevice->ReleaseMemoryBlock( tempPtr );
            }
        }
    
    // Empty data receive queue
    if ( iDataFrameRxQueue )
        {
        while ( !iDataFrameRxQueue->Empty() )
            {
            TDes8* tempPtr = ( TDes8* ) iDataFrameRxQueue->GetFirst();
            iDataFrameRxQueue->DeleteFirst();
            iIscDevice->ReleaseMemoryBlock( tempPtr );
            }
        }
    }

//  End of File  
