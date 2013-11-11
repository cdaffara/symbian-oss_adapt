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
* Description:  Hardware Resource Manager stub plugins light plugin 
*                implementation.
*
*/


#include "LightPlugin.h"
#include <hwrmlightcommands.h>
#include "PluginTimer.h"
#include "Trace.h"

#ifdef PUBLISH_STATE_INFO
const TUid KPSUidHWResourceNotification = {0x101F7A01}; // HWRM private PS Uid
#endif // PUBLISH_STATE_INFO        

CLightPlugin* CLightPlugin::NewL()
    {
    CLightPlugin* self = new(ELeave) CLightPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

CLightPlugin::~CLightPlugin()
    {
    iTimers.ResetAndDestroy();
#ifdef PUBLISH_STATE_INFO
    for ( TInt i=0; i< KHWRMLightMaxTargets; i++ )
        {
        iCmdProperty[i].Close();
        iDataProperty[i].Close();
        }
    iIntensityProperty.Close();
    iSensitivityProperty.Close();
#endif // PUBLISH_STATE_INFO        
    }

CLightPlugin::CLightPlugin()
    {
    }

void CLightPlugin::ConstructL()
    {
#ifdef PUBLISH_STATE_INFO
    TInt currentTarget(0x00010000);
    for ( TInt i=0; i< KHWRMLightMaxTargets; i++ )
        {
        RProperty::Define(KPSUidHWResourceNotification, (KHWRMTestLightCommand | currentTarget), RProperty::EInt);
        RProperty::Define(KPSUidHWResourceNotification, (KHWRMTestLightDataPckg | currentTarget), RProperty::EByteArray, 512);
        iCmdProperty[i].Attach(KPSUidHWResourceNotification, (KHWRMTestLightCommand | currentTarget));
        iDataProperty[i].Attach(KPSUidHWResourceNotification, (KHWRMTestLightDataPckg | currentTarget));
        currentTarget <<= 1;
        }
        
    RProperty::Define(KPSUidHWResourceNotification, KHWRMTestLightIntensityValue, RProperty::EInt);            
    RProperty::Define(KPSUidHWResourceNotification, KHWRMTestLightSensitivityValue, RProperty::EInt); 
    iIntensityProperty.Attach(KPSUidHWResourceNotification, KHWRMTestLightIntensityValue);
    iSensitivityProperty.Attach(KPSUidHWResourceNotification, KHWRMTestLightSensitivityValue);           
#endif // PUBLISH_STATE_INFO        
    }

    
void CLightPlugin::ProcessCommandL( const TInt aCommandId,
                                      const TUint8 aTransId,
#ifdef PUBLISH_STATE_INFO
                                      TDesC8& aData )
#else
                                      TDesC8& /*aData*/ )
#endif // PUBLISH_STATE_INFO
    {
    COMPONENT_TRACE((_L("HWRM LightPlugin: Processing command: 0x%x, TransId: 0x%x"), aCommandId, aTransId));
    
#ifdef PUBLISH_STATE_INFO
    TInt targetMask(0);
#endif // PUBLISH_STATE_INFO            
    switch ( aCommandId )
        {
        case HWRMLightCommand::ELightsOnCmdId:
            {
            COMPONENT_TRACE(_L("HWRM LightPlugin: Processed ELightsOnCmdId"));
#ifdef PUBLISH_STATE_INFO
            HWRMLightCommand::TLightsOnCmdPackage pckg;
            pckg.Copy(aData);
            HWRMLightCommand::TLightsOnData data = pckg();
            if ( data.iIntensity == KHWRMDefaultIntensity )
                {
                data.iIntensity = iDefaultIntensity;
                }
            
            targetMask = data.iTarget << 16;
            HWRMLightCommand::TLightsOnCmdPackage pckg2(data);
            PublishStateInfo(targetMask, aCommandId, pckg2);
#endif // PUBLISH_STATE_INFO
            }
            break;
        case HWRMLightCommand::ELightsOnSensorCmdId:
            {
            COMPONENT_TRACE(_L("HWRM LightPlugin: Processed ELightsOnSensorCmdId"));
#ifdef PUBLISH_STATE_INFO
            HWRMLightCommand::TLightsOnSensorCmdPackage pckg;
            pckg.Copy(aData);
            HWRMLightCommand::TLightsOnSensorData data = pckg();
            
            targetMask = data.iTarget << 16;
            PublishStateInfo(targetMask, aCommandId, pckg);
#endif // PUBLISH_STATE_INFO
            }
            break;
        case HWRMLightCommand::ELightsBlinkCmdId:
            {
            COMPONENT_TRACE(_L("HWRM LightPlugin: Processed ELightsBlinkCmdId"));
#ifdef PUBLISH_STATE_INFO
            HWRMLightCommand::TLightsBlinkCmdPackage pckg;
            pckg.Copy(aData);
            HWRMLightCommand::TLightsBlinkData data = pckg();
            if ( data.iIntensity == KHWRMDefaultIntensity )
                {
                data.iIntensity = iDefaultIntensity;
                }
            
            targetMask = data.iTarget << 16;
            HWRMLightCommand::TLightsBlinkCmdPackage pckg2(data);
            PublishStateInfo(targetMask, aCommandId, pckg2);
#endif // PUBLISH_STATE_INFO
            }
            break;
        case HWRMLightCommand::ELightsOffCmdId:
            {
            COMPONENT_TRACE(_L("HWRM LightPlugin: Processed ELightsOffCmdId"));
#ifdef PUBLISH_STATE_INFO
            HWRMLightCommand::TLightsOffCmdPackage pckg;
            pckg.Copy(aData);
            HWRMLightCommand::TLightsOffData data = pckg();

            targetMask = data.iTarget << 16;
            PublishStateInfo(targetMask, aCommandId, pckg);
#endif // PUBLISH_STATE_INFO
            }
            break;
        case HWRMLightCommand::ESetLightsIntensityCmdId:
            {
            COMPONENT_TRACE(_L("HWRM LightPlugin: Processed ESetLightsIntensityCmdId"));
#ifdef PUBLISH_STATE_INFO
            HWRMLightCommand::TLightsIntensityCmdPackage pckg;
            pckg.Copy(aData);
            HWRMLightCommand::TLightsIntensityData data = pckg();
            
            iDefaultIntensity = data.iIntensity;
            COMPONENT_TRACE( (_L("HWRM LightPlugin: Intensity: %d"), iDefaultIntensity));

            // Currently only general intensity is supported, so ignore target parameter,
            // it is always all supported targets
            iIntensityProperty.Set(iDefaultIntensity);
            
#endif // PUBLISH_STATE_INFO
            }
            break;
        case HWRMLightCommand::ESetLightsSensorSensitivityCmdId:
            {
            COMPONENT_TRACE(_L("HWRM LightPlugin: Processed ESetLightsSensorSensitivityCmdId"));
#ifdef PUBLISH_STATE_INFO
            HWRMLightCommand::TLightsSensorSensitivityCmdPackage pckg;
            pckg.Copy(aData);
            TInt data = pckg();
            
            iSensitivityProperty.Set(data);
#endif // PUBLISH_STATE_INFO
            }
            break;
        default :
            {
            COMPONENT_TRACE((_L("HWRM LightPlugin: Unknown Command: 0x%x"), aCommandId));
            }
            break;
        }
    TInt retval(KErrNone);
    TInt timeout(500); // microseconds

    // create new timer
    CPluginTimer* timer = CPluginTimer::NewL(timeout, iResponseCallback, aCommandId, aTransId, retval, this);
    CleanupStack::PushL(timer); 
    iTimers.AppendL(timer); 
    CleanupStack::Pop(timer); 
    }

#ifdef PUBLISH_STATE_INFO
void CLightPlugin::PublishStateInfo(TInt aTargetMask,
                                    const TInt aCommandId,
                                    TDesC8& aData ) 
    {    
    // publish
    TInt currentTarget(0x10000);
    for ( TInt i=0; i< KHWRMLightMaxTargets; i++ )
        {
        if ( aTargetMask & currentTarget )
            {
            iCmdProperty[i].Set(aCommandId);
            iDataProperty[i].Set(aData);
            }
        currentTarget <<= 1;
        }
    }
#endif // PUBLISH_STATE_INFO

void CLightPlugin::CancelCommandL( const TUint8 aTransId, 
#if defined(_DEBUG) && defined(COMPONENT_TRACE_FLAG)
                                     const TInt aCommandId)
#else
                                     const TInt /*aCommandId*/)
#endif                                     
    {
    COMPONENT_TRACE((_L("HWRM LightPlugin: Cancelling command: 0x%x, TransId: 0x%x"), aCommandId, aTransId));
    COMPONENT_TRACE((_L("HWRM LightPlugin: Cancelling command - iTimers.Count(): %d "), iTimers.Count()));

    for( TInt i = 0; i < iTimers.Count(); i++ )
        {
        if ( iTimers[i]->TransId() == aTransId )
            {
            delete iTimers[i];
            iTimers.Remove(i);
            COMPONENT_TRACE((_L("HWRM LightPlugin: Cancelling command - Removed command: 0x%x, TransId: 0x%x"), aCommandId, aTransId));
            break;
            }
        }
    }

void CLightPlugin::GenericTimerFired(MHWRMPluginCallback* aService,
                  TInt aCommandId,
                  TUint8 aTransId,
                 TInt aRetVal)
    {
    COMPONENT_TRACE((_L("HWRM LightPlugin: GenericTimerFired (0x%x, 0x%x, %d)"), aCommandId, aTransId, aRetVal));

    __ASSERT_ALWAYS(aService != NULL, User::Invariant() );

    HWRMLightCommand::TErrorCodeResponsePackage retvalPackage(aRetVal);
    
    TRAPD(err, aService->ProcessResponseL(aCommandId, aTransId, retvalPackage));    
    
    if ( err != KErrNone )
        {
        COMPONENT_TRACE((_L("HWRM LightPlugin: Error in ProcessResponseL: %d"), err));
        }
    
    // delete obsolete timers
    for( TInt i = (iTimers.Count()-1); i > -1 ; i-- )
        {
        if ( !iTimers[i]->IsActive() )
            {
            delete iTimers[i];
            iTimers.Remove(i);            
            COMPONENT_TRACE((_L("HWRM LightPlugin: GenericTimerFired - Removed obsolete timer")));
            }
        }
    }

