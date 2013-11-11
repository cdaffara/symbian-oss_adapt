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
* ne1_tb\ethernet\shared_ethernet.h
* Ethernet driver common header
*
*/




#ifndef __SHARED_ETHERNET_H__
#define __SHARED_ETHERNET_H__

#include <e32def.h>
#include <kernel/kernel.h>
#include <kernel/kern_priv.h>
#include <ethernet.h>
#include <nkern.h>

//
// Driver Constants
//

class DEthernetPdd;

/******Ethernet POWER HANDLER CLASS*****/
class DEthernetPowerHandler : public DPowerHandler
	{
public:
	DEthernetPowerHandler();
	TInt SetEthernetPdd(DEthernetPdd* aEthernetPdd);
    void PowerDown(TPowerState aState);
    void PowerUp();
	void RequestPower();
	void RelinquishPower();
private:
    TBool iCurPoweredUp;
    TPowerState	iCurPowerState;
    DEthernetPdd* iEthernetPdd;
	};


class DEthernetPdd : public DEthernet
/**
Ethernet PDD class.
*/
	{
public:
    DEthernetPdd();
    ~DEthernetPdd();

    /**
     * Start receiving frames
     * @return KErrNone if driver started
     */
    virtual TInt Start() ;
    /**
     * Stop receiving frames
     * @param aMode The stop mode
     */
    virtual void Stop(TStopMode aMode) = 0;

    /**
     * Validate a new config
     * Validates a new configuration should be called before Configure
     * @param aConfig is the configuration to be validated
     * @return ETrue or EFalse if the Configuration is allowed
     * @see Configure()
     */
    virtual TInt ValidateConfig(const TEthernetConfigV01 &aConfig) const;
    /**
     * Configure the device
     * Reconfigure the device using the new configuration supplied.
     * This should not change the MAC address.
     * @param aConfig The new configuration
     * @see ValidateConfig()
     * @see MacConfigure()
     */
    virtual TInt Configure(TEthernetConfigV01 &aConfig) = 0;
    /**
     * Change the MAC address
     * Attempt to change the MAC address of the device
     * @param aConfig A Configuration containing the new MAC
     * @see Configure()
     */
    virtual void MacConfigure(TEthernetConfigV01 &aConfig) = 0;
    /**
     * Get the current config from the chip
     * This returns the current configuration of the chip with the following fields
     * The Transmit Speed
     * The Duplex Setting
     * The MAC address
     * @param aConfig is a TEthernetConfigV01 reference that will be filled in
     */
    virtual void GetConfig(TEthernetConfigV01 &aConfig) const;
    /**
     * Check a configuration
     * @param aConfig	a reference to the structure TEthernetConfigV01 with configuration to check
     */
    virtual void CheckConfig(TEthernetConfigV01& aConfig);

    /**
     * Query the device's capabilities
     * @param aCaps To be filled in with the capabilites
     */
    virtual void Caps(TDes8 &aCaps) const;

    /**
     * Transmit data
     * @param aBuffer reference to the data to be sent
     * @return KErrNone if the data has been sent
     */
    virtual TInt Send(TBuf8<KMaxEthernetPacket+32> &aBuffer) = 0;
    /**
     * Retrieve data from the device
     * Pull the received data out of the device and into the supplied buffer. 
     * Need to be told if the buffer is OK to use as if it not we could dump 
     * the waiting frame in order to clear the interrupt if necessory.
     * @param aBuffer Reference to the buffer to be used to store the data in
     * @param okToUse Bool to indicate if the buffer is usable
     * @return KErrNone if the buffer has been filled.
     */
    virtual TInt ReceiveFrame(TBuf8<KMaxEthernetPacket+32> &aBuffer, 
                              TBool okToUse) = 0;

	/**
	 * Disables all IRQ's
	 * @return The IRQ level before it was changed
	 * @see RestoreIrqs()
	 */
	virtual TInt DisableIrqs();
	/**
	 * Restore the IRQ's to the supplied level
	 * @param aIrq The level to set the irqs to.
	 * @see DisableIrqs()
	 */
	virtual void RestoreIrqs(TInt aIrq);
	
    /**
     * Return the DFC Queue that this device should use
     * @param aUnit The Channel number
     * @return Then DFC Queue to use
     */
    virtual TDfcQue* DfcQ(TInt aUnit);

	virtual TInt	DoCreate() = 0;
	
	/**
	 * Put the card to sleep
	 */

    virtual void	Sleep() = 0;
	/**
	 * Wake the card up
	 */
    virtual TInt	Wakeup() = 0;

protected:
	static void ServiceRxDfc(TAny *aPtr);

	/**
	 * Does the soft reset of the lan card 
	 */
	TInt CardSoftReset();

protected:
	/**
	 * Contains the default/current configuration of the chip
	 */
	TEthernetConfigV01 iDefaultConfig;
	/**
	 * Is ETrue if the chip has been fully configured.
	 */
	TBool iReady;
	/**
	 * Is ETrue if the pdd has been created
	 */
	TBool iCreated;
	DEthernetPowerHandler iPowerHandler;
	TDynamicDfcQue* iDfcQ;
	TInt32 iInterruptId;
	};


#endif //__SHARED_ETHERNET_H__
