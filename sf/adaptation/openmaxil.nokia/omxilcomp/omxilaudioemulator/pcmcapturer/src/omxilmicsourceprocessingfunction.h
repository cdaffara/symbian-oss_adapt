// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//



/**
   @file
   @internalComponent
*/

#ifndef OMXILMICSOURCEPROCESSINGFUNCTION_H
#define OMXILMICSOURCEPROCESSINGFUNCTION_H

#include <d32soundsc.h>
#include <openmax/il/khronos/v1_x/OMX_Component.h>
#include <openmax/il/common/omxilprocessingfunction.h>
#include <e32msgqueue.h>

/**
    Mic Source PF Panic category
*/
_LIT(KOmxILMicSourcePFPanicCategory, "OmxILPcmCapturerProcFunctionPanic");

// Forward declarations
class MOmxILClockComponentCmdsIf;

const TInt KMaxNumberOfSharedBuffers = 6;

NONSHARABLE_CLASS(COmxILMicSourceProcessingFunction) :
	public COmxILProcessingFunction
	{

public:
	static COmxILMicSourceProcessingFunction* NewL(MOmxILCallbackNotificationIf& aCallbacks,
												   MOmxILClockComponentCmdsIf& aClientClockPort);

	~COmxILMicSourceProcessingFunction();

	OMX_ERRORTYPE StateTransitionIndication(TStateIndex aNewState);
	OMX_ERRORTYPE BufferFlushingIndication(TUint32 aPortIndex, OMX_DIRTYPE aDirection);
	OMX_ERRORTYPE ParamIndication(OMX_INDEXTYPE aParamIndex, const TAny* apComponentParameterStructure);
	OMX_ERRORTYPE ConfigIndication(OMX_INDEXTYPE aConfigIndex, const TAny* apComponentConfigStructure);

	OMX_ERRORTYPE BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE aDirection);
	OMX_ERRORTYPE MediaTimeIndication(const OMX_TIME_MEDIATIMETYPE& aMediaTime);

	OMX_BOOL BufferRemovalIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE aDirection);

	OMX_ERRORTYPE DoBufferAllocation(OMX_U32 aSizeBytes, OMX_U8*& apPortSpecificBuffer, OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition);
	void DoBufferDeallocation(OMX_PTR apPortSpecificBuffer, OMX_PTR apPortPrivate);
	OMX_ERRORTYPE DoBufferWrapping(OMX_U32 aSizeBytes, OMX_U8* apBuffer, OMX_PTR& apPortPrivate, OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition);
	void DoBufferUnwrapping(OMX_PTR apPortSpecificBuffer, OMX_PTR apPortPrivate);

	void FillParamPCMModeType(OMX_AUDIO_PARAM_PCMMODETYPE& aPcmModeType) const;
	TInt GetVolume() const;
	
	OMX_TICKS CalculateTimestamp(TUint64 aBytesTransSinceStart);	
    void ReleasePendingBufferIfExists();	
    
private:
	COmxILMicSourceProcessingFunction(MOmxILCallbackNotificationIf& aCallbacks,
									  MOmxILClockComponentCmdsIf& aClientClockPort);
	void ConstructL();

	void FlushBufferList(RPointerArray<OMX_BUFFERHEADERTYPE>& aBufferList);
	OMX_ERRORTYPE PostVolumeChangeEvent();
	OMX_ERRORTYPE PostAudioFormatChangeEvent();

	void ProcessMediaTimeUpdate(const OMX_TIME_MEDIATIMETYPE& aMediaTime);
	void StartRecording();

	static OMX_U32 ConvertEnumToSampleRate(TSoundRate aEnum);
	static TSoundRate ConvertEnumToSampleRate(OMX_U32 aSampleRate);
	static OMX_U32 ConvertEnumToBitsPerSample(TSoundEncoding aEnum);

	void WaitForTransitionToPauseToFinish();
	void TransitionToPauseFinished();	
private:
	OMX_STATETYPE iState;
	RSoundSc iMicSource;
	
	//We have to start recording when:
	//-We receive a call to move to executing state and clock port is disabled
	//-We receive a call to move to executing state and the clock is running or vice versa
	//As a result we must call RecordData() on the driver but may not have an OpenMAX IL
	//buffer to fill.  On completion of this first RecordData() call if we still do not 
	//have an OMX buffer to fill then we store the details of the buffer within the shared
	//chunk in these member variables.  There is no need to queue a second RecordData(),
	//the driver will continue recording.
	TInt iInitialPendingBufferOffset;
	TInt iInitialPendingBufferLength;
	
	static const OMX_U32 KSampleRates[];

	class TSharedChunkBufConfig : public TSharedChunkBufConfigBase
		{
	public:
		TInt iBufferOffsetList[KMaxNumberOfSharedBuffers];
		};

	class TMicrophoneSettings
		{
	public:
		//Settings in RSoundSc
		TInt iVolume;
		TBool iMute;
		TCurrentSoundFormatV02 iCurSoundFormat;
		TSoundFormatsSupportedV02 iSupportedSoundFormat;
		} iMicSourceSettings;

	class CBufferQueue : public CActive
		{
	public:
		static CBufferQueue* NewL(COmxILMicSourceProcessingFunction& aParent);
		~CBufferQueue();

		// from CActive
		void RunL();
		void DoCancel();

		void RecordData();
		void Cleanup();

        void Pause();
        void Stop();
        void Resume();
        
        OMX_ERRORTYPE AllocateBuffer(OMX_U8*& aPortSpecificBuffer, const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition);
		OMX_ERRORTYPE UseBuffer(OMX_U8* apBuffer, const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition);
		void RemoveBuffer(OMX_PTR apPortSpecificBuffer);
		void FreeBuffer(OMX_PTR aPortSpecificBuffer);

		OMX_ERRORTYPE QueueBuffer(OMX_BUFFERHEADERTYPE* apBufferHeader);
		OMX_BOOL RemoveBuffer(OMX_BUFFERHEADERTYPE* apBufferHeader);
		void FlushBuffers(OMX_DIRTYPE aDirection);
		void FillFirstOpenMaxIlBuffer(TInt aChunkOffset, TInt aLengthOfData);

	private:
		CBufferQueue(COmxILMicSourceProcessingFunction& aParent);
		void ConstructL();
		OMX_ERRORTYPE CreateBuffer(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition);
		void RemoveBufferFromQueue(OMX_PTR apPortSpecificBuffer);
		void FillOpenMaxIlBuffer(OMX_BUFFERHEADERTYPE* aBufferHeader, TInt aChunkOffset, TInt aLengthOfData);
		void EmitEOSBuffer();
        void FlushDriver();

	private:
		COmxILMicSourceProcessingFunction& iParent;

		RChunk iSharedChunk;
		TSharedChunkBufConfig iSharedChuckConfig;
		TInt iSharedBufferIndex;
		TInt iRecordedLength;

		TBool iIsSupplier;
		RFastLock iMutex;
		RPointerArray<OMX_BUFFERHEADERTYPE> iBuffersToFill;
		} *iBufferQueue;


	class CCommandsQueue : public CActive
		{
	public:
		static CCommandsQueue* NewL(COmxILMicSourceProcessingFunction& aParent);
		~CCommandsQueue();

		// from CActive
		void RunL();
		void DoCancel();

		TInt Pause();
		TInt Stop();
        TInt StopAndReset();		
		TInt RecordData();
		TInt StartRecording();
		TInt MediaTimeIndication(const OMX_TIME_MEDIATIMETYPE& aMediaTime);
		TInt CleanupBufferQueue();
		TInt CloseDevice();
		TInt Resume();
		
		enum TMessageType
			{
			EMsgPause,
			EMsgStop,
            EMsgStopAndReset,			
			EMsgRecord,
			EMsgStartRecording,
			EMsgMediaTime,
			EMsgCleanupBuffer,
			EMsgCloseDevice,
			EResume
			};

		class TProcMessage
			{
		public:
			TMessageType iType;
			OMX_TIME_MEDIATIMETYPE iMediaTime;
			};

		RMsgQueue<TProcMessage> iMsgQueue;

	private:
		CCommandsQueue(COmxILMicSourceProcessingFunction& aParent);
		void ConstructL();

	private:
		static const TInt KMaxMsgQueueEntries = 10;
		COmxILMicSourceProcessingFunction& iParent;
		} *iCommandsQueue;

	MOmxILClockComponentCmdsIf* ipClientClockPort;
	OMX_TICKS iMediaStartTime;
	TUint iTotalBytesRecorded;
	
	TBool iStartedRecording;     //To ensure we don't initialise recording twice
	TThreadId iOwnerThreadId;
	CActiveSchedulerWait* iTransitionToPauseWait;
	RSemaphore iTransitionToPauseWaitSemaphore;
	};

#endif // OMXILMICSOURCEPROCESSINGFUNCTION_H
