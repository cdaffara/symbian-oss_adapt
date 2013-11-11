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
* Description:  Feature Manger stub plugin header
*
*/


#ifndef __FEATMGRPLUGIN_H_
#define __FEATMGRPLUGIN_H_

#include <featureinfoplugin.h>
#include "plugintimer.h"


class CFeatMgrPlugin : public CFeatureInfoPlugin,
                       public MPluginTimerCallback
    {
    public:

        static CFeatMgrPlugin* NewL();
        virtual ~CFeatMgrPlugin();

        /**
        * Method to invoke a particular command in the plugin
        * @param aCommandId Command ID
        * @param aTransId   Transaction ID
        * @param aData      Data associated with command
        */
        virtual void ProcessCommandL( const FeatureInfoCommand::TFeatureInfoCmd aCommandId,
                                      const TUint8 aTransId,
                                      TDesC8& aData );
        
        // From MPluginTimerCallback
        void GenericTimerFiredL( MFeatureInfoPluginCallback& aService,
                                const FeatureInfoCommand::TFeatureInfoCmd aCommandId,
                                const TUint8 aTransId,
                                TInt aRetVal );
    private: // Construction

        CFeatMgrPlugin();
        void ConstructL();
        
       // RPointerArray<CPluginTimer> iTimers;
       CPluginTimer* iTimer;
 
    };

#endif // __FEATMGRPLUGIN_H_