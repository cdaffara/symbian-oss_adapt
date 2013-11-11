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
*    It describes the CDosFactoryExample class
*
*/


#ifndef __DSYFACTORY_H__
#define __DSYFACTORY_H__

#include "DosFactoryBase.h"

/**
* Plug-in class that implements the MDosFactoryBaseDSY class and 
* creates all the other plug-in services.
*/
class CDosFactoryExample : public CDosFactoryBase
{
public:
	/**
	* NewL function that creates the object.
	* @return The newly created object.
	*/
	static CDosFactoryExample* NewL();

	//======================================================
	// See MDosFactoryBaseDSY class for a description of
	// the following functions.
	//======================================================

	CDosSysUtilsBase* NewSysUtilsServiceL();
	CDosHelperBase* NewHelperServiceL();
	CDosMtcBase* NewMtcServiceL();
	CDosSelfTestBase* NewSelfTestServiceL();

    CDosSaeBase* NewSaeServiceL();
    
	CDosExtensionBase* NewExtensionServiceL();

private:
	/**
	* Symbian two-phased constructor.
	*/
	void ConstructL();
};

#endif //__DSYFACTORY_H__
