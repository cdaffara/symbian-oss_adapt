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

//User includes
#include "aristatemachine.h"

//---------------------------------------------------------------------------
//Default Constrcutor.
//---------------------------------------------------------------------------
//
CStateMachine::CStateMachine():iState( EUnInitialized )
	{
	PRINT_ENTRY;
	// Initailization
	for ( TInt i = 0; i < KNumOfStates; i++ )
		{
		for ( TInt j = 0; j < KNumOfCommands; j++ )
			{
			iStateChanges [i][j] = EInvalidState;
			}
		}

	// set the valid transition states in the table

	// valid state changes from Uninitailize
	iStateChanges [EUnInitialized][EInitializeCommand] = EInitialized;
	iStateChanges [EUnInitialized][EInitializingCommand] = EInitializing;
	iStateChanges [EUnInitialized][EDeadStateCommand]
	                               = ENonInitializedDeadState;


	// valid state changes from Initailize
	iStateChanges [EInitialized][EStartCommand]		= EStart;
	iStateChanges [EInitialized][EPauseCommand]		= EPause;
	iStateChanges [EInitialized][EResumeCommand]	= EStart;
	iStateChanges [EInitialized][EInputEndCommand]	= EInitializedInStopping;
	iStateChanges [EInitialized][EStopCommand]		= EStop;
	iStateChanges [EInitialized][EDeadStateCommand]	= EInitializedDeadState;

	iStateChanges [EInitializing][EDeadStateCommand] = EInitializedDeadState;
	iStateChanges [EInitializing][EInitializeCommand]	= EInitialized;

	// valid state changes from Start
	iStateChanges [EStart][EStartCommand]		= EStart;
	iStateChanges [EStart][EPauseCommand]		= EPause;
	iStateChanges [EStart][EInputEndCommand]	= EStopping;
	iStateChanges [EStart][EStopCommand]		= EStop;
	iStateChanges [EStart][EDeadStateCommand]	= ENonInitializedDeadState;

	// valid  State changes from pause
	iStateChanges [EPause][EPauseCommand]		= EPause;
	iStateChanges [EPause][EResumeCommand]		= EStart;
	iStateChanges [EPause][EInputEndCommand]	= EPauseInStopping;
	iStateChanges [EPause][EStopCommand]		= EStop;
	iStateChanges [EPause][EDeadStateCommand]	= ENonInitializedDeadState;

	// valid state changes from Stop
	iStateChanges [EStop][EStartCommand]	= EStart;
	iStateChanges [EStop][EInputEndCommand] = EStopInInputEnd;
	iStateChanges [EStop][EStopCommand]		= EStop;
	iStateChanges [EStop][EDeadStateCommand]= ENonInitializedDeadState;

	// valid state changes from InitailizeInStopping
	iStateChanges [EInitializedInStopping][EStartCommand] = EStopping;
	iStateChanges [EInitializedInStopping][EDeadStateCommand]
	                                       = ENonInitializedDeadState;

	// valid  State Changes from EStopping
	iStateChanges [EStopping][EPauseCommand] = EPauseInStopping;
	iStateChanges [EStopping][EStopCommand]	 = EStop;
	iStateChanges [EStopping][EDeadStateCommand] = ENonInitializedDeadState;

	// valid state changes from PauseInStopping
	iStateChanges [EPauseInStopping][EResumeCommand] = EStopping;
	iStateChanges [EPauseInStopping][EStopCommand] = EStop;
	iStateChanges [EPauseInStopping][EDeadStateCommand]
	                                 = ENonInitializedDeadState;

	// valid state changes frm StopInInputEnd
	iStateChanges [EStopInInputEnd][EStartCommand] = EStopping;
	iStateChanges [EStopInInputEnd][EStopCommand] = EStop;
	iStateChanges [EStopInInputEnd][EDeadStateCommand]
	                                = ENonInitializedDeadState;
	}

//---------------------------------------------------------------------------
//Constrcutor.
//---------------------------------------------------------------------------
//
EXPORT_C CStateMachine* CStateMachine::NewL()
	{
	PRINT_ENTRY;
	CStateMachine* self = new ( ELeave ) CStateMachine();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop();
	PRINT_EXIT;
	return( self );
	}

//---------------------------------------------------------------------------
//Symbian 2nd Phase Constrcutor.
//---------------------------------------------------------------------------
//
void CStateMachine::ConstructL()
	{
	PRINT_ENTRY;
	PRINT_EXIT;
	}

//---------------------------------------------------------------------------
//Checks whether transition to new state is possible or not.
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsTransitionValid( TCommand aCommand )
	{
	PRINT_ENTRY;

	if ( iStateChanges[iState][aCommand] != EInvalidState )
		{
		PRINT_EXIT;
		return( ETrue );
		}
	else
		{
		PRINT_EXIT;
		return( EFalse );
		}
	}

//---------------------------------------------------------------------------
//Transits to new state. If transition cann't be done, object will remain in
//the previous state.
//---------------------------------------------------------------------------
//
EXPORT_C TInt CStateMachine::Transit( TCommand aCommand )
	{
	PRINT_ENTRY;
	if ( !IsTransitionValid ( aCommand ) )
		{
		return KErrGeneral;
		}
	iState = iStateChanges[iState][aCommand];
	PRINT_EXIT;
	return KErrNone;
	}

//---------------------------------------------------------------------------
//Tells whether the state is initialized or not. Here initilized means if the
//state is in any state other than EUnInitialized
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsInitialized()
	{
	PRINT_ENTRY;
	if ( iState	!=	EUnInitialized )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Tells whether the state is exactly in EInitialized state or not.
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsInInitializedState()
	{
	PRINT_ENTRY;
	if ( iState	==	EInitialized )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Tells whether the state is initializing or not.
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsInitializing()
	{
	PRINT_ENTRY;
	if ( iState	==	EInitializing )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Tells whether the state is in any one of the input ending
//states i.e
//EInitializedInStopping,
//EStopping,
//EPauseInStopping,
//EStopInInputEnd
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsInputEndPending()
	{
	PRINT_ENTRY;
	if ( iState == EInitializedInStopping ||
		iState == EStopping ||
		iState == EPauseInStopping ||
		iState == EStopInInputEnd )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Tells whether state is in EStart state
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsStarted()
	{
	PRINT_ENTRY;
	if ( iState == EStart )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Tells whether stae is in EPlaying state
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsPlaying()
	{
	PRINT_ENTRY;
	if ( iState == EStart || iState == EStopping )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Tells whether state is in EPause state
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsPaused()
	{
	PRINT_ENTRY;
	if ( ( iState == EPause ) || ( iState == EPauseInStopping ) )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Tells whether stae is in EStop state
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsStopped()
	{
	PRINT_ENTRY;
	if ( iState == EStop )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Tells whether stae is in EInitailize or not
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IfIsStateInInitailize()
	{
	PRINT_ENTRY;
	if ( iState == EInitialized )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Resets the state machine to EUnInitialized state.
//---------------------------------------------------------------------------
//
EXPORT_C void CStateMachine::Reset()
	{
	PRINT_ENTRY;
	iState	=	EUnInitialized;
	PRINT_EXIT;
	}



//---------------------------------------------------------------------------
//Tells whether stae is in EDeadState state
//---------------------------------------------------------------------------
//
EXPORT_C TBool CStateMachine::IsInDeadState()
	{
	PRINT_ENTRY;
	if ( ( iState == EInitializedDeadState ) ||
		 ( iState == ENonInitializedDeadState ) )
		{
		PRINT_EXIT;
		return ETrue;
		}
	else
		{
		PRINT_EXIT;
		return EFalse;
		}
	}

//---------------------------------------------------------------------------
//Destructor for StateMachine
//---------------------------------------------------------------------------
//
CStateMachine::~CStateMachine()
	{}
