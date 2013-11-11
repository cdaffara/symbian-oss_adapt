/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Used as an reference implementation.
*
*/


// INCLUDE FILES
#include <ecom/implementationproxy.h>
#include "WiredAccessoryKeyEventHandler.h"

// CONSTANTS
const TImplementationProxy ImplementationTable[] = 
    {
    IMPLEMENTATION_PROXY_ENTRY( 0x10205075, CWiredAccessoryKeyEventHandler::NewL )//lint !e611
    };


// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// TImplementationProxy* ImplementationGroupProxy
// -----------------------------------------------------------------------------
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aCount )
    {
    aCount = sizeof ( ImplementationTable ) / sizeof ( TImplementationProxy );

    return ImplementationTable;
    }

//  End of File  

