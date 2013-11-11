/*
* Copyright (c) 2004-2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implementation of CCommandProcessingEngine class.
*
*/



// INCLUDE FILES
#include "CommandProcessingEngine.h"
#include "CommandProcessingObserver.h"
#include "StartupAdaptationStubDebug.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCommandProcessingEngine::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCommandProcessingEngine* CCommandProcessingEngine::NewL(
    MCommandProcessingObserver& aObserver )
    {
    CCommandProcessingEngine* self =
        new( ELeave ) CCommandProcessingEngine( aObserver );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }


// Destructor
CCommandProcessingEngine::~CCommandProcessingEngine()
    {
    Cancel();
    iTimer.Close();
    iCommands.Close();
    }


// -----------------------------------------------------------------------------
// CCommandProcessingEngine::ExecuteCommandL
//
// -----------------------------------------------------------------------------
//
void CCommandProcessingEngine::ExecuteCommandL(
    const TInt aCommandId,
    const TInt aDuration )
    {
    RDEBUG_2( _L( "CCommandProcessingEngine::ExecuteCommandL( %d, %d )." ), aCommandId, aDuration );

    TCommandInfo info;
    info.iCommandId = aCommandId;
    info.iDuration = aDuration;
    iCommands.AppendL( info );

    if ( !IsActive() ) // If already active, this happens at the end of RunL.
    {
        ActivateWithTimeout( aDuration );
    }

    RDEBUG( _L( "CCommandProcessingEngine::ExecuteCommandL finished." ) );
    }


// -----------------------------------------------------------------------------
// CCommandProcessingEngine::RunL
//
// -----------------------------------------------------------------------------
//
void CCommandProcessingEngine::RunL()
    {
    RDEBUG( _L( "CCommandProcessingEngine::RunL." ) );
    RDEBUG_1( _L( "Command count: %d." ), iCommands.Count() );

    __ASSERT_ALWAYS( iCommands.Count() > 0, User::Invariant() );

    const TInt& cmd = iCommands[0].iCommandId;
    RDEBUG_1( _L( "Processed command( %d )." ), cmd );

    iObserver.CommandProcessedL( cmd );

    iCommands.Remove( 0 );

    if ( iCommands.Count() > 0 ) // Check if there are more
        {                        // commands waiting to be processed.
        ActivateWithTimeout( iCommands[0].iDuration );
        }

    RDEBUG( _L( "CCommandProcessingEngine::RunL finished." ) );
    }


// -----------------------------------------------------------------------------
// CCommandProcessingEngine::RunError
//
// -----------------------------------------------------------------------------
//
TInt CCommandProcessingEngine::RunError( TInt aError )
    {
    RDEBUG_1( _L( "CCommandProcessingEngine::RunError( %d )." ), aError );
    aError = aError; //To prevent compiler warning.
    RDEBUG( _L( "CCommandProcessingEngine::RunError finished." ) );
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CCommandProcessingEngine::DoCancel
//
// -----------------------------------------------------------------------------
//
void CCommandProcessingEngine::DoCancel()
    {
    RDEBUG( _L( "CCommandProcessingEngine::DoCancel." ) );

    iTimer.Cancel();

    RDEBUG( _L( "CCommandProcessingEngine::DoCancel finished." ) );
    }


// -----------------------------------------------------------------------------
// CCommandProcessingEngine::CCommandProcessingEngine
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCommandProcessingEngine::CCommandProcessingEngine(
    MCommandProcessingObserver& aObserver )
  : CActive( EPriorityStandard ), iObserver( aObserver )
    {
    CActiveScheduler::Add( this );
    }


// -----------------------------------------------------------------------------
// CCommandProcessingEngine::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCommandProcessingEngine::ConstructL()
    {
    User::LeaveIfError( iTimer.CreateLocal() );
    }


// -----------------------------------------------------------------------------
// CCommandProcessingEngine::ActivateWithTimeout
//
// -----------------------------------------------------------------------------
//
void CCommandProcessingEngine::ActivateWithTimeout( const TInt aDuration )
    {
    RDEBUG( _L( "CCommandProcessingEngine::ActivateWithTimeout." ) );

    iTimer.After( iStatus, 1000 * aDuration );
    SetActive();

    RDEBUG( _L( "CCommandProcessingEngine::ActivateWithTimeout finished." ) );
    }


//  End of File
