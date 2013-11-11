/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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



#include <drivers/iic.h>
#include <drivers/iic_channel.h>
#include <assp/naviengine/naviengine_priv.h>
#include "csi_psl.h"
#include "csi_slave.h"

#include "hcrconfig.h"
#include "hcrconfig_csi.h"

// the timeout period to wait for a response from the client.
// We can't predict the frequency, at which the Master will be sending us data,
// it can be from 130kHz (which is 16,25bytes/ms) up to 16,67MHz(2083 bytes/ms).
// Timeout is set for the slowest transfer - for the time that the 32 bytes Tx FIFO would be emptied
// (Rx FIFO filled), so 2ms.
const TInt KClientWaitTime = 2; // when debugging might set up to KMaxWaitTime

// set of interrupt flags used by the driver
const TUint32 KSlaveInterruptFlags =
			KHtCSIControlSSDnIE	 | // SS signal negative-edge interrupt enable
	        KHtCSIControlSSUpIE  | // SS signal positive-edge interrupt enable
	        KHtCSIControlTEndIE  |
	        KHtCSIControlRxTrgEn |
	        KHtCSIControlRxTrgIE;


#ifdef __SMP__
static TSpinLock CsiSpinLock = TSpinLock(TSpinLock::EOrderGenericIrqLow2);
#endif

// this is call-back for iHwGuard timer. It is called in the ISR context
// if the iHwGuardTimer expires.
// It will change the iTransactionStatus to KErrTimedOut to allow exiting from the while-loop..
void DCsiChannelSlave::TimeoutCallback(TAny* aPtr)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelMaster::TimeoutCallback"));
	DCsiChannelSlave *a = (DCsiChannelSlave*) aPtr;
	a->iTransactionStatus = KErrTimedOut;
	}


// this is a static method to be called - when SS line is de-asserted.
// it disabled the interface and interrupts, and then it checks which flags
// need to be specified and then calls NotifyClient using it
void DCsiChannelSlave::NotifyClientEnd(DCsiChannelSlave* aPtr)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("NotifyClientEnd, iTrigger %x", aPtr->iTrigger));

	// disable interrupts and the interface..
	AsspRegister::Modify32(aPtr->iChannelBase + KHoCSIModeControl,
	                       KHtCSIModeEnable | KSlaveInterruptFlags, 0);

	// call NotifyClient with xAllBytes (as specified by the Operation)
	TInt flag = 0;
	if(aPtr->iTrigger & EReceive)
		{
		flag = ERxAllBytes;
		// if received less data, than the buffer size (i.e the Master de-asserted SS line
		// before we could fill the whole buffer) - this is the underrun situation
		if(aPtr->iRxDataEnd != aPtr->iRxData)
			{
			__KTRACE_OPT(KIIC, Kern::Printf("Rx Underrun"));
			flag |= ERxUnderrun;
			}
		}

	if(aPtr->iTrigger & ETransmit)
		{
		flag |= ETxAllBytes;
		// if not everything was transferred, i.e. Master de-asserted SS line
		// before we could transmit all the data - this is the overrun error..
		if(aPtr->iTxDataEnd != aPtr->iTxData)
			{
			__KTRACE_OPT(KIIC, Kern::Printf("Tx Overrun"));
			flag |= ETxOverrun;
			}
		}

	aPtr->NotifyClient(flag);
	}

// ISR Handler
void DCsiChannelSlave::CsiIsr(TAny* aPtr)
	{
	DCsiChannelSlave *a = (DCsiChannelSlave*) aPtr;
	TInt intState = 0;

	// read the interrupt flags - to see, what was causing the interrupt..
	TUint32 status = AsspRegister::Read32(a->iChannelBase + KHoCSIIntStatus);

	// SS signal negative-edge interrupt
	if (status & KHtCSIIntStatusSSDn)
		{
		// falling edge..
		TInt pin = AsspRegister::Read32(a->iChannelBase + KHoCSIControl) & KHtCSIControlSSMon;

		// falling edge.. and if pin active at LOW state - this is the beginning
		// of the transmission from the Master..
		if (a->iSSPinActiveMode == ESpiCSPinActiveLow)
			{
			intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
			if (!pin && !a->iInProgress)
				{
				a->iInProgress = ETrue;
				}
			__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);
			}
		else
		// falling edge.. and if pin active at HIGH state - this is the end
		// of the transmission from the Master..
			{
			TInt8 notify = EFalse;
			intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
			if (pin && a->iInProgress)
				{
				a->iInProgress = EFalse;
				notify = ETrue;
				}
			__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);

			if(notify)
				{
				a->NotifyClientEnd(a);
				return;
				}
			}


		// clear KHtCSIIntStatusSSDn interrupt..
		AsspRegister::Write32(a->iChannelBase + KHoCSIIntStatus, KHtCSIIntStatusSSDn);
		}

	// SS signal positive-edge interrupt
	if (status & KHtCSIIntStatusSSUp)
		{
		// rising edge..
		TInt pin = AsspRegister::Read32(a->iChannelBase + KHoCSIControl) & KHtCSIControlSSMon;

		// rising edge.. and if pin active at HIGH state - this is the beginning
		// of the transmission from the Master..
		if (a->iSSPinActiveMode == ESpiCSPinActiveHigh)
			{
			intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
			if (pin && !a->iInProgress)
				{
				a->iInProgress = ETrue;
				}
			__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);
			}
		else
		// rising edge.. and if pin active at LOW state - this is the END
		// of the transmission from the Master..
			{
			TInt8 notify = EFalse;

			intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
			if(pin && a->iInProgress)
				{
				a->iInProgress = EFalse;
				notify = ETrue;
				}
			__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);

			if(notify)
				{
				a->NotifyClientEnd(a);
				return;
				}
			}

		// clear KHtCSIIntStatusSSUp interrupt..
		AsspRegister::Write32(a->iChannelBase + KHoCSIIntStatus, KHtCSIIntStatusSSUp);
		}

	TInt trigger = 0;
	if (status & (KHtCSIIntStatusTEnd | KHtCSIIntStatusRxTrgIE))
		{
		intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
		trigger = a->iTrigger;
		__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);
		}

	// process TEND end interrupts
	// this ISR happens every time ONE unit has been transfered..
	if (status & KHtCSIIntStatusTEnd)
		{
		// clear TxEnd interrupt..
		AsspRegister::Write32(a->iChannelBase + KHoCSIIntStatus, KHtCSIIntStatusTEnd);

		if(trigger & ETransmit)
			{
			if(a->iTxData == a->iTxDataEnd)
				{
				// if we've moved all the data to the FIFO..
				// notify the PIL with ETxAllBytes | ETxUnderrun;
				// (so that - if the Client set ERxUnderrun - he could provide more data to be sent..
				AsspRegister::Modify32(a->iChannelBase + KHoCSIControl, KHtCSIControlTEndIE, 0);
				a->NotifyClient(ETxAllBytes | ETxUnderrun);
				}
			else
				{
				// if tere's more data to be sent - copy next item to the FIFO window register.
				TUint16 val;

				// in 16bit mode we need to read MSB first..
				if(a->iWordSize > 1)
					{
					val = (*a->iTxData) >> 8; // MSB shifted down..
					val |= *(a->iTxData + 1) & 0xff; // LSB..
					}
				else
					{
					val = *a->iTxData;
					}

				// copy the data item to the FIFO window register
				AsspRegister::Write32(a->iChannelBase + KHoCSIOFifo, val);

				// increment the pointer..
				a->iTxData += a->iWordSize;
				}
			}
		} //end of TXEnd processing

	// process receive threshold interrupt
	if (status & KHtCSIIntStatusRxTrgIE)
		{
		// read data from the FIFO ..
		if(trigger & EReceive)
			{
			while(AsspRegister::Read32(a->iChannelBase + KHoCSIIFifoL))
				{
				// if there's still some place in the buffer - put it into buffer..
				if((a->iRxDataEnd - a->iRxData) >= a->iWordSize)
					{
					// copy data from the FIFO if tere's more space in the buffer
					TUint16 val = AsspRegister::Read32(a->iChannelBase + KHoCSIIFifo);

					// we're big-endian.. (AB)..
					// so in 16bit mode we need to read MSB first..
					if(a->iWordSize > 1)
						{
						*a->iRxData = val >> 8; // MSB shifted down..
						*(a->iRxData + 1) = val & 0xff; // LSB..
						}
					else
						{
						*a->iRxData = val;
						}

					// increment the pointer..
					a->iRxData += a->iWordSize;
					}
				else
					{
					// overrun, i.e Slave has sent more data than expected by the client
					//__KTRACE_OPT(KIIC, Kern::Printf("REND, ERxOverrun"));
					AsspRegister::Modify32(a->iChannelBase + KHoCSIControl, KHtCSIControlRxTrgIE, 0);
					a->NotifyClient(ERxAllBytes | ERxOverrun);
					break;
					}
				}
			}
		else
			{
			// or drop the data, writing 0 to the FIFOL register
			AsspRegister::Write32(a->iChannelBase + KHoCSIIFifoL, 0);
			}

		// Clear the  RxThreshold interrupt
		AsspRegister::Write32(a->iChannelBase + KHoCSIIntStatus, KHtCSIIntStatusRxTrgIE);

		} // end of reception processing..
	}

// overloaded constructor
DCsiChannelSlave::DCsiChannelSlave(TInt aChannelNumber,
	const DIicBusChannel::TBusType aBusType,
	const DIicBusChannel::TChannelDuplex aChanDuplex) :
	DIicBusChannelSlave(aBusType, aChanDuplex, 0),
	iHwGuardTimer(TimeoutCallback, this)
	{
    iHwTimeoutValue = -1;
	iChannelNumber = aChannelNumber;
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelSlave::DCsiChannelSlave, iChannelNumber = %d\n", iChannelNumber));
	}

// 2nd stage object construction
TInt DCsiChannelSlave::DoCreate()
	{
	__KTRACE_OPT(KIIC, Kern::Printf("\nDCsiChannelSlave::DoCreate, ch: %d \n", iChannelNumber));

	TUint32 channelBase;

	HCR::TSettingId channelBaseId;
	channelBaseId.iCat = KHcrCat_MHA_HWBASE;

	TInt r = KErrNone;
	switch (iChannelNumber)
		{
		case 0:
			channelBaseId.iKey = KHcrKey_HwBase_CSI0;
			break;
		case 1:
			channelBaseId.iKey = KHcrKey_HwBase_CSI1;
			break;
		default:
			__KTRACE_OPT(KIIC, Kern::Printf("Wrong ChannelNumber specified (%d)", iChannelNumber));
			return KErrArgument;
		}

	r = HCR::GetUInt(channelBaseId, channelBase);
	iChannelBase = channelBase;

	if(r != KErrNone)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Failed to read HCR setting for category = %d and key = %d\n", channelBaseId.iCat, channelBaseId.iKey));
		return r;
		}

	//Read the timeout value from HCR
	if(iHwTimeoutValue == -1) 
        {
        HCR::TSettingId settingId;
        // csiTimeout values was not yet read from HCR; read it
        settingId.iCat = KHcrCat_HWServ_CSI;
        settingId.iKey = KHcrKey_CSI_Timeout;
    
        TInt r = HCR::GetInt(settingId, iHwTimeoutValue);
        if(r != KErrNone) 
            {
            __KTRACE_OPT(KIIC, Kern::Printf("Failed to read HCR setting for category = %d and key = %d\n", settingId.iCat, settingId.iKey));
            return r;
            }
        }
	
	// PIL Base class initialization.
	r = Init();

	// set the unique Slave's iChannelId
	// THis, along with the instance count of opened Slave channels - will be returned to the client
 	// as he captures the Channel (in the referenced aChannelId making an unique ID) - and if the capture is
	// successful he refers to this channel using the returned ChannelId.

	// For now, let's just use the combination of iChannelNumber/iChannelBase(register address))
	// This might be later replaced by the call into ConfRep to obtain the iChannelId for the PSL.
	iChannelId = 0xffff & (iChannelNumber + iChannelBase);

	return r;
	}

// static method used to construct the DCsiChannelSlave object.
#ifdef STANDALONE_CHANNEL
EXPORT_C
#endif
DCsiChannelSlave* DCsiChannelSlave::New(TInt aChannelNumber, const TBusType aBusType, const TChannelDuplex aChanDuplex)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelSlave::NewL(): aChannelNumber = %d, BusType =%d", aChannelNumber, aBusType));
	DCsiChannelSlave *pChan = new DCsiChannelSlave(aChannelNumber, aBusType, aChanDuplex);

	TInt r = KErrNoMemory;
	if (pChan)
		{
		r = pChan->DoCreate();
		}
	if (r != KErrNone)
		{
		delete pChan;
		pChan = NULL;
		}

	return pChan;
	}

// Validates the various Fields in the transaction header
// THis is a pure virtual.. which.. is never called by the PIL?..
TInt DCsiChannelSlave::CheckHdr(TDes8* aHdrBuff)
	{
	TInt r = KErrNone;

	if(!aHdrBuff)
		{
		r = KErrArgument;
		}
	else
		{
		TConfigSpiBufV01* headerBuf = (TConfigSpiBufV01*) aHdrBuff;
		TConfigSpiV01 &spiHeader = (*headerBuf)();

		if(spiHeader.iTransactionWaitCycles > 15) // (can be 0 - 15)
			{
			__KTRACE_OPT(KIIC, Kern::Printf("iTransactionWaitCycles not supported"));
			r = KErrNotSupported;
			}
		else
			{
			if(spiHeader.iWordWidth != ESpiWordWidth_8 &&
			spiHeader.iWordWidth != ESpiWordWidth_16)
				{
				__KTRACE_OPT(KIIC, Kern::Printf("iWordWidth not supported"));
				r = KErrNotSupported;
				}
			}
		}
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelSlave::CheckHdr() r %d", r));

	return r;
	}

void DCsiChannelSlave::ProcessData(TInt aTrigger, TIicBusSlaveCallback* aCb)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelSlave::ProcessData(), trigger: %x, in progress %d", aTrigger, iInProgress));
	TInt intState;

	// if NotifyClient was called due to SS line de-assertion..
	TInt inProgress;
	intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
	inProgress = iInProgress;
	__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);

	if(!inProgress &&
	   (aTrigger & (ERxAllBytes | ETxAllBytes)))
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Finished, cleaning.."));
		// we migh be still receiving or transmitting data, so must wait until the end of the transmission
		// start the guard timer, which - in case of the following while got stucked - will
		// unblock this dfc by changing iTransactionStatus
		iTransactionStatus = KErrNone;

		iHwGuardTimer.OneShot(NKern::TimerTicks(iHwTimeoutValue));

		// active wait until the transfer has finished
		while((iTransactionStatus == KErrNone) &&
		      (AsspRegister::Read32(iChannelBase + KHoCSIModeControl) & KHtCSIModeTransferState));

		// clear CSIE bit..
		AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeEnable, 0);

		// check if the the guard timer or transaction timer hasn't expired..
		if(iTransactionStatus != KErrNone)
			{
			__KTRACE_OPT(KIIC, Kern::Printf("CsiChannelMaster::TransferEndDfc(): Transaction timed-out"));
			return;
			}
		else
			{
			iHwGuardTimer.Cancel();
			}

		// clear internal flags
		intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
		iTrigger = 0;
		__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);
		}

	// if the transaction was finished
	// NotifyClient() with ERxAllBytes (and)or  ETxAllBytes called, when SS pin is de-asserted
	// this indicated end of the transmission. Check buffer boundaries, as if Rx or Tx buffers
	// provided by the client were bigger - this indicates ERxUnderrun or ETxOverrun - accordingly
	// Rx..
	if(aTrigger & ERxAllBytes)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Rx Buf:    %x", iRxData));
		__KTRACE_OPT(KIIC, Kern::Printf("Rx BufeND: %x", iRxDataEnd));

		__KTRACE_OPT(KIIC, Kern::Printf("\n\nTxFifo level %d", AsspRegister::Read32(iChannelBase + KHoCSIOFifoL)));
		__KTRACE_OPT(KIIC, Kern::Printf("RxFifo level %d\n\n", AsspRegister::Read32(iChannelBase + KHoCSIIFifoL)));

		// clear the internal EReceive flag..
		intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
		iTrigger &= ~EReceive;
		__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);

		// update the buffer information in the Callback object..
		aCb->SetRxWords(iNumRxWords - ((iRxDataEnd - iRxData) / iWordSize));
		}

	// Tx...
	if(aTrigger & ETxAllBytes)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Tx Buf:    %x", iTxData));
		__KTRACE_OPT(KIIC, Kern::Printf("Tx BufeND: %x", iTxDataEnd));

		// set the callback's TxWords value
		__KTRACE_OPT(KIIC, Kern::Printf("aCb->SetTxWords %d", iNumTxWords - ((iTxDataEnd - iTxData) / iWordSize)));

		// clear the internal ETransmit flag..
		intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
		iTrigger &= ~ETransmit;
		__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);

		// update the buffer information in the Callback object..
		aCb->SetTxWords(iNumTxWords - ((iTxDataEnd - iTxData) / iWordSize));
		}

	if(aTrigger & EGeneralBusError)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("BusError.."));
		// clear CSIE bit..and disable HW interrupts..
		AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeEnable | KSlaveInterruptFlags, 0);

		// clear internal flags
		intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
		iTrigger = 0;
		__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);
		}

	// set the callback's trigger..
	aCb->SetTrigger(aTrigger | aCb->GetTrigger());
	}

// Initializes the hardware with the data provided in the transaction and slave-address field
TInt DCsiChannelSlave::ConfigureInterface()
	{
	__KTRACE_OPT(KIIC, Kern::Printf("ConfigureInterface()"));

	HCR::TSettingId settingId;
	TConfigSpiBufV01* aBuf = (TConfigSpiBufV01*) iConfigHeader;
	TConfigSpiV01 &spiHeader = (*aBuf)();

	// CSI initialization procedure:
	// 1. clear CISE bit..
	AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeEnable, 0);

	// 2. wait until CIS_MODE.bit0 (CSOT) changes to 0
	// start the GuardTimer before while-loop..
	iTransactionStatus = KErrNone;

	iHwGuardTimer.OneShot(NKern::TimerTicks(iHwTimeoutValue));

	while((iTransactionStatus == KErrNone) &&
	       AsspRegister::Read32(iChannelBase + KHoCSIModeControl) & KHtCSIModeTransferState);

	// check if the the guard timer or transaction timer hasn't expired..
	if(iTransactionStatus != KErrNone)
		{
		return KErrGeneral;
		}
	else
		{
		iHwGuardTimer.Cancel();
		}

	// 3. set CSIRST bit..
	AsspRegister::Modify32(iChannelBase + KHoCSIControl, 0, KHtCSIControlCSIRst);

	// 4. set KHoCSIClockSelect register..
	TUint32 val = 0;

	// CKP - ClocK Polarity (bit 4) 0: initial value is high, 1: initial val is low
	// DAP - Clock phase select (bit 3) 0: 180 degree delay, 1: 0 degree delay
	// @** don't use DAP=1 when iClock set to KHCSIClockValPCLKdiv4 (HW bug..see SoC errata)
	switch (spiHeader.iClkMode)
		{
		case ESpiPolarityLowRisingEdge: // Active high, odd edges
			val = KHtCSIClockSelectCKP; // CKP 1, DAP 0
			break;
		case ESpiPolarityLowFallingEdge: // Active high, even edges
			val = KHtCSIClockSelectCKP | // CKP 1, DAP 1
			      KHtCSIClockSelectDAP;
			break;

		case ESpiPolarityHighFallingEdge: // Active low,  odd edges
			val = 0; // CKP 0, DAP 0
			break;

		case ESpiPolarityHighRisingEdge: // Active low,  even edges
			val = KHtCSIClockSelectDAP; // CKP 0, DAP 1
			break;
		default:
			break; // there's no default..no other value can be specified as it's an enum ;)
		}

	// we are a Slave - so we use only one setting for the clk speed, ignoring the header
	val |= KHCSIClockValSlave;

	// Set the SS pin configuration..
	val |= KHtCSIClockSelectSSE; // SS pin enable

	if (spiHeader.iSSPinActiveMode == ESpiCSPinActiveHigh)
		{
		val |= KHtCSIClockSelectSSPol; //  1: SS pin high active
		}

	// store iSSPinActiveMode internaly - it will be used in interrupts.
	iSSPinActiveMode = spiHeader.iSSPinActiveMode;

	// set transaction wait time..
	val |= (0xf & spiHeader.iTransactionWaitCycles) << KHsCSIModeTWait;

	// and finally update the register
	AsspRegister::Write32(iChannelBase + KHoCSIClockSelect, val);

	// 5. clear KHtCSIControlCSIRst bit..
	AsspRegister::Modify32(iChannelBase + KHoCSIControl, KHtCSIControlCSIRst, 0);

	// 6. Set Mode Control register:
	// Transmission and reception mode
	val = KHtCSIModeTrEnable;

	// Select transmit data length (8/16 bits)
	if (spiHeader.iWordWidth == ESpiWordWidth_16)
		{
		iWordSize = 2;
		val |= KHtCSIModeDataLen;
		}
	else
		{
		iWordSize = 1;
		}

	// Select Transfer direction (if set-LSB first)
	if (spiHeader.iBitOrder == ELsbFirst)
		{
		val |= KHtCSIModeTransferDir;
		}

	// update the register
	AsspRegister::Write32(iChannelBase + KHoCSIModeControl, val);

	// 7. Set FIFO trigger levels
	TUint8 csiFifoRxTrigerLvl; // set RxTrigger level
	
	settingId.iCat = KHcrCat_HWServ_CSI;
	settingId.iKey = KHcrKey_CSI_FifoRxTrigerLvl;
	TInt r = HCR::GetUInt(settingId, csiFifoRxTrigerLvl);
	if(r != KErrNone) 
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Failed to read HCR setting for category = %d and key = %d\n", settingId.iCat, settingId.iKey));
		return r;
		}
	AsspRegister::Write32(iChannelBase + KHoCSIFifoTrgLvl,
	                      (csiFifoRxTrigerLvl << KHsCSIRxFifoTrgLvl));

	// 8. Clear all interrupts
	AsspRegister::Write32(iChannelBase + KHoCSIIntStatus, KInterruptsAll);

	// 9. Set RxTrig permission and enable TEnd and RxTrg interrupts
	AsspRegister::Write32(iChannelBase + KHoCSIControl, KSlaveInterruptFlags);

	// the timeout period to wait for a response from the client.
	SetClientWaitTime(KClientWaitTime);
	// if the KIIC debug trace flag is set, we need to increase the transaction timeout,
	// so that debug traces don't cause timer expiration. This call will overwrite
	// previously set value (SetClientWaitTime) and default value set by the PIL (SetMasterWaitTime)
	__KTRACE_OPT(KIIC, SetClientWaitTime(KMaxWaitTime));
	__KTRACE_OPT(KIIC, SetMasterWaitTime(KMaxWaitTime));


	settingId.iCat = KHcrCat_MHA_Interrupt;
	settingId.iKey = iChannelNumber == 0 ? KHcrKey_Interrupt_CSI0 : KHcrKey_Interrupt_CSI1;

	TInt32 interruptId;
	r = HCR::GetInt(settingId, interruptId);
	if(r != KErrNone)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Failed to read HCR setting for category = %d and key = %d\n", settingId.iCat, settingId.iKey));
		return r;
		}

	// Bind the ISR for the Slave
	iInterruptId = Interrupt::Bind(interruptId, DCsiChannelSlave::CsiIsr, this);

	// this returns interruptId or error code(err < 0)
	if (iInterruptId < KErrNone)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("ERROR: InterruptBind error.. %d", r));
		Kern::Printf("ERROR: InterruptBind error.. %d", r);
		r = iInterruptId;
		}

	return r;
	}

// This method starts data transfer - filling the Transmit FIFO and enabling the device (CSIE bit)
TInt DCsiChannelSlave::InitTransfer()
	{
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelSlave::InitTransfer()"));
	//__KTRACE_OPT(KIIC, Kern::Printf("Receiving %d, Transmitting %d", (iTrigger & EReceive)>>4, (iTrigger & ETransmit)>>3));

	TUint r = KErrNone;
	TInt intState;
	TInt trigger;

	intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
	if(!iInProgress)
		{
		// clean the FIFOs..
		AsspRegister::Write32(iChannelBase + KHoCSIOFifoL, 0);
		AsspRegister::Write32(iChannelBase + KHoCSIOFifoL, 0);

		// clear CISE bit..re-enable all HW interrupts..
		AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeEnable, KSlaveInterruptFlags);

		// Clear all interrupts
		AsspRegister::Write32(iChannelBase + KHoCSIIntStatus, KInterruptsAll);
		}
	__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);


	intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
	trigger = iTrigger;
	__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);

	// if we are transmitting - Add data to the FIFO..
	if(trigger & ETransmit)
		{
		// Set mode to transmission and reception (Set TRMD)
		AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, 0, KHtCSIModeTrEnable);

		// enable TEND interrupt (e.g. if this is called during the transmission as the result of
		// TxUnderrun event.
		AsspRegister::Modify32(iChannelBase + KHoCSIControl, 0, KHtCSIControlTEndIE);

		iWordSize = iTxGranularity >> 3;
		iTxData = iTxBuf + (iWordSize * iTxOffset);
		iTxDataEnd = iTxData + (iWordSize * iNumTxWords);

		__KTRACE_OPT(KIIC, Kern::Printf("Tx Buf:    %x", iTxData));
		__KTRACE_OPT(KIIC, Kern::Printf("Tx BufeND: %x", iTxDataEnd));

		// copy data to the FIFO..
		while(AsspRegister::Read32(iChannelBase + KHoCSIOFifoL) <= (KHwCSIFifoLMax - iWordSize) && // until FIFO not full..
		      iTxData != iTxDataEnd) // or whole data has been copied.
			{
			TUint16 val;

			// in 16bit mode we need to read MSB first..
			if(iWordSize > 1)
				{
				val = (*iTxData) << 8; // MSB shifted up..
				val |= *(iTxData + 1) & 0xff; // LSB..
				}
			else
				{
				val = *iTxData;
				}

			// write this value to the FIFO window register..
			AsspRegister::Write32(iChannelBase + KHoCSIOFifo, val);

			// increment the pointer.
			iTxData += iWordSize;
			}

		__KTRACE_OPT(KIIC, Kern::Printf("After adding:\n\rTx Buf:    %x", iTxData));
		__KTRACE_OPT(KIIC, Kern::Printf("Tx BufeND: %x", iTxDataEnd));
		}

	if(trigger & EReceive) // we are starting receive transfer only..
		{
		if(!(trigger & ETransmit))
			{
			// if only receiving - set it to reveive-only
			// Set mode to reception only (clear TRMD)
			AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeTrEnable, 0);
			}

		// enable RxTrg interrupt (e.g. if this is called during the transmission as the result
		// of RxOverrun event.
		AsspRegister::Modify32(iChannelBase + KHoCSIControl, 0, KHtCSIControlRxTrgIE);

		iWordSize = iRxGranularity >> 3;
		iRxData = iRxBuf + (iWordSize * iRxOffset);
		iRxDataEnd = iRxData + (iWordSize * iNumRxWords);

		__KTRACE_OPT(KIIC, Kern::Printf("Rx Buffer:  %x", iRxData));
		__KTRACE_OPT(KIIC, Kern::Printf("Rx BufreND: %x", iRxDataEnd));
		}

	// enable interrupts..
	Interrupt::Enable(iInterruptId);

	// enable transmission..this will trigger the HW to start the transmission..
	AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, 0, KHtCSIModeEnable);

	return r;
	}

// aOperation is a bit-mask made of TPslOperation
TInt DCsiChannelSlave::DoRequest(TInt aOperation)
	{
//	__KTRACE_OPT(KIIC, Kern::Printf("\n===>DCsiChannelSlave::DoRequest, Operation %x", aOperation));

	TInt r = KErrNone;
	TInt intState;

	if (aOperation & EAsyncConfigPwrUp)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("EAsyncConfigPwrUp"));
		// this is called when the client calls IicBus::CaptureChannel() asynchronously
		r = ConfigureInterface();
		ChanCaptureCallback(r);
		return r;
		}

	// PowerUp
	if (aOperation & ESyncConfigPwrUp)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("ESyncConfigPwrUp"));
		// this is called when the client calls IicBus::CaptureChannel()
		// - configure the channel with the configuration provided in the header.
		r = ConfigureInterface();
		// we can't continue in this case..
		if (r != KErrNone)
			{
			__KTRACE_OPT(KIIC, Kern::Printf("Coudln't configure the interface..r %d", r));
			return r;
			}
		}

	if (aOperation & ETransmit)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("ETransmit"));
		intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
		iTrigger |= ETransmit;
		__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);
		}

	if (aOperation & EReceive)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("EReceive"));
		intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
		iTrigger |= EReceive;
		__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);
		}

	if (aOperation & (EReceive | ETransmit))
		{
		r = InitTransfer();
		}

	if (aOperation & EAbort)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("EAbort"));
		intState = __SPIN_LOCK_IRQSAVE(CsiSpinLock);
		// disable CSI (clear CSIE bit), and interrupts
		if(!iInProgress)
			{
			AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeEnable | KSlaveInterruptFlags, 0);
			iInProgress = EFalse;
			iTrigger = 0;
			}
		__SPIN_UNLOCK_IRQRESTORE(CsiSpinLock, intState);
		Interrupt::Disable(iInterruptId);
		}

	if (aOperation & EPowerDown)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("EPowerDown"));

		// set CSI Rst bit
		AsspRegister::Write32(iChannelBase + KHoCSIModeControl, KHtCSIControlCSIRst);

		// disable and unbind the ISR,
		Interrupt::Disable(iInterruptId);
		Interrupt::Unbind(iInterruptId);
		}
	return r;
	}

