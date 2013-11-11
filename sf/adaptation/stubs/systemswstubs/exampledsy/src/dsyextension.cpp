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
*    It implements the class CDosExtensionExample.
*
*/


#include "DsyExtension.h"
#include "DsyDebug.h"

//
// ---------------------------------------------------------
// CDosExtensionExample::NewL
// ---------------------------------------------------------
//  
CDosExtensionExample* CDosExtensionExample::NewL()
{
	FLOG(_L("CDosExtensionExample::NewL()"));

	CDosExtensionExample* result = new (ELeave) CDosExtensionExample;

	CleanupStack::PushL(result);
	result->ConstructL();
	CleanupStack::Pop();

	return result;
}

//
// ---------------------------------------------------------
// CDosExtensionExample::ConstructL
// ---------------------------------------------------------
//  
void CDosExtensionExample::ConstructL()
{
	FLOG(_L("CDosExtensionExample::ConstructL()"));
}


//
// ---------------------------------------------------------
// CDosExtensionExample::CallFunctionL
// ---------------------------------------------------------
//  
TInt CDosExtensionExample::CallFunctionL(TInt /*aFunc*/ , TAny* /*aParameter*/ , TInt /*aParLength*/, TBool& /*aParameterModified*/)
{
	FLOG(_L("CDosExtensionExample::CallFunctionL()"));

	return KErrNone;
}


//
// ---------------------------------------------------------
// CDosExtensionExample::CallFunctionAndCompleteL
// ---------------------------------------------------------
//  
void CDosExtensionExample::CallFunctionAndCompleteL(TInt /*aFunc*/ , TAny* /*aParameter*/ , TInt /*aParLength*/, const RMessage2& aMessage)
{
	FLOG(_L("CDosExtensionExample::CallFunctionAndCompleteL()"));

	CompleteRequest(aMessage,KErrNone);
}
