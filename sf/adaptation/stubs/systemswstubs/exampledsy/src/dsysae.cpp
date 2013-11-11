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
*    It implements the class CDosSaeExample.
*
*/


#include <e32base.h>
#include "DosEventManager.h"
#include "DsySae.h"
#include "DsyDebug.h"

//
// ---------------------------------------------------------
// CDosSaeExample::NewL
// ---------------------------------------------------------
//  
CDosSaeExample* CDosSaeExample::NewL()
{
	FLOG(_L("CDosSaeExample::NewL()"));

	CDosSaeExample* result = new (ELeave) CDosSaeExample;

	CleanupStack::PushL(result);
	result->ConstructL();
	CleanupStack::Pop();

	return result;
}


//
// ---------------------------------------------------------
// CDosSaeExample::ConstructL
// ---------------------------------------------------------
//  
void CDosSaeExample::ConstructL()
{
	FLOG(_L("CDosSaeExample::ConstructL()"));
}


//
// ---------------------------------------------------------
// CDosSaeExample::StartSaeL
// ---------------------------------------------------------
//  
TInt CDosSaeExample::StartSaeL()
{
	FLOG(_L("CDosSaeExample::StartSaeL()"));

	EventManager()->PhonePowerOn();
	EventManager()->SimState(ESimStateOk);
	EventManager()->SecurityCode(EPSSecurityCodeNotRequired);
	EventManager()->SimPresentStateChangedL(ESimPresentTrue);

	return KErrNone;
}

