/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Hardware Resource Manager stub plugins PowerState plugin header
*
*/


#ifndef __POWERSTATEPLUGIN_H_
#define __POWERSTATEPLUGIN_H_

#include <hwrmpowerstateplugin.h>


class CPowerStatePlugin : public CHWRMPowerStatePlugin
    {
    public:

        static CPowerStatePlugin* NewL();
        ~CPowerStatePlugin();
        
        // Callback used by CPeriodic
        static TInt Callback(TAny* aPtr);

    private: // Construction

        CPowerStatePlugin();
        void ConstructL();
        
        // Helper to increment the key and/or the value
        // to cover all cases.
        void IncrementKeyAndValue();
        
        // Timer used to fire notifications
        CPeriodic* iPeriodic;
        
        // key name for next notification
        TUint32 iKey;
        // Value for key
        TInt iKeyValue;
    };

#endif // __POWERSTATEPLUGIN_H_