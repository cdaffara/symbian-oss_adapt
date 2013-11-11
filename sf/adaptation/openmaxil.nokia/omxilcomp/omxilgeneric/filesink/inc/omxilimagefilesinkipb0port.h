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

#ifndef OMXILIMAGEFILESINKOPB0PORT_H
#define OMXILIMAGEFILESINKOPB0PORT_H

#include <openmax/il/common/omxilimageport.h>
#include "omxilfilesinkprocessingfunction.h"
#include <openmax/il/khronos/v1_x/OMX_Other.h>

class COmxILImageFileSinkIPB0Port : public COmxILImagePort
	{
public:
	static COmxILImageFileSinkIPB0Port* NewL(const TOmxILCommonPortData& aCommonPortData, 
                                            const RArray<OMX_IMAGE_CODINGTYPE>& aSupportedImageFormats,
                                            const RArray<OMX_COLOR_FORMATTYPE>& aSupportedColorFormats,
                                            const COmxILFileSinkProcessingFunction& aFileSinkPF);

	~COmxILImageFileSinkIPB0Port();
	
	OMX_ERRORTYPE GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const;
	OMX_ERRORTYPE GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const;

	OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE aParamIndex, 
								TAny* apComponentParameterStructure) const;
	OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE aParamIndex, 
								const TAny* apComponentParameterStructure, 
								TBool& aUpdateProcessingFunction);

private:
	COmxILImageFileSinkIPB0Port(const COmxILFileSinkProcessingFunction& aFileSinkPF);
	void ConstructL(const TOmxILCommonPortData& aCommonPortData,
                    const RArray<OMX_IMAGE_CODINGTYPE>& aSupportedImageFormats,
                    const RArray<OMX_COLOR_FORMATTYPE>& aSupportedColorFormats);
	OMX_ERRORTYPE SetFormatInPortDefinition(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition, 
											TBool& aUpdateProcessingFunction);
	TBool IsTunnelledPortCompatible(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const;

private:
	const COmxILFileSinkProcessingFunction& iFileSinkPF;
	RBuf8 iMimeType;
	};

#endif // OMXILIMAGEFILESINKOPB0PORT_H
