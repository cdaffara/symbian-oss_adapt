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
* Description: Audio Stubs -  Declaration of CMMFDevSoundAdaptation::CBody.
*
*/



#ifndef MMFDEVSOUNDADAPTATIONBODY_H
#define MMFDEVSOUNDADAPTATIONBODY_H

//  INCLUDES
#include <e32base.h>
#include <mmfhwdevice.h>
#include <MmfDevSoundAdaptation.h>
//#include <MdaSound.h>
#include "ToneGenerator.h"
#include "DevSoundUtility.h"


// CONSTANTS
const TInt KDevSoundDefaultFrameSize = 0x1000;
const TInt KDevSoundMinFrameSize = 0x800; //2K
const TInt KDevSoundMaxFrameSize = 0x4000;  //16K
const TInt KDevSoundDeltaFrameSize = 0x800; //2K
const TInt KDevSoundFramesPerSecond = 4;

// FORWARD DECLARATIONS
class CMMFHwDeviceStub;
class CTonePlayCompleteTimer;
class CBytesPlayedCIImpl;

class CG711DecoderIntfcCI;
class CG711EncoderIntfcCI;
class CG729DecoderIntfcCI;
class CG729EncoderIntfcCI;
class CIlbcDecoderIntfcCI;
class CIlbcEncoderIntfcCI;
class CSpeechEncoderConfigCI;
class CErrorConcealmentIntfcCI;

// CLASS DECLARATION

/**
*  Class implementing CMMFDevSoundAdaptation behaviour.
*
*  @lib MmfDevSoundAdaptation_Stub.lib
*  @since S60 3.0
*/
class CMMFDevSoundAdaptation::CBody : public CBase,
                                      public MMMFHwDeviceObserver
    {
    public:  // Constructors and destructor

        /**
        * Constructs, and returns a pointer to, a new CBody object.
        * Leaves on failure..
        * @return CBody* A pointer to newly created object.
        */
        static CBody* NewL();

        /**
        * Destructor.
        */
        ~CBody();

    public: // New functions

        /**
        * Initializes to raw audio data PCM16 and Sampling Rate of 8 KHz.
        * On completion of Initialization, calls InitializeComplete() on
        * aDevSoundObserver.
        * Leaves on failure.
        * @since S60 3.0
        * @param MDevSoundAdaptationObserver& aDevSoundObserver A reference
        *        to the DevSound adaptation observer instance.
        * @param TMMFState aMode Mode for which this object will be used.
        * @return void
        */
        void InitializeL(MDevSoundAdaptationObserver& aDevSoundObserver,
                         TMMFState aMode);

        /**
        * Initializes DevSound object for the mode aMode for processing audio
        * data with hardware device aHWDev.
        * On completion of Initialization, calls InitializeComplete() on
        * aDevSoundObserver.
        * Leaves on failure.
        * @since S60 3.0
        * @param MDevSoundAdaptationObserver& aDevSoundObserver A reference
        *        to the DevSound adaptation observer instance.
        * @param TUid aHWDev The CMMFHwDevice implementation identifier.
        * @param TMMFState aMode The mode for which this object will be used
        * @return void
        */
        void InitializeL(MDevSoundAdaptationObserver& aDevSoundObserver,
                         TUid aHWDev, TMMFState aMode);

        /**
        * Initializes DevSound object for the mode aMode for processing audio
        * data with hardware device supporting FourCC aDesiredFourCC.
        * Leaves on failure.
        * @since S60 3.0
        * @param MDevSoundAdaptationObserver& aDevSoundObserver A reference
        *        to the DevSound adaptation observer instance.
        * @param TFourCC aDesiredFourCC The CMMFHwDevice implementation FourCC
        *        code.
        * @param TMMFState aMode The mode for which this object will be used
        * @return KErrNone if successfull, else corresponding error code
        * @return void
        */
        void InitializeL(MDevSoundAdaptationObserver& aDevSoundObserver,
                         TFourCC aDesiredFourCC,
                         TMMFState aMode);

        /**
        * Returns the supported Audio settings ie. encoding, sample rates,
        * mono/stereo operation, buffer size etc..
        * @since S60 3.0
        * @return TMMFCapabilities The device settings.
        */
        TMMFCapabilities Capabilities();

        /**
        * Returns the current device configuration.
        * @since S60 3.0
        * @return TMMFCapabilities The device settings.
        */
        TMMFCapabilities Config() const;

        /**
        * Configure CMMFDevSound object with the settings in aConfig. Use this
        * to set sampling rate, encoding and mono/stereo.
        * Leaves on failure.
        * @since S60 3.0
        * @param const TMMFCapabilities& aConfig The attribute values to which
        *        CMMFDevSound object will be configured to.
        * @return void
        */
        void SetConfigL(const TMMFCapabilities& aCaps);

        /**
        * Returns an integer representing the maximum volume device supports.
        * This is the maximum value which can be passed to
        * CMMFDevSound::SetVolume.
        * @since S60 3.0
        * @return TInt The maximum volume. This value is platform dependent but
        *        is always greater than or equal to one.
        */
        TInt MaxVolume();

        /**
        * Returns an integer representing the current volume.
        * @since S60 3.0
        * @return TInt The current volume level.
        */
        TInt Volume();

        /**
        * Changes the current playback volume to a specified value. The volume
        * can be changed before or during playback and is effective immediately.
        * @since S60 3.0
        * @param TInt aVolume The volume setting. This can be any value from 0
        *        to the value returned by a call to
        *        CMMFDevSound::MaxVolume(). If the volume is not
        *        within this range, the volume is automatically set
        *        to minimum or maximum value based on the value
        *        that is being passed. Setting a zero value mutes
        *        the sound. Setting the maximum value results in
        *        the loudest possible sound.
        * @return void
        */
        void SetVolume(TInt aVolume);

        /**
        * Returns an integer representing the maximum gain the device supports.
        * This is the maximum value which can be passed to CMMFDevSound::SetGain
        * @since S60 3.0
        * @return TInt The maximum gain. This value is platform dependent but is
        *        always greater than or equal to one.
        */
        TInt MaxGain();

        /**
        * Returns an integer representing the current gain.
        * @since S60 3.0
        * @return TInt The current gain level.
        */
        TInt Gain();

        /**
        * Changes the current recording gain to a specified value. The gain can
        * be changed before or during recording and is effective immediately.
        * @since S60 3.0
        * @param TInt aGain The gain setting. This can be any value from zero to
        *        the value returned by a call to
        *        CMMFDevSound::MaxGain(). If the volume
        *        is not within this range, the gain is automatically
        *        set to minimum or maximum value based on the value
        *        that is being passed. Setting a zero value mutes the
        *        sound. Setting the maximum value results in the
        *        loudest possible sound.
        * @return void
        */
        void SetGain(TInt aGain);

        /**
        * Returns the speaker balance set for playing.
        * Leaves on failure.
        * @since S60 3.0
        * @param TInt &aLeftPercentage On return contains the left speaker
        *        volume percentage.
        * @param TInt &aRightPercentage On return contains the right speaker
        *        volume percentage.
        * @return void
        */
        void GetPlayBalanceL(TInt& aLeftPercentage, TInt& aRightPercentage);

        /**
        * Sets the speaker balance for playing. The speaker balance can be
        * changed before or during playback and is effective immediately.
        * Leaves on failure.
        * @since S60 3.0
        * @param TInt aLeftPercentage The left speaker volume percentage. This
        *        can be any value from zero to 100. Setting
        *        a zero value mutes the sound on left
        *        speaker.
        * @param TInt aRightPercentage The right speaker volume percentage.
        *        This can be any value from zero to 100.
        *        Setting a zero value mutes the sound on
        *        right speaker.
        * @return void
        */
        void SetPlayBalanceL(TInt aLeftPercentage, TInt aRightPercentage);

        /**
        * Returns the microphone gain balance set for recording.
        * Leaves on failure.
        * @since S60 3.0
        * @param TInt &aLeftPercentage On return contains the left microphone
        *        gain percentage.
        * @param TInt &aRightPercentage On return contains the right microphone
        *        gain percentage.
        * @return void
        */
        void GetRecordBalanceL(TInt& aLeftPercentage, TInt& aRightPercentage);

        /**
        * Sets the microphone balance for recording. The microphone balance can
        * be changed before or during recording and is effective immediately.
        * Leaves on failure.
        * @since S60 3.0
        * @param TInt aLeftPercentage The left microphone gain percentage. This
        *        can be any value from zero to 100. Setting
        *        a zero value mutes the sound from left
        *        microphone.
        * @param TInt aRightPercentage The right microphone gain percentage.
        *        This can be any value from zero to 100.
        *        Setting a zero value mutes the sound from
        *        right microphone.
        * @return void
        */
        void SetRecordBalanceL(TInt aLeftPercentage, TInt aRightPercentage);

        /**
        * Initializes the audio device and starts the play process. This
        * function queries and acquires the audio policy before initializing
        * audio device. If there was an error during policy initialization,
        * PlayError() function will be called on the observer with error code
        * KErrAccessDenied, otherwise BufferToBeFilled() function will be called
        * with a buffer reference. After reading data into the buffer reference
        * passed, the client should call PlayData() to play data.
        * The amount of data that can be played is specified in
        * CMMFBuffer::RequestSize(). Any data that is read into buffer beyond
        * this size will be ignored.
        * Leaves on failure.
        * @since S60 3.0
        * @return void
        */
        void PlayInitL();

        /**
        * Initializes the audio device and starts the record process. This
        * function queries and acquires the audio policy before initializing
        * audio device. If there was an error during policy initialization,
        * RecordError() function will be called on the observer with error code
        * KErrAccessDenied, otherwise BufferToBeEmptied() function will be
        * called with a buffer reference. This buffer contains recorded or
        * encoded data. After processing data in the buffer reference passed,
        * the client should call RecordData() to continue recording process.
        * The amount of data that is available is specified in
        * CMMFBuffer::RequestSize().
        * Leaves on failure.
        * @since S60 3.0
        * @return void
        */
        void RecordInitL();

        /**
        * Plays data in the buffer at the current volume.
        * The client should fill the buffer with audio data before calling this
        * function. The observer gets a reference to the buffer along with the
        * callback function BufferToBeFilled(). When playing of the audio sample
        * is complete, successfully or otherwise, the function PlayError() on
        * the observer is called.
        * The last buffer of the audio stream being played should have the last
        * buffer flag set using CMMFBuffer::SetLastBuffer(TBool). If a
        * subsequent attempt to play the clip is made, this flag will need
        * resetting by the client.
        * @return void
        */
        void PlayData();

        /**
        * Contine the process of recording.
        * Once the buffer is filled with recorded data, the Observer gets a
        * reference to the buffer along with the callback function
        * BufferToBeEmptied(). After processing the buffer (copying over to a
        * different buffer or writing to file) the client should call this
        * function to continue the recording process.
        * @return void
        */
        void RecordData();

        /**
        * Stops the ongoing operation (Play, Record, TonePlay, Convert).
        * @since S60 3.0
        * @return void
        */
        void Stop();

        /**
        * Temporarily Stops the ongoing operation (Play, Record, TonePlay,
        * Convert).
        * @since S60 3.0
        * @return void
        */
        void Pause();

        /**
        * Returns the Sample recorded so far
        * @since S60 3.0
        * @return TInt Returns the samples recorded.
        */
        TInt SamplesRecorded();

        /**
        * Returns the Sample played so far
        * @since S60 3.0
        * @return TInt Returns the samples played.
        */
        TInt SamplesPlayed();

        /**
        * Initializes the audio device and starts playing a tone. The tone is
        * played with the frequency and duration specified.
        * Leaves on failure.
        * @since S60 3.0
        * @param TInt aFrequency The frequency at which the tone will be played.
        * @param const TTimeIntervalMicroSeconds &aDuration The period over
        *        which the tone will be played. A zero value causes the no tone
        *        to be played.
        * @return void
        */
        void PlayToneL(TInt aFrequency,
                       const TTimeIntervalMicroSeconds& aDuration);

        /**
        * Initializes audio device and starts playing a dual tone. Dual Tone is
        * played with the specified frequencies and for the specified duration.
        * Leaves on failure.
        * @since S60 3.0
        * @param TInt aFrequencyOne The first frequency of dual tone.
        * @param TInt aFrequencyTwo The second frequency of dual tone.
        * @param const TTimeIntervalMicroSeconds &aDuration The period over
        *        which the tone will be played. A zero value causes the no tone
        *        to be played.
        * @return void
        */
        void PlayDualToneL(TInt aFrequencyOne,
                           TInt aFrequencyTwo,
                           const TTimeIntervalMicroSeconds& aDuration);

        /**
        * Initializes the audio device and starts playing the DTMF string
        * aDTMFString.
        * Leaves on failure.
        * @since S60 3.0
        * @param const TDesC &aDTMFString The DTMF sequence in a descriptor.
        * @return void
        */
        void PlayDTMFStringL(const TDesC& aDTMFString);

        /**
        * Initializes the audio device and starts playing a tone sequence.
        * Leaves on failure.
        * @since S60 3.0
        * @param const TDesC8 &aData The tone sequence in a descriptor.
        * @return void
        */
        void PlayToneSequenceL(const TDesC8& aData);

        /**
        * Initializes the audio device and starts playing the specified
        * pre-defined tone sequence.
        * Leaves on failure.
        * @since S60 3.0
        * @param TInt aSequenceNumber The index identifying the specific
        *        pre-defined tone sequence. Index values are relative to zero.
        *        This can be any value from zero to the value returned by a call
        *        to FixedSequenceCount() - 1. The function raises a panic if the
        *        sequence number is not within this range.
        * @return void
        */
        void PlayFixedSequenceL(TInt aSequenceNumber);

        /**
        * Defines the number of times the audio is to be repeated during the
        * tone playback operation. A period of silence can follow each playing
        * of a tone. The tone playing can be repeated indefinitely
        * @since S60 3.0
        * @param TInt aRepeatCount The number of times the tone, together with
        *        the trailing silence, is to be repeated. If this is set to
        *        KMdaRepeatForever, then the tone, together with the trailing
        *        silence, is repeated indefinitely or until Stop() is called.
        *        If this is set to zero, then the tone is not repeated.
        * @param const TTimeIntervalMicroSeconds &aRepeatTrailingSilence An
        *        interval of silence which will be played after each tone.
        *        Supported only during tone playing.
        * @return void
        */
        void SetToneRepeats(
             TInt aRepeatCount,
             const TTimeIntervalMicroSeconds& aRepeatTrailingSilence);

        /**
        * Defines the duration of tone on, tone off and tone pause to be used
        * during the DTMF tone playback operation.
        * Supported only during tone playing.
        * @since S60 3.0
        * @param TTimeIntervalMicroSeconds32 &aToneOnLength The period over
        *        which the tone will be played. If this is set to zero, then the
        *        tone is not played.
        * @param TTimeIntervalMicroSeconds32 &aToneOffLength The period over
        *        which the no tone will be played.
        * @param TTimeIntervalMicroSeconds32 &aPauseLength The period over which
        *        the tone playing will be paused.
        * @return void
        */
        void SetDTMFLengths(TTimeIntervalMicroSeconds32& aToneOnLength,
                            TTimeIntervalMicroSeconds32& aToneOffLength,
                            TTimeIntervalMicroSeconds32& aPauseLength);

        /**
        * Defines the period over which the volume level is to rise smoothly
        * from nothing to the normal volume level.
        * The function is only available before playing.
        * @since S60 3.0
        * @param const TTimeIntervalMicroSeconds &aRampDuration The period over
        *        which the volume is to rise. A zero value causes the tone
        *        sample to be played at the normal level for the full duration
        *        of the playback. A value, which is longer than the duration of
        *        the tone sample means that the sample never reaches its normal
        *        volume level.
        * @return void
        */
        void SetVolumeRamp(const TTimeIntervalMicroSeconds& aRampDuration);

        /**
        * Defines the priority settings that should be used for this instance.
        * @since S60 3.0
        * @param const TMMFPrioritySettings &aPrioritySettings A class type
        *        representing the client's priority, priority preference and
        *        state
        * @return void
        */
        void SetPrioritySettings(const TMMFPrioritySettings& aPrioritySettings);

        /**
        * Retrieves a custom interface to the device.
        * @since S60 3.0
        * @param TUid aInterfaceId The interface UID, defined with the custom
        *        interface.
        * @return TAny* A pointer to the interface implementation, or NULL if
        *        the device does not implement the interface requested. The
        *        return value must be cast to the correct type by the user.
        */
        TAny* CustomInterface(TUid aInterfaceId);

        /**
        * Returns the number of available pre-defined tone sequences.
        * This is the number of fixed sequence supported by DevSound by default.
        * @since S60 3.0
        * @return TInt  The fixed sequence count. This value is implementation
        *        dependent.
        */
        TInt FixedSequenceCount();

        /**
        * Returns the name assigned to a specific pre-defined tone sequence.
        * This is the number of the fixed sequence supported by DevSound by
        * default.
        * The function raises a panic if sequence number specified is invalid.
        * @since S60 3.0
        * @param TInt aSequenceNumber The index identifying the specific
        *        pre-defined tone sequence. Index values are relative to zero.
        *        This can be any value from zero to the value returned by a call
        *        to CMdaAudioPlayerUtility::FixedSequenceCount() - 1. The
        *        function raises a panic if sequence number is not within this
        *        range.
        * @return const TDesC & A reference to a Descriptor containing the fixed
        *        sequence name indexed by aSequenceNumber.
        */
        const TDesC& FixedSequenceName(TInt aSequenceNumber);

        /**
        * Returns a list of the supported input datatypes that can be sent to
        * DevSound for playing audio. The datatypes returned are those that the
        * DevSound supports given the priority settings passed in
        * aPrioritySettings. Note that if no supported data types are found this
        * does not constitute failure, the function will return normally with no
        * entries in aSupportedDataTypes.
        * Leaves on failure.
        * @since S60 3.0
        * @param RArray< TFourCC > &aSupportedDataTypes The array of supported
        *        data types that will be filled in by this function. The
        *        supported data types of the DevSound are in the form of an
        *        array of TFourCC codes. Any existing entries in the array will
        *        be overwritten on calling this function. If no supported data
        *        types are found given the priority settings, then the
        *        aSupportedDatatypes array will have zero entries.
        * @param const TMMFPrioritySettings &aPrioritySettings The priority
        *        settings used to determine the supported datatypes. Note this
        *        does not set the priority settings. For input datatypes the
        *        iState member of the priority settings would be expected to be
        *        either EMMFStatePlaying or EMMFStatePlayingRecording. The
        *        priority settings may effect the supported datatypes depending
        *        on the audio routing.
        * @return void
        */
        void GetSupportedInputDataTypesL(
             RArray<TFourCC>& aSupportedDataTypesconst,
             const TMMFPrioritySettings& aPrioritySettings) const;

        /**
        * Returns a list of the supported output dataypes that can be received
        * from DevSound for recording audio. The datatypes returned are those
        * that the DevSound supports given the priority settings passed in
        * aPrioritySettings. Note that if no supported data types are found this
        * does not constitute failure, the function will return normally with no
        * entries in aSupportedDataTypes.
        * Leaves on failure.
        * @since S60 3.0
        * @param RArray< TFourCC > &aSupportedDataTypes The array of supported
        *        data types that will be filled in by this function. The
        *        supported datatypes of the DevSound are in the form of an array
        *        of TFourCC codes. Any existing entries in the array will be
        *        overwritten on calling this function. If no supported datatypes
        *        are found given the priority settings, then the
        *        aSupportedDatatypes array will have zero entries.
        * @param const TMMFPrioritySettings &aPrioritySettings The priority
        *        settings used to determine the supported data types. Note this
        *        does not set the priority settings. For output data types the
        *        iState member of the priority settings would expected to be
        *        either EMMFStateRecording or EMMFStatePlayingRecording. The
        *        priority settings may effect the supported datatypes depending
        *        on the audio routing.
        * @return void
        */
        void GetSupportedOutputDataTypesL(
             RArray<TFourCC>& aSupportedDataTypes,
             const TMMFPrioritySettings& aPrioritySettings) const;

        /**
        * Sets client configuration
        * @since S60 3.0
        * @param TMMFClientConfig& aClientConfig A reference to client
        *        configuration object.
        * @return void
        */
        void SetClientConfig(const TMMFClientConfig& aClientConfig);

        /**
        * Returns client configuration
        * @since S60 3.0
        * @return const TMMFClientConfig& A constant reference to client
        *        configuration.
        */
        const TMMFClientConfig& ClientConfig() const;

        // MMMFHwDeviceObserver implementation for CMMFHwDevice

        /**
        * Callback function from CMMFHwDevice when it needs data.
        * @since S60 3.0
        * @param CMMFBuffer& aHwBuffer A reference to CMMFBuffer in which the
        *        observer need to fill data into.
        * @return TInt Error code. KErrNone if success.
        */
        TInt FillThisHwBuffer(CMMFBuffer& aHwBuffer);

        /**
        * Callback function from CMMFHwDevice when it has data.
        * @since S60 3.0
        * @param CMMFBuffer& aHwBuffer A reference to CMMFBuffer in which the
        *        observer need to empty data from.
        * @return TInt Error code. KErrNone if success.
        */
        TInt EmptyThisHwBuffer(CMMFBuffer& aHwBuffer);

        /**
        * Callback function from CMMFHwDevice.
        * @since S60 3.0
        * @param TUid aMessageType A Uid identifying type of message.
        * @param const TDesC8& aMsg.A reference to descriptor containing message
        *        information.
        * @return TInt Error code. KErrNone if success.
        */
        TInt MsgFromHwDevice(TUid aMessageType, const TDesC8& aMsg);

        /**
        * Callback function from CMMFHwDevice when playing back/ recording is
        * stopped..
        * @since S60 3.0
        * @return void
        */
        void Stopped();

        /**
        * Callback function from CMMFHwDevice when there is a error during
        * playback and/or recording.
        * @since S60 3.0
        * @param TInt aError A systemwide error code.
        * @return void
        */
        void Error(TInt aError);

        // CMMFHwDevice implementation ends

    private:

        /**
        * Called by Audio Policy Server when play reqeust is granted
        * Leaves on failure.
        * @since S60 3.0
        * @return void
        */
        void StartPlayDataL();

        /**
        * Called by Audio Policy Server when record reqeust is granted
        * Leaves on failure.
        * @since S60 3.0
        * @return void
        */
        void StartRecordDataL();

        /**
        * Called by Audio Policy Server when play tone reqeust is granted
        * Leaves on failure.
        * @since S60 3.0
        * @return void
        */
        void StartPlayToneL();

        /**
        * Called by Audio Policy Server when play dual tone request is granted
        * Leaves on failure.
        * @since S60 3.0
        * @return void
        */
        void StartPlayDualToneL();

        /**
        * Called by Audio Policy Server when play DTMF reqeust is granted
        * Leaves on failure.
        * @since S60 3.0
        * @return void
        */
        void StartPlayDTMFStringL();

        /**
        * Called by Audio Policy Server when play tone sequence reqeust
        * is granted.
        * Leaves on failure.
        * @since S60 3.0
        * @return void
        */
        void StartPlayToneSequenceL();

        /**
        * Play tone
        * Leaves on failure.
        * @since S60 3.0
        * @return void
        */
        void DoPlayL();

        /**
        * Sets active tone
        * @since S60 3.0
        * @return void
        */
        void SetActiveToneBuffer();

        /**
        * Fills free tone buffer with tone data
        * @since S60 3.0
        * @return KErrNone if succesfull, else corresponding error code.
        */
        TInt FillFreeToneBuffer();


        // HwDevice implementation begins

        /**
        * Initialize HwDevice task.
        * @since S60 3.0
        * @return KErrNone if succesfull, else corresponding error code.
        */
        TInt InitTask();

        /**
        * Returns Sampling Frequency the device is currently configured to
        * @since S60 3.0
        * @return KErrNone if succesfull, else corresponding error code.
        */
        TInt SamplingFrequency();

        /**
        * Returns number of channels the device is currently configured to
        * @since S60 3.0
        * @return KErrNone if succesfull, else corresponding error code.
        */
        TInt NumberOfChannels();

        /**
        * returns the number of bytes in each audio sample
        * @since S60 3.0
        * @return TInt Number of of bytes in each audio sample.
        */
        TInt BytesPerAudioSample();

        // HwDevice implementation ends

        /**
        * Initializes DevSound utility
        * @since S60 3.0
        * @return void
        */
        void InitializeDevSoundUtilL() const;

    private:

        /**
        * C++ default constructor.
        */
        CBody();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(RServer2& aPolicyServerHandle);

    protected:  // Data

        CG711DecoderIntfcCI*      iG711DecoderIntfcCI;
        CG711EncoderIntfcCI*      iG711EncoderIntfcCI;
        CG729DecoderIntfcCI*      iG729DecoderIntfcCI;
        CG729EncoderIntfcCI*      iG729EncoderIntfcCI;
        CIlbcDecoderIntfcCI*      iIlbcDecoderIntfcCI;
        CIlbcEncoderIntfcCI*      iIlbcEncoderIntfcCI;
        CSpeechEncoderConfigCI*   iSpeechEncoderConfigCI;
        CErrorConcealmentIntfcCI* iErrorConcealmentIntfcCI;

    private:    // Data

        //Objects (owned or referred to):
        MDevSoundAdaptationObserver* iDevSoundObserver;

        // DevSound capabilities
        TMMFCapabilities        iDeviceCapabilities;
        // Current Configuration
        TMMFCapabilities        iDeviceConfig;

        // Bytes of audio data played in current play cycle
        TInt                    iPlayedBytesCount;
        // Bytes of audio data recorded in current record cycle
        TInt                    iRecordedBytesCount;
        TInt                    iGain;
        TInt                    iVolume;

        //Tone Stuff:
        MMdaToneSynthesis*          iCurrentGenerator;
        TMdaSimpleToneGenerator     iToneGen;
        TMdaDualToneGenerator       iDualToneGen;
        TMdaDTMFGenerator           iDTMFGen;
        TMdaSequenceGenerator       iSequenceGen; // Not Supported
        TInt                        iRepeatCount;
        TTimeIntervalMicroSeconds   iRepeatTrailingSilence;
        TTimeIntervalMicroSeconds   iRampDuration;
        // Double buffer tone playing
        CMMFDataBuffer*             iToneBuffer1;
        CMMFDataBuffer*             iToneBuffer2;
        // Reference to current tone buffer playing
        CMMFDataBuffer*             iActiveToneBuffer;

        // Hardware device implementation
        CMMFHwDeviceStub*           iCMMFHwDevice;
        // Mode in which DevSound instance will be used for
        TMMFState                   iMode;
        // Reference to hardware buffer
        CMMFDataBuffer*             iHwDeviceBuffer;

        TBool                       iLastBufferReceived;

        // Holds true of Audio data given to DevSound needs to ramped up
        mutable CMMFDevSoundUtility* iDevSoundUtil;

        // Fixed sequence related

        // Holds the fixed sequence data
        mutable CPtrC8Array*        iFixedSequences;
        // Holds reference to current sequence
        TPtrC8                      iFixedSequence;

        // Balance
        TInt                        iLeftPlayBalance;
        TInt                        iRightPlayBalance;
        TInt                        iLeftRecordBalance;
        TInt                        iRightRecordBalance;
        TBool                       iPaused;

        TUid                        iHwDeviceID;

        // Hardware device initialization parameters
        THwDeviceInitParams         iDevInfo;

        TBool                       iFirstCallFromHwDevice;

        // Temporary CI testing only
        CBytesPlayedCIImpl*         iBytesPlayedCIImpl;
        // Holds clients configuration
        TMMFClientConfig            iClientConfig;

        // Pointer to CTonePlayCompleteTimer
        CTonePlayCompleteTimer*     iTonePlayCompleteTimer;

    private:    // Friend classes
        friend class CMMFDevSoundAdaptation;

    };

#endif      // MMFDEVSOUNDADAPTATIONBODY_H

// End of File
