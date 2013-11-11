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
* Description:  Hardware Resource Manager stub plugins vibra plugin 
*                implementation.
*
*/


#include "VibraPlugin.h"
#include "PluginTimer.h"
#include <hwrmvibracommands.h>
#include "Trace.h" 


#ifdef PUBLISH_STATE_INFO
const TUid KPSUidHWResourceNotification = {0x101F7A01}; // HWRM private PS Uid
#endif // PUBLISH_STATE_INFO            


CVibraPlugin* CVibraPlugin::NewL()
    {
    CVibraPlugin* self = new(ELeave) CVibraPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

CVibraPlugin::~CVibraPlugin()
    {
    iTimers.ResetAndDestroy();
#ifdef PUBLISH_STATE_INFO
    iCmdProperty.Close();
    iDataProperty.Close();
#endif // PUBLISH_STATE_INFO        
    }

CVibraPlugin::CVibraPlugin()
    {
    }

void CVibraPlugin::ConstructL()
    {
#ifdef PUBLISH_STATE_INFO
    RProperty::Define(KPSUidHWResourceNotification, KHWRMTestVibraCommand, RProperty::EInt);
    RProperty::Define(KPSUidHWResourceNotification, KHWRMTestVibraDataPckg, RProperty::EByteArray, 512);
    iCmdProperty.Attach(KPSUidHWResourceNotification, KHWRMTestVibraCommand);
    iDataProperty.Attach(KPSUidHWResourceNotification, KHWRMTestVibraDataPckg);
#endif // PUBLISH_STATE_INFO        
    }

    
void CVibraPlugin::ProcessCommandL( const TInt aCommandId,
                                      const TUint8 aTransId,
#ifdef PUBLISH_STATE_INFO
                                      TDesC8& aData )
#else
                                      TDesC8& /*aData*/ )
#endif // PUBLISH_STATE_INFO
    {
    COMPONENT_TRACE((_L("HWRM VibraPlugin: Processing command: 0x%x, TransId: 0x%x"), aCommandId, aTransId));
    
    switch ( aCommandId )
        {
        case HWRMVibraCommand::EStartVibraCmdId:
            {
            COMPONENT_TRACE(_L("HWRM VibraPlugin: Processed EStartVibraCmdId"));
            }
            break;
        case HWRMVibraCommand::EStartVibraWithDefaultSettingsCmdId:
            {
            COMPONENT_TRACE(_L("HWRM VibraPlugin: Processed EStartVibraWithDefaultSettingsCmdId"));
            }
            break;
        case HWRMVibraCommand::EPulseVibraCmdId:
            {
            COMPONENT_TRACE(_L("HWRM VibraPlugin: Processed EPulseVibraCmdId"));
            }
            break;
        case HWRMVibraCommand::EStopVibraCmdId:
            {
            COMPONENT_TRACE(_L("HWRM VibraPlugin: Processed EStopVibraCmdId"));
            }
            break;
            
        default :
            {
            COMPONENT_TRACE((_L("HWRM VibraPlugin: Unknown Command: 0x%x"), aCommandId));
            }
            break;
        }
    
    TInt retval(KErrNone);
    TInt timeout(500); // microseconds
    
#ifdef PUBLISH_STATE_INFO
    // publish
    iCmdProperty.Set(aCommandId);
    iDataProperty.Set(aData);
#endif // PUBLISH_STATE_INFO

    // create new timer
    CPluginTimer* timer = CPluginTimer::NewL(timeout, iResponseCallback, aCommandId, aTransId, retval, this);
    CleanupStack::PushL(timer); 
    iTimers.AppendL(timer); 
    CleanupStack::Pop(timer);
    }

void CVibraPlugin::CancelCommandL( const TUint8 aTransId, 
#if defined(_DEBUG) && defined(COMPONENT_TRACE_FLAG)
                                     const TInt aCommandId)
#else
                                     const TInt /*aCommandId*/)
#endif                                     
    {
    COMPONENT_TRACE((_L("HWRM VibraPlugin: Cancelling command: 0x%x, TransId: 0x%x"), aCommandId, aTransId));
    COMPONENT_TRACE((_L("HWRM VibraPlugin: Cancelling command - iTimers.Count(): %d "), iTimers.Count()));
    
    for( TInt i = 0; i < iTimers.Count(); i++ )
        {
        if ( iTimers[i]->TransId() == aTransId )
            {
            delete iTimers[i];
            iTimers.Remove(i);
            COMPONENT_TRACE((_L("HWRM VibraPlugin: Cancelling command - Removed command: 0x%x, TransId: 0x%x"), aCommandId, aTransId));
            break;
            }
        }
    }

void CVibraPlugin::GenericTimerFired(MHWRMPluginCallback* aService,
                  TInt aCommandId,
                 const TUint8 aTransId,
                 TInt aRetVal)
    {
    COMPONENT_TRACE((_L("HWRM VibraPlugin: GenericTimerFired (0x%x, 0x%x, %d)"), aCommandId, aTransId, aRetVal));

    __ASSERT_ALWAYS(aService != NULL, User::Invariant() );

    HWRMVibraCommand::TErrorCodeResponsePackage retvalPackage(aRetVal);

    TRAPD(err, aService->ProcessResponseL(aCommandId, aTransId, retvalPackage));    
    
    if ( err != KErrNone )
        {
        COMPONENT_TRACE((_L("HWRM VibraPlugin: Error in ProcessResponseL: %d"), err));
        }

    // delete obsolete timers
    for( TInt i = (iTimers.Count()-1); i > -1 ; i-- )
        {
        if ( !iTimers[i]->IsActive() )
            {
            delete iTimers[i];
            iTimers.Remove(i);    
            COMPONENT_TRACE((_L("HWRM VibraPlugin: GenericTimerFired - Removed obsolete timer")));
            }
        }
    
    }
