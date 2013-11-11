/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef OMXILPCMRENDERER_H
#define OMXILPCMRENDERER_H

#include <openmax/il/common/omxilcomponent.h>
#include "omxilpcmrendererprocessingfunction.h"

// Forward declarations
class COmxILPcmRendererAPB0Port;
class COmxILClientClockPort;

NONSHARABLE_CLASS(COmxILPcmRenderer) : public COmxILComponent
	{

public:

	static const TUint8 iComponentVersionMajor	  = 1;
	static const TUint8 iComponentVersionMinor	  = 0;
	static const TUint8 iComponentVersionRevision = 0;
	static const TUint8 iComponentVersionStep	  = 0;

public:

	static  TInt CreateComponent(OMX_HANDLETYPE aComponent);

	~COmxILPcmRenderer();

private:

	COmxILPcmRenderer();

	void ConstructL(OMX_HANDLETYPE aComponent);

	COmxILPcmRendererAPB0Port* ConstructAPB0PortL(COmxILPcmRendererProcessingFunction& aProcessingFunction) const;
	COmxILClientClockPort* ConstructClientClockPortL() const;

private:

	const OMX_VERSIONTYPE iOmxILVersion;
	};

#endif // OMXILPCMRENDERER_H
