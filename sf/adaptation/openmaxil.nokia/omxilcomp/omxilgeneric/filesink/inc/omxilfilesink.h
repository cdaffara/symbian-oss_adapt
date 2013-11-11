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

#ifndef OMXILFILESINK_H
#define OMXILFILESINK_H

#include <openmax/il/common/omxilcomponent.h>

class COmxILPort;
class COmxILFileSinkProcessingFunction;

NONSHARABLE_CLASS(COmxILFileSink) : public COmxILComponent
	{		
public:
	static void CreateComponentL(OMX_HANDLETYPE aComponent);
	~COmxILFileSink();
	
private:
	COmxILFileSink();
	void ConstructL(OMX_HANDLETYPE aComponent);
	COmxILPort* ConstructPortL() const;

	};
	
#endif // OMXILFILESINK_H
