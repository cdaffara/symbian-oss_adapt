// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
 * @file
 * @internalTechnology
 */

#ifndef OMXILAUDIOFILESOURCEOPB0PORT_H
#define OMXILAUDIOFILESOURCEOPB0PORT_H

#include <openmax/il/common/omxilaudioport.h>
#include "omxilfilesourceprocessingfunction.h"
#include <openmax/il/khronos/v1_x/OMX_Other.h>

class COmxILAudioFileSourceAPB0Port : public COmxILAudioPort
	{
public:
	static COmxILAudioFileSourceAPB0Port* NewL(const TOmxILCommonPortData& aCommonPortData,
											const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats,
											const COmxILFileSourceProcessingFunction& aFileSourcePF);
	~COmxILAudioFileSourceAPB0Port();

	OMX_ERRORTYPE GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const;
	OMX_ERRORTYPE GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const;

	OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE aParamIndex,
								TAny* apComponentParameterStructure) const;
	OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE aParamIndex,
								const TAny* apComponentParameterStructure,
								TBool& aUpdateProcessingFunction);

private:
	COmxILAudioFileSourceAPB0Port(const COmxILFileSourceProcessingFunction& aFileSourcePF);
	void ConstructL(const TOmxILCommonPortData& aCommonPortData,const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats);
	OMX_ERRORTYPE SetFormatInPortDefinition(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition,
											TBool& aUpdateProcessingFunction);
	TBool IsTunnelledPortCompatible(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const;

private:
	const COmxILFileSourceProcessingFunction& iFileSourcePF;
	RBuf8 iMimeType;
	};

#endif // OMXILAUDIOFILESOURCEOPB0PORT_H
