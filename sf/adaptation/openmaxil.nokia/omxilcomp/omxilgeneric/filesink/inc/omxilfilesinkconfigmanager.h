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



/**
 * @file
 * @internalTechnology
 */

#ifndef OMXILFILESINKCONFIGMANAGER_H
#define OMXILFILESINKCONFIGMANAGER_H

#include <openmax/il/common/omxilconfigmanager.h>

class COmxILFileSinkProcessingFunction;

NONSHARABLE_CLASS(COmxILFileSinkConfigManager) : public COmxILConfigManager
	{
public:
	static COmxILFileSinkConfigManager* NewL(
			const TDesC8& aComponentName,
			const OMX_VERSIONTYPE& aComponentVersion,
			const RPointerArray<TDesC8>& aComponentRoleList,
			COmxILFileSinkProcessingFunction& aFileSinkPF);

	~COmxILFileSinkConfigManager();
	
	OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE aParamIndex, TAny* apComponentParameterStructure) const;
	OMX_ERRORTYPE SetParameter(OMX_INDEXTYPE aParamIndex, const TAny* apComponentParameterStructure, OMX_BOOL aInitTime = OMX_TRUE);
	
private:
	COmxILFileSinkConfigManager(COmxILFileSinkProcessingFunction& aFileSinkPF);
	void ConstructL(const TDesC8& aComponentName, const OMX_VERSIONTYPE& aComponentVersion, const RPointerArray<TDesC8>& aComponentRoleList);

private:
	COmxILFileSinkProcessingFunction& iFileSinkPF;
	};

#endif // OMXILFILESINKCONFIGMANAGER_H
