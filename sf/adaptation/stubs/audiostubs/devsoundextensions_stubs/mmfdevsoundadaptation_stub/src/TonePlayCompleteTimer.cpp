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
* Description: Audio Stubs -  Utility timer object used for playing tone sequence.
*
*/



// INCLUDE FILES
#include "MmfDevSoundAdaptationBody.h"
#include "TonePlayCompleteTimer.h"

#ifdef _DEBUG
#include "e32debug.h"

#define DEBPRN0(str)        RDebug::Print(str, this)
#define DEBPRN1(str, val1)  RDebug::Print(str, this, val1)
#else
#define DEBPRN0(str)
#define DEBPRN1(str, val1)
#endif //_DEBUG



// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CTonePlayCompleteTimer::CTonePlayCompleteTimer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CTonePlayCompleteTimer::CTonePlayCompleteTimer(
    MDevSoundAdaptationObserver& aObserver) :
    CActive(EPriorityNormal), iObserver(aObserver)
    {
    CActiveScheduler::Add(this);
    }

// -----------------------------------------------------------------------------
// CTonePlayCompleteTimer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CTonePlayCompleteTimer::ConstructL()
    {
    User::LeaveIfError(iTimer.CreateLocal());
    }

// -----------------------------------------------------------------------------
// CTonePlayCompleteTimer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CTonePlayCompleteTimer* CTonePlayCompleteTimer::NewL(
                        MDevSoundAdaptationObserver& aObserver)
    {
    CTonePlayCompleteTimer* self = new (ELeave)CTonePlayCompleteTimer(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }


// -----------------------------------------------------------------------------
// CTonePlayCompleteTimer::~CTonePlayCompleteTimer
// Destructor
// -----------------------------------------------------------------------------
//
CTonePlayCompleteTimer::~CTonePlayCompleteTimer()
    {
    // Cancel any outstanding requests
    iTimer.Cancel();
    Cancel();
    iTimer.Close();
    }

// ----------------------------------------------------------------------------
// CTonePlayCompleteTimer::SetTimeOut
// Sets the timeout duration.
// (other items were commented in a header).
// ----------------------------------------------------------------------------
void CTonePlayCompleteTimer::SetTimeOut(TTimeIntervalMicroSeconds32& aTimeOut)
    {
    iTimeOut = aTimeOut;
    }

// ----------------------------------------------------------------------------
// CTonePlayCompleteTimer::Start
// Starts the timer for timeout duration set by SetTimeOut().
// (other items were commented in a header).
// ----------------------------------------------------------------------------
void CTonePlayCompleteTimer::Start()
    {
    if (!IsActive())
        {
        SetActive();
        iTimer.After(iStatus, iTimeOut);
        }
    }

// ----------------------------------------------------------------------------
// CTonePlayCompleteTimer::RunL
// (other items were commented in a header).
// ----------------------------------------------------------------------------
void CTonePlayCompleteTimer::RunL()
    {
    DEBPRN0(_L("CTonePlayCompleteTimer[0x%x]::RunL"));
    iObserver.ToneFinished(KErrNone);
    }

// ----------------------------------------------------------------------------
// CTonePlayCompleteTimer::DoCancel
// (other items were commented in a header).
// ----------------------------------------------------------------------------
void CTonePlayCompleteTimer::DoCancel()
    {
    iTimer.Cancel();
    }

// ----------------------------------------------------------------------------
// CTonePlayCompleteTimer::RunError
// (other items were commented in a header).
// ----------------------------------------------------------------------------
TInt CTonePlayCompleteTimer::RunError(TInt /*aError*/)
    {
    return KErrNone;
    }

// End of File
