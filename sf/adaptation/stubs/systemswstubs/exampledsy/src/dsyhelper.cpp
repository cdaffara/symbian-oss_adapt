/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*    It implements the class CDosHelperExample.
*
*/


#include "DsyHelper.h"
#include "DsyDebug.h"

//
// ---------------------------------------------------------
// CDosHelperExample::NewL
// ---------------------------------------------------------
//  
CDosHelperExample* CDosHelperExample::NewL()
{
	FLOG(_L("CDosHelperExample::NewL()"));

	CDosHelperExample* result = new (ELeave) CDosHelperExample;

	CleanupStack::PushL(result);
	result->ConstructL();
	CleanupStack::Pop();

	return result;
}

//
// ---------------------------------------------------------
// CDosHelperExample::ConstructL
// ---------------------------------------------------------
//  
void CDosHelperExample::ConstructL()
{
	FLOG(_L("CDosHelperExample::ConstructL()"));
}

//
// ---------------------------------------------------------
// CDosHelperExample::GetStartupReasonL
// ---------------------------------------------------------
//  
TInt CDosHelperExample::GetStartupReasonL(TInt& aReason)
{
	FLOG(_L("CDosHelperExample::GetStartupReasonL()"));

/*Returns the Startup Reason from the DOS point of view. These are the values (defined in
  startupreasons.h):

	ENormal
	EAlarm
	EMidnightAlarm
	EHiddenReset
	ECharger
	ETest
	ELocal
	ESelftestFail
	ENotReadFromHardware
	ENotKnown
*/
	aReason = 0; //ENormal

	return KErrNone;
}


//
// ---------------------------------------------------------
// CDosHelperExample::GetSWStartupReasonL
// ---------------------------------------------------------
//  
TInt CDosHelperExample::GetSWStartupReasonL(TInt16& aReason)
{
	FLOG(_L("CDosHelperExample::GetSWStartupReasonL()"));

	// provide a list with the Possible Startup Reasons

	
	//Retrieve from one previousily stored in SetSWStartupReason
	aReason = 100; //Normal


	return KErrNone;
}

//
// ---------------------------------------------------------
// CDosHelperExample::SetSWStartupReasonL
// ---------------------------------------------------------
//  
TInt CDosHelperExample::SetSWStartupReasonL(TInt16 /*aReason*/)
{
	FLOG(_L("CDosHelperExample::SetSWStartupReasonL()"));

	//Store the value to be got from GetSWStartupReason

	return KErrNone;
}


//
// ---------------------------------------------------------
// CDosHelperExample::HiddenResetL
// ---------------------------------------------------------
//  
TBool CDosHelperExample::HiddenResetL()
{
	FLOG(_L("CDosHelperExample::HiddenResetL()"));

	//Checks if the last reset was hidden or not.
	return EFalse;
}


//
// ---------------------------------------------------------
// CDosHelperExample::GetRTCStatusL
// ---------------------------------------------------------
//  
TInt CDosHelperExample::GetRTCStatusL(TBool& aStatus)
{
	FLOG(_L("CDosHelperExample::GetRTCStatusL()"));

	// EFalse asks time and date in startup
	aStatus = ETrue;

	return KErrNone;
}


//
// ---------------------------------------------------------
// CDosHelperExample::GenerateGripEventL
// ---------------------------------------------------------
//  
TInt CDosHelperExample::GenerateGripEventL()
{
	FLOG(_L("CDosHelperExample::GenerateGripEventL()"));

	return KErrNone;
}
