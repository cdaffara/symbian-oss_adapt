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
* Description: Audio Stubs -  CTelephonyAudioRoutingManagerStub class declaration.
*
*/

#ifndef TELEPHONYAUDIOROUTINGMANAGERSTUB_H
#define TELEPHONYAUDIOROUTINGMANAGERSTUB_H

#include <MTelephonyAudioRoutingPolicyObserver.h>

// FORWARD DECLARATIONS
class CTelephonyAudioRoutingManager;

// CLASS DECLARATION
class CTelephonyAudioRoutingManagerStub :
    public CBase,
    public MTelephonyAudioRoutingPolicyObserver
    {
    public:
        // Construct/destruct
        static CTelephonyAudioRoutingManagerStub* NewL();
        ~CTelephonyAudioRoutingManagerStub();

    private:
        // Construct/destruct
        CTelephonyAudioRoutingManagerStub();
        void ConstructL();

    // From MTelephonyAudioRoutingPolicyObserver
    virtual void OutputChangeRequested(
         CTelephonyAudioRoutingManager&
         aTelephonyAudioRoutingManager,
         CTelephonyAudioRouting::TAudioOutput aOutput);

    private:
        CTelephonyAudioRoutingManager* iTelephonyAudioRoutingMan;
        RArray<CTelephonyAudioRouting::TAudioOutput> iAudioOutput;
    };


#endif /* TELEPHONYAUDIOROUTINGMANAGERSTUB_H */
