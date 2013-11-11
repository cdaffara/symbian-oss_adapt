/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -
*
*/



// INCLUDE FILES
#include "MmfHwDeviceStub.h"
#ifdef _DEBUG
#include "e32debug.h"

#define DEBPRN0(str)            RDebug::Print(str, this)
#define DEBPRN1(str, val1)       RDebug::Print(str, this, val1)
#define DEBPRN2(str, val1, val2)       RDebug::Print(str, this, val1, val2)
#else
#define DEBPRN0(str)
#define DEBPRN1(str, val1)
#define DEBPRN2(str, val1, val2)
#endif //_DEBUG

// CONSTANTS
const TUint KBufferLength1 = 0x1000;
const TUint KBufferLength2 = 0x1000;
//const TUint KBufferLength2 = 0x1800; // for creating second buffer with different length than the first buffer.
const TUint KTimerDuration = 200000;

#ifdef __WINSCW__
_LIT(KSample1,"C:\\sample1.wav");
#else
_LIT(KSample1,"Z:\\system\\data\\DevSoundAdaptationStub\\sample1.wav");
#endif // __WINSCW__

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CMMFHwDeviceStub::CMMFHwDeviceStub
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CMMFHwDeviceStub::CMMFHwDeviceStub()
: CActive(EPriorityNormal), iHwDeviceState(EHWDeviceIdle), iCurPlayBuffer(NULL),
  iCurRecdBuffer(NULL)
    {
    CActiveScheduler::Add(this);
    }

// -----------------------------------------------------------------------------
// CMMFHwDeviceStub::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CMMFHwDeviceStub::ConstructL()
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::ConstructL:BEGIN"));
    // Create timer
    User::LeaveIfError(iTimer.CreateLocal());
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::ConstructL:END"));
    }

// -----------------------------------------------------------------------------
// CMMFHwDeviceStub::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CMMFHwDeviceStub* CMMFHwDeviceStub::NewL()
    {
    CMMFHwDeviceStub* self = new (ELeave)CMMFHwDeviceStub;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CMMFHwDeviceStub::~CMMFHwDeviceStub
// Destructor
// -----------------------------------------------------------------------------
//
CMMFHwDeviceStub::~CMMFHwDeviceStub()
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::~CMMFHwDeviceStub:BEGIN"));
    // Cancel A/O and close the timer
    Cancel();
    iTimer.Close();

    // Delete buffers
    delete iPlayBuffer1;
    delete iPlayBuffer2;
    delete iRecdBuffer1;
    delete iRecdBuffer2;

    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::~CMMFHwDeviceStub:END"));
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::Start
// Starts playback/record based on aFuncCmd
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::Start(TDeviceFunc aFuncCmd, TDeviceFlow /*aFlowCmd*/)
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::Start:BEGIN"));
    TInt status(KErrNone);
    switch(aFuncCmd)
        {
        case EDevDecode:
            iCurPlayBuffer = NULL;

            // Initialize buffers
            TRAP(status, InitializePlayBufferL());
            if (status != KErrNone)
                return status;

            // Reset buffers rendered count to zero only if not pause-continue
            if (iHwDeviceState == EHWDeviceIdle)
                iCount = 0;
            // Initialize attributes
            iHwDeviceState = EHWDevicePlay;
            iLastBufferReceived = EFalse;
            // If not already active, launch timer
            if (!IsActive())
                {
                iTimer.After(iStatus,TTimeIntervalMicroSeconds32(KTimerDuration) );
                SetActive();
                }
            break;
        case EDevEncode:
            iCurRecdBuffer = NULL;

            // Initialize buffers
            TRAP(status, InitializeRecdBufferL());
            if (status != KErrNone)
                return status;

            // Reset buffers rendered count to zero only if not pause-continue
            if (iHwDeviceState == EHWDeviceIdle)
                iCount = 0;
            // Initialize attributes
            iHwDeviceState = EHWDeviceRecord;
            // If not already active, launch timer
            if (!IsActive())
                {
                iTimer.After(iStatus,TTimeIntervalMicroSeconds32(KTimerDuration) );
                SetActive();
                }
            break;
        default:
            status = KErrNotSupported;
            break;
        };
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::Start:END"));
    return status;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::Stop
// Stops current operation.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::Stop()
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::Stop"));
    TInt status(KErrNone);
    // Cancel any outstanding requests
    Cancel();
    // Reset attributes
    iHwDeviceState = EHWDeviceIdle;
    iLastBufferReceived = EFalse;
    iCurPlayBuffer = NULL;
    iCurRecdBuffer = NULL;
    // Notify observer Stopped
    iObserver->Stopped();
    return status;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::Pause
// Pauses current operation.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::Pause()
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::Pause"));
    TInt status(KErrNone);

    // Fix to WAV recording issue.
    // Proper way to stop this A/O is by letting the data path to call back
    // Stop() after detecting last buffer.
//  Cancel();
    return status;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::Init
// Initializes CMMFHwDevice.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::Init(THwDeviceInitParams& aDevInfo)
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::Init"));
    TInt status(KErrNone);
    iObserver = aDevInfo.iHwDeviceObserver;
    return status;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::CustomInterface
// Returns a pointer to CustomInterface.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TAny* CMMFHwDeviceStub::CustomInterface(TUid /*aInterfaceId*/)
    {
    return NULL;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::ThisHwBufferFilled
// Returns a pointer to CustomInterface.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::ThisHwBufferFilled(CMMFBuffer& aFillBufferPtr)
    {
    DEBPRN2(_L("CMMFHwDeviceStub[0x%x]::ThisHwBufferFilled:Addr[0x%x]Count[%d]"), iCurPlayBuffer, ++iCount);
    TInt status(KErrNotReady);
    if (iHwDeviceState == EHWDevicePlay)
        {
        status = KErrNone;
        if (aFillBufferPtr.LastBuffer())
            {
            iLastBufferReceived = ETrue;
            DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::ThisHwBufferFilled[LastBuffer]"));
            }

        // If not already active, launch timer
        if (!IsActive())
            {
            iTimer.After(iStatus,TTimeIntervalMicroSeconds32(KTimerDuration) );
            SetActive();
            }
        }
    return status;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::ThisHwBufferEmptied
// Called by client when data is available during recording.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::ThisHwBufferEmptied(CMMFBuffer& aEmptyBufferPtr)
    {
    DEBPRN1(_L("CMMFHwDeviceStub[0x%x]::ThisHwBufferEmptied[%d]"), ++iCount);
    TInt status(KErrNotReady);
    if (iHwDeviceState == EHWDeviceRecord)
        {
        status = KErrNone;

        // Fix to WAV recording issue.
        // In case of real DevSound adaptation implementation, the
        // CMMFSwCodecRecordDataPath sets the last buffer parameter when no
        // more data is in the buffer to process. In case of the stub, this
        // never gets set as the s/w codec is not involved - we are simply
        // copying same fixed 4k block of data over and over again. So, on
        // pause or stop we need to indicate to the data path that we no
        // longer need processing of data by manually setting last buffer
        // parameter and resetting requested data size to 0.
        if (aEmptyBufferPtr.LastBuffer())
            {
            iRecdBuffer1->SetLastBuffer(ETrue);
            iRecdBuffer1->SetRequestSizeL(0);
            iRecdBuffer2->SetLastBuffer(ETrue);
            iRecdBuffer2->SetRequestSizeL(0);
            }

        // If not already active, launch timer
        if (!IsActive())
            {
            iTimer.After(iStatus,TTimeIntervalMicroSeconds32(KTimerDuration) );
            SetActive();
            }
        }

    return status;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::SetConfig
// Configures CMMFHwDevice.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::SetConfig(TTaskConfig& /*aConfig*/)
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::SetConfig"));
    TInt status(KErrNone);
    return status;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::StopAndDeleteCodec
// Stops and deletes codec.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::StopAndDeleteCodec()
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::StopAndDeleteCodec"));
    TInt status(KErrNone);
    Stop();
    return status;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::DeleteCodec
// Deletes codec.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::DeleteCodec()
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::DeleteCodec"));
    TInt status(KErrNone);
    Stop();
    return status;
    }


//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::RunL
// Called by CActive object framework when local timer times out.
// (other items were commented in a header).
//-----------------------------------------------------------------------------
void CMMFHwDeviceStub::RunL()
    {
    DEBPRN1(_L("CMMFHwDeviceStub[0x%x]::RunL:iHwDevState[%d]"), iHwDeviceState);
    switch(iHwDeviceState)
        {
        case EHWDevicePlay:
            // If last buffer is received, send error
            if (iLastBufferReceived)
                {
                iObserver->Error(KErrUnderflow);
                Stop();
                }
            else
                {
                SetActivePlayBufferL();
                iObserver->FillThisHwBuffer(*iCurPlayBuffer);
                }
            break;
        case EHWDeviceRecord:
            SetActiveRecdBufferL();
            iObserver->EmptyThisHwBuffer(*iCurRecdBuffer);
            break;
        default:
            break;
        }
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::DoCancel
// From CActive. Called by Framework when this instance is active and is
// cancelled
//-----------------------------------------------------------------------------
void CMMFHwDeviceStub::DoCancel()
    {
    DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::DoCancel"));
    iTimer.Cancel();
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::Error
// From CActive. Called by Framework when RunL Leaves
//-----------------------------------------------------------------------------
TInt CMMFHwDeviceStub::Error(TInt /*aError*/)
    {
    return KErrNone;
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::InitializePlayBufferL
// Initializes buffer(s) used for playback
//-----------------------------------------------------------------------------
void CMMFHwDeviceStub::InitializePlayBufferL()
    {
    if (!iPlayBuffer1)
        {
        // Create buffers
        iPlayBuffer1 = CMMFDataBuffer::NewL(KBufferLength1);
        }
    if (!iPlayBuffer2)
        {
        // Create buffers
        iPlayBuffer2 = CMMFDataBuffer::NewL(KBufferLength2);
        }
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::InitializeRecdBufferL
// Initializes buffer(s) used for recording
//-----------------------------------------------------------------------------
void CMMFHwDeviceStub::InitializeRecdBufferL()
    {
    if (!iRecdBuffer1)
        {
        DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::InitializeRecdBufferL:Creating buffer..."));
        // Create buffers
        iRecdBuffer1 = CMMFDataBuffer::NewL(KBufferLength1);
        }

    if (!iRecdBuffer2)
        {
        DEBPRN0(_L("CMMFHwDeviceStub[0x%x]::InitializeRecdBufferL:Creating buffer..."));
        // Create buffers
        iRecdBuffer2 = CMMFDataBuffer::NewL(KBufferLength2);
        }

    iPosition = 0;

    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::SetActiveRecdBufferL
// Reads data from the input file to the buffer
//-----------------------------------------------------------------------------
void CMMFHwDeviceStub::SetActiveRecdBufferL()
    {
    if ( !iCurRecdBuffer || ( iCurRecdBuffer == iRecdBuffer2 ) )
        {
        iCurRecdBuffer = iRecdBuffer1;
        }
    else
        {
        iCurRecdBuffer = iRecdBuffer2;
        }

    if ( !iCurRecdBuffer->LastBuffer() )
        {
        RFs rFs;
        RFile rFile;
        User::LeaveIfError(rFs.Connect());
        User::LeaveIfError(rFile.Open(rFs, KSample1, EFileRead));

        TInt size;
        User::LeaveIfError(rFile.Size(size));
        TInt bufLength( iCurRecdBuffer->Data().MaxLength() );
        if (iPosition > (size - bufLength))
            {
            iPosition = 0; //rewind file position index to the beginning
            }
        // Assumption, file size is more than iCurRecdBuffer->Data().MaxLength()
        User::LeaveIfError(rFile.Read(iPosition,
                                      iCurRecdBuffer->Data(),
                                      bufLength) );
        iCurRecdBuffer->SetRequestSizeL(bufLength);
        iCurRecdBuffer->SetLastBuffer(EFalse);
        rFile.Close();
        rFs.Close();

        iPosition += bufLength;
        }
    }

//-----------------------------------------------------------------------------
// CMMFHwDeviceStub::SetActivePlayBufferL
// Reads data from the input file to the buffer
//-----------------------------------------------------------------------------
void CMMFHwDeviceStub::SetActivePlayBufferL()
    {
    if ( !iCurPlayBuffer || ( iCurPlayBuffer == iPlayBuffer2 ) )
        {
        iCurPlayBuffer = iPlayBuffer1;
        }
    else
        {
        iCurPlayBuffer = iPlayBuffer2;
        }
    iCurPlayBuffer->SetRequestSizeL(iCurPlayBuffer->Data().MaxLength());
    iCurPlayBuffer->Data().SetLength(0);
    iCurPlayBuffer->SetLastBuffer(EFalse);
    }

//End of File
