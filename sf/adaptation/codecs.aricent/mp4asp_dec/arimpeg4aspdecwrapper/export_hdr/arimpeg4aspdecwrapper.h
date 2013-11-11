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
* Header file to the Mpeg4Asp Decoder wrapper Implementation.
*
*/

#ifndef ARIMPEG4ASPDECWRAPPER_H
#define ARIMPEG4ASPDECWRAPPER_H

//System includes
#include <e32def.h>
#include <e32base.h>
#include <devvideoconstants.h>

//User includes
#include "aribasecodec.h"
#include "ariprint.h"

const TInt KMaxFrameWidth			= 640;//should be 1280 for enabling 720P decoding;
const TInt KMaxFrameHeight			= 480;//should be 720 for enabling 720P decoding;

// FORWARD DECLARATIONS
class TUncompressedVideoFormat;
class TVideoInputBuffer;
class TVideoPictureHeader;

/**
 *	Class MCodecObserver
 *  whenever a new configuration information finds in the file, ti should
 *  be informated to the hwdevice. so that it will delete the existing
 *  codec object and wil create a new object and configures the codec with
 *  the new configuration information.
 */

class MCodecObserver
	{
	public:
		/**
		 * Callback to inform the HW device when there is new config.
		 * @return "None"
		 */
		virtual void NewConfigFound()=0;

		/**
		 * Callback to inform the HW device when there is new config.
		 * @return "None"
		 */
		virtual void EInputConsumed()=0;
	};
/**
 *	Class CCodecInterface
 *	This class is base class whish is part of Aricent's Mpeg4 ASP/H263 decoder wrapper
 *	used by the Mpeg4 ASP/H263. This HwDevice Plugin to decode Mpeg4 ASP/H263 content to
 *	yuv420.
 *
 */
class CAriMpeg4aspdecWrapper: public MBaseCodec, public CBase
	{
	public:// Constructor and Destructor

		/**
		 * Two-phased constructor.
		 * @return pointer to an instance of CAriMpeg4aspdecWrapper
		 */
		IMPORT_C static CAriMpeg4aspdecWrapper* NewL(
				                                  MCodecObserver* aObserver );

		/**
		 * Destructor
		 */
		virtual ~CAriMpeg4aspdecWrapper();


	public:// MBaseCodec functions

		/**
		* Retrieves a custom interface to the specified hardware device

		* @param	aInpBuf	Coded input data passed by Engine.
		* @param	aOutBuf	Decoded output data.

		* @leave	"The method will leave if an error occurs".

		* @return	one of the TCodecState
		*/
		virtual TInt DoProcessL( TAny *aInpBuf, TAny* aOutBuf = NULL ) = 0;

		/**
		 * Resets the Decoder
		 * @return None
		 */
		virtual void Reset() = 0;

    public: // New Functions
  	   /**
  		* Common API to set the values to the decoder.
  		* @param	aCommand	indicates the action to do.
  		* @param	aCmdData	Any input data required

  		* @leave	"The method will leave if an error occurs".
  		*/
		virtual TInt SetParam( TInt aCommand, TAny* aCmdData ) = 0;

	   /**
		* Common API to get the values to the decoder.
		* @param	aCommand	indicates the action to do.
		* @param	aCmdData	Any output data required

		* @leave	"The method will leave if an error occurs".
		*/
		virtual TInt GetParam( TInt aCommand, TAny* aCmdData ) = 0;
		/**
		* Returns the stream information and creates the decoder
		* @param	aInpBuf	Coded input data.
		* @param	aOutBuf	Decoded output data.

		* @leave	The method will leave if an error occurs.
		*/
		virtual void SetFormatsL( TVideoDataUnitType &aInputFormat,
                                TUncompressedVideoFormat &aOutputFormat ) = 0;

		/**
		* Configure the decoder using the stream passed
		* @param	aInpBuf	Coded input data.

		* @leave	The method will leave if an error occurs.
		*/
        virtual void ConfigDecoderL( const TDesC8* aInpBuf ) = 0;

        /**
		* Returns the length of the ouput frame
        * @return

		* @leave	The method will leave if an error occurs.
		*/
		virtual TInt GetOutputBufferLength() = 0;

		/**
		* Gets the slice loss information
		* @param aFirstMacroBlock
		* @param aNumMacroBlocks
		*/
		virtual void GetSliceLossInfo( TUint& aFirstMacroblock,
				                       TUint& aNumMacroblocks ) = 0;

		/**
		* Gets the picture Loss Information
		* @return percentage value
		*/
		virtual TUint GetPictureLossInfo() = 0;

		/**
		* Gets the Last frame
		* @return none
		*/
		virtual TInt GetLastFrame( TAny* aOutput ) = 0;

		/**
		* Gets the header info
		* @param	aDataUnit	Input stream data.
		* @param	aHeaderPtr  Output header information returned.

		* @leave	The method will leave if an error occurs
                    KErrArgument  - input data is null or zero length
                    KErrCorrupt   - input stream is corrupt
                    KErrUnderflow - input stream is insufficient.
		*/
		IMPORT_C
		static void  GetHeaderInfoL( TVideoInputBuffer& aDataUnit,
                                    TVideoPictureHeader& aHeaderPtr );
	};

#endif //ARIMPEG4ASPDECWRAPPER_H
