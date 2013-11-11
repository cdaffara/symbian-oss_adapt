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
* Implementation of  Base process engine
*
*/
#include "ariprocessengine.h"
#include <e32def.h>

//---------------------------------------------------------------------------
//  Default constructor
//----------------------------------------------------------------------------
//
CBaseProcessEngine::CBaseProcessEngine ()
						: CActive( CActive::EPriorityStandard )
	{
	PRINT_ENTRY;
	iState = EStop;
	iCurInputBuf = NULL;
	iCurOutputBuf = NULL;

	iCmdArray.SetOffset( _FOFF( CCmdPckg,iPriorityLink ) );

	iProcEngineObserver = NULL;
	iCodec = NULL;
	iCurInputBuf = NULL;
	iCurOutputBuf = NULL;
	iCurCmd = NULL;
	iIsProcessing = EFalse;

	iInputBufsAddedSoFar = 0;
	iOutputBufsAddedSoFar = 0;
	iOutputBufferReadyCallBackPending =	EFalse;

	CActiveScheduler::Add( this );
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//   2 - phase construtor of CBaseEngine
//----------------------------------------------------------------------------
//
EXPORT_C CBaseEngine* CBaseEngine::NewL ( MProcessEngineObserver* aObserver,
		MBaseCodec* aCodec,
		TBool aInPlaceProcessing,
		TBool aProcessingAutomatic )
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	return CBaseProcessEngine::NewL ( aObserver, aCodec, aInPlaceProcessing,
									aProcessingAutomatic );
	}

//---------------------------------------------------------------------------
//   2 - phase construtor of CBaseProcessEngine
//----------------------------------------------------------------------------
//
CBaseProcessEngine* CBaseProcessEngine::NewL (
		MProcessEngineObserver* aObserver,
		MBaseCodec* aCodec,
		TBool aInPlaceProcessing,
		TBool aProcessingAutomatic )
	{
	PRINT_ENTRY;
	CBaseProcessEngine* uSelf = new ( ELeave ) CBaseProcessEngine;
	CleanupStack::PushL( uSelf );
	uSelf->ConstructL( aObserver, aCodec, aInPlaceProcessing,
			aProcessingAutomatic);
	CleanupStack::Pop();
	PRINT_EXIT;
	return uSelf;
	}

//---------------------------------------------------------------------------
//  Destructor
//-----------------------------------------------------------------------------
//

CBaseProcessEngine::~CBaseProcessEngine ()
	{
	PRINT_ENTRY;
	Cancel();
	//Flushes out all buffers.
	Reset();
	iInputArray.Close();
	iOutputArray.Close();
	iCmdArray.Reset();
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Second phase constructor
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::ConstructL ( MProcessEngineObserver* aObserver,
		MBaseCodec* aCodec,
		TBool aInPlaceProcessing,
		TBool aProcessingAutomatic )
	{
	PRINT_ENTRY;
	iProcEngineObserver = aObserver;
	iCodec = aCodec;
	iInPlaceProc = aInPlaceProcessing;
	iAutomaticProc = aProcessingAutomatic;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Starts the engine
//----------------------------------------------------------------------------
//

TInt CBaseProcessEngine::Start ()
	{
	PRINT_ENTRY;
	iState = EStart;
	IssueRequest();
	return KErrNone;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Stops the engine
//----------------------------------------------------------------------------
//
TInt CBaseProcessEngine::Stop ()
	{
	PRINT_ENTRY;
	Cancel();
	iState = EStop;
	PRINT_EXIT;
	return KErrNone;
	}

//---------------------------------------------------------------------------
//  Adds an input buffer to the input q
//----------------------------------------------------------------------------
//
TInt CBaseProcessEngine::AddInput ( TAny* aInput )
	{
	PRINT_ENTRY;
	if( !aInput )
		{
		return KErrArgument;
		}
	TInt lError = KErrNone;
	lError = iInputArray.Append( aInput );
	if ( lError )
		{
		return lError;
		}

	iInputBufsAddedSoFar++;

	if( iAutomaticProc ) // true
		{
		CCmdPckg* lCmd = NULL;
		TRAP( lError,lCmd = new ( ELeave ) CCmdPckg( CCmdPckg::EDoProcess,
				ENormalPriority ) );
		if( lError )
			{
			return lError;
			}
		iCmdArray.Add( *lCmd );
		}
	// RunL should be scheduled in Start state. Also, if it is already
	// processing i.e iIsProcessing is ETrue,RunL should not be scheduled

	if ( ( iState == EStart ) && !iIsProcessing )
		{
		IssueRequest();
		}
	PRINT_EXIT;
	return KErrNone;
	}

//---------------------------------------------------------------------------
//  Adds a buffer to the output q
//----------------------------------------------------------------------------
//
TInt CBaseProcessEngine::AddOutput ( TAny* aOutput )
	{
	PRINT_ENTRY;
	if( !aOutput )
		{
		return KErrArgument;
		}
	else if( iInPlaceProc )
		{
		return KErrNotSupported;
		}
	TInt lError = KErrNone;
	lError = iOutputArray.Append( aOutput );
	if ( lError )
		{
		return lError;
		}

	iOutputBufsAddedSoFar++;

	// RunL should be scheduled in Start state. Also, if it is already
	// processing i.e iIsProcessing is ETrue,RunL should not be scheduled

	if ( ( iState == EStart ) && !iIsProcessing )
		{
		IssueRequest();
		}
	PRINT_EXIT;
	return KErrNone;
	}

//---------------------------------------------------------------------------
//  Resets the processing engine and flushes all the pending input
//  and output buffers.Calls InputBufferConsumed and OutputBufferReady
//  to give pending input & output buffers with aError = KErrCancel.
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::Reset ()
	{
	PRINT_ENTRY;
	Stop();

	iCodec->Reset();

	iIsProcessing	=	EFalse;

	if ( iCurInputBuf )
		{
		TAny* lTempBuf	= iCurInputBuf;
		iCurInputBuf	= NULL;

		delete iCurCmd;
		iCurCmd = NULL;

		iProcEngineObserver->InputBufferConsumed( lTempBuf, KErrCancel );
		}
	while ( iInputArray.Count() )
		{
		TAny* lTempBuf	= iInputArray[0];
		iInputArray.Remove( 0 );

		delete iCurCmd;
		iCurCmd = NULL;

		iProcEngineObserver->InputBufferConsumed( lTempBuf, KErrCancel );
		}
	if ( iCurOutputBuf )
		{
		TAny* lTempBuf	= iCurOutputBuf;
		iCurOutputBuf	= NULL;
		iProcEngineObserver->OutputBufferReady (lTempBuf, KErrCancel);
		}
	while ( iOutputArray.Count() )
		{
		TAny* lTempBuf	= iOutputArray[0];
		iOutputArray.Remove( 0 );
		iProcEngineObserver->OutputBufferReady( lTempBuf, KErrCancel );
		}
	if ( iCurCmd )
		{
		if ( iCurCmd->iCmdType != CCmdPckg::EDoProcess )
			{
			iProcEngineObserver->CommandProcessed( iCurCmd->iCmd,
					iCurCmd->iCmdData, KErrCancel );
			}
		delete iCurCmd;
		}
	while ( !iCmdArray.IsEmpty() )
		{
		CCmdPckg* lCurCmd = iCmdArray.First();
		lCurCmd->iPriorityLink.Deque();
		if ( lCurCmd->iCmdType != CCmdPckg::EDoProcess )
			{
			iProcEngineObserver->CommandProcessed( lCurCmd->iCmd,
					lCurCmd->iCmdData, KErrCancel );
			}
		delete lCurCmd;
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//  Returns output buffers.
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::ReturnOutputBuffers ()
	{
	PRINT_ENTRY;
	while ( iOutputArray.Count() )
		{
		iProcEngineObserver->OutputBufferReady( iOutputArray[0], KErrCancel );
		iOutputArray.Remove( 0 );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Returns input buffers
//----------------------------------------------------------------------------
//

void CBaseProcessEngine::ReturnInputBuffers ()
	{
	PRINT_ENTRY;

	//Cancel any on-going processing
	if( iIsProcessing )
		{
		iCodec->Reset();
		iIsProcessing = EFalse;
		}

	// return the current input buffer and delete the corresponding doprocessL
	// command
	if ( iCurInputBuf )
		{
		TAny* lTempBuf	= iCurInputBuf;
		iCurInputBuf	= NULL;

		delete iCurCmd;
		iCurCmd = NULL;

		iProcEngineObserver->InputBufferConsumed( lTempBuf, KErrCancel );
		}

	//Since processing is canceled, output is not valid.
	//So add the output buffer back to the queue for reuse
	if( iCurOutputBuf )
		{
		iOutputArray.Append ( iCurOutputBuf );
		iCurOutputBuf = NULL;
		}

	RPointerArray<CCmdPckg> lTempCmdArray;

	//Since all the input buffers are going to be returned, delete all
	// DoProcess commands from the command array.
	// Other commands should be put into the queue as it is.

	while( !iCmdArray.IsEmpty() )
		{
		CCmdPckg* lCurCmd = iCmdArray.First();
		lCurCmd->iPriorityLink.Deque();

		if ( lCurCmd->iCmdType == CCmdPckg::EDoProcess )
			{
			delete lCurCmd;
			}
		else
			{
			// Store non-DoProcess commands in temporary array. And put them
			// back in the iCmdArray after all the DoProcess commands are
			// removed.Appending to the array to keep the order of commands
			//	intact.
			lTempCmdArray.Append( lCurCmd );
			}
		}

	while ( lTempCmdArray.Count() )
		{
		iCmdArray.Add ( *( lTempCmdArray[0] ) );
		lTempCmdArray.Remove( 0 );
		}

	lTempCmdArray.Close();

	//return all the input buffers
	while ( iInputArray.Count() )
		{
		TAny* lTempBuf = iInputArray[0];
		iInputArray.Remove( 0 );
		iProcEngineObserver->InputBufferConsumed( lTempBuf, KErrCancel );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Processes the commands
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::DoProcessL( TInt aPriority )
	{
	PRINT_ENTRY;
	CCmdPckg* lCmd = new ( ELeave ) CCmdPckg( CCmdPckg::EDoProcess,
			aPriority );
	iCmdArray.Add ( *lCmd );

	// RunL should be scheduled in Start state. Also, if it is already
	// processing i.e iIsProcessing is ETrue,RunL should not be scheduled
	if ( iState == EStart || !iIsProcessing )
		{
		IssueRequest();
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Adds a new command to the priority queue of commands
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::AddCommandL(TInt aPriority, TInt aCmd,
		TAny* aCmdData )
	{
	PRINT_ENTRY;
	CCmdPckg* lCmd = new ( ELeave ) CCmdPckg( CCmdPckg::EOther, aPriority,
			aCmd, aCmdData );
	iCmdArray.Add (*lCmd);
	// RunL should be scheduled in Start state. Also, if it is already
	// processing i.e iIsProcessing is ETrue,RunL should not be scheduled

	if ( iState == EStart || !iIsProcessing )
		{
		IssueRequest();
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Called by MBaseCodec when processing is complete
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::ProcessingComplete( TAny *aInpBuf, TAny* aOutBuf,
		MBaseCodec::TCodecState aState, TInt aError )
	{
	PRINT_ENTRY;
	TAny* lTempBuf	=	NULL;

	iError	=	aError;
	if ( iCurInputBuf != aInpBuf )
		{
		/*This should never happen*/
		User::Panic( _L ( "iCurInputBuf != aInpBuf" ) , 0 );
										
		}
	if ( iCurOutputBuf != aOutBuf )
		{
		/*This should never happen*/
		User::Panic (_L("iCurOutputBuf != aOutBuf" ) , 1 );
										
		}
	if ( !iIsProcessing )
		{
		/*This should never happen*/
		User::Panic (_L("!iIsProcessing " ) , 2 );
												
		}

	// error returned by the DoProcessL must be handled here
	if( aError )
		{
		// call fatal error on hw device
		iProcEngineObserver->FatalErrorFromProcessEngine( aError );
		return;
		}

	if ( iState == EStart )
		{
		IssueRequest();
		}

	if ( iInPlaceProc )
		{
		switch ( aState )
			{
			case MBaseCodec::EInputConsumed :
			case MBaseCodec::EConsumed :
				/*Callback should be the last statement*/
				iIsProcessing = EFalse;
				lTempBuf		=	iCurInputBuf;
				iCurInputBuf	=	NULL;

				delete iCurCmd;
				iCurCmd = NULL;

				iProcEngineObserver->InputBufferConsumed( lTempBuf, iError );
				return;

			case MBaseCodec::EOutputConsumed :
			case MBaseCodec::ENotConsumed :
			default:
				return;
			}
		}
	else
		{
		switch ( aState )
			{
			case MBaseCodec::EInputConsumed :
				/*CallBack should be the last statement*/
				iIsProcessing	=	EFalse;
				lTempBuf		=	iCurInputBuf;
				iCurInputBuf	=	NULL;

				delete iCurCmd;
				iCurCmd = NULL;

				iProcEngineObserver->InputBufferConsumed( lTempBuf, iError );
				return;

			case MBaseCodec::EConsumed :
				/*CallBack should be the last statement*/
				lTempBuf		=	iCurInputBuf;
				iCurInputBuf	=	NULL;

				delete iCurCmd;
				iCurCmd = NULL;


				iProcEngineObserver->InputBufferConsumed( lTempBuf, iError );
				lTempBuf		=	iCurOutputBuf;
				iCurOutputBuf	=	NULL;
				iProcEngineObserver->OutputBufferReady( lTempBuf, iError );

				return;

			case MBaseCodec::EOutputConsumed :
				/*CallBack should be the last statement*/
				iIsProcessing	=	EFalse;
				lTempBuf		=	iCurOutputBuf;
				iCurOutputBuf	=	NULL;
				iProcEngineObserver->OutputBufferReady ( lTempBuf, iError );
				return;
			case MBaseCodec::ENotConsumed :
			default:
				return;
			}
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Called by MBaseCodec when processing of commmands is complete
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::ProcessingCommandComplete( TInt aCommand,
		TAny* aCmdData, TInt aError )
	{
	PRINT_ENTRY;
	iIsProcessing = EFalse;
	delete iCurCmd;
	iCurCmd = NULL;
	IssueRequest();
	iProcEngineObserver->CommandProcessed( aCommand, aCmdData, aError );
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Gets the number of input buffers
//----------------------------------------------------------------------------
//
TInt CBaseProcessEngine::NumInputBuffers ()
	{
	PRINT_ENTRY;
	TInt lCnt = iInputArray.Count();
	if ( iCurInputBuf )
		{
		lCnt	=	lCnt + 1;
		}
	PRINT_EXIT;
	return lCnt;
	}

//---------------------------------------------------------------------------
// Gets the number of output buffers
//----------------------------------------------------------------------------
//
TInt CBaseProcessEngine::NumOutputBuffers ()
	{
	PRINT_ENTRY;
	TInt lCnt = iOutputArray.Count();
	if ( iCurOutputBuf )
		{
		lCnt	=	lCnt + 1;
		}
	PRINT_EXIT;
	return lCnt;
	}

//---------------------------------------------------------------------------
// Runl() of CBaseProcessEngine. Scheduled whenever a new command needs to
// be processed
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::RunL ()
	{
	PRINT_ENTRY;
	/*If in stopped state, or it is already processing, don't do anything*/
	if ( iState == EStop || iIsProcessing )
		{
		return;
		}

	ProcessNextCommand();
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Processeses the next command in the queue
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::ProcessNextCommand()
	{
	PRINT_ENTRY;
	if( iCurCmd  || !iCmdArray.IsEmpty() )
		{
		if( !iCurCmd )
			{
			iCurCmd = iCmdArray.First();
			iCurCmd->iPriorityLink.Deque();
			}

		if( iCurCmd->iCmdType == CCmdPckg::EDoProcess )
			{
			if( IsReadyForProcessing() )
				{
				iIsProcessing = ETrue;

				TInt result = KErrNone;

				TRAPD( lError, result = iCodec->DoProcessL( iCurInputBuf,
						iCurOutputBuf ) );

				// based on the result call
				ProcessingComplete( iCurInputBuf, iCurOutputBuf,
						( MBaseCodec::TCodecState )result, lError );

				iIsProcessing = EFalse;

				IssueRequest ();
				}
			}
		else
			{
			iIsProcessing = ETrue;

			TInt lError = iCodec->SetParam( iCurCmd->iCmd,
					iCurCmd->iCmdData );

			ProcessingCommandComplete( iCurCmd->iCmd, iCurCmd->iCmdData,
					lError );

			iIsProcessing = EFalse;
			}
		}
	PRINT_EXIT;
	}


//---------------------------------------------------------------------------
// Indicates if process engine is ready for processing a new buffer i.e
// if process engine has both input and output buffers in its respective
// queues
//----------------------------------------------------------------------------
//
TBool CBaseProcessEngine::IsReadyForProcessing()
	{
	PRINT_ENTRY;
	if( !iCurInputBuf )
		{
		if( iInputArray.Count() )
			{
			iCurInputBuf = iInputArray[0];
			iInputArray.Remove( 0 );
			if ( iInPlaceProc )
				{
				return ETrue;
				}
			}
		else
			{
			return EFalse;
			}
		}
	if ( !iCurOutputBuf )
		{
		if ( iOutputArray.Count() )
			{
			iCurOutputBuf = iOutputArray[0];
			iOutputArray.Remove( 0 );
			return ETrue;
			}
		return EFalse;
		}
	PRINT_EXIT;
	return ETrue;
	}
//---------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::DoCancel ()
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Issue a new request for processing a command i.e schedule Runl()
//----------------------------------------------------------------------------
//
void CBaseProcessEngine::IssueRequest ()
	{
	PRINT_ENTRY;
	if ( IsActive() )
		{
		return;
		}

	TRequestStatus*  uStatus = &iStatus;
	User::RequestComplete( uStatus, KErrNone );
	SetActive();
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Indicates whether processing of an input buffer is going on or else
//----------------------------------------------------------------------------
//
TBool CBaseProcessEngine::IsProcessing ()
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	return iIsProcessing;
	}

//---------------------------------------------------------------------------
// This func is called if RunL leaves
//----------------------------------------------------------------------------
//
TInt CBaseProcessEngine::RunError( TInt aError )
	{
	PRINT_ENTRY;
	iProcEngineObserver->FatalErrorFromProcessEngine( aError );
	PRINT_EXIT;
	return KErrNone;
	}

//---------------------------------------------------------------------------
// Constructor for CCmdPckg
//----------------------------------------------------------------------------
//
CCmdPckg::CCmdPckg ( TCmdType aCmdType, TInt aPriority, TInt aCmd,
		TAny* aCmdData )
	{
	PRINT_ENTRY;
	iCmdType = aCmdType;
	iCmd = aCmd;
	iCmdData = aCmdData;
	iPriorityLink.iPriority = aPriority;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// Destructor for CCmdPckg
//----------------------------------------------------------------------------
//
CCmdPckg::~CCmdPckg ()
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// returns an Input picture back to the client
//----------------------------------------------------------------------------
//

void CBaseProcessEngine::ReturnInput()
	{
	PRINT_ENTRY;
	TAny* lTempBuf = 0;
	if ( iCurInputBuf )
		{
		lTempBuf = iCurInputBuf;
		delete iCurCmd;
		iCurCmd = NULL;
		}
	else
		{
		if( iInputArray.Count() )
			{
			lTempBuf = iInputArray[0];
			iInputArray.Remove( 0 );
			iCurCmd = iCmdArray.First();
			iCurCmd->iPriorityLink.Deque();
			delete iCurCmd;
			iCurCmd = NULL;
			}
		}
	// return input buffer back to the HwDevice and from there to the client
	if( iProcEngineObserver && lTempBuf )
		{
		iProcEngineObserver->InputBufferConsumed( lTempBuf, KErrCancel );
		}
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
// returns an output buffer back to the client
//----------------------------------------------------------------------------
//
TAny* CBaseProcessEngine::FetchOutputBuffer()
	{
	PRINT_ENTRY;
	TAny* lTempOutput=NULL;
	lTempOutput=iCurOutputBuf;

	if ( !iOutputArray.Count() )
		{
		iCurOutputBuf =  NULL;
		return lTempOutput;
		}
	TAny* uBuffer = iOutputArray[0];
	iOutputArray.Remove( 0 );

	iCurOutputBuf = uBuffer;
	PRINT_EXIT;
	return lTempOutput;

	}
