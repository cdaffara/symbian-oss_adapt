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




#include "omxilotherfilesourceopb0port.h"
#include "omxilfilesourceprocessingfunction.h"
#include <uri8.h>

COmxILOtherFileSourceOPB0Port* COmxILOtherFileSourceOPB0Port::NewL(const TOmxILCommonPortData& aCommonPortData, 
													 const RArray<OMX_OTHER_FORMATTYPE>& aSupportedOtherFormats,
													 const COmxILFileSourceProcessingFunction& aFileSourcePF)
	{
	COmxILOtherFileSourceOPB0Port* self = new (ELeave) COmxILOtherFileSourceOPB0Port(aFileSourcePF);
	CleanupStack::PushL(self);
	self->ConstructL(aCommonPortData, aSupportedOtherFormats);
	CleanupStack::Pop(self);
	return self;
	}

void COmxILOtherFileSourceOPB0Port::ConstructL(const TOmxILCommonPortData& aCommonPortData, const RArray<OMX_OTHER_FORMATTYPE>& aSupportedOtherFormats)
	{
    // base
    COmxILOtherPort::ConstructL(aCommonPortData, aSupportedOtherFormats);    
	GetParamPortDefinition().eDomain = OMX_PortDomainOther;
	GetParamPortDefinition().format.other.eFormat = OMX_OTHER_FormatBinary;
	}

COmxILOtherFileSourceOPB0Port::COmxILOtherFileSourceOPB0Port(const COmxILFileSourceProcessingFunction& aFileSourcePF)
	: iFileSourcePF(aFileSourcePF)
	{
	}

COmxILOtherFileSourceOPB0Port::~COmxILOtherFileSourceOPB0Port()
	{
	CleanUpPort();
	}
	
OMX_ERRORTYPE COmxILOtherFileSourceOPB0Port::GetLocalOmxParamIndexes(RArray<TUint>& aIndexArray) const
	{
	return COmxILOtherPort::GetLocalOmxParamIndexes(aIndexArray);
	}

OMX_ERRORTYPE COmxILOtherFileSourceOPB0Port::GetLocalOmxConfigIndexes(RArray<TUint>& aIndexArray) const
	{
	return COmxILOtherPort::GetLocalOmxConfigIndexes(aIndexArray);
	}

OMX_ERRORTYPE COmxILOtherFileSourceOPB0Port::GetParameter(OMX_INDEXTYPE aParamIndex, 
												   TAny* apComponentParameterStructure) const
	{
	return COmxILOtherPort::GetParameter(aParamIndex, apComponentParameterStructure);
	}

OMX_ERRORTYPE COmxILOtherFileSourceOPB0Port::SetParameter(OMX_INDEXTYPE aParamIndex, 
		           								   const TAny* apComponentParameterStructure, 
		           								   TBool& aUpdateProcessingFunction)
	{
	return COmxILOtherPort::SetParameter(aParamIndex, 
										 apComponentParameterStructure, 
										 aUpdateProcessingFunction);
	}


OMX_ERRORTYPE COmxILOtherFileSourceOPB0Port::SetFormatInPortDefinition(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition, 
																TBool& /*aUpdateProcessingFunction*/)
	{
   OMX_ERRORTYPE OMXErr = OMX_ErrorNone;
    if(aPortDefinition.format.other.eFormat == OMX_OTHER_FormatBinary)
        {
        GetParamPortDefinition().format.other = aPortDefinition.format.other;
        }
    else
        {
        OMXErr=OMX_ErrorUnsupportedSetting;
        }
    return OMXErr;
	}

TBool COmxILOtherFileSourceOPB0Port::IsTunnelledPortCompatible(const OMX_PARAM_PORTDEFINITIONTYPE& aPortDefinition) const
	{
	if(aPortDefinition.eDomain != GetParamPortDefinition().eDomain)
	    {
	    return EFalse;
	    }

	
	if (aPortDefinition.format.other.eFormat == OMX_OTHER_FormatMax)
        {
        return EFalse;
        }
	
	
	return ETrue;	
	}
