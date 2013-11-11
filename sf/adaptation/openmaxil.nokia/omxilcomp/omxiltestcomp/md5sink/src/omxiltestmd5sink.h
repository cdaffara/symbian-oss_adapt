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

#ifndef COMXILTESTMD5SINK_H
#define COMXILTESTMD5SINK_H

#include <openmax/il/common/omxilcomponent.h>

class COmxILTestMD5SinkPort;

NONSHARABLE_CLASS(COmxILTestMD5Sink) : COmxILComponent
	{
public:
	static TInt CreateComponent(OMX_HANDLETYPE hComponent);
	~COmxILTestMD5Sink();
	
protected:
	COmxILTestMD5Sink();
	void ConstructL(OMX_HANDLETYPE hComponent);
	
	void ConstructSinkPortL();
	
private:
	
	COmxILTestMD5SinkPort* ipSinkPort;
	};
	
#endif //COMXILTESTMD5SINK_H
