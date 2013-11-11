/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <openmax/il/common/omxilconfigmanager.h>
#include <openmax/il/common/omxilclientclockport.h>
#include <openmax/il/loader/omxilsymbiancomponentif.h>
#include "log.h"
#include "omxilpcmrenderer.h"
#include "omxilpcmrendererapb0port.h"
#include "omxilpcmrenderer.hrh"


const TUint8 COmxILPcmRenderer::iComponentVersionMajor;
const TUint8 COmxILPcmRenderer::iComponentVersionMinor;
const TUint8 COmxILPcmRenderer::iComponentVersionRevision;
const TUint8 COmxILPcmRenderer::iComponentVersionStep;


OMXIL_COMPONENT_ECOM_ENTRYPOINT(KUidSymbianOmxILPcmRenderer);

// Component Entry Point
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE aComponent)
	{
	// This method should be called as a result of a OMX_GetHandle call. Let's
	// return something that is consistent with the return codes allowed for
	// that API call.	
	return COmxILComponent::SymbianErrorToGetHandleError(COmxILPcmRenderer::CreateComponent(aComponent));
	}


TInt COmxILPcmRenderer::CreateComponent(OMX_HANDLETYPE aComponent)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRenderer::CreateComponent"));

	COmxILPcmRenderer* self = new COmxILPcmRenderer();

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

void COmxILPcmRenderer::ConstructL(OMX_HANDLETYPE aComponent)
	{
    DEBUG_PRINTF(_L8("COmxILPcmRenderer::ConstructL"));

	// STEP 1: Initialize the data received from the IL Core
	COmxILComponent::ConstructL(aComponent);

	// STEP 2: Create the call backs manager...
	MOmxILCallbackNotificationIf* callbackNotificationIf(0);
#ifdef _OMXIL_PCMRENDERER_INCONTEXT_CALLBACKMANAGER_ON
	callbackNotificationIf=CreateCallbackManagerL(COmxILComponent::EInContext);
#else
	callbackNotificationIf=CreateCallbackManagerL(COmxILComponent::EOutofContext);
#endif

	// STEP 3: Create the PCM renderer-specific Processing Function...

	// ...create PCM renderer component Client Clock port
	COmxILClientClockPort* clientClockPort = ConstructClientClockPortL();
	CleanupStack::PushL(clientClockPort);
	
	COmxILProcessingFunction* pProcessingFunction = COmxILPcmRendererProcessingFunction::NewL(*callbackNotificationIf, *clientClockPort);
	RegisterProcessingFunction(pProcessingFunction);
	
	// STEP 4: Create Port manager...
	CreatePortManagerL(COmxILComponent::ENonBufferSharingPortManager,
					iOmxILVersion,               // Component's OMX Version
					1,					         // The number of audio ports in this component
					KPCMRENDERER_APB0PORT_INDEX, // The starting audio port index
					0,					         // The number of image ports in this component
					0,					         // The starting image port index
					0,					         // The number of video ports in this component
					0,					         // The starting video port index
					1,					         // The number of other ports in this component
					KPCMRENDERER_OPB0PORT_INDEX	 // The starting other port index
					);
							
	// ...create PCM renderer component ports (no OPB0 port)
	COmxILPcmRendererProcessingFunction* ptr = static_cast<COmxILPcmRendererProcessingFunction*>(pProcessingFunction);
	COmxILPcmRendererAPB0Port* pb0Port = ConstructAPB0PortL(*ptr);
	CleanupStack::PushL(pb0Port);

	// ..  and add them to the port manager...
	User::LeaveIfError(AddPort(pb0Port, OMX_DirInput));
	CleanupStack::Pop(); //pb0Port

    User::LeaveIfError(AddPort(clientClockPort, OMX_DirInput));
    CleanupStack::Pop(); //clientClockPort

	
	// STEP 5: Create the non-port related configuration manager...
	RPointerArray<TDesC8> componentRoles;
	CleanupClosePushL(componentRoles);
	User::LeaveIfError(componentRoles.Append(&KSymbianOmxILPcmRendererRole()));

	COmxILConfigManager* pConfigManager = COmxILConfigManager::NewL(
		KSymbianOmxILPcmRendererName,
		TOmxILVersion(iComponentVersionMajor,
					  iComponentVersionMinor,
					  iComponentVersionRevision,
					  iComponentVersionStep),
		componentRoles);
		
	RegisterConfigurationManager(pConfigManager);
	
	CleanupStack::Pop();	// componentRoles
	componentRoles.Close(); // Must not destroy pointers

	// And finally, let's get everything started
	InitComponentL();
	}

COmxILPcmRenderer::COmxILPcmRenderer()
	:
	iOmxILVersion(TOmxILSpecVersion())
	{
    DEBUG_PRINTF(_L8("COmxILPcmRenderer::COmxILPcmRenderer"));
	}

COmxILPcmRenderer::~COmxILPcmRenderer()
	{
    DEBUG_PRINTF(_L8("COmxILPcmRenderer::~COmxILPcmRenderer"));
	}


COmxILPcmRendererAPB0Port* COmxILPcmRenderer::ConstructAPB0PortL(COmxILPcmRendererProcessingFunction& aProcessingFunction) const
	{
    DEBUG_PRINTF(_L8("COmxILPcmRenderer::ConstructAPB0PortL"));

	RArray<OMX_AUDIO_CODINGTYPE> supportedAudioFormats;
	CleanupClosePushL(supportedAudioFormats);
	supportedAudioFormats.AppendL(OMX_AUDIO_CodingPCM);

	OMX_AUDIO_PARAM_PCMMODETYPE	paramPcmModeType;
	paramPcmModeType.nSize				= sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
	paramPcmModeType.nVersion			= iOmxILVersion;
	paramPcmModeType.nPortIndex			= KPCMRENDERER_APB0PORT_INDEX;
	paramPcmModeType.nChannels			= KDefaultNumberChannels;
	paramPcmModeType.eNumData			= OMX_NumericalDataSigned;
	paramPcmModeType.eEndian			= OMX_EndianBig;
	paramPcmModeType.bInterleaved		= OMX_TRUE;
	paramPcmModeType.nBitPerSample		= KDefaultBitPerSample;
	paramPcmModeType.nSamplingRate		= KDefaultSampleRate;
	paramPcmModeType.ePCMMode			= OMX_AUDIO_PCMModeLinear;
	paramPcmModeType.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	paramPcmModeType.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

	OMX_AUDIO_CONFIG_VOLUMETYPE	configAudioVolume;
	configAudioVolume.nSize			 = sizeof(OMX_AUDIO_CONFIG_VOLUMETYPE);
	configAudioVolume.nVersion		 = iOmxILVersion;
	configAudioVolume.nPortIndex	 = KPCMRENDERER_APB0PORT_INDEX;
	configAudioVolume.bLinear		 = OMX_FALSE;
	configAudioVolume.sVolume.nValue = 50;
	configAudioVolume.sVolume.nMin	 = 0;
	configAudioVolume.sVolume.nMax	 = 100;

	OMX_AUDIO_CONFIG_MUTETYPE	configAudioMute;
    configAudioMute.nSize	   = sizeof(OMX_AUDIO_CONFIG_MUTETYPE);
	configAudioMute.nVersion   = iOmxILVersion;
	configAudioMute.nPortIndex = KPCMRENDERER_APB0PORT_INDEX;
	configAudioMute.bMute	   = OMX_FALSE;
	TOmxILCommonPortData portData(iOmxILVersion,				// OMX specification version information
                                    KPCMRENDERER_APB0PORT_INDEX,// Port number the structure applies to
                                    OMX_DirInput,				// Direction of this port
                                    1,							// The minimum number of buffers this port requires
                                    15360,						// Minimum size, in bytes, for buffers to be used for this port
                                    OMX_PortDomainAudio,		// Domain of the port
                                    OMX_FALSE,					// Buffers contiguous requirement (true or false)
                                    0,							// Buffer aligment requirements
                                    OMX_BufferSupplyInput,     	// supplier preference when tunneling between two ports
                                    // This component is a sink component so there's no need to
                                    // propagate buffer marks received on input ports
                                    COmxILPort:: KBufferMarkPropagationPortNotNeeded);
	
	COmxILPcmRendererAPB0Port* apb0Port = COmxILPcmRendererAPB0Port::NewL(portData,
                                                                        supportedAudioFormats,
                                                                        paramPcmModeType,
                                                                        configAudioVolume,
									configAudioMute, 
									aProcessingFunction);
	
	CleanupStack::PopAndDestroy(&supportedAudioFormats);
	return apb0Port;
	}

COmxILClientClockPort* COmxILPcmRenderer::ConstructClientClockPortL() const
	{
    DEBUG_PRINTF(_L8("COmxILPcmRenderer::ConstructClientClockPortL"));
    
    TOmxILCommonPortData portData(iOmxILVersion, 
                                    KPCMRENDERER_OPB0PORT_INDEX,	// port index
                                    OMX_DirInput,
                                    4,								// minimum number of buffers
                                    sizeof(OMX_TIME_MEDIATIMETYPE),	// minimum buffer size, in bytes
                                    OMX_PortDomainOther,
                                    OMX_FALSE,						// do not need contigious buffers
                                    0,								// byte alignment
                                    OMX_BufferSupplyUnspecified, 
                                    COmxILPort::KBufferMarkPropagationPortNotNeeded);
	
	RArray<OMX_OTHER_FORMATTYPE> supportedOtherFormats;
	CleanupClosePushL(supportedOtherFormats);
	supportedOtherFormats.AppendL(OMX_OTHER_FormatTime);
	COmxILClientClockPort* aClientClockPort = COmxILClientClockPort::NewL(portData, supportedOtherFormats);
	CleanupStack::PopAndDestroy(&supportedOtherFormats);

	return aClientClockPort;
	}
