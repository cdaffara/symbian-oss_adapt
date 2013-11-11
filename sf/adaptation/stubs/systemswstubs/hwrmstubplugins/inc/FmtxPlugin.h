/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Hardware Resource Manager stub plugins fmtx plugin 
*                implementation.
*
*/


#ifndef __FMTXPLUGIN_H_
#define __FMTXPLUGIN_H_

#include <hwrmpluginservice.h>
#include <hwrmfmtxcommands.h>
#include "PluginTimer.h"

#ifdef PUBLISH_STATE_INFO
#include <e32property.h>
const TUint32 KHWRMTestFmtxCommand             = 0x44440001;
const TUint32 KHWRMTestFmtxDataPckg            = 0x44440002;
#endif // PUBLISH_STATE_INFO

class CPluginTimer;

class CFmtxPlugin : public CHWRMPluginService,
                    public MPluginTimerCallback
    {
    public:

        static CFmtxPlugin* NewL();
        ~CFmtxPlugin();

        /**
        * Method to invoke a particular command in the plugin
        * @param aCommandId Command ID
        * @param aTransId   Transaction ID
        * @param aData      Data associated with command
        */
        virtual void ProcessCommandL(const TInt aCommandId,
                                     const TUint8 aTransId,
                                     TDesC8& aData);

        /**
        * Method to cancel a particular command
        * @param aTransId   Transaction ID
        * @param aCommandId Command ID to optionally double check with the
        *                   transaction ID
        */
        virtual void CancelCommandL(const TUint8 aTransId,
                                    const TInt aCommandId);

        // From MPluginTimerCallback
        void GenericTimerFired(MHWRMPluginCallback* aService,
                               const TInt aCommandId,
                               const TUint8 aTransId,
                               TInt aRetVal);

    private: // Construction

        CFmtxPlugin();
        void ConstructL();

		TInt CheckFrequencyWithinRange(TDesC8& aData);

        RPointerArray<CPluginTimer> iTimers;

#ifdef PUBLISH_STATE_INFO
        RProperty iCmdProperty;   // for publishing command ID
        RProperty iDataProperty;  // for publishing data
#endif // PUBLISH_STATE_INFO

		HWRMFmTxCommand::TFmTxCmd iLastCommand; // the on-going command, if any

		HWRMFmTxCommand::TFmTxHwState iHwState; // the current hardware state

		TInt iFrequency; // the requested frequency
		
		TInt iStatusIndTransId; // the trans ID of last status indication request

		// RSSI scanning
		TInt iChannelsRequested;  // the number of clear frequencies requested
		TUint32 iClearFrequency;     // for returning scan responses
		
        TBuf<1024> iBuf;
    };

#endif /* __FMTXPLUGIN_H_ */
