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

#ifndef OMXILVIDEOFILESINKOPB0PORT_H
#define OMXILVIDEOFILESINKOPB0PORT_H

#include <openmax/il/common/omxilvideoport.h>
#include "omxilfilesinkprocessingfunction.h"
#include <openmax/il/khronos/v1_x/OMX_Other.h>

class COmxILVideoFileSinkVPB0Port : public COmxILVideoPort
	{
public:
	static COmxILVideoFileSinkVPB0Port* NewL(const TOmxILCommonPortData& aCommonPortData, 
	                                         const RArray<OMX_VIDEO_CODINGTYPE>& aSupportedCodings,
	                                         const RArray<OMX_COLOR_FORMATTYPE>& aSupportedColourFormats,
	                                         const COmxILFileSinkProcessingFunction& aFileSinkPF);

	~COmxILVideoFileSinkVPB0Port();
	
	OMX_ERRORTYPE GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const;
	OMX_ERRORTYPE GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const;

	OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE aParamIndex, 
								TAny* apComponentParameterStructure) const;
	OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE aParamIndex, 
								const TAny* apComponentParameterStructure, 
								TBool& aUpdateProcessingFunction);

private:
	COmxILVideoFileSinkVPB0Port(const COmxILFileSinkProcessingFunction& aFileSinkPF);
	void ConstructL(const TOmxILCommonPortData& aCommonPortData,
	        const RArray<OMX_VIDEO_CODINGTYPE>& aSupportedCodings,
            const RArray<OMX_COLOR_FORMATTYPE>& aSupportedColourFormats);
	OMX_ERRORTYPE SetFormatInPortDefinition(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition, 
											TBool& aUpdateProcessingFunction);
	TBool IsTunnelledPortCompatible(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const;

private:
	const COmxILFileSinkProcessingFunction& iFileSinkPF;
	RBuf8 iMimeType;
	};

#endif // OMXILVIDEOFILESINKOPB0PORT_H
