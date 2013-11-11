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
* This is a source file, required for arih264dechwdevice
*
*/

//User includes
#include "arih264dechwdevice.h"
#include "arih264decwrapper.h"
#include "aristatemachine.h"
#include "ariprint.h"

//
//CAriH264decHwDevice
//

_LIT( KManufacturer, "Aricent" );

_LIT( KIdentifier, "H264 Video Decoder" );

//supported h264 mime types
_LIT8( KH264SupportedMimeType, "video/H264" );


//---------------------------------------------------------------------------
//1st phase constructor of CAriH264decHwDevice
//---------------------------------------------------------------------------
//
CMMFVideoDecodeHwDevice* CAriH264decHwDevice::NewL()
	{
	PRINT_ENTRY;
	CAriH264decHwDevice* self = new ( ELeave ) CAriH264decHwDevice;
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop();
	PRINT_EXIT;
	return self;
	}

//---------------------------------------------------------------------------
//Destructor
//---------------------------------------------------------------------------
//
CAriH264decHwDevice::~CAriH264decHwDevice()
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
		for( TInt i = 0 ; i < KMaxOutputBuffers; i++ )
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
		for ( TInt i = 0 ; i < iBufferOptions.iMinNumInputBuffers; i++ )
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
TAny* CAriH264decHwDevice::CustomInterface( TUid /*aInterface*/ )
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
CVideoDecoderInfo* CAriH264decHwDevice::VideoDecoderInfoLC()
	{
    PRINT_ENTRY;

    // construct the video types for iVidTypes
	CCompressedVideoFormat* format = NULL;
	format = CCompressedVideoFormat::NewL( KH264SupportedMimeType );
	CleanupStack::PushL(format);
	TInt status = iSupportedFormats.Append( format );
	if ( status != KErrNone )
		{
		PRINT_MSG( LEVEL_LOW, ( " Format support is not done, retval of "
				"append = %d \n", status ) );
		}
	CleanupStack::Pop( format );

    for ( TUint i = 0; i < KDecoderInfoCount; i++ )
		{
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
            KUidH264DecoderHwDevice,
            KManufacturer,
            KIdentifier,
            TVersion( 1, 0, 0 ),
            iSupportedFormats.Array(),
            // Non-Accelerated
            EFalse,
            // Doesnt support Direct Display
            EFalse,
            TSize( KMaxFrameWidth, KMaxFrameHeight ),
            maxBitRate,
            iMaxPictureRates.Array(),
            //decoder supports picture loss indications
            ETrue,
            //decoder supports slice loss indications
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
TVideoPictureHeader* CAriH264decHwDevice::GetHeaderInformationL(
								TVideoDataUnitType /*aDataUnitType*/,
								TVideoDataUnitEncapsulation aEncapsulation,
								TVideoInputBuffer* aDataUnit )
	{
	PRINT_ENTRY;
    if (!aDataUnit )
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

	TRAPD( err, CAriH264decWrapper::GetHeaderInfoL( *aDataUnit,
													*iVideoPictureHeader ) );

    if ( err == KErrNone )
    	{
    	PRINT_EXIT;
        return ( iVideoPictureHeader );
    	}

    // An error has occured
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
void CAriH264decHwDevice::ReturnHeader( TVideoPictureHeader* aHeader )
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
void CAriH264decHwDevice::SetInputFormatL(
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
             ( aEncapsulation == EDuRtpPayload ) )
             User::Leave( KErrNotSupported );

		if ( ( aFormat.MimeType().FindF( KH264SupportedMimeType ) !=
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
void CAriH264decHwDevice::SynchronizeDecoding( TBool /*aSynchronize*/ )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	return;
	}


//---------------------------------------------------------------------------
//Sets the Buffer Options as specified by the client.
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::SetBufferOptionsL(
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
void CAriH264decHwDevice::GetBufferOptions(
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
void CAriH264decHwDevice::SetHrdVbvSpec(
										THrdVbvSpecification /*aHrdVbvSpec*/,
										const TDesC8& /*aHrdVbvParams*/ )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Sets the output post-processor device to use.
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::SetOutputDevice(
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
			iCustomBufferHandle = ( MMmfVideoBufferManagement* )
	      ( iOutputDevice->CustomInterface( KMmfVideoBuffermanagementUid ) );
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
TTimeIntervalMicroSeconds CAriH264decHwDevice::DecodingPosition()
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
		return ( TTimeIntervalMicroSeconds( 0 ) );
		}
	}

//---------------------------------------------------------------------------
//Returns the current pre-decoder buffer size.
//---------------------------------------------------------------------------
//
TUint CAriH264decHwDevice::PreDecoderBufferBytes()
	{
	PRINT_ENTRY;
    if ( !( iState->IsInitialized() ) )
        {
        iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
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
void CAriH264decHwDevice::GetBitstreamCounters(
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
TUint CAriH264decHwDevice::NumFreeBuffers()
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
		return( 0 );
		}
	}

//---------------------------------------------------------------------------
//Returns an input buffer of size aBufferSize. If no buffer is available
//then NULL is returned.
//---------------------------------------------------------------------------
//
TVideoInputBuffer* CAriH264decHwDevice::GetBufferL( TUint aBufferSize )
	{
	PRINT_ENTRY;
	if ( !( iState->IsInitialized() ) )
        {
        PRINT_ERR( "istate is not initialized yet\n" );
        User::Leave( KErrNotReady );
		return NULL;
        }

    if ( iState->IsInputEndPending() )
	    {
	    PRINT_ERR( "input is pending\n" );
		User::Leave( KErrNotReady );
		return NULL;
	    }

    if ( aBufferSize > iBufferOptions.iMaxInputBufferSize )
        {
        PRINT_ERR( "aBufferSize is more than the allowed size\n" );
        User::Leave( KErrArgument );
        return NULL;
        }


	TVideoInputBuffer *inBuffer;

    if ( iNumberOfInputBuffersAllocated ==
									     iBufferOptions.iMinNumInputBuffers )
        {
        if ( iInputFreeBufferQueue.Count() )
            {
            PRINT_ERR( "CreateInputBufferL returned ETRUE\n" );
            // Reallocate the input buffer if needed
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
        PRINT_ERR( "CreateInputBufferL returned EFALSE\n" );
        // Allocate new buffer of the requested size
        CreateInputBufferL( aBufferSize, EFalse );

        // remove the last created buffer
        inBuffer = iInputFreeBufferQueue[ iInputFreeBufferQueue.Count() - 1];

        iInputFreeBufferQueue.Remove( iInputFreeBufferQueue.Count() - 1 );
        }
    else
        {
        PRINT_ERR( "iNumberOfInputBuffersAllocated is less "
                		"			iBufferOptions.iMinNumInputBuffers\n" );
        User::Leave( KErrGeneral ); // unwanted state;
        return NULL;
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
void CAriH264decHwDevice::WriteCodedDataL( TVideoInputBuffer* aBuffer )
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

	if ( !iCodec )
		{
		iCodec = CAriH264decWrapper::NewL( iDataUnitType,
				                           iOutputFormat,
				                           &aBuffer->iData,
				                           iStreamType );

		iDecoderConfigured = ETrue;
		}

	if ( ( iDataUnitType == EDuSeveralSegments ) ||
		 ( iDataUnitType == EDuVideoSegment ) )
		{
		iBitstreamCounters.iTotalPackets++;
		}

	// if custom interface is supported by o/p device get all the buffers
	//and add it to Engine
	if ( !iOutputBuffersCreated )
		{
		// Set Stream information
		if ( !iDecoderConfigured )
			{
			User::Leave( KErrNotReady );
			}

    	if ( !iCustomBufferHandle )
    		{
			// Get o/p buffer length for creation
			iCodec->GetParam( CONTROL_CMD_GET_OUTPUTBUFFERLENGTH,
							  ( TAny* )&iOutputBufferSize );
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
				  i < iCustomBufferOptions->iNumInputBuffers;
				  i++ )
				{
				videoPicture =
				iCustomBufferHandle->MmvbmGetBufferL(
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
			for( TInt i = 0; i < iOutputFreeBufferQueue.Count(); i++ )
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
	PRINT_MSG( LEVEL_LOW, ( "WriteCodedDataL, addinput err=%d", err ) );
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Configures the H264 decoder by passing the required header
//information for the stream that is getting decoded. The header
//structurespecific  to the Decoder is passed as iOptional member.
//---------------------------------------------------------------------------
//
#ifdef SYMBIAN_MDF_API_V2
void CAriH264decHwDevice::ConfigureDecoderL(
							 const TVideoPictureHeader& aVideoPictureHeader )
	{
	PRINT_ENTRY;
	if ( iState->IsInitialized() )
		{
		PRINT_ERR( "iState is already initialised \n" );
		User::Leave( KErrNotReady );
		}
	if ( !iCodec )
		{
		iCodec = CAriH264decWrapper::NewL( iDataUnitType,
				                       iOutputFormat,
				                       aVideoPictureHeader.iOptional,
				                       iStreamType );

		if ( !iCodec )
			{
			PRINT_ERR( "codec wrapper returned error \n" );
			User::Leave( KErrNotReady );
			}
		iDecoderConfigured = ETrue;
	    iConfigureDecoderCalled = ETrue;
		}

	iWidthSource = aVideoPictureHeader.iSizeInMemory.iWidth;
	iHeightSource = aVideoPictureHeader.iSizeInMemory.iHeight;
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
CPostProcessorInfo* CAriH264decHwDevice::PostProcessorInfoLC()
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

	CPostProcessorInfo* info = CPostProcessorInfo::NewL(
		KUidH264DecoderHwDevice,
		KManufacturer,
		KIdentifier,
		TVersion( 1, 0, 0 ),
		supportedFormats.Array(),
		supportedCombinations.Array(),
		EFalse, //Accelerated or not
		EFalse, //supports DSA
		yuvToRgbCapabilities,
		ERotateNone, //supported rotations
		EFalse, //supports Arbitrary scaling
		supportedScaleFactors.Array(),
		EFalse ) //supports antialiased scaling
        ;

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
void CAriH264decHwDevice::GetOutputFormatListL(
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
    if ( iInputFormat->MimeType().FindF( KH264SupportedMimeType ) !=
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
void CAriH264decHwDevice::SetOutputFormatL(
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

	// check whether the output format is supported or not. if not
    //supported leave
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
void CAriH264decHwDevice::SetClockSource( MMMFClockSource* /*aClock*/ )
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
void CAriH264decHwDevice::SetVideoDestScreenL( TBool aScreen )
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
void CAriH264decHwDevice::SetPostProcessTypesL(
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
void CAriH264decHwDevice::SetInputCropOptionsL( const TRect& /*aRect*/ )
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
void CAriH264decHwDevice::SetYuvToRgbOptionsL(
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
void CAriH264decHwDevice::SetYuvToRgbOptionsL(
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
void CAriH264decHwDevice::SetRotateOptionsL( TRotationType /*aRotationType*/ )
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
void CAriH264decHwDevice::SetScaleOptionsL( const TSize& /*aTargetSize*/,
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
void CAriH264decHwDevice::SetOutputCropOptionsL( const TRect& /*aRect*/ )
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
void CAriH264decHwDevice::SetPostProcSpecificOptionsL(
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
void CAriH264decHwDevice::Initialize()
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
		iCustomBufferOptions->iBufferSize	= TSize( KMaxFrameWidth,
				KMaxFrameHeight );

		PRINT_MSG( LEVEL_HIGH, ( "w = %d, h = %d \n", KMaxFrameWidth,
				KMaxFrameHeight) );

		iCustomBufferHandle->MmvbmSetObserver(
							   ( MMmfVideoBufferManagementObserver * )this );

		iCustomBufferHandle->MmvbmEnable( ETrue );

		TRAP ( err, iCustomBufferHandle->MmvbmSetBufferOptionsL(
												*iCustomBufferOptions ) );

		if ( err != KErrNone )
			{
			iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
			PRINT_ERR( "iCustomBufferHandle->MmvbmSetBufferOptionsL \n" );
			return;
			}
		}
	// Engine Creation
	TRAP( err, iEngine = CBaseEngine::NewL( this,
											 ( MBaseCodec* )iCodec,
											 EFalse ) );
	if ( err != KErrNone )
		{
		iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
		PRINT_ERR( "Baseengine::NewL returned error \n" );
		return;
		}

	// set input & output formats to Codec

	err = iState->Transit( CStateMachine::EInitializeCommand );
	if ( err )
		{
		iMMFDevVideoPlayProxy->MdvppInitializeComplete( this, err );
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
void CAriH264decHwDevice::CommitL()
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
void CAriH264decHwDevice::Revert()
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
void CAriH264decHwDevice::StartDirectScreenAccessL(
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
void CAriH264decHwDevice::SetScreenClipRegion( const TRegion& /*aRegion*/ )
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
void CAriH264decHwDevice::SetPauseOnClipFail( TBool /*aPause*/ )
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
void CAriH264decHwDevice::AbortDirectScreenAccess()
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
TBool CAriH264decHwDevice::IsPlaying()
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
void CAriH264decHwDevice::Redraw()
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
void CAriH264decHwDevice::Start()
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
void CAriH264decHwDevice::Stop()
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
void CAriH264decHwDevice::Pause()
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
void CAriH264decHwDevice::Resume()
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
void CAriH264decHwDevice::SetPosition(
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
void CAriH264decHwDevice::FreezePicture(
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
void CAriH264decHwDevice::ReleaseFreeze(
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
TTimeIntervalMicroSeconds CAriH264decHwDevice::PlaybackPosition()
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
TUint CAriH264decHwDevice::PictureBufferBytes()
	{
	PRINT_ENTRY;
	if ( !( iState->IsInitialized() ) )
		{
		PRINT_ERR( "iState is not initialised \n" );
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	PRINT_EXIT;
	return( KMaxOutputBuffers * iOutputBufferSize );
	}

//---------------------------------------------------------------------------
//Reads various counters related to decoded pictures.
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::GetPictureCounters(
							CMMFDevVideoPlay::TPictureCounters& aCounters )
	{
	PRINT_ENTRY;
    if ( !( iState->IsInitialized() ) )
    	{
    	PRINT_ERR( "iState is not initialised yet \n" );
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
void CAriH264decHwDevice::SetComplexityLevel( TUint /*aLevel*/ )
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
TUint CAriH264decHwDevice::NumComplexityLevels()
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
void CAriH264decHwDevice::GetComplexityLevelInfo( TUint /*aLevel*/,
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
void CAriH264decHwDevice::ReturnPicture( TVideoPicture* aPicture )
	{
	PRINT_ENTRY;
    if ( !( iState->IsInitialized() ) )
    	{
    	PRINT_ERR( "iState is not initialised yet \n" );
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
TBool CAriH264decHwDevice::GetSnapshotL( TPictureData& /*aPictureData*/,
								const TUncompressedVideoFormat& /*aFormat*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	return( 0 );
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::GetTimedSnapshotL(
				TPictureData* /*aPictureData*/,
				const TUncompressedVideoFormat& /*aFormat*/,
				const TTimeIntervalMicroSeconds& /*aPresentationTimestamp*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	//Should take it from Post Proc HwDevice
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::GetTimedSnapshotL(
								TPictureData* /*aPictureData*/,
								const TUncompressedVideoFormat& /*aFormat*/,
								const TPictureId& /*aPictureId*/ )
	{
	PRINT_ENTRY;
	PRINT_ERR( "not supported API\n" );
	 //Should take it from Post Proc HwDevice
	PRINT_EXIT;
	User::Leave( KErrNotSupported );
	}

//---------------------------------------------------------------------------
//Not supported API.
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::CancelTimedSnapshot()
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
void CAriH264decHwDevice::GetSupportedSnapshotFormatsL(
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
void CAriH264decHwDevice::InputEnd()
	{
	PRINT_ENTRY;
	if ( !( iState->IsTransitionValid( CStateMachine::EInputEndCommand ) ) )
		{
		iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
		}

	iInputEndCalled = ETrue;

	if ( iFilledBufferCounter == 0 )
		{
		Stop();
		PRINT_EXIT;
		iMMFDevVideoPlayProxy->MdvppStreamEnd();
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
TInt CAriH264decHwDevice::InputBufferConsumed ( TAny* aInp, TInt aError  )
	{
	PRINT_ENTRY;
	if ( aInp )
		{
		 iInputFreeBufferQueue.Append( ( TVideoInputBuffer* )aInp );

		 //call back to clinet
		 if ( ( aError != KErrCancel ) && ( !iInputEndCalled ) )
			iMMFDevVideoPlayProxy->MdvppNewBuffers();
		}
	PRINT_EXIT;
	return ( KErrNone );
	}


//---------------------------------------------------------------------------
//Notifies the Hw Device that out buffer has decoded data
//---------------------------------------------------------------------------
//
TInt CAriH264decHwDevice::OutputBufferReady ( TAny* aOut, TInt aError )
	{
	PRINT_ENTRY;
    if ( aError == KErrNone )
        {
        iPictureNumber++;
        // call back information regarding slice and picture loss

        SliceAndPictureLoss();

        //Decoded Buffer is available
        TVideoPicture* videoPicture = ( TVideoPicture* )aOut;
        videoPicture->iHeader->iPictureNumber = iPictureNumber;

        iFilledBufferCounter--;

        iPictureCounters.iPicturesDecoded++;
        iPictureCounters.iPicturesDisplayed++;

        iDecodingPosition = videoPicture->iTimestamp;
		if ( !iOutputDevice )
			{
			iMMFDevVideoPlayProxy->MdvppNewPicture( videoPicture );

			if ( iInputEndCalled && ( iFilledBufferCounter == 0 ) )
				{
				Stop();
				PRINT_ERR( "calling streamend\n" );
				iMMFDevVideoPlayProxy->MdvppStreamEnd();
				}

			return( KErrNone );
			}
		else
			{
			TRAPD( error, iOutputDevice->WritePictureL( videoPicture ) );

			if ( error == KErrNone )
				{
				if ( iInputEndCalled && ( iFilledBufferCounter == 0 ) )
					{
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
			iOutputFreeBufferQueue.Append( ( TVideoPicture* )aOut );
			}
		else
			{
			delete ( ( TVideoPicture* )aOut )->iHeader;
			( ( TVideoPicture* )aOut )->iHeader = NULL;

			// release the buffer
			iCustomBufferHandle->MmvbmReleaseBuffer( ( TVideoPicture* )aOut );
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
void CAriH264decHwDevice::FatalErrorFromProcessEngine ( TInt aError )
	{
	PRINT_ENTRY;
	iMMFDevVideoPlayProxy->MdvppFatalError( this, aError );
	PRINT_EXIT;
	}

/*
******************************************************************************
Name            : CommandProcessed
Description     : Callback to indicate the command has been processed
Parameter       :
Return Value    :
Assumptions     : None
Known Issues    : None
******************************************************************************
*/
void CAriH264decHwDevice::CommandProcessed ( TInt aCmd, TAny* aCmdData,
												TInt aError )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	}



//---------------------------------------------------------------------------
//Call back from output device, indicates buffers availability
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::MmvbmoNewBuffers()
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
	if ( videoPicture )
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
void CAriH264decHwDevice::MmvbmoReleaseBuffers()
	{
	PRINT_ENTRY;
	iEngine->Stop();
	iEngine->Reset();
	PRINT_EXIT;
	}



//---------------------------------------------------------------------------
//Set the proxy implementation to be used. Called just
//after the object is constructed
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::SetProxy( MMMFDevVideoPlayProxy& aProxy )
	{
	PRINT_ENTRY;
	iMMFDevVideoPlayProxy = &aProxy;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Default constructor of CAriH264decHwDevice
//---------------------------------------------------------------------------
//
CAriH264decHwDevice::CAriH264decHwDevice()
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
	 iDecoderConfigured( EFalse )
    {
    PRINT_ENTRY;
	iBufferOptions.iPreDecodeBufferSize 		= 0;
	iBufferOptions.iMaxPostDecodeBufferSize   	= 0;
	iBufferOptions.iPreDecoderBufferPeriod 		= 0;
	iBufferOptions.iPostDecoderBufferPeriod 	= 0;
	iBufferOptions.iMinNumInputBuffers 			= KMaxInputBuffers;
	iBufferOptions.iMaxInputBufferSize 			= KMaxInputBufferSize;


    // Initializing iOutputFormat with one of the combination supported by
	iOutputFormat.iDataFormat					= EYuvRawData;
	iOutputFormat.iYuvFormat.iCoefficients		= EYuvBt601Range0;
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
void CAriH264decHwDevice::ConstructL()
	{
	PRINT_ENTRY;
	iState = CStateMachine::NewL();
	PRINT_EXIT;
    }

//---------------------------------------------------------------------------
//Module to create output data
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::CreateOutputBuffersL()
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

		// kunal
		// set the frame widht and height

		( iOutputFreeBuffer + i )->iData.iDataSize =
										TSize( iWidthSource,iHeightSource );
		//Add o/p buffer( s ) to the Queue
		iEngine->AddOutput( ( iOutputFreeBuffer + i ) );
		}
	PRINT_EXIT;
	}


//---------------------------------------------------------------------------
//Gives callbacks to client regarding slice and picture loss
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::SliceAndPictureLoss()
	{
	PRINT_ENTRY;
	TUint firstMacroblock = 0;
	TUint numMacroblocks = 0;
	TPictureId pictureId;
	TUint picLoss = 0;
	pictureId.iIdType = TPictureId::EPictureNumber;

	 // call back to client regading picture loss
	if ( ( iCodec->GetParam( CONTROL_CMD_GET_PICTURELOSSINFO,
							 ( TAny* )&picLoss ) ) >= KMaxAllowPicLoss )
		iMMFDevVideoPlayProxy->MdvppPictureLoss();

	PRINT_MSG( LEVEL_LOW, ( "CAriH264decHwDevice::SliceAndPictureLoss, "
	   			"				calling callback regarding slice loss" ) );

	 // call back to clinet regarding slice loss
	firstMacroblock = iCodec->GetParam( CONTROL_CMD_GET_SLICELOSSINFO,
									    ( TAny* )&numMacroblocks );

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
void CAriH264decHwDevice::CreateInputBufferL( TUint aBufferSize,
											  TBool aReallocate )
	{
	PRINT_ENTRY;
	if ( !aReallocate )
		{
		// Create the Buffer and add it to Queue
		TUint8* ptr = new ( ELeave ) TUint8[aBufferSize];
		TInt* lastBufferFlag  = new ( ELeave ) TInt[sizeof( TInt )];
		iInputFreeBuffers[iNumberOfInputBuffersAllocated].iData.Set(
															ptr,
															0,
															aBufferSize );
		iInputFreeBuffers[iNumberOfInputBuffersAllocated].iUser =
													( TAny* )lastBufferFlag;
		iInputFreeBufferQueue.Append(
						iInputFreeBuffers + iNumberOfInputBuffersAllocated );
		iNumberOfInputBuffersAllocated++;
		}

	else // input buffers are already created and do reallocation here
		{
		TVideoInputBuffer* inBuffer = iInputFreeBufferQueue[0];
		// check the size of the current with the size of the buffer
		//present in i/p Q
		if ( aBufferSize > inBuffer->iData.MaxLength() )
			{
			// delete the previous buffer
			delete [] ( TUint8* )inBuffer->iData.Ptr();
			delete inBuffer->iUser;

			// reallocate the buffer
			TUint8* ptr = new ( ELeave ) TUint8[aBufferSize];
			TInt* lastBufferFlag  = new ( ELeave ) TInt[sizeof( TInt )];
			inBuffer->iData.Set( ptr, 0, aBufferSize );
			inBuffer->iUser = ( TAny* )lastBufferFlag;
			}
		}
	PRINT_EXIT;
	}
//---------------------------------------------------------------------------
//When InputEnd is called while the hw device in Stopping State.
//---------------------------------------------------------------------------
//
void CAriH264decHwDevice::HandleInputEndInStopping()
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
		TInt error = iState->Transit( CStateMachine::EStopCommand );
		if ( error != KErrNone )
			{
			iMMFDevVideoPlayProxy->MdvppFatalError( this, KErrNotReady );
			}
		}
	PRINT_EXIT;
	}


const TImplementationProxy ImplementationTable[] =
	{
	//IMPLEMENTATION_PROXY_ENTRY( 0x20029903, CAriH264decHwDevice::NewL )
    IMPLEMENTATION_PROXY_ENTRY( KUidH264DecoderHwDeviceImplUid,
										CAriH264decHwDevice::NewL )
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


#ifndef EKA2
GLDEF_C TInt E32Dll( TDllReason )
    {
    return KErrNone;
    }

#endif

