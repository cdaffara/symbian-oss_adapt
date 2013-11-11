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
* Declares interface class MBaseCodec whose pure virtual functions should be
* implemented by all video encoder/decoder wrappers
*
*/


#ifndef ARIBASECODEC_H
#define ARIBASECODEC_H

#include <E32def.h>

class MBaseCodec
	{
	public:
		/**
		 * This enumeration should be used to indicate the result
		 * of a call to DoProcessL
		 */
		enum TCodecState {
			EInputConsumed,
			EOutputConsumed,
			EConsumed,
			ENotConsumed
			};
	public:

		/**
		 * Processes an input buffer , returns one of the TCodecState
		 * @param aInpBuf
		 *    Coded input data
		 * @param aOutBuf
		 *    Decoded output data
		 * @leave The method will leave if an error occurs
		 * @return one of the TCodecState members
		 */
		virtual TInt DoProcessL( TAny *aInpBuf, TAny* aOutBuf = NULL ) = 0;

		/**
		 * Used to set codec parameteres
		 * @param aCommand
		 *    Indicates the encoder parameter to set
		 * @param aCmdData
		 *    The value to which the encoder parameter should be set
		 * @return symbian wide error code
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
		virtual TInt GetParam( TInt aCommand, TAny* aCmdData ) = 0;

		/**
		 * Cancels all processing of the commands
		 */
		virtual void Reset() = 0;

	};

#endif //ARIBASECODEC_H


