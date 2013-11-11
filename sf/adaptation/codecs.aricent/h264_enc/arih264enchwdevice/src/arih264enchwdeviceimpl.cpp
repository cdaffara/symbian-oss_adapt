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
* Implementation of member functions of H264 encoder plugin class -
* CAriH264encHwDeviceImpl.
*
*/

// System Includes
#include <e32base.h>
#include <avc.h>
#include <e32math.h>

// User Includes
#include "arih264enchwdeviceimpl.h"
#include "arih264hwdeviceconstants.h"
#include "arih264encwrapper.h"



// MACRO DEFINITIONS - start

// Uncomment this flag to enable instant rate calculations on-the-fly in urel
// builds. In UDEB it is enabled automatically
// By default it is commented
//#define CALCINSTANTBITRATE

// Uncomment to print additional debug information retrieved from
// core encoder
//#define DEBUG_INFO

// MACRO DEFINITIONS - end

//----------------------------------------------------------------------------
//  Maps the num/denom from the MDF to the aspect_ratio_idc value supported
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
//  Two phase constructor for an object of CAriH264encHwDeviceImpl
//----------------------------------------------------------------------------
//
CAriH264encHwDeviceImpl* CAriH264encHwDeviceImpl::NewL()
	{
	PRINT_ENTRY;
    CAriH264encHwDeviceImpl* self = new ( ELeave ) CAriH264encHwDeviceImpl();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
	PRINT_EXIT;
    return self;
	}

//----------------------------------------------------------------------------
//  Default constructor
//----------------------------------------------------------------------------
//
CAriH264encHwDeviceImpl::CAriH264encHwDeviceImpl()
    :iMMFDevVideoRecordProxy( NULL ),
     iInputDevice ( NULL ),
     iClockSource ( NULL ),
	 iInputBufReturnToPreProc( EFalse ),
     iPeriodicTimer ( NULL ),
	 iClockTimeWhenPaused( 0 ),
	 //100 milli seconds
     iPollingInterval(TTimeIntervalMicroSeconds32( KPOLLINGINTERVAL ) ),
     iCodec( NULL ),
     iEngine( NULL ),
	 iOutputBuffers( NULL ),
     iOutputBufferSize( 0 ),
     iPauseOffset( 0 ),
     iTotalTime( 0 ),
     iLastEncodedPictureTimestamp( 0 ),
	 iPictureLoss( EFalse ),
	 iInputEndCalled( EFalse ),
	 iFrozen( EFalse ),
	 iTotalLengthFilledSoFarInPacketMode( 0 ),
	 iTotalOutputBufferLengthInPacketMode( 0 ),
	 iPacketOffSetCurrentPosition( NULL ),
	 iPacketOffSetAndLengthInfoBuffers( NULL ),
	 iInternalOutputBuffers( NULL ),
	 iNumOfBuffersAtClientInPacketModeNeedReAllocation( 0 ),
	 iPacketsPending( EFalse ),
	 iNumOfOutBuffersAvailableInPacketModeChange( 0 ),
	 iNumofBuffersPendingFromClientInChangeToPacketMode( 0 ),
	 iNumOfOutBuffersBeforeChangeToPacketMode( 0 ),
	 iCodedBufferForPacketModeChange( NULL ),
	 iSegmentBuffers( NULL ),
	 iIsConfigDataFilledInFirstOutputBuffer( EFalse ),
	 iIsSliceEnabledInCodedPicture( EFalse ),
	 iNoOfOutputFramesPerSec( 0 ),
	 iSizePerFrame( 0 ),
	 iEncStateMac( NULL )
	{
	PRINT_ENTRY;

	iSupportedDataUnitTypes = EDuCodedPicture | EDuVideoSegment;

	iSupportedDataUnitEncapsulations = EDuElementaryStream
									   | EDuGenericPayload;

    // Default values for Init params - full range
    TUncompressedVideoFormat inputFormat;
    inputFormat.iDataFormat = EYuvRawData;
    inputFormat.iYuvFormat.iCoefficients = EYuvBt709Range1;
    inputFormat.iYuvFormat.iPattern = EYuv420Chroma1;
    inputFormat.iYuvFormat.iDataLayout = EYuvDataPlanar;
    inputFormat.iYuvFormat.iYuv2RgbMatrix = NULL;
    inputFormat.iYuvFormat.iRgb2YuvMatrix = NULL;
    inputFormat.iYuvFormat.iAspectRatioNum = 1;
    inputFormat.iYuvFormat.iAspectRatioDenom = 1;

    iH264EncInitParams.iInputFormat = inputFormat;

    iH264EncInitParams.iLevel = KH264ENCIMPL_LEVEL_UNKNOWN;

	iH264EncInitParams.iAfterInitialize = 0;

    iH264EncInitParams.iOutputFormat        = EH264;

    iH264EncInitParams.iMinNumOutputBuffers
		= KH264ENCIMPL_MAXNUM_OUTPUTBUFFERS;

	iH264EncInitParams.iMaxCodedSegmentSize
		= KH264ENCIMPL_DEFAULT_SEGMENTSIZE;

	iH264EncInitParams.iMaxPictureRate = KH264ENCIMPL_DEFAULT_PICTURERATE;

	// initialize picture counters
	iPictureCounters.iPicturesSkippedBufferOverflow = 0;
	iPictureCounters.iPicturesSkippedProcPower		= 0;
	iPictureCounters.iPicturesSkippedRateControl	= 0;
	iPictureCounters.iPicturesProcessed				= 0;
	iPictureCounters.iInputPictures					= 0;

	// default packet mode is off
	iH264EncInitParams.iDataEncapsulation 	= EDuElementaryStream;

    iH264EncInitParams.iDataUnitType		= EDuCodedPicture;

	iH264EncInitParams.iBitRate = KH264ENCIMPL_DEFAULT_BITRATE;
    iH264EncInitParams.iTargetPictureRate = KH264ENCIMPL_DEFAULT_PICTURERATE;
    iH264EncInitParams.iRandomAccessRate
		= KH264ENCIMPL_DEFAULT_RANDOMACCESSRATE;

    PRINT_EXIT;

	}

//----------------------------------------------------------------------------
//  Destructor
//----------------------------------------------------------------------------
//
CAriH264encHwDeviceImpl::~CAriH264encHwDeviceImpl()
	{
	PRINT_ENTRY;
	PRINT_MSG( LEVEL_LOW, ( "this is %x", this ) );

    iSupportedInputFormats.Close();

	iLevels.Close();

    //allocated formats should be deleted before closing
    while ( iSupportedOutputFormats.Count() > 0 )
		{
        CCompressedVideoFormat* lCompFormat =   iSupportedOutputFormats[0];
        iSupportedOutputFormats.Remove( 0 );
        delete lCompFormat;
		}

    iSupportedOutputFormats.Close();

	// Stop processing
	if( !iEncStateMac->IsStopped() && ( !iEncStateMac->IsInDeadState() ) )
		{
		if( iEncStateMac->IsInitialized() )
			{
			Stop();
			}
		}

	if( iEngine )
		{
		iEngine->Reset();
		delete iEngine;
		iEngine = NULL;
		}

	if( iCodec )
		{
		delete iCodec;
		iCodec = NULL;
		}

    //delete output buffers
    iOutputFreeBufferQueue.Reset();

    if( iOutputBuffers )
		{
		if( iNumOfOutBuffersBeforeChangeToPacketMode )
			{
			for ( TInt i = 0; i < iNumOfOutBuffersBeforeChangeToPacketMode
				; i++ )
				{
				if ( iOutputBuffers[i].iData.Ptr() )
					{
					delete ( TUint8* )iOutputBuffers[i].iData.Ptr();
					}
				}
			}
		else
			{
			for ( TInt i = 0; i < iH264EncInitParams.iMinNumOutputBuffers;
				i++ )
				{
				if ( iOutputBuffers[i].iData.Ptr() )
					{
					delete ( TUint8* )iOutputBuffers[i].iData.Ptr();
					}
				}
			}
		delete []iOutputBuffers;
        }

    delete iPeriodicTimer;
    iPeriodicTimer = NULL;

	if ( iPacketOffSetAndLengthInfoBuffers )
		{
		for( TInt i = 0; i < KH264ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
			{
			delete [] ( TUint8* )iPacketOffSetAndLengthInfoBuffers[i];
			}
		delete []( TUint* )iPacketOffSetAndLengthInfoBuffers;
		}

    //delete temp output buffers
    iInternalOutputBufferQueue.Reset();

    if( iInternalOutputBuffers )
        {
        for ( TInt i = 0; i < KH264ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
            {
            if ( iInternalOutputBuffers[i].iData.Ptr() )
                {
                delete ( TUint8* )iInternalOutputBuffers[i].iData.Ptr();
                }
            }
		delete []iInternalOutputBuffers;
        }

	if ( iCodedBufferForPacketModeChange )
		{
		if ( iCodedBufferForPacketModeChange->iData.Ptr() )
			{
			delete iCodedBufferForPacketModeChange->iData.Ptr();
			}

		delete iCodedBufferForPacketModeChange;
		}

	iFreeBufferQueueForPacketOffsetInfo.Reset();
	iFilledBufferQueueForPacketOffsetInfo.Reset();

	//delete the segment buffers
    if( iSegmentBuffers )
        {
        for ( TInt i = 0; i < iH264EncInitParams.iMinNumOutputBuffers; i++ )
            {
            if ( iSegmentBuffers[i].iData.Ptr() )
                {
                delete ( TUint8* )iSegmentBuffers[i].iData.Ptr();
                }
            }
		delete []iSegmentBuffers;
        }

    if ( iConfigData )
    	{
    	delete iConfigData;
    	iConfigData = NULL;
    	}

    if ( iEncStateMac )
        {
    	delete iEncStateMac;
    	iEncStateMac = NULL;
        }
	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Custom Interface supported by the HwDevice plugin
//----------------------------------------------------------------------------
//
TAny* CAriH264encHwDeviceImpl::CustomInterface( TUid  aInterface )
	{
	PRINT_ENTRY;
	if( aInterface == KH324AnnexKDefinedEncoderConfigDataCIUid )
		{
		PRINT_EXIT;
		return ( ( MH324AnnexKDefinedEncoderConfigDataCI* ) this );
		}
	else
		{
		PRINT_EXIT;
		return NULL;
		}
	}

//----------------------------------------------------------------------------
//  Returns information about this Encoder HW Device
//----------------------------------------------------------------------------
//
CVideoEncoderInfo* CAriH264encHwDeviceImpl::VideoEncoderInfoLC()
	{
	PRINT_ENTRY;

    TSize maxPictureSize = TSize( KH264ENCIMPL_SDTV_WIDTH,
    		KH264ENCIMPL_SDTV_HEIGHT_PAL );

    TUint32 maxBitRate = KMAXSUPPBITRATE;
    RArray<TPictureRateAndSize> maxPictureRatesAndSizes;
    CleanupClosePushL( maxPictureRatesAndSizes );

    TPictureRateAndSize pictureRateAndSize;
    pictureRateAndSize.iPictureRate = KPictureRate30;
    pictureRateAndSize.iPictureSize = TSize( KH264ENCIMPL_QCIF_WIDTH,
											  KH264ENCIMPL_QCIF_HEIGHT );

	TInt error = KErrNone;
    error = maxPictureRatesAndSizes.Append( pictureRateAndSize );

	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending maximum picture rate and size" );
		User::Leave( error );
		}

    pictureRateAndSize.iPictureRate = KPictureRate30;
    pictureRateAndSize.iPictureSize = TSize( KH264ENCIMPL_VGA_WIDTH,
												KH264ENCIMPL_VGA_HEIGHT );

    error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending maximum picture rate and size" );
		User::Leave( error );
		}

    pictureRateAndSize.iPictureRate = KPictureRate30;
    pictureRateAndSize.iPictureSize = TSize( KH264ENCIMPL_CIF_WIDTH,
											  KH264ENCIMPL_CIF_HEIGHT );

    error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending maximum picture rate and size" );
		User::Leave( error );
		}

    pictureRateAndSize.iPictureRate = KPictureRate30;
    pictureRateAndSize.iPictureSize = TSize( KH264ENCIMPL_SQCIF_WIDTH,
											 KH264ENCIMPL_SQCIF_HEIGHT );

    error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending maximum picture rate and size" );
		User::Leave( error );
		}

    pictureRateAndSize.iPictureRate = KPictureRate30;
    pictureRateAndSize.iPictureSize = TSize( KH264ENCIMPL_QVGA_WIDTH,
											 KH264ENCIMPL_QVGA_HEIGHT );

    error = maxPictureRatesAndSizes.Append( pictureRateAndSize );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending maximum picture rate and size" );
		User::Leave( error );
		}

    pictureRateAndSize.iPictureRate = KPictureRate30;
    pictureRateAndSize.iPictureSize = TSize( KH264ENCIMPL_SDTV_WIDTH,
										 KH264ENCIMPL_HALFSDTV_HEIGHT_NTSC);

    error = maxPictureRatesAndSizes.Append( pictureRateAndSize );

	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending maximum picture rate and size" );
		User::Leave( error );
		}

    pictureRateAndSize.iPictureRate = KPictureRate30;
    pictureRateAndSize.iPictureSize = TSize( KH264ENCIMPL_SDTV_WIDTH,
										 KH264ENCIMPL_HALFSDTV_HEIGHT_PAL );

    error = maxPictureRatesAndSizes.Append( pictureRateAndSize );

	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending maximum picture rate and size" );
		User::Leave( error );
		}

    pictureRateAndSize.iPictureRate = KPictureRate30;
    pictureRateAndSize.iPictureSize = TSize( KH264ENCIMPL_SDTV_WIDTH,
											KH264ENCIMPL_SDTV_HEIGHT_NTSC );

    error = maxPictureRatesAndSizes.Append( pictureRateAndSize );

	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending maximum picture rate and size" );
		User::Leave( error );
		}

    pictureRateAndSize.iPictureRate = KH264ENCIMPL_PICTURERATE_25;
    pictureRateAndSize.iPictureSize = TSize( KH264ENCIMPL_SDTV_WIDTH,
											KH264ENCIMPL_SDTV_HEIGHT_PAL );

    error = maxPictureRatesAndSizes.Append( pictureRateAndSize );

	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending maximum picture rate and size" );
		User::Leave( error );
		}

    TUint32 supportedPictureOptions = TVideoPicture::ETimestamp |
										TVideoPicture::EReqInstantRefresh;

	TUint32 supportedDataUnitEncapsulations = EDuElementaryStream |
												EDuGenericPayload;

	TUint32 supportedDataUnitTypes			 = EDuCodedPicture |
												EDuVideoSegment;

    CVideoEncoderInfo* lVideoEncoderInfo = CVideoEncoderInfo::NewL( TUid::
										Uid( KUidH264EncoderHwDeviceImplUid ),
										KH264EncManufacturer,
										KH264EncIdentifier,
										TVersion( KH264ENCIMPL_MAJOR_VERSION,
										KH264ENCIMPL_MINOR_VERSION,
										KH264ENCIMPL_BUILD_VERSION ),
										// Accelerated
										EFalse,
										// Enc doesnt support direct capture
										EFalse,
										iSupportedInputFormats.Array(),
										iSupportedOutputFormats.Array(),
										maxPictureSize,
										supportedDataUnitTypes,
										supportedDataUnitEncapsulations,
										// Max bitrate layers
										1,
										//aSupportsSupplementalEnhancementInfo
										EFalse,
										//aMaxUnequalErrorProtectionLevels
										1,
										maxBitRate,
										maxPictureRatesAndSizes.Array(),
										//aMaxInLayerScalabilitySteps
										1,
										supportedPictureOptions,
										//aSupportsPictureLoss
										ETrue,
										//aSupportsSliceLoss
										ETrue,
										//aCodingStandardSpecificInfo
										KNullDesC8,
										//aImplementationSpecificInfo
										KNullDesC8 );

    CleanupStack::PopAndDestroy();
    CleanupStack::PushL( lVideoEncoderInfo );

	PRINT_EXIT;
    return lVideoEncoderInfo;
	}


//----------------------------------------------------------------------------
//  Sets the encoder output format
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetOutputFormatL(
							const CCompressedVideoFormat& aFormat,
							TVideoDataUnitType aDataUnitType,
							TVideoDataUnitEncapsulation aDataEncapsulation,
							TBool aSegmentationAllowed )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
		{
		PRINT_ERR( "SetOutputFormatL () called before Initialize ()" );
        User::Leave( KErrPermissionDenied );
		}

    TInt error = KErrNotFound;

    TPtrC8 mimeType( aFormat.MimeType() );

    // Check if format is supported else return KErrNotSupported
    for ( TInt i = 0; i < iSupportedOutputFormats.Count(); i++ )
		{
        CCompressedVideoFormat* lFormat = iSupportedOutputFormats[i];

        if( mimeType.CompareF( lFormat->MimeType() ) == 0 )
			{
            iH264EncInitParams.iLevel = iLevels[i];
            error = KErrNone;
            break;
			}
		}

    if( error == KErrNotFound )
		{
		PRINT_ERR( "CAriH264encHwDeviceImpl::SetOutputFormatL() Leaving"
				   " because of unsupported output mimetype" );

        User::Leave( KErrNotSupported );
        return;
		}

    PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetOutputFormatL() "
    		"level is set to %d", (TInt)iH264EncInitParams.iLevel ) );

    if ( ( ( aDataUnitType != EDuCodedPicture ) &&
    	( aDataUnitType != EDuVideoSegment ) ) ||
		( ( aDataEncapsulation != EDuElementaryStream ) &&
		( aDataEncapsulation != EDuGenericPayload ) ) ||
		( ( aSegmentationAllowed ) ) )
		{
		PRINT_ERR( "CAriH264encHwDeviceImpl::SetOutputFormatL() Leaving"
			" because of unsupported data unit type or data unit"
			" encapsulation" );
        User::Leave( KErrNotSupported );
		}

    if( aDataUnitType == EDuCodedPicture )
		{
		PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetOutputFormatL() "
				"Packet mode is OFF" ) );
		}
    else
		{
		PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetOutputFormatL() "
				"Packet mode is ON" ) );
		// must be calculated from and levels set by the client
		iH264EncInitParams.iMinNumOutputBuffers
					= KH264ENCIMPL_MAXNUM_SEGMENTBUFFERS;
		}

    iH264EncInitParams.iDataUnitType = aDataUnitType;
    iH264EncInitParams.iDataEncapsulation = aDataEncapsulation;
    iH264EncInitParams.iSegmentationAllowed = aSegmentationAllowed;
    iH264EncInitParams.iBeforeInitialize |= EEncOutputFormat;

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Sets the pre-processor device that will write data to this encoder
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetInputDevice(
										CMMFVideoPreProcHwDevice* aDevice )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
		{
        ClientFatalError ( KErrPermissionDenied );
        return;
		}

	if( !aDevice )
		{
        ClientFatalError ( KErrArgument );
        return;
		}

    iInputDevice = aDevice;
    iH264EncInitParams.iBeforeInitialize |= EEncInputDevice;
    iInputBufReturnToPreProc			=	 ETrue;

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Sets the number of bit-rate scalability layers to use
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetNumBitrateLayersL( TUint aNumLayers )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
		{
		PRINT_ERR( "SetNumBitrateLayersL () called before Initialize"
				"..leaving" );
        User::Leave( KErrPermissionDenied );
		}

    if( aNumLayers != 1 )
		{
		PRINT_ERR( "Wrong value passed for aNumLayers... Leaving" );
        User::Leave( KErrNotSupported );
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Sets the scalability type for a bit-rate scalability layer. Currently not
//  supported.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetScalabilityLayerTypeL(
									TUint /*aLayer*/,
									TScalabilityType /*aScalabilityType*/ )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
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


//----------------------------------------------------------------------------
//  Sets the reference picture options to be used for all scalability layers
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetGlobalReferenceOptions(
										TUint aMaxReferencePictures,
										TUint aMaxPictureOrderDelay )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
		{
        ClientFatalError( KErrPermissionDenied );
        return;
		}

    if( aMaxPictureOrderDelay != 0 || aMaxReferencePictures
    		> KH264ENCIMPL_MAXNUM_REFERENCEPICTURES )
		{
        ClientFatalError( KErrNotSupported );
        return;
		}

    iH264EncInitParams.iLayerReferenceOptions[0].iMaxReferencePictures
    	= aMaxReferencePictures;

    iH264EncInitParams.iLayerReferenceOptions[0].iMaxPictureOrderDelay
		= aMaxPictureOrderDelay;

    iH264EncInitParams.iBeforeInitialize |= EEncGlobalRefOptions;

    PRINT_EXIT
	}

//----------------------------------------------------------------------------
// Sets the reference picture options to be used for a single scalability
// layer
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetLayerReferenceOptions( TUint /*aLayer*/,
											TUint /*aMaxReferencePictures*/,
											TUint /*aMaxPictureOrderDelay*/ )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
		{
        ClientFatalError( KErrPermissionDenied );
        return;
		}
	ClientFatalError( KErrNotSupported );

	PRINT_EXIT;
	}

///---------------------------------------------------------------------------
// Sets encoder buffering options
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetBufferOptionsL(
									const TEncoderBufferOptions& aOptions )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
		{
		PRINT_ERR( "SetBufferOptionsL () called before Initialize ()" );
        User::Leave( KErrPermissionDenied );
		}

    PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetBufferOptionsL() "
    		"iMaxPreEncoderBufferPictures = %d" ,
    		(TInt)aOptions.iMaxPreEncoderBufferPictures ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetBufferOptionsL() "
    		"iMaxOutputBufferSize = %d",
    		( TInt ) aOptions.iMaxOutputBufferSize ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetBufferOptionsL() "
    		"iMaxCodedPictureSize = %d" ,
    		( TInt )aOptions.iMaxCodedPictureSize ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetBufferOptionsL() "
    		"iHrdVbvSpec = %x", ( TInt ) aOptions.iHrdVbvSpec ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetBufferOptionsL() "
    		"iMinNumOutputBuffers = %d" ,
    		( TInt ) aOptions.iMinNumOutputBuffers ) );
    PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetBufferOptionsL() "
    		"iMaxCodedSegmentSize = %d" ,
			( TInt ) aOptions.iMaxCodedSegmentSize ) );

    if( ( aOptions.iMaxPreEncoderBufferPictures == 0 ) ||
        ( aOptions.iMaxOutputBufferSize == 0 ) ||
        ( aOptions.iMinNumOutputBuffers == 0 ) ||
		( aOptions.iHrdVbvSpec == EHrdVbv3GPP ) )
		{
		PRINT_ERR( "SetBufferOptionsL () - incorrect parameter passed ..."
				"leaving with KErrNotSupported" );
        User::Leave ( KErrNotSupported );
        return;
		}

	if ( aOptions.iHrdVbvParams != KNullDesC8 )
		{
		if ( aOptions.iHrdVbvSpec == EHrdVbvNone )
			{
			PRINT_ERR( "SetBufferOptionsL () - incorrect HrdVbvParams or"
					"iHrdVbvSpec passed ...leaving with KErrNotSupported" );
   			User::Leave( KErrNotSupported );
			return;
			}

		TPckgBuf <TAvcPictureHeader> params;
		params.Copy ( aOptions.iHrdVbvParams );
		TUint size = sizeof( TAvcPictureHeader );
		if ( size != aOptions.iHrdVbvParams.Length() )
			{
			PRINT_ERR( "CAriH264encHwDeviceImpl::SetBufferOptionsL() -"
					   "invalid structure passed for iHrdVbvParams" );
			User::Leave( KErrArgument );
			}

		// Perform the checks on the members of
		if( ( params().iBaselineProfileConstraintSet == ( TBool )EFalse ) ||
            ( params().iMainProfileConstraintSet == ( TBool )ETrue ) ||
            ( params().iExtendedProfileConstraintSet == ( TBool )ETrue ) ||
            ( ( params().iPictureType ) & EAvcPictureTypeB ) ||
			( ( params().iPictureType ) & EAvcPictureTypeSI ) ||
			( ( params().iPictureType ) & EAvcPictureTypeSP ) ||
			( params().iLongTermPicture == ( TBool )ETrue ) ||
			( params().iFrame == ( TBool )EFalse ) ||
			( params().iQPY < 10 ) ||
			( params().iQPY > 51 ) )
			{
			PRINT_ERR( "CAriH264encHwDeviceImpl ::SetBufferOptionsL Invalid "
					"data passed in TAVCHrdParams" );
			User::Leave( KErrNotSupported );
			return;
			}
		}

    iH264EncInitParams.iMaxPreEncoderBufferPictures
				= aOptions.iMaxPreEncoderBufferPictures;

    iH264EncInitParams.iMaxOutputBufferSize = aOptions.iMaxOutputBufferSize;
    iH264EncInitParams.iMaxCodedPictureSize = aOptions.iMaxCodedPictureSize;
    iH264EncInitParams.iMaxCodedSegmentSize = aOptions.iMaxCodedSegmentSize;
    iH264EncInitParams.iMinNumOutputBuffers = aOptions.iMinNumOutputBuffers;
    iH264EncInitParams.iBeforeInitialize |= EEncBufferOptions;

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Sets the encoder output rectangle
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetOutputRectL( const TRect& aRect )
	{
	PRINT_ENTRY;

    if( ( aRect.iTl.iX >= aRect.iBr.iX ) ||
        ( aRect.iTl.iY >= aRect.iBr.iY ) )
		{
		PRINT_ERR( " Invalid parameteres passed..Leaving " );
        User::Leave ( KErrNotSupported );
        return;
		}

    iH264EncInitParams.iOutputRect = aRect;
    iH264EncInitParams.iBeforeInitialize |= EEncOutputRectSize;

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Sets whether bit errors or packets losses can be expected in the video
// transmission
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetErrorsExpected( TBool aBitErrors,
												TBool aPacketLosses )
	{
	PRINT_ENTRY;

    // This can bel called before and after initialize
	 PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetErrorsExpected()"
			 " BitErrors is set to [ %d ]", aBitErrors ) );
	 PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::SetErrorsExpected() "
			 "iPacketLosses is set to [ %d ]", aPacketLosses ) );

	if( iEncStateMac->IsInitialized() )
		{
	    iCurSetH264EncParams.iBitErrors       =   aBitErrors;
	    iCurSetH264EncParams.iPacketLosses    =   aPacketLosses;
		iCurSetH264EncParams.iAfterInitialize |= EEncErrorsExpected;
		}
	else
		{
	    iH264EncInitParams.iBitErrors       =   aBitErrors;
	    iH264EncInitParams.iPacketLosses    =   aPacketLosses;
		iH264EncInitParams.iBeforeInitialize |= EEncErrorsExpected;
		}

	PRINT_EXIT;
	}


//----------------------------------------------------------------------------
// Sets the minimum frequency (in time) for instantaneous random access points
// in the bitstream
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetMinRandomAccessRate( TReal aRate )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
		{
		// simply return no further action is taken
		if( aRate <= 0.0 )
			{
			return;
			}
		iCurSetH264EncParams.iRandomAccessRate = aRate;
		PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl"
				"::SetMinRandomAccessRate() iRandomAccessRate is %f" ,
				( TReal )iH264EncInitParams.iRandomAccessRate ) );

		iCurSetH264EncParams.iAfterInitialize |= EEncRandomAccessRate;
		}
	else
		{
		if( aRate <= 0.0 )
			{
			ClientFatalError ( KErrNotSupported );
			return;
			}
		iH264EncInitParams.iRandomAccessRate = aRate;
		PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl"
				"::SetMinRandomAccessRate() iRandomAccessRate is %f" ,
				( TReal )iH264EncInitParams.iRandomAccessRate ) );

		iH264EncInitParams.iBeforeInitialize |= EEncRandomAccessRate;
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Sets coding-standard specific encoder options.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetCodingStandardSpecificOptionsL(
											const TDesC8& aOptions )
	{
    PRINT_ENTRY;

	if ( aOptions == KNullDesC8 )
		{
		PRINT_ERR( "CAriH264encHwDeviceImpl"
				"::SetCodingStandardSpecificOptionsL() Leaving because Null "
				"Descriptor Passed" );
		User::Leave( KErrArgument );
		}

	// validate the parameters
	TPckgBuf<TAvcVideoMode> avcVideoMode;
	TUint size = sizeof( TAvcVideoMode );
	if ( size != aOptions.Length() )
		{
		PRINT_MSG( LEVEL_HIGH, ( "CAriH264encHwDeviceImpl"
		"::SetCodingStandardSpecificOptionsL() - invalid structure passed") );
		User::Leave( KErrArgument );
		}

	// before init
    if( !iEncStateMac->IsInitialized() )
		{
     	iH264EncInitParams.iCodingStandardSpecificOptions = avcVideoMode();
		iH264EncInitParams.iBeforeInitialize
			|= EEncCodingStandardSpecificOptions;
		}
	avcVideoMode.Copy( aOptions );

	if ( ( avcVideoMode().iAllowedPictureTypes & EAvcPictureTypeB ) ||
		 ( avcVideoMode().iAllowedPictureTypes & EAvcPictureTypeSI ) ||
		 ( avcVideoMode().iAllowedPictureTypes & EAvcPictureTypeSP ) ||
		 ( avcVideoMode().iFlexibleMacroblockOrder ) ||
		 ( avcVideoMode().iRedundantPictures ) ||
		 ( avcVideoMode().iDataPartitioning ) ||
		 ( !avcVideoMode().iFrameMBsOnly ) ||
		 ( avcVideoMode().iMBAFFCoding ) ||
		 ( avcVideoMode().iEntropyCodingCABAC ) ||
		 ( avcVideoMode().iWeightedPPrediction ) ||
		 ( avcVideoMode().iWeightedBipredicitonMode ) )
		{
		PRINT_ERR( "CAriH264encHwDeviceImpl"
			"::SetCodingStandardSpecificOptionsL() - leaving invalid data "
			"passed for TAvcVideoMode" );
		User::Leave( KErrNotSupported );
		}

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Sets coding-standard specific encoder options.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetImplementationSpecificEncoderOptionsL(
												const TDesC8& /*aOptions*/)
	{
	PRINT_ENTRY;

    //This API can be called at any point of time
	PRINT_ERR( "SetImplementationSpecificEncoderOptionsL () called before "
			"Initialize ()" );
    User::Leave( KErrNotSupported );

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Returns coding-standard specific initialization output from the encoder
//----------------------------------------------------------------------------
//
HBufC8* CAriH264encHwDeviceImpl::CodingStandardSpecificInitOutputLC()
	{
	PRINT_ENTRY;

    TUint configlength;
    if( !iEncStateMac->IsInitialized() )
    	{
    	PRINT_ERR( "CodingStandardSpecificInitOutputLC () called before "
    			"Initialize ()" );
        User::Leave( KErrNotReady );
    	}
    TUint err = iCodec->GetParam( CONTROL_CMD_GET_CONFIG_LENGTH,
							   &configlength );
    if ( err )
    	{
    	PRINT_ERR("GetParam failure");
    	User::Leave( err );
    	}

    iConfigData = HBufC8::NewL( configlength );
    err = iCodec->GetParam( CONTROL_CMD_GET_CONFIG_HEADER, iConfigData );

    if ( err )
    	{
    	PRINT_ERR("GetParam failure");
        User::Leave( err );
    	}

	PRINT_EXIT;
    return iConfigData;
	}

//----------------------------------------------------------------------------
//  Returns coding-standard specific initialization output from the encoder
//----------------------------------------------------------------------------
//
HBufC8* CAriH264encHwDeviceImpl::ImplementationSpecificInitOutputLC()
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
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

//----------------------------------------------------------------------------
// Sets the number of unequal error protection levels
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetErrorProtectionLevelsL(TUint /*aNumLevels*/,
												TBool /*aSeparateBuffers*/)
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
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

//----------------------------------------------------------------------------
// Sets the number of unequal error protection levels
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetErrorProtectionLevelL( TUint /*aLevel*/,
													   TUint /*aBitrate*/,
													   TUint /*aStrength*/ )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
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

//----------------------------------------------------------------------------
//  Sets the expected or prevailing channel conditions for an unequal
//  error protection level, in terms of expected packet loss rate
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetChannelPacketLossRate( TUint /*aLevel*/,
							TReal /*aLossRate*/,
							TTimeIntervalMicroSeconds32 /*aLossBurstLength*/ )
	{
	PRINT_ENTRY;

    if ( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}

	//packet mode is already enabled
	if ( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
		{
		PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl"
		" ::SetChannelPacketLossRate()- already in packet mode returning" ) );
		return;
		}

	// Flush the output buffers
	iEngine->ReturnOutputBuffers();

	// output buffers status
	iNumOfOutBuffersAvailableInPacketModeChange
						= iOutputFreeBufferQueue.Count();

	iNumofBuffersPendingFromClientInChangeToPacketMode
				= iH264EncInitParams.iMinNumOutputBuffers
				- iOutputFreeBufferQueue.Count();

	iNumOfOutBuffersBeforeChangeToPacketMode
			= iH264EncInitParams.iMinNumOutputBuffers;

	TInt error = KErrNone;
	// some buffers are available
	if ( iNumOfOutBuffersAvailableInPacketModeChange )
			{
			while( iOutputFreeBufferQueue.Count() )
				{
				// add to engine
				iEngine->AddOutput( iOutputFreeBufferQueue[0] );

				// remove this buffer
				iOutputFreeBufferQueue.Remove( 0 );
				}
			}
	else
		{
		// create one buffer which can hold one encoded picture
		TRAP( error, CreateCodedBufferForModeChangeL() );
		if( error != KErrNone )
			{
			ClientFatalError( error );
			return;
			}
		}

	iH264EncInitParams.iMaxCodedSegmentSize
				= KH264ENCIMPL_DEFAULT_SEGMENTSIZE;

	// Get the maximum output buffer size for the encoder
	TUint maxOutputBufferSize =0;
	error = iCodec->GetParam( CONTROL_CMD_GET_MAX_FRAME_SIZE,
										&maxOutputBufferSize );
	if ( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	iH264EncInitParams.iMinNumOutputBuffers = ( maxOutputBufferSize /
							iH264EncInitParams.iMaxCodedSegmentSize ) *
							KH264ENCIMPL_MAXNUM_OUTPUTBUFFERS;

	TRAP( error, CreateSegmentOutputBuffersL (
								iH264EncInitParams.iMinNumOutputBuffers,
								iH264EncInitParams.iMaxCodedSegmentSize ) )
	if( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	error = iCodec->GetParam( CONTROL_CMD_GET_MAX_NUM_PACKETS,
											&iMaxNumOfPackets );
	if( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	TRAP( error, CreatePacketOffsetLengthInfoBuffersL( iMaxNumOfPackets ) );
	if( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	TBool yesorno = ETrue;
	error = iCodec->SetParam( CONTROL_CMD_SET_PACKET_MODE, &yesorno );
	if( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	// change output mode to videosegment
	iH264EncInitParams.iDataUnitType = EDuVideoSegment;

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Sets the expected or prevailing channel conditions for an unequal error
//  protection level, in terms of expected bit error rate
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetChannelBitErrorRate( TUint /*aLevel*/,
								TReal aErrorRate, TReal /*aStdDeviation*/ )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
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
	TRAPD( error , bitErrorRate = new( ELeave )TReal );

	if ( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

    *bitErrorRate = aErrorRate;
    TRAP( error, iEngine->AddCommandL( CBaseEngine::EHighPriority,
    		CONTROL_CMD_SET_CHANNEL_BIT_ERROR_RATE, bitErrorRate ) );
	if( error != KErrNone )
		{
		delete bitErrorRate;
		ClientFatalError( error );
		return;
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Sets the target size of each coded video segment
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetSegmentTargetSize( TUint aLayer,
												TUint aSizeBytes,
												TUint /*aSizeMacroblocks*/ )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}

   if ( iH264EncInitParams.iDataUnitType == EDuCodedPicture )
		{
		if( aSizeBytes== 0 )
			{
			if( iIsSliceEnabledInCodedPicture )
				{
				iIsSliceEnabledInCodedPicture= EFalse;
				TInt error = iCodec->SetParam( CONTROL_CMD_SET_FRAME_MODE,
												NULL );
				if ( error != KErrNone )
					{
					ClientFatalError( error );
					return;
					}
				}
			return;
			}
		TUint calculatedSegmentSize = 0;
		if( aSizeBytes <= KH264ENCIMPL_MIN_SLICESIZE )
			{
			calculatedSegmentSize = KH264ENCIMPL_MIN_SLICESIZE;
			}
		else
			{
			if ( iH264EncInitParams.iMaxCodedSegmentSize
					< iH264EncInitParams.iMaxCodedPictureSize )
	            {
			    if ( aSizeBytes > iH264EncInitParams.iMaxCodedSegmentSize )
				    {
				    calculatedSegmentSize
						= iH264EncInitParams.iMaxCodedSegmentSize;
				    }
			    else
				    {
				    calculatedSegmentSize = aSizeBytes;
				    }
	            }
		    else
			    {
			    if ( aSizeBytes < iH264EncInitParams.iMaxCodedPictureSize )
				    {
				    calculatedSegmentSize = aSizeBytes;
				    }
			    else
				    {
				    return;
				    }
			    }
			}

        if( calculatedSegmentSize < KH264ENCIMPL_MIN_SLICESIZE )
        	{
        	calculatedSegmentSize = KH264ENCIMPL_MIN_SLICESIZE;
        	}

        TBool yesorno = EFalse;
        TInt error = iCodec->SetParam( CONTROL_CMD_SET_PACKET_MODE,
										&yesorno );

		if( error != KErrNone )
			{
			ClientFatalError( error );
			return;
			}
			// Set the target size on codec
		error = iCodec->SetParam( CONTROL_CMD_SET_PACKET_SIZE,
									&calculatedSegmentSize );

		if ( error != KErrNone )
			{
			ClientFatalError( error );
			return;
			}
		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl"
				"::calculatedSegmentSize()  is  %d" ,
				( TInt )calculatedSegmentSize ) );

	    iH264EncInitParams.iMaxCodedSegmentSize = calculatedSegmentSize;
		iIsSliceEnabledInCodedPicture = ETrue;
		return;

		}

	if ( iH264EncInitParams.iDataUnitType != EDuVideoSegment )
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
	if ( aSizeBytes < KH264ENCIMPL_MIN_SEGMENTSIZE )
		{
		calculatedSegmentSize  = KH264ENCIMPL_MIN_SEGMENTSIZE;
		}
    else
        {
        if ( iH264EncInitParams.iMaxOutputBufferSize
						> KH264ENCIMPL_MAX_SEGMENTSIZE )
            {
		    if ( aSizeBytes < KH264ENCIMPL_MAX_SEGMENTSIZE )
			    {
			    calculatedSegmentSize = aSizeBytes;
			    }
		    else
			    {
			    calculatedSegmentSize = KH264ENCIMPL_MAX_SEGMENTSIZE;
			    }
            }
	    else
		    {
		    if ( aSizeBytes < iH264EncInitParams.iMaxOutputBufferSize )
			    {
			    calculatedSegmentSize = aSizeBytes;
			    }
		    else
			    {
			    calculatedSegmentSize
					= iH264EncInitParams.iMaxOutputBufferSize;
			    }
		    }
        }

	// Set the target size on codec
	TInt error = iCodec->SetParam( CONTROL_CMD_SET_PACKET_SIZE,
											&calculatedSegmentSize );

	if ( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}
    iH264EncInitParams.iMaxCodedSegmentSize = calculatedSegmentSize;

    PRINT_EXIT;
	}


//----------------------------------------------------------------------------
// Sets the bit-rate control options for a layer
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetRateControlOptions( TUint aLayer,
								const TRateControlOptions& aOptions )
	{
	PRINT_ENTRY;

	if ( !iEncStateMac->IsInitialized() )
		{
		ClientFatalError( KErrNotReady );
		return;
		}

	// Since layered bit-rate scalability is not used, options are set for the
	// entire stream
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
		if ( ( aOptions.iQualityTemporalTradeoff < 0 ) ||
			( aOptions.iQualityTemporalTradeoff > 1 ) ||
			( aOptions.iLatencyQualityTradeoff < 0 ) ||
			( aOptions.iLatencyQualityTradeoff > 1 ) ||
			( aOptions.iBitrate == 0 ) )
			{
			return;
			}
		}

	TRateControlOptions* rateControlOptions = NULL;
	TRAPD( error, rateControlOptions = new ( ELeave )TRateControlOptions );
	if( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	rateControlOptions->iControl  =  aOptions.iControl;
	// default bit rate used
	if( aOptions.iControl & EBrControlNone )
		{
		rateControlOptions->iBitrate    =  iH264EncInitParams.iBitRate;
		}
	else
		{
		rateControlOptions->iBitrate = ( aOptions.iBitrate
		<= iH264EncInitParams.iBitRate ) ? ( aOptions.iBitrate )
		: ( iH264EncInitParams.iBitRate );
		}

	rateControlOptions->iPictureQuality  =  aOptions.iPictureQuality;
	rateControlOptions->iPictureRate     =  aOptions.iPictureRate;
	rateControlOptions->iQualityTemporalTradeoff
				= aOptions.iQualityTemporalTradeoff;
	rateControlOptions->iLatencyQualityTradeoff
				= aOptions.iLatencyQualityTradeoff;

	TRAP( error, iEngine->AddCommandL( CBaseEngine::EHighPriority,
			CONTROL_CMD_SET_RATE_CONTROL_OPTIONS, rateControlOptions ) );

	if( error != KErrNone )
		{
		delete rateControlOptions;
		ClientFatalError ( error );
		return;
		}

	if( !iEncStateMac->IsPlaying() )
		{
		TInt ret =  iCodec->SetParam( CONTROL_CMD_SET_RATE_CONTROL_OPTIONS,
														rateControlOptions );
		if ( ret )
			{
			ClientFatalError( ret );
			}
		}

	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetRateControlOptions()"
			"iBitrate is %d", ( TInt )aOptions.iBitrate ) );

	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetRateControlOptions()"
			"iPictureQuality is %d" , ( TInt )aOptions.iPictureQuality ) );

	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetRateControlOptions()"
			" iPictureRate is %f", ( TReal )aOptions.iPictureRate ) );

	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetRateControlOptions()"
			"iQualityTemporalTradeoff is %f" ,
			( TReal )aOptions.iQualityTemporalTradeoff ) );

	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetRateControlOptions()"
			" iLatencyQualityTradeoff is %f " ,
			( TReal ) aOptions.iLatencyQualityTradeoff ) );

	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetRateControlOptions()"
			"iControl is %x" , ( TInt ) aOptions.iControl ) );

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Sets in-layer scalability options for a layer
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetInLayerScalabilityL( TUint /*aLayer*/,
								TUint /*aNumSteps*/,
								TInLayerScalabilityType /*aScalabilityType*/,
								const TArray<TUint>& /*aBitrateShare*/,
								const TArray<TUint>& /*aPictureShare*/ )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
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

//----------------------------------------------------------------------------
// Sets the period for layer promotions points for a scalability layer
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetLayerPromotionPointPeriod( TUint /*aLayer*/,
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

//----------------------------------------------------------------------------
// Returns coding-standard specific settings output from the encoder
//----------------------------------------------------------------------------
//
HBufC8* CAriH264encHwDeviceImpl::CodingStandardSpecificSettingsOutputLC()
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
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

//----------------------------------------------------------------------------
// Returns implementation-specific settings output from the encoder
//----------------------------------------------------------------------------
//
HBufC8* CAriH264encHwDeviceImpl::ImplementationSpecificSettingsOutputLC()
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
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

//----------------------------------------------------------------------------
// Requests the encoder to sends supplemental information in the bitstream
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SendSupplementalInfoL( const TDesC8& /*aData*/ )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
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

//----------------------------------------------------------------------------
// Requests the encoder to sends supplemental information in the bitstream
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SendSupplementalInfoL( const TDesC8& /*aData*/,
							const TTimeIntervalMicroSeconds& /*aTimestamp*/ )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
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

//----------------------------------------------------------------------------
// Cancels the current supplemental information send request
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::CancelSupplementalInfo()
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}
    ClientFatalError( KErrNotSupported );

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Gets the current output buffer status. The information includes
// the number of free output buffers and the total size of free buffers in
// bytes.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::GetOutputBufferStatus( TUint& aNumFreeBuffers,
													TUint& aTotalFreeBytes )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}

    aNumFreeBuffers = iEngine->NumOutputBuffers()
						+ iOutputFreeBufferQueue.Count();

    aTotalFreeBytes = aNumFreeBuffers * iOutputBufferSize;

	PRINT_EXIT;
	}
//----------------------------------------------------------------------------
// Returns a used output buffer back to the encoder
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::ReturnBuffer( TVideoOutputBuffer* aBuffer )
	{
	PRINT_ENTRY;

    if ( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}

	if ( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
		{
		// coded picture buffers so add it to Interanal Q or process engine
		if ( iNumofBuffersPendingFromClientInChangeToPacketMode )
			{
			iEngine->AddOutput( aBuffer );
			--iNumofBuffersPendingFromClientInChangeToPacketMode;
			return;
			}

		// buffers returned by the client are
		if ( iNumOfBuffersAtClientInPacketModeNeedReAllocation )
			{
			TRAPD( error, ReallocateSegmentBufferL( aBuffer ) );
			if( error != KErrNone )
				{
				ClientFatalError( error );
				return;
				}
			iNumOfBuffersAtClientInPacketModeNeedReAllocation--;
			}

		TInt error = KErrNone;
		error = iOutputFreeBufferQueue.Append( aBuffer );

		if( error != KErrNone )
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

			#ifdef CALCINSTANTBITRATE
			CalculateInstantBitRate( *outBuf );
			#endif

			iMMFDevVideoRecordProxy->MdvrpNewBuffer( outBuf );
			}
		}
	else
		{
		aBuffer->iData.Set( ( TUint8* )aBuffer->iData.Ptr(),
										iOutputBufferSize );

		// add the buffer back to queue or process engine
		if ( ( !iEncStateMac->IsInputEndPending() )
				&& ( !iEncStateMac->IsStopped() ) )
			{
			iEngine->AddOutput( ( TAny* )aBuffer );
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

//----------------------------------------------------------------------------
// Indicates a picture loss to the encoder, without specifying the lost
// picture
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::PictureLoss()
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}
	iPictureLoss = ETrue;

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Indicates to the encoder the pictures that have been lost
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::PictureLoss(
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

//----------------------------------------------------------------------------
// Indicates a slice loss to the encoder.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SliceLoss( TUint aFirstMacroblock,
											TUint aNumMacroblocks,
											const TPictureId& /*aPicture*/ )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}

    if ( ( aFirstMacroblock == 0 ) || ( aNumMacroblocks == 0 ) )
    	{
    	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SliceLoss() "
    			"aNumMacroblocks or aFirstMacroblock = 0 return" ) );
        return;
    	}

	TH264EncSliceLoss* sliceLossParams = NULL;
	TRAPD( error, sliceLossParams  = new( ELeave ) TH264EncSliceLoss );
	if( error != KErrNone )
		{
		ClientFatalError( error );
		return;
		}

	sliceLossParams->iFirstMacroblock = aFirstMacroblock;
	sliceLossParams->iNumMacroblocks  = aNumMacroblocks;

	TRAP( error, iEngine->AddCommandL( CBaseEngine::ENormalPriority,
					CONTROL_CMD_SET_SLICELOSS, sliceLossParams ) );

	if( error != KErrNone )
		{
		delete sliceLossParams;
		ClientFatalError ( error );
		return;
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Sends a reference picture selection request to the encoder
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::ReferencePictureSelection(
											const TDesC8& /*aSelectionData*/)
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}
    ClientFatalError( KErrNotSupported );

    PRINT_EXIT;
	}


//----------------------------------------------------------------------------
// Initializes the device, and reserves hardware resources
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::Initialize()
	{
	PRINT_ENTRY;

    //Device should be in unintialized state
    if ( !iEncStateMac->IsTransitionValid(
    		CStateMachine::EInitializingCommand ) )
    	{
		iMMFDevVideoRecordProxy->MdvrpInitializeComplete(this
											, KErrPermissionDenied );
        return;
    	}

	// check for input and output formats set by the client
	if ( ! ( iH264EncInitParams.iBeforeInitialize  & EEncInputFormat ) )
		{
		PRINT_MSG( LEVEL_CRITICAL, ("CAriH264encHwDeviceImpl::Initialize() "
				"- SetInputFormat not called" ) );
		iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this,
												KErrNotSupported );
        return;
		}

	// check for buffer sizes
	if( iH264EncInitParams.iBeforeInitialize & EEncBufferOptions )
		{
		if ( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
			{
			// Buffer size related checks
			if ( ( iH264EncInitParams.iMaxOutputBufferSize >
				( KH264ENCIMPL_FACTOR_FOR_OUTPUTBUFFERSIZE_IN_PACKETMODE *
				KH264ENCIMPL_MAX_SEGMENTSIZE ) ) ||
				( iH264EncInitParams.iMaxCodedSegmentSize >
				KH264ENCIMPL_MAX_SEGMENTSIZE ) ||
				( iH264EncInitParams.iMaxCodedSegmentSize >
				iH264EncInitParams.iMaxOutputBufferSize ) ||
				( iH264EncInitParams.iMaxCodedSegmentSize == 0 ) )
				{
				PRINT_MSG( LEVEL_CRITICAL, ( "CAriH264encHwDeviceImpl"
				"::Initialize() - Wrong size for iMaxCodedSegmentSize" ) );
				iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this,
						KErrNotSupported );
				return;
				}
			}

		else if ( iH264EncInitParams.iDataUnitType == EDuCodedPicture )
			{
			if ( ( iH264EncInitParams.iMaxCodedPictureSize >
				iH264EncInitParams.iMaxOutputBufferSize )
				|| ( iH264EncInitParams.iMaxCodedPictureSize == 0 ) )
				{
				PRINT_MSG( LEVEL_CRITICAL, ( "CAriH264encHwDeviceImpl"
				"::Initialize() - Wrong size for iMaxCodedPictureSize" ) );
				iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this,
														KErrNotSupported );
				return;
				}
			}
		}
	else
		{
		if ( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
			{
			iH264EncInitParams.iMinNumOutputBuffers
				= KH264ENCIMPL_MAXNUM_SEGMENTBUFFERS;
			iH264EncInitParams.iMaxCodedSegmentSize
				= KH264ENCIMPL_DEFAULT_SEGMENTSIZE;
			iH264EncInitParams.iMaxOutputBufferSize
				= KH264ENCIMPL_MAX_SEGMENTSIZE;
			}
		else
			{
			iH264EncInitParams.iMinNumOutputBuffers
				= KH264ENCIMPL_MAXNUM_OUTPUTBUFFERS;
			}
		}

	// based on level set the max bit rate
	switch ( iH264EncInitParams.iLevel )
		{
		//Level 1b
		case KH264ENCIMPL_LEVEL_1b:
			iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_1b;
            iH264EncInitParams.iTargetPictureRate = KPictureRate15;
			break;

		case KH264ENCIMPL_LEVEL_1:
			iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_1;
            iH264EncInitParams.iTargetPictureRate = KPictureRate15;
			break;

		case KH264ENCIMPL_LEVEL_11:
            // Both Level 1b and 1.1 have this value
            // need to decide if 1.1 or 1.b
            // two options - client passes buffer options and client doesnt
			// pass buffer options
            {
            TBool isLevel1b = EFalse;
            if ( iH264EncInitParams.iBeforeInitialize & EEncBufferOptions )
                {
                isLevel1b = ( ( KH264ENCIMPL_LEVEL1b_MAXCPBSIZE <=
								iH264EncInitParams.iMaxCodedPictureSize ) &&
								( iH264EncInitParams.iMaxCodedPictureSize <
								KH264ENCIMPL_LEVEL11_MAXCPBSIZE ) )
                             ? ( ETrue )
                             : ( EFalse );
                }
            //now we have input size information to decide 1b vs 1.1
            else
                {
                isLevel1b = ( ( iH264EncInitParams.iPictureSize.iWidth >
								KH264ENCIMPL_QCIF_WIDTH )
                                && ( iH264EncInitParams.iPictureSize.iHeight >
                                KH264ENCIMPL_QCIF_HEIGHT ) )
                             ? ( EFalse )
                             : ( ETrue );
                }

            if ( isLevel1b )
                {
                iH264EncInitParams.iLevel = KH264ENCIMPL_LEVEL_1b;
                iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_1b;
                iH264EncInitParams.iTargetPictureRate = KPictureRate15;
                }
            else
                {
                iH264EncInitParams.iLevel = KH264ENCIMPL_LEVEL_11;
                iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_11;
                iH264EncInitParams.iTargetPictureRate
					= ( ( iH264EncInitParams.iPictureSize.iWidth
					<= KH264ENCIMPL_QCIF_WIDTH ) &&
					( iH264EncInitParams.iPictureSize.iHeight
					<= KH264ENCIMPL_QCIF_HEIGHT ) ) ? ( KPictureRate15 )
                     : ( KPictureRate75 );
                }
            }
			break;

		case KH264ENCIMPL_LEVEL_12:
			iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_12;
            iH264EncInitParams.iTargetPictureRate = KPictureRate15;
			break;

		case KH264ENCIMPL_LEVEL_13:
			iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_13;
            iH264EncInitParams.iTargetPictureRate = KPictureRate30;
			break;

		case KH264ENCIMPL_LEVEL_2:
			iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_2;
            iH264EncInitParams.iTargetPictureRate = KPictureRate30;
			break;

		case KH264ENCIMPL_LEVEL_21:
			iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_21;
            if ( iH264EncInitParams.iPictureSize.iHeight >
						KH264ENCIMPL_HALFSDTV_HEIGHT_NTSC )
            	{
            	// HALFSDTV-PAL
                iH264EncInitParams.iTargetPictureRate = KPictureRate25;
            	}
            else
            	{
            	// HALFSDTV-NTSC
                iH264EncInitParams.iTargetPictureRate = KPictureRate30;
            	}
			break;

		case KH264ENCIMPL_LEVEL_22:
			iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_22;
            if ( iH264EncInitParams.iPictureSize.iHeight >
								KH264ENCIMPL_SDTV_HEIGHT_NTSC )
            	{
            	// SDTV-PAL
                iH264EncInitParams.iTargetPictureRate
						= KH264ENCIMPL_PICTURERATE_12_5;
            	}
            else
            	{
            	// SDTV-NTSC
                iH264EncInitParams.iTargetPictureRate
						= KPictureRate15;
            	}
			break;

		case KH264ENCIMPL_LEVEL_3:
			iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_3;
            if ( iH264EncInitParams.iPictureSize.iHeight
            		> KH264ENCIMPL_SDTV_HEIGHT_NTSC )
            	{
                iH264EncInitParams.iTargetPictureRate = KPictureRate25;
            	}
            else
            	{
            	// SDTV-NTSC
                iH264EncInitParams.iTargetPictureRate = KPictureRate30;
            	}
			break;

			// level unknown based on size decide bitrate and level
		case 0:
			SetLevelAndBitRate();
			break;

		default:
			break;
		};

    // create codec
    TRAPD( error, iCodec  = CAriH264encWrapper::NewL( iH264EncInitParams ) );
    if( error != KErrNone )
    	{
        // init complete with error message
        iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
		return;
    	}

    // set the sync options to the codec
    if( iClockSource )
    	{
    	TInt error = iCodec->SetSyncOptions( iClockSource );

		if( error != KErrNone )
			{
			iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
			return;
			}
    	}
    //Check whether picture is of QCIF format if VT fast call setup is set

    if ( iH264EncInitParams.iVTFastCallSetUp )
    	{
    	if ( iH264EncInitParams.iPictureSize.iWidth
    			!= KH264ENCIMPL_QCIF_WIDTH ||
    		 iH264EncInitParams.iPictureSize.iHeight
				!= KH264ENCIMPL_QCIF_HEIGHT )
    		{
    		PRINT_MSG( LEVEL_LOW, ("Input resolution should be 176x144 for"
    				" VT Fast call setup" ) );
    		ClientFatalError( KErrNotSupported );
    		}
    	}

    if ( error != KErrNone )
    	{
        iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
		return;
    	}

    //create engine
    TRAP( error, iEngine = CBaseEngine::NewL( this,
							( MBaseCodec* ) iCodec, EFalse ) );

    if( error != KErrNone )
    	{
        //init complete with error message
        iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
        return;
    	}
    //get the max buffer length from the codec
    TInt maxOutputBufferSize = 0;
    error = iCodec->GetParam( CONTROL_CMD_GET_MAX_FRAME_SIZE,
											&maxOutputBufferSize );

    if( error != KErrNone )
    	{
        //init complete with error message
        iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
        return;
    	}

    // if EduCodedPicture mode then check for buffer size passed
    if( iH264EncInitParams.iDataUnitType == EDuCodedPicture )
		{
		if ( iH264EncInitParams.iBeforeInitialize & EEncBufferOptions )
			{
			if( maxOutputBufferSize
					> iH264EncInitParams.iMaxCodedPictureSize )
				{
				PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::Initialize()"
				"maxOutputBufferSize = %d", ( TInt ) maxOutputBufferSize ) );
				//init complete with error message
				iMMFDevVideoRecordProxy->MdvrpInitializeComplete(this,
														KErrNotSupported );
				return;
				}
			}
		else
			{
		    iH264EncInitParams.iMaxOutputBufferSize = maxOutputBufferSize;
			iH264EncInitParams.iMaxCodedPictureSize = maxOutputBufferSize;
			}
		}

	// if packet mode is on then allocate memory for NAL information
	TUint maxNumOfPackets = 0;
	if( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
		{
		error = iCodec->GetParam( CONTROL_CMD_GET_MAX_NUM_PACKETS,
													&iMaxNumOfPackets );
		if( error != KErrNone )
			{
			iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
			return;
			}
		}

	if ( iH264EncInitParams.iDataUnitType != EDuVideoSegment )
		{
		TInt maxSlices = 0;
		if( maxOutputBufferSize )
			{
		    maxSlices = maxOutputBufferSize / KH264ENCIMPL_MIN_SLICESIZE;
	        if( maxOutputBufferSize % KH264ENCIMPL_MIN_SLICESIZE )
	        	{
    	        maxSlices++;
	        	}
			}

#ifndef EMZ_AVC_ENC_CODEC_SEIINFO_ENABLE
		PRINT_MSG( LEVEL_LOW, ( "CreateCodedOutputBuffersL SEI disble " ) );

		// included KNumOfConfigParams*KDoubleWordLength bytes which are
		// required for filling offset length information

		TRAP( error, CreateCodedOutputBuffersL( ( maxOutputBufferSize +
			KWordLength + ( ( KNumOfConfigParams + maxSlices )
			* KDoubleWordLength ) ) ) );
#else
		PRINT_MSG( LEVEL_LOW, ("CreateCodedOutputBuffersL SEI enable " ) );
		TRAP( error, CreateCodedOutputBuffersL( ( maxOutputBufferSize +
			KWordLength + ( ( KNumOfNAL_SEI + maxSlices ) *
			KDoubleWordLength ) ) ) );
#endif
		}
	else
		{
		// Allocate sufficient segment mode buffers  Comment it for some time.
		iH264EncInitParams.iMinNumOutputBuffers = ( maxOutputBufferSize /
				iH264EncInitParams.iMaxCodedSegmentSize ) *
				KH264ENCIMPL_MAXNUM_OUTPUTBUFFERS;

		//  holds output buffer 2. holds packet lenght and offset information
		TRAP( error, CreateInternalOutputBuffersL( maxOutputBufferSize ) );
		if( error != KErrNone )
			{
			iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
			return;
			}

		TRAP( error, CreatePacketOffsetLengthInfoBuffersL( iMaxNumOfPackets *
				KDoubleWordLength ) );
		if( error != KErrNone )
			{
			iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, error );
			return;
			}

		//segement size  will be the max segment size
		TUint segmentSize  = iH264EncInitParams.iMaxOutputBufferSize;
		if ( ( iH264EncInitParams.iDataEncapsulation == EDuGenericPayload ) &&
			( iH264EncInitParams.iMaxOutputBufferSize <
			( KH264ENCIMPL_MAX_SEGMENTSIZE + KDoubleWordLength
			+ KWordLength ) ) )
			{
			segmentSize += KDoubleWordLength + KWordLength;
			}

		TRAP( error, CreateCodedOutputBuffersL( segmentSize ) );
		if( error != KErrNone )
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
	iPictureCounters.iPicturesProcessed	= 0;

	// reset the options set by client
	iH264EncInitParams.iBeforeInitialize = 0;
	iCurSetH264EncParams = iH264EncInitParams;
    iEncStateMac->Transit( CStateMachine::EInitializingCommand );
    iEncStateMac->Transit( CStateMachine::EInitializeCommand );
    iMMFDevVideoRecordProxy->MdvrpInitializeComplete( this, KErrNone );

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Commit all changes since the last CommitL(), Revert() or Initialize()
// to the Hw device
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::CommitL()
	{
	PRINT_ENTRY;

    // This method can only be called after Initialize
    if( !iEncStateMac->IsInitialized() )
    	{
    	PRINT_ERR( "CommitL () called before "
    	    	    			"Initialize ()" );
        User::Leave( KErrNotReady );
    	}

    // Methods that will be affected by CommitL in our case:
    // SetOutputRectL
    // SetCodingStandardSpecificOptionsL
    // SetErrorsExpected
    // SetMinRandomAccessRate

	TH264EncInitParams *currentParams = new( ELeave ) TH264EncInitParams;
	CleanupStack::PushL( currentParams );
	*currentParams	= iCurSetH264EncParams;

	// Add command apply commit settings to the codec
	iEngine->AddCommandL ( CBaseEngine::ENormalPriority,
			CONTROL_CMD_SET_COMMIT_OPTIONS, currentParams );

	iCurSetH264EncParams.iAfterInitialize = 0;
	CleanupStack::Pop();

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Revert all changes since the last CommitL(), Revert() or Initialize()
// back to their previous settings
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::Revert()
	{
	PRINT_ENTRY;

    // This method can only be called after Initialize
    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}

    // Methods that will be affected by Revert in our case:
    // SetOutputRectL
    // SetCodingStandardSpecificOptionsL
    // SetErrorsExpected
    // SetMinRandomAccessRate

	iCurSetH264EncParams = iH264EncInitParams;
	iCurSetH264EncParams.iAfterInitialize = 0;

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// This method is called only in case of client memory buffers
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::WritePictureL( TVideoPicture* aPicture )
	{
	PRINT_ENTRY;

    // This method should only be called only in following states
    if( !iEncStateMac->IsInitialized() )
    	{
    	PRINT_ERR( "WritePictureL () called before Initialize ()" );
        User::Leave( KErrNotReady );
    	}

    if( iEncStateMac->IsInputEndPending() )
    	{
    	PRINT_ERR( "Input end is pending.. leaving with KErrEof" );
        User::Leave( KErrEof );
    	}

    if( !aPicture ||
		( aPicture->iData.iRawData->Length() == 0 ) ||
		( aPicture->iData.iRawData->Length() !=
		( ( iH264EncInitParams.iPictureSize.iWidth *
		iH264EncInitParams.iPictureSize.iHeight * 3 ) / 2 ) ) ||
		( aPicture->iData.iDataFormat != EYuvRawData ) )
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
    if( IsForcedIFrameRequired( aPicture ) )
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
	if( error != KErrNone )
		{
		PRINT_ERR( "CAriH264encHwDeviceImpl::WritePictureL()"
				" AddInput() failed" );
		User::Leave( error );
		return;
		}

    if( this->iH264EncInitParams.iProcessRealtime )
	    {
	    // Check the iDataUnitType
	    if( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
    		{
    		TInt perPictureSegmentBuffers
					= iH264EncInitParams.iMinNumOutputBuffers
									/ KH264ENCIMPL_MAXNUM_OUTPUTBUFFERS;
    		TInt queueCount = iOutputFreeBufferQueue.Count();
    		//  Check for availability of segment buffers
    		if ( queueCount < perPictureSegmentBuffers )
    			{
    			// Sufficient number of segment buffers are not present
    			PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl"
    			"::WritePictureL() Input returned EDuVideoSegment " ) );
    			// Return the earlier input picture added to the queue
    			iEngine->ReturnInput();
				return;
    			}
    		}
    	else
    		{
    		//  Check for availability of buffers.
    		if ( iEngine->NumOutputBuffers() == 0 )
    			{
    			// Return the earlier input picture added to the queue
    			PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl"
    			"::WritePictureL() Input returned EDuCodedPicture " ) );
    			iEngine->ReturnInput();
				return;
    			}
    		}
	    }

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Notifies the hardware device that the end of input data has been reached
// and no more input pictures will be written
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::InputEnd()
	{
	PRINT_ENTRY;

    if ( !iEncStateMac->IsTransitionValid( CStateMachine::EInputEndCommand ) )
    	{
        ClientFatalError( KErrPermissionDenied );
        return;
    	}

    if ( iEngine->NumInputBuffers() == 0 )
    	{
    	PRINT_MSG( LEVEL_HIGH, ("CAriH264encHwDeviceImpl::InputEnds()" ) );
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

//----------------------------------------------------------------------------
// Starts recording video
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::Start()
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsPlaying() && !iEncStateMac->IsPaused() )
    	{
    	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::Start()-already in"
    			"started state, So ignore" ) );
        return;
    	}

    if( !iEncStateMac->IsTransitionValid( CStateMachine::EStartCommand ) )
    	{
        ClientFatalError( KErrPermissionDenied );
        return;
    	}

	if ( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
		{
		// Enable flag which indicates that all picture params and sequence
		//  params are put into 1st buffer
		iIsConfigDataFilledInFirstOutputBuffer = ETrue;
		for( TInt i = 0; i <= iInternalOutputBufferQueue.Count(); i++ )
		 	{
			TVideoOutputBuffer *outputBuffer = iInternalOutputBufferQueue[0];
			iEngine->AddOutput( ( TAny* )outputBuffer );
			iInternalOutputBufferQueue.Remove( 0 );
		 	}
		}
	else
		{
		// add all output buffers to engine
		for( TInt i = 0; i <= iOutputFreeBufferQueue.Count(); i++ )
			{
			TVideoOutputBuffer *outputBuffer = iOutputFreeBufferQueue[0];
			iEngine->AddOutput( ( TAny* )outputBuffer );
			iOutputFreeBufferQueue.Remove( 0 );
			}
		}

    if ( iClockSource )
    	{
        TTime lSystemTime;
        lSystemTime.UniversalTime();
        UpdateTime();
        iPeriodicTimer->Start( iPollingInterval, iPollingInterval,
	    TCallBack( CAriH264encHwDeviceImpl::TimerCallBack, ( TAny* )this ) );
    	}

    iEngine->Start();
    PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::Start() Change to "
    		" start state** ") ) ;
    iEncStateMac->Transit( CStateMachine::EStartCommand );

	PRINT_EXIT;

	}

//----------------------------------------------------------------------------
// Callback function to CPeriodicTimer object
//----------------------------------------------------------------------------
//
TInt CAriH264encHwDeviceImpl::TimerCallBack( TAny* aPtr )
	{
	PRINT_ENTRY;
	if ( !aPtr )
		{
		( ( CAriH264encHwDeviceImpl* )aPtr )->ClientFatalError(
															KErrArgument );
        return KErrNone;
		}
	PRINT_EXIT;
    return ( ( CAriH264encHwDeviceImpl* )aPtr )->UpdateTime();
	}

//----------------------------------------------------------------------------
// Stops recording video.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::Stop()
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsStopped() )
    	{
    	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::Stop() -> already "
    			"in stop state, so ignore" ) );
        return;
    	}

    if( !iEncStateMac->IsTransitionValid( CStateMachine::EStopCommand ) )
    	{
    	PRINT_MSG( LEVEL_CRITICAL, ("CAriH264encHwDeviceImpl::Stop() -> "
    			"fatalerror because Stop called in invalid state" ) );
        ClientFatalError( KErrPermissionDenied );
        return;
    	}

	iInputEndCalled	= EFalse;
	iIsSliceEnabledInCodedPicture = EFalse;

	if ( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
		{
		// new clip started so config data has to be filled in first output
		// buffer
		iIsConfigDataFilledInFirstOutputBuffer = ETrue;
		iTotalOutputBufferLengthInPacketMode = 0;
		iTotalLengthFilledSoFarInPacketMode = 0;
		iPacketsPending	= EFalse;
		}

    iEngine->Stop();
    iEngine->Reset();

    iFrozen = EFalse;
    if( iPeriodicTimer )
    	{
        iPeriodicTimer->Cancel();
    	}
    iEncStateMac->Transit( CStateMachine::EStopCommand );

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Pauses video recording
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::Pause()
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsPaused() )
    	{
    	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::Pause()-> already"
    			"in Paused state, so ignore" ) );
        return;
    	}

    if( !iEncStateMac->IsTransitionValid( CStateMachine::EPauseCommand ) )
    	{
    	PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::Pause() Pause "
    			"called in invalid state" ) );
        ClientFatalError( KErrPermissionDenied );
        return;
    	}

    // No arguments to be passed in Pause
    if( iClockSource )
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

//----------------------------------------------------------------------------
// Resumes video recording
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::Resume()
	{
	PRINT_ENTRY;

	// doing it before transitionvalid check because initialize->resume is
	// not added
    if( iEncStateMac->IsInInitializedState() )
    	{
    	PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::Resume()->Got resume"
    			" in initialized state. So go to start()" ) );
        Start();
        return;
    	}

    if( iEncStateMac->IsPlaying() && !iEncStateMac->IsPaused() )
    	{
    	PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::Resume()->already in"
    			" playing-not-paused state, so ignore" ) );
        return;
    	}

    if( !iEncStateMac->IsTransitionValid( CStateMachine::EResumeCommand ) )
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

        // Send Update time before sending Resume command, so that clock
        // source is set before

        UpdateTime ();
        iPeriodicTimer->Start( iPollingInterval, iPollingInterval,
		TCallBack( CAriH264encHwDeviceImpl::TimerCallBack, ( TAny* )this ) );
    	}

	// Start the engine
	iEngine->Start();

	if ( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
		{

		// send the remaining pending packets to client
		while ( iOutputFreeBufferQueue.Count() && iPacketsPending )
			{
			TVideoOutputBuffer *outBuf = iOutputFreeBufferQueue[0];
			FillVideoSegment( outBuf, iInternalOutputBufferQueue[0] );
			iOutputFreeBufferQueue.Remove( 0 );

			#ifdef CALCINSTANTBITRATE
			CalculateInstantBitRate( *outBuf );
			#endif

			iMMFDevVideoRecordProxy->MdvrpNewBuffer( outBuf );
			}
		}
    //Change the state of the encoder
    iEncStateMac->Transit( CStateMachine::EResumeCommand );

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Freezes the input picture
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::Freeze()
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
    	PRINT_MSG( LEVEL_CRITICAL, ("CAriH264encHwDeviceImpl::Freeze() Freeze"
    			" called in invalid state" ) );
        ClientFatalError( KErrNotReady );
        return;
    	}
    iFrozen = ETrue;

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Releases a frozen input picture
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::ReleaseFreeze()
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
    	PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::ReleaseFreeze() "
    			"called in invalid state" ) );
        ClientFatalError( KErrNotReady );
        return;
    	}
    iFrozen = EFalse;

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Returns the current recording position
//----------------------------------------------------------------------------
//
TTimeIntervalMicroSeconds CAriH264encHwDeviceImpl::RecordingPosition()
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
    	PRINT_MSG( LEVEL_HIGH, ("CAriH264encHwDeviceImpl::RecordingPosition()"
    			" called in invalid state" ) );
        ClientFatalError( KErrNotReady );
        return TTimeIntervalMicroSeconds( 0 );
    	}

    PRINT_EXIT;
    return TTimeIntervalMicroSeconds( iLastEncodedPictureTimestamp );
	}

//----------------------------------------------------------------------------
// Reads various counters related to processed video pictures
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::GetPictureCounters(
		CMMFDevVideoRecord::TPictureCounters& aCounters )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}
	TUint numofpacktsskipped;
	TUint err = iCodec->GetParam( CONTROL_CMD_GET_PICTURES_SKIPPED,
												&numofpacktsskipped );
	if ( err )
		{
		ClientFatalError( err );
		return;
		}

	iPictureCounters.iPicturesSkippedRateControl
		= iPictureCounters.iPicturesSkippedRateControl + numofpacktsskipped;

	iPictureCounters.iPicturesProcessed
		= iPictureCounters.iInputPictures
		- iPictureCounters.iPicturesSkippedRateControl;

	aCounters = iPictureCounters;

	//reset the counters
	iPictureCounters.iInputPictures = 0;
	iPictureCounters.iPicturesSkippedRateControl = 0;
	iPictureCounters.iPicturesProcessed	= 0;

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Reads the frame stabilisation output picture position.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::GetFrameStabilisationOutput( TRect&/* aRect*/ )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
    ClientFatalError( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Retrieves the number of complexity control levels available for this
// hardware device
//----------------------------------------------------------------------------
//
TUint CAriH264encHwDeviceImpl::NumComplexityLevels()
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return 0;
    	}

	PRINT_EXIT;
    return( KH264ENCIMPL_NUM_COMPLEXITYLEVELS );
	}

//----------------------------------------------------------------------------
// Sets the complexity level to use for video processing in a hardware device
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetComplexityLevel( TUint aLevel )
	{
	PRINT_ENTRY;

    if( !iEncStateMac->IsInitialized() )
    	{
        ClientFatalError( KErrNotReady );
        return;
    	}

    if( aLevel >= KH264ENCIMPL_NUM_COMPLEXITYLEVELS )
    	{
    	PRINT_MSG( LEVEL_CRITICAL, ("CAriH264encHwDeviceImpl"
    			"::SetComplexityLevel() Unsupported level Passed" ) );
        ClientFatalError( KErrArgument );
    	}

    iH264EncInitParams.iComplexityLevel = aLevel;
    TH264EncInitParams *currentParams = NULL;
	TRAPD( error, currentParams = new( ELeave ) TH264EncInitParams );
	*currentParams	= iH264EncInitParams;

	// Add command apply commit settings to the codec
	TRAP( error, iEngine->AddCommandL( CBaseEngine::ENormalPriority,
			CONTROL_CMD_SET_COMPLEXITY_LEVEL, currentParams ) );

	if( error != KErrNone )
		{
		delete currentParams;
		ClientFatalError( error );
		return;
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Retrieves information about the pre-processing capabilities of this
// hardware device.
//----------------------------------------------------------------------------
//
CPreProcessorInfo* CAriH264encHwDeviceImpl::PreProcessorInfoLC()
	{
	PRINT_ENTRY;

	TInt cleanupstackpushcount = 0;
    if( iEncStateMac->IsInitialized() )
    	{
    	PRINT_ERR( "SetErrorProtectionLevelsL () called after Initialize()"
    			"..leaving" );
        User::Leave( KErrPermissionDenied );
    	}

    _LIT( KManufacturer, "" );
    _LIT( KIdentifier, "" );

    TPtrC8 implementationSpecificInfo( NULL,0 );
    RArray<TUncompressedVideoFormat> inputFormats;
	CleanupClosePushL( inputFormats );
	cleanupstackpushcount++;
    inputFormats.Reset();

    RArray<TUncompressedVideoFormat>  outputFormats;
	CleanupClosePushL( outputFormats );
	cleanupstackpushcount++;
    outputFormats.Reset();

    RArray<TUint32>  supportedCombinations;
	CleanupClosePushL( supportedCombinations );
	cleanupstackpushcount++;
    supportedCombinations.Reset();

    RArray<TScaleFactor> supportedScaleFactors;
	CleanupClosePushL( supportedScaleFactors );
	cleanupstackpushcount++;
    supportedScaleFactors.Reset();

    TYuvToYuvCapabilities yuvToYuvCapabilities;
    TUid uid;

    CPreProcessorInfo *preProcessorInfo = CPreProcessorInfo::NewL( uid.Null(),
                                            KManufacturer,
                                            KIdentifier,
                                            TVersion( 0,0,0 ),
                                            EFalse,
                                            EFalse,
                                            inputFormats.Array(),
                                            outputFormats.Array(),
                                            supportedCombinations.Array(),
                                            EFalse,
                                            EFalse,
                                            supportedScaleFactors.Array(),
                                            yuvToYuvCapabilities,
                                            0,
                                            0,
                                            implementationSpecificInfo );

    CleanupStack::PushL( preProcessorInfo );
    CleanupStack::Pop( cleanupstackpushcount );

	PRINT_EXIT;
    return preProcessorInfo ;
	}

//----------------------------------------------------------------------------
// Sets the hardware device input format
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetInputFormatL(
		const TUncompressedVideoFormat& aFormat, const TSize& aPictureSize )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
    	{
    	PRINT_ERR( "SetInputFormatL () called after Initialize()..leaving" );
        User::Leave( KErrPermissionDenied );
    	}

    if( !CheckInputFormat( aFormat ) )
    	{
    	PRINT_ERR("CAriH264encHwDeviceImpl"
    	"::SetInputFormatL() Leaving because of not support input format" );
        User::Leave( KErrNotSupported );
    	}

    // Check if passed size is supported by encoder
	// Check if encoder supports the passed input format
	// supports all sizes which are multiple of 16 pixels

	if ( ( !aPictureSize.iWidth ) || ( !aPictureSize.iHeight ) ||
		( aPictureSize.iWidth % 16 ) || ( aPictureSize.iHeight % 16 ) ||
		( aPictureSize.iWidth > KH264ENCIMPL_SDTV_WIDTH ) ||
		( aPictureSize.iHeight > KH264ENCIMPL_SDTV_HEIGHT_PAL ) )
		{
		PRINT_ERR( "Incorrect parameter passed.. leaving" );
		User::Leave( KErrNotSupported );
		}

	TInt32 aspectratio = ::MapAspectRatio(
			aFormat.iYuvFormat.iAspectRatioNum,
			aFormat.iYuvFormat.iAspectRatioDenom );

	if ( aspectratio == -1 )
		{
		PRINT_ERR( "Aspect ratio is -1.. leaving" );
		User::Leave( KErrNotSupported );
		}

	iH264EncInitParams.iAspectRatio = aspectratio;
    iH264EncInitParams.iInputFormat = aFormat;
    iH264EncInitParams.iPictureSize = aPictureSize;
    iH264EncInitParams.iBeforeInitialize |= EEncInputFormat;

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Sets the data source to be a camera, and sets the device to use direct
// capture for input.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetSourceCameraL( TInt /*aCameraHandle*/,
													TReal /*aPictureRate*/ )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
    	{
    	PRINT_ERR( "SetSourceCameraL() called after Initialize()..leaving" );
        User::Leave( KErrPermissionDenied );
        return;
    	}

    User::Leave( KErrNotSupported );
	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Sets the data source to be memory buffers.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetSourceMemoryL( TReal aMaxPictureRate,
						TBool aConstantPictureRate,
						TBool aProcessRealtime )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
    	{
    	PRINT_ERR( "SetSourceMemoryL() called after Initialize()..leaving" );
        User::Leave( KErrPermissionDenied );
        return;
    	}

    if( ( aMaxPictureRate <= 0 ) ||
        ( aMaxPictureRate > KH264ENCIMPL_MAX_PICTURERATE ) )
    	{
    	PRINT_ERR( "Incorrect value of max picture rate..leaving" );
        User::Leave( KErrNotSupported );
        return;
    	}

    // Check for the picture rates supported
    PRINT_MSG( LEVEL_LOW, ( "CAriH264encHwDeviceImpl::SetSourceMemoryL() "
    		"Memory Picture rate is set as %f", aMaxPictureRate ) );

    iH264EncInitParams.iMaxPictureRate = aMaxPictureRate;
    iH264EncInitParams.iConstantPictureRate = aConstantPictureRate;
    iH264EncInitParams.iProcessRealtime = aProcessRealtime;
    iH264EncInitParams.iBeforeInitialize |= EEncSourceMemory;

	PRINT_EXIT;
	}


//----------------------------------------------------------------------------
// Sets the clock source to use for video timing. If no clock
// source is set. video encoding will not be synchronized, but
// will proceed as fast as possible, depending on input data and
// output buffer availability.
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetClockSource( MMMFClockSource* aClock )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
		{
        ClientFatalError( KErrPermissionDenied );
        return;
		}

    if( !aClock )
		{
        ClientFatalError( KErrArgument );
        return;
		}

    PRINT_MSG( LEVEL_LOW, ( " CAriH264encHwDeviceImpl::SetClockSource() "
    		"encoder clockSource is %x", ( TInt ) aClock ) );

    iClockSource = aClock;

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Sets pre-processing options for RGB to YUV color space conversion
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetRgbToYuvOptionsL( TRgbRange /*aRange*/,
		const TYuvFormat& /*aOutputFormat*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetRgbToYuvOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets pre-processing options for YUV to YUV data format conversion
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetYuvToYuvOptionsL(
		const TYuvFormat& /*aInputFormat*/,
		const TYuvFormat& /*aOutputFormat*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetYuvToYuvOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets the pre-processing types to be used in a hardware device
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetPreProcessTypesL(
											TUint32 /*aPreProcessTypes*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetPreProcessTypesL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets pre-processing options for rotation
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetRotateOptionsL(
											TRotationType /*aRotationType*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetRotateOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets pre-processing options for scaling
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetScaleOptionsL( const TSize& /*aTargetSize*/,
											TBool /*aAntiAliasFiltering*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetScaleOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets pre-processing options for input cropping
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetInputCropOptionsL( const TRect& /*aRect*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetInputCropOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets pre-processing options for output cropping
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetOutputCropOptionsL( const TRect& /*aRect*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetOutputCropOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets pre-processing options for output padding
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetOutputPadOptionsL(
		const TSize& /*aOutputSize*/, const TPoint& /*aPicturePos*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetOutputPadOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets color enhancement pre-processing options
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetColorEnhancementOptionsL(
								const TColorEnhancementOptions& /*aOptions*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetColorEnhancementOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets frame stabilisation options
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetFrameStabilisationOptionsL(
		const TSize& /*aOutputSize*/, TBool /*aFrameStabilisation*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetFrameStabilisationOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
// Sets custom implementation-specific pre-processing options.
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SetCustomPreProcessOptionsL(
												const TDesC8& /*aOptions*/)
	{
	PRINT_ENTRY;
	PRINT_ERR( "SetCustomPreProcessOptionsL not supported...leaving \n" );
	PRINT_EXIT;
    User::Leave( KErrNotSupported );
	}

//----------------------------------------------------------------------------
//  Proxy which recieves callbacks from Hw Device
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetProxy( MMMFDevVideoRecordProxy& aProxy )
	{
	PRINT_ENTRY;

    if( iEncStateMac->IsInitialized() )
		{
        ClientFatalError( KErrPermissionDenied );
        return ;
		}
    iMMFDevVideoRecordProxy = &aProxy;

	PRINT_EXIT;
	}


//----------------------------------------------------------------------------
// Callback to indicate the input buffer is consumed
//----------------------------------------------------------------------------
//

TInt CAriH264encHwDeviceImpl::InputBufferConsumed ( TAny* aInp, TInt aError )
	{
	PRINT_ENTRY;

	if( !aInp )
		{
		return KErrArgument;
		}

	TVideoPicture *picture = (TVideoPicture *) aInp;
	if( ( picture->iOptions & TVideoPicture::ETimestamp )
			&& aError != ( KErrCancel ) )
	    {
	    iLastEncodedPictureTimestamp = picture->iTimestamp.Int64();
	    }

	// if custom buffer emabled then add it to queue else return the picture
	// to client
	if ( !iInputBufReturnToPreProc )
		{
		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl"
		"::InputBufferConsumed()-return picture back to client" ) );

	    iMMFDevVideoRecordProxy->MdvrpReturnPicture( picture );
		}
	else
		{
		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl"
		"::InputBufferConsumed()-return picture back to Input Device" ) );
	    iInputDevice->ReturnPicture( picture );
		}

    if ( iInputEndCalled && ( iEngine->NumInputBuffers() == 0 )
    	&& ( iCodec->IsCurrentPictureSkipped() ) )
    	{
    	PRINT_MSG( LEVEL_HIGH, ("CAriH264encHwDeviceImpl"
    			"::InputBufferConsumed()::Calling stream end " ) );
        Stop();
        iMMFDevVideoRecordProxy->MdvrpStreamEnd();
        return( KErrNone );
		}

	PRINT_EXIT;
	return KErrNone;
	}

//----------------------------------------------------------------------------
// Callback to indicate the output buffer is ready
//----------------------------------------------------------------------------
//

TInt CAriH264encHwDeviceImpl::OutputBufferReady ( TAny* aOup, TInt aError )
	{
	PRINT_ENTRY;

    TVideoOutputBuffer *outputBuf = ( TVideoOutputBuffer* ) aOup;
	TInt error = KErrNone;
	TInt length = outputBuf->iData.Length();

	if (iFrozen)
		{
		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::OutputBufferReady() "
				"Frozen state, so drop output picture" ) );
		outputBuf->iData.Set( ( TUint8* )outputBuf->iData.Ptr()
														,iOutputBufferSize );
		iEngine->AddOutput( outputBuf );
		return KErrNone;
		}

    if( aError == KErrNone )
    	{
		if ( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
			{
			error = iInternalOutputBufferQueue.Append( outputBuf );
			if ( error != KErrNone )
				{
				ClientFatalError ( error );
				return KErrNone;
				}

			// get the packet offset infor
			error = iCodec->GetParam( CONTROL_CMD_GET_PACKET_OFFSET_DATA,
								iFreeBufferQueueForPacketOffsetInfo[0] );
			if ( error != KErrNone )
				{
				ClientFatalError( error );
				return KErrNone;
				}

			//remove the first element and add it to filled Queue
			TUint* filledOffsetLengthInfoBuffer
				  = iFreeBufferQueueForPacketOffsetInfo[0];
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
				iPacketOffSetCurrentPosition
					= iFilledBufferQueueForPacketOffsetInfo[0];
				iPacketsPending	 = ETrue;

				while ( iOutputFreeBufferQueue.Count() && iPacketsPending )
					{
					TVideoOutputBuffer *outBuf = iOutputFreeBufferQueue[0];
					FillVideoSegment( outBuf, iInternalOutputBufferQueue[0] );
					iOutputFreeBufferQueue.Remove( 0 );

					#ifdef CALCINSTANTBITRATE
					CalculateInstantBitRate( *outBuf );
					#endif

					iMMFDevVideoRecordProxy->MdvrpNewBuffer( outBuf );
					}
				}
			 TInt ts
			   = ( TInt ) ( ( outputBuf->iCaptureTimestamp ).Int64() / 1000 );
 			iSizePerFrame = 0;
			}
			else
				{
				#ifdef CALCINSTANTBITRATE
				CalculateInstantBitRate( *outputBuf );
				#endif

				iSizePerFrame = outputBuf->iData.Length();
				iSizePerFrame = 0 ;
				// inform devvideo record that the new encoded buffer is
				// avaibable
				iMMFDevVideoRecordProxy->MdvrpNewBuffer( outputBuf );
				}
           iNoOfOutputFramesPerSec++;
		   }

    else if ( aError == KErrCancel )
    	{
        // add the buffer back  to outputQueue
        outputBuf->iData.Set( ( TUint8* )outputBuf->iData.Ptr(),
												iOutputBufferSize );

		if ( iH264EncInitParams.iDataUnitType == EDuVideoSegment )
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
    	PRINT_MSG( LEVEL_HIGH, ("CAriH264encHwDeviceImpl"
    			"::OutputBufferReady()::Calling stream end " ) );
        Stop();
        iMMFDevVideoRecordProxy->MdvrpStreamEnd();
        return( KErrNone );
		}

	PRINT_EXIT;
	return KErrNone;
	}

//----------------------------------------------------------------------------
// Callback to indicate the command has been processed
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::CommandProcessed ( TInt aCmd, TAny* aCmdData,
												TInt aError )
	{
	PRINT_ENTRY;

    switch( aCmd )
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
			TH264EncInitParams *currentParams
				= ( TH264EncInitParams* ) aCmdData;
			if( ( aError != KErrNone ) || ( aError != KErrCancel ) )
				{
				delete currentParams;
				return;
				}
			else{
				iH264EncInitParams.iBitErrors
					= currentParams->iBitErrors;
				iH264EncInitParams.iPacketLosses
					= currentParams->iPacketLosses;
				iH264EncInitParams.iRandomAccessRate
					= currentParams->iRandomAccessRate;

				delete currentParams;
				}
		    }
		    break;

		case CONTROL_CMD_SET_COMPLEXITY_LEVEL:
			{
			TH264EncInitParams *currentParams
				= ( TH264EncInitParams* ) aCmdData;
			delete currentParams;
		    }
		    break;

		case CONTROL_CMD_SET_CHANNEL_BIT_ERROR_RATE:
			{
			TReal* currentParams = ( TReal* )aCmdData;
			delete currentParams;
		    }
		    break;

		case CONTROL_CMD_SET_SLICELOSS:
			{
			TH264EncSliceLoss* currentParams
					= ( TH264EncSliceLoss* )aCmdData;
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

void CAriH264encHwDeviceImpl::FatalErrorFromProcessEngine( TInt aError )
	{
	PRINT_ENTRY;

    ClientFatalError( aError );

    PRINT_EXIT;
	}


//----------------------------------------------------------------------------
//  Enables inserting H324 pre-defined config data (VOL / SPS / PPS / etc. )
//----------------------------------------------------------------------------
//

TInt CAriH264encHwDeviceImpl::H324AnnexKDefinedEncoderConfigDataOn()
	{
	PRINT_ENTRY;

	if( iEncStateMac->IsInitialized() )
    	{
    	PRINT_MSG( LEVEL_LOW, ("Not permitted as the encoder is already "
    			"initialised" ) );
        return KErrNotReady;
    	}
	iH264EncInitParams.iVTFastCallSetUp = ETrue;

	PRINT_EXIT;
    return KErrNone;
	}


//----------------------------------------------------------------------------
//  2 phase constructor
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::ConstructL()
	{
	PRINT_ENTRY;

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
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iPattern = EYuv420Chroma2;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iPattern = EYuv420Chroma3;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iCoefficients = EYuvBt709Range1;
    inputFormat.iYuvFormat.iPattern = EYuv420Chroma1;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iPattern = EYuv420Chroma2;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iPattern = EYuv420Chroma3;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}


    inputFormat.iYuvFormat.iCoefficients = EYuvBt601Range0;
    inputFormat.iYuvFormat.iPattern = EYuv420Chroma1;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iPattern = EYuv420Chroma2;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iPattern = EYuv420Chroma3;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iCoefficients = EYuvBt601Range1;
    inputFormat.iYuvFormat.iPattern = EYuv420Chroma1;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iPattern = EYuv420Chroma2;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    inputFormat.iYuvFormat.iPattern = EYuv420Chroma3;
    error = iSupportedInputFormats.Append( inputFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending input format...leaving \n" );
		User::Leave( error );
		}

    // Create Array of Supported Output Formats
    CCompressedVideoFormat* compressedVideoFormat =
                    CCompressedVideoFormat::NewL( KH264MimeType );

	CleanupStack::PushL( compressedVideoFormat );
    error = iSupportedOutputFormats.Append( compressedVideoFormat );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

	// unknown level is set for short mime type - decide in initialize
    error = iLevels.Append( KH264ENCIMPL_LEVEL_UNKNOWN );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	// Level 1
    CCompressedVideoFormat* compressedVideoFormat2 =
                    CCompressedVideoFormat::NewL( KH264MimeType2 );

	CleanupStack::PushL( compressedVideoFormat2 );
    error = iSupportedOutputFormats.Append( compressedVideoFormat2 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

    error = iLevels.Append( KH264ENCIMPL_LEVEL_1 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	// Level 1.1
    // Bitrate and level will be decided in Initialize
	CCompressedVideoFormat* compressedVideoFormat3 =
                    CCompressedVideoFormat::NewL( KH264MimeType3 );

	CleanupStack::PushL( compressedVideoFormat3 );
    error = iSupportedOutputFormats.Append( compressedVideoFormat3 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

    error = iLevels.Append( KH264ENCIMPL_LEVEL_11 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	// Level 1.2
	CCompressedVideoFormat* compressedVideoFormat4 =
                    CCompressedVideoFormat::NewL( KH264MimeType4 );

	CleanupStack::PushL( compressedVideoFormat4 );
    error = iSupportedOutputFormats.Append( compressedVideoFormat4 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

    error = iLevels.Append( KH264ENCIMPL_LEVEL_12 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	// Level 1.3
	CCompressedVideoFormat* compressedVideoFormat5 =
                    CCompressedVideoFormat::NewL( KH264MimeType5 );

	CleanupStack::PushL( compressedVideoFormat5 );
    error = iSupportedOutputFormats.Append( compressedVideoFormat5 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

    error = iLevels.Append( KH264ENCIMPL_LEVEL_13 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	// Level 2
    CCompressedVideoFormat* compressedVideoFormat6 =
                    CCompressedVideoFormat::NewL( KH264MimeType6 );

	CleanupStack::PushL( compressedVideoFormat6 );

    error = iSupportedOutputFormats.Append( compressedVideoFormat6 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

	error = iLevels.Append( KH264ENCIMPL_LEVEL_2 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	// Level 2.1
	CCompressedVideoFormat* compressedVideoFormat7 =
                    CCompressedVideoFormat::NewL( KH264MimeType7 );

	CleanupStack::PushL( compressedVideoFormat7 );

    error = iSupportedOutputFormats.Append( compressedVideoFormat7 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

    error = iLevels.Append( KH264ENCIMPL_LEVEL_21 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	// Level 2.2
	CCompressedVideoFormat* compressedVideoFormat8 =
                    CCompressedVideoFormat::NewL( KH264MimeType8 );

	CleanupStack::PushL( compressedVideoFormat8 );

    error = iSupportedOutputFormats.Append( compressedVideoFormat8 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

    error = iLevels.Append( KH264ENCIMPL_LEVEL_22 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	// Level 3
	CCompressedVideoFormat* compressedVideoFormat9 =
                    CCompressedVideoFormat::NewL( KH264MimeType9 );

	CleanupStack::PushL( compressedVideoFormat9 );

    error = iSupportedOutputFormats.Append( compressedVideoFormat9 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

    error = iLevels.Append( KH264ENCIMPL_LEVEL_3 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	// Level 1b
	CCompressedVideoFormat* compressedVideoFormat10 =
                    CCompressedVideoFormat::NewL( KH264MimeType10 );

	CleanupStack::PushL( compressedVideoFormat10 );

    error = iSupportedOutputFormats.Append( compressedVideoFormat10 );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending output format...leaving \n" );
		User::Leave( error );
		}
	CleanupStack::Pop();

    error = iLevels.Append( KH264ENCIMPL_LEVEL_1b );
	if( error != KErrNone )
		{
		PRINT_ERR( "Error while appending supported level...leaving \n" );
		User::Leave( error );
		}

	iPeriodicTimer	= CPeriodic::NewL( CActive::EPriorityIdle );
    iOutputFreeBufferQueue.Reset();
    iInternalOutputBufferQueue.Reset();
    iEncStateMac = CStateMachine::NewL();

    PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Sends the updated time to the codec
//----------------------------------------------------------------------------
//
TInt CAriH264encHwDeviceImpl::UpdateTime()
	{
	PRINT_ENTRY;

    if ( !iClockSource )
    	{
        ClientFatalError( KErrBadHandle );
        return -1;
    	}

    // send the time values to the codec
    iCodec->SetUpdatedRefernceTime( iTotalTime );

	PRINT_EXIT;
    return 1;
	}

//----------------------------------------------------------------------------
// Create the output buffers in Coded picture mode
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::CreateCodedOutputBuffersL( TUint aSize )
	{
    PRINT_ENTRY;

    if( iH264EncInitParams.iDataUnitType != EDuVideoSegment )
    	{
    	iOutputBufferSize = aSize;
		}

    // Allocate memory for TVideoOutputBuffer
    iOutputBuffers  = new ( ELeave ) TVideoOutputBuffer[
								  iH264EncInitParams.iMinNumOutputBuffers];

    for ( TInt i = 0; i < iH264EncInitParams.iMinNumOutputBuffers; i++ )
    	{
        iOutputBuffers[i].iData.Set( NULL, 0);
		}

    // Create the Buffer and add it to Queue
    for ( TInt i = 0; i < iH264EncInitParams.iMinNumOutputBuffers; i++ )
    	{
        TUint8* ptr = new ( ELeave ) TUint8[aSize];
        CleanupStack::PushL( ptr );
        iOutputBuffers[i].iData.Set( ptr, aSize );
        CleanupStack::Pop();
        InitializeOuputCodedBuffer( iOutputBuffers[i] );
        TInt error = iOutputFreeBufferQueue.Append( iOutputBuffers + i );
		if ( error != KErrNone )
			{
			PRINT_ERR( "Error while appending output buffer to the"
					"output buffer queue \n" );
			User::Leave( error );
			return;
			}
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Creates the temporary output buffers
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::CreateInternalOutputBuffersL(
													TUint aBufferSize )
	{
	PRINT_ENTRY;

	iOutputBufferSize = aBufferSize;

    // Allocate memory for TVideoOutputBuffer
    iInternalOutputBuffers  = new ( ELeave ) TVideoOutputBuffer[
									KH264ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS];

    for ( TInt i = 0; i < KH264ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
		{
        iInternalOutputBuffers[i].iData.Set( NULL, 0 );
		}

    // Create the Buffer and add it to Queue
    for ( TInt i = 0; i < KH264ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
		{
        TUint8* ptr = new ( ELeave ) TUint8[aBufferSize];
        CleanupStack::PushL( ptr );
        iInternalOutputBuffers[i].iData.Set( ptr, aBufferSize );
        CleanupStack::Pop();
        InitializeOuputCodedBuffer( iInternalOutputBuffers[i] );
        TInt error
			= iInternalOutputBufferQueue.Append( iInternalOutputBuffers + i );

		if ( error != KErrNone )
			{
			ClientFatalError( error );
			return;
			}
		}

	PRINT_EXIT;
	}


//----------------------------------------------------------------------------
// Creates the buffers required to store length and offset
// info of packets
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::CreatePacketOffsetLengthInfoBuffersL(
														TUint aNumOfPackets )
	{
	PRINT_ENTRY;

	iPacketOffSetAndLengthInfoBuffers = new ( ELeave )
			( TUint*[KH264ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS] );

	for ( TInt i = 0; i < KH264ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
		{
		iPacketOffSetAndLengthInfoBuffers[i] = NULL;
		}

	for ( TInt i = 0; i < KH264ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS; i++ )
		{
		iPacketOffSetAndLengthInfoBuffers[i] = ( TUint* ) ( new ( ELeave )
				TUint8[aNumOfPackets] );
		TInt error = iFreeBufferQueueForPacketOffsetInfo.Append(
								iPacketOffSetAndLengthInfoBuffers[i] );
		if( error != KErrNone )
			{
			PRINT_ERR( "Error while appending packet lengthoffset buffer to "
					"queue \n" );
			User::Leave( error );
			}
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// decides whether picture can be encoded or not
//----------------------------------------------------------------------------
//
TBool  CAriH264encHwDeviceImpl::CanEncode( TVideoPicture *aPicture )
	{
	PRINT_ENTRY;

	// check with the last encoded picture and decide whether it can be
	//	processed or skipped
    if( aPicture->iTimestamp.Int64() < iLastEncodedPictureTimestamp )
		{
        return EFalse;
    	}

    // check with current clock
    if( ( aPicture->iTimestamp.Int64() ) < ( ( iClockSource->Time().Int64() -
    		iTotalTime ) + KH264ENCIMPL_ENCODEAHEAD ) )
    	{
        return EFalse;
    	}

	PRINT_EXIT;
    return ETrue;
	}
//----------------------------------------------------------------------------
//  Skips the Input Picture
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::SkipInputPicture( TVideoPicture *aPicture )
	{

	PRINT_ENTRY;

	// add the buffer back to queue
	if ( !iInputBufReturnToPreProc )
		{
		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::SkipInputPicture()"
				"-return picture back to client" ) );
		iMMFDevVideoRecordProxy->MdvrpReturnPicture( aPicture );
		}
	else
		{
		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::SkipInputPicture()-"
				"return picture back to Input Device" ) );
		iInputDevice->ReturnPicture( aPicture );
		}

	if ( !iFrozen )
		{
		iPictureCounters.iPicturesSkippedRateControl++;
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
// Indicates whether the next frame is to be encoded as an I frame
//----------------------------------------------------------------------------
//
TBool CAriH264encHwDeviceImpl::IsForcedIFrameRequired(
													TVideoPicture* aPicture )
	{
	PRINT_ENTRY;

    if( ( aPicture->iOptions & TVideoPicture::EReqInstantRefresh ) ||
	   iPictureLoss )
		{
		PRINT_MSG( LEVEL_HIGH, ("CAriH264encHwDeviceImpl"
				"::IsForcedIFrameRequired() ETrue 1--" ) );
        return ETrue;
		}

	PRINT_EXIT;
    return EFalse;
	}

//----------------------------------------------------------------------------
//  Extracts 1 packet from a frame and Fills output buffer with the same
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::FillVideoSegment(
		TVideoOutputBuffer* aOutputBuf, TVideoOutputBuffer* aSrcOutputBuf )
	{
	PRINT_ENTRY;

	// first time fill all config information in one o/p buffer
	if ( iIsConfigDataFilledInFirstOutputBuffer )
		{
		TUint configDataLength = 0;
        TUint* startPtr = iPacketOffSetCurrentPosition;
        TInt noOfConfigParamas = 0;

#ifdef EMZ_AVC_ENC_CODEC_SEIINFO_ENABLE
        PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::FillVideoSegment() "
        		"SEI enabled" ) );
		noOfConfigParamas= KNumOfConfigParams_SEI;
#else
		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::FillVideoSegment() "
				"SEI disabled" ) );
		noOfConfigParamas= KNumOfConfigParams;
#endif
		for( TInt i = 0; i < noOfConfigParamas; i++ )
			{
			configDataLength += * ( ++startPtr ) ++;
			}

		// copy data into output buffer
		Mem::Copy( ( TUint8* )( ( TUint8* )aOutputBuf->iData.Ptr() ),
				( TUint8* )( aSrcOutputBuf->iData.Ptr() ), configDataLength );

		iTotalLengthFilledSoFarInPacketMode += configDataLength;

		if( iH264EncInitParams.iDataEncapsulation == EDuGenericPayload )
			{

			// word boundary calculation
			TUint32 offset =  ( TUint32 )( ( TUint8* )aOutputBuf->iData.Ptr()
							+ configDataLength	);

			if ( offset % KWordLength )
				{
				offset	=  KWordLength - ( offset % KWordLength );
				}
			else
				{
				offset	=  0;
				}

			configDataLength += offset;
			TUint8* ptr = ( ( TUint8* )aOutputBuf->iData.Ptr()
							+ configDataLength );

			// copy offset and length information
			Mem::Copy( ptr, ( TUint8* )iPacketOffSetCurrentPosition,
						( noOfConfigParamas*KDoubleWordLength ) );

			ptr += ( noOfConfigParamas * KDoubleWordLength );

			//put the num of nal units information
			TUint* nalInfo = ( TUint* )ptr;
			nalInfo[0] = noOfConfigParamas;

			configDataLength += ( ( noOfConfigParamas*KDoubleWordLength ) +
								KWordLength );
			}

        // increment the packet offsetpointer
		iPacketOffSetCurrentPosition += ( noOfConfigParamas * 2 );
		aOutputBuf->iData.Set( aOutputBuf->iData.Ptr(),configDataLength );
		iIsConfigDataFilledInFirstOutputBuffer = EFalse;
		}
	else
		{
		TUint currentPacketLength = *( ++iPacketOffSetCurrentPosition );

		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl::FillVideoSegment()"
				" currentPacketLength is %d", ( TInt )currentPacketLength ) );

		if ( iTotalLengthFilledSoFarInPacketMode
				< iTotalOutputBufferLengthInPacketMode )
			{
			Mem::Copy( ( TUint8* )( aOutputBuf->iData.Ptr() ),
				( TUint8* )( aSrcOutputBuf->iData.Ptr() +
				iTotalLengthFilledSoFarInPacketMode ), currentPacketLength );

			aOutputBuf->iData.Set( aOutputBuf->iData.Ptr(),
										currentPacketLength );

			iTotalLengthFilledSoFarInPacketMode
				= iTotalLengthFilledSoFarInPacketMode + currentPacketLength;

			if ( iH264EncInitParams.iDataEncapsulation == EDuGenericPayload )
				{
				// fill the offset and length information
				TUint32 offset = ( TUint32 )( aOutputBuf->iData.Ptr()
									+ currentPacketLength );

				if ( offset % KWordLength )
					{
					offset = ( KWordLength - ( offset % KWordLength ) );
					}
				else
					{
					offset	=  0;
					}

				TUint* ptr = ( TUint* ) ( aOutputBuf->iData.Ptr()
							+ currentPacketLength + offset );

				ptr[0] = 0;
				ptr[1] = *( iPacketOffSetCurrentPosition );
				ptr[2] = 1;

				aOutputBuf->iData.Set( aOutputBuf->iData.Ptr(),
							currentPacketLength + offset + KDoubleWordLength
							+ KWordLength );
				}

				if ( iTotalLengthFilledSoFarInPacketMode  ==
							iTotalOutputBufferLengthInPacketMode )
					{
					iTotalLengthFilledSoFarInPacketMode = 0;
					iPacketsPending	 = EFalse;

					// remove packet offset info buffer from filled Q and
					// add it to free Q
					TUint* freeOffsetLengthInfoBuffer
							= iFilledBufferQueueForPacketOffsetInfo[0];

					iFilledBufferQueueForPacketOffsetInfo.Remove( 0 );
					TInt error = iFreeBufferQueueForPacketOffsetInfo.Append(
											freeOffsetLengthInfoBuffer );

					if ( error != KErrNone )
						{
						ClientFatalError( error );
						return;
						}

					// Remove the internal buffer and add it back to process
					// engine
					if ( iInternalOutputBufferQueue.Count() )
						{
						TVideoOutputBuffer* outBuf
							= iInternalOutputBufferQueue[0];
						iInternalOutputBufferQueue.Remove( 0 );
						outBuf->iData.Set( ( TUint8* )outBuf->iData.Ptr(),
														iOutputBufferSize );

						// Add the Buffer back to the Process Engine
						if( ( !iEncStateMac->IsInputEndPending() ) &&
								( !iEncStateMac->IsStopped() ) )
							{
							iEngine->AddOutput( ( TAny* )outBuf );
							}
						}

					// Still more encoded packets are available
					if( iInternalOutputBufferQueue.Count() )
						{
						iPacketsPending = ETrue;
						iTotalOutputBufferLengthInPacketMode
							= iInternalOutputBufferQueue[0]->iData.Length();

						// Get the packet offset info buffer
						if( iFilledBufferQueueForPacketOffsetInfo.Count() )
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
		}

    // Update the specific members of the output buffer
	aOutputBuf->iRequiredSeveralPictures
		= aSrcOutputBuf->iRequiredSeveralPictures;
    aOutputBuf->iRandomAccessPoint = aSrcOutputBuf->iRandomAccessPoint;
	aOutputBuf->iCaptureTimestamp  = aSrcOutputBuf->iCaptureTimestamp;

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Initializes the members of the output coded buffers created
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::InitializeOuputCodedBuffer(
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

//----------------------------------------------------------------------------
//   Sets level and bitrate based on size information
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::SetLevelAndBitRate()
	{
	PRINT_ENTRY;

	if( (iH264EncInitParams.iPictureSize.iWidth <= KH264ENCIMPL_QCIF_WIDTH )
				&& ( iH264EncInitParams.iPictureSize.iHeight
								<= KH264ENCIMPL_QCIF_HEIGHT ) )
		{
	    iH264EncInitParams.iLevel = KH264ENCIMPL_LEVEL_1;
		iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_1;
        iH264EncInitParams.iTargetPictureRate = KPictureRate15;
		}
	//QVGA
	else
		if( ( iH264EncInitParams.iPictureSize.iWidth
						<= KH264ENCIMPL_QVGA_WIDTH ) &&
							( iH264EncInitParams.iPictureSize.iHeight
								<= KH264ENCIMPL_QVGA_HEIGHT ) )
		{
	    iH264EncInitParams.iLevel = KH264ENCIMPL_LEVEL_12;
		iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_12;
        iH264EncInitParams.iTargetPictureRate = KPictureRate15;
		}
	//CIF
	else
		if( ( iH264EncInitParams.iPictureSize.iWidth
					<= KH264ENCIMPL_CIF_WIDTH ) &&
						( iH264EncInitParams.iPictureSize.iHeight
								<= KH264ENCIMPL_CIF_HEIGHT ) )
		{
	    iH264EncInitParams.iLevel = KH264ENCIMPL_LEVEL_13;
		iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_13;
        iH264EncInitParams.iTargetPictureRate = KPictureRate30;
		}
	// VGA
	else
		if( ( iH264EncInitParams.iPictureSize.iWidth
				<= KH264ENCIMPL_VGA_WIDTH ) &&
					( iH264EncInitParams.iPictureSize.iHeight
							<= KH264ENCIMPL_VGA_HEIGHT ) )
		{
	    iH264EncInitParams.iLevel = KH264ENCIMPL_LEVEL_22;
		iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_22;
        iH264EncInitParams.iTargetPictureRate = KPictureRate15;
		}
	else
		{
	    iH264EncInitParams.iLevel = KH264ENCIMPL_LEVEL_3;
		iH264EncInitParams.iBitRate = KH264ENCIMPL_BITRATE_LEVEL_3;
        iH264EncInitParams.iTargetPictureRate = KPictureRate25;
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Checks if the specified input format is supported or not
//----------------------------------------------------------------------------
//
TBool CAriH264encHwDeviceImpl::CheckInputFormat(
					const TUncompressedVideoFormat& aFormat )
	{
	PRINT_ENTRY;

	// Check if encoder supports the passed input format
	if ( aFormat.iDataFormat != EYuvRawData )
		{
		return EFalse;
		}

	if ( aFormat.iYuvFormat.iDataLayout != EYuvDataPlanar )
	   {
		PRINT_EXIT;
		return EFalse;
	   }

	if ( ( aFormat.iYuvFormat.iCoefficients != EYuvBt709Range0 ) &&
		 ( aFormat.iYuvFormat.iCoefficients != EYuvBt709Range1 ) &&
		 ( aFormat.iYuvFormat.iCoefficients != EYuvBt601Range0 ) &&
		 ( aFormat.iYuvFormat.iCoefficients != EYuvBt601Range1 ) )
		{
		return EFalse;
		}

	if ( ( aFormat.iYuvFormat.iPattern != EYuv420Chroma1 ) &&
		 ( aFormat.iYuvFormat.iPattern != EYuv420Chroma2 ) &&
		 ( aFormat.iYuvFormat.iPattern != EYuv420Chroma3 ) )
		{
		return EFalse;
		}

	PRINT_EXIT;
	return ETrue;
	}


//----------------------------------------------------------------------------
//  Nofities the client that the fatal error happend in Hw device
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::ClientFatalError( TInt aError )
	{
	PRINT_ENTRY;
	PRINT_MSG( LEVEL_CRITICAL, ("CAriH264encHwDeviceImpl::ClientFatalError()"
			" Error is %d", aError ) );

    if( iClockSource )
    	{
        iPeriodicTimer->Cancel();
    	}

	// Stop processing
	if( !iEncStateMac->IsStopped() )
		{
		if( iEncStateMac->IsInitialized() )
			{
			Stop();
			}
		}
	iEncStateMac->Transit( CStateMachine::EDeadStateCommand );
	iMMFDevVideoRecordProxy->MdvrpFatalError( this, aError );

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Output buffers are created with the specified size
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::ReallocSegmentOutputBuffersL( TUint aSize )
	{
	PRINT_ENTRY;

	TInt count = iOutputFreeBufferQueue.Count();
    for ( TInt i = 0; i < count; i++ )
    	{
		// dellallocate memory for this buffer
		TVideoOutputBuffer* ouputBuf = iOutputFreeBufferQueue[i];
        if ( ouputBuf->iData.Ptr() )
        	{
            delete ( TUint8* )ouputBuf->iData.Ptr();
			}

		// and allocate memory and store it in temp Q
		TUint8* ptr = new ( ELeave ) TUint8[aSize];
		CleanupStack::PushL( ptr );
		ouputBuf->iData.Set( ptr, aSize );
		CleanupStack::Pop();
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Reallocates single buffer
//----------------------------------------------------------------------------
//
void CAriH264encHwDeviceImpl::ReallocateSegmentBufferL(
									TVideoOutputBuffer* aBuffer )
	{
	PRINT_ENTRY;

	TInt size = iH264EncInitParams.iMaxCodedSegmentSize;
	if( iH264EncInitParams.iDataEncapsulation == EDuGenericPayload )
		{
		size += KDoubleWordLength + KWordLength;
		}

    if ( aBuffer->iData.Ptr() )
		{
        delete ( TUint8* )aBuffer->iData.Ptr();
		}

	// and allocate memory and store it in temp Q
	TUint8* ptr = new ( ELeave ) TUint8[size];
	CleanupStack::PushL( ptr );
	aBuffer->iData.Set( ptr, ( size ) );
	CleanupStack::Pop();

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Creates single buffer for frame mode to packet mode change
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::CreateCodedBufferForModeChangeL()
	{
	PRINT_ENTRY;

	iCodedBufferForPacketModeChange = new ( ELeave ) TVideoOutputBuffer;
	TUint8* ptr = new ( ELeave ) TUint8[iOutputBufferSize];
	CleanupStack::PushL( ptr );
	iCodedBufferForPacketModeChange->iData.Set( ptr, iOutputBufferSize );
	CleanupStack::Pop();
    InitializeOuputCodedBuffer( *iCodedBufferForPacketModeChange );

	PRINT_EXIT;
	}


//----------------------------------------------------------------------------
//  Creates buffers which are used as output buffers in packet mode. Function
//  is called only when frame mode - packet mode change occurs while encoding
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::CreateSegmentOutputBuffersL(
										TUint aNumOfBuffers, TUint aSize )
	{
	PRINT_ENTRY;

	TInt segmentSize = aSize;
	// 4+4 bytes for offset and length
	if( iH264EncInitParams.iDataEncapsulation == EDuGenericPayload )
		{
		segmentSize += KDoubleWordLength + KWordLength;
		}

    // Allocate memory for TVideoOutputBuffer
    iSegmentBuffers  = new ( ELeave ) TVideoOutputBuffer[aNumOfBuffers];
    for ( TInt i = 0; i < aNumOfBuffers; i++ )
    	{
        iSegmentBuffers[i].iData.Set( NULL, 0 );
		}

    // Create the Buffer and add it to Queue
    for ( TInt i = 0; i < aNumOfBuffers; i++ )
    	{
        TUint8* ptr = new ( ELeave ) TUint8[segmentSize];
        CleanupStack::PushL( ptr );
        iSegmentBuffers[i].iData.Set( ptr, segmentSize );
		CleanupStack::Pop();
        InitializeOuputCodedBuffer( iSegmentBuffers[i] );
        TInt error = iOutputFreeBufferQueue.Append( iSegmentBuffers + i );
		if ( error != KErrNone )
			{
			PRINT_ERR( "Error while appending segment output buffer to the"
					"segment output buffer queue" );
			User::Leave( error );
			return;
			}
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Calculates the instant bit rate
//----------------------------------------------------------------------------
//

void CAriH264encHwDeviceImpl::CalculateInstantBitRate(
							const TVideoOutputBuffer &aOutBuf )
    {
	PRINT_ENTRY;

    TInt size = 0;
	TReal ts = ( ( aOutBuf.iCaptureTimestamp ).Int64() ) / 1000000.0;
	size = aOutBuf.iData.Length();
	TInt32 partInt = 0;
	Math::Int( partInt, ts );

	// Calculate stream size, averaged per 1 second
	if ( partInt > iCurrentIntTS )
		{
	#ifdef DEBUG_INFO
		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl"
				"::CalculateInstantBitRate(), Stream size per 1 second:  %d",
				iStreamSize * 8 ) );
		PRINT_MSG( LEVEL_LOW, ("CAriH264encHwDeviceImpl"
				"::CalculateInstantBitRate(), Number of frames per 1 second: "
				" %d", iNoOfOutputFramesPerSec ) );
	#endif
		// Reset global stream size
		iStreamSize = 0;
		iNoOfOutputFramesPerSec = 0;
		iCurrentIntTS = partInt;
		}
    iSizePerFrame = iSizePerFrame + size;
	iStreamSize = iStreamSize + size;

	PRINT_EXIT;
    }

//----------------------------------------------------------------------------
//  The implementation table entry which indicates the 1st function
// to call when H264 encoder hwdevice plugin is selected
//----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
		{ KUidH264EncoderHwDeviceImplUid,
				( TProxyNewLPtr )( CAriH264encHwDeviceImpl::NewL ) }
    };

//----------------------------------------------------------------------------
// Returns the implementation table
//----------------------------------------------------------------------------
//

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(
													TInt& aTableCount )
{
	aTableCount = sizeof( ImplementationTable )
					/ sizeof( TImplementationProxy );
	return ImplementationTable;
}
















