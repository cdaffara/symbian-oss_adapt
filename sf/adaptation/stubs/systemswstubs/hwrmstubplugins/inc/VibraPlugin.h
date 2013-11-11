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
* Description:  Hardware Resource Manager stub plugins Vibra plugin header
*
*/


#ifndef __VIBRAPLUGIN_H_
#define __VIBRAPLUGIN_H_

#include <hwrmpluginservice.h>
#include "PluginTimer.h"

#ifdef PUBLISH_STATE_INFO
#include <e32property.h>
#include <HWRMVibra.h>
const TUint32 KHWRMTestVibraCommand             = 0x11110001;
const TUint32 KHWRMTestVibraDataPckg            = 0x11110002;
#endif // PUBLISH_STATE_INFO

class CPluginTimer;

class CVibraPlugin : public CHWRMPluginService,
                     public MPluginTimerCallback
    {
    public:

        static CVibraPlugin* NewL();
        ~CVibraPlugin();

        /**
        * Method to invoke a particular command in the plugin
        * @param aCommandId Command ID
        * @param aTransId   Transaction ID
        * @param aData      Data associated with command
        */
        virtual void ProcessCommandL( const TInt aCommandId,
                                      const TUint8 aTransId,
                                      TDesC8& aData );

        /**
        * Method to cancel a particular command
        * @param aTransId   Transaction ID
        * @param aCommandId Command ID to optionally double check with the 
        *                   transaction ID
        */
        virtual void CancelCommandL( const TUint8 aTransId, 
                                     const TInt aCommandId);

        // From MPluginTimerCallback
        void GenericTimerFired(MHWRMPluginCallback* aService,
                         const TInt aCommandId,
                         const TUint8 aTransId,
                         TInt aRetVal);

    private: // Construction

        CVibraPlugin();
        void ConstructL();
        
        RPointerArray<CPluginTimer> iTimers;
        
#ifdef PUBLISH_STATE_INFO
        RProperty iCmdProperty;   // for publishing command ID
        RProperty iDataProperty;  // for publishing data
#endif // PUBLISH_STATE_INFO        

    };

#endif // __VIBRAPLUGIN_H_