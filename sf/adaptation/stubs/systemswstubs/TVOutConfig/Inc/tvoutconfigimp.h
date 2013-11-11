/*
* Copyright (c) 2003-2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  This file provides the headers used in 
*                TVOutConfig Dummy Implementation.
*
*/


#ifndef __TVOUTCONFIGIMP_H__
#define __TVOUTCONFIGIMP_H__

//- Include Files  ----------------------------------------------------------

#include <e32base.h>
#include "tvoutconfig.h"

//- Class Definitions -------------------------------------------------------

/**
Inherited implementation of CTvOutConfig
*/
NONSHARABLE_CLASS(CTvOutConfigImp) : public CTvOutConfig
    {
    public:
        CTvOutConfigImp();
        virtual ~CTvOutConfigImp();
        virtual TInt GetNumberOfHdmiModes(TUint& aNumberOfModes);
        virtual TInt GetSupportedHdmiMode(TUint aModeNumber, TSupportedHdmiDviMode& aReadMode);
        virtual TInt GetTvHwCapabilities(TTvSettings::TOutput aConnector, THwCapabilities& aReadCapabilities);
        virtual TInt SetConfig(const TTvSettings& aTvSettings);
        virtual TInt SetConfig(const THdmiDviTimings& aTvSettings);
        virtual TInt GetConfig(TTvSettings& aTvSettings);
        virtual TInt GetConfig(THdmiDviTimings& aTvSettings);
        virtual TInt SettingsChangedListener(TRequestStatus& aListenerRequest);
        virtual TInt SettingsChangedListenerCancel();
        virtual TInt StandByFigureMaxSizeInBytes(TUint& aMaxFigureSize);
        virtual TInt StandByFigure(const TStandByFigure& aStandByFigure);
        virtual TInt GetStandByFigure(TStandByFigure& aStandByFigure);
        virtual TInt Enable();
        virtual TInt Disable();
        virtual TInt OnOffListener(TRequestStatus& aListenerRequest);
        virtual TInt OnOffListenerCancel();
        virtual TBool Enabled();
        virtual TInt HdmiCableListener(TRequestStatus& aListenerRequest);
        virtual TInt HdmiCableListenerCancel();
        virtual TBool HdmiCableConnected();
        virtual TInt SetAvailableTvConfigList(const RArray<TTvSettings>& aAnalogConfigs, const RArray<THdmiDviTimings>& aHdmiConfigs);
        virtual TInt GetAvailableTvConfigList(RArray<TTvSettings>& aAnalogConfigs, RArray<THdmiDviTimings>& aHdmiConfigs);
        virtual TInt AvailableTvConfigListListener(TRequestStatus& aListenerRequest);
        virtual TInt AvailableTvConfigListListenerCancel();
        virtual TInt CopyProtectionStatusListener(TRequestStatus& aListenerRequest);
        virtual TInt CopyProtectionStatusCancel();
        virtual TBool CopyProtectionStatus();
        virtual TInt CopyProtection(TBool aCopyProtectEnabled);
        virtual void ConstructL();
    };

#endif //__TVOUTCONFIGIMP_H__

// End of File

