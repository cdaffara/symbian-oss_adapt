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

#include <openmax/il/khronos/v1_x/OMX_Component.h>
#include <openmax/il/common/omxilutil.h>
#include <openmax/il/shai/OMX_Symbian_ExtensionNames.h>
#include "log.h"
#include "omxilpcmrendererapb0port.h"
#include "omxilpcmrendererconst.h"
#include "omxilpcmrendererprocessingfunction.h"


COmxILPcmRendererAPB0Port* COmxILPcmRendererAPB0Port::NewL(
	const TOmxILCommonPortData& aCommonPortData,
	const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats,
	const OMX_AUDIO_PARAM_PCMMODETYPE& aParamAudioPcm,
	const OMX_AUDIO_CONFIG_VOLUMETYPE& aConfigAudioVolume,
	const OMX_AUDIO_CONFIG_MUTETYPE& aConfigAudioMute,
	COmxILPcmRendererProcessingFunction& aProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::NewL"));

	COmxILPcmRendererAPB0Port* self = new (ELeave)COmxILPcmRendererAPB0Port(
		aParamAudioPcm,
		aConfigAudioVolume,
		aConfigAudioMute,
		aProcessingFunction);
	
	CleanupStack::PushL(self);
	self->ConstructL(aCommonPortData, aSupportedAudioFormats);
	CleanupStack::Pop(self);
	return self;

	}


void
COmxILPcmRendererAPB0Port::ConstructL(const TOmxILCommonPortData& aCommonPortData, const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::ConstructL"));
	COmxILAudioPort::ConstructL(aCommonPortData, aSupportedAudioFormats);
	OMX_PARAM_PORTDEFINITIONTYPE& paramPortDefinition=GetParamPortDefinition();
	// We have to finish with iParamPortDefinition
	paramPortDefinition.eDomain = OMX_PortDomainAudio;
	paramPortDefinition.format.audio.pNativeRender = 0;

	iMimeTypeBuf.CreateL(KMimeTypeAudioPcm(), KMimeTypeAudioPcm().Length() + 1);

	TUint8* pTUint2 = const_cast<TUint8*>(iMimeTypeBuf.PtrZ());
	paramPortDefinition.format.audio.cMIMEType = reinterpret_cast<OMX_STRING>(pTUint2);

	paramPortDefinition.format.audio.bFlagErrorConcealment = OMX_FALSE;
	paramPortDefinition.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	
	// Init iConfigVolumeRamp here...
	iConfigVolumeRamp.nSize			  = sizeof(OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE);
	iConfigVolumeRamp.nVersion		  = TOmxILSpecVersion();
	iConfigVolumeRamp.nPortIndex		  = paramPortDefinition.nPortIndex;
	iConfigVolumeRamp.nChannel                = 0;
	iConfigVolumeRamp.bLinear                 = OMX_FALSE;
	iConfigVolumeRamp.sStartVolume.nValue     = 0;
	iConfigVolumeRamp.sStartVolume.nMin       = 0;
	iConfigVolumeRamp.sStartVolume.nMax       = 0;
	iConfigVolumeRamp.sEndVolume.nValue       = 0;
	iConfigVolumeRamp.sEndVolume.nMin        = 0;
	iConfigVolumeRamp.sEndVolume.nMax         = 0;
	iConfigVolumeRamp.nRampDuration           = 0;
	iConfigVolumeRamp.bRampTerminate          = OMX_FALSE;
	iConfigVolumeRamp.sCurrentVolume.nValue   = 0;
	iConfigVolumeRamp.sCurrentVolume.nMin    = 0;
	iConfigVolumeRamp.sCurrentVolume.nMax     = 0;
	iConfigVolumeRamp.nRampCurrentTime        = 0;
	iConfigVolumeRamp.nRampMinDuration        = 0;
	iConfigVolumeRamp.nRampMaxDuration        = 0;
	iConfigVolumeRamp.nVolumeStep             = 0;
	}


COmxILPcmRendererAPB0Port::COmxILPcmRendererAPB0Port(
	const OMX_AUDIO_PARAM_PCMMODETYPE& aParamAudioPcm,
	const OMX_AUDIO_CONFIG_VOLUMETYPE& aConfigAudioVolume,
	const OMX_AUDIO_CONFIG_MUTETYPE& aConfigAudioMute,
	COmxILPcmRendererProcessingFunction& aProcessingFunction)
	:
	iParamAudioPcm(aParamAudioPcm),
	iConfigAudioVolume(aConfigAudioVolume),
	iConfigAudioMute(aConfigAudioMute),
	iProcessingFunction(aProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::COmxILPcmRendererAPB0Port"));
	}


COmxILPcmRendererAPB0Port::~COmxILPcmRendererAPB0Port()
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::~COmxILPcmRendererAPB0Port"));

	CleanUpPort();
	iMimeTypeBuf.Close();

	}


OMX_ERRORTYPE
COmxILPcmRendererAPB0Port::SetFormatInPortDefinition(
	const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition,
	TBool& aUpdateProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::SetFormatInPortDefinition"));
	// There's no point on chaging the cMIMEType type of this port
	// Same thing for pNativeRender and eEncoding
	GetParamPortDefinition().format.audio.bFlagErrorConcealment =
		aPortDefinition.format.audio.bFlagErrorConcealment;
	// Error concealment not currently needed at the processing function
	aUpdateProcessingFunction = EFalse;

	return OMX_ErrorNone;

	}


TBool
COmxILPcmRendererAPB0Port::IsTunnelledPortCompatible(
	const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const
	{
	DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::IsTunnelledPortCompatible"));

	// First, the easy checks...
	if(aPortDefinition.eDomain != GetParamPortDefinition().eDomain)
		{
		return EFalse;
		}

	if(aPortDefinition.format.audio.eEncoding != OMX_AUDIO_CodingPCM)
		{
		return EFalse;
		}
	
    TBool retValue = ETrue;
    
#ifdef _OMXIL_PCMRENDERER_CHECK_MIME_TYPE_ON
	RBuf8 tunnelMimeTypeBuf;
	tunnelMimeTypeBuf.Create(KMimeTypeAudioPcm().Length() + 1);
	tunnelMimeTypeBuf =
		const_cast<const TUint8*>(
			reinterpret_cast<TUint8*>(aPortDefinition.format.audio.cMIMEType));


	if (iMimeTypeBuf != tunnelMimeTypeBuf)
		{
		retValue = EFalse;
		}

	tunnelMimeTypeBuf.Close();
#endif

	return retValue;
	}


OMX_ERRORTYPE
COmxILPcmRendererAPB0Port::GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::GetLocalOmxParamIndexes"));

	// Always collect local indexes from parent
	OMX_ERRORTYPE omxRetValue =
		COmxILAudioPort::GetLocalOmxParamIndexes(aIndexArray);

	if (OMX_ErrorNone != omxRetValue)
		{
		return omxRetValue;
		}

	TInt err = aIndexArray.InsertInOrder(OMX_IndexParamAudioPcm);

	// Note that index duplication is OK.
	if (KErrNone != err && KErrAlreadyExists != err)
		{
		return OMX_ErrorInsufficientResources;
		}

	return OMX_ErrorNone;

	}

OMX_ERRORTYPE
COmxILPcmRendererAPB0Port::GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::GetLocalOmxConfigIndexes"));

	// Always collect local indexes from parent
	OMX_ERRORTYPE omxRetValue =
		COmxILAudioPort::GetLocalOmxConfigIndexes(aIndexArray);

	if (OMX_ErrorNone != omxRetValue)
		{
		return omxRetValue;
		}

	TInt err = aIndexArray.InsertInOrder(OMX_IndexConfigAudioVolume);

	// Note that index duplication is OK.
	if (KErrNone != err && KErrAlreadyExists != err)
		{
	    return OMX_ErrorInsufficientResources;
		}
	
	err = aIndexArray.InsertInOrder(OMX_IndexConfigAudioMute);

	if (KErrNone != err && KErrAlreadyExists != err)
	    {
	    return OMX_ErrorInsufficientResources;
	    }
	
	err = aIndexArray.InsertInOrder(
	        OMX_SYMBIANINDEXCONFIGAUDIOPCMVOLUMERAMP);
                
	if (KErrNone != err && KErrAlreadyExists != err)
		{
		return OMX_ErrorInsufficientResources;
		}

    err = aIndexArray.InsertInOrder(
            OMX_SYMBIANINDEXCONFIGAUDIODATAAMOUNT);
                
    if (KErrNone != err && KErrAlreadyExists != err)
        {
        return OMX_ErrorInsufficientResources;
        }
	
	return OMX_ErrorNone;
	}


OMX_ERRORTYPE
COmxILPcmRendererAPB0Port::GetParameter(OMX_INDEXTYPE aParamIndex,
										TAny* apComponentParameterStructure) const
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::GetParameter"));

	OMX_ERRORTYPE omxRetValue = OMX_ErrorNone;
	switch(aParamIndex)
		{
	case OMX_IndexParamAudioPcm:
		{
		if (OMX_ErrorNone !=
			(omxRetValue =
			 TOmxILUtil::CheckOmxStructSizeAndVersion(
				 apComponentParameterStructure,
				 sizeof(OMX_AUDIO_PARAM_PCMMODETYPE))))
			{
			return omxRetValue;
			}

		OMX_AUDIO_PARAM_PCMMODETYPE* pPcmMode
			= static_cast<OMX_AUDIO_PARAM_PCMMODETYPE*>(
				apComponentParameterStructure);

		*pPcmMode = iParamAudioPcm;
		}
		break;

	default:
		{
		// Try the parent's indexes
		return COmxILAudioPort::GetParameter(aParamIndex,
											 apComponentParameterStructure);
		}
		};

	return OMX_ErrorNone;

	}


OMX_ERRORTYPE
COmxILPcmRendererAPB0Port::SetParameter(OMX_INDEXTYPE aParamIndex,
										const TAny* apComponentParameterStructure,
										TBool& aUpdateProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::SetParameter"));

	aUpdateProcessingFunction = EFalse;

	OMX_ERRORTYPE omxRetValue = OMX_ErrorNone;
	switch(aParamIndex)
		{
	case OMX_IndexParamAudioPcm:
		{
		if (OMX_ErrorNone !=
			(omxRetValue =
			 TOmxILUtil::CheckOmxStructSizeAndVersion(
				 const_cast<OMX_PTR>(apComponentParameterStructure),
				 sizeof(OMX_AUDIO_PARAM_PCMMODETYPE))))
			{
			return omxRetValue;
			}

		const OMX_AUDIO_PARAM_PCMMODETYPE* pPcmMode
			= static_cast<const OMX_AUDIO_PARAM_PCMMODETYPE*>(
				apComponentParameterStructure);

		switch (pPcmMode->nSamplingRate)
			{
			case 8000 :
			case 11025 :
			case 12000 :
			case 16000 :
			case 22050 :
			case 24000 :
			case 32000 :
			case 44100 :
			case 48000 :
				break;
			default:
				return OMX_ErrorBadParameter;
			}

		switch (pPcmMode->nBitPerSample)
			{
			case 8 :
			case 16 :
				break;
			default:
				return OMX_ErrorBadParameter;
			}

		// Set the new default values
		if (iParamAudioPcm.nChannels	 != pPcmMode->nChannels			||
			iParamAudioPcm.eNumData		 != pPcmMode->eNumData			||
			iParamAudioPcm.eEndian		 != pPcmMode->eEndian			||
			iParamAudioPcm.bInterleaved	 != pPcmMode->bInterleaved		||
			iParamAudioPcm.nBitPerSample != pPcmMode->nBitPerSample		||
			iParamAudioPcm.nSamplingRate != pPcmMode->nSamplingRate		||
			iParamAudioPcm.ePCMMode		 != pPcmMode->ePCMMode)
			{
			iParamAudioPcm.nChannels	 = pPcmMode->nChannels;
			iParamAudioPcm.eNumData		 = pPcmMode->eNumData;
			iParamAudioPcm.eEndian		 = pPcmMode->eEndian;
			iParamAudioPcm.bInterleaved	 = pPcmMode->bInterleaved;
			iParamAudioPcm.nBitPerSample = pPcmMode->nBitPerSample;
		    iParamAudioPcm.nSamplingRate = pPcmMode->nSamplingRate;
			iParamAudioPcm.ePCMMode		 = pPcmMode->ePCMMode;
			// This is an indication to the PortManager that the processing
			// function needs to get updated
			aUpdateProcessingFunction = ETrue;
			}

		for (TInt i=0; i<OMX_AUDIO_MAXCHANNELS; i++)
			{
			if (iParamAudioPcm.eChannelMapping[i] != pPcmMode->eChannelMapping[i])
				{
				iParamAudioPcm.eChannelMapping[i] = pPcmMode->eChannelMapping[i];
				// This is an indication to the PortManager that the processing
				// function needs to get updated
				aUpdateProcessingFunction = ETrue;
				}
			}

		}
		break;
	default:
		{
		// Try the parent's indexes
		return COmxILAudioPort::SetParameter(aParamIndex,
											 apComponentParameterStructure,
											 aUpdateProcessingFunction);
		}
		};

	return OMX_ErrorNone;

	}


OMX_ERRORTYPE
COmxILPcmRendererAPB0Port::GetConfig(OMX_INDEXTYPE aConfigIndex,
									 TAny* apComponentConfigStructure) const
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::GetConfig"));

	OMX_ERRORTYPE omxRetValue = OMX_ErrorNone;
	switch(aConfigIndex)
		{
	case OMX_SYMBIANINDEXCONFIGAUDIOPCMVOLUMERAMP:
		{
		if (OMX_ErrorNone !=
			(omxRetValue =
			 TOmxILUtil::CheckOmxStructSizeAndVersion(
				 apComponentConfigStructure,
				 sizeof(OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE))))
			{
			return omxRetValue;
			}

		OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE*
			pPcmVolumeRamp
			= static_cast<
			OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE*>(
				apComponentConfigStructure);

		*pPcmVolumeRamp = iConfigVolumeRamp;
		}
		break;

	case OMX_SYMBIANINDEXCONFIGAUDIODATAAMOUNT:
		{
		if (OMX_ErrorNone !=
		        (omxRetValue =
		                TOmxILUtil::CheckOmxStructSizeAndVersion(
		                        apComponentConfigStructure,
		                        sizeof(OMX_SYMBIAN_AUDIO_CONFIG_PROCESSEDDATAAMOUNTTYPE))))
		    {
			return omxRetValue;
			}
		
		OMX_SYMBIAN_AUDIO_CONFIG_PROCESSEDDATAAMOUNTTYPE*
			pPcmDataAmount
			= static_cast<
			OMX_SYMBIAN_AUDIO_CONFIG_PROCESSEDDATAAMOUNTTYPE*>(
				apComponentConfigStructure);

		TInt bytePlayed = iProcessingFunction.GetBytesPlayed();

        // Convert into OMX_TICKS
		// #define OMX_TICKS_PER_SECOND 1000000
		
#ifndef OMX_SKIP64BIT
		OMX_TICKS ticks = static_cast<OMX_TICKS>(bytePlayed) * 8 * OMX_TICKS_PER_SECOND / 
		        static_cast<OMX_TICKS>(iParamAudioPcm.nBitPerSample) / static_cast<OMX_TICKS>(iParamAudioPcm.nSamplingRate);
		pPcmDataAmount->nProcessedDataAmount = ticks;
#else
		TInt64 ticks = static_cast<TInt64>(bytePlayed) * 8 * OMX_TICKS_PER_SECOND/
		        static_cast<TInt64>(iParamAudioPcm.nBitPerSample) / static_cast<TInt64>(iParamAudioPcm.nSamplingRate);

		pPcmDataAmount->nProcessedDataAmount.nLowPart = ticks & 0xffffffff;
		pPcmDataAmount->nProcessedDataAmount.nHighPart = ticks >> 32;
#endif		
		
		}
		break;
		
	case OMX_IndexConfigAudioVolume:
		{
		if (OMX_ErrorNone !=
			(omxRetValue =
			 TOmxILUtil::CheckOmxStructSizeAndVersion(
				 apComponentConfigStructure,
				 sizeof(OMX_AUDIO_CONFIG_VOLUMETYPE))))
			{
			return omxRetValue;
			}

		OMX_AUDIO_CONFIG_VOLUMETYPE* pConfigVolume
			= static_cast<OMX_AUDIO_CONFIG_VOLUMETYPE*>(
				apComponentConfigStructure);

		*pConfigVolume = iConfigAudioVolume;
		}
		break;

	case OMX_IndexConfigAudioMute:
		{
		if (OMX_ErrorNone !=
			(omxRetValue =
			 TOmxILUtil::CheckOmxStructSizeAndVersion(
				 apComponentConfigStructure,
				 sizeof(OMX_AUDIO_CONFIG_MUTETYPE))))
			{
			return omxRetValue;
			}

		OMX_AUDIO_CONFIG_MUTETYPE* pConfigMute
			= static_cast<OMX_AUDIO_CONFIG_MUTETYPE*>(
				apComponentConfigStructure);

		*pConfigMute = iConfigAudioMute;
		}
		break;

	default:
		{
		// There's no need to try the parent indexes as we know there isn't any
		// other config in the parent audio classes
		return OMX_ErrorUnsupportedIndex;
		}
		};

	return OMX_ErrorNone;

	}

OMX_ERRORTYPE
COmxILPcmRendererAPB0Port::SetConfig(OMX_INDEXTYPE aConfigIndex,
									 const TAny* apComponentConfigStructure,
									 TBool& aUpdateProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::SetConfig"));

	aUpdateProcessingFunction = EFalse;

	OMX_ERRORTYPE omxRetValue = OMX_ErrorNone;
	switch(aConfigIndex)
		{
	case OMX_SYMBIANINDEXCONFIGAUDIOPCMVOLUMERAMP:
		{
		if (OMX_ErrorNone !=
			(omxRetValue =
			 TOmxILUtil::CheckOmxStructSizeAndVersion(
				 const_cast<OMX_PTR>(apComponentConfigStructure),
				 sizeof(OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE))))
			{
			return omxRetValue;
			}

		const OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE*
			pPcmVolumeRamp
			= static_cast<
			const OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE*>(
				apComponentConfigStructure);

		if (iConfigVolumeRamp.nRampDuration != pPcmVolumeRamp->nRampDuration)
			{
			// This is an indication to the PortManager that the processing
			// function needs to get updated
			aUpdateProcessingFunction = ETrue;

			iConfigVolumeRamp.nRampDuration = pPcmVolumeRamp->nRampDuration;
			}

		}
		break;
		
	case OMX_IndexConfigAudioVolume:
		{
		if (OMX_ErrorNone !=
			(omxRetValue =
			 TOmxILUtil::CheckOmxStructSizeAndVersion(
				 const_cast<OMX_PTR>(apComponentConfigStructure),
				 sizeof(OMX_AUDIO_CONFIG_VOLUMETYPE))))
			{
			return omxRetValue;
			}

		const OMX_AUDIO_CONFIG_VOLUMETYPE* pConfigVolume
			= static_cast<const OMX_AUDIO_CONFIG_VOLUMETYPE*>(
				apComponentConfigStructure);

		// Set the new default values
		if (iConfigAudioVolume.bLinear		  != pConfigVolume->bLinear			||
			iConfigAudioVolume.sVolume.nValue != pConfigVolume->sVolume.nValue	||
			iConfigAudioVolume.sVolume.nMin	  != pConfigVolume->sVolume.nMin	||
			iConfigAudioVolume.sVolume.nMax	  != pConfigVolume->sVolume.nMax)
			{
			iConfigAudioVolume.bLinear		  = pConfigVolume->bLinear;
			iConfigAudioVolume.sVolume.nValue = pConfigVolume->sVolume.nValue;
			iConfigAudioVolume.sVolume.nMin	  = pConfigVolume->sVolume.nMin;
			iConfigAudioVolume.sVolume.nMax	  = pConfigVolume->sVolume.nMax;
			// This is an indication to the PortManager that the processing
			// function needs to get updated
			aUpdateProcessingFunction = ETrue;
			}

		}
		break;
	case OMX_IndexConfigAudioMute:
		{
		if (OMX_ErrorNone !=
			(omxRetValue =
			 TOmxILUtil::CheckOmxStructSizeAndVersion(
				 const_cast<OMX_PTR>(apComponentConfigStructure),
				 sizeof(OMX_AUDIO_CONFIG_MUTETYPE))))
			{
			return omxRetValue;
			}

		const OMX_AUDIO_CONFIG_MUTETYPE* pConfigMute
			= static_cast<const OMX_AUDIO_CONFIG_MUTETYPE*>(
				apComponentConfigStructure);

		if (iConfigAudioMute.bMute != pConfigMute->bMute)
			{
			iConfigAudioMute.bMute = pConfigMute->bMute;
			// This is an indication to the PortManager that the processing
			// function needs to get updated
			aUpdateProcessingFunction = ETrue;
			}

		}
		break;
	default:
		{
		// There's no need to try the parent indexes as we know there isn't any
		// other config in the parent audio classes
		return OMX_ErrorUnsupportedIndex;
		}
		};

	return OMX_ErrorNone;
	}

OMX_ERRORTYPE
COmxILPcmRendererAPB0Port::GetExtensionIndex(
	OMX_STRING aParameterName,
	OMX_INDEXTYPE* apIndexType) const
	{
    DEBUG_PRINTF(_L8("COmxILPcmRendererAPB0Port::GetExtensionIndex"));

	TPtrC8 requestedParameterNamePtr(
		const_cast<const TUint8*>(
			reinterpret_cast<TUint8*>(aParameterName)));

	TPtrC8 parameterNamePtr(
		reinterpret_cast<const TUint8*>(OMX_SYMBIAN_INDEX_CONFIG_AUDIO_VOLUMERAMP_NAME));

	if (requestedParameterNamePtr == parameterNamePtr)
		{
		*apIndexType = static_cast<OMX_INDEXTYPE>(OMX_SYMBIANINDEXCONFIGAUDIOPCMVOLUMERAMP);
			
		return OMX_ErrorNone;
		}
	
	parameterNamePtr.Set(reinterpret_cast<const TUint8*>(OMX_SYMBIAN_INDEX_CONFIG_AUDIO_DATAAMOUNT_NAME));

	if (requestedParameterNamePtr == parameterNamePtr)
	        {
		*apIndexType = static_cast<OMX_INDEXTYPE>(OMX_SYMBIANINDEXCONFIGAUDIODATAAMOUNT);
		
		return OMX_ErrorNone;
		}

	*apIndexType = OMX_IndexMax;
	return OMX_ErrorUnsupportedIndex;
	}
