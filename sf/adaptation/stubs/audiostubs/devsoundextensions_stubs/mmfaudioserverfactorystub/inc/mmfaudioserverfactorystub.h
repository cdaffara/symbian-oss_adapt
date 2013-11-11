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
* Description: Audio Stubs -  CMMFAudioServerFactoryStub class declaration.
*
*/

#ifndef MMFAUDIOSERVERFACTORYSTUB_H
#define MMFAUDIOSERVERFACTORYSTUB_H

#include <a3f/mmfaudiosvrservice.h>

// FORWARD DECLARATION
class CTelephonyAudioRoutingManagerStub;

// CLASS DECLARATION
class CMMFAudioServerFactoryStub : public CBase,
                                   public MAudioSvrService
    {
    public:
    IMPORT_C static MAudioSvrService* NewL();

    virtual void PassDestructorKey(TUid aUid);
    virtual TInt Load();
    virtual TInt Start();
    virtual void Stop();
    virtual void Release();

    private:
    CMMFAudioServerFactoryStub();
    TUid iDestructorKeyUid;

    private:
    CTelephonyAudioRoutingManagerStub* iTelephonyAudioRoutingManStub;
    };

#endif // MMFAUDIOSERVERFACTORYSTUB_H
