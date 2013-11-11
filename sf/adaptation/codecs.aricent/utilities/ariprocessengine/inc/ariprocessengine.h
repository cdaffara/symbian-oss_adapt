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
* Header file of process engine. Declares class CBaseProcessEngine
* which derives from CBaseEngine and provides implementation for
* all pure virtual functions
*
*/
#ifndef ARIPROCESSENGINE_H
#define ARIPROCESSENGINE_H

#include <E32base.h>
#include "aribaseengine.h"
#include "ariprint.h"

NONSHARABLE_CLASS (CCmdPckg): public CBase
	{
	public:
		/**
		 * Enumeration to be used to specify command type. All commands should
		 *  be of one of the 2 types given in this enumeration.
		 */
		enum TCmdType
			{
			/**
			 *  All commands for encoding a buffer should be of this type
			 *  @see CCmdPckg::CCmdPckg
			 *
			 */
			EDoProcess,

			/**
			 *  Rest of the commands will be of this type
			 */
			EOther
			};
		/**
		 * Default constructor.Assigns the parameteres to corresponding
		 * member variables.
		 * @param aCmdType
		 *    The type of command (EDoProcess/EOther)
		 * @param aPriority
		 *    The priority assigned to this command
		 * @param aCmd
		 * 	  The command
		 * @param aCmdData
		 * 	  Command specific data
		 */
		CCmdPckg ( TCmdType aCmdType, TInt aPriority, TInt aCmd = -1,
				TAny* aCmdData = NULL );

		/**> Destructor */
		~CCmdPckg();
	public:
		/**
		 * Command type which will be either one of - EDoProcess/EOther
		 */
		TCmdType 	iCmdType;

		/**
		 * The command. Any command which is not of type EDoProcess will be
		 * processed in the wrapper.
		 */
		TInt		iCmd;

		/**
		 * Command specific data.
		 */
		TAny* 		iCmdData;

		/**
		 * The priority link to store the commands in a priority based doubly
		 * linked list.
		 */
		TPriQueLink iPriorityLink;
	};

NONSHARABLE_CLASS ( CBaseProcessEngine ) :  public CBaseEngine,
								public CActive
{
	public:

		/**
		 * Enumeration to be used to specify the state of the process engine
		 */
		enum TState
		{
			EStart,
			EStop
		};

	public:
		/**
		 * Two-phased constructor.
		 * @param aObserver
		 *    Pointer to observer to the process engine - HwDevice
		 * @param aCodec
		 *    Pointer to wrapper
		 * @param aInPlaceProcessing
		 *
		 * @param aProcessingAutomatic
		 *
		 * @return pointer to an instance of CBaseProcessEngine
		 */
		static CBaseProcessEngine* NewL ( MProcessEngineObserver* aObserver,
										  MBaseCodec* aCodec,
										  TBool aInPlaceProcessing,
										 TBool aProcessingAutomatic = ETrue );
		/**> Destructor */
		virtual ~CBaseProcessEngine ();

		/**
		 *  Default constructor
		 */
		CBaseProcessEngine();

	public:
		/**
		 * From CBaseEngine
		 * To add the input buffer to the Q
		 * @param aBuffer
		 * 	  Pointer to an input buffer.
		 * @return symbian wide error code
		 */
		TInt AddInput( TAny* aBuffer);

		/**
		 * From CBaseEngine
		 * To add the output buffer to the Q
		 * @param aBuffer
		 * 	  Pointer to an output buffer.
		 * @return symbian wide error code, KErrNone on success
		 */
		TInt AddOutput( TAny* aBuffer);

		/**
		 * From CBaseEngine
		 * Starts the processing
		 * @return symbian wide error code, KErrNone on success
		 */
		TInt Start();

		/**
		 * From CBaseEngine
		 * Stops the processing
		 * @return symbian wide error code, KErrNone on success
		 */
		TInt Stop();

		/**
		 * From CBaseEngine
		 * Processes the commands
		 * @param aPriority
		 * 	  Priority of the encode command object
		 */
		void DoProcessL( TInt aPriority);

		/**
		 * From CBaseEngine
		 * Adds a new command to the priority queue of commands
		 * @param aPriority
		 * 	  Priority of the command object
		 * @param aCmd
		 * 	  The command
		 * @param aCmdData
		 * 	  Command specific data
		 */
		void AddCommandL( TInt aPriority, TInt aCmd, TAny* aCmdData );


		/**
		 * From CBaseEngine
		 * Resets the processing engine and flushes all the pending input and
		 * output buffers.
		 * Calls InputBufferConsumed and OutputBufferReady to give pending
		 * input & output buffers with aError = KErrCancel.
		 */
		void Reset();

		/**
		 * From CBaseEngine
		 * Returns a output buffer back to the HwDevice
		 */
        void ReturnOutputBuffers();

        /**
         * From CBaseEngine
		 * Gets the number of input buffers
		 * @return returns the number of input buffers in process engine q
		 */
		TInt NumInputBuffers();

        /**
         * From CBaseEngine
		 * Gets the number of output buffers
		 * @return returns the number of output buffers in process engine q
		 */
		TInt NumOutputBuffers();

		/**
		 * From CBaseEngine
		 * Returns whether processing of an input buffer is going on or else
		 * @return ETrue if processing in on going, else EFalse
		 */
		TBool IsProcessing();

		/**
		 * From CBaseEngine
		 * Returns input buffer back to the HwDevice through callback
		 */
		void ReturnInputBuffers();

		/**
		 * From CBaseEngine
		 * Returns all the input pictures added previously through callback
		 */
		void ReturnInput();


		/**
		 * From CBaseEngine
		 * Returns the 1st buffer from the output buffer queue
		 * @return returns pointer to the fetched output buffer
		 */
		virtual TAny* FetchOutputBuffer();

	public:
		/**
		 * From CBaseEngine
		 * Called by MBaseCodec when processing is complete
		 * @param aInpBuf
		 * 	  The input buffer sent for processing
		 * @param aOutBuf
		 *    The output buffer
		 * @param aState
		 *    The result of processing
		 * @param aError
		 *    Error value returned from DoProcessL
		 */
		void ProcessingComplete( TAny *aInpBuf, TAny* aOutBuf,
									MBaseCodec::TCodecState aState,
									TInt aError );

		/**
		 * From CBaseEngine
		 * Called by MBaseCodec when processing of commmands is complete
		 * @param aCommand
		 * 	  The command which has been processed
		 * @param aCmdData
		 *    Command specific data
		 * @param aState
		 *    The result of processing
		 * @param aError
		 * 	  Result of processing command
		 */
		void ProcessingCommandComplete( TInt aCommand, TAny* aCmdData,
															TInt aError );

	private:

		void RunL ();

		void DoCancel ();

		TInt RunError( TInt aError );

	private:

		void IssueRequest ();

		void ConstructL( MProcessEngineObserver* aObserver,
						 MBaseCodec* aCodec,
						 TBool aInPlaceProcessing,
						 TBool aProcessingAutomatic );

		TBool IsReadyForProcessing ();

		void ProcessNextCommand ();

	private:

		TPriQue<CCmdPckg> 				iCmdArray;
		RArray<TAny*> 					iInputArray;
		RArray<TAny*> 					iOutputArray;
		TState 							iState;
		MProcessEngineObserver* 		iProcEngineObserver;
		MBaseCodec* 				    iCodec;
		TBool 							iInPlaceProc;
		TBool 							iAutomaticProc;
		TAny* 							iCurInputBuf;
		TAny* 							iCurOutputBuf;
		CCmdPckg*						iCurCmd;
		TBool							iIsProcessing;
		TBool							iOutputBufferReadyCallBackPending;
		TInt							iError;

        TInt iInputBufsAddedSoFar;
        TInt iOutputBufsAddedSoFar;
};


#endif //ARIPROCESSENGINE_H



