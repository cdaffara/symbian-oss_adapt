/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: CI stub plugin
*
*/



// This file defines the API for AudiPlayRoutingControlCIStub.dll

#ifndef CISTUBPLUGIN_H
#define CISTUBPLUGIN_H

#include <e32base.h>	// CBase
#include <e32std.h>	 // TBuf
#include <e32debug.h>
#include <ecom.h>
#include "RestrictedAudioOutputCIStub.h"
#include "AudioOutputCI.h"
#include "AudioInputCI.h"
#include "AudioOutputMessageTypes.h"
#include "AudioInputMessageTypes.h"

class MyMCustomInterface
    {
public:
    virtual TAny* CustomInterface(TUid aCustomeInterface) = 0;
    virtual void Release() = 0;
    virtual void PassDestructionKey(TUid aUid) = 0;
    };

class CCIStubPlugin : public CBase,public MyMCustomInterface
    {
public:

    static MyMCustomInterface* NewL();
    static CCIStubPlugin* NewLC();
    ~CCIStubPlugin();
    TAny* CustomInterface(TUid aInterfaceId);
    void Release();
    void PassDestructionKey(TUid aUid);

private:
    TUid iDtor_ID_Key;
    CCIStubPlugin();
    void ConstructL();
    };

#endif  // __AUDIPLAYROUTINGCONTROLHWDEVICE_H__
