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
*    It describes the CDosSysUtilsExample class
*
*/


#ifndef __DSYSYSUTILS_H__
#define __DSYSYSUTILS_H__

#include "DosSysUtilsBase.h"

/**
* Plug-in class that implements the MDosSysUtilsBaseDSY class and 
* satisfies the RDosSysUtils requests from the clients.
*/
class CDosSysUtilsExample : public CDosSysUtilsBase
{
public:
	/**
	* NewL function that creates the object.
	* @return The newly created object.
	*/
	static CDosSysUtilsExample* NewL();

	//======================================================
	// See MDosSysUtilsBaseDSY class for a description of
	// the following functions.
	//======================================================
	TInt GetSimLanguageL(TInt& aSimLanguage);
	void PerformDosRfsL(TUint aReason, const RMessagePtr2 aMessage);
	TInt SetDosAlarmL(TTime* aAlarmTime);

private:
	/**
	* Symbian two-phased constructor.
	*/
	void ConstructL();
};

#endif //__DSYSYSUTILS_H__
