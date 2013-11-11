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
* Description:  CTvOutBehaviourImpl class implementation.
*
*/

#include "tvoutbehaviourimpl.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Symbian two phased constructor.
// ---------------------------------------------------------------------------
//
CTvOutBehaviourImpl* CTvOutBehaviourImpl::NewL()
    {
    CTvOutBehaviourImpl* self = new (ELeave) CTvOutBehaviourImpl();
    CleanupStack::Pop( self );
    return self;
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::~CTvOutBehaviourImpl
//
CTvOutBehaviourImpl::~CTvOutBehaviourImpl()
    {
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::SetTvOutSettings
//
TInt CTvOutBehaviourImpl::SetTvOutSettings( const TTvOutSettings& /*aParams*/ )
    {
    return KErrNotSupported;
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::DeActivateSettings
//
TInt CTvOutBehaviourImpl::DeActivateSettings()
    {
    return KErrNotSupported;
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::GetTvOutScreenDevices
//
TUint CTvOutBehaviourImpl::GetTvOutScreenDevices()
    {
    return EScreenNone;
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::GetTvOutDisplayNumber
//
TInt CTvOutBehaviourImpl::GetTvOutDisplayNumber()
    {
    return KErrNotSupported;
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::GetTvOutSettings
//
TInt CTvOutBehaviourImpl::GetTvOutSettings( TTvOutSettings& /*aParams*/ )
    {
    return KErrNotSupported;
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::SettingsListener
//
TInt CTvOutBehaviourImpl::SettingsListener( TRequestStatus& /*aStatus*/ )
    {
    return KErrNotSupported;
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::CancelSettingsListener
//
TBool CTvOutBehaviourImpl::CancelSettingsListener()
    {
    return EFalse;
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::GetTvOutDefaultSettings
//
TInt CTvOutBehaviourImpl::GetTvOutDefaultSettings( TTvOutSettings& /*aParams*/ )
    {
    return KErrNotSupported;
    }

//---------------------------------------------------------------------------
// CTvOutBehaviourImpl::CTvOutBehaviourImpl
//
CTvOutBehaviourImpl::CTvOutBehaviourImpl()
    {
    }

// End of File
