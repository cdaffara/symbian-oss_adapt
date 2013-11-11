/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <openmax/il/khronos/v1_x/OMX_Audio.h>

#include <d32soundsc.h>

#include "log.h"
#include <openmax/il/common/omxilcallbacknotificationif.h>
#include <openmax/il/common/omxilclockcomponentcmdsif.h>
#include <openmax/il/shai/OMX_Symbian_AudioExt.h>
#include "omxilpcmrendererprocessingfunction.h"

const TInt COmxILPcmRendererProcessingFunction::CPFHelper::KMaxMsgQueueEntries;

COmxILPcmRendererProcessingFunction*
COmxILPcmRendererProcessingFunction::NewL(MOmxILCallbackNotificationIf& aCallbacks, 
										MOmxILClockComponentCmdsIf& aClientClockPort)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererProcessingFunction::NewL"));

	COmxILPcmRendererProcessingFunction* self =
		new (ELeave)COmxILPcmRendererProcessingFunction(aCallbacks, aClientClockPort);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;

	}

void
COmxILPcmRendererProcessingFunction::ConstructL()
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererProcessingFunction::ConstructL"));

    iAudioDevice = CAudioDevice::NewL(*this);
    iPFHelper = CPFHelper::NewL(*iAudioDevice);
	}

COmxILPcmRendererProcessingFunction::COmxILPcmRendererProcessingFunction(
	MOmxILCallbackNotificationIf& aCallbacks, 
	MOmxILClockComponentCmdsIf& aClientClockPort)
	:
	COmxILProcessingFunction(aCallbacks),
	iClientClockPortPtr(&aClientClockPort)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererProcessingFunction::COmxILPcmRendererProcessingFunction"));

	}

COmxILPcmRendererProcessingFunction::~COmxILPcmRendererProcessingFunction()
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererProcessingFunction::~COmxILPcmRendererProcessingFunction"));

	// Check in case the Sound Device has not been closed. That would happen in
	// an scenario where the component is not being deleted in an orderer way.
	if(iAudioDevice && iPFHelper &&
	   (iState == OMX_StateInvalid   ||
	    iState == OMX_StateExecuting ||
	    iState == OMX_StatePause))
		{
		// Ignore error if the following call fails
		iPFHelper->CloseDeviceOnError();
		}

	// Buffer headers are not owned by the processing function
    iBuffersToEmpty.Close();
    iBuffersEmptied.Close();
    delete iAudioDevice;
    delete iPFHelper;
	}


OMX_ERRORTYPE
COmxILPcmRendererProcessingFunction::StateTransitionIndication(TStateIndex aNewState)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererProcessingFunction::StateTransitionIndication"));

    OMX_ERRORTYPE err = OMX_ErrorNone;
	switch(aNewState)
		{
	case EStateExecuting:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StateExecuting"));
		if (iPFHelper->Execute() != KErrNone)
			{
			return OMX_ErrorInsufficientResources;
			}
		}
		break;
	case EStateInvalid:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StateInvalid"));
		if (iPFHelper->Stop() != KErrNone)
			{ // InsufficientResources to stop???
			return OMX_ErrorInsufficientResources;
			}
		}
		break;
	case EStatePause:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StatePause"));
		err = iAudioDevice->MoveToPausedState();
		}
		break;
	case EStateIdle:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StateIdle"));
		iBuffersToEmpty.Reset();
		if (iPFHelper->Stop() != KErrNone)
			{ // InsufficientResources to stop???
			return OMX_ErrorInsufficientResources;
			}
		}
		break;
	case EStateLoaded:
	case EStateWaitForResources:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : OMX_StateLoaded, OMX_StateWaitForResources"));
		if (iPFHelper->Stop() != KErrNone)
			{ // InsufficientResources to stop???
			return OMX_ErrorInsufficientResources;
			}
		}
		break;
	case ESubStateLoadedToIdle:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : ESubStateLoadedToIdle"));
		if (iPFHelper->OpenDevice() != KErrNone)
			{
			return OMX_ErrorInsufficientResources;
			}
		}
		break;
	case ESubStateIdleToLoaded:
		{
		DEBUG_PRINTF(_L8("StateTransitionIndication : ESubStateIdleToLoaded"));
		if (iPFHelper->CloseDevice() != KErrNone)
			{ // InsufficientResources to close???
			return OMX_ErrorInsufficientResources;
			}
		}
		break;
	case ESubStateExecutingToIdle:
	case ESubStatePauseToIdle:
		{
		// Ignore these transitions...
		return OMX_ErrorNone;
		}
	default:
		{
		// Always ASSERT; This would be a problem in the framework.
		ASSERT(0);
		return OMX_ErrorIncorrectStateTransition;
		}
		};

	return err;

	}


OMX_ERRORTYPE
COmxILPcmRendererProcessingFunction::BufferFlushingIndication(
	TUint32 aPortIndex,
	OMX_DIRTYPE aDirection)
	{
    DEBUG_PRINTF2(_L8("COmxILPcmRendererProcessingFunction::BufferFlushingIndication : aPortIndex[%d]"), aPortIndex);

    if ((aPortIndex == OMX_ALL && aDirection == OMX_DirMax) ||
		(aPortIndex == KPCMRENDERER_APB0PORT_INDEX && aDirection == OMX_DirInput))
		{
		// If we are currently processing a buffer then cancel
		if (iPFHelper->CancelDevice() != KErrNone)
			{
			return OMX_ErrorInsufficientResources;
			}

		// Send BufferDone notifications for each emptied buffer...
		FlushBufferList(iBuffersEmptied);

	    // Send BufferDone notifications for each pending buffer...
		FlushBufferList(iBuffersToEmpty);
		
		return OMX_ErrorNone;
		}
	else if (aPortIndex == KPCMRENDERER_OPB0PORT_INDEX && aDirection == OMX_DirInput)
		{
		// Since the clock port buffers are returned immediately, 
		// there's nothing to flush for the port
		return OMX_ErrorNone;
		}
	else
		{
		// Always ASSERT; This would be a problem in the framework.
		ASSERT(0);
		return OMX_ErrorBadParameter;
		}
	}

void
COmxILPcmRendererProcessingFunction::FlushBufferList(
	RPointerArray<OMX_BUFFERHEADERTYPE>& aBufferList)
	{
    DEBUG_PRINTF2(_L8("COmxILPcmRendererProcessingFunction::FlushBufferList : [%s]"),
				  &aBufferList == &iBuffersToEmpty ? "iBuffersToEmpty" : "iBuffersEmptied");

	const TUint bufferCount = aBufferList.Count();
	OMX_BUFFERHEADERTYPE* pBufferHeader = 0;
	// We know there is only one input port...
	OMX_DIRTYPE portDirection = OMX_DirInput;

	for (TUint i=0; i<bufferCount; ++i)
		{
		pBufferHeader = aBufferList[i];
		pBufferHeader->nFilledLen = 0;
		iCallbacks.
			BufferDoneNotification(
				pBufferHeader,
				pBufferHeader->nInputPortIndex,
				portDirection
				);
		}

	// Empty buffer list...
	aBufferList.Reset();

	}


OMX_ERRORTYPE
COmxILPcmRendererProcessingFunction::ParamIndication(
	OMX_INDEXTYPE aParamIndex,
	const TAny* apComponentParameterStructure)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererProcessingFunction::ParamIndication"));

    OMX_ERRORTYPE err = OMX_ErrorNone;
    switch(aParamIndex)
	{
	case OMX_IndexParamAudioPcm:
		{
		const OMX_AUDIO_PARAM_PCMMODETYPE* pPcmProfile
			= static_cast<const OMX_AUDIO_PARAM_PCMMODETYPE*>(
				apComponentParameterStructure);

		if((pPcmProfile->nChannels == 1 || pPcmProfile->nChannels == 2) &&
		  (pPcmProfile->eNumData == OMX_NumericalDataSigned) &&
		  (pPcmProfile->eEndian == OMX_EndianBig) &&
		  (pPcmProfile->bInterleaved == OMX_TRUE) &&
		  (pPcmProfile->nBitPerSample == 16) &&
		  ((pPcmProfile->nSamplingRate == 8000)  ||
		   (pPcmProfile->nSamplingRate == 11025) ||
		   (pPcmProfile->nSamplingRate == 12000) ||
		   (pPcmProfile->nSamplingRate == 16000) ||
		   (pPcmProfile->nSamplingRate == 22050) ||
		   (pPcmProfile->nSamplingRate == 24000) ||
		   (pPcmProfile->nSamplingRate == 32000) ||
		   (pPcmProfile->nSamplingRate == 44100) ||
		   (pPcmProfile->nSamplingRate == 48000)) &&
		  (pPcmProfile->ePCMMode == OMX_AUDIO_PCMModeLinear) &&
		  (pPcmProfile->eChannelMapping[0] == OMX_AUDIO_ChannelLF) &&
		  (pPcmProfile->eChannelMapping[1] == OMX_AUDIO_ChannelRF))
			{
			if (iPFHelper->ParamIndication(pPcmProfile) != KErrNone)
				{
				err = OMX_ErrorInsufficientResources;
				}
			}
		else
			{
			err = OMX_ErrorBadParameter;
			}
		}
		break;
	default:
		{
		// Ignore other port param changes...
		}
	};

	return err;

	}

OMX_ERRORTYPE
COmxILPcmRendererProcessingFunction::ConfigIndication(OMX_INDEXTYPE aConfigIndex,
													  const TAny* apComponentConfigStructure)
	{
    DEBUG_PRINTF2(_L8("COmxILPcmRendererProcessingFunction::ConfigIndication %X"), aConfigIndex);

    OMX_ERRORTYPE err = OMX_ErrorNone;
    switch(aConfigIndex)
	{
	case OMX_SYMBIANINDEXCONFIGAUDIOPCMVOLUMERAMP:
		{
		const OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE*
			pPcmVolumeRamp
			= static_cast<
			const OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE*>(
					apComponentConfigStructure);

		if (iPFHelper->SetVolumeRamp(pPcmVolumeRamp->nRampDuration) != KErrNone)
			{
			err = OMX_ErrorInsufficientResources;
			}
		}
		break;

	case OMX_IndexConfigAudioVolume:
		{
		const OMX_AUDIO_CONFIG_VOLUMETYPE* pVolumeType
			= static_cast<const OMX_AUDIO_CONFIG_VOLUMETYPE*>(
					apComponentConfigStructure);

        if (pVolumeType->bLinear == OMX_TRUE)
            {
            // Some configuration structures contain read-only fields. The 
            // OMX_SetConfig method will preserve read-only fields in configuration
            // structures that contain them, and shall not generate an error when 
            // the caller attempts to change the value of a read-only field.
            err = OMX_ErrorNone;
            break;
            }
			
		if ((pVolumeType->sVolume.nValue <= pVolumeType->sVolume.nMax) &&
		   (pVolumeType->sVolume.nValue >= pVolumeType->sVolume.nMin))
			{
			if (iPFHelper->SetVolume(pVolumeType->sVolume.nValue) != KErrNone)
				{
				err = OMX_ErrorInsufficientResources;
				}
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
		= static_cast<const OMX_AUDIO_CONFIG_MUTETYPE*>(
				apComponentConfigStructure);

		if (iPFHelper->SetMuted(pVolumeType->bMute) != KErrNone)
			{
			err = OMX_ErrorInsufficientResources;
			}
		}
		break;

	default:
		{
		// Ignore other port config changes...
		}
	};

	return err;

	}

OMX_ERRORTYPE
COmxILPcmRendererProcessingFunction::BufferIndication(
	OMX_BUFFERHEADERTYPE* apBufferHeader,
	OMX_DIRTYPE aDirection)
	{
    DEBUG_PRINTF2(_L8("COmxILPcmRendererProcessingFunction::BufferIndication : [%X]"), apBufferHeader);

	if (aDirection != OMX_DirInput)
		{
		return OMX_ErrorBadParameter;
		}
    
	if (iBuffersToEmpty.Append(apBufferHeader) != KErrNone)
		{
		return OMX_ErrorInsufficientResources;
		}

	// If we are not in an executing state or if the audio device is busy, delay playing back the buffer
	if (iState != OMX_StateExecuting || iAudioDevice->IsActive())
		{
		return OMX_ErrorNone;
		}
				
	if (iPFHelper->BufferIndication() != KErrNone)
		{
		return OMX_ErrorInsufficientResources;
		}

	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::MediaTimeIndication(const OMX_TIME_MEDIATIMETYPE& aMediaTimeType)
	{
	// Received a requested media time notification.
	DEBUG_PRINTF5(_L8("MediaTimeIndication : eUpdateType = %d eState = %d xScale = %d nMediaTimestamp = %d "), 
						aMediaTimeType.eUpdateType, aMediaTimeType.eState, aMediaTimeType.xScale, aMediaTimeType.nMediaTimestamp);
	
	iPFHelper->MediaTimeIndication(aMediaTimeType);
	return OMX_ErrorNone;
	}

OMX_BOOL
COmxILPcmRendererProcessingFunction::BufferRemovalIndication(
	OMX_BUFFERHEADERTYPE* apBufferHeader,
	OMX_DIRTYPE /* aDirection */)
	{
    DEBUG_PRINTF2(_L8("COmxILPcmRendererProcessingFunction::BufferRemovalIndication : BUFFER [%X]"), apBufferHeader);

    TBool headerDeletionResult = ETrue;
    // Check if the buffer we want to remove is the one is being currently processed
    if (iAudioDevice->IsActive() && iAudioDevice->GetCurrentBuffer() == apBufferHeader)
		{
		if (iPFHelper->CancelDevice() != KErrNone)
			{
			return OMX_FALSE;
			}

		// if you cancel the audio device then you send the buffer to the other end of the tunnel
		// so you shouldn't say that you had the buffer in the processing function in this situation.
		headerDeletionResult = EFalse;
		}
    else
		{
		TInt headerIndexInArray = KErrNotFound;
		if (KErrNotFound !=
			(headerIndexInArray =
			 iBuffersToEmpty.Find(apBufferHeader)))
			{
			iBuffersToEmpty.Remove(headerIndexInArray);
			}
		else if(KErrNotFound !=
				(headerIndexInArray =
				 iBuffersEmptied.Find(apBufferHeader)))
			{
			iBuffersEmptied.Remove(headerIndexInArray);
			}
		else
			{
			headerDeletionResult = EFalse;
			}
		}

    DEBUG_PRINTF2(_L8("BufferRemovalIndication : Removal result [%s]"), (headerDeletionResult ? "YES" : "NO"));
    return (headerDeletionResult ? OMX_TRUE : OMX_FALSE);
	}

TInt
COmxILPcmRendererProcessingFunction::GetBytesPlayed() const
	{
	return iAudioDevice->GetBytesPlayed();
	}


COmxILPcmRendererProcessingFunction::CAudioDevice* COmxILPcmRendererProcessingFunction::CAudioDevice::NewL(COmxILPcmRendererProcessingFunction& aParent)
	{
	CAudioDevice* self = new (ELeave) CAudioDevice(aParent);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

COmxILPcmRendererProcessingFunction::CAudioDevice::CAudioDevice(COmxILPcmRendererProcessingFunction& aParent)
: CActive(EPriorityUserInput),
  iParent(aParent),
  iSampleRate(KDefaultSampleRate),
  iChannels(KDefaultNumberChannels),
  iEncoding(KDefaultEncoding),
  iVolume(KDefaultVolume),
  iMuted(KDefaultMuted),
  iBufferSize(KBufferSize),
  iClockStateRunning(EFalse),
  iPausedClockViaScale(EFalse),
  iIsStartTimeFlagSet(EFalse)
	{
	CActiveScheduler::Add(this);
	}


void COmxILPcmRendererProcessingFunction::CAudioDevice::ConstructL()
    {
	iCachedPlayBuffer.CreateL(0);
	}

COmxILPcmRendererProcessingFunction::CAudioDevice::~CAudioDevice()
	{
	delete iPeriodic;
	Cancel();
	iCachedPlayBuffer.Close();
	}

void COmxILPcmRendererProcessingFunction::CAudioDevice::RunL()
	{
	DEBUG_PRINTF(_L8("CAudioDevice::RunL : "));
	if (iStatus != KErrNone)
		{
		switch(iStatus.Int())
			{
		case KErrUnderflow:
			DEBUG_PRINTF(_L8("CAudioDevice::RunL : KErrUnderflow"));
			iParent.iCallbacks.ErrorEventNotification(OMX_ErrorUnderflow);
			break;

		case KErrOverflow:
			DEBUG_PRINTF(_L8("CAudioDevice::RunL : KErrOverflow"));
			iParent.iCallbacks.ErrorEventNotification(OMX_ErrorOverflow);
			break;

		default:
			DEBUG_PRINTF2(_L8("CAudioDevice::RunL : [%d] -> OMX_ErrorHardware"), iStatus.Int());
			iParent.iCallbacks.ErrorEventNotification(OMX_ErrorHardware);
			};
		}

	ASSERT(iCurrentBuffer);
	// Update the last value of bytes played...
	iLastBytesPlayedValue = iSoundDevice.BytesPlayed();

	// Return the emptied buffer to the IL Client or the tunnelled
	// component..
	SignalBufferCompletion(iCurrentBuffer);
	iCurrentBuffer = 0;
		
	// Make sure to clear the aggregated cache buffer, if it was used
	iCachedPlayBuffer.Zero();
	}

void COmxILPcmRendererProcessingFunction::CAudioDevice::SignalBufferCompletion(
	OMX_BUFFERHEADERTYPE* apCurrentBuffer)
	{
	DEBUG_PRINTF2(_L8("CAudioDevice::SignalBufferCompletion : BUFFER = [%X]"), apCurrentBuffer);

	iParent.iBuffersEmptied.Append(apCurrentBuffer);

	// Process the queue only if in executing state...
	if (iParent.iState == OMX_StateExecuting)
		{
		const TUint bufferCount = iParent.iBuffersEmptied.Count();
		OMX_BUFFERHEADERTYPE* pBufferHeader = 0;
		for (TUint i=0; i<bufferCount; ++i)
			{
			pBufferHeader = iParent.iBuffersEmptied[i];
			TBool lastBuffer = EFalse;
			if (pBufferHeader->nFlags & OMX_BUFFERFLAG_EOS)
				{
				lastBuffer = ETrue;
				}

			pBufferHeader->nFilledLen = 0;
			iParent.iCallbacks.BufferDoneNotification(pBufferHeader,
													  pBufferHeader->nInputPortIndex,
													  OMX_DirInput);
			if (lastBuffer)
				{
				pBufferHeader->nFlags |= OMX_BUFFERFLAG_EOS;
				// propagate the EOS flag
				iParent.iCallbacks.EventNotification(
					OMX_EventBufferFlag,
					KPCMRENDERER_APB0PORT_INDEX,
					pBufferHeader->nFlags,
					NULL);
				}
			}

		// Empty list...
		iParent.iBuffersEmptied.Reset();
		}

	if (iParent.iBuffersToEmpty.Count() > 0)
		{
		DEBUG_PRINTF2(_L8("CAudioDevice::RunL : iBuffersToEmpty.Count = [%d]"),
					  iParent.iBuffersToEmpty.Count());
		iParent.iPFHelper->BufferIndication();
		}		
		
	}


TBool COmxILPcmRendererProcessingFunction::CAudioDevice::ConstructAndStartUpdateTimer()
    {
    // Need this check if:
    // - The component state transitions from Execution-Idle-Execution
    // - The Clock's state transitions from Running-Stop-Running
    if (iPeriodic == NULL)
        {
        iPeriodic = CPeriodic::New(EPriorityStandard);
        
        if (iPeriodic == NULL)
            {
            iParent.iCallbacks.ErrorEventNotification(OMX_ErrorInsufficientResources);
            return EFalse;
            }
        }
    
    StartUpdateTimer();
    
    return ETrue;
    }


void COmxILPcmRendererProcessingFunction::CAudioDevice::ProcessNextBuffer()
	{
	if (iParent.iBuffersToEmpty.Count() == 0)
		return;

	// To implement A/V Sync, we should start playing only once the clock component gives the appopriate command
	// If the clock component is not available, we start playing immediately
	// Since the PCM Renderer supplies the reference clock, we make sure to initialise the clock component with
	// the reference when we receive the first buffer
	
	if (!iParent.iClientClockPortPtr->IsClockComponentAvailable())
		{
		PlayData();
		return;
		}

	OMX_BUFFERHEADERTYPE* bufferPtr = iParent.iBuffersToEmpty[0];
	
	TBool bufferHasStartTime = bufferPtr->nFlags & OMX_BUFFERFLAG_STARTTIME;
	
    if (!iClockStateRunning)
        {
        if (!bufferHasStartTime)
            {
            // Connected with the Clock but OMX_BUFFERFLAG_STARTTIME isn't set; simply queue the buffer
            return;
            }
        else
            {
            OMX_ERRORTYPE err = iParent.iClientClockPortPtr->SetStartTime(static_cast<OMX_TICKS>(bufferPtr->nTimeStamp));
            
            if (err == OMX_ErrorNone)
                {
                // Clear the returning buffer's flag
                bufferPtr->nFlags &= ~OMX_BUFFERFLAG_STARTTIME;
                }
            else
                {
                // NOTE: If the Clock is not in OMX_TIME_ClockStateWaitingForStartTime,
                //       currently SetStartTime will return OMX_ErrorIncorrectStateOperation
                
                // It is not the PCM renderer to flag a Clock component error;
                // therefore, ignore the error.
                // 
                // As the Clock is not in OMX_TIME_ClockStateRunning state, the Renderer needs  
                // to keep the OMX_BUFFERFLAG_STARTTIME in the first buffer until the Clock 
                // moves into OMX_TIME_ClockStateWaitingForStartTime or OMX_TIME_ClockStateRunning
                // state
                DEBUG_PRINTF2(_L8("CAudioDevice::ProcessNextBuffer SetStartTime() return %d"), err);
                }
            
            // Update the iStartMediaTime
            iParent.iStartMediaTime = static_cast<OMX_TICKS>(bufferPtr->nTimeStamp);
            iIsStartTimeFlagSet = ETrue;
            }
        } // (!iClockStateRunning)
    else
		{
        if (bufferHasStartTime)
            {
            // The Clock moves straight into OMX_TIME_ClockStateRunning state,
            // clear the returning buffer's flag.
            bufferPtr->nFlags &= ~OMX_BUFFERFLAG_STARTTIME;
            }
        
        if (!iPlayData)
            {
            // Not allowed to render audio. This could be due to:
            //  - The renderer is waiting for a time completion notification from the Clock;
            return;
            }
		
		if (!iIsStartTimeFlagSet)
		    {
		    // As the StartTimeFlag is not mandatory; therefore it might be missing from the first audio buffer
		    // In such a case, we use the first buffer's timestamp as the StartMediaTime.
		    //
		    // NOTE: Since the Clock is running, calling SetStartTime() to the Clock is meaningless
		    
		    // Update the iStartMediaTime
		    iParent.iStartMediaTime = static_cast<OMX_TICKS>(bufferPtr->nTimeStamp);
		    iIsStartTimeFlagSet = ETrue;
		    
            // Cross checking the Clock's media timestamp with iStartMediaTime to see 
            // data can be rendered straight away 
            if (!CanPlayNow())
                {
                return;
                }
            
            if (!ConstructAndStartUpdateTimer())
                {
                return;
                }
		    }
		
		DEBUG_PRINTF3(_L8("ProcessNextBuffer : iStartMediaTime = %d nTimeStamp = %d"), 
	            I64LOW(iParent.iStartMediaTime), I64LOW(bufferPtr->nTimeStamp));
		
		if (!iPausedClockViaScale)	//if clock scale is zero then we are effectively paused
			{
			PlayData();
			}
		} // else
	}


void COmxILPcmRendererProcessingFunction::CAudioDevice::PlayData()
	{
	DEBUG_PRINTF(_L8("COmxILPcmRendererProcessingFunction::CAudioDevice::PlayData()++"));
	if (iParent.iBuffersToEmpty.Count() == 0 || iSoundDevice.Handle() == 0 || IsActive())
		{
		DEBUG_PRINTF(_L8("COmxILPcmRendererProcessingFunction::CAudioDevice::PlayData() nothing to play, or there is an outstanding request"));
		return;
		}

	iCurrentBuffer = iParent.iBuffersToEmpty[0];

	iParent.iBuffersToEmpty.Remove(0);

	TPtr8 ptrData(iCurrentBuffer->pBuffer, iCurrentBuffer->nFilledLen, iCurrentBuffer->nFilledLen);
	
	// Attenuate the amplitude of the samples if volume ramping has been changed
	if (iRampAudioSample)
		{
		iRampAudioSample = RampAudio(ptrData);
		}

	// First, check whether the buffer length is sufficient not to cause underflows in the device driver
	TBool isFilledLengthSufficient = IsBufferLengthSufficient(iCurrentBuffer->nFilledLen);
	// This variable defines whether we should send the data to the driver directly, or append it to an aggregated buffer
	// We append the buffer instead of sending it directly, if (i) the buffer is too small, or (ii)  we have already aggregated something.
	TBool appendBuffer = (!isFilledLengthSufficient || iCachedPlayBuffer.Length() > 0) ? ETrue : EFalse;
	if (!appendBuffer)
		{
        SendBufferToSoundDevice(ptrData);
		}
	else
		{
		// Check if we need to allocate the cached buffer
		if (iCachedPlayBuffer.MaxLength() == 0)
			{
			// The RMdaDevSound shim allocates the shared chunk according to the maxLength of the descriptor it receives
			// For this reason, we must allocate our buffer conservatively, otherwise the chunk may be insufficient and the RMdaDevSound shim will panic			
			TInt err = iCachedPlayBuffer.ReAlloc(GetMinBufferLength() + iCurrentBuffer->nAllocLen);
			if  (err != KErrNone)
				{
				iParent.iCallbacks.ErrorEventNotification(OMX_ErrorInsufficientResources);
				return;
				}
			}
		
		iCachedPlayBuffer.Append(ptrData);
				
		// If we have sufficient length aggregated, play the cached buffer
		// Also if this is the last buffer, we have to play it now, there's nothing left to cache
		if (IsBufferLengthSufficient(iCachedPlayBuffer.Length()) || iCurrentBuffer->nFlags & OMX_BUFFERFLAG_EOS)
			{
			SendBufferToSoundDevice(iCachedPlayBuffer);
			}
		// If not, make sure to notify that we notify that the buffer is done for the OMX tunnnel, so that the port will be able to reuse it
		else
			{
			SignalBufferCompletion(iCurrentBuffer);
			iCurrentBuffer = NULL;
			}
		}
	}

void COmxILPcmRendererProcessingFunction::CAudioDevice::SendBufferToSoundDevice(TDes8& aBuffer) 
	{
	ASSERT(!IsActive());
	iStatus = KRequestPending;
	DEBUG_PRINTF2(_L8("COmxILPcmRendererProcessingFunction::CAudioDevice::SendBufferToSoundDevice() PlayData [%d]"), aBuffer.Length());
	iSoundDevice.PlayData(iStatus, aBuffer);
	if (iResumeAfterNextPlay)
		{
		iResumeAfterNextPlay = EFalse;
		iSoundDevice.ResumePlaying();
		}
	SetActive();				
	}
	
TInt COmxILPcmRendererProcessingFunction::CAudioDevice::GetMinBufferLength() const
	{
	TInt minBufSize = KMinBufferMilliseconds * iSampleRate * iChannels / 1000;
	if (iEncoding == RMdaDevSound::EMdaSoundEncoding16BitPCM)
		{
		minBufSize *= 2; // All other encodings use 1 byte per sample
		}
	return minBufSize;
	}
	
TBool COmxILPcmRendererProcessingFunction::CAudioDevice::IsBufferLengthSufficient(TInt aBufferLength) const
	{
	return aBufferLength >= GetMinBufferLength() ? ETrue : EFalse;
	}


void COmxILPcmRendererProcessingFunction::CAudioDevice::ProcessMediaTimeIndication(OMX_TIME_MEDIATIMETYPE& aMediaTimeType)
	{
	switch (aMediaTimeType.eUpdateType)
		{
		case OMX_TIME_UpdateClockStateChanged:
			{
			HandleClockStateChanged(aMediaTimeType);
			}
			break;
			
		case OMX_TIME_UpdateRequestFulfillment:
			{
			// As the Clock was using another earlier start time;
			// it is time for the PCM renderer to start playing the audio.
			iPlayData = ETrue;
			PlayData();
			
			if (!ConstructAndStartUpdateTimer())
			    {
			    return;
			    }
			}
			break;
			
		case OMX_TIME_UpdateScaleChanged:
			{
			HandleClockScaleChanged(aMediaTimeType);
			}
			break;
			
		default:
			{
			// Do nothing here
			}
		}
	}


void COmxILPcmRendererProcessingFunction::CAudioDevice::StartUpdateTimer()
	{
	if (!iIsStartTimeFlagSet)
		{
		DEBUG_PRINTF(_L8("COmxILPcmRendererProcessingFunction::CAudioDevice::StartUpdateTimer() iIsStartTimeFlagSet == EFalse!!"));
		return;
		}
	
	// In case the timer already started
	ASSERT(iPeriodic);
	iPeriodic->Cancel();
	
	TCallBack callback(UpdateClockMediaTime, this);
	
	// Start updating the Clock comp. every KPcmRendererTimePlayedDelay
	iPeriodic->Start(KPcmRendererTimePlayedDelay, KPcmRendererTimePlayedDelay, callback);
	}


TBool COmxILPcmRendererProcessingFunction::CAudioDevice::CanPlayNow()
    {
    if (iParent.iState != OMX_StateExecuting)
        {
        return EFalse;
        }
    
    if (iClockMediaTime < iParent.iStartMediaTime)
        {
        // Once all required Clock's clients have responded, the clock component starts 
        // the media clock using the earliest client start time.
        // Therefore, start playing the audio only when such time comes; send a time
        // completion request to the Clock component
        OMX_ERRORTYPE err = iParent.iClientClockPortPtr->MediaTimeRequest(NULL, iParent.iStartMediaTime, 0);
        
        if (err != OMX_ErrorNone)
            {
            DEBUG_PRINTF2(_L8("CAudioDevice::CanPlayNow() MediaTimeRequest() return %d"), err);
            }
        
        // Indicate that processing a new buffer should not trigger a false start on playback.
        iPlayData = EFalse;
        return EFalse;
        }
    else if (iClockMediaTime > iParent.iStartMediaTime)
        {
        // NOTE: The spec. states that the clock should use the minimum of the received start times
        //       Therefore the Clock should NOT jump forwards with timestamp greater than the PCM
        //       Renderer iStartMediaTime 
        DEBUG_PRINTF3(_L8("CanPlayNow() nMediaTimestamp(%d) > iStartMediaTime(%d) IGNORE this use case"), 
                I64LOW(iClockMediaTime), I64LOW(iParent.iStartMediaTime));
        
        // However if the Clock sends out a timestamp greater than the buffer's timestamp
        // drop the buffers that fall outside the Clock specified MediaTimestamp;
        
        OMX_BUFFERHEADERTYPE* bufferPtr = iParent.iBuffersToEmpty[0];
        iParent.iBuffersToEmpty.Remove(0);
        SignalBufferCompletion(bufferPtr);
        bufferPtr = NULL;
        
        // Since the iStartMediaTime doesn't make sense anymore, reset iIsStartTimeFlagSet until the buffer's timestamp 
        // is within the the Clock's Media timestamp
        iIsStartTimeFlagSet = EFalse;
        return EFalse;
        }
    
    return ETrue;
    }


void COmxILPcmRendererProcessingFunction::CAudioDevice::HandleClockStateChanged(const OMX_TIME_MEDIATIMETYPE& aMediaTimeType)
	{	
	switch (aMediaTimeType.eState)
	    {
	    case OMX_TIME_ClockStateRunning:
	        {
	        iClockMediaTime = aMediaTimeType.nMediaTimestamp;
	        
	        // There are two possibilities:
	        // case 1 - The clock goes straight into running and the PCM renderer processes the  
	        //          StateChanged notification prior the audio buffer
	         
	        if (iIsStartTimeFlagSet)
	            {
	            // OR
	            // case 2 - The PCM recieves the audio buffer, and the clock StateChanged notification
	            // comes later
	            
	            // Clear the returning buffer's flag.
	            iParent.iBuffersToEmpty[0]->nFlags &= ~OMX_BUFFERFLAG_STARTTIME;
	            
	            // Cross checking the Clock's media timestamp with iStartMediaTime to see 
	            // data can be rendered straight away
	            if (!CanPlayNow())
	                {
	                break;
	                }
	            
	            // Start playing the audio and start updating the Clock media time regularly 
	            PlayData();

	            if (!ConstructAndStartUpdateTimer())
	                {
	                return;
	                }
	            }
			else if ((!iClockStateRunning) && (iParent.iState == OMX_StateExecuting))
				{
				// The clock has gone to running but no start time flag was received. This would
				// indicate that the client moved it straight from stopped to running. As we may
				// have received buffers while in stopped state, we need to start processing
				// them now.
                DEBUG_PRINTF(_L8("HandleClockStateChanged() Gone to running without start time flag set"));
				iClockStateRunning = ETrue;
				ProcessNextBuffer();
				}
	        
	        // Otherwise, the queue is empty;
	        //
	        // NOTE: When !iIsStartTimeFlagSet && !iClockStateRunning would drop the incoming buffers;
	        //       if the first buffer does not have the OMX_BUFFERFLAG_STARTTIME set
	        }
	        break;
	    
        case OMX_TIME_ClockStateWaitingForStartTime:
            {
            if (iClockStateRunning)
                {
                DEBUG_PRINTF(_L8("HandleClockStateChanged() OMX_TIME_ClockStateRunning -> OMX_TIME_ClockStateWaitingForStartTime IGNORED!!"));
                }
            else
                {
                // Let's try to process buffers (if any).
                ProcessNextBuffer();
                }
            }
            break;

        case OMX_TIME_ClockStateStopped:
            {
            if (iClockStateRunning)
                {
                // The Clock was in "Running" state but not anymore; stop the audio
                if (IsActive())
                    {
                    Cancel();
                    }
                else
                    {
                    DoCancel();
                    }

                iPlayData = ETrue;
                if (iIsStartTimeFlagSet)
                    {
                    iIsStartTimeFlagSet = EFalse;
                    iPeriodic->Cancel();
                    }
                }
            
            // Otherwise, ignore other possibilities
            }
            
            break;
         
        default:
            {
            DEBUG_PRINTF2(_L8("HandleClockStateChanged() aMediaTimeType.eState = %d IGNORED!!"), aMediaTimeType.eState);
            }
         break;
	    }
   
	iClockStateRunning = (aMediaTimeType.eState == OMX_TIME_ClockStateRunning) ? ETrue : EFalse;
	}	


void COmxILPcmRendererProcessingFunction::CAudioDevice::HandleClockScaleChanged(const OMX_TIME_MEDIATIMETYPE& aMediaTimeType)
	{
	if (aMediaTimeType.xScale == 0)
		{
		PauseAudio();
		iPausedClockViaScale = ETrue;
        DEBUG_PRINTF2(_L8("HandleClockScaleChanged() pausing iPausedClockViaScale = %d"), iPausedClockViaScale);
 		}
	else if (aMediaTimeType.xScale == 0x10000)
		{
		// The scale is a Q16 value
		iPausedClockViaScale = EFalse;
		DEBUG_PRINTF2(_L8("HandleClockScaleChanged() resuming iPausedClockViaScale = %d"), iPausedClockViaScale);

		// If we are active then there is an outstanding PlayData() request so we need to call ResumePlaying().
		// However calling ResumePlaying() without an outstanding PlayData() request can cause the TimePLayed() API
		// to go awry, so we should defer calling ResumePlaying() until the next PlayData() call.
		if (IsActive())
			{
			iSoundDevice.ResumePlaying();
			}
		else
			{
			iResumeAfterNextPlay = ETrue;
			}
		iParent.iPFHelper->BufferIndication();	//handles the race condition where both 1) iSoundDevice was paused after the last PlayData() completed and before it was passed any data & 2) BufferIndication()s came in for all the IL buffers while in this paused state
		StartUpdateTimer();
		}

	// TODO: Handle the rest of the scale values

	}


void COmxILPcmRendererProcessingFunction::CAudioDevice::DoCancel()
	{
	if (iSoundDevice.Handle() != 0)
		{
		iSoundDevice.CancelPlayData();
		iSoundDevice.FlushPlayBuffer();
		}
	
	if (iCurrentBuffer)
	    {
		iCurrentBuffer->nFilledLen = 0;
		
		iParent.iCallbacks.BufferDoneNotification(iCurrentBuffer,
		        iCurrentBuffer->nInputPortIndex,
		        OMX_DirInput);
		
		iCachedPlayBuffer.Zero();
		iCurrentBuffer = NULL;
		}
	}


OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::OpenDevice()
	{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	if(!iSoundDevice.Handle())
		{
		if(KErrNone != iSoundDevice.Open(KSoundScTxUnit0))
			{
			err = OMX_ErrorHardware;
			}
		}
	return err;
	}

OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::CloseDevice()
	{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	if(iSoundDevice.Handle())
		{
		iSoundDevice.Close();
		iResumeAfterNextPlay = EFalse;
		}
	return err;
	}

OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::Execute()
	{
	if(!iSoundDevice.Handle())
		{
		if(KErrNone != iSoundDevice.Open())
			{
			return OMX_ErrorHardware;
			}
		}

	if(iParent.iState == OMX_StatePause)
		{
		// Now we can send BufferDone notifications for each emptied
		// buffer...
		iParent.FlushBufferList(iParent.iBuffersEmptied);
		// If we are active then there is an outstanding PlayData() request so we need to call ResumePlaying().
		// However calling ResumePlaying() without an outstanding PlayData() request can cause the TimePLayed() API
		// to go awry, so we should defer calling ResumePlaying() until the next PlayData() call.
		if (IsActive())
			{
			iSoundDevice.ResumePlaying();
			}
		else
			{
			iResumeAfterNextPlay = ETrue;
			}
		StartUpdateTimer();
		}
	else
		{
		// Set play format
		RMdaDevSound::TCurrentSoundFormatBuf buf;
		iSoundDevice.GetPlayFormat(buf);
		buf().iRate = iSampleRate;
		buf().iChannels = iChannels;
		buf().iBufferSize = iBufferSize;
		buf().iEncoding = iEncoding;
		if(KErrNone != iSoundDevice.SetPlayFormat(buf))
			{
			return OMX_ErrorHardware;
			}
		
		if (iMuted)
		    {
		    iSoundDevice.SetVolume(0);
		    }
		else
		    {
		    iSoundDevice.SetVolume(iVolume);
		    }
		}

	iParent.iState = OMX_StateExecuting;		
		
	// Make sure to start processing of queued up buffers (if any)
	if (!IsActive() && iParent.iBuffersToEmpty.Count() > 0)
		{
		ProcessNextBuffer();
		}

	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::MoveToPausedState()
	{
	iParent.iState = OMX_StatePause;
	PauseAudio();

	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::Stop()
	{
	if(iParent.iState == OMX_StateExecuting || iParent.iState == OMX_StatePause)
		{
		// Cancel and flush the device driver
		Cancel();

		// Cancel timer to stop calling RSoundSc::TimePlayed()
		if (iIsStartTimeFlagSet)
			{
			ASSERT(iPeriodic);
			iPeriodic->Cancel();
			}
		
		iParent.iState = OMX_StateIdle;

		// If the audio device is still open, store the last value of bytes
		// played before closing the audio device...
		if(iSoundDevice.Handle() != 0)
			{
			iLastBytesPlayedValue = iSoundDevice.BytesPlayed();
			// Close the sound device
			iSoundDevice.Close();
			iResumeAfterNextPlay = EFalse;
			}
		}
	
	iClockStateRunning = EFalse;
	iIsStartTimeFlagSet = EFalse;
	iPlayData = ETrue;

	return OMX_ErrorNone;
	}


void COmxILPcmRendererProcessingFunction::CAudioDevice::PauseAudio()
	{
	// Cancel timer to stop calling RSoundSc::TimePlayed()
	if (iIsStartTimeFlagSet)
		{
		ASSERT(iPeriodic);
		iPeriodic->Cancel();
		}
	
	if (iSoundDevice.Handle())
		{
		iSoundDevice.PausePlayBuffer();
		}
	}


OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::SetChannels(TUint aChannels)
	{
	iChannels = aChannels;
	return SetPlayFormat();
	}

OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::SetSampleRate(TUint aSampleRate)
	{
	iSampleRate = aSampleRate;
	return SetPlayFormat();
	}

TInt ConvertOmxToSymbianVolume(TInt aVolume)
	{
	// OpenMax volume is in millibels while Symbian volume is in 0.5 db increments in the [0..255] range
	// We divide by 50 as 0.5 dB = 50 millibells
	TInt res = KMedianVolume + aVolume / 50;
	if (res < 0)
		return 0;
	if (res > KMaxVolume)
		return KMaxVolume;
	return res;
	}
	
OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::SetVolume(TInt aVolume)
	{
	iVolume = ConvertOmxToSymbianVolume(aVolume);
	if ((!iMuted) && (iSoundDevice.Handle() != 0))
		{
		iSoundDevice.SetVolume(iVolume);
		}
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::SetVolumeRamp(const TTimeIntervalMicroSeconds& aRampDuration)
	{
	iVolumeRamp = aRampDuration;
	if(iVolumeRamp.Int64() != 0)
		{
		iRampAudioSample = ETrue;
		ConfigAudioRamper(
			iVolumeRamp.Int64());
		}
	else
		{
		iRampAudioSample = EFalse;
		}
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::SetMuted(TBool aMuted)
	{
	iMuted = aMuted;
	
	if (iSoundDevice.Handle() == 0)
	    {
		// Just cache the value; the value will be set once the the device is opened
	    return OMX_ErrorNone;
	    }
	
	if (iMuted)
		{
		iSoundDevice.SetVolume(0);
		}
	else
		{
		iSoundDevice.SetVolume(iVolume);
		}
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILPcmRendererProcessingFunction::CAudioDevice::SetPlayFormat()
	{
	if (iParent.iState == OMX_StateExecuting)
		{
		RMdaDevSound::TCurrentSoundFormatBuf buf;
		iSoundDevice.GetPlayFormat(buf);
		buf().iRate = iSampleRate;
		buf().iChannels = iChannels;
		buf().iBufferSize = iBufferSize;
		buf().iEncoding = iEncoding;
		if(KErrNone != iSoundDevice.SetPlayFormat(buf))
			{
			return OMX_ErrorHardware;
			}
		}
	return OMX_ErrorNone;
	}

OMX_BUFFERHEADERTYPE* COmxILPcmRendererProcessingFunction::CAudioDevice::GetCurrentBuffer()
	{
	return iCurrentBuffer;
	}

TInt COmxILPcmRendererProcessingFunction::CAudioDevice::GetBytesPlayed()
	{
	if(iSoundDevice.Handle() != 0)
		{
		return iSoundDevice.BytesPlayed();
		}

	return iLastBytesPlayedValue;
	}

void COmxILPcmRendererProcessingFunction::CAudioDevice::ConfigAudioRamper(TInt64 aRampTime)
	{
	iRampSamples = I64LOW(((TInt64(iSampleRate) * aRampTime) /1000000 )); // Add this
	iRampSamplesLeft = iRampSamples;
	iRampIncr = 0;
	iSkip = ETrue;
	}

TBool COmxILPcmRendererProcessingFunction::CAudioDevice::RampAudio(TDes8& aBuffer)
	{
	TInt i=0;
	TInt length = aBuffer.Length()>>1;
	if (length == 0)
		{
		return EFalse;
		}
		
	TInt16* sample = REINTERPRET_CAST(TInt16*,&aBuffer[0]);
	TInt64 theResult(0);
	while ((i < length) && (iRampIncr < iRampSamples))
		{
		theResult = sample[i];
		theResult *= iRampIncr;
		theResult /= iRampSamples;
		sample[i] = STATIC_CAST(TInt16, I64LOW(theResult) );

		if ((iChannels == 1) || (!iSkip))
			{
			iRampIncr++;
			}
		iSkip = !iSkip;
		i++;
		}

	if (iRampIncr < iRampSamples)
		return ETrue;
	else
		return EFalse;
	}


TInt COmxILPcmRendererProcessingFunction::CAudioDevice::UpdateClockMediaTime(TAny* aPtr)
	{
	CAudioDevice* ptr = (CAudioDevice*)aPtr;
	TTimeIntervalMicroSeconds playedTime(0);
  	
	if (ptr->iSoundDevice.GetTimePlayed(playedTime) != KErrNone)
		{
		ptr->iParent.iCallbacks.ErrorEventNotification(OMX_ErrorHardware);
		return EFalse;
		}
	
  	OMX_ERRORTYPE err;
  	
	// Update the clock component audio reference clock
  	err = ptr->iParent.iClientClockPortPtr->SetAudioReference(ptr->iParent.iStartMediaTime + playedTime.Int64());

	if (err != OMX_ErrorNone)
		{
		ptr->iParent.iCallbacks.ErrorEventNotification(err);
		return EFalse;
		}
	
	DEBUG_PRINTF2(_L8("CAudioDevice::UpdateClockMediaTime : playedTime = %d"), 
					I64LOW(playedTime.Int64()));

	return ETrue;
	}


void COmxILPcmRendererProcessingFunction::CAudioDevice::ProcessParamIndication(const OMX_AUDIO_PARAM_PCMMODETYPE& aPcmModeType)
	{
	if(SetChannels(aPcmModeType.nChannels) != OMX_ErrorNone)
		{
		iParent.iCallbacks.ErrorEventNotification(OMX_ErrorHardware);
		return;
		}
	
	if (SetSampleRate(aPcmModeType.nSamplingRate) != OMX_ErrorNone)
		{
		iParent.iCallbacks.ErrorEventNotification(OMX_ErrorHardware);
		return;
		}
	}

COmxILPcmRendererProcessingFunction::CPFHelper* COmxILPcmRendererProcessingFunction::CPFHelper::NewL(CAudioDevice& aAudioDevice)
	{
	CPFHelper* self = new (ELeave) CPFHelper(aAudioDevice);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

COmxILPcmRendererProcessingFunction::CPFHelper::CPFHelper(CAudioDevice& aAudioDevice)
: CActive(EPriorityUserInput),
  iAudioDevice(aAudioDevice)
	{
	}
	
void COmxILPcmRendererProcessingFunction::CPFHelper::ConstructL()
	{
	CActiveScheduler::Add(this);
	User::LeaveIfError(iCallerSemaphore.CreateGlobal(KNullDesC, 0));
	User::LeaveIfError(iMsgQueue.CreateLocal(KMaxMsgQueueEntries));
	iMsgQueue.NotifyDataAvailable(iStatus);
	RThread thisThread;
	iHelperThreadId = thisThread.Id();
	thisThread.Close();
	SetActive();
	}
	
COmxILPcmRendererProcessingFunction::CPFHelper::~CPFHelper()
	{
	Cancel(); 
	iMsgQueue.Close();
	iCallerSemaphore.Close();
	}

void COmxILPcmRendererProcessingFunction::CPFHelper::RunL()
	{

	TProcMessage msg;
	while (iMsgQueue.Receive(msg)==KErrNone)
		{
		switch (msg.iType)
			{
			case EOpenDevice:
				{
				iAudioDevice.OpenDevice();
				break;
				}
			case ECloseDevice:
				{
				iAudioDevice.CloseDevice();
				break;
				}
			case ECloseDeviceOnError:
				{
				iAudioDevice.Cancel();
				iAudioDevice.CloseDevice();
				iCallerSemaphore.Signal();
				break;
				}
			case EExecuteCommand:
				{
				iAudioDevice.Execute();	
				break;
				}				

			case EStopCommand:
				{
				iAudioDevice.Stop();	
				break;
				}
				
			case ECancelCommand:
				{
				iAudioDevice.Cancel();	
				break;
				}
				
			case EBufferIndication:
				{
				iAudioDevice.ProcessNextBuffer();	
				break;
				}
			case EMediaTimeIndication:
				{
				iAudioDevice.ProcessMediaTimeIndication(msg.iMediaTimeType);	
				break;
				}
			case EParamIndication:
				{
				iAudioDevice.ProcessParamIndication(msg.iPcmModeType);
				break;
				}
			case ESetVolumeRamp:
				{
				iAudioDevice.SetVolumeRamp(TTimeIntervalMicroSeconds(msg.iRampDuration));
				break;
				}
			case ESetVolume:
				{
				iAudioDevice.SetVolume(msg.iVolumeValue);
				break;
				}
			case ESetMuted:
				{
				iAudioDevice.SetMuted(msg.iMuted);
				break;
				}
			default:
				{
				break;
				}					
			}
		}
	DEBUG_PRINTF2(_L8("COmxILPcmRendererProcessingFunction::CPFHelper::RunL : msg.iType[%d]"), msg.iType);		
	// setup for next callbacks		
	iMsgQueue.NotifyDataAvailable(iStatus);
	SetActive();
	}

void COmxILPcmRendererProcessingFunction::CPFHelper::DoCancel()
	{
	if (iMsgQueue.Handle()!=NULL)
		{
		iMsgQueue.CancelDataAvailable();
		}
	}

TInt COmxILPcmRendererProcessingFunction::CPFHelper::OpenDevice()
	{
	TProcMessage message;
	message.iType = EOpenDevice;
	return iMsgQueue.Send(message);
	}

TInt COmxILPcmRendererProcessingFunction::CPFHelper::CloseDevice()
	{
	DEBUG_PRINTF2(_L8("COmxILPcmRendererProcessingFunction::CPFHelper::CloseDevice : IsActive[%s]"),
				  (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = ECloseDevice;
	return iMsgQueue.Send(message);
	}

TInt COmxILPcmRendererProcessingFunction::CPFHelper::CloseDeviceOnError()
	{
	DEBUG_PRINTF2(_L8("COmxILPcmRendererProcessingFunction::CPFHelper::CloseDeviceOnError : IsActive[%d]"), (IsActive() ? 1 : 0));

	RThread thisThread;
	if (thisThread.Id() == iHelperThreadId)
		{
		// Just do it...
		iAudioDevice.Cancel();
		iAudioDevice.CloseDevice();
		}
	else
		{

		TProcMessage message;
		message.iType = ECloseDeviceOnError;
		TInt error = iMsgQueue.Send(message);
		if (KErrNone == error)
			{
			// only wait if the message was sent into the queue...
			iCallerSemaphore.Wait();
			}
		}

	thisThread.Close();

	return KErrNone;

	}

TInt COmxILPcmRendererProcessingFunction::CPFHelper::Execute()
	{
	DEBUG_PRINTF2(_L8("CPFHelper::Execute : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = EExecuteCommand;
	return iMsgQueue.Send(message);
	}
		
TInt COmxILPcmRendererProcessingFunction::CPFHelper::Stop()
	{
	DEBUG_PRINTF2(_L8("CPFHelper::Stop : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = EStopCommand;
	return iMsgQueue.Send(message);
	}
	
TInt COmxILPcmRendererProcessingFunction::CPFHelper::CancelDevice()
	{
	DEBUG_PRINTF2(_L8("CPFHelper::CancelDevice : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = ECancelCommand;
	return iMsgQueue.Send(message);
	}
	
TInt COmxILPcmRendererProcessingFunction::CPFHelper::BufferIndication()
	{
	DEBUG_PRINTF2(_L8("CPFHelper::BufferIndication : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = EBufferIndication;
	return iMsgQueue.Send(message);
	}

TInt COmxILPcmRendererProcessingFunction::CPFHelper::MediaTimeIndication(const OMX_TIME_MEDIATIMETYPE& aMediaTimeType)
	{
	DEBUG_PRINTF2(_L8("CPFHelper::MediaTimeIndication : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = EMediaTimeIndication;
	message.iMediaTimeType.eUpdateType = aMediaTimeType.eUpdateType;
	message.iMediaTimeType.eState = aMediaTimeType.eState;
	message.iMediaTimeType.xScale = aMediaTimeType.xScale;	
	message.iMediaTimeType.nMediaTimestamp = aMediaTimeType.nMediaTimestamp;
	
	return iMsgQueue.Send(message);
	}

TInt COmxILPcmRendererProcessingFunction::CPFHelper::ParamIndication(const OMX_AUDIO_PARAM_PCMMODETYPE* aPcmModeType)
	{
	DEBUG_PRINTF2(_L8("CPFHelper::ParamIndication : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = EParamIndication;
	message.iPcmModeType.nChannels =  aPcmModeType->nChannels;
	message.iPcmModeType.nSamplingRate =  aPcmModeType->nSamplingRate;
	
	return iMsgQueue.Send(message);
	}

TInt COmxILPcmRendererProcessingFunction::CPFHelper::SetVolumeRamp(const OMX_U64 aRampDuration)
	{
	DEBUG_PRINTF2(_L8("CPFHelper::SetVolumeRamp : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = ESetVolumeRamp;
	message.iRampDuration = aRampDuration;
	
	return iMsgQueue.Send(message);
	}

TInt COmxILPcmRendererProcessingFunction::CPFHelper::SetVolume(const OMX_S32 aVolumeValue)
	{
	DEBUG_PRINTF2(_L8("CPFHelper::SetVolume : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = ESetVolume;
	message.iVolumeValue = aVolumeValue;
	
	return iMsgQueue.Send(message);	
	}

TInt COmxILPcmRendererProcessingFunction::CPFHelper::SetMuted(const OMX_BOOL aMuted)
	{
	DEBUG_PRINTF2(_L8("CPFHelper::SetMuted : IsActive[%s]"), (IsActive() ? "YES" : "NO"));
	TProcMessage message;
	message.iType = ESetMuted;
	message.iMuted = aMuted;
	
	return iMsgQueue.Send(message);
	}
