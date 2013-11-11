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

#include <openmax/il/khronos/v1_x/OMX_Component.h>
#include <openmax/il/common/omxilutil.h>

#include "log.h"
#include "omxilmicsourceapb0port.h"
#include "omxilmicsourceprocessingfunction.h"

_LIT8(KMimeTypeAudioPcm, "audio/pcm");

COmxILMicSourceAPB0Port*
COmxILMicSourceAPB0Port::NewL(
	const TOmxILCommonPortData& aCommonPortData,
	const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats,
	const OMX_AUDIO_PARAM_PCMMODETYPE& aParamAudioPcm,
	const OMX_AUDIO_CONFIG_VOLUMETYPE& aConfigAudioVolume,
	const OMX_AUDIO_CONFIG_MUTETYPE& aConfigAudioMute,
	COmxILMicSourceProcessingFunction& aProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::NewL"));

	COmxILMicSourceAPB0Port* self = 
		new (ELeave)COmxILMicSourceAPB0Port(aParamAudioPcm,
											aConfigAudioVolume,
											aConfigAudioMute,
											aProcessingFunction);

	CleanupStack::PushL(self);
	self->ConstructL(aCommonPortData, aSupportedAudioFormats);
	CleanupStack::Pop(self);
	return self;

	}


void
COmxILMicSourceAPB0Port::ConstructL(const TOmxILCommonPortData& aCommonPortData, const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats)
	{
	// base 
	COmxILAudioPort::ConstructL(aCommonPortData, aSupportedAudioFormats);
	
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::ConstructL"));
    OMX_PARAM_PORTDEFINITIONTYPE& paramPortDefinition=GetParamPortDefinition();
	// We have to finish with iParamPortDefinition
    paramPortDefinition.eDomain = OMX_PortDomainAudio;
    paramPortDefinition.format.audio.pNativeRender = 0;

	iMimeTypeBuf.CreateL(KMimeTypeAudioPcm(), KMimeTypeAudioPcm().Length() + 1);

	TUint8* pTUint2 = const_cast<TUint8*>(iMimeTypeBuf.PtrZ());
	paramPortDefinition.format.audio.cMIMEType = reinterpret_cast<OMX_STRING>(pTUint2);

	paramPortDefinition.format.audio.bFlagErrorConcealment = OMX_FALSE;
	paramPortDefinition.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	
	iConfigAudioVolume.bLinear = OMX_FALSE;	
	}


COmxILMicSourceAPB0Port::COmxILMicSourceAPB0Port(
	const OMX_AUDIO_PARAM_PCMMODETYPE& aParamAudioPcm,
	const OMX_AUDIO_CONFIG_VOLUMETYPE& aConfigAudioVolume,
	const OMX_AUDIO_CONFIG_MUTETYPE& aConfigAudioMute,
	COmxILMicSourceProcessingFunction& aProcessingFunction)
	:
	iParamAudioPcm(aParamAudioPcm),
	iConfigAudioVolume(aConfigAudioVolume),
	iConfigAudioMute(aConfigAudioMute),
	iProcessingFunction(aProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::COmxILMicSourceAPB0Port"));
	}


COmxILMicSourceAPB0Port::~COmxILMicSourceAPB0Port()
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::~COmxILMicSourceAPB0Port"));

	iMimeTypeBuf.Close();
	}


OMX_ERRORTYPE
COmxILMicSourceAPB0Port::SetFormatInPortDefinition(
	const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition,
	TBool& aUpdateProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::SetFormatInPortDefinition"));
    
	// There's no point on chaging the cMIMEType type of this port
	// Same thing for pNativeRender and eEncoding
	GetParamPortDefinition().format.audio.bFlagErrorConcealment =
		aPortDefinition.format.audio.bFlagErrorConcealment;
	
	// Error concealment not currently needed at the processing function
	aUpdateProcessingFunction = EFalse;

	return OMX_ErrorNone;
	}


TBool
COmxILMicSourceAPB0Port::IsTunnelledPortCompatible(
	const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::IsTunnelledPortCompatible"));

	// First, the easy checks...
	if(aPortDefinition.eDomain != GetParamPortDefinition().eDomain)
		{
		return EFalse;
		}

	if(aPortDefinition.format.audio.eEncoding != OMX_AUDIO_CodingPCM)
		{
		return EFalse;
		}

	TBool retValue(EFalse);
	RBuf8 tunnelMimeTypeBuf;
	TInt err = tunnelMimeTypeBuf.Create(KMimeTypeAudioPcm().Length() + 1);
	if(err == KErrNone)
		{
	
		tunnelMimeTypeBuf =
			const_cast<const TUint8*>(
				reinterpret_cast<TUint8*>(aPortDefinition.format.audio.cMIMEType));
	
		if (iMimeTypeBuf == tunnelMimeTypeBuf)
			{
			retValue = ETrue;
			}

		tunnelMimeTypeBuf.Close();
		}

	return retValue;
	}


OMX_ERRORTYPE
COmxILMicSourceAPB0Port::GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::GetLocalOmxParamIndexes"));

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
COmxILMicSourceAPB0Port::GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::GetLocalOmxConfigIndexes"));

	// Always collect local indexes from parent
	OMX_ERRORTYPE omxRetValue =
		COmxILAudioPort::GetLocalOmxConfigIndexes(aIndexArray);

	if (OMX_ErrorNone != omxRetValue)
		{
		return omxRetValue;
		}

	TInt err = aIndexArray.InsertInOrder(OMX_IndexConfigAudioVolume);

	// Note that index duplication is OK.
	if (KErrNone == err || KErrAlreadyExists == err)
		{
		err = aIndexArray.InsertInOrder(OMX_IndexConfigAudioMute);
		}

	if (KErrNone != err && KErrAlreadyExists != err)
		{
		return OMX_ErrorInsufficientResources;
		}

	return OMX_ErrorNone;

	}


OMX_ERRORTYPE
COmxILMicSourceAPB0Port::GetParameter(OMX_INDEXTYPE aParamIndex,
										TAny* apComponentParameterStructure) const
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::GetParameter"));

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
COmxILMicSourceAPB0Port::SetParameter(OMX_INDEXTYPE aParamIndex,
										const TAny* apComponentParameterStructure,
										TBool& aUpdateProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::SetParameter"));

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

		// Set the new default values
		if (iParamAudioPcm.eNumData		 != pPcmMode->eNumData			||
			iParamAudioPcm.eEndian		 != pPcmMode->eEndian			||
			iParamAudioPcm.bInterleaved	 != pPcmMode->bInterleaved		||
			iParamAudioPcm.nBitPerSample != pPcmMode->nBitPerSample		||
			iParamAudioPcm.nSamplingRate != pPcmMode->nSamplingRate		||
			iParamAudioPcm.ePCMMode		 != pPcmMode->ePCMMode)
			{
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
		
		if(iParamAudioPcm.nChannels	 != pPcmMode->nChannels)
			{
			iParamAudioPcm.nChannels = pPcmMode->nChannels;
			aUpdateProcessingFunction = ETrue;
			
			//Only change channel mappings when the number of channels has been changed.
			for (TInt i=0; i<OMX_AUDIO_MAXCHANNELS; i++)
				{
				
				if (iParamAudioPcm.eChannelMapping[i] != pPcmMode->eChannelMapping[i])
					{
					iParamAudioPcm.eChannelMapping[i] = pPcmMode->eChannelMapping[i];
					}
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
COmxILMicSourceAPB0Port::GetConfig(OMX_INDEXTYPE aConfigIndex,
									 TAny* apComponentConfigStructure) const
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::GetConfig"));

	OMX_ERRORTYPE omxRetValue = OMX_ErrorNone;
	switch(aConfigIndex)
		{
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
COmxILMicSourceAPB0Port::SetConfig(OMX_INDEXTYPE aConfigIndex,
									 const TAny* apComponentConfigStructure,
									 TBool& aUpdateProcessingFunction)
	{
    DEBUG_PRINTF(_L8("COmxILMicSourceAPB0Port::SetConfig"));

	aUpdateProcessingFunction = EFalse;

	OMX_ERRORTYPE omxRetValue = OMX_ErrorNone;
	switch(aConfigIndex)
		{
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
COmxILMicSourceAPB0Port::DoBufferAllocation(OMX_U32 aSizeBytes, 
											OMX_U8*& apPortSpecificBuffer, 
											OMX_PTR& /* apPortPrivate */, 
											OMX_PTR& /* apPlatformPrivate */,
											OMX_PTR  /* apAppPrivate */)
	{
	//Simply delegate the call down to processing function class.
	return iProcessingFunction.DoBufferAllocation(aSizeBytes, 
												  apPortSpecificBuffer, 
												  GetParamPortDefinition());
	}

void COmxILMicSourceAPB0Port::DoBufferDeallocation(OMX_PTR apPortSpecificBuffer, 
												   OMX_PTR apPortPrivate, 
												   OMX_PTR /*apPlatformPrivate*/, 
												   OMX_PTR /* apAppPrivate */)
	{
	//Simply delegate the call down to processing function class.
	iProcessingFunction.DoBufferDeallocation(apPortSpecificBuffer, apPortPrivate);
	}

OMX_ERRORTYPE 
COmxILMicSourceAPB0Port::DoBufferWrapping(OMX_U32 aSizeBytes, 
										  OMX_U8* apBuffer, 
										  OMX_PTR& apPortPrivate, 
										  OMX_PTR& /*apPlatformPrivate*/,
										  OMX_PTR /* apAppPrivate */ )
	{
	if(aSizeBytes > GetParamPortDefinition().nBufferSize)
		{
		//Make sure the sharing buffers is smaller than the native buffers
		GetParamPortDefinition().nBufferSize = aSizeBytes;
		}
	
	return iProcessingFunction.DoBufferWrapping(aSizeBytes, 
											    apBuffer, 
											    apPortPrivate, 
											    GetParamPortDefinition());
	}

void COmxILMicSourceAPB0Port::DoBufferUnwrapping(OMX_PTR apPortSpecificBuffer, 
												 OMX_PTR apPortPrivate, 
												 OMX_PTR /*apPlatformPrivate*/,
												 OMX_PTR /* apAppPrivate */)
	{
	//Simply delegate the call down to processing function class.
	iProcessingFunction.DoBufferUnwrapping(apPortSpecificBuffer, apPortPrivate);
	}

