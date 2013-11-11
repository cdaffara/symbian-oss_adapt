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
* Implementation of member functions of Plugin class (CAriAmrWbDecMmfCodec).
*
*/


// System includes
#include <ecom.h>
#include <implementationproxy.h>
// User includes
#include "ariamrwbdecwrapper.h"
#include "ariamrwbdecmmfcodec.h"
#include "ariamrwbdecmmfcodec_uid.hrh"
#include "ariprint.h"

// Maximum input and output buffer length
const TUint KAMRWBMINOutBufLength = 640;


//---------------------------------------------------------------------------
//  Two-phased constructor.
//  Creates an instance of CAriAmrWbDecMmfCodec.
//  Instance is not left on cleanup stack.
//---------------------------------------------------------------------------
//
CMMFCodec* CAriAmrWbDecMmfCodec::NewL()
	{
	PRINT_ENTRY;
	CAriAmrWbDecMmfCodec* self = new ( ELeave ) CAriAmrWbDecMmfCodec();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	PRINT_EXIT;
	return ( CMMFCodec* )self;
	}

//---------------------------------------------------------------------------
//  Destructor;Destroys the decoder instance and any internal buffers
//---------------------------------------------------------------------------
//
CAriAmrWbDecMmfCodec::~CAriAmrWbDecMmfCodec()
	{
	PRINT_ENTRY;
	if ( iInternalInputBuffer )
		{
		delete iInternalInputBuffer;
		iInternalInputBuffer = NULL;
		}
	if ( iInternalOutputBuffer )
		{
		delete iInternalOutputBuffer;
		iInternalOutputBuffer = NULL;
		}

	if ( iCodec )
		{
		delete iCodec;
		iCodec = NULL;
		}

	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  From class CMMFCodec.
//  The function Sets codec configuration.
//  The value used for aConfigType must be KUidMmfCodecAudioSettings
//  (defined in include\mmf\plugins\mmfCodecImplementationUIDs.hrh)
//---------------------------------------------------------------------------
//
void CAriAmrWbDecMmfCodec::ConfigureL( TUid /* aConfigType */,
									const TDesC8& /*aParam*/ )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	}
//---------------------------------------------------------------------------
//  From class CMMFCodec.
//  This function is used to flush out status information when a
//  reposition occurs.
//  This is used if the codec requires resetting prior to use.
//---------------------------------------------------------------------------
//
void CAriAmrWbDecMmfCodec::ResetL()
	{
	PRINT_ENTRY;
	iInternalInputBufferResidueLen = 0;
	iInternalOutputBufferPos = 0;
	iInternalOutputBufferResidueLen = 0;
	PRINT_EXIT;
	}


//---------------------------------------------------------------------------
// From class CMMFCodec.
// This function is used to decode the given source and fill the destination
// buffer with the decode data.
// The buffers can be of any size.  Since the buffers can be of any size there
// is no guarantee that all the source buffer can be processed to fill the
// destination buffer or that the all the source buffer may be processed
// before the destination is full.  Therefore the ProcessL needs to return a
// TCodecProcessResult returing the number of source bytes processed and the
// number of destination bytes processed along with a process result code
// defined thus:
// - EProcessComplete: the codec processed all the source data into the sink
//	 buffer
// - EProcessIncomplete: the codec filled sink buffer before all the source
//   buffer was processed
// - EDstNotFilled: the codec processed the source buffer but the sink buffer
//   was not filled
// - EEndOfData: the codec detected the end data - all source data in
//   processed but sink may not be full
// - EProcessError: the codec process error condition
//
// The ProcessL should start processing the source buffer from the iPosition
// data member of the source data and start filling the destination buffer
// from its iPosition.
//---------------------------------------------------------------------------
//
TCodecProcessResult CAriAmrWbDecMmfCodec::ProcessL( const CMMFBuffer& aSrc,
													CMMFBuffer& aDst )
	{
	PRINT_ENTRY;

	// total decoded bytes added to the dst buffer
	TInt totalDstBytesAdded = 0;
	// total src bytes added to the internal src buffer
	TInt totalSrcBytesCopied = 0;
	// temporary variable to use for copying the sorce or destination data
	TInt numberOfBytesCopied;
	// Flag for finding valid sync
	TBool syncFound = EFalse;

	/**
	* Process the dst buffer, update the dstBufferPos and check
	* whether dst buffer is NULL or not.
	*/
	CMMFDataBuffer* dst = static_cast<CMMFDataBuffer*>( &aDst );

	const TInt dstMaxLen = dst->Data().MaxLength();
	TUint8* dstPtr = const_cast<TUint8*>( dst->Data().Ptr() );
	TInt dstBufferPos = dst->Position();

	/**
	* Process the src buffer, update srcbuffer length, position and
	* flag for last frame. check whether src buffer is NULL or not
	* and check src buffer contains any data
	*/
	const CMMFDataBuffer* src = static_cast<const CMMFDataBuffer*>( &aSrc );

	TUint8* srcPtr = const_cast <TUint8*>( src->Data().Ptr() );
	TInt srcBufferLen = src->Data().Length();
	TInt srcBufferPos = src->Position();
	TBool lastFrame = src->LastBuffer();

	if ( srcBufferLen == 0 && iInternalInputBufferResidueLen == 0 )
		{
		PRINT_ERR( "source buffer length is zero" );
		User::Leave( KErrArgument );
		}

	/**
	* if any destination bytes from internal destination buffer is not
	* given to the dst buffer from the previous call, give it to the
	* dst buffer. After this block, it ensures that no bytes are remaining
	* in the internal destination buffer.
	*/
	if ( iInternalOutputBufferResidueLen - iInternalOutputBufferPos > 0 )
		{
		numberOfBytesCopied = CopyToDstBuffer( dst, totalDstBytesAdded );

		if ( iInternalOutputBufferResidueLen - iInternalOutputBufferPos > 0 )
			{
			PRINT_EXIT;
			return Result( TCodecProcessResult::EProcessIncomplete,
							totalSrcBytesCopied, totalDstBytesAdded );
			}
		else
			{
			if ( ( lastFrame ) && ( srcBufferLen - srcBufferPos == 0 )&&
								( iInternalInputBufferResidueLen == 0 ) )
				{
				iInternalOutputBufferResidueLen = 0;
				iInternalInputBufferResidueLen = 0;
				iInternalOutputBufferPos = 0;
				PRINT_EXIT;
				return Result( TCodecProcessResult::EEndOfData,
								totalSrcBytesCopied, totalDstBytesAdded );
				}
			iInternalOutputBufferPos = 0;
			iInternalOutputBufferResidueLen = 0;
			}
		}

	/**
	* copy the src buffer data into the internal buffer till internal buffer
	* holds minimum bytes to process i.e KMinBytesInput. After this block, it
	* ensures that internal source buffer holds KMinBytesInput.
	* if it is a last frame, treat remaining residual buffer as internal
	* buffer.
	*/
	if ( ( KAMRWBMINOutBufLength - iInternalInputBufferResidueLen > 0 )
							&& ( srcBufferLen - srcBufferPos > 0 ) )
		{
		numberOfBytesCopied = CopyFromSrcBuffer( src, totalSrcBytesCopied );
		}

	if ( ( KAMRWBMINOutBufLength > iInternalInputBufferResidueLen )
														&& ( !lastFrame ) )
		{
		PRINT_EXIT;
		return Result( TCodecProcessResult::EDstNotFilled,
						srcBufferLen - srcBufferPos, totalDstBytesAdded );
		}

	/**
	* process the src buffer till destination buffer or source buffer
	* or both buffers are exhausted.
	*/
	do
	{
		TInt srcBufferRemainingBytes = srcBufferLen
									  - srcBufferPos
									  - totalSrcBytesCopied;
		TInt dstBufferRemainingBytes = dstMaxLen
									   - dstBufferPos
									   - totalDstBytesAdded;
		TInt internalInputBufferPos = 0;


		/**
		* initialize the variables like srcUsed and dstLen accordingly.
		* call Decode.
		*/

		TInt32 srcUsed = iInternalInputBufferResidueLen
						- internalInputBufferPos;
		TInt32 dstLen  = KAMRWBMINOutBufLength;

		TInt error =
		iCodec->Decode( &iInternalInputBuffer[internalInputBufferPos],
						srcUsed,( TInt32* )iInternalOutputBuffer, dstLen );

		if ( KErrNone != error )
			{
			iInternalInputBufferResidueLen = 0;
			PRINT_ERR( "Amr Wb Decoder decoding is failed" );
			return Result(
					TCodecProcessResult::EProcessError,
					totalSrcBytesCopied, totalDstBytesAdded );
			}

		/**
		* Fill Destination Buffer
		*/

		iInternalOutputBufferResidueLen = dstLen;
		numberOfBytesCopied = CopyToDstBuffer( dst, totalDstBytesAdded );
		dstBufferRemainingBytes -= numberOfBytesCopied;

		/***
		* Fill Sorce Buffer
		*/

		internalInputBufferPos += srcUsed ;
		ShiftData( internalInputBufferPos, 0 );
		numberOfBytesCopied = CopyFromSrcBuffer( src, totalSrcBytesCopied );
		srcBufferRemainingBytes -= numberOfBytesCopied;
		internalInputBufferPos = 0;

		/***
		* check four conditions if else for src and if else for dst
		*/

		// src buffer has available bytes to decode
		if ( srcBufferRemainingBytes > 0 )
			{
			if ( dstBufferRemainingBytes == 0 )
				{
				PRINT_EXIT;
				return Result( TCodecProcessResult::EProcessIncomplete,
						totalSrcBytesCopied, totalDstBytesAdded );
				}
			}

		else
			{
			// dst buffer has availabe space for decoded bytes
			if ( dstBufferRemainingBytes > 0 )
				{
				// last frame of the input stream
				if ( lastFrame )
					{
					if ( iInternalInputBufferResidueLen == 0 )
						{
						PRINT_EXIT;
						return Result( TCodecProcessResult::EEndOfData,
								totalSrcBytesCopied, totalDstBytesAdded );
						}
					}
				else
					{
					PRINT_EXIT;
					return Result( TCodecProcessResult::EDstNotFilled,
							totalSrcBytesCopied, totalDstBytesAdded );
					}
				}
			else
				{
				/**
				 *internal output buffer has decoded bytes which is not
				 *given to dst buffer.
				 */
				if ( iInternalOutputBufferResidueLen
									- iInternalOutputBufferPos > 0 )
					{
					PRINT_EXIT;
					return Result( TCodecProcessResult::EProcessIncomplete,
									totalSrcBytesCopied, totalDstBytesAdded );
					}
				// last frame of the input stream
				else if ( lastFrame )
					{
					// if internal buffer has available bytes to decode
					if ( iInternalInputBufferResidueLen > 0 )
						{
						PRINT_EXIT;
						return Result(
								TCodecProcessResult::EProcessIncomplete,
								totalSrcBytesCopied, totalDstBytesAdded );
						}
					else
						{
						iInternalInputBufferResidueLen = 0;
						PRINT_EXIT;
						return Result( TCodecProcessResult::EEndOfData,
									totalSrcBytesCopied, totalDstBytesAdded );
						}
					}
				else
					{
					PRINT_EXIT;
					return Result( TCodecProcessResult::EProcessComplete,
									totalSrcBytesCopied, totalDstBytesAdded );
					}
				}
			}
	}while ( 1 );
	}



//---------------------------------------------------------------------------
//  Default constructor for performing 1st stage construction.
//  Should not contain any code that could leave.
//---------------------------------------------------------------------------
//
CAriAmrWbDecMmfCodec::CAriAmrWbDecMmfCodec():
iCodec( NULL ),	iInternalInputBuffer( NULL ),
iInternalOutputBuffer( NULL ), iInternalInputBufferResidueLen( 0 ),
iInternalOutputBufferResidueLen( 0 ), iInternalOutputBufferPos( 0 )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Destructor;Destroys the decoder instance and any internal buffers
//---------------------------------------------------------------------------
//
void CAriAmrWbDecMmfCodec::ConstructL()
	{
	PRINT_ENTRY;

	iCodec = CAriAmrWbDecWrapper::NewL();

	if ( !iCodec )
		{
		PRINT_ERR( "Amr Nb Decoder creation is failed" );
		User::Leave( KErrGeneral );
		}

	User::LeaveIfError( iCodec->Reset() ) ;

	iInternalInputBuffer = new( ELeave ) TUint8[KAMRWBMINOutBufLength];
	if ( !iInternalInputBuffer )
		{
		PRINT_ERR( "internal input buffer creation is failed" );
		User::Leave( KErrNoMemory );
		}

	iInternalOutputBuffer = new( ELeave ) TUint8[KAMRWBMINOutBufLength];
	if ( !iInternalOutputBuffer )
		{
		PRINT_ERR( "internal output buffer creation is failed" );
		User::Leave( KErrNoMemory );
		}

	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//	Updates the result of the processing
//----------------------------------------------------------------------------
//
TCodecProcessResult CAriAmrWbDecMmfCodec::Result(
					TCodecProcessResult::TCodecProcessResultStatus aStatus,
 					TInt aSrcBytesConsumed, TInt aDstBytesAdded )

	 {
	 PRINT_ENTRY;
	 TCodecProcessResult result;
	 // update destination bytes
	 result.iDstBytesAdded = aDstBytesAdded;
	 // update source bytes
	 result.iSrcBytesProcessed = aSrcBytesConsumed;

	 // update status
	 switch ( aStatus )
		 {
		 case TCodecProcessResult::EProcessComplete:
			result.iStatus = TCodecProcessResult::EProcessComplete;
			break;
		 case TCodecProcessResult::EProcessIncomplete:
		 	result.iStatus = TCodecProcessResult::EProcessIncomplete;
		 	break;
		 case TCodecProcessResult::EEndOfData:
		 	result.iStatus = TCodecProcessResult::EEndOfData;
		 	break;
		 case TCodecProcessResult::EDstNotFilled:
		 	result.iStatus = TCodecProcessResult::EDstNotFilled;
		 	break;
		 case TCodecProcessResult::EProcessError:
		 	result.iStatus = TCodecProcessResult::EProcessError;
		 	break;
		 default:
			result.iStatus = TCodecProcessResult::EProcessError;
			break;
		 }
	 PRINT_MSG( LEVEL_HIGH, ( "result.iSrcBytesProcessed = %d",
								result.iSrcBytesProcessed ) );
	 PRINT_MSG( LEVEL_HIGH, ( "result.iDstBytesAdded = %d",
								result.iDstBytesAdded ) );
	 PRINT_MSG( LEVEL_HIGH, ( "result.iStatus = %d",
								result.iStatus ) );
	 PRINT_EXIT;
	 return result;
	 }

//----------------------------------------------------------------------------
// Copy the bytes to destination buffer from the internal buffer
// first checks whether the number of bytes to be copied is lesser of the
// destination buffer reamining bytes and internal input internal remaining
// remaining bytes and then copies that many bytes.
//----------------------------------------------------------------------------
//
 TInt CAriAmrWbDecMmfCodec::CopyToDstBuffer( CMMFDataBuffer* aDst,
											 TInt &aDstBytesConsumed )
	{
	PRINT_ENTRY;
	TInt numberOfBytesToBeCopied;
	const TInt dstMaxLen = aDst->Data().MaxLength();
	TUint8* dstPtr = const_cast<TUint8*>( aDst->Data().Ptr() );
	TInt dstBufferPos = aDst->Position();

	// destination buffer remaining bytes
	TInt dstBufferRemainingBytes = dstMaxLen
								   - dstBufferPos
								   - aDstBytesConsumed;
	// internal output buffer remaining bytes
	TInt internalOutputBufferRemainingBytes =
										 iInternalOutputBufferResidueLen
										 - iInternalOutputBufferPos;

	if ( internalOutputBufferRemainingBytes > dstBufferRemainingBytes )
		{
		numberOfBytesToBeCopied = dstBufferRemainingBytes;
		}
	else
		{
		numberOfBytesToBeCopied = internalOutputBufferRemainingBytes;
		iInternalOutputBufferResidueLen = 0;
		}

	// copy data to destination buffer from internal ouput buffer
	Mem::Copy( dstPtr + dstBufferPos + aDstBytesConsumed,
		iInternalOutputBuffer + iInternalOutputBufferPos,
		numberOfBytesToBeCopied );

	// update internal output buffer position
	if( iInternalOutputBufferResidueLen )
		{
		iInternalOutputBufferPos += dstBufferRemainingBytes;
		}
	else
		{
		iInternalOutputBufferPos = 0;
		}

	aDstBytesConsumed += numberOfBytesToBeCopied;
	aDst->Data().SetLength( dstBufferPos +  aDstBytesConsumed );
	PRINT_EXIT;
	return numberOfBytesToBeCopied;
	}

//---------------------------------------------------------------------------
// Copy the bytes from the source buffer to the internal input buffer.
// first it checks number of bytes to be copied is lesser of the source buffer
// remaining bytes or internal input buffer remaining bytes and then copies
// that many bytes.
//---------------------------------------------------------------------------
//
 TInt CAriAmrWbDecMmfCodec::CopyFromSrcBuffer( const CMMFDataBuffer* aSrc,
												 TInt &aSrcBytesConsumed )
	{
	PRINT_ENTRY;
	TInt numberOfBytesToBeCopied;
	TUint8* srcPtr = const_cast <TUint8*>( aSrc->Data().Ptr() );
	TInt srcBufferLen = aSrc->Data().Length();
	TInt srcBufferPos = aSrc->Position();

	// calculate the source buffer remaining bytes
	TInt srcBufferRemainingBytes = srcBufferLen - srcBufferPos
								   - aSrcBytesConsumed;

	// calculate internal input buffer remaining bytes
	TInt internalInputBufferRemaingBytes = KAMRWBMINOutBufLength
										   - iInternalInputBufferResidueLen;

	if ( internalInputBufferRemaingBytes > srcBufferRemainingBytes )
		{
		numberOfBytesToBeCopied = srcBufferRemainingBytes;
		}
	else
		{
		numberOfBytesToBeCopied = internalInputBufferRemaingBytes;
		}

	// copy data from source buffer to internal input buffer
	Mem::Copy( iInternalInputBuffer + iInternalInputBufferResidueLen,
				srcPtr + srcBufferPos + aSrcBytesConsumed,
				numberOfBytesToBeCopied );

	// update internal input buffer residue length
	iInternalInputBufferResidueLen += numberOfBytesToBeCopied;
	aSrcBytesConsumed += numberOfBytesToBeCopied;
	PRINT_EXIT;
	return numberOfBytesToBeCopied;
	}
//---------------------------------------------------------------------------
// Moves the data of the internal input buffer to the start position
//---------------------------------------------------------------------------
//
void CAriAmrWbDecMmfCodec::ShiftData( TInt aFromPos, TInt aToPos )
	{
	PRINT_ENTRY;
	for ( TInt i = aToPos; i < ( iInternalInputBufferResidueLen - aFromPos );
																	i++ )
		{
		iInternalInputBuffer[i] = iInternalInputBuffer[i + aFromPos];
		}
	iInternalInputBufferResidueLen -= aFromPos;
	PRINT_EXIT;
	}

// __________________________________________________________________________
//    *********************   ECOM Instantiation *********************
// __________________________________________________________________________

const TImplementationProxy ImplementationTable[] =
	{
	IMPLEMENTATION_PROXY_ENTRY(KUidARIAMRWBDecImplUid,
												CAriAmrWbDecMmfCodec::NewL)
	};

EXPORT_C const TImplementationProxy*
								ImplementationGroupProxy( TInt& aTableCount )
	{
	PRINT_ENTRY;
	aTableCount = sizeof( ImplementationTable) /
											sizeof(TImplementationProxy );
	PRINT_EXIT;
	return ImplementationTable;
	}
