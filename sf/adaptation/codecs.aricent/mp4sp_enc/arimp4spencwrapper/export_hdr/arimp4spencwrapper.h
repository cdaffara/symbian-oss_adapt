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
* Mpeg4SP/H263 encoder wrapper.The member functions are pure virtual functions
* which are to be implemented by the derived class.
*
*/


#ifndef ARIMP4SPENCWRAPPER_H
#define ARIMP4SPENCWRAPPER_H

//	INCLUDES
#include <e32def.h>
#include <e32base.h>
#include <devvideoconstants.h>
#include "aribasecodec.h"


// FORWARD DECLARATIONS
class MBaseCodecObserver;
class TMpeg4H263HWDeviceInitParams;
class TMpeg4H263EncoderInitParams;


class CAriMp4spencWrapper:  public CBase, public MBaseCodec
	{

	public:// Constructor and Destructor

		/**
		 * Two-phased constructor.
		 * @param aEncoderParams
		 *    Encoder params used to create the encoder.
		 * @return pointer to an instance of CAriMp4spencWrapper
		 */
		IMPORT_C static CAriMp4spencWrapper* NewL(TMpeg4H263EncoderInitParams &aParams);

		/**> Destructor */
		virtual ~CAriMp4spencWrapper();


	public:// MBaseCodec functions

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
		virtual TInt DoProcessL( TAny *aInpBuf, TAny* aOutBuf) = 0;

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

		virtual TInt SetParam( TInt aCommand, TAny* aCmdData ) = 0;

		/**
		 * Used to get codec parameteres
		 * @param aCommand
		 *    Indicates the encoder parameter to get
		 * @param aCmdData
		 *    The value of the encoder parameter (OUT)
		 * @return symbian wide error code
		*/
		virtual TInt GetParam (TInt aCommand, TAny* aCmdData) = 0;


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

	};

#endif //ARIMP4SPENCWRAPPER_H
