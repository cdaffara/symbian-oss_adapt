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
* Description: Audio Stubs -  Stub implementation of the MMF DevSound Server adaptation.
*
*/



// INCLUDE FILES
#include <MmfDevSoundAdaptation.h>
#include "MmfDevSoundAdaptationBody.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CMMFDevSoundAdaptation
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CMMFDevSoundAdaptation::CMMFDevSoundAdaptation()
    {
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CMMFDevSoundAdaptation::ConstructL(
    RServer2& aPolicyServerHandle)
    {
    iBody = CBody::NewL();
    // This is needed because in iBody->ConstructL(), Adaptation need to have
    // a pointer reference to iBody to call SetDevSoundInfo.
    iBody->ConstructL(aPolicyServerHandle);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CMMFDevSoundAdaptation* CMMFDevSoundAdaptation::NewL(
    RServer2& aPolicyServerHandle)
    {
    CMMFDevSoundAdaptation* self = new (ELeave)CMMFDevSoundAdaptation;
    CleanupStack::PushL(self);
    self->ConstructL(aPolicyServerHandle);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::ConstructL
// Destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CMMFDevSoundAdaptation::~CMMFDevSoundAdaptation()
    {
    delete iBody;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::InitializeL
// Initializes CMMFDevSoundProxy object to play and record PCM16 raw audio data
// with sampling rate of 8 KHz.On completion of Initialization, calls
// InitializeComplete() on aDevSoundObserver.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::InitializeL(
    MDevSoundAdaptationObserver& aDevSoundObserver,
    TMMFState aMode)

    {
    iBody->InitializeL(aDevSoundObserver, aMode);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::InitializeL
// Initializes DevSound object for the mode aMode for processing audio data
// with hardware device aHWDev. On completion of Initialization, the observer
// will be notified via call back InitializeComplete().
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::InitializeL(
    MDevSoundAdaptationObserver& aDevSoundObserver,
    TUid aHWDev,
    TMMFState aMode)
    {
    iBody->InitializeL(aDevSoundObserver, aHWDev, aMode);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::InitializeL
// Initializes DevSound object for the mode aMode for processing audio data
// using an array of Hardware devices identified by aHWDevArray identifier
// array. The hardware devices are chained together with data flow starting
// with first array element.On completion of Initialization, the observer
// will be notified via call back InitializeComplete().
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::InitializeL(
    MDevSoundAdaptationObserver& aDevSoundObserver,
    TFourCC aDesiredFourCC,
    TMMFState aMode)
    {
    iBody->InitializeL(aDevSoundObserver, aDesiredFourCC, aMode);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::Capabilities
// Returns the supported Audio settings.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TMMFCapabilities CMMFDevSoundAdaptation::Capabilities()
    {
    return iBody->Capabilities();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::Config
// Returns the current audio settings.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TMMFCapabilities CMMFDevSoundAdaptation::Config() const
    {
    return iBody->Config();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SetConfigL
// Configure CMMFDevSoundProxy object for the settings in aConfig.
// Use this to set sampling rate, Encoding and Mono/Stereo.
// As part of defect 20796, the iRecordFormat has been set under the iPlayFormat,
// before it was not set at all.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetConfigL(
    const TMMFCapabilities& aConfig)
    {
    iBody->SetConfigL(aConfig);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::MaxVolume
// Returns an integer representing the maximum volume.
// This is the maximum value which can be passed to CMMFDevSoundProxy::SetVolume.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CMMFDevSoundAdaptation::MaxVolume()
    {
    return iBody->MaxVolume();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::Volume
// Returns an integer representing the current volume.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CMMFDevSoundAdaptation::Volume()
    {
    return iBody->Volume();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SetVolume
// Changes the current playback volume to a specified value.
// The volume can be changed before or during playback and is effective
// immediately.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetVolume(
    TInt aVolume)
    {
    iBody->SetVolume(aVolume);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::MaxGain
// Returns an integer representing the maximum gain.
// This is the maximum value which can be passed to CMMFDevSoundProxy::SetGain.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CMMFDevSoundAdaptation::MaxGain()
    {
    return iBody->MaxGain();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::Gain
// Returns an integer representing the current gain.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CMMFDevSoundAdaptation::Gain()
    {
    return iBody->Gain();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SetGain
// Changes the current recording gain to a specified value.
// The gain can be changed before or during recording and is effective
// immediately.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetGain(
    TInt aGain)
    {
    iBody->SetGain(aGain);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::GetPlayBalanceL
// Returns the speaker balance set for playing.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::GetPlayBalanceL(
    TInt& aLeftPercentage,
    TInt& aRightPercentage)
    {
    iBody->GetPlayBalanceL(aLeftPercentage, aRightPercentage);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SetPlayBalanceL
// Sets the speaker balance for playing. The speaker balance can be changed
// before or during playback and is effective immediately.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetPlayBalanceL(
    TInt aLeftPercentage,
    TInt aRightPercentage)
    {
    iBody->SetPlayBalanceL(aLeftPercentage, aRightPercentage);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::GetRecordBalanceL
// Returns the microphone gain balance set for recording.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::GetRecordBalanceL(
    TInt& aLeftPercentage,
    TInt& aRightPercentage)
    {
    iBody->GetRecordBalanceL(aLeftPercentage, aRightPercentage);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SetRecordBalanceL
// Sets the microphone gain balance for recording.
// The microphone gain balance can be changed before or during recording and
// is effective immediately.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetRecordBalanceL(
    TInt aLeftPercentage,
    TInt aRightPercentage)
    {
    iBody->SetRecordBalanceL(aLeftPercentage, aRightPercentage);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::PlayInitL
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
EXPORT_C void CMMFDevSoundAdaptation::PlayInitL()
    {
    iBody->PlayInitL();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::RecordInitL
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
EXPORT_C void CMMFDevSoundAdaptation::RecordInitL()
    {
    iBody->RecordInitL();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::PlayData
// Plays data in the buffer at the current volume. The client should fill
// the buffer with audio data before calling this method. The Observer gets
// reference to buffer along with callback BufferToBeFilled(). When playing of
// the audio sample is complete, successfully or otherwise, the method
// PlayError() on observer is called.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::PlayData()
    {
    iBody->PlayData();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::RecordData
// Contine the process of recording. Once the buffer is filled with recorded
// data, the Observer gets reference to buffer along with callback
// BufferToBeEmptied(). After processing the buffer (copying over to a
// different buffer or writing to file) the client should call this
// method to continue recording process.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::RecordData()
    {
    iBody->RecordData();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::Stop
// Stops the ongoing operation (Play, Record, TonePlay, Convert)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::Stop()
    {
    iBody->Stop();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::Pause
// Temporarily Stops the ongoing operation (Play, Record, TonePlay, Convert)
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::Pause()
    {
    iBody->Pause();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SamplesRecorded
// Returns the sample recorded so far.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CMMFDevSoundAdaptation::SamplesRecorded()
    {
    return iBody->SamplesRecorded();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SamplesPlayed
// Returns the sample played so far.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CMMFDevSoundAdaptation::SamplesPlayed()
    {
    return iBody->SamplesPlayed();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::PlayToneL
// Initializes audio device and start playing tone. Tone is played with
// frequency and for duration specified.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::PlayToneL(
    TInt aFrequency,
    const TTimeIntervalMicroSeconds& aDuration)
    {
    iBody->PlayToneL(aFrequency, aDuration);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::PlayDualToneL
// Initializes audio device and start playing a dual tone.
// The tone consists of two sine waves of different frequencies summed together
// Dual Tone is played with specified frequencies and for specified duration.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::PlayDualToneL(
    TInt aFrequencyOne,
    TInt aFrequencyTwo,
    const TTimeIntervalMicroSeconds& aDuration)
    {
    iBody->PlayDualToneL(aFrequencyOne, aFrequencyTwo, aDuration);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::PlayDTMFStringL
// Initializes audio device and start playing DTMF string aDTMFString.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::PlayDTMFStringL(
    const TDesC& aDTMFString)
    {
    iBody->PlayDTMFStringL(aDTMFString);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::PlayToneSequenceL
// Initializes audio device and start playing tone sequence.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::PlayToneSequenceL(
    const TDesC8& aData)
    {
    iBody->PlayToneSequenceL(aData);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::PlayFixedSequenceL
// Initializes audio device and start playing the specified pre-defined tone
// sequence.
// Leaves on failure.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::PlayFixedSequenceL(
    TInt aSequenceNumber)
    {
    iBody->PlayFixedSequenceL(aSequenceNumber);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SetToneRepeats
// Defines the number of times the audio is to be repeated during the tone
// playback operation. A period of silence can follow each playing of tone.
// The tone playing can be repeated indefinitely.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetToneRepeats(
    TInt aRepeatCount,
    const TTimeIntervalMicroSeconds& aRepeatTrailingSilence)
    {
    iBody->SetToneRepeats(aRepeatCount, aRepeatTrailingSilence);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SetDTMFLengths
// Defines the duration of tone on, tone off and tone pause to be used during the
// DTMF tone playback operation.
// Supported only during tone playing.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetDTMFLengths(
    TTimeIntervalMicroSeconds32& aToneOnLength,
    TTimeIntervalMicroSeconds32& aToneOffLength,
    TTimeIntervalMicroSeconds32& aPauseLength)
    {
    iBody->SetDTMFLengths(aToneOnLength, aToneOffLength, aPauseLength);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SetVolumeRamp
// Defines the period over which the volume level is to rise smoothly from
// nothing to the normal volume level.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetVolumeRamp(
    const TTimeIntervalMicroSeconds& aRampDuration)
    {
    iBody->SetVolumeRamp(aRampDuration);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::SetPrioritySettings
// Defines the priority settings that should be used for this instance.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetPrioritySettings(
    const TMMFPrioritySettings& aPrioritySettings)
    {
    iBody->SetPrioritySettings(aPrioritySettings);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::CustomInterface
// see sounddevice.h
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TAny* CMMFDevSoundAdaptation::CustomInterface(
    TUid aInterfaceId)
    {
    return iBody->CustomInterface(aInterfaceId);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::FixedSequenceCount
// Returns the number of available pre-defined tone sequences.
// This is the number of fixed sequence supported by DevSound by default.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CMMFDevSoundAdaptation::FixedSequenceCount()
    {
    return iBody->FixedSequenceCount();
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::FixedSequenceName
// Returns the name assigned to a specific pre-defined tone sequence.
// This is the number of fixed sequence supported by DevSound by default.
// The function raises a panic if sequence number specified invalid.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C const TDesC& CMMFDevSoundAdaptation::FixedSequenceName(
    TInt aSequenceNumber)
    {
    return iBody->FixedSequenceName(aSequenceNumber);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::GetSupportedInputDataTypesL
// see sounddevice.h
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::GetSupportedInputDataTypesL(
    RArray<TFourCC>& aSupportedDataTypes,
    const TMMFPrioritySettings& aPrioritySettings) const
    {
    iBody->GetSupportedInputDataTypesL(aSupportedDataTypes, aPrioritySettings);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::GetSupportedOutputDataTypesL
// see sounddevice.h
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::GetSupportedOutputDataTypesL(
    RArray<TFourCC>& aSupportedDataTypes,
    const TMMFPrioritySettings& aPrioritySettings) const
    {
    iBody->GetSupportedOutputDataTypesL(aSupportedDataTypes, aPrioritySettings);
    }

// -----------------------------------------------------------------------------
// SetClientConfig
// Sets client capabilities for this instance of DevSound Adaptation.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CMMFDevSoundAdaptation::SetClientConfig(
    const TMMFClientConfig& aClientConfig)
    {
    iBody->SetClientConfig(aClientConfig);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundAdaptation::ClientConfig
// Returns client capabilities of this instance of DevSound Adaptation.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C const TMMFClientConfig& CMMFDevSoundAdaptation::ClientConfig() const
    {
    return iBody->ClientConfig();
    }

// End of file
