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
*    It implements the class CDosSysUtilsExample.
*
*/


#include "DsySysUtils.h"
#include "DsyDebug.h"

//
// ---------------------------------------------------------
// CDosSysUtilsExample::NewL
// ---------------------------------------------------------
//  
CDosSysUtilsExample* CDosSysUtilsExample::NewL()
{
	FLOG(_L("CDosSysUtilsExample::NewL()"));

	CDosSysUtilsExample* result = new (ELeave) CDosSysUtilsExample;

	CleanupStack::PushL(result);
	result->ConstructL();
	CleanupStack::Pop();

	return result;
}


//
// ---------------------------------------------------------
// CDosSysUtilsExample::ConstructL
// ---------------------------------------------------------
//  
void CDosSysUtilsExample::ConstructL()
{
	FLOG(_L("CDosSysUtilsExample::ConstructL()"));
}

//
// ---------------------------------------------------------
// CDosSysUtilsExample::GetSimLanguageL
// ---------------------------------------------------------
//  
TInt CDosSysUtilsExample::GetSimLanguageL(TInt& aSimLanguage)
{
	FLOG(_L("CDosSysUtilsExample::GetSimLanguageL()"));

	//Checks the default language in the SIM card and returns it.

	aSimLanguage = 1;
  
	return KErrNone;
}

//
// ---------------------------------------------------------
// CDosSysUtilsExample::PerformDosRfsL
// ---------------------------------------------------------
//  
void CDosSysUtilsExample::PerformDosRfsL(TUint /*aReason*/, const RMessagePtr2 aMessage)
{
	FLOG(_L("CDosSysUtilsExample::PerformDosRfsL()"));

	RequestComplete(aMessage,KErrNone);
}


//
// ---------------------------------------------------------
// CDosSysUtilsExample::SetDosAlarmL
// ---------------------------------------------------------
//  
TInt CDosSysUtilsExample::SetDosAlarmL(TTime* /*aAlarmTime*/)
{
	FLOG(_L("CDosSysUtilsExample::SetDosAlarmL()"));

	return KErrNone;
}


