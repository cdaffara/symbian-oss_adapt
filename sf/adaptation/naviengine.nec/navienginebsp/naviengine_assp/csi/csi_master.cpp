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
#include <drivers/gpio.h>
#include "csi_psl.h"
#include "csi_master.h"

#include "hcrconfig.h"
#include "hcrconfig_csi.h"

// This method ensures, that all Timers have been canceled..
// interrupts disabled prior to completing the request.
// This method is used to complete request and notify PIL in various situations.
void DCsiChannelMaster::ExitComplete(TInt aErr, TBool aComplete /*= ETrue*/)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("ExitComplete, r %d, complete %d", aErr, aComplete));

	// make sure we've disabled ints and canceled dfcs/timers..
	// Disable interrupts for CSI
	Interrupt::Disable(iInterruptId);

	// cancel timers and DFCs..
	CancelTimeOut();
	iHwGuardTimer.Cancel();
	iTransferEndDfc.Cancel();

	// change state to EIdle..
	iState = EIdle;

	// complete the request..calling the PIL method
	if(aComplete)
		{
		CompleteRequest(aErr);
		}
	}

// this is call-back for iHwGuard timer. It is called in the ISR context
// if the iHwGuardTimer expires.
// It will change the iTransactionStatus to KErrTimedOut to allow exiting from the while-loop..
void DCsiChannelMaster::TimeoutCallback(TAny* aPtr)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelMaster::TimeoutCallback"));
	DCsiChannelMaster *a = (DCsiChannelMaster*) aPtr;
	a->iTransactionStatus = KErrTimedOut;
	}

// This method is called by the PIL in the case of timeout for the transaction expiration.
// The PIL will call CompleteRequest() after this function returns, so we need only to clean-up
// and de-assert the SS line.
TInt DCsiChannelMaster::HandleSlaveTimeout()
	{
	__KTRACE_OPT(KIIC, Kern::Printf("HandleSlaveTimeout"));

	// make sure, that CSIE bit is cleared (disable the interface)
	AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeEnable, 0);

	// stop the driver's operation..
	ExitComplete(KErrTimedOut, EFalse);

	// bring the CS signal pin back to inactive state..
	GPIO::SetOutputState(iSSPin, iSSPinActiveStateOff);

	return KErrTimedOut;
	}

//DFC for TransferComplete
void DCsiChannelMaster::TransferEndDfc(TAny* aPtr)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelMaster::TransferEndDfc"));
	DCsiChannelMaster *a = (DCsiChannelMaster*) aPtr;

	// we are still receiving the data, so must wait until the end of the transmission
	// start the guard timer, which - in case of the following while got stucked - will
	// unblock this dfc by changing iTransactionStatus
	a->iTransactionStatus = KErrNone;

	a->iHwGuardTimer.OneShot(NKern::TimerTicks(a->iHwTimeoutValue));

	// active wait until the transfer has finished
	while((AsspRegister::Read32(a->iChannelBase + KHoCSIModeControl) & KHtCSIModeTransferState) &&
	      (a->iTransactionStatus == KErrNone));

	// bring the CS signal pin back to inactive state, but only if this is not an extended transaction,
	// in which case we want to leave the bus alone so that the multiple transactions making up the
	// extended transaction become one big transaction as far as the bus is concerned
	if (!(a->iCurrTransaction->Flags() & KTransactionWithMultiTransc))
		{
		GPIO::SetOutputState(a->iSSPin, a->iSSPinActiveStateOff);
		}

	// clear CSIE bit..
	AsspRegister::Modify32(a->iChannelBase + KHoCSIModeControl, KHtCSIModeEnable, 0);

	// check if the the guard timer or transaction timer hasn't expired..
	if(a->iTransactionStatus != KErrNone)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("CsiChannelMaster::TransferEndDfc(): Transaction timed-out"));
		a->ExitComplete(a->iTransactionStatus); // report the error situation..
		return;
		}
	else
		{
		a->iHwGuardTimer.Cancel();
		}

	// drain the Rx FIFO buffer (there might be one item left)
	if(a->iOperation.iOp.iIsReceiving)
		{
		if(AsspRegister::Read32(a->iChannelBase + KHoCSIIFifoL) &&
		  (a->iRxDataEnd - a->iRxData >= a->iWordSize))
			{
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
		}
	// else - we don't care about the read data, it will be flushed before the next transfer..

	// check, if there are more transfers in this transaction
	if(a->iState == EBusy)
		{
		TInt err = a->ProcessNextTransfers();

		// if for any reason coudln't start next transfer-complete the transaction with err..
		if(err != KErrNone)
			{
			a->ExitComplete(err);
			}
		}
	}

// ISR Handler
void DCsiChannelMaster::CsiIsr(TAny* aPtr)
	{
	DCsiChannelMaster *a = (DCsiChannelMaster*) aPtr;

	// read the interrupt flags - to see, what was causing the interrupt..
	TUint32 status = AsspRegister::Read32(a->iChannelBase + KHoCSIIntStatus);

	// process TEND end interrupts
	// this ISR happens every time ONE unit has been transfered..
	if(status & KHtCSIIntStatusTEnd)
		{
		// clear TxEnd interrupt..
		AsspRegister::Write32(a->iChannelBase + KHoCSIIntStatus, KHtCSIIntStatusTEnd);

		if(a->iTxData == a->iTxDataEnd)
			{
			// if the Tx FIFO is empty (this was the last item transferred)
			// confirm that the transfer was completed successfully:
			// - set the transfer status as KErrNone
			// - disable interrupts..
			// - queue a DFC, which will finish the Tx-asserting/de-asserting CS line
			// and start another transfer in the transaction or complete transaction
			if(!AsspRegister::Read32(a->iChannelBase + KHoCSIOFifoL))
				{
				Interrupt::Disable(a->iInterruptId);
				a->iTransferEndDfc.Add();
				}
			}
		else
			{
			// if tere's more data to be sent - copy next item to the FIFO window register.
			// if we are in 'receive-only' mode - (e.g. simple read request) we're only
			// sending '0' (or other value defined as KValueSentOnRead) - to generate a clock for the slave.
			if(a->iOperation.iOp.iIsTransmitting)
				{
				// copy data to the FIFO..
				TUint16 val;

				// in 16bit mode we need to write two bytes as once MSB first..
				if(a->iWordSize > 1)
					{
					val = (*a->iTxData) << 8; // MSB shifted up..
					val |= *(a->iTxData + 1) & 0xff; // LSB..
					}
				else
					{
					val = *a->iTxData;
					}

				// write this value to the register..
				AsspRegister::Write32(a->iChannelBase + KHoCSIOFifo, val);

				// increment the pointer..
				a->iTxData += a->iWordSize;
				}
			}
		} //end of TXEnd processing

	// process receive threshold interrupt
	if(status & KHtCSIIntStatusRxTrgIE)
		{
		// read data from the FIFO ..
		if(a->iOperation.iOp.iIsReceiving)
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
					}
				else
					{
					// overrun, i.e Slave has sent more data than expected by the client
					// (e.g. too small buffer size for the transmission)
					a->iTransactionStatus = KErrOverflow;
					break;
					}

				// increment the pointer..
				a->iRxData += a->iWordSize;
				}
			}
		else
			{
			// or drop the data, writing 0 to the FIFOL register
			AsspRegister::Write32(a->iChannelBase + KHoCSIIFifoL, 0);
			}

		// if we are in the 'half-duplex' 'receive only' mode,
		// once the receive buffer is full we are to finish the transmission..
		if((a->iOperation.iValue == TCsiOperationType::EReceiveOnly) &&
		   (a->iRxDataEnd == a->iRxData))
			{
			Interrupt::Disable(a->iInterruptId);
			a->iTransferEndDfc.Add();
			}

		// Clear the  RxThreshold interrupt
		AsspRegister::Write32(a->iChannelBase + KHoCSIIntStatus, KHtCSIIntStatusRxTrgIE);

		} // end of reception processing..
	}

// constructor 1-st stage
DCsiChannelMaster::DCsiChannelMaster(TInt aChannelNumber, TBusType aBusType, TChannelDuplex aChanDuplex) :
	DIicBusChannelMaster(aBusType, aChanDuplex), // !!call overloaded base class constructor..
	iTransferEndDfc(TransferEndDfc, this, KCsiDfcPriority),
	iHwGuardTimer(TimeoutCallback, this)
	{
    iHwTimeoutValue = -1;
	iSSPin = 0;
	iChannelNumber = aChannelNumber; //Set the iChannelNumber of the Base Class
	iState = EIdle;
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelMaster::DCsiChannelMaster: iChannelNumber = %d", iChannelNumber));
	}

// 2nd stage construction..
TInt DCsiChannelMaster::DoCreate()
	{
	__KTRACE_OPT(KIIC, Kern::Printf("\nDCsiChannelMaster::DoCreate() ch: %d \n", iChannelNumber)); //__THREAD_AND_CPU;

	TInt32 interruptId;
	TUint32 channelBase;
	
	HCR::TSettingId settingId;
	settingId.iCat = KHcrCat_MHA_Interrupt;

	HCR::TSettingId channelBaseId;
	channelBaseId.iCat = KHcrCat_MHA_HWBASE;

	TInt r = KErrNone;
	switch(iChannelNumber)
		{
		case 0:
			settingId.iKey = KHcrKey_Interrupt_CSI0;
			channelBaseId.iKey = KHcrKey_HwBase_CSI0;
			break;
		case 1:
			settingId.iKey = KHcrKey_Interrupt_CSI1;
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

	r = HCR::GetInt(settingId, interruptId);
	if(r != KErrNone)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Failed to read HCR setting for category = %d and key = %d\n", settingId.iCat, settingId.iKey));
		return r;
		}
	
	//Read the timeout value from HCR
	//The value will not be changed during transaction, so it needs to be read only once from HCR
	if(iHwTimeoutValue == -1) 
        {
        // csiTimeout values was not yet read from HCR; read it
        HCR::TSettingId settingId;
        settingId.iCat = KHcrCat_HWServ_CSI;
        settingId.iKey = KHcrKey_CSI_Timeout;
        r = HCR::GetInt(settingId, iHwTimeoutValue);
        if(r != KErrNone) 
            {
            __KTRACE_OPT(KIIC, Kern::Printf("Failed to read HCR setting for category = %d and key = %d\n", settingId.iCat, settingId.iKey));
            return r;
            }
        }


	// Create kernel DFCQ (thread) for the driver..
	if(!iDfcQ)
		{
		TBuf8<KMaxName> threadName (KSpiThreadName);
		threadName.AppendNum(iChannelNumber);
		TDynamicDfcQue* dynamicDfcQ;
		r = Kern::DynamicDfcQCreate(dynamicDfcQ, KSpiThreadPriority, threadName);
		if(r != KErrNone)
			{
			__KTRACE_OPT(KIIC, Kern::Printf("DFC Queue creation failed, ch: %d, r = %d\n", iChannelNumber, r));
			return r;
			}
		iDfcQ = dynamicDfcQ;
		}
	
	// PIL Base class initialization. This must!! be called prior to SetDfcQ(iDfcQ) ..
	r = Init();
	if(r == KErrNone)
		{
		// set iDFCQ
		SetDfcQ(iDfcQ); // base class..
		iTransferEndDfc.SetDfcQ(iDfcQ); // transfer-end DFC
		
#ifdef CPU_AFFINITY_ANY
		NKern::ThreadSetCpuAffinity((NThread*)(iDfcQ->iThread), KCpuAffinityAny);
#endif

		// Bind the Interrupt.

		iInterruptId = Interrupt::Bind(interruptId, CsiIsr, this);

		// this returns interruptId or error code(err < 0)
		if(iInterruptId < KErrNone)
			{
			__KTRACE_OPT(KIIC, Kern::Printf("ERROR: InterruptBind error.. %d", r));
			r = iInterruptId;
			iInterruptId = 0;
			}
		}

	return r;
	}

// static method used to construct the DCsiChannelMaster object.
// Export the channel creating function for client use in controller-less mode
#ifdef STANDALONE_CHANNEL
EXPORT_C
#endif
DCsiChannelMaster* DCsiChannelMaster::New(TInt aChannelNumber, const TBusType aBusType, const TChannelDuplex aChanDuplex)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelMaster::NewL(): aChannelNumber = %d, BusType =%d", aChannelNumber, aBusType));
	DCsiChannelMaster *pChan = new DCsiChannelMaster(aChannelNumber, aBusType, aChanDuplex);

	TInt r = KErrNoMemory;

	if(pChan)
		{
		r = pChan->DoCreate();
		}
	if(r != KErrNone)
		{
		delete pChan;
		pChan = NULL;
		}
	return pChan;
	}

#ifdef STANDALONE_CHANNEL
DCsiChannelMaster::~DCsiChannelMaster()
	{
	// This destructor will only be called in controller-less mode, when iDfcQ is a TDynamicDfcQ,
	// In here, detroy the dfc queue and unbind the interrupt for next channel creation 	
	
	// stop the Hardware
	// clear CISE bit..(disables CSI)
	AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeEnable, 0);

	// set CSIRST bit..
	AsspRegister::Modify32(iChannelBase + KHoCSIControl, 0, KHtCSIControlCSIRst);

	if(iInterruptId)
	    {
	    Interrupt::Unbind(iInterruptId);
	    }
	if(iDfcQ)
		{
		((TDynamicDfcQue*)iDfcQ)->Destroy();
		}
	}
#endif

// this method compares if the previous transaction header and slave-select(chip-select) pin are different
// If configuration is the same - HW will not be re-initialized. It also makes a copy of the new header
// and new pin number used to configure the interface.
TBool DCsiChannelMaster::TransConfigDiffersFromPrev()
	{
	TConfigSpiBufV01* headerBuf = (TConfigSpiBufV01*) (GetTransactionHeader(iCurrTransaction));
	TConfigSpiV01 &spiHeader = (*headerBuf)();

	// the busId - has a SlaveAddress which in the case of SPI is a SlaveSelect pin (GPIO pin number)
	TUint32 busId = ((TIicBusTransaction*)iCurrTransaction)->GetBusId();

	// get the slave address
	TUint16 csPin;
	csPin = GET_SLAVE_ADDR(busId);

	// compare it to the previous configuration..
	if(csPin                            != iSSPin ||
	   spiHeader.iWordWidth             != iSpiHeader.iWordWidth ||
	   spiHeader.iClkSpeedHz            != iSpiHeader.iClkSpeedHz ||
	   spiHeader.iClkMode               != iSpiHeader.iClkMode ||
	   spiHeader.iTimeoutPeriod         != iSpiHeader.iTimeoutPeriod ||
	   spiHeader.iBitOrder              != iSpiHeader.iBitOrder ||
	   spiHeader.iTransactionWaitCycles != iSpiHeader.iTransactionWaitCycles ||
	   spiHeader.iSSPinActiveMode       != iSpiHeader.iSSPinActiveMode)
		{
		iSSPin = csPin; // copy CsPin number.
		iSpiHeader = spiHeader; // copy the new config params
#ifdef _DEBUG
		DumpConfiguration(spiHeader, iSSPin);
#endif
		return ETrue;
		}

	return EFalse;
	}

// Validates various fields in the transaction header
// this pure-virtual method is called by the PIL in the context of the
// client's thread - whenever he makes a call to QueueTransaction().
TInt DCsiChannelMaster::CheckHdr(TDes8* aHdrBuff)
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
			else
				{
				if(spiHeader.iClkSpeedHz != 130000  &&
				   spiHeader.iClkSpeedHz != 260000  &&
                   spiHeader.iClkSpeedHz != 521000  &&
                   spiHeader.iClkSpeedHz != 1040000 &&
                   spiHeader.iClkSpeedHz != 2080000 &&
                   spiHeader.iClkSpeedHz != 4170000 &&
                   spiHeader.iClkSpeedHz != 16670000)
					{
					__KTRACE_OPT(KIIC, Kern::Printf("iClock not supported"));
					r = KErrNotSupported;
					}
				}
			}
		}
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelMaster::CheckHdr() r %d", r));
	return r;
	}

// Initializes the hardware with the data provided in the transaction and slave-address field
TInt DCsiChannelMaster::ConfigureInterface()
	{
	__KTRACE_OPT(KIIC, Kern::Printf("ConfigureInterface()"));

	HCR::TSettingId settingId;

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
	switch(iSpiHeader.iClkMode)
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

	// set the clock..
	switch(iSpiHeader.iClkSpeedHz)
		{
		case 130000:
			val |= KHCSIClockValPCLKdiv512; //  1/512 PCLK (master mode)     130 kHz
			break;
		case 260000:
			val |= KHCSIClockValPCLKdiv256; //  1/256 PCLK (master mode)     260 kHz
			break;
		case 521000:
			val |= KHCSIClockValPCLKdiv128; //  1/128 PCLK (master mode)     521 kHz
			break;
		case 1040000:
			val |= KHCSIClockValPCLKdiv64; //  1/64 PCLK (master mode) 1.04 MHz
			break;
		case 2080000:
			val |= KHCSIClockValPCLKdiv32; //  1/32 PCLK (master mode) 2.08 MHz
			break;
		case 4170000:
			val |= KHCSIClockValPCLKdiv16; //  1/16 PCLK (master mode) 4.17 MHz
			break;
		case 16670000:
			if(val & KHtCSIClockSelectDAP) // see @**
				{
				__KTRACE_OPT(KIIC, Kern::Printf("Unsupported CLK/Clock mode"));
				return KErrArgument;
				}
			val |= KHCSIClockValPCLKdiv4; //  1/4 PCLK (master mode)   16.67 MHz
			break;
		default:
			return KErrNotSupported;
		}

	// set transaction wait time..
	val |= (0xf & iSpiHeader.iTransactionWaitCycles) << KHsCSIModeTWait;

	// and finally update the register
	AsspRegister::Write32(iChannelBase + KHoCSIClockSelect, val);

	// 5. clear KHtCSIControlCSIRst bit..
	AsspRegister::Modify32(iChannelBase + KHoCSIControl, KHtCSIControlCSIRst, 0);

	// 6. Set Mode Control register:
	// Transmission and reception mode
	val = KHtCSIModeTrEnable;

	// Select transmit data length (8/16 bits)
	if(iSpiHeader.iWordWidth == ESpiWordWidth_16)
		{
		iWordSize = 2;
		val |= KHtCSIModeDataLen;
		}
	else
		{
		iWordSize = 1;
		}

	// Select Transfer direction (if set-LSB first)
	if(iSpiHeader.iBitOrder == ELsbFirst)
		{
		val |= KHtCSIModeTransferDir;
		}

	// update the register
	AsspRegister::Write32(iChannelBase + KHoCSIModeControl, val);

	// 7. Set FIFO trigger levels
	TUint8 csiFifoRxTrigerLvl;
	
	settingId.iCat = KHcrCat_HWServ_CSI;
	settingId.iKey = KHcrKey_CSI_FifoRxTrigerLvl;
	
	TInt r = HCR::GetUInt(settingId, csiFifoRxTrigerLvl);
	if(r != KErrNone) 
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Failed to read HCR setting for category = %d and key = %d\n", settingId.iCat, settingId.iKey));
		return r;
		}
	AsspRegister::Write32(iChannelBase + KHoCSIFifoTrgLvl, (csiFifoRxTrigerLvl << KHsCSIRxFifoTrgLvl));

	// 8. Clear all interrupts
	AsspRegister::Write32(iChannelBase + KHoCSIIntStatus, KInterruptsAll);

	// 9. Set RxTrig permission and enable TEnd and RxTrg interrupts
	AsspRegister::Write32(iChannelBase + KHoCSIControl, KHtCSIControlRxTrgEn |
	                                                    KHtCSIControlTEndIE    |
	                                                    KHtCSIControlRxTrgIE);

	// 10. finally set CSIE bit
	AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, 0, KHtCSIModeEnable);

	// enable and configure GPIO pin - it is CS Pin used for the transmission..
	// if specified different (e.g. 0) - iSSPin will be ignored during the transmission..
	if(iSSPin < 1 || iSSPin > 32) // can be 1 - 32
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Wrong pin number specified()"));
		return KErrArgument;
		}
	else
		{
		GPIO::SetPinMode(iSSPin, GPIO::EEnabled);
		GPIO::SetPinDirection(iSSPin, GPIO::EOutput);
		GPIO::SetDebounceTime(iSSPin, 0);

		// and set active high bit (KtCSPinHigh)
		if(iSpiHeader.iSSPinActiveMode == ESpiCSPinActiveHigh)
			{
			iSSPinActiveStateOn = GPIO::EHigh;
			iSSPinActiveStateOff = GPIO::ELow;
			}
		else
			{
			iSSPinActiveStateOn = GPIO::ELow;
			iSSPinActiveStateOff = GPIO::EHigh;
			}
		}

	// clear KHtCSIModeEnable.. it will be set to trigger the transmission in DoTransfer()
	AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeEnable, 0);

	return KErrNone;
	}

// This method starts data transfer - filling the Transmit FIFO and enabling the device (CSIE bit)
TInt DCsiChannelMaster::DoTransfer(TInt8 *aBuff, TUint aNumOfBytes)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("\nDCsiChannelMaster::DoTransfer()"));
	__KTRACE_OPT(KIIC, Kern::Printf("Receiving %d, Transmitting %d", iOperation.iOp.iIsReceiving, iOperation.iOp.iIsTransmitting));

	if(aNumOfBytes == 0) // wanted to transfer an empty buffer??
		{
		return KErrArgument;
		}

	// store current Tx buffer addresses - to use them later in the ISR,
	// when re-filling the buffer if its level drops down to the threshold value..
	iTxData = aBuff;
	iTxDataEnd = (TInt8*) (aBuff + aNumOfBytes);
	__KTRACE_OPT(KIIC, Kern::Printf("Tx: Start: %x, End %x, bytes %d\n\n", iTxData, iTxDataEnd, aNumOfBytes));

	// ensure, we won't be sending until we've filled up the FIFO..
	AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeEnable, 0);

	// clean the FIFO..
	AsspRegister::Write32(iChannelBase + KHoCSIOFifoL, 0);

	// if we are transmitting - Add data to the FIFO..
	if(iOperation.iOp.iIsTransmitting)
		{
		// Set mode to transmission and reception (Set TRMD)
		AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, 0, KHtCSIModeTrEnable);

		while(AsspRegister::Read32(iChannelBase + KHoCSIOFifoL) < KHwCSIFifoLMax && // until FIFO not full..
		      iTxData != iTxDataEnd) // or whole data has been copied.
			{
			// copy data to the FIFO..
			TUint16 val;

			// in 16bit mode we need to write two bytes as once MSB first..
			if(iWordSize > 1)
				{
				val = (*iTxData) << 8; // MSB shifted up..
				val |= *(iTxData + 1) & 0xff; // LSB..
				}
			else
				{
				val = *iTxData;
				}

			// write this value to the register..
			AsspRegister::Write32(iChannelBase + KHoCSIOFifo, val);

			// increment the pointer.
			iTxData += iWordSize;
			}
		}
	else // we are starting receive transfer only..
		{
		// Set mode to reception only (clear TRMD)
		AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, KHtCSIModeTrEnable, 0);
		}

	// change the SS line status - start of the transmission. This should be set back to high - after
	// the transmission has been finished (iTransferEndDfc)
	GPIO::SetOutputState(iSSPin, iSSPinActiveStateOn);

	// enable interrupts..
	Interrupt::Enable(iInterruptId);

	// enable transmission..this will trigger the HW to start the transmission..
	AsspRegister::Modify32(iChannelBase + KHoCSIModeControl, 0, KHtCSIModeEnable);

	return KErrNone;
	}

// this method starts the given transfer..
TInt DCsiChannelMaster::StartTransfer(TIicBusTransfer* aTransferPtr, TUint8 aType)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelMaster::StartTransfer()"));

	TInt r = KErrNone;

	switch(aType)
		{
		case TIicBusTransfer::EMasterWrite:
			{
			__KTRACE_OPT(KIIC, Kern::Printf("Starting EMasterWrite, duplex=%d", iFullDTransfer));

			const TDes8* aBufPtr = GetTferBuffer(aTransferPtr);

			__KTRACE_OPT(KIIC, Kern::Printf("Length %d, iWordSize %d", aBufPtr->Length(), iWordSize));

			// set this flag - to indicate, that we'll be transmitting the data.
			// if this is set to 0 prior to calling to DoTransfer() - only '0'(or KValueSentOnRead) values will be
			// sent out of the interface (e.g. if only receiving from the slave - to generate the clock)
			iOperation.iOp.iIsTransmitting = ETrue;

			// initiate the transmission..
			r = DoTransfer((TInt8 *) aBufPtr->Ptr(), aBufPtr->Length());

			if(r != KErrNone)
				{
				__KTRACE_OPT(KIIC, Kern::Printf("Starting Write filed, r = %d", r));
				}
			break;
			}
		case TIicBusTransfer::EMasterRead:
			{
			__KTRACE_OPT(KIIC, Kern::Printf("Starting EMasterRead, duplex=%x", iFullDTransfer));

			// store the current address and ending address for Reception- to use them later in the ISR and DFC
			const TDes8* aBufPtr = GetTferBuffer(aTransferPtr);

			iRxData = (TInt8*) aBufPtr->Ptr();
			iRxDataEnd = (TInt8*) (iRxData + aBufPtr->Length());

			__KTRACE_OPT(KIIC, Kern::Printf("Rx: Start: %x, End %x, bytes %d", iRxData, iRxDataEnd, aBufPtr->Length()));

			// set the flag - that we're going to receive the data..
			iOperation.iOp.iIsReceiving = ETrue;

			// if this is half-duplex transfer.. (read-only)
			// we still need to transmit '0' to generate CLK and SS signals for the Slave
			if(!iFullDTransfer)
				{
				// make sure transmitting flag is cleared
				iOperation.iOp.iIsTransmitting = EFalse;

				// set pointers - as if we're transmitting the same amount of data..
				r = DoTransfer((TInt8 *) aBufPtr->Ptr(), aBufPtr->Length());
				}
			}
			break;
		default:
			{
			__KTRACE_OPT(KIIC, Kern::Printf("Unsupported TrasactionType %x", aType));
			r = KErrArgument;
			break;
			}
		}

	return r;
	}

// calling this method whenever a transfer has been finished - will process all transfers in the transaction
TInt DCsiChannelMaster::ProcessNextTransfers()
	{
	// transfers are queued in linked list.. so just go through that list and start them
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelMaster::ProcessNextTransfers(),BUSY=%d", iState));

	// clear flags..
	iOperation.iValue = TCsiOperationType::ENop;

	// in both cases - full-duplex - or simple half-duplex
	// we need to flush the buffers for the transmission:
	AsspRegister::Write32(iChannelBase + KHoCSIIFifoL, 0); // write 0 to FIFOL will drop data
	AsspRegister::Write32(iChannelBase + KHoCSIOFifoL, 0); // write 0 to FIFOL will drop data

	// if this is the first transfer in the transaction..(called from DoCreate)
	if(iState == EIdle)
		{
		// Get the pointer to half-duplex transfer object..
		iHalfDTransfer = GetTransHalfDuplexTferPtr(iCurrTransaction);

		// Get the pointer to full-duplex transfer object..
		iFullDTransfer = GetTransFullDuplexTferPtr(iCurrTransaction);

		// from now on - our state is EBusy.
		iState = EBusy;

		// kick-off the transaction timer..
		// it's default handler, run in ISR context will change the iTransactionStatus
		// and queue TranferEndDfc (in the case if it wasn't queued) - this DFC
		// will then finish transfer and report this error to the PIL.
		__ASSERT_DEBUG(iSpiHeader.iTimeoutPeriod > 0, Kern::Fault("NE1_TB SPI: timeout value not set,line: %d", __LINE__));
		__KTRACE_OPT(KIIC, Kern::Printf("Timeout for transaction %d", iSpiHeader.iTimeoutPeriod));
		iTransactionStatus = KErrNone;
		// Initiate the timer for the transaction. When it expires - PIL will call
		// HandleSlaveTimeout() - which will stop the HW operations..
		StartSlaveTimeOutTimer(iSpiHeader.iTimeoutPeriod);
		// When the KIIC trace flag is enabled, we need cancel the transaction timeout,
		// so that debug traces don't cause slave timer expiration.
		__KTRACE_OPT(KIIC, CancelTimeOut());

		}
	else
	// We continue with next transfer in the transaction..(Called from TransferEndDfc)
		{
		// Get the pointer the next half-duplex transfer object..
		iHalfDTransfer = GetTferNextTfer(iHalfDTransfer);

		// Get the pointer to the next half-duplex transfer object..
		if(iFullDTransfer)
			{
			iFullDTransfer = GetTferNextTfer(iFullDTransfer);
			}
		}

	// all of the transfers were completed, just notify the PIL and return.
	// (if either Rx or Tx has not finished properly ExitComplete() would have been called
	// from TransferEndDfc if there was an error during the transfer)
	TInt r = KErrNone;
	if(!iFullDTransfer && !iHalfDTransfer)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("All transfers completed successfully"));
		// complete the request..
		ExitComplete(KErrNone);
		}
	else
		{
		// start transfers..
		// if this is full-duplex transfer - we need to Start EMasterRead transfer first
		// this is because buffer pointers and flags for this transfer type must be initialized
		// prior to starting EMasterWrite (which always triggers the transmission start)
		TInt8 hDTrType = (TInt8) GetTferType(iHalfDTransfer);

		if(iFullDTransfer)
			{
			// if iHalfDTransfer is EMasterRead - start it first..
			if(hDTrType == TIicBusTransfer::EMasterRead)
				{
				r = StartTransfer(iHalfDTransfer, TIicBusTransfer::EMasterRead);
				if(r != KErrNone)
					{
					return r;
					}
				r = StartTransfer(iFullDTransfer, TIicBusTransfer::EMasterWrite);
				}
			else // hDTrType == TIicBusTransfer::EMasterWrite)
				{
				r = StartTransfer(iFullDTransfer, TIicBusTransfer::EMasterRead);
				if(r != KErrNone)
					{
					return r;
					}
				r = StartTransfer(iHalfDTransfer, TIicBusTransfer::EMasterWrite);
				}
			}
		else
		// this is normal halfDuplex transfer - so just start it
			{
			r = StartTransfer(iHalfDTransfer, hDTrType);
			}
		}
	return r;
	}

// Gateway function for PSL implementation - this method is an entry point - it is called by the PIL
// to initiate the transaction. After finishing it's processing PSL calls CompleteRequest(error_status)
TInt DCsiChannelMaster::DoRequest(TIicBusTransaction* aTransaction)
	{
	__KTRACE_OPT(KIIC, Kern::Printf("DCsiChannelMaster::DoRequest (aTransaction=0x%x)\n", aTransaction));

	// Check, if pointers are not NULL..
	if(!aTransaction || !GetTransactionHeader(aTransaction))
		{
		return KErrArgument;
		}

	// Make sure if the PIL doesn't try to start another one- before we've confirmed the previous one..
	if(iState != EIdle)
		{
		return KErrInUse;
		}

	// copy pointer to the transaction..
	iCurrTransaction = aTransaction;

	// check, if Hardware needs re-configuration
	// (i.e. this transaction and SlaveAddress (iSSPin) are the same as for the previous one)
	TInt r = KErrNone;
	if(TransConfigDiffersFromPrev())
		{
		r = ConfigureInterface();
		if(r != KErrNone)
			{
			iSSPin = 0;
			return r;
			}
		}

	// start processing transfers of this transaction.
	r = ProcessNextTransfers();

	return r;
	}

