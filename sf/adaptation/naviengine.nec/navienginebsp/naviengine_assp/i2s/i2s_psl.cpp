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
* bsp\nwip_nec_naviengine\naviengine_assp\i2s\i2s_psl.cpp
*
*/



#include "navi_i2s.h"
#include <naviengine.h>

#ifdef __SMP__
static TSpinLock I2sLock = TSpinLock(TSpinLock::EOrderGenericIrqLow2);
#endif

// All channels have the same configuration. 
// base for registers are shifted for each channel by 0x400 (1<<10)
#define HW_CHAN_SHIFT 10

// Registers 
// these macros will return register address for a given interface
#define KHwI2SControl(intefaceId)		((KHwBaseI2S0 + KHoI2SCtrl)     + (intefaceId << HW_CHAN_SHIFT))
#define KHwI2SFifoControl(intefaceId) 	((KHwBaseI2S0 + KHoI2SFifoCtrl) + (intefaceId << HW_CHAN_SHIFT))
#define KHwI2SFifoStatus(intefaceId) 	((KHwBaseI2S0 + KHoI2SFifoSts)  + (intefaceId << HW_CHAN_SHIFT))
#define KHwI2SInterruptFlag(intefaceId) ((KHwBaseI2S0 + KHoI2SIntFlg)   + (intefaceId << HW_CHAN_SHIFT))
#define KHwI2SInterruptMask(intefaceId)	((KHwBaseI2S0 + KHoI2SIntMask)  + (intefaceId << HW_CHAN_SHIFT))
#define KHwI2SDataIn(intefaceId) 		((KHwBaseI2S0 + KHoI2SRx)       + (intefaceId << HW_CHAN_SHIFT))
#define KHwI2SDataOut(intefaceId) 		((KHwBaseI2S0 + KHoI2STx)       + (intefaceId << HW_CHAN_SHIFT))

#define AsspIsBitSet(addr, bit)	     (AsspRegister::Read32(addr)& (bit))
#define CLEARMASK(shift,len)         (((1 << len) - 1) << shift)
#define AsspGetBits(w,shift,len)     ((AsspRegister::Read32(w) >> shift) & ((1 << (len)) - 1))
#define AsspSetBits(w,set,shift,len) (AsspRegister::Modify32(w, (CLEARMASK(shift, len)) , (set << shift)))


NONSHARABLE_CLASS(D2sChannelNE1_TB) : public DI2sChannelBase
	{
public:
	D2sChannelNE1_TB(TInt aInterfaceId);
	
	virtual TInt ConfigureInterface(TDes8* aConfig);
	virtual TInt GetInterfaceConfiguration(TDes8& aConfig);
	virtual TInt SetSamplingRate(TInt aSamplingRate);
	virtual TInt GetSamplingRate(TInt& aSamplingRate);
	virtual TInt SetFrameLengthAndFormat(TInt aFrameLength, TInt aLeftFramePhaseLength);
	virtual TInt GetFrameFormat(TInt& aLeftFramePhaseLength, TInt& aRightFramePhaseLength);
	virtual TInt SetSampleLength(TInt aFramePhase, TInt aSampleLength);
	virtual TInt GetSampleLength(TInt aFramePhase, TInt& aSampleLength);
	virtual TInt SetDelayCycles(TInt aFramePhase, TInt aDelayCycles);
	virtual TInt GetDelayCycles(TInt aFramePhase, TInt& aDelayCycles);
	virtual TInt ReadReceiveRegister(TInt aFramePhase, TInt& aData);
	virtual TInt WriteTransmitRegister(TInt aFramePhase, TInt aData);
	virtual TInt ReadTransmitRegister(TInt aFramePhase, TInt& aData);
	virtual TInt ReadRegisterModeStatus(TInt aFramePhase, TInt& aFlags);
	virtual TInt EnableRegisterInterrupts(TInt aFramePhase, TInt aInterrupt);
	virtual TInt DisableRegisterInterrupts(TInt aFramePhase, TInt aInterrupt);
	virtual TInt IsRegisterInterruptEnabled(TInt aFramePhase, TInt& aEnabled);
	virtual TInt EnableFIFO(TInt aFramePhase, TInt aFifoMask);
	virtual TInt DisableFIFO(TInt aFramePhase, TInt aFifoMask);
	virtual TInt IsFIFOEnabled(TInt aFramePhase, TInt& aEnabled);
	virtual TInt SetFIFOThreshold(TInt aFramePhase, TInt aDirection, TInt aThreshold);
	virtual TInt ReadFIFOModeStatus(TInt aFramePhase, TInt& aFlags);
	virtual TInt EnableFIFOInterrupts(TInt aFramePhase, TInt aInterrupt);
	virtual TInt DisableFIFOInterrupts(TInt aFramePhase, TInt aInterrupt);
	virtual TInt IsFIFOInterruptEnabled(TInt aFramePhase, TInt& aEnabled);
	virtual TInt ReadFIFOLevel(TInt aFramePhase, TInt aDirection, TInt& aLevel);
	virtual TInt EnableDMA(TInt aFifoMask);
	virtual TInt DisableDMA(TInt aFifoMask);
	virtual TInt IsDMAEnabled(TInt& aEnabled);
	virtual TInt Start(TInt aDirection);
	virtual TInt Stop(TInt aDirection);
	virtual TInt IsStarted(TInt aDirection, TBool& aStarted);		

private: 
	TBool iConfigured;
	TInt iLastPhaseInWriteFifo;
	TInt iLastPhaseInReadFifo;
	};

// this static method, creates the DI2sChannelBase corresponding
// to the interfaceId passed and return the channel
// NB: if each channel was implemented independently (e.g. on a separate file)
// this function would have to be provided spearately and know how to map
// the interface ID to the appropriate channel object to be created
// (e.g. each channel would have a different implementation D2sChannelXXX
// and this function call the appropriate constructor for each interface Id)

TInt DI2sChannelBase::CreateChannels(DI2sChannelBase*& aChannel, TInt aInterfaceId)
	{
	DI2sChannelBase* chan = new D2sChannelNE1_TB(aInterfaceId);
	if (!chan)
		{
		return KErrNoMemory;
		}

	aChannel = chan;
	
	return KErrNone;
	}


// Default constructor.
D2sChannelNE1_TB::D2sChannelNE1_TB(TInt aInterfaceId):
	iConfigured(EFalse), 
	iLastPhaseInWriteFifo(I2s::ERight), // the first phase in write fifo should be left, so initialize to right 
	iLastPhaseInReadFifo(I2s::ERight) // the first phase in read fifo should be left, so initialize to right
	{
	iInterfaceId=aInterfaceId;
	}

TInt D2sChannelNE1_TB::ConfigureInterface(TDes8 *aConfig)
	{
	__KTRACE_OPT(KDLL, Kern::Printf("DI2sChannelNE1_TB::ConfigureInterfaceInterface (Id: %d)", iInterfaceId));

	if(AsspIsBitSet(KHwI2SControl(iInterfaceId), KHtI2SCtrl_TEN | KHtI2SCtrl_REN))
		{
		return KErrInUse;
		}
	
	TI2sConfigBufV01 &conf = ((TI2sConfigBufV01&)*aConfig);

	// this interface doesn's support EController mode
	if (conf().iType == I2s::EController)
		{
		return KErrNotSupported;
		}
	
	if(conf().iRole == I2s::EMaster)
		{
		AsspRegister::Modify32(KHwI2SControl(iInterfaceId), 0, KHtI2SCtrl_MSMODE);
		}
		
	// copy configuration.. it will be used in Start/Stop
	iConfig = conf();		// ok, thread context only and one client thread per channel
	
	 // select I2S format
	AsspSetBits(KHwI2SControl(iInterfaceId), 4, KHsI2SCtrl_FORMAT, 3);

	iConfigured = ETrue;	// this API can only be called in thread context and we assume that a channel is for use of a single thread
	
	return KErrNone;
	}

TInt D2sChannelNE1_TB::GetInterfaceConfiguration(TDes8 &aConfig)
	{
	if (!iConfigured)	// this API can only be called in thread context and we assume that a channel is for use of a single thread
		{
		aConfig.SetLength(0); //no configuration  present yet..
		}
	else
		{
		TPckgBuf<TI2sConfigV01> conf(iConfig);
		aConfig.Copy(conf);
		}
	return KErrNone;
	}

TInt D2sChannelNE1_TB::SetSamplingRate(TInt aSamplingRate)
	{
	if (iConfig.iRole == I2s::ESlave)
		{
		return KErrNotSupported; 
		}
	
	if (AsspIsBitSet(KHwI2SControl(iInterfaceId), KHtI2SCtrl_TEN | KHtI2SCtrl_REN))
		{
		return KErrInUse;
		}
	
	TUint32 val = 0, div = 0;
	
	switch(aSamplingRate)
		{
		case I2s::E8KHz: 		// 0000: 8 kHz
			div = 1; 			// MCLK = 24.5760 MHz(1*)
			break;

		case I2s::E11_025KHz:	// 1000: 11.025 kHz
			val = 8;
			div = 4; 			// MCLK = 33.8688 MHz(4*) or 22.5792(5*) or 16.9344 MHz(6)
			break;

		case I2s::E12KHz:		// 0001: 12 kHz
			val = 1;
			div = 1; 			// MCLK = 24.5760 MHz(1*) or 18.432MHz(2)
			break;

		case I2s::E16KHz:		// 0010: 16 kHz
			val = 2;
			div = 1; 			// MCLK = 24.5760 MHz(1*)
			break;

		case I2s::E22_05KHz:	// 1001: 22.05 kHz
			val = 9;
			div = 4; 			// MCLK = 33.8688 MHz(4*) or 22.5792(5*) or 16.9344 MHz(6)  
			break;
			
		case I2s::E24KHz:		// 0011: 24 kHz
			val = 3;
			div = 1; 			// MCLK = 24.5760 MHz(1*) or 18.432MHz(2)
			break;
			
		case I2s::E32KHz:		// 	0100: 32 kHz
			val = 4;
			div = 1; 			// MCLK = 24.5760 MHz
			break;
			
		case I2s::E44_1KHz:		// 1010: 44.1 kHz
			val = 10;
			div = 4;			// MCLK = 33.8688 MHz(4*) or 22.5792(5*) or 16.9344 MHz(6)
			break;
			
		case I2s::E48KHz:		// 0101: 48 kHz
			val = 5;
			div = 2; 			// MCLK = 24.5760 MHz(1*) or 18.432MHz(2)
			break;

		default:
			return KErrNotSupported;
		}

	TInt irq=__SPIN_LOCK_IRQSAVE(I2sLock);		// seems that we must guarantee the following sequence is uninterrupted...
	// before changing FSCLKSEL and/or FSMODE - mask I2S bit
	// MSK_I2Sx (x=0:3): bits 18:21 in MaskCtrl Register of the System Ctrl Unit 
	AsspRegister::Modify32(KHwBaseSCU + KHoSCUClockMaskCtrl, 0, (1<<iInterfaceId)<<18);
	
	// change the divide I2SCLK ctrl value for this channel..
	AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, 0xf<<(iInterfaceId<<3), div<<(iInterfaceId<<3));  

	// update the KHwI2SControl register
	AsspSetBits(KHwI2SControl(iInterfaceId), val, KHsI2SCtrl_FCKLKSEL, 4); 

	// after changing FSCLKSEL and FSMODE - clear MSK_I2Sx mask bit
	AsspRegister::Modify32(KHwBaseSCU + KHoSCUClockMaskCtrl, (1<<iInterfaceId)<<18, 0);
	__SPIN_UNLOCK_IRQRESTORE(I2sLock,irq);

	return KErrNone;
	}

TInt D2sChannelNE1_TB::GetSamplingRate(TInt& aSamplingRate)
	{
	if (iConfig.iRole == I2s::ESlave)
		{
		return KErrNotSupported; 
		}
	
	TUint32 val = (AsspRegister::Read32(KHwI2SControl(iInterfaceId))>>KHsI2SCtrl_FCKLKSEL) & 0xf;
	
	switch(val)
		{
		case 0:
			aSamplingRate = I2s::E8KHz;		// 0000: 8 kHz
			break;

		case 8: 
			aSamplingRate = I2s::E11_025KHz;	// 1000: 11.025 kHz
			break;

		case 1:
			aSamplingRate = I2s::E12KHz;		// 0001: 12 kHz
			break;

		case 2:
			aSamplingRate = I2s::E16KHz;		// 0010: 16 kHz
			break;

		case 9:
			aSamplingRate = I2s::E22_05KHz;		// 1001: 22.05 kHz
			break;
			
		case 3:
			aSamplingRate = I2s::E24KHz;		// 0011: 24 kHz
			break;
			
		case 4:
			aSamplingRate = I2s::E32KHz;		// 	0100: 32 kHz
			break;
			
		case 10:
			aSamplingRate = I2s::E44_1KHz;		// 1010: 44.1 kHz
			break;
			
		case 5:
			aSamplingRate = I2s::E48KHz;		// 0101: 48 kHz
			break;
		}
	
	return KErrNone;
	}

TInt D2sChannelNE1_TB::SetFrameLengthAndFormat(TInt aFrameLength, TInt /*aLeftFramePhaseLength*/)
	{
	if (AsspIsBitSet(KHwI2SControl(iInterfaceId), KHtI2SCtrl_TEN | KHtI2SCtrl_REN))
		{
		return KErrInUse;
		}
	
	TUint32 val=0;
	
	switch(aFrameLength)
		{
		case I2s::EFrame32Bit:
			val = 0;
			break;
		
		case I2s::EFrame48Bit: 
			val = 1;
			break;
		
		case I2s::EFrame64Bit:
			val = 2;
			break;

		case I2s::EFrame128Bit:
			val = 3;
			break;

		default:
			return KErrNotSupported;
		};
	
	TInt irq=__SPIN_LOCK_IRQSAVE(I2sLock);		// seems that we must guarantee the following sequence is uninterrupted
	// before changing FSCLKSEL and/or FSMODE - mask I2S bit
	// MSK_I2Sx (x=0:3): bits 18:21 in MaskCtrl Register of the System Ctrl Unit 
	AsspRegister::Modify32(KHwBaseSCU + KHoSCUClockMaskCtrl, 0, (1<<iInterfaceId)<<18);
	
	// update the register (3 bits at KHsI2SCtrl_FSMODE)
	AsspSetBits(KHwI2SControl(iInterfaceId), val, KHsI2SCtrl_FSMODE, 3);

	// after changing FSCLKSEL and FSMODE - clear MSK_I2Sx mask bit
	AsspRegister::Modify32(KHwBaseSCU + KHoSCUClockMaskCtrl, (1<<iInterfaceId)<<18, 0);
	__SPIN_UNLOCK_IRQRESTORE(I2sLock,irq);
	
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::GetFrameFormat(TInt& aLeftFramePhaseLength, TInt& aRightFramePhaseLength)
	{
	TInt frameLength=0;
	switch(AsspGetBits(KHwI2SControl(iInterfaceId), KHsI2SCtrl_FSMODE, 3))
		{
		case 0:
			frameLength = I2s::EFrame32Bit;
			break;
		
		case 1:
			frameLength = I2s::EFrame48Bit;
			break;
		
		case 2: 
			frameLength = I2s::EFrame64Bit;
			break;

		case 3:
			frameLength = I2s::EFrame128Bit;
			break;

		default:
			return KErrGeneral; //unexpected value??
		};

	aLeftFramePhaseLength=aRightFramePhaseLength=frameLength/2;	// on NaviEngine frames are symmetrical

	return KErrNone; 
	}

TInt D2sChannelNE1_TB::SetSampleLength(TInt /*aFramePhase*/, TInt aSampleLength)
	{
	if (AsspIsBitSet(KHwI2SControl(iInterfaceId), KHtI2SCtrl_TEN | KHtI2SCtrl_REN))
		{
		return KErrInUse;
		}

	TUint32 val=0;
	
	switch(aSampleLength)
		{
		case I2s::ESample8Bit:
			val = 0x8;
			break;
			
		case I2s::ESample16Bit:
			val = 0x10;
			break;
		
		case I2s::ESample24Bit:
			val = 0x18;
			break;

		case I2s::ESample12Bit:
		case I2s::ESample32Bit:
			return KErrNotSupported;
		}
	
	// update the register.. 
	AsspSetBits(KHwI2SControl(iInterfaceId), val, KHsI2SCtrl_DLENGTH,5); // [4:0] sampling data length);
	
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::GetSampleLength(TInt /*aFramePhase*/, TInt& aSampleLength)
	{
	// sample length can't be configured separately for left/right channels..
	// .. in this chip, so aFramePhase is ignored
	
	TUint32 val=AsspRegister::Read32(KHwI2SControl(iInterfaceId)) & 0x1F;
	
	switch(val)
		{
		case 0x8:
			aSampleLength = I2s::ESample8Bit;
			break;
			
		case 0x10: 
			aSampleLength = I2s::ESample16Bit;
			break;
		
		case 0x18: 
			aSampleLength = I2s::ESample24Bit;
			break;
		}
	
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::SetDelayCycles(TInt aFramePhase, TInt aDelayCycles)
	{
	return KErrNotSupported; 
	}

TInt D2sChannelNE1_TB::GetDelayCycles(TInt aFramePhase, TInt& aDelayCycles)
	{
	return KErrNotSupported; 
	}

TInt D2sChannelNE1_TB::ReadReceiveRegister(TInt aFramePhase, TInt& aData)
	{
	// should check here, if sample length was configured, 
	// but once the interface is properly configured, this check would add unnecessary overhead.
	
	// since there is only one fifo for both channels in this chip, 
	// we need to read Left and then Right channel data in sequence. 

	TInt oldFP=__e32_atomic_swp_ord32(&iLastPhaseInReadFifo,aFramePhase);	// atomic as this may be used in ISR too
	if(aFramePhase == oldFP)
		{
		return KErrNotSupported;
		}

	// and get the current data from the fifo register
	aData = AsspRegister::Read32(KHwI2SDataIn(iInterfaceId));
	return KErrNone; 	
	}

TInt D2sChannelNE1_TB::WriteTransmitRegister(TInt aFramePhase, TInt aData)
	{
	// should check here, if sample length was configured, 
	// but once the interface is properly configured, this check would add unnecessary overhead.
	
	// since there is only one fifo for both channels in this chip, 
	// we need to write Left and then right channel data in sequence. 

	TInt oldFP=__e32_atomic_swp_ord32(&iLastPhaseInWriteFifo,aFramePhase);	// atomic as this may be used in ISR too
	if(aFramePhase == oldFP)
		{
		return KErrNotSupported;
		}

	// and update the fifo register
	AsspRegister::Write32(KHwI2SDataOut(iInterfaceId), aData);

	return KErrNone; 
	}

TInt D2sChannelNE1_TB::ReadTransmitRegister(TInt aFramePhase, TInt& aData)
	{
	// since there is only one fifo for both channels in this chip, 
	// we can only read the data for the last written phase (either left or right) 

	TInt curFp=__e32_atomic_load_acq32(&iLastPhaseInWriteFifo);
	if(aFramePhase != curFp)
		{
		return KErrArgument;
		}
	
	// read the register
	aData = AsspRegister::Read32(KHwI2SDataOut(iInterfaceId));
	
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::ReadRegisterModeStatus(TInt aFramePhase, TInt& aFlags)
	{
	return KErrNotSupported; // register PIO mode not supported (FIFO always present)
	}

TInt D2sChannelNE1_TB::EnableRegisterInterrupts(TInt aFramePhase, TInt aInterrupt)
	{
	return KErrNotSupported; // register PIO mode not supported (FIFO always present)
	}

TInt D2sChannelNE1_TB::DisableRegisterInterrupts(TInt aFramePhase, TInt aInterrupt)
	{
	return KErrNotSupported; // register PIO mode not supported (FIFO always present)
	}

TInt D2sChannelNE1_TB::IsRegisterInterruptEnabled(TInt aFramePhase, TInt& aEnabled)
	{
	return KErrNotSupported; // register PIO mode not supported (FIFO always present)
	}

TInt D2sChannelNE1_TB::EnableFIFO(TInt /*aFramePhase*/, TInt aFifoMask)
	{
	TInt val=0;
	
	// Set and clear fifo init bits for transmit/receive
	// tere are only two FIFOs - not separated to phases..
	if(aFifoMask & I2s::ETx) 
		val = KHtI2SFifoCtrl_TFIFOCLR;

	if(aFifoMask & I2s::ERx) 
		val |= KHtI2SFifoCtrl_RFIFOCLR;
			
	AsspRegister::Modify32(KHwI2SFifoControl(iInterfaceId), 0,  val);
	AsspRegister::Modify32(KHwI2SFifoControl(iInterfaceId), val, 0);

	return KErrNone; 
	}

TInt D2sChannelNE1_TB::DisableFIFO(TInt aFramePhase, TInt aFifoMask)
	{
	// fifo is always enabled in this chip.. 
	return KErrNotSupported; 
	}

TInt D2sChannelNE1_TB::IsFIFOEnabled(TInt /*aFramePhase*/, TInt& aEnabled)
	{
	// fifo is always enabled in this chip.. 
	aEnabled = I2s::ERx|I2s::ETx;
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::SetFIFOThreshold(TInt aFramePhase, TInt aDirection, TInt aThreshold)
	{
	// supported threshold values for this chip: 
	// 011b - 16-word space available
	// 010b - 8-word space available
	// 001b - 4-word space available
	// 000b - 2-word space available
	if (aThreshold < 0)
		{
		return KErrNotSupported;
		}

	// Determine, what value was specified, and adjust it down to one  
	// of the possible values
	TInt i=15;
	while(i>1)
		{
		if ( aThreshold!=(aThreshold & i))
			break;
		i>>=1;
		}
	aThreshold = i+1; //this will now contain one of possible values (2-16);
	
	// now any of 16/8/4/2, shifted right until == 0 will give us requested register_value+2
	// (instead of using 'switch(aThreshold)') 
	i=0;
	while(aThreshold)
		{
		aThreshold>>=1;
		++i;
		}
	aThreshold = i-2 >= 0 ? i-2 : 0; //if i-2 gives <0 (e.g.for aThreshold<4) -adjust to 0;
	
	if (AsspIsBitSet(KHwI2SControl(iInterfaceId), KHtI2SCtrl_TEN | KHtI2SCtrl_REN))
		{
		return KErrInUse;
		}

	TUint32 clr, set;
	if(aDirection & I2s::ERx)  // receive fifo..
		{
		if (aFramePhase == I2s::ELeft)   	
			{
			clr = CLEARMASK(KHsI2SFifoCtrl_RFIFOLT, 3);
			set = (aThreshold << KHsI2SFifoCtrl_RFIFOLT);
			}

		else //if (aFramePhase == I2s::ERight)
			{
			clr = CLEARMASK(KHsI2SFifoCtrl_RFIFORT, 3);
			set = (aThreshold << KHsI2SFifoCtrl_RFIFORT);
			}
		}
	else 	// transmit fifo..
		{
		if (aFramePhase == I2s::ELeft)  
			{
			clr = CLEARMASK(KHsI2SFifoCtrl_TFIFOLT, 3);
			set = (aThreshold << KHsI2SFifoCtrl_TFIFOLT);

			}
		else // if (aFramePhase == I2s::ERight)
			{
			clr = CLEARMASK(KHsI2SFifoCtrl_TFIFORT, 3);
			set = (aThreshold << KHsI2SFifoCtrl_TFIFORT);
			}
		}
	// updating register with value will also clear FIFO initialization bits.. 
	AsspRegister::Modify32(KHwI2SFifoControl(iInterfaceId), clr, set);

	return KErrNone; 
	}

TInt D2sChannelNE1_TB::ReadFIFOModeStatus(TInt aFramePhase, TInt& aFlags)
	{
	aFlags = 0;

	TInt irq=__SPIN_LOCK_IRQSAVE(I2sLock);		// (read and clear), can be used in ISR
	TUint32 flags = AsspRegister::Read32(KHwI2SInterruptFlag(iInterfaceId));
	AsspRegister::Write32(KHwI2SInterruptFlag(iInterfaceId), flags);	// clear the status flags (after reading)
	__SPIN_UNLOCK_IRQRESTORE(I2sLock,irq);
	
	if (aFramePhase == I2s::ELeft)
		{
		if(flags & KHtI2SIntFlg_TFLURINT)	// L-ch transmit FIFO underrun
			aFlags |= I2s::ETxUnderrun;

		if(flags & KHtI2SIntFlg_TFLEINT)	// L-ch transmit FIFO reached the empty trigger level
			aFlags |= I2s::ETxEmpty;

		if(flags & KHtI2SIntFlg_RFLORINT)	// L-ch receive FIFO overrun
			aFlags |= I2s::ERxOverrun;
		
		if(flags & KHtI2SIntFlg_RFLFINT)	// L-ch receive FIFO reached the full trigger level
			aFlags |= I2s::ERxFull;
		}
	else
		{
		if(flags & KHtI2SIntFlg_TFRURINT)	// R-ch transmit FIFO underrun
			aFlags |= I2s::ETxUnderrun;

		if(flags & KHtI2SIntFlg_TFREINT)	// R-ch transmit FIFO reached the empty trigger level
			aFlags |= I2s::ETxEmpty;

		if(flags & KHtI2SIntFlg_RFRORINT)	// R-ch receive FIFO overrun
			aFlags |= I2s::ERxOverrun;
		
		if(flags & KHtI2SIntFlg_RFRFINT)	// R-ch receive FIFO reached the full trigger level
			aFlags |= I2s::ERxFull;
		}
	
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::EnableFIFOInterrupts(TInt aFramePhase, TInt aInterrupt)
	{
	TUint32 val=0;
	
	if(aInterrupt & I2s::ERxFull)
		{
		if (aFramePhase == I2s::ELeft)
			val |= KHtI2SIntMask_RFLFINT;

		if (aFramePhase == I2s::ERight)
			val |= KHtI2SIntMask_RFRFINT;
		}

	if(aInterrupt & I2s::ETxEmpty)
		{
		if (aFramePhase == I2s::ELeft)
			val |= KHtI2SIntMask_TFLEINT;

		if (aFramePhase == I2s::ERight)
			val |= KHtI2SIntMask_TFREINT;
		}

	if(aInterrupt & I2s::ERxOverrun)
		{
		if (aFramePhase == I2s::ELeft)
			val |= KHtI2SIntMask_RFLORINT;

		if (aFramePhase == I2s::ERight)
			val |= KHtI2SIntMask_RFRORINT;
		}

	if(aInterrupt & I2s::ETxUnderrun)
		{
		if (aFramePhase == I2s::ELeft)
			val |= KHtI2SIntMask_TFLURINT;

		if (aFramePhase == I2s::ERight)
			val |= KHtI2SIntMask_TFRURINT;
		}
	
	if(aInterrupt & I2s::EFramingError)
		{
		// not supported, do nothing
		}

	if (val)
		{
		// update the register
		AsspRegister::Write32(KHwI2SInterruptMask(iInterfaceId), val);
		}
	
	return KErrNone;
	}

TInt D2sChannelNE1_TB::DisableFIFOInterrupts(TInt aFramePhase, TInt aInterrupt)
	{
	TUint32 val = KHmI2SIntMask_ALL;
	
	if(aInterrupt & I2s::ERxFull)
		{
		if (aFramePhase == I2s::ELeft)
			val &= ~KHtI2SIntMask_RFLFINT;

		if (aFramePhase == I2s::ERight)
			val &= ~KHtI2SIntMask_RFRFINT;
		}

	if(aInterrupt & I2s::ETxEmpty)
		{
		if (aFramePhase == I2s::ELeft)
			val &= ~KHtI2SIntMask_TFLEINT;

		if (aFramePhase == I2s::ERight)
			val &= ~KHtI2SIntMask_TFREINT;
		}

	if(aInterrupt & I2s::ERxOverrun)
		{
		if (aFramePhase == I2s::ELeft)
			val &= ~KHtI2SIntMask_RFLORINT;

		if (aFramePhase == I2s::ERight)
			val &= ~KHtI2SIntMask_RFRORINT;
		}

	if(aInterrupt & I2s::ETxUnderrun)
		{
		if (aFramePhase == I2s::ELeft)
			val &= ~KHtI2SIntMask_TFLURINT;

		if (aFramePhase == I2s::ERight)
			val &= ~KHtI2SIntMask_TFRURINT;
		}
	
	if(aInterrupt & I2s::EFramingError)
		{
		// not supported, do nothing
		}

	TInt irq=__SPIN_LOCK_IRQSAVE(I2sLock);		// (read and clear), not used from ISR but made safe nevertheless...
	TUint32 oldVal = AsspRegister::Read32(KHwI2SInterruptMask(iInterfaceId));
	if (val!=oldVal)
		{
		// update the register
		AsspRegister::Write32(KHwI2SInterruptMask(iInterfaceId), val);
		}
	__SPIN_UNLOCK_IRQRESTORE(I2sLock,irq);
	
	return KErrNone;
	}

TInt D2sChannelNE1_TB::IsFIFOInterruptEnabled(TInt aFramePhase, TInt& aEnabled)
	{
	// check, if any interrupt is enabled.. 
	TUint32 val = AsspRegister::Read32(KHwI2SInterruptMask(iInterfaceId));

	aEnabled=0;
	if(aFramePhase== I2s::ELeft)
		{
		if(val & KHtI2SIntMask_TFLURINT)// L-ch transmit FIFO underrun int enable
			aEnabled |= I2s::ETxUnderrun;
		if(val & KHtI2SIntMask_TFLEINT)// L-ch transmit FIFO reached the empty trigger level int enable
			aEnabled |= I2s::ETxEmpty;
		if(val & KHtI2SIntMask_RFLORINT)// L-ch receive FIFO overrun int enable
			aEnabled |= I2s::ERxOverrun;
		if(val & KHtI2SIntMask_RFLFINT)// L-ch receive FIFO reached the full trigger level int enable
			aEnabled |= I2s::ERxFull;
		}
	else
		{
		if(val & KHtI2SIntMask_TFRURINT)// R-ch transmit FIFO underrun int enable
			aEnabled |= I2s::ETxUnderrun;
		if(val & KHtI2SIntMask_TFREINT)// R-ch transmit FIFO reached the empty trigger level int enable
			aEnabled |= I2s::ETxEmpty;
		if(val & KHtI2SIntMask_RFRORINT)// R-ch receive FIFO overrun int enable
			aEnabled |= I2s::ERxOverrun;
		if(val & KHtI2SIntMask_RFRFINT)// R-ch receive FIFO reached the full trigger level int enable
			aEnabled |= I2s::ERxFull;
		}
		
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::ReadFIFOLevel(TInt aFramePhase, TInt aDirection, TInt& aLevel)
	{
	// This device only allows us to see, if the receive FIFO or transmit FIFO is either 
	// empty or full, so return: 
	// TX FIFO: 1-FULL,  0-EMPTY, 
	// RX FIFO: 1-EMPTY, 0-FULL
	
	TUint32 val = AsspRegister::Read32(KHwI2SFifoStatus(iInterfaceId));

	if(aDirection & I2s::ETx)
		{
		if (aFramePhase == I2s::ELeft)
			{
			aLevel = (val & KHtI2SFifoSts_TFL_FULL) >> 17;  // L-ch transmit FIFO full (bit 17)
			}
		else // I2s::ERight
			{
			aLevel = (val & KHtI2SFifoSts_TFR_FULL) >> 16;  // R-ch transmit FIFO full (bit 16)
			}
		}
	else  // I2s::ERx
		{
		if (aFramePhase == I2s::ELeft)
			{
			aLevel = (val & KHtI2SFifoSts_RFL_EMPTY) >> 1;  // L-ch receive FIFO empty (bit 1)
			}
		else // I2s::ERight
			{
			aLevel = val & KHtI2SFifoSts_RFR_EMPTY;  // R-ch receive FIFO empty (bit 0)
			}
		}
	
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::EnableDMA(TInt aFifoMask)
	{
	TUint32 val=0;

	if(aFifoMask & I2s::ETx)
		val |= KHtI2SFifoCtrl_TDMAEN;

	if(aFifoMask & I2s::ERx)
		val |= KHtI2SFifoCtrl_RDMAEN;
	
	AsspRegister::Modify32(KHwI2SFifoControl(iInterfaceId), 0, val);

	return KErrNone; 
	}

TInt D2sChannelNE1_TB::DisableDMA(TInt aFifoMask)
	{
	TUint32 val=0;

	if(aFifoMask & I2s::ETx)
		val |= KHtI2SFifoCtrl_TDMAEN;

	if(aFifoMask & I2s::ERx)
		val |= KHtI2SFifoCtrl_RDMAEN;
	
	AsspRegister::Modify32(KHwI2SFifoControl(iInterfaceId), val, 0);

	return KErrNone; 
	}

TInt D2sChannelNE1_TB::IsDMAEnabled(TInt& aEnabled)
	{ 
	TUint32 val = AsspRegister::Read32(KHwI2SFifoControl(iInterfaceId));
	aEnabled = 0;
	
	if(val & KHtI2SFifoCtrl_TDMAEN)
		aEnabled |= I2s::ETx;

	if(val & KHtI2SFifoCtrl_RDMAEN)
		aEnabled |= I2s::ERx;
	
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::Start(TInt aDirection)
	{
	TUint32 val=0;
	
	if(aDirection & I2s::ERx)  
		{
		// check, if the interface was configured for reception.. 
		if (iConfig.iType != I2s::EReceiver && iConfig.iType != I2s::EBidirectional)
			{
			return KErrNotSupported;
			}

		__KTRACE_OPT(KDLL, Kern::Printf("I2S channel %d: Start Rx", iInterfaceId));
		val |= KHtI2SCtrl_REN;	
		}
	else
		{
		// check, if the interface was configured for transmission.. 
		if (iConfig.iType != I2s::ETransmitter && iConfig.iType != I2s::EBidirectional)
			{
			return KErrNotSupported;
			}
		
		__KTRACE_OPT(KDLL, Kern::Printf("I2S channel %d: Start Tx", iInterfaceId));
		val |= KHtI2SCtrl_TEN;
		}

	// update the register
	AsspRegister::Modify32(KHwI2SControl(iInterfaceId), 0, val);

	return KErrNone; 
	}

TInt D2sChannelNE1_TB::Stop(TInt aDirection)
	{
	TUint32 val=0;
	
	if(aDirection & I2s::ERx)  
		{
		__KTRACE_OPT(KDLL, Kern::Printf("I2S channel %d: Stop Rx", iInterfaceId));
		val = KHtI2SCtrl_REN;	
		}
	else
		{
		__KTRACE_OPT(KDLL, Kern::Printf("I2S channel %d: Stop Rx", iInterfaceId));
		val = KHtI2SCtrl_TEN;
		}

	// update the register
	AsspRegister::Modify32(KHwI2SControl(iInterfaceId), val, 0);
	
	return KErrNone; 
	}

TInt D2sChannelNE1_TB::IsStarted(TInt aDirection, TBool& aStarted)
	{
	TUint32 val=AsspRegister::Read32(KHwI2SControl(iInterfaceId));
	
	if(aDirection & I2s::ERx)
		{
		aStarted = (val & KHtI2SCtrl_REN) >> 24; // bit 24
		}
	else
		{
		aStarted = (val & KHtI2SCtrl_TEN) >> 25; // bit 25
		}
	return KErrNone; 
	}


