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
*    It implements the class CDosSelfTestExample.
*
*/


#include "DsySelfTest.h"
#include "DosEventManager.h"
#include "DsyDebug.h"

//
// ---------------------------------------------------------
// CDosSelfTestExample::NewL
// ---------------------------------------------------------
//  
CDosSelfTestExample* CDosSelfTestExample::NewL()
{
	FLOG(_L("CDosSelfTestExample::NewL()"));

	CDosSelfTestExample* result = new (ELeave) CDosSelfTestExample;

	CleanupStack::PushL(result);
	result->ConstructL();
	CleanupStack::Pop();

	return result;
}


//
// ---------------------------------------------------------
// CDosSelfTestExample::ConstructL
// ---------------------------------------------------------
//  
void CDosSelfTestExample::ConstructL()
{
	FLOG(_L("CDosSelfTestExample::ConstructL()"));
}


//
// ---------------------------------------------------------
// CDosSelfTestExample::PerformSelfTestL
// ---------------------------------------------------------
//  
TInt CDosSelfTestExample::PerformSelfTestL()
{
	FLOG(_L("CDosSelfTestExample::PerformSelfTestL()"));
	EventManager()->NotifySelfTestStatus(3); //EStartupSTOk = 3
	return KErrNone;
}
