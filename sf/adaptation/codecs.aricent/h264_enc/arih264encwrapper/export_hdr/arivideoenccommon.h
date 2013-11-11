/*
* Copyright (c) 2009 Aricent and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Aricent - initial contribution.
*
* Contributors:
*
* Description:
* Defines the structures shared by HwDevice and wrapper
*
*/

#ifndef ARIVIDEOENCCOMMON_H
#define ARIVIDEOENCCOMMON_H

#include <e32def.h>
#include <devvideobase.h>
#include <avc.h>

#define MAX_SCALABILITY_LAYERS 1


//Control SUB MEssages
enum TSetCommands
	{
	CONTROL_CMD_SET_BIT_RATE =	2051,
	CONTROL_CMD_SET_FRAME_RATE = 2052,
	CONTROL_CMD_SET_FORCED_I_FRAME = 2053,
	CONTROL_CMD_SET_GOVLENGTH = 2054,
	CONTROL_CMD_SET_CHANNEL_PACKET_LOSS_RATE = 2059,
	CONTROL_CMD_SET_CHANNEL_BIT_ERROR_RATE = 2060,
	CONTROL_CMD_SET_SEGMENT_TARGET_SIZE = 2061,
	CONTROL_CMD_SET_RATE_CONTROL_OPTIONS = 2062,
	CONTROL_CMD_SET_COMMIT_OPTIONS = 2065,
	CONTROL_CMD_SET_COMPLEXITY_LEVEL = 2068,
	CONTROL_CMD_SET_SLICELOSS  = 2069,
	CONTROL_CMD_SET_PACKET_SIZE = 2070,
	CONTROL_CMD_SET_PACKET_MODE = 2071,
	CONTROL_CMD_SET_FRAME_MODE  = 2072
	};

enum TGetCommands
	{
	CONTROL_CMD_GET_CONFIG_HEADER = 3051,
	CONTROL_CMD_GET_PACKET_OFFSET_DATA,
	CONTROL_CMD_GET_MAX_FRAME_SIZE,
	CONTROL_CMD_GET_MAX_NUM_PACKETS,
	CONTROL_CMD_GET_CONFIG_LENGTH,
	CONTROL_CMD_GET_PICTURES_SKIPPED
	
	
	};
	
/*
	Enumeration covering SetInputFormatL, SetOutputFormatL, SetInputDevice, 
	SetNumBitrateLayersL, SetScalabilityLayerTypeL, SetGlobalReferenceOptions,
	SetBufferOptionsL, SetMinRandomAccessRate,SetSourceMemoryL of Hw Device.
    All these seven methods can be called only before
    Initialize.
*/

enum TOutputFormat
    {
	EH264		=	0x00000001,
    };


enum TPanicCodes
	{
	EPanicArgument=1,
	EPanicInitializationPreCondViolation,
	EPanicNotSupported,
	EPanicNotPaused,
	EPanicNotFreezed,
	EPanicInvalidState,
	EPanicAlreadyStopped
	};

enum TEncBeforeInitialize
    {
    EEncBeforeInitNone     =   0x00000000,
	EEncInputFormat		=	0x00000001,
	EEncOutputFormat		=	0x00000002,
	EEncInputDevice		=	0x00000004,
	EEncNumBitrateLayers	=	0x00000008,
	EEncScalabilityLayer	=	0x00000010,
	EEncGlobalRefOptions	=	0x00000020,
	EEncBufferOptions		=	0x00000040,
    EEncRandomAccessRate   =	0x00000080,
    EEncSourceMemory       =   0x00000100,
    EEncInpBufferOptions   =   0x00000200,
	EEncErrorProtectionLevelFEC = 0x00000400,
	EEncSegmentTargetSize = 0x00000800,
	EEncCodingStandardSpecificOptions	=	0x00001000,
	EEncOutputRectSize	=	0x00002000,
	EEncErrorsExpected	=	0x00004000,
	EEncSourceCamera = 0x00008000,
	EEncComplexityLevel = 0x00010000,
	EEncLayerRefOptions	=	0x00020000,
	};

class TH264EncLayerReferenceOptions
{
public:
	TUint	iMaxReferencePictures;
	TUint	iMaxPictureOrderDelay;

	TH264EncLayerReferenceOptions () :
		iMaxReferencePictures (1),
		iMaxPictureOrderDelay (0)
	{
	}
};

/*
 * slice loss structure 
*/
class TH264EncSliceLoss
{
public:
		TUint iFirstMacroblock;
		TUint iNumMacroblocks;

	TH264EncSliceLoss() :
		iFirstMacroblock (0),
		iNumMacroblocks (0)
	{
	}
};

/* 
 *	Parameters used by the HwDevice which are sent to codec
*/

class TH264EncInitParams
    {
public:
	// Specifies which parameters are being sent in the Initialize method
    TUint32         	        iBeforeInitialize;
	TUint32						iAfterInitialize;
	
	// For SetInputFormatL
	TUncompressedVideoFormat	iInputFormat;
	TInt32						iAspectRatio;
	TSize						iPictureSize;

	// For SetOutputFormatL
	TOutputFormat               iOutputFormat;
	TVideoDataUnitType			iDataUnitType; 
	TVideoDataUnitEncapsulation iDataEncapsulation;
    TBool						iSegmentationAllowed;

    // For SetSourceMemoryL
    TReal                       iMaxPictureRate;
    TBool                       iConstantPictureRate;
    TBool                       iProcessRealtime;
    TUint                       iNumInputBuffers;
    TSize                       iInputSize;

	// For SetNumBitrateLayersL
	TUint						iNumBitRateLayers;
	
	// For SetScalabilityLayerTypeL
	TUint						iLayer;
	TScalabilityType			iScalabilityType;
	
	// For SetGlobalReferenceOptions & SetLayerReferenceOptions
	TH264EncLayerReferenceOptions	iLayerReferenceOptions[
	                             	               MAX_SCALABILITY_LAYERS];
	
	// For SetBufferOptionsL
	TUint iMaxPreEncoderBufferPictures;
	THrdVbvSpecification iHrdVbvSpec;
	T3gppHrdVbvParams iHrdVbvParams;
	TUint iMaxOutputBufferSize;
	TUint iMaxCodedPictureSize;
	TUint iMaxCodedSegmentSize;
	TUint iMinNumOutputBuffers;

	// For SetErrorProtectionLevelL
	TUint iFECStrength;

    // For SetRamdomAccessRate
    TReal                       iRandomAccessRate;

	// For SetOutputRectL
	TRect	iOutputRect;

	// For SetCodingStandardSpecificOptionsL
	TAvcVideoMode		iCodingStandardSpecificOptions;


	// For SetErrorsExpected
	TBool iBitErrors;
	TBool iPacketLosses;

	// For SetMinRandomAccessRate
	TReal iRate;
	TInt  iLevel;

	TUint iBitRate;
    TReal iTargetPictureRate; 
	// complexity level
	TUint iComplexityLevel;
	
	//for H324AnnexK VTFastCallSetUp
	TBool iVTFastCallSetUp;
    };


class TPictureCountersInfo
    {
public:
    TUint iPicturesSkippedBufferOverflow;
    TUint iPicturesSkippedProcPower;
    TUint iPicturesSkippedRateControl;
    TUint iPicturesProcessed;
    TUint iInputPictures;
    };

#endif //ARIVIDEOENCCOMMON_H
