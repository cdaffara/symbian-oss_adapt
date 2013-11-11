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
* This is a source file, which is required for Mp4DecHwDevice.
*
*/

//User includes
#include "arimpeg4aspdechwdevice.h"
#include "aristatemachine.h"
#include "ariprint.h"

//manufacturer name
_LIT( KManufacturer, "Aricent" );

_LIT( KIdentifier, "MPEG4/H263 Video Decoder" );

//supported mime types for mp4
_LIT8( KMPEG4SupportedMimeType, "video/mp4v-es" );
// profile level 0
_LIT8( KMPEG4SupportedProfileLevel_0, "video/mp4v-es; profile-level-id=8" );
// profile level 0b / 11
_LIT8( KMPEG4SupportedProfileLevel_0b, "video/mp4v-es; profile-level-id=9" );
// profile level 1
_LIT8( KMPEG4SupportedProfileLevel_1, "video/mp4v-es; profile-level-id=1" );
// profile level 2
_LIT8( KMPEG4SupportedProfileLevel_2, "video/mp4v-es; profile-level-id=2" );
// profile level 3
_LIT8( KMPEG4SupportedProfileLevel_3, "video/mp4v-es; profile-level-id=3" );
// profile level 4
_LIT8( KMPEG4SupportedProfileLevel_4, "video/mp4v-es; profile-level-id=4" );
// profile level 5
_LIT8( KMPEG4SupportedProfileLevel_5, "video/mp4v-es; profile-level-id=5" );

//supported mime types for h263
_LIT8( KH263SupportedMimeType, "video/h263-2000" );
_LIT8( KH26398SupportedMimeType, "video/h263-1998" );

// h263 profile level 10
_LIT8( KH263SupportedProfileLevel_10, "video/h263-2000;profile=0;level=10" );
// h263 profile level 20
_LIT8( KH263SupportedProfileLevel_20, "video/h263-2000;profile=0;level=20" );
// h263 profile level 30
_LIT8( KH263SupportedProfileLevel_30, "video/h263-2000;profile=0;level=30" );
// h263 profile level 45
_LIT8( KH263SupportedProfileLevel_45, "video/h263-2000;profile=0;level=45" );


//---------------------------------------------------------------------------
//1st phase constructor of CAriMpeg4aspdecHwDevice
//---------------------------------------------------------------------------
//
CMMFVideoDecodeHwDevice* CAriMpeg4aspdecHwDevice::NewL()
	{
	PRINT_ENTRY;
	CAriMpeg4aspdecHwDevice* self = new ( ELeave ) CAriMpeg4aspdecHwDevice;
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	PRINT_EXIT;
	return self;
	}


//---------------------------------------------------------------------------
//Destructor
//---------------------------------------------------------------------------
//
CAriMpeg4aspdecHwDevice::~CAriMpeg4aspdecHwDevice()
	{
	PRINT_ENTRY;
	delete iInputFormat;
	delete iEngine;
	delete iCodec;
	delete iState;
	
    delete iVideoPictureHeader;

	// This is required becoz user may say exit before start at that point
	// OutputFreeBuffer is not created
	if ( iOutputFreeBuffer )
		{
		for ( TInt i = 0 ; i < KMaxOutputBuffers; i++ )
			{
            if ( ( iOutputFreeBuffer + i )->iHeader )
                {
			    delete ( iOutputFreeBuffer + i )->iHeader;
                }
			if ( ( iOutputFreeBuffer + i )->iData.iRawData )
				{
				delete
				( TUint8* )( iOutputFreeBuffer + i )->iData.iRawData->Ptr();
				}
            if ( ( iOutputFreeBuffer + i )->iData.iRawData )
                {
			    delete ( iOutputFreeBuffer + i )->iData.iRawData;
                }
			}
		delete [] iOutputFreeBuffer;
		}

	if ( iInputFreeBuffers )
		{
		TInt i;
		for ( i = 0 ; i < TInt( iBufferOptions.iMinNumInputBuffers ); i++ )
			{
			delete [] ( TUint8* )iInputFreeBuffers[i].iData.Ptr();
			delete iInputFreeBuffers[i].iUser;
			}
		delete [] iInputFreeBuffers;
		}

	if ( iCustomBufferOptions )
		{
		delete iCustomBufferOptions;
		}
    iOutputFreeBufferQueue.Close();
	iInputFreeBufferQueue.Close();

	iMaxPictureRates.Reset();
	iMaxPictureRates.Close();
	iSupportedFormats.Reset();
	iSupportedFormats.Close();

    iOutputDevice = NULL;
    iMMFDevVideoPlayProxy = NULL;
    iCustomBufferHandle = NULL;
    PRINT_EXIT;
	}
//---------------------------------------------------------------------------
//Retrieves a custom interface to the device.
//---------------------------------------------------------------------------
//
TAny* CAriMpeg4aspdecHwDevice::CustomInterface( TUid /*aInterface*/ )
	{
	PRINT_ENTRY;
	PRINT_EXIT;

	return NULL;
	}


//---------------------------------------------------------------------------
//Retrieves decoder information about this hardware device. The device
//creates a CVideoDecoderInfo structure, fills it with correct data,
//pushes it to the cleanup stack and returns it. The client will delete the
//object when it is no longer needed.
//---------------------------------------------------------------------------
//
CVideoDecoderInfo* CAriMpeg4aspdecHwDevice::VideoDecoderInfoLC()
    {
    PRINT_ENTRY;
    for ( TUint i = 0; i < KDecoderInfoCount; i++ )
		{
		// construct the video types for iVidTypes
		CCompressedVideoFormat* format = NULL;
		TPtrC8 mimeType = KDecoderInfoMimeArray[i];
		format = CCompressedVideoFormat::NewL( mimeType );
		CleanupStack::PushL(format);
		TInt status = iSupportedFormats.Append( format );
		if ( status != KErrNone )
			{
			PRINT_MSG( LEVEL_LOW, ( " Format support is not done, retval of "
					"append = %d \n", status ) );
			}
		CleanupStack::Pop( format );

		// max picture rates
		TPictureRateAndSize pictureRateAndSize;
		pictureRateAndSize.iPictureSize = TSize( KMaxFrameWidth,
				                                KMaxFrameHeight );
		pictureRateAndSize.iPictureRate = KPicRate;
		status = iMaxPictureRates.Append(pictureRateAndSize);
		if ( status != KErrNone )
			{
			PRINT_MSG( LEVEL_LOW, ( " error in appendng picturerates = %d \n",
					status ) );
			}
		}

    TUint maxBitRate = KMaxBitRate;

    CVideoDecoderInfo* videoDecoderInfo = CVideoDecoderInfo::NewL(
        	KUidMp4H263DecoderHwDevice,
        	KManufacturer,
        	KIdentifier,
         	TVersion( 1, 0, 0 ),
        	iSupportedFormats.Array(),
        	EFalse,
        	EFalse,
			TSize( KMaxFrameWidth, KMaxFrameHeight ),
			maxBitRate,
			iMaxPictureRates.Array(),
			ETrue,
			ETrue );

    CleanupStack::PushL( videoDecoderInfo );

    PRINT_MSG( LEVEL_LOW, ( "Closing maxPictureRates&supportedFormats\n" ) );

    PRINT_EXIT;

    return videoDecoderInfo;
}

//---------------------------------------------------------------------------
//Reads header information from a coded data unit.
//---------------------------------------------------------------------------
//
TVideoPictureHeader* CAriMpeg4aspdecHwDevice::GetHeaderInformationL(
								TVideoDataUnitType /*aDataUnitType*/,
								TVideoDataUnitEncapsulation aEncapsulation,
								TVideoInputBuffer* aDataUnit )
	{
	PRINT_ENTRY;
    if ( !aDataUnit )
	    {
	    PRINT_ERR( "Input argument is not proper, aDataUnit is null \n" );
		User::Leave( KErrArgument );
		}

    if ( aEncapsulation != EDuElementaryStream )
    	{
    	PRINT_ERR( "aEncapsulation is not supported type \n" );
        User::Leave( KErrNotSupported );
    	}

    iVideoPictureHeader = new ( ELeave ) TVideoPictureHeader;

	TRAPD( err, CAriMpeg4aspdecWrapper::GetHeaderInfoL(  *aDataUnit,
												  *iVideoPictureHeader ) );

    if ( err == KErrNone )
		{
		PRINT_EXIT;
        return ( iVideoPictureHeader );
    	}
    delete iVideoPictureHeader;
    iVideoPictureHeader = NULL;

    switch ( err )
        {
        case KErrCorrupt:
        case KErrNoMemory:
        case KErrArgument:
        case KErrNotSupported:
            {
            PRINT_ERR( "GetHeaderInfoL is returned with error \n" );
            User::Leave( err );
            break;
            }
        case KErrUnderflow:
            {
            PRINT_ERR( "GetHeaderInfoL returnd with error KErrUnderflow\n" );
            break;
            }
        default:
            {
            PRINT_ERR( "GetHeaderInfoL is returned with error \n" );
            User::Leave( KErrGeneral );
            }
        };

    PRINT_EXIT;
	return NULL;
	}

//---------------------------------------------------------------------------
//Returns a header from GetHeaderInformationL() back to the decoder so that
//the memory can be freed.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::ReturnHeader( TVideoPictureHeader* aHeader )
	{
	PRINT_ENTRY;
	if ( !iState->IsInitialized() )
		{
		if ( iVideoPictureHeader == aHeader )
			{
			delete iVideoPictureHeader;
            iVideoPictureHeader = NULL;
			}
		else
			{
			iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrArgument );
			}
		}
	else
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Sets the device input format to a compressed video format.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetInputFormatL(
								   const CCompressedVideoFormat& aFormat,
								   TVideoDataUnitType aDataUnitType,
								   TVideoDataUnitEncapsulation aEncapsulation,
								   TBool aDataInOrder )
	{
	PRINT_ENTRY;
	if ( !iState->IsInitialized() )
		{
		if ( ( !aDataInOrder ) ||
             ( aEncapsulation == EDuGenericPayload ) ||
             ( aEncapsulation == EDuRtpPayload ) ||
             ( aDataUnitType == EDuArbitraryStreamSection ) )
             User::Leave( KErrNotSupported );

		if ( ( aFormat.MimeType().FindF( KMPEG4SupportedMimeType ) !=
															KErrNotFound ) ||
			( aFormat.MimeType().FindF( KH263SupportedMimeType ) !=
														    KErrNotFound ) ||
			( aFormat.MimeType().FindF( KH26398SupportedMimeType ) !=
														    KErrNotFound ) )

			{
			iInputFormat = CCompressedVideoFormat::NewL( aFormat );
			iDataUnitType  = aDataUnitType;
			iEncapsulation = aEncapsulation;
			}
		else
			{
			PRINT_ERR( "aFormat is not supported \n" );
			User::Leave( KErrNotSupported );
			}
		}
	else
		{
		PRINT_ERR( "istate is not initialized yet\n" );
		User::Leave( KErrNotReady );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Sets whether decoding should be synchronized to the current clock source,
//if any, or if pictures should instead be decoded as soon as possible.
//If decoding is synchronized, decoding timestamps are used if available,
//presentation timestamps are used if not. When decoding is not synchronized,
//pictures are decoded as soon as source data is available for them, and the
//decoder has a free output buffer. If a clock source is not available,
//decoding will not be synchronized.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SynchronizeDecoding( TBool /*aSynchronize*/ )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	return;
	}


//---------------------------------------------------------------------------
//Sets the Buffer Options as specified by the client.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetBufferOptionsL(
						const CMMFDevVideoPlay::TBufferOptions& aOptions )
	{
	PRINT_ENTRY;
	if ( !iState->IsInitialized() )
		{
		TTimeIntervalMicroSeconds zerotime( 0 );
        if ( ( aOptions.iMinNumInputBuffers < KMinInputBuffers ) ||
             ( aOptions.iPreDecodeBufferSize != 0 ) ||
             ( aOptions.iMaxPostDecodeBufferSize != 0 ) ||
             ( aOptions.iPreDecoderBufferPeriod != zerotime ) ||
             ( aOptions.iPostDecoderBufferPeriod != zerotime ) )
        	{
            User::Leave( KErrArgument );
        	}

		if ( ( aOptions.iMinNumInputBuffers > KMaxInputBuffers ) ||
               ( aOptions.iMaxInputBufferSize > KMaxInputBufferSize ) )
			{
		    User::Leave( KErrNotSupported );
			}

		iBufferOptions = aOptions;
		}
	else
		{
		PRINT_ERR( "istate is not initialized yet\n" );
		User::Leave( KErrNotReady );
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Gets the video decoder buffer options actually in use.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::GetBufferOptions(
								CMMFDevVideoPlay::TBufferOptions& aOptions )
	{
	PRINT_ENTRY;
	if ( !iState->IsInitialized() )
		{
		aOptions = iBufferOptions;
		}
	else
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Indicates which HRD/VBV specification is fulfilled in the input stream and
//any related parameters.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetHrdVbvSpec(
								THrdVbvSpecification /*aHrdVbvSpec*/,
								const TDesC8& /*aHrdVbvParams*/ )
	{
	PRINT_ENTRY;
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Sets the output post-processor device to use.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetOutputDevice(
										CMMFVideoPostProcHwDevice* aDevice )
	{
	PRINT_ENTRY;
	if ( !iState->IsInitialized() )
		{
		if ( !aDevice )
			{
			iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrArgument );
			}
		else
			{
			iOutputDevice = aDevice;

			// checks whether customBuffer supported by o/p device
            iCustomBufferHandle =
             ( MMmfVideoBufferManagement* )( iOutputDevice->CustomInterface(
										KMmfVideoBuffermanagementUid ) );
			}
		}
	else
		{
		PRINT_ERR( "istate is not initialized yet, "
				"calling MdvppFatalError on iMMFDevVideoPlayProxy\n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Returns the current decoding position, i.e.
//the timestamp for the most recently decoded picture.
//---------------------------------------------------------------------------
//
TTimeIntervalMicroSeconds CAriMpeg4aspdecHwDevice::DecodingPosition()
	{
	PRINT_ENTRY;
	if ( iState->IsInitialized() )
		{
		PRINT_EXIT;
		return iDecodingPosition;
		}
	else
		{
		PRINT_ERR( "iState is not initialised, calling  MdvppFatalError\n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		PRINT_EXIT;
		return ( TTimeIntervalMicroSeconds( 0 ) );
		}
	}

//---------------------------------------------------------------------------
//Returns the current pre-decoder buffer size.
//---------------------------------------------------------------------------
//
TUint CAriMpeg4aspdecHwDevice::PreDecoderBufferBytes()
	{
	PRINT_ENTRY;
    if ( !( iState->IsInitialized() ) )
        {
        iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
        PRINT_EXIT;
		return ( 0 );
        }

    PRINT_EXIT;
    return 0;
	}

//---------------------------------------------------------------------------
//Reads various counters related to the received input bitstream
//and coded data units.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::GetBitstreamCounters(
							CMMFDevVideoPlay::TBitstreamCounters& aCounters )
	{
	PRINT_ENTRY;
	if ( iState->IsInitialized() )
		{
		aCounters = iBitstreamCounters;
		//Reset the counters to Zero
		iBitstreamCounters.iLostPackets = 0;
		iBitstreamCounters.iTotalPackets = 0;
		}
	else
		{
		PRINT_ERR( "iState is not initialised, calling  MdvppFatalError\n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Retrieves the number of free input buffers the decoder has available
//---------------------------------------------------------------------------
//
TUint CAriMpeg4aspdecHwDevice::NumFreeBuffers()
	{
	PRINT_ENTRY;
	if ( iState->IsInitialized() )
		{
		if ( iNumberOfInputBuffersAllocated == 0 )
		    {
		    PRINT_EXIT;
		    return ( iBufferOptions.iMinNumInputBuffers );
		    }
		else
		    {
		    PRINT_EXIT;
		    return  ( iInputFreeBufferQueue.Count() );
		    }
		}
	else
		{
		PRINT_ERR( "iState is not initialised, calling  MdvppFatalError\n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		PRINT_EXIT;
		return( KErrNone );
		}
	}

//---------------------------------------------------------------------------
//Returns an input buffer of size aBufferSize. If no buffer is available
//then NULL is returned.
//---------------------------------------------------------------------------
//
TVideoInputBuffer* CAriMpeg4aspdecHwDevice::GetBufferL( TUint aBufferSize )
	{
	PRINT_ENTRY;
	if ( !( iState->IsInitialized() ) )
        {
        PRINT_ERR( "istate is not initialized yet\n" );
        User::Leave( KErrNotReady );
        }

    if ( iState->IsInputEndPending() )
	    {
	    PRINT_ERR( "input is pending\n" );
		User::Leave( KErrNotReady );
	    }

    if ( aBufferSize > iBufferOptions.iMaxInputBufferSize )
        {
        PRINT_ERR( "aBufferSize is more than the allowed size\n" );
    	PRINT_MSG( LEVEL_HIGH, ( "reqBufSize = %d, maxAllowedBufSize = %d",
    			aBufferSize, iBufferOptions.iMaxInputBufferSize ) );
        User::Leave( KErrArgument );
        }

	TVideoInputBuffer *inBuffer = NULL;

    if ( iNumberOfInputBuffersAllocated ==
								iBufferOptions.iMinNumInputBuffers )
        {
        if ( iInputFreeBufferQueue.Count() )
            {
            // Reallocate the input buffer if needed
            PRINT_MSG( LEVEL_HIGH, ( "Reallocate input buffer if needed" ) );
			CreateInputBufferL( aBufferSize, ETrue );
			inBuffer = iInputFreeBufferQueue[0];
			iInputFreeBufferQueue.Remove( 0 );
            }
        else
            {
            PRINT_ERR( "No input free buffers available \n" );
            return NULL;
            }
        }
    else if ( iNumberOfInputBuffersAllocated <
									iBufferOptions.iMinNumInputBuffers )
        {
        // Allocate new buffer of the requested size
        //PRINT_ERR( "CreateInputBufferL returned EFALSE\n" );
        CreateInputBufferL( aBufferSize, EFalse );

        // remove the last created buffer
        inBuffer = iInputFreeBufferQueue[ iInputFreeBufferQueue.Count() - 1];

        iInputFreeBufferQueue.Remove( iInputFreeBufferQueue.Count() - 1 );
        }
    else
        {
        PRINT_ERR( "iNumberOfInputBuffersAllocated is less "
        		"			iBufferOptions.iMinNumInputBuffers\n" );
        User::Leave( KErrGeneral );
        }

    PRINT_EXIT;
	return inBuffer;
	}

//---------------------------------------------------------------------------
//Called by DevVideoPlay to write the coded data.
//Writes a piece of coded video data to the decoder. The data buffer must
//be retrieved from the decoder with GetBufferL()..
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::WriteCodedDataL( TVideoInputBuffer* aBuffer )
	{
	PRINT_ENTRY;
	if ( !iState->IsInitialized() )
		{
		PRINT_ERR( "istate is not initialized yet \n" );
		User::Leave( KErrNotReady );
		}

    if ( !aBuffer )
		{
		PRINT_ERR( "aBuffer is null \n" );
		User::Leave( KErrArgument );
		}

    if ( aBuffer->iData.Length() == 0 )
	    {
	    PRINT_ERR( "length of aBuffer is zero \n" );
		User::Leave( KErrArgument );
	    }

	if ( iState->IsInputEndPending() )
		{
		PRINT_ERR( "IsInputEndPending is true \n" );
		User::Leave( KErrEof );
		}

	if ( ( iDataUnitType == EDuSeveralSegments ) ||
			( iDataUnitType == EDuVideoSegment ) )
	    {
		iBitstreamCounters.iTotalPackets++;
	    }

	// if custom interface is supported by o/p device get all the buffers
	// and add it to Engine
	if ( !iOutputBuffersCreated )
		{
		// Set Stream information
		if ( !iDecoderConfigured )
			{
    		iCodec->ConfigDecoderL( &( aBuffer->iData ) );
			// set input & output formats to Codec
			iCodec->SetFormatsL( iDataUnitType, iOutputFormat );
            iDecoderConfigured = ETrue;
			}

    	if ( !iCustomBufferHandle )
    		{
			// Get o/p buffer length for creation
			iOutputBufferSize = iCodec->GetOutputBufferLength();
			CreateOutputBuffersL();
    		}
		iOutputBuffersCreated = ETrue;
		}

	// add all the o/p buffers to engine
	if ( !iBufferAdded )
		{
		if ( iCustomBufferHandle )
			{
			TVideoPicture* videoPicture = NULL;
			for ( TInt i = 0;
				  i < TInt( iCustomBufferOptions->iNumInputBuffers );
				  i++ )
				{
				videoPicture = iCustomBufferHandle->MmvbmGetBufferL(
										iCustomBufferOptions->iBufferSize );
				if ( videoPicture )
					{
					TRAPD( lErr, videoPicture->iHeader =
									new ( ELeave ) TVideoPictureHeader ) ;
					if ( lErr != KErrNone )
                    	{
                    	iCustomBufferHandle->MmvbmReleaseBuffer(
															  videoPicture );
                    	iMMFDevVideoPlayProxy->MdvppFatalError( this,
															  KErrArgument );
                    	}
                    else
                    	{
					    iEngine->AddOutput( videoPicture );
                    	}
					}
				else
					{
					break;
					}
				}
			}
		else
			{
			for ( TInt i = 0; i < iOutputFreeBufferQueue.Count(); i++ )
				{
				iEngine->AddOutput( iOutputFreeBufferQueue[0] );
				iOutputFreeBufferQueue.Remove( 0 );
				}
			}
		iBufferAdded = ETrue;
		}

	iPictureCounters.iTotalPictures++;
	iFilledBufferCounter++;
	TInt err = iEngine->AddInput( aBuffer );
	PRINT_MSG( LEVEL_LOW, ( "WriteCodedDataL, addinput err = %d", err ) );
	PRINT_EXIT;
	}


#ifdef SYMBIAN_MDF_API_V2
//---------------------------------------------------------------------------
//Configures the Mpeg4/H263 decoder by passing the required header
//information for the stream that is getting decoded. The header
//structurespecific  to the Decoder is passed as iOptional member.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::ConfigureDecoderL(
							const TVideoPictureHeader& aVideoPictureHeader )
    {
    PRINT_ENTRY;
	if ( iState->IsInitialized() )
		{
		PRINT_ERR( "iState is already initialised \n" );
		User::Leave( KErrNotReady );
		}

    TInt level = aVideoPictureHeader.iLevel;
    PRINT_MSG( LEVEL_LOW, ( "ConfigureDecoderL,level = %d \n", level ) );
    if ( aVideoPictureHeader.iOptional )
        {
        // Mpeg-4
        switch ( level )
            {
			case 0:
			case KMP4Level11: //level 0b
			case KMP4Level1:
			case KMP4Level2:
			case KMP4Level3:
			case KMP4Level4:
			case KMP4Level5:
			case -1:
				break;
			default:
				{
				PRINT_ERR( "level is not supported \n" );
				User::Leave( KErrNotSupported );
				}
            }
        }
    else
        {
        // H.263
        switch ( level )
            {
			case KH263Level10:
			case KH263Level20:
			case KH263Level30:
			case KH263Level45:
			case -1:
				break;
			default:
				{
				PRINT_ERR( "level is not supported \n" );
				User::Leave( KErrNotSupported );
				}
            }
        }

	// Create Codec and configure the decoder
	iCodec = CAriMpeg4aspdecWrapper::NewL( this );

	// configure the decoder in case of mpeg-4 otherwise ie in
	// case of h.263 wait till the first call of WriteCodedDataL()
    if ( aVideoPictureHeader.iOptional )
        {
	    iCodec->ConfigDecoderL( aVideoPictureHeader.iOptional );
    	iDecoderConfigured = ETrue;
        }
    iConfigureDecoderCalled = ETrue;

    PRINT_EXIT;
    }
#endif

//---------------------------------------------------------------------------
//Retrieves post-processing information about this hardware device.
//The device creates a CPostProcessorInfo structure, fills it with correct
//data, pushes it to the cleanup stack and returns it. The client will
//delete the object when it is no longer needed.
//---------------------------------------------------------------------------
//
CPostProcessorInfo* CAriMpeg4aspdecHwDevice::PostProcessorInfoLC()
	{
	PRINT_ENTRY;
	//Aricent decoder does not support any post processing functionality
	RArray<TUncompressedVideoFormat> supportedFormats;
	CleanupClosePushL( supportedFormats );

	RArray<TUint32> supportedCombinations;
	CleanupClosePushL( supportedCombinations );

	TYuvToRgbCapabilities yuvToRgbCapabilities;
	RArray<TScaleFactor> supportedScaleFactors;
	CleanupClosePushL( supportedScaleFactors );

	yuvToRgbCapabilities.iSamplingPatterns = 0;
	yuvToRgbCapabilities.iCoefficients = 0;
	yuvToRgbCapabilities.iRgbFormats = 0;
	yuvToRgbCapabilities.iLightnessControl = EFalse;
	yuvToRgbCapabilities.iSaturationControl = EFalse;
	yuvToRgbCapabilities.iContrastControl = EFalse;
	yuvToRgbCapabilities.iGammaCorrection = EFalse;
	yuvToRgbCapabilities.iDitherTypes = 0;


	CPostProcessorInfo* info = CPostProcessorInfo::NewL(
		KUidMp4H263DecoderHwDevice,
		KManufacturer,
		KIdentifier,
		TVersion( 1, 0, 0 ),
		supportedFormats.Array(),
		supportedCombinations.Array(),
		//Accelerated or not
		EFalse,
		//supports DSA
		EFalse,
		yuvToRgbCapabilities,
		//supported rotations
		ERotateNone,
		//supports Arbitrary scaling
		EFalse,
		supportedScaleFactors.Array(),
		//supports antialiased scaling
		EFalse );

	CleanupStack::PushL( info );
	CleanupStack::Pop( &supportedScaleFactors );
	supportedScaleFactors.Close();
	CleanupStack::Pop( &supportedCombinations );
	supportedCombinations.Close();
	CleanupStack::Pop( &supportedFormats );
	supportedFormats.Close();

	PRINT_EXIT;
	return info;
	}

//---------------------------------------------------------------------------
//Retrieves the list of the output formats that the device supports.
//The list can depend on the device source format, and therefore
//SetSourceFormatL() must be called before calling this method.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::GetOutputFormatListL(
								RArray<TUncompressedVideoFormat>& aFormats )
	{
	PRINT_ENTRY;
    if ( iState->IsInitialized() )
    	{
    	PRINT_ERR( "iState is already initialised \n" );
        User::Leave( KErrNotReady );
    	}

    if ( !iInputFormat )
	    {
		//Input format should be set before calling this function, as output
        // format is decided based on input format
	    PRINT_ERR( "iInputFormat is not set yet \n" );
		User::Leave( KErrNotReady );
	    }

	TUncompressedVideoFormat format;
	format.iDataFormat = EYuvRawData;
	format.iYuvFormat.iYuv2RgbMatrix = NULL;
	format.iYuvFormat.iRgb2YuvMatrix = NULL;
	format.iYuvFormat.iAspectRatioNum = 1;
	format.iYuvFormat.iAspectRatioDenom = 1;

	aFormats.Reset();

    //The following formats are supported by both H.263 and Mpeg-4

	//YUV 420 Chroma0 Planar
	format.iYuvFormat.iCoefficients = EYuvBt601Range0;
	format.iYuvFormat.iPattern      = EYuv420Chroma1;
	format.iYuvFormat.iDataLayout   = EYuvDataPlanar;
	aFormats.Append( format );

    //YUV 422 Chroma LE Interleaved
    format.iYuvFormat.iPattern      = EYuv422Chroma1;
	format.iYuvFormat.iDataLayout   = EYuvDataInterleavedLE;
    aFormats.Append( format );

    //YUV 422 Chroma BE Interleaved
    format.iYuvFormat.iDataLayout   = EYuvDataInterleavedBE;
    aFormats.Append( format );

    //The following formats are only supported by Mpeg-4
    if ( iInputFormat->MimeType().FindF( KMPEG4SupportedMimeType ) !=
															   KErrNotFound )
        {
        //BT 601.5 Full Range
        // YUV 420 Chroma0 Planar
        format.iYuvFormat.iCoefficients = EYuvBt601Range1;
	    format.iYuvFormat.iPattern      = EYuv420Chroma1;
	    format.iYuvFormat.iDataLayout   = EYuvDataPlanar;
        aFormats.Append( format );

        //YUV 422 Chroma LE Interleaved
        format.iYuvFormat.iPattern      = EYuv422Chroma1;
	    format.iYuvFormat.iDataLayout   = EYuvDataInterleavedLE;
        aFormats.Append( format );

        //YUV 422 Chroma BE Interleaved
        format.iYuvFormat.iDataLayout   = EYuvDataInterleavedBE;
        aFormats.Append( format );

        //BT 709 Reduced Range
	    // YUV 420 Chroma0 Planar
	    format.iYuvFormat.iCoefficients = EYuvBt709Range0;
	    format.iYuvFormat.iPattern      = EYuv420Chroma1;
	    format.iYuvFormat.iDataLayout   = EYuvDataPlanar;
	    aFormats.Append( format );

        //YUV 422 Chroma LE Interleaved
        format.iYuvFormat.iPattern      = EYuv422Chroma1;
	    format.iYuvFormat.iDataLayout   = EYuvDataInterleavedLE;
        aFormats.Append( format );

        //YUV 422 Chroma BE Interleaved
        format.iYuvFormat.iDataLayout   = EYuvDataInterleavedBE;
        aFormats.Append( format );

        //BT 709 Full Range
        // YUV 420 Chroma0 Planar
	    format.iYuvFormat.iCoefficients = EYuvBt709Range1;
	    format.iYuvFormat.iPattern      = EYuv420Chroma1;
	    format.iYuvFormat.iDataLayout   = EYuvDataPlanar;
	    aFormats.Append( format );

        //YUV 422 Chroma LE Interleaved
        format.iYuvFormat.iPattern      = EYuv422Chroma1;
	    format.iYuvFormat.iDataLayout   = EYuvDataInterleavedLE;
        aFormats.Append( format );

        //YUV 422 Chroma BE Interleaved
        format.iYuvFormat.iDataLayout   = EYuvDataInterleavedBE;
        aFormats.Append( format );
	    }
    PRINT_EXIT;
    }

//---------------------------------------------------------------------------
// Sets the device output format.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetOutputFormatL(
									const TUncompressedVideoFormat &aFormat )
	{
	PRINT_ENTRY;
    if ( iState->IsInitialized() )
    	{
    	PRINT_ERR( "istate is already initialised \n" );
        User::Leave( KErrNotReady );
    	}

    if ( !iInputFormat )
	    {
		//Input format should be set before calling this function, as output
        // format is decided based on input format
	    PRINT_ERR( "iInputFormat is not set yet \n" );
		User::Leave( KErrNotReady );
	    }

	// check whether output format is supported or not.if not supported leave
	RArray<TUncompressedVideoFormat> formats;
	CleanupClosePushL( formats );
	GetOutputFormatListL( formats );

	TBool flag = EFalse;

	for ( TInt i = 0; i < formats.Count(); i++ )
		{
		if ( aFormat == formats[i] )
			{
			flag = ETrue;
			break;
			}
		}
	CleanupStack::Pop ( &formats );
    formats.Close();

	if ( !flag )
		{
		PRINT_ERR( "outputformat is not supported \n" );
        User::Leave( KErrNotSupported );
		}

    iOutputFormat = aFormat;
    PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the clock source to use for video timing.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetClockSource( MMMFClockSource* /*aClock*/ )
	{
	PRINT_ENTRY;
    if ( iState->IsInitialized() )
    	{
    	PRINT_ERR( "iState is initialised,calling MdvppFatalError \n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
    	}

    PRINT_EXIT;
	return;
	}

//---------------------------------------------------------------------------
// Sets the device video output destination.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetVideoDestScreenL( TBool aScreen )
	{
	PRINT_ENTRY;
	if ( !iState->IsInitialized() )
		{
		if ( aScreen )
			{
			PRINT_ERR( "KErrNotSupported \n" );
			User::Leave( KErrNotSupported );
			}
		}
	else
		{
		PRINT_ERR( "KErrNotSupported \n" );
		User::Leave( KErrNotReady );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Sets the post-processing types to be used.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetPostProcessTypesL(
										TUint32 /*aPostProcCombination*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "KErrNotSupported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets post-processing options for input (pan-scan ) cropping.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetInputCropOptionsL( const TRect& /*aRect*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "KErrNotSupported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets post-processing options for YUV to RGB color space conversion
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetYuvToRgbOptionsL(
								    const TYuvToRgbOptions& /*aOptions*/,
									const TYuvFormat& /*aYuvFormat*/,
									TRgbFormat /*aRgbFormat*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "KErrNotSupported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets post-processing options for YUV to RGB color space conversion. Uses
//the device input and output formats.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetYuvToRgbOptionsL(
									const TYuvToRgbOptions& /*aOptions*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "KErrNotSupported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
// Sets post-processing options for rotation. SetPostProcessTypesL() must be
//called before this method is used.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetRotateOptionsL(
											TRotationType /*aRotationType*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "KErrNotSupported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Sets post-processing options for scaling. SetPostProcessTypesL() must be
//called before this method is used.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetScaleOptionsL( const TSize& /*aTargetSize*/,
											 TBool /*aAntiAliasFiltering*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "KErrNotSupported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Sets post-processing options for output cropping.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetOutputCropOptionsL( const TRect& /*aRect*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "KErrNotSupported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Sets post-processing plug-in specific options.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetPostProcSpecificOptionsL(
												const TDesC8& /*aOptions*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "KErrNotSupported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Initializes the device. This method is asynchronous, the device calls
//MdvppInitializeComplete() of MMFVideoPlayProxy after initialization has
//completed. After this method has successfully completed, further
//configuration changes are not possible except where separately noted.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::Initialize()
	{
	PRINT_ENTRY;
    if ( !iState->IsTransitionValid( CStateMachine::EInitializeCommand ) )
		{
		PRINT_ERR( "state is not valid in statemachine \n" );
		iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, KErrNotReady );
        return;
		}

    TRAPD( err, iInputFreeBuffers	=
	  new ( ELeave ) TVideoInputBuffer[iBufferOptions.iMinNumInputBuffers] );
    if ( err )
        {
        iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
        PRINT_ERR( "creation of iInputFreeBuffers returned error\n" );
		return;
		}

	iPictureCounters.iPicturesDisplayed = 0;
	iPictureCounters.iPicturesSkipped = 0;
	iPictureCounters.iTotalPictures = 0;
	iPictureCounters.iPicturesDecoded = 0;

	iBitstreamCounters.iLostPackets = 0;
	iBitstreamCounters.iTotalPackets = 0;

	// if custom interface is supported then enable the interface
	if ( iCustomBufferHandle )
		{

		TRAP( err, iCustomBufferOptions =
		          new ( ELeave ) MMmfVideoBufferManagement::TBufferOptions );

		if ( err != KErrNone )
			{
			iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
			PRINT_ERR( "creation of iCustomBuffeOptions returned error \n" );
			return;
			}

		iCustomBufferOptions->iNumInputBuffers = KMaxOutputBuffers + 1;
		iCustomBufferOptions->iBufferSize = TSize( KMaxFrameWidth,
												   KMaxFrameHeight );

		iCustomBufferHandle->MmvbmSetObserver(
							( MMmfVideoBufferManagementObserver * )this );

		iCustomBufferHandle->MmvbmEnable( ETrue );

		TRAP( err,
		iCustomBufferHandle->MmvbmSetBufferOptionsL(
											       *iCustomBufferOptions ) );

		if ( err != KErrNone )
			{
			PRINT_ERR( "iCustomBufferHandle->MmvbmSetBufferOptionsL \n" );
			iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
			return;
			}
		}

	if ( !iConfigureDecoderCalled )
		{
        TRAP( err, iCodec = CAriMpeg4aspdecWrapper::NewL( this ) );
        if ( err != KErrNone )
            {
		    iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
		    PRINT_ERR( "CCodecInterface::NewL returned error \n" );
		    return;
            }
		}

	// Engine Creation
	TRAP( err, iEngine = CBaseEngine::NewL(
								     this, ( MBaseCodec* )iCodec, EFalse ) );
	if ( err != KErrNone )
		{
		iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
		PRINT_ERR( "CBaseEngine::NewL returned error \n" );
		return;
		}

	// set input & output formats to Codec
	if ( iConfigureDecoderCalled )
		{
		TRAP( err, iCodec->SetFormatsL(  iDataUnitType, iOutputFormat ) );
        if ( err != KErrNone )
            {
		    iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
		    PRINT_ERR( "iCodec->SetFormatsL returned error \n" );
		    return;
            }
		}

	err = iState->Transit( CStateMachine::EInitializeCommand );
	if ( err )
		{
		iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
		PRINT_ERR( "state is not set to initialised \n" );
		return;
		}

	iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, KErrNone );

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Commit all changes since the last CommitL(), Revert() or Initialize()
//to the hardware device. This only applies to methods which can be called
//both before AND after DevVideoPlay has been initialized.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::CommitL()
	{
	PRINT_ENTRY;
    if ( !( iState->IsInitialized() ) )
	    {
	    PRINT_ERR( "istate is not initialised yet \n" );
		User::Leave( KErrNotReady );
	    }
	PRINT_EXIT;
    return;
	}

//---------------------------------------------------------------------------
//Revert all changes since the last CommitL(), Revert() or Initialize() back
//to their previous settings. This only applies to methods which can be
//called both before AND after DevVideoPlay has been initialized..
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::Revert()
	{
	PRINT_ENTRY;
    if ( !( iState->IsInitialized() ) )
	    {
	    PRINT_ERR( "istate is not initialised yet \n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	    }
    PRINT_EXIT;
    return;
	}

//---------------------------------------------------------------------------
//Not Supported.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::StartDirectScreenAccessL(
										const TRect& /*aVideoRect*/,
										CFbsScreenDevice& /*aScreenDevice*/,
										const TRegion& /*aClipRegion*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "Not supported API \n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Not Supported.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetScreenClipRegion( const TRegion& /*aRegion*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "Not supported API \n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	return;
	}

//---------------------------------------------------------------------------
//Not Supported.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetPauseOnClipFail( TBool /*aPause*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "Not supported API \n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	return;
	}

//---------------------------------------------------------------------------
//Not Supported.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::AbortDirectScreenAccess()
	{
	PRINT_ENTRY;
	PRINT_ERR( "Not supported API \n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	return;
	}

//---------------------------------------------------------------------------
//Indicates whether playback is proceeding. This method can be used to check
//whether playback was paused or not in response to a new clipping region
//or DSA abort
//---------------------------------------------------------------------------
//
TBool CAriMpeg4aspdecHwDevice::IsPlaying()
	{
	PRINT_ENTRY;
	if ( !( iState->IsInitialized() ) )
		{
		 PRINT_ERR( "istate is not initialised yet \n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}
	PRINT_EXIT;
	return ( iState->IsStarted() );
	}

//---------------------------------------------------------------------------
//Not Supported.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::Redraw()
	{
	PRINT_ENTRY;
	PRINT_ERR( "Not supported API \n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	return;
	}

//---------------------------------------------------------------------------
//Starts video playback, including decoding, post-processing, and rendering.
//Playback will proceed until it has been stopped or paused, or the end of
//the bitstream is reached.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::Start()
	{
	PRINT_ENTRY;
    if ( iState->IsPlaying() )
		{
		PRINT_EXIT;
		PRINT_ERR( "already in playing state \n" );
		return;
		}
	if ( !iState->IsTransitionValid( CStateMachine::EStartCommand ) )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	iDecodingPosition = TTimeIntervalMicroSeconds( 0 );
	iEngine->Start();

	TInt error = iState->Transit( CStateMachine::EStartCommand );
	if ( error != KErrNone )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Stops video playback. No new pictures will be decoded, post-processed,
//or rendered.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::Stop()
	{
	PRINT_ENTRY;
    if ( iState->IsStopped() )
        {
        PRINT_EXIT;
        PRINT_ERR( "already in stopped state \n" );
        return;
		}
	if ( !iState->IsTransitionValid( CStateMachine::EStopCommand ) )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	iPictureNumber = 0;
	// Stop & Reset the Engine
	iEngine->Stop();
	iEngine->Reset();
	iBufferAdded			= EFalse;
	iFilledBufferCounter	= 0;
	iInputEndCalled			= EFalse;
	iNoLastOut				= EFalse;
	iInputBufferConsumedCounter = 0;
	TInt error = iState->Transit( CStateMachine::EStopCommand );
	if ( error != KErrNone )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Pauses video playback, including decoding, post-processing, and rendering.
//No pictures will be decoded, post-processed, or rendered until playback has
//been resumed.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::Pause()
	{
	PRINT_ENTRY;
    if ( iState->IsPaused() )
        {
        PRINT_EXIT;
        PRINT_ERR( "already in paused state \n" );
        return;
		}

	if ( !iState->IsTransitionValid( CStateMachine::EPauseCommand ) )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	//Stop the Engine
	iEngine->Stop();

	TInt error = iState->Transit( CStateMachine::EPauseCommand );
	if ( error != KErrNone )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Resumes video playback after a pause.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::Resume()
	{
	PRINT_ENTRY;
    if ( iState->IsPlaying() )
        {
        PRINT_EXIT;
        PRINT_ERR( "already in playing state \n" );
        return;
		}

	if ( !iState->IsTransitionValid( CStateMachine::EResumeCommand ) )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	// Start The Engine
	iEngine->Start();

	TInt error = iState->Transit( CStateMachine::EResumeCommand );
	if ( error != KErrNone )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Changes to a new decoding and playback position,used for randomly accessing
//(seeking)the input stream. The position change flushes all input and output
//buffers. Pre-decoder and post-decoder buffering are handled as if a new
//bitstream was being decoded. If the device still has buffered pictures that
//precede the new playback position, they will be discarded. If playback is
//synchronized to a clock source, the client is responsible for setting the
//clock source to the new position..
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetPosition(
						const TTimeIntervalMicroSeconds& aPlaybackPosition )
	{
	PRINT_ENTRY;
	//Start Decoding from new position
	if ( iState->IsInitialized() )
		{
		iEngine->Stop();
		iEngine->Reset();
        iDecodingPosition = aPlaybackPosition;
		iBufferAdded = EFalse;
		iEngine->Start();
		}
	else
		{
		PRINT_ERR( "istate is not initialised yet\n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Not Supported.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::FreezePicture(
							const TTimeIntervalMicroSeconds& /*aTimestamp*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API \n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	return;
	}

//---------------------------------------------------------------------------
//Not Supported.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::ReleaseFreeze(
							const TTimeIntervalMicroSeconds& /*aTimestamp*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API \n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	return;
	}

//---------------------------------------------------------------------------
//Returns the current playback position, i.e. the timestamp for the most
//recently displayed or virtually displayed picture. If the device output
//is written to another device, the most recent output picture is used
//---------------------------------------------------------------------------
//
TTimeIntervalMicroSeconds CAriMpeg4aspdecHwDevice::PlaybackPosition()
	{
	PRINT_ENTRY;
	// In Decoder case decoding position is same as playback position
	if ( !( iState->IsInitialized() ) )
		{
		PRINT_ERR( "iState is not initialised \n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}
	PRINT_EXIT;
	return iDecodingPosition;
	}

//---------------------------------------------------------------------------
//Returns the total amount of memory allocated for uncompressed pictures.
//---------------------------------------------------------------------------
//
TUint CAriMpeg4aspdecHwDevice::PictureBufferBytes()
	{
	PRINT_ENTRY;
	if ( !( iState->IsInitialized() ) )
		{
		PRINT_ERR( "iState is not initialised yet \n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}
	PRINT_EXIT;
	return( KMaxOutputBuffers * iOutputBufferSize );
	}

//---------------------------------------------------------------------------
//Reads various counters related to decoded pictures.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::GetPictureCounters(
							CMMFDevVideoPlay::TPictureCounters& aCounters )
	{
	PRINT_ENTRY;
    if ( !( iState->IsInitialized() ) )
    	{
		PRINT_ERR( "iState is not initialised yet\n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
    	}

	aCounters = iPictureCounters;

	//Reset the counters to Zero
	iPictureCounters.iPicturesDisplayed = 0;
	iPictureCounters.iPicturesSkipped = 0;
	iPictureCounters.iTotalPictures = 0;
	iPictureCounters.iPicturesDecoded = 0;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetComplexityLevel( TUint /*aLevel*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
TUint CAriMpeg4aspdecHwDevice::NumComplexityLevels()
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	return 0;
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::GetComplexityLevelInfo( TUint /*aLevel*/,
						CMMFDevVideoPlay::TComplexityLevelInfo& /*aInfo*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Called by the Post Proc hwdevice when a picture is displayed on the screen.
//Returns a picture back to the device.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::ReturnPicture( TVideoPicture* aPicture )
	{
	PRINT_ENTRY;
    if ( !( iState->IsInitialized() ) )
    	{
    	PRINT_ERR( "iState is not initialised yet\n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
    	}

    if ( iCustomBufferHandle )
		{
	    delete aPicture->iHeader ;
	    aPicture->iHeader = NULL ;
		}
	else
		{
		// add to engine if state is not stopped
		if ( iState->IsStopped() )
			{
			iOutputFreeBufferQueue.Append( aPicture );
			}
		else
			{
			iEngine->AddOutput( aPicture );
			}
        }
    PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
TBool CAriMpeg4aspdecHwDevice::GetSnapshotL( TPictureData& /*aPictureData*/,
								const TUncompressedVideoFormat& /*aFormat*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	return( KErrNone );
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::GetTimedSnapshotL( TPictureData* /*aPictureData*/,
				const TUncompressedVideoFormat& /*aFormat*/,
				const TTimeIntervalMicroSeconds& /*aPresentationTimestamp*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	PRINT_EXIT;
	//Should take it from Post Proc HwDevice
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::GetTimedSnapshotL( TPictureData* /*aPictureData*/,
								const TUncompressedVideoFormat& /*aFormat*/,
								const TPictureId& /*aPictureId*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	PRINT_EXIT;
	//Should take it from Post Proc HwDevice
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::CancelTimedSnapshot()
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	PRINT_EXIT;
	return;
	}


//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::GetSupportedSnapshotFormatsL(
							RArray<TUncompressedVideoFormat>& /*aFormats*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//sets the Flag iInputEndCalled to ETrue
//Notifies the hardware device that the end of input data has been reached
//and no more input data will be written
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::InputEnd()
	{
	PRINT_ENTRY;
	if ( !( iState->IsTransitionValid( CStateMachine::EInputEndCommand ) ) )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	iInputEndCalled = ETrue;

	if ( ( iFilledBufferCounter - iInputBufferConsumedCounter ) == 1 )
		{
		GetLastFrame();
		Stop();
		iMMFDevVideoPlayProxy->MdvppStreamEnd();
		PRINT_EXIT;
        return;
		}

	TInt error = iState->Transit( CStateMachine::EInputEndCommand );

	if ( error != KErrNone )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Notifies the Hw Device that input buffer is free
//---------------------------------------------------------------------------
//
TInt CAriMpeg4aspdecHwDevice::InputBufferConsumed ( TAny* aInp, TInt aError )
	{
	PRINT_ENTRY;

	if ( aInp )
		{
		 iInputFreeBufferQueue.Append( REINTERPRET_CAST( TVideoInputBuffer*,
														 aInp ) );

		 //call back to clinet
		 if ( ( aError != KErrCancel ) && ( !iInputEndCalled ) )
			iMMFDevVideoPlayProxy->MdvppNewBuffers();
		}
	if ( iInputEndCalled && iNoLastOut )
		{
		HandleInputEndInStopping();
		iMMFDevVideoPlayProxy->MdvppStreamEnd();
		}

	PRINT_EXIT;
	return ( KErrNone );
	}

//---------------------------------------------------------------------------
//Notifies the Hw Device that out buffer has decoded data
//---------------------------------------------------------------------------
//
TInt CAriMpeg4aspdecHwDevice::OutputBufferReady ( TAny* aOut, TInt aError )
	{
	PRINT_ENTRY;
    if ( aError == KErrNone )
        {
        iPictureNumber++;
        // call back information regarding slice and picture loss

        SliceAndPictureLoss();

        //Decoded Buffer is available
        TVideoPicture* videoPicture = REINTERPRET_CAST( TVideoPicture*,
														aOut );
        
        videoPicture->iHeader->iPictureNumber = iPictureNumber;
        
        iFilledBufferCounter--;

        iPictureCounters.iPicturesDecoded++;
        iPictureCounters.iPicturesDisplayed++;

        iDecodingPosition = videoPicture->iTimestamp;

		if ( !iOutputDevice )
			{
			iMMFDevVideoPlayProxy->MdvppNewPicture( videoPicture );

			if ( iInputEndCalled &&
				 ( ( iFilledBufferCounter - iInputBufferConsumedCounter )
					== 1 )
				)
				{
				GetLastFrame();
				Stop();
				PRINT_ERR( "calling streamend\n" );
				iMMFDevVideoPlayProxy->MdvppStreamEnd();
				}
			PRINT_EXIT;
			return( KErrNone );
			}
		else
			{
			TRAPD( error, iOutputDevice->WritePictureL( videoPicture ) );

			if ( error == KErrNone )
				{
				if ( iInputEndCalled &&
					 ( ( iFilledBufferCounter - iInputBufferConsumedCounter )
							 == 1 ) )
					{
					GetLastFrame();
					HandleInputEndInStopping();
					PRINT_ERR( "calling streamend\n" );
					iMMFDevVideoPlayProxy->MdvppStreamEnd();
					}
				}
	            else if ( error ) // KErrArgument, KErrNotReady
	               {
	                iPictureCounters.iPicturesDisplayed--;
	                // Currently not handled
	               }
            }
        }

    else if ( aError == KErrCancel )
		{
		 // Add buffers to output free buffer queue if cutombuffer is not set
		if ( !iCustomBufferHandle )
			{
			iOutputFreeBufferQueue.Append( REINTERPRET_CAST( TVideoPicture*,
																	aOut ) );
			}
		else
			{

			delete ( ( TVideoPicture* )aOut )->iHeader;
			( ( TVideoPicture* )aOut )->iHeader = NULL;

			// release the buffer
			iCustomBufferHandle->MmvbmReleaseBuffer( REINTERPRET_CAST(
												    TVideoPicture*, aOut ) );
			}

		}

    else // other errors
        {
        // currently not handled
        }

    PRINT_EXIT;
	return ( KErrNone );
	}

//---------------------------------------------------------------------------
//Notifies the hw devcie that Error has occured in PE
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::FatalErrorFromProcessEngine ( TInt aError )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	iMMFDevVideoPlayProxy->MdvppFatalError( this, aError );
	PRINT_EXIT;
	}
//---------------------------------------------------------------------------
//Callback to indicate the command has been processed
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::CommandProcessed ( TInt aCmd, TAny* aCmdData,
												TInt aError )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Call back from output device, indicates buffers availability
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::MmvbmoNewBuffers()
	{
	PRINT_ENTRY;
	TVideoPicture* videoPicture = NULL;

	TRAPD( err, videoPicture = iCustomBufferHandle->MmvbmGetBufferL(
									iCustomBufferOptions->iBufferSize ) );

	if ( err != KErrNone )
		{
		PRINT_ERR( "iCustomBufferHandle->MmvbmGetBufferL returned error\n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrArgument );
		}
	if (  videoPicture )
		{
		if ( videoPicture->iHeader )
			{
			delete videoPicture->iHeader;
			videoPicture->iHeader = NULL;
			}

        TRAPD(  err1, videoPicture->iHeader =
									   new ( ELeave ) TVideoPictureHeader ) ;

        if ( err1 != KErrNone )
        	{
        	iCustomBufferHandle->MmvbmReleaseBuffer( videoPicture ) ;
        	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrArgument );
        	}
        else
        	{
		    iEngine->AddOutput( videoPicture );
        	}
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Callback from output device.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::MmvbmoReleaseBuffers()
	{
	PRINT_ENTRY;
	iEngine->Stop();
	iEngine->Reset();
	PRINT_EXIT;
	}



//---------------------------------------------------------------------------
//Call back from the codec wrapper when there is new config information.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::NewConfigFound()
	{
	PRINT_ENTRY;

    /* Reset the all the output buffers added to the ProcessEngine.
	 * Create them again if custom buffers are not used.
	 */
	while ( iEngine->NumOutputBuffers() )
		  OutputBufferReady ( iEngine->FetchOutputBuffer(), KErrCancel );

	iOutputBuffersCreated = EFalse;
	iBufferAdded          = EFalse;

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Call back from the codec wrapper when EInputConsumed for Not yet get I frame
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::EInputConsumed()
	{
	PRINT_ENTRY;

	iInputBufferConsumedCounter++;

	if ( iInputEndCalled && ( iFilledBufferCounter ==
							  ( iInputBufferConsumedCounter + 1 ) ) )
		{
		iNoLastOut = ETrue;
		}

	PRINT_EXIT;
	}
//---------------------------------------------------------------------------
//Set the proxy implementation to be used. Called just
//after the object is constructed
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SetProxy( MMMFDevVideoPlayProxy& aProxy )
	{
	PRINT_ENTRY;
	iMMFDevVideoPlayProxy = &aProxy;
	PRINT_EXIT;
	}


//---------------------------------------------------------------------------
//Default constructor of CAriMpeg4aspdecHwDevice
//---------------------------------------------------------------------------
//
CAriMpeg4aspdecHwDevice::CAriMpeg4aspdecHwDevice()
    :iInputFreeBuffers( NULL ),
     iOutputDevice( NULL ),
     iMMFDevVideoPlayProxy( NULL ),
	 iState( NULL ),
     iVideoPictureHeader( NULL ),
	 iDecodingPosition( TTimeIntervalMicroSeconds( 0 ) ),
     iInputFormat( NULL ),
     iCodec( NULL ),
     iEngine( NULL ),
     iInputEndCalled( EFalse ),
     iOutputFreeBuffer( NULL ),
     iFilledBufferCounter( 0 ),
     iDataUnitType( EDuCodedPicture ),
     iEncapsulation( EDuElementaryStream ),
     iOutputBufferSize( 0 ),
     iOutputBuffersCreated( EFalse ),
     iPictureNumber( 0 ),
     iNumberOfInputBuffersAllocated( 0 ),
	 iCustomBufferHandle( NULL ),
	 iCustomBufferOptions( NULL ),
	 iBufferAdded( EFalse ),
     iConfigureDecoderCalled( EFalse ),
	 iDecoderConfigured( EFalse ),
	 iInputBufferConsumedCounter( 0 ),
	 iNoLastOut( EFalse )
    {
    PRINT_ENTRY;
	iBufferOptions.iPreDecodeBufferSize 		= 0;
	iBufferOptions.iMaxPostDecodeBufferSize   	= 0;
	iBufferOptions.iPreDecoderBufferPeriod 		= 0;
	iBufferOptions.iPostDecoderBufferPeriod 	= 0;
	iBufferOptions.iMinNumInputBuffers 			= KMaxInputBuffers;
	iBufferOptions.iMaxInputBufferSize 			= KMaxInputBufferSize;


    // Initializing iOutputFormat with one of the combination supported by
    // Mpeg-4/H.263 hwdevice

	iOutputFormat.iDataFormat					= EYuvRawData;
    // For H.263 it is always Bt 601.5 reduced range
    // For Mpeg-4 input stream contains this information
	iOutputFormat.iYuvFormat.iCoefficients		= EYuvBt601Range0;
    // The input stream format ( H.263 & Mpeg-4 ) comes in Chroma1 format and
	//so output is also decoded into Chroma1 format
	iOutputFormat.iYuvFormat.iPattern			= EYuv420Chroma1;
	iOutputFormat.iYuvFormat.iDataLayout		= EYuvDataPlanar;
	iOutputFormat.iYuvFormat.iYuv2RgbMatrix		= NULL;
	iOutputFormat.iYuvFormat.iRgb2YuvMatrix		= NULL;
	iOutputFormat.iYuvFormat.iAspectRatioNum	= 1;
	iOutputFormat.iYuvFormat.iAspectRatioDenom	= 1;
	PRINT_EXIT;
    }


//---------------------------------------------------------------------------
//This is the 2nd phase constructor
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::ConstructL()
	{
	PRINT_ENTRY;
	iState = CStateMachine::NewL();
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Module to create output data
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::CreateOutputBuffersL()
	{
	PRINT_ENTRY;
	if ( iOutputFreeBuffer )
		{
		for ( TInt i = 0 ; i < KMaxOutputBuffers; i++ )
			{
			if ( ( iOutputFreeBuffer + i )->iHeader )
				{
				delete ( iOutputFreeBuffer + i )->iHeader;
				}
			if ( ( iOutputFreeBuffer + i )->iData.iRawData )
				{
				delete
				( TUint8* )( iOutputFreeBuffer + i )->iData.iRawData->Ptr();
				}
			if ( ( iOutputFreeBuffer + i )->iData.iRawData )
				{
				delete ( iOutputFreeBuffer + i )->iData.iRawData;
				}
			}
		delete [] iOutputFreeBuffer;
		}
	for ( TInt i = 0; i < iOutputFreeBufferQueue.Count(); i++ )
		{
		iOutputFreeBufferQueue.Remove( 0 );
		}
	// Create the output Buffer( s ) and buffers to engine
	iOutputFreeBuffer = new ( ELeave ) TVideoPicture[KMaxOutputBuffers];
	TInt i;
    for ( i = 0 ; i < KMaxOutputBuffers; i++ )
		{
        ( iOutputFreeBuffer + i )->iData.iRawData = NULL;
		( iOutputFreeBuffer + i )->iHeader = NULL;
        }

	for ( i = 0 ; i < KMaxOutputBuffers; i++ )
		{
		TUint8* ptr;
		TPtr8*  temp;

		ptr = new ( ELeave ) TUint8[iOutputBufferSize];
		CleanupStack::PushL( ptr );
		temp = new ( ELeave ) TPtr8( ptr, 0, iOutputBufferSize );
		CleanupStack::Pop( ptr );

		( iOutputFreeBuffer + i )->iData.iRawData = temp;
		( iOutputFreeBuffer + i )->iHeader =
									    new ( ELeave ) TVideoPictureHeader;

		//Add o/p buffer( s ) to the Queue
		iEngine->AddOutput( ( iOutputFreeBuffer + i ) );
		}
    iBufferAdded = ETrue;
    PRINT_EXIT;
	}


//---------------------------------------------------------------------------
//Gives callbacks to client regarding slice and picture loss
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::SliceAndPictureLoss()
	{
	PRINT_ENTRY;
	TUint firstMacroblock;
	TUint numMacroblocks;
	TPictureId pictureId;
	pictureId.iIdType = TPictureId::EPictureNumber;

	// call back to client regading picture loss
	if ( ( iCodec->GetPictureLossInfo() ) >= KMaxAllowPicLoss )
		{
	  	PRINT_MSG( LEVEL_LOW, ( "Mpeg4aspdecHwDevice::SliceAndPictureLoss, "
	  			"				calling callback regading picture loss" ) );
		iMMFDevVideoPlayProxy->MdvppPictureLoss();
		}

   	PRINT_MSG( LEVEL_LOW, ( "CAriMpeg4aspdecHwDevice::SliceAndPictureLoss, "
   			"				calling callback regarding slice loss" ) );
	 // call back to clinet regarding slice loss
	 iCodec->GetSliceLossInfo( firstMacroblock, numMacroblocks );

	if ( ( firstMacroblock != 0 ) || ( numMacroblocks != 0 ) )
	 	iMMFDevVideoPlayProxy->MdvppSliceLoss( firstMacroblock,
											   numMacroblocks,
											   pictureId );

	PRINT_EXIT;
	}


//---------------------------------------------------------------------------
//Creates one input Buffer
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::CreateInputBufferL( TUint aBufferSize,
											   TBool aReallocate )
	{
	PRINT_ENTRY;

	if ( !aReallocate )
		{
	   	PRINT_MSG( LEVEL_LOW, ( "CreateInputBufferL 1++" ) );
		// Create the Buffer and add it to Queue
		TUint8* ptr = new ( ELeave ) TUint8[aBufferSize];
	   	PRINT_MSG( LEVEL_LOW, ( "CreateInputBufferL 2++" ) );
		TInt* lastBufferFlag  = new ( ELeave ) TInt[sizeof( TInt )];
	   	PRINT_MSG( LEVEL_LOW, ( "CreateInputBufferL 3++" ) );
		iInputFreeBuffers[iNumberOfInputBuffersAllocated].iData.Set( ptr,
													0,
													TInt( aBufferSize ) );
		iInputFreeBuffers[iNumberOfInputBuffersAllocated].iUser =
								   REINTERPRET_CAST( TAny*, lastBufferFlag );
		iInputFreeBufferQueue.Append(
				        iInputFreeBuffers + iNumberOfInputBuffersAllocated );
		iNumberOfInputBuffersAllocated++;
	   	PRINT_MSG( LEVEL_LOW, ( "CreateInputBufferL 4++" ) );
		}

	else // input buffers are already created and do reallocation here
		{
		PRINT_MSG( LEVEL_LOW, ( "CreateInputBufferL 5++" ) );
		TVideoInputBuffer* inBuffer = iInputFreeBufferQueue[0];
		// check the size of the current with the size
		//of the buffer present in i/p Q
		if ( aBufferSize > TUint( inBuffer->iData.MaxLength() ) )
			{
			PRINT_MSG( LEVEL_LOW, ( "CreateInputBufferL 6++" ) );
			// delete the previous buffer
			delete [] ( TUint8* )inBuffer->iData.Ptr();
			PRINT_MSG( LEVEL_LOW, ( "CreateInputBufferL 7++" ) );
			delete inBuffer->iUser;

			// reallocate the buffer
			TUint8* ptr = new ( ELeave ) TUint8[aBufferSize];
			PRINT_MSG( LEVEL_LOW, ( "CreateInputBufferL 8++" ) );
			//delete inBuffer->iUser;
			TInt* lastBufferFlag  = new ( ELeave ) TInt[sizeof( TInt )];
			inBuffer->iData.Set( ptr, 0, TInt( aBufferSize ) );
			inBuffer->iUser = REINTERPRET_CAST( TAny*, lastBufferFlag );
			}
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//When InputEnd is called while the hw device in Stopping State.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::HandleInputEndInStopping()
	{
	PRINT_ENTRY;
	if ( iState->IsInputEndPending() )
		{
		iPictureNumber = 0;
		// Stop & Reset the Engine
		iEngine->Stop();
		iBufferAdded			= EFalse;
		iFilledBufferCounter	= 0;
		iInputEndCalled			= EFalse;
		iInputBufferConsumedCounter = 0;
		iNoLastOut				= EFalse;
		TInt error = iState->Transit( CStateMachine::EStopCommand );
		if ( error != KErrNone )
			{
			iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
			}
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Gets the last frame of the sequence.
//---------------------------------------------------------------------------
//
void CAriMpeg4aspdecHwDevice::GetLastFrame()
	{
	PRINT_ENTRY;
	TInt err = KErrNone;

	if ( ( iEngine->NumOutputBuffers() ) > 0 )
		{
		TVideoPicture* lVideoPicture =
						( TVideoPicture* )( iEngine->FetchOutputBuffer() );
		err = iCodec->GetLastFrame( lVideoPicture );

		if ( err == KErrNone )
			{
			lVideoPicture->iHeader->iPictureNumber = iPictureNumber;

			iPictureCounters.iPicturesDecoded++;
			iPictureCounters.iPicturesDisplayed++;

			iDecodingPosition = lVideoPicture->iTimestamp;

			if ( !iOutputDevice )
				{
				iMMFDevVideoPlayProxy->MdvppNewPicture( lVideoPicture );
				}
			else
				{
				TRAPD( error,
					   iOutputDevice->WritePictureL( lVideoPicture ) );
				if ( error != KErrNone )
					{
					iPictureCounters.iPicturesDisplayed--;
					// Currently not handled
		           	}
				}
			}
		else
			{
			//Add the output buffer back to ProcessEngine
			iEngine->AddOutput( lVideoPicture );
			}
		}
	PRINT_EXIT;
	}

const TImplementationProxy ImplementationTable[] =
	{
    IMPLEMENTATION_PROXY_ENTRY( KUidMp4DecoderHwDeviceImplUid,
								CAriMpeg4aspdecHwDevice::NewL )
	};


EXPORT_C const TImplementationProxy* ImplementationGroupProxy(
														TInt& aTableCount )
	{
	PRINT_ENTRY;
	aTableCount =
			sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
	PRINT_EXIT;
	return ImplementationTable;
	}
