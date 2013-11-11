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
* Implementation of member functions of Plugin class
* (CAriAmrNbEncMmfCodec).
*
*/

#include <ecom.h>
#include <implementationproxy.h>

#include "ariamrnbencwrapper.h"
#include "ariamrnbencmmfcodec.h"
#include "ariamrnbencmmfcodec_uid.hrh"
#include "AriPrint.h"

//Minimum bytes that contain 160 samples that has to be encoded at a time
const TUint KMinBytesInput = 320;
const TUint KMinDstLen = 32;


// --------------------------------------------------------------------------
//  Two-phased constructor.
//  Creates an instance of CAriAmrNbEncMmfCodec.
//  Instance is not left on cleanup stack.
// --------------------------------------------------------------------------
//
CMMFCodec* CAriAmrNbEncMmfCodec::NewL()
    {
    PRINT_ENTRY;
    CAriAmrNbEncMmfCodec* self = new ( ELeave ) CAriAmrNbEncMmfCodec();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    PRINT_EXIT;
    return ( CMMFCodec* )self;
    }

// --------------------------------------------------------------------------
//  Destructor;Destroys the encoder instance and any internal buffers
// --------------------------------------------------------------------------
//
CAriAmrNbEncMmfCodec::~CAriAmrNbEncMmfCodec()
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
    iConfigured = EFalse;
    PRINT_EXIT;
    }


// --------------------------------------------------------------------------
//  From class CMMFCodec.
//  The function sets codec configuration.
//  value used for aConfigType must be KUidMmfCodecAudioSettings
//  (defined in include\mmf\plugins\mmfCodecImplementationUIDs.hrh)
//
// --------------------------------------------------------------------------
//
void CAriAmrNbEncMmfCodec::ConfigureL( TUid /* aConfigType */,
                                                const TDesC8& aParam )
    {
    PRINT_ENTRY;
    if ( !iConfigured )
        {
        TInt encMode = 0;
        TInt dtx = 0;
        TInt offset = 0;

        Mem::Copy( &encMode,  aParam.Ptr() + offset,  sizeof( TInt ) );
        offset += sizeof(TInt);

        Mem::Copy( &dtx,  aParam.Ptr() + offset,  sizeof( TInt ) );
        offset += sizeof(TInt);

        iParam.iMode = ( TInt16 )encMode;
        iParam.iDtx = ( TInt16 )dtx;

        PRINT_MSG( LEVEL_HIGH, ( "Mode: %d", iParam.iMode ) );
        PRINT_MSG( LEVEL_HIGH, ( "DTX: %d", iParam.iDtx ) );

        User::LeaveIfError( iCodec->Reset( &iParam ) );

        iConfigured = ETrue;
        }
    PRINT_EXIT;
    }

// ---------------------------------------------------------------------------
//  From class CMMFCodec.
//  This function is used to flush out status information when a
//  reposition occurs.
//  This is used if the codec requires resetting prior to use.
// ---------------------------------------------------------------------------
//
void CAriAmrNbEncMmfCodec::ResetL()
    {
    PRINT_ENTRY;
    User::LeaveIfError( iCodec->Reset( &iParam ) );
    PRINT_EXIT;
    }


// --------------------------------------------------------------------------
//  From class CMMFCodec.
//  This function is used to encode the given source and fill the destination
//  buffer with the encode data.
//  The buffers can be of any size.  Since the buffers can be of any size
//  there is no guarantee that all the source buffer can be processed to fill
//  the destination buffer or that the all the source buffer may be processed
//  before the destination is full.  Therefore the ProcessL needs to return a
//  TCodecProcessResult returing the number of source bytes processed and the
//  number of destination bytes processed along with a process result code
//  defined thus:
//  - EProcessComplete: the codec processed all the source data into the
//    sink buffer
//  - EProcessIncomplete: the codec filled sink buffer before all the source
//    buffer
//    was processed
//  - EDstNotFilled: the codec processed the source buffer but the sink
//    buffer was not filled
//  - EEndOfData: the codec detected the end data - all source data is
//    processed but sink may not be full
//  - EProcessError: the codec process error condition
//
//  The ProcessL should start processing the source buffer from the iPosition
//  data member of the source data and start filling the destination buffer
//  from its iPosition.
//  -------------------------------------------------------------------------
//
TCodecProcessResult CAriAmrNbEncMmfCodec::ProcessL( const CMMFBuffer& aSrc,
                                                        CMMFBuffer& aDst )
    {
    PRINT_ENTRY;

    // total encoded bytes added to the dst buffer
    TInt totalDstBytesAdded = 0;
    // total src bytes added to the internal src buffer
    TInt totalSrcBytesCopied = 0;
    // temporary variable to use for copying the sorce or destination data
    TInt numberOfBytesCopied = 0;
    TInt dstBufferRemainingBytes = 0;
    TInt srcBufferRemainingBytes = 0;

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

    PRINT_MSG( LEVEL_HIGH, ( "Src Buffer Pos: %d", srcBufferPos ) );
    PRINT_MSG( LEVEL_HIGH, ( "Dst Buffer Pos: %d", dstBufferPos ) );
    PRINT_MSG( LEVEL_HIGH, ( "Residue in internal output buffer: %d",
           iInternalOutputBufferResidueLen - iInternalOutputBufferPos ) );
    PRINT_MSG( LEVEL_HIGH, ( "Residue in internal input buffer: %d",
                                          iInternalInputBufferResidueLen ) );

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
            if ( lastFrame && ( srcBufferLen - srcBufferPos == 0 ) &&
                                    ( iInternalInputBufferResidueLen == 0 ) )
                {
                totalSrcBytesCopied = 0;
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
    if ( ( iSrclenToProcess - iInternalInputBufferResidueLen > 0 ) &&
                                        ( srcBufferLen - srcBufferPos > 0 ) )
        {
        numberOfBytesCopied = CopyFromSrcBuffer( src, totalSrcBytesCopied );
        }

    if ( ( iSrclenToProcess > iInternalInputBufferResidueLen ) &&
                                                            ( !lastFrame ) )
        {
        PRINT_EXIT;
        return Result( TCodecProcessResult::EDstNotFilled,
                srcBufferLen - srcBufferPos, totalDstBytesAdded );
        }
    srcBufferRemainingBytes = srcBufferLen - srcBufferPos -
                                                    totalSrcBytesCopied;
    dstBufferRemainingBytes = dstMaxLen - dstBufferPos - totalDstBytesAdded;

    if ( lastFrame && ( ( iSrclenToProcess > iInternalInputBufferResidueLen )
                        && ( iSrclenToProcess > srcBufferRemainingBytes ) )
                        && ( iInternalOutputBufferResidueLen -
                                        iInternalOutputBufferPos == 0 ) )
        {
        totalSrcBytesCopied = 0;
        iInternalOutputBufferResidueLen = 0;
        iInternalInputBufferResidueLen = 0;
        iInternalOutputBufferPos = 0;
        PRINT_EXIT;
        return Result( TCodecProcessResult::EEndOfData,
                                    totalSrcBytesCopied, totalDstBytesAdded );
        }

    if ( iInternalOutputBufferResidueLen - iInternalOutputBufferPos == 0 )
        {
        iInternalOutputBufferResidueLen = 0;
        iInternalOutputBufferPos = 0;
        }
    /**
    * process the src buffer till destination buffer or source buffer or
    * both buffers are exhausted.
    */
    do
        {
        srcBufferRemainingBytes = srcBufferLen - srcBufferPos -
                                                        totalSrcBytesCopied;
        dstBufferRemainingBytes = dstMaxLen - dstBufferPos -
                                                        totalDstBytesAdded;
        TInt internalInputBufferPos = 0;

        /**
        * initialize the variables like srcUsed and dstLen accordingly.
        * call encode.
        */
        TInt index;
        TInt srcUsed = iSrclenToProcess;
        TInt dstLen  = KMinDstLen;
        TInt* tempIn = NULL;
        TInt len = KMinBytesInput / sizeof( TInt );
        tempIn = ( TInt* ) ( ( iInternalInputBuffer +
                                                internalInputBufferPos ) );
        for ( index = 0; index < len; index++ )
            {
            tempIn[index] &= 0xfff8fff8;
            }

        TInt error = iCodec->Encode( tempIn, srcUsed,
                                            iInternalOutputBuffer, dstLen );
        if ( error != 0 )
            {
            iInternalInputBufferResidueLen = 0;
            totalSrcBytesCopied = srcBufferLen;
            PRINT_ERR( error );
            return Result(
                    TCodecProcessResult::EProcessError,
                    totalSrcBytesCopied, totalDstBytesAdded + dstBufferPos );
            }

        /**
        * Fill Destination Buffer
        */
        iInternalOutputBufferResidueLen = dstLen;
        numberOfBytesCopied = CopyToDstBuffer( dst, totalDstBytesAdded );
        dstBufferRemainingBytes -= numberOfBytesCopied;

        /***
        * Fill Source Buffer
        */
        internalInputBufferPos += srcUsed ;
        ShiftData( internalInputBufferPos, 0 );
        numberOfBytesCopied = CopyFromSrcBuffer( src, totalSrcBytesCopied );
        srcBufferRemainingBytes -= numberOfBytesCopied;

        /***
        * check four conditions if else for src and if else for dst
        */
        // src has available bytes
        if ( srcBufferRemainingBytes > 0 || iInternalInputBufferResidueLen >=
                                                        iSrclenToProcess )
            {
            if( dstBufferRemainingBytes == 0 )
                {
                PRINT_EXIT;
                return Result(
                        TCodecProcessResult::EProcessIncomplete,
                        totalSrcBytesCopied, totalDstBytesAdded );
                }
            }
        else
            {
            // dst buffer has availabe space for encoded bytes
            if ( dstBufferRemainingBytes > 0 )
                {
                if ( lastFrame )
                    {
                    if ( iInternalInputBufferResidueLen == 0 )
                        {
                        iInternalOutputBufferResidueLen = 0;
                        iInternalInputBufferResidueLen = 0;
                        iInternalOutputBufferPos = 0;
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
                if ( iInternalOutputBufferResidueLen -
                                            iInternalOutputBufferPos > 0 )
                    {
                    PRINT_EXIT;
                    return Result( TCodecProcessResult::EProcessIncomplete,
                                    totalSrcBytesCopied, totalDstBytesAdded );
                    }
                else
                    {
                    if ( lastFrame &&
                                ( iInternalInputBufferResidueLen == 0 ) )
                        {
                        iInternalOutputBufferResidueLen = 0;
                        iInternalInputBufferResidueLen = 0;
                        iInternalOutputBufferPos = 0;
                        PRINT_EXIT;
                        return Result( TCodecProcessResult::EEndOfData,
                                totalSrcBytesCopied, totalDstBytesAdded );
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


// --------------------------------------------------------------------------
//  Default constructor for performing 1st stage construction.
//  Should not contain any code that could leave.
// --------------------------------------------------------------------------
//
CAriAmrNbEncMmfCodec::CAriAmrNbEncMmfCodec()
    {
    PRINT_ENTRY;
    iCodec = NULL;
    iConfigured = EFalse;
    iSrclenToProcess = KMinBytesInput;
    iInternalInputBufferResidueLen = 0;
    iInternalOutputBufferResidueLen = 0;
    iInternalOutputBufferPos = 0;
    iInternalInputBuffer = NULL;
    iInternalOutputBuffer = NULL;
    //default Parameters
    iParam.iMode = 0;
    iParam.iDtx = 0;
    iParam.iFileFormat = 0;
    PRINT_EXIT;
    }


// --------------------------------------------------------------------------
//  Second phase of the two-phased constructor.
//  Creates an instance of encoder
// --------------------------------------------------------------------------
//
void CAriAmrNbEncMmfCodec::ConstructL()
    {
    PRINT_ENTRY;
    iCodec = CAriAmrNbEncWrapper::NewL();

    iInternalInputBuffer = ( TUint8* ) User::AllocZL( KMinBytesInput );
    iInternalOutputBuffer = ( TUint8* ) User::AllocZL( KMinDstLen );

    PRINT_MSG( LEVEL_HIGH, ( "Default mode: %d", iParam.iMode ) );
    PRINT_MSG( LEVEL_HIGH, ( "Default DTX: %d", iParam.iDtx ) );
    //Reset encoder with default parameters
    User::LeaveIfError( iCodec->Reset( &iParam ) );
    PRINT_EXIT;
    }

//----------------------------------------------------------------------------
//  Updates the result of the processing
//----------------------------------------------------------------------------
//
TCodecProcessResult CAriAmrNbEncMmfCodec::Result(
                    TCodecProcessResult::TCodecProcessResultStatus aStatus,
                    TInt aSrcBytesConsumed, TInt aDstBytesAdded )
     {
     PRINT_ENTRY;
     TCodecProcessResult result;

     result.iDstBytesAdded = aDstBytesAdded;
     result.iSrcBytesProcessed = aSrcBytesConsumed;

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
 TInt CAriAmrNbEncMmfCodec::CopyToDstBuffer( CMMFDataBuffer* aDst,
                                     TInt &aDstBytesConsumed )
    {
    PRINT_ENTRY;
    TInt numberOfBytesToBeCopied;
    const TInt dstMaxLen = aDst->Data().MaxLength();
    TUint8* dstPtr = const_cast<TUint8*>( aDst->Data().Ptr() );
    TInt dstBufferPos = aDst->Position();

    TInt dstBufferRemainingBytes = dstMaxLen
                                   - dstBufferPos
                                   - aDstBytesConsumed;
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

    Mem::Copy( dstPtr + dstBufferPos + aDstBytesConsumed,
        iInternalOutputBuffer + iInternalOutputBufferPos,
        numberOfBytesToBeCopied );

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
 TInt CAriAmrNbEncMmfCodec::CopyFromSrcBuffer(const CMMFDataBuffer* aSrc,
                                         TInt &aSrcBytesConsumed )
    {
    PRINT_ENTRY;
    TInt numberOfBytesToBeCopied;
    TUint8* srcPtr = const_cast <TUint8*>( aSrc->Data().Ptr() );
    TInt srcBufferLen = aSrc->Data().Length();
    TInt srcBufferPos = aSrc->Position();

    TInt srcBufferRemainingBytes = srcBufferLen - srcBufferPos
                                   - aSrcBytesConsumed;

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

    Mem::Copy( iInternalInputBuffer + iInternalInputBufferResidueLen,
    srcPtr + srcBufferPos + aSrcBytesConsumed,
    numberOfBytesToBeCopied );

    iInternalInputBufferResidueLen += numberOfBytesToBeCopied;
    aSrcBytesConsumed += numberOfBytesToBeCopied;
    PRINT_EXIT;
    return numberOfBytesToBeCopied;
    }

//---------------------------------------------------------------------------
// Moves the data of the internal input buffer to the start position
//---------------------------------------------------------------------------
//
void CAriAmrNbEncMmfCodec::ShiftData( TInt aFromPos, TInt aToPos )
    {
    PRINT_ENTRY;
    for ( TInt i = aFromPos; i < ( iInternalInputBufferResidueLen -
            aFromPos ); i++ )
        {
        iInternalInputBuffer[i] =
            iInternalInputBuffer[i + aFromPos];
        }
    iInternalInputBufferResidueLen -= aFromPos;
    PRINT_EXIT;
    }


// --------------------------------------------------------------------------
//  Exported proxy for instantiation method resolution
//  Define the interface UIDs
// --------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( KUidAmrNbEncMmfImplUid,
                                                CAriAmrNbEncMmfCodec::NewL )
    };

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(
                                                        TInt& aTableCount )
    {
    PRINT_ENTRY;
    aTableCount = sizeof( ImplementationTable ) /
                                            sizeof( TImplementationProxy );
    PRINT_EXIT;
    return ImplementationTable;
    }
