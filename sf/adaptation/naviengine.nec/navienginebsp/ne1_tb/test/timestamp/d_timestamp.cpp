/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


#include <kern_priv.h>
#include <kernel.h>
#include "d_timestamp.h"
#include "d_timestamp_dev.h"
#include "ne1_tb_power.h"

// Name for PDD, will be LDD name and this suffix
_LIT(KTimestampPddSuffix,".NE1_TB");


class DNE1_TimestampTestPddChannel : public DTimestampTestPddChannel
	{
public:
	// Inherited from DTimestampTestPddChanel. These called by the LDD.
	virtual void StartLPMEntryCheck();
    virtual TBool EndLPMEntryCheck();
    virtual void TestConfig(STimestampTestConfig& aInfo);
private:
	TUint iInitialIdleCount;
	};

/**
  Logical Device (factory class) for DNE1_TimestampTestPddChannel
*/
class DNE1_TimestampTestPddFactory : public DPhysicalDevice
	{
public:
	DNE1_TimestampTestPddFactory();
	//	Inherited from DLogicalDevice
	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
    virtual TInt Create(DBase*& aChannel, TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);
	virtual TInt Validate(TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);
private:
    TVersion iVersion;
	};

//
// DNE1_TimestampTestPddFactory
//

/**
  Standard export function for PDDs. This creates a DPhysicalDevice derived object,
  in this case, our DNE1_TimestampTestPddFactory
*/
DECLARE_STANDARD_PDD()
	{
	return new DNE1_TimestampTestPddFactory();
	}

/**
 * constructor
 */
DNE1_TimestampTestPddFactory::DNE1_TimestampTestPddFactory()
	{
	// Set version number for this device
	iVersion=RTimestampTest::VersionRequired();
	}

/**
  Second stage constructor for DPhysicalDevice derived objects.
  This must at least set a name for the driver object.

  @return KErrNone or standard error code.
*/
TInt DNE1_TimestampTestPddFactory::Install()
    {
    TName name(RTimestampTest::Name());
    name.Append(KTimestampPddSuffix);
    return SetName(&name);
	}

/**
  Returns the drivers capabilities. This is not used by the Symbian OS device driver framework
  but may be useful for the LDD to use.

  @param aDes Descriptor to write capabilities information into
*/
void DNE1_TimestampTestPddFactory::GetCaps(TDes8& aDes) const
	{
	// Create a capabilities object
	RTimestampTest::TCaps caps;
	caps.iVersion = iVersion;
    // Write it back to user memory
	Kern::InfoCopy(aDes,(TUint8*)&caps,sizeof(caps));
	}

/**
  Called by the kernel's device driver framework to create a Physical Channel.
  This is called in the context of the user thread (client) which requested the creation of a Logical Channel
  (E.g. through a call to RBusLogicalChannel::DoCreate)
  The thread is in a critical section.

  @param aChannel Set to point to the created Physical Channel
  @param aUnit The unit argument supplied by the client to RBusLogicalChannel::DoCreate
  @param aInfo The info argument supplied by the client to RBusLogicalChannel::DoCreate
  @param aVer The version number of the Logical Channel which will use this Physical Channel 

  @return KErrNone or standard error code.
*/
TInt DNE1_TimestampTestPddFactory::Create(DBase*& aChannel, TInt aUnit, const TDesC8* aInfo, const TVersion& aVer)
	{
	// Ignore the parameters we aren't interested in...
	(void)aUnit;
	(void)aInfo;
	(void)aVer;

	// Create a new physical channel
	DNE1_TimestampTestPddChannel* channel=new DNE1_TimestampTestPddChannel;
    aChannel = channel;
    return (channel) ? KErrNone : KErrNoMemory;
	}

/**
  Called by the kernel's device driver framework to check if this PDD is suitable for use with a Logical Channel.
  This is called in the context of the user thread (client) which requested the creation of a Logical Channel
  (E.g. through a call to RBusLogicalChannel::DoCreate)
  The thread is in a critical section.

  @param aUnit The unit argument supplied by the client to RBusLogicalChannel::DoCreate
  @param aInfo The info argument supplied by the client to RBusLogicalChannel::DoCreate
  @param aVer The version number of the Logical Channel which will use this Physical Channel 

  @return KErrNone or standard error code.
*/
TInt DNE1_TimestampTestPddFactory::Validate(TInt aUnit, const TDesC8* aInfo, const TVersion& aVer)
	{
	// Check version numbers
	if (!Kern::QueryVersionSupported(iVersion,aVer))
		return KErrNotSupported;
        
    (void)aInfo;
    (void) aUnit;    
    return KErrNone;
    }

////
// Channel implementation


/**
   Called before each cycle in the test. Takes a copy of current idle count in power controller
*/
void  DNE1_TimestampTestPddChannel::StartLPMEntryCheck()
    {
    iInitialIdleCount = TNE1_TBPowerController::IdleCount();
    }

/**
   Called at the end of each cycle. Should return true if we have entered idle since call to
   StartLPMEntryCheck. This will be the case if the idle count has changed
*/
TBool  DNE1_TimestampTestPddChannel::EndLPMEntryCheck()
    {
    return (iInitialIdleCount!=TNE1_TBPowerController::IdleCount());
    }


/**
   Called to allow baseport to override test parameters. For Navi defaults are fine
*/
void DNE1_TimestampTestPddChannel::TestConfig(STimestampTestConfig& aInfo) 
    {
    }
