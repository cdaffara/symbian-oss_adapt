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

#include "log.h"
#include <openmax/il/common/omxilclientclockport.h>
#include <openmax/il/common/omxilconfigmanager.h>
#include <openmax/il/common/omxilspecversion.h>
#include <openmax/il/loader/omxilsymbiancomponentif.h>

#include "omxilmicsource.h"
#include "omxilmicsource.hrh"
#include "omxilmicsourceconst.h"
#include "omxilmicsourceapb0port.h"
#include "omxilmicsourceprocessingfunction.h"

OMXIL_COMPONENT_ECOM_ENTRYPOINT(KUidSymbianOmxILMicSource);

// Component Entry Point
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE aComponent)
	{
	// This method should be called as a result of a OMX_GetHandle call. Let's
	// return something that is consistent with the return codes allowed for
	// that API call.	
	return COmxILComponent::SymbianErrorToGetHandleError(COmxILMicSource::CreateComponent(aComponent));
	}


TInt
COmxILMicSource::CreateComponent(OMX_HANDLETYPE aComponent)
	{
    DEBUG_PRINTF(_L8("COmxILMicSource::CreateComponent"));

	COmxILMicSource* self = new COmxILMicSource();

	if (!self)
		{
		return KErrNoMemory;
		}

	TRAPD(err, self->ConstructL(aComponent));
	if (err != KErrNone)
		{
		delete self;
		}
	return err;

	}

void
COmxILMicSource::ConstructL(OMX_HANDLETYPE aComponent)
	{
    DEBUG_PRINTF(_L8("COmxILMicSource::ConstructL"));

	// STEP 1: Initialize the data received from the IL Core
	COmxILComponent::ConstructL(aComponent);
	
	// STEP 2: Create the call backs manager...
	MOmxILCallbackNotificationIf* callbackNotificationIf=CreateCallbackManagerL(COmxILComponent::EOutofContext);

	// STEP 3: Create the audio capturer-specific Processing Function...
	// Create the clock port first
	COmxILClientClockPort*opb0Port = ConstructOPB0PortL();
	CleanupStack::PushL(opb0Port);
	
	COmxILProcessingFunction* pProcessingFunction = 
	        COmxILMicSourceProcessingFunction::NewL(*callbackNotificationIf, *opb0Port);
    RegisterProcessingFunction(pProcessingFunction); 

	// STEP 4: Create Port manager...
	CreatePortManagerL(COmxILComponent::ENonBufferSharingPortManager,
			iOmxILVersion,      // Component's OMX Version
			1,					// The number of audio ports in this component
			0,					// The starting audio port index
			0,					// The number of image ports in this component
			0,					// The starting image port index
			0,					// The number of video ports in this component
			0,					// The starting video port index
			1,					// The number of other ports in this component
			1					// The starting other port index
		);

	// STEP 5: Create audio capturer component port
	COmxILMicSourceAPB0Port* apb0Port = ConstructAPB0PortL();
	CleanupStack::PushL(apb0Port);
	
	// ..  and add them to the port manager...
	User::LeaveIfError(AddPort(apb0Port, OMX_DirOutput));
	CleanupStack::Pop();
	User::LeaveIfError(AddPort(opb0Port, OMX_DirInput));
	CleanupStack::Pop();
	
	// STEP 6: Create the non-port related configuration manager...
	RPointerArray<TDesC8> componentRoles;
	CleanupClosePushL(componentRoles);
	componentRoles.AppendL(&KSymbianOmxILMicSourceRole());

	COmxILConfigManager* pConfigManager  = COmxILConfigManager::NewL(
											KSymbianOmxILMicSourceName,
											TOmxILVersion(KMicSourceComponentVersionMajor,
														  KMicSourceComponentVersionMinor,
														  KMicSourceComponentVersionRevision,
														  KMicSourceComponentVersionStep),
											componentRoles);
	RegisterConfigurationManager(pConfigManager);

	CleanupStack::PopAndDestroy();	// componentRoles

	// And finally, let's get everything started
	InitComponentL();

	}

COmxILMicSource::COmxILMicSource()
	:
	iOmxILVersion(TOmxILSpecVersion())
	{
    DEBUG_PRINTF(_L8("COmxILMicSource::COmxILMicSource"));
	}

COmxILMicSource::~COmxILMicSource()
	{
    DEBUG_PRINTF(_L8("COmxILMicSource::~COmxILMicSource"));
	}

COmxILMicSourceAPB0Port*
COmxILMicSource::ConstructAPB0PortL() const
	{
    DEBUG_PRINTF(_L8("COmxILMicSource::ConstructAPB0PortL"));

	RArray<OMX_AUDIO_CODINGTYPE> supportedAudioFormats;
	CleanupClosePushL(supportedAudioFormats);
	supportedAudioFormats.AppendL(OMX_AUDIO_CodingPCM);

	OMX_AUDIO_PARAM_PCMMODETYPE	paramPcmModeType;
	paramPcmModeType.nSize				= sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
	paramPcmModeType.nVersion			= iOmxILVersion;
	paramPcmModeType.nPortIndex			= KMICSOURCE_APB0PORT_INDEX;

	static_cast<COmxILMicSourceProcessingFunction*>(GetProcessingFunction())->FillParamPCMModeType(paramPcmModeType);

	OMX_AUDIO_CONFIG_VOLUMETYPE	configAudioVolume;
	configAudioVolume.nSize			 = sizeof(OMX_AUDIO_CONFIG_VOLUMETYPE);
	configAudioVolume.nVersion		 = iOmxILVersion;
	configAudioVolume.nPortIndex	 = KMICSOURCE_APB0PORT_INDEX;
	configAudioVolume.bLinear		 = OMX_TRUE;
	configAudioVolume.sVolume.nValue = static_cast<COmxILMicSourceProcessingFunction*>(GetProcessingFunction())->GetVolume();
	configAudioVolume.sVolume.nMin	 = KMICSOURCE_VOLUME_MIN;
	configAudioVolume.sVolume.nMax	 = KMICSOURCE_VOLUME_MAX;

	OMX_AUDIO_CONFIG_MUTETYPE	configAudioMute;
    configAudioMute.nSize	   = sizeof(OMX_AUDIO_CONFIG_MUTETYPE);
	configAudioMute.nVersion   = iOmxILVersion;
	configAudioMute.nPortIndex = KMICSOURCE_APB0PORT_INDEX;
	configAudioMute.bMute	   = OMX_FALSE;

	COmxILMicSourceAPB0Port* apb0Port = COmxILMicSourceAPB0Port::NewL(
		TOmxILCommonPortData(
			iOmxILVersion,				// OMX specification version information
			KMICSOURCE_APB0PORT_INDEX,	// Port number the structure applies to
			OMX_DirOutput,				// Direction of this port
			KMICSOURCE_PCMPORT_BUFFERCOUNT_MIN,	// The minimum number of buffers this port requires
			KMICSOURCE_PCMPORT_BUFFERSIZE_MIN,  // Minimum size, in bytes, for buffers to be used for this port
			OMX_PortDomainAudio,		// Domain of the port
			OMX_FALSE,					// Buffers contiguous requirement (true or false)
			0,							// Buffer aligment requirements
			OMX_BufferSupplyOutput,	// supplier preference when tunneling between two ports
			0),
		supportedAudioFormats,
		paramPcmModeType,
		configAudioVolume,
		configAudioMute,
		static_cast<COmxILMicSourceProcessingFunction&>(*(GetProcessingFunction()))
		);

	CleanupStack::PopAndDestroy(&supportedAudioFormats);
	return apb0Port;
	}

COmxILClientClockPort*
COmxILMicSource::ConstructOPB0PortL() const
	{
    DEBUG_PRINTF(_L8("COmxILMicSource::ConstructOPB0PortL"));

    TOmxILCommonPortData portData(
		iOmxILVersion,
		KMICSOURCE_OPB0PORT_INDEX,		// port index
		OMX_DirInput,
		KMICSOURCE_CLOCKPORT_BUFFERCOUNT_MIN,// minimum number of buffers
		sizeof(OMX_TIME_MEDIATIMETYPE), // minimum buffer size, in bytes
		OMX_PortDomainOther,
		OMX_FALSE,				// do not need contigious buffers
		0,						// byte alignment
		OMX_BufferSupplyUnspecified,
		COmxILPort::KBufferMarkPropagationPortNotNeeded
		);

	// TODO: Clean up the code below. The supportedOtherFormats variable has been added in order to make
	// this component compilable during integration from the MM projectintegration branch .
	RArray<OMX_OTHER_FORMATTYPE> supportedOtherFormats;
	CleanupClosePushL(supportedOtherFormats);
	supportedOtherFormats.AppendL(OMX_OTHER_FormatTime);
	COmxILClientClockPort* aClientClockPort = COmxILClientClockPort::NewL(portData, supportedOtherFormats);
	CleanupStack::PopAndDestroy(&supportedOtherFormats);
	
	return aClientClockPort;
	}
