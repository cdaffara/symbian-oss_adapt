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

#ifndef COMXILTESTMD5SINKCONFIGMANAGER_H_
#define COMXILTESTMD5SINKCONFIGMANAGER_H_

#include <openmax/il/common/omxilconfigmanager.h>

class COmxILTestMD5SinkConfigManager : public COmxILConfigManager
	{
public:
	static COmxILTestMD5SinkConfigManager* NewL(
			const TDesC8& aComponentName,
			const OMX_VERSIONTYPE& aComponentVersion,
			const RPointerArray<TDesC8>& aComponentRoles);
	~COmxILTestMD5SinkConfigManager();
	
	OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE aParamIndex,
			   TAny* apComponentParameterStructure) const;

	OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE aParamIndex,
			   const TAny* apComponentParameterStructure,
			   OMX_BOOL aInitTime = OMX_TRUE);

	void SetHash(const TDesC8& aHash);
	
private:
	COmxILTestMD5SinkConfigManager();
	void ConstructL(const TDesC8& aComponentName,
			const OMX_VERSIONTYPE& aComponentVersion,
			const RPointerArray<TDesC8>& aComponentRoles);
	
	TBuf8<16> iHash;
	};

#endif //COMXILTESTMD5SINKCONFIGMANAGER_H
