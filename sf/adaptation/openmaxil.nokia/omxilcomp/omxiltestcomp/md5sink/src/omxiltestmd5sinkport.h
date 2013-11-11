/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef COMXILTESTMD5SINKPORT_H
#define COMXILTESTMD5SINKPORT_H

#include <openmax/il/common/omxilotherport.h>

class COmxILTestMD5SinkPort : public COmxILOtherPort
	{
public:
	static COmxILTestMD5SinkPort* NewL(const TOmxILCommonPortData& aCommonPortData);
	~COmxILTestMD5SinkPort();
	
	OMX_ERRORTYPE GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const;
	OMX_ERRORTYPE GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const;
	OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE aParamIndex,
			                   TAny* apComponentParameterStructure) const;
	OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE aParamIndex,
			const TAny* apComponentParameterStructure,
			TBool& aUpdateProcessingFunction);

protected:
	OMX_ERRORTYPE SetFormatInPortDefinition(
		const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition,
		TBool& aUpdateProcessingFunction);
	
	TBool IsTunnelledPortCompatible(
		const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const;

private:
	COmxILTestMD5SinkPort();
	void ConstructL(const TOmxILCommonPortData& aCommonPortData);
	};

#endif //COMXILTESTMD5SINKPORT_H
