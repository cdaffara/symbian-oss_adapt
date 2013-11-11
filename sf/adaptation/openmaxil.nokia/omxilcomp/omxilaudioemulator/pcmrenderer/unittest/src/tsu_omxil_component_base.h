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
*/

#ifndef TSU_OMXIL_COMPONENT_BASE_H
#define TSU_OMXIL_COMPONENT_BASE_H

#include <e32msgqueue.h>
#include <testframework.h>
#include <openmax/il/khronos/v1_x/OMX_Core.h>

#include "tsu_omxil_component_constants.h"
#include <openmax/il/common/omxilstatedefs.h>

// Forward declarations
class COmxILComponentIf;
struct OMX_COMPONENTTYPE;
class RAsyncTestStepOmxILComponentBase;


class CAacTestFile : public CBase
	{

public:

	static CAacTestFile* NewL();

	~CAacTestFile();

	TInt ReadTestFileInBuffer(const TDesC& aFileName);
    void ReadNextBuffer(TDes8& aDataBuffer);
    void ResetPos();
    TInt GetPos();

private:

	CAacTestFile();
	void ConstructL();


private:

	HBufC8* iSourceFile;
	TInt iSourceFileReadPos;
	TBool iFinished;

	};


class CAacOutputTestFile : public CBase
	{

public:

	static CAacOutputTestFile* NewL();

	~CAacOutputTestFile();

	TInt SetUpOutputFile(const TDesC& aFileName);
	TInt WriteDataToFile(const TDesC8& aDataBuffer);
	TInt AddWavHeader();
	void CloseOutputFile();

private:

	CAacOutputTestFile();
	void ConstructL();


private:

	RFs iFileServerSession;
	RFile iOutputFile;
	TInt iWrittenDataTotal;
	TInt iBuffersWrittenCount;

	};

class MTimerObserver
	{

public:

	virtual void TimerExpired() = 0;

	};

class CUtilityTimer : public CTimer
	{

public:

	static CUtilityTimer* NewL(TTimeIntervalMicroSeconds32& aDelay,
							   MTimerObserver& aObserver);
	~CUtilityTimer();

	void InitializeTimer();

private:

    CUtilityTimer(MTimerObserver& aObserver);
    void ConstructL(TTimeIntervalMicroSeconds32& aDelay);

	// from CActive
    void RunL();
	void DoCancel();

private:

    TTimeIntervalMicroSeconds32 iDelay;
	MTimerObserver& iObserver;

    };


class CCallbackHandler : public CActive
	{
public:

	enum TMessageType
		{
		EFillBufferCallback,
		EEmptyBufferCallback,
		EEventCallback
		};

	class TEventParams
		{
	public:
		OMX_EVENTTYPE iEvent;
		TUint iData1;
		TUint iData2;
		TAny* iExtra;
		};

	class TOmxMessage
		{
	public:
		TMessageType iType;
		OMX_HANDLETYPE iComponent;
		union
			{
			OMX_BUFFERHEADERTYPE* iBuffer;
			TEventParams iEventParams;
			};
		};


	static const TInt KMaxMsgQueueEntries = 10;

public:

	static CCallbackHandler* NewL(RAsyncTestStepOmxILComponentBase& aDecoderTest);
	virtual ~CCallbackHandler();

	operator OMX_CALLBACKTYPE*();

	void RunL();
	void DoCancel();

	static OMX_ERRORTYPE FillBufferDone(OMX_HANDLETYPE aComponent,
										TAny* aAppData,
										OMX_BUFFERHEADERTYPE* aBuffer);

	static OMX_ERRORTYPE EmptyBufferDone(OMX_HANDLETYPE aComponent,
										 TAny* aAppData,
										 OMX_BUFFERHEADERTYPE* aBuffer);

	static OMX_ERRORTYPE EventHandler(OMX_HANDLETYPE aComponent,
									  TAny* aAppData,
									  OMX_EVENTTYPE aEvent,
									  TUint32 aData1,
									  TUint32 aData2,
									  TAny* aExtra);


private:

	void ConstructL();
	CCallbackHandler(RAsyncTestStepOmxILComponentBase& aDecoderTest);

	OMX_ERRORTYPE DoFillBufferDone(OMX_HANDLETYPE aComponent,
								   OMX_BUFFERHEADERTYPE* aBuffer);
	OMX_ERRORTYPE DoEmptyBufferDone(OMX_HANDLETYPE aComponent,
									OMX_BUFFERHEADERTYPE* aBuffer);
	OMX_ERRORTYPE DoEventHandler(OMX_HANDLETYPE aComponent,
								 TEventParams aParams);

	//
	//
	//


private:

	RAsyncTestStepOmxILComponentBase& iDecoderTest;
	RMsgQueue<TOmxMessage> iMsgQueue;
	OMX_CALLBACKTYPE iHandle;

	};


class RAsyncTestStepOmxILComponentBase : public RAsyncTestStep
	{

public:

	RAsyncTestStepOmxILComponentBase(const TDesC& aTestName, TInt aComponentUid);

	~RAsyncTestStepOmxILComponentBase();

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();

	//
	virtual void DoFillBufferDoneL(OMX_HANDLETYPE aComponent,
								  OMX_BUFFERHEADERTYPE* aBuffer);

	virtual void DoEmptyBufferDoneL(OMX_HANDLETYPE aComponent,
								   OMX_BUFFERHEADERTYPE* aBuffer);

	virtual void DoEventHandlerL(OMX_HANDLETYPE aComponent,
								OMX_EVENTTYPE aEvent,
								TUint aData1,
								TUint aData2,
								TAny* aExtra) = 0;


	static OMX_ERRORTYPE ConvertSymbianErrorType(TInt aError);



protected:

	void PrintOmxState(OMX_STATETYPE aOmxState);

	// Async Call backs
	static TInt ComponentBaseCallBack(TAny* aPtr);
	void DoComponentBaseCallBack();

	static TInt StopTestCallBack(TAny* aPtr);
	void DoStopTestCallBack();

protected:

	TInt iComponentUid;
	CAsyncCallBack* ipKickoffAOp;
	CAsyncCallBack* ipKickoffStop;
	COmxILComponentIf* ipCOmxILComponent;
	CCallbackHandler* ipCallbackHandler;
	CAacTestFile* ipTestFile;
	TPtrC iTestFileName;
    CAacOutputTestFile* ipOutputTestFile;
	OMX_COMPONENTTYPE* ipCompHandle;
	};

#endif // TSU_OMXIL_COMPONENT_BASE_H
