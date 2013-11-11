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


#ifdef STANDALONE_CHANNEL
#include <drivers/iic_transaction.h>
#include "csi_master.h"
#include "csi_slave.h"
#else
#include <drivers/iic.h>
#endif
#include <drivers/iic_channel.h>
#include <drivers/gpio.h>
#include "cs42l51.h"

#include "d_csi.h"

#define __KTRACE(s)
//#define __KTRACE(s) __KTRACE_OPT(KIIC, s)

const TInt KMaxNumChannels = 1; // we only support one client at the time

// generic check-error macro - to not put this manually on each operation.
#define CHECK_ERR(r) \
          if(r != KErrNone){ \
          __KTRACE(Kern::Printf("d_csi.cpp, line: %d returned r=%d", __LINE__, r)); \
          return r;}

// Default Test Header
const TConfigSpiV01 DefaultHeader =
	{
	ESpiWordWidth_8, //iWordWidth
	260000, //iClkSpeed
	ESpiPolarityLowRisingEdge, //iClkMode
	500, // iTimeoutPeriod
	EBigEndian, // iEndianness
	EMsbFirst, //iBitOrder
	0, //iTransactionWaitCycles
	ESpiCSPinActiveLow //iCsPinActiveMode
	};

_LIT(KLddRootName,"d_csi");
_LIT(KIicClientThreadName,"IicClientLddThread");

// Constructor
DDeviceIicClient::DDeviceIicClient()
	{
	 __KTRACE(Kern::Printf("DDeviceIicClient::DDeviceIicClient()"));
	iParseMask = 0; // No info, no PDD, no Units
	iUnitsMask = 0;
	iVersion = TVersion(KIicClientMajorVersionNumber,
	        KIicClientMinorVersionNumber, KIicClientBuildVersionNumber);
	}

// Destructor
DDeviceIicClient::~DDeviceIicClient()
	{
	__KTRACE(Kern::Printf("DDeviceIicClient::~DDeviceIicClient()"));
	}

// Install the device driver.
TInt DDeviceIicClient::Install()
	{
	__KTRACE(Kern::Printf("DDeviceIicClient::Install()"));
	return (SetName(&KLddRootName));
	}

void DDeviceIicClient::GetCaps(TDes8& aDes) const
// Return the IicClient capabilities.
	{
	TPckgBuf<TCapsIicClient> b;
	b().version = TVersion(KIicClientMajorVersionNumber, KIicClientMinorVersionNumber, KIicClientBuildVersionNumber);
	Kern::InfoCopy(aDes, b);
	}

// Create a channel on the device.
TInt DDeviceIicClient::Create(DLogicalChannelBase*& aChannel)
	{
	__KTRACE(Kern::Printf("DDeviceIicClient::Create(DLogicalChannelBase*& aChannel)"));

	if (iOpenChannels >= KMaxNumChannels)
		{
		return KErrOverflow;
		}
	aChannel = new DChannelIicClient;
	return aChannel ? KErrNone : KErrNoMemory;
	}

DChannelIicClient::DChannelIicClient()
	{
	iClient = &Kern::CurrentThread();
	iSlaveCallback = NULL;
	// Increase the DThread's ref count so that it does not close without us
	iClient->Open();
	}

DChannelIicClient::~DChannelIicClient()
	{
	__KTRACE(Kern::Printf("DChannelIicClient::~DChannelIicClient()"));
	iDfcQue->Destroy();
	
	//free memory:
	delete iSpiHeader;
#ifdef STANDALONE_CHANNEL
#ifdef MASTER_MODE
	if(iMasterChannel)
		delete iMasterChannel;
#endif/*MASTER_MODE*/
	
#ifdef SLAVE_MODE
	if(iSlaveChannel)
		delete iSlaveChannel;
#endif/*SLAVE_MODE*/
#endif
	
	// decrement the DThread's reference count
    Kern::SafeClose((DObject*&) iClient, NULL);
	}

TInt DChannelIicClient::DoCreate(TInt aUnit, const TDesC8* /*aInfo*/,
        const TVersion& /*aVer*/)
	{
	__KTRACE(Kern::Printf("DChannelIicClient::DoCreate(), UNIT %d", aUnit));
	TInt r = KErrNone;

	// Bus Realisation Config iCsiBusId;
	iCsiBusId = 0;
	SET_BUS_TYPE(iCsiBusId,DIicBusChannel::ESpi);
	SET_CHAN_NUM(iCsiBusId,KCodecChannelNumber);
	SET_SLAVE_ADDR(iCsiBusId,14);
	
#ifdef STANDALONE_CHANNEL
#ifdef MASTER_MODE
	// create channel 0 - as master..
	iMasterChannel = DCsiChannelMaster::New(0, DIicBusChannel::ESpi, DIicBusChannel::EFullDuplex);
	if (!iMasterChannel)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Error no memory"));
		return KErrNoMemory;
		}
#endif /*MASTER_MODE*/
	
#ifdef SLAVE_MODE
	// and created channel 1 - as slave..
	iSlaveChannel = DCsiChannelSlave::New(1, DIicBusChannel::ESpi, DIicBusChannel::EFullDuplex);
	if (!iSlaveChannel)
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Error no memory"));
		#ifdef MASTER_MODE
		delete iMasterChannel; //Free memory
		#endif
		return KErrNoMemory;
		}
#endif  /*SLAVE_MODE*/


#endif/*STANDALONE_CHANNEL*/
	
	r = Kern::DynamicDfcQCreate(iDfcQue, KIicClientThreadPriority, KIicClientThreadName);
	if(r == KErrNone)
		{
		iSpiHeader = new TConfigSpiBufV01(DefaultHeader);
		if(iSpiHeader)
			{
			SetDfcQ(iDfcQue);
			iMsgQ.Receive();
			}
		else
			{
			r = KErrNoMemory;
			}
		}
	return r;
	}

void DChannelIicClient::HandleMsg(TMessageBase* aMsg)
	{
	TThreadMessage& m = *(TThreadMessage*) aMsg;
	TInt id = m.iValue;

	if (id == ECloseMsg)
		{
		// make sure, we've released the SlaveChannel on closure..
		DoControl(RBusCsiTestClient::EReleaseSlaveChannel, NULL, NULL);

		iMsgQ.iMessage->Complete(KErrNone, EFalse);
		return;
		}
	else if (id == KMaxTInt)
		{
		DoCancel(m.Int0());
		m.Complete(KErrNone, ETrue);
		return;
		}

	if (id < 0)
		{
		TRequestStatus* pS = (TRequestStatus*) m.Ptr0();
		TInt r = DoRequest(~id, pS, m.Ptr1(), m.Ptr2());
		if (r != KErrNone)
			{
			Kern::RequestComplete(iClient, pS, r);
			}
		m.Complete(KErrNone, ETrue);
		}
	else
		{
		TInt r = DoControl(id, m.Ptr0(), m.Ptr1());
		m.Complete(r, ETrue);
		}
	}

void DChannelIicClient::DoCancel(TInt aMask)
	{
	// Cancel an outstanding request.
	// Not implemented.
	return;
	}

#ifdef MASTER_MODE
TInt DChannelIicClient::QueueTransaction(TInt aBusId, TIicBusTransaction* aTransaction, TIicBusCallback *aCallback/*=NULL*/)
	{
#ifndef STANDALONE_CHANNEL
	if(!aCallback)
		return IicBus::QueueTransaction(aBusId, aTransaction);
	else
		return IicBus::QueueTransaction(aBusId, aTransaction, aCallback);
#else
	if(iMasterChannel)
		{
		aTransaction->iBusId = aBusId;
		if(!aCallback)
			return iMasterChannel->QueueTransaction(aTransaction);
		else
			return iMasterChannel->QueueTransaction(aTransaction, aCallback);
		}
	else 
		return KErrGeneral; //iMaster not initialised? - channels not created?
#endif
	}
#endif /*MASTER_MODE*/

#ifdef SLAVE_MODE
TInt DChannelIicClient::RegisterRxBuffer(TInt aChannelId, TPtr8 aRxBuffer, TInt8 aBufGranularity, TInt8 aNumWords, TInt8 aOffset)
	{
#ifdef STANDALONE_CHANNEL
	if(iSlaveChannel)
		return iSlaveChannel->RegisterTxBuffer(aRxBuffer, aBufGranularity, aNumWords, aOffset);
	else 
		return KErrGeneral;
#else
	return IicBus::RegisterRxBuffer(aChannelId, aRxBuffer, aBufGranularity, aNumWords, aOffset);
#endif
	}

TInt DChannelIicClient::RegisterTxBuffer(TInt aChannelId, TPtr8 aRxBuffer, TInt8 aBufGranularity, TInt8 aNumWords, TInt8 aOffset)
	{
#ifdef STANDALONE_CHANNEL
	if(iSlaveChannel)
		return iSlaveChannel->RegisterTxBuffer(aRxBuffer, aBufGranularity, aNumWords, aOffset);
	else 
		return KErrGeneral;
#else
	return IicBus::RegisterTxBuffer(aChannelId, aRxBuffer, aBufGranularity, aNumWords, aOffset);
#endif
	}

TInt DChannelIicClient::CaptureChannel(TInt aBusId, TDes8* aConfigHdr, TIicBusSlaveCallback* aCallback, TInt& aChannelId, TBool aAsynch)
	{
#ifdef STANDALONE_CHANNEL
	if(iSlaveChannel)
		return iSlaveChannel->CaptureChannel(aConfigHdr, aCallback, aChannelId, aAsynch);
	else 
		return KErrGeneral;
#else
	return IicBus::CaptureChannel(aBusId, aConfigHdr, aCallback, aChannelId, aAsynch);
#endif
	}


TInt DChannelIicClient::ReleaseChannel(TInt aChannelId)
	{
#ifdef STANDALONE_CHANNEL
	if(iSlaveChannel)
		return iSlaveChannel->ReleaseChannel();
	else 
		return KErrGeneral;
#else
		return IicBus::ReleaseChannel(aChannelId);
#endif
	}

TInt DChannelIicClient::SetNotificationTrigger(TInt aChannelId, TInt aTrigger)
	{
#ifdef STANDALONE_CHANNEL
	if(iSlaveChannel)
		return iSlaveChannel->SetNotificationTrigger(aTrigger);
	else
		return KErrGeneral;
#else
		return IicBus::SetNotificationTrigger(aChannelId, aTrigger);
#endif
	}
#endif /*SLAVE_MODE*/

// This test tests the timeout for the half-duplex Write requests.
// it creates a transaction with one transfer.
// Clk 200kHz actually sets the clock to  260000 kHz. With 8bit granularity - gives 25kB/s
// 32,5 bytes should be transferred in 1ms
// so e.g. setting the timeout to 1ms and the transfer size to more than that 32 bytes should
// cause a transfer timeout for transmission.
// test case:
// 1. setting timeout to 1ms with buffer 128 bytes - should result in KErrTImeout return value.
// 2. setting timeout to 5ms (~160 bytes transfer) - should result in KErrNone return value.
#ifdef MASTER_MODE
TInt DChannelIicClient::TestTransferTimeout()
	{
	__KTRACE(Kern::Printf("DChannelIicClient::TestTransferTimeout()"));
	TInt r = KErrNone;

	TUint32 busId = 0;
	SET_BUS_TYPE(busId,DIicBusChannel::ESpi);
	SET_CHAN_NUM(busId,0);
	SET_SLAVE_ADDR(busId,13); // test it with any number between 1-32

	// Create a transaction header
	const TConfigSpiV01 TestHeader =
		{
		ESpiWordWidth_8, //iWordWidth
		260000, //iClkSpeed
		ESpiPolarityLowRisingEdge, //iClkMode
		1, // iTimeoutPeriod
		EBigEndian, // iEndianness
		EMsbFirst, //iBitOrder
		0, //iTransactionWaitCycles
		ESpiCSPinActiveLow //iCsPinActiveMode
		};

	TPckgBuf<TConfigSpiV01> header(TestHeader);

	// create transfer object
	TBuf8<128> transfer_buf; // buffer..
	transfer_buf.SetLength(transfer_buf.MaxLength());

	TIicBusTransfer transfer(TIicBusTransfer::EMasterWrite, 8, &transfer_buf);

	// Create a transaction using header and transfer..
	TIicBusTransaction transaction1(&header, &transfer);

	// this transaction should return KErrTimeout
	r = QueueTransaction(busId, &transaction1);

	if (r != KErrTimedOut)
		{
		__KTRACE(Kern::Printf("no timeout??, r= %d", r));
		return r;
		}

	// change the timeout to  and create another transaction..
	header().iTimeoutPeriod = 6; // it works for 4 too (~130Bytes)
	transfer_buf.SetLength(transfer_buf.MaxLength());
	TIicBusTransfer transfer2(TIicBusTransfer::EMasterWrite, 8, &transfer_buf);

	TIicBusTransaction transaction2(&header, &transfer2);

	// this transaction should return KErrNone
	r = QueueTransaction(busId, &transaction2);

	if (r != KErrNone)
		{
		__KTRACE(Kern::Printf("Transaction failed, r= %d", r));
		}

	return r;
	}

TInt DChannelIicClient::TestHalfDuplexTransaction()
	{
	__KTRACE(Kern::Printf("DChannelIicClient::TestHalfDuplexTransaction()"));
	TInt r = KErrNone;

	TUint32 busId = 0;
	SET_BUS_TYPE(busId,DIicBusChannel::ESpi);
	SET_CHAN_NUM(busId,0);
	SET_SLAVE_ADDR(busId,13); // test it with any number between 1-32

	// create header
	const TConfigSpiV01 TestHeader =
		{
		ESpiWordWidth_8, //iWordWidth
		260000, //iClkSpeed
		ESpiPolarityLowRisingEdge, //iClkMode
		500, // iTimeoutPeriod
		EBigEndian, // iEndianness
		EMsbFirst, //iBitOrder
		0, //iTransactionWaitCycles
		ESpiCSPinActiveLow //iCsPinActiveMode
		};

	TPckgBuf<TConfigSpiV01> header(TestHeader);

	// create transfer object
	TBuf8<32> txTransferBuf; // buffer..
	TBuf8<32> rxTransferBuf; // buffer..
	for (TInt i = 0; i < txTransferBuf.MaxLength(); ++i)
		{
		txTransferBuf.Append(i);
		}
	txTransferBuf.SetLength(txTransferBuf.MaxLength());
	rxTransferBuf.SetLength(rxTransferBuf.MaxLength());

	// combine some transfers -in one transaction.
	TIicBusTransfer txTransfer(TIicBusTransfer::EMasterWrite, 8, &txTransferBuf);
	TIicBusTransfer rxTransfer(TIicBusTransfer::EMasterRead, 8, &rxTransferBuf);
	txTransfer.LinkAfter(&rxTransfer);

	TIicBusTransfer txTransfer2(TIicBusTransfer::EMasterWrite, 8, &txTransferBuf);
	rxTransfer.LinkAfter(&txTransfer2);

	TIicBusTransfer  rxTransfer2(TIicBusTransfer::EMasterRead, 8, &rxTransferBuf);
	txTransfer2.LinkAfter(&rxTransfer2);

	// Create a transaction using header and list of transfers..
	TIicBusTransaction transaction(&header, &txTransfer);

	// queue the transaction - it should complete successful
	r = QueueTransaction(busId, &transaction);

	return r;
	}

TInt DChannelIicClient::TestBulkTransfer()
	{
	//Create a Transaction object which  contains 2 Transfers. of buffer sizes say 256 and 64
	const TUint8 KTrBuf1Length = 128;
	const TUint8 KTrBuf2Length = 64;

	TUint32 busId = 0;
	SET_BUS_TYPE(busId,DIicBusChannel::ESpi);
	SET_CHAN_NUM(busId,0);
	SET_SLAVE_ADDR(busId,5);

	// create header
	const TConfigSpiV01 TestHeader =
		{
		ESpiWordWidth_16, //iWordWidth
		260000, //iClkSpeed
		ESpiPolarityLowRisingEdge, //iClkMode
		3000, // iTimeoutPeriod
		EBigEndian, // iEndianness
		EMsbFirst, //iBitOrder
		0, //iTransactionWaitCycles
		ESpiCSPinActiveLow //iCsPinActiveMode
		};

	TPckgBuf<TConfigSpiV01> header(TestHeader);

	TBuf8<KTrBuf1Length> transBuf1;
	TBuf8<KTrBuf2Length> transBuf2;
	TInt r = KErrNone;

	for (TInt i = 0; i < KTrBuf1Length; ++i)
		{
		transBuf1.Append(i + '0');
		}

	for (TInt i = 0; i < KTrBuf2Length; ++i)
		{
		transBuf2.Append(i + 'a');
		}

	transBuf1.SetLength(KTrBuf1Length);
	transBuf2.SetLength(KTrBuf2Length);

	//TIicBusTransfer  trans1;
	TIicBusTransfer trans1(TIicBusTransfer::TIicBusTransfer::EMasterWrite, 8, &transBuf2);
	TIicBusTransfer trans2(TIicBusTransfer::TIicBusTransfer::EMasterRead, 8, &transBuf1);
	trans1.LinkAfter(&trans2);

	// for the other direction - re-use buffers..(they will be interleaved)
	TIicBusTransfer trans3(TIicBusTransfer::TIicBusTransfer::EMasterRead, 8, &transBuf1);
	TIicBusTransfer trans4(TIicBusTransfer::TIicBusTransfer::EMasterWrite, 8, &transBuf2);
	trans3.LinkAfter(&trans4);

	// transaction
	TIicBusTransaction trans(&header, &trans1); // this will set the transaction with trans1 and trans2
	trans.SetFullDuplexTrans(&trans3); // and this will add trans3 and trans4 for the other direction

	r = QueueTransaction(busId, &trans);

	return r;
	}

// Testing CSI interface with AudioCodec:
// this involves several CSI write transactions to set-up the AudioCodec
// and configure it to generate the sound (of a particular frequency)

// CSI configuration parameters needed to transmit data to the Codec
// these are used as the transaction header object.
const TConfigSpiV01 KCodecSpiV01Config =
	{
	ESpiWordWidth_8, //iWordWidth
	260000, //iClkSpeed
	ESpiPolarityHighFallingEdge, //iClkMode
	100, // iTimeoutPeriod
	EBigEndian, // iEndianness
	EMsbFirst, //iBitOrder
	0, //iTransactionWaitCycles
	ESpiCSPinActiveLow //iCsPinActiveMode
	};

// helper function - to update buffer data and queue the transaction with the updated data
TInt DChannelIicClient::QueueRegisterWriteTransaction(TInt16 aRegister,
        TInt16 aValue, TIicBusTransaction* aTrans, TInt aConfig)
	{
	iTransBuff().iRegister = aRegister;
	iTransBuff().iData = aValue;
	TInt r = QueueTransaction(aConfig, aTrans);
	return r;
	}

// this method configures the codec and sets its registers to create a sound of
// requested frequency, duration time, volume and the time..etc..
// to set these params we will be using single transactions - reusing all objects created on the stack:
// - transaction (TIicBusTransaction),
// - header object (TConfigSpiV01 - embedded in  TPckgBuf<TConfigSpiV01> headerBuff)
// - TIicBusRealisationConfig - busId / config

TInt DChannelIicClient::TestAudioCodecBeep(TUint8 aFreq, TUint8 aTime,
        TUint8 aOffTime, TUint8 aVolume, TBool aRepeat /*=0*/)
	{
	__KTRACE(Kern::Printf("DChannelIicClient::TestAudioCodecBeep()"));

	// enable and configure pin 25 - it is connected to the chip's reset line
	GPIO::SetPinMode(KCodecResetPin, GPIO::EEnabled);
	GPIO::SetPinDirection(KCodecResetPin, GPIO::EOutput);
	GPIO::SetDebounceTime(KCodecResetPin, 0);

	// Bus Realisation Config
	TUint32 iCsiBusId = 0;
	SET_BUS_TYPE(iCsiBusId,DIicBusChannel::ESpi);
	SET_CHAN_NUM(iCsiBusId,KCodecChannelNumber);
	SET_SLAVE_ADDR(iCsiBusId,KCodecCSPin); // use codec's CS pin number as a Slave address

	TInt config = iCsiBusId;

	// create header
	TPckgBuf<TConfigSpiV01> header(KCodecSpiV01Config);

	// create transfer object - use iTransBuff member, which contains RCS42AudioCodec::TCodecConfigData
	TIicBusTransfer transfer(TIicBusTransfer::EMasterWrite, 8, &iTransBuff);
	TIicBusTransaction transaction(&header, &transfer);

	// setup the transaction - to use this header and transfer object
	transaction.SetHalfDuplexTrans(&header, &transfer);

	// fill the TCodecConfigData.Address only once, it won't change..
	iTransBuff().iAddress = KCodecWriteCommand;

	// power-up sequence..
	// put !reset line high to start the power-up sequence..
	//    GPIO::SetOutputState(KCodecResetPin, GPIO::ELow);
	GPIO::SetOutputState(KCodecResetPin, GPIO::EHigh);

	TInt r = KErrNone;

	r = QueueRegisterWriteTransaction(KHwCS42L51PwrCtrl, KHtCS42L51PwrCtrl_PDN, &transaction, config);
	CHECK_ERR(r);

	r = QueueRegisterWriteTransaction(KHwCS42L51PwrCtrl, KHtCS42L51PwrCtrl_PDN_ALL, &transaction, config);
	CHECK_ERR(r);

	// freeze all registers.. until are set-up.
	r = QueueRegisterWriteTransaction(KHwCS42L51DACControl, KHtCS42L51DACControl_FREEZE, &transaction, config);
	CHECK_ERR(r);

	//Set Mic power control and speed control register(0x03)
	r = QueueRegisterWriteTransaction(KHwCS42L51MicPwrSpeed, KHtCS42L51MicPwrSpeed_AUTO, &transaction, config);
	CHECK_ERR(r);

	// interface control (0x04) // serial port settings..
	// I2s, Slave, SDOUT->SDIN internally connected.. Digimix->ON?
	// use I2S format, Slave, Digital & Mic mix
	TUint16 val = (KHCS42L51CtrlI2sUpto24bit << KHsCS42L51CtrlFormat)
	        | KHtCS42L51Ctrl_DIGMIX | KHtCS42L51Ctrl_MICMIX;
	r = QueueRegisterWriteTransaction(KHwCS42L51Ctrl, val, &transaction, config);
	CHECK_ERR(r);

	// DAC output select (0x08)
	//      7      6        5        4          3        2         1        0
	// HP_GAIN2 HP_GAIN1 HP_GAIN0 DAC_SNGVOL INV_PCMB INV_PCMA DACB_MUTE DACA_MUTE
	r = QueueRegisterWriteTransaction(KHwCS42L51DACOutputControl, KHtCS42L51DACOutputControl_DAC_SNGVOL, &transaction, config);
	CHECK_ERR(r);

	// ALCX & PGAX ctrl, A(0x0A), B (0x0B)
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_PGA_A_Control, 0, &transaction, config);
	CHECK_ERR(r);
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_PGA_B_Control, 0, &transaction, config);
	CHECK_ERR(r);

	// ADCx Mixer Volume Ctrl A(0x0E), B (0x0F)
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_ADC_A_MixVolume, 0x10, &transaction, config);
	CHECK_ERR(r);

	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_ADC_B_MixVolume, 0x10, &transaction, config);
	CHECK_ERR(r);

	// PCMx Volume Ctrl A(0x10), B (0x11)
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_PCM_A_MixVolume, 0x10, &transaction, config);
	CHECK_ERR(r);
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_PCM_B_MixVolume, 0x10, &transaction, config);
	CHECK_ERR(r);

	// Volume Control: AOUTA (Address 16h) & AOUTB (Address 17h)
	//send next one..
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_Out_A_Volume, 0x10, &transaction, config);
	CHECK_ERR(r);
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_Out_B_Volume, 0x10, &transaction, config);
	CHECK_ERR(r);

	// DAC Control (Address 09h)
	//      7        6       5       4       3      2      1        0
	// DATA_SEL1 DATA_SEL0 FREEZE Reserved DEEMPH AMUTE DAC_SZC1 DAC_SZC0
	// DATA_SEL1 DATA_SEL0:
	// 00 - PCM Serial Port to DAC
	// 01 - Signal Processing Engine to DAC
	// 10 - ADC Serial Port to DAC    (11 - Reserved)
	r = QueueRegisterWriteTransaction(KHwCS42L51DACControl, (1 << KHsCS42L51DACControl_DATA_SEL), &transaction, config);
	CHECK_ERR(r);

	// power-up sequence..end - clear PDN ..after loading register settings..
	r = QueueRegisterWriteTransaction(KHwCS42L51PwrCtrl, 0, &transaction, config);
	CHECK_ERR(r);

	// now - the codec is ready to generate the beep of requested frequency..
	// Set the beep frequency and time..
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_Beep_FQ_Time, (aFreq
	    << KHsCS42L51ALC_Beep_FQ) | (aTime & KHmCS42L51ALC_Beep_Time_Mask), &transaction, config);
	CHECK_ERR(r);

	// Set the Volume and off time
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_Beep_Off_Volume,
	    (aOffTime << KHsCS42L51ALC_Beep_Off) | (aVolume & KHmCS42L51ALC_Beep_Volume_Mask),
	    &transaction, config);
	CHECK_ERR(r);

	// set the 'repeat' bit and enable the beep..
	r = QueueRegisterWriteTransaction(KHwCS42L51ALC_Beep_Conf_Tone,
	    (aRepeat ? KHtCS42L51ALC_Beep_Conf_Tone_REPEAT : 0) |
	    KHtCS42L51ALC_Beep_Conf_Tone_BEEP, &transaction, config);
	CHECK_ERR(r);

	NKern::Sleep(500); // give it some time to play ;)

	// power down the codec:
	r = QueueRegisterWriteTransaction(KHwCS42L51PwrCtrl, KHtCS42L51PwrCtrl_PDN, &transaction, config);
	CHECK_ERR(r);

	GPIO::SetOutputState(KCodecResetPin, GPIO::ELow);
	return KErrNone;
	}


// Test Transaction with different configuration settings
// 1) Different Header settinngs (TConfigSpiBufV01))
// 2) Different Bus Config settings (Bus Realisation Config)
TInt DChannelIicClient::TestConfigParams(TInt aBusId)
	{
	TInt r = KErrNone;

	const TUint8 KTrasferBufferLength = 120;

	//First Create a Half Duplex Transfer List of 3 Transfers of Type W,R,W
	TBuf8<KTrasferBufferLength> trBuf1;
	TBuf8<KTrasferBufferLength> trBuf2;

	TIicBusTransfer hdTrasfer1(TIicBusTransfer::EMasterWrite, 8, &trBuf1);
	TIicBusTransfer hdTrasfer2(TIicBusTransfer::EMasterRead, 8, &trBuf2);

	//Link the Half Duplex Transfers 1->2
	hdTrasfer1.LinkAfter(&hdTrasfer2);

	//Create A Ful Duplex Transfer of 3 Transfers of type ,R,W,R
	TBuf8<KTrasferBufferLength> trBuf3;
	TBuf8<KTrasferBufferLength> trBuf4;

	TIicBusTransfer fdTrasfer1(TIicBusTransfer::EMasterRead, 8, &trBuf3);
	TIicBusTransfer fdTrasfer2(TIicBusTransfer::EMasterWrite, 8, &trBuf4);

	//Link the Full Duplex Transfers 1->2
	fdTrasfer1.LinkAfter(&fdTrasfer2);

	for (TUint i = 0; i < KTrasferBufferLength; i++)
		{
		//Fill the Buffers  associated with Write Transfer with some data
		trBuf1.Append(i + '0');
		trBuf4.Append(i + 'A');
		}

	trBuf1.SetLength(KTrasferBufferLength);
	trBuf2.SetLength(KTrasferBufferLength);
	trBuf3.SetLength(KTrasferBufferLength);
	trBuf4.SetLength(KTrasferBufferLength);

	// Create a transaction using header and transfer..
	TIicBusTransaction fdTransaction(iSpiHeader, &hdTrasfer1);

	fdTransaction.SetFullDuplexTrans(&fdTrasfer1);

	// Queue Transaction
	r = QueueTransaction(aBusId, &fdTransaction);

	return r;
	}


// Test Duplex Transaction
// Construct two chain of combined transfers for the transaction:
// 1) A Half Duplex Transfer with a Read , Write and Read  operation
// 2) A Full Duplex Transfer with a Write, Read and Write Operation.
TInt DChannelIicClient::TestDuplexTransaction()
	{
	__KTRACE(Kern::Printf("DChannelIicClient::TestDuplexTransaction\n"));

	TInt r = KErrNone;

	TUint32 busId = 0;
	SET_BUS_TYPE(busId,DIicBusChannel::ESpi);
	SET_CHAN_NUM(busId,0);
	SET_SLAVE_ADDR(busId,1);

	// create header
	const TConfigSpiV01 DefaultHeader =
		{
		ESpiWordWidth_8, //iWordWidth
		260000, //iClkSpeed
		ESpiPolarityLowRisingEdge, //iClkMode
		1000, // iTimeoutPeriod
		EBigEndian, // iEndianness
		EMsbFirst, //iBitOrder
		0, //iTransactionWaitCycles
		ESpiCSPinActiveLow//iCsPinActiveMode
		};

	const TUint8 KTrasferBufferLength = 120;

	TConfigSpiBufV01 header(DefaultHeader);

	//First Create a Half Duplex Transfer List of 3 Transfers of Type W,R,W
	TBuf8<KTrasferBufferLength> trBuf1;
	TBuf8<KTrasferBufferLength> trBuf2;
	TBuf8<KTrasferBufferLength> trBuf3;

	TIicBusTransfer hdTrasfer1(TIicBusTransfer::EMasterWrite, 8, &trBuf1);
	TIicBusTransfer hdTrasfer2(TIicBusTransfer::EMasterRead, 8, &trBuf2);
	TIicBusTransfer hdTrasfer3(TIicBusTransfer::EMasterWrite, 8, &trBuf3);

	//Link the Half Duples Transfers 1->2->3
	hdTrasfer1.LinkAfter(&hdTrasfer2);
	hdTrasfer2.LinkAfter(&hdTrasfer3);

	//Create A Ful Duples Transfer of 3 Transfers of type ,R,W,R
	TBuf8<KTrasferBufferLength> trBuf4;
	TBuf8<KTrasferBufferLength> trBuf5;
	TBuf8<KTrasferBufferLength> trBuf6;

	TIicBusTransfer fdTrasfer1(TIicBusTransfer::EMasterRead, 8, &trBuf4);
	TIicBusTransfer fdTrasfer2(TIicBusTransfer::EMasterWrite, 8, &trBuf5);
	TIicBusTransfer fdTrasfer3(TIicBusTransfer::EMasterRead, 8, &trBuf6);

	//Link the Full Duples Transfers 1->2->3
	fdTrasfer1.LinkAfter(&fdTrasfer2);
	fdTrasfer2.LinkAfter(&fdTrasfer3);

	for (TUint i = 0; i < KTrasferBufferLength; i++)
		{
		//Fill the Buffers  associated with Write Transfer with some data
		trBuf1.Append(i + '0');
		trBuf3.Append(i + 'A');
		trBuf5.Append(i + 'a');
		//Read Buffers
		trBuf2.Append('$');
		trBuf4.Append('?');
		trBuf6.Append('*');
		}

	trBuf1.SetLength(KTrasferBufferLength);
	trBuf2.SetLength(KTrasferBufferLength);
	trBuf3.SetLength(KTrasferBufferLength);
	trBuf4.SetLength(KTrasferBufferLength);
	trBuf5.SetLength(KTrasferBufferLength);
	trBuf6.SetLength(KTrasferBufferLength);

	// Create a transaction using header and transfer..
	TIicBusTransaction fdTransaction(&header, &hdTrasfer1);
	fdTransaction.SetFullDuplexTrans(&fdTrasfer1);

	//Queue Transaction
	r = QueueTransaction(busId, &fdTransaction);

	return r;
	}

// Callback for Master's asynchronous transactions
// this is called whenever the transaction has been finished.
void DChannelIicClient::AsyncTransCallbackFunc(TIicBusTransaction* aTransaction, TInt /*aBusId*/, TInt aResult, TAny* aParam)
	{
	__KTRACE(Kern::Printf("DChannelIicClient::AsyncTransCallbackFunc  aResult  =%d\n",aResult));
	TCallBckRequest<3> *cr = (TCallBckRequest<3>*)aParam;

	// complete request..
	Kern::RequestComplete(cr->iClient, cr->iReqStatus, aResult);

	// now can finally delete cr object = wchich will delete 1-6 (i don't like that..TIicBusTransaction has all this info..)
	delete cr;
	if(aTransaction)
	    delete aTransaction;
	}

/*
Test Asynchronous Transaction
with a chain of HD(HalfDuplex) transfers:  Read -> Write -> Read

 Memory management for asynchronous transactions: we create the following on the heap:
  1. transaction header (in DoRequest),
  2. transaction
  3. transfers for transaction (chained in linked-list)
  4. buffers for transfers
  5. IIC callback object (TIicBusCallback)
  6. callback request - which stores pointers to callback object(TIicBusCallback),
     and pointers to the client and the request status object (used by RequestComplete)

  All of these need to be deleted in the AsyncTransCallbackFunc
  (at the end of transaction)
*/
const TUint8 KTrasferBufferLength = 120;

TInt DChannelIicClient::TestAsynTransaction(TRequestStatus* aStatus, TConfigSpiBufV01* aSpiHeader, TInt aBusId)
	{
	__KTRACE(Kern::Printf("DChannelIicClient::TestAsynTransaction\n"));
	TInt r = KErrNone;

	// Store required RequestComplete pointers in our TCbRequest (6.)
	TCallBckRequest<3> *cbRequest = new TCallBckRequest<3>(iClient, aStatus, aSpiHeader);

	//Create the CallBack Function (5.)
	TIicBusCallback *callback = new TIicBusCallback(AsyncTransCallbackFunc, (TAny*)cbRequest, iDfcQue, 5); // 5 arbitrary
	cbRequest->iCallback = callback;

	// create buffers (4.)
	HBuf8* trBuf1 = HBuf8::New(KTrasferBufferLength);
	cbRequest->iBuffers[0] = trBuf1;
	HBuf8* trBuf2 = HBuf8::New(KTrasferBufferLength);
	cbRequest->iBuffers[1] = trBuf2;
	HBuf8* trBuf3 = HBuf8::New(KTrasferBufferLength);
	cbRequest->iBuffers[2] = trBuf3;

	// create transfers..(3.)
	TIicBusTransfer* hdTrasfer1 = new TIicBusTransfer(TIicBusTransfer::EMasterRead, 8, trBuf1);
	cbRequest->iTransfers[0] = hdTrasfer1;
	TIicBusTransfer* hdTrasfer2 = new TIicBusTransfer(TIicBusTransfer::EMasterWrite, 8, trBuf2);
	cbRequest->iTransfers[1] = hdTrasfer2;
	TIicBusTransfer* hdTrasfer3 = new TIicBusTransfer(TIicBusTransfer::EMasterRead, 8, trBuf3);
	cbRequest->iTransfers[2] = hdTrasfer3;

	// Create a transaction using header and transfer..(2.)
	TIicBusTransaction* transaction = new TIicBusTransaction(aSpiHeader, hdTrasfer1);

	if(!trBuf1 || !trBuf2 || !trBuf2 ||
	   !hdTrasfer1 || !hdTrasfer2 || !hdTrasfer3 ||
	   !transaction || ! cbRequest || !callback)
		{
		delete cbRequest;
		__KTRACE(Kern::Printf("No memory for allocating transfer buffers"));
		return KErrNoMemory;
		}


	// put some dummy data in buffers..
	for (TUint i = 0; i < KTrasferBufferLength; ++i)
		{
		//Fill the Buffers  associated with Write Transfer with some data
		trBuf1->Append(i + '0');
		trBuf2->Append(i + 'a');
		trBuf3->Append(i + 'A');
		}

	//Link the transfers 1->2->3
	hdTrasfer1->LinkAfter(hdTrasfer2);
	hdTrasfer2->LinkAfter(hdTrasfer3);

	// Queue Transaction
	r = QueueTransaction(aBusId, transaction, callback);
	return r;
	}
#endif/*MASTER_MODE*/

// =======================
//  Slave channel tests
//
// callbackForOneduplex asynchronous transaction..
void DChannelIicClient::MasterCallbackFunc(TIicBusTransaction* aTransction,
                                           TInt aBusId,
                                           TInt aResult,
                                           TAny* aParam)
	{
//	DChannelIicClient* a = (DChannelIicClient*) aParam;
	__KTRACE(Kern::Printf("MasterCallbackFunc, aResult = %d", aResult));
	}

#ifdef MASTER_MODE
// this method asynchronously queues one duplex transaction using
// the buffers, which are part of the DChannelIicClient().
TInt DChannelIicClient::QueueOneDuplexTransaction(TInt aSize)
	{
	__KTRACE(Kern::Printf("QueueOneDuplexTransaction, aSize = %d", aSize));

	TInt r = KErrNone;
	if(aSize > KMaxSlaveTestBufferSize)
		{
		return KErrArgument;
		}

	TUint32 busId = 0;
	SET_BUS_TYPE(busId,DIicBusChannel::ESpi);
	SET_CHAN_NUM(busId,0);
	SET_SLAVE_ADDR(busId,13); // test it with any number between 1-32

	// create header
	const TConfigSpiV01 TestHeader =
		{
		ESpiWordWidth_8, //iWordWidth
		260000, //iClkSpeed
		ESpiPolarityLowRisingEdge, //iClkMode
		500, // iTimeoutPeriod
		EBigEndian, // iEndianness
		EMsbFirst, //iBitOrder
		0, //iTransactionWaitCycles
		ESpiCSPinActiveLow //iCsPinActiveMode
		};
	TPckgBuf<TConfigSpiV01> header(TestHeader);

	TBuf8<KMaxSlaveTestBufferSize> txBuf; // buffer..
	TBuf8<KMaxSlaveTestBufferSize> rxBuf; // buffer..

	// create transfer object
	for (TInt i = 0; i < aSize; ++i)
		{
		txBuf.Append(i);
		}
	rxBuf.SetLength(aSize);

	// combine some transfers -in one transaction.
	TIicBusTransfer txTransfer(TIicBusTransfer::EMasterWrite, 8, &txBuf);
	TIicBusTransfer rxTransfer(TIicBusTransfer::EMasterRead, 8, &rxBuf);

	// Create a transaction using header and list of transfers..
	iMasterTransaction.SetHalfDuplexTrans(&header, &txTransfer);
	iMasterTransaction.SetFullDuplexTrans(&rxTransfer);

	// queue the transaction
	r = QueueTransaction(busId, &iMasterTransaction);

	return r;
	}
#endif /*MASTER_MODE*/

#ifdef SLAVE_MODE
TInt DChannelIicClient::RegisterSlaveBuffers()
	{
	__KTRACE(Kern::Printf("RegisterSlaveBuffers(), size %d", iSlaveBufSize));
	TInt r = KErrNone;
	// put some data in the buffer - so we could see-if this is actually beeing transfered...
	for(TInt i = 0; i < iSlaveBufSize; ++i)
		{
		iTxBuf.Append(i);
		}

	TPtr8 rxBuf(0, 0);
	TPtr8 txBuf(0, 0);

	rxBuf.Set((TUint8*) iRxBuf.Ptr(), iSlaveBufSize, iRxBuf.MaxLength());
	txBuf.Set((TUint8*) iTxBuf.Ptr(), iSlaveBufSize, iTxBuf.MaxLength());

	r = RegisterRxBuffer(iChannelId, rxBuf, 8, iSlaveBufSize, 0);

	if(r != KErrNone)
		{
		__KTRACE(Kern::Printf("Error Register Rx Buffer, r %d", r));
		}
	else
		{
		r = RegisterTxBuffer(iChannelId, txBuf, 8, iSlaveBufSize, 0);	
		}

	return r;
	}

TInt DChannelIicClient::CaptureSlaveChannel(TInt aBufSize, TBool aAsynch /* = EFalse*/)
	{
	if(aBufSize > KMaxSlaveTestBufferSize)
		{
		return KErrArgument;
		}

	TInt r = KErrNone;
	TUint32 slaveBusId = 0;
	SET_BUS_TYPE(slaveBusId,DIicBusChannel::ESpi);
	SET_CHAN_NUM(slaveBusId,1);
	
	// create header
	const TConfigSpiV01 TestHeader =
		{
		ESpiWordWidth_8, //iWordWidth
		260000, //iClkSpeed
		ESpiPolarityLowRisingEdge, //iClkMode
		500, // iTimeoutPeriod
		EBigEndian, // iEndianness
		EMsbFirst, //iBitOrder
		0, //iTransactionWaitCycles
		ESpiCSPinActiveLow //iCsPinActiveMode
		};

	TPckgBuf<TConfigSpiV01> header(TestHeader);

	// create callback objects for slave tests..
	if (!iSlaveCallback)
		{
		iSlaveCallback = new TIicBusSlaveCallback(SlaveCallbackFunc, (TAny*) this, iDfcQue, 0);
		}

	if (!iMasterCallback)
		{
		iMasterCallback = new TIicBusCallback(MasterCallbackFunc, (TAny*) this, iDfcQue, 0);
		}

	if (!(iSlaveCallback && iMasterCallback))
		{
		if(iSlaveCallback)
			{
			delete iSlaveCallback;
			}

		if(iMasterCallback)
			{
			delete iMasterCallback;
			}
		return KErrNoMemory;
		}

	TInt channelId = 0;
	// capture channel..
	r = CaptureChannel(slaveBusId, &header, iSlaveCallback, channelId, aAsynch);
	if (r != KErrNone)
		{
		__KTRACE(Kern::Printf("Error capturing the channel, r %d", r));
		}
	else
		{
		iSlaveBufSize = aBufSize;

		if(!aAsynch)
			{
			iChannelId = channelId;
			__KTRACE(Kern::Printf("channelId %d", iChannelId));
			r = RegisterSlaveBuffers();
			}
		}

	return r;
	}

TInt DChannelIicClient::ReleaseSlaveChannel()
	{
	TInt r = KErrNone;
	// release the channel

	if (iChannelId)
		{
		r = ReleaseChannel(iChannelId);
		if (r == KErrNone)
			{
			iChannelId = 0;
			if(iSlaveCallback)
				{
				delete iSlaveCallback;
				iSlaveCallback = NULL;
				}

			if(iMasterCallback)
				{
				delete iMasterCallback;
				iMasterCallback = NULL;
				}
			}
		else
			{
			__KTRACE(Kern::Printf("Error releasing the channel, r %d", r));
			}
		}
	//	else // if the channel was released already - silently return KErrNone..

	return r;
	}

#ifdef _DEBUG
void PrintTrigger(TInt aTrigger)
	{
	Kern::Printf("\naReturn:");
	if(aTrigger & ETxAllBytes){Kern::Printf("ETxAllBytes");}
	if(aTrigger & ETxOverrun){Kern::Printf("ETxOverrun");}
	if(aTrigger & ETxUnderrun){Kern::Printf("ETxUnderrun");}

	if(aTrigger & ERxAllBytes){Kern::Printf("ERxAllBytes");}
	if(aTrigger & ERxOverrun){Kern::Printf("ERxOverrun");}
	if(aTrigger & ERxUnderrun){Kern::Printf("ERxUnderrun");}

	if(aTrigger & EGeneralBusError){Kern::Printf("EGeneralBusError");}
	if(aTrigger & EAsyncCaptChan){Kern::Printf("EAsyncCaptChan");}

	Kern::Printf("\n");
	}
#endif


void DChannelIicClient::SlaveCallbackFunc(TInt aChannelId, TInt aReturn,
        TInt aTrigger, TInt16 aRxWords, TInt16 aTxWords, TAny* aParam)
	{
	__KTRACE(Kern::Printf("SlaveClientCallbackFunc(),aChannelId=0x%x,aReturn=%d,aTrigger=0x%x,aRxWords=0x%x,aTxWords=0x%x,aParam=0x%x\n",aChannelId,aReturn,aTrigger,aRxWords,aTxWords,aParam));

	DChannelIicClient* aClient = (DChannelIicClient*) aParam;
	if(aClient)
		{
		if(aTrigger & (EAsyncCaptChan))
			{
			if(aReturn == KErrCompletion)
				{
				aClient->iChannelId = aChannelId;
				__KTRACE(Kern::Printf("channelId %d", aChannelId));
				TInt r = aClient->RegisterSlaveBuffers();
				//AsyncCaptChan expects a KErrCompletion returned. 
				//If RegisterSlaveBuffers fails, the error code will be assigned to aReturn,
				//then passed back to client. 
				if(r != KErrNone)
				    aReturn = r; 
				}

			Kern::RequestComplete(aClient->iClient, aClient->iSlaveReqStatus, aReturn);
			return;
			}

		if(aTrigger & (EGeneralBusError))
			{
#ifdef _DEBUG
			Kern::Printf("\n\naRxWords %d", aRxWords);
			Kern::Printf("aTxWords %d", aTxWords);
			PrintTrigger(aReturn);
#endif
			Kern::RequestComplete(aClient->iClient, aClient->iSlaveReqStatus, KErrGeneral);
			return;
			}

		TInt r = KErrNone;
		// if there was an underrun/overrun
		if(aTrigger & ETxUnderrun)
			{
			TPtr8 txBuf(0, 0);

			txBuf.Set((TUint8*) aClient->iTxBuf.Ptr(), aClient->iTxBuf.MaxLength(), aClient->iTxBuf.MaxLength());

			// use aTxWords as an offset..
			if(aTxWords + 32 <= KMaxSlaveTestBufferSize)
				{
				r = aClient->RegisterTxBuffer(aClient->iChannelId, txBuf, 8, 32, aTxWords);
				if (r != KErrNone)
					{
			         Kern::Printf("Error Register Tx Buffer, r %d", r);
					}
				else
					{
					r = aClient->SetNotificationTrigger(aClient->iChannelId, ETxAllBytes);
					if (r != KErrNone)
						{
						Kern::Printf("Error setting notification trigger, r %d", r);
						}
					}

				// updated the buffer - so return..
				return;
				}
			}

		// if we've finished..
		if(aReturn & (ETxAllBytes | ERxAllBytes))
			{
			//PrintTrigger(aReturn);
			Kern::RequestComplete(aClient->iClient, aClient->iSlaveReqStatus, KErrNone);
			}

#if 0
		TInt8* ptr = (TInt8*)aClient->iRxBuf.Ptr();
		for(TInt i = 0; i < aRxWords; ++i)
			{
			Kern::Printf("Rx item %d: %x", i, (TInt8)*(ptr+i));
			}
#endif

		if(aTrigger & (/*ERxAllBytes |*/ ETxAllBytes))
			{
			Kern::RequestComplete(aClient->iClient, aClient->iSlaveReqStatus, KErrNone);
			}
		}
	else
		{
		Kern::RequestComplete(aClient->iClient, aClient->iSlaveReqStatus, KErrGeneral);
		}

	}

TInt DChannelIicClient::SetSlaveNotificationTrigger(TUint32 aTrigger)
	{
	TInt r = KErrNone;
	__KTRACE(Kern::Printf("DChannelIicClient::SetSlaveNotificationTrigger() aTrigger=0x%x", aTrigger));

	if (!iChannelId)
		{
		__KTRACE(Kern::Printf("Channel not captured..Call CaptureSlaveChannel() first.."));
		return KErrArgument;
		}

	//    ERxAllBytes         = 0x01,
	//    ERxUnderrun         = 0x02,
	//    ERxOverrun          = 0x04,
	//    ETxAllBytes         = 0x08,
	//    ETxUnderrun         = 0x10,
	//    ETxOverrun          = 0x20,
	//    EGeneralBusError    = 0x40

	r = SetNotificationTrigger(iChannelId, aTrigger);

	if (r != KErrNone)
		{
		__KTRACE(Kern::Printf("Error setting notification trigger, r %d", r));
		}

	return r;
	}
#endif /*SLAVE_MODE*/

// to handle synchronous requests from the client
TInt DChannelIicClient::DoControl(TInt aId, TAny* a1, TAny* a2)
	{
	TInt r = KErrNone;
	switch (aId)
		{
		case RBusCsiTestClient::ETestAudioCodecBeep:
			{
#ifdef MASTER_MODE
			r = TestAudioCodecBeep(0, 7, 3, 5);
#else/* MASTER_MODE */
			r = KErrNotSupported; 
#endif/* MASTER_MODE */
			break;
			}
		case RBusCsiTestClient::ETestTransferTimeout:
			{
#ifdef MASTER_MODE			
			r = TestTransferTimeout();
#else/* MASTER_MODE */
			r = KErrNotSupported; 
#endif/* MASTER_MODE */
			break;
			}
		case RBusCsiTestClient::ETestTestHalfDuplex:
			{
#ifdef MASTER_MODE
			r = TestHalfDuplexTransaction();
#else /* MASTER_MODE */
            r = KErrNotSupported; 
#endif /* MASTER_MODE */
			break;
			}
		case RBusCsiTestClient::ETestBulkTransfer:
			{
#ifdef MASTER_MODE
			r = TestBulkTransfer();
#else/* MASTER_MODE */
            r = KErrNotSupported; 
#endif/* MASTER_MODE */
			break;
			}
		case RBusCsiTestClient::ETestDuplexTransaction:
			{
#ifdef MASTER_MODE
			r = TestDuplexTransaction();
#else/* MASTER_MODE */
            r = KErrNotSupported; 
#endif/* MASTER_MODE */
			break;
			}
		case RBusCsiTestClient::ETestConfigParams:
			{
			// This is an asynchronous transaction - so we're using iSpiHeader member
			// to copy the data from the user.
#ifdef MASTER_MODE
			r = Kern::ThreadDesRead(iClient, (TDes8*) a1, *iSpiHeader, 0, KChunkShiftBy0);
			if(r == KErrNone)
				{
				// a2 is a BusConfig value
				r = TestConfigParams((TInt)a2);
				}
#else/* MASTER_MODE */
			r = KErrNotSupported;
#endif/* MASTER_MODE */
			break;
			}
		case RBusCsiTestClient::ECaptureSlaveChannel:
			{
#ifdef SLAVE_MODE
			r = CaptureSlaveChannel((TInt)a1);
#else/* SLAVE_MODE */
			r = KErrNotSupported;
#endif/* SLAVE_MODE */
			break;
			}
		case RBusCsiTestClient::EReleaseSlaveChannel:
			{
#ifdef SLAVE_MODE			
			r = ReleaseSlaveChannel();
#else/* SLAVE_MODE */
            r = KErrNotSupported;
#endif/* SLAVE_MODE */
			break;
			}
		case RBusCsiTestClient::EQueueOneDuplexTransaction:
			{
#ifdef MASTER_MODE
			r = QueueOneDuplexTransaction((TInt)a1);
#else/* MASTER_MODE */
            r = KErrNotSupported; 
#endif/* MASTER_MODE */
			break;
			}
		default:
			{
			__KTRACE(Kern::Printf("DChannelIicClient::DoControl():Unrecognized value for aId=0x%x\n", aId));
			r = KErrArgument;
			break;
			}
		}
	return r;
	}

// to handle asynchronous requests from the client
TInt DChannelIicClient::DoRequest(TInt aId, TRequestStatus* aStatus, TAny* a1, TAny* a2)
	{
	__KTRACE(Kern::Printf("DChannelIicClient::DoRequest(aId=0x%x, aStatus=0x%x, a1=0x%x, a2=0x%x\n",
					aId, aStatus, a1, a2));

	TInt r = KErrNone;
	switch (aId)
		{
		case RBusCsiTestClient::ETestAsynTransaction:
			{
#ifdef MASTER_MODE
			// Create a header, and copy it's content from the User.
			// it will be deleted after the transaction is completed.
			TConfigSpiBufV01* spiHeader = new TConfigSpiBufV01;
			if(spiHeader)
				{
				r = Kern::ThreadDesRead(iClient, (TDes8*) a1, *spiHeader, 0, KChunkShiftBy0);
				if(r == KErrNone)
					{
					// a2 is a BusConfig value
					r = TestAsynTransaction(aStatus, spiHeader, (TInt) a2);
					}
				else
					{
					delete spiHeader;
					}
				}
			else
				{
				r = KErrNoMemory;
				}
#else
			r = KErrNotSupported;
#endif/*MASTER_MODE*/
			break;
			}

		case (RBusCsiTestClient::ECaptureSlaveChannelAsync):
			{
#ifdef SLAVE_MODE
			// a1 - size
			iSlaveReqStatus = aStatus;
			r = CaptureSlaveChannel((TInt) a1, ETrue);
#else
			r = KErrNotSupported;
#endif/*SLAVE_MODE*/
			break;
			}

		case (RBusCsiTestClient::ESetSlaveNotificationTrigger):
			{
#ifdef SLAVE_MODE
			// a1 = trigger
			iSlaveReqStatus = aStatus;
			r = SetSlaveNotificationTrigger((TUint32) a1);
#else
			r = KErrNotSupported;
#endif
			break;
			}

		default:
			{
			__KTRACE(Kern::Printf("DChannelIicClient::DoRequest(): unrecognized value for aId=0x%x\n", aId));
			r = KErrArgument;
			break;
			}
		}
	return r;
	}

// LDD entry point
DECLARE_STANDARD_LDD()
	{
	return new DDeviceIicClient;
	}

