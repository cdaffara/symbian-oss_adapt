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
* ne1_tb\specific\cs42l51.cpp
*
*/



#include <kernel/kern_priv.h>
#include <naviengine.h>
#include <naviengine_priv.h>
#include <gpio.h>
#include "cs42l51.h"

#define __THREAD_AND_CPU 	Kern::Printf("(Thread %T, CPU: %d)\n", NKern::CurrentThread(), NKern::CurrentCpu())
//#define __KTRACE_SCODEC(s) s
#define __KTRACE_SCODEC(s)  __KTRACE_OPT(KSOUND1, s)


#if _DEBUG
static const char KCodecPanicCat[] = "AUDIO CODEC, line:";
#endif

// other constants
const TUint KCodecCSIChannel = 0;
const TInt8 KBufGranularity = 8; // width of a transfer word in bits

// CSI configuration parameters for the data transmission to the Codec.
const TConfigSpiV01 KCodecSpiV01Config =
	{
	ESpiWordWidth_8, //iWordWidth
	260000,          //iClkSpeed
	ESpiPolarityHighFallingEdge, //iClkMode
	40,            // iTimeoutPeriod
	EBigEndian,    // iEndianness
	EMsbFirst,     // iBitOrder
	0,             // iTransactionWaitCycles
	ESpiCSPinActiveLow //iCsPinActiveMode
	};

// static members
RCS42AudioCodec* RCS42AudioCodec::iSelf = 0;
TInt RCS42AudioCodec::iRefCnt = 0;

// default constructor
RCS42AudioCodec::RCS42AudioCodec() :
	iHeaderBuff(KCodecSpiV01Config)
	{
	}

TInt RCS42AudioCodec::Create()
	{
	__KTRACE_SCODEC(Kern::Printf("\n\nRCS42AudioCodec::Create()"));
	if (!iSelf)
		{
		iSelf = new RCS42AudioCodec;
		if(!iSelf)
			{
			return KErrNoMemory;
			}
		}
	return KErrNone;
	}

// free resources when the driver is beeing closed.
void RCS42AudioCodec::Destroy()
	{
	__KTRACE_SCODEC(Kern::Printf("RCS42AudioCodec::Destroy()"));
	__ASSERT_DEBUG(iSelf, Kern::Fault(KCodecPanicCat, __LINE__));

	delete iSelf;
	iSelf = 0; // static member
	}

// this static method is called from DNE1_TBSoundScPddChannel::PowerUp()
// to open an instance to the codec. If this is the first instance
// beeing opened - the codec is initialized.
TInt RCS42AudioCodec::Open(RCS42AudioCodec* &aSelf)
	{
	__KTRACE_SCODEC(Kern::Printf("RCS42AudioCodec::Open()"));

	TInt r = KErrNone;
	if(!iSelf)
		{
		r = Create();
		if(r != KErrNone)
			{
			return r;
			}
		}

	if (iRefCnt == 0)
		{
		r = iSelf->Init();
		if (r != KErrNone)
			{
			iSelf->PowerDown();
			return r;
			}
		}
	// increment the reference counter
	++iRefCnt;

	// copy object address back to the client
	aSelf = iSelf;

	return KErrNone;
	}

// this static method is called from DNE1_TBSoundScPddChannel::PowerDown()
// to close the reference and power down the codec if the last reference is beeing closed.
void RCS42AudioCodec::Close(RCS42AudioCodec* &aSelf)
	{
	__KTRACE_SCODEC(Kern::Printf("RCS42AudioCodec::Close()"))	;

	if (!aSelf || aSelf != iSelf)
		{
		return;
		}

	// decrement the reference counter
	--iRefCnt;

	// if closing the last instance - power down the codec
	if (iRefCnt == 0)
		{
		iSelf->PowerDown();
		aSelf = 0;
		Destroy();
		}
	}

// this method powers down the codec
void RCS42AudioCodec::PowerDown()
	{
	if (iSelf)
		{
		iSelf->StartWrite();
		iSelf->Write(KHwCS42L51DACOutputControl,
				KHtCS42L51DACOutputControl_DACAB_MUTE);

		// - set PDN bit
		iSelf->Write(KHwCS42L51PwrCtrl, KHtCS42L51PwrCtrl_PDN);

#ifdef _DEBUG
		TInt r = iSelf->StopWrite();
		__ASSERT_DEBUG(r == KErrNone, Kern::Printf("Coulnd't power down the CODEC r=%d ", r));
		__ASSERT_DEBUG(r == KErrNone, Kern::Fault(KCodecPanicCat, __LINE__));
#else
		iSelf->StopWrite();
#endif

		// put !reset back to low..
		GPIO::SetOutputState(KCodecResetPin, GPIO::ELow);
		}
	}


// this is an internal method - to synchronously write the data to the bus. It sets up the transfer
// and waits for Interrupt - which puts back the CS (Chip Select) pin - to low after the
// data was sent out of the bus.
TInt RCS42AudioCodec::DoWrite(TUint16 aRegAddr, TUint16 aData)
	{
	__KTRACE_SCODEC(Kern::Printf("RCS42AudioCodec::DoWrite()"));
	TInt r = KErrNone;

	iTransBuff().iRegister = aRegAddr;
	iTransBuff().iData = aData;

	// create a transfer object..
	TIicBusTransfer transfer(TIicBusTransfer::EMasterWrite, KBufGranularity, &iTransBuff);

	// Create transaction
	TIicBusTransaction transaction(&iHeaderBuff, &transfer);

	// synchronously queue the write transaction
	r = IicBus::QueueTransaction(iCsiBusConfig, &transaction);
	return r;
	}

// to avoid multiple checking for each Write() call - if they are called in a row,
// precede each block of writes to be checked with StartWrite() which clears the iResult.
void RCS42AudioCodec::StartWrite()
	{
#if _DEBUG
	iStartWriteCalled = ETrue;
#endif
	iResult = KErrNone;
	}

// After each block calls to Write() check the global status of them by calling this method.
TInt RCS42AudioCodec::StopWrite()
	{
#if _DEBUG
	iStartWriteCalled = EFalse;
#endif
	return iResult;
	}

// this is an internal method - used to configure the codec. It can be called
// multiple times - whithout checkin for results. Precondition is - to call StartWrite()
// and the overall result of multiple calls to this method are gathered using StopWrite()
void RCS42AudioCodec::Write(TUint16 aRegAddr, TUint16 aData)
	{
	__ASSERT_DEBUG(iStartWriteCalled, Kern::Printf("block of multiple Write() calls should be preceded with StartWrite()"));
	__ASSERT_DEBUG(iStartWriteCalled, Kern::Fault(KCodecPanicCat, __LINE__));

	if (iResult != KErrNone)
		{
		return; // there was an error during one of previous write calls, just return
		}
	// if all calls proceeding StartWrite() were successful, continue to call the proper write
	iResult = DoWrite(aRegAddr, aData);
	}

// This method is used to configure the CSI interface and then - the Codec.
TInt RCS42AudioCodec::Init()
	{
	__KTRACE_SCODEC(Kern::Printf("RCS42AudioCodec::Init()"));
	TUint32 val;

	// First byte for the Cocec's write request on the CSI bus: (first seven bits-address, 8th-Write idication)
	iTransBuff().iAddress = KCodecWriteCommand;

	// Enter the BusRealisation config specific to CSI
	SET_BUS_TYPE(iCsiBusConfig,DIicBusChannel::ESpi); //Bus Type
	SET_CHAN_NUM(iCsiBusConfig,(TUint8)KCodecCSIChannel); // SPI uses channel numbers 1,2 (CSI0 and CSI1)
	SET_SLAVE_ADDR(iCsiBusConfig,KCodecCSPin); //  Codec Pin Number

	// enable and configure pin 25 - it is connected to the chip's reset line
	GPIO::SetPinMode(KCodecResetPin, GPIO::EEnabled);
	GPIO::SetPinDirection(KCodecResetPin, GPIO::EOutput);
	GPIO::SetDebounceTime(KCodecResetPin, 0);

	//====================================
	//configure the CODEC:
	// put !reset line high to start the power-up sequence..
	GPIO::SetOutputState(KCodecResetPin, GPIO::EHigh);

	// start multiple Write() block here
	StartWrite();

	// Write are used here..
	// power-up sequence..
	Write(KHwCS42L51PwrCtrl, KHtCS42L51PwrCtrl_PDN);
	Write(KHwCS42L51PwrCtrl, KHtCS42L51PwrCtrl_PDN_ALL);

	// freeze all registers.. until are set-up..
	Write(KHwCS42L51DACControl, KHtCS42L51DACControl_FREEZE);

	// Mic power control and speed control (0x03)
	Write(KHwCS42L51MicPwrSpeed, KHtCS42L51MicPwrSpeed_AUTO
			| KHtCS42L51MicPwrSpeed_MCLKDIV2); // auto,

	// interface control (0x04) // serial port settings..
	// I2s, Slave, SDOUT->SDIN internally connected.. Digimix->ON?

	// use I2S format, Slave
	iInterfaceCtrlVal = KHCS42L51CtrlI2sUpto24bit << KHsCS42L51CtrlFormat;

	// Digital & Mic mix
	iInterfaceCtrlVal |= KHtCS42L51Ctrl_DIGMIX | KHtCS42L51Ctrl_MICMIX;
	Write(KHwCS42L51Ctrl, iInterfaceCtrlVal);

	// MIC Control - enable mic pre-amplifierboost (there is also ADC digital boost)
	// which we can use in addition to this one - but it would- work fine whitout any.
	val = KHtCS42L51MicCtrl_MICA_BOOST;
	Write(KHwCS42L51MicCtrl, val);

	// ADCx Input Select, Invert & Mute
	/*
	 PDN_PGAx AINx_MUX[1:0] Selected Path to ADC
	 0 	  00 		AIN1x-->PGAx
	 0	  01 		AIN2x-->PGAx
	 0 	  10 		AIN3x/MICINx-->PGAx
	 0 	  11 		AIN3x/MICINx-->Pre-Amp(+16/+32 dB Gain)-->PGAx
	 1 	  00 		AIN1x
	 1 	  01 		AIN2x
	 1 	  10 		AIN3x/MICINx
	 1 	  11 		Reserved */

	val = 3 << KHsCS42L51ADCInputMute_AINA_MUX;
	val |= 3 << KHsCS42L51ADCInputMute_AINB_MUX;
	Write(KHwCS42L51ADCInputMute, val);

	// DAC output select (0x08)
	// HP_GAIN2 HP_GAIN1 HP_GAIN0 DAC_SNGVOL INV_PCMB INV_PCMA DACB_MUTE DACA_MUTE
	Write(KHwCS42L51DACOutputControl, KHtCS42L51DACOutputControl_DAC_SNGVOL);

	// ALCX & PGAX ctrl, A(0x0A), B (0x0B)
	Write(KHwCS42L51ALC_PGA_A_Control, 0);
	Write(KHwCS42L51ALC_PGA_B_Control, 0);

	// ADCx Mixer Volume Ctrl A(0x0E), B (0x0F)
	Write(KHwCS42L51ALC_ADC_A_MixVolume, KHbCS42L51ALC_Volume_Min);
	Write(KHwCS42L51ALC_ADC_B_MixVolume, KHbCS42L51ALC_Volume_Min);

	// PCMx Volume Ctrl A(0x10), B (0x11)
	Write(KHwCS42L51ALC_PCM_A_MixVolume, 0x18);
	Write(KHwCS42L51ALC_PCM_B_MixVolume, 0x18);

	// Volume Control: AOUTA (Address 16h) & AOUTB (Address 17h)
	Write(KHwCS42L51ALC_Out_A_Volume, KHbCS42L51ALC_Volume_Min);
	Write(KHwCS42L51ALC_Out_B_Volume, KHbCS42L51ALC_Volume_Min);

	// DAC Control (Address 09h)
	//  	7	  	 6	   	 5	     4	 	 3		2	   1	 	0
	// DATA_SEL1 DATA_SEL0 FREEZE Reserved DEEMPH AMUTE DAC_SZC1 DAC_SZC0
	// DATA_SEL1 DATA_SEL0:
	// 00 - PCM Serial Port to DAC
	// 01 - Signal Processing Engine to DAC
	// 10 - ADC Serial Port to DAC    (11 - Reserved)
	Write(KHwCS42L51DACControl, 1<<KHsCS42L51DACControl_DATA_SEL); // also clearing freeze bit will update settings..

	// let's rock!! - boost bass and treble a bit ;)
	// values for nibbles change from 0: +12dB (maximum) boost,
	// to 15:(minimum) -10.5dB boost. 8: 0dB - play 'as it is'
	val = (5 << KHsCS42L51ALC_ToneCtrl_TREB) | (6
			<< KHsCS42L51ALC_ToneCtrl_BASS);
	Write(KHwCS42L51ALC_ToneCtrl, val);

	// power-up sequence..   - clear PDN ..after loading register settings..
	Write(KHwCS42L51PwrCtrl, 0);

	// This will return the error status, if any of Write() was not successful or KErrNone otherwise
	return StopWrite();
	}

// this method is called from the sound-driver thread context to set the requested playback volume
TInt RCS42AudioCodec::SetPlayVolume(TInt aVolume)
	{
	__KTRACE_SCODEC(Kern::Printf("RCS42AudioCodec::SetPlayVolume(%d)"));
	// +12 db = 0001 1000 (24)
	// 0db 	  = 0000 0000 (0)
	//-0.5db  = 1111 1111 (255)
	//-102db  = 0001 1001 (25)
	TUint8 volume = 0xff & (aVolume + KHbCS42L51ALC_Volume_Min);

	StartWrite();
	Write(KHwCS42L51ALC_Out_A_Volume, volume);
	Write(KHwCS42L51ALC_Out_B_Volume, volume);
	TInt r = StopWrite();

	return r;
	}

// this method is called from the sound-driver thread context to set the requested record volume
TInt RCS42AudioCodec::SetRecordVolume(TInt aVolume)
	{
	__KTRACE_SCODEC(Kern::Printf("RCS42AudioCodec::SetRecordVolume(%d)"));
	// +12 db = 0001 1000 (24)
	// 0db 	  = 0000 0000 (0)
	//-0.5db  = 1111 1111 (255)
	//-102db  = 0001 1001 (25)
	TUint8 volume = 0xff & (aVolume + KHbCS42L51ALC_Volume_Min);

	StartWrite();
	Write(KHwCS42L51ALC_ADC_A_MixVolume, volume);
	Write(KHwCS42L51ALC_ADC_B_MixVolume, volume);
	TInt r = StopWrite();

	return r;
	}
