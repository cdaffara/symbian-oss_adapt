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
* Plugin class for Mp3 decoder. This class inherits CMMFCodec 
* class and implements the pure virtual functions of CMMFCodec.
* The class also has a few private funtions.
*
*/

#ifndef ARIMP3DECMMFCODEC_H
#define ARIMP3DECMMFCODEC_H

// system include files
#include <MmfCodec.h>
// user include files
#include "arimp3decmmfcodec_uid.hrh"

// Forward Declaration
class CAriMp3DecWrapper;

// Class declaration
class CAriMp3DecMmfCodec :public CMMFCodec
	{

public: //constructors and destructors
	/**
	 * Two-phased constructor.
	 * @return Pointer to an instance of CMMFCodec
	 */
	static CMMFCodec* NewL();

	/**> Destructor */
	~CAriMp3DecMmfCodec();

public: // From CMMFCodec

	/**
	 * From CMMFCodec
	 * Sets codec configuration.
	 * The configuration data is passed in as a descriptor of type TDesC8.
	 * @param aConfigType
	 * The UID of the configuration data. 
	 * @param aParam
	 * Descriptor with frame info parameters in the form of RArray
	 */

	 virtual void ConfigureL( TUid aConfigType, const TDesC8& aParam );

	 /**
	  * From CMMFCodec
	  * Codec reset function used to flush out status information when a 
	  *	reposition occurs. 
	  *	This is used if the codec requires resetting prior to use.
	  */
	 virtual void ResetL();

	 /**
	  * From CMMFCodec
	  * Processes the data in the specified source buffer and writes the 
	  * processed data to the specified destination buffer.
	  * This function is synchronous, when the function returns the data 
	  * has been processed. The source buffer is converted to the appropriate 
	  * coding type in the destination buffer. The buffers can be of any 
	  * size, therefore there is no guarantee that all the source buffer can 
	  * be processed to fill the destination buffer or that all the source 
	  * buffer may be processed before the destination is full. This function
	  * therefore returns the number of source, and destination, bytes 
	  * processed along with a process result code indicating completion 
	  * status.This function is synchronous, when the function returns the 
	  * data has been processed. The source buffer is converted to the 
	  * appropriate coding type in the destination buffer. The buffers can be
	  * of any size, therefore there is no guarantee that all the source 
	  * buffer can be processed to fill the destination buffer or that all 
	  * the source buffer may be processed before the destination is full. 
	  * This function therefore returns the number of source,and destination,
	  * bytes processed along with a process result code indicating completion
	  * status.The aSource and aSink buffers passed in are derived from 
	  * CMMFBuffer. The buffer type (e.g. a CMMFDataBuffer) passed in should 
	  * be supported by the codec otherwise this function should leave with 
	  * KErrNotSupported. The position of the source buffer should be checked
	  * by calling the source buffer's Position() member which indicates the 
	  * current source read position in bytes. The codec should start 
	  * processing from the current source buffer read position. The position
	  * of the destination buffer should be checked by calling the destination
	  * buffer's Position() method which indicates the current destination 
	  * write position in bytes. The codec should start writing to the 
	  * destination buffer at the current destination buffer write position.
	  * This is a virtual function that each derived class must implement.
	  * @see enum TCodecProcessResult 
	  * @param  aSource
	  *         The source buffer containing data to encode or decode.
	  *	@param  aDestination 
	  *         The destination buffer to hold the data after encoding or 
	  *         decoding.
	  * @return The result of the processing.
	  * @pre    The function ConfigureL() should have been called.
	  */

	 virtual TCodecProcessResult ProcessL( const CMMFBuffer& aSource, 
										 CMMFBuffer& aDestination );

private:
	/**
	 *  Default Constructor
	 */
	CAriMp3DecMmfCodec();
	/**
	 *  Symbian 2nd phase constructor .
	 */
	void ConstructL();
	
	/**
	 * Update the result with result status, source bytes consumed
	 * and destination bytes added.
	 * @param 	aStatus
	 * 		  	status of the result like EProcessComplete
	 * @param 	aSrcBytesConsumed
	 * 			total bytes consumed from the source buffer
	 * @param	aDstBytesAdded
	 * 			total bytes added to the destination buffer
	 * @return	result of the processing
	 */	
	TCodecProcessResult Result(
			TCodecProcessResult::TCodecProcessResultStatus aStatus,
			TInt aSrcBytesConsumed,TInt aDstBytesAdded);
	
	/**
	 * Copy the bytes from internal output buffer to destination buffer
	 * @param	aDst
	 * 			pointer to the destination buffer
	 * @param	aDstBytesConsumed
	 * 			total bytes added to the destination buffer
	 * @return 	number of bytes copied to the destination buffer.
	 */
	TInt CopyToDstBuffer( CMMFDataBuffer* aDst, TInt& aDstBytesConsumed);
	
	/**
	 * Copy the bytes from the source buffer to the internal input burrer
	 * @param	aSrc
	 * 			pointer to the source buffer
	 * @param	aSrcBytesConsumed
	 * 			total bytes consumed from the source buffer
	 * @return 	number bytes copied from the source buffer
	 */
	TInt CopyFromSrcBuffer(const CMMFDataBuffer* aSrc,
						TInt& aSrcBytesConsumed);
	/**
	 * Shifts the data in the input internal buffer to start position
	 * @param	aFromPos
	 * 			position from where data has to shift
	 * @param	aToPos
	 * 			position to where data has to shift
	 */
	void ShiftData( TInt aFromPos, TInt aToPos );
	
	/**
	 * Gets the new data from the source buffer
	 * @param	aSrc
	 * 			pointer to the source buffer
	 * @param	aDstBytesConsumed
	 * 			total number bytes added to the destination buffer
	 * @param	aSrcBytesConsumed
	 * 			total number bytes consumed from the source buffer
	 * @return	returns the processin result
	 */
	TCodecProcessResult GetNewData(const CMMFDataBuffer* aSrc,
				TInt &aDstBytesConsumed, TInt &aSrcBytesConsumed);

private: //Data
	
	// handle to the wrapper instance
	CAriMp3DecWrapper* 	iCodec;		
	// Residue input buffer length
	TInt				iInternalInputBufferResidueLen;
	// Residue output buffer length
	TInt				iInternalOutputBufferResidueLen;
	// max output buffer size
	TInt				iOutFrameSize;
  	// Input internal buffer
	TUint8 				*iInternalInputBuffer;
  	// Output internal buffer
	TUint8 				*iInternalOutputBuffer;
	// internal output buffer position
	TInt 				iInternalOutputBufferPos;

	};

#endif \\ ARIMP3DECMMFCODEC_H \\

//-----------------------------------------------------------------------------
//  End of File
//-----------------------------------------------------------------------------
