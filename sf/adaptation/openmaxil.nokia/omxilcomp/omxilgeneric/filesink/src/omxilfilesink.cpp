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



#include <openmax/il/common/omxilport.h>
#include <openmax/il/common/omxilspecversion.h>
#include <openmax/il/loader/omxilsymbiancomponentif.h>

#include "omxilfilesink.h"
#include "omxilfilesinkprocessingfunction.h"
#include "omxilfilesinkconfigmanager.h"
#include "omxilfilesink.hrh"


#ifdef OMXIL_AUDIO_FILESINK
#include "omxilaudiofilesinkapb0port.h"
_LIT8(KNokiaOMXFileSinkComponentName, "OMX.NOKIA.AUDIO.FILESINK");
_LIT8(KNokiaOMXFileSinkRole, "audio_writer.binary");
OMXIL_COMPONENT_ECOM_ENTRYPOINT(KUidNokiaOmxILAudioFileSink);

#elif defined(OMXIL_VIDEO_FILESINK)
#include "omxilvideofilesinkvpb0port.h"
_LIT8(KNokiaOMXFileSinkComponentName, "OMX.NOKIA.VIDEO.FILESINK");
_LIT8(KNokiaOMXFileSinkRole, "video_writer.binary");
OMXIL_COMPONENT_ECOM_ENTRYPOINT(KUidNokiaOmxILVideoFileSink);

#elif defined(OMXIL_IMAGE_FILESINK)
#include "omxilimagefilesinkipb0port.h"
_LIT8(KNokiaOMXFileSinkComponentName, "OMX.NOKIA.IMAGE.FILESINK");
_LIT8(KNokiaOMXFileSinkRole, "image_writer.binary");
OMXIL_COMPONENT_ECOM_ENTRYPOINT(KUidNokiaOmxILImageFileSink);

#elif defined(OMXIL_OTHER_FILESINK)
#include "omxilotherfilesinkopb0port.h"
_LIT8(KNokiaOMXFileSinkComponentName, "OMX.NOKIA.OTHER.FILESINK");
_LIT8(KNokiaOMXFileSinkRole, "other_writer.binary");
OMXIL_COMPONENT_ECOM_ENTRYPOINT(KUidSymbianOmxILOtherFileSink);

#endif

const TUint8 KComponentVersionMajor = 1;
const TUint8 KComponentVersionMinor = 1;
const TUint8 KComponentVersionRevision = 0;
const TUint8 KComponentVersionStep = 0;

static const TInt KMinBuffers = 1;
static const TInt KMinBufferSize = 15360;


// Component Entry Point
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE aComponent)
	{
	TRAPD(err, COmxILFileSink::CreateComponentL(aComponent));
	if (err == KErrNone)
		{
		return OMX_ErrorNone;
		}
	else
		{
		return err == KErrNoMemory ? OMX_ErrorInsufficientResources : OMX_ErrorUndefined;
		}
	}

void COmxILFileSink::CreateComponentL(OMX_HANDLETYPE aComponent)
	{
	COmxILFileSink* self = new (ELeave) COmxILFileSink();
	CleanupStack::PushL(self);
	self->ConstructL(aComponent);
	CleanupStack::Pop(self);
	}

COmxILFileSink::COmxILFileSink()
	{
	// nothing to do
	}

COmxILFileSink::~COmxILFileSink()
	{
	}

void COmxILFileSink::ConstructL(OMX_HANDLETYPE aComponent)
	{
    // STEP 1: Initialize the data received from the IL Core
    COmxILComponent::ConstructL(aComponent);
    
    // STEP 2: Create the call backs manager...
    MOmxILCallbackNotificationIf* callbackNotificationIf=CreateCallbackManagerL(COmxILComponent::EOutofContext);

	// STEP 3: Create the file sink-specific Processing Function...
    COmxILFileSinkProcessingFunction* pProcessingFunction = COmxILFileSinkProcessingFunction::NewL(*callbackNotificationIf);
    RegisterProcessingFunction(pProcessingFunction);
    
	// STEP 4: Create Port manager...

#ifdef OMXIL_AUDIO_FILESINK
    CreatePortManagerL(COmxILComponent::ENonBufferSharingPortManager,
        TOmxILSpecVersion(),    // OMX Version
        1,                      // The number of audio ports in this component
        0,                      // The starting audio port index
        0,                      // The number of image ports in this component
        0,                      // The starting image port index
        0,                      // The number of video ports in this component
        0,                      // The starting video port index
        0,                      // The number of other ports in this component
        0                       // The starting other port index
        );

#elif defined(OMXIL_VIDEO_FILESINK)
    CreatePortManagerL(COmxILComponent::ENonBufferSharingPortManager,
        TOmxILSpecVersion(),    // OMX Version
        0,                      // The number of audio ports in this component
        0,                      // The starting audio port index
        0,                      // The number of image ports in this component
        0,                      // The starting image port index
        1,                      // The number of video ports in this component
        0,                      // The starting video port index
        0,                      // The number of other ports in this component
        0                       // The starting other port index
        );

#elif defined(OMXIL_IMAGE_FILESINK)
    CreatePortManagerL(COmxILComponent::ENonBufferSharingPortManager,
        TOmxILSpecVersion(),    // OMX Version
        0,                      // The number of audio ports in this component
        0,                      // The starting audio port index
        1,                      // The number of image ports in this component
        0,                      // The starting image port index
        0,                      // The number of video ports in this component
        0,                      // The starting video port index
        0,                      // The number of other ports in this component
        0                       // The starting other port index
        );


#elif defined(OMXIL_OTHER_FILESINK)
    CreatePortManagerL(COmxILComponent::ENonBufferSharingPortManager,
        TOmxILSpecVersion(),    // OMX Version
        0,                      // The number of audio ports in this component
        0,                      // The starting audio port index
        0,                      // The number of image ports in this component
        0,                      // The starting image port index
        0,                      // The number of video ports in this component
        0,                      // The starting video port index
        1,                      // The number of other ports in this component
        0                       // The starting other port index
        );
#endif

	// STEP 5: Create the File Sink component port...
    COmxILPort* pPort = ConstructPortL();
	CleanupStack::PushL(pPort);
	
	// STEP 6: Add to the port manager...
	User::LeaveIfError(AddPort(pPort, OMX_DirInput));
	CleanupStack::Pop(); //pPort
	
	// STEP 7: Create the non-port related configuration manager...
	RPointerArray<TDesC8> componentRoles;
	CleanupClosePushL(componentRoles);

	componentRoles.AppendL(&KNokiaOMXFileSinkRole);
	COmxILFileSinkConfigManager* pConfigManager = COmxILFileSinkConfigManager::NewL(
		KNokiaOMXFileSinkComponentName,
		TOmxILVersion(KComponentVersionMajor,
					  KComponentVersionMinor,
					  KComponentVersionRevision,
					  KComponentVersionStep),
		componentRoles,
		*pProcessingFunction);
    RegisterConfigurationManager(pConfigManager);
	CleanupStack::PopAndDestroy(&componentRoles);

	// STEP 9: Finally, let's get everything started
	InitComponentL();
	}

COmxILPort* COmxILFileSink::ConstructPortL() const
	{
	OMX_U32 thisPortIndex = 0;
	//const TUint32 KBufferAlignment = 4;
#ifdef OMXIL_AUDIO_FILESINK
	RArray<OMX_AUDIO_CODINGTYPE> supportedAudioFormats;
	CleanupClosePushL(supportedAudioFormats);
	supportedAudioFormats.AppendL(OMX_AUDIO_CodingUnused);
	COmxILAudioFileSinkAPB0Port* pPort = COmxILAudioFileSinkAPB0Port::NewL(
			TOmxILCommonPortData (
			TOmxILSpecVersion(),	// OMX specification version information
			thisPortIndex, 			// Port number the structure applies to
			OMX_DirInput, 			// Direction of this port
			KMinBuffers,			// The minimum number of buffers this port requires
			KMinBufferSize,			// Minimum size, in bytes, for buffers to be used for this port
			OMX_PortDomainAudio,	// Domain of the port
			OMX_FALSE,				// Buffers contiguous requirement (true or false)
			0,		                // Buffer aligment requirements
			OMX_BufferSupplyOutput,	// supplier preference when tunneling between two ports
			COmxILPort::KBufferMarkPropagationPortNotNeeded),
			supportedAudioFormats,
			static_cast<COmxILFileSinkProcessingFunction&>(*GetProcessingFunction()));
	CleanupStack::PopAndDestroy(&supportedAudioFormats);
	return pPort;

#elif defined(OMXIL_VIDEO_FILESINK)
    RArray<OMX_VIDEO_CODINGTYPE> supportedVideoFormats;
    CleanupClosePushL(supportedVideoFormats);
    RArray<OMX_COLOR_FORMATTYPE> supportedColourFormats;
    CleanupClosePushL(supportedColourFormats);
    COmxILVideoFileSinkVPB0Port* pPort = COmxILVideoFileSinkVPB0Port::NewL(
            TOmxILCommonPortData (
            TOmxILSpecVersion(),    // OMX specification version information
            thisPortIndex,          // Port number the structure applies to
            OMX_DirInput,           // Direction of this port
            KMinBuffers,            // The minimum number of buffers this port requires
            KMinBufferSize,         // Minimum size, in bytes, for buffers to be used for this port
            OMX_PortDomainVideo,    // Domain of the port
            OMX_FALSE,              // Buffers contiguous requirement (true or false)
            0,                      // Buffer aligment requirements
            OMX_BufferSupplyOutput, // supplier preference when tunneling between two ports
            COmxILPort::KBufferMarkPropagationPortNotNeeded),
            supportedVideoFormats,
            supportedColourFormats,
            static_cast<COmxILFileSinkProcessingFunction&>(*GetProcessingFunction()));
    CleanupStack::PopAndDestroy(2);
    return pPort;

#elif defined(OMXIL_IMAGE_FILESINK)
    RArray<OMX_IMAGE_CODINGTYPE> supportedImageFormats;
    CleanupClosePushL(supportedImageFormats);
    //supportedImageFormats.AppendL(OMX_IMAGE_CodingUnused);
    RArray<OMX_COLOR_FORMATTYPE> supportedColourFormats;
    CleanupClosePushL(supportedColourFormats);
    COmxILImageFileSinkIPB0Port* pPort = COmxILImageFileSinkIPB0Port::NewL(
            TOmxILCommonPortData (
            TOmxILSpecVersion(),    // OMX specification version information
            thisPortIndex,          // Port number the structure applies to
            OMX_DirInput,           // Direction of this port
            KMinBuffers,            // The minimum number of buffers this port requires
            KMinBufferSize,         // Minimum size, in bytes, for buffers to be used for this port
            OMX_PortDomainImage,    // Domain of the port
            OMX_FALSE,              // Buffers contiguous requirement (true or false)
            0,                      // Buffer aligment requirements
            OMX_BufferSupplyOutput, // supplier preference when tunneling between two ports
            COmxILPort::KBufferMarkPropagationPortNotNeeded),
            supportedImageFormats,
            supportedColourFormats,
            static_cast<COmxILFileSinkProcessingFunction&>(*GetProcessingFunction()));
    CleanupStack::PopAndDestroy(2);
    return pPort;

#elif defined(OMXIL_OTHER_FILESINK)
	RArray<OMX_OTHER_FORMATTYPE> supportedOtherFormats;
	CleanupClosePushL(supportedOtherFormats);
	supportedOtherFormats.AppendL(OMX_OTHER_FormatBinary);
	COmxILOtherFileSinkOPB0Port* pPort = COmxILOtherFileSinkOPB0Port::NewL(
			TOmxILCommonPortData (
			TOmxILSpecVersion(),	// OMX specification version information
			thisPortIndex, 			// Port number the structure applies to
			OMX_DirInput, 			// Direction of this port
			KMinBuffers,			// The minimum number of buffers this port requires
			KMinBufferSize,			// Minimum size, in bytes, for buffers to be used for this port
			OMX_PortDomainOther,	// Domain of the port
			OMX_FALSE,				// Buffers contiguous requirement (true or false)
			0,		                // Buffer aligment requirements
			OMX_BufferSupplyOutput,	// supplier preference when tunneling between two ports
			COmxILPort::KBufferMarkPropagationPortNotNeeded),
			supportedOtherFormats,
			static_cast<COmxILFileSinkProcessingFunction&>(*GetProcessingFunction()));
	CleanupStack::PopAndDestroy(&supportedOtherFormats);
	return pPort;

#endif
	}
