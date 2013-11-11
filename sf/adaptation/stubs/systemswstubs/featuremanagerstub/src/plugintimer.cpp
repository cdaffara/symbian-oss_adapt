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
* Description:  Feature Manager stub plugin timer implementation
*
*/


#include "plugintimer.h"
#include <featureinfoplugin.h>
#include "trace.h"

CPluginTimer* CPluginTimer::NewL( const TTimeIntervalMicroSeconds32& anInterval, 
                                  MFeatureInfoPluginCallback& aService,
                                  FeatureInfoCommand::TFeatureInfoCmd aCommandId,
                                  TUint8 aTransId, 
                                  TInt aRetVal,
                                  MPluginTimerCallback* aCallback )
    {
    CPluginTimer* self = new( ELeave ) CPluginTimer(0, aService, aCommandId, 
                                                    aTransId, aRetVal, aCallback);
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    CActiveScheduler::Add(self);

    COMPONENT_TRACE((_L("FeatMgr Plugin - CPluginTimer::NewL - Setting timer: %d"), anInterval));

    self->After(anInterval);

    return self;
    }

CPluginTimer::~CPluginTimer()    
    {
    // PCLint demands
    iCallback = NULL; 
    }

CPluginTimer::CPluginTimer( TInt aPriority, 
                            MFeatureInfoPluginCallback& aService,
                            FeatureInfoCommand::TFeatureInfoCmd aCommandId,
                            TUint8 aTransId,
                            TInt aRetVal,
                            MPluginTimerCallback* aCallback )
    : CTimer(aPriority),
    iService(aService),
    iCommandId(aCommandId),
    iTransId(aTransId),
    iRetVal(aRetVal),
    iCallback(aCallback)
    {
    }

void CPluginTimer::ConstructL()
    {
    CTimer::ConstructL();
    }

    
void CPluginTimer::RunL(  )
    {
    iCallback->GenericTimerFiredL(iService, iCommandId, iTransId, iRetVal );

    }

