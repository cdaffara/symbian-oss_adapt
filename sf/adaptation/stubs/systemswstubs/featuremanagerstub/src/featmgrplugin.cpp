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
* Description:  Feature Manager stub plugin 
*                implementation.
*
*/


#include "featmgrplugin.h"
#include <featureinfoplugin.h>
#include "trace.h"     

CFeatMgrPlugin* CFeatMgrPlugin::NewL()
    {
    CFeatMgrPlugin* self = new(ELeave) CFeatMgrPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

CFeatMgrPlugin::~CFeatMgrPlugin()
    {
    if ( iTimer )
        {
        delete iTimer;
        iTimer = NULL;
        }
    }

CFeatMgrPlugin::CFeatMgrPlugin()
    {
    }

void CFeatMgrPlugin::ConstructL()
    {
    }

    
void CFeatMgrPlugin::ProcessCommandL( const FeatureInfoCommand::TFeatureInfoCmd aCommandId,
                                      const TUint8 aTransId,
                                      TDesC8& /*aData*/ )
    {
    COMPONENT_TRACE((_L("FeatMgrPlugin: Processing command: 0x%x, TransId: 0x%x"), 
                         aCommandId, aTransId));
           
    switch ( aCommandId )
        {
        case FeatureInfoCommand::ELoadFeatureInfoCmdId:
            {
            COMPONENT_TRACE(_L("FeatMgrPlugin: Processed ELoadFeatureInfoCmdId"));
            // Load feature info
            }
            break;
        default :
            {
            COMPONENT_TRACE((_L("FeatMgrPlugin: Unknown Command: 0x%x"), aCommandId));
            }
            break;
        }
        
    TInt retval(KErrNone);
    TInt timeout( 1000 ); // microseconds

    // create new timer


    iTimer = CPluginTimer::NewL(timeout, *iResponseCallback, aCommandId, aTransId, retval,this);
    }

void CFeatMgrPlugin::GenericTimerFiredL(
    MFeatureInfoPluginCallback& aService,
    FeatureInfoCommand::TFeatureInfoCmd aCommandId,
    TUint8 aTransId,
    TInt /*aRetVal*/)
    {
    COMPONENT_TRACE((_L("FeatMgr Plugin: GenericTimerFiredL (0x%x, 0x%x)"), 
                         aCommandId, aTransId));

    //__ASSERT_ALWAYS(!(&aService), User::Invariant() );
    
    RArray<FeatureInfoCommand::TFeature>* list = 
    new(ELeave) RArray<FeatureInfoCommand::TFeature>(1);
    
    FeatureInfoCommand::TFeatureInfo featureList;
    TInt err( KErrNone );
    
    //
    // TEST CASES
    //
    
    /***/
    // For testing, case 1.
    // Returns empty list and KErrNone.

    featureList.iErrorCode = KErrNone;
    featureList.iList = *list;
    
    FeatureInfoCommand::TFeatureInfoRespPckg resPackage(featureList);
    
    TRAP(err, aService.ProcessResponseL(aCommandId, aTransId, resPackage));    
    /***/
    
    /***
    // For testing, case 2.
    // Returns one feature and KErrNone.
    
    FeatureInfoCommand::TFeature feature1; 
    feature1.iFeatureID = KFeatureIdBt;
    feature1.iValue = ETrue;
    
    list->Append(feature1);
    
    featureList.iErrorCode = KErrNone;
    featureList.iList = *list;
        
    FeatureInfoCommand::TFeatureInfoRespPckg resPackage(featureList);
    
    TRAP(err, aService.ProcessResponseL(aCommandId, aTransId, resPackage));   
    ***/
        
    /***
    // For testing, case 3.
    // Returns three features and KErrNone
    
    FeatureInfoCommand::TFeature feature1; 
    FeatureInfoCommand::TFeature feature2; 
    FeatureInfoCommand::TFeature feature3; 
    
    feature1.iFeatureID = KFeatureIdBt;
    feature1.iValue = EFalse;
    
    feature1.iFeatureID = KFeatureIdIrda;
    feature1.iValue = ETrue;
    
    feature1.iFeatureID = KFeatureIdProtocolWlan;
    feature1.iValue = EFalse;
    
    list->Append(feature1);
    list->Append(feature2);
    list->Append(feature3);
    
    featureList.iErrorCode = KErrNone;
    featureList.iList = *list;
    
    FeatureInfoCommand::TFeatureInfoRespPckg resPackage(featureList);
    
    TRAP(err, aService.ProcessResponseL(aCommandId, aTransId, resPackage));    
    ***/
    
    /***
    // For testing, case 4.
    // Returns empty list and KErrArgument. 
        
    featureList.iErrorCode = KErrArgument;
    featureList.iList = *list;
    
    FeatureInfoCommand::TFeatureInfoRespPckg resPackage(featureList);
    
    TRAP(err, aService.ProcessResponseL(aCommandId, aTransId, resPackage));    
    ***/
            
    /***
    // For testing, case 5.
    // ProcessResponseL never called.
    ***/
    
    list->Reset();
    delete list;
    
    // TEST CASES END
 
    if ( err != KErrNone )
        {
        COMPONENT_TRACE((_L("FeatMgr Plugin: Error in ProcessResponseL: %d"), err));
        }
    
    if ( iTimer && !iTimer->IsActive() )
        {
        delete iTimer;
        iTimer = NULL;            
        COMPONENT_TRACE((_L("FeatMgr Plugin: GenericTimerFiredL - Removed obsolete timer")));
        }
    }

