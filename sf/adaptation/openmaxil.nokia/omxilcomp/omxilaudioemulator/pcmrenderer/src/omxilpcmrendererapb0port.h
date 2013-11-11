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

#ifndef OMXILPCMRENDERERAPB0PORT_H
#define OMXILPCMRENDERERAPB0PORT_H

#include <openmax/il/common/omxilaudioport.h>
#include <openmax/il/shai/OMX_Symbian_AudioExt.h>

class COmxILPcmRendererProcessingFunction;

class COmxILPcmRendererAPB0Port : public COmxILAudioPort
	{

public:

	static COmxILPcmRendererAPB0Port* NewL(
		const TOmxILCommonPortData& aCommonPortData,
		const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats,
		const OMX_AUDIO_PARAM_PCMMODETYPE& aParamAudioPcm,
		const OMX_AUDIO_CONFIG_VOLUMETYPE& aConfigAudioVolume,
		const OMX_AUDIO_CONFIG_MUTETYPE& aConfigAudioMute,
		COmxILPcmRendererProcessingFunction& aProcessingFunction);


	~COmxILPcmRendererAPB0Port();

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
	
	OMX_ERRORTYPE GetExtensionIndex(OMX_STRING aParameterName,
									OMX_INDEXTYPE* apIndexType) const;

protected:

	COmxILPcmRendererAPB0Port(
		const OMX_AUDIO_PARAM_PCMMODETYPE& aParamAudioPcm,
		const OMX_AUDIO_CONFIG_VOLUMETYPE& aConfigAudioVolume,
		const OMX_AUDIO_CONFIG_MUTETYPE& aConfigAudioMute,
		COmxILPcmRendererProcessingFunction& aProcessingFunction);

	void ConstructL(const TOmxILCommonPortData& aCommonPortData, const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats);

	OMX_ERRORTYPE SetFormatInPortDefinition(
		const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition,
		TBool& aUpdateProcessingFunction);

	TBool IsTunnelledPortCompatible(
		const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const;

protected:

	OMX_AUDIO_PARAM_PCMMODETYPE iParamAudioPcm;
	OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE iConfigVolumeRamp;
	OMX_AUDIO_CONFIG_VOLUMETYPE iConfigAudioVolume;
	OMX_AUDIO_CONFIG_MUTETYPE iConfigAudioMute;
	RBuf8 iMimeTypeBuf;
	COmxILPcmRendererProcessingFunction& iProcessingFunction;
	};

#endif // OMXILPCMRENDERERAPB0PORT_H
