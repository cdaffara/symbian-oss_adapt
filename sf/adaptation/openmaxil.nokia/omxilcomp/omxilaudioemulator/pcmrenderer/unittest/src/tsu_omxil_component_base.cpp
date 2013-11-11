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

#include <ecom/ecom.h>
#include <openmax/il/khronos/v1_x/OMX_Component.h>
#include <openmax/il/loader/omxilcomponentif.h>

#include "log.h"
#include "tsu_omxil_component_base.h"


const TInt CCallbackHandler::KMaxMsgQueueEntries;


CAacTestFile*
CAacTestFile::NewL()
	{
    DEBUG_PRINTF(_L8("CAacTestFile::NewL"));
	CAacTestFile* self = new (ELeave) CAacTestFile();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void
CAacTestFile::ConstructL()
	{
    DEBUG_PRINTF(_L8("CAacTestFile::ConstructL"));
	}

CAacTestFile::CAacTestFile() :
	iSourceFile(0),
	iSourceFileReadPos(0),
	iFinished(EFalse)
	{
    DEBUG_PRINTF(_L8("CAacTestFile::CAacTestFile"));
	}

CAacTestFile::~CAacTestFile()
	{
    DEBUG_PRINTF(_L8("CAacTestFile::~CAacTestFile"));

	delete iSourceFile;

	}

TInt
CAacTestFile::ReadTestFileInBuffer(const TDesC& aFileName)
	{
    DEBUG_PRINTF(_L8("CAacTestFile::ReadTestFileInBuffer"));

	RFs fs;
	TInt err;
	err = fs.Connect();
	
	if (err != KErrNone)
		{
		return err;
		}
	
	RFile file;
	err = file.Open(fs, aFileName, EFileRead);
	
	if (err == KErrNone)
		{
		TInt size;
		err = file.Size(size);
		if (err == KErrNone)
			{
			iSourceFile = HBufC8::NewMax(size);
			if(!iSourceFile)
				{
				return KErrNoMemory;
				}
			TPtr8 ptr = iSourceFile->Des();
			file.Read(ptr,size);
			file.Close();
			}
		fs.Close();
		}

    return err;

	}


void
CAacTestFile::ReadNextBuffer(TDes8& aDataBuffer)
    {
    DEBUG_PRINTF(_L8("CAacTestFile::ReadNextBuffer"));

    while (ETrue)
		{
		// check if finished
		if (iFinished)
			{
			DEBUG_PRINTF(_L8("CAacTestFile::ReadNextBuffer : File finished"));
			aDataBuffer.Zero();
			return;
			}

		TInt srcLength = iSourceFile->Size();
		if (iSourceFileReadPos < srcLength)
			{
			TInt size = srcLength;
			if (size > aDataBuffer.MaxLength())
				{
				size = aDataBuffer.MaxLength();
				}
			Mem::Copy((TAny*)aDataBuffer.Ptr(), (TAny*)iSourceFile->Mid(iSourceFileReadPos).Ptr(), size);
			aDataBuffer.SetLength(size);
			iSourceFileReadPos += size;
			DEBUG_PRINTF2(_L8("CAacTestFile::ReadNextBuffer : data read = [%d] bytes"), iSourceFileReadPos);
			DEBUG_PRINTF2(_L8("CAacTestFile::ReadNextBuffer : aDataBuffer Size = [%u] bytes"), aDataBuffer.Length());
			if (iSourceFileReadPos >= srcLength)
				{
				DEBUG_PRINTF(_L8("CAacTestFile::ReadNextBuffer : end of data"));
				iFinished = ETrue;
				}
			return;

			}
		else
			{
			// no more data
			DEBUG_PRINTF(_L8("CAacTestFile::ReadNextBuffer : end of data"));
			iFinished = ETrue;
			return;
			}
		}
    }

void
CAacTestFile::ResetPos()
	{
	iSourceFileReadPos = 0;
	iFinished = EFalse;
	}

TInt
CAacTestFile::GetPos()
	{
	return iSourceFileReadPos;
	}


CAacOutputTestFile*
CAacOutputTestFile::NewL()
	{
    DEBUG_PRINTF(_L8("CAacOutputTestFile::NewL"));
	CAacOutputTestFile* self = new (ELeave) CAacOutputTestFile();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void
CAacOutputTestFile::ConstructL()
	{
    DEBUG_PRINTF(_L8("CAacOutputTestFile::ConstructL"));
	}

CAacOutputTestFile::CAacOutputTestFile() :
	iFileServerSession(),
	iOutputFile(),
	iWrittenDataTotal(0),
	iBuffersWrittenCount(0)
	{
    DEBUG_PRINTF(_L8("CAacOutputTestFile::CAacOutputTestFile"));
	}

CAacOutputTestFile::~CAacOutputTestFile()
	{
    DEBUG_PRINTF(_L8("CAacOutputTestFile::~CAacOutputTestFile"));
	}

TInt
CAacOutputTestFile::SetUpOutputFile(const TDesC& aFileName)
	{
    DEBUG_PRINTF(_L8("CAacOutputTestFile::SetUpOutputFile"));

	TInt err;
	
	err = iFileServerSession.Connect();

	if (err != KErrNone)
		{
		return err;
		}
		
	err = iOutputFile.Create(iFileServerSession,
 			   					  aFileName,
 			   					  EFileWrite|EFileShareExclusive);
	if(err != KErrNone)
		{
		err = iOutputFile.Replace(iFileServerSession,
							aFileName,
							EFileWrite|EFileShareExclusive);
		}
	return err;

	}

TInt
CAacOutputTestFile::WriteDataToFile(const TDesC8& aDataBuffer)
	{
    DEBUG_PRINTF(_L8("CAacOutputTestFile::WriteDataToFile"));


	TInt err = KErrNone;
	err = iOutputFile.Write(aDataBuffer);
	if(err != KErrNone)
		{
		return err;
		}
	// keep record of amount of data and the number of buffers written out
	iWrittenDataTotal += aDataBuffer.Size();
	iBuffersWrittenCount++;

    DEBUG_PRINTF2(_L8("CAacOutputTestFile::WriteDataToFile : aDataBuffer.Size()[%d]"), aDataBuffer.Size());
    DEBUG_PRINTF2(_L8("CAacOutputTestFile::WriteDataToFile : iWrittenDataTotal[%d]"), iWrittenDataTotal);
	DEBUG_PRINTF2(_L8("CAacOutputTestFile::WriteDataToFile : iBuffersWrittenCount[%d]"), iBuffersWrittenCount);

	return err;

	}


TInt
CAacOutputTestFile::AddWavHeader()
	{
    DEBUG_PRINTF(_L8("CAacOutputTestFile::AddWavHeader"));

	TInt err;
	RFs fs;
	err = fs.Connect();
	
	if (err != KErrNone)
		{
		return err;
		}
		
	RFile file;
	err = file.Open(fs, KAacDecoderOutputTestFile, EFileRead);
	if (err != KErrNone)
		{
		return err;
		}

	TInt size;
	err = file.Size(size);
	if (err != KErrNone)
		{
		return err;
		}

	HBufC8* rawDecFile = HBufC8::NewMax(size);
	if(!rawDecFile)
		{
		return KErrNoMemory;
		}

	TPtr8 ptr = rawDecFile->Des();
	file.Read(ptr,size);
	file.Close();

	// add headers
	err = file.Replace(fs, KAacDecoderOutputTestFile, EFileWrite);
	if (err != KErrNone)
		{
		return err;
		}

	// this is canonical WAV file format header
	TInt32 chunkSize = size + KTestWavFormatPCMChunkHeaderSize;
	TInt32 subchunk1size = KTestWavFormatPCMSubchunk1Size;
	TInt16 audioFormat = KTestAudioFormatPCM;
	TInt16 numChannels = KTestNumChannels;
	TInt32 sampleRate = KTestSampleRate;
	TInt16 bitsPerSample = KTestBitsPerSample;
	TInt32 byteRate = sampleRate * numChannels * (bitsPerSample / 8);
	TInt16 blockAlign = numChannels * (bitsPerSample / 8);
	TInt32 subchunk2size = size;

	file.Write(_L8("RIFF"));
	{ TPtrC8 buf((TText8*)&chunkSize,sizeof(TInt32)); file.Write(buf); }
	file.Write(_L8("WAVEfmt "));
	{ TPtrC8 buf((TText8*)&subchunk1size,sizeof(TInt32)); file.Write(buf); }
	{ TPtrC8 buf((TText8*)&audioFormat,sizeof(TInt16)); file.Write(buf); }
	{ TPtrC8 buf((TText8*)&numChannels,sizeof(TInt16)); file.Write(buf); }
	{ TPtrC8 buf((TText8*)&sampleRate,sizeof(TInt32)); file.Write(buf); }
	{ TPtrC8 buf((TText8*)&byteRate,sizeof(TInt32)); file.Write(buf); }
	{ TPtrC8 buf((TText8*)&blockAlign,sizeof(TInt16)); file.Write(buf); }
	{ TPtrC8 buf((TText8*)&bitsPerSample,sizeof(TInt16)); file.Write(buf); }
	file.Write(_L8("data"));
	{ TPtrC8 buf((TText8*)&subchunk2size,sizeof(TInt32)); file.Write(buf); }
	file.Write(ptr,size);

	// store file size
	file.Size(iWrittenDataTotal);

	file.Close();
	fs.Close();

	delete rawDecFile;

	return KErrNone;

	}

void
CAacOutputTestFile::CloseOutputFile()
	{
    DEBUG_PRINTF(_L8("CAacOutputTestFile::CloseOutputFile"));

	iOutputFile.Close();
	iFileServerSession.Close();

	}

//
// CUtilityTimer
//
CUtilityTimer*
CUtilityTimer::NewL(TTimeIntervalMicroSeconds32& aDelay,
					MTimerObserver& aObserver)
    {
    CUtilityTimer* self = new (ELeave) CUtilityTimer(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL(aDelay);
    CleanupStack::Pop(self);
    return self;
    }

void
CUtilityTimer::ConstructL(TTimeIntervalMicroSeconds32& aDelay)
    {
    CTimer::ConstructL();

    iDelay = aDelay;
    CActiveScheduler::Add(this);
    }

CUtilityTimer::~CUtilityTimer()
    {
    Cancel();
    }

void
CUtilityTimer::InitializeTimer()
    {
	// Request another wait - assume not active
	CTimer::After(iDelay);
    }

void
CUtilityTimer::RunL()
    {
	if (iStatus.Int() == KErrNone)
		iObserver.TimerExpired();
    }

void
CUtilityTimer::DoCancel()
	{
	}


CUtilityTimer::CUtilityTimer(MTimerObserver& aObserver) :
	CTimer(CActive::EPriorityUserInput),
    iObserver(aObserver)
	{
    }

//
// CCallbackHandler
//
CCallbackHandler*
CCallbackHandler::NewL(RAsyncTestStepOmxILComponentBase& aDecoderTest)
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::NewL"));

	CCallbackHandler* self = new (ELeave) CCallbackHandler(aDecoderTest);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;

	}


void
CCallbackHandler::ConstructL()
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::ConstructL"));

	OMX_CALLBACKTYPE h =
		{
		CCallbackHandler::EventHandler,
		CCallbackHandler::EmptyBufferDone,
		CCallbackHandler::FillBufferDone
		};

	iHandle = h;
	CActiveScheduler::Add(this);

	User::LeaveIfError(iMsgQueue.CreateLocal(KMaxMsgQueueEntries));
	iMsgQueue.NotifyDataAvailable(iStatus);
	SetActive();

	}

CCallbackHandler::CCallbackHandler(RAsyncTestStepOmxILComponentBase& aDecoderTest)
	: CActive(EPriorityNormal),
	  iDecoderTest(aDecoderTest)
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::CCallbackHandler"));
	}


CCallbackHandler::operator OMX_CALLBACKTYPE*()
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::operator OMX_CALLBACKTYPE*"));

	return &iHandle;

	}


void
CCallbackHandler::RunL()
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::RunL"));

	TOmxMessage msg;
	while (iMsgQueue.Receive(msg)==KErrNone)
		{
		switch (msg.iType)
			{
		case EEmptyBufferCallback:
			{
			iDecoderTest.DoEmptyBufferDoneL(msg.iComponent,
											msg.iBuffer);
			}
			break;
		case EFillBufferCallback:
			{
			iDecoderTest.DoFillBufferDoneL(msg.iComponent,
										   msg.iBuffer);
			}
			break;
		case EEventCallback:
			{
			iDecoderTest.DoEventHandlerL(msg.iComponent,
										 msg.iEventParams.iEvent,
										 msg.iEventParams.iData1,
										 msg.iEventParams.iData2,
										 msg.iEventParams.iExtra);
			}
			break;
		default:
			{
			// This is an invalid state
			ASSERT(EFalse);
			}
			};
		}

	// setup for next callbacks
	iStatus = KRequestPending;
	iMsgQueue.NotifyDataAvailable(iStatus);
	SetActive();

	}

CCallbackHandler::~CCallbackHandler()
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::~CCallbackHandler"));

	Cancel();
	iMsgQueue.Close();

	}


void
CCallbackHandler::DoCancel()
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::DoCancel"));

	if (iMsgQueue.Handle())
		{
		iMsgQueue.CancelDataAvailable();
		}

	}

OMX_ERRORTYPE
CCallbackHandler::FillBufferDone(OMX_HANDLETYPE aComponent,
								 TAny* aAppData,
								 OMX_BUFFERHEADERTYPE* aBuffer)
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::FillBufferDone"));

	return static_cast<CCallbackHandler*>(aAppData)->DoFillBufferDone(aComponent, aBuffer);

	}

OMX_ERRORTYPE
CCallbackHandler::EmptyBufferDone(OMX_HANDLETYPE aComponent,
								  TAny* aAppData,
								  OMX_BUFFERHEADERTYPE* aBuffer)
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::EmptyBufferDone"));

	return static_cast<CCallbackHandler*>(aAppData)->DoEmptyBufferDone(aComponent, aBuffer);

	}

OMX_ERRORTYPE
CCallbackHandler::EventHandler(OMX_HANDLETYPE aComponent,
							   TAny* aAppData,
							   OMX_EVENTTYPE aEvent,
							   TUint32 aData1,
							   TUint32 aData2,
							   TAny* aExtra)
	{
    DEBUG_PRINTF4(_L8("CCallbackHandler::EventHandler : EVENT[%d] Data1[%d] Data2[%d]"), aExtra, aData1, aData2);

	CCallbackHandler::TEventParams eventParams;
	eventParams.iEvent = aEvent;
	eventParams.iData1 = aData1;
	eventParams.iData2 = aData2;
	eventParams.iExtra = aExtra;
	return static_cast<CCallbackHandler*>(aAppData)->DoEventHandler(aComponent, eventParams);

	}

OMX_ERRORTYPE
CCallbackHandler::DoFillBufferDone(OMX_HANDLETYPE aComponent,
								   OMX_BUFFERHEADERTYPE* aBuffer)
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::DoFillBufferDone"));

	TOmxMessage message;
	message.iType = EFillBufferCallback;
	message.iComponent = aComponent;
	message.iBuffer = aBuffer;
	return RAsyncTestStepOmxILComponentBase::ConvertSymbianErrorType(
		iMsgQueue.Send(message));

	}

OMX_ERRORTYPE
CCallbackHandler::DoEmptyBufferDone(OMX_HANDLETYPE aComponent,
									OMX_BUFFERHEADERTYPE* aBuffer)
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::DoEmptyBufferDone"));

	TOmxMessage message;
	message.iType = EEmptyBufferCallback;
	message.iComponent = aComponent;
	message.iBuffer = aBuffer;
	return RAsyncTestStepOmxILComponentBase::ConvertSymbianErrorType(
		iMsgQueue.Send(message));

	}

OMX_ERRORTYPE
CCallbackHandler::DoEventHandler(OMX_HANDLETYPE aComponent,
								 TEventParams aEventParams)
	{
    DEBUG_PRINTF(_L8("CCallbackHandler::DoEventHandler"));

	TOmxMessage message;
	message.iType = EEventCallback;
	message.iComponent = aComponent;
	message.iEventParams = aEventParams;
	return RAsyncTestStepOmxILComponentBase::ConvertSymbianErrorType(
		iMsgQueue.Send(message));

	}

//
// RAsyncTestStepOmxILComponentBase
//
RAsyncTestStepOmxILComponentBase::RAsyncTestStepOmxILComponentBase(const TDesC& aTestName, TInt aComponentUid)
	:
	iComponentUid(),
	ipKickoffAOp(0),
	ipKickoffStop(0),
	ipCOmxILComponent(0),
	ipCallbackHandler(0),
	ipTestFile(0),
	iTestFileName(KAacDecoderTestFile()),
	ipCompHandle(0)
	{
    DEBUG_PRINTF2(_L8("RAsyncTestStepOmxILComponentBase::RAsyncTestStepOmxILComponentBase: UID[%X]"), aComponentUid);
	iTestStepName = aTestName;
	iComponentUid = aComponentUid;
	// Default heap size is 32K. Increased to avoid the KErrNoMemory for this test step.
	iHeapSize = KTestHeapSize;

	}

RAsyncTestStepOmxILComponentBase::~RAsyncTestStepOmxILComponentBase()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::~RAsyncTestStepOmxILComponentBase"));
	// nothing here just yet
	}

void
RAsyncTestStepOmxILComponentBase::PrintOmxState(OMX_STATETYPE aOmxState)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::PrintOmxState"));

	switch(aOmxState)
		{
    case OMX_StateInvalid:
		{
		INFO_PRINTF1(_L("OMX STATE : OMX_StateInvalid"));
		}
		break;
    case OMX_StateLoaded:
		{
		INFO_PRINTF1(_L("OMX STATE : OMX_StateLoaded"));
		}
		break;
    case OMX_StateIdle:
		{
		INFO_PRINTF1(_L("OMX STATE : OMX_StateIdle"));
		}
		break;
    case OMX_StateExecuting:
		{
		INFO_PRINTF1(_L("OMX STATE : OMX_StateExecuting"));
		}
		break;
    case OMX_StatePause:
		{
		INFO_PRINTF1(_L("OMX STATE : OMX_StatePause"));
		}
		break;
    case OMX_StateWaitForResources:
		{
		INFO_PRINTF1(_L("OMX STATE : OMX_StateWaitForResources"));
		}
		break;
	default:
		{
		INFO_PRINTF1(_L("OMX STATE : Wrong state found"));
		}
		};

	}

OMX_ERRORTYPE
RAsyncTestStepOmxILComponentBase::ConvertSymbianErrorType(TInt aError)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::ConvertSymbianErrorType"));

	OMX_ERRORTYPE err = OMX_ErrorNone;
	switch (aError)
		{
	case KErrNone:
		err = OMX_ErrorNone;
		break;
	case KErrNoMemory:
		err = OMX_ErrorInsufficientResources;
		break;
	case KErrGeneral:
		break;
	default:
		err = OMX_ErrorUndefined;
		}
	return err;

	}


/**
   This method is used at the beginning of the test, and initialises the
   asynchronous calls that will be activated once the call returns. The
   ActiveScheduler is active at this point.  If this test leaves, then
   StopTest() will be called with the leave value, so implicitly the test
   stops.

*/
void
RAsyncTestStepOmxILComponentBase::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::KickoffTestL"));

	__MM_HEAP_MARK;

	// Set up an asynchronous call
	TCallBack callback (ComponentBaseCallBack, this);
	delete ipKickoffAOp;
	ipKickoffAOp = NULL;
	ipKickoffAOp =
		new (ELeave) CAsyncCallBack (callback, CActive::EPriorityLow);
	// Queues this active object to be run once.
	ipKickoffAOp->Call();

	}

TInt
RAsyncTestStepOmxILComponentBase::ComponentBaseCallBack(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::ComponentBaseCallBack"));

	RAsyncTestStepOmxILComponentBase* self = static_cast<RAsyncTestStepOmxILComponentBase*> (aPtr);
	self->DoComponentBaseCallBack();
	return KErrNone;

	}

void
RAsyncTestStepOmxILComponentBase::DoComponentBaseCallBack()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::DoComponentBaseCallBack"));

	TRAPD(err, ipCOmxILComponent =
		  COmxILComponentIf::CreateImplementationL(TUid::Uid(iComponentUid)));

	//return StopTest(err, EFail);
	if (err != KErrNone)
		{
		INFO_PRINTF2(_L("RAsyncTestStepOmxILComponentBase::DoComponentBaseCallBack : err [%d]"), err);
		if (KErrNotFound == err)
			{
			INFO_PRINTF1(_L("DoComponentBaseCallBack : CreateImplementationL returned KErrNotFound"));
// PacketVideo's AAC decoder libraries only provided for UDEB
#ifndef _DEBUG
			INFO_PRINTF1(_L("IMPORTANT NOTE : THIS SUITE CAN ONLY BE RUN IN UDEB MODE"));
			INFO_PRINTF1(_L("IMPORTANT NOTE : PACKETVIDEO'S AAC DECODER LIBRARY ONLY AVAILABLE IN UDEB MODE"));
#endif
			}
		return StopTest(err, EFail);
		}

	ipCompHandle = static_cast<OMX_COMPONENTTYPE*>(ipCOmxILComponent->Handle());
	if (!ipCompHandle)
		{
		return StopTest(KErrGeneral, EFail);
		}

	TRAP(err, ipCallbackHandler = CCallbackHandler::NewL(*this));
	if (err != KErrNone)
		{
		return StopTest(err, EFail);
		}

	TRAP(err, ipTestFile = CAacTestFile::NewL());
	if (err != KErrNone)
		{
		return StopTest(err, EFail);
		}

	err = ipTestFile->ReadTestFileInBuffer(iTestFileName);
	if (err != KErrNone)
		{
		return StopTest(err, EFail);
		}

	TRAP(err, ipOutputTestFile = CAacOutputTestFile::NewL());
	if (err != KErrNone)
		{
		return StopTest(err, EFail);
		}

	err = ipOutputTestFile->SetUpOutputFile(KAacDecoderOutputTestFile);
	if (err != KErrNone)
		{
		return StopTest(err, EFail);
		}


	}

TInt
RAsyncTestStepOmxILComponentBase::StopTestCallBack(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::StopTestCallBack"));
	RAsyncTestStepOmxILComponentBase* self =
		static_cast<RAsyncTestStepOmxILComponentBase*> (aPtr);
	self->DoStopTestCallBack();
	return KErrNone;
	}

void
RAsyncTestStepOmxILComponentBase::DoStopTestCallBack()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::DoStopTestCallBack"));

	StopTest();

	}


void
RAsyncTestStepOmxILComponentBase::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::CloseTest"));

    if (ipCompHandle)
    	{
    	ipCompHandle->ComponentDeInit(ipCompHandle);
    	}
	delete ipCOmxILComponent;
	ipCOmxILComponent = 0;
	ipCompHandle = 0;

	delete ipKickoffAOp; // no need to Cancel
	ipKickoffAOp = 0;

	delete ipCallbackHandler;
	ipCallbackHandler = 0;

	delete ipTestFile;
	ipTestFile = 0;

	if (ipOutputTestFile)
		{
		ipOutputTestFile->CloseOutputFile();
		// We ignore here this error...
		TInt err = ipOutputTestFile->AddWavHeader();
		delete ipOutputTestFile;
		ipOutputTestFile = 0;
		}

	REComSession::FinalClose();
	__MM_HEAP_MARKEND;

	}

void
RAsyncTestStepOmxILComponentBase::DoFillBufferDoneL(OMX_HANDLETYPE /* aComponent */,
												   OMX_BUFFERHEADERTYPE* /* aBuffer */)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::DoFillBufferDoneL"));
	// By default, no callback expected here...
	return StopTest(KErrGeneral, EFail);
	}

void
RAsyncTestStepOmxILComponentBase::DoEmptyBufferDoneL(OMX_HANDLETYPE /* aComponent */,
													OMX_BUFFERHEADERTYPE* /* aBuffer */)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILComponentBase::DoEmptyBufferDoneL"));
	// By default, no callback expected here...
	return StopTest(KErrGeneral, EFail);
	}
