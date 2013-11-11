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



#include <hwrmfmtxcommands.h>
#include "FmtxPlugin.h"
#include "PluginTimer.h"
#include "Trace.h"

const TUint32 KFmTxStubPluginFreqMax  = 107900; // KHz
const TUint32 KFmTxStubPluginFreqMin  = 88100; // KHz
const TUint32 KFmTxStubPluginStepSize = 50; // KHz
const TInt KMaxLengthMarker = 63;

#ifdef PUBLISH_STATE_INFO
const TUid KPSUidHWResourceNotification = {0x101F7A01}; // HWRM private PS Uid
#endif /* PUBLISH_STATE_INFO */


CFmtxPlugin* CFmtxPlugin::NewL()
    {
    COMPONENT_TRACE(( _L("CFmtxPlugin::NewL()") ));
    
    CFmtxPlugin* self = new(ELeave) CFmtxPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }


CFmtxPlugin::~CFmtxPlugin()
    {
    COMPONENT_TRACE(( _L("CFmtxPlugin::~CFmtxPlugin()") ));
    
    iTimers.ResetAndDestroy();
#ifdef PUBLISH_STATE_INFO
    iCmdProperty.Close();
    iDataProperty.Close();
#endif /* PUBLISH_STATE_INFO */
    }


CFmtxPlugin::CFmtxPlugin() : iLastCommand(HWRMFmTxCommand::ENoCommandId),
                             iHwState(HWRMFmTxCommand::EFmTxHwStateOff),
                             iClearFrequency(KFmTxStubPluginFreqMin)
    {
    COMPONENT_TRACE(( _L("CFmtxPlugin::CFmtxPlugin()") ));
    }


void CFmtxPlugin::ConstructL()
    {
    COMPONENT_TRACE(( _L("CFmtxPlugin::ConstructL") ));

#ifdef PUBLISH_STATE_INFO
    RProperty::Define(KPSUidHWResourceNotification, KHWRMTestFmtxCommand, RProperty::EInt);
    RProperty::Define(KPSUidHWResourceNotification, KHWRMTestFmtxDataPckg, RProperty::EByteArray, 512);
    iCmdProperty.Attach(KPSUidHWResourceNotification, KHWRMTestFmtxCommand);
    iDataProperty.Attach(KPSUidHWResourceNotification, KHWRMTestFmtxDataPckg);
#endif /* PUBLISH_STATE_INFO */
    }


TInt CFmtxPlugin::CheckFrequencyWithinRange(TDesC8& aData)
	{
	HWRMFmTxCommand::TSetFrequencyPackage pckg;
	pckg.Copy(aData);
	TInt frequency = pckg();
			
	if ( frequency < KFmTxStubPluginFreqMin ||
		 frequency > KFmTxStubPluginFreqMax )
		{
		COMPONENT_TRACE((_L("CFmtxPlugin::CheckFrequencyWithinRange, frequency %d is out of range"), frequency));
		return KErrArgument;
		}
	else
		{
		iFrequency = frequency;
		return KErrNone;
		}
	}


void CFmtxPlugin::ProcessCommandL(const TInt aCommandId,
                                  const TUint8 aTransId,
                                  TDesC8& aData)
    {
    COMPONENT_TRACE((_L("CFmtxPlugin::ProcessCommandL, command: 0x%x, TransId: 0x%x"), aCommandId, aTransId));

    TInt retVal(KErrNone);

    switch (aCommandId)
        {
        case HWRMFmTxCommand::ETxOnCmdId:
            {
            COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ETxOnCmdId"));
            
            // check TSetFrequencyPackage param
			if ( iHwState != HWRMFmTxCommand::EFmTxHwStateOff )
				{
				retVal = KErrInUse;
				}
			else
			    {
			    retVal = CheckFrequencyWithinRange(aData);
			    }
			}
            break;
            
        case HWRMFmTxCommand::ETxOffCmdId:
            {
            COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ETxOffCmdId"));
            
			if ( iHwState == HWRMFmTxCommand::EFmTxHwStateOff )
				{
				retVal = KErrNotReady;
				}
            }
            break;
            
        case HWRMFmTxCommand::ESetTxFrequencyCmdId:
            {
            COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ESetTxFrequencyCmdId"));

			if ( iHwState == HWRMFmTxCommand::EFmTxHwStateOn )
				{
            	// check TSetFrequencyPackage param
				retVal = CheckFrequencyWithinRange(aData);
				}
			else
				{
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: HW isn't ready, iHwState = %d"), iHwState));
				retVal = KErrNotReady;
				}			
            }
            break;

        case HWRMFmTxCommand::ETxScanRequestCmdId:
        	{
        	COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ETxScanRequestCmdId"));
        	
			if ( iHwState == HWRMFmTxCommand::EFmTxHwStateOn )
				{        	
        		// check TScanRequestPackage param        	
        		HWRMFmTxCommand::TScanRequestPackage pckg;
				pckg.Copy(aData);        	
				TUint channelsRequested = pckg();			
                if( channelsRequested > 10 )
                    {
                    // Should not come here because input verified in client end
                    retVal = KErrArgument;
                    }
				iChannelsRequested = ( channelsRequested <= 10 ) ? channelsRequested : 10;
				}
			else
				{
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: HW isn't ready, iHwState = %d"), iHwState));
				retVal = KErrNotReady;
				}
        	}
        	break;

        case HWRMFmTxCommand::EGetTxFrequencyRangeCmdId:
        	{
        	COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed EGetTxFrequencyRangeCmdId"));
        	
        	// No params to check
        	}
        	break;        

        case HWRMFmTxCommand::ESetTxRdsPsCmdId:
        	{
        	COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ESetTxRdsPsCmdId"));

			if ( iHwState == HWRMFmTxCommand::EFmTxHwStateOn )
				{
				// check TRdsPsPackage param
				}
			else
				{
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: HW isn't ready, iHwState = %d"), iHwState));
				retVal = KErrNotReady;
				}
        	}
        	break;        
        
        case HWRMFmTxCommand::ESetTxRdsPtyCmdId:
        	{
        	COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ESetTxRdsPtyCmdId"));
        	
			if ( iHwState == HWRMFmTxCommand::EFmTxHwStateOn )
				{
				// Check TRdsPtyPackage param
        		HWRMFmTxCommand::TRdsPtyPackage pckg;
				pckg.Copy(aData);        	
				TInt programType = pckg();			
                const TInt KRdsPtyFirst = 0;
                const TInt KRdsPtyLast = 31;

                if( programType < KRdsPtyFirst || programType > KRdsPtyLast )
                    {
                    // Should not come here because input verified in client end
                    retVal = KErrArgument;
                    }
				}
			else
				{
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: HW isn't ready, iHwState = %d"), iHwState));
				retVal = KErrNotReady;
				}        	
        	}
        	break;        	        
        
        case HWRMFmTxCommand::ESetTxRdsPtynCmdId:
        	{
        	COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ESetTxRdsPtynCmdId"));
        	
			if ( iHwState == HWRMFmTxCommand::EFmTxHwStateOn )
				{
        		// Check TRdsPtynPackage param
        		HWRMFmTxCommand::TRdsPtynPackage pckg;
				pckg.Copy(aData);        	
				HWRMFmTxCommand::TRdsPtyn prgTypeName = pckg();			
                const TInt KMaxRdsPtynLength = 8;
                if( prgTypeName.Length() > KMaxRdsPtynLength )
                    {
                    // Should not come here because input verified in client end
                    retVal = KErrArgument;
                    }
				}
			else
				{
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: HW isn't ready, iHwState = %d"), iHwState));
				retVal = KErrNotReady;
				}        	
        	}
        	break;
        
        case HWRMFmTxCommand::ESetTxRdsMsCmdId:
        	{
        	COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ESetTxRdsMsCmdId"));
        	
			if ( iHwState == HWRMFmTxCommand::EFmTxHwStateOn )
				{
        		// No params to check
				}
			else
				{
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: HW isn't ready, iHwState = %d"), iHwState));
				retVal = KErrNotReady;
				}         	
        	}
        	break;        
        
        case HWRMFmTxCommand::ESetTxRdsLangIdCmdId:
        	{
        	COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ESetTxRdsLangIdCmdId"));
        	        	
			if ( iHwState == HWRMFmTxCommand::EFmTxHwStateOn )
				{
        		// Check TRdsLangIdPackage param
        		HWRMFmTxCommand::TRdsLangIdPackage pckg;
				pckg.Copy(aData);        	
				TInt languageId = pckg();
                const TInt KRdsLanguageFirst = 0x00;
                const TInt KRdsLanguageLast = 0x7F;

                if( languageId < KRdsLanguageFirst || languageId > KRdsLanguageLast )
                    {
                    // Should not come here because input verified in client end
                    User::Leave(KErrArgument);
                    }
				}
			else
				{
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: HW isn't ready, iHwState = %d"), iHwState));
				retVal = KErrNotReady;
				}           	
        	}
        	break;        
        
        case HWRMFmTxCommand::ESetTxRtCmdId:
        	{
        	COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed ESetTxRtCmdId"));
        	        	
			if ( iHwState == HWRMFmTxCommand::EFmTxHwStateOn )
				{
        		// Check TRtPackage params
	            HWRMFmTxCommand::TRtPackage pckg;
	            pckg.Copy(aData);
                HWRMFmTxCommand::TRtData rtData = pckg();

	            if( rtData.iTag1.iContentType > KMaxLengthMarker || rtData.iTag2.iContentType > KMaxLengthMarker ||
                    rtData.iTag1.iLengthMarker > KMaxLengthMarker || rtData.iTag2.iLengthMarker > KMaxLengthMarker )
                    {
                    retVal = KErrArgument;
                    }
				}
			else
				{
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: HW isn't ready, iHwState = %d"), iHwState));
				retVal = KErrNotReady;
				}            	
        	}
        	break;        
        
        case HWRMFmTxCommand::EClearTxRtCmdId:
        	{
        	COMPONENT_TRACE(_L("HWRM FmtxPlugin: Processed EClearTxRtCmdId"));
        	
			if ( iHwState != HWRMFmTxCommand::EFmTxHwStateOn )			
				{
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: HW isn't ready, iHwState = %d"), iHwState));
				retVal = KErrNotReady;
				}             	        	
			// No params to check
        	}
        	break;

		case HWRMFmTxCommand::ENoCommandId:   // fall through
        default :
            {
            COMPONENT_TRACE((_L("HWRM FmtxPlugin: Unknown Command: 0x%x"), aCommandId));
            }
            break;
        }

    // Check for concurrent requests. Scan request (ETxScanRequestCmdId) may precede
    // set frequency (ESetTxFrequencyCmdId), because it is handled as a split command.
    if ( iLastCommand != HWRMFmTxCommand::ENoCommandId &&
         !(iLastCommand == HWRMFmTxCommand::ETxScanRequestCmdId && aCommandId == HWRMFmTxCommand::ESetTxFrequencyCmdId) )
    	{
		COMPONENT_TRACE(_L("HWRM FmtxPlugin: Not ready due to concurrent command"));
    	retVal = KErrNotReady;
    	}
	iLastCommand = static_cast<HWRMFmTxCommand::TFmTxCmd>(aCommandId);

    TInt timeout(500); // microseconds

    // Increase timeout for scan requests
    if (HWRMFmTxCommand::ETxScanRequestCmdId == aCommandId)
    	{
    	timeout = 2*1000*1000; // 2 seconds
    	}

#ifdef PUBLISH_STATE_INFO
    // publish
    iCmdProperty.Set(aCommandId);
    iDataProperty.Set(aData);
#endif /* PUBLISH_STATE_INFO */

    // create new timer
    CPluginTimer* timer = CPluginTimer::NewL(timeout, iResponseCallback, aCommandId, aTransId, retVal, this);
    CleanupStack::PushL(timer);
    iTimers.AppendL(timer);
    CleanupStack::Pop(timer);
    
    COMPONENT_TRACE((_L("HWRM FmtxPlugin: Processing command - return")));
    }


void CFmtxPlugin::CancelCommandL(const TUint8 aTransId,
#if defined(_DEBUG) && defined(COMPONENT_TRACE_FLAG)
                                 const TInt aCommandId)
#else
                                 const TInt /*aCommandId*/)
#endif
    {
    COMPONENT_TRACE((_L("HWRM FmtxPlugin: Cancelling command: 0x%x, TransId: 0x%x"), aCommandId, aTransId));
    COMPONENT_TRACE((_L("HWRM FmtxPlugin: Cancelling command - iTimers.Count(): %d "), iTimers.Count()));

    for( TInt i = 0; i < iTimers.Count(); i++ )
        {
        if ( iTimers[i]->TransId() == aTransId )
            {
            delete iTimers[i];
            iTimers.Remove(i);
            COMPONENT_TRACE((_L("HWRM FmtxPlugin: Cancelling command - Removed command: 0x%x, TransId: 0x%x"), aCommandId, aTransId));
            break;
            }
        }

    if ( iTimers.Count() == 0 )
    	{
    	// no more commands on-going
    	iLastCommand = HWRMFmTxCommand::ENoCommandId;
    	}
    }

void CFmtxPlugin::GenericTimerFired(MHWRMPluginCallback* aService,
                                    TInt aCommandId,
                                    const TUint8 aTransId,
                                    TInt aRetVal)
    {
    COMPONENT_TRACE((_L("HWRM FmtxPlugin: GenericTimerFired (0x%x, 0x%x, %d)"), aCommandId, aTransId, aRetVal));

    __ASSERT_ALWAYS(aService != NULL, User::Invariant() );

	TInt err = KErrNone;
	
	HWRMFmTxCommand::TFmTxHwState tempState = HWRMFmTxCommand::EFmTxHwStateOff;

    switch (aCommandId)
        {
        case HWRMFmTxCommand::EGetTxFrequencyRangeCmdId:
        	{
			COMPONENT_TRACE((_L("HWRM FmtxPlugin: Returning freq range")));
			HWRMFmTxCommand::TFrequencyRangeData freqRange;
			freqRange.iErrorCode = KErrNone;
			freqRange.iMinFrequency = KFmTxStubPluginFreqMin;
			freqRange.iMaxFrequency = KFmTxStubPluginFreqMax;
			freqRange.iStepSize = KFmTxStubPluginStepSize;
    		HWRMFmTxCommand::TFrequencyRangePackage freqRangePckg(freqRange);
    		TRAP(err, aService->ProcessResponseL(aCommandId, aTransId, freqRangePckg)); 
        	}
        	break;

        case HWRMFmTxCommand::ETxScanRequestCmdId:
        	{
			HWRMFmTxCommand::TScanResponseData scanData;
			scanData.iErrorCode = aRetVal;
			if ( aRetVal == KErrNone )
				{
				if ( (iClearFrequency += KFmTxStubPluginStepSize) > KFmTxStubPluginFreqMax )
					{
					iClearFrequency = KFmTxStubPluginFreqMin;
					}
				scanData.iErrorCode = KErrNone;
    			scanData.iFrequenciesFound = iChannelsRequested;
    			TUint32 clearFrequency = iClearFrequency;
    			for(TInt i=0; i<iChannelsRequested; i++)
    			    {
    			    scanData.iChannels.Copy(&clearFrequency,i+1);
    				if ( (clearFrequency += KFmTxStubPluginStepSize) > KFmTxStubPluginFreqMax )
    					{
    					clearFrequency = KFmTxStubPluginFreqMin;
    					}
    			    }
    			COMPONENT_TRACE((_L("HWRM FmtxPlugin: Returning 1 clear frequency")));
				}
			else
				{
				scanData.iFrequenciesFound = 0;
				COMPONENT_TRACE((_L("HWRM FmtxPlugin: Returning 0 clear frequencies")));
				}			
    		HWRMFmTxCommand::TScanResponsePackage scanPckg(scanData);
    		TRAP(err, aService->ProcessResponseL(aCommandId, aTransId, scanPckg)); 
    		}
        	break;

        case HWRMFmTxCommand::ETxOnCmdId:           // fall through
        case HWRMFmTxCommand::ESetTxFrequencyCmdId: 
        	tempState = HWRMFmTxCommand::EFmTxHwStateOn; // fall through
        case HWRMFmTxCommand::ETxOffCmdId:          
        	{
        	if ( aRetVal == KErrNone)
        		{
        		iHwState = tempState;
        	
				if ( iStatusIndTransId ) // if request successful, and status indication has been requested
	        		{	
    	    		// also send frequency/state changed indications for these
 					COMPONENT_TRACE((_L("HWRM FmtxPlugin: sending status indication")));
 					HWRMFmTxCommand::TFmTxStatusData statusData;
 					statusData.state     = tempState;
					statusData.frequency = iFrequency;
		    		HWRMFmTxCommand::TStatusIndicationPackage statusIndPckg(statusData);
		    		TInt tempTransId = iStatusIndTransId; // status indication may be reissued immediately...
	    			iStatusIndTransId = 0;                // ...so null here first
					TRAP(err, aService->ProcessResponseL(HWRMFmTxCommand::ETxStatusIndId, tempTransId, statusIndPckg)); 
					if ( err != KErrNone )
						{
						COMPONENT_TRACE((_L("HWRM FmtxPlugin: Error sending status indication")));
						}
					}
				}
        	} // fall through

        case HWRMFmTxCommand::ESetTxRdsPsCmdId:
        case HWRMFmTxCommand::ESetTxRtCmdId:
        default :
            {
            COMPONENT_TRACE((_L("HWRM FmtxPlugin: Returning error code %d, Command: 0x%x"), aRetVal, aCommandId));
    		HWRMFmTxCommand::TErrorCodeResponsePackage retvalPackage(aRetVal);
    		TRAP(err, aService->ProcessResponseL(aCommandId, aTransId, retvalPackage));          
            }
            break;
        }

    if ( err != KErrNone )
        {
        COMPONENT_TRACE((_L("HWRM FmtxPlugin: Error in ProcessResponseL: %d"), err));
        }

    // delete obsolete timers
    for( TInt i = (iTimers.Count()-1); i > -1 ; i-- )
        {
        if ( !iTimers[i]->IsActive() )
            {
            delete iTimers[i];
            iTimers.Remove(i);
            COMPONENT_TRACE((_L("HWRM FmtxPlugin: GenericTimerFired - Removed obsolete timer")));
            }
        }
        
    if ( iTimers.Count() == 0 )
    	{
    	// no more commands on-going
    	iLastCommand = HWRMFmTxCommand::ENoCommandId;
    	}
    }
