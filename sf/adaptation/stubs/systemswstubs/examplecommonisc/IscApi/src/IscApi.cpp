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
* Description:  Implementation of RIscApi class
*
*/



// INCLUDE FILES
#include <E32svr.H>
#include <f32file.h>
#include <iscapi.h>
#include <IscDefinitions.h>
#include <iscdefinitions.h>
#include "IscTrace.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS
_LIT( KIscApiPanic,"IscApi Panic" );
_LIT( KIscDriverName,"IscDriver" );
_LIT( KIscDriverExtensionName,"IscDriverExtension" );

// MACROS

// LOCAL CONSTANTS AND MACROS
_LIT( KIniFile,"Z:\\resource\\ISC\\isc_config.ini" );
_LIT( KMultiplexerLdd,"MultiplexerLdd" );
_LIT( KDataTransmissionLdd,"DataTransmissionLdd" );
_LIT( KDataTransmissionPdd,"DataTransmissionPdd" );
_LIT( KPdd,"PDD" );
_LIT( KLdd,"LDD" );
_LIT( KHashSign,"#" );
_LIT( KSemiColon,";" );
_LIT( KEqualSign,"=" );

_LIT( KIscDriverLdd, "iscdriver.ldd" );
_LIT( KIscDriverExtensionLdd, "iscdriverextension.ldd" );

const TInt KZeroLength( KErrNone );
const TInt KStartPosition( KErrNone );
const TInt KOneParam( 1 );
const TInt KTwoParams( 2 );
const TInt KThreeParams( 3 );
const TInt KFirstParam( 0 );
const TInt KSecondParam( 1 );
const TInt KThirdParam( 2 );
const TInt KMajor( KErrNone );
const TInt KMinor( KErrNone );
const TInt KBuild( KErrNone );
const TInt KAddPosition( 1 );
const TInt KInfoLength( 1 );

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RIscApi::RIscApi
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C RIscApi::RIscApi()
    :iNeededLen( NULL, KZeroLength ),
     iNeededDataLen( NULL, KZeroLength ),
     iConnectionStatus( NULL, KZeroLength ),
     iFlowControlStatus( NULL, KZeroLength ),
     iOpen( EFalse ),   
     iChannelNumber( KIscFrameReceiverNotFound )
#ifdef __WINS__
     ,iInitializing( EFalse )
#endif
    {
    }


EXPORT_C RIscApi::~RIscApi()
    {
    
    if( iOpen || iChannelNumber != KIscFrameReceiverNotFound )
        {
        Close();
        }
    else
    	{
    	// Do nothing.	
    	}
    	
    }


// -----------------------------------------------------------------------------
// RIscApi::Initialize
// Initialize the ISC Interface
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::InitializeModemInterface
        ( 
        TRequestStatus& aStatus
        )
    {
    A_TRACE( ( _T( "RIscApi::InitializeModemInterface(0x%x)" ), &aStatus ) );

#ifdef __WINS__
    iInitializing = ETrue;
#endif

    CTrapCleanup* cleanup = CTrapCleanup::New();
    TInt r( KErrNone );
    TInt r2( KErrNone );
    HBufC8* multiplexerInit = NULL;
    TRAP( r, ( multiplexerInit = HBufC8::NewL( KIscIniLineLength ) ) );
    // buffer to hold the init-string for second-level driver
    HBufC8* dataTransmissionInit = NULL;
    TRAP( r2, ( dataTransmissionInit = HBufC8::NewL( KIscIniLineLength ) ) );
    __ASSERT_ALWAYS( ( KErrNone == r && KErrNone == r2 ),
    	User::Panic( KIscApiPanic, EIscApiMemoryAllocationFailure ) );
    // Read ini-file and load needed drivers.
    TRAP( r, LoadL( *multiplexerInit, *dataTransmissionInit ) );
    if( KErrAlreadyExists == r )
        {
        TRACE_ASSERT_ALWAYS;
        C_TRACE( ( _T( "RIscApi::InitializeModemInterface DRIVERS ERROR" ) ) );
        delete multiplexerInit;
        multiplexerInit = NULL;
        delete dataTransmissionInit;
        dataTransmissionInit = NULL;
        delete cleanup;
        cleanup = NULL;
        TRequestStatus* ptrStatus = &aStatus;
        User::RequestComplete( ptrStatus, r );
        return;
        }
    else
        {
        __ASSERT_ALWAYS( ( KErrNone == r ),
                           User::Panic( KIscApiPanic,EIscApiDriverLoadFail ) );
        }

    // Open control channel
    TInt channel( KIscControlChannel );
    TInt err = DoCreate( KIscDriverName, 
                            TVersion( KMajor, KMinor, KBuild ), 
                            channel,
                            NULL,
                            NULL,
                            EOwnerThread );
    __ASSERT_ALWAYS( KErrNone == err,
                     User::Panic( KIscApiPanic, EIscApiChannelCreateFail ) );
    iChannelNumber = static_cast<TUint16>( channel );
    iOpen = ETrue;

    TPtrC8 multiplexerInitString( multiplexerInit->Des() );
    TPtrC8 dataTransmissionInitString( dataTransmissionInit->Des() );
    TAny* params[ KThreeParams ];
    params[ KFirstParam ] = (TAny*)&aStatus;
    params[ KSecondParam ] = static_cast<TAny*>( &multiplexerInitString );
    params[ KThirdParam ] = static_cast<TAny*>( &dataTransmissionInitString );
    
    aStatus = KRequestPending;
    //Initialize Data transmission 
    DoSvControl( EIscAsyncInitializeModemInterface, params );
  
    delete multiplexerInit;
    multiplexerInit = NULL;
    delete dataTransmissionInit;
    dataTransmissionInit = NULL;
    delete cleanup;
    cleanup = NULL;
    
#ifdef __WINS__
    iInitializing = EFalse;
#endif
    A_TRACE( ( _T( "RIscApi::InitializeModemInterfaceL - return void" ) ) );
    
    }

// -----------------------------------------------------------------------------
// RIscApi::InitializeCancel
// Cancel ISC Initialization sequence
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::InitializeCancel()
    {
    A_TRACE( ( _T( "RIscApi::InitializeCancel()" ) ) );

	if( iOpen && iChannelNumber == KIscControlChannel )
		{
		DoControl( EIscCancelAsyncInitialize );			
		}
	else
		{
		// Do nothing if not controlchannel who is handling the initialization.	
		}

    A_TRACE( ( _T( "RIscApi::InitializeCancel - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::LoadL
// Load required drivers using Isc_config.ini file
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void RIscApi::LoadL
        ( 
        const TDesC8& aMultiplexerInit,     // Multiplexer init. 
        const TDesC8& aDataTransmissionInit // Datatransmission init.
        ) const
    {
    C_TRACE( ( _T( "RIscApi::LoadL(0x%x, 0x%x)" ), &aMultiplexerInit,
    											   &aDataTransmissionInit ) );

    TInt err( KErrNone );
    TInt loadStatus( KErrNone );
    TInt pos( KErrNone );
    
    // Temporary buffers.
    HBufC* driverNameBuf = HBufC::NewLC( KIscIniLineLength );
    HBufC* iniFileLineBuf = HBufC::NewLC( KIscIniLineLength );
    HBufC* initStringBuf = HBufC::NewLC( KIscIniLineLength );
    // Temporary pointers.
    TPtr driverName( driverNameBuf->Des() );
    TPtr initString( initStringBuf->Des() );
    TPtr iniFileLine( iniFileLineBuf->Des() );

    RFs fsSession;
    CleanupClosePushL( fsSession );
    RFile file;
    CleanupClosePushL( file );
    TFileText fileText;
    User::LeaveIfError( fsSession.Connect() );
    User::LeaveIfError( file.Open( fsSession, KIniFile,
    							   EFileStreamText | EFileRead ) );
    fileText.Set( file );

    while( KErrNone == fileText.Read( iniFileLine ) )
        {
        if( KErrNone < iniFileLine.Length() )
            {
            C_TRACE( ( iniFileLine ) );
            pos = KErrNotFound;
            loadStatus = KErrNotFound;
            if ( iniFileLine.Find( KHashSign ) != KErrNotFound )
                {
                pos = KErrNone;
                loadStatus = KErrNone;
                }
            else if ( iniFileLine.Find( KDataTransmissionPdd ) != KErrNotFound )
                {
                C_TRACE( ( _T( "RIscApi::LoadL - datatransmissionpdd" ) ) );
                iniFileLine.Trim();
                pos = InifileLineParser( iniFileLine, driverName );
                if( KErrNotFound != pos )
                    {
                    loadStatus = User::LoadPhysicalDevice( driverName );
                    }
                else
                	{
                	// Do nothing.
                	}
                }          
            else if ( iniFileLine.Find( KDataTransmissionLdd ) != KErrNotFound )
                {
                C_TRACE( ( iniFileLine ) );
                pos = InifileLineParser( iniFileLine, driverName );
                if( KErrNotFound != pos )
                    {
                    DriverNameParser( initString, driverName,
                    				 *&aDataTransmissionInit );
                    loadStatus = User::LoadLogicalDevice( driverName );
                    }
                else
                	{
                	// Do nothing.
                	}
                }
            else if ( iniFileLine.Find( KMultiplexerLdd ) != KErrNotFound )
                {
                C_TRACE( ( iniFileLine ) );
                pos = InifileLineParser( iniFileLine, driverName );
                if( KErrNotFound != pos )
                    {
                    DriverNameParser( initString, driverName,
                    				  *&aMultiplexerInit );
                    loadStatus = User::LoadLogicalDevice( driverName );
                    }
                else
                	{
                	// Do nothing.
                	}
                }
            else if ( iniFileLine.Find( KPdd ) != KErrNotFound )
                {
                C_TRACE( ( _T( "RIscApi::LoadL - pdd" ) ) );
                iniFileLine.Trim();
                pos = InifileLineParser( iniFileLine, driverName );
                if( KErrNotFound != pos )
                    {
                    loadStatus = User::LoadPhysicalDevice( driverName );
                    }
                else
                	{
                	// Do nothing.
                	}
                }
            else if ( iniFileLine.Find( KLdd ) != KErrNotFound )
                {
                RDebug::Print( _L( "RIscApi::LoadL ldd" ) );
                iniFileLine.Trim();
                pos = InifileLineParser( iniFileLine, driverName );
                if( KErrNotFound != pos )
                    {
                    loadStatus = User::LoadLogicalDevice( driverName );
                    }
                else
                	{
                	// Do nothing.
                	}
                }
            __ASSERT_ALWAYS( pos != KErrNotFound,
            				 User::Panic( KIscApiPanic, EIscApiFalseIniFile ) );
            __ASSERT_ALWAYS( KErrAlreadyExists != loadStatus,
            				 User::Leave( loadStatus ) );
            __ASSERT_ALWAYS( KErrNone == loadStatus,
            				 User::Panic( KIscApiPanic, EIscApiDriverLoadFail ) );
            }
        else
        	{
        	C_TRACE( ( _T( "RIscApi::LoadL IniFileLength = KErrNone" ) ) );
        	}
        }
    loadStatus = User::LoadLogicalDevice( KIscDriverLdd );
    err = User::LoadLogicalDevice( KIscDriverExtensionLdd );
    C_TRACE( ( _T( "RIscApi::LoadL drivers iscdriver %d, iscdriverext %d" ),
    			    loadStatus, err ) );
    __ASSERT_ALWAYS( ( KErrNone == loadStatus|| KErrNone == err ),
                       User::Panic( KIscApiPanic, EIscApiDriverLoadFail ) );
    // R-Classes fsSession and file Close-method is called when deleted
    // from CleanupStack.
    CleanupStack::PopAndDestroy( KTwoParams );
    CleanupStack::PopAndDestroy( initStringBuf );
    CleanupStack::PopAndDestroy( iniFileLineBuf );
    CleanupStack::PopAndDestroy( driverNameBuf );
    C_TRACE( ( _T( "RIscApi::LoadL - return void" ) ) );
    
    }

// -----------------------------------------------------------------------------
// IscApi::LoadLHelper
// Helps parsing the .ini file.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt RIscApi::InifileLineParser
        (
        TDes& aInifileLine,
        TDes& aDriverName 
        ) const
    {
    C_TRACE( ( _T( "RIscApi::InifileLineParser" ) ) );
    
    TInt position( KErrNotFound );
    position = aInifileLine.Find( KEqualSign );
    if( KErrNotFound != position )
        {
        // Split the string.
        C_TRACE( ( aInifileLine ) );
        aDriverName.Zero();
        aDriverName.Append( aInifileLine.Mid( position + KAddPosition ) );
        aInifileLine.Delete(  KStartPosition, KIscIniLineLength );
        C_TRACE( ( aDriverName ) );
        }
    else
    	{
    	// Do nothing.	
    	}         
    C_TRACE( ( _T( "RIscApi::InifileLineParser return %d" ), position ) );
    return position;
       
    }

// -----------------------------------------------------------------------------
// IscApi::LoadLSecondHelper
// Helps parsing the .ini file.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//    
void RIscApi::DriverNameParser
        (
        TDes& aInitString,
        TDes& aDriverName,
        const TDesC8& aString 
        )const    
    {
    C_TRACE( ( _T( "RIscApi::DriverNameParser" ) ) );

    TInt position = aDriverName.Find( KSemiColon );
    if( KErrNotFound != position )
        {
        C_TRACE( ( aInitString ) );
        // Split the string.
        aInitString.Zero();
        aInitString.Append( aDriverName.Mid( position + KAddPosition ) );
        aDriverName.Delete( position, KIscIniLineLength );
        C_TRACE( ( aDriverName ) );
        TPtr8 tmpPtr( ( static_cast<HBufC8*>( &const_cast<TDesC8&>( aString ) ) )->Des() );
        tmpPtr.Append( aInitString );
        C_TRACE( ( aInitString ) );
        }
    else
    	{
    	// Do nothing.	
    	}         
    C_TRACE( ( _T( "RIscApi::DriverNameParser return " ) ) );

    }

// -----------------------------------------------------------------------------
// RIscApi::Open
// Open ISC channel asyncronously
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::Open(    
    const TUint16 aChannelNumber,    
    TRequestStatus& aStatus,
    TOwnerType aType,
    const TDesC8* aOpenParams )
    {
    A_TRACE( ( _T( "RIscApi::Open(0x%x, 0x%x, 0x%x, 0x%x) this %x" ),
    			  aChannelNumber, &aStatus, aType, aOpenParams, this ) );

    // In case if async Close() is called but channel is not destructed,
    // release kernel side memory allocations before actual channel opening.
    if( iChannelNumber != KIscFrameReceiverNotFound && iOpen == EFalse )
        {
        Close();
        }
    else
    	{
    	// Do nothing.	
    	}        

#ifdef __WINS__
    // InitializeModemInterface() must be called to load correct drivers.
    // In HW this is done e.g. from EStart 

    RMutex iscapiMutex;
    if ( !iInitializing )
        {
        C_TRACE( ( _T( "RIscApi::Open mutex creation" ) ) );
        TInt createErr( iscapiMutex.CreateGlobal( _L( "ISCAPICONNECT" ) ) );
        if ( createErr == KErrNoMemory )
            {
            C_TRACE( ( _T( "RIscApi::Open mutex creation FAILED" ) ) );
            TRequestStatus* ptrStatus = &aStatus;
            User::RequestComplete( ptrStatus, KErrNoMemory );
            return;
            }
        else if ( createErr )
            {
            TInt openErr( iscapiMutex.OpenGlobal( _L( "ISCAPICONNECT" ) ) );
            if ( openErr )
                {
                C_TRACE( ( _T( "RIscApi::Open mutex open FAILED" ) ) );
                TRequestStatus* ptrStatus = &aStatus;
                User::RequestComplete( ptrStatus, KErrNoMemory );
                return;
                }
			else
				{
				// Do nothing.	
				}    
            }
	    else
	    	{
	    	// Do nothing.	
	    	}            
        iscapiMutex.Wait();
        }
        
    RDevice handle;
    // If already openend, initialization not needed
    TInt r = handle.Open( KIscDriverName );
    if( r == KErrNotFound && !iInitializing )
        {
        RIscApi temp;
        TRequestStatus status;
        temp.InitializeModemInterface( status );
        iInitializing = EFalse;
        
        User::WaitForRequest( status );
        r = status.Int();

        temp.Close();
        }
    else
    	{
    	// Do nothing.	
    	}        
    
    if( !iInitializing )
        {
        iscapiMutex.Signal();
        }
    else
    	{
    	// Do nothing.	
    	}            

    if( r != KErrNone )
        {
        C_TRACE( ( _T( "RIscApi::Open Initialization FAILED" ) ) );
        TRequestStatus* ptrStatus = &aStatus;
        User::RequestComplete( ptrStatus, r );
        return;
        }
    else
    	{
    	// Do nothing.	
    	}        
#endif

    TBuf8<KInfoLength> info;
    TInt error( KErrInUse );
    if( !iOpen )
        {
        // handle channels 1 .. 31, open channel to IscDriver
        if ( aChannelNumber >= KIscFirstChannel 
            && aChannelNumber < KIscMaxChannelsInLdd )
            {
            C_TRACE( ( _T( "RIscApi::Open 1st ldd" ) ) );
            info.Append( ( TUint8 )aChannelNumber );
            error = DoCreate( KIscDriverName, 
                                TVersion( KMajor, KMinor, KBuild ), 
                                ( TInt )aChannelNumber, 
                                NULL, 
                                &info, 
                                aType );
            
            }
        // handle channels 32 .. 63, open channel to IscDriverExtension
        else if ( aChannelNumber >= KIscMaxChannelsInLdd
                  && aChannelNumber < KIscNumberOfUnits )
            {
            C_TRACE( ( _T( "RIscApi::Open 2nd ldd" ) ) );
            info.Append( ( TUint8 )aChannelNumber );
            TInt channelCount( aChannelNumber - KIscMaxChannelsInLdd );
            error = DoCreate( KIscDriverExtensionName, 
                                TVersion( KMajor, KMinor, KBuild ), 
                                channelCount, 
                                NULL, 
                                &info, 
                                aType );
            }
        // channel number out of range
        else // illegal channel number
            {
            User::Panic( KIscApiPanic,EIscApiChannelNumberOutOfRange );
            }
        }

    if ( error != KErrNone )
        {
        C_TRACE( ( _T( "RIscApi::Open - Channel open Failed,\
         channel 0x%x error %d" ), aChannelNumber, error ) );
        TRequestStatus* ptrStatus = &aStatus;
        User::RequestComplete( ptrStatus, error );
        }
    else
        {
        iChannelNumber = aChannelNumber;
        C_TRACE( ( _T( "RIscApi::Open iChannelNumber %d " ), iChannelNumber ) );
        iOpen = ETrue;
	    TAny* params[ KThreeParams ];   
	    params[ KFirstParam ] = (TAny*)&aStatus;
	    params[ KSecondParam ] = (TAny*)aChannelNumber;
	    params[ KThirdParam ] = (TAny*)aOpenParams;

        aStatus = KRequestPending;
        error = DoSvControl( EIscAsyncOpen, params );
        }

    A_TRACE(  (  _T(  "RIscApi::Open - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::OpenCancel
// Cancel asynchronous channel opening
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::OpenCancel()
    {
    A_TRACE( ( _T( "RIscApi::OpenCancel() channel 0x%x" ), iChannelNumber ) );

    PanicIfNotOpen();

    DoSvControl( EIscCancelAsyncOpen );
    Close();
    
    A_TRACE( ( _T( "RIscApi::OpenCancel - return void" )  ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::Close
// Close ISC Channel
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::Close()
    {
    A_TRACE( ( _T( "RIscApi::Close channel 0x%x" ), iChannelNumber ) );

    if( iChannelNumber <= KIscLastChannel && iOpen )
        {
        DoSvControl( EIscSyncClose );
        }
    else
    	{
    	// Do nothing.	
    	}        

    iChannelNumber = KIscFrameReceiverNotFound;
    iOpen = EFalse;
    RHandleBase::Close();

    A_TRACE( ( _T( "RIscApi::Close - return void" )  ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::Close
// Close ISC Channel
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::Close( TRequestStatus& aStatus )
    {
    A_TRACE( ( _T( "RIscApi::Close(0x%x) channel (0x%x)" ), &aStatus,
    			  iChannelNumber ) );
    
    TAny* params[ KOneParam ];

    params[ KFirstParam ] = (TAny*)&aStatus;
    aStatus = KRequestPending;
    if ( iChannelNumber <= KIscLastChannel && 
         iOpen )
        {
        DoSvControl( EIscAsyncClose, params );
        }
    else												
        {
        C_TRACE((_T("RIscApi::Close FAILED")));
        TRequestStatus* ptrStatus = &aStatus;
        User::RequestComplete( ptrStatus, KErrNotReady );
        }

    iOpen = EFalse;

    A_TRACE( ( _T( "RIscApi::Close - return void" )  ) );
    }

// -----------------------------------------------------------------------------
// RIsaApi::ResetBuffers
// Resets buffers.
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::ResetBuffers()
    {
    A_TRACE( ( _T( "RIscApi::ResetBuffers() 0x%x" ), iChannelNumber ) );
    
    PanicIfNotOpen();
    
    DoSvControl( EIscSyncResetBuffers );
    
    A_TRACE( ( _T( "RIscApi::ResetBuffers - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::Send
// Send data to Domestic OS ( asynchronous )
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::Send( 
    TRequestStatus& aStatus, 
    const TDesC8& aData )
    {
    A_TRACE( ( _T( "RIscApi::Send(0x%x, 0x%x) iChannelNumber (0x%x) this %x" ),
    			   &aStatus, &aData, iChannelNumber, this ) );
    
    PanicIfNotOpen();
    
    TAny* params[ KTwoParams ];

    params[ KFirstParam ] = (TAny*)&aStatus;
    params[ KSecondParam ] = (TAny*)&aData;

    aStatus = KRequestPending;
    DoControl( EIscAsyncSend, params );

    A_TRACE( ( _T( "RIscApi::Send - return void" )  ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::Send
// Send data to Domestic OS ( synchronous )
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RIscApi::Send( 
    const TDesC8& aData )
    {
    A_TRACE( ( _T( "RIscApi::Send(0x%x) iChannelNumber (0x%x) this %x" ),
    			   &aData, iChannelNumber, this ) );    
    
    
    PanicIfNotOpen();
    
    TInt error( KErrNone );
         
    TAny* params[ KOneParam ];
    params[ KFirstParam ] = (TAny*)&aData;
    
    error = DoControl( EIscSyncSend, params );
    
    A_TRACE( ( _T( "RIscApi::Send - return %d" ), error ) );
    return error;
    }

// -----------------------------------------------------------------------------
// RIscApi::SendCancel
// Cancel asynchronous Send request
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::SendCancel()
    {
    A_TRACE(  (  _T(  "RIscApi::SendCancel(), channel 0x%x" ), iChannelNumber ) );

    PanicIfNotOpen();

    DoSvControl( EIscCancelAsyncSend );

    A_TRACE( ( _T( "RIscApi::SendCancel - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::Receive
// Receive data from Domestic OS
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::Receive( 
    TRequestStatus& aStatus, 
    TDes8& aData,
    TUint16& aNeededBufLen )
    {
    A_TRACE( ( _T( "RIscApi::Receive 0x%x, 0x%x, 0x%x iChannelNumber 0x%x this\
    		    0x%x" ), &aStatus, &aData, aNeededBufLen, iChannelNumber, this ) );
        
    PanicIfNotOpen();
    
    TAny* params[ KThreeParams ];
    
    iNeededLen.Set( ( TUint8* )&aNeededBufLen, sizeof ( TUint16 ), sizeof ( TUint16 ) );

    params[ KFirstParam ] = (TAny*)&aStatus;
    params[ KSecondParam ] = (TAny*)&aData;
    params[ KThirdParam ] = (TAny*)&iNeededLen;

    aStatus = KRequestPending;
    DoControl( EIscAsyncReceive, params );

    A_TRACE( ( _T( "RIscApi::Receive - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::ReceiveCancel
// Cancel data receiving from Domestic OS
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::ReceiveCancel()
    {
    A_TRACE( ( _T( "RIscApi::ReceiveCancel() iChannelNumber (0x%x)" ),
    			    iChannelNumber ) );
    
    PanicIfNotOpen();
    
    DoControl( EIscCancelAsyncReceive );

    A_TRACE( ( _T( "RIscApi::ReceiveCancel - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::DataReceiveCancel
// Cancel data receiving from Domestic OS
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::DataReceiveCancel()
    {
    A_TRACE( ( _T( "RIscApi::DataReceiveCancel() channel 0x%x" ),
    			    iChannelNumber ) );

    PanicIfNotOpen();
    
    DoControl( EIscCancelAsyncDataReceive );

    A_TRACE( ( _T( "RIscApi::DataReceiveCancel - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::ConnectionStatus
// Get the current connection status
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RIscApi::ConnectionStatus() 
    {
    A_TRACE( ( _T( "RIscApi::ConnectionStatus() channel 0x%x" ),
    			    iChannelNumber ) );
    
    PanicIfNotOpen();
    
    TInt state( EIscConnectionOk );

    state = DoControl( EIscSyncGetConnectionStatus );
    
    A_TRACE( ( _T( "RIscApi::ConnectionStatus - return 0x%x" ), state ) );

    return state;
    }

// -----------------------------------------------------------------------------
// RIscApi::NotifyConnectionStatus
// Listen for connection status changes
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::NotifyConnectionStatus( 
    TRequestStatus& aStatus,
    TInt& aConnectionStatus )
    {
    A_TRACE( ( _T( "RIscApi::NotifyConnectionStatus(0x%x, 0x%x)\
    iChannelNumber (0x%x)" ), &aStatus, aConnectionStatus, iChannelNumber ) );
    
    PanicIfNotOpen();
        
    TAny* params[ KTwoParams ];
    
    iConnectionStatus.Set( ( TUint8* )&aConnectionStatus, 
    						 sizeof ( TInt ), sizeof ( TInt ) );

    params[ KFirstParam ] = (TAny*)&aStatus;
    params[ KSecondParam ] = (TAny*)&iConnectionStatus;
    
    aStatus = KRequestPending;
    DoControl( EIscAsyncNotifyConnectionStatus, params );

    A_TRACE( ( _T( "RIscApi::NotifyConnectionStatus - return void" ) ) );    
    }

// -----------------------------------------------------------------------------
// RIscApi::NotifyConnectionStatusCancel
// Stop listening for connection status changes
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::NotifyConnectionStatusCancel()
    {
    A_TRACE( ( _T( "RIscApi::NotifyConnectionStatusCancel() channel 0x%x" ),
    			    iChannelNumber ) );

    PanicIfNotOpen();
    
    DoControl( EIscCancelAsyncNotifyConnection );

    A_TRACE( ( _T( "RIscApi::NotifyConnectionStatusCancel - return void" ) ) );
    }


// -----------------------------------------------------------------------------
// RIscApi::CustomFunction
// Asynchronous API extension function
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::CustomFunction( 
    TRequestStatus& aStatus, 
    const TUint16 aOperation, 
    TAny* aParameters )
    {
    A_TRACE( ( _T( "RIscApi::CustomFunction(0x%x, 0x%x, 0x%x) iChannelNumber\
     0x%x" ), &aStatus, aOperation, aParameters, iChannelNumber ) );
    
    PanicIfNotOpen();

    // Check if custom operation is out of range. 
    if ( aOperation < EIscAsyncCustomOperation1 
        || aOperation > EIscAsyncCustomOperation5 )
        {
        User::Panic( KIscApiPanic, EIscApiCustomFunctionUndefined );
        }
    TAny* params[ KTwoParams ];
    
    params[ KFirstParam ] = (TAny*)&aStatus;
    params[ KSecondParam ] = aParameters;

    aStatus = KRequestPending;
    DoSvControl( aOperation, params );
    
    A_TRACE( ( _T( "RIscApi::CustomFunction - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::CustomFunction
// Synchronous API extension function
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RIscApi::CustomFunction( 
    const TUint16 aOperation, 
    TAny* aParameters )
    {
    A_TRACE( ( _T( "RIscApi::CustomFunction(0x%x, 0x%x) iChannelNumber\
    	 (0x%x) this %x" ), aOperation, aParameters, iChannelNumber, this ) );
    
    PanicIfNotOpen();

    // Check if custom operation is out of range. 
    if ( aOperation < EIscSyncCustomOperation1 
        || aOperation > EIscSyncCustomOperation5 )
        {
        User::Panic( KIscApiPanic, EIscApiCustomFunctionUndefined );
        }

    TInt error( KErrNone );
    TAny* params[ KOneParam ];
    params[ KFirstParam ] = aParameters;

    error = DoSvControl( aOperation, params );

    A_TRACE( ( _T( "RIscApi::CustomFunction - return %d" ), error ) );
    return error;
    }

// -----------------------------------------------------------------------------
// RIscApi::CustomFunctionCancel
// Cancel the execution of the asynchronous API extension function
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::CustomFunctionCancel( 
    const TUint16 aOperation )
    {
    A_TRACE( ( _T( "RIscApi::CustomFunctionCancel(0x%x) iChannelNumber (0x%x)" )
    			  , aOperation, iChannelNumber ) );
    
    PanicIfNotOpen();

    // Check if custom operation is out of range
    if ( aOperation < EIscCancelAsyncCustomOperation1 
        || aOperation > EIscCancelAsyncCustomOperation5 )
        {
        User::Panic( KIscApiPanic, EIscApiCustomFunctionUndefined );
        }

    DoSvControl( aOperation );

    A_TRACE( ( _T( "RIscApi::CustomFunctionCancel - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::DataSend
// Send data to Domestic OS ( asynchronous )
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::DataSend( 
    TRequestStatus& aStatus,
    const TDesC8& aData )
    {
    A_TRACE( ( _T( "RIscApi::DataSend(0x%x, 0x%x) iChannelNumber 0x%x" ),
    			  &aStatus, &aData, iChannelNumber ) );
    
    PanicIfNotOpen();
    
    TAny* params[ KTwoParams ];
    
    params[ KFirstParam ] = (TAny*)&aStatus;
    params[ KSecondParam ] = (TAny*)&aData;

    aStatus = KRequestPending;
    DoControl( EIscAsyncDataSend, params );
    
    A_TRACE( ( _T( "RIscApi::DataSend - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::DataSend
// Send data to Domestic OS ( synchronous )
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RIscApi::DataSend( 
    const TDesC8& aData )
    {
    A_TRACE( ( _T( "RIscApi::DataSend(0x%x) iChannelNumber 0x%x" ),
    			    &aData, iChannelNumber ) );

    PanicIfNotOpen();

    TInt error(  KErrNone );
    TAny* params[ KOneParam ];
    
    params[ KFirstParam ] = (TAny*)&aData;

    error = DoControl( EIscSyncDataSend, params );

    A_TRACE( ( _T( "RIscApi::DataSend - return %d" ), error ) );
    return error;
    }

// -----------------------------------------------------------------------------
// RIscApi::DataSendCancel
// Cancel asynchronous DataSend request
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::DataSendCancel()
    {
    A_TRACE( ( _T( "RIscApi::DataSendCancel() iChannelNumber 0x%x" ),
    			   iChannelNumber ) );

    PanicIfNotOpen();

    DoSvControl( EIscCancelAsyncDataSend );

    A_TRACE( ( _T( "RIscApi::DataSendCancel - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::DataReceive
// Receive data from Domestic OS
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::DataReceive( 
    TRequestStatus& aStatus,
    TDes8& aData,
    TUint16& aNeededBufLen )
    {
    A_TRACE( ( _T( "RIscApi::DataReceive(0x%x, 0x%x, 0x%x) iChannelNumber 0x%x" ),
    			   &aStatus, &aData, aNeededBufLen, iChannelNumber ) );
    
    PanicIfNotOpen();
    
    TAny* params[ KThreeParams ];
    
    iNeededDataLen.Set( ( TUint8* )&aNeededBufLen, sizeof ( TUint16 ),sizeof ( TUint16 ) );

    params[ KFirstParam ] = (TAny*)&aStatus;
    params[ KSecondParam ] = (TAny*)&aData;
    params[ KThirdParam ] = (TAny*)&iNeededDataLen;

    aStatus = KRequestPending;
    DoControl( EIscAsyncDataReceive, params );
    
    A_TRACE( ( _T( "RIscApi::DataReceive - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::FlowControlStatus
// Get the current flow control status ( in uplink-direction )
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RIscApi::FlowControlStatus()
    {
    A_TRACE( ( _T( "RIscApi::FlowControlStatus() channel 0x%x" ),
    			   iChannelNumber ) );
    
    PanicIfNotOpen();

    TInt state( EIscFlowControlOff );

    state = DoControl( EIscSyncGetFlowControlStatus );

    A_TRACE( ( _T( "RIscApi::FlowControlStatus - return %d" ), state ) );

    return state;
    }


// -----------------------------------------------------------------------------
// RIscApi::NotifyFlowControl
// Listen for flow control status changes
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::NotifyFlowControl( 
    TRequestStatus& aStatus,
    TInt& aFlowControlStatus )
    {
    A_TRACE( ( _T( "RIscApi::NotifyFlowControl(0x%x, 0x%x) channel 0x%x" ),
    			  &aStatus, aFlowControlStatus, iChannelNumber ) );
    
    PanicIfNotOpen();
    
    TAny* params[ KTwoParams ];
    
    iFlowControlStatus.Set( ( TUint8* )&aFlowControlStatus,sizeof ( TInt ),sizeof ( TInt ) );

    params[ KFirstParam ] = (TAny*)&aStatus;
    params[ KSecondParam ] = (TAny*)&iFlowControlStatus;
    
    aStatus = KRequestPending;
    DoControl( EIscAsyncNotifyFlowControlStatus, params );
    
    A_TRACE( ( _T( "RIscApi::NotifyFlowControl - return void" ) ) );
    }


// -----------------------------------------------------------------------------
// RIscApi::NotifyFlowControlCancel
// Stop listening for flow control status changes
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C void RIscApi::NotifyFlowControlCancel()
    {
    A_TRACE(  (  _T(  "RIscApi::NotifyFlowControlCancel()" ) ) );

    PanicIfNotOpen();
    
    DoControl( EIscCancelAsyncNotifyFlowControl );

    A_TRACE( ( _T( "RIscApi::NotifyFlowControlCancel - return void" ) ) );
    }

// -----------------------------------------------------------------------------
// RIscApi::MaximumDataSize
// Get maximun data size ( for sending and receiving )
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RIscApi::MaximumDataSize()
    {
    A_TRACE( ( _T( "RIscApi::MaximumDataSize() channel 0x%x" ),
    				iChannelNumber ) );
    
    PanicIfNotOpen();

    TInt r = DoControl( EIscSyncGetMaximunDataSize );
    A_TRACE( ( _T( "RIscApi::MaximumDataSize - return 0x%x" ), r ) );
    return r;
        
    }

// -----------------------------------------------------------------------------
// RIscApi::GetChannelInfo
// Get channel info from multiplexer
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt RIscApi::GetChannelInfo( 
    const TUint16 aChannel, 
    TDes8& aInfo )
    {
    A_TRACE( ( _T( "RIscApi::GetChannelInfo(0x%x, 0x%x)" ),
    			    aChannel, &aInfo ) );
    
    PanicIfNotOpen();

    TAny* params[ KTwoParams ];  
    params[ KFirstParam ] = (TAny*)aChannel;
    params[ KSecondParam ] = (TAny*)&aInfo;
    
    TInt error = DoControl( EIscSyncGetChannelInfo, params );

    A_TRACE( ( _T( "RIscApi::GetChannelInfo - return %d" ), error ) );
    return error;
    }

// -----------------------------------------------------------------------------
// IscApi::PanicIfNotOpen
// Panic if channel is not opened yet
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
void RIscApi::PanicIfNotOpen() const
    {
    
    if( !iOpen ) 
        {
        C_TRACE( ( _T( "IscApi::PanicIfNotOpen() channel 0x%x" ),
        			   iChannelNumber ) );
        User::Panic( KIscApiPanic, EIscApiChannelNotOpen );
        }
    else
        {
        // Do nothing.
        }
    }

//  End of File  
