/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  
* ne1_tb\inc\mconf.h
* NE1_TBVariant Persistent Machine Configuration
*
*/



#ifndef __MCONF_H__
#define __MCONF_H__
#include <kernel/kernel.h>

class TDigitizerCalibrateValues
    {
public:
    TInt iR11;
    TInt iR12;
    TInt iR21;
    TInt iR22;
    TInt iTx;
    TInt iTy;
    };

class TTemplateMachineConfig : public TMachineConfig
	{
public:
	TSoundInfoV1 iSoundInfo;
    TOnOffInfoV1 iOnOffInfo;
	TTimeK iMainBatteryInsertionTime;
    Int64 iMainBatteryInUseMicroSeconds;
	Int64 iExternalPowerInUseMicroSeconds;
	Int64 iMainBatteryMilliAmpTicks;
	TDigitizerCalibrateValues iCalibration;
	TDigitizerCalibrateValues iCalibrationSaved;
	TDigitizerCalibrateValues iCalibrationFactory;
	};

typedef TTemplateMachineConfig TActualMachineConfig;

inline TActualMachineConfig& TheActualMachineConfig()
	{return (TActualMachineConfig&)Kern::MachineConfig();}

#endif
