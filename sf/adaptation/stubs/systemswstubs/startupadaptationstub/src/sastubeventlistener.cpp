/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implementation of CSAStubEventListener class.
*
*/


#ifdef __STARTER_MODULE_TEST_SUPPORT__

#include "sastubeventlistener.h"
#include "sastubeventsender.h"
#include "startupadaptationstubdebug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// ?description_if_needed
// ---------------------------------------------------------------------------
//
CSAStubEventListener::CSAStubEventListener( MSAStubEventSender& aSender )
  : CActive( EPriorityStandard ),
    iSender( aSender )
    {
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// ?description_if_needed
// ---------------------------------------------------------------------------
//
void CSAStubEventListener::ConstructL()
    {
    RProperty::Define( TUid::Uid( 0x0AA01499 ), 1, 0 );
    User::LeaveIfError( iProperty.Attach( TUid::Uid( 0x0AA01499 ), 1 ) );

    iProperty.Subscribe( iStatus );
    SetActive();
    }

// ---------------------------------------------------------------------------
// ?description_if_needed
// ---------------------------------------------------------------------------
//
CSAStubEventListener* CSAStubEventListener::NewL( MSAStubEventSender& aSender )
    {
    CSAStubEventListener* self = new( ELeave ) CSAStubEventListener( aSender );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// ?description_if_needed
// ---------------------------------------------------------------------------
//
CSAStubEventListener::~CSAStubEventListener()
    {
    Cancel();

    iProperty.Close();
    }

// ---------------------------------------------------------------------------
// From class CActive.
// ---------------------------------------------------------------------------
//
void CSAStubEventListener::DoCancel()
    {
    iProperty.Cancel();
    }

// ---------------------------------------------------------------------------
// From class CActive.
// ---------------------------------------------------------------------------
//
void CSAStubEventListener::RunL()
    {
    if ( iStatus == KErrNone )
        {
        TInt newValue;
        TInt errorCode = iProperty.Get( newValue );

        if ( errorCode == KErrNone )
            {
            RDEBUG_1( _L( "CStartupAdaptationStub: Event: 0x%08x." ), newValue );

            iProperty.Set( 0 );
            TInt event = newValue & 0xFFFF0000;
            event = event >> 16;
            TInt arg = newValue & 0x0000FFFF;

            iSender.SendEvent( event, arg );
            }
        }

    if ( iStatus != KErrCancel )
        {
        iProperty.Subscribe( iStatus );
        SetActive();
        }
    }

#endif // __STARTER_MODULE_TEST_SUPPORT__
