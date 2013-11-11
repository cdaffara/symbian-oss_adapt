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
* Export header file for wrapper APIs.Interface class for
* H264 encoder wrapper.The member functions are pure virtual functions which
* are to be implemented by the derived class.
*
*/


#ifndef ARIH264ENCWRAPPER_H
#define ARIH264ENCWRAPPER_H

//	INCLUDES
#include <e32def.h>
#include <e32base.h>
#include <devvideoconstants.h>
#include "aribasecodec.h"

//Uncomment to generate debug information useful for tracking bitrate problems
//#define STORETOLOG

// FORWARD DECLARATIONS
class MBaseCodecObserver;
class TH264EncInitParams;


class CAriH264encWrapper: public CBase, public MBaseCodec
	{

	public:

		/** 
		 * Two-phased constructor.	
		 * @param aEncoderParams
		 *    Encoder params used to create the encoder.
		 * @return pointer to an instance of CAriH264encWrapper
		 */
		IMPORT_C static CAriH264encWrapper* NewL(
										TH264EncInitParams &aEncoderParams );

		/**> Destructor */
		virtual ~CAriH264encWrapper();

	
	public:


		/** 
		 * From MBaseCodec
		 * Encodes an input buffer
		 * @param	aInpBuf
		 *    Coded input data passed by Engine.
		 * @param	aOutBuf
		 *	  Encoded output data
		 * @leave	"The method will leave if an error occurs".
		 * @return	one of the TCodecState
		 */

		virtual TInt DoProcessL ( TAny *aInpBuf, TAny* aOutBuf ) = 0;

		/**
		 * From MBaseCodec
		 * Encodes an input buffer
		 * @param	aCommand
		 *    The command passed - indicates encoder parameter to be set.
		 * @param	aOutBuf
		 *	  The value used to set the parameter.
		 * @leave	"The method will leave if an error occurs".
		 * @return	one of the TCodecState
		*/

		virtual TInt SetParam (TInt aCommand, TAny* aCmdData) = 0;
		
		/**
		 * Used to get codec parameteres
		 * @param aCommand
		 *    Indicates the encoder parameter to get
		 * @param aCmdData
		 *    The value of the encoder parameter (OUT)
		 * @return symbian wide error code
		 */

		virtual TInt GetParam(TInt aCommand, TAny* aCmdData) = 0;


		/**
		 * Cancels all processing of the commands
		 * @return None
		 */
		virtual void Reset () = 0;

		/**
		 * Sets the clocksource.
		 * @param aClockSource
   		 *    The clocksource which will be used by wrapper.
   		 * @param aProcessingTime
   		 *    Not used currently.
		 * @return TInt KErrArgument if clocksource is NULL
		 *              else returns KErrNone
		 */

		virtual TInt SetSyncOptions( TAny* aClockSource,
									TInt aProcessingTime = 0 ) = 0;

		/**
		 * Sets the reference time to be used by wrapper.
		 * @param aReferenceTime
		 *    The reference time passed by the plugin
		 * @return None
		 */

		virtual void SetUpdatedRefernceTime( TInt64 &aReferenceTime ) = 0;

		/**
		 * Indicates whether current picture has been encoded or not.
		 * @return TBool ETrue if ppicture is skipped.
		 */
		
		virtual TBool IsCurrentPictureSkipped() = 0;

#ifdef STORETOLOG
     public:
 	TInt 	iRemainingFramesInSecLog;
 	TInt	iRemainingBitsInSecLog;
 	TInt	iTargetLog;
 	TInt    iRemainingBitsInSecBHLog;
	TInt    iFrameSkipHappenedLog;
	TInt	iTimeStampEncLog;
	TInt	iCheckingForDropLog;
	TInt	iAbsFrameCntLog;
	TInt	iFrameCntLog;
	TInt	iRCLevelLog;
	TInt	iScarceBitsLog;
	TInt	iExcessCountLog;
#endif

	};

#endif //ARIH264ENCWRAPPER_H
