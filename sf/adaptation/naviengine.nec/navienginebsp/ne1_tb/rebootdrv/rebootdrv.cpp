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
* bsp\hwip_nec_naviengine\ne1_tb\rebootdrv.cpp
*
*/



#ifndef NAVIREBOOT_H
#define NAVIREBOOT_H
#include <rebootdrv.h>
#include <rebootdrv_ldd.h>
#include <naviengine.h>
#endif //NAVIREBBOT_H

/**
 * Class Constructor
 */
DLddDeviceReboot::DLddDeviceReboot()
	{
	iVersion=TVersion(KMajorVersionNumber,KMinorVersionNumber,KBuildVersionNumber);
	//No units, no info, no PDD
	}

/**
 * Implementatin of Create method of DLogicalDevice
 */
TInt DLddDeviceReboot::Create(DLogicalChannelBase*& aChannel)
    {
	aChannel=new DLddReboot;
	return aChannel?KErrNone:KErrNoMemory;
    }

/**
 * Implementatin of Install method of DLogicalDevice
 */
TInt DLddDeviceReboot::Install()
	{
	return SetName(&KRebootLddName);
	}

/**
 * Implementatin of GetCaps method of DLogicalDevice
 */
void DLddDeviceReboot::GetCaps(TDes8 &aDes) const
	{
	TCapsRebootV1 b;
	b.version=iVersion;
	aDes.FillZ(aDes.MaxLength());
	aDes.Copy((TUint8 *)&b,Min(aDes.MaxLength(),sizeof(b)));
	}

/**
 * Class Constructor
 */
DLddReboot::DLddReboot()
	{
	// we are in client's thread context, let's make sure that it knows about us
	iClient=&Kern::CurrentThread();
	((DObject*)iClient)->Open();
	}

/**
 * Class Destructor
 */
DLddReboot::~DLddReboot()
	{
	Kern::SafeClose((DObject*&)iClient, NULL);
	}

/**
 * Implementation of GetCaps method of DLogicalChannelBase
 */

TInt DLddReboot::Request(TInt aReqNo,TAny* a1,TAny* a2)
	{
	// we are in client's thread context, let's make sure we aren't
	// killed or suspended
	NKern::ThreadEnterCS();		
	
	switch (aReqNo)
		{
	case RReboot::EReboot:
		{
		TNandMediaInfo mediaInfo;
		NKern::ThreadLeaveCS(); // leave CS as getting mediaInfo may kill us
		kumemget(&mediaInfo,a1,sizeof(mediaInfo));
		NKern::ThreadEnterCS();
		(void)Reboot(mediaInfo);
		}
		break;
	case RReboot::EGenericReboot:
		{
		(void)GenericReboot();
		}
	default:
		break;
		}
	
	NKern::ThreadLeaveCS();			// unnecessary we should be dead by now...
	return KErrNone;
	}

TInt DLddReboot::Reboot(TNandMediaInfo& /*aNandMediaInfo*/)
	{
	// indicate to boot loader that NAND mini boot routine is required
	Kern::Restart(TInt(KtRestartReasonNANDImage ));
	return KErrNone;
	}


TInt DLddReboot::GenericReboot()
	{
	Kern::Restart(KtRestartReasonHardRestart);
	return KErrNone;
	}

/*
 * LDD Entry point
 */
DECLARE_STANDARD_LDD()
	{
    return new DLddDeviceReboot;
    }

// @}

