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

#ifndef __ASNIMPLEMENTATION_PAN__
#define __ASNIMPLEMENTATION_PAN__

//  Data Types

enum TAsnImplementationPanic
    {
    EAsnImplementationNullPointer
    };

//  Function Prototypes

GLREF_C void Panic(TAsnImplementationPanic aPanic);

#endif  // __ASNIMPLEMENTATION_PAN__

