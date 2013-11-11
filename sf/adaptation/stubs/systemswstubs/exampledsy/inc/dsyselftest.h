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
*    It describes the CDosSelfTestExample class
*
*/


#ifndef __DSYSELFTEST_H__
#define __DSYSELFTEST_H__

#include "DosSelfTestBase.h"


/**
* Plug-in class that implements the MDosSelfTestBaseDSY class and 
* satisfies the RDosSelftTest requests from the clients.
*/
class CDosSelfTestExample : public CDosSelfTestBase
{
public:
	/**
	* NewL function that creates the object.
	* @return The newly created object.
	*/
	static CDosSelfTestExample* NewL();

	//======================================================
	// See MDosSelfTestBaseDSY class for a description of
	// the following function.
	//======================================================
	TInt PerformSelfTestL();

private:
	/**
	* Symbian two-phased constructor.
	*/
	void ConstructL();
};



#endif //__DSYSELFTEST_H__
