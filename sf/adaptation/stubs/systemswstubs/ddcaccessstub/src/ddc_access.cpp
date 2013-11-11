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
 * Description:
 * CDdcPortAccess class implementation.
 *
 */

// SYSTEM INCLUDES

// USER INCLUDES
#include "ddc_access.h"
#include "ddc_accessimpl.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Symbian two phased constructor.
// ---------------------------------------------------------------------------
//
EXPORT_C CDdcPortAccess* CDdcPortAccess::NewL()
    {
    return CDdcPortAccessImpl::NewL();
    }

//------------------------------------------------------------------------------
// CDdcPortAccess::ApiVersion
//------------------------------------------------------------------------------
//
EXPORT_C TUint CDdcPortAccess::ApiVersion()
    {
    return KDdcAccessVersion;
    }

// End of file
