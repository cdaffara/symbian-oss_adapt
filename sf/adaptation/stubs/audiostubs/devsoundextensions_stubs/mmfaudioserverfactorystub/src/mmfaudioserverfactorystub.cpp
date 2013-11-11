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
* Description: Audio Stubs -  Audio Server factory stub class implementation.
*
*/

#include "devsoundextensionstubs_common.h"
#include "mmfaudioserverfactorystub.h"
#include "telephonyaudioroutingmanagerstub.h"

CMMFAudioServerFactoryStub::CMMFAudioServerFactoryStub()
    {
    	// No Impl
    }

EXPORT_C MAudioSvrService* CMMFAudioServerFactoryStub::NewL()
    {
    DB_IN;
    CMMFAudioServerFactoryStub* self = new(ELeave)CMMFAudioServerFactoryStub();
    MAudioSvrService* ptr = static_cast<MAudioSvrService*>(self);
    DB_OUT;
    return ptr;
    }

void CMMFAudioServerFactoryStub::PassDestructorKey(TUid aUid)
    {
    DB_IN;
    iDestructorKeyUid = aUid;
    DB_OUT;
    }

TInt CMMFAudioServerFactoryStub::Load()
    {
    DB_IN;
    TInt err(KErrNone);
	DB_OUT;
	return err;
    }

TInt CMMFAudioServerFactoryStub::Start()
    {
    DB_IN;
    TInt err(KErrNone);
    iTelephonyAudioRoutingManStub = CTelephonyAudioRoutingManagerStub::NewL();
    DB_OUT;
    return err;
    }

void CMMFAudioServerFactoryStub::Stop()
    {
		// No Impl
	}

void CMMFAudioServerFactoryStub::Release()
    {
    DB_IN;
	delete iTelephonyAudioRoutingManStub;
    REComSession::DestroyedImplementation(iDestructorKeyUid);
    DB_OUT;
    }
