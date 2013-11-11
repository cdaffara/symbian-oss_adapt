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
* State machine for all video hwdevice plugins.
*
*/

#ifndef ARISTATEMACHINE_H
#define ARISTATEMACHINE_H

#include <e32base.h>

#include "ariprint.h"


// Constants
const TUint KNumOfStates    = 13;
const TUint	KNumOfCommands	= 8;

class CStateMachine : public CBase
{
public:
	/**
     * All the states of the state machine.
	 */
	enum TState
		{
		EUnInitialized = 0,
		EInitializing,
		EInitialized,
		EStart,
		EPause,
		EStop,
		EInitializedInStopping,
		EStopping,
		EPauseInStopping,
		EStopInInputEnd,
		EInitializedDeadState,
		ENonInitializedDeadState,
		EInvalidState
		};
	/**
     * Commands that are given to state machine
	 */
	enum TCommand
		{
		EInitializeCommand = 0 ,
		EInitializingCommand,
		EStartCommand,
		EPauseCommand,
		EResumeCommand,
		EInputEndCommand,
		EStopCommand,
		EDeadStateCommand
		};

public:

	/**
	 *Constructor. State will be in EUnInitialized
	 *
	 *@param	"None"
	 *@leave	"None"
	 *@return	"None"
	 */
	IMPORT_C static CStateMachine* NewL();

	/**
	 *Checks whether transition to new state is possible or not
	 *
	 *@param	"aCommand"	"Command for the transition"
	 *@leave	"None"
	 *@return	"Return ETrue if transition is valid, else EFalse
	 */
	IMPORT_C TBool	IsTransitionValid (TCommand aCommand);

	/**
	 *Transits to new state. If transition cann't be done, object will
	 *remain in the previous state.
	 *
	 *@param	"aCommand"	"Command for the transition"
	 *@return	"Return KErrNone if transition happens else error.
	 */
	IMPORT_C TInt	Transit (TCommand aCommand);

	/**
	 *Tells whether the state is initialized or not. Here initilized means
	 *if the state is in any state other than EUnInitialized
	 *
	 *@param	"None"
	 *@return	"Return ETrue, if state is initialized else EFalse.
	 */
	IMPORT_C TBool	IsInitialized ();

	/**
	 *Tells whether the state is exactly in EInitialized state or not.
	 *
	 *@param	"None"
	 *@return	Return ETrue, if in EInitialized state else EFalse.
	 */
    IMPORT_C TBool IsInInitializedState ();

	/**
	 *Tells whether the state is in initializing state or not.
	 *
	 *@param	"None"
	 *@return	"Return ETrue, if state is initializing else EFalse.
	 */
	IMPORT_C TBool	IsInitializing ();

	/**
	 *Tells whether the state is in any one of the input ending states i.e
	 *	EInitializedInStopping,
	 *	EStopping,
	 *	EPauseInStopping,
	 *	EStopInInputEnd
	 *
	 *
	 *@param	None
	 *@return	Return ETrue, if state is in any of inputend state else EFalse
	 */
	IMPORT_C TBool	IsInputEndPending ();

	/**
	 *Tells whether state is in EStart state
	 *
	 *@param	"None"
	 *@return	"Return ETrue if state is EStart, elase EFalse
	 */
	IMPORT_C TBool	IsStarted ();

	/**
	 *Tells whether state is in EPause state
	 *
	 *@param	"None"
	 *@return	"Return ETrue if state is EPause, elase EFalse
	 */
	IMPORT_C TBool	IsPaused ();

	/**
	 *Tells whether state is in EStop state
	 *
	 *@param	"None"
	 *@return	"Return ETrue if state is EStop, elase EFalse
	 */
	IMPORT_C TBool	IsStopped ();

	/**
	 *Tells whether state is in EPlaying state
	 *
	 *@param	"None"
	 *@return	"Return ETrue if state is EPlaying, elase EFalse
	 */
	IMPORT_C TBool	IsPlaying ();

	/**
	 *Tells whether state is in EInitializedDeadState/ENonInitializedDeadState
	 *state
	 *
	 *@param	"None"
	 *@return	"Return ETrue if state is
	 *           Return EInitializedDeadState/ENonInitializedDeadState,
	 *           elase EFalse"
	 */
	IMPORT_C TBool	IsInDeadState ();

	/**
	 *Resets the state machine to EUnInitialized state
	 *
	 *@param	"None"
	 *@return	"None"
	 */
	IMPORT_C void	Reset();

	/**
	 * Returns whether the current state is in initailize or not
	 *
	 *@return	"ETrue if current state is in Initailize"
	 */
	IMPORT_C TBool IfIsStateInInitailize();


	/**
	 * Destructor of the statemachine
	 *
	 *@return	"None"
	 */
	~CStateMachine();

private:

	/**
	 *Symbian 2nd phase constructor
	 *@return	"None"
	 */
	void ConstructL();

    /**
	 *Default Constructor
	 */
	CStateMachine();

private:
	//Stores the state of the state machine
	TState			iState;

	TState			iStateChanges[KNumOfStates][KNumOfCommands];
};

#endif // ARISTATEMACHINE_H

