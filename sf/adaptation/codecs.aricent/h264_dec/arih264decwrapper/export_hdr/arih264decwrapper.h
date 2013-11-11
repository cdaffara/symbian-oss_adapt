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
* Header file to the H264 Decoder wrapper Implementation.
*
*/

#ifndef ARIH264DECWRAPPER_H
#define ARIH264DECWRAPPER_H

//	INCLUDES
#include <e32def.h>
#include <e32base.h>
#include <devvideoconstants.h>
#include "aribasecodec.h"
#include "ariprint.h"

// FORWARD DECLARATIONS
class TUncompressedVideoFormat;
class TVideoInputBuffer;
class TVideoPictureHeader;

enum TSetCommands
{
	CONTROL_CMD_SET_TIMESTAMP
};

enum TGetCommands
{
	CONTROL_CMD_GET_SLICELOSSINFO,
	CONTROL_CMD_GET_PICTURELOSSINFO,
	CONTROL_CMD_GET_OUTPUTBUFFERLENGTH
};

const TInt KMaxFrameWidth = 640;//should be 1280 for enabling 720P decoding;
const TInt KMaxFrameHeight = 480;//should be 720 for enabling 720P decoding;

/**
 *	Class CAriH264decWrapper
 *	This class is part of Aricent's H264 decoder wrapper used by the H264
 *	This HwDevice Plugin to decode H264 content to yuv420.
 *
 */
class CAriH264decWrapper: public CBase, public MBaseCodec
	{

	public:// Constructor and Destructor

		/**
		 * Two-phased constructor.
			@param	"aInputFormat"	"type of input format, decides packetmode
			                         or framemode".
			@param	"aOutputFormat"	"supported output formats".
			@param	"aInpBuf"	    "configuration data required to create the
			                         decoder".

			@leave	"The method will leave if an error occurs".

		 * @return pointer to an instance of CAriH264decWrapper
		 */
		IMPORT_C static CAriH264decWrapper* NewL(
				                     TVideoDataUnitType &aInputFormat,
                                     TUncompressedVideoFormat &aOutputFormat,
                                     const TDesC8* aInpBuf, TInt aStreamType);

		/**
		 * Destructor
		 */
		virtual ~CAriH264decWrapper();


	public:// MBaseCodec functions

		/**
		Retrieves a custom interface to the specified hardware device

		@param	"aInpBuf"	"Coded input data passed by Engine".
		@param	"aOutBuf"	"Decoded output data".

		@leave	"The method will leave if an error occurs".

		@return	one of the TCodecState
		*/
		virtual TInt DoProcessL ( TAny *aInpBuf, TAny* aOutBuf = NULL ) = 0;

		/**
		 Resets the Decoder
		 @return None
		 */
		virtual void Reset () = 0;

    public: // New Functions
		virtual TInt SetParam (TInt aCommand, TAny* aCmdData) = 0;
		virtual TInt GetParam (TInt aCommand, TAny* aCmdData) = 0;

		/**
		Gets the header info
		@param	"aDataUnit"	"Input stream data".
		@param	"aHeaderPtr" "Output header information returned".

		@leave	"The method will leave if an error occurs
                KErrArgument  - input data is null or zero length
                KErrCorrupt   - input stream is corrupt
                KErrUnderflow - input stream is insufficient".
		*/
		IMPORT_C
		static void  GetHeaderInfoL(TVideoInputBuffer& aDataUnit,
                                    TVideoPictureHeader& aHeaderPtr);
	};

#endif //ARIH264DECWRAPPER_H
