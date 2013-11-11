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
* Description:  Feature Manager stub plugin timer definition
*
*/



#ifndef __PLUGINTIMER_H_
#define __PLUGINTIMER_H_

#include <featureinfoplugin.h>

/**
*  An interface for handling timer expiration events.
*/
class MPluginTimerCallback
    {
    public:
        /** 
        * Called when the timer fires
        */
        virtual void GenericTimerFiredL( MFeatureInfoPluginCallback& aService,
                                       FeatureInfoCommand::TFeatureInfoCmd aCommandId,
                                       TUint8 aTransId,
                                       TInt aRetVal ) = 0;
    };


class CPluginTimer : public CTimer
    {
    public:
        CPluginTimer( TInt aPriority, 
                      MFeatureInfoPluginCallback& aService,
                      FeatureInfoCommand::TFeatureInfoCmd aCommandId,
                      TUint8 aTransId,
                      TInt aRetVal,
                      MPluginTimerCallback* aCallback );
        
        ~CPluginTimer();

        CPluginTimer(TInt aPriority);

        static CPluginTimer* NewL( const TTimeIntervalMicroSeconds32& anInterval, 
                                   MFeatureInfoPluginCallback& aService,
                                   FeatureInfoCommand::TFeatureInfoCmd aCommandId,
                                   TUint8 aTransId,
                                   TInt aRetVal,
                                   MPluginTimerCallback* aCallback );
                         
        void ConstructL();
        
        void RunL();
        
        inline TUint8 TransId() const
            {
            return  iTransId;
            };
    
    private:
    
       MFeatureInfoPluginCallback& iService;  // Not owned
        
       FeatureInfoCommand::TFeatureInfoCmd iCommandId;
       TUint8 iTransId;
       TInt iRetVal;
       
       MPluginTimerCallback* iCallback;  // Not owned
    
    };


#endif // __PLUGINTIMER_H_