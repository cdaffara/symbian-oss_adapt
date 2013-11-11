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

#ifndef OMXILMICSOURCEAPB0PORT_H
#define OMXILMICSOURCEAPB0PORT_H

#include <openmax/il/common/omxilaudioport.h>

class COmxILMicSourceProcessingFunction;

class COmxILMicSourceAPB0Port : public COmxILAudioPort
	{

public:

	static COmxILMicSourceAPB0Port* NewL(
		const TOmxILCommonPortData& aCommonPortData,
		const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats,
		const OMX_AUDIO_PARAM_PCMMODETYPE& aParamAudioPcm,
		const OMX_AUDIO_CONFIG_VOLUMETYPE& aConfigAudioVolume,
		const OMX_AUDIO_CONFIG_MUTETYPE& aConfigAudioMute,
		COmxILMicSourceProcessingFunction& aProcessingFunction);


	~COmxILMicSourceAPB0Port();

	OMX_ERRORTYPE GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const;

	OMX_ERRORTYPE GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const;

	OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE aParamIndex,
							   TAny* apComponentParameterStructure) const;

	OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE aParamIndex,
							   const TAny* apComponentParameterStructure,
							   TBool& aUpdateProcessingFunction);

	OMX_ERRORTYPE GetConfig(OMX_INDEXTYPE aConfigIndex,
							TAny* apComponentConfigStructure) const;

	OMX_ERRORTYPE SetConfig(OMX_INDEXTYPE aConfigIndex,
							const TAny* apComponentConfigStructure,
							TBool& aUpdateProcessingFunction);

protected:

	COmxILMicSourceAPB0Port(
		const OMX_AUDIO_PARAM_PCMMODETYPE& aParamAudioPcm,
		const OMX_AUDIO_CONFIG_VOLUMETYPE& aConfigAudioVolume,
		const OMX_AUDIO_CONFIG_MUTETYPE& aConfigAudioMute,
		COmxILMicSourceProcessingFunction& iProcessingFunction);

	void ConstructL(const TOmxILCommonPortData& aCommonPortData, const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats);

	OMX_ERRORTYPE SetFormatInPortDefinition(
		const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition,
		TBool& aUpdateProcessingFunction);

	TBool IsTunnelledPortCompatible(
		const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const;
	
	OMX_ERRORTYPE DoBufferAllocation(OMX_U32 aSizeBytes, OMX_U8*& apPortSpecificBuffer, OMX_PTR& apPortPrivate, OMX_PTR& apPlatformPrivate, OMX_PTR apAppPrivate = 0);
	void DoBufferDeallocation(OMX_PTR apPortSpecificBuffer, OMX_PTR apPortPrivate, OMX_PTR apPlatformPrivate, OMX_PTR apAppPrivate = 0);
	OMX_ERRORTYPE DoBufferWrapping(OMX_U32 aSizeBytes, OMX_U8* apBuffer, OMX_PTR& apPortPrivate, OMX_PTR& apPlatformPrivate, OMX_PTR apAppPrivate = 0);
	void DoBufferUnwrapping(OMX_PTR apPortSpecificBuffer, OMX_PTR apPortPrivate, OMX_PTR apPlatformPrivate, OMX_PTR apAppPrivate = 0);
	
protected:

	OMX_AUDIO_PARAM_PCMMODETYPE iParamAudioPcm;
	OMX_AUDIO_CONFIG_VOLUMETYPE iConfigAudioVolume;
	OMX_AUDIO_CONFIG_MUTETYPE iConfigAudioMute;
	RBuf8 iMimeTypeBuf;
	
	COmxILMicSourceProcessingFunction& iProcessingFunction;	
	};

#endif // OMXILMICSOURCEAPB0PORT_H
