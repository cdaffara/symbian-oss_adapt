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
*    It describes the CDosSaeExample class
*
*/


#ifndef __DSYSAE_H__
#define __DSYSAE_H__

#include "DosSaeBase.h"

//Class for the SAE Service

/**
* Plug-in class that implements the MDosSaeBaseDSY class and 
* satisfies the RDosSae requests from the clients.
*/
class CDosSaeExample : public CDosSaeBase
{
public:
	/**
	* NewL function that creates the object.
	* @return The newly created object.
	*/
	static CDosSaeExample* NewL();

	//======================================================
	// See MDosSaeBaseDSY class for a description of
	// the following function.
	//======================================================
	TInt StartSaeL();

private:
	/**
	* Symbian two-phased constructor.
	*/
	void ConstructL();
};


#endif //__DSYSAE_H__
