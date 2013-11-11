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
*    It describes the CDosHelperExample class
*
*/


#ifndef __DSYHELPER_H__
#define __DSYHELPER_H__

#include "DosHelperBase.h"

/**
* Plug-in class that implements the MDosHelperBaseDSY class and 
* satisfies the RDosHelper requests from the clients.
*/
class CDosHelperExample : public CDosHelperBase
{
public:
	/**
	* NewL function that creates the object.
	* @return The newly created object.
	*/
	static CDosHelperExample* NewL();

	//======================================================
	// See MDosHelperBaseDSY class for a description of
	// the following functions.
	//======================================================
	TInt GetStartupReasonL(TInt& aReason);
	TInt GetSWStartupReasonL(TInt16& aReason);
	TInt SetSWStartupReasonL(TInt16 aReason);
	TBool HiddenResetL();
	TInt GetRTCStatusL(TBool& aStatus);
	TInt GenerateGripEventL();

private:
	/**
	* Symbian two-phased constructor.
	*/
	void ConstructL();
};

#endif //__DSYHELPER_H__
