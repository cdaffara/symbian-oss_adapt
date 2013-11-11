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

#include <e32std.h>
#include <openmax/il/khronos/v1_x/OMX_Audio.h>
#include <openmax/il/common/omxilcallbacknotificationif.h>
#include <openmax/il/common/omxilclockcomponentcmdsif.h>
#include <openmax/il/common/omxilspecversion.h>

#include "log.h"
#include "omxilmicsourceprocessingfunction.h"
#include "omxilmicsourceconst.h"

//The maxium number of sample rates the sound driver can support
const TInt KSAMPERATESNUMBER = ESoundRate48000Hz + 1;

const OMX_U32
COmxILMicSourceProcessingFunction::KSampleRates[KSAMPERATESNUMBER] = {7350, 8000, 8820,
																	  9600, 11025, 12000,
																	  14700, 16000,22050,
																	  24000, 29400, 32000,
																	  44100, 48000};

const TInt COmxILMicSourceProcessingFunction::CCommandsQueue::KMaxMsgQueueEntries;

COmxILMicSourceProcessingFunction*
COmxILMicSourceProcessingFunction::NewL(MOmxILCallbackNotificationIf& aCallbacks,
										MOmxILClockComponentCmdsIf& aClientClockPort)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::NewL"));

	COmxILMicSourceProcessingFunction* self =
		new (ELeave)COmxILMicSourceProcessingFunction(aCallbacks, aClientClockPort);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void
COmxILMicSourceProcessingFunction::ConstructL()
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::ConstructL"));

    iTransitionToPauseWait = new(ELeave) CActiveSchedulerWait();
    User::LeaveIfError(iTransitionToPauseWaitSemaphore.CreateLocal(0));

    //record the ID of the creator thread for later use
    iOwnerThreadId = RThread().Id();
    
    //Loading physical sound device
    TInt err = User::LoadPhysicalDevice(KSndPddFileName);
	if(err != KErrNone && err != KErrAlreadyExists)
		{
		User::Leave(err);
		}

    //Loading logical sound driver
	err = User::LoadLogicalDevice(KSndLddFileName);
	if(err != KErrNone && err != KErrAlreadyExists)
		{
		User::Leave(err);
		}


#ifdef SYMBIAN_MICSOURCE_DYNAMIC_SETTINGS
    User::LeaveIfError(iMicSource.Open(KSoundScRxUnit0));

    //Get initial setting from device
    iMicSourceSettings.iVolume = iMicSource.Volume();

    TSoundFormatsSupportedV02Buf capsBuf;
    iMicSource.Caps(capsBuf);
    iMicSourceSettings.iSupportedSoundFormat = capsBuf();

    TCurrentSoundFormatV02Buf audioFormatBuf;
    iMicSource.AudioFormat(audioFormatBuf);
    iMicSourceSettings.iCurSoundFormat = audioFormatBuf();
    iMicSource.Close();
#else //SYMBIAN_MICSOURCE_DYNAMIC_SETTINGS
    iMicSourceSettings.iVolume = KMICSOURCE_VOLUME_DEFAULT;
	iMicSourceSettings.iMute = EFalse;

    //fill in default supported settings
    iMicSourceSettings.iSupportedSoundFormat.iChannels = KMICSOURCE_CHANNELS_SUPPORT;
    iMicSourceSettings.iSupportedSoundFormat.iDataFormats = KMICSOURCE_DATAFORMATS_SUPPORT;
    iMicSourceSettings.iSupportedSoundFormat.iDirection = KMICSOURCE_SOUNDDIRECTION;
    iMicSourceSettings.iSupportedSoundFormat.iEncodings = KMICSOURCE_ENCODING_SUPPORT;
    iMicSourceSettings.iSupportedSoundFormat.iHwConfigNotificationSupport = KMICSOURCE_HWCONFIGNOTIFICATION_SUPPORT;
    iMicSourceSettings.iSupportedSoundFormat.iRates = KMICSOURCE_SAMPLERATES_SUPPORT;
    iMicSourceSettings.iSupportedSoundFormat.iRequestAlignment = KMICSOURCE_REQUESTALIGNMENT_SUPPORT;
    iMicSourceSettings.iSupportedSoundFormat.iRequestMinSize = KMICSOURCE_REQUESTMINSIZE_SUPPORT;

    //fill in default current sound settings
    iMicSourceSettings.iCurSoundFormat.iChannels = KMICSOURCE_CHANNELS_DEFAULT;
    iMicSourceSettings.iCurSoundFormat.iDataFormat = KMICSOURCE_SOUNDDATAFORMAT_DEFAULT;
    iMicSourceSettings.iCurSoundFormat.iEncoding = KMICSOURCE_SOUNDENCODING_DEFAULT;
    iMicSourceSettings.iCurSoundFormat.iRate = KMICSOURCE_SOUNDRATE_DEFAULT;
#endif //SYMBIAN_MICSOURCE_DYNAMIC_SETTINGS

	iBufferQueue = CBufferQueue::NewL(*this);
	iCommandsQueue = CCommandsQueue::NewL(*this);

	iMediaStartTime = 0;
	iTotalBytesRecorded = 0;
	}

COmxILMicSourceProcessingFunction::COmxILMicSourceProcessingFunction(
	MOmxILCallbackNotificationIf& aCallbacks,
	MOmxILClockComponentCmdsIf& aClientClockPort)
	:
	COmxILProcessingFunction(aCallbacks),
	iInitialPendingBufferOffset(-1),
	ipClientClockPort(&aClientClockPort)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::COmxILMicSourceProcessingFunction"));
	}

COmxILMicSourceProcessingFunction::~COmxILMicSourceProcessingFunction()
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::~COmxILMicSourceProcessingFunction"));

    delete iTransitionToPauseWait;
    iTransitionToPauseWaitSemaphore.Close();
    delete iBufferQueue;
    delete iCommandsQueue;
    iMicSource.Close();

	User::FreeLogicalDevice(KDevSoundScName);

	TName pddName(KDevSoundScName);
	pddName.Append(KSndPddWildcardExtension);
	TFindPhysicalDevice findPD(pddName);
	TFullName findResult;
	while(KErrNone == findPD.Next(findResult))
		{
		User::FreePhysicalDevice(findResult);
		findPD.Find(pddName); // Reset the find handle now that we have deleted something from the container.
		}
	}

void COmxILMicSourceProcessingFunction::FillParamPCMModeType(OMX_AUDIO_PARAM_PCMMODETYPE& aPcmModeType) const
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::FillParamPCMModeType"));

	aPcmModeType.eNumData = OMX_NumericalDataSigned;
    aPcmModeType.eEndian = OMX_EndianBig;

    aPcmModeType.bInterleaved =
	(iMicSourceSettings.iCurSoundFormat.iDataFormat == ESoundDataFormatInterleaved)
		? OMX_TRUE : OMX_FALSE;

	aPcmModeType.nBitPerSample = ConvertEnumToBitsPerSample(
		iMicSourceSettings.iCurSoundFormat.iEncoding);

    aPcmModeType.nSamplingRate = ConvertEnumToSampleRate(iMicSourceSettings.iCurSoundFormat.iRate);
	aPcmModeType.ePCMMode = OMX_AUDIO_PCMModeLinear;

    aPcmModeType.nChannels = static_cast<OMX_U32>(iMicSourceSettings.iCurSoundFormat.iChannels);
	if(aPcmModeType.nChannels == 1)
		{
		aPcmModeType.eChannelMapping[0] = OMX_AUDIO_ChannelCF;
		}
	else
		{
		//Assume the device only supports 2 channels as maxium
		//TODO: Add more channel mapping options for different number of channels.
		aPcmModeType.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
		aPcmModeType.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
		}
	}

TInt COmxILMicSourceProcessingFunction::GetVolume() const
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::GetVolume"));

	return iMicSourceSettings.iVolume;
	}

OMX_U32 COmxILMicSourceProcessingFunction::ConvertEnumToSampleRate(TSoundRate aEnum)
	{
	return KSampleRates[aEnum];
	}

OMX_U32 COmxILMicSourceProcessingFunction::ConvertEnumToBitsPerSample(
	TSoundEncoding aEnum)
	{
	OMX_U32 nBitPerSample = 0;
	switch(aEnum)
	{
	case ESoundEncoding8BitPCM:
		nBitPerSample = 8;
		break;
	case ESoundEncoding16BitPCM:
		nBitPerSample = 16;
		break;
	case ESoundEncoding24BitPCM:
		nBitPerSample = 24;
		break;
	default:
		// Assume the default is 16 bits
		nBitPerSample = 16;
		break;
	};

	return nBitPerSample;
	}

TSoundRate COmxILMicSourceProcessingFunction::ConvertEnumToSampleRate(OMX_U32 aSampleRate)
	{
	TSoundRate rate = ESoundRate48000Hz;
	for(TInt index = 0; index < KSAMPERATESNUMBER; ++index)
		{
		if(KSampleRates[index] == aSampleRate)
			{
			rate = static_cast<TSoundRate>(index);
			break;
			}
		}

	return rate;
	}


OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::StateTransitionIndication(TStateIndex aNewState)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::StateTransitionIndication"));

    OMX_ERRORTYPE err = OMX_ErrorNone;
	switch(aNewState)
		{
	case EStateExecuting:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StateExecuting"));

		OMX_STATETYPE previousState = iState;
        iState = OMX_StateExecuting;
		if (ipClientClockPort->IsClockComponentAvailable())
		    {
		    // Only record if clock is running
		    OMX_TIME_CONFIG_CLOCKSTATETYPE clockState;
		    OMX_ERRORTYPE omxError = ipClientClockPort->GetClockState(clockState);
		    if (OMX_ErrorNone != omxError)
		        {
		        iCallbacks.ErrorEventNotification(omxError);
		        break;     
		        }
		    if (OMX_TIME_ClockStateRunning != clockState.eState)
		        {
		        break;
		        }
		    }
		
		TInt error(KErrGeneral);
		if(previousState == OMX_StatePause) //going from Paused to Executing -> resume recording
		    {
		    error = iCommandsQueue->Resume();
		    }
		else
		    {
		    error = iCommandsQueue->StartRecording();
		    }
		
        if (error != KErrNone)
			{
			//TODO: should not we roll back to the previous state here? ie: iState = previousState;
			return OMX_ErrorInsufficientResources;
			}
		}
		break;
	case EStateInvalid:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StateInvalid"));
		iStartedRecording = EFalse;
		
		if(iState == OMX_StateExecuting || iState == OMX_StatePause)
			{
			iCommandsQueue->CleanupBufferQueue();
			iCommandsQueue->CloseDevice();
			}

		iState = OMX_StateInvalid;
		}
		break;
	case EStatePause:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StatePause"));

		if(iState == OMX_StateExecuting) //only do anything iff pausing in executing state
		    {
		    iStartedRecording = EFalse;
            if (iCommandsQueue->Pause() != KErrNone)
                {
                iState = OMX_StateExecuting; //transition to Paused state failed; roll back state to Exectuing
                return OMX_ErrorInsufficientResources;
                }
            WaitForTransitionToPauseToFinish(); //blocking this state transition here until Bufferqueue finishes processing data
		    }
		else //going from a state other than Executing -> no action needed
		    {
		    iState = OMX_StatePause;
		    }
		}
		break;
	case EStateIdle:
		{
		iStartedRecording = EFalse;
		
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StateIdle"));
		iState = OMX_StateIdle;
		}
		break;
	case EStateLoaded:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StateLoaded"));

		iCommandsQueue->CloseDevice();
		iState = OMX_StateLoaded;
		}
		break;
	case EStateWaitForResources:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StateLoaded, OMX_StateWaitForResources"));
		if(iState == OMX_StateExecuting || iState == OMX_StatePause)
			{
			if (iCommandsQueue->StopAndReset() != KErrNone)
				{
				return OMX_ErrorInsufficientResources;
				}

			if (iCommandsQueue->CleanupBufferQueue() != KErrNone)
				{
				return OMX_ErrorInsufficientResources;
				}
			}
		iState = OMX_StateWaitForResources;
		}
		break;
	case ESubStateLoadedToIdle:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : ESubStateLoadedToIdle"));

		//always cleanup the buffer queue before other initialization
		iBufferQueue->Cleanup();

		if (iMicSource.Open(KSoundScRxUnit0) != KErrNone)
			{
			return OMX_ErrorInsufficientResources;
			}

		TInt currentVolume = iMicSource.Volume();
		TInt volume = (iMicSourceSettings.iMute) ? 0 : iMicSourceSettings.iVolume;
		if(currentVolume != volume && KErrNone != iMicSource.SetVolume(volume))
			{
			iMicSourceSettings.iVolume = currentVolume;
			iMicSourceSettings.iMute = EFalse;
			err = PostVolumeChangeEvent();
			}

		TPckgBuf<TCurrentSoundFormatV02> curSoundFormatBuf(iMicSourceSettings.iCurSoundFormat);
		if(KErrNone != iMicSource.SetAudioFormat(curSoundFormatBuf))
			{
		    TCurrentSoundFormatV02Buf audioFormatBuf;
		    iMicSource.AudioFormat(audioFormatBuf);
		    iMicSourceSettings.iCurSoundFormat = audioFormatBuf();
			err = PostAudioFormatChangeEvent();
			}
		}
		break;
	case ESubStateIdleToLoaded:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : ESubStateIdleToLoaded"));
		if (iCommandsQueue->CleanupBufferQueue() != KErrNone)
			{
			return OMX_ErrorInsufficientResources;
			}
		}
		break;
	case ESubStateExecutingToIdle:
	case ESubStatePauseToIdle:
		{
		if (iCommandsQueue->StopAndReset() != KErrNone)
			{
			return OMX_ErrorInsufficientResources;
			}
		}
		break;
	default:
		{
		return OMX_ErrorIncorrectStateTransition;
		}
		};

	return err;
	}

OMX_ERRORTYPE COmxILMicSourceProcessingFunction::PostVolumeChangeEvent()
	{
	OMX_AUDIO_CONFIG_VOLUMETYPE volumeType;
	volumeType.sVolume.nValue = iMicSourceSettings.iVolume;
	volumeType.sVolume.nMin = KMICSOURCE_VOLUME_MIN;
	volumeType.sVolume.nMax = KMICSOURCE_VOLUME_MAX;
	volumeType.nSize = sizeof(OMX_AUDIO_CONFIG_VOLUMETYPE);
	volumeType.bLinear = OMX_TRUE;
	volumeType.nPortIndex = KMICSOURCE_APB0PORT_INDEX;
	volumeType.nVersion = TOmxILVersion(KMicSourceComponentVersionMajor,
										KMicSourceComponentVersionMinor,
										KMicSourceComponentVersionRevision,
										KMicSourceComponentVersionStep);

	TPckgBuf<OMX_AUDIO_CONFIG_VOLUMETYPE> volumeTypeBuf(volumeType);
	return iCallbacks.PortSettingsChangeNotification(KMICSOURCE_APB0PORT_INDEX,
													 OMX_IndexConfigAudioVolume,
													 volumeTypeBuf);
	}

OMX_ERRORTYPE COmxILMicSourceProcessingFunction::PostAudioFormatChangeEvent()
	{
	OMX_AUDIO_PARAM_PCMMODETYPE pcmProfile;
	pcmProfile.nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
	pcmProfile.nVersion = TOmxILVersion(KMicSourceComponentVersionMajor,
										KMicSourceComponentVersionMinor,
										KMicSourceComponentVersionRevision,
										KMicSourceComponentVersionStep);
	pcmProfile.nPortIndex = KMICSOURCE_APB0PORT_INDEX;

	FillParamPCMModeType(pcmProfile);

	TPckgBuf<OMX_AUDIO_PARAM_PCMMODETYPE> pcmProfileBuf(pcmProfile);
	return iCallbacks.PortSettingsChangeNotification(KMICSOURCE_APB0PORT_INDEX,
													OMX_IndexParamAudioPcm,
													pcmProfileBuf);

	}

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::BufferFlushingIndication(TUint32 aPortIndex,
															OMX_DIRTYPE aDirection)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::BufferFlushingIndication"));

    if ((aPortIndex == OMX_ALL && aDirection == OMX_DirMax) ||
		(aPortIndex == 0 && aDirection == OMX_DirOutput))
		{
		if(iCommandsQueue->Stop() != KErrNone)
			{
			return OMX_ErrorInsufficientResources;
			}
		iBufferQueue->FlushBuffers(OMX_DirOutput);
		}
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::ParamIndication(OMX_INDEXTYPE aParamIndex,
												const TAny* apComponentParameterStructure)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::ParamIndication"));

    OMX_ERRORTYPE err = OMX_ErrorNone;
    switch(aParamIndex)
	{
	case OMX_IndexParamAudioPcm:
		{
		const OMX_AUDIO_PARAM_PCMMODETYPE* pPcmProfile
			= static_cast<const OMX_AUDIO_PARAM_PCMMODETYPE*>(
				apComponentParameterStructure);

		TCurrentSoundFormatV02 newSoundFormat;
		newSoundFormat.iChannels = pPcmProfile->nChannels;
		newSoundFormat.iReserved1 = 0;
		if(0 == (iMicSourceSettings.iSupportedSoundFormat.iChannels & (KSoundMonoChannel << (newSoundFormat.iChannels - 1))))
			{
			err = OMX_ErrorUnsupportedSetting;
			break;
			}

		newSoundFormat.iDataFormat = (pPcmProfile->bInterleaved == OMX_TRUE)
									 ? ESoundDataFormatInterleaved : ESoundDataFormatNonInterleaved;
		if(0 == (iMicSourceSettings.iSupportedSoundFormat.iDataFormats & (KSoundDataFormatInterleaved << newSoundFormat.iDataFormat)))
			{
			err = OMX_ErrorUnsupportedSetting;
			break;
			}

		switch(pPcmProfile->nBitPerSample)
			{
			case 8:
				newSoundFormat.iEncoding = ESoundEncoding8BitPCM;
				break;
			case 24:
				newSoundFormat.iEncoding = ESoundEncoding24BitPCM;
				break;
			default:
				newSoundFormat.iEncoding = ESoundEncoding16BitPCM;
				break;
			}
		if(0 == (iMicSourceSettings.iSupportedSoundFormat.iEncodings & (KSoundEncoding8BitPCM << newSoundFormat.iEncoding)))
			{
			err = OMX_ErrorUnsupportedSetting;
			break;
			}

		newSoundFormat.iRate = ConvertEnumToSampleRate(pPcmProfile->nSamplingRate);
		if(0 == (iMicSourceSettings.iSupportedSoundFormat.iRates & (KSoundRate7350Hz << newSoundFormat.iRate)))
			{
			err = OMX_ErrorUnsupportedSetting;
			break;
			}

		if(iState > OMX_StateLoaded && iState != OMX_StateWaitForResources)
			{
			TPckgBuf<TCurrentSoundFormatV02> curSoundFormatBuf(newSoundFormat);
			if(KErrNone != iMicSource.SetAudioFormat(curSoundFormatBuf))
				{
				return OMX_ErrorHardware;
				}
			}

		iMicSourceSettings.iCurSoundFormat = newSoundFormat;
		}
		break;
	default:
		{
		// Ignore other port param changes...
		}
		break;
	};

	return err;
	}

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::ConfigIndication(OMX_INDEXTYPE aConfigIndex,
													const TAny* apComponentConfigStructure)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::ConfigIndication"));

    OMX_ERRORTYPE err = OMX_ErrorNone;
    switch(aConfigIndex)
    {
	case OMX_IndexConfigAudioVolume:
		{
		const OMX_AUDIO_CONFIG_VOLUMETYPE* pVolumeType
			= static_cast<const OMX_AUDIO_CONFIG_VOLUMETYPE*>(apComponentConfigStructure);

		if((pVolumeType->bLinear == OMX_TRUE) &&
		  (pVolumeType->sVolume.nMax == 255) &&
		  (pVolumeType->sVolume.nMin == 0) &&
		  (pVolumeType->sVolume.nValue <= pVolumeType->sVolume.nMax) &&
		  (pVolumeType->sVolume.nValue >= pVolumeType->sVolume.nMin))
			{
			if(iState > OMX_StateLoaded && iState != OMX_StateWaitForResources)
				{
				if(KErrNone != iMicSource.SetVolume(pVolumeType->sVolume.nValue))
					{
					return OMX_ErrorHardware;
					}
				}
			iMicSourceSettings.iVolume = pVolumeType->sVolume.nValue;
			iMicSourceSettings.iMute = EFalse;
			}
		else
			{
			err = OMX_ErrorBadParameter;
			}
		}
		break;

	case OMX_IndexConfigAudioMute:
		{
		const OMX_AUDIO_CONFIG_MUTETYPE* pVolumeType
			= static_cast<const OMX_AUDIO_CONFIG_MUTETYPE*>(apComponentConfigStructure);

		if(pVolumeType->bMute && !iMicSourceSettings.iMute)
			{
			if(iState > OMX_StateLoaded && iState != OMX_StateWaitForResources)
				{
				if(iMicSource.SetVolume(0) != KErrNone)
					{
					return OMX_ErrorHardware;
					}
				}
			iMicSourceSettings.iMute = ETrue;
			}
		else if(!pVolumeType->bMute && iMicSourceSettings.iMute)
			{
			if(iState > OMX_StateLoaded && iState != OMX_StateWaitForResources)
				{
				if(iMicSource.SetVolume(iMicSourceSettings.iVolume) != KErrNone)
					{
					return OMX_ErrorHardware;
					}
				}
			iMicSourceSettings.iMute = EFalse;
			}
		}
		break;

	default:
		{
		// Ignore other port config changes...
		}
		break;
	};

	return err;
	}

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader,
													OMX_DIRTYPE aDirection)
	{
    DEBUG_PRINTF2(_L8("COmxILMicSourceProcessingFunction::BufferIndication : [%X]"), apBufferHeader);

    if (aDirection == OMX_DirOutput)
		{
		if(iState > OMX_StateLoaded && iState != OMX_StateWaitForResources)
			{
			//Don't queue buffer unless we are in executing/pause state
			return iBufferQueue->QueueBuffer(apBufferHeader);
			}
		else
			{
			return OMX_ErrorNotReady;
			}
		}
    else
		{
		return OMX_ErrorBadParameter;
		}
	}

OMX_BOOL
COmxILMicSourceProcessingFunction::BufferRemovalIndication(OMX_BUFFERHEADERTYPE* apBufferHeader,
														   OMX_DIRTYPE /* aDirection */)
	{
    DEBUG_PRINTF2(_L8("COmxILMicSourceProcessingFunction::BufferRemovalIndication : BUFFER [%X]"), apBufferHeader);
	return iBufferQueue->RemoveBuffer(apBufferHeader);
	}

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::DoBufferAllocation(OMX_U32 /* aSizeBytes */,
													  OMX_U8*& apPortSpecificBuffer,
													  OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition)
	{
	return iBufferQueue->AllocateBuffer(apPortSpecificBuffer, aPortDefinition);
	}

void COmxILMicSourceProcessingFunction::DoBufferDeallocation(OMX_PTR apPortSpecificBuffer,
															 OMX_PTR /* apPortPrivate */)
	{
	iBufferQueue->FreeBuffer(apPortSpecificBuffer);
	}

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::DoBufferWrapping(OMX_U32 /* aSizeBytes */,
													OMX_U8* apBuffer,
													OMX_PTR& /* apPortPrivate */,
													OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition)
	{
	return iBufferQueue->UseBuffer(apBuffer, aPortDefinition);
	}

void COmxILMicSourceProcessingFunction::DoBufferUnwrapping(OMX_PTR apPortSpecificBuffer,
														   OMX_PTR /* apPortPrivate */)
	{
	iBufferQueue->RemoveBuffer(apPortSpecificBuffer);
	}

void COmxILMicSourceProcessingFunction::ReleasePendingBufferIfExists()
    {
    if (iInitialPendingBufferOffset >= 0)
        {
        iMicSource.ReleaseBuffer(iInitialPendingBufferOffset);
        iInitialPendingBufferOffset = -1;
        }
    }

OMX_TICKS COmxILMicSourceProcessingFunction::CalculateTimestamp(TUint64 aBytesTransSinceStart)
    {
    OMX_S64 bitsPerSample = ConvertEnumToBitsPerSample(iMicSourceSettings.iCurSoundFormat.iEncoding);
    OMX_S64 sampleRate = ConvertEnumToSampleRate(iMicSourceSettings.iCurSoundFormat.iRate);
    TUint64 bytesPerSec = (bitsPerSample / 8) * sampleRate;
    TUint64 bytesTicksPerSec = aBytesTransSinceStart * static_cast<TUint64>(OMX_TICKS_PER_SECOND);
	bytesTicksPerSec /= iMicSourceSettings.iCurSoundFormat.iChannels;

    // Calculate the presentation time (rounded up to nearest
    // integer). The media start time + presentation time is the
    // timestamp for the first sample of each buffer.
    OMX_TICKS  currentAudioPresentationTime = ((bytesTicksPerSec + (bytesPerSec/2))/bytesPerSec);

    // Set the buffer timestamp and start flag if needed
    return (iMediaStartTime + currentAudioPresentationTime);    
    }

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::MediaTimeIndication(const OMX_TIME_MEDIATIMETYPE& aMediaTime)
	{
	DEBUG_PRINTF5(_L8("COmxILMicSourceProcessingFunction::MediaTimeIndication : eUpdateType = %d eState = %d xScale = %d nMediaTimestamp = %d "), aMediaTime.eUpdateType, aMediaTime.eState, aMediaTime.xScale, aMediaTime.nMediaTimestamp);

	// Only OMX_TIME_UpdateClockStateChanged is supported. The verification is done here to avoid overloading the commands queue with bogus updates in the component conformance suite which treats clock port as any other inpuit port
	if (OMX_TIME_UpdateClockStateChanged != aMediaTime.eUpdateType)
		{
		return OMX_ErrorNone;
		}	
	
	if (iCommandsQueue->MediaTimeIndication(aMediaTime) != KErrNone)
		{
		return OMX_ErrorInsufficientResources;
		}

	return OMX_ErrorNone;
	}

void COmxILMicSourceProcessingFunction::StartRecording()
	{
	// Ensure that we don't issue RecordData twice				
	if (iStartedRecording)
		return;

	iStartedRecording = ETrue;
	if (!ipClientClockPort->IsClockComponentAvailable())
		{
		//Don't reset iTotalBytesRecorded or iMediaStartTime here as this method is called when going from Paused->Exec, not just Idle->Exec
		// If we transitioned to Idle previously, then the counters would've reset at that point
		iBufferQueue->RecordData();
		}
	else
		{
		//Implicit that we are clock is running, checks performed when message queued
		// Reinitialise our timestamp calculation to the current media time and begin recording
		// In the presence of a clock component, we use its media time as our starting point, so we reset all the aggregated counters, to avoid jumping ahead in time					
		iTotalBytesRecorded = 0;
		iMicSource.ResetBytesTransferred();
		ipClientClockPort->GetMediaTime(iMediaStartTime);
		iBufferQueue->RecordData();
		}	
	}
	
void
COmxILMicSourceProcessingFunction::ProcessMediaTimeUpdate(const OMX_TIME_MEDIATIMETYPE& aMediaTime)
	{
	DEBUG_PRINTF5(_L8("COmxILMicSourceProcessingFunction::ProcessMediaTimeUpdate : eUpdateType[%d] eState[%d] xScale[%d] nMediaTimestamp[%d]"), aMediaTime.eUpdateType, aMediaTime.eState, aMediaTime.xScale, aMediaTime.nMediaTimestamp);

	// Only processing of clock state changes are supported (no seeking) - the verification of the correct update type was done before adding the message to the command queue
	ASSERT(OMX_TIME_UpdateClockStateChanged == aMediaTime.eUpdateType);

	// Process here the clock state changes:
	// 
	// 1 - Transition to running:
	//
	// - If executing trigger a call to initialise our copy of the media time and start recording
	//
	// 2 - Transition to WaitForStartTime:
	//
	// - Simply report audio start time as the current media time
	//
	// 3 - Transition to Stopped
	//
	// - Stop recording, all counters reinitialised if recording resumes

	// TODO: Consider what to do with media time updates in all states, not
	// only EXE and PAUSE


	OMX_ERRORTYPE omxError = OMX_ErrorNone;
	switch(aMediaTime.eState)
		{
	case OMX_TIME_ClockStateRunning:
		{
		if (iState == OMX_StateExecuting)
			{
			StartRecording();
			}
		}
		break;

	case OMX_TIME_ClockStateWaitingForStartTime:
		{
		iStartedRecording = EFalse;
		// For now, only considering this could happen during Exe or Pause
		if (iState == OMX_StateExecuting || iState == OMX_StatePause)
			{
			// Send start time
			OMX_TICKS mediaTime;
			omxError = ipClientClockPort->GetMediaTime(mediaTime);
			if (OMX_ErrorNone == omxError)
				{
				// Ignore the error returned
				ipClientClockPort->SetStartTime(mediaTime);
				}
			}
		DEBUG_PRINTF2(_L8("COmxILMicSourceProcessingFunction::ProcessMediaTimeUpdate : GetWallTime returned [%d]"), omxError);
		}
		break;

	case OMX_TIME_ClockStateStopped:
		{
		iStartedRecording = EFalse;
		// For now, only considering this in Exe and Pause
		if (iState == OMX_StateExecuting || iState == OMX_StatePause)
			{
			ReleasePendingBufferIfExists();
			iMicSource.CancelRecordData();
			}
		}
		break;

	default:
		{
		DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::ProcessMediaTimeUpdate : Ignoring invalid clock state"));
		}
		};
	}

void COmxILMicSourceProcessingFunction::WaitForTransitionToPauseToFinish()
    {
    if(RThread().Id() == iOwnerThreadId)
        {
        //if the owner thread is the same thread as the one created the active objects in this processing function
        //then we can wait by using CActiveSchedulerWait
        DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::WaitForTransitionToPauseToFinish - blocking transition to pause with active scheduler wait now"));
        iTransitionToPauseWait->Start();
        }
    else
        {
        //if this is a thread different from the creator thread then semaphore is needed to block this thread until the transition
        //to paused state completes
        DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::WaitForTransitionToPauseToFinish - blocking thread with semaphore now"));
        iTransitionToPauseWaitSemaphore.Wait();
        }
    }

void COmxILMicSourceProcessingFunction::TransitionToPauseFinished()
    {
    if(iTransitionToPauseWait->IsStarted())
        {
        DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::TransitionToPauseFinished - unblocking transition to pause (active scheduler wait) now"));
        iTransitionToPauseWait->AsyncStop();
        }
    else
        {
        DEBUG_PRINTF(_L8("COmxILMicSourceProcessingFunction::TransitionToPauseFinished - unblocking transition to pause (semaphore) now"));
        iTransitionToPauseWaitSemaphore.Signal();
        }
    }


COmxILMicSourceProcessingFunction::CCommandsQueue* COmxILMicSourceProcessingFunction::CCommandsQueue::NewL(COmxILMicSourceProcessingFunction& aParent)
	{
	CCommandsQueue* self = new (ELeave) CCommandsQueue(aParent);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

COmxILMicSourceProcessingFunction::CCommandsQueue::CCommandsQueue(COmxILMicSourceProcessingFunction& aParent)
: CActive(EPriorityStandard),
  iParent(aParent)
	{
	CActiveScheduler::Add(this);
	}

void COmxILMicSourceProcessingFunction::CCommandsQueue::ConstructL()
	{
	User::LeaveIfError(iMsgQueue.CreateLocal(KMaxMsgQueueEntries));
	iMsgQueue.NotifyDataAvailable(iStatus);
	SetActive();
	}

COmxILMicSourceProcessingFunction::CCommandsQueue::~CCommandsQueue()
	{
	Cancel();
	iMsgQueue.Close();
	}

void COmxILMicSourceProcessingFunction::CCommandsQueue::RunL()
	{
	TProcMessage msg;
	while (iMsgQueue.Receive(msg)==KErrNone)
		{
	    DEBUG_PRINTF2(_L8("COmxILMicSourceProcessingFunction::CCommandsQueue::RunL : msg.iType[%d]"), msg.iType);
		switch (msg.iType)
			{
			case EMsgStop:
				{
				//This variant, which does not reset the timestamping counters, is needed for when recording without a clock, have received a flush buffer indication, yet have not been moved to Idle
	            iParent.ReleasePendingBufferIfExists();
				iParent.iBufferQueue->Stop();
				break;
				}
            case EMsgStopAndReset:
                {
                //The timestamping conters are reset here for when recording without a clock
                iParent.iTotalBytesRecorded = 0;
                iParent.iMediaStartTime = 0;
                iParent.ReleasePendingBufferIfExists();
                iParent.iBufferQueue->Stop();
                iParent.iMicSource.ResetBytesTransferred();
                break;
                }				
			case EMsgPause:
				{
				iParent.iBufferQueue->Pause();
                iParent.iState = OMX_StatePause;
				break;
				}
			
			case EMsgMediaTime:
				{
				iParent.ProcessMediaTimeUpdate(msg.iMediaTime);
				break;
				}

			case EMsgRecord:
				{
				if (iParent.iInitialPendingBufferOffset >= 0)
					{
					iParent.iBufferQueue->FillFirstOpenMaxIlBuffer(
						iParent.iInitialPendingBufferOffset,
						iParent.iInitialPendingBufferLength);
					iParent.iInitialPendingBufferOffset = -1;
					}
				else
					{
					iParent.iBufferQueue->RecordData();
					}
				break;
				}

            case EMsgStartRecording:
                {
				iParent.StartRecording();
                break;
                }				
				
			case EMsgCleanupBuffer:
				{
				iParent.iBufferQueue->Cleanup();
				}
				break;
			case EMsgCloseDevice:
				{
				if(iParent.iMicSource.Handle())
					{
					iParent.iMicSource.Close();
					}
				}
				break;
				
			case EResume:
                {
                iParent.iBufferQueue->Resume();
                }
                break;
			    
			default:
				{
				break;
				}
			}
		}

	// setup for next callbacks
	iMsgQueue.NotifyDataAvailable(iStatus);
	SetActive();
	}

void COmxILMicSourceProcessingFunction::CCommandsQueue::DoCancel()
	{
	iMsgQueue.CancelDataAvailable();
	}

TInt COmxILMicSourceProcessingFunction::CCommandsQueue::Pause()
	{
	DEBUG_PRINTF2(_L8("CCommandsQueue::Pause : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = EMsgPause;
	return iMsgQueue.Send(message);
	}

TInt COmxILMicSourceProcessingFunction::CCommandsQueue::Stop()
	{
	DEBUG_PRINTF2(_L8("CCommandsQueue::Stop : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = EMsgStop;
	return iMsgQueue.Send(message);
	}

TInt COmxILMicSourceProcessingFunction::CCommandsQueue::StopAndReset()
    {
    DEBUG_PRINTF2(_L8("CCommandsQueue::StopAndReset : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
    TProcMessage message;
    message.iType = EMsgStopAndReset;
    return iMsgQueue.Send(message);
    }

TInt COmxILMicSourceProcessingFunction::CCommandsQueue::RecordData()
	{
	DEBUG_PRINTF2(_L8("CCommandsQueue::RecordData : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	TProcMessage message;
	message.iType = EMsgRecord;

	return iMsgQueue.Send(message);
	}

TInt COmxILMicSourceProcessingFunction::CCommandsQueue::StartRecording()
    {
    DEBUG_PRINTF2(_L8("CCommandsQueue::StartRecording : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

    TProcMessage message;
    message.iType = EMsgStartRecording;

    return iMsgQueue.Send(message);
    }

TInt COmxILMicSourceProcessingFunction::CCommandsQueue::MediaTimeIndication(
	const OMX_TIME_MEDIATIMETYPE& aMediaTime)
	{
	DEBUG_PRINTF2(_L8("CCommandsQueue::MediaTimeIndication : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	TProcMessage message;
	message.iType = EMsgMediaTime;
	message.iMediaTime = aMediaTime;

	return iMsgQueue.Send(message);
	}

TInt COmxILMicSourceProcessingFunction::CCommandsQueue::CleanupBufferQueue()
	{
	DEBUG_PRINTF2(_L8("CCommandsQueue::CleanupBufferQueue : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = EMsgCleanupBuffer;
	return iMsgQueue.Send(message);
	}

TInt COmxILMicSourceProcessingFunction::CCommandsQueue::CloseDevice()
	{
	DEBUG_PRINTF2(_L8("CCommandsQueue::CloseDevice : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = EMsgCloseDevice;
	return iMsgQueue.Send(message);
	}

TInt COmxILMicSourceProcessingFunction::CCommandsQueue::Resume()
    {
    DEBUG_PRINTF2(_L8("CCommandsQueue::Resume : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
    TProcMessage message;
    message.iType = EResume;
    return iMsgQueue.Send(message);
    }

COmxILMicSourceProcessingFunction::CBufferQueue* COmxILMicSourceProcessingFunction::CBufferQueue::NewL(COmxILMicSourceProcessingFunction& aParent)
	{
	CBufferQueue* self = new (ELeave) CBufferQueue(aParent);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
    return self;
	}

COmxILMicSourceProcessingFunction::CBufferQueue::CBufferQueue(COmxILMicSourceProcessingFunction& aParent)
	:CActive(EPriorityStandard),iParent(aParent), iIsSupplier(ETrue)
	{
	CActiveScheduler::Add(this);
	}

void COmxILMicSourceProcessingFunction::CBufferQueue::ConstructL()
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::ConstructL : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	User::LeaveIfError(iMutex.CreateLocal());
	}

COmxILMicSourceProcessingFunction::CBufferQueue::~CBufferQueue()
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::~CBufferQueue : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	Cancel();

	iBuffersToFill.Reset();
	if(iSharedChunk.Handle())
		{
		iSharedChunk.Close();
		}

	if(iParent.iMicSource.Handle())
		{
		iParent.iMicSource.Close();
		}

	iMutex.Close();
	}

void COmxILMicSourceProcessingFunction::CBufferQueue::RecordData()
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::RecordData : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	iMutex.Wait();
	if(!IsActive())
		{
		iParent.iMicSource.RecordData(iStatus, iRecordedLength);
		SetActive();
		}
	iMutex.Signal();
	}

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::CBufferQueue::AllocateBuffer(OMX_U8*& aPortSpecificBuffer,
																const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition)
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::AllocateBuffer : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	if (!iSharedChunk.Handle())
		{
		OMX_ERRORTYPE err;
		if(OMX_ErrorNone != (err = CreateBuffer(aPortDefinition)))
			{
			return err;
			}

		iIsSupplier = ETrue;
		iSharedBufferIndex = 0;
		}
	else
		{
		++iSharedBufferIndex;
		}

	ASSERT(iSharedBufferIndex < iSharedChuckConfig.iNumBuffers);
	aPortSpecificBuffer = iSharedChunk.Base() + iSharedChuckConfig.iBufferOffsetList[iSharedBufferIndex];
	DEBUG_PRINTF2(_L8("CBufferQueue::AllocateBuffer: [%X]"), aPortSpecificBuffer);
	return OMX_ErrorNone;
	}


OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::CBufferQueue::UseBuffer(OMX_U8* /* apBuffer */,
														   const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition)
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::UseBuffer : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	if(!iSharedChunk.Handle() || (iSharedChuckConfig.iBufferSizeInBytes < aPortDefinition.nBufferSize))
		{
		OMX_ERRORTYPE err;
		if(OMX_ErrorNone != (err = CreateBuffer(aPortDefinition)))
			{
			return err;
			}

		iIsSupplier = EFalse;
		}
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::CBufferQueue::CreateBuffer(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition)
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::CreateBuffer : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	iSharedChuckConfig.iNumBuffers = aPortDefinition.nBufferCountActual;
	iSharedChuckConfig.iBufferSizeInBytes = aPortDefinition.nBufferSize;
	iSharedChuckConfig.iFlags = KScFlagBufOffsetListInUse;

	//Enforce the sound driver to stop recording.
	Stop();

	TPckgBuf<TSharedChunkBufConfigBase> configBuf(iSharedChuckConfig);
	RChunk chunk;
	TInt ret = iParent.iMicSource.SetBufferChunkCreate(configBuf, chunk);
	if(ret == KErrNone)
		{
		//RSoundSc::SetBufferChunkCreate give the chunk a handle owned by the thread only
		//So we need to change the chunk handle to be shared by the whole process.
		RThread thread;
		iSharedChunk.SetHandle(chunk.Handle());
		ret = iSharedChunk.Duplicate(thread);
		thread.Close();

		//Get the actual buffer configuration after the buffer creating call succeed.
		TPtr8 sharedChunkBufConfigBuf(reinterpret_cast<TUint8*>(&iSharedChuckConfig), sizeof(TSharedChunkBufConfig));
		iParent.iMicSource.GetBufferConfig(sharedChunkBufConfigBuf);
		}

	chunk.Close();

	OMX_ERRORTYPE err;
	switch(ret)
		{
		case KErrNone:
			err = OMX_ErrorNone;
			break;
		case KErrNoMemory:
			err = OMX_ErrorInsufficientResources;
			break;
		case KErrInUse:
			err = OMX_ErrorIncorrectStateOperation;
			break;
		default:
			err = OMX_ErrorBadParameter;
			break;
		};

	return err;
	}

OMX_ERRORTYPE
COmxILMicSourceProcessingFunction::CBufferQueue::QueueBuffer(OMX_BUFFERHEADERTYPE* apBufferHeader)
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::QueueBuffer: [%X]"), apBufferHeader->pBuffer);

	if(iIsSupplier)
		{
		//Empty buffer called from tunnelled port/client. Call on device to release the buffer
		iParent.iMicSource.ReleaseBuffer(apBufferHeader->pBuffer - iSharedChunk.Base());
		}

	iMutex.Wait();
		
	iBuffersToFill.Append(apBufferHeader);
	iMutex.Signal();

	// If we are not in the executing state we want to queue the buffer to be filled but not start recording
	if (iParent.iState != OMX_StateExecuting)
	    {
	    return OMX_ErrorNone;
	    }
	    
	TInt err = iParent.iCommandsQueue->RecordData();
	
	return (err == KErrNone) ? OMX_ErrorNone : OMX_ErrorInsufficientResources;
	}

OMX_BOOL
COmxILMicSourceProcessingFunction::CBufferQueue::RemoveBuffer(OMX_BUFFERHEADERTYPE* apBufferHeader)
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::RemoveBuffer1: [%X]"), apBufferHeader->pBuffer);

	TInt headerIndexInArray = KErrNotFound;

	iMutex.Wait();
	if (KErrNotFound !=	(headerIndexInArray = iBuffersToFill.Find(apBufferHeader)))
		{
		iBuffersToFill.Remove(headerIndexInArray);
		}
	iMutex.Signal();

	return (headerIndexInArray == KErrNotFound) ? OMX_FALSE : OMX_TRUE;
	}

void COmxILMicSourceProcessingFunction::CBufferQueue::RemoveBuffer(OMX_PTR apPortSpecificBuffer)
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::RemoveBuffer2 [%X]"), apPortSpecificBuffer);

	if(!iIsSupplier)
		{
		iMutex.Wait();
		RemoveBufferFromQueue(apPortSpecificBuffer);
		iMutex.Signal();
		}
	}


void COmxILMicSourceProcessingFunction::CBufferQueue::FreeBuffer(OMX_PTR apPortSpecificBuffer)
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::FreeBuffer [%X]"), apPortSpecificBuffer);

	iMutex.Wait();
	if(iIsSupplier && iSharedChunk.Handle())
		{
		RemoveBufferFromQueue(apPortSpecificBuffer);
		TBool found(EFalse);
		for(TInt index = 0; index < iSharedChuckConfig.iNumBuffers; ++index)
			{
			OMX_PTR bufferAllocated = iSharedChunk.Base() + iSharedChuckConfig.iBufferOffsetList[index];
			if(apPortSpecificBuffer == bufferAllocated)
				{
				found = ETrue;
				break;
				}
			}

		if(found && (--iSharedBufferIndex < 0))
			{
			//All buffers have been release by client
			iSharedChunk.Close();
			}
		}
	iMutex.Signal();
	}

void COmxILMicSourceProcessingFunction::CBufferQueue::RemoveBufferFromQueue(OMX_PTR apPortSpecificBuffer)
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::RemoveBufferFromQueue : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	const TUint bufferCount = iBuffersToFill.Count();
	OMX_BUFFERHEADERTYPE* pBufferHeader = 0;

	for (TUint i=0; i < bufferCount; ++i)
		{
		pBufferHeader = iBuffersToFill[i];
		if(pBufferHeader->pBuffer == apPortSpecificBuffer)
			{
			iBuffersToFill.Remove(i);
			break;
			}
		}
	}


void COmxILMicSourceProcessingFunction::CBufferQueue::FlushBuffers(OMX_DIRTYPE aDirection)
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::FlushBuffers : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	iMutex.Wait();
	const TUint bufferCount = iBuffersToFill.Count();
	OMX_BUFFERHEADERTYPE* pBufferHeader = 0;
	for (TUint i=0; i < bufferCount; ++i)
		{
		pBufferHeader = iBuffersToFill[i];
		pBufferHeader->nFilledLen = 0;
		iParent.iCallbacks.BufferDoneNotification(
				pBufferHeader,
				pBufferHeader->nOutputPortIndex,
				aDirection
				);
		}
	// Empty buffer lists...
	iBuffersToFill.Reset();

	iMutex.Signal();
	}

void COmxILMicSourceProcessingFunction::CBufferQueue::RunL()
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::RunL : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	TInt chunkOffset = iStatus.Int(); 
	if(iParent.iState != OMX_StateExecuting && iParent.iState != OMX_StatePause)
        {
        if (chunkOffset >= 0)
            {
            iParent.iMicSource.ReleaseBuffer(chunkOffset);
            }
        return;
        }
	    
	    
    TBool queueRecordRequest(ETrue); // whether we queue another RecordData request, if currently recording the driver will continue recording regardless of whether we make another call to RecordData	
	if(chunkOffset < 0) // When negative the request status = error code
        {
        switch(chunkOffset)
            {
            case KErrCancel:
                //Recording is paused or stoped
                if(iParent.iState == OMX_StatePause)
                    {
                    // Handling transition to pause.
                    // Normally we issue RecordData requests for all data buffered in the driver, when the
                    // driver runs out of data it immediately completes with KErrCancel and we send an empty
                    // buffer marked with EOS. 
                    // nb. It is possible that all buffers returned during a pause are full length (though unlikely).
                    // There is special handling in the buffer processing code so that if we run out of OMX buffers
                    // before emptying the driver, we will mark the last OMX buffer with EOS and flush the driver.
                    EmitEOSBuffer();
                    iParent.TransitionToPauseFinished(); //the processing function can now complete the transition to paused state
                    }
                else //print out an error msg
                    {
                    DEBUG_PRINTF2(_L8("CBufferQueue::RunL : [KErrCancel] received in iParent.iState = [%d]"), static_cast<TInt>(iParent.iState));
                    }
                queueRecordRequest = EFalse;
                break;
            case KErrInUse:
                //It's overflow error in term of OMX conception: The buffer was not available when it was needed.
                DEBUG_PRINTF(_L8("CBufferQueue::RunL : KErrInUse"));
                iParent.iCallbacks.ErrorEventNotification(OMX_ErrorOverflow);
                break;
            case KErrOverflow:
				{
                //RSoundSc has to overwrite pre-filled buffer.
                //It's underflow error in term of OMX conception: buffer is filled before next is available.
                DEBUG_PRINTF(_L8("CBufferQueue::RunL : KErrOverflow"));
                iParent.iCallbacks.ErrorEventNotification(OMX_ErrorUnderflow);
                
                //In the event of an overflow our internal byte counter has fallen  
                //behind the one in the driver by a number of buffers, we have no way
                //of knowing how many, so we reinitialise.  Throwing away the data in the
                //driver also gives us time to catch up and will result in one, longer loss of audio
				//as opposed to potentially numerous, small ones
                TInt upToDateBytesTrans = iParent.iMicSource.BytesTransferred();
                iParent.ReleasePendingBufferIfExists();
                iParent.iMicSource.CancelRecordData();
                iParent.iMicSource.ResetBytesTransferred();
                iParent.iMicSource.RecordData(iStatus, iRecordedLength);
                SetActive();
                iParent.iMediaStartTime += iParent.CalculateTimestamp(static_cast<TUint64>(upToDateBytesTrans));
				}
                break;
            default:
                queueRecordRequest = EFalse;
                DEBUG_PRINTF2(_L8("CBufferQueue::RunL : [%d] -> OMX_ErrorHardware"), iStatus.Int());
                iParent.iCallbacks.ErrorEventNotification(OMX_ErrorHardware);
                break;
            };    
        }
    else    // When positive the request status = offset of buffer in the chunk
        {
		iMutex.Wait();  
        if (iBuffersToFill.Count() > 0)
            {
            OMX_BUFFERHEADERTYPE* pBufferHeader = iBuffersToFill[0];
            iBuffersToFill.Remove(0);
            TInt buffersToFill = iBuffersToFill.Count();
 			iMutex.Signal();

			if((iParent.iState == OMX_StatePause) && (buffersToFill <= 0))
			    {
		        DEBUG_PRINTF(_L8("CBufferQueue::RunL - In transition to Pause. No more OMX buffers available to queue more RecodData. Forcing EOS and stopping recording!"));
		        // We are pausing and have run out of OMX buffers so can not continue retrieving data from the driver,
			    // therefore mark this buffer as the last one, send it out and discard further data.
			    pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                FillOpenMaxIlBuffer(pBufferHeader, chunkOffset, iRecordedLength);
			    queueRecordRequest = EFalse;
			    FlushDriver();
		        iParent.TransitionToPauseFinished();
			    }
			else
			    {
	            FillOpenMaxIlBuffer(pBufferHeader, chunkOffset, iRecordedLength);
			    }

            // Update the clock component with the current audio time using
            // BytesTransferred() (rounded up to nearest integer). The
            // reference time is the current media time (as seen from the audio
            // device) to be notified to the clock component.
            OMX_S64 bitsPerSample = ConvertEnumToBitsPerSample(iParent.iMicSourceSettings.iCurSoundFormat.iEncoding);
            OMX_S64 sampleRate = ConvertEnumToSampleRate(iParent.iMicSourceSettings.iCurSoundFormat.iRate);
            TUint64 bytesPerSec = (bitsPerSample / 8) * sampleRate;
            
            TUint64 bytesTicksPerSec = static_cast<TUint64>(iParent.iMicSource.BytesTransferred()) * static_cast<TUint64>(OMX_TICKS_PER_SECOND);
            bytesTicksPerSec /= iParent.iMicSourceSettings.iCurSoundFormat.iChannels;
            OMX_TICKS audioReferenceTime = ((bytesTicksPerSec + (bytesPerSec/2))/bytesPerSec);
            
            // The return code is ignored intentionally, since if the clock component is missing or cannot accept the audio reference, it should not affect the main processing
            (void)iParent.ipClientClockPort->SetAudioReference(iParent.iMediaStartTime + audioReferenceTime);
			
            DEBUG_PRINTF2(_L8("CBufferQueue::RunL : bitsPerSample = [%d]"), bitsPerSample);
            DEBUG_PRINTF2(_L8("CBufferQueue::RunL : sampleRate = [%d]"), sampleRate);
            DEBUG_PRINTF2(_L8("CBufferQueue::RunL : bytesPerSec = [%d]"), bytesPerSec);
            DEBUG_PRINTF2(_L8("CBufferQueue::RunL : number of channels = [%d]"), iParent.iMicSourceSettings.iCurSoundFormat.iChannels);

            DEBUG_PRINTF2(_L8("CBufferQueue::RunL : iMicSource.BytesTransferred = [%d]"), iParent.iMicSource.BytesTransferred());
            DEBUG_PRINTF2(_L8("CBufferQueue::RunL : bytesTicksPerSec = [%lu]"), bytesTicksPerSec);
            DEBUG_PRINTF2(_L8("CBufferQueue::RunL : audioReferenceTime = [%d]"), audioReferenceTime);
            DEBUG_PRINTF2(_L8("CBufferQueue::RunL : approx. [%f] sec data recorded from the driver so far"), (static_cast<TReal>(iParent.iTotalBytesRecorded)/static_cast<TReal>(iParent.iMicSourceSettings.iCurSoundFormat.iChannels))/static_cast<TReal>(bytesPerSec));
            
            }
        else
            {
			// Unlock mutex that was lock before if statement
			iMutex.Signal();
            __ASSERT_DEBUG(iParent.iInitialPendingBufferOffset < 0, User::Panic(KOmxILMicSourcePFPanicCategory, 1));  // If we still have a buffer handed to us from the driver, that has not been copied into an OpenMAX buffer, we should never have queued another RecordData() request
            queueRecordRequest = EFalse;    // the driver will continue recording, we will queue another RecordData() when this buffer has been used

            if(iParent.iState == OMX_StatePause)
                {
                DEBUG_PRINTF(_L8("CBufferQueue::RunL - In transition to Pause. Data coming in from driver and no OMX buffer available to copy into. Unblocking client and stopping recording!"));
                // We are pausing and have run out of OMX buffers so can not continue retrieving data from the driver,
                // therefore unblock client. At this point we can not send any EOS buffers.
                FlushDriver();
                iParent.TransitionToPauseFinished();
                }

            iParent.iInitialPendingBufferOffset = chunkOffset;
            iParent.iInitialPendingBufferLength = iRecordedLength;
            }
        }
    iMutex.Wait();
	TInt buffersToFillCount = iBuffersToFill.Count();
    DEBUG_PRINTF2(_L8("CBufferQueue::RunL - number of buffers to fill: [%d]"), buffersToFillCount);
	iMutex.Signal();

    if (queueRecordRequest && (buffersToFillCount > 0) && !IsActive())
        {
        SetActive();
        iParent.iMicSource.RecordData(iStatus, iRecordedLength);
        DEBUG_PRINTF(_L8("CBufferQueue::RunL - issuing RecordData again"));
        }    


	}

void COmxILMicSourceProcessingFunction::CBufferQueue::DoCancel()
	{
    DEBUG_PRINTF2(_L8("CBufferQueue::DoCancel : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	if(iParent.iMicSource.Handle())
		{
		iParent.ReleasePendingBufferIfExists();
		Stop();
		}
	}

void COmxILMicSourceProcessingFunction::CBufferQueue::Cleanup()
	{
	DEBUG_PRINTF2(_L8("CBufferQueue::Cleanup : IsActive[%s]"), (IsActive() ? "YES" : "NO"));

	if(IsActive())
		{
		Cancel();
		}

	iMutex.Wait();
	iBuffersToFill.Reset();
	if(iSharedChunk.Handle())
		{
		iSharedChunk.Close();
		}
	iMutex.Signal();
	}


void COmxILMicSourceProcessingFunction::CBufferQueue::FillFirstOpenMaxIlBuffer(
	TInt aChunkOffset,
	TInt aLengthOfData)
    {
	iMutex.Wait();
	if (iBuffersToFill.Count() > 0)
		{
		OMX_BUFFERHEADERTYPE* pBuffer = iBuffersToFill[0];
		iBuffersToFill.Remove(0);
		// Better to unlock mutext before calling callback manager
		iMutex.Signal();
		FillOpenMaxIlBuffer(pBuffer, aChunkOffset, aLengthOfData);
		}
	else
		{
		iMutex.Signal();
		}
	}

void COmxILMicSourceProcessingFunction::CBufferQueue::FillOpenMaxIlBuffer(OMX_BUFFERHEADERTYPE* aBufferHeader, TInt aChunkOffset, TInt aLengthOfData)
    {
    //************************************************************************************************************
    // TODO: TEMPORARY WORKAROUND - If the length of data provided by the driver is greater than the buffer size,
    // we set the the specificied data length to the maximum buffer length specified in the buffer header. This is
    // to prevent descriptor overflow during the copy.
    
    // This is a known issue within the LDD (DTW-MM00769)   
    if(aLengthOfData > aBufferHeader->nAllocLen)
        {
        aLengthOfData = aBufferHeader->nAllocLen;
        }
    //************************************************************************************************************
    
    aBufferHeader->nFilledLen = aLengthOfData;
    aBufferHeader->nOffset = 0;
    if(iIsSupplier)
        {
        //We are supplier so just use the buffer in the shared chunk
        aBufferHeader->pBuffer = iSharedChunk.Base() + aChunkOffset;
        }
    else
        {
        //Have to do the data copy from shared chunk to supplied buffer
        TPtr8 ptr(aBufferHeader->pBuffer, aBufferHeader->nAllocLen);
        ptr.Copy(iSharedChunk.Base() + aChunkOffset, aLengthOfData);
        iParent.iMicSource.ReleaseBuffer(aChunkOffset);
        }

    // Set the buffer timestamp and start flag if needed
    aBufferHeader->nTimeStamp = iParent.CalculateTimestamp(static_cast<TUint64>(iParent.iTotalBytesRecorded));
    if (iParent.iTotalBytesRecorded == 0)
        {
        aBufferHeader->nFlags |= OMX_BUFFERFLAG_STARTTIME;
        }

    // Update the total bytes recorded
    iParent.iTotalBytesRecorded += aLengthOfData;

    DEBUG_PRINTF2(_L8("CBufferQueue::FillOpenMaxIlBuffer : startTime = [%d]"), iParent.iMediaStartTime);
    DEBUG_PRINTF2(_L8("CBufferQueue::FillOpenMaxIlBuffer : iTotalBytesRecorded = [%d]"), iParent.iTotalBytesRecorded);
    DEBUG_PRINTF2(_L8("CBufferQueue::FillOpenMaxIlBuffer : OMX_TICKS_PER_SECOND = [%d]"), OMX_TICKS_PER_SECOND);
    DEBUG_PRINTF2(_L8("CBufferQueue::FillOpenMaxIlBuffer : nTimeStamp = [%d]"), aBufferHeader->nTimeStamp);

    iParent.iCallbacks.BufferDoneNotification(aBufferHeader, aBufferHeader->nOutputPortIndex, OMX_DirOutput); 
    }

void COmxILMicSourceProcessingFunction::CBufferQueue::Pause()
    {
    DEBUG_PRINTF2(_L8("CBufferQueue::Pause : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
    iParent.iMicSource.Pause();
    if(!IsActive()) //no outstanding recorddata 
        {
        DEBUG_PRINTF(_L8("CBufferQueue::Pause - Not active. Seinding out EOS buffer and flushing driver!"));
        // Almost certainly completly out of OMX buffers, but there is a slim chance that we have 
        // just been given a buffer and there is a RecordData command queued behind this pause command...
        // In this case the EmitEOSBuffer will send an empty buffer marked with EOS.
        EmitEOSBuffer();
        FlushDriver();
        iParent.TransitionToPauseFinished();
        }
    //if BufferQueue is active it means that there is outstanding recorddata request
    //therefore the transition to Paused state will be completed at a later point in RunL 
    }

void COmxILMicSourceProcessingFunction::CBufferQueue::Stop()
    {
    DEBUG_PRINTF2(_L8("CBufferQueue::Stop : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
    iParent.iMicSource.CancelRecordData();
    }

void COmxILMicSourceProcessingFunction::CBufferQueue::Resume()
    {
    DEBUG_PRINTF(_L8("CBufferQueue::Resume"));
	// Make sure that to discard a partial buffer aggregated from the previous run (if exists). See the header file for more detailed explanation on iInitialPendingBufferOffset
	iParent.iInitialPendingBufferOffset = -1;
    if(!IsActive())
        {
        iParent.iMicSource.Resume();
        SetActive();
        iParent.iMicSource.RecordData(iStatus, iRecordedLength);
        }
    }


void COmxILMicSourceProcessingFunction::CBufferQueue::EmitEOSBuffer()
    {
    DEBUG_PRINTF(_L8("CBufferQueue::EmitEOSBuffer"));
    iMutex.Wait();  
    if (iBuffersToFill.Count() > 0)
        {
        OMX_BUFFERHEADERTYPE* pBufferHeader = iBuffersToFill[0];
        iBuffersToFill.Remove(0);
        iMutex.Signal();
        pBufferHeader->nFilledLen = 0;
        pBufferHeader->nTimeStamp = iParent.CalculateTimestamp(static_cast<TUint64>(iParent.iTotalBytesRecorded));
        pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
        iParent.iCallbacks.BufferDoneNotification(pBufferHeader, pBufferHeader->nOutputPortIndex, OMX_DirOutput);
        DEBUG_PRINTF(_L8("CBufferQueue::EmitEOSBuffer - Sending out empty OMX buffer marked with EOS flag!"));
        }
    else
        {
        iMutex.Signal();
        DEBUG_PRINTF(_L8("CBufferQueue::EmitEOSBuffer - Error! No OMX bufferheader available to mark with EOS flag!"));
        }
    }

void COmxILMicSourceProcessingFunction::CBufferQueue::FlushDriver()
    {
    DEBUG_PRINTF(_L8("CBufferQueue::FlushDriver"));
    TRequestStatus status;
    TInt recordedLength;
    TInt error(KErrNone);
    //flush the driver: fetch all buffers and throw them away until KErrCancel is received
    //obviously this assumes that a Pause or Stop has been isssued prior to this
    do  {
        iParent.iMicSource.RecordData(status, recordedLength);
        User::WaitForRequest(status);
        error = status.Int();
        if(error >= 0)
            {
            DEBUG_PRINTF3(_L8("CBufferQueue::FlushDriver - throwing away [%d] bytes of data from driver buffer [%d]"), error, recordedLength);
            iParent.iMicSource.ReleaseBuffer(error);
            }
        } while(error!=KErrCancel);
    
    //Now correct the number of bytes received
    iParent.iTotalBytesRecorded = iParent.iMicSource.BytesTransferred();
    }
