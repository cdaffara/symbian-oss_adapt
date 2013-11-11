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
* Description: Declaration of ASN plugin
*
*/

//  Include Files  

#include "asnimplementation.h"	// CAsnImplementation
#include "asnimplementation.pan"	  	// panic codes
#include <ecom/implementationproxy.h>
#include "asnimplementationBase.h"

//  Member Functions

EXPORT_C CAsnImplementation* CAsnImplementation::NewLC()
    {
    CAsnImplementation* self = new (ELeave) CAsnImplementation;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CAsnImplementation* CAsnImplementation::NewL()
    {
    CAsnImplementation* self = CAsnImplementation::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

CAsnImplementation::CAsnImplementation()
// note, CBase initialises all member variables to zero
    {
    }

void CAsnImplementation::ConstructL()
    {
    // second phase constructor, anything that may leave must be constructed here
    iString = new (ELeave) TAsnImplementationExampleString;
    }

EXPORT_C CAsnImplementation::~CAsnImplementation()
    {
    delete iString;
    }

EXPORT_C TVersion CAsnImplementation::Version() const
    {
    // Version number of example API
    const TInt KMajor = 1;
    const TInt KMinor = 0;
    const TInt KBuild = 1;
    return TVersion(KMajor, KMinor, KBuild);
    }

EXPORT_C void CAsnImplementation::ExampleFuncAddCharL(const TChar& aChar)
    {
    __ASSERT_ALWAYS(iString != NULL, Panic(EAsnImplementationNullPointer));

    if (iString->Length() >= KAsnImplementationBufferLength)
        {
        User::Leave(KErrTooBig);
        }

    iString->Append(aChar);
    }

EXPORT_C void CAsnImplementation::ExampleFuncRemoveLast()
    {
    __ASSERT_ALWAYS(iString != NULL, Panic(EAsnImplementationNullPointer));

    if (iString->Length() > 0)
        {
        iString->SetLength(iString->Length() - 1);
        }
    }

EXPORT_C const TPtrC CAsnImplementation::ExampleFuncString() const
    {
    __ASSERT_ALWAYS(iString != NULL, Panic(EAsnImplementationNullPointer));
    return *iString;
    }

const TImplementationProxy ImplementationTable[] =
    {
    
    #ifdef __EABI__ 
        IMPLEMENTATION_PROXY_ENTRY(0xE4D72222, CAsnImplementationBase::NewL) 
    #else
        { { 0xE4D72222 },  CAsnImplementationBase::NewL}
    #endif        
        
    };

// Exported proxy for instantiation method resolution.
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(
    TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }
