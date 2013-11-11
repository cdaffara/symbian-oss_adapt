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

#include <openmax/il/common/omxilspecversion.h>
#include <openmax/il/loader/omxilcomponentif.h>
#include <openmax/il/loader/omxilsymbiancomponentif.h>

#include "omxiltestmd5sink.h"
#include "omxiltestmd5sinkport.h"
#include "omxiltestmd5sinkprocessingfunction.h"
#include "omxiltestmd5sinkconfigmanager.h"
#include "omxiltestmd5sink.hrh"
#include "omxiltestmd5sinkconst.h"

#ifdef DOMAIN_OTHER
_LIT8(KOmxILComponentName, "OMX.NOKIA.TEST.OTHER.MD5SINK");
#endif

_LIT8(KOmxILRole, "????");

#ifdef DOMAIN_OTHER
OMXIL_COMPONENT_ECOM_ENTRYPOINT(KUidSymbianOmxILTestOtherMD5Sink);
#endif

// Component Entry Point
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE aComponent)
	{
	TInt err = COmxILTestMD5Sink::CreateComponent(aComponent);
	if (err == KErrNone)
		{
		return OMX_ErrorNone;
		}
	else
		{
		// TODO translate Symbian error code into OMX_ERRORTYPE
		return OMX_ErrorInsufficientResources;
		}
	}

TInt COmxILTestMD5Sink::CreateComponent(OMX_HANDLETYPE hComponent)
	{
	COmxILTestMD5Sink* self = new COmxILTestMD5Sink();

	if (!self)
		{
		return KErrNoMemory;
		}

	TRAPD(err, self->ConstructL(hComponent));
	return err;
	}

COmxILTestMD5Sink::COmxILTestMD5Sink()
	{
	// nothing to do
	}

void COmxILTestMD5Sink::ConstructL(OMX_HANDLETYPE hComponent)
	{
	// STEP 1: Initialize the data received from the IL Core
    COmxILComponent::ConstructL(hComponent);
    
	// STEP 2: Create the call backs holder...
    MOmxILCallbackNotificationIf* callbackNotificationIf=CreateCallbackManagerL(COmxILComponent::EOutofContext);

	// STEP 3: Create the MD5 sink-specific Processing Function...
    COmxILTestMD5SinkProcessingFunction* pProcessingFunction = COmxILTestMD5SinkProcessingFunction::NewL(*callbackNotificationIf);
    RegisterProcessingFunction(pProcessingFunction);
    
	// STEP 4: Create Port manager...
    CreatePortManagerL(COmxILComponent::ENonBufferSharingPortManager,
		TOmxILSpecVersion(),	        // OMX Version
		0,						// The number of audio ports in this component
		0,						// The starting audio port index
		0,						// The number of image ports in this component
		0,						// The starting image port index
		0,						// The number of video ports in this component
		0,						// The starting video port index
#ifdef DOMAIN_OTHER
		1,						// The number of other ports in this component
#else
		0,						// The number of other ports in this component
#endif
		KMD5SINK_OPB0PORT_INDEX		// The starting other port index
		);

	// STEP 5: Create the non-port related configuration manager...
	RPointerArray<TDesC8> roleList;
	CleanupClosePushL(roleList);
	roleList.AppendL(&KOmxILRole);
	COmxILTestMD5SinkConfigManager* apSinkConfigManager = COmxILTestMD5SinkConfigManager::NewL(KOmxILComponentName, TOmxILSpecVersion(), roleList);
	RegisterConfigurationManager(apSinkConfigManager);
	CleanupStack::PopAndDestroy(&roleList);

	static_cast<COmxILTestMD5SinkProcessingFunction*>(GetProcessingFunction())->SetConfigManager(apSinkConfigManager);
	
	// create the input port
	ConstructSinkPortL();
	CleanupStack::PushL(ipSinkPort);
	// ..  and add it to the port manager...
	User::LeaveIfError(AddPort(ipSinkPort, OMX_DirInput));
	CleanupStack::Pop();

	// And finally, let's get everything started
	InitComponentL();
	}

COmxILTestMD5Sink::~COmxILTestMD5Sink()
	{
	}

void COmxILTestMD5Sink::ConstructSinkPortL()
	{
	TOmxILCommonPortData aPortData(
			TOmxILSpecVersion(),
			KMD5SINK_OPB0PORT_INDEX, 
			OMX_DirInput, 
			KMinBuffers,
			KMinBufferSize,
#ifdef DOMAIN_OTHER
			OMX_PortDomainOther,
#endif
			OMX_FALSE,
			0,
			OMX_BufferSupplyInput,
			COmxILPort::KBufferMarkPropagationPortNotNeeded);

	ipSinkPort = COmxILTestMD5SinkPort::NewL(aPortData);
	}
