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

#ifndef OMXILFILESOURCE_H
#define OMXILFILESOURCE_H

#include <openmax/il/common/omxilcomponent.h>

class COmxILPort;
class COmxILFileSourceProcessingFunction;

NONSHARABLE_CLASS(COmxILFileSource) : public COmxILComponent
	{
public:
	static void CreateComponentL(OMX_HANDLETYPE aComponent);
	~COmxILFileSource();

private:
	COmxILFileSource();
	void ConstructL(OMX_HANDLETYPE aComponent);
	COmxILPort* ConstructPortL() const;
	};

#endif // OMXILFILESOURCE_H
