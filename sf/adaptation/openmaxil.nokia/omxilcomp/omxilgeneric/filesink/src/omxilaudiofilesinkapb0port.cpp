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


#include "omxilaudiofilesinkapb0port.h"
#include "omxilfilesinkprocessingfunction.h"
#include <string.h>
#include <uri8.h>

COmxILAudioFileSinkAPB0Port* COmxILAudioFileSinkAPB0Port::NewL(const TOmxILCommonPortData& aCommonPortData, 
													 const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats,
													 const COmxILFileSinkProcessingFunction& aFileSinkPF)
	{
	COmxILAudioFileSinkAPB0Port* self = new (ELeave) COmxILAudioFileSinkAPB0Port(aFileSinkPF);
	CleanupStack::PushL(self);
	self->ConstructL(aCommonPortData, aSupportedAudioFormats);
	CleanupStack::Pop(self);
	return self;
	}

void COmxILAudioFileSinkAPB0Port::ConstructL(const TOmxILCommonPortData& aCommonPortData, const RArray<OMX_AUDIO_CODINGTYPE>& aSupportedAudioFormats)
	{
	
	
	// base
	COmxILAudioPort::ConstructL(aCommonPortData, aSupportedAudioFormats);
	GetParamPortDefinition().eDomain = OMX_PortDomainAudio;
	}

COmxILAudioFileSinkAPB0Port::COmxILAudioFileSinkAPB0Port(const COmxILFileSinkProcessingFunction& aFileSinkPF)
	: iFileSinkPF(aFileSinkPF)
	{
	}

COmxILAudioFileSinkAPB0Port::~COmxILAudioFileSinkAPB0Port()
	{
	iMimeType.Close();
	CleanUpPort();
	}
	
OMX_ERRORTYPE COmxILAudioFileSinkAPB0Port::GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const
	{
	return COmxILAudioPort::GetLocalOmxParamIndexes(aIndexArray);
	}

OMX_ERRORTYPE COmxILAudioFileSinkAPB0Port::GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const
	{
	return COmxILAudioPort::GetLocalOmxConfigIndexes(aIndexArray);
	}

OMX_ERRORTYPE COmxILAudioFileSinkAPB0Port::GetParameter(OMX_INDEXTYPE aParamIndex, 
														TAny* apComponentParameterStructure) const
	{
	return COmxILAudioPort::GetParameter(aParamIndex, apComponentParameterStructure);
	}
	

OMX_ERRORTYPE COmxILAudioFileSinkAPB0Port::SetParameter(OMX_INDEXTYPE aParamIndex, 
														const TAny* apComponentParameterStructure, 
														TBool& aUpdateProcessingFunction)
	{
	return COmxILAudioPort::SetParameter(aParamIndex, 
										 apComponentParameterStructure, 
										 aUpdateProcessingFunction);
	}

OMX_ERRORTYPE COmxILAudioFileSinkAPB0Port::SetFormatInPortDefinition(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition, 
																TBool& /*aUpdateProcessingFunction*/)
	{
	OMX_ERRORTYPE omxErr = OMX_ErrorNone;
	if (aPortDefinition.format.audio.eEncoding == OMX_AUDIO_CodingUnused)
        {
        OMX_STRING ptempMIMEType = GetParamPortDefinition().format.audio.cMIMEType;
        GetParamPortDefinition().format.audio = aPortDefinition.format.audio;
        GetParamPortDefinition().format.audio.cMIMEType = ptempMIMEType;
        
        if(aPortDefinition.format.audio.cMIMEType)
            {
            TInt len= strlen (aPortDefinition.format.audio.cMIMEType);
            if(len > 0)
                {
                TPtrC8 mimetype(reinterpret_cast<const TUint8 *>(aPortDefinition.format.audio.cMIMEType), len +1 );
                HBufC8 *pHBuf = mimetype.Alloc();
                if(pHBuf != NULL )
                    {
                    iMimeType.Close();
                    iMimeType.Assign(pHBuf );
                    TUint8* pTempBuff = const_cast<TUint8*>(iMimeType.PtrZ() );
                    GetParamPortDefinition().format.audio.cMIMEType = reinterpret_cast<OMX_STRING>(pTempBuff );
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

TBool COmxILAudioFileSinkAPB0Port::IsTunnelledPortCompatible(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const
	{
	if(aPortDefinition.eDomain != GetParamPortDefinition().eDomain)
	    {
	    return EFalse;
	    }

	if (aPortDefinition.format.audio.eEncoding >= OMX_AUDIO_CodingMax)
        {
        return EFalse;
        }	
	return ETrue;	
	}
