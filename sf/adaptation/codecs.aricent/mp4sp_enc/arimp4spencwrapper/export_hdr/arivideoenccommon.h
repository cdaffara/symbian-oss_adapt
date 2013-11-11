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

//Includes
#include <E32def.h>
#include <Devvideobase.h>
#include <Mpeg4Visual.h>

#define MAX_SCALABILITY_LAYERS 1

enum TSetCommands
{
	CONTROL_CMD_SET_BIT_RATE = 2051,
	CONTROL_CMD_SET_FRAME_RATE,
	CONTROL_CMD_SET_FORCED_I_FRAME,
	CONTROL_CMD_SET_GOVLENGTH,
	CONTROL_CMD_SET_CHANNEL_PACKET_LOSS_RATE,
	CONTROL_CMD_SET_CHANNEL_BIT_ERROR_RATE,
	CONTROL_CMD_SET_SEGMENT_TARGET_SIZE,
	CONTROL_CMD_SET_RATE_CONTROL_OPTIONS,
	CONTROL_CMD_SET_COMMIT_OPTIONS,
	CONTROL_CMD_GET_CONFIG_HEADER,
	CONTROL_CMD_SET_COMPLEXITY_LEVEL,
	CONTROL_CMD_SET_SLICELOSS,
    CONTROL_CMD_SET_PACKETSIZE
};

enum TGetCommands
{

	CONTROL_CMD_GET_MAXBUFFERLENGTH = 3051,
	CONTROL_CMD_GET_NUMOFPICTSKIPPED,
	CONTROL_CMD_GET_MAXNUMOFPACKETS,
	CONTROL_CMD_GET_NUMOFPACKETS,
	CONTROL_CMD_GET_PACKETBOUNDARYDATA,
	CONTROL_CMD_GET_PACKETSIZE,
	CONTROL_CMD_GET_CONFIGDATA,
	CONTROL_CMD_GET_CONFIGDATALENGTH
};



/*
 * Enumeration covering SetInputFormatL, SetOutputFormatL, SetInputDevice, SetNumBitrateLayersL,
 * SetScalabilityLayerTypeL, SetGlobalReferenceOptions, SetBufferOptionsL, SetMinRandomAccessRate
 * SetSourceMemoryL of Hw Device. All these seven methods can be called only before
 * Initialize.
*/

enum TOutputFormat
    {
	EH263		=	0x00000000,
	EMpeg4      =   0x00000001,
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

class TMpeg4H263EncoderInitParams
    {
public:
	// Specifies which parameters are being sent in the Initialize method

    TUint32         	        iBeforeInitialize;
	TUint32						iAfterInitialize;

	TSize						iPictureSize;
	TInt 						iVopRate;
	TInt						iNumOfGOBHdrs;
	TInt						iGOVLength;
	TInt						iSceneCutDetection;
	TInt						iBandwidthAdaptation;
	TInt						iPreprocessing;
	TInt						iRCModel;
	TInt						iSearchRange;
	TUint32						iTimerResolution;
	TInt						iPacketmode;
	TInt						iPacketSize;
	TInt						iReversibleVLC;
	TInt						iDataPartitioning;
	TInt						iMAPS;

	// For SetOutputFormatL
	TOutputFormat               iOutputFormat;
	TVideoDataUnitType			iDataUnitType;
	TVideoDataUnitEncapsulation iDataEncapsulation;
    TBool						iSegmentationAllowed;

    // For SetSourceMemoryL
    TReal                       iMaxPictureRate;
    TBool                       iConstantPictureRate;
    TBool                       iProcessRealtime;

    // For SetRamdomAccessRate
    TReal                       iRandomAccessRate;

	// For SetErrorsExpected
	TBool iBitErrors;
	TBool iPacketLosses;

	// For SetMinRandomAccessRate
	TInt  iLevel;

	TUint iBitRate;
    TReal iTargetPictureRate;
	// complexity level
	TUint iComplexityLevel;

	TUncompressedVideoFormat	iInputFormat;
	TInt32						iAspectRatio;
    TUint                       iNumInputBuffers;
    TSize                       iInputSize;
	// For SetNumBitrateLayersL
	TUint						iNumBitRateLayers;

	// For SetScalabilityLayerTypeL
	TUint						iLayer;
	TScalabilityType			iScalabilityType;
	TH264EncLayerReferenceOptions		iLayerReferenceOptions[MAX_SCALABILITY_LAYERS];

	// For SetBufferOptionsL
	TUint iMaxPreEncoderBufferPictures;
	THrdVbvSpecification iHrdVbvSpec;
	T3gppHrdVbvParams iHrdVbvParams;
	TUint iMaxOutputBufferSize;
	TUint iMaxCodedPictureSize;
	TUint iMaxCodedSegmentSize;
	TUint iMinNumOutputBuffers;
	TMPEG4VisualMode		iCodingStandardSpecificOptions;
	TRect	iOutputRect;

    };

/*
 * slice loss structure
*/
class TMPEG4H263EncSliceLoss
{
public:
		TUint iFirstMacroblock;
		TUint iNumMacroblocks;

		TMPEG4H263EncSliceLoss() :
		iFirstMacroblock (0),
		iNumMacroblocks (0)
	{
	}
};

/*
 *	Parameters used by the hw device which are sent to codec
*/




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
