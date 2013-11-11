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

#include <openmax/il/common/omxilutil.h>
#include "omxiltestmd5sinkport.h"

COmxILTestMD5SinkPort* COmxILTestMD5SinkPort::NewL(const TOmxILCommonPortData& aCommonPortData)
	{
	COmxILTestMD5SinkPort* self = new(ELeave) COmxILTestMD5SinkPort();
    CleanupStack::PushL(self);
    self->ConstructL(aCommonPortData);
    CleanupStack::Pop(self);
    return self;	
	
	}

void COmxILTestMD5SinkPort::ConstructL(const TOmxILCommonPortData& aCommonPortData)
    {
    RArray<OMX_OTHER_FORMATTYPE> supportedOtherFormats;
    COmxILOtherPort::ConstructL(aCommonPortData, supportedOtherFormats);
    GetParamPortDefinition().format.other.eFormat = OMX_OTHER_FormatBinary;
    }

COmxILTestMD5SinkPort::COmxILTestMD5SinkPort()
	{
	}

COmxILTestMD5SinkPort::~COmxILTestMD5SinkPort()
	{
	// nothing to do
	}
	
OMX_ERRORTYPE COmxILTestMD5SinkPort::GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const
	{
	return COmxILOtherPort::GetLocalOmxParamIndexes(aIndexArray);
	}

OMX_ERRORTYPE COmxILTestMD5SinkPort::GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const
	{
    return COmxILOtherPort::GetLocalOmxConfigIndexes(aIndexArray);
	}

OMX_ERRORTYPE COmxILTestMD5SinkPort::GetParameter(OMX_INDEXTYPE aParamIndex,
                                               TAny* apComponentParameterStructure) const
	{
    return COmxILOtherPort::GetParameter(aParamIndex, apComponentParameterStructure);
	}

OMX_ERRORTYPE COmxILTestMD5SinkPort::SetParameter(OMX_INDEXTYPE aParamIndex,
		                                       const TAny* apComponentParameterStructure,
		                                       TBool& aUpdateProcessingFunction)
	{
	return COmxILOtherPort::SetParameter(aParamIndex, apComponentParameterStructure, 
	        aUpdateProcessingFunction);
	}

OMX_ERRORTYPE COmxILTestMD5SinkPort::SetFormatInPortDefinition(
		const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition,
		TBool& aUpdateProcessingFunction)
	{
#ifdef DOMAIN_OTHER
	GetParamPortDefinition().format.other = aPortDefinition.format.other;
#endif
	aUpdateProcessingFunction = EFalse;
	return OMX_ErrorNone;
	}

TBool COmxILTestMD5SinkPort::IsTunnelledPortCompatible(
		const OMX_PARAM_PORTDEFINITIONTYPE& /*aPortDefinition*/) const
	{
    // Note to be revisited when stricter port domain checking is introduced as a 
    // result of current in progress spec. clarrifications. 
	return ETrue;
	}
