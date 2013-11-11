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



#include "omxilvideofilesinkvpb0port.h"
#include "omxilfilesinkprocessingfunction.h"
#include <string.h>
#include <uri8.h>

COmxILVideoFileSinkVPB0Port* COmxILVideoFileSinkVPB0Port::NewL(const TOmxILCommonPortData& aCommonPortData,
	                                                 const RArray<OMX_VIDEO_CODINGTYPE>& aSupportedCodings,
	                                                 const RArray<OMX_COLOR_FORMATTYPE>& aSupportedColourFormats,
													 const COmxILFileSinkProcessingFunction& aFileSinkPF)
	{
	COmxILVideoFileSinkVPB0Port* self = new (ELeave) COmxILVideoFileSinkVPB0Port(aFileSinkPF);
	CleanupStack::PushL(self);
	self->ConstructL(aCommonPortData, aSupportedCodings, aSupportedColourFormats);
	CleanupStack::Pop(self);
	return self;
	}

void COmxILVideoFileSinkVPB0Port::ConstructL(const TOmxILCommonPortData& aCommonPortData,
                                        const RArray<OMX_VIDEO_CODINGTYPE>& aSupportedCodings,
                                        const RArray<OMX_COLOR_FORMATTYPE>& aSupportedColourFormats
)
	{
    COmxILVideoPort::ConstructL(aCommonPortData, aSupportedCodings, aSupportedColourFormats);
	GetSupportedVideoFormats().AppendL(OMX_VIDEO_CodingUnused);
	GetParamPortDefinition().eDomain = OMX_PortDomainVideo;
	}

COmxILVideoFileSinkVPB0Port::COmxILVideoFileSinkVPB0Port(const COmxILFileSinkProcessingFunction& aFileSinkPF)
	: iFileSinkPF(aFileSinkPF)
	{
	}

COmxILVideoFileSinkVPB0Port::~COmxILVideoFileSinkVPB0Port()
	{
	iMimeType.Close();
	CleanUpPort();
	}
	
OMX_ERRORTYPE COmxILVideoFileSinkVPB0Port::GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const
	{
	return COmxILVideoPort::GetLocalOmxParamIndexes(aIndexArray);
	}

OMX_ERRORTYPE COmxILVideoFileSinkVPB0Port::GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const
	{
	return COmxILVideoPort::GetLocalOmxConfigIndexes(aIndexArray);
	}

OMX_ERRORTYPE COmxILVideoFileSinkVPB0Port::GetParameter(OMX_INDEXTYPE aParamIndex, 
														TAny* apComponentParameterStructure) const
	{
	return COmxILVideoPort::GetParameter(aParamIndex, apComponentParameterStructure);
	}

OMX_ERRORTYPE COmxILVideoFileSinkVPB0Port::SetParameter(OMX_INDEXTYPE aParamIndex, 
														const TAny* apComponentParameterStructure, 
														TBool& aUpdateProcessingFunction)
	{
	return COmxILVideoPort::SetParameter(aParamIndex, 
										 apComponentParameterStructure, 
										 aUpdateProcessingFunction);
	}

OMX_ERRORTYPE COmxILVideoFileSinkVPB0Port::SetFormatInPortDefinition(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition, 
																TBool& /*aUpdateProcessingFunction*/)
	{
    OMX_ERRORTYPE omxErr = OMX_ErrorNone;
    if (aPortDefinition.format.video.eCompressionFormat == OMX_VIDEO_CodingUnused)
        {
        OMX_STRING ptempMIMEType = GetParamPortDefinition().format.video.cMIMEType;
        GetParamPortDefinition().format.video = aPortDefinition.format.video;
        
        GetParamPortDefinition().format.video.cMIMEType = ptempMIMEType;
        if(aPortDefinition.format.video.cMIMEType)
            {
            TInt len= strlen (aPortDefinition.format.video.cMIMEType);
            if(len > 0)
                {
                TPtrC8 mimetype(reinterpret_cast<const TUint8 *>(aPortDefinition.format.video.cMIMEType), len +1 );
                HBufC8 *pHBuf = mimetype.Alloc();
                if(pHBuf != NULL )
                    {
                    iMimeType.Close();
                    iMimeType.Assign(pHBuf );
                    TUint8* pTempBuff = const_cast<TUint8*>(iMimeType.PtrZ() );
                    GetParamPortDefinition().format.video.cMIMEType = reinterpret_cast<OMX_STRING>(pTempBuff );
                    }
                }
            }
        }
    else
        {
        omxErr = OMX_ErrorUnsupportedSetting;
        }
    
    return omxErr; 
	}

TBool COmxILVideoFileSinkVPB0Port::IsTunnelledPortCompatible(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const
	{
	if(aPortDefinition.eDomain != GetParamPortDefinition().eDomain)
	    {
	    return EFalse;
	    }

	if (aPortDefinition.format.video.eCompressionFormat == OMX_VIDEO_CodingMax)
        {
        return EFalse;
        }	
	return ETrue;	
	}
