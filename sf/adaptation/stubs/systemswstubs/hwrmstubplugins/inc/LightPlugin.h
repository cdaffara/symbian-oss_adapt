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
* Description:  Hardware Resource Manager stub plugins Light plugin header
*
*/


#ifndef __LIGHTPLUGIN_H_
#define __LIGHTPLUGIN_H_

#include <hwrmpluginservice.h>
#include "PluginTimer.h"

#ifdef PUBLISH_STATE_INFO
#include <e32property.h>
#include <HWRMLight.h>

// Each target has own set of these two keys
// Bytes 2-4 contain target  >-----------------------vvv
const TUint32 KHWRMTestLightCommand             = 0x00000001;
const TUint32 KHWRMTestLightDataPckg            = 0x00000002;

const TUint32 KHWRMTestLightIntensityValue      = 0x10000007;
const TUint32 KHWRMTestLightSensitivityValue    = 0x10000008;

const TInt KHWRMLightMaxTargets(4);   
#endif // PUBLISH_STATE_INFO

class CPluginTimer;

class CLightPlugin : public CHWRMPluginService,
                     public MPluginTimerCallback
    {
    public:

        static CLightPlugin* NewL();
        virtual ~CLightPlugin();

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

        CLightPlugin();
        void ConstructL();

        RPointerArray<CPluginTimer> iTimers;
        
#ifdef PUBLISH_STATE_INFO
        void PublishStateInfo(TInt aTargetMask,
                              const TInt aCommandId,
                              TDesC8& aData );
        RProperty iCmdProperty[KHWRMLightMaxTargets];   // for publishing command ID
        RProperty iDataProperty[KHWRMLightMaxTargets];  // for publishing data
        RProperty iIntensityProperty;
        RProperty iSensitivityProperty;
        TInt      iDefaultIntensity;
#endif // PUBLISH_STATE_INFO        

        
    };

#endif // __LIGHTPLUGIN_H_