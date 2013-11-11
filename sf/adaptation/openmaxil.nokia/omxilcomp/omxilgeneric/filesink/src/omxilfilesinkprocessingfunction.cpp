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
#include "omxilfilesinkprocessingfunction.h"

const TInt KMaxMsgQueueEntries = 25;

COmxILFileSinkProcessingFunction* COmxILFileSinkProcessingFunction::NewL(MOmxILCallbackNotificationIf& aCallbacks)
	{
	COmxILFileSinkProcessingFunction* self = new (ELeave) COmxILFileSinkProcessingFunction(aCallbacks);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

COmxILFileSinkProcessingFunction::COmxILFileSinkProcessingFunction(MOmxILCallbackNotificationIf& aCallbacks)
: COmxILProcessingFunction(aCallbacks)
	{
	}

void COmxILFileSinkProcessingFunction::ConstructL()
	{
	iState = OMX_StateLoaded;
	ipFileSinkAccess = CFileSinkAccess::NewL(*this);
	ipPFHelper = CPFHelper::NewL(*this, *ipFileSinkAccess);
	}

COmxILFileSinkProcessingFunction::~COmxILFileSinkProcessingFunction()
	{
	if(ipPFHelper &&
	   (iState == OMX_StateInvalid  ||
	    iState == OMX_StateExecuting ||
	    iState == OMX_StatePause))
		{
		ipPFHelper->StopSync();
		}

	delete ipPFHelper;
	delete ipFileSinkAccess;
	delete ipUri;
	delete ipFileName;

	// Buffer headers are not owned by the processing function
    iBuffersToEmpty.Close();
	}

OMX_ERRORTYPE COmxILFileSinkProcessingFunction::StateTransitionIndication(TStateIndex aNewState)
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

        case ESubStateLoadedToIdle:
        case ESubStateIdleToLoaded:
		case ESubStatePauseToIdle:
			break;

		default:
			{
			err = OMX_ErrorIncorrectStateTransition;
			}
		};
	return err;
	}

OMX_ERRORTYPE COmxILFileSinkProcessingFunction::BufferFlushingIndication(TUint32 aPortIndex, OMX_DIRTYPE aDirection)
	{
	OMX_ERRORTYPE err = OMX_ErrorNone;
    if ((aPortIndex == OMX_ALL && aDirection == OMX_DirMax) ||
        (aPortIndex == 0 && aDirection == OMX_DirInput))
        {
        // Send BufferDone notifications for each bufer...
        for (TUint i=0, bufferCount=iBuffersToEmpty.Count(); i<bufferCount; ++i)
            {
            OMX_BUFFERHEADERTYPE* pBufferHeader = iBuffersToEmpty[i];
            iCallbacks.BufferDoneNotification(pBufferHeader,
                                              pBufferHeader->nInputPortIndex,
                                              OMX_DirInput);
            }
        // Empty buffer lists...
        iBuffersToEmpty.Reset();
        }
    else
        {
        err = OMX_ErrorBadParameter;
        }
    return err;
	}

OMX_ERRORTYPE COmxILFileSinkProcessingFunction::ParamIndication(OMX_INDEXTYPE aParamIndex, const TAny* apComponentParameterStructure)
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

OMX_ERRORTYPE COmxILFileSinkProcessingFunction::ConfigIndication(OMX_INDEXTYPE /*aConfigIndex*/, const TAny* /*apComponentConfigStructure*/)
	{
	return OMX_ErrorNone;
	}

OMX_ERRORTYPE COmxILFileSinkProcessingFunction::BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE aDirection)
	{
	if (aDirection == OMX_DirInput)
    	{
    	if (ipPFHelper->BufferIndication(apBufferHeader) != KErrNone)
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

OMX_BOOL COmxILFileSinkProcessingFunction::BufferRemovalIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE /*aDirection*/)
	{
	OMX_BOOL headerDeletionResult = OMX_TRUE;

	TInt headerIndexInArray = KErrNotFound;
	if (KErrNotFound != (headerIndexInArray = iBuffersToEmpty.Find(apBufferHeader)))
		{
		iBuffersToEmpty.Remove(headerIndexInArray);
		}
	else
		{
		headerDeletionResult = OMX_FALSE;
		}

    return headerDeletionResult;
	}

MOmxILCallbackNotificationIf& COmxILFileSinkProcessingFunction::GetCallbacks()
	{
	return iCallbacks;
	}

OMX_ERRORTYPE COmxILFileSinkProcessingFunction::SetFileName(const OMX_PARAM_CONTENTURITYPE* aContentUriType)
	{
	ASSERT(aContentUriType);
	delete ipFileName;
	ipFileName = NULL;
	delete ipUri;
	ipUri = NULL;

	//TInt dataLength = aContentUriType->nSize - sizeof(OMX_PARAM_CONTENTURITYPE) + 4;
	TInt sizeOfUri = aContentUriType->nSize - _FOFF(OMX_PARAM_CONTENTURITYPE, contentURI); //Actual size of URI
	if (sizeOfUri <= 0)
		{
		return OMX_ErrorBadParameter;
		}

	// Don't include the zero character at the end.
	//TPtrC8 uriDes(reinterpret_cast<const TUint8*>(&aContentUriType->contentURI), dataLength - 1);
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


const HBufC* COmxILFileSinkProcessingFunction::FileName() const
	{
	return ipFileName;
	}

const HBufC8* COmxILFileSinkProcessingFunction::Uri() const
	{
	return ipUri;
	}

COmxILFileSinkProcessingFunction::CFileSinkAccess* COmxILFileSinkProcessingFunction::CFileSinkAccess::NewL(COmxILFileSinkProcessingFunction& aParent)
	{
	CFileSinkAccess* self = new (ELeave) CFileSinkAccess(aParent);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

COmxILFileSinkProcessingFunction::CFileSinkAccess::CFileSinkAccess(COmxILFileSinkProcessingFunction& aParent)
	: CActive(EPriorityStandard),
	iParent(aParent),
	iBufferOffset(0),
	iWriteBuffer(0,0)
	{
	CActiveScheduler::Add(this);
	}

void COmxILFileSinkProcessingFunction::CFileSinkAccess::ConstructL()
	{
	User::LeaveIfError(iFs.Connect());
	}

COmxILFileSinkProcessingFunction::CFileSinkAccess::~CFileSinkAccess()
	{
	Cancel();

	iFileHandle.Close();
	iFs.Close();
	}

void COmxILFileSinkProcessingFunction::CFileSinkAccess::RunL()
	{

	// The buffer is not on the list implies that they have already been flushed/spotted
	// via BufferFlushingIndication/BufferRemovalIndication
	TInt index = iParent.iBuffersToEmpty.Find(iCurrentBuffer);
	if (KErrNotFound != index)
		{
		switch(iStatus.Int())
			{
			case KErrNone:
				{
				// Consumed all data completely
				if(OMX_BUFFERFLAG_EOS & iCurrentBuffer->nFlags)
					{
					iFileHandle.Close();
					iParent.GetCallbacks().EventNotification(OMX_EventBufferFlag, iCurrentBuffer->nInputPortIndex, OMX_BUFFERFLAG_EOS, NULL);
					}
				iCurrentBuffer->nFilledLen = 0;
				iCurrentBuffer->nOffset = 0;
				iCurrentBuffer->nFlags = 0;
				iCurrentBuffer->nTimeStamp = 0;
				break;
				}
			default:
				{
				// Leave actual value of iCurrentBuffer->nFilledLen
				}
			};

		iParent.GetCallbacks().BufferDoneNotification(iCurrentBuffer,iCurrentBuffer->nInputPortIndex,OMX_DirInput);
		iParent.iBuffersToEmpty.Remove(index);
		iCurrentBuffer = NULL;
		if(iFileHandle.SubSessionHandle() != 0)
			{
			ProcessNextBuffer();
			}
		}
	}

TInt COmxILFileSinkProcessingFunction::CFileSinkAccess::ProcessNextBuffer()
	{
	if ((iParent.iBuffersToEmpty.Count() > 0) && !IsActive() && iParent.iState == OMX_StateExecuting)
		{
		iCurrentBuffer = iParent.iBuffersToEmpty[0];
		iWriteBuffer.Set(iCurrentBuffer->pBuffer, iCurrentBuffer->nFilledLen, iCurrentBuffer->nAllocLen);

		// If the buffer is empty, we should not invoke RFile::Write, but self-complete instead
    	if (iCurrentBuffer->nFilledLen == 0 || iFileHandle.SubSessionHandle() == 0)
    		{
     		SetActive();
			TRequestStatus* status(&iStatus);
			User::RequestComplete(status, KErrNone);
		    }
		else
			{
			iFileHandle.Write(iWriteBuffer, iCurrentBuffer->nFilledLen, iStatus);
			SetActive();
			}
		}
	return KErrNone;
	}

void COmxILFileSinkProcessingFunction::CFileSinkAccess::DoCancel()
	{
	if (iFileHandle.SubSessionHandle() != 0)
	    {
	    iFileHandle.Close();
	    }
	iCurrentBuffer = NULL;
	}

TInt COmxILFileSinkProcessingFunction::CFileSinkAccess::Execute()
	{
	iParent.iState = OMX_StateExecuting;
	return ProcessNextBuffer();
	}

void COmxILFileSinkProcessingFunction::CFileSinkAccess::Pause()
	{
	iParent.iState = OMX_StatePause;
	Cancel();
	}

void COmxILFileSinkProcessingFunction::CFileSinkAccess::Idle()
    {
    iParent.iBuffersToEmpty.Reset();
    iParent.iState = OMX_StateIdle;
    }

void COmxILFileSinkProcessingFunction::CFileSinkAccess::Stop()
	{
	if(iParent.iState == OMX_StateExecuting || iParent.iState == OMX_StatePause)
		{
		Cancel();
		iParent.iState = OMX_StateIdle;
		}
	}

COmxILFileSinkProcessingFunction::CPFHelper* COmxILFileSinkProcessingFunction::CPFHelper::NewL(COmxILFileSinkProcessingFunction& aParent, CFileSinkAccess& aFileSinkAccess)
	{
	CPFHelper* self = new (ELeave) CPFHelper(aParent, aFileSinkAccess);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

COmxILFileSinkProcessingFunction::CPFHelper::CPFHelper(COmxILFileSinkProcessingFunction& aParent, CFileSinkAccess& aFileSinkAccess)
: CActive(EPriorityStandard),
  iParent(aParent),
  iFileSinkAccess(aFileSinkAccess)
	{
	CActiveScheduler::Add(this);
	}

void COmxILFileSinkProcessingFunction::CPFHelper::ConstructL()
	{
	User::LeaveIfError(iMsgQueue.CreateLocal(KMaxMsgQueueEntries));
	SetActive();
	iMsgQueue.NotifyDataAvailable(iStatus);
	}

COmxILFileSinkProcessingFunction::CPFHelper::~CPFHelper()
	{
	Cancel();
	iMsgQueue.Close();
	}

void COmxILFileSinkProcessingFunction::CPFHelper::RunL()
	{
	TInt err = ProcessQueue();
	if (err != KErrNone)
		{
		iParent.GetCallbacks().ErrorEventNotification( ConvertSymbianErrorType(err));
		}

	// setup for next callbacks
	SetActive();
	iMsgQueue.NotifyDataAvailable(iStatus);
	}

void COmxILFileSinkProcessingFunction::CPFHelper::DoCancel()
	{
	if (iMsgQueue.Handle())
		{
		ProcessQueue();
		iMsgQueue.CancelDataAvailable();
		}
	}

TInt COmxILFileSinkProcessingFunction::CPFHelper::ProcessQueue()
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
	            if (fileName)
                    {
                    TUint fileMode = EFileWrite | EFileShareExclusive;
                    if(iFileSinkAccess.iFileHandle.SubSessionHandle() == 0)
                        {
                        err = iFileSinkAccess.iFileHandle.Replace(iFileSinkAccess.iFs, *fileName, fileMode);
                        }
                    }
                if ( err == KErrNone)
                    {
                    err = iFileSinkAccess.Execute();
                    }
				break;
				}

			case EStopCommand:
				{
				iFileSinkAccess.Stop();
				break;
				}

			case EPauseCommand:
				{
				iFileSinkAccess.Pause();
				break;
				}

            case EIdleCommand:
                {
                iFileSinkAccess.Idle();
                break;
                }

			case EBufferIndication:
				{
				OMX_BUFFERHEADERTYPE* bufferHeader = reinterpret_cast<OMX_BUFFERHEADERTYPE*>(msg.iPtr);
				if ( bufferHeader && (iParent.iState == OMX_StateExecuting || iParent.iState == OMX_StatePause
				        || iParent.iState == OMX_StateIdle) )
					{
					err = iParent.iBuffersToEmpty.Append(bufferHeader);
					if(err == KErrNone)
						{
						if(iParent.iState != OMX_StateIdle)
						    {
						    err = iFileSinkAccess.ProcessNextBuffer();
						    }
						}
					else
					    {
					    // to prevent potential buffer leakage if the Append operation fails
					    iParent.GetCallbacks().BufferDoneNotification(bufferHeader, bufferHeader->nInputPortIndex,OMX_DirInput);
					    }
					}
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

TInt COmxILFileSinkProcessingFunction::CPFHelper::ExecuteAsync()
	{
	TProcMessage message;
	message.iType = EExecuteCommand;
	return iMsgQueue.Send(message);
	}

TInt COmxILFileSinkProcessingFunction::CPFHelper::StopAsync()
	{
	TProcMessage message;
	message.iType = EStopCommand;
	return iMsgQueue.Send(message);
	}

TInt COmxILFileSinkProcessingFunction::CPFHelper::PauseAsync()
    {
    TProcMessage message;
    message.iType = EPauseCommand;
    return iMsgQueue.Send(message);
    }

TInt COmxILFileSinkProcessingFunction::CPFHelper::BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader)
	{
	TProcMessage message;
	message.iType = EBufferIndication;
	message.iPtr = apBufferHeader;
	return iMsgQueue.Send(message);
	}

void COmxILFileSinkProcessingFunction::CPFHelper::StopSync()
    {
    // Cancel to process the existing queue before handling this command
    Cancel();
    iFileSinkAccess.Stop();

    // setup for next callbacks
    SetActive();
    iMsgQueue.NotifyDataAvailable(iStatus);
    }

TInt COmxILFileSinkProcessingFunction::CPFHelper::IdleAsync()
    {
    TProcMessage message;
    message.iType = EIdleCommand;
    return iMsgQueue.Send(message);
    }
/**
 Converts a Symbian error code to an OpenMAX error code.
 @param     aError The Symbian error code.
 @return    The OpenMAX error code.
 */
OMX_ERRORTYPE COmxILFileSinkProcessingFunction::CPFHelper::ConvertSymbianErrorType(TInt aError)
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
