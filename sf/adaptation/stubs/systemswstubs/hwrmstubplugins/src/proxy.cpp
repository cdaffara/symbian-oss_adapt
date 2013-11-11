/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Hardware Resource Manager stub plugins proxy definition file
*
*/


#include <e32std.h>
#include <ecom/implementationproxy.h>

#include "LightPlugin.h"
#include "VibraPlugin.h"
#include "FmtxPlugin.h"
#include "PowerStatePlugin.h"

// Map the interface implementation UIDs to implementation factory functions
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY(0x1020503F,	CLightPlugin::NewL),
    IMPLEMENTATION_PROXY_ENTRY(0x1020503E,	CVibraPlugin::NewL),
    IMPLEMENTATION_PROXY_ENTRY(0x2000BF15,	CFmtxPlugin::NewL),
    IMPLEMENTATION_PROXY_ENTRY(0x1020503D,	CPowerStatePlugin::NewL)
    };

// Exported proxy for instantiation method resolution.
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }
