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

#include <uri8.h>
#include <openmax/il/common/omxilcallbacknotificationif.h>
#include "omxilfilesourceprocessingfunction.h"

const TInt KMaxMsgQueueEntries = 25;
// This represents the number of times this component will respond to a client when the file handle is invalid
// (e.g a URI is not specified) before sending a OMX_BUFFERFLAG_EOS notification flag. This is to prevent
// the endless buffer passing. 250 is chosen as it is more than the number of buffers passed by Khronos conformance
// suite during port communication test.
const TInt KMaxBuffersWhenNoFileName =250; 
const TInt KBufferSizeReturnedWhenNoFileName = 2;

COmxILFileSourceProcessingFunction* COmxILFileSourceProcessingFunction::NewL(MOmxILCallbackNotificationIf& aCallbacks)
	{
	COmxILFileSourceProcessingFunction* self = new (ELeave) COmxILFileSourceProcessingFunction(aCallbacks);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

COmxILFileSourceProcessingFunction::COmxILFileSourceProcessingFunction(MOmxILCallbackNotificationIf& aCallbacks)
: COmxILProcessingFunction(aCallbacks)
	{
	}

void COmxILFileSourceProcessingFunction::ConstructL()
	{
	iState = OMX_StateLoaded;
	ipFileSourceAccess = CFileSourceAccess::NewL(*this);
	ipBufferArrivalHandler = CBufferArrivalHandler::NewL(*this, *ipFileSourceAccess);
	ipPFHelper = CPFHelper::NewL(*this, *ipFileSourceAccess, *ipBufferArrivalHandler);
	}

COmxILFileSourceProcessingFunction::~COmxILFileSourceProcessingFunction()
	{
	if(ipPFHelper &&
	   (iState == OMX_StateInvalid  ||
	    iState == OMX_StateExecuting ||
	    iState == OMX_StatePause))
		{
		ipPFHelper->StopSync();
		}

	delete ipBufferArrivalHandler;
	delete ipPFHelper;
	delete ipFileSourceAccess;
	delete ipUri;
	delete ipFileName;

	// Buffer headers are not owned by the processing function
    iBuffersToFill.Close();
	}

OMX_ERRORTYPE COmxILFileSourceProcessingFunction::StateTransitionIndication(TStateIndex aNewState)
	{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	switch(aNewState)
		{
		case EStateExecuting:
			{
			if (ipPFHelper->ExecuteAsync() != KErrNone)
				{
				err = OMX_ErrorInsufficientResources;
				}
			}
			break;

		case EStateInvalid:
			{
            if (ipPFHelper && ipPFHelper->StopAsync() != KErrNone)
                {
                err = OMX_ErrorInsufficientResources;
                }

			}
			break;

		case EStatePause:
			{
			ipPFHelper->PauseAsync();
			}
			break;

		case EStateIdle:
			{
			ipPFHelper->IdleAsync();
			}
			break;

		case EStateLoaded:
		case EStateWaitForResources:
			{
			if (ipPFHelper && ipPFHelper->StopAsync() != KErrNone)
				{
				err = OMX_ErrorInsufficientResources;
				}
			}
			break;

		case ESubStateExecutingToIdle:
			{
			ipPFHelper->StopAsync();
			}
			break;
		case ESubStatePauseToIdle:
	    case ESubStateLoadedToIdle:
	    case ESubStateIdleToLoaded:
			break;

		default:
			{
			err = OMX_ErrorIncorrectStateTransition;
			}
		};

	return err;
	}

OMX_ERRORTYPE COmxILFileSourceProcessingFunction::BufferFlushingIndication(TUint32 aPortIndex, OMX_DIRTYPE aDirection)
	{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if ((aPortIndex == OMX_ALL && aDirection == OMX_DirMax) ||
        (aPortIndex == 0 && aDirection == OMX_DirOutput))
        {
        err = CPFHelper::ConvertSymbianErrorType(ipPFHelper->FlushIndication());
        }
    else
        {
        err = OMX_ErrorBadParameter;
        }
    return err;
    }

OMX_ERRORTYPE COmxILFileSourceProcessingFunction::ParamIndication(OMX_INDEXTYPE aParamIndex, const TAny* apComponentParameterStructure)
	{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	switch(aParamIndex)
		{
		case OMX_IndexParamPortDefinition:
			{
			//const OMX_PARAM_PORTDEFINITIONTYPE* portDefinition = static_cast<const OMX_PARAM_PORTDEFINITIONTYPE*>(apComponentParameterStructure);
			//nothing to do
			//
			//the number of buffers may change depending on capture mode (single shot vs burst mode)
			//in that case, we need to do something for PF...
			break;
			}
		case OMX_IndexParamContentURI:
			{
			const OMX_PARAM_CONTENTURITYPE* contentUriType = reinterpret_cast<const OMX_PARAM_CONTENTURITYPE*>(apComponentParameterStructure);
			err = SetFileName(contentUriType);
			break;
			}
		default:
			{
			err = OMX_ErrorUnsupportedIndex;
			}
		}
	return err;
	}

OMX_ERRORTYPE COmxILFileSourceProcessingFunction::ConfigIndication(OMX_INDEXTYPE /*aConfigIndex*/, const TAny* /*apComponentConfigStructure*/)
	{
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILFileSourceProcessingFunction::BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE aDirection)
	{
	if (aDirection == OMX_DirOutput)
    	{
    	if (ipBufferArrivalHandler->BufferIndication(apBufferHeader) != KErrNone)
    		{
    		return OMX_ErrorInsufficientResources;
    		}
		}
    else
    	{
    	return OMX_ErrorBadParameter;
    	}

    return OMX_ErrorNone;
	}

OMX_BOOL COmxILFileSourceProcessingFunction::BufferRemovalIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE /*aDirection*/)
	{
	return ipPFHelper->RemoveBufferIndication(apBufferHeader);
	}

MOmxILCallbackNotificationIf& COmxILFileSourceProcessingFunction::GetCallbacks()
	{
	return iCallbacks;
	}

OMX_ERRORTYPE COmxILFileSourceProcessingFunction::SetFileName(const OMX_PARAM_CONTENTURITYPE* aContentUriType)
	{
	ASSERT(aContentUriType);
	delete ipFileName;
	ipFileName = NULL;
	delete ipUri;
	ipUri = NULL;

	TInt sizeOfUri = aContentUriType->nSize - _FOFF(OMX_PARAM_CONTENTURITYPE, contentURI); //Actual size of URI
	if (sizeOfUri <= 0)
		{
		return OMX_ErrorBadParameter;
		}

	// Don't include the zero character at the end.
	TPtrC8 uriDes(aContentUriType->contentURI,sizeOfUri);

	TInt err = KErrNone;
	do
		{
		TUriParser8 parser;
		err = parser.Parse(uriDes);
		if (err != KErrNone)
			{
			break;
			}

		TRAP(err, ipFileName = parser.GetFileNameL());
		if (err != KErrNone)
			{
			break;
			}

		// Remove Null charcter '\0' if any.
		TPtr filePtr(ipFileName->Des());
		TInt index = filePtr.LocateReverse('\0');
		if (index != KErrNotFound && index == filePtr.Length()-1)
		    {
		    filePtr.Delete(index,1);
		    }

		uriDes.Set(reinterpret_cast<const TUint8 *>(aContentUriType), aContentUriType->nSize );
        ipUri = uriDes.Alloc();
        if (!ipUri)
            {
            err = KErrNoMemory;
            break;
            }

		return OMX_ErrorNone;
		}
	while (EFalse);

	// Something failed.
	__ASSERT_DEBUG( (err == KErrNone), User::Panic(_L("UriParsing"), err) );

	delete ipFileName;
	ipFileName = NULL;
	delete ipUri;
	ipUri = NULL;
	return (err == KErrNoMemory ? OMX_ErrorInsufficientResources : OMX_ErrorBadParameter);
	}


const HBufC* COmxILFileSourceProcessingFunction::FileName() const
	{
	return ipFileName;
	}

const HBufC8* COmxILFileSourceProcessingFunction::Uri() const
	{
	return ipUri;
	}

COmxILFileSourceProcessingFunction::CFileSourceAccess* COmxILFileSourceProcessingFunction::CFileSourceAccess::NewL(COmxILFileSourceProcessingFunction& aParent)
	{
	CFileSourceAccess* self = new (ELeave) CFileSourceAccess(aParent);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

COmxILFileSourceProcessingFunction::CFileSourceAccess::CFileSourceAccess(COmxILFileSourceProcessingFunction& aParent)
	: CActive(EPriorityStandard),
	iParent(aParent),
	iBufferOffset(0),
	iReadBuffer(0,0),
	iEOSFlag(EFalse)
	{
	CActiveScheduler::Add(this);
	}

void COmxILFileSourceProcessingFunction::CFileSourceAccess::ConstructL()
	{
	User::LeaveIfError(iFs.Connect());
	}

COmxILFileSourceProcessingFunction::CFileSourceAccess::~CFileSourceAccess()
	{
	Cancel();

	iFileHandle.Close();
	iFs.Close();
	}

void COmxILFileSourceProcessingFunction::CFileSourceAccess::RunL()
	{
	// The buffer is not on the list implies that they have already been flushed/spotted
	// via BufferFlushingIndication/BufferRemovalIndication
	TInt index = iParent.iBuffersToFill.Find(iCurrentBuffer);
	if (KErrNotFound != index)
		{
		switch(iStatus.Int())
			{
			case KErrNone:
				{
				if (iReadBuffer.Length()==0) //the end of the file
				    {
				    iFileHandle.Close();
				    iCurrentBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
				    iParent.GetCallbacks().EventNotification(OMX_EventBufferFlag, iCurrentBuffer->nOutputPortIndex, OMX_BUFFERFLAG_EOS, NULL);
				    iEOSFlag = ETrue;
				    }
				iCurrentBuffer->nFilledLen=iReadBuffer.Length();
				iCurrentBuffer->nOffset = 0;
				break;
				}
			default:
				{
				User::Leave(iStatus.Int());
				}
			};

		iParent.GetCallbacks().BufferDoneNotification(iCurrentBuffer,iCurrentBuffer->nOutputPortIndex,OMX_DirOutput);
		iParent.iBuffersToFill.Remove(index);
		iCurrentBuffer = NULL;
		}
	
    if(iFileHandle.SubSessionHandle() != 0)
        {
        ProcessNextBuffer();
        }
	}

TInt COmxILFileSourceProcessingFunction::CFileSourceAccess::ProcessNextBuffer()
	{
	TInt err = KErrNone;
	if ((iParent.iBuffersToFill.Count() > 0) && !IsActive() && iParent.iState == OMX_StateExecuting)
		{
		iCurrentBuffer = iParent.iBuffersToFill[0];
		iReadBuffer.Set(iCurrentBuffer->pBuffer, iCurrentBuffer->nAllocLen, iCurrentBuffer->nAllocLen);
        if (iFileHandle.SubSessionHandle() == 0)
            {
            if(iEOSFlag == EFalse)
                {
                // This condition caters for when an IL client does not set the URI before trasitioning to 
                // executing state (like the Khronos conformance suite). The proposed way to deal with this is to keep
                // sending BufferDone notification with blank buffers with size >0 for KMaxBuffersWhenNoFileName number
                // of times before OMX_BUFFERFLAG_EOS flag is sent to client. This simulates a normal communication.
                if( iEOSBufferNotificationCount <= KMaxBuffersWhenNoFileName )
                    {
                    TUint nBufferSize = 0;
                    if (iEOSBufferNotificationCount < KMaxBuffersWhenNoFileName)
                        {
                        nBufferSize = KBufferSizeReturnedWhenNoFileName;
                        }
                    iReadBuffer.Set(iCurrentBuffer->pBuffer, nBufferSize, iCurrentBuffer->nAllocLen);                  
                    iEOSBufferNotificationCount++;              
                    SetActive();
                    TRequestStatus* status(&iStatus);
                    User::RequestComplete(status, KErrNone);
                    }
                }
            else
                {
                iReadBuffer.Set(iCurrentBuffer->pBuffer, iCurrentBuffer->nAllocLen, iCurrentBuffer->nAllocLen);
                // do nothing. this menas that all operations have completed succesfully.
                }
            
            }
        else
            {
            iFileHandle.Read(iReadBuffer, iCurrentBuffer->nAllocLen, iStatus);
            SetActive();
            }
		}
	return err;
	}

void COmxILFileSourceProcessingFunction::CFileSourceAccess::DoCancel()
	{
	if (iFileHandle.SubSessionHandle() != 0)
	    {
	    iFileHandle.Close();
	    }
	iEOSFlag = EFalse;
	iEOSBufferNotificationCount = 0;
	
	iCurrentBuffer = NULL;
	}

TInt COmxILFileSourceProcessingFunction::CFileSourceAccess::Execute()
	{
	iEOSFlag = EFalse;
	iEOSBufferNotificationCount = 0;
	iParent.iState = OMX_StateExecuting;
	return ProcessNextBuffer();
	}

void COmxILFileSourceProcessingFunction::CFileSourceAccess::Pause()
	{
	iParent.iState = OMX_StatePause;
	}

void COmxILFileSourceProcessingFunction::CFileSourceAccess::Idle()
    {
    iParent.iState = OMX_StateIdle;
    iParent.iBuffersToFill.Reset();
    }

void COmxILFileSourceProcessingFunction::CFileSourceAccess::CancelDataTransfer()
    {
    if(iFileHandle.SubSessionHandle() != 0)
        {
        iFileHandle.ReadCancel();
        }
    }

void COmxILFileSourceProcessingFunction::CFileSourceAccess::Stop()
	{
	if(iParent.iState == OMX_StateExecuting || iParent.iState == OMX_StatePause)
		{
		Cancel();
		iParent.iState = OMX_StateIdle;
		}
	}

COmxILFileSourceProcessingFunction::CPFHelper* COmxILFileSourceProcessingFunction::CPFHelper::NewL(COmxILFileSourceProcessingFunction& aParent, CFileSourceAccess& aFileSourceAccess, CBufferArrivalHandler& aBufferArrivalHandler)
	{
	CPFHelper* self = new (ELeave) CPFHelper(aParent, aFileSourceAccess, aBufferArrivalHandler);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

COmxILFileSourceProcessingFunction::CPFHelper::CPFHelper(COmxILFileSourceProcessingFunction& aParent, CFileSourceAccess& aFileSourceAccess, CBufferArrivalHandler& aBufferArrivalHandler)
: CActive(CActive::EPriorityUserInput),
  iParent(aParent),
  iFileSourceAccess(aFileSourceAccess),
  iBufferArrivalHandler(aBufferArrivalHandler)
	{
	CActiveScheduler::Add(this);
    iRunLContext = RThread();
	}

void COmxILFileSourceProcessingFunction::CPFHelper::ConstructL()
	{
    User::LeaveIfError(iWaitSemaphore.CreateLocal(0));
	User::LeaveIfError(iMsgQueue.CreateLocal(KMaxMsgQueueEntries));
	SetActive();
	iMsgQueue.NotifyDataAvailable(iStatus);
	}

COmxILFileSourceProcessingFunction::CPFHelper::~CPFHelper()
	{
	Cancel();
	iMsgQueue.Close();
    iRunLContext.Close();
    iWaitSemaphore.Close();
	}

void COmxILFileSourceProcessingFunction::CPFHelper::RunL()
	{
	TInt err = ProcessQueue();
	if (err != KErrNone)
		{
		iParent.GetCallbacks().ErrorEventNotification( ConvertSymbianErrorType(err) );
		}

	// setup for next callbacks
	SetActive();
	iMsgQueue.NotifyDataAvailable(iStatus);
	}

void COmxILFileSourceProcessingFunction::CPFHelper::DoCancel()
	{
	if (iMsgQueue.Handle())
		{
		ProcessQueue();
		iMsgQueue.CancelDataAvailable();
		}
	}

TInt COmxILFileSourceProcessingFunction::CPFHelper::ProcessQueue()
	{
	TProcMessage msg;
	TInt err = KErrNone;

	while (iMsgQueue.Receive(msg) == KErrNone)
		{
		switch (msg.iType)
			{
			case EExecuteCommand:
				{
                const HBufC* fileName = iParent.FileName();
	            if (fileName && iFileSourceAccess.iFileHandle.SubSessionHandle() == 0)
                    {
                    err = iFileSourceAccess.iFileHandle.Open(iFileSourceAccess.iFs, *fileName, EFileRead | EFileShareReadersOnly);
                    }
	            // Transition to execute only when opening the file succeeds or there is no file name at all
                if ( err == KErrNone)
                    {
                    err = iFileSourceAccess.Execute();
                    }
				break;
				}

			case EStopCommand:
				{
				iFileSourceAccess.Stop();
				break;
				}

			case EPauseCommand:
				{
				iFileSourceAccess.Pause();
				break;
				}

			case EIdleCommand:
			    {
			    iFileSourceAccess.Idle();
			    break;
			    }
			    
			case EFlushCommand:
			    {
			    // Carry out the flush and wake up the non-Active Scheduler client.
			    PerformFlush();
			    iWaitSemaphore.Signal();
			    break;
			    }
			    
			case ERemoveBufferCommand:
			    {
			    // Carry out the buffer removal and wake up the non-Active Scheduler client.
			    TBufferRemovalArgs* args = reinterpret_cast<TBufferRemovalArgs*>(msg.iPtr);
			    *(args->iHdrRemoved) = PerformBufferRemoval(args->ipHeader);
			    iWaitSemaphore.Signal();
			    break;
			    }
			    
			default:
				{
				break;
				}
			}

		if (err)
			{
			break;
			}
		}
	return err;
	}

TInt COmxILFileSourceProcessingFunction::CPFHelper::ExecuteAsync()
	{
	TProcMessage message;
	message.iType = EExecuteCommand;
	return iMsgQueue.Send(message);
	}

TInt COmxILFileSourceProcessingFunction::CPFHelper::StopAsync()
	{
	TProcMessage message;
	message.iType = EStopCommand;
	return iMsgQueue.Send(message);
	}

TInt COmxILFileSourceProcessingFunction::CPFHelper::PauseAsync()
    {
    TProcMessage message;
    message.iType = EPauseCommand;
    return iMsgQueue.Send(message);
    }

void COmxILFileSourceProcessingFunction::CPFHelper::StopSync()
    {
    // Cancel to process the existing queue before handling this command
    Cancel();
    iFileSourceAccess.Stop();

    // setup for next callbacks
    SetActive();
    iMsgQueue.NotifyDataAvailable(iStatus);
    }

TInt COmxILFileSourceProcessingFunction::CPFHelper::IdleAsync()
    {
    TProcMessage message;
    message.iType = EIdleCommand;
    return iMsgQueue.Send(message);
    }

TInt COmxILFileSourceProcessingFunction::CPFHelper::FlushIndication()
    {
    TInt error = KErrNone;    
    RThread currentThread = RThread();
    if (currentThread.Id() == iRunLContext.Id())
        {
        // Running in the same context as CFileSourceAccess::RunL() so 
        // sure of no concurrency issues.
        PerformFlush();
        }
    else
        {
        // Running in a different thread, meaning the client does
        // not have an active scheduler.  To avoid concurrency problems 
        // during the flush, switch to the thread in which the active objects'
        // execution is handled.
        TProcMessage message;
        message.iType = EFlushCommand;
        error =  iMsgQueue.Send(message);
        if (error == KErrNone)
            {
            iWaitSemaphore.Wait();
            }
        }
    currentThread.Close();
    return error;
    }

void COmxILFileSourceProcessingFunction::CPFHelper::PerformFlush()
    {
    // Running in the same context as CFileSourceAccess::RunL() so 
    // sure of no concurrency issues.
    
    // So that we don't continue to write to the buffer once it's returned.
    iFileSourceAccess.CancelDataTransfer();
    
    while (iParent.iBuffersToFill.Count() > 0)
        {
        OMX_BUFFERHEADERTYPE* pBufferHeader = iParent.iBuffersToFill[0];
        iParent.iBuffersToFill.Remove(0);
        iParent.iCallbacks.BufferDoneNotification(pBufferHeader,
                                                pBufferHeader->nOutputPortIndex,
                                                OMX_DirOutput);
        }
    
    iBufferArrivalHandler.FlushIndication();    
    }

OMX_BOOL COmxILFileSourceProcessingFunction::CPFHelper::RemoveBufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader)
    {
    TInt error = KErrNone;    
    RThread currentThread = RThread();
    if (currentThread.Id() == iRunLContext.Id())
        {
        currentThread.Close();
        // Running in the same context as CFileSourceAccess::RunL() so 
        // sure of no concurrency issues.
        return PerformBufferRemoval(apBufferHeader);
        }
    else
        {
        currentThread.Close();
        // Running in a different thread, meaning the client does
        // not have an active scheduler.  To avoid concurrency problems 
        // during the flush, switch to the thread in which the active objects'
        // execution is handled.
        TProcMessage message;
        OMX_BOOL bufferFound;
        TBufferRemovalArgs messageArgs;
        messageArgs.ipHeader = apBufferHeader;
        messageArgs.iHdrRemoved = &bufferFound;
        message.iType = ERemoveBufferCommand;
        message.iPtr = &messageArgs;
        error =  iMsgQueue.Send(message);
        if (error == KErrNone)
            {
            iWaitSemaphore.Wait();
            return bufferFound;
            }
        iParent.GetCallbacks().ErrorEventNotification(OMX_ErrorInsufficientResources);
        return OMX_FALSE;
        }
    }

OMX_BOOL COmxILFileSourceProcessingFunction::CPFHelper::PerformBufferRemoval(OMX_BUFFERHEADERTYPE* apBufferHeader)
    {
    if (iParent.iBuffersToFill.Count() < 1)
        {
        return OMX_FALSE;
        }
    
    if (iParent.iBuffersToFill[0] == apBufferHeader)
        {
        iFileSourceAccess.CancelDataTransfer();
        iParent.iBuffersToFill.Remove(0);
        return OMX_TRUE;
        }
    
    TInt count = iParent.iBuffersToFill.Count();
    for (TInt index = 1; index < count; ++index)
        {
        if (iParent.iBuffersToFill[index] == apBufferHeader)
            {
            iParent.iBuffersToFill.Remove(index);
            return OMX_TRUE;
            }
        }
    
    return iBufferArrivalHandler.RemoveBufferIndication(apBufferHeader);
    }

/**
 Converts a Symbian error code to an OpenMAX error code.
 @param     aError The Symbian error code.
 @return    The OpenMAX error code.
 */
OMX_ERRORTYPE COmxILFileSourceProcessingFunction::CPFHelper::ConvertSymbianErrorType(TInt aError)
    {
    // In the current implementation this function is only used for the return code in the
    // callback methods. Currently the only expected errors KErrNone and KErrOverflow.

    OMX_ERRORTYPE err = OMX_ErrorNone;
    switch (aError)
        {
    case KErrNone:
        err = OMX_ErrorNone;
        break;
    case KErrOverflow:
    case KErrNoMemory:
        err = OMX_ErrorInsufficientResources;
        break;
    case KErrNotSupported:
        err = OMX_ErrorNotImplemented;
        break;
    case KErrNotReady:
        err = OMX_ErrorNotReady;
        break;
    case KErrGeneral:
    default:
        err = OMX_ErrorUndefined;
        }
    return err;
    }

COmxILFileSourceProcessingFunction::CBufferArrivalHandler* COmxILFileSourceProcessingFunction::CBufferArrivalHandler::NewL(COmxILFileSourceProcessingFunction& aParent, CFileSourceAccess& aFileSourceAccess)
    {
    CBufferArrivalHandler* self = new (ELeave) CBufferArrivalHandler(aParent, aFileSourceAccess);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

COmxILFileSourceProcessingFunction::CBufferArrivalHandler::CBufferArrivalHandler(COmxILFileSourceProcessingFunction& aParent, CFileSourceAccess& aFileSourceAccess)
: CActive(CActive::EPriorityStandard),
  iParent(aParent),
  iFileSourceAccess(aFileSourceAccess)
    { 
    CActiveScheduler::Add(this);
    }

void COmxILFileSourceProcessingFunction::CBufferArrivalHandler::ConstructL()
    {
    User::LeaveIfError(iMsgQueue.CreateLocal(KMaxMsgQueueEntries));
    SetActive();
    iMsgQueue.NotifyDataAvailable(iStatus);
    }

COmxILFileSourceProcessingFunction::CBufferArrivalHandler::~CBufferArrivalHandler()
    {
    Cancel();
    iMsgQueue.Close();  // Queue should be emptied by now
    }
        
TInt COmxILFileSourceProcessingFunction::CBufferArrivalHandler::BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader)
    {
    TBufferIndicationMessage msg;
    msg.iBufferHdr = apBufferHeader;
    return iMsgQueue.Send(msg);    
    }

/*
 * Bear in mind that this results in BufferDoneNotifications and so should be called at the end of of the 
 * flush sequence to ensure headers are completed in the order they were received.
 */
void COmxILFileSourceProcessingFunction::CBufferArrivalHandler::FlushIndication()
    {
    TBufferIndicationMessage msg;
    
    while (iMsgQueue.Receive(msg) == KErrNone)
        {
        iParent.GetCallbacks().BufferDoneNotification(msg.iBufferHdr, msg.iBufferHdr->nOutputPortIndex,OMX_DirOutput);
        }
    }

OMX_BOOL COmxILFileSourceProcessingFunction::CBufferArrivalHandler::RemoveBufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader)
    {
    RArray<TBufferIndicationMessage> bufferedMsgs;
    TBufferIndicationMessage msg;
    
    OMX_BOOL hdrFound = OMX_FALSE;
    while (iMsgQueue.Receive(msg) == KErrNone)
        {
        if (msg.iBufferHdr == apBufferHeader)
            {
            hdrFound = OMX_TRUE;
            }
        else
            {
            TInt error = bufferedMsgs.Append(msg);
            if (error != KErrNone)
                {
                // to prevent potential buffer leakage if the Append operation fails
                iParent.GetCallbacks().BufferDoneNotification(msg.iBufferHdr, msg.iBufferHdr->nOutputPortIndex,OMX_DirOutput);
                }
            }
        }
    
    TInt count = bufferedMsgs.Count();
    for (TInt index = 0; index < count; ++index)
        {
        // There's the potential here, in an environment when 3+ threads
        // (which is possible in the case of a non-AS client and a multi-threaded
        // tunneled component, e.g. Bellagio) are manipulatng the component, for
        // a buffer to be inserted out of order if a FillThisBuffer() command comes
        // in during the move to Idle and with particular thread scheduling 
        // sequences.  However as we only have one port and this command is called as
        // part of a move to Idle or port disablement then we should get a 
        // BufferRemovalNotification() for every buffer so in this instance it shouldn't
        // be a problem.
        iMsgQueue.Send(bufferedMsgs[index]);
        }
    return hdrFound;
    }

void COmxILFileSourceProcessingFunction::CBufferArrivalHandler::RunL()
    {
    TBufferIndicationMessage msg;
    TInt err = iMsgQueue.Receive(msg);
    
    if ((err == KErrNone) && (iParent.iState == OMX_StateExecuting ||
                                iParent.iState == OMX_StatePause || 
                                iParent.iState == OMX_StateIdle))
        {
        err = iParent.iBuffersToFill.Append(msg.iBufferHdr);
        
        if(err == KErrNone)
            {
            if(iParent.iState != OMX_StateIdle)
                {
                err = iFileSourceAccess.ProcessNextBuffer();
                }
            }
        else
            {
            iParent.GetCallbacks().ErrorEventNotification(OMX_ErrorInsufficientResources);
            // to prevent potential buffer leakage if the Append operation fails
            iParent.GetCallbacks().BufferDoneNotification(msg.iBufferHdr, msg.iBufferHdr->nOutputPortIndex,OMX_DirOutput);
            }        
        }
            
    SetActive();
    iMsgQueue.NotifyDataAvailable(iStatus);  
    }

void COmxILFileSourceProcessingFunction::CBufferArrivalHandler::DoCancel()
    {
    iMsgQueue.CancelDataAvailable();
    }
