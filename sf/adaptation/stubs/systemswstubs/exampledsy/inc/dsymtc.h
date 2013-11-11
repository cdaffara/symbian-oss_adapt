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
*    It describes the CDosMtcExample class
*
*/


#ifndef __DSYMTC_H__
#define __DSYMTC_H__

#include "DosMtcBase.h"

/**
* Plug-in class that implements the MDosMtcBaseDSY class and 
* satisfies the RDosMtc requests from the clients.
*/
class CDosMtcExample : public CDosMtcBase
{
public:
	/**
	* NewL function that creates the object.
	* @return The newly created object.
	*/
	static CDosMtcExample* NewL();

	//======================================================
	// See MDosMtcBaseDSY class for a description of
	// the following functions.
	//======================================================
	TInt PowerOnL();
	TInt PowerOffL();
	TInt DosSyncL();
	TInt ResetGenerateL();
	TInt SetStateL(const TInt aStateType);

	TInt SetStateFlagL(const TDosStateFlag aFlag);
	TInt GetStateFlagL(TDosStateFlag& aFlag);

private:
	/**
	* Symbian two-phased constructor.
	*/
	void ConstructL();
};

#endif //__DSYMTC_H__
