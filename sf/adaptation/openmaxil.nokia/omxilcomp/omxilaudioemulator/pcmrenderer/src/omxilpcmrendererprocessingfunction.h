/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
*
*/


/**
 @file
 @internalComponent
*/

#ifndef OMXILPCMRENDERERPROCESSINGFUNCTION_H
#define OMXILPCMRENDERERPROCESSINGFUNCTION_H

#include "omxilpcmrendererconst.h"
#include <openmax/il/common/omxilprocessingfunction.h>
#include <e32msgqueue.h> 

// Forward declarations
class MOmxILClockComponentCmdsIf;

NONSHARABLE_CLASS(COmxILPcmRendererProcessingFunction) :
	public COmxILProcessingFunction
	{

public:
	static COmxILPcmRendererProcessingFunction* NewL(
		MOmxILCallbackNotificationIf& aCallbacks,
		MOmxILClockComponentCmdsIf& aClientClockPort);

	~COmxILPcmRendererProcessingFunction();

	OMX_ERRORTYPE StateTransitionIndication(TStateIndex aNewState);

	OMX_ERRORTYPE BufferFlushingIndication(TUint32 aPortIndex,
										   OMX_DIRTYPE aDirection);

	OMX_ERRORTYPE ParamIndication(OMX_INDEXTYPE aParamIndex,
									  const TAny* apComponentParameterStructure);

	OMX_ERRORTYPE ConfigIndication(OMX_INDEXTYPE aConfigIndex,
									   const TAny* apComponentConfigStructure);

	OMX_ERRORTYPE BufferIndication(
		OMX_BUFFERHEADERTYPE* apBufferHeader,
		OMX_DIRTYPE aDirection);

	OMX_ERRORTYPE MediaTimeIndication(const OMX_TIME_MEDIATIMETYPE& aMediaTimeType);
	
	OMX_BOOL BufferRemovalIndication(
		OMX_BUFFERHEADERTYPE* apBufferHeader,
		OMX_DIRTYPE aDirection);

	TInt GetBytesPlayed() const;
	
private:

	COmxILPcmRendererProcessingFunction(MOmxILCallbackNotificationIf& aCallbacks,
									MOmxILClockComponentCmdsIf& aClientClockPort);
	
	void ConstructL();

	void FlushBufferList(RPointerArray<OMX_BUFFERHEADERTYPE>& aBufferList);
	
private:
	RPointerArray<OMX_BUFFERHEADERTYPE> iBuffersToEmpty;
	RPointerArray<OMX_BUFFERHEADERTYPE> iBuffersEmptied;
	OMX_STATETYPE iState;
	MOmxILClockComponentCmdsIf* iClientClockPortPtr;
	OMX_TICKS iStartMediaTime;	
	
	class CAudioDevice : public CActive
		{
	public:
		static CAudioDevice* NewL(COmxILPcmRendererProcessingFunction& aParent);
		~CAudioDevice();

		// from CActive
		void RunL();
		void DoCancel();

		OMX_ERRORTYPE OpenDevice();
		OMX_ERRORTYPE CloseDevice();
		OMX_ERRORTYPE Execute();
		OMX_ERRORTYPE MoveToPausedState();
		OMX_ERRORTYPE Stop();

		OMX_ERRORTYPE SetChannels(TUint aChannels);
		OMX_ERRORTYPE SetSampleRate(TUint aSampleRate);
		OMX_ERRORTYPE SetVolume(TInt aVolume);
		OMX_ERRORTYPE SetVolumeRamp(const TTimeIntervalMicroSeconds& aRampDuration);
		OMX_ERRORTYPE SetMuted(TBool aMuted);
		OMX_BUFFERHEADERTYPE* GetCurrentBuffer();
		TInt GetBytesPlayed();
		void ConfigAudioRamper(TInt64 aRampTime);
		TBool RampAudio(TDes8& aBuffer);
		TBool ConstructAndStartUpdateTimer();
		void ProcessNextBuffer();
		void PlayData();
		void ProcessMediaTimeIndication(OMX_TIME_MEDIATIMETYPE& aMediaTimeType);
		void ProcessParamIndication(const OMX_AUDIO_PARAM_PCMMODETYPE& aPcmModeType);
		
	private:
		CAudioDevice(COmxILPcmRendererProcessingFunction& aParent);
		void ConstructL();
		OMX_ERRORTYPE SetPlayFormat();
		void SignalBufferCompletion(OMX_BUFFERHEADERTYPE* apCurrentBuffer);
		TBool IsBufferLengthSufficient(TInt aBufferLength) const;
		TInt GetMinBufferLength() const;
		void SendBufferToSoundDevice(TDes8& aBuffer);
		void StartUpdateTimer();
		void HandleClockStateChanged(const OMX_TIME_MEDIATIMETYPE& aMediaTimeType);
		TBool CanPlayNow();
		void HandleClockScaleChanged(const OMX_TIME_MEDIATIMETYPE& aMediaTimeType);		
		void PauseAudio();

		// From MGenericTimerClient
		static TInt UpdateClockMediaTime(TAny* aPtr );

	private:
		RMdaDevSound iSoundDevice;
		OMX_BUFFERHEADERTYPE* iCurrentBuffer;
		COmxILPcmRendererProcessingFunction& iParent;
		TInt iSampleRate;
		TInt iChannels;
		RMdaDevSound::TMdaSoundEncoding iEncoding;
		TUint iVolume;
		TBool iMuted;
		TUint iBufferSize;
		TTimeIntervalMicroSeconds iVolumeRamp;
		TBool iRampAudioSample;
		TInt iRampSamples;
		TInt iRampSamplesLeft;
		TInt iRampIncr;
		TBool iSkip;
		TBool iClockStateRunning;
		TBool iPausedClockViaScale;
		TBool iIsStartTimeFlagSet;
		OMX_TICKS iClockMediaTime;
		TBool iPlayData;  // To indicate whether it can start rendering audio or not 

		/// If the component is in a state other than OMX_StateExecuting, this
		/// internal variable is used to store the last value of bytes played by
		/// the Sound Device.
		TInt iLastBytesPlayedValue;
		
		/// This is used to send a media time update to the clock component
		CPeriodic* iPeriodic;
		
		// This variable is used to optionally allocate a memory segment to cache OMX buffers if the tunnelled component does not utilise them well
		// If the tunnelled components don't fill in sufficient data in the buffers, this can create underflows in the renderer, as the driver won't have enough data to play
		RBuf8 iCachedPlayBuffer;		
		TBool iResumeAfterNextPlay;

		} *iAudioDevice;

	class CPFHelper : public CActive
		{
	public:
		static CPFHelper* NewL(CAudioDevice& aAudioDevice);
		~CPFHelper();

		// from CActive
		void RunL();
		void DoCancel();

		TInt OpenDevice();
		TInt CloseDevice();
		TInt CloseDeviceOnError();
		TInt Execute();
		TInt Stop();
		TInt CancelDevice();
		TInt BufferIndication();
		TInt MediaTimeIndication(const OMX_TIME_MEDIATIMETYPE& aMediaTimeType);
		TInt ParamIndication(const OMX_AUDIO_PARAM_PCMMODETYPE* aPcmModeType);
		TInt SetVolumeRamp(const OMX_U64 aRampDuration);
		TInt SetVolume(const OMX_S32 aVolumeValue);
		TInt SetMuted(const OMX_BOOL aVolumeValue);
		
		enum TMessageType 
			{
			EOpenDevice,
			ECloseDevice,
			ECloseDeviceOnError,
			EExecuteCommand,
			EStopCommand,
			ECancelCommand,
			EBufferIndication,
			EMediaTimeIndication,
			EParamIndication,
			ESetVolumeRamp,
			ESetVolume,
			ESetMuted
			};

		class TProcMessage
			{
		public:
			TMessageType iType;
			OMX_TIME_MEDIATIMETYPE iMediaTimeType;
			OMX_AUDIO_PARAM_PCMMODETYPE iPcmModeType;
			OMX_U64 iRampDuration;
			OMX_S32 iVolumeValue;
			OMX_BOOL iMuted;
			};
			
		RMsgQueue<TProcMessage> iMsgQueue;
	
	private:
		CPFHelper(CAudioDevice& aAudioDevice);
		void ConstructL();

	private:

		static const TInt KMaxMsgQueueEntries = 150;

		CAudioDevice& iAudioDevice;
		TThreadId iHelperThreadId;
		RSemaphore iCallerSemaphore;

		} *iPFHelper;

	};

#endif // OMXILPCMRENDERERPROCESSINGFUNCTION_H
