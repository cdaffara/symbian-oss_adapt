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
* SMCS 9118 Ethernet driver implementation.
*
*/



#include "variant.h"
#include "smcs9118_ethernet.h"

const TUint32 ONE_MSEC				= 1000; // in nanoseconds
const TUint32 TWO_SECONDS			= 2000; // in milliseconds
const TUint32 SMCS9118_MAC_TIMEOUT	= 50;
const TUint32 SMCS9118_MAX_RETRIES	= 100;

 
DEthernetPddFactory::DEthernetPddFactory()
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetPddFactory::DEthernetPddFactory()");
	#endif
	iVersion=TVersion(KEthernetMajorVersionNumber,
			KEthernetMinorVersionNumber,
			KEthernetBuildVersionNumber);
	}

TInt DEthernetPddFactory::Install()
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetPddFactory::Install()");
	#endif
	return SetName(&KEthernetPddName);
	}

void DEthernetPddFactory::GetCaps(TDes8& /*aDes*/) const
/*
 * Return the drivers capabilities.
 */
	{
	}


TInt DEthernetPddFactory::Create(DBase*& aChannel, TInt aUnit, const TDesC8* aInfo, const TVersion& aVer)
/*
 * Create a Driver for the device.
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetPddFactory::Create()");
	#endif
	TInt r = Validate(aUnit, aInfo, aVer);
	if (r != KErrNone)
		{
		return r;
		}
	DEthernetPdd* pP = new DEthernetSMCS9118Pdd;
	aChannel = pP;
	if (!pP)
		{
		return KErrNoMemory;
		}
	r = pP->DoCreate();
	return r;
	}

TInt DEthernetPddFactory::Validate(TInt /*aUnit*/, const TDesC8* /*aInfo*/, const TVersion& aVer)
/*
 *  Validate the requested configuration
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetPddFactory::Validate()");
	#endif

	if (!Kern::QueryVersionSupported(iVersion,aVer))
		{
		return KErrNotSupported;
		}

	return KErrNone;
	}

#ifdef __SMP__
TSpinLock DEthernetSMCS9118PddLock(SMCS9118_LOCK_ORDER);
#endif
DEthernetSMCS9118Pdd::DEthernetSMCS9118Pdd()
//Constructor
	 :iRxDfc(ServiceRxDfc, this, 1)
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetSMCS9118Pdd::DEthernetSMCS9118Pdd()");	
    #endif

#ifdef __SMP__
	iDriverLock = &DEthernetSMCS9118PddLock;
#endif
	iReady = EFalse;
	}

DEthernetSMCS9118Pdd::~DEthernetSMCS9118Pdd()
//Destructor
	{

	//cancel any pending DFC requests
	iRxDfc.Cancel();
	
	// UnRegister the power handler with Symbian Power Framework
	iPowerHandler.RelinquishPower();
	iPowerHandler.Remove();

	// UnRegister interrupts
	DisableInterrupt(iInterruptId);
	UnbindInterrupt(iInterruptId);

	if (iDfcQ)
		iDfcQ->Destroy();
	}

void DEthernetSMCS9118Pdd::Stop(TStopMode aMode)
/**
 * Stop receiving frames
 * @param aMode The stop mode
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetSMCS9118Pdd::Stop()");
	#endif

	switch (aMode)
		{
	case EStopNormal:
	case EStopEmergency:
		iReady = EFalse;
		iRxDfc.Cancel();
		// disable Rx, Tx
		TUint32 status;
		TInt32 err = ReadMac(SMCS9118_MAC_CR, status);
		status &= ~(SMCS9118_MAC_RXEN | SMCS9118_MAC_TXEN | SMCS9118_MAC_RXALL);
		err = WriteMac(SMCS9118_MAC_CR, status);
		// clear any pending interrupts
		Write32(SMCS9118_INT_EN, 0);
		ByteTestDelay(1);
		ClearInterrupt(iInterruptId);
		// turn off the LED
		status = Read32(SMCS9118_GPIO_CFG) & ~SMCS9118_GPIO_LED_EN;
		Write32(SMCS9118_GPIO_CFG, status);
		ByteTestDelay(1);

		break;
		}
	}

TInt DEthernetSMCS9118Pdd::Configure(TEthernetConfigV01& /*aConfig*/)
/**
 * Configure the device
 * Reconfigure the device using the new configuration supplied.
 * This should not change the MAC address.
 * @param aConfig The new configuration
 * @see ValidateConfig()
 * @see MacConfigure()
 * assume iDriverLock not held
 */
	{
	TUint32 retry;
	TUint32 status;
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetSMCS9118Pdd::Configure()");
    #endif

  	if ((status = CardSoftReset()) != (TUint32)KErrNone)
		{
		return status;
		}

	TInt irq = DriverLock();
	// disable chip interrupts
	Write32(SMCS9118_INT_EN, 0);
	ByteTestDelay(1);
	Write32(SMCS9118_INT_STS, 0xffffffff);
	ByteTestDelay(2);
	Write32(SMCS9118_FIFO_INT, 0);
	ByteTestDelay(1);
	Write32(SMCS9118_IRQ_CFG, SMCS9118_IRQ_CFG_DEAS|SMCS9118_IRQ_CFG_TYPE);
	ByteTestDelay(3);

	// AutoFlowControl setup
	Write32(SMCS9118_AFC_CFG, SMCS9118_AFC_CFG_VAL);
	ByteTestDelay(1);

	// TX FIFO setup
	status = Read32(SMCS9118_HW_CFG);
	status |= SMCS9118_TX_FIFO_SZ; 
	Write32(SMCS9118_HW_CFG, status);
	ByteTestDelay(1);

	// wait for EEPROM load
	retry = SMCS9118_MAX_RETRIES;
	do
		{
		status = Read32(SMCS9118_E2P_CMD);
		if (!(status & SMCS9118_E2P_CMD_BUSY))
			{
			break;
			}
		Kern::NanoWait(ONE_MSEC);
		} while (--retry);
	if (retry == 0)
		{
		DriverUnlock(irq);
		return KErrGeneral;
		}

	// GPIO setup - turn on the LED !
	Write32(SMCS9118_GPIO_CFG, SMCS9118_GPIO_GPIOBUF|SMCS9118_GPIO_LED_EN);
	ByteTestDelay(1);
	
	// PHY reset
	status = Read32(SMCS9118_PMT_CTRL);
	status |= SMCS9118_PMT_PHY_RST; 
	Write32(SMCS9118_PMT_CTRL, status);
	ByteTestDelay(7);
	retry = SMCS9118_MAX_RETRIES;
	do
		{
		status = Read32(SMCS9118_PMT_CTRL);
		if (!(status & SMCS9118_PMT_PHY_RST))
			{
			break;
			}
		Kern::NanoWait(ONE_MSEC);
		} while (--retry);
	if (retry == 0)
		{
		DriverUnlock(irq);
		return KErrGeneral;
		}
	ByteTestDelay(1);

	// Auto negotiate setup
	TInt32 err = ReadPhy(SMCS9118_PHY_AUTONEG_AD, status);
	if (err != KErrNone)
		{
		DriverUnlock(irq);
		return err;
		}
	status |= SMCS9118_PHY_DEF_ANEG;
	err = WritePhy(SMCS9118_PHY_AUTONEG_AD, status);
	if (err != KErrNone)
		{
		DriverUnlock(irq);
		return err;
		}

	err = ReadPhy(SMCS9118_PHY_AUTONEG_AD, status);
	if (err != KErrNone)
		{
		DriverUnlock(irq);
		return err;
		}

	err = ReadPhy(SMCS9118_PHY_BCR, status);
	if (err != KErrNone)
		{
		DriverUnlock(irq);
		return err;
		}
	status |= (SMCS9118_PHY_ANEG_RESTART|SMCS9118_PHY_ANEG_EN);
	err = WritePhy(SMCS9118_PHY_BCR, status);
	if (err != KErrNone)
		{
		DriverUnlock(irq);
		return err;
		}

	// wait for auto negotiation
	DriverUnlock(irq);
	NKern::Sleep(TWO_SECONDS);
	irq = DriverLock();

	err = ReadPhy(SMCS9118_PHY_BSR, status);
	if (err != KErrNone)
		{
		DriverUnlock(irq);
		return err;
		}
	if (!(status & SMCS9118_PHY_ANEG_CMPL))
		{
		DriverUnlock(irq);
		return KErrGeneral;
		}

	// update the config based on what we negotiated
	if (status & (SMCS9118_PHY_100BTX|SMCS9118_PHY_100BTXFD))
		{
		iDefaultConfig.iEthSpeed  = KEthSpeed100BaseTX;
		}
	if (status & (SMCS9118_PHY_10BTFD|SMCS9118_PHY_100BTXFD))
		{
		iDefaultConfig.iEthDuplex = KEthDuplexFull;
		}

	// setup store + forward
	status = Read32(SMCS9118_HW_CFG);
	status |= SMCS9118_HW_CFG_SF; 
	Write32(SMCS9118_HW_CFG, status);
	ByteTestDelay(1);

	// setup Tx and Rx
	Write32(SMCS9118_TX_CFG, SMCS9118_TX_CFG_TXSAO|SMCS9118_TX_CFG_TX_ON);
	ByteTestDelay(1);

	TInt r;
	r = WriteMac(SMCS9118_MAC_CR, SMCS9118_MAC_RXEN|SMCS9118_MAC_TXEN|SMCS9118_MAC_RXALL);
	if (r != KErrNone)
		{
		DriverUnlock(irq);
		return r;
		}

	// Enable interrupts to CPU
    r = EnableInterrupt(iInterruptId);
    if(r != KErrNone)
		{
		TInt err;
    	__KTRACE_OPT(KHARDWARE,Kern::Printf("DEthernetSMSC9118Pdd::Start --- Interrupt::Enable()=%d", r));
    	
    	// Disable TX, RX and exit
		status &= ~(SMCS9118_MAC_RXEN | SMCS9118_MAC_TXEN | SMCS9118_MAC_RXALL);
		err = WriteMac(SMCS9118_MAC_CR, status);
		if (err != KErrNone)
			{
			DriverUnlock(irq);
			return err;
			}

		DriverUnlock(irq);
		return r;
		}
    
	// Rx Interrupt
	Write32(SMCS9118_INT_EN, SMCS9118_INT_EN_RSFL);
	ByteTestDelay(1);

	DriverUnlock(irq);
	return KErrNone;
	}


void DEthernetSMCS9118Pdd::MacConfigure(TEthernetConfigV01& aConfig)
/**
 * Change the MAC address
 * Attempt to change the MAC address of the device
 * @param aConfig A Configuration containing the new MAC
 * @see Configure()
 * assume iDriverLock not held
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetSMCS9118Pdd::MacConfigure()");
    #endif


	TUint32 mac, checkMac;
	TInt	err;
	
	TInt irq = DriverLock();
	mac = aConfig.iEthAddress[0];
	mac |= aConfig.iEthAddress[1]<<8;
	mac |= aConfig.iEthAddress[2]<<16;
	mac |= aConfig.iEthAddress[3]<<24;

	err = WriteMac(SMCS9118_MAC_ADDRL, mac);
	if (err)
		{
		__KTRACE_OPT(KHARDWARE, Kern::Printf("DEthernetSMCS9118Pdd::MacConfigure() -- Failed to set MAC Address"));
		DriverUnlock(irq);
		return;
		}
	err = ReadMac(SMCS9118_MAC_ADDRL, checkMac);
	if (err || checkMac != mac)
		{
		__KTRACE_OPT(KHARDWARE, Kern::Printf("DEthernetSMCS9118Pdd::MacConfigure() -- Failed to set MAC Address"));
		DriverUnlock(irq);
		return;
		}
	
	mac = aConfig.iEthAddress[4];
	mac |= aConfig.iEthAddress[5]<<8;
	err = WriteMac(SMCS9118_MAC_ADDRH, mac);
	if (err)
		{
		__KTRACE_OPT(KHARDWARE, Kern::Printf("DEthernetSMCS9118Pdd::MacConfigure() -- Failed to set MAC Address"));
		DriverUnlock(irq);
		return;
		}
	err = ReadMac(SMCS9118_MAC_ADDRH, checkMac);
	if (err || checkMac != mac)
		{
		__KTRACE_OPT(KHARDWARE, Kern::Printf("DEthernetSMCS9118Pdd::MacConfigure() -- Failed to set MAC Address"));
		DriverUnlock(irq);
		return;
		}
	
	for (TInt i=0; i<=5; i++)
		{
		iDefaultConfig.iEthAddress[i] = aConfig.iEthAddress[i];
		}

	__KTRACE_OPT(KHARDWARE, Kern::Printf("-- MAC address %2x.%2x.%2x.%2x.%2x.%2x",
				iDefaultConfig.iEthAddress[0], iDefaultConfig.iEthAddress[1],
				iDefaultConfig.iEthAddress[2], iDefaultConfig.iEthAddress[3],
				iDefaultConfig.iEthAddress[4], iDefaultConfig.iEthAddress[5]));

	DriverUnlock(irq);
	return;
	}


TInt DEthernetSMCS9118Pdd::Send(TBuf8<KMaxEthernetPacket+32>& aBuffer)
/**
 * Transmit data
 * @param aBuffer reference to the data to be sent
 * @return KErrNone if the data has been sent
 * assume iDriverLock not held
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    KPROFILE_PRINT("DEthernetSMCS9118Pdd::Send()");
    #endif

    // Always request for power
    iPowerHandler.RequestPower();
	TUint32* dataP = (TUint32 *)aBuffer.Ptr();
	TUint32 length = aBuffer.Length();

	TInt irq = DriverLock();
	// can it fit
	TUint32 status = Read32(SMCS9118_TX_FIFO_INF);
	if ((status & SMCS9118_TX_SPACE_MASK) < length)
		{
		#if defined(INSTR) && defined(KTRACE_SYNCH)
		KPROFILE_PRINT("eth -- Error... Send KErrSMCS9118TxOutOfMenory");
		#endif
		DriverUnlock(irq);
		return KErrTxOutOfMemory;
		}

	status = (length & 0x7ff) | SMCS9118_TX_FIRSTSEG | SMCS9118_TX_LASTSEG;
	Write32(SMCS9118_TX_DATA_FIFO, status);
	status = (length & 0x7ff) | SMCS9118_TX_PKT_TAG;
	Write32(SMCS9118_TX_DATA_FIFO, status);

	// calculate number of full words + remaining bytes
	
	TUint32 words = length >> 2;
	TUint32 bytes = length & 3;

	// write words
	while (words--)
		{
		Write32(SMCS9118_TX_DATA_FIFO, *dataP++);
		}
	
	// write bytes
	if (bytes)
		{
		TUint8 *dataBytes = (TUint8*) dataP;
		status = 0;
		switch (bytes)
			{
			case 3:
				status |= dataBytes[2] << 16;
				// fallthrough
			case 2:
				status |= dataBytes[1] << 8;
				// fallthrough
			case 1:
				status |= dataBytes[0];
			}
		Write32(SMCS9118_TX_DATA_FIFO, status);
		}

	// Clear interrupt
	TUint32 retries = SMCS9118_MAX_RETRIES;
	while(retries-- && !((status = Read32(SMCS9118_INT_STS)) & SMCS9118_INT_STS_TX))
		{
		ByteTestDelay(1); // delay
		}
	if (retries == 0 || (status & SMCS9118_INT_STS_TXE))
		{
		DriverUnlock(irq);
		return KErrGeneral;
		}
	Write32(SMCS9118_INT_STS, SMCS9118_INT_STS_TX);
	ByteTestDelay(2);

	DriverUnlock(irq);
	return KErrNone;
	}

TInt DEthernetSMCS9118Pdd::DiscardFrame()
/**
 * Discard the frame by fast forwarding over it
 *
 * Optional: if this doesn't
 * clear the frame, stop the receiver, dump the whole RX FIFO
 * and restart the receiver
 * assume iDriverLock held
 */
	{
	TUint32 retries = SMCS9118_MAX_RETRIES;

	// if it is 4 words or less then just read it
	TInt32 status = Read32(SMCS9118_RX_STATUS);
	TInt32 length = (status >> SMCS9118_RX_LEN_SHIFT) & SMCS9118_RX_LEN_MASK;
	TInt32 words = length >> 2;
	if (length & 3)
		{
		words++;
		}
	if (words <= 4)
		{
		while (words--)
			{
			status = Read32(SMCS9118_RX_DATA_FIFO);
			}
		status =  Read32(SMCS9118_RX_DATA_FIFO);
		return KErrNone;
		}

	// FFWD over the frame
	Write32(SMCS9118_RX_DP_CTL, SMCS9118_RX_DP_FFWD);
	ByteTestDelay(1);
	while((Read32(SMCS9118_RX_DP_CTL) & SMCS9118_RX_DP_FFWD) && --retries)
		{
		ByteTestDelay(1); // delay
		}
	if (retries != 0)
		{
		return KErrNone;
		}

#ifdef SMCS9118_DUMP_FIFO
	// stop the receiver
	TUint32 status;
	TInt32 err;
	err = ReadMac(SMCS9118_MAC_CR, status);
	if (err != KErrNone)
		{
		return err;
		}
	status &= ~SMCS9118_MAC_RXEN;
	err = WriteMac(SMCS9118_MAC_CR, status);
	if (err != KErrNone)
		{
		return err;
		}

	// wait for reciever to stop
	retries = SMCS9118_MAX_RETRIES;
	Write32(SMCS9118_RX_DP_CTL, SMCS9118_RX_DP_FFWD);
	ByteTestDelay(2);
	while((Read32(SMCS9118_INT_STS) & SMCS9118_RXSTOP_INT) && --retries)
		{
		ByteTestDelay(1); // delay
		}
	if (retries == 0)
		{
		return KErrGeneral;
		}

	// dump the whole FIFO
	retries = SMCS9118_MAX_RETRIES;
	Write32(SMCS9118_RX_CFG, SMCS9118_RX_DUMP);
	ByteTestDelay(1);
	while((Read32(SMCS9118_RX_CFG) & SMCS9118_RX_DUMP) && --retries)
		{
		ByteTestDelay(1); // delay
		}
	if (retries != 0)
		{
		// re-enable RX
		err = ReadMac(SMCS9118_MAC_CR, status);
		if (err != KErrNone)
			{
			return err;
			}
		status |= SMCS9118_MAC_RXEN;
		err = WriteMac(SMCS9118_MAC_CR, status);
		return err;
		}
#endif
	return KErrGeneral;
	}



TInt DEthernetSMCS9118Pdd::ReceiveFrame(TBuf8<KMaxEthernetPacket+32>& aBuffer, TBool aOkToUse)
/**
 * Retrieve data from the device - called by the RxDFC queued by the (variant's) ISR. 
 * Pull the received data out of the device and into the supplied buffer.
 * Need to be told if the buffer is OK to use as if it not we could dump
 * the waiting frame in order to clear the interrupt if necessory.
 * @param aBuffer Reference to the buffer to be used to store the data in
 * @param aOkToUse Bool to indicate if the buffer is usable
 * @return KErrNone if the buffer has been filled.
 */
	{
	TInt32 status;
	TUint32 length;
	
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetSMCS9118Pdd::ReceiveFrame()");
	#endif
	// Always request for power (Needs to be done incase of external wakeup event)
	iPowerHandler.RequestPower();

	TInt irq = DriverLock();
	// If no buffer available dump frame
	if (!aOkToUse)
		{
		#if defined(INSTR) && defined(KTRACE_SYNCH)
		KPROFILE_PRINT("SMCS9118: No Rx buffer available");
		#endif

		if ((status = DiscardFrame()) == KErrNone)
			{
			status = KErrGeneral;
			}
		DriverUnlock(irq);
		return status;
		}


	status = Read32(SMCS9118_RX_FIFO_INF);
	if(!(status & 0xffff))
		{
		#if defined(INSTR) && defined(KTRACE_SYNCH)
		KPROFILE_PRINT("SMCS9118: Empty Rx FIFO");
		#endif
		DriverUnlock(irq);
		return KErrGeneral;
		}

	// discard bad packets
	status = Read32(SMCS9118_RX_STATUS);
	length = (status >> SMCS9118_RX_LEN_SHIFT) & SMCS9118_RX_LEN_MASK;
	if (status & SMCS9118_RX_ES)
		{
		#if defined(INSTR) && defined(KTRACE_SYNCH)
		KPROFILE_PRINT("SMCS9118: Bad Rx Packet");
		#endif

		if ((status = DiscardFrame()) == (TUint32)KErrNone)
			{
			status = KErrGeneral;
			}
		DriverUnlock(irq);
		return status;
		}

	TUint32 words = length >> 2;
	TUint32 *dataP = (TUint32*) aBuffer.Ptr();

	if (length & 3)
		{
		words++;
		}

	while (words--)
		{
		*dataP++ = Read32(SMCS9118_RX_DATA_FIFO);
		}
	aBuffer.SetLength(length-4);

	DriverUnlock(irq);
	return KErrNone;
	}


const TInt KEthernetDfcThreadPriority = 24;
_LIT(KEthernetDfcThread,"EthernetDfcThread");


TInt DEthernetSMCS9118Pdd::DoCreate()
/**
 * Does the hard and soft reset of the lan card.
 * Puts the default configuration in iDefaultConfig member
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetSMCS9118Pdd::DoCreate()");
	#endif


	// Register the power handler with Symbian Power Framework
	iPowerHandler.Add();
	TInt r = iPowerHandler.SetEthernetPdd(this);
	#if defined(INSTR) && defined(KTRACE_SYNCH)
    __KTRACE_OPT(KPOWER, Kern::Printf("iPowerHandler.SetEthernetPdd() returned [%d]",r));
	#endif

	// Allocate a kernel thread to run the DFC 
	r = Kern::DynamicDfcQCreate(iDfcQ, KEthernetDfcThreadPriority, KEthernetDfcThread);

	if (r != KErrNone)
		{
		return r; 	
		}

#ifdef CPU_AFFINITY_ANY
	NKern::ThreadSetCpuAffinity((NThread*) iDfcQ->iThread, KCpuAffinityAny);
#endif
	
	iRxDfc.SetDfcQ(iDfcQ);

	TInt irq = DriverLock();
	iDefaultConfig.iEthSpeed  = KEthSpeed10BaseT;
	iDefaultConfig.iEthDuplex = KEthDuplexHalf;

	// detect if SMSC9118 card is available, ignore revision
	TUint32 id = Read32(SMCS9118_ID_REV) & SMCS9118_ID_MASK;
	if(id != SMCS9118_ID_VAL)
		{
		DriverUnlock(irq);
		return KErrHardwareNotAvailable;
		}

	TUint32 mac;
	r = ReadMac(SMCS9118_MAC_ADDRL, mac);
	if (r != KErrNone)
		{
		DriverUnlock(irq);
		return r;
		}
	
	iDefaultConfig.iEthAddress[0] = (TUint8)(mac);
	iDefaultConfig.iEthAddress[1] = (TUint8)(mac>>8);
	iDefaultConfig.iEthAddress[2] = (TUint8)(mac>>16);
	iDefaultConfig.iEthAddress[3] = (TUint8)(mac>>24);
	r = ReadMac(SMCS9118_MAC_ADDRH, mac);
	if (r != KErrNone)
		{
		DriverUnlock(irq);
		return r;
		}
	iDefaultConfig.iEthAddress[4] = (TUint8)(mac);
	iDefaultConfig.iEthAddress[5] = (TUint8)(mac>>8);

	// Serial number is the bottom 4 bytes of the MAC address
	TInt serialNum =  (iDefaultConfig.iEthAddress[2] << 24)
	                | (iDefaultConfig.iEthAddress[3] << 16)
	                | (iDefaultConfig.iEthAddress[4] <<  8)
	                | (iDefaultConfig.iEthAddress[5]      );

	// Push the serial numberinto the variant config so it can be retrieved via HAL
	NE1_TBVariant::SetSerialNumber(serialNum);

	iInterruptId = KEthernetInterruptId;

	DriverUnlock(irq);
	__KTRACE_OPT(KHARDWARE, Kern::Printf("-- MAC address %2x.%2x.%2x.%2x.%2x.%2x",
				iDefaultConfig.iEthAddress[0], iDefaultConfig.iEthAddress[1],
				iDefaultConfig.iEthAddress[2], iDefaultConfig.iEthAddress[3],
				iDefaultConfig.iEthAddress[4], iDefaultConfig.iEthAddress[5]));

	// Register ISR
	r = BindInterrupt(KEthernetInterruptId, Isr, this);
    if(r != KErrNone)
	{
		#if defined(INSTR) && defined(KTRACE_SYNCH)
		KPROFILE_PRINT("-- Error!!! Ethernet failed to bind interrupt.");
		#endif
		return r;
	}
	return r;
	}

void DEthernetSMCS9118Pdd::Sleep()
/**
 * Put the card into D1 sleep
 * assume iDriverLock not held
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetSMCS9118Pdd::Sleep()");
	#endif

	TUint32 status;
	TInt irq = DriverLock();
	status = Read32(SMCS9118_PMT_CTRL) | SMCS9118_PM_MODE_D1;
	Write32(SMCS9118_PMT_CTRL, status);
	DriverUnlock(irq);
	}

TInt DEthernetSMCS9118Pdd::Wakeup()
/**
 * Wake the card up
 * assume iDriverLock not held
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetSMCS9118Pdd::Wakeup()");
	#endif

	TUint32 status;

	// has card woken up yet ?
	
	TInt irq = DriverLock();
	TUint32 retry = SMCS9118_MAX_RETRIES;
	do
		{
		Write32(SMCS9118_BYTE_TEST, 0x12345678);
		status = Read32(SMCS9118_PMT_CTRL);
		if (status & SMCS9118_PMT_READY)
			{
			break;
			}
		Kern::NanoWait(ONE_MSEC);
		} while (--retry);

	if (retry == 0)
		{
		#if defined(INSTR) && defined(KTRACE_SYNCH)
		KPROFILE_PRINT("-- Error!!! Problem in Wakeup of SMCS9118 card.");
		#endif
		DriverUnlock(irq);
		return KErrGeneral;
		}
	DriverUnlock(irq);
	return KErrNone;
	}

TInt DEthernetSMCS9118Pdd::CardSoftReset()
/**
 * Does the soft reset of the lan card
 * assume iDriverLock not held
 */
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetSMCS9118Pdd::CardSoftReset()");
	#endif

	TInt32 status;

	// wake the card up
	status = Wakeup();
	if (status != KErrNone)
		{
		return status;
		}

	// do soft reset
	TInt irq = DriverLock();
	status = Read32(SMCS9118_HW_CFG);
	status |= SMCS9118_HW_CFG_SRST;
	Write32(SMCS9118_HW_CFG, status);

	ByteTestDelay(1);

	TUint32 retry = SMCS9118_MAX_RETRIES;
	do
		{
		status = Read32(SMCS9118_HW_CFG);
		if (!(status & SMCS9118_HW_CFG_SRST))
			{
			break;
			}
		Kern::NanoWait(ONE_MSEC);
		} while (--retry);

	if (retry == 0)
		{
		DriverUnlock(irq);
		#if defined(INSTR) && defined(KTRACE_SYNCH)
		KPROFILE_PRINT("-- Error!!! Problem in soft reset of SMCS9118 card.");
		#endif
		return KErrGeneral;
		}

	ByteTestDelay(1);
	DriverUnlock(irq);

	return KErrNone;
	}


/**
 * service the Isr
 */
void DEthernetSMCS9118Pdd::Isr(TAny* aPtr)
	{

	DEthernetSMCS9118Pdd &d=*(DEthernetSMCS9118Pdd*)aPtr;
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetSMCS9118Pdd::Isr");
	#endif

	// get interrupt status
	TUint32 status = d.Read32(SMCS9118_INT_STS);

	if (d.IsReady() && (status & SMCS9118_INT_STS_RSFL))
		{
		// We received an Rx Interrupt, so clear it
		// and queue on dfc
		d.Write32(SMCS9118_INT_EN, 0);
		d.iRxDfc.Add();
		}
	else
		{
		// spurious interrupt ?
		d.Write32(SMCS9118_INT_STS, status);
		}
	d.ClearInterrupt(d.iInterruptId);
	}

//
// Queued by the ISR on a receive interrupt.  Calls into LDD which calls back into 
// PDD.ReceiveFrame() to get the data and write it into the LDD managed FIFO. 
// Assume iDriverLock not held
// 

void DEthernetSMCS9118Pdd::ServiceRxDfc(TAny* aPtr)
	{
	#if defined(INSTR) && defined(KTRACE_SYNCH)
	KPROFILE_PRINT("DEthernetSMCS9118Pdd::ServiceRxDfc");
	#endif

	DEthernetSMCS9118Pdd &d=*(DEthernetSMCS9118Pdd*)aPtr;

	d.ReceiveIsr();

	TInt irq = d.DriverLock();
	// reset status
	d.Write32(SMCS9118_INT_STS, SMCS9118_INT_STS_RSFL);
	d.ByteTestDelay(2);

	// reenable interrupt
	d.Write32(SMCS9118_INT_EN, SMCS9118_INT_EN_RSFL);
	d.ByteTestDelay(1);
	d.DriverUnlock(irq);

	return;
	}

/**
 * Read a MAC register
 * assume iDriverLock held
 */
TInt32 DEthernetSMCS9118Pdd::ReadMac(TUint32 aReg, TUint32 &aVal)
	{
	TUint32 timeout;

	for(timeout = 0; timeout < SMCS9118_MAC_TIMEOUT; timeout ++)
		{
		if(Read32(SMCS9118_MAC_CSR_CMD) & SMCS9118_MAC_CSR_BUSY)
			{
			Kern::NanoWait(ONE_MSEC);
			}
		}

	TUint32 cmd = 0;

	Write32(SMCS9118_MAC_CSR_CMD, cmd);
	ByteTestDelay(1);

	cmd = (aReg & 0xff) | SMCS9118_MAC_CSR_BUSY | SMCS9118_MAC_CSR_READ;

	Write32(SMCS9118_MAC_CSR_CMD, cmd);
	ByteTestDelay(1);

	for(timeout = 0; timeout < SMCS9118_MAC_TIMEOUT; timeout ++)
		{
		if(Read32(SMCS9118_MAC_CSR_CMD) & SMCS9118_MAC_CSR_BUSY)
			{
			Kern::NanoWait(ONE_MSEC);
			}
		else
			{
			aVal = Read32(SMCS9118_MAC_CSR_DATA);
			return KErrNone;
			}
		}
	return KErrTimedOut;
	}

/**
 * Write a MAC register
 * assume iDriverLock held
 */
TInt32 DEthernetSMCS9118Pdd::WriteMac(TUint32 aReg, TUint32 aVal)
	{
	if (Read32(SMCS9118_MAC_CSR_CMD) & SMCS9118_MAC_CSR_BUSY)
		{
		return KErrTimedOut;
		}

	TUint32 cmd = 0;
	TUint32 timeout;

	Write32(SMCS9118_MAC_CSR_CMD, cmd);
	ByteTestDelay(1);

	cmd = (aReg & 0xff) | SMCS9118_MAC_CSR_BUSY;

	Write32(SMCS9118_MAC_CSR_DATA, aVal);
	ByteTestDelay(1);
	Write32(SMCS9118_MAC_CSR_CMD, cmd);
	ByteTestDelay(1);

	for (timeout = 0; timeout < SMCS9118_MAC_TIMEOUT; timeout ++)
		{
		if(Read32(SMCS9118_MAC_CSR_CMD) & SMCS9118_MAC_CSR_BUSY)
			{
			Kern::NanoWait(ONE_MSEC);
			}
		else
			{
			return KErrNone;
			}
		}
	return KErrTimedOut;
	}

/**
 * Read a PHY register
 * assume iDriverLock held
 */
TInt32 DEthernetSMCS9118Pdd::ReadPhy(TUint32 aReg, TUint32 &aValue)
	{
	TUint32 cmd;
	TUint32 timeout;
	TInt32 err;

	// bail out if busy
	err = ReadMac(SMCS9118_MAC_MII_ACC, aValue);
	if (err != KErrNone)
		{
		return err;
		}
		
	if (aValue & SMCS9118_MII_BUSY)
		{
		return KErrTimedOut;
		}

	cmd = SMCS9118_PHY_ADDR | (aReg << 6) | SMCS9118_MII_BUSY;

	err = WriteMac(SMCS9118_MAC_MII_ACC, cmd);
	if (err != KErrNone)
		{
		return err;
		}

	for(timeout = 0; timeout < SMCS9118_MAC_TIMEOUT; timeout++)
		{
		err = ReadMac(SMCS9118_MAC_MII_ACC, aValue);
		if (err != KErrNone)
			{
			return err;
			}
		
		if (!(aValue & SMCS9118_MII_BUSY))
			{
			err = ReadMac(SMCS9118_MAC_MII_DATA, aValue);
			return err;
			}
		}
	return KErrTimedOut;
	}

/**
 * Write a PHY register
 * assume iDriverLock held
 */
TInt32 DEthernetSMCS9118Pdd::WritePhy(TUint32 aReg, TUint32 aVal)
	{
	TUint32 cmd;
	TUint32 timeout;
	TUint32 status;
	TInt32	err;

	// bail out if busy
	err = ReadMac(SMCS9118_MAC_MII_ACC, status);
	if (err != KErrNone)
		{
		return err;
		}
		
	if (status & SMCS9118_MII_BUSY)
		{
		return KErrTimedOut;
		}

	cmd = SMCS9118_PHY_ADDR | (aReg << 6) | SMCS9118_MII_WRITE | SMCS9118_MII_BUSY;

	err = WriteMac(SMCS9118_MAC_MII_DATA, aVal & 0xffff);
	if (err != KErrNone)
			{
			return err;
			}
	err = WriteMac(SMCS9118_MAC_MII_ACC, cmd);
	if (err != KErrNone)
			{
			return err;
			}

	for(timeout = 0; timeout < SMCS9118_MAC_TIMEOUT; timeout++)
		{
		err = ReadMac(SMCS9118_MAC_MII_ACC, status);
		if (err != KErrNone)
			{
			return err;
			}
		if(!(status & SMCS9118_MII_BUSY))
			{
			return KErrNone;
			}
		}
	return KErrTimedOut;
	}
// PDD entry point
DECLARE_STANDARD_PDD()
	{
	return new DEthernetPddFactory;
	}
