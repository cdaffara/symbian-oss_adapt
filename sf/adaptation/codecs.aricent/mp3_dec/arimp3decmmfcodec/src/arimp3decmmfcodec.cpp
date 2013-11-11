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
* Implementation of member functions of Plugin class (CAriMp3DecMmfCodec).
*
*/

// System includes
#include <ecom.h>
#include <ImplementationProxy.h>
// User includes
#include "arimp3decwrapper.h"
#include "arimp3decmmfcodec.h"
#include "arimp3decmmfcodec_uid.hrh"
#include "ariprint.h"

//Maximum size of a single input frame in an Mp3 stream
const TInt KMinBytesInput = 1440;


// ---------------------------------------------------------------------------
//  Two-phased constructor.
//  Creates an instance of CAriMp3DecMmfCodec.
//  Instance is not left on cleanup stack.
// ---------------------------------------------------------------------------
//

CMMFCodec* CAriMp3DecMmfCodec::NewL()
	{
	PRINT_ENTRY;
	CAriMp3DecMmfCodec* self = new ( ELeave ) CAriMp3DecMmfCodec();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	PRINT_EXIT;
	return ( CMMFCodec* )self;
	}

// ---------------------------------------------------------------------------
//  Destructor;Destroys the decoder instance and any internal buffers
// ---------------------------------------------------------------------------
//

CAriMp3DecMmfCodec::~CAriMp3DecMmfCodec()
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

// ---------------------------------------------------------------------------
//  From class CMMFCodec.
//  The function Sets codec configuration.
//  The value used for aConfigType must be KUidMmfCodecAudioSettings
//  (defined in include\mmf\plugins\mmfCodecImplementationUIDs.hrh)
// ---------------------------------------------------------------------------
//

 void CAriMp3DecMmfCodec::ConfigureL( TUid /*aConfigType*/,
		const TDesC8& /*aParam*/ )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	}

 // --------------------------------------------------------------------------
 //  From class CMMFCodec.
 //  This function is used to flush out status information when a
 //  reposition occurs.
 //  This is used if the codec requires resetting prior to use.
 // --------------------------------------------------------------------------
 //

 void CAriMp3DecMmfCodec::ResetL()
	{
	PRINT_ENTRY;
	iInternalInputBufferResidueLen = 0;
	iInternalOutputBufferPos = 0;
	iInternalOutputBufferResidueLen = 0;
	PRINT_EXIT;
	}

 //---------------------------------------------------------------------------
 //  From class CMMFCodec.
 //  This function is used to decode the given source and fill the
 //  destination buffer with the decode data.
 //  The buffers can be of any size.  Since the buffers can be of any size
 //  there is no guarantee that all the source buffer can be processed to
 //  fill the destination buffer or that the all the source buffer may be
 //  processed before the destination is full.  Therefore the ProcessL needs
 //  to return a TCodecProcessResult returing the number of source bytes
 //  processed and the number of destination bytes processed along with a
 //  process result code defined thus:
 //  - EProcessComplete: the codec processed all the source data into the
 //  sink buffer
 //  - EProcessIncomplete: the codec filled sink buffer before all the source
 //   buffer was processed
 //  - EDstNotFilled: the codec processed the source buffer but the sink
 //  buffer was not filled
 //  - EEndOfData: the codec detected the end data - all source data in
 //  processed but sink may not be full
 //  - EProcessError: the codec process error condition
 //
 //  The ProcessL should start processing the source buffer from the iPosition
 //  data member of the source data and start filling the destination buffer
 //   from its iPosition.
 //---------------------------------------------------------------------------
 //

TCodecProcessResult CAriMp3DecMmfCodec::ProcessL( const CMMFBuffer& aSrc,
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
	if ( ( KMinBytesInput - iInternalInputBufferResidueLen > 0 )
							&& ( srcBufferLen - srcBufferPos > 0 ) )
		{
		numberOfBytesCopied = CopyFromSrcBuffer( src, totalSrcBytesCopied );
		}

	if ( ( KMinBytesInput > iInternalInputBufferResidueLen )
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
		/**
		* call seeksync to find the valid fram start offset i.e syncpos.
		* update internal buffer position to that offset position if it is
		* success.if it is failed then there is no valid frame start in
		* the available buffer. Go for new src buffer. all bytes present
		* in the internal buffer are discarded.
		*/
		TInt syncpos;
		TInt srcBufferRemainingBytes = srcBufferLen
									  - srcBufferPos
									  - totalSrcBytesCopied;
		TInt dstBufferRemainingBytes = dstMaxLen
									   - dstBufferPos
									   - totalDstBytesAdded;
		TInt internalInputBufferPos = 0;

		if ( KErrNone != iCodec->SeekSync(
				&iInternalInputBuffer[internalInputBufferPos],
				( iInternalInputBufferResidueLen - internalInputBufferPos ),
					syncpos ) )
			{
			TCodecProcessResult result = GetNewData( src, totalSrcBytesCopied,
														totalDstBytesAdded );

			if ( result.iStatus == TCodecProcessResult::EDstNotFilled ||
					 result.iStatus == TCodecProcessResult::EEndOfData )
				{
				return result;
				}
			}
		else
			{
			syncFound = ETrue;
			internalInputBufferPos += syncpos;
			}
		/**
		* call GetFrameInfo to find whether valid frame is present in the
		* availabel buffer.if it is success and framelength is 0 then
		* there is no valid frame is present,  discard the present buffer
		* till sync position and add new buffer.
		*/
		if ( syncFound )
			{
			TInt frameLen;
			TMp3FrameInfo frameInfo;
			if ( KErrNone != iCodec->GetFrameInfo(
				&iInternalInputBuffer[internalInputBufferPos],
				( iInternalInputBufferResidueLen - internalInputBufferPos ),
				frameLen, frameInfo ) )
				{
				PRINT_ERR( "Decoder Getframeinfo failed" );
				User::Leave( KErrGeneral );
				}
			if ( frameLen == 0 )
				{
				TCodecProcessResult result = GetNewData( src,
									totalSrcBytesCopied, totalDstBytesAdded );

				if ( result.iStatus == TCodecProcessResult::EDstNotFilled ||
						 result.iStatus == TCodecProcessResult::EEndOfData )
					{
					return result;
					}
				}

			/**
			* if the buffer has less than framelen then fill the internal
			* sorce buffer with KMinBytesInput bytes.
			*/
			if ( frameLen   > ( iInternalInputBufferResidueLen
								- internalInputBufferPos ) )
				{
				if( lastFrame )
					{
					iInternalInputBufferResidueLen = 0;
					iInternalOutputBufferResidueLen = 0;
					iInternalOutputBufferPos = 0;
					PRINT_EXIT;
					return Result(
							TCodecProcessResult::EEndOfData,
							totalSrcBytesCopied, totalDstBytesAdded );
					}

				ShiftData( internalInputBufferPos, 0 );
				numberOfBytesCopied = CopyFromSrcBuffer( src,
														totalSrcBytesCopied );
				internalInputBufferPos = 0;

				if ( iInternalInputBufferResidueLen < KMinBytesInput )
					{
					PRINT_EXIT;
					return Result(
							TCodecProcessResult::EDstNotFilled,
							totalSrcBytesCopied, totalDstBytesAdded );
					}
				}

			/**
			* initialize the variables like srcUsed and dstLen accordingly.
			* call Decode.
			*/

			TInt srcUsed = iInternalInputBufferResidueLen
							- internalInputBufferPos;
			TInt dstLen  = iOutFrameSize;

			TInt error = iCodec->Decode(
						   &iInternalInputBuffer[internalInputBufferPos],
									srcUsed, iInternalOutputBuffer, dstLen );

			if ( KErrNone != error )
				{
				iInternalInputBufferResidueLen = 0;
				PRINT_ERR( error );
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
			numberOfBytesCopied = CopyFromSrcBuffer( src,
													totalSrcBytesCopied );
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
						return Result(
									TCodecProcessResult::EProcessIncomplete,
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
							return Result(
									TCodecProcessResult::EEndOfData,
									totalSrcBytesCopied, totalDstBytesAdded );
							}
						}
					else
						{
						PRINT_EXIT;
						return Result(
								TCodecProcessResult::EProcessComplete,
								totalSrcBytesCopied, totalDstBytesAdded );
						}
					}
				}
			}
	}while ( 1 );
	}

//----------------------------------------------------------------------------
//  Default constructor for performing 1st stage construction.
//  Should not contain any code that could leave.
//----------------------------------------------------------------------------
//

CAriMp3DecMmfCodec::CAriMp3DecMmfCodec():
iCodec( NULL ), iInternalInputBufferResidueLen( 0 ),
iInternalOutputBufferResidueLen( 0 ), iOutFrameSize( 0 ),
iInternalInputBuffer( NULL ), iInternalOutputBuffer( NULL ),
iInternalOutputBufferPos( 0 )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	}

//----------------------------------------------------------------------------
//  Second phase of the two-phased constructor.
//  Creates an instance of decoder
//----------------------------------------------------------------------------
//

void CAriMp3DecMmfCodec::ConstructL()
	{
	PRINT_ENTRY;
	TMp3FrameInfo frameInfo;

	TRAPD( creatErr,iCodec = CAriMp3DecWrapper::NewL() );
	if( KErrNone != creatErr )
		{
		PRINT_ERR( creatErr );
		User::Leave( creatErr );
		}

	User::LeaveIfError( iCodec->Reset() );

	TInt frameLength;

	User::LeaveIfError( iCodec->GetMaxFrameInfo( frameLength, frameInfo ) );

	iOutFrameSize = frameInfo.iBufferSize;

	iInternalInputBuffer = new( ELeave ) TUint8[KMinBytesInput];
	if ( !iInternalInputBuffer )
		{
		PRINT_ERR( "iInternalInputBuffer is not created" );
		User::Leave( KErrNoMemory );
		}

	iInternalOutputBuffer = new( ELeave ) TUint8[iOutFrameSize];
	if ( !iInternalOutputBuffer )
		{
		PRINT_ERR( "iInternalOutputBuffer is not created" );
		User::Leave( KErrNoMemory );
		}
	}

//----------------------------------------------------------------------------
//	Updates the result of the processing
//----------------------------------------------------------------------------
//
TCodecProcessResult CAriMp3DecMmfCodec::Result(
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
 TInt CAriMp3DecMmfCodec::CopyToDstBuffer( CMMFDataBuffer* aDst,
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
 TInt CAriMp3DecMmfCodec::CopyFromSrcBuffer( const CMMFDataBuffer* aSrc,
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
	TInt internalInputBufferRemaingBytes = KMinBytesInput
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
void CAriMp3DecMmfCodec::ShiftData( TInt aFromPos, TInt aToPos )
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
//---------------------------------------------------------------------------
// Gets the new data from the source buffer.
// checks whether it is last buffer from the source buffer and copies the data
// and then checks if source buffer remianing bytes are not and returns the
// result of the processing.
//---------------------------------------------------------------------------
//
TCodecProcessResult CAriMp3DecMmfCodec::GetNewData(
							const CMMFDataBuffer* aSrc,
							TInt &aSrcBytesConsumed, TInt &aDstBytesConsumed )
	{
	PRINT_ENTRY;
	TUint8* srcPtr = const_cast <TUint8*>( aSrc->Data().Ptr() );
	TInt srcBufferLen = aSrc->Data().Length();
	TInt srcBufferPos = aSrc->Position();
	TBool lastFrame = aSrc->LastBuffer();
	// calculate source buffer remaining bytes
	TInt srcBufferRemainingBytes = srcBufferLen - srcBufferPos
								   - aSrcBytesConsumed;

	TCodecProcessResult result;
	result.iStatus = TCodecProcessResult::EProcessError;

	// if it is last frame return end of data
	if ( ( lastFrame ) && ( srcBufferRemainingBytes == 0 ) )
		{
		iInternalInputBufferResidueLen = 0;
		iInternalOutputBufferResidueLen = 0;
		iInternalOutputBufferPos = 0;
		PRINT_EXIT;
		return Result( TCodecProcessResult::EEndOfData,
						aSrcBytesConsumed, aDstBytesConsumed );
		}
	else
	{
		iInternalInputBufferResidueLen = 0;
		TInt numberOfBytesToBeCopied = CopyFromSrcBuffer( aSrc,
														 aSrcBytesConsumed );

		if ( iInternalInputBufferResidueLen < KMinBytesInput )
			{
			PRINT_EXIT;
			return Result( TCodecProcessResult::EDstNotFilled,
							aSrcBytesConsumed, aDstBytesConsumed );
			}
	}
	PRINT_EXIT;
	return result;
	}
/* __________________________________________________________________________
* Exported proxy for instantiation method resolution
* Define the interface UIDs
*/

const TImplementationProxy ImplementationTable[] =
	{
	IMPLEMENTATION_PROXY_ENTRY( KUidMp3DecCodecImplUid,
								CAriMp3DecMmfCodec::NewL )
	};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(
														TInt& aTableCount )
{
	PRINT_ENTRY;
	aTableCount = sizeof( ImplementationTable )
					/ sizeof( TImplementationProxy );
	PRINT_EXIT;
	return ImplementationTable;
}

//---------------------------------------------------------------------------
//  End of File
//---------------------------------------------------------------------------
