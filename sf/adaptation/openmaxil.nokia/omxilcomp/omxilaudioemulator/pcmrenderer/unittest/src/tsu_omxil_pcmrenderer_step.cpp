// Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <e32base.h>
#include <e32cmn.h>

#include <openmax/il/khronos/v1_x/OMX_Component.h>
#include <openmax/il/khronos/v1_x/OMX_Types.h>

#include <openmax/il/loader/omxilcomponentif.h>
#include <openmax/il/shai/OMX_Symbian_AudioExt.h>
#include <openmax/il/shai/OMX_Symbian_ExtensionNames.h>

#include "log.h"
#include "omxilpcmrenderer.hrh"
#include <openmax/il/common/omxilspecversion.h>

#include "tsu_omxil_pcmrenderer_step.h"

_LIT8(KSymbianOmxILPcmRendererRole, "audio_renderer.pcm");
_LIT8(KTestOmxILWrongRendererRole, "audio_renderer.wrong");
_LIT(KPcmRendererTestFile, "c:\\omxil\\testfiles\\pcm16stereo8khz.raw");
_LIT(KPcmRendererTestFile2, "c:\\omxil\\testfiles\\pcm16stereo44khz.raw");

OMX_ERRORTYPE ComponentInit(OMX_HANDLETYPE /*aComponent*/)
	{
	return OMX_ErrorNone;
	}


//
// RAsyncTestStepOmxILPcmRenderer0020
//
RAsyncTestStepOmxILPcmRenderer0020::RAsyncTestStepOmxILPcmRenderer0020(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILComponentBase(aTestName, aComponentUid),
	ipKickoff01(NULL),
	ipKickoff02(NULL),
	ipKickoff03(NULL),
	ipKickoff04(NULL),
	ipKickoff05(NULL),
	ipKickoff06(NULL)

	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0020::RAsyncTestStepOmxILPcmRenderer0020"));
	}


void
RAsyncTestStepOmxILPcmRenderer0020::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0020::KickoffTestL"));

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test20  "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Init Component "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Obtain the component's state (GetState) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Obtain the component's version (GetComponentVersion) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Provide callbacks to component (SetCallbacks) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 5.- Obtain the port def params for Port #0 (GetParameter) "));

	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	// This is the only step in this test...
	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0020Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	// The successful termination...
	TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
	delete ipKickoffStop;
	ipKickoffStop = NULL;
	ipKickoffStop =
		new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
	ipKickoffStop->Call();

	}

void
RAsyncTestStepOmxILPcmRenderer0020::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0020::CloseTest"));

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	RAsyncTestStepOmxILComponentBase::CloseTest();

	}


void
RAsyncTestStepOmxILPcmRenderer0020::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE /*aEvent*/,
											  TUint /*aData1*/,
											  TUint /*aData2*/,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0020::DoEventHandlerL"));

	// We don't expect here any callback
	return StopTest(KErrGeneral, EFail);

	}


TInt
RAsyncTestStepOmxILPcmRenderer0020::RAsyncTestStepOmxILPcmRenderer0020Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0020::RAsyncTestStepOmxILPcmRenderer0020Step01"));
    RAsyncTestStepOmxILPcmRenderer0020* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0020*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0020Step01();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0020::DoRAsyncTestStepOmxILPcmRenderer0020Step01()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0020::DoRAsyncTestStepOmxILPcmRenderer0020Step01"));


	//
	// Obtain the component's state
	//
	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateLoaded != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	char componentNameArray[OMX_MAX_STRINGNAME_SIZE];
	OMX_VERSIONTYPE componentVersion;
	OMX_VERSIONTYPE specVersion;
	OMX_UUIDTYPE componentUUID;

	//
	// Obtain the component's version
	//

	if (OMX_ErrorNone != ipCompHandle->GetComponentVersion(
			ipCompHandle,
			componentNameArray,
			&componentVersion,
			&specVersion,
			&componentUUID))
		{
		return StopTest(KErrGeneral, EFail);
		}

	TBuf8<128> componentNameBuf8;
	componentNameBuf8 = const_cast<const TUint8*>(reinterpret_cast<TUint8*>(componentNameArray));
	TBuf<128> componentNameBuf16;
	INFO_PRINTF2(_L("Component Name length: %d"), componentNameBuf8.Length());
	componentNameBuf16.Copy(componentNameBuf8);
	componentNameBuf16.PtrZ();

	// INFO_PRINTF2(_L("Component Name: %s"), dst.Ptr());
	INFO_PRINTF2(_L("Component Name: %S"), &componentNameBuf16);
	INFO_PRINTF2(_L("Component Version Major: %d"), componentVersion.s.nVersionMajor);
	INFO_PRINTF2(_L("Component Version Minor: %d"), componentVersion.s.nVersionMinor);
	INFO_PRINTF2(_L("Component Version Revision: %d"), componentVersion.s.nRevision);
	INFO_PRINTF2(_L("Component Version Step: %d"), componentVersion.s.nStep);
	INFO_PRINTF2(_L("OMX Version Major: %d"), specVersion.s.nVersionMajor);
	INFO_PRINTF2(_L("OMX Version Minor: %d"), specVersion.s.nVersionMinor);
	INFO_PRINTF2(_L("OMX Version Revision: %d"), specVersion.s.nRevision);
	INFO_PRINTF2(_L("OMX Version Step: %d"), specVersion.s.nStep);
	INFO_PRINTF2(_L("Component UUID: %X"), componentUUID);


	//
	// Provide callbacks to component
	//
	OMX_CALLBACKTYPE* omxCallbacks = *ipCallbackHandler;

	if (OMX_ErrorNone != ipCompHandle->SetCallbacks(
			ipCompHandle,
			omxCallbacks,
			ipCallbackHandler))
		{
		return StopTest(KErrGeneral, EFail);
		}

	INFO_PRINTF2(_L("Component Callbacks set successfully: %X"), ipCallbackHandler);

	//
	// Obtain the port def params for Port #0
	//
	OMX_PARAM_PORTDEFINITIONTYPE portParamsInputPort;
	portParamsInputPort.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portParamsInputPort.nVersion = TOmxILSpecVersion();
	portParamsInputPort.nPortIndex = 0;
	if (OMX_ErrorNone != ipCompHandle->GetParameter(
			ipCompHandle,
			OMX_IndexParamPortDefinition,
			&portParamsInputPort))
		{
		return StopTest(KErrGeneral, EFail);
		}

	INFO_PRINTF2(_L("PORTDEFINITIONTYPE nPortIndex: %d"), portParamsInputPort.nPortIndex);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE eDir: %d"), portParamsInputPort.eDir);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE nBufferCountActual: %d"), portParamsInputPort.nBufferCountActual);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE nBufferCountMin: %d"), portParamsInputPort.nBufferCountMin);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE nBufferSize: %d"), portParamsInputPort.nBufferSize);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE bEnabled: %d"), portParamsInputPort.bEnabled == OMX_FALSE ? 0 : 1);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE bPopulated: %d"), portParamsInputPort.bPopulated == OMX_FALSE ? 0 : 1);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE eDomain: %d"), portParamsInputPort.eDomain);

	TBuf8<128> cMIMEType8Input;
	cMIMEType8Input = const_cast<const TUint8*>(reinterpret_cast<TUint8*>(portParamsInputPort.format.audio.cMIMEType));

	TBuf<128> cMIMEType16Input;
	cMIMEType16Input.Copy(cMIMEType8Input);
	cMIMEType16Input.PtrZ();

	INFO_PRINTF2(_L("PORTDEFINITIONTYPE format.cMIMEType: %S"), &cMIMEType16Input);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE format.pNativeRender: %X"), portParamsInputPort.format.audio.pNativeRender);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE format.bFlagErrorConcealment: %d"),
				 portParamsInputPort.format.audio.bFlagErrorConcealment == OMX_FALSE ? 0 : 1);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE format.eEncoding: %d"), portParamsInputPort.format.audio.eEncoding);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE bBuffersContiguous: %d"),
				 portParamsInputPort.bBuffersContiguous == OMX_FALSE ? 0 : 1);
	INFO_PRINTF2(_L("PORTDEFINITIONTYPE nBufferAlignment: %d"), portParamsInputPort.nBufferAlignment);
	}

//
// RAsyncTestStepOmxILPcmRenderer0021
//
RAsyncTestStepOmxILPcmRenderer0021::RAsyncTestStepOmxILPcmRenderer0021(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILComponentBase(aTestName, aComponentUid),
	ipKickoff01(NULL),
	ipKickoff02(NULL),
	ipKickoff03(NULL),
	ipKickoff04(NULL),
	ipKickoff05(NULL),
	ipKickoff06(NULL)

	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0021::RAsyncTestStepOmxILPcmRenderer0021"));
	}


void
RAsyncTestStepOmxILPcmRenderer0021::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0021::KickoffTestL"));

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test21  "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Init Component "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Provide callbacks to component (SetCallbacks) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Obtain the component's state (GetState) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Obtain component's role #1 with null string (ComponentRoleEnum fails with OMX_ErrorBadParameter) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 5.- Obtain component's role #1 (ComponentRoleEnum returns audio_renderer.pcm) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 6.- Obtain component's role #2 (ComponentRoleEnum fails with OMX_ErrorNoMore) "));

	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	// This is the only step in this test...
	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0021Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	// The successful termination...
	TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
	delete ipKickoffStop;
	ipKickoffStop = NULL;
	ipKickoffStop =
		new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
	ipKickoffStop->Call();

	}

void
RAsyncTestStepOmxILPcmRenderer0021::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0021::CloseTest"));

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	RAsyncTestStepOmxILComponentBase::CloseTest();

	}


void
RAsyncTestStepOmxILPcmRenderer0021::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE /*aEvent*/,
											  TUint /*aData1*/,
											  TUint /*aData2*/,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0021::DoEventHandlerL"));

	// We don't expect here any callback
	return StopTest(KErrGeneral, EFail);

	}


TInt
RAsyncTestStepOmxILPcmRenderer0021::RAsyncTestStepOmxILPcmRenderer0021Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0021::RAsyncTestStepOmxILPcmRenderer0021Step01"));
    RAsyncTestStepOmxILPcmRenderer0021* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0021*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0021Step01();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0021::DoRAsyncTestStepOmxILPcmRenderer0021Step01()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0021::DoRAsyncTestStepOmxILPcmRenderer0021Step01"));

	//
	// Provide callbacks to component
	//
	OMX_CALLBACKTYPE* omxCallbacks = *ipCallbackHandler;

	if (OMX_ErrorNone != ipCompHandle->SetCallbacks(
			ipCompHandle,
			omxCallbacks,
			ipCallbackHandler))
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Obtain the component's state
	//
	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateLoaded != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	INFO_PRINTF2(_L("Component Callbacks set successfully: %X"), ipCallbackHandler);


	//
	// Try to obtain the component's first role with null string  (must fail with error OMX_ErrorBadParameter)
	//
	OMX_U8* pNullUint = 0;
	if (OMX_ErrorBadParameter != ipCompHandle->ComponentRoleEnum(
			ipCompHandle,
			pNullUint,
			0 // This is index # 0, the first of the component's roles
			))
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Obtain the component's first role (success)
	//
	char componentRoleArray[OMX_MAX_STRINGNAME_SIZE];
	if (OMX_ErrorNone != ipCompHandle->ComponentRoleEnum(
			ipCompHandle,
			reinterpret_cast<OMX_U8*>(componentRoleArray),
			0 // This is index # 0, the first of the component's roles
			))
		{
		return StopTest(KErrGeneral, EFail);
		}

	TBuf8<128> componentRoleBuf8;
	componentRoleBuf8 = const_cast<const TUint8*>(reinterpret_cast<TUint8*>(componentRoleArray));
	TBuf<128> componentRoleBuf16;
	INFO_PRINTF2(_L("Component's Role length: %d"), componentRoleBuf8.Length());
	componentRoleBuf16.Copy(componentRoleBuf8);
	componentRoleBuf16.PtrZ();

	INFO_PRINTF2(_L("Component's 1st Role [%S] "), &componentRoleBuf16);

	if (componentRoleBuf8.Compare(KSymbianOmxILPcmRendererRole()) != 0)
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Obtain the component's second role (must fail with error OMX_ErrorNoMore)
	//
	if (OMX_ErrorNoMore != ipCompHandle->ComponentRoleEnum(
			ipCompHandle,
			reinterpret_cast<OMX_U8*>(componentRoleArray),
			1 // This is index # 1, the second of the component's roles
			))
		{
		return StopTest(KErrGeneral, EFail);
		}

	}

//
// RAsyncTestStepOmxILPcmRenderer0022
//
RAsyncTestStepOmxILPcmRenderer0022::RAsyncTestStepOmxILPcmRenderer0022(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILComponentBase(aTestName, aComponentUid),
	ipKickoff01(NULL),
	ipKickoff02(NULL),
	ipKickoff03(NULL),
	ipKickoff04(NULL),
	ipKickoff05(NULL),
	ipKickoff06(NULL)

	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0022::RAsyncTestStepOmxILPcmRenderer0022"));
	}


void
RAsyncTestStepOmxILPcmRenderer0022::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0022::KickoffTestL"));

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test22  "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Init Component "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Provide callbacks to component (SetCallbacks) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Obtain the component's state (GetState) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Get component's current role (GetParameter(OMX_IndexParamStandardComponentRole) )"));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 5.- Set unsupported audio_renderer.wrong role (SetParameter(OMX_IndexParamStandardComponentRole) )"));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 5a.- SetParameter returns OMX_ErrorBadParameter (audio_renderer.wrong role not supported)"));

	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	// This is the only step in this test...
	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0022Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	// The successful termination...
	TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
	delete ipKickoffStop;
	ipKickoffStop = NULL;
	ipKickoffStop =
		new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
	ipKickoffStop->Call();

	}

void
RAsyncTestStepOmxILPcmRenderer0022::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0022::CloseTest"));

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	RAsyncTestStepOmxILComponentBase::CloseTest();

	}


void
RAsyncTestStepOmxILPcmRenderer0022::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE /*aEvent*/,
											  TUint /*aData1*/,
											  TUint /*aData2*/,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0022::DoEventHandlerL"));

	// We don't expect here any callback
	return StopTest(KErrGeneral, EFail);

	}


TInt
RAsyncTestStepOmxILPcmRenderer0022::RAsyncTestStepOmxILPcmRenderer0022Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0022::RAsyncTestStepOmxILPcmRenderer0022Step01"));
    RAsyncTestStepOmxILPcmRenderer0022* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0022*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0022Step01();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0022::DoRAsyncTestStepOmxILPcmRenderer0022Step01()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0022::DoRAsyncTestStepOmxILPcmRenderer0022Step01"));

	//
	// Provide callbacks to component
	//
	OMX_CALLBACKTYPE* omxCallbacks = *ipCallbackHandler;

	if (OMX_ErrorNone != ipCompHandle->SetCallbacks(
			ipCompHandle,
			omxCallbacks,
			ipCallbackHandler))
		{
		return StopTest(KErrGeneral, EFail);
		}

	INFO_PRINTF2(_L("Component Callbacks set successfully: %X"), ipCallbackHandler);

	//
	// Obtain the component's state
	//
	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateLoaded != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Get component's current standard role
	//
	OMX_PARAM_COMPONENTROLETYPE componentRoleType;
	componentRoleType.nSize = sizeof(OMX_PARAM_COMPONENTROLETYPE);
	componentRoleType.nVersion = TOmxILSpecVersion();
	if (OMX_ErrorNone != ipCompHandle->GetParameter(
			ipCompHandle,
			OMX_IndexParamStandardComponentRole,
			&componentRoleType))
		{
		return StopTest(KErrGeneral, EFail);
		}

	TBuf8<128> componentRoleBuf8;
	componentRoleBuf8 = const_cast<const TUint8*>(reinterpret_cast<TUint8*>(componentRoleType.cRole));
	TBuf<128> componentRoleBuf16;
	INFO_PRINTF2(_L("Component Role length: %d"), componentRoleBuf8.Length());
	componentRoleBuf16.Copy(componentRoleBuf8);
	componentRoleBuf16.PtrZ();

	INFO_PRINTF2(_L("Component's current Role: %S"), &componentRoleBuf16);


	//
	// Set unsupported standard role
	//
	TPtr8 role(reinterpret_cast<TUint8*>(componentRoleType.cRole),
			   OMX_MAX_STRINGNAME_SIZE);
	role.Copy(KTestOmxILWrongRendererRole());
	role.PtrZ();

	if (OMX_ErrorBadParameter != ipCompHandle->SetParameter(
			ipCompHandle,
			OMX_IndexParamStandardComponentRole,
			&componentRoleType))
		{
		return StopTest(KErrGeneral, EFail);
		}

	TBuf8<128> cRole8;
	cRole8 = const_cast<const TUint8*>(reinterpret_cast<TUint8*>(componentRoleType.cRole));

	TBuf<128> cRole16;
	cRole16.Copy(cRole8);
	cRole16.PtrZ();

	INFO_PRINTF2(_L("SetParameter of an unsupported standard role failed (success): %S"), &cRole16);

	}



//
// RAsyncTestStepOmxILPcmRenderer0001
//
RAsyncTestStepOmxILPcmRenderer0001::RAsyncTestStepOmxILPcmRenderer0001(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILComponentBase(aTestName, aComponentUid),
	ipBufferHeaderInput(0),
	ipBufferHeaderOutput(0),
	iTerminateNow(EFalse),
	iSamplingRate(8000),
	iTestState(EStateDefault),
	ipKickoff01(NULL),
	ipKickoff02(NULL),
	ipKickoff03(NULL),
	ipKickoff04(NULL),
	ipKickoff05(NULL),
	ipKickoff06(NULL)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0001::RAsyncTestStepOmxILPcmRenderer0001"));
	}


void
RAsyncTestStepOmxILPcmRenderer0001::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0001::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test01  "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Loaded(depopulation) "));

	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0001Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	}

void
RAsyncTestStepOmxILPcmRenderer0001::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0001::CloseTest"));

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	TInt count = ipClientClockPortBufferArray.Count();
	
	for (TInt n = 0; n < count; n++)
		{
		delete ipClientClockPortBufferArray[n];
		}
	
	ipClientClockPortBufferArray.Close();
	
	RAsyncTestStepOmxILComponentBase::CloseTest();

	}


void
RAsyncTestStepOmxILPcmRenderer0001::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE /*aEvent*/,
											  TUint /*aData1*/,
											  TUint /*aData2*/,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0001::DoEventHandlerL"));

	if (!iTerminateNow)
		{
		TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0001Step02, this);
		delete ipKickoff02;
		ipKickoff02 = NULL;
		ipKickoff02 =
			new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
		ipKickoff02->Call();
		}
	else
		{
		OMX_STATETYPE compState;
		if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
													&compState))
			{
			return StopTest(KErrGeneral, EFail);
			}

		PrintOmxState(compState);

		if (OMX_StateLoaded != compState)
			{
			return StopTest(KErrGeneral, EFail);
			}

		// The successful termination...
		TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
		delete ipKickoffStop;
		ipKickoffStop = NULL;
		ipKickoffStop =
			new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
		ipKickoffStop->Call();
		}

	}


TInt
RAsyncTestStepOmxILPcmRenderer0001::RAsyncTestStepOmxILPcmRenderer0001Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0001::RAsyncTestStepOmxILPcmRenderer0001Step01"));
    RAsyncTestStepOmxILPcmRenderer0001* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0001*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step01();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0001::DoRAsyncTestStepOmxILPcmRenderer0001Step01()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0001::DoRAsyncTestStepOmxILPcmRenderer0001Step01"));

	//
	// Provide callbacks to component
	//
	OMX_CALLBACKTYPE* omxCallbacks = *ipCallbackHandler;

	if (OMX_ErrorNone != ipCompHandle->SetCallbacks(
			ipCompHandle,
			omxCallbacks,
			ipCallbackHandler))
		{
		return StopTest(KErrGeneral, EFail);
		}

	INFO_PRINTF2(_L("Component Callbacks set successfully: %X"), ipCallbackHandler);

	//
	// Obtain the port def params for Port #0
	//
	OMX_PARAM_PORTDEFINITIONTYPE portParamsInputPort;
	portParamsInputPort.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portParamsInputPort.nVersion = TOmxILSpecVersion();
	portParamsInputPort.nPortIndex = 0;
	if (OMX_ErrorNone != ipCompHandle->GetParameter(
			ipCompHandle,
			OMX_IndexParamPortDefinition,
			&portParamsInputPort))
		{
		return StopTest(KErrGeneral, EFail);
		}
	
	//
	// Obtain the port def params for Client Clock Port #1
	//
	OMX_PARAM_PORTDEFINITIONTYPE portParamsInputPort1;
	portParamsInputPort1.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portParamsInputPort1.nVersion = TOmxILSpecVersion();
	portParamsInputPort1.nPortIndex = 1;

	if (OMX_ErrorNone != ipCompHandle->GetParameter(
			ipCompHandle,
			OMX_IndexParamPortDefinition,
			&portParamsInputPort1))
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Change the sampling rate on PCM Renderer's port #0
	//
	OMX_AUDIO_PARAM_PCMMODETYPE pcmMode;
	pcmMode.nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
	pcmMode.nVersion = TOmxILSpecVersion();
	pcmMode.nPortIndex = 0;
	if (OMX_ErrorNone != ipCompHandle->GetParameter(
			ipCompHandle,
			OMX_IndexParamAudioPcm,
			&pcmMode))
		{
		return StopTest(KErrGeneral, EFail);
		}
	pcmMode.nSamplingRate = iSamplingRate;
	if (OMX_ErrorNone != ipCompHandle->SetParameter(
			ipCompHandle,
			OMX_IndexParamAudioPcm,
			&pcmMode))
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Move component to OMX_StateIdle
	//

	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StateIdle,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Moving component to OMX_StateIdle"));

	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateLoaded != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Allocate buffer on input port
	//
	if (OMX_ErrorNone != ipCompHandle->AllocateBuffer(
			ipCompHandle,
			&ipBufferHeaderInput,
			0, // input port
			0,
			portParamsInputPort.nBufferSize))
		{
		return StopTest(KErrGeneral, EFail);
		}
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Populated buffer (AllocateBuffer) in INPUT port"));
	
	//
	// Allocate buffer on Client Clock Port
	//
	TInt bufferCount = portParamsInputPort1.nBufferCountActual;
	
	for (TInt n = 0; n < bufferCount; n++)
		{
		OMX_BUFFERHEADERTYPE* clockBufPtr;
		
		if (OMX_ErrorNone != ipCompHandle->AllocateBuffer(
				ipCompHandle,
				&clockBufPtr,
				1, // Clock input port
				0,
				portParamsInputPort1.nBufferSize))
			{
			return StopTest(KErrGeneral, EFail);
			}
		
		if (ipClientClockPortBufferArray.Append(clockBufPtr) != KErrNone)
			{
			return StopTest(KErrNoMemory, EFail);
			}
		}
	
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Populated buffer (AllocateBuffer) in Clock INPUT port"));
	}

TInt
RAsyncTestStepOmxILPcmRenderer0001::RAsyncTestStepOmxILPcmRenderer0001Step02(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0001::RAsyncTestStepOmxILPcmRenderer0001Step02"));
    RAsyncTestStepOmxILPcmRenderer0001* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0001*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step02();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0001::DoRAsyncTestStepOmxILPcmRenderer0001Step02()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0001::DoRAsyncTestStepOmxILPcmRenderer0001Step02"));

	//
	// Move component to OMX_StateLoaded
	//

	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StateLoaded,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Moving component to OMX_StateLoaded"));

	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateIdle != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Free buffer on input port
	//
	if (OMX_ErrorNone != ipCompHandle->FreeBuffer(
			ipCompHandle,
			0, // input port
			ipBufferHeaderInput))
		{
		return StopTest(KErrGeneral, EFail);
		}
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Depopulated buffer (FreeBuffer) in INPUT port"));

	//
	// Free Client Clock Port buffer on input port
	//
	TInt count =  ipClientClockPortBufferArray.Count();
	
	for (TInt n = 0; n < count; n++)
		{
		if (OMX_ErrorNone != ipCompHandle->FreeBuffer(
				ipCompHandle,
				1, // Client Clock Port
				ipClientClockPortBufferArray[0]))
			{
			return StopTest(KErrGeneral, EFail);
			}

		ipClientClockPortBufferArray.Remove(0);
		}
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Depopulated Client Clock Port buffer (FreeBuffer) in INPUT port"));

	iTerminateNow = ETrue;

	}


//
// RAsyncTestStepOmxILPcmRenderer0002
//
RAsyncTestStepOmxILPcmRenderer0002::RAsyncTestStepOmxILPcmRenderer0002(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0001(aTestName, aComponentUid),
	ipInputBuffer(0),
	ipOutputBuffer(0)

	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0002::RAsyncTestStepOmxILPcmRenderer0002"));
	}


void
RAsyncTestStepOmxILPcmRenderer0002::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0002::KickoffTestL"));

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test02 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(UseBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Loaded(depopulation) "));

	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0002Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	}

void
RAsyncTestStepOmxILPcmRenderer0002::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0002::CloseTest"));

	delete ipInputBuffer;
	ipInputBuffer = NULL;
	delete ipOutputBuffer;
	ipOutputBuffer = NULL;

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	ipClientClockPortBufferArray.Close();
	
	TInt count = iClockPortBufferPtrArray.Count();
	
	for (TInt n = 0; n < count; n++)
		{
		delete iClockPortBufferPtrArray[n];
		}
	
	iClockPortBufferPtrArray.Close();
	
	RAsyncTestStepOmxILComponentBase::CloseTest();

	}

void
RAsyncTestStepOmxILPcmRenderer0002::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE /*aEvent*/,
											  TUint /*aData1*/,
											  TUint /*aData2*/,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0002::DoEventHandlerL"));

	if (!iTerminateNow)
		{
		TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0002Step02, this);
		delete ipKickoff02;
		ipKickoff02 = NULL;
		ipKickoff02 =
			new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
		ipKickoff02->Call();
		}

	if (iTerminateNow)
		{
		OMX_STATETYPE compState;
		if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
													&compState))
			{
			return StopTest(KErrGeneral, EFail);
			}

		PrintOmxState(compState);

		if (OMX_StateLoaded != compState)
			{
			return StopTest(KErrGeneral, EFail);
			}

		// The successful termination...
		TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
		delete ipKickoffStop;
		ipKickoffStop = NULL;
		ipKickoffStop =
			new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
		ipKickoffStop->Call();
		}

	}


TInt
RAsyncTestStepOmxILPcmRenderer0002::RAsyncTestStepOmxILPcmRenderer0002Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0002::RAsyncTestStepOmxILPcmRenderer0002Step01"));
	RAsyncTestStepOmxILPcmRenderer0002* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0002*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0002Step01();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0002::DoRAsyncTestStepOmxILPcmRenderer0002Step01()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0002::DoRAsyncTestStepOmxILPcmRenderer0002Step01"));

	//
	// Provide callbacks to component
	//
	OMX_CALLBACKTYPE* omxCallbacks = *ipCallbackHandler;

	if (OMX_ErrorNone != ipCompHandle->SetCallbacks(
			ipCompHandle,
			omxCallbacks,
			ipCallbackHandler))
		{
		return StopTest(KErrGeneral, EFail);
		}

	INFO_PRINTF2(_L("Component Callbacks set successfully: %X"), ipCallbackHandler);

	//
	// Obtain the port def params for Port #0
	//
	OMX_PARAM_PORTDEFINITIONTYPE portParamsInputPort;
	portParamsInputPort.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portParamsInputPort.nVersion = TOmxILSpecVersion();
	portParamsInputPort.nPortIndex = 0;
	if (OMX_ErrorNone != ipCompHandle->GetParameter(
			ipCompHandle,
			OMX_IndexParamPortDefinition,
			&portParamsInputPort))
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Obtain the port def params for Client Clock Port #1
	//
	OMX_PARAM_PORTDEFINITIONTYPE portParamsInputPort1;
	portParamsInputPort1.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portParamsInputPort1.nVersion = TOmxILSpecVersion();
	portParamsInputPort1.nPortIndex = 1;

	if (OMX_ErrorNone != ipCompHandle->GetParameter(
			ipCompHandle,
			OMX_IndexParamPortDefinition,
			&portParamsInputPort1))
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Move component to OMX_StateIdle
	//

	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StateIdle,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}

	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateLoaded != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}


	//
	// Allocate buffer on input port
	//
	TRAPD(err, ipInputBuffer = HBufC8::NewL(portParamsInputPort.nBufferSize));
	if(err != KErrNone)
		{
		return StopTest(KErrGeneral, EFail);
		}


	if (OMX_ErrorNone != ipCompHandle->UseBuffer(
			ipCompHandle,
			&ipBufferHeaderInput,
			0,					// input port
			0,		// pAppPrivate
			portParamsInputPort.nBufferSize,
			const_cast<TUint8*>(ipInputBuffer->Des().Ptr())))
		{
		return StopTest(KErrGeneral, EFail);
		}
	INFO_PRINTF1(_L("Populated buffer (UseBuffer) in INPUT port"));

	//
	// Allocate Client Clock Port buffer
	//
	TInt bufferCount = portParamsInputPort1.nBufferCountActual;
	
	for (TInt n = 0; n < bufferCount; n++)
		{
		HBufC8* bufPtr;
		
		TRAPD(err, bufPtr = HBufC8::NewL(portParamsInputPort1.nBufferSize));
		
		if(err != KErrNone)
			{
			return StopTest(KErrGeneral, EFail);
			}
		
		if (iClockPortBufferPtrArray.Append(bufPtr) != KErrNone)
			{
			delete bufPtr;
			return StopTest(KErrGeneral, EFail);
			}
		
		OMX_BUFFERHEADERTYPE* clockBufPtr;
		
		if (OMX_ErrorNone != ipCompHandle->UseBuffer(
				ipCompHandle,
				&clockBufPtr,
				1,					// Clock input port
				0,		// pAppPrivate
				portParamsInputPort.nBufferSize,
				const_cast<TUint8*>(bufPtr->Des().Ptr())))
			{
			return StopTest(KErrGeneral, EFail);
			}
		
		TRAPD(error, ipClientClockPortBufferArray.AppendL(clockBufPtr));
		if(error != KErrNone)
		  {
		    return StopTest(KErrGeneral, EFail);
		  }
		}
	INFO_PRINTF1(_L("Populated buffer (UseBuffer) in Client Clock Port"));

	}

TInt
RAsyncTestStepOmxILPcmRenderer0002::RAsyncTestStepOmxILPcmRenderer0002Step02(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0002::RAsyncTestStepOmxILPcmRenderer0002Step02"));
	// NOTE, Here we reuse inherited code, to depopulate the component and make it go to OMX_StateLoaded
	RAsyncTestStepOmxILPcmRenderer0002* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0002*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step02();
	return KErrNone;
	}


//
// RAsyncTestStepOmxILPcmRenderer0003
//
RAsyncTestStepOmxILPcmRenderer0003::RAsyncTestStepOmxILPcmRenderer0003(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILComponentBase(aTestName, aComponentUid),
	iTerminateNow(EFalse),
	ipKickoff01(NULL),
	ipKickoff02(NULL),
	ipKickoff03(NULL),
	ipKickoff04(NULL),
	ipKickoff05(NULL),
	ipKickoff06(NULL)

	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0003::RAsyncTestStepOmxILPcmRenderer0003"));
	}


void
RAsyncTestStepOmxILPcmRenderer0003::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0003::KickoffTestL"));

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test03 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->WaitForResources "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- WaitForResources->Loaded "));

	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0003Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	// This will be used later in DoRAsyncTestStepOmxILPcmRenderer0003Step02
	TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
	delete ipKickoffStop;
	ipKickoffStop = NULL;
	ipKickoffStop =
		new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);

	}

void
RAsyncTestStepOmxILPcmRenderer0003::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0003::CloseTest"));

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	RAsyncTestStepOmxILComponentBase::CloseTest();

	}

void
RAsyncTestStepOmxILPcmRenderer0003::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE /*aEvent*/,
											  TUint /*aData1*/,
											  TUint /*aData2*/,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0003::DoEventHandlerL"));

	TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0003Step02, this);
	delete ipKickoff02;
	ipKickoff02 = NULL;
	ipKickoff02 =
		new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
	ipKickoff02->Call();

	}


TInt
RAsyncTestStepOmxILPcmRenderer0003::RAsyncTestStepOmxILPcmRenderer0003Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0003::RAsyncTestStepOmxILPcmRenderer0003Step01"));
	RAsyncTestStepOmxILPcmRenderer0003* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0003*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0003Step01();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0003::DoRAsyncTestStepOmxILPcmRenderer0003Step01()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0003::DoRAsyncTestStepOmxILPcmRenderer0003Step01"));

	//
	// Obtain the component's state
	//
	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateLoaded != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Provide callbacks to component
	//
	OMX_CALLBACKTYPE* omxCallbacks = *ipCallbackHandler;

	if (OMX_ErrorNone != ipCompHandle->SetCallbacks(
			ipCompHandle,
			omxCallbacks,
			ipCallbackHandler))
		{
		return StopTest(KErrGeneral, EFail);
		}

	INFO_PRINTF2(_L("Component Callbacks set successfully: %X"), ipCallbackHandler);

	//
	// Move component to OMX_StateWaitForResources
	//

	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StateWaitForResources,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}

	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateWaitForResources != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

    //
    // The following code is to improve code coerage only
    //
    OMX_AUDIO_PARAM_PCMMODETYPE pcmMode;
    pcmMode.nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
    pcmMode.nVersion = TOmxILSpecVersion();
    pcmMode.nPortIndex = 0;
    if (OMX_ErrorNone != ipCompHandle->GetParameter(
            ipCompHandle,
            OMX_IndexParamAudioPcm,
            &pcmMode))
        {
        return StopTest(KErrGeneral, EFail);
        }
    pcmMode.nSamplingRate = 44100;
    if (OMX_ErrorNone != ipCompHandle->SetParameter(
            ipCompHandle,
            OMX_IndexParamAudioPcm,
            &pcmMode))
        {
        return StopTest(KErrGeneral, EFail);
        }
    OMX_BUFFERHEADERTYPE temp;
    OMX_BUFFERHEADERTYPE* bufferHeaderInput=&temp;
    if (OMX_ErrorIncorrectStateOperation != ipCompHandle->AllocateBuffer(
            ipCompHandle,
            &bufferHeaderInput,
            0, // input port
            0,
            1024))
        {
        return StopTest(KErrGeneral, EFail);
        }

    if (OMX_ErrorBadPortIndex != ipCompHandle->EmptyThisBuffer(
            ipCompHandle,
            bufferHeaderInput))
        {
        return StopTest(KErrGeneral, EFail);
        }

    if (OMX_ErrorBadPortIndex != ipCompHandle->FillThisBuffer(
            ipCompHandle,
            bufferHeaderInput))
        {
        return StopTest(KErrGeneral, EFail);
        }
    
    if (OMX_ErrorBadPortIndex != ipCompHandle->FreeBuffer(
            ipCompHandle,
            2,
            bufferHeaderInput))
        {
        return StopTest(KErrGeneral, EFail);
        }
    
    if (OMX_ErrorNone != ipCompHandle->SendCommand(
            ipCompHandle,
            OMX_CommandPortEnable,
            0,
            0))
        {
        return StopTest(KErrGeneral, EFail);
        }
    
    OMX_TUNNELSETUPTYPE tunnelSetup;
    OMX_U32 outputPort = 1;
    OMX_U32 inputPort = 0;
        
    if (OMX_ErrorIncorrectStateOperation !=
        ipCompHandle->ComponentTunnelRequest(
            ipCompHandle,
            outputPort,
            ipCompHandle,
            inputPort,
            &tunnelSetup))
        {
        return StopTest(KErrGeneral, EFail);
        }
	}

TInt
RAsyncTestStepOmxILPcmRenderer0003::RAsyncTestStepOmxILPcmRenderer0003Step02(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0003::RAsyncTestStepOmxILPcmRenderer0003Step02"));
	RAsyncTestStepOmxILPcmRenderer0003* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0003*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0003Step02();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0003::DoRAsyncTestStepOmxILPcmRenderer0003Step02()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0003::DoRAsyncTestStepOmxILPcmRenderer0003Step02"));

	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateWaitForResources != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Move component to OMX_StateLoaded
	//

	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StateLoaded,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}

	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateLoaded != compState)
		{		return StopTest(KErrGeneral, EFail);
		}

	// The successful termination...
	ipKickoffStop->Call();

	}


//
// RAsyncTestStepOmxILPcmRenderer0004
//
RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0001(aTestName, aComponentUid)

	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004"));
	}


void
RAsyncTestStepOmxILPcmRenderer0004::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test04 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Pause "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Pause->Idle "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Idle->Loaded(depopulation) "));

	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0004Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	}

void
RAsyncTestStepOmxILPcmRenderer0004::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::CloseTest"));

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	TInt count = ipClientClockPortBufferArray.Count();
	
	for (TInt n = 0; n < count; n++)
		{
		delete ipClientClockPortBufferArray[n];
		}
	
	ipClientClockPortBufferArray.Close();
	
	RAsyncTestStepOmxILComponentBase::CloseTest();

	}

void
RAsyncTestStepOmxILPcmRenderer0004::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE aEvent,
											  TUint aData1,
											  TUint aData2,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::DoEventHandlerL"));

	switch (aEvent)
		{
	case OMX_EventError:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX_EventError [%d]"), aData1);
		return StopTest(KErrGeneral, EFail);
		}
	case OMX_EventCmdComplete:
		{
		switch(aData1)
			{
		case OMX_CommandStateSet:
			{
			switch(aData2)
				{
			case OMX_StateLoaded:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateLoaded [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToLoadedComplete;
					OMX_STATETYPE compState;
					if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
																&compState))
						{
						return StopTest(KErrGeneral, EFail);
						}

					PrintOmxState(compState);

					if (OMX_StateLoaded != compState)
						{
						return StopTest(KErrGeneral, EFail);
						}

					// The successful termination...
					TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
					delete ipKickoffStop;
					ipKickoffStop = NULL;
					ipKickoffStop =
						new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
					ipKickoffStop->Call();
					}
				}
				break;
			case OMX_StateIdle:
				{
				if (EStateLoadKickoffTestLComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateLoadKickoffTestLComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0004Step02, this);
					delete ipKickoff02;
					ipKickoff02 = NULL;
					ipKickoff02 =
						new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
					ipKickoff02->Call();

					}
				else if (EStateTransitionToPauseComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateTransitionToPauseComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback04 (RAsyncTestStepOmxILPcmRenderer0004Step04, this);
					delete ipKickoff04;
					ipKickoff04 = NULL;
					ipKickoff04 =
						new (ELeave) CAsyncCallBack (callback04, CActive::EPriorityLow);
					ipKickoff04->Call();

					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			case OMX_StatePause:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StatePause [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToPauseComplete;
					TCallBack callback03 (RAsyncTestStepOmxILPcmRenderer0004Step03, this);
					delete ipKickoff03;
					ipKickoff03 = NULL;
					ipKickoff03 =
						new (ELeave) CAsyncCallBack (callback03, CActive::EPriorityLow);
					ipKickoff03->Call();
					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StatePause [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			default:
				{
				INFO_PRINTF2(_L("DoEventHandlerL: OMX_State [%d]"), aData2);
				return StopTest(KErrGeneral, EFail);
				}
				};
			}
			break;
		default:
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_Command [%d]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
			};
		}
		break;
	default:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX Event [%d]"), aEvent);
		return StopTest(KErrGeneral, EFail);
		}
		};

	}


TInt
RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004Step01"));
	// NOTE, Here we reuse inherited code, to populate the component and make it go to OMX_StateIdle
	RAsyncTestStepOmxILPcmRenderer0004* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0004*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step01();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004Step02(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004Step02"));
	RAsyncTestStepOmxILPcmRenderer0004* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0004*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0004Step02();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0004::DoRAsyncTestStepOmxILPcmRenderer0004Step02()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::DoRAsyncTestStepOmxILPcmRenderer0004Step02"));

	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateIdle != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Move component to OMX_StatePause
	//

	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StatePause,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}

	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StatePause != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	}

TInt
RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004Step03(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004Step03"));
	RAsyncTestStepOmxILPcmRenderer0004* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0004*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0004Step03();
	return KErrNone;
	}


void
RAsyncTestStepOmxILPcmRenderer0004::DoRAsyncTestStepOmxILPcmRenderer0004Step03()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::DoRAsyncTestStepOmxILPcmRenderer0004Step03"));

	//
	// Move component to OMX_StateIdle
	//

	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StateIdle,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}

	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateIdle != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	}


TInt
RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004Step04(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0004::RAsyncTestStepOmxILPcmRenderer0004Step04"));
	// NOTE, Here we reuse inherited code, to depopulate the component and to
	// make it go to OMX_StateLoaded
	RAsyncTestStepOmxILPcmRenderer0004* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0004*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step02();
	return KErrNone;
	}


//
// RAsyncTestStepOmxILPcmRenderer0005
//
RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0004(aTestName, aComponentUid)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005"));
	}


void
RAsyncTestStepOmxILPcmRenderer0005::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test05 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Executing "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2a. - Call EmptyThisBuffer on PCM Renderer "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2b. - EmptyBufferDone callback received "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Executing->Idle(BufferFlushing) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Idle->Loaded(Depopulation) "));

	// This will change the test file
	iTestFileName.Set(KPcmRendererTestFile);
	
	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0005Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	}

void
RAsyncTestStepOmxILPcmRenderer0005::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::CloseTest"));

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	TInt count = ipClientClockPortBufferArray.Count();
	
	for (TInt n = 0; n < count; n++)
		{
		delete ipClientClockPortBufferArray[n];
		}
	
	ipClientClockPortBufferArray.Close();
	
	RAsyncTestStepOmxILComponentBase::CloseTest();

	}

void
RAsyncTestStepOmxILPcmRenderer0005::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE aEvent,
											  TUint aData1,
											  TUint aData2,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::DoEventHandlerL"));

	switch (aEvent)
		{
	case OMX_EventError:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX_EventError [%d]"), aData1);
		return StopTest(KErrGeneral, EFail);
		}
	case OMX_EventCmdComplete:
		{
		switch(aData1)
			{
		case OMX_CommandStateSet:
			{
			switch(aData2)
				{
			case OMX_StateLoaded:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateLoaded [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToLoadedComplete;
					OMX_STATETYPE compState;
					if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
																&compState))
						{
						return StopTest(KErrGeneral, EFail);
						}

					PrintOmxState(compState);

					if (OMX_StateLoaded != compState)
						{
						return StopTest(KErrGeneral, EFail);
						}

					// The successful termination...
					TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
					delete ipKickoffStop;
					ipKickoffStop = NULL;
					ipKickoffStop =
						new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
					ipKickoffStop->Call();
					}
				}
				break;
			case OMX_StateIdle:
				{
				if (EStateLoadKickoffTestLComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateLoadKickoffTestLComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0005Step02, this);
					delete ipKickoff02;
					ipKickoff02 = NULL;
					ipKickoff02 =
						new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
					ipKickoff02->Call();

					}
				else if (EStateTransitionToExecutingComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateTransitionToPauseComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback04 (RAsyncTestStepOmxILPcmRenderer0005Step04, this);
					delete ipKickoff04;
					ipKickoff04 = NULL;
					ipKickoff04 =
						new (ELeave) CAsyncCallBack (callback04, CActive::EPriorityLow);
					ipKickoff04->Call();

					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			case OMX_StateExecuting:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToExecutingComplete;
					
					TPtr8 ptrData(ipBufferHeaderInput->pBuffer, ipBufferHeaderInput->nAllocLen, ipBufferHeaderInput->nAllocLen);
					ipTestFile->ReadNextBuffer(ptrData);
					ipBufferHeaderInput->nFilledLen = ptrData.Length();

					// Send a buffer to the input port
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [Send a buffer to the input port]"));
					INFO_PRINTF2(_L("ipBufferHeaderInput->pBuffer [%X]"), ipBufferHeaderInput->pBuffer);
					INFO_PRINTF2(_L("ipBufferHeaderInput->nAllocLen [%d]"), ipBufferHeaderInput->nAllocLen);
					INFO_PRINTF2(_L("ipBufferHeaderInput->nFilledLen [%d]"), ipBufferHeaderInput->nFilledLen);
					INFO_PRINTF2(_L("ipBufferHeaderInput->nOffset [%d]"), ipBufferHeaderInput->nOffset);
					INFO_PRINTF2(_L("ipBufferHeaderInput->pAppPrivate [%X]"), ipBufferHeaderInput->pAppPrivate);
					INFO_PRINTF2(_L("ipBufferHeaderInput->pPlatformPrivate [%X]"), ipBufferHeaderInput->pPlatformPrivate);
					INFO_PRINTF2(_L("ipBufferHeaderInput->pInputPortPrivate [%X]"), ipBufferHeaderInput->pInputPortPrivate);
					INFO_PRINTF2(_L("ipBufferHeaderInput->pOutputPortPrivate [%X]"), ipBufferHeaderInput->pOutputPortPrivate);
					INFO_PRINTF2(_L("ipBufferHeaderInput->hMarkTargetComponent [%X]"), ipBufferHeaderInput->hMarkTargetComponent);
					INFO_PRINTF2(_L("ipBufferHeaderInput->pMarkData [%X]"), ipBufferHeaderInput->pMarkData);
					INFO_PRINTF2(_L("ipBufferHeaderInput->nTickCount [%d]"), ipBufferHeaderInput->nTickCount);
					INFO_PRINTF2(_L("ipBufferHeaderInput->nFlags [%X]"), ipBufferHeaderInput->nFlags);
					INFO_PRINTF2(_L("ipBufferHeaderInput->nOutputPortIndex [%X]"), ipBufferHeaderInput->nOutputPortIndex);
					INFO_PRINTF2(_L("ipBufferHeaderInput->nInputPortIndex [%X]"), ipBufferHeaderInput->nInputPortIndex);

					if (OMX_ErrorNone != ipCompHandle->EmptyThisBuffer(
							ipCompHandle, ipBufferHeaderInput))
						{
						return StopTest(KErrGeneral, EFail);
						}
					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			default:
				{
				INFO_PRINTF2(_L("DoEventHandlerL: OMX_State [%d]"), aData2);
				return StopTest(KErrGeneral, EFail);
				}
				};
			}
			break;
		default:
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_Command [%d]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
			};
		}
		break;
	default:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX Event [%d]"), aEvent);
		return StopTest(KErrGeneral, EFail);
		}
		};

	}

void
RAsyncTestStepOmxILPcmRenderer0005::DoEmptyBufferDoneL(OMX_HANDLETYPE /*aComponent*/,
													 OMX_BUFFERHEADERTYPE* /*aBuffer*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::DoEmptyBufferDoneL"));

	INFO_PRINTF1(_L("DoEmptyBufferDoneL : Callback received "));

	TCallBack callback03 (RAsyncTestStepOmxILPcmRenderer0005Step03, this);
	delete ipKickoff03;
	ipKickoff03 = NULL;
	ipKickoff03 =
		new (ELeave) CAsyncCallBack (callback03, CActive::EPriorityLow);
	ipKickoff03->Call();
	}


TInt
RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005Step01"));
	// NOTE, Here we reuse inherited code, to populate the component and make it go to OMX_StateIdle
	RAsyncTestStepOmxILPcmRenderer0005* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0005*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step01();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005Step02(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005Step02"));
	RAsyncTestStepOmxILPcmRenderer0005* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0005*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0005Step02();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0005::DoRAsyncTestStepOmxILPcmRenderer0005Step02()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::DoRAsyncTestStepOmxILPcmRenderer0005Step02"));

	//
	// Move component to OMX_StateExecuting
	//

	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StateExecuting,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}

	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateExecuting != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	}

TInt
RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005Step03(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005Step03"));
	RAsyncTestStepOmxILPcmRenderer0005* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0005*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0004Step03();
	return KErrNone;
	}


TInt
RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005Step04(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0005::RAsyncTestStepOmxILPcmRenderer0005Step04"));
	// NOTE, Here we reuse inherited code, to depopulate the component and to
	// make it go to OMX_StateLoaded
	RAsyncTestStepOmxILPcmRenderer0005* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0005*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step02();
	return KErrNone;
	}



//
// RAsyncTestStepOmxILPcmRenderer0006
//
RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0005(aTestName, aComponentUid)

	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006"));
	}


void
RAsyncTestStepOmxILPcmRenderer0006::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test06 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Executing "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Executing->Pause "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Pause->Executing "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 5.- Executing->Idle(BufferFlushing) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 6.- Idle->Loaded(depopulation) "));

	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0006Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	}

void
RAsyncTestStepOmxILPcmRenderer0006::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::CloseTest"));

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	TInt count = ipClientClockPortBufferArray.Count();
	
	for (TInt n = 0; n < count; n++)
		{
		delete ipClientClockPortBufferArray[n];
		}
	
	ipClientClockPortBufferArray.Close();
	
	RAsyncTestStepOmxILComponentBase::CloseTest();

	}


void
RAsyncTestStepOmxILPcmRenderer0006::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE aEvent,
											  TUint aData1,
											  TUint aData2,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::DoEventHandlerL"));

	switch (aEvent)
		{
	case OMX_EventError:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX_EventError [%d]"), aData1);
		return StopTest(KErrGeneral, EFail);
		}
	case OMX_EventCmdComplete:
		{
		switch(aData1)
			{
		case OMX_CommandStateSet:
			{
			switch(aData2)
				{
			case OMX_StateLoaded:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateLoaded [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToLoadedComplete;
					OMX_STATETYPE compState;
					if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
																&compState))
						{
						return StopTest(KErrGeneral, EFail);
						}

					PrintOmxState(compState);

					if (OMX_StateLoaded != compState)
						{
						return StopTest(KErrGeneral, EFail);
						}

					// The successful termination...
					TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
					delete ipKickoffStop;
					ipKickoffStop = NULL;
					ipKickoffStop =
						new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
					ipKickoffStop->Call();
					}
				}
				break;
			case OMX_StateIdle:
				{
				if (EStateLoadKickoffTestLComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateLoadKickoffTestLComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0006Step02, this);
					delete ipKickoff02;
					ipKickoff02 = NULL;
					ipKickoff02 =
						new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
					ipKickoff02->Call();

					}
				else if (EStateTransitionToExecutingComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateTransitionToExecutingComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback06 (RAsyncTestStepOmxILPcmRenderer0006Step06, this);
					delete ipKickoff06;
					ipKickoff06 = NULL;
					ipKickoff06 =
						new (ELeave) CAsyncCallBack (callback06, CActive::EPriorityLow);
					ipKickoff06->Call();

					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			case OMX_StateExecuting:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToExecutingComplete;
					TCallBack callback03 (RAsyncTestStepOmxILPcmRenderer0006Step03, this);
					delete ipKickoff03;
					ipKickoff03 = NULL;
					ipKickoff03 =
						new (ELeave) CAsyncCallBack (callback03, CActive::EPriorityLow);
					ipKickoff03->Call();

					}
				else if (EStateTransitionToPauseComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [EStateTransitionToPauseComplete]"));
					iTestState = EStateTransitionToExecutingComplete;
					TCallBack callback05 (RAsyncTestStepOmxILPcmRenderer0006Step05, this);
					delete ipKickoff05;
					ipKickoff05 = NULL;
					ipKickoff05 =
						new (ELeave) CAsyncCallBack (callback05, CActive::EPriorityLow);
					ipKickoff05->Call();

					}
				else
					{
					INFO_PRINTF2(_L("DoEventHandlerL: OMX_StateExecuting iTestState[%d]"), iTestState);
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
				case OMX_StatePause:
					{
					if (EStateTransitionToExecutingComplete == iTestState)
						{
						INFO_PRINTF1(_L("DoEventHandlerL: OMX_StatePause [EStateTransitionToExecutingComplete]"));
						iTestState = EStateTransitionToPauseComplete;
						TCallBack callback04 (RAsyncTestStepOmxILPcmRenderer0006Step04, this);
						delete ipKickoff04;
						ipKickoff04 = NULL;
						ipKickoff04 =
							new (ELeave) CAsyncCallBack (callback04, CActive::EPriorityLow);
						ipKickoff04->Call();
						}
					else
						{
						INFO_PRINTF2(_L("DoEventHandlerL: OMX_StatePause iTestState[%d]"), iTestState);
						return StopTest(KErrGeneral, EFail);
						}

					}
				break;
			default:
				{
				INFO_PRINTF2(_L("DoEventHandlerL: OMX_State [%d]"), aData2);
				return StopTest(KErrGeneral, EFail);
				}
				};
			}
			break;
		default:
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_Command [%d]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
			};
		}
		break;
	default:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX Event [%d]"), aEvent);
		return StopTest(KErrGeneral, EFail);
		}
		};

	}


TInt
RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step01"));
	// NOTE, Here we reuse inherited code, to populate the component and make it go to OMX_StateIdle
	RAsyncTestStepOmxILPcmRenderer0006* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0006*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step01();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step02(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step02"));
	RAsyncTestStepOmxILPcmRenderer0006* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0006*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0005Step02();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step03(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step03"));
	RAsyncTestStepOmxILPcmRenderer0006* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0006*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0006Step03();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0006::DoRAsyncTestStepOmxILPcmRenderer0006Step03()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::DoRAsyncTestStepOmxILPcmRenderer0006Step03"));

	//
	// Move component to OMX_StatePause
	//
	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StatePause,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}

	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StatePause != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}

	}


TInt
RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step04(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step04"));
	// NOTE, Here we reuse inherited code, to make it go to OMX_StateIdle
	RAsyncTestStepOmxILPcmRenderer0006* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0006*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0005Step02();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step05(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step05"));
	// NOTE, Here we reuse inherited code, to make it go to OMX_StateIdle
	RAsyncTestStepOmxILPcmRenderer0006* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0006*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0004Step03();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step06(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0006::RAsyncTestStepOmxILPcmRenderer0006Step06"));
	// NOTE, Here we reuse inherited code, to make it go to OMX_StateLoaded
	RAsyncTestStepOmxILPcmRenderer0006* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0006*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step02();
	return KErrNone;
	}


//
// RAsyncTestStepOmxILPcmRenderer0007
//
RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0006(aTestName, aComponentUid)

	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007"));
	}


void
RAsyncTestStepOmxILPcmRenderer0007::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0007::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test07 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Executing "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Executing->Pause "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Pause->Idle(BufferFlushing)"));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 5.- Idle->Loaded(depopulation) "));

	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0007Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();

	}

void
RAsyncTestStepOmxILPcmRenderer0007::CloseTest()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0007::CloseTest"));

	delete ipKickoff01; // no need to Cancel
	ipKickoff01 = NULL;
	delete ipKickoff02; // no need to Cancel
	ipKickoff02 = NULL;
	delete ipKickoff03; // no need to Cancel
	ipKickoff03 = NULL;
	delete ipKickoff04; // no need to Cancel
	ipKickoff04 = NULL;
	delete ipKickoff05; // no need to Cancel
	ipKickoff05 = NULL;
	delete ipKickoff06; // no need to Cancel
	ipKickoff06 = NULL;

	delete ipKickoffStop; // no need to Cancel
	ipKickoffStop = 0;

	TInt count = ipClientClockPortBufferArray.Count();
	
	for (TInt n = 0; n < count; n++)
		{
		delete ipClientClockPortBufferArray[n];
		}
	
	ipClientClockPortBufferArray.Close();
	
	RAsyncTestStepOmxILComponentBase::CloseTest();

	}


void
RAsyncTestStepOmxILPcmRenderer0007::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE aEvent,
											  TUint aData1,
											  TUint aData2,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0007::DoEventHandlerL"));

	switch (aEvent)
		{
	case OMX_EventError:
		{
		//  Ignore underflow errors...
		if (OMX_ErrorUnderflow == (OMX_S32)aData1)
			{
			INFO_PRINTF1(_L("DoEventHandlerL: [OMX_ErrorUnderflow]"));
			}
		else
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_EventError [%x]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
		}
		break;

	case OMX_EventCmdComplete:
		{
		switch(aData1)
			{
		case OMX_CommandStateSet:
			{
			switch(aData2)
				{
			case OMX_StateLoaded:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateLoaded [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToLoadedComplete;
					OMX_STATETYPE compState;
					if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
																&compState))
						{
						return StopTest(KErrGeneral, EFail);
						}

					PrintOmxState(compState);

					if (OMX_StateLoaded != compState)
						{
						return StopTest(KErrGeneral, EFail);
						}

					// The successful termination...
					TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
					delete ipKickoffStop;
					ipKickoffStop = NULL;
					ipKickoffStop =
						new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
					ipKickoffStop->Call();
					}
				}
				break;
			case OMX_StateIdle:
				{
				if (EStateLoadKickoffTestLComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateLoadKickoffTestLComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0007Step02, this);
					delete ipKickoff02;
					ipKickoff02 = NULL;
					ipKickoff02 =
						new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
					ipKickoff02->Call();

					}
				else if (EStateTransitionToPauseComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateTransitionToPauseComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback05 (RAsyncTestStepOmxILPcmRenderer0007Step05, this);
					delete ipKickoff05;
					ipKickoff05 = NULL;
					ipKickoff05 =
						new (ELeave) CAsyncCallBack (callback05, CActive::EPriorityLow);
					ipKickoff05->Call();

					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			case OMX_StateExecuting:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToExecutingComplete;
					TCallBack callback03 (RAsyncTestStepOmxILPcmRenderer0007Step03, this);
					delete ipKickoff03;
					ipKickoff03 = NULL;
					ipKickoff03 =
						new (ELeave) CAsyncCallBack (callback03, CActive::EPriorityLow);
					ipKickoff03->Call();

					}
				else
					{
					INFO_PRINTF2(_L("DoEventHandlerL: OMX_StateExecuting iTestState[%d]"), iTestState);
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			case OMX_StatePause:
				{
				if (EStateTransitionToExecutingComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StatePause [EStateTransitionToExecutingComplete]"));
					iTestState = EStateTransitionToPauseComplete;
					TCallBack callback04 (RAsyncTestStepOmxILPcmRenderer0007Step04, this);
					delete ipKickoff04;
					ipKickoff04 = NULL;
					ipKickoff04 =
						new (ELeave) CAsyncCallBack (callback04, CActive::EPriorityLow);
					ipKickoff04->Call();
					}
				else
					{
					INFO_PRINTF2(_L("DoEventHandlerL: OMX_StatePause iTestState[%d]"), iTestState);
					return StopTest(KErrGeneral, EFail);
					}

				}
				break;
			default:
				{
				INFO_PRINTF2(_L("DoEventHandlerL: OMX_State [%d]"), aData2);
				return StopTest(KErrGeneral, EFail);
				}
				};
			}
			break;
		default:
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_Command [%d]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
			};
		}
		break;
	default:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX Event [%d]"), aEvent);
		return StopTest(KErrGeneral, EFail);
		}
		};

	}


TInt
RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step01"));
	// NOTE, Here we reuse inherited code, to populate the component and make it go to OMX_StateIdle
	RAsyncTestStepOmxILPcmRenderer0007* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0007*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step01();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step02(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step02"));
	RAsyncTestStepOmxILPcmRenderer0007* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0007*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0005Step02();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step03(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step03"));
	RAsyncTestStepOmxILPcmRenderer0007* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0007*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0006Step03();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step04(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step04"));
	// NOTE, Here we reuse inherited code, to make it go to OMX_StateIdle
	RAsyncTestStepOmxILPcmRenderer0007* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0007*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0004Step03();
	return KErrNone;
	}

TInt
RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step05(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0007::RAsyncTestStepOmxILPcmRenderer0007Step05"));
	// NOTE, Here we reuse inherited code, to make it go to OMX_StateIdle
	RAsyncTestStepOmxILPcmRenderer0007* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0007*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0001Step02();
	return KErrNone;
	}

//
// RAsyncTestStepOmxILPcmRenderer0008
//
RAsyncTestStepOmxILPcmRenderer0008::RAsyncTestStepOmxILPcmRenderer0008(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0005(aTestName, aComponentUid)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0008::RAsyncTestStepOmxILPcmRenderer0008"));
	}


void
RAsyncTestStepOmxILPcmRenderer0008::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0008::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test08 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Executing "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Repeat these two steps until the whole file has beeen empited "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2a. - Call EmptyThisBuffer on PCM Renderer "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2b. - EmptyBufferDone callback received "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Executing->Idle(BufferFlushing) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Idle->Loaded(Depopulation) "));

	// This will change the test file
	iTestFileName.Set(KPcmRendererTestFile);
	
	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0005Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();
	}

void
RAsyncTestStepOmxILPcmRenderer0008::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE aEvent,
											  TUint aData1,
											  TUint aData2,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0008::DoEventHandlerL"));

	switch (aEvent)
		{
	case OMX_EventError:
		{
		//  Ignore underflow errors...
		if (OMX_ErrorUnderflow == (OMX_S32)aData1)
			{
			INFO_PRINTF1(_L("DoEventHandlerL: [OMX_ErrorUnderflow]"));
			}
		else
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_EventError [%x]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
		}
		break;

	case OMX_EventCmdComplete:
		{
		switch(aData1)
			{
		case OMX_CommandStateSet:
			{
			switch(aData2)
				{
			case OMX_StateLoaded:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateLoaded [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToLoadedComplete;
					OMX_STATETYPE compState;
					if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
																&compState))
						{
						return StopTest(KErrGeneral, EFail);
						}

					PrintOmxState(compState);

					if (OMX_StateLoaded != compState)
						{
						return StopTest(KErrGeneral, EFail);
						}

					// The successful termination...
					TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
					delete ipKickoffStop;
					ipKickoffStop = NULL;
					ipKickoffStop =
						new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
					ipKickoffStop->Call();
					}
				}
				break;
			case OMX_StateIdle:
				{
				if (EStateLoadKickoffTestLComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateLoadKickoffTestLComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0005Step02, this);
					delete ipKickoff02;
					ipKickoff02 = NULL;
					ipKickoff02 =
						new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
					ipKickoff02->Call();

					}
				else if (EStateTransitionToExecutingComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateTransitionToPauseComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback04 (RAsyncTestStepOmxILPcmRenderer0005Step04, this);
					delete ipKickoff04;
					ipKickoff04 = NULL;
					ipKickoff04 =
						new (ELeave) CAsyncCallBack (callback04, CActive::EPriorityLow);
					ipKickoff04->Call();

					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			case OMX_StateExecuting:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToExecutingComplete;
					
					TPtr8 ptrData(ipBufferHeaderInput->pBuffer, ipBufferHeaderInput->nAllocLen, ipBufferHeaderInput->nAllocLen);
					ipTestFile->ReadNextBuffer(ptrData);
					
					// Send a buffer to the input port
					ipBufferHeaderInput->nFilledLen = ptrData.Length();
					
					if (OMX_ErrorNone != ipCompHandle->EmptyThisBuffer(
							ipCompHandle, ipBufferHeaderInput))
						{
						StopTest(KErrGeneral, EFail);
						}
					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			default:
				{
				INFO_PRINTF2(_L("DoEventHandlerL: OMX_State [%d]"), aData2);
				return StopTest(KErrGeneral, EFail);
				}
				};
			}
			break;
		default:
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_Command [%d]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
			};
		}
		break;
	default:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX Event [%d]"), aEvent);
		return StopTest(KErrGeneral, EFail);
		}
		};

	}

void
RAsyncTestStepOmxILPcmRenderer0008::DoEmptyBufferDoneL(OMX_HANDLETYPE /*aComponent*/,
													 OMX_BUFFERHEADERTYPE* /*aBuffer*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0008::DoEmptyBufferDoneL"));
	INFO_PRINTF1(_L("DoEmptyBufferDoneL : Callback received "));
	
	TPtr8 ptrData(ipBufferHeaderInput->pBuffer, ipBufferHeaderInput->nAllocLen, ipBufferHeaderInput->nAllocLen);
	ipTestFile->ReadNextBuffer(ptrData);
	
	if (ptrData.Length() > 0)
		{
		// Send a buffer to the input port
		ipBufferHeaderInput->nFilledLen = ptrData.Length();
		
		if (OMX_ErrorNone != ipCompHandle->EmptyThisBuffer(
				ipCompHandle, ipBufferHeaderInput))
			{
			StopTest(KErrGeneral, EFail);
			}
		}
	else
		{
		TCallBack callback03 (RAsyncTestStepOmxILPcmRenderer0005Step03, this);
		delete ipKickoff03;
		ipKickoff03 = NULL;
		ipKickoff03 =
			new (ELeave) CAsyncCallBack (callback03, CActive::EPriorityLow);
		ipKickoff03->Call();
		}
	}

//
// RAsyncTestStepOmxILPcmRenderer0009
//
RAsyncTestStepOmxILPcmRenderer0009::RAsyncTestStepOmxILPcmRenderer0009(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0008(aTestName, aComponentUid)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0009::RAsyncTestStepOmxILPcmRenderer0009"));
    iSamplingRate = 44100;
	}


void
RAsyncTestStepOmxILPcmRenderer0009::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0009::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test09 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Executing "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Repeat these two steps until the whole file has beeen empited "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2a. - Call EmptyThisBuffer on PCM Renderer "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2b. - EmptyBufferDone callback received "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Executing->Pause and Pause->Executing (after 5 and again after 10 buffers played) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Executing->Idle(BufferFlushing) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 5.- Idle->Loaded(Depopulation) "));

	// This will change the test file
	iTestFileName.Set(KPcmRendererTestFile2);
	
	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0005Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();
	}

void
RAsyncTestStepOmxILPcmRenderer0009::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE aEvent,
											  TUint aData1,
											  TUint aData2,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0009::DoEventHandlerL"));
	
	switch (aEvent)
		{
	case OMX_EventError:
		{
		//  Ignore underflow errors...
		if (OMX_ErrorUnderflow == (OMX_S32)aData1)
			{
			INFO_PRINTF1(_L("DoEventHandlerL: [OMX_ErrorUnderflow]"));
			}
		else
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_EventError [%x]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
		}
		break;

	case OMX_EventCmdComplete:
		{
		switch(aData1)
			{
		case OMX_CommandStateSet:
			{
			switch(aData2)
				{
			case OMX_StateLoaded:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateLoaded [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToLoadedComplete;
					OMX_STATETYPE compState;
					if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
																&compState))
						{
						return StopTest(KErrGeneral, EFail);
						}

					PrintOmxState(compState);

					if (OMX_StateLoaded != compState)
						{
						return StopTest(KErrGeneral, EFail);
						}

					// The successful termination...
					TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
					delete ipKickoffStop;
					ipKickoffStop = NULL;
					ipKickoffStop =
						new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
					ipKickoffStop->Call();
					}
				}
				break;
			case OMX_StateIdle:
				{
				if (EStateLoadKickoffTestLComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateLoadKickoffTestLComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0005Step02, this);
					delete ipKickoff02;
					ipKickoff02 = NULL;
					ipKickoff02 =
						new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
					ipKickoff02->Call();

					}
				else if (EStateTransitionToExecutingComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateTransitionToPauseComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback04 (RAsyncTestStepOmxILPcmRenderer0005Step04, this);
					delete ipKickoff04;
					ipKickoff04 = NULL;
					ipKickoff04 =
						new (ELeave) CAsyncCallBack (callback04, CActive::EPriorityLow);
					ipKickoff04->Call();

					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			case OMX_StateExecuting:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToExecutingComplete;
					
					TPtr8 ptrData(ipBufferHeaderInput->pBuffer, ipBufferHeaderInput->nAllocLen, ipBufferHeaderInput->nAllocLen);
					ipTestFile->ReadNextBuffer(ptrData);
					
					// Send a buffer to the input port
					ipBufferHeaderInput->nFilledLen = ptrData.Length();
					
					if (OMX_ErrorNone != ipCompHandle->EmptyThisBuffer(
							ipCompHandle, ipBufferHeaderInput))
						{
						StopTest(KErrGeneral, EFail);
						}
					}
				else if (EStateTransitionToPauseComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToExecutingComplete;
					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			case OMX_StatePause:
				{
				if (EStateTransitionToExecutingComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StatePause [EStateTransitionToExecutingComplete]"));
					iTestState = EStateTransitionToPauseComplete;
					
					// Wait for a second...
					User::After(1000000);
					
					// Resume playing...
					TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0005Step02, this);
					delete ipKickoff02;
					ipKickoff02 = NULL;
					ipKickoff02 =
						new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
					ipKickoff02->Call();
					}
				else
					{
					INFO_PRINTF2(_L("DoEventHandlerL: OMX_StatePause iTestState[%d]"), iTestState);
					return StopTest(KErrGeneral, EFail);
					}

				}
				break;
			default:
				{
				INFO_PRINTF2(_L("DoEventHandlerL: OMX_State [%d]"), aData2);
				return StopTest(KErrGeneral, EFail);
				}
				};
			}
			break;
		default:
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_Command [%d]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
			};
		}
		break;
	default:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX Event [%d]"), aEvent);
		return StopTest(KErrGeneral, EFail);
		}
		};

	}

void
RAsyncTestStepOmxILPcmRenderer0009::DoEmptyBufferDoneL(OMX_HANDLETYPE /*aComponent*/,
													 OMX_BUFFERHEADERTYPE* /*aBuffer*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0009::DoEmptyBufferDoneL"));
	INFO_PRINTF1(_L("DoEmptyBufferDoneL : Callback received "));
	
	iNumOfEmptiedBuffers++;
	if (iNumOfEmptiedBuffers == 5 || iNumOfEmptiedBuffers == 10)
		{
		//
		// Move component to OMX_StatePause
		//
		if (OMX_ErrorNone != ipCompHandle->SendCommand(
				ipCompHandle,
				OMX_CommandStateSet,
				OMX_StatePause,
				0))
			{
			return StopTest(KErrGeneral, EFail);
			}

		OMX_STATETYPE compState;
		if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
													&compState))
			{
			return StopTest(KErrGeneral, EFail);
			}

		PrintOmxState(compState);
		if (OMX_StatePause != compState)
			{
			return StopTest(KErrGeneral, EFail);
			}
		}

	TPtr8 ptrData(ipBufferHeaderInput->pBuffer, ipBufferHeaderInput->nAllocLen, ipBufferHeaderInput->nAllocLen);
	ipTestFile->ReadNextBuffer(ptrData);
	
	if (ptrData.Length() > 0)
		{
		// Send a buffer to the input port
		ipBufferHeaderInput->nFilledLen = ptrData.Length();
		
		if (OMX_ErrorNone != ipCompHandle->EmptyThisBuffer(
				ipCompHandle, ipBufferHeaderInput))
			{
			StopTest(KErrGeneral, EFail);
			}
		}
	else
		{
		TCallBack callback03 (RAsyncTestStepOmxILPcmRenderer0005Step03, this);
		delete ipKickoff03;
		ipKickoff03 = NULL;
		ipKickoff03 =
			new (ELeave) CAsyncCallBack (callback03, CActive::EPriorityLow);
		ipKickoff03->Call();
		}
	}

//
// RAsyncTestStepOmxILPcmRenderer0010
//
RAsyncTestStepOmxILPcmRenderer0010::RAsyncTestStepOmxILPcmRenderer0010(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0008(aTestName, aComponentUid)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0010::RAsyncTestStepOmxILPcmRenderer0010"));
    iSamplingRate = 44100;
	}


void
RAsyncTestStepOmxILPcmRenderer0010::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0010::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test10 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Executing "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Repeat these two steps until the whole file has beeen empited "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2a. - Call EmptyThisBuffer on PCM Renderer "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2b. - EmptyBufferDone callback received "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Executing->Idle and Idle->Executing (after 5 and again after 10 buffers played) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Executing->Idle(BufferFlushing) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 5.- Idle->Loaded(Depopulation) "));

	// This will change the test file
	iTestFileName.Set(KPcmRendererTestFile2);
	
	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0005Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();
	}

void
RAsyncTestStepOmxILPcmRenderer0010::DoEventHandlerL(OMX_HANDLETYPE /*aComponent*/,
											  OMX_EVENTTYPE aEvent,
											  TUint aData1,
											  TUint aData2,
											  TAny* /*aExtra*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0010::DoEventHandlerL"));

	switch (aEvent)
		{
	case OMX_EventError:
		{
		//  Ignore underflow errors...
		if (OMX_ErrorUnderflow == (OMX_S32)aData1)
			{
			INFO_PRINTF1(_L("DoEventHandlerL: [OMX_ErrorUnderflow]"));
			}
		else
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_EventError [%x]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
		}
		break;

	case OMX_EventCmdComplete:
		{
		switch(aData1)
			{
		case OMX_CommandStateSet:
			{
			switch(aData2)
				{
			case OMX_StateLoaded:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateLoaded [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToLoadedComplete;
					OMX_STATETYPE compState;
					if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
																&compState))
						{
						return StopTest(KErrGeneral, EFail);
						}

					PrintOmxState(compState);

					if (OMX_StateLoaded != compState)
						{
						return StopTest(KErrGeneral, EFail);
						}

					// The successful termination...
					TCallBack callbackStop (RAsyncTestStepOmxILComponentBase::StopTestCallBack, this);
					delete ipKickoffStop;
					ipKickoffStop = NULL;
					ipKickoffStop =
						new (ELeave) CAsyncCallBack (callbackStop, CActive::EPriorityLow);
					ipKickoffStop->Call();
					}
				}
				break;
			case OMX_StateIdle:
				{
				if (EStateLoadKickoffTestLComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateLoadKickoffTestLComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0005Step02, this);
					delete ipKickoff02;
					ipKickoff02 = NULL;
					ipKickoff02 =
						new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
					ipKickoff02->Call();

					}
				else if (EStateTransitionToExecutingComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [EStateTransitionToExecutingComplete]"));
					iTestState = EStateTransitionToIdleComplete;
					
					if(iNumOfEmptiedBuffers <= 20)
						{
						// Wait for a second...
						User::After(1000000);
						
						// Resume playing...
						TCallBack callback02 (RAsyncTestStepOmxILPcmRenderer0005Step02, this);
						delete ipKickoff02;
						ipKickoff02 = NULL;
						ipKickoff02 =
							new (ELeave) CAsyncCallBack (callback02, CActive::EPriorityLow);
						ipKickoff02->Call();
						}
					else
						{
						// Playback finished
						TCallBack callback04 (RAsyncTestStepOmxILPcmRenderer0005Step04, this);
						delete ipKickoff04;
						ipKickoff04 = NULL;
						ipKickoff04 =
							new (ELeave) CAsyncCallBack (callback04, CActive::EPriorityLow);
						ipKickoff04->Call();
						}
					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateIdle [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			case OMX_StateExecuting:
				{
				if (EStateTransitionToIdleComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToExecutingComplete;
					
					TPtr8 ptrData(ipBufferHeaderInput->pBuffer, ipBufferHeaderInput->nAllocLen, ipBufferHeaderInput->nAllocLen);
					ipTestFile->ReadNextBuffer(ptrData);
					
					// Send a buffer to the input port
					ipBufferHeaderInput->nFilledLen = ptrData.Length();
					
					if (OMX_ErrorNone != ipCompHandle->EmptyThisBuffer(
							ipCompHandle, ipBufferHeaderInput))
						{
						StopTest(KErrGeneral, EFail);
						}
					}
				else if (EStateTransitionToPauseComplete == iTestState)
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [EStateTransitionToIdleComplete]"));
					iTestState = EStateTransitionToExecutingComplete;
					}
				else
					{
					INFO_PRINTF1(_L("DoEventHandlerL: OMX_StateExecuting [Other]"));
					return StopTest(KErrGeneral, EFail);
					}
				}
				break;
			default:
				{
				INFO_PRINTF2(_L("DoEventHandlerL: OMX_State [%d]"), aData2);
				return StopTest(KErrGeneral, EFail);
				}
				};
			}
			break;
		default:
			{
			INFO_PRINTF2(_L("DoEventHandlerL: OMX_Command [%d]"), aData1);
			return StopTest(KErrGeneral, EFail);
			}
			};
		}
		break;
	default:
		{
		INFO_PRINTF2(_L("DoEventHandlerL: OMX Event [%d]"), aEvent);
		return StopTest(KErrGeneral, EFail);
		}
		};

	}

void
RAsyncTestStepOmxILPcmRenderer0010::DoEmptyBufferDoneL(OMX_HANDLETYPE /*aComponent*/,
													 OMX_BUFFERHEADERTYPE* /*aBuffer*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0010::DoEmptyBufferDoneL"));
	INFO_PRINTF1(_L("DoEmptyBufferDoneL : Callback received "));
	
	iNumOfEmptiedBuffers++;
	if (iNumOfEmptiedBuffers == 5 || iNumOfEmptiedBuffers == 10)
		{
		//
		// Move component to OMX_StateIdle
		//

		if (OMX_ErrorNone != ipCompHandle->SendCommand(
				ipCompHandle,
				OMX_CommandStateSet,
				OMX_StateIdle,
				0))
			{
			return StopTest(KErrGeneral, EFail);
			}

		OMX_STATETYPE compState;
		if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
													&compState))
			{
			return StopTest(KErrGeneral, EFail);
			}

		PrintOmxState(compState);

		if (OMX_StateIdle != compState)
			{
			return StopTest(KErrGeneral, EFail);
			}
		
		// Reset the file position
		ipTestFile->ResetPos();
		}
	else
		{
        TPtr8 ptrData(ipBufferHeaderInput->pBuffer, ipBufferHeaderInput->nAllocLen, ipBufferHeaderInput->nAllocLen);
		ipTestFile->ReadNextBuffer(ptrData);
		
		if (ptrData.Length() > 0)
			{
			// Send a buffer to the input port
			ipBufferHeaderInput->nFilledLen = ptrData.Length();
			
			if (OMX_ErrorNone != ipCompHandle->EmptyThisBuffer(
					ipCompHandle, ipBufferHeaderInput))
				{
				StopTest(KErrGeneral, EFail);
				}
			}
		else
			{
			TCallBack callback03 (RAsyncTestStepOmxILPcmRenderer0005Step03, this);
			delete ipKickoff03;
			ipKickoff03 = NULL;
			ipKickoff03 =
				new (ELeave) CAsyncCallBack (callback03, CActive::EPriorityLow);
			ipKickoff03->Call();
			}
		}
	}

TInt
RAsyncTestStepOmxILPcmRenderer0010::RAsyncTestStepOmxILPcmRenderer0010Step01(TAny* aPtr)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0010::RAsyncTestStepOmxILPcmRenderer0010Step01"));
	RAsyncTestStepOmxILPcmRenderer0010* self =
		static_cast<RAsyncTestStepOmxILPcmRenderer0010*> (aPtr);
	self->DoRAsyncTestStepOmxILPcmRenderer0010Step01();
	return KErrNone;
	}

void
RAsyncTestStepOmxILPcmRenderer0010::DoRAsyncTestStepOmxILPcmRenderer0010Step01()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0010::DoRAsyncTestStepOmxILPcmRenderer0010Step01"));

	//
	// Provide callbacks to component
	//
	OMX_CALLBACKTYPE* omxCallbacks = *ipCallbackHandler;

	if (OMX_ErrorNone != ipCompHandle->SetCallbacks(
			ipCompHandle,
			omxCallbacks,
			ipCallbackHandler))
		{
		return StopTest(KErrGeneral, EFail);
		}

	INFO_PRINTF2(_L("Component Callbacks set successfully: %X"), ipCallbackHandler);

	//
	// Obtain the port def params for Port #0
	//
	OMX_PARAM_PORTDEFINITIONTYPE portParamsInputPort;
	portParamsInputPort.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portParamsInputPort.nVersion = TOmxILSpecVersion();
	portParamsInputPort.nPortIndex = 0;
	if (OMX_ErrorNone != ipCompHandle->GetParameter(
			ipCompHandle,
			OMX_IndexParamPortDefinition,
			&portParamsInputPort))
		{
		return StopTest(KErrGeneral, EFail);
		}
	
	//
	// Change the sampling rate on PCM Renderer's port #0
	//
	OMX_AUDIO_PARAM_PCMMODETYPE pcmMode;
	pcmMode.nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
	pcmMode.nVersion = TOmxILSpecVersion();
	pcmMode.nPortIndex = 0;
	if (OMX_ErrorNone != ipCompHandle->GetParameter(
			ipCompHandle,
			OMX_IndexParamAudioPcm,
			&pcmMode))
		{
		return StopTest(KErrGeneral, EFail);
		}
	pcmMode.nSamplingRate = 44100;
	if (OMX_ErrorNone != ipCompHandle->SetParameter(
			ipCompHandle,
			OMX_IndexParamAudioPcm,
			&pcmMode))
		{
		return StopTest(KErrGeneral, EFail);
		}

	//
	// Move component to OMX_StateIdle
	//

	if (OMX_ErrorNone != ipCompHandle->SendCommand(
			ipCompHandle,
			OMX_CommandStateSet,
			OMX_StateIdle,
			0))
		{
		return StopTest(KErrGeneral, EFail);
		}
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Moving component to OMX_StateIdle"));

	OMX_STATETYPE compState;
	if (OMX_ErrorNone != ipCompHandle->GetState(ipCompHandle,
												&compState))
		{
		return StopTest(KErrGeneral, EFail);
		}

	PrintOmxState(compState);

	if (OMX_StateLoaded != compState)
		{
		return StopTest(KErrGeneral, EFail);
		}


	//
	// Allocate buffer on input port
	//
	if (OMX_ErrorNone != ipCompHandle->AllocateBuffer(
			ipCompHandle,
			&ipBufferHeaderInput,
			0, // input port
			0,
			portParamsInputPort.nBufferSize))
		{
		return StopTest(KErrGeneral, EFail);
		}
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Populated buffer (AllocateBuffer) in INPUT port"));
	}


//
// RAsyncTestStepOmxILPcmRenderer0030
//
RAsyncTestStepOmxILPcmRenderer0030::RAsyncTestStepOmxILPcmRenderer0030(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0008(aTestName, aComponentUid)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0030::RAsyncTestStepOmxILPcmRenderer0030"));
	}


void
RAsyncTestStepOmxILPcmRenderer0030::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0030::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test30 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Executing "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Repeat these two steps until the whole file has beeen empited "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2a. - Call EmptyThisBuffer on PCM Renderer "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2b. - EmptyBufferDone callback received "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2c. - Get Data Amount parameter and compare with the bytes readed from file "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Executing->Idle(BufferFlushing) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Idle->Loaded(Depopulation) "));

	// This will change the test file
	iTestFileName.Set(KPcmRendererTestFile);
	
	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0005Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();
	}

void
RAsyncTestStepOmxILPcmRenderer0030::DoEmptyBufferDoneL(OMX_HANDLETYPE /*aComponent*/,
													 OMX_BUFFERHEADERTYPE* /*aBuffer*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0030::DoEmptyBufferDoneL"));
	INFO_PRINTF1(_L("DoEmptyBufferDoneL : Callback received "));
	
	OMX_INDEXTYPE audioSpecificConfigIndex = OMX_IndexMax;
	if (OMX_ErrorNone != ipCompHandle->GetExtensionIndex(
							     ipCompHandle,
							     OMX_SYMBIAN_INDEX_CONFIG_AUDIO_DATAAMOUNT_NAME,
							     &audioSpecificConfigIndex))
	  {
	    return StopTest(KErrGeneral, EFail);
	  }

	// Check that GetConfig is returning the correct amount of bytes played
	OMX_SYMBIAN_AUDIO_CONFIG_PROCESSEDDATAAMOUNTTYPE bytesPlayedStruct;
	bytesPlayedStruct.nSize = sizeof(OMX_SYMBIAN_AUDIO_CONFIG_PROCESSEDDATAAMOUNTTYPE);
	bytesPlayedStruct.nVersion = TOmxILSpecVersion();
	bytesPlayedStruct.nPortIndex = 0;

	if (OMX_ErrorNone != ipCompHandle->GetConfig(ipCompHandle, 
	        audioSpecificConfigIndex, 
	        &bytesPlayedStruct))
		{
		StopTest(KErrGeneral, EFail);
		}

	OMX_AUDIO_PARAM_PCMMODETYPE pcmModeType;
	pcmModeType.nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
	pcmModeType.nVersion = TOmxILSpecVersion();
	pcmModeType.nPortIndex = 0;

	if (OMX_ErrorNone != ipCompHandle->GetParameter(ipCompHandle, 
	        static_cast<OMX_INDEXTYPE>(OMX_IndexParamAudioPcm), 
	        &pcmModeType))
	    {
	    StopTest(KErrGeneral, EFail);
	    }
	   
	//check the number of bytes played by the device corresponds
	//to the number of bytes readed from file
	//allow an extra margin of one buffer.
	TUint maxAllowedBytesPlayed =ipTestFile->GetPos();

	// Convert into OMX_TICKS
	
#ifndef OMX_SKIP64BIT	
	OMX_TICKS ticks = static_cast<OMX_TICKS>(maxAllowedBytesPlayed) * 8 * OMX_TICKS_PER_SECOND / 
	        static_cast<OMX_TICKS>(pcmModeType.nBitPerSample) / static_cast<OMX_TICKS>(pcmModeType.nSamplingRate);
#else
	TInt64 ticks = static_cast<TInt64>(maxAllowedBytesPlayed) * 8 * OMX_TICKS_PER_SECOND/
	        static_cast<TInt64>(pcmModeType.nBitPerSample) / static_cast<TInt64>(pcmModeType.nSamplingRate);
#endif
        
	if (bytesPlayedStruct.nProcessedDataAmount > ticks)
		{
		StopTest(KErrGeneral, EFail);
		}
	
	INFO_PRINTF2(_L("DoEmptyBufferDoneL : bytesPlayedStruct.nProcessedDataAmount=%d "), bytesPlayedStruct.nProcessedDataAmount);
	
	TPtr8 ptrData(ipBufferHeaderInput->pBuffer, ipBufferHeaderInput->nAllocLen, ipBufferHeaderInput->nAllocLen);
	ipTestFile->ReadNextBuffer(ptrData);
	
	if (ptrData.Length() > 0)
		{
		// Send a buffer to the input port
		ipBufferHeaderInput->nFilledLen = ptrData.Length();
		
		if (OMX_ErrorNone != ipCompHandle->EmptyThisBuffer(
				ipCompHandle, ipBufferHeaderInput))
			{
			StopTest(KErrGeneral, EFail);
			}
		}
	else
		{
		TCallBack callback03 (RAsyncTestStepOmxILPcmRenderer0005Step03, this);
		delete ipKickoff03;
		ipKickoff03 = NULL;
		ipKickoff03 =
			new (ELeave) CAsyncCallBack (callback03, CActive::EPriorityLow);
		ipKickoff03->Call();
		}
	}

//
// RAsyncTestStepOmxILPcmRenderer0031
//
RAsyncTestStepOmxILPcmRenderer0031::RAsyncTestStepOmxILPcmRenderer0031(const TDesC& aTestName, TInt aComponentUid)
	:
	RAsyncTestStepOmxILPcmRenderer0008(aTestName, aComponentUid)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0031::RAsyncTestStepOmxILPcmRenderer0031"));
    iSamplingRate = 44100;    
	}


void
RAsyncTestStepOmxILPcmRenderer0031::KickoffTestL()
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0031::KickoffTestL"));
	iTestState = EStateLoadKickoffTestLComplete;

	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : Test31 "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 1.- Loaded->Idle(AllocateBuffer-population) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Idle->Executing "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2.- Repeat these two steps until the whole file has beeen empited "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2a. - Call EmptyThisBuffer on PCM Renderer "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 2b. - EmptyBufferDone callback received "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 3.- Set Volume Ramp to 2s (after 5 buffers played) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 4.- Executing->Idle(BufferFlushing) "));
	INFO_PRINTF1(_L("UNIT TEST OMX IL PCM RENDERER : 5.- Idle->Loaded(Depopulation) "));

	// This will change the test file
	iTestFileName.Set(KPcmRendererTestFile2);
	
	// This will install one callback
	RAsyncTestStepOmxILComponentBase::KickoffTestL();

	TCallBack callback01 (RAsyncTestStepOmxILPcmRenderer0005Step01, this);
	delete ipKickoff01;
	ipKickoff01 = NULL;
	ipKickoff01 =
		new (ELeave) CAsyncCallBack (callback01, CActive::EPriorityLow);
	ipKickoff01->Call();
	}

void
RAsyncTestStepOmxILPcmRenderer0031::DoEmptyBufferDoneL(OMX_HANDLETYPE /*aComponent*/,
													 OMX_BUFFERHEADERTYPE* /*aBuffer*/)
	{
    DEBUG_PRINTF(_L8("RAsyncTestStepOmxILPcmRenderer0031::DoEmptyBufferDoneL"));
	INFO_PRINTF1(_L("DoEmptyBufferDoneL : Callback received "));
	
	iNumOfEmptiedBuffers++;
	if (iNumOfEmptiedBuffers == 5)
		{
		OMX_INDEXTYPE audioSpecificConfigIndex = OMX_IndexMax;
		if (OMX_ErrorNone != ipCompHandle->GetExtensionIndex(
								     ipCompHandle,
								     OMX_SYMBIAN_INDEX_CONFIG_AUDIO_VOLUMERAMP_NAME,
								     &audioSpecificConfigIndex))
			{
			  return StopTest(KErrGeneral, EFail);
			}

		OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE volumeRampStruct;
		volumeRampStruct.nSize = sizeof(OMX_SYMBIAN_AUDIO_CONFIG_VOLUMERAMPTYPE);
		volumeRampStruct.nVersion = TOmxILSpecVersion();
		volumeRampStruct.nPortIndex = 0;

		if (OMX_ErrorNone != ipCompHandle->GetConfig(ipCompHandle, 
                                                     audioSpecificConfigIndex, 
													 &volumeRampStruct))
			{
			StopTest(KErrGeneral, EFail);
			}
		volumeRampStruct.nRampDuration = 2000000;	// 2 second
		if (OMX_ErrorNone != ipCompHandle->SetConfig(ipCompHandle, 
							     audioSpecificConfigIndex, 
							     &volumeRampStruct))
			{
			StopTest(KErrGeneral, EFail);
			}
		INFO_PRINTF1(_L("Volume ramp set"));
		}

	TPtr8 ptrData(ipBufferHeaderInput->pBuffer, ipBufferHeaderInput->nAllocLen, ipBufferHeaderInput->nAllocLen);
	ipTestFile->ReadNextBuffer(ptrData);
	
	if (ptrData.Length() > 0)
		{
		// Send a buffer to the input port
		ipBufferHeaderInput->nFilledLen = ptrData.Length();
		
		if (OMX_ErrorNone != ipCompHandle->EmptyThisBuffer(
				ipCompHandle, ipBufferHeaderInput))
			{
			StopTest(KErrGeneral, EFail);
			}
		}
	else
		{
		TCallBack callback03 (RAsyncTestStepOmxILPcmRenderer0005Step03, this);
		delete ipKickoff03;
		ipKickoff03 = NULL;
		ipKickoff03 =
			new (ELeave) CAsyncCallBack (callback03, CActive::EPriorityLow);
		ipKickoff03->Call();
		}
	}
