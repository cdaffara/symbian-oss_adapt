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
#ifndef COMXILTESTMD5SINKPROCESSINGFUNCTION_H
#define COMXILTESTMD5SINKPROCESSINGFUNCTION_H

#include <e32base.h>
#include <e32msgqueue.h>
#include <f32file.h>
#include <openmax/il/khronos/v1_x/OMX_Index.h>
#include <openmax/il/khronos/v1_x/OMX_Component.h>
#include <openmax/il/common/omxilprocessingfunction.h>

class COmxILTestMD5SinkConfigManager;
class CMD5Active;
class CFlushActive;

class COmxILTestMD5SinkProcessingFunction : public COmxILProcessingFunction
	{
public:
	static COmxILTestMD5SinkProcessingFunction* NewL(MOmxILCallbackNotificationIf&);
	~COmxILTestMD5SinkProcessingFunction();

	void SetConfigManager(COmxILTestMD5SinkConfigManager* aConfigManager);
	
	OMX_ERRORTYPE StateTransitionIndication(TStateIndex aNewState);
	OMX_ERRORTYPE BufferFlushingIndication(TUint32 aPortIndex, OMX_DIRTYPE aDirection);
	OMX_ERRORTYPE ParamIndication(OMX_INDEXTYPE aParamIndex, const TAny* apComponentParameterStructure);
	OMX_ERRORTYPE ConfigIndication(OMX_INDEXTYPE aConfigIndex, const TAny* apComponentConfigStructure);
	OMX_ERRORTYPE BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE aDirection);
	OMX_BOOL BufferRemovalIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE aDirection);

private:
	COmxILTestMD5SinkProcessingFunction(MOmxILCallbackNotificationIf&);
	void ConstructL(MOmxILCallbackNotificationIf&);
			
private:
	CMD5Active* iMD5Active;
	CFlushActive* iFlushHandler;
	};

class CMD5;

class CMD5Active : public CActive
	{
public:
	static CMD5Active* NewL(MOmxILCallbackNotificationIf& aCallback);
	~CMD5Active();
	
	OMX_ERRORTYPE DeliverBuffer(OMX_BUFFERHEADERTYPE*);
	OMX_BOOL RemoveBuffer(OMX_BUFFERHEADERTYPE*);
	
	void FlushHeaders();
	void SetConfigManager(COmxILTestMD5SinkConfigManager* aConfigManager);
		
protected:
	void RunL();
	void DoCancel();
	
private:
	CMD5Active(MOmxILCallbackNotificationIf& aCallback);
	void ConstructL();
	
	TBool RemoveFromQueueL(RMsgQueue<OMX_BUFFERHEADERTYPE*>& aQueue, OMX_BUFFERHEADERTYPE* aBufferHeader);

	COmxILTestMD5SinkConfigManager* iConfigManager;
	RMsgQueue<OMX_BUFFERHEADERTYPE*> iBuffers;
	MOmxILCallbackNotificationIf& iCallback;
	CMD5* iMD5;
	};

/*
 * Dependant on whether the client has an Active Scheduler or not the active objects may be running in a 
 * different thread context to that of the caller, in which case for flush to work it needs to run in this
 * context to ensure we don't complete while CMD5Active::RunL() is executing concurrently which would 
 * fail the conformance suite and the spec. because we complete the flush call yet still own a buffer.
 */
class CFlushActive : public CActive
    {
public:
    static CFlushActive* NewL(CMD5Active& aBufferHandler);
    ~CFlushActive();
    
    void Flush();
    
protected:
    CFlushActive(CMD5Active& aBufferHandler);
    void ConstructL();
    
    void RunL();
    void DoCancel();
    
private:
    CMD5Active& iBufferHandler;
    RSemaphore iWaitSemaphore;
    RThread iRunLContext;
    };

#endif //COMXILTESTMD5SINKPROCESSINGFUNCTION_H
