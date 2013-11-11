/*
* Copyright (c) 2004-2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implementation of CStartupAdaptationStub class.
*
*/



// INCLUDE FILES
#include "StartupAdaptationStub.h"
#include "CommandProcessingEngine.h"
#include "StartupAdaptationStubModel.h"
#include "sastubeventlistener.h"
#include "StartupAdaptationStubDebug.h"
#include <startupadaptationobserver.h>

using namespace StartupAdaptation;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CStartupAdaptationStub::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CStartupAdaptationStub* CStartupAdaptationStub::NewL( TAny* aConstructionParameters )
    {
    RDEBUG( _L( "CStartupAdaptationStub::NewL." ) );

    CStartupAdaptationStub* self =
        new( ELeave ) CStartupAdaptationStub(
            static_cast<MStartupAdaptationObserver*>( aConstructionParameters ) );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    RDEBUG( _L( "CStartupAdaptationStub::NewL finished." ) );

    return self;
    }


// Destructor
CStartupAdaptationStub::~CStartupAdaptationStub()
    {
    RDEBUG( _L( "CStartupAdaptationStub::~CStartupAdaptationStub." ) );

    delete iEventListener;
    delete iModel;
    delete iEngine;

    RDEBUG( _L( "CStartupAdaptationStub::~CStartupAdaptationStub finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStub::CommandL
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStub::CommandL(
    const TCommand aCommandId,
    TDesC8& /*aData*/ )
    {
    RDEBUG_1( _L( "CStartupAdaptationStub::CommandL( %d )." ), aCommandId );

    TInt duration = iModel->GetDurationForNextCall( aCommandId );
    iEngine->ExecuteCommandL( aCommandId, duration );

    RDEBUG( _L( "CStartupAdaptationStub::CommandL finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStub::CommandL
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStub::CancelCommandL(
    const TCommand /*aCommandId*/ )
    {
    RDEBUG( _L( "CStartupAdaptationStub::CancelCommandL." ) );

    iEngine->Cancel();

    RDEBUG( _L( "CStartupAdaptationStub::CancelCommandL finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStub::Version
//
// -----------------------------------------------------------------------------
//
TVersion CStartupAdaptationStub::Version() const
    {
    return TVersion( 0, 1, 0 );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStub::CommandProcessedL
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStub::CommandProcessedL(
    const TInt aCommandId )
    {
    RDEBUG_1( _L( "CStartupAdaptationStub::CommandProcessedL( %d )." ), aCommandId );

    switch ( aCommandId )
        {
        case EGlobalStateChange:
            {
            TResponsePckg responsePckg(
                iModel->GetGlobalStateChangeResponse() );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EExecuteSelftests:
            {
            TResponsePckg responsePckg( iModel->GetSelftestResponse() );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EValidateRTCTime:
            {
            TResponsePckg responsePckg(
                iModel->GetRTCTimeValidityResponse() );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case ESetWakeupAlarm:
            {
            TResponsePckg responsePckg( iModel->GetSetAlarmResponse() );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case ECancelWakeupAlarm:
            {
            TResponsePckg responsePckg( KErrNone );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EExecuteReset:
            {
            TResponsePckg responsePckg( iModel->GetResetResponse() );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EExecuteShutdown:
            {
            TResponsePckg responsePckg( iModel->GetShutdownResponse() );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EExecuteDOSRfs:
            {
            TResponsePckg responsePckg( iModel->GetRFSResponse() );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EGetSimChanged:
            {
            TBooleanResponse response;
            iModel->GetSimChangedResponse( response );
            TBooleanResponsePckg responsePckg( response );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EGetSimOwned:
            {
            TBooleanResponse response;
            iModel->GetSimOwnedResponse( response );
            TBooleanResponsePckg responsePckg( response );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case ESecurityStateChange:
            {
            TSecurityStateResponse response;
            iModel->GetSecurityStateChangeResponse( response );
            TSecurityStateResponsePckg responsePckg( response );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EGetGlobalStartupMode:
            {
            TGlobalStartupModeResponse response;
            iModel->GetStartupModeResponse( response );
            TGlobalStartupModeResponsePckg responsePckg( response );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EGetSIMLanguages:
            {
            RLanguageListResponse response;
            CleanupClosePushL( response );
            iModel->GetLanguageListResponse( response );
            TLanguageListResponsePckg responsePckg( response );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            CleanupStack::PopAndDestroy();
            break;
            }
        case EActivateRfForEmergencyCall:
            {
            TResponsePckg responsePckg( iModel->GetActivateRfResponse() );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EDeactivateRfAfterEmergencyCall:
            {
            TResponsePckg responsePckg( iModel->GetDeactivateRfResponse() );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        case EGetHiddenReset:
            {
            TBooleanResponse response;
            iModel->GetHiddenResetResponse( response );
            TBooleanResponsePckg responsePckg( response );
            iObserver->ResponseL(
                static_cast<TCommand>( aCommandId ),
                responsePckg );
            break;
            }
        default:
            RDEBUG_1( _L( "CStartupAdaptationStub: Unexpected command: %d." ), aCommandId );
            User::Panic( _L( "Unexpected command" ), KErrNotSupported );
            break;
        };


    RDEBUG( _L( "CStartupAdaptationStub::CommandProcessedL finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStub::SendEvent
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStub::SendEvent( const TInt aEvent, const TInt aArg )
    {
    RDEBUG_2( _L( "CStartupAdaptationStub: SendEvent 0x%08x, 0x%08x." ), aEvent, aArg );

    TInt errorCode = KErrNone;
    TEvent eventID = static_cast<TEvent>( aEvent );
    if ( eventID == EFatalError )
        {
        TFatalErrorTypePckg params( static_cast<TFatalErrorType>( aArg ) );
        TRAP( errorCode, iObserver->EventL( eventID, params ) );
        }
    else if ( eventID == ESimEvent )
        {
        TSimEventTypePckg params( static_cast<TSimEventType>( aArg ) );
        TRAP( errorCode, iObserver->EventL( eventID, params ) );
        }
    else
        {
        TBuf8<1> dummy;
        TRAP( errorCode, iObserver->EventL( eventID, dummy ) );
        }

    if ( errorCode != KErrNone )
        {
        RDEBUG_1( _L( "CStartupAdaptationStub: EventL leave code %d." ), errorCode );
        }
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStub::CStartupAdaptationStub
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CStartupAdaptationStub::CStartupAdaptationStub(
    MStartupAdaptationObserver* anObserver )
  : iObserver( anObserver ),
    iEventListener( NULL )
    {
    RDEBUG( _L( "CStartupAdaptationStub::CStartupAdaptationStub." ) );

    __ASSERT_ALWAYS( anObserver, User::Invariant() );

    RDEBUG( _L( "CStartupAdaptationStub::CStartupAdaptationStub finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStub::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStub::ConstructL()
    {
    RDEBUG( _L( "CStartupAdaptationStub::ConstructL." ) );

    iEngine = CCommandProcessingEngine::NewL( *this );
    iModel = CStartupAdaptationStubModel::NewL();

#ifdef __STARTER_MODULE_TEST_SUPPORT__
    iEventListener = CSAStubEventListener::NewL( *this );
#endif // __STARTER_MODULE_TEST_SUPPORT__

    RDEBUG( _L( "CStartupAdaptationStub::ConstructL finished." ) );
    }


//  End of File
