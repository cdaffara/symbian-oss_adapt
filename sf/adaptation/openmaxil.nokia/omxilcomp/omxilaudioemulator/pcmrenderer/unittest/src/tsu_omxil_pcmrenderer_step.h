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

#ifndef TSU_OMXIL_PCMRENDERER_STEP_H
#define TSU_OMXIL_PCMRENDERER_STEP_H

#include "tsu_omxil_component_base.h"



//
// RAsyncTestStepOmxILPcmRenderer0020
//
// Test20 Description
//     -# Init Component
//          -# Obtain the component's state (GetState)
//          -# Obtain the component's version (GetComponentVersion)
//          -# Provide callbacks to component (SetCallbacks)
//          -# Obtain component's role #1 (ComponentRoleEnum)
//          -# Obtain component's role #2 (ComponentRoleEnum fails with OMX_ErrorNoMore)
//
class RAsyncTestStepOmxILPcmRenderer0020 : public RAsyncTestStepOmxILComponentBase
	{

public:

	RAsyncTestStepOmxILPcmRenderer0020(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);


protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0020Step01(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0020Step01();

protected:

	CAsyncCallBack* ipKickoff01;
	CAsyncCallBack* ipKickoff02;
	CAsyncCallBack* ipKickoff03;
	CAsyncCallBack* ipKickoff04;
	CAsyncCallBack* ipKickoff05;
	CAsyncCallBack* ipKickoff06;

	};

// Test21 Description
//     -# Init Component
//          -# Provide callbacks to component (SetCallbacks)
//          -# Obtain the component's state (GetState)
//          -# Obtain component's role #1 with null string (ComponentRoleEnum fails with OMX_ErrorBadParameter)
//          -# Obtain component's role #1 (ComponentRoleEnum successfully returns audio_decoder.aac)
//          -# Obtain component's role #2 (ComponentRoleEnum fails with OMX_ErrorNoMore)
//
class RAsyncTestStepOmxILPcmRenderer0021 : public RAsyncTestStepOmxILComponentBase
	{

public:

	RAsyncTestStepOmxILPcmRenderer0021(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);


protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0021Step01(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0021Step01();

protected:

	CAsyncCallBack* ipKickoff01;
	CAsyncCallBack* ipKickoff02;
	CAsyncCallBack* ipKickoff03;
	CAsyncCallBack* ipKickoff04;
	CAsyncCallBack* ipKickoff05;
	CAsyncCallBack* ipKickoff06;

	};

// Test22 Description
//     -# Init Component
//          -# Provide callbacks to component (SetCallbacks)
//          -# Obtain the component's state (GetState)
//          -# Get component's current role (GetParameter(OMX_IndexParamStandardComponentRole) )
//          -# Set unsupported audio_decoder.mp3 role (SetParameter(OMX_IndexParamStandardComponentRole) )
//               -# SetParameter returns OMX_ErrorBadParameter (audio_decoder.mp3 role not supported)
//
class RAsyncTestStepOmxILPcmRenderer0022 : public RAsyncTestStepOmxILComponentBase
	{

public:

	RAsyncTestStepOmxILPcmRenderer0022(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);


protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0022Step01(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0022Step01();

protected:

	CAsyncCallBack* ipKickoff01;
	CAsyncCallBack* ipKickoff02;
	CAsyncCallBack* ipKickoff03;
	CAsyncCallBack* ipKickoff04;
	CAsyncCallBack* ipKickoff05;
	CAsyncCallBack* ipKickoff06;

	};



//
// RAsyncTestStepOmxILPcmRenderer0001
//
// Test01
//     -# Init Component
//          -# Loaded->Idle(AllocateBuffer-population)
//          -# Idle->Loaded(depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0001 : public RAsyncTestStepOmxILComponentBase
	{

public:

	RAsyncTestStepOmxILPcmRenderer0001(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);


protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0001Step01(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0001Step01();

	static TInt RAsyncTestStepOmxILPcmRenderer0001Step02(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0001Step02();

protected:


	// Set of states for simple state machine
	enum TTestState
		{
		EStateDefault,
		EStateLoadKickoffTestLComplete,
		EStateTransitionToLoadedComplete,
		EStateTransitionToIdleComplete,
		EStateTransitionToWaiForResourcesComplete,
		EStateTransitionToPauseComplete,
		EStateTransitionToExecutingComplete,
		};

	OMX_BUFFERHEADERTYPE* ipBufferHeaderInput;
	OMX_BUFFERHEADERTYPE* ipBufferHeaderOutput;
	RArray<OMX_BUFFERHEADERTYPE*> ipClientClockPortBufferArray;
	
	TBool iTerminateNow;
	TInt iSamplingRate;

	TTestState iTestState;

	CAsyncCallBack* ipKickoff01;
	CAsyncCallBack* ipKickoff02;
	CAsyncCallBack* ipKickoff03;
	CAsyncCallBack* ipKickoff04;
	CAsyncCallBack* ipKickoff05;
	CAsyncCallBack* ipKickoff06;

	};

//
// RAsyncTestStepOmxILPcmRenderer0002
//
// Test02
//     -# Init Component
//          -# Loaded->Idle(UseBuffer-population)
//          -# Idle->Loaded(depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0002 : public RAsyncTestStepOmxILPcmRenderer0001
	{

public:

	RAsyncTestStepOmxILPcmRenderer0002(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);


protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0002Step01(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0002Step01();

	static TInt RAsyncTestStepOmxILPcmRenderer0002Step02(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0002Step02();


protected:
	HBufC8* ipInputBuffer;
	HBufC8* ipOutputBuffer;
	RArray<HBufC8*> iClockPortBufferPtrArray;
	};

//
// RAsyncTestStepOmxILPcmRenderer0003
//
// Test03
//     -# Init Component
//          -# Loaded->WaitForResources
//          -# WaitForResources->Loaded
//
class RAsyncTestStepOmxILPcmRenderer0003 : public RAsyncTestStepOmxILComponentBase
	{

public:

	RAsyncTestStepOmxILPcmRenderer0003(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);


protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0003Step01(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0003Step01();

	static TInt RAsyncTestStepOmxILPcmRenderer0003Step02(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0003Step02();


protected:

	TBool iTerminateNow;

	CAsyncCallBack* ipKickoff01;
	CAsyncCallBack* ipKickoff02;
	CAsyncCallBack* ipKickoff03;
	CAsyncCallBack* ipKickoff04;
	CAsyncCallBack* ipKickoff05;
	CAsyncCallBack* ipKickoff06;


	};

//
// RAsyncTestStepOmxILPcmRenderer0004
//
// Test04
//     -# Init Component
//          -# Loaded->Idle(AllocateBuffer-population)
//          -# Idle->Pause
//          -# Pause->Idle
//          -# Idle->Loaded(depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0004 : public RAsyncTestStepOmxILPcmRenderer0001
	{

public:

	RAsyncTestStepOmxILPcmRenderer0004(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);


protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0004Step01(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0004Step01();

	static TInt RAsyncTestStepOmxILPcmRenderer0004Step02(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0004Step02();

	static TInt RAsyncTestStepOmxILPcmRenderer0004Step03(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0004Step03();

	static TInt RAsyncTestStepOmxILPcmRenderer0004Step04(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0004Step04();

	};

//
// RAsyncTestStepOmxILPcmRenderer0005
//
// Test05
//     -# Init Component
//     -# Loaded->Idle(AllocateBuffer-population)
//     -# Idle->Executing
//          -# Call EmptyThisBuffer on PCM Renderer
//          -# EmptyBufferDone callbacks received
//     -# Executing->Idle(BufferFlushing)
//     -# Idle->Loaded(Depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0005 : public RAsyncTestStepOmxILPcmRenderer0004
	{

public:

	RAsyncTestStepOmxILPcmRenderer0005(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);

	void DoEmptyBufferDoneL(OMX_HANDLETYPE aComponent,
						   OMX_BUFFERHEADERTYPE* aBuffer);

protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0005Step01(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0005Step01();

	static TInt RAsyncTestStepOmxILPcmRenderer0005Step02(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0005Step02();

	static TInt RAsyncTestStepOmxILPcmRenderer0005Step03(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0005Step03();

	static TInt RAsyncTestStepOmxILPcmRenderer0005Step04(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0005Step04();
	};

//
// RAsyncTestStepOmxILPcmRenderer0006
//
// Test06
//     -# Init Component
//         -# Loaded->Idle(AllocateBuffer-population)
//         -# Idle->Executing
//         -# Executing->Pause
//         -# Pause->Executing
//         -# Executing->Idle(BufferFlushing)
//         -# Idle->Loaded(depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0006 : public RAsyncTestStepOmxILPcmRenderer0005
	{

public:

	RAsyncTestStepOmxILPcmRenderer0006(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);


protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0006Step01(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0006Step01();

	static TInt RAsyncTestStepOmxILPcmRenderer0006Step02(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0006Step02();

	static TInt RAsyncTestStepOmxILPcmRenderer0006Step03(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0006Step03();

	static TInt RAsyncTestStepOmxILPcmRenderer0006Step04(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0006Step04();

	static TInt RAsyncTestStepOmxILPcmRenderer0006Step05(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0006Step05();

	static TInt RAsyncTestStepOmxILPcmRenderer0006Step06(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0006Step06();

	};

//
// RAsyncTestStepOmxILPcmRenderer0007
//
// Test07
//     -# Init Component
//         -# Loaded->Idle(AllocateBuffer-population)
//         -# Idle->Executing
//         -# Executing->Pause
//         -# Pause->Idle(BufferFlushing)
//         -# Idle->Loaded(depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0007 : public RAsyncTestStepOmxILPcmRenderer0006
	{

public:

	RAsyncTestStepOmxILPcmRenderer0007(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	void CloseTest();


	//
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);


protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0007Step01(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0007Step01();

	static TInt RAsyncTestStepOmxILPcmRenderer0007Step02(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0007Step02();

	static TInt RAsyncTestStepOmxILPcmRenderer0007Step03(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0007Step03();

	static TInt RAsyncTestStepOmxILPcmRenderer0007Step04(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0007Step04();

	static TInt RAsyncTestStepOmxILPcmRenderer0007Step05(TAny* aPtr);
	// void DoRAsyncTestStepOmxILPcmRenderer0007Step05();
	};

//
// RAsyncTestStepOmxILPcmRenderer0008
//
// Test08
//     -# Init Component
//     -# Loaded->Idle(AllocateBuffer-population)
//     -# Idle->Executing
//		-# Repeat these steps until the whole file has been emptied
//          -# Call EmptyThisBuffer on PCM Renderer
//          -# EmptyBufferDone callbacks received
//     -# Executing->Idle(BufferFlushing)
//     -# Idle->Loaded(Depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0008 : public RAsyncTestStepOmxILPcmRenderer0005
	{
public:
	RAsyncTestStepOmxILPcmRenderer0008(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();

	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
						OMX_EVENTTYPE aEvent,
						TUint aData1,
						TUint aData2,
						TAny* aExtra);

	void DoEmptyBufferDoneL(OMX_HANDLETYPE aComponent,
						   OMX_BUFFERHEADERTYPE* aBuffer);
	};
	
//
// RAsyncTestStepOmxILPcmRenderer0009
//
// Test09
//     -# Init Component
//     -# Loaded->Idle(AllocateBuffer-population)
//     -# Idle->Executing
//		-# Repeat these steps until the whole file has been emptied
//          -# Call EmptyThisBuffer on PCM Renderer
//          -# EmptyBufferDone callbacks received
//		-# Executing->Pause and Pause->Executing (after 5 and again after 10 buffers played)
//     -# Executing->Idle(BufferFlushing)
//     -# Idle->Loaded(Depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0009 : public RAsyncTestStepOmxILPcmRenderer0008
	{
public:
	RAsyncTestStepOmxILPcmRenderer0009(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
			OMX_EVENTTYPE aEvent,
			TUint aData1,
			TUint aData2,
			TAny* aExtra);

	void DoEmptyBufferDoneL(OMX_HANDLETYPE aComponent,
						   OMX_BUFFERHEADERTYPE* aBuffer);
	
private:
	TInt iNumOfEmptiedBuffers;
	};

//
// RAsyncTestStepOmxILPcmRenderer0010
//
// Test09
//     -# Init Component
//     -# Loaded->Idle(AllocateBuffer-population)
//     -# Idle->Executing
//		-# Repeat these steps until the whole file has been emptied
//          -# Call EmptyThisBuffer on PCM Renderer
//          -# EmptyBufferDone callbacks received
//		-# Executing->Idle and Idle->Executing (after 5 and again after 10 buffers played)
//     -# Executing->Idle(BufferFlushing)
//     -# Idle->Loaded(Depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0010 : public RAsyncTestStepOmxILPcmRenderer0008
	{
public:
	RAsyncTestStepOmxILPcmRenderer0010(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();
	
	void DoEventHandlerL(OMX_HANDLETYPE aComponent,
			OMX_EVENTTYPE aEvent,
			TUint aData1,
			TUint aData2,
			TAny* aExtra);

	void DoEmptyBufferDoneL(OMX_HANDLETYPE aComponent,
						   OMX_BUFFERHEADERTYPE* aBuffer);

protected:

	// Call backs
	static TInt RAsyncTestStepOmxILPcmRenderer0010Step01(TAny* aPtr);
	void DoRAsyncTestStepOmxILPcmRenderer0010Step01();

private:


	TInt iNumOfEmptiedBuffers;
	};

//
// RAsyncTestStepOmxILPcmRenderer0030
//
// Test30
//     -# Init Component
//     -# Loaded->Idle(AllocateBuffer-population)
//     -# Idle->Executing
//		-# Repeat these steps until the whole file has been emptied
//          -# Call EmptyThisBuffer on PCM Renderer
//          -# EmptyBufferDone callbacks received
//			 -# Get Bytes Played parameter and compare with the bytes readed from file
//     -# Executing->Idle(BufferFlushing)
//     -# Idle->Loaded(Depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0030 : public RAsyncTestStepOmxILPcmRenderer0008
	{
public:
	RAsyncTestStepOmxILPcmRenderer0030(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();

	void DoEmptyBufferDoneL(OMX_HANDLETYPE aComponent,
						   OMX_BUFFERHEADERTYPE* aBuffer);
	};

//
// RAsyncTestStepOmxILPcmRenderer0031
//
// Test09
//     -# Init Component
//     -# Loaded->Idle(AllocateBuffer-population)
//     -# Idle->Executing
//		-# Repeat these steps until the whole file has been emptied
//          -# Call EmptyThisBuffer on PCM Renderer
//          -# EmptyBufferDone callbacks received
//		-# Set Volume Ramp to 2s (after 5 buffers played)
//     -# Executing->Idle(BufferFlushing)
//     -# Idle->Loaded(Depopulation)
//
class RAsyncTestStepOmxILPcmRenderer0031 : public RAsyncTestStepOmxILPcmRenderer0008
	{
public:
	RAsyncTestStepOmxILPcmRenderer0031(const TDesC& aTestName, TInt aComponentUid);

	// From RAsyncTestStep
	void KickoffTestL();

	void DoEmptyBufferDoneL(OMX_HANDLETYPE aComponent,
						   OMX_BUFFERHEADERTYPE* aBuffer);
	
private:
	TInt iNumOfEmptiedBuffers;
	};

#endif // TSU_OMXIL_PCMRENDERER_STEP_H
