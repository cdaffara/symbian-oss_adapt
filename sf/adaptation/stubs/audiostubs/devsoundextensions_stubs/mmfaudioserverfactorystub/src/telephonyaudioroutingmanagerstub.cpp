/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  TelephonyAudioRoutingManagerStub class implementation.
*				Starts the TelephonyAudioRoutingManager.
*
*/

#include <TelephonyAudioRoutingManager.h>

#include "devsoundextensionstubs_common.h"
#include "telephonyaudioroutingmanagerstub.h"

CTelephonyAudioRoutingManagerStub::CTelephonyAudioRoutingManagerStub()
    {
    	// No Impl
    }

CTelephonyAudioRoutingManagerStub::~CTelephonyAudioRoutingManagerStub()
    {
    DB_IN;
    iAudioOutput.Close();
    delete iTelephonyAudioRoutingMan;
    DB_OUT;
    }

CTelephonyAudioRoutingManagerStub* CTelephonyAudioRoutingManagerStub::NewL()
    {
    DB_IN;
    CTelephonyAudioRoutingManagerStub* self = new (ELeave) CTelephonyAudioRoutingManagerStub;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    DB_OUT;
    return self;
    }

void CTelephonyAudioRoutingManagerStub::ConstructL()
    {
    DB_IN;
    // Configure and launch Telephony Audio Routing Manager
    iAudioOutput.Reset();
    iAudioOutput.Append(CTelephonyAudioRouting::EHandset);
    iAudioOutput.Append(CTelephonyAudioRouting::ELoudspeaker);

    iTelephonyAudioRoutingMan = CTelephonyAudioRoutingManager::NewL(*this, iAudioOutput.Array());

    DB_CUSTOM0(_L("Telephony Audio Routing Manager created"));
    DB_OUT;
    }

void CTelephonyAudioRoutingManagerStub::OutputChangeRequested (
    CTelephonyAudioRoutingManager& /*aTelephonyAudioRoutingManager*/,
    CTelephonyAudioRouting::TAudioOutput /*aOutput*/)
    {
    	// No Impl
    }
