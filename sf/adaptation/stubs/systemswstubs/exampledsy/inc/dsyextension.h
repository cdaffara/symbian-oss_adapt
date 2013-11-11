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
*    It describes the CDosExtensionExample class
*
*/


#ifndef __DSYEXTENSION_H__
#define __DSYEXTENSION_H__

#include "DosExtensionBase.h"

/**
* Plug-in class that implements the MDosExtensionBaseDSY class and to 
* satisfy the RDosExtension requests from the client side.
*/
class CDosExtensionExample : public CDosExtensionBase
{
public:
	/**
	* NewL function that creates the object.
	* @return The newly created object.
	*/
	static CDosExtensionExample* NewL();

	//======================================================
	// See MDosExtensionBaseDSY class for a description of
	// the following functions.
	//======================================================
	TInt CallFunctionL(TInt aFunc , TAny *aParameter , TInt aParLength, TBool& aParameterModified);
	void CallFunctionAndCompleteL(TInt aFunc , TAny *aParameter , TInt aParLength, const RMessage2& aMessage);

private:
	/**
	* Symbian two-phased constructor.
	*/
	void ConstructL();
};


#endif //__DSYEXTENSION_H__
