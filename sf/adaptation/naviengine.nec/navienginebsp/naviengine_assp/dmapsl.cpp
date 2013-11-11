/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* naviengine_assp\dmapsl.cpp
* DMA Platform Specific Layer (PSL) for Navi Engine.
*
*/




#include <kernel/kern_priv.h>
#include <assp.h>
#include <naviengine_priv.h>
#include <navienginedma.h>	
#include <dma.h>

// Debug support
static const char KDmaPanicCat[] = "DMA PSL";
static const TInt KDesCount = 1024;	// DMA descriptor count - sufficient to serve all channels at the time. 

/* Maps logical DMA channels into physical ones */
static const TDMAChannelLocator KDMAChannelLocator[EDmaChannelCount]=
    {
  // controller, 	 group,	subchannel   DCHS (exc. SEL) TransferShiftSize
	{EDMACtrl32  	,2     ,2			,KHvDMACHC_SDR,		1}, //EDMAChannelSD0,
	{EDMACtrl32  	,2     ,3			,KHvDMACHC_SDW,		1},	//EDMAChannelSD1,

	{EDMACtrl32  	,3     ,0			,KHvDMACHC_I2SR,	1},	//EDMAChannelI2S0RX,
	{EDMACtrl32  	,3     ,1			,KHvDMACHC_I2SW,	1},	//EDMAChannelI2S0TX,
	{EDMACtrl32  	,3     ,2			,KHvDMACHC_I2SR,	1},	//EDMAChannelI2S1RX,
	{EDMACtrl32  	,3     ,3			,KHvDMACHC_I2SW,	1},	//EDMAChannelI2S1TX,
	{EDMACtrl32  	,3     ,4			,KHvDMACHC_I2SR,	1},	//EDMAChannelI2S2RX,
	{EDMACtrl32  	,3     ,5			,KHvDMACHC_I2SW,	1},	//EDMAChannelI2S2TX,
	{EDMACtrl32  	,3     ,6			,KHvDMACHC_I2SR,	1},	//EDMAChannelI2S3RX,
	{EDMACtrl32  	,3     ,7			,KHvDMACHC_I2SW,	1},	//EDMAChannelI2S3TX,
	
	{EDMACtrl32  	,0     ,2			,KHvDMACHC_SW,		2},	//EDMAChannelUART0RX,
	{EDMACtrl32  	,0     ,3			,KHvDMACHC_SW,		2},	//EDMAChannelUART0TX,
	{EDMACtrl32  	,0     ,4			,KHvDMACHC_SW,		2},	//EDMAChannelUART1RX,
	{EDMACtrl32  	,0     ,5			,KHvDMACHC_SW,		2},	//EDMAChannelUART1TX,
	{EDMACtrl32  	,0     ,6			,KHvDMACHC_SW,		2},	//EDMAChannelUART2RX,
	{EDMACtrl32  	,0     ,7			,KHvDMACHC_SW,		2},	//EDMAChannelUART2TX,

	{EDMACtrl32  	,0     ,0			,KHvDMACHC_SW,		2},	//EDmaMemToMem0,
	{EDMACtrl32		,0     ,1			,KHvDMACHC_SW,		2},	//EDmaMemToMem1,
	{EDMACtrl32		,2     ,4			,KHvDMACHC_SW,		2},	//EDmaMemToMem2,
	{EDMACtrl32		,2     ,5			,KHvDMACHC_SW,		2},	//EDmaMemToMem3,
    };

/* Maps physical EDMACtrl32 channels into logical ones */
static const int DMAC32_HWChannelsLocator[KDmaHWCtrl32Count][KDmaCtrl32HWSubChannelCount] = 
	{
	{EDmaMemToMem0,EDmaMemToMem1,EDMAChannelUART0RX,EDMAChannelUART0TX,
								EDMAChannelUART1RX,EDMAChannelUART1TX,EDMAChannelUART2RX,EDMAChannelUART2TX},
	{-1,-1,-1,-1,-1,-1,-1,-1},
	{-1,-1,EDMAChannelSD0,EDMAChannelSD1,EDmaMemToMem2,EDmaMemToMem3,-1,-1},
	{ EDMAChannelI2S0RX,EDMAChannelI2S0TX,EDMAChannelI2S1RX,EDMAChannelI2S1TX,
									EDMAChannelI2S2RX,EDMAChannelI2S2TX,EDMAChannelI2S3RX,EDMAChannelI2S3TX},
	{-1,-1,-1,-1,-1,-1,-1,-1},
	};

class TDmaDesc
//
// Hardware DMA descriptor
//
	{
public:
	TPhysAddr iSrcAddr;
	TPhysAddr iDestAddr;
	TInt iCount;	      //Transfer counter in bytes
	};


//
// Test Support
//
//The list of S/W channels to be tested by t_dma
TUint32 TestNEChannels[] = { EDmaMemToMem0, EDmaMemToMem1, EDmaMemToMem2, EDmaMemToMem3};
TDmaTestInfo TestInfo =
	{
	4*KMaxDMAUnitTransferLen, //a word is a unit of transfer for mem-to-mem DMA
	3,	// Word alignement applies fow S/W (mem-to-mem) transfer
	0,  // No need for cookie.
	4,	// The number of S/W DMA channels to test
	TestNEChannels,
	0,
	NULL,
	0,
	NULL
	};

EXPORT_C const TDmaTestInfo& DmaTestInfo()
	{
	return TestInfo;
	}

//
// Helper Functions
//

inline TBool IsHwDesAligned(TAny* aDes)
//
// We do no need H/W descriptors to be aligned as Navi Engine DMA32 cotroller doesn't
// support linked descriptors. Instead, they are linked by S/W. Therefore, the ordinary
// word alignement applies (which is enforced by compiler).
//
	{
	return ((TLinAddr)aDes & 0x3) == 0;
	}

// Channel class.
// For double buffering, we cannot use the provided TDmaDbChannel class because 
// NE DMA has two sets of registers (base & work) - wich is not supported by TDmaDbChannel.
#if defined(NE1_DMA_DOUBLE_BUFFER)
class TNE1DmaChannel : public TDmaChannel
#else
class TNE1DmaChannel : public TDmaSbChannel
#endif
	{
public:
	TNE1DmaChannel();
	void ProcessIrq();
	void ProcessErrorIrq();
	void StopTransfer();
	void Close();
	inline TBool IsIdle() const;

private:
#if defined(NE1_DMA_DOUBLE_BUFFER)
	virtual void DoQueue(DDmaRequest& aReq);
	virtual void DoCancelAll();
	virtual void DoDfc(DDmaRequest& aCurReq, SDmaDesHdr*& aCompletedHdr);
private:
	enum { EIdle = 0, ETransferring, ETransferringLast } iState;
#endif
	virtual void QueuedRequestCountChanged();

	inline void ProcessTC(TBool aClearStatus);
	inline void ProcessEnd(TBool aClearStatus);
	inline void ClearStatus(TUint32 aBitmask);
	inline void HandleIsr(TBool aIsComplete);

public:
	TInt iHWCtrlBase;// Base address of H/W registers for this channel.
	TInt iSubChannel;// Subchannel number (0-7) within H/W controller.
	TInt iDMACHCReg; // The content of configuration (CHC) register for this channel.
	TInt iTransferDataShift;//log2 of basic unit of transfer. See TDMAChannelLocator::iTransferShiftSize

	// The following members are public so that they can be
	// modified by the TNaviEngineDmac class

	/**
	This flag is set when the base register set is filled

	It allows the ISR to detect the case where a base to work
	register set changeover has happened (END interrupt) but has been masked by
	the completion of the work register set
	*/
	TBool iBaseValidSet;

	/**
	This counter is incremented each time a LaunchTransfer is started
	and decremented when the ISR handles the transfer complete (TC
	bit). Ie. it does not keep count of the number of times the base
	register set is filled. This allows missed TC interrupts to be
	detected.
	*/
	TInt iTcIrqCount;

	/**
	This spinlock is used to protect both the iBaseValidSet and iTcIrqCount
	variables. It synchronises access between threads and the ISR.

	For the ISR the setting of iBaseValidSet must appear to be atomic with
	the launch of the transfer.

	For iTcIrqCount The spinlock makes the transfer launch and subsequent
	increase of the count appear atomic to the ISR. Otherwise it could
	observe a completed transfer before the count was incremented or
	vice-versa
	*/
	TSpinLock iIsrLock;
	};

//
// Derived Controller Class
//
class TNaviEngineDmac : public TDmac
	{
public:
	TNaviEngineDmac();
	TInt Create();

	friend void TNE1DmaChannel::HandleIsr(TBool); // Allow channel HandleIsr to call TDmac::HandleIsr
private:
	// from TDmac (PIL pure virtual)
	virtual void Transfer(const TDmaChannel& aChannel, const SDmaDesHdr& aHdr);
	virtual void StopTransfer(const TDmaChannel& aChannel);
	virtual TBool IsIdle(const TDmaChannel& aChannel);
	virtual TInt MaxTransferSize(TDmaChannel& aChannel, TUint aFlags, TUint32 aPslInfo);
	virtual TUint MemAlignMask(TDmaChannel& aChannel, TUint aFlags, TUint32 aPslInfo);
	// from TDmac (PIL virtual)
	virtual void InitHwDes(const SDmaDesHdr& aHdr, TUint32 aSrc, TUint32 aDest, TInt aCount,
 						   TUint aFlags, TUint32 aPslInfo, TUint32 aCookie);
	virtual void ChainHwDes(const SDmaDesHdr& aHdr, const SDmaDesHdr& aNextHdr);
	virtual void AppendHwDes(const TDmaChannel& aChannel, const SDmaDesHdr& aLastHdr,
							 const SDmaDesHdr& aNewHdr);
	virtual void UnlinkHwDes(const TDmaChannel& aChannel, SDmaDesHdr& aHdr);
	// other
	static void DMAC32_Isr(TAny* aThis, TInt aController, TInt aTcsMask, TInt aCompleted);
	static void DMAC32_0_End_Isr(TAny* aThis);
	static void DMAC32_0_Err_Isr(TAny* aThis);
	static void DMAC32_2_End_Isr(TAny* aThis);
	static void DMAC32_2_Err_Isr(TAny* aThis);
	static void DMAC32_3_End_Isr(TAny* aThis);
	static void DMAC32_3_Err_Isr(TAny* aThis);
	static void InitHWChannel (ENaviEngineDmaController aNEController, TInt aGroup, TInt aSubChannel);
	static void InitAllHWChannels ();
	inline TDmaDesc* HdrToHwDes(const SDmaDesHdr& aHdr);
private:
	static const SCreateInfo KInfo;
public:

	void PopulateWorkSet(TNE1DmaChannel& aChannel, const SDmaDesHdr& aHdr);
	void LaunchTransfer(TNE1DmaChannel& aChannel, TBool aBaseSetValid);
#if defined(NE1_DMA_DOUBLE_BUFFER)
	void PopulateBaseSet(TNE1DmaChannel& aChannel, const SDmaDesHdr& aHdr);
	void ContinueTransfer(TNE1DmaChannel& aChannel, TBool aBaseSetValid);
#endif

	TNE1DmaChannel iChannels[EDmaChannelCount];
	};

static TNaviEngineDmac Controller;

const TDmac::SCreateInfo TNaviEngineDmac::KInfo =
	{
	EDmaChannelCount,
	KDesCount,
	TDmac::KCapsBitHwDes,
	sizeof(TDmaDesc),
	EMapAttrSupRw | EMapAttrFullyBlocking //??
	};

TNaviEngineDmac::TNaviEngineDmac()
//
// Constructor.
//
	: TDmac(KInfo)
	{}

TInt TNaviEngineDmac::Create()
//
// Second phase construction.
//
	{
	TInt r = TDmac::Create(KInfo);							// Base class Create()
	if (r == KErrNone)
		{
		__DMA_ASSERTA(ReserveSetOfDes(EDmaChannelCount) == KErrNone);
		
		//Read KDMAChannelLocator constants and populate the values in channel objects.
		
		for (TInt i=0; i < EDmaChannelCount; ++i)
			{
			TUint ctrlBase = 0;
			switch (KDMAChannelLocator[i].iDMACtrl)
				{
				case EDmaCtrlExBus: ctrlBase = KDMACExBusBase; break;
				case EDMACtrl32:	ctrlBase = KDMAC32Base;	   break;
				default:			__DMA_CANT_HAPPEN();
				}
			iChannels[i].iHWCtrlBase = ctrlBase + KDMAChannelLocator[i].iGroup * KDMAGroupOffset + 
														KDMAChannelLocator[i].iSubChannel * KDMAChannelOffset;
			iChannels[i].iSubChannel = KDMAChannelLocator[i].iSubChannel;
			iChannels[i].iDMACHCReg = KDMAChannelLocator[i].iDMACHCReg | iChannels[i].iSubChannel;
			iChannels[i].iTransferDataShift = KDMAChannelLocator[i].iTransferShiftSize;
			iFreeHdr = iFreeHdr->iNext;
			}
		
		//Bind DMA interrupt for channels we support
		TInt irqh0 = Interrupt::Bind(KIntDMAC32_0_End, DMAC32_0_End_Isr, this); __DMA_ASSERTA(irqh0>=0); 
		TInt irqh1 = Interrupt::Bind(KIntDMAC32_0_Err, DMAC32_0_Err_Isr, this); __DMA_ASSERTA(irqh1>=0); 
		TInt irqh2 = Interrupt::Bind(KIntDMAC32_2_End, DMAC32_2_End_Isr, this); __DMA_ASSERTA(irqh2>=0); 
		TInt irqh3 = Interrupt::Bind(KIntDMAC32_2_Err, DMAC32_2_Err_Isr, this); __DMA_ASSERTA(irqh3>=0); 
		TInt irqh4 = Interrupt::Bind(KIntDMAC32_3_End, DMAC32_3_End_Isr, this); __DMA_ASSERTA(irqh4>=0); 
		TInt irqh5 = Interrupt::Bind(KIntDMAC32_3_Err, DMAC32_3_Err_Isr, this); __DMA_ASSERTA(irqh5>=0); 


		if (r == KErrNone)
			{
			InitAllHWChannels();
			
			r = Interrupt::Enable(irqh0); __DMA_ASSERTA(r==KErrNone);
			r = Interrupt::Enable(irqh1); __DMA_ASSERTA(r==KErrNone);
			r = Interrupt::Enable(irqh2); __DMA_ASSERTA(r==KErrNone);
			r = Interrupt::Enable(irqh3); __DMA_ASSERTA(r==KErrNone);
			r = Interrupt::Enable(irqh4); __DMA_ASSERTA(r==KErrNone);
			r = Interrupt::Enable(irqh5); __DMA_ASSERTA(r==KErrNone);

			}
		}
	return r;
	}

// Initialises all H/W channels. This will make sure they are off on soft restart.
void TNaviEngineDmac::InitAllHWChannels()
	{
	int i,j;
	for (i=0;i<KDmaHWCtrl32Count;i++)
		{
		for (j=0;j<KDmaCtrl32HWSubChannelCount; j++) InitHWChannel(EDMACtrl32, i, j);
		AsspRegister::Write32(KDMAC32Base+i*KDMAGroupOffset+KHoDMACONT, 0);
		}
	}

//Initialises a single H/W channel
void TNaviEngineDmac::InitHWChannel (ENaviEngineDmaController aNEController, TInt aGroup, TInt aSubChannel)
	{
	TUint neCtrlBase = 0;
	switch(aNEController)
		{
		case EDMACtrl32: 	neCtrlBase = KDMAC32Base   ; break;
		default:			__DMA_CANT_HAPPEN();
		}
	neCtrlBase += aGroup*KDMAGroupOffset + aSubChannel*KDMAChannelOffset;
	AsspRegister::Write32(neCtrlBase+KHoDMACHS, KHtDMACHS_EN_EN);	//disable channel
	}

#if defined(NE1_DMA_DOUBLE_BUFFER)

#ifdef _DEBUG
//These values indicate whether all corner cases are running.
//Proper test shouldn't leave any of these values to zero.
TInt InterruptCounter_DMA32 = 0;	// Interrupt counter
TInt Transfer_IdleOnStart = 0;	    // DMA channel is idle on the start of Transfer.
TInt Transfer_NotIdleOnStart = 0;   // DMA channel is not idle on the start of Transfer.
TInt Transfer_MatchWorkSetTrue = 0; // Descriptor matches "work set" registers
TInt Transfer_MatchWorkSetFalse = 0;// Descriptor doesn't match "work set" descriptor.
#endif

void TNaviEngineDmac::Transfer(const TDmaChannel& aChannel, const SDmaDesHdr& aHdr)
//
// Initiates a (previously constructed) request on a specific channel.
//
	{
	TDmaChannel& mutableChannel = const_cast<TDmaChannel&>(aChannel);
	TNE1DmaChannel& channel = static_cast<TNE1DmaChannel&>(mutableChannel);

	const TInt irq =  __SPIN_LOCK_IRQSAVE(channel.iIsrLock);
	// The fragment descriptor (src/dest address, size) should be placed into either "Work Set" or "Base Set"
	// depending on the actual state of the H/W
	if (IsIdle(channel))
		{
		// The channel is idle, for the (most likely) reason that both "Work Set" and "Base Set" transfers are
		// completed since the last time we run this function. 
		#ifdef _DEBUG
		Transfer_IdleOnStart++;
		#endif
		PopulateWorkSet(channel, aHdr); // Populate "Work Set"
		LaunchTransfer(channel, EFalse);   // Start the transfer, base set is invalid
		}
	else
		{
		// "Work Set" transfer is still going on. It seems we will manage to place
		// the next fragment in time for continious traffic flow.
		#ifdef _DEBUG
		Transfer_NotIdleOnStart++;
		#endif
		PopulateBaseSet(channel, aHdr);  // Populate "Base Set"
		ContinueTransfer(channel, ETrue);// Indicate Base Set is valid (bvalid = ETrue)

		// We should expect here that the "work set" traffic is still in progress.
		// Once it is completed, "Base Set" content is copied into "Work Set" and the traffic will go on.
		// However, there is a corner case where we configure "Base Set" too late.
		// Therefore, check if transfer is still active.
		if (IsIdle(channel))
			{
			// There is no DMA traffic. There could be two reason for that. Either,
			// 1. The transfer we have just configured in "Base Set" has already completed, or
			// 2. We configured base set too late, after "Work Set" transfer has already finished.

			// Check BVALID bit
			// if its now clear, then it was set in time, if it's
			// still set it was set too late
			const TUint32 dchs = AsspRegister::Read32(channel.iHWCtrlBase+KHoDMACHS);
			const TBool bvalidSet = dchs & KHtDMACHS_BVALID;

			if (!bvalidSet)
				{
				DMA_PSL_CHAN_TRACE_STATIC(channel, "Base set transferred already");
				#ifdef _DEBUG
				Transfer_MatchWorkSetTrue++;
				#endif
				}
			else
				{
				DMA_PSL_CHAN_TRACE_STATIC(channel, "Too late for base set");

				// BVALID bit was set after "Work Set" transfer completed, and DMA H/W didn't
				// copy the content of "Base Set" into "Work Set". We have to re-launch the transfer.
				// This time we have to configure "Work Set"
				#ifdef _DEBUG
				Transfer_MatchWorkSetFalse++;
				#endif
				PopulateWorkSet(channel, aHdr); // Populate "Work Set".
				LaunchTransfer(channel, EFalse);   // Start the transfer, "Base Set" is invalid.
				}
			}
		}
	__SPIN_UNLOCK_IRQRESTORE(channel.iIsrLock, irq);
	}
#else
void TNaviEngineDmac::Transfer(const TDmaChannel& aChannel, const SDmaDesHdr& aHdr)
//
// Initiates a (previously constructed) request on a specific channel.
//
	{
	TDmaChannel& mutableChannel = const_cast<TDmaChannel&>(aChannel);
	TNE1DmaChannel& channel = static_cast<TNE1DmaChannel&>(mutableChannel);

	DMA_PSL_CHAN_TRACE_STATIC1(channel, ">TNaviEngineDmac::Transfer des=0x%08X", HdrToHwDes(aHdr));

	const TInt irq =  __SPIN_LOCK_IRQSAVE(channel.iIsrLock);
	// The fragment descriptor (src/dest address, size) should be placed into either "Work Set" or "Base Set"
	// depending on the actual state of the H/W
	__NK_ASSERT_ALWAYS(IsIdle(channel));
	PopulateWorkSet(channel, aHdr); // Populate "Work Set"
	LaunchTransfer(channel, EFalse);   // Start the transfer, base set is invalid
	__SPIN_UNLOCK_IRQRESTORE(channel.iIsrLock, irq);
	}
#endif


void TNaviEngineDmac::StopTransfer(const TDmaChannel& aChannel)
//
// Stops a running channel.
//
	{
	TDmaChannel& mutableChannel = const_cast<TDmaChannel&>(aChannel);
	TNE1DmaChannel& channel = static_cast<TNE1DmaChannel&>(mutableChannel);

	__KTRACE_OPT(KDMA, Kern::Printf(">TNaviEngineDmac::StopTransfer channel=%d", channel.PslId()));

	channel.StopTransfer();

	__KTRACE_OPT(KDMA, Kern::Printf("<TNaviEngineDmac::StopTransfer channel=%d", channel.PslId()));
	}

TBool TNaviEngineDmac::IsIdle(const TDmaChannel& aChannel)
	{
	TDmaChannel& mutableChannel = const_cast<TDmaChannel&>(aChannel);
	TNE1DmaChannel& channel = static_cast<TNE1DmaChannel&>(mutableChannel);

	const TBool idle = channel.IsIdle();
	__KTRACE_OPT(KDMA, Kern::Printf(">Dmac::IsIdle channel=%d, idle=%d", channel.PslId(), idle));
	return idle;
	}

// Places the descriptor into "Work Set"
void TNaviEngineDmac::PopulateWorkSet(TNE1DmaChannel& aChannel, const SDmaDesHdr& aHdr)
	{
	TDmaDesc* pD = HdrToHwDes(aHdr);
	__KTRACE_OPT(KDMA, Kern::Printf(">TNaviEngineDmac::PopulateWorkSet channel=%d des=0x%08X", aChannel.PslId(), pD));
	
	AsspRegister::Write32(aChannel.iHWCtrlBase+KHoDMACHC, aChannel.iDMACHCReg);	//configure channel
	AsspRegister::Write32(aChannel.iHWCtrlBase+KHoDMASAW, pD->iSrcAddr); 		//source addr 
	AsspRegister::Write32(aChannel.iHWCtrlBase+KHoDMADAW, pD->iDestAddr);		//dest addr
	AsspRegister::Write32(aChannel.iHWCtrlBase+KHoDMATCW, (pD->iCount>>aChannel.iTransferDataShift)-1);	//transfer counter
	}

// Starts the transfer.
// @pre The chanel is idle.
// @arg aBaseSetValid if true, BVALID bit should be set.
// @pre iIsrLock must be held
void TNaviEngineDmac::LaunchTransfer(TNE1DmaChannel& aChannel, TBool aBaseSetValid)
	{
	__KTRACE_OPT(KDMA, Kern::Printf(">TNaviEngineDmac::LaunchTransfer channel=%d", aChannel.PslId()));
	TInt val = KHtDMACHS_EN|KHtDMACHS_EN_EN;
	if (TUint(aChannel.iDMACHCReg ^ aChannel.iSubChannel) == (TUint)KHvDMACHC_SW)
		val |=KHtDMACHS_STG;
	if (aBaseSetValid)
		val|=KHtDMACHS_BVALID;

	aChannel.iBaseValidSet = aBaseSetValid;

	AsspRegister::Write32(aChannel.iHWCtrlBase+KHoDMACHS, val);

	aChannel.iTcIrqCount++;
	DMA_PSL_CHAN_TRACE_STATIC1(aChannel, "inc iTcIrqCount to %d", aChannel.iTcIrqCount);
	}

#if defined(NE1_DMA_DOUBLE_BUFFER)
// Places the descriptor into "Base Set"
void TNaviEngineDmac::PopulateBaseSet(TNE1DmaChannel& aChannel, const SDmaDesHdr& aHdr)
	{
	TDmaDesc* pD = HdrToHwDes(aHdr);
	__KTRACE_OPT(KDMA, Kern::Printf(">TNaviEngineDmac::PopulateBaseSet channel=%d des=0x%08X", aChannel.PslId(), pD));

	AsspRegister::Write32(aChannel.iHWCtrlBase+KHoDMASAB, pD->iSrcAddr); 								// Source addr
	AsspRegister::Write32(aChannel.iHWCtrlBase+KHoDMADAB, pD->iDestAddr);								// Dest addr
	AsspRegister::Write32(aChannel.iHWCtrlBase+KHoDMATCB, (pD->iCount>>aChannel.iTransferDataShift)-1); // Transfer counter
	}

// @pre DMA transfer is in progress.
// @arg aBaseSetValid if true, BVALID bit should be set.
// @pre iIsrLock must be held
void TNaviEngineDmac::ContinueTransfer(TNE1DmaChannel& aChannel, TBool aBaseSetValid)
	{
	__KTRACE_OPT(KDMA, Kern::Printf(">TNaviEngineDmac::ContinueTransfer channel=%d", aChannel.PslId()));
	TInt val = 0;
	if (TUint(aChannel.iDMACHCReg ^ aChannel.iSubChannel) == (TUint)KHvDMACHC_SW)
		val |=KHtDMACHS_STG;	// Set software trigger
	if (aBaseSetValid)
		{
		__NK_ASSERT_DEBUG(!aChannel.iBaseValidSet);
		aChannel.iBaseValidSet = ETrue;
		val|=KHtDMACHS_BVALID;
		}

	if (val)
		{
		AsspRegister::Write32(aChannel.iHWCtrlBase+KHoDMACHS, val);
		}
	}

// As in TDmaDbChannel, except for EIdle state as we have place the 1st and the 2nd 
// fragment into different registers.
void TNE1DmaChannel::DoQueue(DDmaRequest& aReq)
	{
	TNaviEngineDmac* controller = (TNaviEngineDmac*)iController; 
	
	switch (iState)
		{
	case EIdle:
		{
		controller->PopulateWorkSet(*this, *iCurHdr);
		const TInt irq =  __SPIN_LOCK_IRQSAVE(iIsrLock);
		if (iCurHdr->iNext)
			{
			controller->PopulateBaseSet(*this, *(iCurHdr->iNext));
			controller->LaunchTransfer(*this, ETrue);//BaseSetValid=True
			iState = ETransferring;
			}
		else
			{
			controller->LaunchTransfer(*this, EFalse);//BaseSetValid=False
			iState = ETransferringLast;
			}
		__SPIN_UNLOCK_IRQRESTORE(iIsrLock, irq);
		break;
		}
	case ETransferring:
		// nothing to do
		break;
	case ETransferringLast:
		iController->Transfer(*this, *(aReq.iFirstHdr));
		iState = ETransferring;
		break;
	default:
		__DMA_CANT_HAPPEN();
		}
	}

//As in TDmaDbChannel
void TNE1DmaChannel::DoCancelAll()
	{
	iState = EIdle;
	}

// As in TDmaDbChannel
void TNE1DmaChannel::DoDfc(DDmaRequest& /*aCurReq*/, SDmaDesHdr*& aCompletedHdr)
	{
	aCompletedHdr = iCurHdr;
	iCurHdr = iCurHdr->iNext;
	switch (iState)
		{
	case ETransferringLast:
		iState = EIdle;
		break;
	case ETransferring:
		if (iCurHdr->iNext == NULL)
			iState = ETransferringLast;
		else
			iController->Transfer(*this, *(iCurHdr->iNext));
		break;
	default:
		__DMA_CANT_HAPPEN();
		}
	}
#endif

TNE1DmaChannel::TNE1DmaChannel()
	:iBaseValidSet(EFalse), iTcIrqCount(0), iIsrLock(TSpinLock::EOrderGenericIrqHigh0)
	{}

/**
Handles normal interrupts as well as recovering from missed interrupts.
It must therefore be called during an ISR, for every valid, open channel
on a DMAC .ie not just channels which have status bits set.
*/
void TNE1DmaChannel::ProcessIrq()
	{
	// The spinlock protects access to the iBaseValidSet flag
	// This is needed because it is possible for TNaviEngineDmac::Transfer to
	// attempt to populate the base set, set iBaseValidSet, but then
	// realize it was too late, and have to unset it.
	const TInt irq =  __SPIN_LOCK_IRQSAVE(iIsrLock);

	// check that channel is open
	if(iController == NULL)
		{
		__SPIN_UNLOCK_IRQRESTORE(iIsrLock, irq);
		return;
		}

	const TInt irqCount = iTcIrqCount;
	__NK_ASSERT_ALWAYS(irqCount >= 0);
	__NK_ASSERT_ALWAYS(irqCount < 3);

	TUint32 dchs = AsspRegister::Read32(iHWCtrlBase+KHoDMACHS);

	// Detect if we have missed 1 TC interrupt.
	// This can happen when there is one transfer in progress,
	// and the channel attempts to populate the base reg set, but
	// is too late and launches a new transfer. The second transfer
	// may then complete during the ISR of the first, or the ISR may
	// not run untill after the second has already completed.
	if((irqCount > 0) && IsIdle())
		{
		// Reread status now that we have observed channel as idle.
		// If a transfer completed between the first read and now, we
		// can handle that as a normal interrupt instead of as a
		// missed interrupt. This is not essential, just neater
		dchs = AsspRegister::Read32(iHWCtrlBase+KHoDMACHS);
		if(irqCount == 1) // There may or may not be a missed IRQ
			{
			if((dchs & KHtDMACHS_TC) == 0)
				{
				DMA_PSL_CHAN_TRACE1("Channel had missed TC IRQ irqs=%d", irqCount);
				ProcessTC(EFalse);
				}
			}
		else if(irqCount == 2) // There is 1 missed and 1 normal IRQ
			{
			DMA_PSL_CHAN_TRACE1("Channel had missed TC IRQ irqs=%d", irqCount);
			ProcessTC(EFalse);

			 // Ensure that remaining IRQ will be dealt with in next block
			__NK_ASSERT_ALWAYS((dchs & KHtDMACHS_TC));
			}
		else
			{
			// It should not be possible for there to be more than 2
			// outstanding transfers launched
			FAULT();
			}
		}

	// Deal with normal interrupts
	if (dchs&KHtDMACHS_TC)
		{
		// ISR should not be able to observe the BVALID bit itself
		// since TNaviEngineDmac::Transfer should hold iIsrLock whilst
		// it decides if it was set in time
		__NK_ASSERT_DEBUG(!(dchs & KHtDMACHS_BVALID));

		// Here we find out if a base-set-copy (END) interrupt has
		// been missed. If a TC comes shortly after an END IRQ then
		// it would be impossible to tell by looking at the status
		// register alone
		if(iBaseValidSet)
			{
			DMA_PSL_CHAN_TRACE("END irq missed ");
			ProcessEnd(EFalse);
			}
		ProcessTC(ETrue);
		}
	else if	(dchs&KHtDMACHS_END)
		{
		ProcessEnd(ETrue);
		}

	__SPIN_UNLOCK_IRQRESTORE(iIsrLock, irq);
	}

void TNE1DmaChannel::ProcessErrorIrq()
	{
	const TInt irq =  __SPIN_LOCK_IRQSAVE(iIsrLock);

	// check that channel is open
	if(iController == NULL)
		{
		__SPIN_UNLOCK_IRQRESTORE(iIsrLock, irq);
		return;
		}

	// reset channel
	ClearStatus(KHtDMACHS_FCLR);
	TInt badIrqCount = iTcIrqCount;
	iTcIrqCount = 0;

	if(iBaseValidSet)
		{
		iBaseValidSet = EFalse;
		badIrqCount++;
		}

	// complete all outstanding requests as being in error
	for(TInt i=0; i < badIrqCount; i++)
		{
		HandleIsr(EFalse);
		}

	__SPIN_UNLOCK_IRQRESTORE(iIsrLock, irq);
	}
/**
Handle a transfer complete (work set transfer complete and base set was
empty) on this channel.

@param aClearStatus - Status bits should be cleared
*/
void TNE1DmaChannel::ProcessTC(TBool aClearStatus)
	{
	// iTcIrqCount may be zero if StopTransfer were called
	// between the transfer being started and the ISR running
	if(iTcIrqCount>0)
		{
		DMA_PSL_CHAN_TRACE1("dec iTcIrqCount to %d", iTcIrqCount);
		iTcIrqCount--;
		DMA_PSL_CHAN_TRACE("TC");
		HandleIsr(ETrue);
		}

	__NK_ASSERT_DEBUG(iTcIrqCount >= 0);

	if(aClearStatus)
		ClearStatus(KHtDMACHS_TC|KHtDMACHS_END); //Traffic completed BVALID=OFF
	}

/**
Handle a END (transfer complete, base set loaded in to work set) on this channel.

@param aClearStatus - Status bit should be cleared
*/
void TNE1DmaChannel::ProcessEnd(TBool aClearStatus)
	{
	if(iBaseValidSet)
		{
		DMA_PSL_CHAN_TRACE("END");
		iBaseValidSet = EFalse;
		HandleIsr(ETrue);
		}

	if(aClearStatus)
		ClearStatus(KHtDMACHS_END); //Traffic completed BVALID=ON
	}

/**
@param aBitmask The bits to be cleared in this channel's status register
*/
void TNE1DmaChannel::ClearStatus(TUint32 aBitmask)
	{
	if (TUint((this->iDMACHCReg) ^ (this->iSubChannel)) == (TUint)KHvDMACHC_SW)
		aBitmask |= KHtDMACHS_STG; //Add STG for S/W channel

	AsspRegister::Write32(iHWCtrlBase+KHoDMACHS, aBitmask); //End-of-Int
	}

/**
Call HandleIsr for this channel
*/
void TNE1DmaChannel::HandleIsr(TBool aIsComplete)
	{
	// iController must be casted so that the private method
	// TDmac::HandleIsr can be called since this method is
	// a friend of TNaviEngineDmac, but not TDmac.
	static_cast<TNaviEngineDmac*>(iController)->HandleIsr(*this, aIsComplete);
	}

/**
Stop transfer for this channel
*/
void TNE1DmaChannel::StopTransfer()
	{
	const TInt irq =  __SPIN_LOCK_IRQSAVE(iIsrLock);
	// At this point, device driver should have cancelled DMA request.

	// The procedure for clearing the EN bit to 0 via CPU access during DMA transfer (EN bit = 1)
	TUint32 dmaCHS = AsspRegister::Read32(iHWCtrlBase+KHoDMACHS);

	// Read the DCHSn register to be cleared at the relevant channel and confirm that
	// both the RQST and ACT bits are cleared to 0. If either or both of them are 1,
	// perform polling until their values become 0..
	// OR unless KHtDMACHS_EN is already cleared by the HW at the end of the transfer -
	// while we're polling...
	while( (dmaCHS & KHtDMACHS_EN) &&
			dmaCHS & (KHtDMACHS_RQST | KHtDMACHS_ACT) )
		{
		dmaCHS = AsspRegister::Read32(iHWCtrlBase+KHoDMACHS);
		}

	// enable writing to EN bit..
	dmaCHS |= KHtDMACHS_EN_EN;
	AsspRegister::Write32(iHWCtrlBase+KHoDMACHS, dmaCHS);

	// clear the EN bit to 0
	// and set the FCLR bit of the DCHSn register to 1.
	dmaCHS &= (~KHtDMACHS_EN);
	dmaCHS |= KHtDMACHS_FCLR;
	AsspRegister::Write32(iHWCtrlBase+KHoDMACHS, dmaCHS);

	// check that channel is idle, and status bits have been cleared
	__NK_ASSERT_ALWAYS(IsIdle());
	dmaCHS = AsspRegister::Read32(iHWCtrlBase+KHoDMACHS);
	__NK_ASSERT_ALWAYS((dmaCHS & (KHtDMACHS_TC | KHtDMACHS_END)) == 0);
	__NK_ASSERT_ALWAYS(iTcIrqCount >=0);

	// given the above checks, clear the iTcIrqCount and iBaseValidSet so
	// that the ISR won't mistakenly think there are missed interrupts
	iTcIrqCount = 0;
	iBaseValidSet = EFalse;

	__SPIN_UNLOCK_IRQRESTORE(iIsrLock, irq);
	}

/**
@pre Channel has been stopped
*/
void TNE1DmaChannel::Close()
	{
	// The lock prevents a channel being closed
	// during an ISR.
	const TInt irq =  __SPIN_LOCK_IRQSAVE(iIsrLock);
	DMA_PSL_CHAN_TRACE("Close");

	// Check that the channel was Idle and Stopped
	__NK_ASSERT_ALWAYS(IsIdle());
	__NK_ASSERT_ALWAYS(iTcIrqCount == 0);
	__NK_ASSERT_ALWAYS(!iBaseValidSet);


	// Here we clear iController in advance of the PIL
	// If we did not do this, then when we release the lock, the ISR
	// could observe it as non-null, proceed, but then have the PIL
	// clear it mid-isr
	iController = NULL;

	__SPIN_UNLOCK_IRQRESTORE(iIsrLock, irq);
	}

TBool TNE1DmaChannel::IsIdle() const
	{
	TUint status = AsspRegister::Read32(iHWCtrlBase+KHoDMACHS);
	return !(status & KHtDMACHS_EN);
	}


void TNE1DmaChannel::QueuedRequestCountChanged()
	{
	const TInt qreqs = __e32_atomic_load_acq32(&iQueuedRequests);
	DMA_PSL_CHAN_TRACE1("TNE1DmaChannel::QueuedRequestCountChanged() %d", qreqs);
	__DMA_ASSERTA(qreqs >= 0);
	}


TInt TNaviEngineDmac::MaxTransferSize(TDmaChannel& aChannel, TUint /*aFlags*/, TUint32 /*aPslInfo*/)
//
// Returns the maximum transfer size for a given transfer.
//
	{
	TNE1DmaChannel& channel = (TNE1DmaChannel&)aChannel;
	return (1<<channel.iTransferDataShift) * KMaxDMAUnitTransferLen;
	}


TUint TNaviEngineDmac::MemAlignMask(TDmaChannel& aChannel, TUint /*aFlags*/, TUint32 /*aPslInfo*/)
//
// Returns the memory buffer alignment restrictions mask for a given transfer.
//
	{
	TNE1DmaChannel& channel = (TNE1DmaChannel&)aChannel;
	return (1<<channel.iTransferDataShift) - 1;
	}


void TNaviEngineDmac::InitHwDes(const SDmaDesHdr& aHdr, TUint32 aSrc, TUint32 aDest, TInt aCount,
							  TUint aFlags, TUint32 /*aPslInfo*/, TUint32 /*aCookie*/)
//
// Sets up (from a passed in request) the descriptor with that fragment's source and destination address,
// the fragment size, and the (driver/DMA controller) specific transfer parameters (mem/peripheral,
// burst size, transfer width).
//
	{
	TDmaDesc* pD = HdrToHwDes(aHdr);

	__KTRACE_OPT(KDMA, Kern::Printf("TNaviEngineDmac::InitHwDes 0x%08X", pD));

	// Unaligned descriptor? Bug in generic layer!
	__DMA_ASSERTD(IsHwDesAligned(pD));

	pD->iSrcAddr = (aFlags & KDmaPhysAddrSrc) ? aSrc : Epoc::LinearToPhysical(aSrc);
	pD->iDestAddr = (aFlags & KDmaPhysAddrDest) ? aDest : Epoc::LinearToPhysical(aDest);
	pD->iCount = aCount;
	}

void TNaviEngineDmac::ChainHwDes(const SDmaDesHdr& /*aHdr*/, const SDmaDesHdr& /*aNextHd*/)
//
// Chains hardware descriptors together.
// DMAC32 doesn't support linked descriptors, therefore there is nothing we have to do here.
//
	{
	}

void TNaviEngineDmac::AppendHwDes(const TDmaChannel& /*aChannel*/, const SDmaDesHdr& /*aLastHdr*/,
								const SDmaDesHdr& /*aNewHdr*/)
//
// Appends a descriptor to the chain while the channel is running.
// DMAC32 doesn't support linked descriptors, therefore there is nothing we have to do here.
//
	{
	}

void TNaviEngineDmac::UnlinkHwDes(const TDmaChannel& /*aChannel*/, SDmaDesHdr& /*aHdr*/)
//
// Unlink the last item in the h/w descriptor chain from a subsequent chain that it was possibly linked to.
// DMAC32 doesn't support linked descriptors, therefore there is nothing we have to do here.
//
	{
	}

 void TNaviEngineDmac::DMAC32_Isr(TAny* aThis, TInt aController, TInt aDmaStat, TInt aCompleted)
 //
 // Generic part for all DMA32 interrupts.
 // Reads the interrupt identification and calls back into the base class
 // interrupt service handler with the channel identifier and an indication whether the
 // transfer completed correctly or with an error.
 //
	{
	DMA_PSL_TRACE("Begin ISR");

	TNaviEngineDmac& me = *static_cast<TNaviEngineDmac*>(aThis);
	int i;

	if (aCompleted)	// Transfer-completed interrupt has occured
		{
		// Go through the all eight subchannels to check which event has occured.
		for (i=0;i<KDmaCtrl32HWSubChannelCount;i++)
			{
			TInt channel = DMAC32_HWChannelsLocator[aController][i];
			#ifdef _DEBUG
			if (channel >= EDma32ChannelCount) __DMA_CANT_HAPPEN();
			#endif

			// Skip unused physical channels
			// .ie those with no corresponding entry
			// in KDMAChannelLocator
			if(channel == -1)
				continue;

			TNE1DmaChannel& ne1Chan = me.iChannels[channel];

			ne1Chan.ProcessIrq();
			}
		}
	else	// Error interrupt has occured. aDmaStat is not valid. Should read H/W registers.
		{
		// Go through the all eight subchannels to check which event has occured.
		for (i=0;i<KDmaCtrl32HWSubChannelCount;i++)
			{
			TInt dchs= AsspRegister::Read32(KDMAC32Base+aController*KDMAGroupOffset+i*KDMAChannelOffset+KHoDMACHS);
			
			if (dchs&KHtDMACHS_ERR)
				{
				TInt channel = DMAC32_HWChannelsLocator[aController][i];
				#ifdef _DEBUG
				if (channel >= EDmaChannelCount) __DMA_CANT_HAPPEN();
				#endif
				TNE1DmaChannel& ne1Chan = me.iChannels[channel];
				ne1Chan.ProcessErrorIrq();
				}
			}
		}
#if defined(NE1_DMA_DOUBLE_BUFFER)
	#ifdef _DEBUG
	InterruptCounter_DMA32++;
	#endif
#endif
 	}

 
 void TNaviEngineDmac::DMAC32_0_End_Isr(TAny* aThis)
	{
	TInt stat = (TInt)AsspRegister::Read32(KDMAC32Base+0*KDMAGroupOffset+KHoDMASTAT);
	DMAC32_Isr(aThis, 0, stat, 1);
	}
 void TNaviEngineDmac::DMAC32_2_End_Isr(TAny* aThis)
  	{
  	TInt stat = (TInt)AsspRegister::Read32(KDMAC32Base+2*KDMAGroupOffset+KHoDMASTAT);
  	DMAC32_Isr(aThis, 2, stat, 1);
  	}
 void TNaviEngineDmac::DMAC32_3_End_Isr(TAny* aThis)
  	{
  	TInt stat = (TInt)AsspRegister::Read32(KDMAC32Base+3*KDMAGroupOffset+KHoDMASTAT);
  	DMAC32_Isr(aThis, 3, stat, 1);
  	}

 void TNaviEngineDmac::DMAC32_0_Err_Isr(TAny* aThis)
	{
  	DMAC32_Isr(aThis, 0, 0, 0);
	}
void TNaviEngineDmac::DMAC32_2_Err_Isr(TAny* aThis)
	{
  	DMAC32_Isr(aThis, 2, 0, 0);
	}
void TNaviEngineDmac::DMAC32_3_Err_Isr(TAny* aThis)
	{
  	DMAC32_Isr(aThis, 3, 0, 0);
	}

inline TDmaDesc* TNaviEngineDmac::HdrToHwDes(const SDmaDesHdr& aHdr)
//
// Changes return type of base class call.
//
	{
	return static_cast<TDmaDesc*>(TDmac::HdrToHwDes(aHdr));
	}

//
// Channel Opening/Closing (Channel Allocator)
//

TDmaChannel* DmaChannelMgr::Open(TUint32 aOpenId)
	{
	__KTRACE_OPT(KDMA, Kern::Printf(">DmaChannelMgr::Open aOpenId=%d", aOpenId));

	__DMA_ASSERTA(aOpenId < static_cast<TUint32>(EDmaChannelCount));

	TDmaChannel* pC = Controller.iChannels + aOpenId;
	if (pC->IsOpened())
		pC = NULL;
	else
		{
		pC->iController = &Controller;
		pC->iPslId = aOpenId;
		}
	return pC;
	}

void DmaChannelMgr::Close(TDmaChannel* aChannel)
	{
	static_cast<TNE1DmaChannel*>(aChannel)->Close();
	}

TInt DmaChannelMgr::StaticExtension(TInt /* aCmd */, TAny* /* aArg */)
	{
	return KErrNotSupported;
	}

//
// DLL Exported Function
//

DECLARE_STANDARD_EXTENSION()
//
// Creates and initializes a new DMA controller object on the kernel heap.
//
	{
	__KTRACE_OPT2(KBOOT, KDMA, Kern::Printf("Starting DMA Extension"));

	return Controller.Create();
	}
