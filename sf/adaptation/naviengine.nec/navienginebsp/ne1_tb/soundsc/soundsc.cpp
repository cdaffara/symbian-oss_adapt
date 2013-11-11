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
* \bsp\hwip_nec_naviengine\ne1_tb\soundsc\sound.cpp
* Implementation of a sound physical device driver (PDD) factory
*
*/



#include "soundsc_plat.h"
#include <naviengine.h>
#include <i2s.h>

_LIT(KSoundScPddName,"SoundSc.NE1_TBVariant");

// Definitions for the kernel thread created for this sound driver.
_LIT(KSoundScDriverThreadName,"SoundDriverThread");
const TInt KSoundScDriverThreadPriority=26;				// One less than DFC thread 0 (26)

/**
Define a function at ordinal 0 which returns a new instance of a DPhysicalDevice-derived factory class.
*/
DECLARE_STANDARD_PDD()
	{
	return new DSoundScPddNE1_TB;
	}

/**
Constructor for the shared chunk sound PDD factory class.
*/
DSoundScPddNE1_TB::DSoundScPddNE1_TB()
	{
	__KTRACE_SND(Kern::Printf(">DSoundScPddNE1_TB::DSoundScPddNE1_TB"));

	// Support units KSoundScTxUnit0 & KSoundScRxUnit0.
    iUnitsMask=(1<<KSoundScRxUnit0)|(1<<KSoundScTxUnit0);

    // Set version number for this device.
	iVersion=RSoundSc::VersionRequired();
	}

/**
Destructor for the shared chunk sound PDD factory class.
This function is called from the client thread context.
*/
DSoundScPddNE1_TB::~DSoundScPddNE1_TB()
	{
	__KTRACE_SND(Kern::Printf(">DSoundScPddNE1_TB::~DSoundScPddNE1_TB"));

	// Destroy the kernel thread.
	if (iDfcQ)
		{
		iDfcQ->Destroy();
		}
	}

/**
Second stage constructor for the shared chunk sound PDD factory class.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DSoundScPddNE1_TB::Install()
	{
	__KTRACE_SND(Kern::Printf(">DSoundScPddNE1_TB::Install"));
	TInt r = KErrNone;
	if (!iDfcQ)
		{
		// Create a new sound driver DFC queue (and associated kernel thread).
		r = Kern::DynamicDfcQCreate(iDfcQ, KSoundScDriverThreadPriority, KSoundScDriverThreadName);
		if (r != KErrNone)
			{
			return r;
			}
		}

#ifdef CPU_AFFINITY_ANY
	NKern::ThreadSetCpuAffinity((NThread*)(iDfcQ->iThread), KCpuAffinityAny);
#endif

	r = SetName(&KSoundScPddName); 				// Set the name of the driver object

	return(r);
	}

/**
Returns the PDD's capabilities. This is not used by the Symbian OS device driver framework
or by the LDD.
@param aDes A descriptor to write capabilities information into
*/
void DSoundScPddNE1_TB::GetCaps(TDes8& /*aDes*/) const
	{}

/**
Called by the kernel's device driver framework to check if this PDD is suitable for use
with a logical channel.
This is called in the context of the client thread which requested the creation of a logical
channel - through a call to RBusLogicalChannel::DoCreate().
The thread is in a critical section.
@param aUnit The unit argument supplied by the client to RBusLogicalChannel::DoCreate().
@param aInfo The info argument supplied by the client to RBusLogicalChannel::DoCreate() - not used.
@param aVer The version number of the logical channel which will use this physical channel.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DSoundScPddNE1_TB::Validate(TInt aUnit, const TDesC8* /*aInfo*/, const TVersion& aVer)
	{
	// Check that the version specified is compatible.
	if (!Kern::QueryVersionSupported(RSoundSc::VersionRequired(), aVer))
		{
		return(KErrNotSupported);
		}

	// Check the unit number is compatible
	if (aUnit!=KSoundScTxUnit0 && aUnit!=KSoundScRxUnit0)
		{
		return(KErrNotSupported);
		}

	return(KErrNone);
	}

/**
Called by the kernel's device driver framework to create a physical channel object.
This is called in the context of the client thread which requested the creation of a logical
channel - through a call to RBusLogicalChannel::DoCreate().
The thread is in a critical section.
@param aChannel Set by this function to point to the created physical channel object.
@param aUnit The unit argument supplied by the client to RBusLogicalChannel::DoCreate().
@param aInfo The info argument supplied by the client to RBusLogicalChannel::DoCreate().
@param aVer The version number of the logical channel which will use this physical channel.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DSoundScPddNE1_TB::Create(DBase*& aChannel, TInt aUnit, const TDesC8* /*anInfo*/, const TVersion& /*aVer*/)
	{
	__KTRACE_SND(Kern::Printf(">DSoundScPddNE1_TB::Create"));

	TInt r = KErrNone;

	// Create the appropriate PDD channel object.
	DNE1_TBSoundScPddChannel* pD = NULL;

	if (aUnit==KSoundScRxUnit0)
		{
		// Create a record PDD channel object
		pD = new DNE1_TBSoundScPddChannel(ESoundDirRecord);
		}
	else
		{
		// Create a playback PDD channel object
		pD = new DNE1_TBSoundScPddChannel(ESoundDirPlayback);
		}

	r = KErrNoMemory;
	if (pD)
		{
		pD->iPhysicalDevice = this;
		r = pD->DoCreate();
		aChannel = pD;
		}
	return(r);
	}
