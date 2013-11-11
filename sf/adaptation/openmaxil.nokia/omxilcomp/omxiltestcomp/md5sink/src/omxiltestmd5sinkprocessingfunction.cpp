/*
* Copyright (c) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <hash.h>
#include "omxiltestmd5sinkprocessingfunction.h"
#include "omxiltestmd5sinkconfigmanager.h"
#include <openmax/il/common/omxilcallbacknotificationif.h>
#include "omxiltestmd5sinkconst.h"

enum { KMsgQueueSize = 10 };

COmxILTestMD5SinkProcessingFunction* COmxILTestMD5SinkProcessingFunction::NewL(MOmxILCallbackNotificationIf& aCallbacks)
	{
	COmxILTestMD5SinkProcessingFunction* self = new (ELeave) COmxILTestMD5SinkProcessingFunction(aCallbacks);
	CleanupStack::PushL(self);
	self->ConstructL(aCallbacks);
	CleanupStack::Pop(self);
	return self;
	}

COmxILTestMD5SinkProcessingFunction::COmxILTestMD5SinkProcessingFunction(MOmxILCallbackNotificationIf& aCallbacks)
: COmxILProcessingFunction(aCallbacks)
	{
	}

void COmxILTestMD5SinkProcessingFunction::ConstructL(MOmxILCallbackNotificationIf& aCallback)
	{
	iMD5Active = CMD5Active::NewL(aCallback);
	iFlushHandler = CFlushActive::NewL(*iMD5Active);
	}

COmxILTestMD5SinkProcessingFunction::~COmxILTestMD5SinkProcessingFunction()
	{
    delete iFlushHandler;
	delete iMD5Active;
	}

void COmxILTestMD5SinkProcessingFunction::SetConfigManager(COmxILTestMD5SinkConfigManager* aConfigManager)
	{
	iMD5Active->SetConfigManager(aConfigManager);
	}

OMX_ERRORTYPE COmxILTestMD5SinkProcessingFunction::StateTransitionIndication(TStateIndex /*aNewState*/)
	{
    return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILTestMD5SinkProcessingFunction::BufferFlushingIndication(TUint32 aPortIndex, OMX_DIRTYPE aDirection)
	{
    if ((aPortIndex != OMX_ALL) && ((aPortIndex != KMD5SINK_OPB0PORT_INDEX) || (aDirection != OMX_DirInput)))
        {
        return OMX_ErrorNone;
        }
    
    iFlushHandler->Flush();
    
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILTestMD5SinkProcessingFunction::ParamIndication(OMX_INDEXTYPE /*aParamIndex*/,
													const TAny* /*apComponentParameterStructure*/)
	{
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILTestMD5SinkProcessingFunction::ConfigIndication(OMX_INDEXTYPE /*aConfigIndex*/,
												 const TAny* /*apComponentConfigStructure*/)
	{
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILTestMD5SinkProcessingFunction::BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader,
																OMX_DIRTYPE aDirection)			
	{
	__ASSERT_ALWAYS(aDirection == OMX_DirInput, User::Invariant());
	return iMD5Active->DeliverBuffer(apBufferHeader);
	}

OMX_BOOL COmxILTestMD5SinkProcessingFunction::BufferRemovalIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE aDirection)
	{
	__ASSERT_ALWAYS(aDirection == OMX_DirInput, User::Invariant());
	return iMD5Active->RemoveBuffer(apBufferHeader);
	}

///////////////////////////////////////////////////////////////////////////////////////////

CMD5Active* CMD5Active::NewL(MOmxILCallbackNotificationIf& aCallback)
	{
	CMD5Active* self = new(ELeave) CMD5Active(aCallback);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CMD5Active::CMD5Active(MOmxILCallbackNotificationIf& aCallback):
CActive(CActive::EPriorityStandard),
iCallback(aCallback)
	{
	CActiveScheduler::Add(this);
	}

void CMD5Active::ConstructL()
	{
	iMD5 = CMD5::NewL();
	User::LeaveIfError(iBuffers.CreateLocal(KMsgQueueSize));
	iBuffers.NotifyDataAvailable(iStatus);
	SetActive();
	}

CMD5Active::~CMD5Active()
	{
	Cancel();
	delete iMD5;
	}

void CMD5Active::SetConfigManager(COmxILTestMD5SinkConfigManager* aConfigManager)
	{
	iConfigManager = aConfigManager;
	}

OMX_ERRORTYPE CMD5Active::DeliverBuffer(OMX_BUFFERHEADERTYPE* aBuffer)
	{
	TInt aErr = iBuffers.Send(aBuffer);
	if(aErr == KErrNone)
		{
		return OMX_ErrorNone;
		}
	else if(aErr == KErrOverflow)
		{
		return OMX_ErrorInsufficientResources;
		}
	else
		{
		return OMX_ErrorUndefined;
		}
	}

OMX_BOOL CMD5Active::RemoveBuffer(OMX_BUFFERHEADERTYPE* aBuffer)
	{
	TBool removed = EFalse;
	TRAPD(err, removed = RemoveFromQueueL(iBuffers, aBuffer));
	if(err)
		{
		User::Invariant();
		}
	return removed ? OMX_TRUE : OMX_FALSE;
	}

TBool CMD5Active::RemoveFromQueueL(RMsgQueue<OMX_BUFFERHEADERTYPE*>& aQueue, OMX_BUFFERHEADERTYPE* aBufferHeader)
	{
	TBool removed = EFalse;
	RMsgQueue<OMX_BUFFERHEADERTYPE*> tempQueue;
	User::LeaveIfError(tempQueue.CreateLocal(KMsgQueueSize));
	OMX_BUFFERHEADERTYPE* bufferHeader = NULL;
	while(aQueue.Receive(bufferHeader) != KErrUnderflow)
		{
		if(bufferHeader != aBufferHeader)
			{
			__ASSERT_ALWAYS(tempQueue.Send(bufferHeader) == KErrNone, User::Invariant());
			}
		else
			{
			removed = ETrue;
			}
		}
	while(tempQueue.Receive(bufferHeader) != KErrUnderflow)
		{
		__ASSERT_ALWAYS(aQueue.Send(bufferHeader) == KErrNone, User::Invariant());
		}
	tempQueue.Close();
	return removed;
	}

void CMD5Active::RunL()
	{
	OMX_BUFFERHEADERTYPE* aBuff;
	// may have an empty queue despite being notified if a RemoveBuffer() occured in the meantime
	if(iBuffers.Receive(aBuff) != KErrUnderflow)
		{	
		// hash the buffer
		TPtrC8 aDes(aBuff->pBuffer + aBuff->nOffset, aBuff->nFilledLen);
		TPtrC8 aHash = iMD5->Hash(aDes);
		iConfigManager->SetHash(aHash);
		aBuff->nFilledLen = 0;
		OMX_U32 flags = aBuff->nFlags;
		iCallback.BufferDoneNotification(aBuff, KMD5SINK_OPB0PORT_INDEX, OMX_DirInput);
		if(flags)
			{
			iCallback.EventNotification(OMX_EventBufferFlag, 0, flags, NULL);
			}
		}	
	iBuffers.NotifyDataAvailable(iStatus);
	SetActive();
	}

void CMD5Active::FlushHeaders()
    {
    OMX_BUFFERHEADERTYPE* aBuff;
    
    while(iBuffers.Receive(aBuff) == KErrNone)
        {   
        aBuff->nFilledLen = 0;
        iCallback.BufferDoneNotification(aBuff, KMD5SINK_OPB0PORT_INDEX, OMX_DirInput);
        }
    }

void CMD5Active::DoCancel()
	{
	iBuffers.CancelDataAvailable();
	}

///////////////////////////////////////////////////////////////////////////////////////////

CFlushActive* CFlushActive::NewL(CMD5Active& aBufferHandler)
    {
    CFlushActive* self = new (ELeave) CFlushActive(aBufferHandler);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CFlushActive::CFlushActive(CMD5Active& aBufferHandler):
CActive(CActive::EPriorityHigh),    // Higher priority than CMD5Active to ensure we run before them when the Active Scheduler is picking who to run next
iBufferHandler(aBufferHandler)
    {
    CActiveScheduler::Add(this);
    iRunLContext = RThread();
    iStatus = KRequestPending;
    SetActive();
    }

void CFlushActive::ConstructL()
    {
    User::LeaveIfError(iWaitSemaphore.CreateLocal(0));
    }

CFlushActive::~CFlushActive()
    {
    Cancel();
    iRunLContext.Close();
    iWaitSemaphore.Close(); 
    }

void CFlushActive::Flush()
    {
    RThread currentContext;
    if (currentContext.Id() == iRunLContext.Id())
        {
        // The same context so cannot be running concurrently with CMD5Active::RunL() so safe to drain buffer queue
        iBufferHandler.FlushHeaders();
        return;
        }
    
    // Different thread to the one in which the Active objects are running,
    TRequestStatus* selfStatus = &iStatus;
    iRunLContext.RequestComplete(selfStatus, KErrNone);
    iWaitSemaphore.Wait();
    }
     
void CFlushActive::RunL()
    {
    // Now sure we are not running concurrently with CMD5Active::RunL()
    iBufferHandler.FlushHeaders();
    iStatus = KRequestPending;
    SetActive();
    iWaitSemaphore.Signal();
    }

void CFlushActive::DoCancel()
    {
    iWaitSemaphore.Signal();    // Only happens as part of deletion so irrelevant whether we are blocked or not
    TRequestStatus* selfStatus = &iStatus;
    User::RequestComplete(selfStatus, KErrNone);
    }
