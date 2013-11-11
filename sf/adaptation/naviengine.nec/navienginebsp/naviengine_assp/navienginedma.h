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
* naviengine_assp\navienginedma.h
* Definitions and constants of NE SoC DMA Controller.
* See 2.10 DMA Controllers in NaviEngine User Manual
*
*/


#ifndef __NAVIENGINEDMA_H__
#define __NAVIENGINEDMA_H__

#include <drivers/dma.h>
#include <assp/naviengine/naviengine.h>

#include <platform.h>


#define FUNC_LOG __KTRACE_OPT(KDMA, Kern::Printf(__PRETTY_FUNCTION__))
#define PRINT(S) __KTRACE_OPT(KDMA, Kern::Printf("%s = 0x%08x", #S, (S)))

/* 
 * The list of DMA logical channels.
 * Use these values to populate iCookie when openning DMA channel.
 */ 
enum ENaviengineDmaChannels
	{
	EDMAChannelSD0,			///< Internal, clients get value from HCR 
	EDMAChannelSD1,			///< Internal, clients get value from HCR 
	
	EDMAChannelI2S0RX,		///< Internal, clients get value from HCR 
	EDMAChannelI2S0TX,		///< Internal, clients get value from HCR 
	EDMAChannelI2S1RX,		///< Internal, clients get value from HCR 
	EDMAChannelI2S1TX,		///< Internal, clients get value from HCR 
	EDMAChannelI2S2RX,		///< Internal, clients get value from HCR 
	EDMAChannelI2S2TX,		///< Internal, clients get value from HCR 
	EDMAChannelI2S3RX,		///< Internal, clients get value from HCR 
	EDMAChannelI2S3TX,		///< Internal, clients get value from HCR 
	
	EDMAChannelUART0RX,		///< Internal, clients get value from HCR 
	EDMAChannelUART0TX,		///< Internal, clients get value from HCR 
	EDMAChannelUART1RX,		///< Internal, clients get value from HCR 
	EDMAChannelUART1TX,		///< Internal, clients get value from HCR 
	EDMAChannelUART2RX,		///< Internal, clients get value from HCR 
	EDMAChannelUART2TX,		///< Internal, clients get value from HCR 

	EDmaMemToMem0,			///< Internal, clients get value from HCR 
	EDmaMemToMem1,			///< Internal, clients get value from HCR 
	EDmaMemToMem2,			///< Internal, clients get value from HCR 
	EDmaMemToMem3,			///< Internal, clients get value from HCR 

	EDma32ChannelCount, //End of DMA32 channels

	//Enum indexes into KDMAChannelLocator array - it doesn't have a gap
	EDma64MemToMem0 = EDma32ChannelCount,
	EDma64MemToMem1,
	EDma64MemToMem2,
	EDma64MemToMem3,
	
	EDmaChannelCount, // End of DMA64 channels
	};

/*
 * The list of DMA controllers in NE SoC
 */
enum ENaviEngineDmaController
	{
	EDmaCtrlExBus,	// not supported - there is no any H/W of interest
	EDMACtrl32,		// DMAC32(8C)
	EDmaCtrl64		// DMAC64(4C) - DMA controller on 64 Bit AXI bus
	};

/*
 * Specifies H/W parameters of logical DMA channels (@see ENaviengineDmaChannels).
 */
struct TDMAChannelLocator
	{
	ENaviEngineDmaController iDMACtrl;
	TInt iGroup; 		// 0 for EDmaCtrlExBus & 0-4 EDMACtrl32
	TInt iSubChannel; 	// 0-3 for EDmaCtrlExBus & 0-7 for each group of EDMACtrl32
	TInt iDMACHCReg;    // The content CHC register (excluding subchannel)
	TInt iTransferShiftSize;//0-Byte, 1-HalfWord, 2-Word, 4-FourWords, 5-EightWords, 6-SixteenWords
	};

const TInt KDmaHWCtrl32Count = 5; 			// The number of DMA32 controllers
const TInt KDmaCtrl32HWSubChannelCount = 8;	// The number of subchannels per controller.

const TUint KDMAGroupOffset = 0x1000;	// Regitsers' offset per cotroller
const TUint KDMAChannelOffset = 0x20;	// Registers' offset per subchannel

/****** Offsets and bitmasks of DMA32 registers ************/
const TUint KHoDMASAB	= 0x00; // Source Address Base
const TUint KHoDMADAB	= 0x04; // Destination Address Base
const TUint KHoDMATCB	= 0x08; // Transfer Count Base
const TUint KHoDMASAW	= 0x0c; // Source Address Work
const TUint KHoDMADAW	= 0x10; // Destination Address Work
const TUint KHoDMATCW	= 0x14; // Transfer Count Work
const TUint KHoDMACHC	= 0x18; // Channel Control Register
const TUint KHmDMACHC_SEL 	= 7;	   // Channel selection mask
const TUint KHvDMACHC_SW	 = 0xd1226000; // Settings for SW transfer. Word alignement for both source & dest.
const TUint KHvDMACHC_SDR = 0xd0292600; // Settings for SD0 Read channel 
const TUint KHvDMACHC_SDW = 0xd0922610; // Settings for SD1 Write channel

const TUint KHsDMACHC_HP = 	28;	    // Sets the values output to HPROT[3:0] during DMA transfer. 1101 initial value
const TUint KHvDMACHC_HP =  0xd;    // initial value for KHsDMACHC_HP bits 
const TUint KHtDMACHC_TM = 	(1<<24);  // transfer mode- 0:single transfer, 1:block transfer (memory-to-memory transfers only!)
const TUint KHtDMACHC_DAD = (1<<23);  // destination address count direction- 0:increment, 1:fixed
const TUint KHtDMACHC_SAD = (1<<19);  // source address count direction- 0:increment, 1:fixed
const TUint KHsDMACHC_DDS = 20;	    // transfer data size at the DMA transfer destination.
const TUint KHsDMACHC_SDS = 16;	    // transfer data size at the DMA transfer source.
/*	000: Byte (8 bits) (initial value)
	001: Half-word (16 bits)
	010: Word (32 bits)
	100: 4 words (128 bits)
	101: 8 words (256 bits)
	110: 16 words (512 bits)
	Others: Setting prohibited */
const TUint KHtDMACHC_TCM = (1<<14); // Masks output to DMATCO[n]: 0: not mask the output, 1:mast the output	

const TUint KHsDMACHC_AM  = 12; // Sets the DMAACK[n] output timing, 00: Pulse mode (active for one clock), 
								// 01: Level mode (active as long as the selected DMAREQ inputs remain) 1x: Masks DMAACK[n].		
const TUint KHtDMACHC_LVL  = (1<<10); // DMA request is detected 1: at the level or 0: edge of the signal.
const TUint KHtDMACHC_HIEN = (1<<9);  // 1: high level or rising edge of signal, 0: does not recognize	
const TUint KHtDMACHC_LOEN = (1<<8);  // 1: low level or falling edge of signal, 0: does not recognize	
const TUint KHtDMACHC_REQD = (1<<4);  // Selects whether DMAREQ SEL bit is on the source or 	destination side.
				/* Also selects the timing at which DMAACK becomes active.
				0: Source side. DMAACK becomes active upon read (initial value).
				1: Destination side. DMAACK becomes active upon write.*/	
const TUint KHsDMACHC_SEL  = 0; // Selects one of eight DMAREQ/DMAACK/DMATCO signals.
	
// Settings for I2S TX channels
const TUint KHvDMACHC_I2SW =  
		KHvDMACHC_HP << KHsDMACHC_HP  | // use initial value for KHsDMACHC_HP bits
					    KHtDMACHC_DAD | // destination address fixed, increment source only
				1	 << KHsDMACHC_DDS | // transfer data size at the DMA transfer destination.
				1	 << KHsDMACHC_SDS | // transfer data size at the DMA transfer source.
				0	 << KHsDMACHC_AM  | // 00: Pulse mode (active for one clock),01: Level mode (active as long as the selected DMAREQ inputs remain) 1x: Masks DMAACK[n].	
					    KHtDMACHC_LVL | // DMA request is detected 1: at the level or 0: edge of the signal.
					    KHtDMACHC_HIEN| // 1: high level or rising edge of signal, 0: does not recognize
				0	 << KHsDMACHC_SEL;
					   
				
// Settings for I2S RX channels
const TUint KHvDMACHC_I2SR =  
		KHvDMACHC_HP << KHsDMACHC_HP  | // use initial value for KHsDMACHC_HP bits
					    KHtDMACHC_SAD | // source address fixed, increment destination only
				1	 << KHsDMACHC_DDS | // transfer data size at the DMA transfer destination.
				1	 << KHsDMACHC_SDS | // transfer data size at the DMA transfer source.
				0	 << KHsDMACHC_AM  | // 00: Pulse mode (active for one clock),01: Level mode (active as long as the selected DMAREQ inputs remain) 1x: Masks DMAACK[n].	
					    KHtDMACHC_LVL | // DMA request is detected 1: at the level or 0: edge of the signal.
					    KHtDMACHC_HIEN| // 1: high level or rising edge of signal, 0: does not recognize
				0	 << KHsDMACHC_SEL;
					   	
	
const TUint KHoDMACHS	= 0x1c; // Channel Status Register
const TUint KHtDMACHS_EN		= 0x001;// Enables/disables DMA transfer.
const TUint KHtDMACHS_EN_EN	= 0x002;// Enables writing to EN bit.
const TUint KHtDMACHS_STG	= 0x004;// SW activation of DMA transfer.
const TUint KHtDMACHS_FCLR	= 0x008;// Clears TC, END, ERR & RQST bits.
const TUint KHtDMACHS_RQST	= 0x010;// Indicates that transfer request has been received - RO.
const TUint KHtDMACHS_ACT 	= 0x020;// Indicates that DMAC is performing transfer - RO.
const TUint KHtDMACHS_ERR 	= 0x040;// Error in transfer - RO.
const TUint KHtDMACHS_END 	= 0x080;// DMA transfer from working set is completed.
const TUint KHtDMACHS_TC 	= 0x100;// DMA transfer from working set is completed while BVALID is 0.
const TUint KHtDMACHS_BVALID = 0x200;// Indicates valid values in base set.
const TUint KHoDMACONT	= 0x300; // Control Register
const TUint KHvDMACONT_INIT_VALUE	= 0;
const TUint KHoDMASTAT	= 0x304; // Status Register
const TUint KHmDSTAT_TC_DMA32	= 0xff000000;
const TUint KHsDSTAT_TC			= 24;
const TUint KHsDSTAT_END		= 20;
const TUint KHsDSTAT_ER			= 16;
const TUint KHmDSTAT_DMA32		= 0x01000001;

	
static const TUint KMaxDMAUnitTransferLen = 0x10000;// max DMA transfer length in units. 

/**
Constants for the 64 bit AXI DMA controller
*/
namespace Dma64
{
	const TInt KChannelCount = 4; // The number of channels on the controller.

	/**
	Hardware definitions used for DMA channels
	*/
	namespace Channel
	{
		/**
		Register set offsets
		*/
		namespace RegSet
		{
			enum TRegSets {ENext0, ENext1, ECurrent, ERegSetCount};
			const TUint32 KHoBases[ERegSetCount] = {0x0, 0xc, 0x18};
			const TUint32 KHoSrcAddr = 0x0;
			const TUint32 KHoDstAddr = 0x4;
			const TUint32 KHoTranByte = 0x8; // Transaction byte register
		};

		/**
		Base offsets for each channel
		*/
		const TUint32 KHoBases[KChannelCount] = { 0x0, 0x40, 0x80, 0xC0 };

		/**
		Offset and bit definitions for channel status register
		*/
		namespace Status
		{
			const TUint32 KHoBase = 0x24;

			const TUint32 KHtDescErr = KBit10;
			const TUint32 KHtDescWb = KBit9;
			const TUint32 KHtDescLoad = KBit8;
			const TUint32 KHtTc = KBit6;
			const TUint32 KHtEnd = KBit5;
			const TUint32 KHtSuspended = KBit3;
			const TUint32 KHtAct = KBit2;
			const TUint32 KHtRqst = KBit1;
			const TUint32 KHtEnabled = KBit0;
		}

		/**
		Offset and bit definitions for channel control register
		*/
		namespace Ctrl
		{
			const TUint32 KHoBase = 0x28;

			const TUint32 KHtSetEnable = KBit0;
			const TUint32 KHtSwTrigger = KBit2;
			const TUint32 KHtSwReset = KBit3;
			const TUint32 KHtClrEnd = KBit5;
			const TUint32 KHtClrTc = KBit6; //Clear the transfer complete status
			const TUint32 KHtSetSuspend = KBit8;
			const TUint32 KHtClrSuspend = KBit9;
		};

		/**
		Offset and bit definitions for channel configuration register
		*/
		namespace Cfg
		{
			const TUint32 KHoBase = 0x2c;

			const TUint32 KHtLinkMode = KBit31;
			const TUint32 KHtCompMask = KBit25;
			const TUint32 KHtEndMask = KBit24;
			const TUint32 KHtTransMode = KBit22;

			const TUint32 KHsDestDataSize = 16;
			const TUint32 KHsSrcDataSize = 12;

			const TUint32 KHmDestDataSize = (0xF << KHsDestDataSize);
			const TUint32 KHmSrcDataSize = (0xF << KHsSrcDataSize);
		};

		/**
		Offset and bit definitions for channel interval register
		*/
		namespace Interval
			{
			const TUint32 KHo = 0x30;

			const TUint32 KHm = 0x0000FFFF;
			};

		const TUint32 KHoExt = 0x34;

		const TUint32 KHoNxtLnkAddr = 0x38;
		const TUint32 KHoCurrtLnkAddr = 0x3c;
	};

	/**
	Registers common to all channels
	*/
	namespace Cmn
	{
		const TUint32 KHoCtrl = 0x300;
		const TUint32 KHoEn = 0x310;
		const TUint32 KHoErr = 0x314;
		const TUint32 KHoEnd = 0x318;
		const TUint32 KHoTc = 0x31C;
		const TUint32 KHoSus = 0x320;
	};

	const TUint KDma64MaxTransferBytes = KMaxTUint32;

/**
   Layout of the 64 bit DMAC's transfer descriptor
*/
	struct TDma64Desc
		{
		TDma64Desc() {Clear();}
		void Clear() {memclr(this, sizeof(*this));}

		enum TTransferSize
			{
			E512Bit = 0x6,
			};

		void SetSourceDataSize(TTransferSize aTransferSize)
			{
			iConfig &= ~Channel::Cfg::KHmSrcDataSize;
			iConfig |= (aTransferSize << Channel::Cfg::KHsSrcDataSize);
			}

		void SetDestDataSize(TTransferSize aTransferSize)
			{
			iConfig &= ~Channel::Cfg::KHmDestDataSize;
			iConfig |= (aTransferSize << Channel::Cfg::KHsDestDataSize);
			}

#ifdef _DEBUG
		void Print()
			{
			FUNC_LOG;
			PRINT(iHeader);
			PRINT(iSrcAddr);
			PRINT(iDestAddr);
			PRINT(iTransactionByte);
			PRINT(iConfig);
			PRINT(iInterval);
			PRINT(iExtension);
			PRINT(iNextLink);

			PRINT(Epoc::LinearToPhysical((TUint32)this));
			}
#endif

		TUint32 iHeader;
		TUint32 iSrcAddr;
		TUint32 iDestAddr;
		TUint32 iTransactionByte;
		TUint32 iConfig;
		TUint32 iInterval;
		TUint32 iExtension;
		TUint32 iNextLink;
		};

	/** Bit defs for hardware descriptor header */
	namespace HwDesHeader
	{
		const TUint32 KHtDim = KBit3; // Descriptor header load interrupt mask: 1: do not issue irq; 0: issue irq.
		const TUint32 KHtWbd = KBit2; // Write back disable. Set to 1 to disable writeback
		const TUint32 KHtLe = KBit1; // 1: This is the final link
		const TUint32 KHtLv = KBit0; // 1: This link is valid
	};

};

#endif	// #ifndef __NAVIENGINEDMA_H__
