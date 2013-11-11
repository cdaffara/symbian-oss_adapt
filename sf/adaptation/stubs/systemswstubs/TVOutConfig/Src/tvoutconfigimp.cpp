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
* Description:  Implements the all the virtual functions derived from 
                 CTvOutConfig  TVOutConfig Dummy Implementation.
*
*/


//- Include Files  ----------------------------------------------------------

#include "tvoutconfigimp.h"

//- External Data -----------------------------------------------------------

//- External Function Prototypes --------------------------------------------

//- Constants ---------------------------------------------------------------

//- Macros ------------------------------------------------------------------

//- Global and Local Variables ----------------------------------------------

//- Local Function Prototypes -----------------------------------------------

//- Local Functions ---------------------------------------------------------

//- Member Functions --------------------------------------------------------

//=============================================================================

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
CTvOutConfigImp::CTvOutConfigImp():
        CTvOutConfig()
    {

    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
CTvOutConfigImp::~CTvOutConfigImp()
    {
    }

//---------------------------------------------------------------------------
/**
    Dummy implementation.
*/
void CTvOutConfigImp::ConstructL()
    {
    //This is dummy implementation. does nothing
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::SetConfig( const TTvSettings& /*aTvSettings*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::StandByFigureMaxSizeInBytes( TUint& /*aMaxFigureSize*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::StandByFigure( const TStandByFigure& /*aStandByFigure*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::Enable()
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::Disable()
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TBool CTvOutConfigImp::Enabled()
    {
    return EFalse;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::OnOffListener( TRequestStatus& /*aListenerRequest*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::OnOffListenerCancel()
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::HdmiCableListener(TRequestStatus& /*aListenerRequest*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::HdmiCableListenerCancel()
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::SetConfig( const THdmiDviTimings& /*aTvSettings*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TBool CTvOutConfigImp::HdmiCableConnected()
    {
    return EFalse;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::GetNumberOfHdmiModes(TUint& /*aNumberOfModes*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::GetSupportedHdmiMode(TUint /*aModeNumber*/, TSupportedHdmiDviMode& /*aReadMode*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::GetTvHwCapabilities(TTvSettings::TOutput /*aConnector*/, THwCapabilities& /*aReadCapabilities*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::GetConfig(TTvSettings& /*aTvSettings*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::GetConfig(THdmiDviTimings& /*aTvSettings*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::GetStandByFigure(TStandByFigure& /*aStandByFigure*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::SetAvailableTvConfigList(const RArray<TTvSettings>& /*aAnalogConfigs*/, const RArray<THdmiDviTimings>& /*aHdmiConfigs*/)
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
 */
TInt CTvOutConfigImp::GetAvailableTvConfigList( RArray<TTvSettings>& /*aAnalogConfigs*/, RArray<THdmiDviTimings>& /*aHdmiConfigs*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::AvailableTvConfigListListener(TRequestStatus& /*aListenerRequest*/)
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::AvailableTvConfigListListenerCancel()
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::SettingsChangedListener(TRequestStatus& /*aListenerRequest*/)
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::SettingsChangedListenerCancel()
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::CopyProtection( TBool /*aCopyProtectEnabled*/ )
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TBool CTvOutConfigImp::CopyProtectionStatus()
    {
    return EFalse;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::CopyProtectionStatusCancel()
    {
    return KErrNotReady;
    }

//---------------------------------------------------------------------------
/**
    @see TvOutConfig.h
*/
TInt CTvOutConfigImp::CopyProtectionStatusListener( TRequestStatus& /*aListenerRequest*/ )
    {
    return KErrNotReady;
    }

// End of File
