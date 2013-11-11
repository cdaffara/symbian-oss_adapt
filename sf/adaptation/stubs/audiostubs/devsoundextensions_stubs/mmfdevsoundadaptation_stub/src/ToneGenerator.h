/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies).
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



#ifndef TONEGENERATOR_H
#define TONEGENERATOR_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
const TInt KMaxSineTable = 256;
const TUint KToneBufferSize = 8192;
// one second in microseconds
const TInt KOneMillionMicroSeconds = 1000000;
const TInt KMaxSequenceStack = 6;
const TInt KBufferLength = 0x1000;

// CLASS DECLARATION

/**
*  Utility class used by DevSound Adaptation to generage PCM data for playing
*  back tone on WINSCW Reference Implementation..
*
*  @lib MmfDevSoundAdaptation.lib
*  @since Series 60 3.0
*/
class TSineGen
    {
    public:

        /**
        * Sets frequency and amplitude.
        * @since Series 60 3.0
        * @param TInt aFrequency Frequency to generate PCM data.
        * @param TInt aAmplitude Amplitude to generate PCM data.
        * @return void
        */
        void SetFrequency(TInt aFrequency,TInt aAmplitude);

        /**
        * Returns the next sample.
        * @since Series 60 3.0
        * @return TInt Sample.
        */
        TInt NextSample();

    private:

        TUint iPosition;
        TUint iStep;
        TInt iAmplitude;
        static const TInt16 SineTable[KMaxSineTable];
        static const TInt16 IncTable[KMaxSineTable];
    };

/**
*  Utility class used by DevSound Adaptation to generage PCM data for playing
*  back tone on WINSCW Reference Implementation..
*
*  @lib MmfDevSoundAdaptation.lib
*  @since Series 60 3.0
*/
class TSineWave
    {
    public:

        /**
        * Generate Sine wave representing tone.
        * @since Series 60 3.0
        * @param TInt16* aDest Destination
        * @param TInt aCount Count
        * @return void
        */
        void Generate(TInt16* aDest,TInt aCount);

        /**
        * Set frequency amplitude.
        * @since Series 60 3.0
        * @param TInt aFrequency Frequency to generate PCM data.
        * @param TInt aAmplitude Amplitude to generate PCM data.
        * @return void
        */
        void SetFrequency(TInt aFrequency,TInt aAmplitude);

        /**
        * Set frequency amplitude.
        * @since Series 60 3.0
        * @param TInt aFrequency1 Frequency to generate PCM data.
        * @param TInt aAmplitude1 Amplitude to generate PCM data.
        * @param TInt aFrequency2 Frequency to generate PCM data.
        * @param TInt aAmplitude2 Amplitude to generate PCM data.
        * @return void
        */
        void SetFrequency(TInt aFrequency1,TInt aAmplitude1,TInt aFrequency2,TInt aAmplitude2);

    private:
        TSineGen iGen1;
        TSineGen iGen2;
    };


/**
*  Tone synthesis interface
*  Defines the abstract interface for tone synthesis
*  Capable of filling buffers with audio data
*
*  @lib MmfDevSoundAdaptation.lib
*  @since Series 60 3.0
*/
class MMdaToneSynthesis
    {
    public:

        /**
        * Allocate necessary resources for this kind of synthesis
        * @since Series 60 3.0
        * @param TInt aRate Sampling rate.
        * @param TInt aChannels Mono/Stereo.
        * @param TInt aRepeats Tone repeats.
        * @param TInt aSilence Silence duration between repetation.
        * @param TInt aRampUp Volume ramping duration.
        * @return void
        */
        virtual void Configure(TInt aRate,
                               TInt aChannels,
                               TInt aRepeats,
                               TInt aSilence,
                               TInt aRampUp)=0;

        /**
        * Reset generator
        * @since Series 60 3.0
        * @return void
        */
        virtual void Reset()=0;

        /**
        * Fill supplied buffer with next block of 16bit PCM audio data
        * @since Series 60 3.0
        * @param TDes8& aBuffer A descriptor reference to buffer where data need
        *        to be filled.
        * @return KErrNone if successfull, else corresponding error code
        */
        virtual TInt FillBuffer(TDes8& aBuffer)=0;
    };


/**
*  Tone generator base class
*
*  @lib MmfDevSoundAdaptation.lib
*  @since Series 60 3.0
*/
class TMdaToneGenerator : public MMdaToneSynthesis
    {
    public:

        /**
        * Configures Tone generator.
        * @since Series 60 3.0
        * @param TInt aRate Sampling rate.
        * @param TInt aChannels Mono/Stereo.
        * @param TInt aRepeats Tone repeats.
        * @param TInt aSilence Silence duration between repetation.
        * @param TInt aRampUp Volume ramping duration.
        * @return void
        */
        virtual void Configure(TInt aRate,
                               TInt aChannels,
                               TInt aRepeats,
                               TInt aSilence,
                               TInt aRampUp);

        /**
        * Fill supplied buffer with next block of 16bit PCM audio data
        * @since Series 60 3.0
        * @param TDes8& aBuffer A descriptor reference to buffer where data need
        *        to be filled.
        * @return KErrNone if successfull, else corresponding error code
        */
        virtual TInt FillBuffer(TDes8& aBuffer);

    protected:

        /**
        * Gets the next tone buffer
        * @since Series 60 3.0
        * @return KErrNone if successfull, else corresponding error code
        */
        virtual TInt GetNextTone()=0;

        /**
        * Converts duration to PCM samples.
        * @since Series 60 3.0
        * @param const TTimeIntervalMicroSeconds& aDuration Duration.
        * @return KErrNone if successfull, else corresponding error code
        */
        TInt DurationToSamples(const TTimeIntervalMicroSeconds& aDuration);

    protected:
        TSineWave iSineWave;
        TInt iRate;
        TInt iChannels;
        TInt iSamplesLeft;
        TInt iTrailingSilence;
        TBool iRampUp;
        TBool iRampDown;
        TInt iRepeats;
        TInt iSilenceBetweenRepeats;
        TBool iAfterRepeatSilence;
        TInt iRampUpCount;
        TInt iRampUpLeft;
    };


/**
*  Simple tone synthesis
*
*  @lib MmfDevSoundAdaptation.lib
*  @since Series 60 3.0
*/
class TMdaSimpleToneGenerator : public TMdaToneGenerator
    {
    public:

        /**
        * Sets frequency and duration for generating PCM data.
        * @since Series 60 3.0
        * @param TInt aFrequency Frequency of tone to be generated.
        * @param const TTimeIntervalMicroSeconds& aDuration Duration of tone to
        *        be generated.
        * @return void
        */
        void SetFrequencyAndDuration(TInt aFrequency,
                                     const TTimeIntervalMicroSeconds& aDuration);

        /**
        * Reset generator
        * @since Series 60 3.0
        * @return void
        */
        virtual void Reset();

        /**
        * Gets the next tone buffer
        * @since Series 60 3.0
        * @return KErrNone if successfull, else corresponding error code
        */
        virtual TInt GetNextTone();

    private:
        TTimeIntervalMicroSeconds iDuration;
        TInt iFrequency;
        TBool iPlayed;
    };

/**
*  Dual tone synthesis
*  Generates a tone consisting of two sine waves of different
*  frequencies summed together.
*
*  @lib MmfDevSoundAdaptation.lib
*  @since Series 60 3.0
*/
class TMdaDualToneGenerator : public TMdaToneGenerator
    {
    public:

        /**
        * ?member_description.
        * @since Series 60 3.0
        * @param ?arg1
        */
        void SetFrequencyAndDuration(TInt aFrequencyOne,
                                     TInt aFrequencyTwo,
                                     const TTimeIntervalMicroSeconds& aDuration);

        /**
        * Reset generator
        * @since Series 60 3.0
        * @return void
        */
        virtual void Reset();

        /**
        * Gets the next tone buffer
        * @since Series 60 3.0
        * @return KErrNone if successfull, else corresponding error code
        */
        virtual TInt GetNextTone();

    private:
        TTimeIntervalMicroSeconds iDuration;
        TInt iFrequencyOne;
        TInt iFrequencyTwo;
        TBool iPlayed;
    };

/**
*  DTMF tone synthesis
*  ?other_description_lines
*
*  @lib MmfDevSoundAdaptation.lib
*  @since Series 60 3.0
*/
class TMdaDTMFGenerator : public TMdaToneGenerator
    {
    public:

        /**
        * Reset generator
        * @since Series 60 3.0
        * @return void
        */
        virtual void Reset();

        /**
        * ?member_description.
        * @since Series 60 3.0
        * @param ?arg1
        */
        void SetToneDurations(  const TTimeIntervalMicroSeconds32 aOn,
                                const TTimeIntervalMicroSeconds32 aOff,
                                const TTimeIntervalMicroSeconds32 aPause);

        /**
        * ?member_description.
        * @since Series 60 3.0
        * @param ?arg1
        */
        void SetString(const TDesC& aDTMFString);

    private:

        /**
        * Gets the next tone buffer
        * @since Series 60 3.0
        * @return KErrNone if successfull, else corresponding error code
        */
        virtual TInt GetNextTone();

    private:
        const TDesC* iDTMFString;
        TTimeIntervalMicroSeconds32 iOn;
        TTimeIntervalMicroSeconds32 iOff;
        TTimeIntervalMicroSeconds32 iPause;
        TInt iOnSamples;
        TInt iOffSamples;
        TInt iPauseSamples;
        TInt iChar;
        TBool iPlayToneOff;
    };

/**
*  Tone sequence synthesis
*  ?other_description_lines
*
*  @lib MmfDevSoundAdaptation.lib
*  @since Series 60 3.0
*/
class TMdaSequenceGenerator : public TMdaToneGenerator
    {
    public:

        /**
        * Reset generator
        * @since Series 60 3.0
        * @return void
        */
        virtual void Reset();

        /**
        * ?member_description.
        * @since Series 60 3.0
        * @param ?arg1
        */
        void SetSequenceData(const TDesC8& aSequenceData);

    private:

        /**
        * Gets the next tone buffer
        * @since Series 60 3.0
        * @return KErrNone if successfull, else corresponding error code
        */
        virtual TInt GetNextTone();

    private:
        const TDesC8* iSequenceData;
        const TInt16* iInstructionPtr;
        const TInt16* iLastInstruction;
        TInt iStack[KMaxSequenceStack];
        TInt iStackIndex;
    };


/**
*  Public Media Server includes
*  ?other_description_lines
*
*  @lib MmfDevSoundAdaptation.lib
*  @since Series 60 3.0
*/
class TMdaPtr8 : public TPtr8 //needed for this WINS Impl of Tone Gen
    {
    public:
        TMdaPtr8()
            : TPtr8(0,0,0) {};
        inline void Set(const TDes8& aDes)
            { TPtr8::Set((TUint8*)(aDes.Ptr()),aDes.Length(),aDes.MaxLength()); };
        inline void SetLengthOnly(const TDes8& aDes)
            { TPtr8::Set((TUint8*)(aDes.Ptr()),aDes.Length(),aDes.Length()); };
        inline void Set(const TPtrC8& aDes)
            { TPtr8::Set((TUint8*)(aDes.Ptr()),aDes.Length(),aDes.Length()); };
        inline void Shift(TInt aOffset)
            { SetLength(Length()-aOffset); iMaxLength-=aOffset; iPtr+=aOffset; };
    };

#endif      // TONEGENERATOR_H

// End of File

