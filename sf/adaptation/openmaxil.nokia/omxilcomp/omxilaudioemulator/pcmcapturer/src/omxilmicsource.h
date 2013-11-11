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
	@file
	@internalComponent
*/

#ifndef OMXILMICSOURCE_H
#define OMXILMICSOURCE_H

#include <openmax/il/common/omxilcomponent.h>

// Forward declarations
class COmxILMicSourceAPB0Port;
class COmxILClientClockPort;

NONSHARABLE_CLASS(COmxILMicSource) : public COmxILComponent
	{
public:
	static  TInt CreateComponent(OMX_HANDLETYPE aComponent);
	~COmxILMicSource();

private:
	COmxILMicSource();
	void ConstructL(OMX_HANDLETYPE aComponent);

	COmxILMicSourceAPB0Port* ConstructAPB0PortL() const;
	COmxILClientClockPort* ConstructOPB0PortL() const;

private:
	const OMX_VERSIONTYPE iOmxILVersion;
	};

#endif // OMXILMICSOURCE_H
