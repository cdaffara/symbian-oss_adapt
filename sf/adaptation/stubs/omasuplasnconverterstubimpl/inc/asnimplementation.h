/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: ASN plugin
*
*/

// This file defines the API for AsnImplementation.dll

#ifndef __ASNIMPLEMENTATION_H__
#define __ASNIMPLEMENTATION_H__

//  Include Files

#include <e32base.h>	// CBase
#include <e32std.h>	 // TBuf
#include <ecom/implementationproxy.h>

//  Constants

const TInt KAsnImplementationBufferLength = 15;
typedef TBuf<KAsnImplementationBufferLength> TAsnImplementationExampleString;

//  Class Definitions

class CAsnImplementation : public CBase
    {
public:
    // new functions
    IMPORT_C static CAsnImplementation* NewL();
    IMPORT_C static CAsnImplementation* NewLC();
    IMPORT_C ~CAsnImplementation();

public:
    // new functions, example API
    IMPORT_C TVersion Version() const;
    IMPORT_C void ExampleFuncAddCharL(const TChar& aChar);
    IMPORT_C void ExampleFuncRemoveLast();
    IMPORT_C const TPtrC ExampleFuncString() const;

private:
    // new functions
    CAsnImplementation();
    void ConstructL();

private:
    // data
    TAsnImplementationExampleString* iString;
    };

#endif  // __ASNIMPLEMENTATION_H__

