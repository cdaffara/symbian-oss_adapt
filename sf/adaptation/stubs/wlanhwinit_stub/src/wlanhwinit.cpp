/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The implementation of CWlanHwInit class
*
*/

/*
* %version: 5 %
*/

#include <wlanhwinit.h>

// ============================ MEMBER FUNCTIONS ===============================

CWlanHwInit::CWlanHwInit() :
    iMain( NULL )
    {
    }

void CWlanHwInit::ConstructL()
    {
    }

EXPORT_C CWlanHwInit* CWlanHwInit::NewL()
    {
    CWlanHwInit* self = new( ELeave ) CWlanHwInit;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
    
EXPORT_C CWlanHwInit::~CWlanHwInit()
    {
    }

// -----------------------------------------------------------------------------
// CWlanHwInit::GetHwInitData
// -----------------------------------------------------------------------------
//
EXPORT_C void CWlanHwInit::GetHwInitData(
    const TUint8** aInitData,
    TUint& aInitLength,
    const TUint8** aFwData,
    TUint& aFwLength )
    {
    *aInitData = NULL;
    aInitLength = 0;
    *aFwData = NULL;
    aFwLength = 0;
    }

// -----------------------------------------------------------------------------
// CWlanHwInit::GetMacAddress
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWlanHwInit::GetMacAddress(
    TMacAddr& aMacAddress )
    {
    aMacAddress.iMacAddress[0] = 0x00;
    aMacAddress.iMacAddress[1] = 0xE0;
    aMacAddress.iMacAddress[2] = 0xDE;
    aMacAddress.iMacAddress[3] = 0xAD;
    aMacAddress.iMacAddress[4] = 0xBE;
    aMacAddress.iMacAddress[5] = 0xEF;   
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CWlanHwInit::GetHwTestInitData
// -----------------------------------------------------------------------------
//
EXPORT_C void CWlanHwInit::GetHwTestInitData(
    const TUint8** aInitData,
    TUint& aInitLength,
    const TUint8** aFwData,
    TUint& aFwLength )
    {
    *aInitData = NULL;
    aInitLength = 0;
    *aFwData = NULL;
    aFwLength = 0;    
    }

// -----------------------------------------------------------------------------
// CWlanHwInit::GetHwTestData
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWlanHwInit::GetHwTestData(
    TUint /*aId*/,
    TDes8& /*aData*/ )
    {
    return KErrNotSupported;
    }

// -----------------------------------------------------------------------------
// CWlanHwInit::SetHwTestData
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CWlanHwInit::SetHwTestData(
    TUint /*aId*/,
    TDesC8& /*aData*/ )
    {
    return KErrNotSupported;
    }
