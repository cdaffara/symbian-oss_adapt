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
* ne1_tb\ethernet\shared_ethernet.cpp
* Ethernet driver implementation.
*
*/



/**
 @addtogroup 
 @ingroup 
*/

#include "shared_ethernet.h"


/********************************************************************/
/*  Class DEthernetPowerHandler implementation                      */
/********************************************************************/

/*
 * Device power down modes
 */
enum TPowerDownMode
	{
	ENormalPowerDown,
	EEmergencyPowerDown
	};

_LIT(KLitEthernet,"Ethernet");

DEthernetPowerHandler::DEthernetPowerHandler()
:DPowerHandler(KLitEthernet),
iCurPoweredUp(EFalse),
iCurPowerState(EPwActive)
	{
	}

void DEthernetPowerHandler::RequestPower()
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetPowerHandler::RequestPower()");
	#endif

    // Do power up the Ethernet if not already powered up.
    if(!iCurPoweredUp)
		{
	    iCurPoweredUp = ETrue;
		}
	}

TInt DEthernetPowerHandler::SetEthernetPdd(DEthernetPdd* aEthernetPdd)
	{
   #if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetPowerHandler::SetEthernetPdd()");
   #endif

	 iEthernetPdd = aEthernetPdd;
	 return KErrNone;
	}

void DEthernetPowerHandler::PowerUp()
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetPowerHandler::PowerUp()");	
    #endif
    iCurPowerState = EPwActive;
	iEthernetPdd->Wakeup();
    PowerUpDone();
	}

void DEthernetPowerHandler::PowerDown(TPowerState aState)
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetPowerHandler::PowerDown()");	
    #endif
	iCurPowerState = aState;
	// Stop recieving more frames.
	iEthernetPdd->Stop(EStopNormal);

    // Do power down on Ethernet
	RelinquishPower();
    PowerDownDone();
	}

void DEthernetPowerHandler::RelinquishPower()
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetPowerHandler::RelinquishPower()");	
    #endif

    // Do power standby on Ethernet if it is powered up.
    if(iCurPoweredUp)
		{
 	    iCurPoweredUp = EFalse;
		}
	iEthernetPdd->Sleep();
	}
 
DEthernetPdd::DEthernetPdd()
//Constructor
	{
	}

DEthernetPdd::~DEthernetPdd()
//Destructor
	{
	}

TInt DEthernetPdd::Start()
/**
 * Start receiving frames
 * @return KErrNone if driver started
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetPdd::Start()");
    #endif

	if(iReady)
		{
		return KErrNone;
		}
	iReady = ETrue;
	TInt r = Configure(iDefaultConfig);
	return r;
	}

TInt DEthernetPdd::ValidateConfig(const TEthernetConfigV01& aConfig) const
/**
 * Validate a new config
 * Validates a new configuration should be called before Configure
 * @param aConfig is the configuration to be validated
 * @return ETrue or EFalse if the Configuration is allowed
 * @see Configure()
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetPdd::ValidateConfig()");
    #endif
	switch(aConfig.iEthSpeed)
		{
	case KEthSpeedAuto:
	case KEthSpeed10BaseT:
	case KEthSpeed100BaseTX:
		break;
	case KEthSpeedUnknown:
	default:
		return KErrNotSupported;
		}

	switch(aConfig.iEthDuplex)
		{
	case KEthDuplexAuto:
	case KEthDuplexFull:
	case KEthDuplexHalf:
		break;
	default:
	case KEthDuplexUnknown:
		return KErrNotSupported;
		}

	return KErrNone;
	}



void DEthernetPdd::GetConfig(TEthernetConfigV01& aConfig) const
/**
 * Get the current config from the chip
 * This returns the current configuration of the chip with the folling fields
 * The Transmit Speed
 * The Duplex Setting
 * The MAC address
 * @param aConfig is a TEthernetConfigV01 reference that will be filled in
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetPdd::GetConfig()");
    #endif
	aConfig = iDefaultConfig;
	return;
	}


void DEthernetPdd::CheckConfig(TEthernetConfigV01& /*aConfig*/)
/**
 * Check a configuration
 * @param aConfig	a reference to the structure TEthernetConfigV01 with configuration to check
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetPdd::CheckConfig()");
    #endif
	return;
	}


void DEthernetPdd::Caps(TDes8& /*aCaps*/) const
/**
 * Query the device's capabilities
 * @param aCaps To be filled in with the capabilites
 */
	{	
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetPdd::Caps()");
    #endif
	return;
	}

TInt DEthernetPdd::DisableIrqs()
/**
 * Disables all IRQ's
 * @return The IRQ level before it was changed
 * @see RestoreIrqs()
 */
	{
	return KErrNotSupported;
	}

void DEthernetPdd::RestoreIrqs(TInt /*aIrq*/)
/**
 * Restore the IRQ's to the supplied level
 * @param aIrq The level to set the irqs to.
 * @see DisableIrqs()
 */
	{
	}

TDfcQue* DEthernetPdd::DfcQ(TInt /*aUnit*/)
/**
 * Return the DFC Queue that this device should use
 * @param aUnit The Channel number
 * @return Then DFC Queue to use
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetPdd::DfcQ()");
	#endif

	return iDfcQ;
	}
