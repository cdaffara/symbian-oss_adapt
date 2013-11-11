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
* Description:  Hardware Resource Manager stub plugins Power State plugin 
*                implementation.
*
*/


#include <hwrmpowerstatesdkpskeys.h>
#include "PowerStatePlugin.h"


//CONSTANTS
#ifdef ENABLE_NOTIFICATIONS
    // Timer begins triggering notifications after this time period
    const TInt KDelay(5000000);      //5 seconds
    // Timer triggers notifications at this time interval
    const TInt KInterval(5000000);   //5 seconds
#endif


CPowerStatePlugin* CPowerStatePlugin::NewL()
    {
    CPowerStatePlugin* self = new(ELeave) CPowerStatePlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

CPowerStatePlugin::~CPowerStatePlugin()
    {
#ifdef ENABLE_NOTIFICATIONS
    delete iPeriodic;
#endif
    }

CPowerStatePlugin::CPowerStatePlugin() : iKey( KHWRMBatteryLevel ),
                                         iKeyValue( EBatteryLevelUnknown )
    {
    }

void CPowerStatePlugin::ConstructL()
    {
    // Only send periodic notifications if flag is defined
#ifdef ENABLE_NOTIFICATIONS
    RDebug::Print( _L("HWRM PowerStatePlugin: Notifications enabled.") );
    // Use CPeriodic to generate state changes
    iPeriodic = CPeriodic::NewL( CActive::EPriorityStandard );
    
    // Start the timer after KDelay and for every KInterval
    iPeriodic->Start( KDelay, KInterval,
                      TCallBack( CPowerStatePlugin::Callback, this ) );
#else
    RDebug::Print( _L("HWRM PowerStatePlugin: Notifications disabled.") );
#endif
    }

TInt CPowerStatePlugin::Callback( TAny* aPtr )
    {
    CPowerStatePlugin* ptr = (CPowerStatePlugin*) aPtr;
    // use callback to call the notification method in the plugin callback
    RDebug::Print( _L("HWRM PowerStatePlugin: Key=%d, Value=%d"), ptr->iKey, ptr->iKeyValue );
    TRAPD( err, ptr->iNotificationCallback->NotifyStateChange(
                                                ptr->iKey, ptr->iKeyValue ) );
    ptr->IncrementKeyAndValue();
    
    return err;
    }

void CPowerStatePlugin::IncrementKeyAndValue()
    {
    switch( iKey )
        {
        case KHWRMBatteryLevel:
            {
            // key has values -1 to 7
            if( iKeyValue == EBatteryLevelLevel7 )
                {
                // last value so set next key
                iKey = KHWRMBatteryStatus;
                iKeyValue = EBatteryStatusUnknown;
                }
            else
                {
                // Otherwise just increment it
                iKeyValue++;
                }
            break;
            }
        case KHWRMBatteryStatus:
            {
            // key has values -1 to 2
            if( iKeyValue == EBatteryStatusEmpty )
                {
                // last value so set next key
                iKey = KHWRMChargingStatus;
                iKeyValue = EChargingStatusError;
                }
            else
                {
                // Otherwise just increment it
                iKeyValue++;
                }
            break;
            }
        case KHWRMChargingStatus:
            {
            // key has values -1 to 5
            if( iKeyValue == EChargingStatusChargingContinued )
                {
                // last value so set next key
                iKey = KHWRMBatteryLevel;
                iKeyValue = EBatteryLevelUnknown;
                }
            else
                {
                // Otherwise just increment it
                iKeyValue++;
                }
            break;
            }
        default:
            break;
        }
    }
