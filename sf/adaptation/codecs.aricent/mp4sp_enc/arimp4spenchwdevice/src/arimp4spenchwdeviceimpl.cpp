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
* Implementation of member functions of Mpeg4SP/H263 encoder plugin class -
* CAriMp4spencHwDeviceImpl.
*
*/

//Includes
#include "arimp4spenchwdeviceimpl.h"
#include "arimp4sphwdeviceconstants.h"
#include "aricommon.h"

#include <E32base.h>
#include <AVC.h>
#include <H263.h>
#include <Implementationproxy.h>

//---------------------------------------------------------------------------
//  Maps the num/denom from the MDF to the aspect_ratio_idc value supported
//---------------------------------------------------------------------------
//

TInt32 MapAspectRatio( TUint32 aNumerator, TUint32 aDenominator )
	{
	PRINT_ENTRY;

	TInt32 aspectratio = -1;
	switch ( aDenominator )
		{
		case 1:
			if ( aNumerator == 1 )
				{
				aspectratio = 1;
				}
			break;
		case 11:
			switch ( aNumerator )
				{
				case 12:
					aspectratio = 2;
					break;
				case 10:
					aspectratio = 3;
					break;
				case 16:
					aspectratio = 4;
					break;
				case 24:
					aspectratio = 6;
					break;
				case 20:
					aspectratio = 7;
					break;
				case 32:
					aspectratio = 8;
					break;
				case 18:
					aspectratio = 10;
					break;
				case 15:
					aspectratio = 11;
					break;
				default:
					break;
				}
			break;
		case 33:
			switch ( aNumerator )
				{
				case 40:
					aspectratio = 5;
					break;
				case 80:
					aspectratio = 9;
					break;
				case 64:
					aspectratio = 12;
					break;
				default:
					break;
				}
			break;
		case 99:
			if ( aNumerator == 160 )
				{
				aspectratio = 13;
				}

			break;
		default:
			break;
		}

	PRINT_EXIT;
	return aspectratio;
	}

//---------------------------------------------------------------------------
//  Two phase constructor for an object of CAriMp4spencHwDeviceImpl
//---------------------------------------------------------------------------
//
CAriMp4spencHwDeviceImpl* CAriMp4spencHwDeviceImpl::NewL()
	{
	PRINT_ENTRY;

	CAriMp4spencHwDeviceImpl* self =
			new ( ELeave ) CAriMp4spencHwDeviceImpl();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	PRINT_EXIT;
	return self;
	}

//---------------------------------------------------------------------------
//  Default constructor
//---------------------------------------------------------------------------
//
CAriMp4spencHwDeviceImpl::CAriMp4spencHwDeviceImpl() :
			iMMFDevVideoRecordProxy( NULL ),
			iInputDevice( NULL ),
			iClockSource( NULL ),
			iInputBufReturnToPreProc( EFalse ),
			iPeriodicTimer( NULL ),
			iClockTimeWhenPaused( 0 ),
			//100 milli seconds
			iPollingInterval( TTimeIntervalMicroSeconds32( 100000 ) ),
			iCodec( NULL ), iEngine( NULL ), iOutputBuffers( NULL ),
			iOutputBufferSize( 0 ), iPauseOffset( 0 ), iTotalTime( 0 ),
			iLastEncodedPictureTimestamp( 0 ), iPictureLoss( EFalse ),
			iInputEndCalled( EFalse ), iFrozen( EFalse ),
			iTotalLengthFilledSoFarInPacketMode( 0 ),
			iTotalOutputBufferLengthInPacketMode( 0 ),
			iPacketOffSetCurrentPosition( NULL ),
			iPacketOffSetAndLengthInfoBuffers( NULL ),
			iInternalOutputBuffers( NULL ), iPacketsPending( EFalse )

	{
	PRINT_ENTRY;

	iSupportedDataUnitTypes = EDuCodedPicture | EDuVideoSegment;
	iSupportedDataUnitEncapsulations = EDuElementaryStream;

	// Default values for Init params - full range
	TUncompressedVideoFormat inputFormat;
	inputFormat.iDataFormat = EYuvRawData;
	inputFormat.iYuvFormat.iCoefficients = EYuvBt709Range0;
	inputFormat.iYuvFormat.iPattern = EYuv420Chroma1;
	inputFormat.iYuvFormat.iDataLayout = EYuvDataPlanar;
	inputFormat.iYuvFormat.iYuv2RgbMatrix = NULL;
	inputFormat.iYuvFormat.iRgb2YuvMatrix = NULL;
	inputFormat.iYuvFormat.iAspectRatioNum = 1;
	inputFormat.iYuvFormat.iAspectRatioDenom = 1;

	iSetMpeg4H263HWParams.iInputFormat = inputFormat;
	iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_UNKNOWN;
	iSetMpeg4H263HWParams.iAfterInitialize = 0;
	iSetMpeg4H263HWParams.iOutputFormat = EH263;
	iSetMpeg4H263HWParams.iMinNumOutputBuffers
			= KMPEG4H263ENCIMPL_MAXNUM_OUTPUTBUFFERS;

	iSetMpeg4H263HWParams.iPacketSize = KMPEG4H263ENCIMPL_DEFAULT_SEGMENTSIZE;
	iSetMpeg4H263HWParams.iMaxPictureRate
			= KMPEG4H263ENCIMPL_DEFAULT_PICTURERATE;

	// initialize picture counters
	iPictureCounters.iPicturesSkippedBufferOverflow = 0;
	iPictureCounters.iPicturesSkippedProcPower = 0;
	iPictureCounters.iPicturesSkippedRateControl = 0;
	iPictureCounters.iPicturesProcessed = 0;
	iPictureCounters.iInputPictures = 0;
	iSetMpeg4H263HWParams.iDataEncapsulation = EDuElementaryStream;

	// default packet mode is off
	iSetMpeg4H263HWParams.iDataUnitType = EDuCodedPicture;

	iSetMpeg4H263HWParams.iBitRate = KH263ENCIMPL_DEFAULT_BITRATE;
	iSetMpeg4H263HWParams.iTargetPictureRate
			= KMPEG4H263ENCIMPL_DEFAULT_PICTURERATE;
	iSetMpeg4H263HWParams.iRandomAccessRate
			= KMPEG4H263ENCIMPL_DEFAULT_RANDOMACCESSRATE;
	 // scene detection is ON
	iSetMpeg4H263HWParams.iSceneCutDetection = 1;
	//preprocessing disabled by default
	iSetMpeg4H263HWParams.iPreprocessing = 0;
	iSetMpeg4H263HWParams.iRCModel = CBR;
	iSetMpeg4H263HWParams.iComplexityLevel = HIGH;
	iSetMpeg4H263HWParams.iSearchRange = 31;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------------
//
CAriMp4spencHwDeviceImpl::~CAriMp4spencHwDeviceImpl()
	{
	PRINT_ENTRY;

	PRINT_MSG( LEVEL_LOW, ( "this is %x", this ) );
	iSupportedInputFormats.Close();
	iLevels.Close();

	//allocated formats should be deleted before closing
	while ( iSupportedOutputFormats.Count() > 0 )
		{
		CCompressedVideoFormat* lCompFormat = iSupportedOutputFormats[0];
		iSupportedOutputFormats.Remove( 0 );
		delete lCompFormat;
		}
	iSupportedOutputFormats.Close();

	// Stop processing
	if ( !iEncStateMac->IsStopped() && ( !iEncStateMac->IsInDeadState() ) )
		{
		if ( iEncStateMac->IsInitialized() )
			{
			Stop();
			}
		}

	if ( iEngine )
		{
		iEngine->Reset();
		delete iEngine;
		iEngine = NULL;
		}

	if ( iCodec )
		{
		delete iCodec;
		iCodec = NULL;
		}

	//delete output buffers
	iOutputFreeBufferQueue.Reset();
	iOutputFreeBufferQueue.Close();

	if ( iOutputBuffers )
		{
		if ( iSetMpeg4H263HWParams.iMinNumOutputBuffers )
			{
			for ( TInt i = 0; i < iSetMpeg4H263HWParams.iMinNumOutputBuffers;
				 i++ )
				{
				if ( iOutputBuffers[i].iData.Ptr() )
					{
					delete ( TUint8* ) iOutputBuffers[i].iData.Ptr();
					}
				}
			}

		delete[] iOutputBuffers;
		iOutputBuffers = NULL;
		}

	if ( iPeriodicTimer )
		{
		delete iPeriodicTimer;
		iPeriodicTimer = NULL;
		}

	if ( iPacketOffSetAndLengthInfoBuffers )
		{
		for ( TInt i = 0; i < KMPEG4H263ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS;
			 i++ )
			{
			delete[] ( TUint8* ) iPacketOffSetAndLengthInfoBuffers[i];
			}
		delete[] ( TUint* ) iPacketOffSetAndLengthInfoBuffers;
		iPacketOffSetAndLengthInfoBuffers = NULL;
		}

	//delete temp output buffers
	iInternalOutputBufferQueue.Reset();
	iInternalOutputBufferQueue.Close();

	if ( iInternalOutputBuffers )
		{
		for ( TInt i = 0; i < KMPEG4H263ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS;
			i++ )
			{
			if ( iInternalOutputBuffers[i].iData.Ptr() )
				{
				delete ( TUint8* ) iInternalOutputBuffers[i].iData.Ptr();
				}
			}
		delete[] iInternalOutputBuffers;
		iInternalOutputBuffers = NULL;
		}

	iFreeBufferQueueForPacketOffsetInfo.Reset();
	iFilledBufferQueueForPacketOffsetInfo.Reset();

	if ( iConfigData )
		{
		delete iConfigData;
		iConfigData = NULL;
		}

	if ( iPacketOffsetBuf )
		{
		delete iPacketOffsetBuf;
		iPacketOffsetBuf = NULL;
		}

	if ( iEncStateMac )
		{
		delete iEncStateMac;
		iEncStateMac = NULL;
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Custom Interface supported by the HwDevice plugin
//---------------------------------------------------------------------------
//
TAny* CAriMp4spencHwDeviceImpl::CustomInterface( TUid /* aInterface */ )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	return NULL;
	}

//---------------------------------------------------------------------------
//  Returns information about this Encoder HW Device
//---------------------------------------------------------------------------
//
CVideoEncoderInfo* CAriMp4spencHwDeviceImpl::VideoEncoderInfoLC()
	{
	PRINT_ENTRY;

	TSize maxPictureSize = TSize( KMPEG4H263ENCIMPL_720P_WIDTH,
			KMPEG4H263ENCIMPL_720P_HEIGHT );

	TUint32 maxBitRate = KMaxSupportedBitRate;
	RArray<TPictureRateAndSize> maxPictureRatesAndSizes;
	CleanupClosePushL( maxPictureRatesAndSizes );
	TPictureRateAndSize pictureRateAndSize;
	TInt error = KErrNone;

	// Max frame rate in mpeg4 sp encoding for any resolution is 15 fps.
	// 30 fps is supported only in H263 encoding for SQCIF, QCIF, CIF only.

	pictureRateAndSize.iPictureRate = KPictureRate30;
	pictureRateAndSize.iPictureSize = TSize( KMPEG4H263ENCIMPL_SQCIF_WIDTH,
											KMPEG4H263ENCIMPL_SQCIF_HEIGHT );

	error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if  ( error != KErrNone )
		{
		User::Leave( error );
		}
	pictureRateAndSize.iPictureRate = KPictureRate30;
	pictureRateAndSize.iPictureSize = TSize( KMPEG4H263ENCIMPL_QCIF_WIDTH,
											KMPEG4H263ENCIMPL_QCIF_HEIGHT );

	error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if  ( error != KErrNone )
		{
		User::Leave( error );
		}

	pictureRateAndSize.iPictureRate = KPictureRate30;
	pictureRateAndSize.iPictureSize = TSize( KMPEG4H263ENCIMPL_CIF_WIDTH,
			KMPEG4H263ENCIMPL_CIF_HEIGHT );

	error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if  ( error != KErrNone )
		{
		User::Leave( error );
		}

	pictureRateAndSize.iPictureRate = KPictureRate15;
	pictureRateAndSize.iPictureSize = TSize( KMPEG4H263ENCIMPL_QVGA_WIDTH,
											KMPEG4H263ENCIMPL_QVGA_HEIGHT );

	error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if  ( error != KErrNone )
		{
			User::Leave( error );
		}

	pictureRateAndSize.iPictureRate = KPictureRate15;
	pictureRateAndSize.iPictureSize = TSize( KMPEG4H263ENCIMPL_VGA_WIDTH,
											KMPEG4H263ENCIMPL_VGA_HEIGHT );

	error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if  ( error != KErrNone )
		{
			User::Leave( error );
		}

	pictureRateAndSize.iPictureRate = KPictureRate15;
	pictureRateAndSize.iPictureSize = TSize( KMPEG4H263ENCIMPL_720P_WIDTH,
											KMPEG4H263ENCIMPL_720P_HEIGHT );

	error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if  ( error != KErrNone )
		{
		User::Leave( error );
		}

	TUint32 lSupportedPictureOptions = TVideoPicture::ETimestamp
			| TVideoPicture::EReqInstantRefresh | TVideoPicture::ESceneCut;

	TUint32 supportedDataUnitEncapsulations = EDuElementaryStream;
	TUint32 supportedDataUnitTypes = EDuCodedPicture | EDuVideoSegment;
	CVideoEncoderInfo* videoEncoderInfo = CVideoEncoderInfo::NewL( TUid::Uid(
								KUidMpeg4H263EncoderHwDeviceImplUid ),
								KMPEG4H263EncManufacturer,
								KMPEG4H263EncIdentifier,
								TVersion( KMPEG4H263ENCIMPL_MAJOR_VERSION,
										  KMPEG4H263ENCIMPL_MINOR_VERSION,
										  KMPEG4H263ENCIMPL_BUILD_VERSION ),
								// Accelerated
								EFalse,
								// Enc doesnt support direct capture
								EFalse,
								iSupportedInputFormats.Array(),
								iSupportedOutputFormats.Array(),
								maxPictureSize,
								supportedDataUnitTypes,
								// Max bitrate layers
								supportedDataUnitEncapsulations, 1,
								//aSupportsSupplementalEnhancementInfo
								EFalse,
								//aMaxUnequalErrorProtectionLevels
								1,
								maxBitRate, maxPictureRatesAndSizes.Array(),
								1,
								//aSupportsPictureLoss
								lSupportedPictureOptions,
								ETrue,
								//aSupportsSliceLoss
								ETrue,
								//aCodingStandardSpecificInfo
								KNullDesC8,
								//aImplementationSpecificInfo
								KNullDesC8 );

	CleanupStack::PopAndDestroy();
	CleanupStack::PushL( videoEncoderInfo );

	PRINT_EXIT;
	return videoEncoderInfo;
	}

//---------------------------------------------------------------------------
//  Sets the encoder output format
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetOutputFormatL(
		const CCompressedVideoFormat& aFormat,
		TVideoDataUnitType aDataUnitType,
		TVideoDataUnitEncapsulation aDataEncapsulation,
		TBool aSegmentationAllowed )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		PRINT_ERR( "SetOutputFormatL () called before Initialize ()" );
		User::Leave( KErrPermissionDenied );
		}

	TInt error = KErrNotFound;
	TPtrC8 mimeType( aFormat.MimeType() );
	TInt i;
	for ( i = 0; i < iSupportedOutputFormats.Count(); i++ )
		{
		CCompressedVideoFormat* format = iSupportedOutputFormats[i];
		if ( mimeType.CompareF( format->MimeType() ) == 0 )
			{
			iSetMpeg4H263HWParams.iLevel = iLevels[i];
			error = KErrNone;
			break;
			}
		}

	if ( error == KErrNotFound )
		{
		PRINT_ERR( "CAriMp4spencHwDeviceImpl::SetOutputFormatL() Leaving"
				   " because of unsupported output mimetype" );
		User::Leave( KErrNotSupported );
		return;
		}

	// Index values 0 - 10 contain MPEG4 mime types and 11-17 contain
	// H263 mime types
	if ( i >= 0 && i <= KMPEG4SUPPMIMEARRAYMAXINDEXVALUE )
		{
		iSetMpeg4H263HWParams.iOutputFormat = EMpeg4;
		}

	else if ( i >= KMPEG4SUPPMIMEARRAYMAXINDEXVALUE + 1 && i
			<= KH263SUPPMIMEARRAYMAXINDEXVALUE )
		{
		iSetMpeg4H263HWParams.iOutputFormat = EH263;
		}

		PRINT_MSG( LEVEL_HIGH, ( "level is set to %d"
						,(TInt)iSetMpeg4H263HWParams.iLevel ) );

	if ( ( ( aDataUnitType != EDuCodedPicture ) && ( aDataUnitType
		!= EDuVideoSegment) ) || ( aDataEncapsulation
		!= EDuElementaryStream ) || (aSegmentationAllowed ) )

		{
		PRINT_ERR( "CAriMp4spencHwDeviceImpl::SetOutputFormatL() Leaving"
			" because of unsupported data unit type or data unit"
			" encapsulation" );
		User::Leave( KErrNotSupported );
		}

	if ( aDataUnitType == EDuCodedPicture )
		{
		iSetMpeg4H263HWParams.iPacketmode = EFalse;
		iSetMpeg4H263HWParams.iNumOfGOBHdrs = 0;
			PRINT_MSG( LEVEL_HIGH, ( "CAriMp4spencHwDeviceImpl::"
							"SetOutputFormatL() Packet mode is OFF " ) );
		}
	else
		{
		iSetMpeg4H263HWParams.iPacketmode = ETrue;
		if ( iSetMpeg4H263HWParams.iOutputFormat == EH263 )
			{
			iSetMpeg4H263HWParams.iNumOfGOBHdrs = 1;
			}
			PRINT_MSG( LEVEL_HIGH, ( "CAriMp4spencHwDeviceImpl::"
							"SetOutputFormatL() Packet mode is ON " ) );

		// must be calculated from and levels set by the client
		iSetMpeg4H263HWParams.iMinNumOutputBuffers
				= KMPEG4H263ENCIMPL_MAXNUM_SEGMENTBUFFERS;
		}

	iSetMpeg4H263HWParams.iDataUnitType = aDataUnitType;
	iSetMpeg4H263HWParams.iDataEncapsulation = aDataEncapsulation;
	iSetMpeg4H263HWParams.iSegmentationAllowed = aSegmentationAllowed;
	iSetMpeg4H263HWParams.iBeforeInitialize |= EEncOutputFormat;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Sets the pre-processor device that will write data to this encoder
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetInputDevice(
		CMMFVideoPreProcHwDevice* aDevice )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrPermissionDenied );
		return;
		}

	if ( !aDevice )
		{
		ClientFatalError( KErrArgument );
		return;
		}

	iInputDevice = aDevice;
	iSetMpeg4H263HWParams.iBeforeInitialize |= EEncInputDevice;
	iInputBufReturnToPreProc = ETrue;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Sets the number of bit-rate scalability layers to use
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetNumBitrateLayersL( TUint aNumLayers )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		PRINT_ERR( "SetNumBitrateLayersL () called before Initialize"
				"..leaving" );
		User::Leave( KErrPermissionDenied );
		}

	if (aNumLayers != 1)
		{
		PRINT_ERR( "Wrong value passed for aNumLayers... Leaving" );
		User::Leave( KErrNotSupported );
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Sets the scalability type for a bit-rate scalability layer. Currently not
//  supported
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetScalabilityLayerTypeL( TUint /*aLayer*/,
		TScalabilityType /*aScalabilityType*/ )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		PRINT_ERR( "SetScalabilityLayerTypeL() called before Initialize"
				"..leaving" );
		User::Leave( KErrPermissionDenied );
		}

    PRINT_ERR( "SetScalabilityLayerTypeL() not supported...Leaving with "
    		"KErrNotSupported" );
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Sets the reference picture options to be used for all scalability layers
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetGlobalReferenceOptions(
		TUint aMaxReferencePictures, TUint aMaxPictureOrderDelay )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrPermissionDenied );
		return;
		}

	if ( aMaxPictureOrderDelay != 0 || aMaxReferencePictures
			> KMPEG4H263ENCIMPL_MAXNUM_REFERENCEPICTURES )
		{
		ClientFatalError( KErrNotSupported );
		return;
		}

	iSetMpeg4H263HWParams.iLayerReferenceOptions[0].iMaxReferencePictures
			= aMaxReferencePictures;

	iSetMpeg4H263HWParams.iLayerReferenceOptions[0].iMaxPictureOrderDelay
			= aMaxPictureOrderDelay;

	iSetMpeg4H263HWParams.iBeforeInitialize |= EEncGlobalRefOptions;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the reference picture options to be used for a single scalability
// layer
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetLayerReferenceOptions( TUint /*aLayer*/,
		TUint /*aMaxReferencePictures*/, TUint /*aMaxPictureOrderDelay*/ )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrPermissionDenied );
		return;
		}
	ClientFatalError( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets encoder buffering options
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetBufferOptionsL(
		const TEncoderBufferOptions& aOptions )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		PRINT_ERR( "SetBufferOptionsL () called before Initialize ()" );
		User::Leave( KErrPermissionDenied );
		}

	PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl::SetBufferOptionsL() "
    		"iMaxPreEncoderBufferPictures = %d" ,
    		(TInt)aOptions.iMaxPreEncoderBufferPictures ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl::SetBufferOptionsL() "
    		"iMaxOutputBufferSize = %d",
    		( TInt ) aOptions.iMaxOutputBufferSize ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl::SetBufferOptionsL() "
    		"iMaxCodedPictureSize = %d" ,
    		( TInt )aOptions.iMaxCodedPictureSize ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl::SetBufferOptionsL() "
    		"iHrdVbvSpec = %x", ( TInt ) aOptions.iHrdVbvSpec ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl::SetBufferOptionsL() "
    		"iMinNumOutputBuffers = %d" ,
    		( TInt ) aOptions.iMinNumOutputBuffers ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl::SetBufferOptionsL() "
    		"iMaxCodedSegmentSize = %d" ,
			( TInt ) aOptions.iMaxCodedSegmentSize ) );

	if ( ( aOptions.iMaxPreEncoderBufferPictures == 0 )
			|| ( aOptions.iMaxOutputBufferSize == 0 )
			|| ( aOptions.iMinNumOutputBuffers == 0 )
			|| ( aOptions.iHrdVbvSpec == EHrdVbv3GPP ) )
		{
		PRINT_ERR( "SetBufferOptionsL () - incorrect parameter passed ..."
				"leaving with KErrNotSupported" );
		User::Leave( KErrNotSupported );
		return;
		}

	iSetMpeg4H263HWParams.iMaxPreEncoderBufferPictures
			= aOptions.iMaxPreEncoderBufferPictures;
	iSetMpeg4H263HWParams.iMaxOutputBufferSize
			= aOptions.iMaxOutputBufferSize;
	iSetMpeg4H263HWParams.iMaxCodedPictureSize
			= aOptions.iMaxCodedPictureSize;
	iSetMpeg4H263HWParams.iPacketSize = aOptions.iMaxCodedSegmentSize;
	iSetMpeg4H263HWParams.iMinNumOutputBuffers
			= aOptions.iMinNumOutputBuffers;
	iSetMpeg4H263HWParams.iBeforeInitialize |= EEncBufferOptions;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the encoder output rectangle
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetOutputRectL( const TRect& aRect )
	{
	PRINT_ENTRY;

	if ( ( aRect.iTl.iX >= aRect.iBr.iX )
			|| ( aRect.iTl.iY >= aRect.iBr.iY ) )
		{
		PRINT_ERR( " Invalid parameteres passed..Leaving " );
		User::Leave( KErrNotSupported );
		return;
		}

	iSetMpeg4H263HWParams.iOutputRect = aRect;
	iSetMpeg4H263HWParams.iBeforeInitialize |= EEncOutputRectSize;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets whether bit errors or packets losses can be expected in the video
// transmission
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetErrorsExpected( TBool aBitErrors,
		TBool aPacketLosses )
	{
	PRINT_ENTRY;

	// This can bel called before and after initialize
	 PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl::SetErrorsExpected()"
			 " BitErrors is set to [ %d ]", aBitErrors ) );
	 PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::SetErrorsExpected() "
			 "iPacketLosses is set to [ %d ]", aPacketLosses ) );

	if ( iEncStateMac->IsInitialized() )
		{
		iCurSetMpeg4H263HWParams.iBitErrors = aBitErrors;
		iCurSetMpeg4H263HWParams.iPacketLosses = aPacketLosses;
		iCurSetMpeg4H263HWParams.iAfterInitialize |= EEncErrorsExpected;
		}
	else
		{
		iSetMpeg4H263HWParams.iBitErrors = aBitErrors;
		iSetMpeg4H263HWParams.iPacketLosses = aPacketLosses;
		iSetMpeg4H263HWParams.iBeforeInitialize |= EEncErrorsExpected;
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the minimum frequency (in time) for instantaneous random access points
// in the bitstream
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetMinRandomAccessRate( TReal aRate )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		// simply return no further action is taken
		if ( aRate <= 0.0 )
			{
			return;
			}
		iCurSetMpeg4H263HWParams.iRandomAccessRate = aRate;
		PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl"
				"::SetMinRandomAccessRate() iRandomAccessRate is %f" ,
				( TReal )iSetMpeg4H263HWParams.iRandomAccessRate ) );
		iCurSetMpeg4H263HWParams.iAfterInitialize |= EEncRandomAccessRate;
		}
	else
		{
		if ( aRate <= 0.0 )
			{
			ClientFatalError( KErrNotSupported );
			return;
			}
		iSetMpeg4H263HWParams.iRandomAccessRate = aRate;

		iSetMpeg4H263HWParams.iBeforeInitialize |= EEncRandomAccessRate;
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Sets coding-standard specific encoder options.
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetCodingStandardSpecificOptionsL(
		const TDesC8& /*aOptions*/ )
	{
	PRINT_ENTRY;
	User::Leave( KErrNotSupported );
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Sets coding-standard specific encoder options.
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetImplementationSpecificEncoderOptionsL(
		const TDesC8& /*aOptions*/ )
	{
	PRINT_ENTRY;

	//This API can be called at any point of time
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Returns coding-standard specific initialization output from the encoder
//---------------------------------------------------------------------------
//

HBufC8* CAriMp4spencHwDeviceImpl::CodingStandardSpecificInitOutputLC()
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "CodingStandardSpecificInitOutputLC () called before "
    			"Initialize ()" );
		User::Leave( KErrNotReady );
		}

	TUint configLength;
	TUint err = iCodec->GetParam( CONTROL_CMD_GET_CONFIGDATALENGTH,
			&configLength );
	if ( err )
		{
		User::Leave( err );
		}

	iConfigData = HBufC8::NewL( configLength );
	err = iCodec->GetParam( CONTROL_CMD_GET_CONFIGDATA, iConfigData );

	if ( err )
		{
    	PRINT_ERR("GetParam failure");
		User::Leave( err );
		}

	PRINT_EXIT;
	return iConfigData;
	}

//---------------------------------------------------------------------------
//  Returns coding-standard specific initialization output from the encoder
//---------------------------------------------------------------------------
//

HBufC8* CAriMp4spencHwDeviceImpl::ImplementationSpecificInitOutputLC()
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "ImplementationSpecificInitOutputLC () called before "
    			"Initialize ()" );
		User::Leave( KErrNotReady );
		return NULL;
		}

	PRINT_ERR( "ImplementationSpecificInitOutputLC () not supported...leaving"
			"with KErrNotSupported" );
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	return NULL;
	}

//---------------------------------------------------------------------------
// Sets the number of unequal error protection levels
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetErrorProtectionLevelsL(
		TUint /*aNumLevels*/, TBool /*aSeparateBuffers*/ )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "SetErrorProtectionLevelsL () called before "
    	    			"Initialize ()" );
		User::Leave( KErrNotReady );
		}

	PRINT_ERR( "SetErrorProtectionLevelsL() not supported...leaving"
			"with KErrNotSupported" );
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the number of unequal error protection levels
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetErrorProtectionLevelL(TUint /*aLevel*/,
		TUint /*aBitrate*/, TUint /*aStrength*/)
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "SetErrorProtectionLevelsL () called before "
    	    			"Initialize ()" );
		User::Leave( KErrNotReady );
		}

	PRINT_ERR( "SetErrorProtectionLevelsL() not supported...leaving"
			"with KErrNotSupported" );
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Sets the expected or prevailing channel conditions for an unequal
//  error protection level, in terms of expected packet loss rate
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetChannelPacketLossRate(TUint /*aLevel*/,
		TReal /*aLossRate*/, TTimeIntervalMicroSeconds32 /*aLossBurstLength*/)
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
        return;
		}

	PRINT_EXIT;
	ClientFatalError( KErrNotSupported );

	}

//---------------------------------------------------------------------------
// Sets the expected or prevailing channel conditions for an unequal error
// protection level, in terms of expected bit error rate
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetChannelBitErrorRate(TUint /*aLevel*/,
		TReal aErrorRate, TReal /*aStdDeviation*/)
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	// ignore the negative and 0 error rate values - #AANV-6QSC9N
	if ( aErrorRate < 0.0 )
		{
		return;
		}

	TReal* bitErrorRate = NULL;
	TRAPD ( error , bitErrorRate = new ( ELeave ) TReal );
	if ( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	*bitErrorRate = aErrorRate;
	TRAP ( error, iEngine->AddCommandL( CBaseEngine::EHighPriority,
					CONTROL_CMD_SET_CHANNEL_BIT_ERROR_RATE, bitErrorRate ) );

	if ( error != KErrNone )
		{
		delete bitErrorRate;
		ClientFatalError( error );
		return;
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Sets the target size of each coded video segment
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetSegmentTargetSize( TUint aLayer,
		TUint aSizeBytes, TUint /*aSizeMacroblocks*/ )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	if ( iSetMpeg4H263HWParams.iDataUnitType != EDuVideoSegment )
		{
		return;
		}

	// aLayer should be zero since layered bit-rate scalability is not used.
	if ( aLayer != 0 )
		{
		return;
		}

	TUint calculatedSegmentSize = 0;
	// if value < 0 or > iMaxOutputBufferSize the adjust this value
	if ( aSizeBytes < KMPEG4H263ENCIMPL_MIN_SEGMENTSIZE )
		{
		calculatedSegmentSize = KMPEG4H263ENCIMPL_MIN_SEGMENTSIZE;
		}
	else
		{
		if ( iSetMpeg4H263HWParams.iMaxOutputBufferSize
				> KMPEG4H263ENCIMPL_MAX_SEGMENTSIZE )
			{
			if ( aSizeBytes < KMPEG4H263ENCIMPL_MAX_SEGMENTSIZE )
				{
				calculatedSegmentSize = aSizeBytes;
				}
			else
				{
				calculatedSegmentSize = KMPEG4H263ENCIMPL_MAX_SEGMENTSIZE;
				}
			}
		else
			{
			if ( aSizeBytes < iSetMpeg4H263HWParams.iMaxOutputBufferSize )
				{
				calculatedSegmentSize = aSizeBytes;
				}
			else
				{
				calculatedSegmentSize
						= iSetMpeg4H263HWParams.iMaxOutputBufferSize;
				}
			}
		}

	// Set the target size on codec

	TInt error = iCodec->SetParam( CONTROL_CMD_SET_PACKETSIZE,
			&calculatedSegmentSize );

	if ( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}
	iSetMpeg4H263HWParams.iPacketSize = calculatedSegmentSize;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the bit-rate control options for a layer
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetRateControlOptions( TUint aLayer,
		const TRateControlOptions& aOptions )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	// Since layered bit-rate scalability is not used, options are set for the
	//entire stream

	if ( aLayer != 0 )
		{
		return;
		}

	if ( ( aOptions.iPictureRate <= 0 ) ||
			( aOptions.iControl & EBrControlPicture ) )
		{
		return;
		}

	if ( aOptions.iControl & EBrControlStream )
		{
		if ( ( aOptions.iQualityTemporalTradeoff < 0 )
				|| ( aOptions.iQualityTemporalTradeoff > 1 )
				|| ( aOptions.iLatencyQualityTradeoff < 0 )
				|| ( aOptions.iLatencyQualityTradeoff > 1 )
				|| ( aOptions.iBitrate == 0 ) )
			{
			return;
			}
		}

	TRateControlOptions* rateControlOptions = NULL;
	TRAPD ( error, rateControlOptions = new ( ELeave ) TRateControlOptions );

	if ( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	rateControlOptions->iControl = aOptions.iControl;
	// default bit rate used
	if ( aOptions.iControl & EBrControlNone )
		{
		rateControlOptions->iBitrate = iSetMpeg4H263HWParams.iBitRate;
		}
	else
		{
		rateControlOptions->iBitrate = ( aOptions.iBitrate
				<= iSetMpeg4H263HWParams.iBitRate ) ? ( aOptions.iBitrate )
				: ( iSetMpeg4H263HWParams.iBitRate );
		}

	rateControlOptions->iPictureQuality = aOptions.iPictureQuality;
	rateControlOptions->iPictureRate = aOptions.iPictureRate;
	rateControlOptions->iQualityTemporalTradeoff
			= aOptions.iQualityTemporalTradeoff;
	rateControlOptions->iLatencyQualityTradeoff
			= aOptions.iLatencyQualityTradeoff;

	TRAP ( error, iEngine->AddCommandL( CBaseEngine::EHighPriority,
				CONTROL_CMD_SET_RATE_CONTROL_OPTIONS, rateControlOptions ) );

	if ( error != KErrNone )
		{
		delete rateControlOptions;
		ClientFatalError( error );
		return;
		}

	PRINT_MSG( LEVEL_LOW,( "CAriMp4spencHwDeviceImpl::SetRateControlOptions"
			"iBitrate is %d", ( TInt )aOptions.iBitrate ) );

	PRINT_MSG( LEVEL_LOW,( "CAriMp4spencHwDeviceImpl::SetRateControlOptions"
			"iPictureQuality is %d" , ( TInt )aOptions.iPictureQuality ) );

	PRINT_MSG( LEVEL_LOW,( "CAriMp4spencHwDeviceImpl::SetRateControlOptions"
			" iPictureRate is %f", ( TReal )aOptions.iPictureRate ) );

	PRINT_MSG( LEVEL_LOW,( "CAriMp4spencHwDeviceImpl::SetRateControlOptions"
			"iQualityTemporalTradeoff is %f" ,
			( TReal )aOptions.iQualityTemporalTradeoff ) );

	PRINT_MSG( LEVEL_LOW,( "CAriMp4spencHwDeviceImpl::SetRateControlOptions"
			" iLatencyQualityTradeoff is %f " ,
			( TReal ) aOptions.iLatencyQualityTradeoff ) );

	PRINT_MSG( LEVEL_LOW,( "CAriMp4spencHwDeviceImpl::SetRateControlOptions"
			"iControl is %x" , ( TInt ) aOptions.iControl ) );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets in-layer scalability options for a layer
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetInLayerScalabilityL( TUint /*aLayer*/,
		TUint /*aNumSteps*/, TInLayerScalabilityType /*aScalabilityType*/,
		const TArray<TUint>& /*aBitrateShare*/,
		const TArray<TUint>& /*aPictureShare*/ )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "SetInLayerScalabilityL () called before "
    	    			"Initialize ()" );
		User::Leave( KErrNotReady );
		}

	PRINT_ERR( "SetInLayerScalabilityL () not supported..leaving with "
	    			"KErrNotSupported" );
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the period for layer promotions points for a scalability layer
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetLayerPromotionPointPeriod(
		TUint /*aLayer*/,
		TUint /*aPeriod*/ )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}
	ClientFatalError( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Returns coding-standard specific settings output from the encoder
//---------------------------------------------------------------------------
//
HBufC8* CAriMp4spencHwDeviceImpl::CodingStandardSpecificSettingsOutputLC()
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "CodingStandardSpecificSettingsOutputLC () called before "
    	    			"Initialize ()" );
		User::Leave( KErrNotReady );
		}
	PRINT_ERR( "CodingStandardSpecificSettingsOutputLC () not supported.."
			"leaving with KErrNotSupported" );
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	return NULL;
	}

//---------------------------------------------------------------------------
// Returns implementation-specific settings output from the encoder
//---------------------------------------------------------------------------
//
HBufC8* CAriMp4spencHwDeviceImpl::ImplementationSpecificSettingsOutputLC()
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "ImplementationSpecificSettingsOutputLC () called before "
    	    	    			"Initialize ()" );
		User::Leave( KErrNotReady );
		}

	PRINT_ERR( "ImplementationSpecificSettingsOutputLC () not supported.."
			"leaving with KErrNotSupported" );
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	return NULL;
	}

//---------------------------------------------------------------------------
// Requests the encoder to sends supplemental information in the bitstream
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SendSupplementalInfoL(
													const TDesC8& /*aData*/)
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "SendSupplementalInfoL () called before "
    	    	    			"Initialize ()" );
		User::Leave( KErrNotReady );
		}

	PRINT_ERR( "SendSupplementalInfoL () not supported.."
			"leaving with KErrNotSupported" );
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Requests the encoder to sends supplemental information in the bitstream
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SendSupplementalInfoL(
							const TDesC8& /*aData*/,
							const TTimeIntervalMicroSeconds& /*aTimestamp*/)
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "SendSupplementalInfoL () called before "
    	    	    			"Initialize ()" );
		User::Leave( KErrNotReady );
		}

	PRINT_ERR( "SendSupplementalInfoL () not supported.."
			"leaving with KErrNotSupported" );
	User::Leave( KErrNotSupported );

    PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Cancels the current supplemental information send request
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::CancelSupplementalInfo()
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}
    ClientFatalError( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Gets the current output buffer status. The information includes
// the number of free output buffers and the total size of free buffers in
// bytes.
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::GetOutputBufferStatus(
													TUint& aNumFreeBuffers,
													TUint& aTotalFreeBytes )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	aNumFreeBuffers = iEngine->NumOutputBuffers()
			+ iOutputFreeBufferQueue.Count();
	aTotalFreeBytes = aNumFreeBuffers * iOutputBufferSize;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Returns a used output buffer back to the encoder
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::ReturnBuffer( TVideoOutputBuffer* aBuffer )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	if (iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment)
		{
		TInt error = KErrNone;
		error = iOutputFreeBufferQueue.Append( aBuffer );

		if ( error != KErrNone )
			{
			ClientFatalError( error );
			return;
			}

		if ( !iPacketsPending )
			{
			return;
			}

		// still has packets in temporary buffer
		if ( iTotalLengthFilledSoFarInPacketMode
				< iTotalOutputBufferLengthInPacketMode )
			{
			TVideoOutputBuffer *outBuf = iOutputFreeBufferQueue[0];
			FillVideoSegment( outBuf, iInternalOutputBufferQueue[0] );
			iOutputFreeBufferQueue.Remove( 0 );
			iMMFDevVideoRecordProxy->MdvrpNewBuffer( outBuf );
			}
		}
	else
		{

		aBuffer->iData.Set( ( TUint8* ) aBuffer->iData.Ptr()
						, iOutputBufferSize );

		// add the buffer back to queue or process engine
		if ( ( !iEncStateMac->IsInputEndPending() )
				&& ( !iEncStateMac->IsStopped() ) )
			{
			iEngine->AddOutput( ( TAny* ) aBuffer );
			}
		else
			{
			TInt error = iOutputFreeBufferQueue.Append( aBuffer );
			if ( error != KErrNone )
				{
				ClientFatalError( error );
				return;
				}
			}
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Indicates a picture loss to the encoder, without specifying the lost
// picture
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::PictureLoss()
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}
	iPictureLoss = ETrue;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Indicates to the encoder the pictures that have been lost
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::PictureLoss(
						const TArray<TPictureId>& /*aPictures*/ )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}
	iPictureLoss = ETrue;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Indicates a slice loss to the encoder.
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SliceLoss( TUint aFirstMacroblock,
		TUint aNumMacroblocks, const TPictureId& /*aPicture*/ )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	if ( ( aFirstMacroblock == 0 ) || ( aNumMacroblocks == 0 ) )
		{
    	PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl::SliceLoss() "
    			"aNumMacroblocks or aFirstMacroblock = 0 return" ) );
		return;
		}

	TMPEG4H263EncSliceLoss* sliceLossParams = NULL;
	TRAPD ( error, sliceLossParams = new( ELeave ) TMPEG4H263EncSliceLoss );

	if ( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	sliceLossParams->iFirstMacroblock = aFirstMacroblock;
	sliceLossParams->iNumMacroblocks = aNumMacroblocks;

	TRAP ( error, iEngine->AddCommandL( CBaseEngine::ENormalPriority
						, CONTROL_CMD_SET_SLICELOSS, sliceLossParams ) );

	if ( error != KErrNone )
		{
		delete sliceLossParams;
		ClientFatalError( error );
		return;
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sends a reference picture selection request to the encoder
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::ReferencePictureSelection(
											const TDesC8& /*aSelectionData*/)
	{
	PRINT_ENTRY;

	if (!iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}
    ClientFatalError( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Initializes the device, and reserves hardware resources
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::Initialize()
	{
	PRINT_ENTRY;

	//Device should be in unintialized state
	if ( !iEncStateMac->IsTransitionValid(
								CStateMachine::EInitializingCommand ) )
		{
		iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this,
				KErrPermissionDenied );
		return;
		}

	// check for input and output formats set by the client
	if ( !( iSetMpeg4H263HWParams.iBeforeInitialize & EEncInputFormat ) )
		{
		PRINT_MSG( LEVEL_CRITICAL, ("CAriMp4spencHwDeviceImpl::Initialize() "
				"- SetInputFormat not called" ) );
		iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this
														, KErrNotSupported );
		return;
		}

	// check for buffer sizes
	if ( iSetMpeg4H263HWParams.iBeforeInitialize & EEncBufferOptions )
		{
		if ( iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment )
			{
			// Buffer size related checks
			if ( ( iSetMpeg4H263HWParams.iPacketSize
					> KMPEG4H263ENCIMPL_MAX_SEGMENTSIZE )
					|| ( iSetMpeg4H263HWParams.iPacketSize
							> iSetMpeg4H263HWParams.iMaxOutputBufferSize )
					|| ( iSetMpeg4H263HWParams.iPacketSize == 0 ) )
				{
				iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this,
						KErrNotSupported );
				return;
				}
			}

		else if ( iSetMpeg4H263HWParams.iDataUnitType == EDuCodedPicture )
			{
			if ( ( iSetMpeg4H263HWParams.iMaxCodedPictureSize
					> iSetMpeg4H263HWParams.iMaxOutputBufferSize )
					|| ( iSetMpeg4H263HWParams.iMaxCodedPictureSize == 0 ) )
				{
				iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this,
						KErrNotSupported );
				return;
				}
			}
		}
	else
		{
		if ( iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment )
			{
			iSetMpeg4H263HWParams.iMinNumOutputBuffers
					= KMPEG4H263ENCIMPL_MAXNUM_SEGMENTBUFFERS;
			iSetMpeg4H263HWParams.iPacketSize
					= KMPEG4H263ENCIMPL_DEFAULT_SEGMENTSIZE;
			iSetMpeg4H263HWParams.iMaxOutputBufferSize
					= KMPEG4H263ENCIMPL_MAX_SEGMENTSIZE;
			}
		else
			{
			iSetMpeg4H263HWParams.iMinNumOutputBuffers
					= KMPEG4H263ENCIMPL_MAXNUM_OUTPUTBUFFERS;
			}
		}

	if ( iSetMpeg4H263HWParams.iOutputFormat == EMpeg4 )
		{
		if ( iSetMpeg4H263HWParams.iLevel == KMPEG4_LEVEL_UNKNOWN )
			{

			if ( ( ( iSetMpeg4H263HWParams.iLevel == KMPEG4_LEVEL_UNKNOWN )
					&& ( iSetMpeg4H263HWParams.iPictureSize.iWidth
					== KMPEG4H263ENCIMPL_QCIF_WIDTH
					&& iSetMpeg4H263HWParams.iPictureSize.iHeight
					== KMPEG4H263ENCIMPL_QCIF_HEIGHT ) )
					|| ( iSetMpeg4H263HWParams.iLevel == KMPEG4_LEVEL_0 ) )
				{
				iSetMpeg4H263HWParams.iBitRate
					= KMPEG4ENCIMPL_BITRATE_LEVEL_0;
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_0;
				}

			else if ( ( ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_UNKNOWN )
					&& ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
					* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256
					<= 99 ) ) || ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_1 ) )
				{
				iSetMpeg4H263HWParams.iBitRate
					= KMPEG4ENCIMPL_BITRATE_LEVEL_1;
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_1;
				}

			else if ( ( ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_UNKNOWN)
					&& ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
					* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256
					<= 396 ) ) || ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_3 ) )
				{
				iSetMpeg4H263HWParams.iBitRate
					= KMPEG4ENCIMPL_BITRATE_LEVEL_3;
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_3;
				}

			else if ( ( ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_UNKNOWN )
					&& ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
					* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256
					<= 1200 ) ) || ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_4 ) )
				{
				iSetMpeg4H263HWParams.iBitRate
					= KMPEG4ENCIMPL_BITRATE_LEVEL_4A;
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_4;
				}

			else if ( ( ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_UNKNOWN)
					&& ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
					* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256
					<= 1620 ) ) || ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_5 ) )
				{
				iSetMpeg4H263HWParams.iBitRate
					= KMPEG4ENCIMPL_BITRATE_LEVEL_5;
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_5;
				}

			else if ( ( (iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_UNKNOWN )
					&& ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
					* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256
					<= 3600 ) ) || ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_6 ) )
				{
				iSetMpeg4H263HWParams.iBitRate
					= KMPEG4ENCIMPL_BITRATE_LEVEL_6;
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_6;
				}

			else if ( ( ( iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_UNKNOWN )
					&& ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
					* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256
					<= 8160 ) ) || (iSetMpeg4H263HWParams.iLevel
					== KMPEG4_LEVEL_7 ) )
				{
				iSetMpeg4H263HWParams.iBitRate
					= KMPEG4ENCIMPL_BITRATE_LEVEL_7;
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_7;
				}

			else if ( iSetMpeg4H263HWParams.iLevel == KMPEG4_LEVEL_0B )
				{
				iSetMpeg4H263HWParams.iBitRate
					= KMPEG4ENCIMPL_BITRATE_LEVEL_0B;
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_0B;
				}

			else if (iSetMpeg4H263HWParams.iLevel == KMPEG4_LEVEL_2)
				{
				iSetMpeg4H263HWParams.iBitRate
					= KMPEG4ENCIMPL_BITRATE_LEVEL_2;
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				iSetMpeg4H263HWParams.iLevel = KMPEG4_LEVEL_2;
				}
			}

		}
	else if ( iSetMpeg4H263HWParams.iOutputFormat == EH263 )
		{
		if ( ( ( iSetMpeg4H263HWParams.iLevel == KH263_LEVEL_UNKNOWN)
			&& ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
			* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256
			<= 99 ) ) || ( iSetMpeg4H263HWParams.iLevel == KH263_LEVEL_10 ) )
			{
			iSetMpeg4H263HWParams.iBitRate = KH263ENCIMPL_BITRATE_LEVEL_10;
			iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
			iSetMpeg4H263HWParams.iLevel = KH263_LEVEL_10;
			}

		else if ( ( ( iSetMpeg4H263HWParams.iLevel == KH263_LEVEL_UNKNOWN )
				&& ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
				* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256
				<= 396 ) ) || ( iSetMpeg4H263HWParams.iLevel
				== KH263_LEVEL_30 ) )
			{
			iSetMpeg4H263HWParams.iBitRate = KH263ENCIMPL_BITRATE_LEVEL_30;
			iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate30;
			iSetMpeg4H263HWParams.iLevel = KH263_LEVEL_30;
			}

		else if ( iSetMpeg4H263HWParams.iLevel == KH263_LEVEL_20 )
			{
			iSetMpeg4H263HWParams.iBitRate = KH263ENCIMPL_BITRATE_LEVEL_20;
			iSetMpeg4H263HWParams.iLevel = KH263ENCIMPL_LEVEL20;
			if ( ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
				* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256) <= 396 )
				{
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate15;
				}
			else if ( ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
				* iSetMpeg4H263HWParams.iPictureSize.iHeight ) / 256) <= 99 )
				{
				iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate30;
				}
			}

		else if ( iSetMpeg4H263HWParams.iLevel == KH263_LEVEL_40 )
			{
			iSetMpeg4H263HWParams.iBitRate = KH263ENCIMPL_BITRATE_LEVEL_40;
			iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate30;
			iSetMpeg4H263HWParams.iLevel = KH263ENCIMPL_LEVEL40;
			}

		else if ( iSetMpeg4H263HWParams.iLevel == KH263_LEVEL_45 )
			{
			iSetMpeg4H263HWParams.iBitRate = KH263ENCIMPL_BITRATE_LEVEL_45;
			iSetMpeg4H263HWParams.iTargetPictureRate = KPictureRate30;
			iSetMpeg4H263HWParams.iLevel = KH263ENCIMPL_LEVEL45;
			}
		}

	if ( iSetMpeg4H263HWParams.iOutputFormat == EH263 )
		{
		iSetMpeg4H263HWParams.iTimerResolution = KDefaultTimerResolution;
		iSetMpeg4H263HWParams.iReversibleVLC = E_ON;
		iSetMpeg4H263HWParams.iDataPartitioning = E_ON;
		iSetMpeg4H263HWParams.iMAPS = E_ON;
		}
	else
		{
		iSetMpeg4H263HWParams.iTimerResolution = KDefaultTimerResolution;
		iSetMpeg4H263HWParams.iReversibleVLC = E_OFF;
		iSetMpeg4H263HWParams.iDataPartitioning = E_OFF;
		iSetMpeg4H263HWParams.iMAPS = E_OFF;
		}

	TUint maxNumOfPackets;
	// create codec
	TRAPD ( error, iCodec
			= CAriMp4spencWrapper::NewL ( iSetMpeg4H263HWParams ) );

	if ( error != KErrNone )
		{
		// init complete with error message
		iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
		return;
		}

	// set the sync options to the codec
	if ( iClockSource )
		{
		TInt error = iCodec->SetSyncOptions( iClockSource );
		if ( error != KErrNone )
			{
			iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
			return;
			}
		}

		//create engine
		TRAP ( error, iEngine
				= CBaseEngine::NewL( this, ( MBaseCodec* )iCodec, EFalse ) );

	if ( error != KErrNone )
		{
		//init complete with error message
		iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
		return;
		}
	//get the max buffer length from the codec
	TUint maxOutputBufferSize = 0;
	error = iCodec->GetParam( CONTROL_CMD_GET_MAXBUFFERLENGTH,
			&maxOutputBufferSize );

	if ( error != KErrNone )
		{
		//init complete with error message
		iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
		return;
		}

	// if EduCodedPicture mode then check for buffer size passed
	if ( iSetMpeg4H263HWParams.iDataUnitType == EDuCodedPicture )
		{
		if ( iSetMpeg4H263HWParams.iBeforeInitialize & EEncBufferOptions )
			{
			if ( maxOutputBufferSize
					> iSetMpeg4H263HWParams.iMaxCodedPictureSize )
				{
				//init complete with error message
				iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this,
						KErrNotSupported );
				return;
				}
			}
		else
			{
			iSetMpeg4H263HWParams.iMaxOutputBufferSize = maxOutputBufferSize;
			iSetMpeg4H263HWParams.iMaxCodedPictureSize = maxOutputBufferSize;
			}
		}

	// if packet mode is on then allocate memory for NAL information
	iMaxNumOfPackets = 0;
	if ( iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment )
		{
		error = iCodec->GetParam( CONTROL_CMD_GET_MAXNUMOFPACKETS,
				&iMaxNumOfPackets );
		if ( error != KErrNone )
			{
			iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
			return;
			}
		maxNumOfPackets = iMaxNumOfPackets * KDoubleWordLength;
		}

	if ( iSetMpeg4H263HWParams.iDataUnitType != EDuVideoSegment )
		{
			TRAP ( error,CreateCodedOutputBuffersL( (
													maxOutputBufferSize ) ) );
		}

	else
		{
		// Allocate sufficient segment mode buffers
		iSetMpeg4H263HWParams.iMinNumOutputBuffers = ( maxOutputBufferSize
				/ iSetMpeg4H263HWParams.iPacketSize )
				* KMPEG4H263ENCIMPL_MAXNUM_OUTPUTBUFFERS;

		TRAP ( error, CreateInternalOutputBuffersL( maxOutputBufferSize ) );
		if ( error != KErrNone )
			{
			iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
			return;
			}
			TRAP( error, CreatePacketOffsetLengthInfoBuffersL(
														maxNumOfPackets ) );
		if ( error != KErrNone )
			{
			iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
			return;
			}

		TUint* ptr = iFreeBufferQueueForPacketOffsetInfo[0];
		TRAPD ( err, iPacketOffsetBuf = new( ELeave ) TPtr8( ( TUint8* ) ptr,
						maxNumOfPackets, maxNumOfPackets ) );
		if ( err )
			{
			ClientFatalError( KErrNoMemory );
			}

		//segment size  will be the max segment size
		TUint segmentSize = iSetMpeg4H263HWParams.iPacketSize;
		TRAP ( error, CreateCodedOutputBuffersL( segmentSize ) );
		if ( error != KErrNone )
			{
			iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
			return;
			}
		}

	if ( error != KErrNone )
		{
		iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
		return;
		}

	//reset the picture counters
	iPictureCounters.iInputPictures = 0;
	iPictureCounters.iPicturesSkippedRateControl = 0;
	iPictureCounters.iPicturesProcessed = 0;
	// reset the options set by client
	iSetMpeg4H263HWParams.iBeforeInitialize = 0;
	iCurSetMpeg4H263HWParams = iSetMpeg4H263HWParams;
	iEncStateMac->Transit( CStateMachine::EInitializingCommand );
	iEncStateMac->Transit( CStateMachine::EInitializeCommand );
	iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, KErrNone );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Commit all changes since the last CommitL(), Revert() or Initialize()
// to the Hw device
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::CommitL()
	{
	PRINT_ENTRY;

	// This method can only be called after Initialize
	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "CommitL () called before Initialize ()" );
		User::Leave( KErrNotReady );
		}

	// Methods that will be affected by CommitL in our case:
	// SetOutputRectL
	// SetCodingStandardSpecificOptionsL
	// SetErrorsExpected
	// SetMinRandomAccessRate

	TMpeg4H263EncoderInitParams *currentParams =
			new ( ELeave ) TMpeg4H263EncoderInitParams;

	CleanupStack::PushL( currentParams );
	*currentParams = iCurSetMpeg4H263HWParams;

	// Add command apply commit settings to the codec
	iEngine->AddCommandL( CBaseEngine::ENormalPriority,
			CONTROL_CMD_SET_COMMIT_OPTIONS, currentParams );

	iCurSetMpeg4H263HWParams.iAfterInitialize = 0;
	iSetMpeg4H263HWParams = iCurSetMpeg4H263HWParams;
	CleanupStack::Pop( currentParams );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Revert all changes since the last CommitL(), Revert() or Initialize()
// back to their previous settings
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::Revert()
	{
	PRINT_ENTRY;

	// This method can only be called after Initialize
	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	// Methods that will be affected by Revert in our case:
	// SetOutputRectL
	// SetCodingStandardSpecificOptionsL
	// SetErrorsExpected
	// SetMinRandomAccessRate
	iCurSetMpeg4H263HWParams = iSetMpeg4H263HWParams;
	iCurSetMpeg4H263HWParams.iAfterInitialize = 0;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// This method is called only in case of client memory buffers
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::WritePictureL( TVideoPicture* aPicture )
	{
	PRINT_ENTRY;

	// This method should only be called only in following states
	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "WritePictureL () called before Initialize ()" );
		User::Leave( KErrNotReady );
		}

	if ( iEncStateMac->IsInputEndPending() )
		{
    	PRINT_ERR( "Input end is pending.. leaving with KErrEof" );
		User::Leave( KErrEof );
		}

	if ( !aPicture || ( aPicture->iData.iRawData->Length() == 0 )
		|| ( aPicture->iData.iRawData->Length()
		!= ( ( iSetMpeg4H263HWParams.iPictureSize.iWidth
		* iSetMpeg4H263HWParams.iPictureSize.iHeight * 3 ) / 2 ) )
		|| ( aPicture->iData.iDataFormat != EYuvRawData ) )
		{
		PRINT_ERR( "Incorrect parameter passed" );
		User::Leave( KErrArgument );
		return;
		}

	// Added a check to return the input picture if application tries to add
	// pictures after calling InputEnd() or Stop() and before calling
	// Start() again.
	if ( iEncStateMac->IsStopped() )
		{
		SkipInputPicture( aPicture );
		return;
		}

	// counter to represent the num of input pictures received
	iPictureCounters.iInputPictures++;
	if ( IsForcedIFrameRequired( aPicture ) )
		{
		iEngine->AddCommandL( CBaseEngine::ENormalPriority,
				CONTROL_CMD_SET_FORCED_I_FRAME, NULL );
		iPictureLoss = EFalse;
		}

	//Check if clock source is enabled skip logic
	if ( iClockSource )
		{
		if ( ( !CanEncode( aPicture ) ) || iFrozen )
			{
			SkipInputPicture( aPicture );
			return;
			}
		}

    TInt error = iEngine->AddInput( aPicture );
	if ( error != KErrNone )
		{
		PRINT_ERR( "CAriMp4spencHwDeviceImpl::WritePictureL()"
				" AddInput() failed" );
		User::Leave( error );
		return;
		}

	//If output buffer is available for encoding immedietly, only then send it
	// for encoding
	if ( this->iSetMpeg4H263HWParams.iProcessRealtime )
		{
		if ( iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment )
			{
			TInt perPictureSegmentBuffers =
					iSetMpeg4H263HWParams.iMinNumOutputBuffers
							/ KMPEG4H263ENCIMPL_MAXNUM_OUTPUTBUFFERS;
			TInt queueCount = iOutputFreeBufferQueue.Count();
			//  Check for availability of segment buffers
			if ( queueCount < perPictureSegmentBuffers )
				{
				// Sufficient number of segment buffers are not present
    			PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl"
    			"::WritePictureL() Input returned EDuVideoSegment " ) );
				/* Return the earlier input picture added to the queue. */
				iEngine->ReturnInput();
				return;
				}
			}
		else
			{
			//  Check for availability of buffers
			if ( iEngine->NumOutputBuffers() == 0 )
				{
				// Return the earlier input picture added to the queue
    			PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl"
    			"::WritePictureL() Input returned EDuCodedPicture " ) );
				iEngine->ReturnInput();
				return;
				}
			}
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Notifies the hardware device that the end of input data has been reached
// and no more input pictures will be written
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::InputEnd()
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsTransitionValid( CStateMachine::EInputEndCommand ) )
		{
		ClientFatalError( KErrPermissionDenied );
		return;
		}

	if ( iEngine->NumInputBuffers() == 0 )
		{
    	PRINT_MSG( LEVEL_HIGH, ("CAriMp4spencHwDeviceImpl::InputEnds()" ) );
		Stop();
		iMMFDevVideoRecordProxy->MdvrpStreamEnd();
		return;
		}
	else
		{
		iInputEndCalled = ETrue;
		}
	iEncStateMac->Transit( CStateMachine::EInputEndCommand );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Starts recording video
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::Start()
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsPlaying() && !iEncStateMac->IsPaused() )
		{
    	PRINT_MSG( LEVEL_LOW, ( "CAriMp4spencHwDeviceImpl::Start()-already "
    			"in started state, So ignore" ) );
		return;
		}

	if ( !iEncStateMac->IsTransitionValid( CStateMachine::EStartCommand ) )
		{
		ClientFatalError( KErrPermissionDenied );
		return;
		}

	if ( iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment )
		{
		// Enable flag which indicates that all picture params and sequence
		// params are put into 1st buffer
		for ( TInt i = 0; i <= iInternalOutputBufferQueue.Count(); i++ )
			{
			TVideoOutputBuffer *outputBuffer = iInternalOutputBufferQueue[0];
			iEngine->AddOutput( ( TAny* ) outputBuffer );
			iInternalOutputBufferQueue.Remove( 0 );
			}
		}
	else
		{
		// add all output buffers to engine
		for ( TInt i = 0; i <= iOutputFreeBufferQueue.Count(); i++ )
			{
			TVideoOutputBuffer *outputBuffer = iOutputFreeBufferQueue[0];
			iEngine->AddOutput( ( TAny* ) outputBuffer );
			iOutputFreeBufferQueue.Remove( 0 );
			}
		}

	if ( iClockSource )
		{
		TTime lSystemTime;
		lSystemTime.UniversalTime();
		UpdateTime();
		iPeriodicTimer->Start( iPollingInterval, iPollingInterval, TCallBack(
				CAriMp4spencHwDeviceImpl::TimerCallBack, ( TAny* ) this ) );
		}

	iEngine->Start();
    PRINT_MSG( LEVEL_LOW,("CAriMp4spencHwDeviceImpl::Start()Change to start"
    		"state**") ) ;
	iEncStateMac->Transit( CStateMachine::EStartCommand );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Callback function to CPeriodicTimer object
//---------------------------------------------------------------------------
//
TInt CAriMp4spencHwDeviceImpl::TimerCallBack( TAny* aPtr )
	{
	PRINT_ENTRY;

	if ( !aPtr )
		{
		( ( CAriMp4spencHwDeviceImpl* ) aPtr )->ClientFatalError(
															KErrArgument );
        return KErrNone;
		}

	PRINT_EXIT;
	return ( ( CAriMp4spencHwDeviceImpl* ) aPtr )->UpdateTime();
	}

//---------------------------------------------------------------------------
// Stops recording video.
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::Stop()
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsStopped() )
		{
    	PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::Stop() -> already"
    			"stop state, so ignore" ) );
		return;
		}

	if ( !iEncStateMac->IsTransitionValid( CStateMachine::EStopCommand ) )
		{
    	PRINT_MSG( LEVEL_CRITICAL, ("CAriMp4spencHwDeviceImpl::Stop() -> "
    			"fatalerror because Stop called in invalid state" ) );
		ClientFatalError( KErrPermissionDenied );
		return;
		}

	iInputEndCalled = EFalse;
	if ( iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment )
		{
		iTotalOutputBufferLengthInPacketMode = 0;
		iTotalLengthFilledSoFarInPacketMode = 0;
		iPacketsPending = EFalse;
		}

	iEngine->Stop();
	iEngine->Reset();
	iFrozen = EFalse;
	if ( iPeriodicTimer )
		{
		iPeriodicTimer->Cancel();
		}
	iEncStateMac->Transit( CStateMachine::EStopCommand );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Pauses video recording
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::Pause()
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsPaused() )
		{
    	PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::Pause()-> already "
    			" Paused state, so ignore" ) );
		return;
		}

	if ( !iEncStateMac->IsTransitionValid( CStateMachine::EPauseCommand ) )
		{
    	PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::Pause() Pause "
    			"called in invalid state" ) );
		ClientFatalError( KErrPermissionDenied );
		return;
		}

	if ( iClockSource )
		{
		iPeriodicTimer->Cancel();
		//Note down the clock when paued
		iClockTimeWhenPaused = iClockSource->Time().Int64();
		}

	// Stop the engine
	iEngine->Stop();

	//Change the state of the encoder
	iEncStateMac->Transit( CStateMachine::EPauseCommand );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Resumes video recording
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::Resume()
	{
	PRINT_ENTRY;

	// doing it before transitionvalid check because initialize->resume is
	// not added
	if ( iEncStateMac->IsInInitializedState() )
		{
    	PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::Resume()->Got "
    			"resume in initialized state. So go to start()" ) );
		Start();
		return;
		}

	if ( iEncStateMac->IsPlaying() && !iEncStateMac->IsPaused() )
		{
    	PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::Resume()->already "
    			"in playing-not-paused state, so ignore" ) );
		return;
		}

	if ( !iEncStateMac->IsTransitionValid( CStateMachine::EResumeCommand ) )
		{
    	PRINT_MSG( LEVEL_CRITICAL, ("Resume called in invalid state" ) );
		ClientFatalError( KErrPermissionDenied );
		return;
		}

	// No arguments to be passed in Resume
	if ( iClockSource )
		{
		//Clock may or may not be paused, when HWDevice is paused, so
		//add the paused duration to clock
		iPauseOffset = iClockSource->Time().Int64();
		iPauseOffset -= iClockTimeWhenPaused;

		//calculate the total time and deviation
		iTotalTime += iPauseOffset;
		TTime lSystemTime;
		lSystemTime.UniversalTime();

		// Send Update time before sending Resume command, so that clock source
		// is set before
		UpdateTime();
		iPeriodicTimer->Start( iPollingInterval, iPollingInterval, TCallBack(
				CAriMp4spencHwDeviceImpl::TimerCallBack, ( TAny* ) this ) );
		}

	// Start the engine
	iEngine->Start();
	if ( iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment )
		{
		// send the remaining pending packets to client
		while ( iOutputFreeBufferQueue.Count() && iPacketsPending )
			{
			TVideoOutputBuffer *outBuf = iOutputFreeBufferQueue[0];
			FillVideoSegment( outBuf, iInternalOutputBufferQueue[0] );
			iOutputFreeBufferQueue.Remove( 0 );
			iMMFDevVideoRecordProxy->MdvrpNewBuffer( outBuf );
			}
		}

	//Change the state of the encoder
	PRINT_EXIT;
	iEncStateMac->Transit( CStateMachine::EResumeCommand );
	}

//---------------------------------------------------------------------------
// Freezes the input picture
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::Freeze()
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_MSG( LEVEL_CRITICAL,("CAriMp4spencHwDeviceImpl::Freeze() Freeze"
    			" called in invalid state" ) );
		ClientFatalError( KErrNotReady );
		return;
		}
	iFrozen = ETrue;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Releases a frozen input picture
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::ReleaseFreeze()
	{

	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::ReleaseFreeze() "
    			"called in invalid state" ) );
		ClientFatalError( KErrNotReady );
		return;
		}
	iFrozen = EFalse;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Returns the current recording position
//---------------------------------------------------------------------------
//
TTimeIntervalMicroSeconds CAriMp4spencHwDeviceImpl::RecordingPosition()
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
    	PRINT_MSG( LEVEL_HIGH, ("CAriMp4spencHwDeviceImpl::"
    			"RecordingPosition() called in invalid state" ) );
		ClientFatalError( KErrNotReady );
		return TTimeIntervalMicroSeconds( 0 );
		}

	PRINT_EXIT;
	return TTimeIntervalMicroSeconds( iLastEncodedPictureTimestamp );
	}

//---------------------------------------------------------------------------
// Reads various counters related to processed video pictures
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::GetPictureCounters(
		CMMFDevVideoRecord::TPictureCounters& aCounters )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	TUint numofpacktsskipped;
	TUint err = iCodec->GetParam( CONTROL_CMD_GET_NUMOFPICTSKIPPED,
			&numofpacktsskipped );
	iPictureCounters.iPicturesSkippedRateControl
		= iPictureCounters.iPicturesSkippedRateControl + numofpacktsskipped;
	iPictureCounters.iPicturesProcessed = iPictureCounters.iInputPictures
			- iPictureCounters.iPicturesSkippedRateControl;

	aCounters = iPictureCounters;
	//reset the counters
	iPictureCounters.iInputPictures = 0;
	iPictureCounters.iPicturesSkippedRateControl = 0;
	iPictureCounters.iPicturesProcessed = 0;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Reads the frame stabilisation output picture position.
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::GetFrameStabilisationOutput(
												TRect&/* aRect*/)
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	ClientFatalError( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Retrieves the number of complexity control levels available for this
// hardware device
//---------------------------------------------------------------------------
//
TUint CAriMp4spencHwDeviceImpl::NumComplexityLevels()
	{

	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return 0;
		}

	PRINT_EXIT;
	return ( KMPEG4H263ENCIMPL_NUM_COMPLEXITYLEVELS );
	}

//---------------------------------------------------------------------------
// Sets the complexity level to use for video processing in a hardware device
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::SetComplexityLevel( TUint aLevel )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	if ( aLevel >= KMPEG4H263ENCIMPL_NUM_COMPLEXITYLEVELS )
		{
    	PRINT_MSG( LEVEL_CRITICAL, ("CAriMp4spencHwDeviceImpl"
    			"::SetComplexityLevel() Unsupported level Passed" ) );
		ClientFatalError( KErrArgument );
		}

	iSetMpeg4H263HWParams.iComplexityLevel = aLevel;
	TMpeg4H263EncoderInitParams *currentParams = NULL;
	TRAPD ( error, currentParams
			= new ( ELeave ) TMpeg4H263EncoderInitParams );

	*currentParams = iSetMpeg4H263HWParams;

	// Add command apply commit settings to the codec
	TRAP ( error, iEngine->AddCommandL( CBaseEngine::ENormalPriority,
					CONTROL_CMD_SET_COMPLEXITY_LEVEL, currentParams ) );

	if ( error != KErrNone )
		{
		delete currentParams;
		ClientFatalError( error );
		return;
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Retrieves information about the pre-processing capabilities of this
// hardware device.
//---------------------------------------------------------------------------
//
CPreProcessorInfo* CAriMp4spencHwDeviceImpl::PreProcessorInfoLC()
	{
	PRINT_ENTRY;

	TInt cleanupStackPushCount = 0;
	if ( iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "SetErrorProtectionLevelsL () called after Initialize()"
    			"..leaving" );
		User::Leave( KErrPermissionDenied );
		}
	_LIT( KManufacturer, "" );
	_LIT( KIdentifier, "" );

	TPtrC8 implementationSpecificInfo( NULL, 0 );
	RArray<TUncompressedVideoFormat> inputFormats;
	CleanupClosePushL( inputFormats );
	cleanupStackPushCount++;
	inputFormats.Reset();

	RArray<TUncompressedVideoFormat> outputFormats;
	CleanupClosePushL( outputFormats );
	cleanupStackPushCount++;
	outputFormats.Reset();

	RArray<TUint32> supportedCombinations;
	CleanupClosePushL( supportedCombinations );
	cleanupStackPushCount++;
	supportedCombinations.Reset();

	RArray<TScaleFactor> supportedScaleFactors;
	CleanupClosePushL( supportedScaleFactors );
	cleanupStackPushCount++;
	supportedScaleFactors.Reset();

	TYuvToYuvCapabilities yuvToYuvCapabilities;
	TUid uid;
	CPreProcessorInfo *preProcessorInfo = CPreProcessorInfo::NewL( uid.Null(),
			KManufacturer, KIdentifier, TVersion( 0, 0, 0 ), EFalse, EFalse,
			inputFormats.Array(), outputFormats.Array(),
			supportedCombinations.Array(), EFalse, EFalse,
			supportedScaleFactors.Array(), yuvToYuvCapabilities, 0, 0,
			implementationSpecificInfo );

	CleanupStack::PushL( preProcessorInfo );
	CleanupStack::Pop( cleanupStackPushCount );

	PRINT_EXIT;
	return preProcessorInfo;
	}

//---------------------------------------------------------------------------
// Sets the hardware device input format
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetInputFormatL(
		const TUncompressedVideoFormat& aFormat, const TSize& aPictureSize )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "SetInputFormatL () called after Initialize()..leaving" );
		User::Leave( KErrPermissionDenied );
		}

	if ( !CheckInputFormat( aFormat ) )
		{
    	PRINT_ERR("CAriMp4spencHwDeviceImpl"
    	"::SetInputFormatL() Leaving because of not support input format" );
		User::Leave( KErrNotSupported );
		}

	TInt32 aspectRatio = ::MapAspectRatio(
									aFormat.iYuvFormat.iAspectRatioNum,
									aFormat.iYuvFormat.iAspectRatioDenom );
	if ( aspectRatio == -1 )
		{
		User::Leave( KErrNotSupported );
		}

	iSetMpeg4H263HWParams.iAspectRatio = aspectRatio;
	iSetMpeg4H263HWParams.iInputFormat = aFormat;
	iSetMpeg4H263HWParams.iPictureSize = aPictureSize;
	TInt height = iSetMpeg4H263HWParams.iPictureSize.iHeight;
	TInt width = iSetMpeg4H263HWParams.iPictureSize.iWidth;

	if ( height > KMPEG4H263ENCIMPL_720P_HEIGHT ||
			width > KMPEG4H263ENCIMPL_720P_WIDTH )
		{
		User::Leave( KErrNotSupported );
		}
	iSetMpeg4H263HWParams.iBeforeInitialize |= EEncInputFormat;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the data source to be a camera, and sets the device to use direct .
// capture for input
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetSourceCameraL( TInt /*aCameraHandle*/
, TReal /*aPictureRate*/ )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
    	PRINT_ERR( "SetSourceCameraL() called after Initialize()..leaving" );
		User::Leave( KErrPermissionDenied );
		return;
		}
	User::Leave( KErrNotSupported );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the data source to be memory buffers.
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetSourceMemoryL( TReal aMaxPictureRate,
		TBool aConstantPictureRate, TBool aProcessRealtime )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		User::Leave( KErrPermissionDenied );
		return;
		}

	if ( ( aMaxPictureRate <= 0 ) || ( aMaxPictureRate
			> KMPEG4H263ENCIMPL_MAX_PICTURERATE ) )
		{
    	PRINT_ERR( "Incorrect value of max picture rate..leaving" );
		User::Leave( KErrNotSupported );
		return;
		}

	// Check for the picture rates supported
    PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::SetSourceMemoryL() "
    		"Memory Picture rate is set as %f", aMaxPictureRate ) );
	iSetMpeg4H263HWParams.iMaxPictureRate = aMaxPictureRate;
	iSetMpeg4H263HWParams.iConstantPictureRate = aConstantPictureRate;
	iSetMpeg4H263HWParams.iProcessRealtime = aProcessRealtime;
	iSetMpeg4H263HWParams.iBeforeInitialize |= EEncSourceMemory;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the clock source to use for video timing. If no clock
// source is set. video encoding will not be synchronized, but
// will proceed as fast as possible, depending on input data and
// output buffer availability.
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetClockSource( MMMFClockSource* aClock )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrPermissionDenied );
		return;
		}

	if ( !aClock )
		{
		ClientFatalError( KErrArgument );
		return;
		}

    PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::SetClockSource() "
    		"Encoder ClockSource is %x", ( TInt ) aClock ) );
	iClockSource = aClock;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets pre-processing options for RGB to YUV color space conversion
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetRgbToYuvOptionsL( TRgbRange /*aRange*/,
		const TYuvFormat& /*aOutputFormat*/ )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets pre-processing options for YUV to YUV data format conversion
//---------------------------------------------------------------------------
//


void CAriMp4spencHwDeviceImpl::SetYuvToYuvOptionsL(
									const TYuvFormat& /*aInputFormat*/,
									const TYuvFormat& /*aOutputFormat*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetYuvToYuvOptionsL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets the pre-processing types to be used in a hardware device
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetPreProcessTypesL(
												TUint32 /*aPreProcessTypes*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetPreProcessTypesL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets pre-processing options for rotation
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetRotateOptionsL(
											TRotationType /*aRotationType*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetRotateOptionsL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets pre-processing options for scaling
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetScaleOptionsL(
										const TSize& /*aTargetSize*/,
										TBool /*aAntiAliasFiltering*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetScaleOptionsL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets pre-processing options for input cropping
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetInputCropOptionsL(
												const TRect& /*aRect*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetInputCropOptionsL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets pre-processing options for output cropping
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetOutputCropOptionsL(
											const TRect& /*aRect*/)
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetOutputCropOptionsL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets pre-processing options for output padding
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetOutputPadOptionsL(
		const TSize& /*aOutputSize*/, const TPoint& /*aPicturePos*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetOutputPadOptionsL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets color enhancement pre-processing options
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetColorEnhancementOptionsL(
		const TColorEnhancementOptions& /*aOptions*/)
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetColorEnhancementOptionsL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets frame stabilisation options
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetFrameStabilisationOptionsL(
		const TSize& /*aOutputSize*/, TBool /*aFrameStabilisation*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetFrameStabilisationOptionsL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets custom implementation-specific pre-processing options.
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetCustomPreProcessOptionsL(
												const TDesC8& /*aOptions*/)
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetCustomPreProcessOptionsL not supported...leaving \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//  Proxy which recieves callbacks from Hw Device
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SetProxy( MMMFDevVideoRecordProxy& aProxy )
	{
	PRINT_ENTRY;

	if ( iEncStateMac->IsInitialized() 	)
		{
		ClientFatalError( KErrPermissionDenied );
		return;
		}
	iMMFDevVideoRecordProxy = &aProxy;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Callback to indicate the input buffer is consumed
//---------------------------------------------------------------------------
//

TInt CAriMp4spencHwDeviceImpl::InputBufferConsumed( TAny* aInp,
														TInt aError )
	{
	PRINT_ENTRY;

	if ( !aInp )
		{
		return KErrArgument;
		}

	TVideoPicture *picture = (TVideoPicture *) aInp;
	if ( ( picture->iOptions & TVideoPicture::ETimestamp ) && aError
			!= ( KErrCancel ) )
		{
		iLastEncodedPictureTimestamp = picture->iTimestamp.Int64();
		}

	// if custom buffer emabled then add it to queue else return the picture
	// to client
	if ( !iInputBufReturnToPreProc )
		{
		PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl"
		"::InputBufferConsumed()-return picture back to client" ) );

		iMMFDevVideoRecordProxy->MdvrpReturnPicture( picture );
		}
	else
		{
		PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl"
		"::InputBufferConsumed()-return picture back to Input Device" ) );
		iInputDevice->ReturnPicture( picture );
		}

	if ( iInputEndCalled && ( iEngine->NumInputBuffers() == 0 )
			&& ( iCodec->IsCurrentPictureSkipped() ) )
		{
    	PRINT_MSG( LEVEL_HIGH, ("CAriMp4spencHwDeviceImpl"
    			"::InputBufferConsumed()::Calling stream end " ) );

		Stop();
		iMMFDevVideoRecordProxy->MdvrpStreamEnd();
		return ( KErrNone );
		}

	PRINT_EXIT;
	return KErrNone;
	}

//---------------------------------------------------------------------------
// Callback to indicate the output buffer is ready
//---------------------------------------------------------------------------
//

TInt CAriMp4spencHwDeviceImpl::OutputBufferReady( TAny* aOup, TInt aError )
	{
	PRINT_ENTRY;

	TVideoOutputBuffer *outputBuf = ( TVideoOutputBuffer* ) aOup;
	TInt error = KErrNone;
	if ( iFrozen )
		{
		PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::OutputBufferReady()"
				"Frozen state, so drop output picture" ) );

		outputBuf->iData.Set( ( TUint8* ) outputBuf->iData.Ptr(),
				iOutputBufferSize );

		iEngine->AddOutput( outputBuf );
		return KErrNone;
		}

	if ( aError == KErrNone )
		{
		if ( iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment )
			{
			error = iInternalOutputBufferQueue.Append( outputBuf );
			if ( error != KErrNone )
				{
				ClientFatalError( error );
				return KErrNone;
				}

			// get the packet offset infor
			TUint* ptr = iFreeBufferQueueForPacketOffsetInfo[0];
			iPacketOffsetBuf->Set( ( TUint8* ) ptr, iMaxNumOfPackets
				* KDoubleWordLength, iMaxNumOfPackets * KDoubleWordLength);

			error = iCodec->GetParam( CONTROL_CMD_GET_PACKETBOUNDARYDATA,
					iPacketOffsetBuf );
			if ( error != KErrNone )
				{
				ClientFatalError( error );
				return KErrNone;
				}

			//remove the first element and add it to filled Queue
			TUint* filledOffsetLengthInfoBuffer =
					iFreeBufferQueueForPacketOffsetInfo[0];
			iFreeBufferQueueForPacketOffsetInfo.Remove( 0 );
			error = iFilledBufferQueueForPacketOffsetInfo.Append(
					filledOffsetLengthInfoBuffer );
			if ( error != KErrNone )
				{
				ClientFatalError( error );
				return KErrNone;
				}

			if ( !iTotalLengthFilledSoFarInPacketMode )
				{
				iTotalOutputBufferLengthInPacketMode
						= iInternalOutputBufferQueue[0]->iData.Length();
				if ( iTotalOutputBufferLengthInPacketMode != 0 )
					{
					iPacketOffSetCurrentPosition
							= iFilledBufferQueueForPacketOffsetInfo[0];

					iPacketsPending = ETrue;
					while ( iOutputFreeBufferQueue.Count()
							&& iPacketsPending )
						{
						TVideoOutputBuffer *outBuf
							= iOutputFreeBufferQueue[0];

						FillVideoSegment( outBuf,
								iInternalOutputBufferQueue[0] );

						iOutputFreeBufferQueue.Remove( 0 );
						iMMFDevVideoRecordProxy->MdvrpNewBuffer( outBuf );
						}
					}
				else
					{
					iPacketsPending = EFalse;

					// remove packet offset info buffer from filled Q and
					// add it to free Q
					TUint* freeOffsetLengthInfoBuffer =
							iFilledBufferQueueForPacketOffsetInfo[0];
					iFilledBufferQueueForPacketOffsetInfo.Remove( 0 );

					TInt error = iFreeBufferQueueForPacketOffsetInfo .Append(
							freeOffsetLengthInfoBuffer );

					if ( error != KErrNone )
						{
						ClientFatalError( error );
						return KErrNone;
						}

					// Remove the internal buffer and add it back to process
					// engine
					if ( iInternalOutputBufferQueue.Count() )
						{
						TVideoOutputBuffer* outBuf =
								iInternalOutputBufferQueue[0];
						iInternalOutputBufferQueue.Remove( 0 );

						outBuf->iData.Set( ( TUint8* )outBuf->iData.Ptr(),
								iOutputBufferSize );

						//  Add the Buffer back to the Process Engine
						if ( ( !iEncStateMac->IsInputEndPending() )
								&& ( !iEncStateMac->IsStopped() ) )
							{
							iEngine->AddOutput( ( TAny* )outBuf );
							}
						}
					}
				}
			}
		else
			{
			// inform devvideo record that the new encoded buffer is avaibable
			iMMFDevVideoRecordProxy->MdvrpNewBuffer( outputBuf );
			}
		}

	else if ( aError == KErrCancel )
		{
		// add the buffer back  to outputQueue
		outputBuf->iData.Set( ( TUint8* )outputBuf->iData.Ptr(),
				iOutputBufferSize );

		if ( iSetMpeg4H263HWParams.iDataUnitType == EDuVideoSegment )
			{
			error = iInternalOutputBufferQueue.Append( outputBuf );
			if ( error != KErrNone )
				{
				ClientFatalError( error );
				return KErrNone;
				}
			}
		else
			{
			error = iOutputFreeBufferQueue.Append( outputBuf );
			if ( error != KErrNone )
				{
				ClientFatalError( error );
				return KErrNone;
				}
			}

		if ( error != KErrNone )
			{
			ClientFatalError( error );
			return KErrNone;
			}
		}
	else
		{
		ClientFatalError( aError );
		return KErrNone;
		}

	if ( iInputEndCalled && ( iEngine->NumInputBuffers() == 0 ) )
		{
    	PRINT_MSG( LEVEL_HIGH, ("CAriMp4spencHwDeviceImpl"
    			"::OutputBufferReady()::Calling stream end " ) );
		Stop();
		iMMFDevVideoRecordProxy->MdvrpStreamEnd();
		return ( KErrNone );
		}

	PRINT_EXIT;
	return KErrNone;
	}

//---------------------------------------------------------------------------
// Callback to indicate the command has been processed
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::CommandProcessed( TInt aCmd, TAny* aCmdData,
													TInt aError )
	{
	PRINT_ENTRY;

	switch ( aCmd )
		{
		case CONTROL_CMD_SET_RATE_CONTROL_OPTIONS:
			{
			TRateControlOptions *options = ( TRateControlOptions* ) aCmdData;
			delete options;
			}
			break;

		case CONTROL_CMD_SET_FORCED_I_FRAME:
			{
			}
			break;

		case CONTROL_CMD_SET_COMMIT_OPTIONS:
			{
			TMpeg4H263EncoderInitParams *currentParams =
					( TMpeg4H263EncoderInitParams* ) aCmdData;

			if ( ( aError != KErrNone ) || ( aError != KErrCancel ) )
				{
				delete currentParams;
				return;
				}
			else
				{
				iSetMpeg4H263HWParams.iBitErrors = currentParams->iBitErrors;
				iSetMpeg4H263HWParams.iPacketLosses
						= currentParams->iPacketLosses;
				iSetMpeg4H263HWParams.iRandomAccessRate
						= currentParams->iRandomAccessRate;
				delete currentParams;
				}
			}
			break;

		case CONTROL_CMD_SET_COMPLEXITY_LEVEL:
			{
			TMpeg4H263EncoderInitParams *currentParams =
					( TMpeg4H263EncoderInitParams* ) aCmdData;
			delete currentParams;
			}
			break;

		case CONTROL_CMD_SET_CHANNEL_BIT_ERROR_RATE:
			{
			TReal* currentParams = ( TReal* ) aCmdData;
			delete currentParams;
			}
			break;

		case CONTROL_CMD_SET_SLICELOSS:
			{
			TMPEG4H263EncSliceLoss* currentParams =
					( TMPEG4H263EncSliceLoss* ) aCmdData;
			delete currentParams;
			}
			break;

		default:
			break;
		}
	PRINT_EXIT;

	return;
	}

//----------------------------------------------------------------------------
// Called when a fatal error occurs in process engine
//----------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::FatalErrorFromProcessEngine( TInt aError )
	{
	PRINT_ENTRY;
	ClientFatalError( aError );
    PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  2 phase constructor
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::ConstructL()
	{
	PRINT_ENTRY;

	TInt index = 0;
	TInt error = KErrNone;

	// Create Array of Supported Input Formats
	TUncompressedVideoFormat inputFormat;
	inputFormat.iDataFormat = EYuvRawData;
	inputFormat.iYuvFormat.iCoefficients = EYuvBt709Range0;
	inputFormat.iYuvFormat.iPattern = EYuv420Chroma1;
	inputFormat.iYuvFormat.iDataLayout = EYuvDataPlanar;
	inputFormat.iYuvFormat.iYuv2RgbMatrix = NULL;
	inputFormat.iYuvFormat.iRgb2YuvMatrix = NULL;
	inputFormat.iYuvFormat.iAspectRatioNum = 1;
	inputFormat.iYuvFormat.iAspectRatioDenom = 1;

	error = iSupportedInputFormats.Append( inputFormat );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	inputFormat.iYuvFormat.iCoefficients = EYuvBt601Range0;
	error = iSupportedInputFormats.Append( inputFormat );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	inputFormat.iYuvFormat.iPattern = EYuv420Chroma2;
	error = iSupportedInputFormats.Append( inputFormat );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	inputFormat.iYuvFormat.iCoefficients = EYuvBt709Range0;
	error = iSupportedInputFormats.Append( inputFormat );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	inputFormat.iYuvFormat.iCoefficients = EYuvBt709Range1;
	error = iSupportedInputFormats.Append( inputFormat );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	inputFormat.iYuvFormat.iCoefficients = EYuvBt601Range1;
	error = iSupportedInputFormats.Append( inputFormat );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	// Create Array of Supported Output Formats
	CCompressedVideoFormat* compressedVideoFormat[18];

	//Adding mime types for all supported MPEG4 levels.
	compressedVideoFormat[index]
	                       = CCompressedVideoFormat::NewL( KMPEG4MimeType );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;
	error = iLevels.Append( KMPEG4_LEVEL_UNKNOWN );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index]
	                       = CCompressedVideoFormat::NewL( KMPEG4VTMimeType );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_UNKNOWN );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KMPEG4MimeTypeLevel0  );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_0 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KMPEG4MimeTypeLevel0B );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_0B );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KMPEG4MimeTypeLevel1 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_1 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KMPEG4MimeTypeLevel2 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_2 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KMPEG4MimeTypeLevel3 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_3 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KMPEG4MimeTypeLevel4 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_4 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KMPEG4MimeTypeLevel5 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_5 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KMPEG4MimeTypeLevel6 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_6 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KMPEG4MimeTypeLevel7 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_7 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	//Adding mime types for all supported H263 levels.
	compressedVideoFormat[index]
	                       = CCompressedVideoFormat::NewL( KH263MimeType );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KH263_LEVEL_UNKNOWN );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KH263MimeTypeProfile0 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KMPEG4_LEVEL_7 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KH263MimeTypeLevel10 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KH263_LEVEL_10 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KH263MimeTypeLevel20 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KH263_LEVEL_20 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KH263MimeTypeLevel30 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KH263_LEVEL_30 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KH263MimeTypeLevel40 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KH263_LEVEL_40 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	compressedVideoFormat[index] = CCompressedVideoFormat::NewL(
			KH263MimeTypeLevel45 );
	CleanupStack::PushL( compressedVideoFormat[index] );
	index++;

	error = iLevels.Append( KH263_LEVEL_45 );
	if ( error != KErrNone )
		{
		User::Leave( error );
		}

	//Append all the formats to the array
	for ( TInt i = 0; i < index; i++ )
		{
		error = iSupportedOutputFormats.Append( compressedVideoFormat[i] );
		if ( error != KErrNone )
			{
			User::Leave( error );
			}
		}

	iPeriodicTimer = CPeriodic::NewL( CActive::EPriorityIdle );
	iOutputFreeBufferQueue.Reset();
	iInternalOutputBufferQueue.Reset();
	iEncStateMac = CStateMachine::NewL();

	// pop all the pushed items from cleanup stack.
	CleanupStack::Pop( index );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sends the updated time to the codec
//---------------------------------------------------------------------------
//

TInt CAriMp4spencHwDeviceImpl::UpdateTime()
	{
	PRINT_ENTRY;

	if ( !iClockSource )
		{
		ClientFatalError( KErrBadHandle  );
		return -1;
		}
	// send the time values to the codec
	iCodec->SetUpdatedRefernceTime( iTotalTime );

	PRINT_EXIT;
	return 1;
	}

//---------------------------------------------------------------------------
// Create the output buffers in Coded picture mode
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::CreateCodedOutputBuffersL( TUint aSize )
	{
	PRINT_ENTRY;

	if ( iSetMpeg4H263HWParams.iDataUnitType != EDuVideoSegment )
		{
		iOutputBufferSize = aSize;
		}

	// Allocate memory for TVideoOutputBuffer
	iOutputBuffers
	= new ( ELeave ) TVideoOutputBuffer[
	                             iSetMpeg4H263HWParams.iMinNumOutputBuffers];

	for ( TInt i = 0; i < iSetMpeg4H263HWParams.iMinNumOutputBuffers; i++ )
		{
		iOutputBuffers[i].iData.Set( NULL, 0 );
		}

	// Create the Buffer and add it to Queue
	for ( TInt i = 0; i < iSetMpeg4H263HWParams.iMinNumOutputBuffers; i++ )
		{
		TUint8* ptr = new ( ELeave ) TUint8[aSize];
		CleanupStack::PushL( ptr );
		iOutputBuffers[i].iData.Set( ptr, aSize );
		CleanupStack::Pop();
		InitializeOuputCodedBuffer( iOutputBuffers[i] );
		TInt error = iOutputFreeBufferQueue.Append( iOutputBuffers + i );
		if ( error != KErrNone )
			{
			User::Leave( error );
			return;
			}
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Creates the temporary output buffers
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::CreateInternalOutputBuffersL(
														TUint aBufferSize )
	{
	PRINT_ENTRY;

	iOutputBufferSize = aBufferSize;

	// Allocate memory for TVideoOutputBuffer
	iInternalOutputBuffers
			= new ( ELeave ) TVideoOutputBuffer
					[KMPEG4H263ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS];

	for ( TInt i = 0; i < KMPEG4H263ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
		{
		iInternalOutputBuffers[i].iData.Set( NULL, 0 );
		}

	// Create the Buffer and add it to Queue
	for ( TInt i = 0; i < KMPEG4H263ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
		{
		TUint8* ptr = new ( ELeave ) TUint8[aBufferSize];
		CleanupStack::PushL( ptr );
		iInternalOutputBuffers[i].iData.Set( ptr, aBufferSize );
		CleanupStack::Pop();
		InitializeOuputCodedBuffer( iInternalOutputBuffers[i] );
		TInt error = iInternalOutputBufferQueue.Append(
				iInternalOutputBuffers + i );

		if ( error != KErrNone )
			{
			ClientFatalError( error );
			return;
			}
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Creates the buffers required to store length and offset
// info of packets
//---------------------------------------------------------------------------
//


void CAriMp4spencHwDeviceImpl::CreatePacketOffsetLengthInfoBuffersL(
		TUint aNumOfPackets )
	{
	PRINT_ENTRY;
	iPacketOffSetAndLengthInfoBuffers
		= new ( ELeave ) (TUint*[KMPEG4H263ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS]);

	for ( TInt i = 0; i < KMPEG4H263ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
		{
		iPacketOffSetAndLengthInfoBuffers[i] = NULL;
		}

	for ( TInt i = 0; i < KMPEG4H263ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
		{
		iPacketOffSetAndLengthInfoBuffers[i]
				= ( TUint* ) ( new ( ELeave ) TUint8[aNumOfPackets] );
		TInt error = iFreeBufferQueueForPacketOffsetInfo.Append(
				iPacketOffSetAndLengthInfoBuffers[i] );
		if ( error != KErrNone )
			{
			User::Leave( error );
			}
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// decides whether picture can be encoded or not
//---------------------------------------------------------------------------
//

TBool CAriMp4spencHwDeviceImpl::CanEncode( TVideoPicture *aPicture )
	{
	PRINT_ENTRY;

	// check with the lastly encoded picture  and decide whether it can be
	//	processed or skipped
	if ( aPicture->iTimestamp.Int64() < iLastEncodedPictureTimestamp )
		{
		return EFalse;
		}

	// check with current clock
	if ( ( aPicture->iTimestamp.Int64() ) < ( ( iClockSource->Time().Int64()
			- iTotalTime ) ) )
		{
		return EFalse;
		}

	PRINT_EXIT;
	return ETrue;
	}

//---------------------------------------------------------------------------
//  Skips the Input Picture
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::SkipInputPicture( TVideoPicture *aPicture )
	{
	PRINT_ENTRY;

	// add the buffer back to queue
	if ( !iInputBufReturnToPreProc )
		{
		PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::SkipInputPicture()"
				"-return picture back to client" ) );
		iMMFDevVideoRecordProxy->MdvrpReturnPicture( aPicture );
		}
	else
		{
		PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::SkipInputPicture()-"
				"return picture back to Input Device" ) );
		iInputDevice->ReturnPicture( aPicture );
		}

	if ( !iFrozen )
		{
		iPictureCounters.iPicturesSkippedRateControl++;
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Indicates whether the next frame is to be encoded as an I frame
//---------------------------------------------------------------------------
//
TBool CAriMp4spencHwDeviceImpl::IsForcedIFrameRequired(
		TVideoPicture* aPicture )
	{
	PRINT_ENTRY;

	if ( ( aPicture->iOptions & TVideoPicture::EReqInstantRefresh )
			|| iPictureLoss )
		{
		PRINT_MSG( LEVEL_HIGH, ("CAriMp4spencHwDeviceImpl"
				"::IsForcedIFrameRequired() ETrue 1--" ) );
		return ETrue;
		}

	PRINT_EXIT;
	return EFalse;
	}

//---------------------------------------------------------------------------
//  Extracts 1 packet from a frame and fills output buffer with the same
//---------------------------------------------------------------------------
//
void CAriMp4spencHwDeviceImpl::FillVideoSegment(
		TVideoOutputBuffer* aOutputBuf, TVideoOutputBuffer* aSrcOutputBuf )
	{
	PRINT_ENTRY;

	TUint currentPacketLength = *( iPacketOffSetCurrentPosition );
	PRINT_MSG( LEVEL_LOW, ("CAriMp4spencHwDeviceImpl::FillVideoSegment()"
				" currentPacketLength is %d", ( TInt )currentPacketLength ) );

	if ( iTotalLengthFilledSoFarInPacketMode
			< iTotalOutputBufferLengthInPacketMode )
		{
		Mem::Copy( ( TUint8* ) ( aOutputBuf->iData.Ptr() ),
			( TUint8* ) ( aSrcOutputBuf->iData.Ptr()
			+ iTotalLengthFilledSoFarInPacketMode ), currentPacketLength );

		aOutputBuf->iData.Set( aOutputBuf->iData.Ptr(), currentPacketLength );

		iTotalLengthFilledSoFarInPacketMode
				= iTotalLengthFilledSoFarInPacketMode + currentPacketLength;

		if ( iTotalLengthFilledSoFarInPacketMode
				== iTotalOutputBufferLengthInPacketMode )
			{
			iTotalLengthFilledSoFarInPacketMode = 0;
			iPacketsPending = EFalse;

			// remove packet offset info buffer from filled Q and add it
			// to free Q
			TUint* freeOffsetLengthInfoBuffer =
					iFilledBufferQueueForPacketOffsetInfo[0];
			iFilledBufferQueueForPacketOffsetInfo.Remove( 0 );
			TInt error = iFreeBufferQueueForPacketOffsetInfo.Append(
					freeOffsetLengthInfoBuffer );
			if ( error != KErrNone )
				{
				ClientFatalError( error );
				return;
				}

			// Remove the internal buffer and add it back to process engine
			if ( iInternalOutputBufferQueue.Count() )
				{
				TVideoOutputBuffer* outBuf = iInternalOutputBufferQueue[0];
				iInternalOutputBufferQueue.Remove( 0 );
				outBuf->iData.Set( ( TUint8* )outBuf->iData.Ptr(),
						iOutputBufferSize );

				//  Add the Buffer back to the Process Engine
				if ( ( !iEncStateMac->IsInputEndPending() )
						&& ( !iEncStateMac->IsStopped() ) )
					{
					iEngine->AddOutput( ( TAny* )outBuf );
					}
				}

			// Still more encoded packets are available
			if ( iInternalOutputBufferQueue.Count() )
				{
				iPacketsPending = ETrue;
				iTotalOutputBufferLengthInPacketMode
						= iInternalOutputBufferQueue[0]->iData.Length();

				// Get the packet offset info buffer
				// is this check required - will be valid always
				if ( iFilledBufferQueueForPacketOffsetInfo.Count() )
					{
					iPacketOffSetCurrentPosition
							= iFilledBufferQueueForPacketOffsetInfo[0];
					}
				}
			}
		else
			{
			++iPacketOffSetCurrentPosition;
			}
		}

	// Update the specific members of the output buffer
	aOutputBuf->iRequiredSeveralPictures
			= aSrcOutputBuf->iRequiredSeveralPictures;
	aOutputBuf->iRandomAccessPoint = aSrcOutputBuf->iRandomAccessPoint;
	aOutputBuf->iCaptureTimestamp = aSrcOutputBuf->iCaptureTimestamp;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Initializes the members of the output coded buffers created
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::InitializeOuputCodedBuffer(
		TVideoOutputBuffer& aOutputBuffer )
	{
	PRINT_ENTRY;
	aOutputBuffer.iOrderNumber = 0;
	aOutputBuffer.iMinErrorProtectionLevel = 1;
	aOutputBuffer.iMaxErrorProtectionLevel = 1;
	aOutputBuffer.iRequiredThisPicture = EFalse;
	aOutputBuffer.iLayer = 0;
	aOutputBuffer.iInLayerScalabilityStep = 0;
	aOutputBuffer.iDataPartitionNumber = 0;
	aOutputBuffer.iHrdVbvParams.Set( NULL, 0 );
	aOutputBuffer.iCodingStandardSpecificData.Set( NULL, 0 );
	aOutputBuffer.iImplementationSpecificData.Set( NULL, 0 );

	// The following members will be modified later during execution
	aOutputBuffer.iRequiredSeveralPictures = EFalse;
	aOutputBuffer.iRandomAccessPoint = EFalse;
	aOutputBuffer.iCaptureTimestamp = 0;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Checks if the specified input format is supported or not
//---------------------------------------------------------------------------
//

TBool CAriMp4spencHwDeviceImpl::CheckInputFormat(
		const TUncompressedVideoFormat& aFormat )
	{
	PRINT_ENTRY;

	TInt i = 0;
	for ( TInt i = 0; i < iSupportedInputFormats.Count(); i++ )
		{
		TUncompressedVideoFormat inputFormat = iSupportedInputFormats[i];
		if ( inputFormat == aFormat )
			return ETrue;
		}
	PRINT_EXIT;

	return EFalse;
	}

//---------------------------------------------------------------------------
//  Nofities the client that the fatal error happend in Hw device
//---------------------------------------------------------------------------
//

void CAriMp4spencHwDeviceImpl::ClientFatalError( TInt aError )
	{
	PRINT_ENTRY;

	PRINT_MSG( LEVEL_CRITICAL, ("CAriMp4spencHwDeviceImpl::ClientFatalError()"
			" Error is %d", aError ) );
	if ( iClockSource )
		{
		iPeriodicTimer->Cancel();
		}

	// Stop processing
	if ( !iEncStateMac->IsStopped() )
		{
		if ( iEncStateMac->IsInitialized() )
			{
			Stop();
			}
		}
	iEncStateMac->Transit( CStateMachine::EDeadStateCommand );
	iMMFDevVideoRecordProxy->MdvrpFatalError( this, aError );

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// The implementation table entry which indicates the 1st function
// to call when Mpeg4-h263 encoder hwdevice plugin is selected
//----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
	{ KUidMpeg4H263EncoderHwDeviceImplUid,
				( TProxyNewLPtr )( CAriMp4spencHwDeviceImpl::NewL ) }
    };

//----------------------------------------------------------------------------
// Returns the implementation table
//----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(
												TInt& aTableCount )
{
	aTableCount = sizeof( ImplementationTable ) /
						sizeof( TImplementationProxy );
	return ImplementationTable;
}
