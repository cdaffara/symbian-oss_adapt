// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// omap3530/assp/inc/omap3530_irqmap.h
//

#ifndef OMAP3530_IRQMAP_H
#define OMAP3530_IRQMAP_H

#include <assp/omap3530_assp/omap3530_hardware_base.h>

										   
#define INTCPS_BASE 			Omap3530HwBase::KL4_Core +   0x200000 
#define INTCPS_SYSCONFIG  		INTCPS_BASE + 0x10 
#define INTCPS_SYSSTATUS  		INTCPS_BASE + 0x14
#define INTCPS_PROTECTION		INTCPS_BASE + 0x4c
#define INTCPS_IRQ_PRIORITY		INTCPS_BASE + 0x60
#define INTCPS_FIQ_PRIORITY		INTCPS_BASE + 0x64


#define INTCPS_ITR(n)			(INTCPS_BASE +  0x80  +( 0x20 *n))
#define INTCPS_THRESHOLD		INTCPS_BASE + 0x64
#define INTCPS_IDLE 			INTCPS_BASE + 0x50
//#define INTC_INIT_REGISTER1 	0x470C8010
//#define INTC_INIT_REGISTER2 	0x470C8050 
#define INTCPS_ILRM(n)			(INTCPS_BASE + 0x100 +(0x04 *n))

//current interrupt vector & clear regs
#define INTCPS_SIR_IRQ			INTCPS_BASE + 0x40
#define INTCPS_SIR_FIQ			INTCPS_BASE + 0x44
#define INTCPS_CONTROL			INTCPS_BASE + 0x48


#define INTCPS_PENDING_IRQ(n)	(INTCPS_BASE + 0x98 + (0x20 * n))
#define INTCPS_PENDING_FIQ(n)	(INTCPS_BASE + 0x9c + (0x20 * n))
//masks on /off 
#define INTCPS_MIRn(n)			(INTCPS_BASE + 0x084 + (n *0x20))
#define INTCPS_MIR_SETn(n)		(INTCPS_BASE + 0x08c + (n * 0x20))
#define INTCPS_MIR_CLEARn(n)	(INTCPS_BASE + 0x088 + (n *0x20))


#define INTCPS_ISRSET(n) 		(INTCPS_BASE + 0x090 + (n *0x20))
#define INTCPS_ISR_CLEAR(n)		(INTCPS_BASE + 0x094 + (n *0x20))

//regvals
#define INTCPS_SYSCONFIG_AUTOIDLE 	0x1
#define INTCPS_IDLE_FUNCIDLE		0x0
#define INTCPS_IDLE_TURBO			0x1
#define INTCPS_ILRM_DEF_PRI			(0x1 <<2)
#define INTCPS_ILRM_ROUTE_FIQ		0x1
#define INTCPS_ILRM_ROUTE_IRQ       0x00
#define INTCPS_MIR_ALL_UNSET		0x00000000
#define INTCPS_MIR_ALL_SET			0xffffffff

#define INTCPS_CONTROL_IRQ_CLEAR	0x1
#define INTCPS_CONTROL_FIQ_CLEAR    (0x1 << 1)
#define INTCPS_INIT_RG_LOW_PWR		0x1			
#define INTCPS_PENDING_MASK			0x7f



// Base of each interrupt range supported within the ASSP layer
// Used to index the correct interrupt handler object
enum TIrqRangeIndex
	{
	EIrqRangeBaseCore,	// main interrupt controller
	EIrqRangeBasePrcm,	// PRCM sub-controller interrupt sources
	EIrqRangeBaseGpio,	// GPIO sub-controller interrupt sources
	EIrqRangeBasePsu,	// Place-holder for off-board PSU device, reserved here because
						// we know there will always be one  (probably a TPD65950 or similar)

	KIrqRangeCount
	};

const TInt	KIrqRangeIndexShift		= 16;
const TInt	KIrqNumberMask			= 0xFFFF;

/** Class defining an interrupt dispatcher */
class MInterruptDispatcher
	{
	public:
		IMPORT_C void Register( TIrqRangeIndex aIndex );
		
		virtual TInt Bind(TInt aId, TIsr aIsr, TAny* aPtr) = 0;
		virtual TInt Unbind(TInt aId) = 0;
		virtual TInt Enable(TInt aId) = 0;
		virtual TInt Disable(TInt aId) = 0;
		virtual TInt Clear(TInt aId) = 0;
		virtual TInt SetPriority(TInt aId, TInt aPriority) = 0;
	};

/*
(1) All the IRQ signals are active at low level.
(2) These interrupts are internally generated within the MPU subsystem.

Table 10-4. Interrupt Mapping to the MPU Subsystem (continued)
*/
enum TOmap3530_IRQ {

	EOmap3530_IRQ0_EMUINT = (EIrqRangeBaseCore << KIrqRangeIndexShift),  				//MPU emulation(2)
	EOmap3530_IRQ1_COMMTX, 					//MPU emulation(2)
	EOmap3530_IRQ2_COMMRX, 					//MPU emulation(2)
	EOmap3530_IRQ3_BENCH, 					//MPU emulation(2)
	EOmap3530_IRQ4_MCBSP2_ST_IRQ, 			//Sidetone MCBSP2 overflow
	EOmap3530_IRQ5_MCBSP3_ST_IRQ, 			//Sidetone MCBSP3 overflow
	EOmap3530_IRQ6_SSM_ABORT_IRQ, 			//MPU subsystem secure state-machine abort (2)
	EOmap3530_IRQ7_SYS_NIRQ, 				//External source (active low)
	EOmap3530_IRQ8_RESERVED, 				//RESERVED
	EOmap3530_IRQ9_SMX_DBG_IRQ, 			//SMX error for debug
	EOmap3530_IRQ10_SMX_APP_IRQ, 			//SMX error for application
	EOmap3530_IRQ11_PRCM_MPU_IRQ, 			//PRCM module IRQ
	EOmap3530_IRQ12_SDMA_IRQ0, 				//System DMA request 0(3)
	EOmap3530_IRQ13_SDMA_IRQ1, 				//System DMA request 1(3)
	EOmap3530_IRQ14_SDMA_IRQ2, 				//System DMA request 2
	EOmap3530_IRQ15_SDMA_IRQ3, 				//System DMA request 3
	EOmap3530_IRQ16_MCBSP1_IRQ, 			//McBSP module 1 IRQ (3)
	EOmap3530_IRQ17_MCBSP2_IRQ, 			//McBSP module 2 IRQ (3)
	EOmap3530_IRQ18_SR1_IRQ, 				//SmartReflex™ 1
	EOmap3530_IRQ19_SR2_IRQ, 				//SmartReflex™ 2
	EOmap3530_IRQ20_GPMC_IRQ, 				//General-purpose memory controller module
	EOmap3530_IRQ21_SGX_IRQ, 				//2D/3D graphics module
	EOmap3530_IRQ22_MCBSP3_IRQ, 			//McBSP module 3(3)
	EOmap3530_IRQ23_MCBSP4_IRQ, 			//McBSP module 4(3)
	EOmap3530_IRQ24_CAEM_IRQ0, 				//Camera interface request 0
	EOmap3530_IRQ25_DSS_IRQ, 				//Display subsystem module(3)
	EOmap3530_IRQ26_MAIL_U0_MPU_IRQ, 		//Mailbox user 0 request
	EOmap3530_IRQ27_MCBSP5_IRQ, 			//McBSP module 5 (3)
	EOmap3530_IRQ28_IVA2_MMU_IRQ, 			//IVA2 MMU
	EOmap3530_IRQ29_GPIO1_MPU_IRQ, 			//GPIO module 1(3)
	EOmap3530_IRQ30_GPIO2_MPU_IRQ, 			//GPIO module 2(3)
	EOmap3530_IRQ31_GPIO3_MPU_IRQ, 			//GPIO module 3(3)
	EOmap3530_IRQ32_GPIO4_MPU_IRQ, 			//GPIO module 4(3)
	EOmap3530_IRQ33_GPIO5_MPU_IRQ, 			//GPIO module 5(3)
	EOmap3530_IRQ34_GPIO6_MPU_IRQ, 			//GPIO module 6(3)
	EOmap3530_IRQ35_USIEM_IRQ,		 		//USIM interrupt (HS devices only) (4)
	EOmap3530_IRQ36_WDT3_IRQ, 				//Watchdog timer module 3 overflow
	EOmap3530_IRQ37_GPT1_IRQ, 				//General-purpose timer module 1
	EOmap3530_IRQ38_GPT2_IRQ, 				//General-purpose timer module 2
	EOmap3530_IRQ39_GPT3_IRQ, 				//General-purpose timer module 3
	EOmap3530_IRQ40_GPT4_IRQ, 				//General-purpose timer module 4
	EOmap3530_IRQ41_GPT5_IRQ, 				//General-purpose timer module 5(3)
	EOmap3530_IRQ42_GPT6_IRQ, 				//General-purpose timer module 6(3)
	EOmap3530_IRQ43_GPT7_IRQ, 				//General-purpose timer module 7(3)
	EOmap3530_IRQ44_GPT8_IRQ, 				//General-purpose timer module 8(3)
	EOmap3530_IRQ45_GPT9_IRQ, 				//General-purpose timer module 9
	EOmap3530_IRQ46_GPT10_IRQ, 				//General-purpose timer module 10
	EOmap3530_IRQ47_GPT11_IRQ, 				//General-purpose timer module 11
	EOmap3530_IRQ48_SPI4_IRQ, 				//McSPI module 4
	EOmap3530_IRQ49_SHA1MD5_IRQ2, 			//SHA-1/MD5 crypto-accelerator 2 (HS devices only)(4)
	EOmap3530_IRQ50_FPKA_IRQREADY_N, 		//PKA crypto-accelerator (HS devices only) (4)
	EOmap3530_IRQ51_SHA2MD5_IRQ, 			//SHA-2/MD5 crypto-accelerator 1 (HS devices only) (4)
	EOmap3530_IRQ52_RNG_IRQ, 				//RNG module (HS devices only) (4)
	EOmap3530_IRQ53_MG_IRQ, 				//MG function (3)
	EOmap3530_IRQ54_MCBSP4_IRQTX, 			//McBSP module 4 transmit(3)
	EOmap3530_IRQ55_MCBSP4_IRQRX, 			//McBSP module 4 receive(3)
	EOmap3530_IRQ56_I2C1_IRQ, 				//I2C module 1
	EOmap3530_IRQ57_I2C2_IRQ, 				//I2C module 2
	EOmap3530_IRQ58_HDQ_IRQ, 				//HDQ/One-wire
	EOmap3530_IRQ59_McBSP1_IRQTX, 			//McBSP module 1 transmit(3)
	EOmap3530_IRQ60_McBSP1_IRQRX, 			//McBSP module 1 receive(3)
	EOmap3530_IRQ61_I2C3_IRQ, 				//I2C module 3
	EOmap3530_IRQ62_McBSP2_IRQTX, 			//McBSP module 2 transmit(3)
	EOmap3530_IRQ63_McBSP2_IRQRX, 			//McBSP module 2 receive(3)
	EOmap3530_IRQ64_FPKA_IRQRERROR_N, 		//PKA crypto-accelerator (HS devices only) (4)
	EOmap3530_IRQ65_SPI1_IRQ, 				//McSPI module 1
	EOmap3530_IRQ66_SPI2_IRQ, 				//McSPI module 2
	EOmap3530_IRQ67_RESERVED, 				//RESERVED
	EOmap3530_IRQ68_RESERVED, 				//RESERVED
	EOmap3530_IRQ69_RESERVED, 				//RESERVED
	EOmap3530_IRQ70_RESERVED, 				//RESERVED
	EOmap3530_IRQ71_RESERVED, 				//RESERVED
	EOmap3530_IRQ72_UART1_IRQ, 				//UART module 1
	EOmap3530_IRQ73_UART2_IRQ, 				//UART module 2
	EOmap3530_IRQ74_UART3_IRQ, 				//UART module 3 (also infrared)(3)
	EOmap3530_IRQ75_PBIAS_IRQ, 				//Merged interrupt for PBIASlite1 and 2
	EOmap3530_IRQ76_OHCI_IRQ, 				//OHCI controller HSUSB MP Host Interrupt
	EOmap3530_IRQ77_EHCI_IRQ, 				//EHCI controller HSUSB MP Host Interrupt
	EOmap3530_IRQ78_TLL_IRQ, 				//HSUSB MP TLL Interrupt
	EOmap3530_IRQ79_PARTHASH_IRQ, 			//SHA2/MD5 crypto-accelerator 1 (HS devices only) (4)
	EOmap3530_IRQ80_RESERVED, 				//Reserved
	EOmap3530_IRQ81_MCBSP5_IRQTX, 			//McBSP module 5 transmit(3)
	EOmap3530_IRQ82_MCBSP5_IRQRX, 			//McBSP module 5 receive(3)
	EOmap3530_IRQ83_MMC1_IRQ, 				//MMC/SD module 1
	EOmap3530_IRQ84_MS_IRQ, 				//MS-PRO module
	EOmap3530_IRQ85_RESERVED, 				//Reserved
	EOmap3530_IRQ86_MMC2_IRQ, 				//MMC/SD module 2
	EOmap3530_IRQ87_MPU_ICR_IRQ, 			//MPU ICR
	EOmap3530_IRQ88_RESERVED, 				//RESERVED
	EOmap3530_IRQ89_MCBSP3_IRQTX, 			//McBSP module 3 transmit(3)
	EOmap3530_IRQ90_MCBSP3_IRQRX, 			//McBSP module 3 receive(3)
	EOmap3530_IRQ91_SPI3_IRQ, 				//McSPI module 3
	EOmap3530_IRQ92_HSUSB_MC_NINT, 			//High-Speed USB OTG controller
	EOmap3530_IRQ93_HSUSB_DMA_NINT, 		//High-Speed USB OTG DMA controller
	EOmap3530_IRQ94_MMC3_IRQ, 				//MMC/SD module 3
	EOmap3530_IRQ95_GPT12_IRQ, 				//General-purpose timer module 12

// IRQ virtual IDs
	EOmap3530_GPIOIRQ_FIRST,

	EOmap3530_GPIOIRQ_PIN_0,
	EOmap3530_GPIOIRQ_PIN_1,
	EOmap3530_GPIOIRQ_PIN_2,
	EOmap3530_GPIOIRQ_PIN_3,
	EOmap3530_GPIOIRQ_PIN_4,
	EOmap3530_GPIOIRQ_PIN_5,	
	EOmap3530_GPIOIRQ_PIN_6,
	EOmap3530_GPIOIRQ_PIN_7,
	EOmap3530_GPIOIRQ_PIN_8,
	EOmap3530_GPIOIRQ_PIN_9,
	EOmap3530_GPIOIRQ_PIN_10,
	EOmap3530_GPIOIRQ_PIN_11,
	EOmap3530_GPIOIRQ_PIN_12,
	EOmap3530_GPIOIRQ_PIN_13,
	EOmap3530_GPIOIRQ_PIN_14,
	EOmap3530_GPIOIRQ_PIN_15,
	EOmap3530_GPIOIRQ_PIN_16,
	EOmap3530_GPIOIRQ_PIN_17,
	EOmap3530_GPIOIRQ_PIN_18,
	EOmap3530_GPIOIRQ_PIN_19,
	EOmap3530_GPIOIRQ_PIN_20,
	EOmap3530_GPIOIRQ_PIN_21,
	EOmap3530_GPIOIRQ_PIN_22,
	EOmap3530_GPIOIRQ_PIN_23,
	EOmap3530_GPIOIRQ_PIN_24,
	EOmap3530_GPIOIRQ_PIN_25,
	EOmap3530_GPIOIRQ_PIN_26,
	EOmap3530_GPIOIRQ_PIN_27,
	EOmap3530_GPIOIRQ_PIN_28,
	EOmap3530_GPIOIRQ_PIN_29,
	EOmap3530_GPIOIRQ_PIN_30,
	EOmap3530_GPIOIRQ_PIN_31,
	EOmap3530_GPIOIRQ_PIN_32,
	EOmap3530_GPIOIRQ_PIN_33,
	EOmap3530_GPIOIRQ_PIN_34,
	EOmap3530_GPIOIRQ_PIN_35,
	EOmap3530_GPIOIRQ_PIN_36,
	EOmap3530_GPIOIRQ_PIN_37,
	EOmap3530_GPIOIRQ_PIN_38,
	EOmap3530_GPIOIRQ_PIN_39,
	EOmap3530_GPIOIRQ_PIN_40,
	EOmap3530_GPIOIRQ_PIN_41,
	EOmap3530_GPIOIRQ_PIN_42,
	EOmap3530_GPIOIRQ_PIN_43,
	EOmap3530_GPIOIRQ_PIN_44,
	EOmap3530_GPIOIRQ_PIN_45,
	EOmap3530_GPIOIRQ_PIN_46,
	EOmap3530_GPIOIRQ_PIN_47,
	EOmap3530_GPIOIRQ_PIN_48,
	EOmap3530_GPIOIRQ_PIN_49,
	EOmap3530_GPIOIRQ_PIN_50,
	EOmap3530_GPIOIRQ_PIN_51,
	EOmap3530_GPIOIRQ_PIN_52,
	EOmap3530_GPIOIRQ_PIN_53,
	EOmap3530_GPIOIRQ_PIN_54,
	EOmap3530_GPIOIRQ_PIN_55,
	EOmap3530_GPIOIRQ_PIN_56,
	EOmap3530_GPIOIRQ_PIN_57,
	EOmap3530_GPIOIRQ_PIN_58,
	EOmap3530_GPIOIRQ_PIN_59,
	EOmap3530_GPIOIRQ_PIN_60,
	EOmap3530_GPIOIRQ_PIN_61,
	EOmap3530_GPIOIRQ_PIN_62,
	EOmap3530_GPIOIRQ_PIN_63,
	EOmap3530_GPIOIRQ_PIN_64,
	EOmap3530_GPIOIRQ_PIN_65,
	EOmap3530_GPIOIRQ_PIN_66,
	EOmap3530_GPIOIRQ_PIN_67,
	EOmap3530_GPIOIRQ_PIN_68,
	EOmap3530_GPIOIRQ_PIN_69,
	EOmap3530_GPIOIRQ_PIN_70,
	EOmap3530_GPIOIRQ_PIN_71,
	EOmap3530_GPIOIRQ_PIN_72,
	EOmap3530_GPIOIRQ_PIN_73,
	EOmap3530_GPIOIRQ_PIN_74,
	EOmap3530_GPIOIRQ_PIN_75,
	EOmap3530_GPIOIRQ_PIN_76,
	EOmap3530_GPIOIRQ_PIN_77,
	EOmap3530_GPIOIRQ_PIN_78,
	EOmap3530_GPIOIRQ_PIN_79,
	EOmap3530_GPIOIRQ_PIN_80,
	EOmap3530_GPIOIRQ_PIN_81,
	EOmap3530_GPIOIRQ_PIN_82,
	EOmap3530_GPIOIRQ_PIN_83,
	EOmap3530_GPIOIRQ_PIN_84,
	EOmap3530_GPIOIRQ_PIN_85,
	EOmap3530_GPIOIRQ_PIN_86,
	EOmap3530_GPIOIRQ_PIN_87,
	EOmap3530_GPIOIRQ_PIN_88,
	EOmap3530_GPIOIRQ_PIN_89,
	EOmap3530_GPIOIRQ_PIN_90,
	EOmap3530_GPIOIRQ_PIN_91,
	EOmap3530_GPIOIRQ_PIN_92,
	EOmap3530_GPIOIRQ_PIN_93,
	EOmap3530_GPIOIRQ_PIN_94,
	EOmap3530_GPIOIRQ_PIN_95,
	EOmap3530_GPIOIRQ_PIN_96,
	EOmap3530_GPIOIRQ_PIN_97,
	EOmap3530_GPIOIRQ_PIN_98,
	EOmap3530_GPIOIRQ_PIN_99,
	EOmap3530_GPIOIRQ_PIN_100,
	EOmap3530_GPIOIRQ_PIN_101,
	EOmap3530_GPIOIRQ_PIN_102,
	EOmap3530_GPIOIRQ_PIN_103,
	EOmap3530_GPIOIRQ_PIN_104,
	EOmap3530_GPIOIRQ_PIN_105,
	EOmap3530_GPIOIRQ_PIN_106,
	EOmap3530_GPIOIRQ_PIN_107,
	EOmap3530_GPIOIRQ_PIN_108,
	EOmap3530_GPIOIRQ_PIN_109,
	EOmap3530_GPIOIRQ_PIN_110,
	EOmap3530_GPIOIRQ_PIN_111,
	EOmap3530_GPIOIRQ_PIN_112,
	EOmap3530_GPIOIRQ_PIN_113,
	EOmap3530_GPIOIRQ_PIN_114,
	EOmap3530_GPIOIRQ_PIN_115,
	EOmap3530_GPIOIRQ_PIN_116,
	EOmap3530_GPIOIRQ_PIN_117,
	EOmap3530_GPIOIRQ_PIN_118,
	EOmap3530_GPIOIRQ_PIN_119,
	EOmap3530_GPIOIRQ_PIN_120,
	EOmap3530_GPIOIRQ_PIN_121,
	EOmap3530_GPIOIRQ_PIN_122,
	EOmap3530_GPIOIRQ_PIN_123,
	EOmap3530_GPIOIRQ_PIN_124,
	EOmap3530_GPIOIRQ_PIN_125,
	EOmap3530_GPIOIRQ_PIN_126,
	EOmap3530_GPIOIRQ_PIN_127,
	EOmap3530_GPIOIRQ_PIN_128,
	EOmap3530_GPIOIRQ_PIN_129,
	EOmap3530_GPIOIRQ_PIN_130,
	EOmap3530_GPIOIRQ_PIN_131,
	EOmap3530_GPIOIRQ_PIN_132,
	EOmap3530_GPIOIRQ_PIN_133,
	EOmap3530_GPIOIRQ_PIN_134,
	EOmap3530_GPIOIRQ_PIN_135,
	EOmap3530_GPIOIRQ_PIN_136,
	EOmap3530_GPIOIRQ_PIN_137,
	EOmap3530_GPIOIRQ_PIN_138,
	EOmap3530_GPIOIRQ_PIN_139,
	EOmap3530_GPIOIRQ_PIN_140,
	EOmap3530_GPIOIRQ_PIN_141,
	EOmap3530_GPIOIRQ_PIN_142,
	EOmap3530_GPIOIRQ_PIN_143,
	EOmap3530_GPIOIRQ_PIN_144,
	EOmap3530_GPIOIRQ_PIN_145,
	EOmap3530_GPIOIRQ_PIN_146,
	EOmap3530_GPIOIRQ_PIN_147,
	EOmap3530_GPIOIRQ_PIN_148,
	EOmap3530_GPIOIRQ_PIN_149,
	EOmap3530_GPIOIRQ_PIN_150,
	EOmap3530_GPIOIRQ_PIN_151,
	EOmap3530_GPIOIRQ_PIN_152,
	EOmap3530_GPIOIRQ_PIN_153,
	EOmap3530_GPIOIRQ_PIN_154,
	EOmap3530_GPIOIRQ_PIN_155,
	EOmap3530_GPIOIRQ_PIN_156,
	EOmap3530_GPIOIRQ_PIN_157,
	EOmap3530_GPIOIRQ_PIN_158,
	EOmap3530_GPIOIRQ_PIN_159,
	EOmap3530_GPIOIRQ_PIN_160,
	EOmap3530_GPIOIRQ_PIN_161,
	EOmap3530_GPIOIRQ_PIN_162,
	EOmap3530_GPIOIRQ_PIN_163,
	EOmap3530_GPIOIRQ_PIN_164,
	EOmap3530_GPIOIRQ_PIN_165,
	EOmap3530_GPIOIRQ_PIN_166,
	EOmap3530_GPIOIRQ_PIN_167,
	EOmap3530_GPIOIRQ_PIN_168,
	EOmap3530_GPIOIRQ_PIN_169,
	EOmap3530_GPIOIRQ_PIN_170,
	EOmap3530_GPIOIRQ_PIN_171,
	EOmap3530_GPIOIRQ_PIN_172,
	EOmap3530_GPIOIRQ_PIN_173,
	EOmap3530_GPIOIRQ_PIN_174,
	EOmap3530_GPIOIRQ_PIN_175,
	EOmap3530_GPIOIRQ_PIN_176,
	EOmap3530_GPIOIRQ_PIN_177,
	EOmap3530_GPIOIRQ_PIN_178,
	EOmap3530_GPIOIRQ_PIN_179,
	EOmap3530_GPIOIRQ_PIN_180,
	EOmap3530_GPIOIRQ_PIN_181,
	EOmap3530_GPIOIRQ_PIN_182,
	EOmap3530_GPIOIRQ_PIN_183,
	EOmap3530_GPIOIRQ_PIN_184,
	EOmap3530_GPIOIRQ_PIN_185,
	EOmap3530_GPIOIRQ_PIN_186,
	EOmap3530_GPIOIRQ_PIN_187,
	EOmap3530_GPIOIRQ_PIN_188,
	EOmap3530_GPIOIRQ_PIN_189,
	EOmap3530_GPIOIRQ_PIN_190,
	EOmap3530_GPIOIRQ_PIN_191,
	
	EOmap3530_GPIOIRQ_TOTAL,

	EOmap3530_TOTAL_IRQS
};



const TInt KNumOmap3530Ints = (EOmap3530_GPIOIRQ_FIRST -1);

const TInt KOmap3530MaxIntPriority =0;
const TInt KOmap3530MinIntPriority =63;
const TInt KOmap3530DefIntPriority =KOmap3530MinIntPriority /2;
IMPORT_C void ClearAndDisableTestInterrupt(TInt anId);
IMPORT_C void TestInterrupts(TInt id,TIsr func);


#endif /*Omap3530_IRQMAP_H*/
