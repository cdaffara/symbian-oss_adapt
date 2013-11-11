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
*    It implements the class CDosFactoryExample.
*
*/


#include "DsySysUtils.h"
#include "DsyHelper.h"
#include "DsyMtc.h"
#include "DsySelfTest.h"

#include "DsySae.h"

#include "DsyExtension.h"
#include "DsyFactory.h"
#include "DsyDebug.h"
//
// ---------------------------------------------------------
// CDosFactoryExample::NewL
// ---------------------------------------------------------
//  
CDosFactoryExample* CDosFactoryExample::NewL()
{
	FLOG(_L("CDosFactoryExample::NewL()"));

	CDosFactoryExample* result = new (ELeave) CDosFactoryExample;

	CleanupStack::PushL(result);
	result->ConstructL();
	CleanupStack::Pop();

	return result;
}

//
// ---------------------------------------------------------
// CDosFactoryExample::ConstructL
// ---------------------------------------------------------
//  
void CDosFactoryExample::ConstructL()
{
	FLOG(_L("CDosFactoryExample::ConstructL()"));
}


//
// ---------------------------------------------------------
// CDosFactoryExample::NewSysUtilsServiceL
// ---------------------------------------------------------
//  
CDosSysUtilsBase* CDosFactoryExample::NewSysUtilsServiceL()
{
	FLOG(_L("CDosFactoryExample::NewSysUtilsServiceL()"));

	return CDosSysUtilsExample::NewL();
}

//
// ---------------------------------------------------------
// CDosFactoryExample::NewHelperServiceL
// ---------------------------------------------------------
//  
CDosHelperBase* CDosFactoryExample::NewHelperServiceL()
{
	FLOG(_L("CDosFactoryExample::NewHelperServiceL()"));

	return CDosHelperExample::NewL();
}

//
// ---------------------------------------------------------
// CDosFactoryExample::NewMtcServiceL
// ---------------------------------------------------------
//  
CDosMtcBase* CDosFactoryExample::NewMtcServiceL()
{
	FLOG(_L("CDosFactoryExample::NewMtcServiceL()"));

	return CDosMtcExample::NewL();
}

//
// ---------------------------------------------------------
// CDosFactoryExample::NewSelfTestServiceL
// ---------------------------------------------------------
//  
CDosSelfTestBase* CDosFactoryExample::NewSelfTestServiceL()
{
	FLOG(_L("CDosFactoryExample::NewSelfTestServiceL()"));

	return CDosSelfTestExample::NewL();
}

//
// ---------------------------------------------------------
// CDosFactoryExample::NewSaeServiceL
// ---------------------------------------------------------
//  
CDosSaeBase* CDosFactoryExample::NewSaeServiceL()
{
	FLOG(_L("CDosFactoryExample::NewSaeServiceL()"));

	return CDosSaeExample::NewL();
}

//
// ---------------------------------------------------------
// CDosFactoryExample::NewExtensionServiceL
// ---------------------------------------------------------
//  
CDosExtensionBase* CDosFactoryExample::NewExtensionServiceL()
{
	FLOG(_L("CDosFactoryExample::NewExtensionServiceL()"));

	return CDosExtensionExample::NewL();
}

//
// ---------------------------------------------------------
// Polymorphic Dll Entry Point
// ---------------------------------------------------------
//  
EXPORT_C CDosFactoryBase* LibEntryL()
{
	FLOG(_L("CDosFactoryExample::LibEntryL()"));

	return CDosFactoryExample::NewL();
}
