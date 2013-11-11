/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  DevSound adaptation stub body implementation.
*
*/


// INCLUDE FILES
#include <e32base.h>
#include <mdaaudiotoneplayer.h>

#include "G711DecoderIntfc.h"
#include "G729DecoderIntfc.h"
#include "IlbcDecoderIntfc.h"
#include "G711EncoderIntfc.h"
#include "G729EncoderIntfc.h"
#include "IlbcEncoderIntfc.h"
#include "ErrorConcealmentIntfc.h"
#include "SpeechEncoderConfig.h"

#include "AudioInputMessageTypes.h"
#include "AudioOutputMessageTypes.h"
//#include "AudioResourceMessageTypes.h"

#include "AudioInputCI.h"
#include "AudioOutputCI.h"
#include "G711DecoderIntfcCI.h"
#include "G729DecoderIntfcCI.h"
#include "IlbcDecoderIntfcCI.h"
#include "G711EncoderIntfcCI.h"
#include "G729EncoderIntfcCI.h"
#include "IlbcEncoderIntfcCI.h"
#include "ErrorConcealmentIntfcCI.h"
#include "SpeechEncoderConfigCI.h"

//#include <AudioResourceCIStub.h>
//#include <AudioEqualizerCI.h>
//#include <EnvironmentalReverbCI.h>
//#include <StereoWideningCI.h>
//#include <BassBoostCI.h>
//#include <SourceDopplerBase.h>
//#include <ListenerDopplerBase.h>
//#include <SourceDopplerCI.h>
//#include <ListenerDopplerCI.h>
//#include <ListenerLocationCI.h>
//#include <SourceLocationCI.h>
//#include <ListenerOrientationCI.h>
//#include <SourceOrientationCI.h>
//#include <DistanceAttenuationCI.h>
//#include <LoudnessCI.h>

//#include <AddedDevSoundControlCI.h>
//#include <AddedDevSoundControlCIStub.h>
//#include <RestrictedAudioOutputCI.h>

#include "MmfDevSoundAdaptationBody.h"
#include "MmfHwDeviceStub.h"
#include "TonePlayCompleteTimer.h"


// CONSTANTS
#ifdef _DEBUG
#include "e32debug.h"

#define DEBPRN0(str)                RDebug::Print(str, this)
#define DEBPRN1(str, val1)          RDebug::Print(str, this, val1)
#define DEBPRN2(str, val1, val2)    RDebug::Print(str, this, val1, val2)
#else
#define DEBPRN0(str)
#define DEBPRN1(str, val1)
#define DEBPRN2(str, val1, val2)
#endif //_DEBUG


const TUint KMaxVolume = 10;
const TUint KToneSamplingRate = 8000;
const TUint KToneChannels = 2;
// Time to play one note 1/10th of 1/2 a second
const TUint KToneNotePlayTime = 50000;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::CBody
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CMMFDevSoundAdaptation::CBody::CBody()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CBody"));
    iMode= EMMFStateIdle;
    //Set reasonable default values for DTMF
    iDTMFGen.SetToneDurations(250000,50000,250000);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::ConstructL
// Symbian 2nd phase constructor can leave.
// assumes that iParent has already been set up properly
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::ConstructL(
    RServer2& /*aPolicyServerHandle*/)
    {
    // Default
    // set the default capability
    iDeviceCapabilities.iRate = EMMFSampleRate8000Hz;
    iDeviceCapabilities.iEncoding = EMMFSoundEncoding16BitPCM;
    iDeviceCapabilities.iChannels = EMMFMono;
    iDeviceCapabilities.iBufferSize = KBufferLength;

    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::ConstructL:EXIT"));
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CMMFDevSoundAdaptation::CBody* CMMFDevSoundAdaptation::CBody::NewL()
    {
    CMMFDevSoundAdaptation::CBody* self = new (ELeave) CBody;
    return self;
    }


// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBodye::~CBody
// Destructor
// -----------------------------------------------------------------------------
//
CMMFDevSoundAdaptation::CBody::~CBody()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::~CBody:ENTER"));
    delete iToneBuffer1;
    delete iToneBuffer2;
    delete iDevSoundUtil;
    delete iFixedSequences;
    delete iCMMFHwDevice;
    delete iTonePlayCompleteTimer;
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::~CBody:EXIT"));
    }


// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::InitializeL
// Initializes CMMFDevSoundProxy object to play and record PCM16 raw audio data
// with sampling rate of 8 KHz.
//
// On completion of Initialization, calls InitializeComplete() on
// aDevSoundObserver.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::InitializeL(
    MDevSoundAdaptationObserver& aDevSoundObserver,
    TMMFState aMode)
    {
    // if no HwDevice id specified, load default null implementation
    TUid rawUid = {0};
    InitializeL(aDevSoundObserver, rawUid, aMode);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::InitializeL
// Initializes DevSound object for the mode aMode for processing audio data
// with hardware device aHWDev.
//
// On completion of Initialization, the observer will be notified via call back
// InitializeComplete().
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::InitializeL(
    MDevSoundAdaptationObserver& aDevSoundObserver,
    TUid aHWDev,
    TMMFState aMode)
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::InitializeL:ENTER"));
    TInt initError = KErrNone;
    iDevSoundObserver = &aDevSoundObserver;

    if (aMode == EMMFStateIdle)
        {
        User::Leave(KErrNotSupported);
        }
    iMode= aMode;


    iDevSoundObserver = &aDevSoundObserver;
    iHwDeviceID.iUid = aHWDev.iUid;
    if(iCMMFHwDevice)
        {
        delete iCMMFHwDevice;
        iHwDeviceBuffer = NULL; // buffer is deleted by HwDevice delete
        }

    iCMMFHwDevice = NULL;
    iCMMFHwDevice = CMMFHwDeviceStub::NewL();

    iDevInfo.iHwDeviceObserver = this;
    initError = iCMMFHwDevice->Init(iDevInfo);

    iDevSoundObserver->InitializeComplete(initError);

    if (initError)
        {
        User::Leave(initError);
        }
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::InitializeL:EXIT"));
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::InitializeL
// Initializes DevSound object for the mode aMode for processing audio data
// with hardware device supporting FourCC aDesiredFourCC.
//
// On completion of Initialization, the observer will be notified via call back
// InitializeComplete().
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::InitializeL(
    MDevSoundAdaptationObserver& aDevSoundObserver,
    TFourCC /*aDesiredFourCC*/,
    TMMFState aMode)
    {
    TUid implUid = {0};
    InitializeL(aDevSoundObserver, implUid, aMode);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::Capabilities
// Returns the supported Audio settings.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TMMFCapabilities CMMFDevSoundAdaptation::CBody::Capabilities()
    {
    return iDeviceCapabilities;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::Config
// Returns the current audio settings.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TMMFCapabilities CMMFDevSoundAdaptation::CBody::Config() const
    {
    return iDeviceConfig;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetConfigL
// Configure CMMFDevSoundProxy object for the settings in aConfig.
// Use this to set sampling rate, Encoding and Mono/Stereo.
// As part of defect 20796, the iRecordFormat has been set under the iPlayFormat,
//  before it was not set at all.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetConfigL(
    const TMMFCapabilities& aConfig)
    {
    iDeviceConfig = aConfig;

    // Fix to WAV recording problem.
    // A kludge to init channel number to 'something' in case the device returns 0.
    if (!iDeviceConfig.iChannels)
        {
        iDeviceConfig.iChannels = EMMFMono;
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::MaxVolume
// Returns an integer representing the maximum volume.
// This is the maximum value which can be passed to CMMFDevSoundProxy::SetVolume.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::MaxVolume()
    {
    return KMaxVolume;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::Volume
// Returns an integer representing the current volume.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::Volume()
    {
    return iVolume;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetVolume
// Changes the current playback volume to a specified value.
// The volume can be changed before or during playback and is effective
// immediately.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetVolume(
    TInt aVolume)
    {
    // Check and make sure that the volume is in valid range
    if (aVolume < 0)
        {
        aVolume = 0;
        }
    if (aVolume > MaxVolume())
        {
        aVolume = MaxVolume();
        }
    iVolume = aVolume;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::MaxGain
// Returns an integer representing the maximum gain.
// This is the maximum value which can be passed to CMMFDevSoundProxy::SetGain.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::MaxGain()
    {
    return KMaxVolume;//uses iMaxVolume for iMaxGain
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::Gain
// Returns an integer representing the current gain.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::Gain()
    {
    return iGain;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetGain
// Changes the current recording gain to a specified value.
//
// The gain can be changed before or during recording and is effective
// immediately.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetGain(TInt aGain)
    {
    // make sure it falls with the correct range
    if (aGain > MaxGain())
        {
        aGain = MaxGain();
        }
    else if (aGain < 0)
        {
        aGain = 0;
        }
    iGain = aGain;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::GetPlayBalanceL
// Returns the speaker balance set for playing.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::GetPlayBalanceL(
    TInt& aLeftPercentage,
    TInt& aRightPercentage)
    {
    aLeftPercentage = iLeftPlayBalance;
    aRightPercentage = iRightPlayBalance;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetPlayBalanceL
// Sets the speaker balance for playing.
// The speaker balance can be changed before or during playback and is
// effective immediately.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetPlayBalanceL(
    TInt aLeftPercentage,
    TInt aRightPercentage)
    {
    if (aLeftPercentage < 0)
        {
        aLeftPercentage = 0;
        }
    else if (aLeftPercentage > 100)
        {
        aLeftPercentage = 100;
        }
    if (aRightPercentage < 0)
        {
        aRightPercentage = 0;
        }
    else if (aRightPercentage > 100)
        {
        aRightPercentage = 100;
        }
    iLeftPlayBalance = aLeftPercentage;
    iRightPlayBalance = aRightPercentage;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::GetRecordBalanceL
// Returns the microphone gain balance set for recording.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::GetRecordBalanceL(
    TInt& aLeftPercentage,
    TInt& aRightPercentage)
    {
    aLeftPercentage = iLeftRecordBalance;
    aRightPercentage = iRightRecordBalance;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetRecordBalanceL
// Sets the microphone gain balance for recording.
// The microphone gain balance can be changed before or during recording and
// is effective immediately.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetRecordBalanceL(
    TInt aLeftPercentage,
    TInt aRightPercentage)
    {
    if (aLeftPercentage < 0)
        {
        aLeftPercentage = 0;
        }
    else if (aLeftPercentage > 100)
        {
        aLeftPercentage = 100;
        }
    if (aRightPercentage < 0)
        {
        aRightPercentage = 0;
        }
    else if (aRightPercentage > 100)
        {
        aRightPercentage = 100;
        }
    iLeftRecordBalance = aLeftPercentage;
    iRightRecordBalance = aRightPercentage;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::PlayInitL
// Initializes audio device and start play process. This method queries and
// acquires the audio policy before initializing audio device. If there was an
// error during policy initialization, PlayError() method will be called on
// the observer with error code KErrAccessDenied, otherwise BufferToBeFilled()
// method will be called with a buffer reference. After reading data into the
// buffer reference passed, the client should call PlayData() to play data.
//
// The amount of data that can be played is specified in
// CMMFBuffer::RequestSize(). Any data that is read into buffer beyond this
// size will be ignored.
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::PlayInitL()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::PlayInitL"));
    if (!iDevSoundObserver)
        {
        User::Leave(KErrNotReady);
        }

    StartPlayDataL();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::RecordInitL
// Initializes audio device and start record process. This method queries and
// acquires the audio policy before initializing audio device. If there was an
// error during policy initialization, RecordError() method will be called on
// the observer with error code KErrAccessDenied, otherwise BufferToBeEmptied()
// method will be called with a buffer reference. This buffer contains recorded
// or encoded data. After processing data in the buffer reference passed, the
// client should call RecordData() to continue recording process.
//
// The amount of data that is available is specified in
// CMMFBuffer::RequestSize().
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::RecordInitL()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::RecordInitL"));
    if (!iDevSoundObserver)
        {
        User::Leave(KErrNotReady);
        }

    StartRecordDataL();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::PlayData
// Plays data in the buffer at the current volume. The client should fill
// the buffer with audio data before calling this method. The Observer gets
// reference to buffer along with callback BufferToBeFilled(). When playing of
// the audio sample is complete, successfully or otherwise, the method
// PlayError() on observer is called.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::PlayData()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::PlayData"));
    ASSERT(iDevSoundObserver);

    if (iMode== EMMFStateIdle)
        {
        return;
        }

    TInt error = KErrNone;

    if(iCMMFHwDevice)
        {
        if (iPaused)
            {
            iPaused = EFalse;
            //note PlayData does not leave or return an error code so the
            //Start() fails we cannot report the error back at this point
            if (iCMMFHwDevice->IsActive())
                {
                iCMMFHwDevice->Cancel();
                }
            //restart hw device after pause
            error = iCMMFHwDevice->Start(EDevDecode, EDevOutFlow);
            }
       else if(iMode== EMMFStatePlaying)
            {
            TInt len = iHwDeviceBuffer->Data().Length();
            iPlayedBytesCount += len;
            if (iHwDeviceBuffer->LastBuffer())
                {
                iLastBufferReceived = ETrue;
                }

            // Pass the data buffer to HwDevice
            if (iMode== EMMFStatePlaying)
                {
                error = iCMMFHwDevice->ThisHwBufferFilled(*iHwDeviceBuffer);
                }
            }
        }
    if (error != KErrNone)
        {
        iDevSoundObserver->PlayError(error);
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::RecordData
// Contine the process of recording. Once the buffer is filled with recorded
// data, the Observer gets reference to buffer along with callback
// BufferToBeEmptied(). After processing the buffer (copying over to a
// different buffer or writing to file) the client should call this
// method to continue recording process.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::RecordData()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::RecordData"));
    ASSERT(iDevSoundObserver);
    if(iCMMFHwDevice)
        {
        if(iMode == EMMFStateRecording)
            {
            // Fix to WAV recording issue.
            // In case of real DevSound adaptation implementation, the
            // CMMFSwCodecRecordDataPath sets the last buffer parameter when no
            // more data is in the buffer to process. In case of the stub, this
            // never gets set as the s/w codec is not involved - we are simply
            // copying same fixed 4k block of data over and over again. So, on
            // pause or stop we need to indicate to the data path that we no
            // longer need processing of data by manually setting last buffer
            // parameter and resetting requested data size to 0.
            if (iPaused)
                {
                iHwDeviceBuffer->SetLastBuffer(ETrue);
                iHwDeviceBuffer->Data().SetLength(0);
                }
            else
                {
                iHwDeviceBuffer->Data().SetLength(iHwDeviceBuffer->RequestSize());
                }

            iCMMFHwDevice->ThisHwBufferEmptied(*iHwDeviceBuffer);
            }
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::Stop
// Stops the ongoing operation (Play, Record, TonePlay, Convert)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::Stop()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::Stop"));

    iPaused = EFalse;

    if (iMode== EMMFStateIdle)
        {
        return;
        }

    // For custom interface

     // Stop the hw device first - this unloads sound drivers
    if(iCMMFHwDevice)
        {
        iCMMFHwDevice->Stop();
        }

    if ((iMode== EMMFStateTonePlaying) && (iTonePlayCompleteTimer))
        {
        iTonePlayCompleteTimer->Cancel();
        }

    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::Pause
// Temporarily Stops the ongoing operation (Play, Record, TonePlay, Convert)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::Pause()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::Pause"));
    iPaused = ETrue;

    if (iMode== EMMFStateIdle)
        {
        return;
        }

    // Pause the HW device first
    if(iCMMFHwDevice)
        {
        iCMMFHwDevice->Pause();
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SamplesRecorded
// Returns the sample recorded so far.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::SamplesRecorded()
    {
    TInt samples = 0;
    samples = iRecordedBytesCount;
    if(NumberOfChannels() > 1)
        {
        samples /= NumberOfChannels();
        }
    if(BytesPerAudioSample() > 1)
        {
        samples /= BytesPerAudioSample();
        }
    return samples;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SamplesPlayed
// Returns the sample played so far.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::SamplesPlayed()
    {
    TInt samples = 0;
    samples = iPlayedBytesCount;
    if(NumberOfChannels() > 1)
        {
        samples /= NumberOfChannels();
        }

    if(BytesPerAudioSample() > 1)
        {
        samples /= BytesPerAudioSample();
        }
    return samples; //each sample is 2 bytes
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::PlayToneL
// Initializes audio device and start playing tone. Tone is played with
// frequency and for duration specified.
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::PlayToneL(
    TInt aFrequency,
    const TTimeIntervalMicroSeconds& aDuration)
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::PlayToneL"));
    if (iMode!= EMMFStateTonePlaying)
        {
        //tone playing only supported in tone play state
        User::Leave(KErrNotSupported);
        }
    // Check whether frequency and duration is valid or not
    TInt64 zeroInt64(0);
    if ((aFrequency<0) || (aDuration.Int64() < zeroInt64))
        {
        User::Leave(KErrArgument);
        }
    if (!iDevSoundObserver)
        {
        User::Leave(KErrNotReady);
        }
    iToneGen.SetFrequencyAndDuration(aFrequency,aDuration);

    StartPlayToneL();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::PlayDualToneL
// Initializes audio device and start playing a dual tone.
// The tone consists of two sine waves of different frequencies summed together
// Dual Tone is played with specified frequencies and for specified duration.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::PlayDualToneL(
    TInt aFrequencyOne,
    TInt aFrequencyTwo,
    const TTimeIntervalMicroSeconds& aDuration)
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::PlayDualToneL"));

    // Check whether frequencies and duration are valid or not
    TInt64 zeroInt64(0);
    if ((aFrequencyOne<0) || (aFrequencyTwo<0) ||
        (aDuration.Int64() < zeroInt64))
        {
        User::Leave(KErrArgument);
        }
    if (!iDevSoundObserver)
        {
        User::Leave(KErrNotReady);
        }
    iDualToneGen.SetFrequencyAndDuration(aFrequencyOne,
                                         aFrequencyTwo,
                                         aDuration);
    StartPlayDualToneL();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::PlayDTMFStringL
// Initializes audio device and start playing DTMF string aDTMFString.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::PlayDTMFStringL(
    const TDesC& aDTMFString)
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::PlayDTMFStringL"));

    if (!iDevSoundObserver)
        {
        User::Leave(KErrNotReady);
        }
    if (iMode!= EMMFStateTonePlaying)
        {
        //tone playing only supported in tone play state
        User::Leave(KErrNotSupported);
        }
    iDTMFGen.SetString(aDTMFString);
    StartPlayDTMFStringL();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::PlayToneSequenceL
// Initializes audio device and start playing tone sequence.
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::PlayToneSequenceL(const TDesC8& aData)
    {
    DEBPRN1(
    _L("CMMFDevSoundAdaptationStub[0x%x]::CBody::PlayToneSequenceL:Length[%d]"),
    aData.Length());

    if (!iDevSoundObserver)
        {
        User::Leave(KErrNotReady);
        }
    if (iMode!= EMMFStateTonePlaying)
        {
        //tone playing only supported in tone play state
        User::Leave(KErrNotSupported);
        }

    InitializeDevSoundUtilL();
    // Check whether the sequence is valid or not
    if (!iDevSoundUtil->RecognizeSequence(aData))
        {
        User::Leave(KErrCorrupt);
        }

    // For playing Tone sequence, we don't use PCM generator.
    // We use a timer instead
    // iSequenceGen.SetSequenceData(aData);

    if (!iTonePlayCompleteTimer)
        {
        iTonePlayCompleteTimer =
            CTonePlayCompleteTimer::NewL(*iDevSoundObserver);
        }
    // Determine the time out based on iRepeatCount and number of notes
    // in the sequence
    TUint repeats = ((iRepeatCount > 0) ? iRepeatCount : 1);
    TTimeIntervalMicroSeconds32 time(KToneNotePlayTime*aData.Length()*repeats);
    iTonePlayCompleteTimer->SetTimeOut(time);

    StartPlayToneSequenceL();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::PlayFixedSequenceL
// Initializes audio device and start playing the specified pre-defined tone
// sequence.
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::PlayFixedSequenceL(TInt aSequenceNumber)
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::PlayFixedSequenceL"));

    if (!iDevSoundObserver)
        {
        User::Leave(KErrNotReady);
        }
    if (iMode!= EMMFStateTonePlaying)
        {
        //tone playing only supported in tone play state
        User::Leave(KErrNotSupported);
        }
    ASSERT((aSequenceNumber >= 0) &&
           (aSequenceNumber < iFixedSequences->Count()));

    iFixedSequence.Set(iFixedSequences->MdcaPoint(aSequenceNumber));
    iSequenceGen.SetSequenceData(iFixedSequence);

    StartPlayToneSequenceL();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetToneRepeats
// Defines the number of times the audio is to be repeated during the tone
// playback operation. A period of silence can follow each playing of tone.
// The tone playing can be repeated indefinitely.
// Supported only during tone playing.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetToneRepeats(
    TInt aRepeatCount,
    const TTimeIntervalMicroSeconds& aRepeatTrailingSilence)
    {
    iRepeatCount = aRepeatCount;
    iRepeatTrailingSilence = aRepeatTrailingSilence;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetDTMFLengths
// Defines the duration of tone on, tone off and tone pause to be used during the
// DTMF tone playback operation.
// Supported only during tone playing.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetDTMFLengths(
    TTimeIntervalMicroSeconds32& aToneOnLength,
    TTimeIntervalMicroSeconds32& aToneOffLength,
    TTimeIntervalMicroSeconds32& aPauseLength)
    {

    if(aToneOnLength.Int() < KMdaInfiniteDurationDTMFToneOnLength)
        {
        aToneOnLength = TTimeIntervalMicroSeconds32(0);
        }
    if(aToneOffLength.Int() < 0)
        {
        aToneOffLength = TTimeIntervalMicroSeconds32(0);
        }
    if(aPauseLength.Int() < 0)
        {
        aPauseLength = TTimeIntervalMicroSeconds32(0);
        }

    iDTMFGen.SetToneDurations(aToneOnLength,aToneOffLength,aPauseLength);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetVolumeRamp
// Defines the period over which the volume level is to rise smoothly from
// nothing to the normal volume level.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetVolumeRamp(
    const TTimeIntervalMicroSeconds& aRampDuration)
    {
    // save ramp duration for tone generator
    iRampDuration = aRampDuration;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetPrioritySettings
// Defines the priority settings that should be used for this instance.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetPrioritySettings(
    const TMMFPrioritySettings& /*aPrioritySettings*/)
    {
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::CustomInterface
// @see sounddevice.h
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TAny* CMMFDevSoundAdaptation::CBody::CustomInterface(TUid aInterfaceId)
    {
// Note: These can only be uncommented when supported by the
//       MessageHandlerFactory and CustomInterfaceProxyFactory stubs.
//       Will result in memory leak if re-enabled without updating of
//       the proxy and message factory stubs.
//
    DEBPRN1(
    _L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:InterfaceId[0x%x]"),
    aInterfaceId);

    if (aInterfaceId == KUidSpeechEncoderConfig)
        {
        TRAP_IGNORE(iSpeechEncoderConfigCI = CSpeechEncoderConfigCI::NewL());
        DEBPRN0(
        _L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CSpeechEncoderConfigCI..."));
        return iSpeechEncoderConfigCI;
        }
    else if (aInterfaceId == KUidErrorConcealmentIntfc)
        {
        TRAP_IGNORE(iErrorConcealmentIntfcCI = CErrorConcealmentIntfcCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CErrorConcealmentIntfcCI..."));
        return iErrorConcealmentIntfcCI;
        }
    else if (aInterfaceId == KUidG711DecoderIntfc)
        {
        TRAP_IGNORE(iG711DecoderIntfcCI = CG711DecoderIntfcCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CG711DecoderIntfcCI..."));
        return iG711DecoderIntfcCI;
        }
    else if (aInterfaceId == KUidG729DecoderIntfc)
        {
        TRAP_IGNORE(iG729DecoderIntfcCI = CG729DecoderIntfcCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CG729DecoderIntfcCI..."));
        return iG729DecoderIntfcCI;
        }
    else if (aInterfaceId == KUidIlbcDecoderIntfc)
        {
        TRAP_IGNORE(iIlbcDecoderIntfcCI = CIlbcDecoderIntfcCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CIlbcDecoderIntfcCI..."));
        return iIlbcDecoderIntfcCI;
        }
    else if (aInterfaceId == KUidG711EncoderIntfc)
        {
        TRAP_IGNORE(iG711EncoderIntfcCI = CG711EncoderIntfcCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CG711EncoderIntfcCI..."));
        return iG711EncoderIntfcCI;
        }
    else if (aInterfaceId == KUidG729EncoderIntfc)
        {
        TRAP_IGNORE(iG729EncoderIntfcCI = CG729EncoderIntfcCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CG729EncoderIntfcCI..."));
        return iG729EncoderIntfcCI;
        }
    else if (aInterfaceId == KUidIlbcEncoderIntfc)
        {
        TRAP_IGNORE(iIlbcEncoderIntfcCI = CIlbcEncoderIntfcCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CIlbcEncoderIntfcCI..."));
        return iIlbcEncoderIntfcCI;
        }
    else if (aInterfaceId == KUidAudioInput)
        {
        CAudioInputCI* retCI(NULL);
        TRAP_IGNORE(retCI = CAudioInputCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CAudioInputCI..."));
        return retCI;
        }
    else if(aInterfaceId == KUidAudioOutput)
        {
        CAudioOutputCI* retCI(NULL);
        TRAP_IGNORE(retCI = CAudioOutputCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CAudioOutputCI..."));
        return retCI;
        }/*
    else if (aInterfaceId == KUidSbcEncoderIntfc)
        {
        TRAP_IGNORE(iSbcEncoderIntfcCI = CSbcEncoderIntfcCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CSbcEncoderIntfcCI..."));
        return iSbcEncoderIntfcCI;
        }
    else if (aInterfaceId == KUidAudioVibraControl)
        {
        TRAP_IGNORE(iAudioVibraControlCI = CAudioVibraControlCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CAudioVibraControlCI..."));
        return iAudioVibraControlCI;
        }
    else if (aInterfaceId == KUidAudioResource)
        {
        CAudioResourceCIStub* retCI(NULL);
        TRAP_IGNORE(retCI = CAudioResourceCIStub::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CAudioResourceCIStub..."));
        return retCI;
        }
    else if (aInterfaceId == KUidAudioEqualizerEffect)
        {
        CAudioEqualizerCI* retCI(NULL);
        TRAP_IGNORE(retCI = CAudioEqualizerCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CAudioEqualizerCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidEnvironmentalReverbEffect)
        {
        CEnvironmentalReverbCI* retCI(NULL);
        TRAP_IGNORE(retCI = CEnvironmentalReverbCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CEnvironmentalReverbCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidStereoWideningEffect)
        {
        CStereoWideningCI* retCI(NULL);
        TRAP_IGNORE(retCI = CStereoWideningCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CStereoWideningCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidBassBoostEffect)
        {
        CBassBoostCI* retCI(NULL);
        TRAP_IGNORE(retCI = CBassBoostCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CBassBoostCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidSourceDopplerEffect)
        {
        CSourceDopplerCI* retCI(NULL);
        TRAP_IGNORE(retCI = CSourceDopplerCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CSourceDopplerCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidListenerDopplerEffect)
        {
        CListenerDopplerCI* retCI(NULL);
        TRAP_IGNORE(retCI = CListenerDopplerCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CListenerDopplerCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidListenerLocationEffect)
        {
        CListenerLocationCI* retCI(NULL);
        TRAP_IGNORE(retCI = CListenerLocationCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CListenerLocationCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidSourceLocationEffect)
        {
        CSourceLocationCI* retCI(NULL);
        TRAP_IGNORE(retCI = CSourceLocationCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CSourceLocationCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidListenerOrientationEffect)
        {
        CListenerOrientationCI* retCI(NULL);
        TRAP_IGNORE(retCI = CListenerOrientationCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CListenerOrientationCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidSourceOrientationEffect)
        {
        CSourceOrientationCI* retCI(NULL);
        TRAP_IGNORE(retCI = CSourceOrientationCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CSourceOrientationCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidDistanceAttenuationEffect)
        {
        CDistanceAttenuationCI* retCI(NULL);
        TRAP_IGNORE(retCI = CDistanceAttenuationCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CDistanceAttenuationCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidLoudnessEffect)
        {
        CLoudnessCI* retCI(NULL);
        TRAP_IGNORE(retCI = CLoudnessCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CLoudnessCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidAddedDevSoundControlInterface)
        {
        CAddedDevSoundControlCI* retCI(NULL);
        TRAP_IGNORE(retCI = CAddedDevSoundControlCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CAddedDevSoundControlCI..."));
        return (MAddedDevSoundControl*)retCI;
        }
    else if (aInterfaceId == KUidRestrictedAudioOutput)
        {
        CRestrictedAudioOutputCI* retCI(NULL);
        TRAP_IGNORE(retCI = CRestrictedAudioOutputCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CRestrictedAudioOutputCI..."));
        return retCI;
        }
    else if (aInterfaceId == KUidAacDecoderConfig)
        {
        TRAP_IGNORE(iAacDecoderConfigCI = CAacDecoderConfigCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CAacDecoderConfigCI..."));
        return iAacDecoderConfigCI;
        }
    else if (aInterfaceId == KUidEAacPlusDecoderIntfc)
        {
        TRAP_IGNORE(iEAacPlusDecoderConfigCI = CEAacPlusDecoderIntfcCI::NewL());
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning CEAacPlusDecoderIntfcCI..."));
        return iEAacPlusDecoderConfigCI;
        }*/
    else
        {
        DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::CustomInterface:returning NULL..."));
        return NULL;
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::FixedSequenceCount
// Returns the number of available pre-defined tone sequences.
// This is the number of fixed sequence supported by DevSound by default.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::FixedSequenceCount()
    {
    return iFixedSequences->Count();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::FixedSequenceName
// Returns the name assigned to a specific pre-defined tone sequence.
// This is the number of fixed sequence supported by DevSound by default.
// The function raises a panic if sequence number specified invalid.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
const TDesC& CMMFDevSoundAdaptation::CBody::FixedSequenceName(
    TInt aSequenceNumber)
    {
    ASSERT((aSequenceNumber >= 0) &&
           (aSequenceNumber < iFixedSequences->Count()));

    TRAPD(err, InitializeDevSoundUtilL());
    if (err == KErrNone)
        {
        return iDevSoundUtil->FixedSequenceName(aSequenceNumber);
        }
    else
        {
        return KNullDesC;
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::GetSupportedInputDataTypesL
// @see sounddevice.h
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::GetSupportedInputDataTypesL(
    RArray<TFourCC>& aSupportedDataTypes,
    const TMMFPrioritySettings& /*aPrioritySettings*/) const
    {
    //aPrioritySettings not used on ref DevSound
    //search for playing datatypes
    InitializeDevSoundUtilL();
    iDevSoundUtil->SeekHwDevicePluginsL(aSupportedDataTypes, EMMFStatePlaying);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::GetSupportedOutputDataTypesL
// @see sounddevice.h
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::GetSupportedOutputDataTypesL(
    RArray<TFourCC>& aSupportedDataTypes,
    const TMMFPrioritySettings& /*aPrioritySettings*/) const
    {
    //aPrioritySettings not used on ref DevSound
    // search for recording datatypes
    InitializeDevSoundUtilL();
    iDevSoundUtil->SeekHwDevicePluginsL(aSupportedDataTypes,
                                        EMMFStateRecording);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetClientConfig
// Sets client capabilities for this instance of DevSound Adaptation.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetClientConfig(
    const TMMFClientConfig& aClientConfig)
    {
    iClientConfig = aClientConfig;
    }

// -----------------------------------------------------------------------------
// TMMFClientConfig& CMMFDevSoundAdaptation::CBody::ClientConfig
// Returns client capabilities of this instance of DevSound Adaptation.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
const TMMFClientConfig& CMMFDevSoundAdaptation::CBody::ClientConfig() const
    {
    return iClientConfig;
    }

/********************************************************************************
 *              Implementations of Non Exported public functions begins here    *
 ********************************************************************************/

//////////////////////////////////////////////////////////////////////////////////
//              Audio Policy specific implementation begins here                //
//////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::StartPlayDataL
// Called by Audio Policy Server when a request to play is approved by the
// Audio Policy Server.
//
// Leaves on failure??.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::StartPlayDataL()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::StartPlayDataL"));

    ASSERT(iMode== EMMFStatePlaying);

    TInt error = KErrNone;

    if(iCMMFHwDevice)
        {
        // Initialize attribute values
        iPlayedBytesCount = 0;
        iLastBufferReceived = EFalse;

        // Start HwDevice
        if (iCMMFHwDevice->IsActive())
            {
            iCMMFHwDevice->Cancel();
            }
        error = iCMMFHwDevice->Start(EDevDecode, EDevOutFlow);
        }
    else
        {
        error = KErrNotReady;
        }

    if (error != KErrNone)
        {
        iDevSoundObserver->PlayError(error);
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::StartRecordDataL
// Called by Audio Policy Server when a request to record is approved by the
// Audio Policy Server.
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::StartRecordDataL()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::StartRecordDataL"));

    ASSERT(iMode== EMMFStateRecording);

     if(iCMMFHwDevice)
        {
        TInt error = KErrNone;
        // Initialize attribute values
        iRecordedBytesCount = 0;

        if (iCMMFHwDevice->IsActive())
            {
            iCMMFHwDevice->Cancel();
            }
        error = iCMMFHwDevice->Start(EDevEncode, EDevInFlow);

        if (iHwDeviceBuffer)
            {
            iHwDeviceBuffer->SetLastBuffer(EFalse);
            }
        if (error != KErrNone)
            {
            iDevSoundObserver->RecordError(error);
            return;
            }
        }
    else
        {
        iDevSoundObserver->RecordError(KErrNotReady);
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::StartPlayToneL
// Called by Audio Policy Server when a request to play tone is approved by
// the Audio Policy Server.
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::StartPlayToneL()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::StartPlayToneL"));

    ASSERT(iMode== EMMFStateTonePlaying);

     if(iCMMFHwDevice)
        {
        // Initialize attribute values
        iPlayedBytesCount = 0;

        // Configure tone generator
        iToneGen.Configure(
            KToneSamplingRate,
            KToneChannels,
            iRepeatCount,
            I64LOW((iRepeatTrailingSilence.Int64()*KToneSamplingRate)/1000000),
            I64LOW((iRampDuration.Int64()*KToneSamplingRate)/1000000)
            );

        iCurrentGenerator = &iToneGen;

        // Start playback
        DoPlayL();

        }
    else
        {
        iDevSoundObserver->ToneFinished(KErrNotReady);
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::StartPlayDualToneL
// Called by Audio Policy Server when a request to play a dual tone is approved
// by the Audio Policy Server.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::StartPlayDualToneL()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::StartPlayDualToneL"));

    ASSERT(iMode== EMMFStateTonePlaying);

     if(iCMMFHwDevice)
        {
        // Initialize attribute values
        iPlayedBytesCount = 0;

        // Configure dual tone generator
        iDualToneGen.Configure(
            KToneSamplingRate,
            KToneChannels,
            iRepeatCount,
            I64LOW((iRepeatTrailingSilence.Int64()
                    *KToneSamplingRate)/KOneMillionMicroSeconds),
            I64LOW((iRampDuration.Int64()
                    *KToneSamplingRate)/KOneMillionMicroSeconds)
            );

        iCurrentGenerator = &iDualToneGen;

        // Start playback
        DoPlayL();
        }
    else
        iDevSoundObserver->ToneFinished(KErrNotReady);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::StartPlayDTMFStringL
// Called by Audio Policy Server when a request to play DTMF String is approved
// by the Audio Policy Server.
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::StartPlayDTMFStringL()
    {
    DEBPRN0(
    _L("CMMFDevSoundAdaptationStub[0x%x]::CBody::StartPlayDTMFStringL"));

    ASSERT(iMode== EMMFStateTonePlaying);

    if(iCMMFHwDevice)
        {
        TInt error = KErrNone;
        // Initialize attribute values
        iPlayedBytesCount = 0;

        iDTMFGen.Configure(
            KToneSamplingRate,
            KToneChannels,
            iRepeatCount,
            I64LOW((iRepeatTrailingSilence.Int64()*KToneSamplingRate)/1000000),
            I64LOW((iRampDuration.Int64()*KToneSamplingRate)/1000000)
            );

        iCurrentGenerator = &iDTMFGen;

        // Start playback
        //need to trap this as we can leave with KErrUnderflow
        //if there was no data to play - the error has already
        //been sent to the observer and we don't want to call RunError
        TRAP(error,DoPlayL());
        if ((error != KErrUnderflow)&&(error != KErrNone))
            {
            User::Leave(error);
            }
        }
    else
        {
        iDevSoundObserver->ToneFinished(KErrNotReady);
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::StartPlayToneSequenceL
// Called by Audio Policy Server when a request to play tone sequence is
// approved by the Audio Policy Server.
//
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::StartPlayToneSequenceL()
    {
    DEBPRN0(
    _L("CMMFDevSoundAdaptationStub[0x%x]::CBody::StartPlayToneSequenceL"));

    ASSERT(iMode == EMMFStateTonePlaying);

    // For playing Tone sequence, we don't use PCM generator.
    // We use a timer instead
/*
    if(iCMMFHwDevice)
        {
        // Initialize attribute values
        iPlayedBytesCount = 0;

        iSequenceGen.Configure(
            KToneSamplingRate,
            KToneChannels,
            iRepeatCount,
            I64LOW((iRepeatTrailingSilence.Int64()*KToneSamplingRate)/1000000),
            I64LOW((iRampDuration.Int64()*KToneSamplingRate)/1000000)
            );

        iCurrentGenerator = &iSequenceGen;

        // Start playback
        DoPlayL();
        }
    else
        iDevSoundObserver->ToneFinished(KErrNotReady);
*/
    if (iTonePlayCompleteTimer->IsActive())
        {
        iTonePlayCompleteTimer->Cancel();
        }
    iTonePlayCompleteTimer->Start();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::FillThisHwBuffer
// MMMFHwDeviceObserver mixin implementation.
// The CMMFHwDevice implementation object calls this method during decoding
// (playing), when it needs the encoded data in the buffer
// aHwDataBuffer.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::FillThisHwBuffer(
    CMMFBuffer& aHwDataBuffer)
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::FillThisHwBuffer"));

    TInt err = KErrNone;
    // Keep a reference to this Hw data Buffer. We need to send the
    // reference back to HwDevice implementation
    iHwDeviceBuffer = static_cast<CMMFDataBuffer*> (&aHwDataBuffer);
    // Set the request length, From HwDevice this comes with buffer
    // length.
    TInt len = iHwDeviceBuffer->Data().MaxLength();
    // Ignore error. since buffer size = Buffer Length
    TRAP(err, iHwDeviceBuffer->SetRequestSizeL(len));

    if (iMode== EMMFStatePlaying) // Get Data from Observer
        {
        if (iLastBufferReceived)
            {
            iHwDeviceBuffer->Data().SetLength(0);
            // Pass the buffer to the he device
            err = iCMMFHwDevice->ThisHwBufferFilled(*iHwDeviceBuffer);
            }
        else
            {
            // Pass the buffer to the observer
            iDevSoundObserver->BufferToBeFilled(&aHwDataBuffer);
            }
        }
    else if (iMode== EMMFStateTonePlaying)
        {
        // Hw device will call this method right after its Start was called.
        // When it calls this for the first time it hasn't played one single
        // buffer yet so check that.
        // In this case there's no need to set the active buffer as it's already
        // waiting to be played.
        if (!iFirstCallFromHwDevice)
            {
            SetActiveToneBuffer();
            }

        // If there is no data in the active buffer, tone play is finished.
        // DevSound just have to wait for completion event from audio device.
        if (iActiveToneBuffer->Data().Length() > 0)
            {
            len = iActiveToneBuffer->Data().Length();
            // Copy data from tone buffer to hw device buffer
            Mem::Copy((TAny*)(iHwDeviceBuffer->Data().Ptr()),
                      (TAny*)(iActiveToneBuffer->Data().Ptr()),
                      len);

            iHwDeviceBuffer->Data().SetLength(len);
            if (len < iHwDeviceBuffer->RequestSize())
                {
                iHwDeviceBuffer->SetLastBuffer(ETrue);
                }
            // Play data and try to generate next data block
            err = iCMMFHwDevice->ThisHwBufferFilled(*iHwDeviceBuffer);
            if (err != KErrNone)
                {
                return err;
                }

            // Check again whether this is the first call from Hw device.
            // FillFreeToneBuffer assumes the iActiveToneBuffer has already
            // been played.
            if (!iFirstCallFromHwDevice)
                {
                err = FillFreeToneBuffer();
                }
            else
                {
                iFirstCallFromHwDevice = EFalse;  // Reset flag
                }
            }
        else if (iFirstCallFromHwDevice)
            {
            //we have no data in the tone buffer and thus have no
            //outstanding requests to play
            err = KErrUnderflow; //simulate underrun
            iHwDeviceBuffer->SetLastBuffer(ETrue);
            // Play data and try to generate next data block
            err = iCMMFHwDevice->ThisHwBufferFilled(*iHwDeviceBuffer);
            if (err != KErrNone)
                {
                return err;
                }
            }

        // If there was an error filling the buffer could be corrupt data
        // notify the client and stop playing.Set err to KErrNone.
        if (err != KErrNone)
            {
            Error(err);//Updates Bytes played informs client
            err = KErrNone;
            iCMMFHwDevice->Stop();//unloads sound device
            Stopped();//Updates policy
            }
        }
    else
        {
        err = KErrGeneral;
        iDevSoundObserver->PlayError(KErrGeneral);
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::EmptyThisHwBuffer
// MMMFHwDeviceObserver mixin implementation.
// The CMMFHwDevice implementation object calls this method during encoding
// (recording), when it fills the buffer aHwDataBuffer with
// encoded data.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::EmptyThisHwBuffer(
    CMMFBuffer& aHwDataBuffer)
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::EmptyThisHwBuffer"));

    TInt err = KErrNone;
    if(iMode== EMMFStateRecording)
        {
        // Keep a reference to this Hw data Buffer. We need to send the
        // reference back to HwDevice implementation
        iHwDeviceBuffer = static_cast<CMMFDataBuffer*>(&aHwDataBuffer);

        // Set the request length, From HwDevice this comes with buffer
        // length. MMF will use RequestSize attribute of the buffer.
        // We can avoid this by setting in HwDevice implemenation
        TInt len = iHwDeviceBuffer->Data().Length();
        iRecordedBytesCount += len;
        TRAP(err, iHwDeviceBuffer->SetRequestSizeL(len));

        // if we're pausing (i.e. flushing) set the last buffer flag
        // when we get an empty buffer from the logical driver
        if(iPaused  && iHwDeviceBuffer->Data().Length() == 0)
            {
            iPaused = EFalse;
            iHwDeviceBuffer->SetLastBuffer(ETrue);
            }

        // Send Data from Observer
        iDevSoundObserver->BufferToBeEmptied(iHwDeviceBuffer);
        }
    else
        {
        err = KErrGeneral;
        iDevSoundObserver->RecordError(KErrGeneral);
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::MsgFromHwDevice
// MMMFHwDeviceObserver mixin implementation.
// The CMMFHwDevice implementation object calls this method when a message from
// the hardware device implementation is received.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::MsgFromHwDevice(
    TUid /*aMessageType*/,
    const TDesC8& /*aMsg*/)
    {
    return KErrNotSupported;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::Stopped
// MMMFHwDeviceObserver mixin implementation.
//
// The CMMFHwDevice implementation object calls this method when the current
// encode or decode task is finished or stopped.  The policy state is updated
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::Stopped()
    {
    //for swcodec wrap hw devices bytes played updated in MsgFromHwDevice
    //but non Swcodec wrappers hw devices may do it differently also don't
    //know if non Swcodec wrap hw device will call Stopped or Error first
    iLastBufferReceived = EFalse;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::Error
// MMMFHwDeviceObserver mixin implementation
//  Processes error from hw device
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::Error(
    TInt aError)
    {
    DEBPRN1(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::Error(%d)"), aError);

    if (iMode== EMMFStatePlaying)
        {
        iDevSoundObserver->PlayError(aError);
        }
    else if (iMode== EMMFStateRecording)
        {
        iDevSoundObserver->RecordError(aError);
        }
    else if (iMode== EMMFStateTonePlaying)
        {
        if (aError == KErrUnderflow)
            {
            iDevSoundObserver->ToneFinished(KErrNone);
            }
        else
            {
            iDevSoundObserver->ToneFinished(aError);
            }
        }
    //else can't handle error
    }

/********************************************************************************
 *              Non Exported public functions ends here                         *
 ********************************************************************************/


/********************************************************************************
 *              Private functions begins here                                   *
 ********************************************************************************/

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::DoPlayL
// Creates buffer and begin playback using the specified tone generator.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::DoPlayL()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::DoPlayL"));

    // Delete any buffer from previous call and try to create maximum buffer
    // size. Double Buffer the Tone data.
    if (iToneBuffer1)
        {
        delete iToneBuffer1;
        iToneBuffer1 = NULL;
        }

    iToneBuffer1 = CMMFDataBuffer::NewL(KDevSoundDefaultFrameSize);
    User::LeaveIfError(iCurrentGenerator->FillBuffer(iToneBuffer1->Data()));

    if (iToneBuffer2)
        {
        delete iToneBuffer2;
        iToneBuffer2 = NULL;
        }
    iToneBuffer2 = CMMFDataBuffer::NewL(KDevSoundDefaultFrameSize);
    User::LeaveIfError(iCurrentGenerator->FillBuffer(iToneBuffer2->Data()));

    // Assign active buffer
    iActiveToneBuffer = iToneBuffer1;

    // Hw device hasn't played anything yet so don't change
    // active buffer. This is checked in FillThisHwBuffer.
    iFirstCallFromHwDevice = ETrue;

    // Start HwDevice to play data
    if (iCMMFHwDevice->IsActive())
        {
        iCMMFHwDevice->Cancel();
        }
    User::LeaveIfError(iCMMFHwDevice->Start(EDevDecode, EDevOutFlow));
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SetActiveToneBuffer
// This method assigns the other buffer as active buffer. The tone audio
//  generator should fill data in the other buffer by now.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::SetActiveToneBuffer()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::SetActiveToneBuffer"));

    if (iActiveToneBuffer == iToneBuffer1)
        {
        iActiveToneBuffer = iToneBuffer2;
        }
    else if (iActiveToneBuffer == iToneBuffer2)
        {
        iActiveToneBuffer = iToneBuffer1;
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::FillFreeToneBuffer
// This method fills data into the free buffer.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::FillFreeToneBuffer()
    {
    DEBPRN0(_L("CMMFDevSoundAdaptationStub[0x%x]::CBody::FillFreeToneBuffer"));

    TInt err(KErrNone);
    if (iActiveToneBuffer == iToneBuffer1)
        {
        err = iCurrentGenerator->FillBuffer(iToneBuffer2->Data());
        }
    else if (iActiveToneBuffer == iToneBuffer2)
        {
        err = iCurrentGenerator->FillBuffer(iToneBuffer1->Data());
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::InitTask
// Initializes audio device node by setting volume, and sampling rate.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::InitTask()
    {
    // No Implementation
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::SamplingFrequency
// Returns an integer representing Sampling Frequency the device is currently
//  configured to.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::SamplingFrequency()
    {
    if(iDeviceConfig.iRate == EMMFSampleRate8000Hz)
        {
        return 8000;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate11025Hz)
        {
        return 11025;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate12000Hz)
        {
        return 12000;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate16000Hz)
        {
        return 16000;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate22050Hz)
        {
        return 22050;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate24000Hz)
        {
        return 24000;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate32000Hz)
        {
        return 32000;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate44100Hz)
        {
        return 44100;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate48000Hz)
        {
        return 48000;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate88200Hz)
        {
        return 88200;
        }
    else if(iDeviceConfig.iRate == EMMFSampleRate96000Hz)
        {
        return 96000;
        }
    else
        {
        return 8000; //default
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::NumberOfChannels
// Returns an integer representing number of channels the device is currently
//  configured to.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::NumberOfChannels()
    {
    if(iDeviceConfig.iChannels == EMMFMono)
        {
        return 1;
        }
    else
        {
        return 2;
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::BytesPerAudioSample
// Returns an integer representing number of bytes in each audio sample
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CMMFDevSoundAdaptation::CBody::BytesPerAudioSample()
    {
    TInt bytes=1;
    switch (iDeviceConfig.iEncoding)
        {
        case EMMFSoundEncoding8BitPCM:
        case EMMFSoundEncoding8BitALaw:
        case EMMFSoundEncoding8BitMuLaw:
            {
            bytes=1;
            }
        break;
        case EMMFSoundEncoding16BitPCM:
            {
            bytes=2;
            }
        break;
        }
    return bytes;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CBody::InitializeDevSoundUtilL
// Initializes DevSoundUtil object.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::CBody::InitializeDevSoundUtilL() const
    {
    if (!iDevSoundUtil)
        {
        DEBPRN0(
        _L("CMMFDevSoundAdaptationStub[0x%x]::CBody::InitializeDevSoundUtilL:ENTER"));

        iDevSoundUtil = CMMFDevSoundUtility::NewL();

        // Initialize Fixed sequence related
        DEBPRN0(
        _L("CMMFDevSoundAdaptationStub[0x%x]::CBody::InitializeDevSoundUtilL"));
        iDevSoundUtil->InitializeFixedSequenceL(&iFixedSequences);

        DEBPRN0(
        _L("CMMFDevSoundAdaptationStub[0x%x]::CBody::InitializeDevSoundUtilL"));
        }

    }

//End of File
