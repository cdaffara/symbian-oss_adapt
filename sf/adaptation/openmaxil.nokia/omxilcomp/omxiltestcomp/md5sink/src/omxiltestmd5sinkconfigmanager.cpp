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

#include "omxiltestmd5sinkconfigmanager.h"
#include <openmax/il/extensions/test/omxiltestmd5param.h>

COmxILTestMD5SinkConfigManager* COmxILTestMD5SinkConfigManager::NewL(
		const TDesC8& aComponentName,
		const OMX_VERSIONTYPE& aComponentVersion,
		const RPointerArray<TDesC8>& aComponentRoles)
	{
	COmxILTestMD5SinkConfigManager* self = new(ELeave) COmxILTestMD5SinkConfigManager();
	CleanupStack::PushL(self);
	self->ConstructL(aComponentName, aComponentVersion, aComponentRoles);
	CleanupStack::Pop(self);
	return self;
	}

COmxILTestMD5SinkConfigManager::COmxILTestMD5SinkConfigManager()
	{
	// nothing to do
	}

void COmxILTestMD5SinkConfigManager::ConstructL(const TDesC8& aComponentName,
		const OMX_VERSIONTYPE& aComponentVersion,
		const RPointerArray<TDesC8>& aComponentRoles)
	{
	COmxILConfigManager::ConstructL(aComponentName, aComponentVersion, aComponentRoles);
	}

COmxILTestMD5SinkConfigManager::~COmxILTestMD5SinkConfigManager()
	{

	}
	
OMX_ERRORTYPE COmxILTestMD5SinkConfigManager::GetParameter(OMX_INDEXTYPE aParamIndex,
			   TAny* apComponentParameterStructure) const
	{
	// try the base class first
	OMX_ERRORTYPE error = COmxILConfigManager::GetParameter(aParamIndex, apComponentParameterStructure);
	if(error != OMX_ErrorUnsupportedIndex)
		{
		return error;
		}
		
	if(aParamIndex == OMX_IndexParam_Symbian_MD5)
		{
		OMX_PARAM_SYMBIAN_MD5TYPE* aMD5Struct = static_cast<OMX_PARAM_SYMBIAN_MD5TYPE*>(apComponentParameterStructure);
		TPtr8 aDest(aMD5Struct->pHash, 16);
		aDest = iHash;
		return OMX_ErrorNone;
		}
	
	return OMX_ErrorUnsupportedIndex;
	}

OMX_ERRORTYPE COmxILTestMD5SinkConfigManager::SetParameter(OMX_INDEXTYPE aParamIndex,
			   const TAny* apComponentParameterStructure, OMX_BOOL aInitTime)
	{
	// try the base class first
	OMX_ERRORTYPE error = COmxILConfigManager::SetParameter(aParamIndex, apComponentParameterStructure, aInitTime);
	if(error != OMX_ErrorUnsupportedIndex)
		{
		return error;
		}
	
	return OMX_ErrorUnsupportedIndex;
	}

void COmxILTestMD5SinkConfigManager::SetHash(const TDesC8& aHash)
	{
	iHash = aHash;
	}
