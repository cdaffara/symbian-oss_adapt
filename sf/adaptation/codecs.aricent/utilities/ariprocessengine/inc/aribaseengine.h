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
* Contains base class for process engine (CBaseEngine) and an abstract class
* (MProcessEngineObserver) which has callback methods to indicate events like
* completion of encoding, processing of a command etc.
*
*/


#ifndef ARIBASEENGINE_H
#define ARIBASEENGINE_H

#include <E32base.h>
#include "aribasecodec.h"

/**
 * An abstract class which contains call back functions to be derived and
 * implemented by any module wishing to use the process engine.
 */

class MProcessEngineObserver
	{
	public:

		/**
		 * From MProcessEngineObserver
		 * The function is a callback to indicate the input buffer is consumed
		 * @param aInp
		 *    Pointer to the input picture that has been processed
		 * @param aError
		 *   Error code returned by process engine
		 * @return error value
		 */
		virtual TInt InputBufferConsumed( TAny* aInp, TInt aError ) = 0;

		/**
		 * From MProcessEngineObserver
		 * The function is a callback to indicate the output buffer is ready
		 * @param aOup
		 *    Pointer to the output picture that has been processed
		 * @param aError
		 *   Error code returned by process engine
		 * @return error value
		 */
		virtual TInt OutputBufferReady( TAny* aOup, TInt aError  ) = 0;

		/**
		 * From MProcessEngineObserver
		 * The function indicates to hwdevice that process engine has finished
		 * with the processing of command requested by hwdevice
		 * @param aCmd
		 *    Command that has been processed by process engine
		 * @param aCmdData
		 *   Pointer to command data that has been processed by process engine
		 * @param aError
		 *   Error code corresponding to the command
		 */
		virtual void CommandProcessed( TInt aCmd, TAny* aCmdData,
														TInt aError ) = 0;
		/**
		 * From CMMFVideoEncodeHwDevice
		 * The function indicates to hwdevice that process engine has met with 
		 * an unrecoverable error during its processing
		 * @param aError
		 *    The fatal error code
		 */
		virtual	void FatalErrorFromProcessEngine( TInt aError ) = 0;
	};


	/**
	 * Base class for base process engine. This calls creates class
	 * CAriH264encHwDeviceImpl which derives this class & provides
	 * implementation for pure virtual functions present in this class.
	 */
class CBaseEngine
	{
	public:

	/**
	 * Enumeration to be used to set priority of an command
	 */
	enum TCmdPriority
		{
		ENormalPriority  = 0,
		EStandardPriority = 1,
		EHighPriority = 2,
		EVeryHighPriority = 3
		};

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

	IMPORT_C static CBaseEngine* NewL( MProcessEngineObserver* aObserver,
			MBaseCodec* aCodec,
			TBool aInPlaceProcessing,
			TBool aProcessingAutomatic = ETrue );

	/**> Destructor */
	virtual ~CBaseEngine ()
		{
		}


	public:

	/**
	 * Adds an input buffer to the Q
	 * @param aBuffer
	 * 	  Pointer to an input buffer.
	 * @return symbian wide error code
	 */
	virtual TInt AddInput( TAny* aBuffer ) = 0;

	/**
	 * Adds an output buffer to the Q
	 * @param aBuffer
	 * 	  Pointer to an output buffer.
	 * @return symbian wide error code
	 */
	virtual TInt AddOutput( TAny* aBuffer ) = 0;

	/**
	 * Starts the processing
	 * @return symbian wide error code, KErrNone on success
	 */
	virtual TInt Start() = 0;

	/**
	 * Stops the processing
	 * @return symbian wide error code, KErrNone on success
	 */
	virtual TInt Stop() = 0;

	/**
	 * Processes the commands
	 * @param aPriority
	 * 	  Priority of the encode command object
	 */
	virtual void DoProcessL( TInt aPriority ) = 0;

	/**
	 * Adds a new command to the priority queue of commands
	 * @param aPriority
	 * 	  Priority of the command object
	 * @param aCmd
	 * 	  The command
	 * @param aCmdData
	 * 	  Command specific data
	 */
	virtual void AddCommandL( TInt aPriority, TInt aCmd, TAny* aCmdData ) = 0;

	/**
	 * Resets the processing engine and flushes all the pending input and
	 * output buffers.
	 * Calls InputBufferConsumed and OutputBufferReady to give pending
	 * input & output buffers with aError = KErrCancel.
	 */
	virtual void Reset() = 0;

	/**
	 * Returns a output buffer back to the HwDevice
	 */
	virtual void ReturnOutputBuffers() = 0;

	/**
	 * Gets the number of input buffers
	 * @return returns the number of input buffers in process engine q
	 */
	virtual TInt NumInputBuffers() = 0;

	/**
	 * Gets the number of output buffers
	 * @return returns the number of output buffers in process engine q
	 */
	virtual TInt NumOutputBuffers() = 0;

	/**
	 * Returns whether processing of an input buffer is going on or else
	 * @return ETrue if processing in on going, else EFalse
	 */
	virtual TBool IsProcessing() = 0;

	/**
	 * Returns input buffer back to the HwDevice
	 */
	virtual void ReturnInputBuffers() = 0;

	/**
	 * Returns all the input pictures added previously.
	 */
	virtual void ReturnInput() = 0;

	public:

	/**
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
	virtual void ProcessingComplete( TAny *aInpBuf, TAny* aOutBuf,
					MBaseCodec::TCodecState aState, TInt aError ) = 0;

	/**
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

	virtual void ProcessingCommandComplete( TInt aCommand, TAny* aCmdData,
															TInt aError ) = 0;
	/**
	 * Returns the 1st buffer from the output buffer queue
	 * @return returns pointer to the fetched output buffer
	 */
	virtual TAny* FetchOutputBuffer() = 0;
	};

#endif //ARIBASEENGINE_H



