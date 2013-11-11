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
 * Description: Generic Stub plugin that load required CI stub in WINSCW 
 *
 */



#include "cistubplugin.h"	

#ifdef _DEBUG
#define DEBPRN0         RDebug::Printf( "*CI STUB PLUGIN* %s", __PRETTY_FUNCTION__);
#define DEBPRN1(str)    RDebug::Printf( "%s %s", __PRETTY_FUNCTION__, str );
#else
#define DEBPRN0
#define DEBPRN1(str)
#endif

#define KTenSeconds 3000000
//  Member Functions


CCIStubPlugin* CCIStubPlugin::NewLC()
    {
    DEBPRN0;
    CCIStubPlugin* self = new (ELeave) CCIStubPlugin;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

MyMCustomInterface* CCIStubPlugin::NewL()
    {
    DEBPRN0;
    CCIStubPlugin* self = CCIStubPlugin::NewLC();
    CleanupStack::Pop(self);
    return (MyMCustomInterface*) self;
    }

CCIStubPlugin::CCIStubPlugin()
// note, CBase initialises all member variables to zero
    {
    DEBPRN0;
    }

void CCIStubPlugin::ConstructL()
    {
    DEBPRN0;
    }

CCIStubPlugin::~CCIStubPlugin()
    {
    DEBPRN0;
    }

TAny* CCIStubPlugin::CustomInterface(TUid aInterfaceId)
    {
    DEBPRN0;
    if (aInterfaceId == KUidRestrictedAudioOutput)
        {
        return (TAny*)CRestrictedAudioOutputCI::NewL();
        }
    if(aInterfaceId == KUidAudioOutput)
        {
        return (TAny*)CAudioOutputCI::NewL();
        }
    if(aInterfaceId == KUidAudioInput)
        {
        return (TAny*)CAudioInputCI::NewL();
        }
    return NULL;
    }

void CCIStubPlugin::Release()
    {
    DEBPRN0;
    REComSession::DestroyedImplementation(iDtor_ID_Key);
    delete this;
    }
void CCIStubPlugin::PassDestructionKey(TUid aUid)
    {
    DEBPRN0;
    iDtor_ID_Key = aUid;
    }
