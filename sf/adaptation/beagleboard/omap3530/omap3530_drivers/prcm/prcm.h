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
// \omap3530\omap3530_assp\prcm.h
//

#ifndef PRCM_H_
#define PRCM_H_


// Dummy location for redirecting writes which have no effect on a particular clock
// More efficient than having to test for it in code
TUint32 __dummypoke;
#define	KDummy	(TUint32)&__dummypoke

// PLL modes
const TUint32 KPllModeStop	= 0x1;
const TUint32 KPllModeBypass = 0x5;
const TUint32 KPllModeFastRelock = 0x6;
const TUint32 KPllModeLock = 0x7;
const TUint32 KPllModeMask = 0x7;
const TUint32 KPllAutoOff = 0x0;
const TUint32 KPllAutoOn = 0x1;
const TUint32 KPllAutoMask = 0x7;

#ifdef _DEBUG	// to stop warings about unused definitions
const TUint	KPllMaximumDivider		= 127;
const TUint	KPllMaximumMultiplier	= 2047;
#endif
const TUint	KPllDividerMask		= 127;
const TUint	KPllMultiplierMask	= 2047;
const TUint	KPllFreqRangeMask	= 15;
const TUint	KPllRampMask		= 3;

const TUint KPllLpModeMaximumFrequency = 600000000;

// TPll to TClock lookup table
static const Prcm::TClock KPllToClock [] =
	{
	Prcm::EClkMpu,
	Prcm::EClkIva2Pll,
	Prcm::EClkCore,
	Prcm::EClkPeriph,
	Prcm::EClkPeriph2
	};

// struct of info on how to configure each PLL
// this doesn't include settings which are the same for all PLLs
struct TPllControlInfo
	{
	TUint32	iConfigRegister;		// register containing configuration settings
	TUint32	iMulDivRegister;		// register containing multiplier and divider setting
	TUint32	iStatusRegister;		// register containing PLL status
	TUint	iMultShift;				// shift to move multiplier into position
	TUint	iDivShift;				// shift to move divider into position
	TUint	iFreqSelShift;			// shift to move frequency range selection into position
	TUint	iRampShift;				// shift to move ramp bits into position
	TUint	iDriftShift;			// shift to move driftsel into position
	TUint	iLpShift;				// shift to move LP bit into position
	TUint	iLockBit;				// bit number of lock flag in iStatusRegister
	};

static const TPllControlInfo KPllControlInfo[ Prcm::KSupportedPllCount ] =
	{
	//	ConfReg				MulDivReg			StatusReg				MulShift	DivShift	FreqShift	RampShift	DriftShift	LpShift	LockBit
		{ KCM_CLKEN_PLL_MPU,  KCM_CLKSEL1_PLL_MPU,	KCM_IDLEST_PLL_MPU,		8,		0,			4,			8,			3,			10,		0 },		// DPLL1 (mpu)
		{ KCM_CLKEN_PLL_IVA2, KCM_CLKSEL1_PLL_IVA2,	KCM_IDLEST_PLL_IVA2,	8,		0,			4,			8,			3,			10,		0 },		// DPLL2 (iva2)
		{ KCM_CLKEN_PLL,	KCM_CLKSEL1_PLL,		KCM_IDLEST_CKGEN,		16,		8,			4,			8,			3,			10,		0 },		// DPLL3 (core)
		{ KCM_CLKEN_PLL,	KCM_CLKSEL2_PLL,		KCM_IDLEST_CKGEN,		8,		0,			20,			24,			19,			26,		1 },		// DPLL4 (periph)
		{ KCM_CLKEN2_PLL,	KCM_CLKSEL4_PLL,		KCM_IDLEST2_CKGEN,		8,		0,			4,			8,			3,			10,		0 }		// DPLL5 (periph2)
	};
__ASSERT_COMPILE( (sizeof(KPllControlInfo) / sizeof( KPllControlInfo[0] )) == Prcm::KSupportedPllCount );

struct TPllModeInfo
	{
	TUint32		iModeRegister;
	TUint32		iAutoRegister;
	TUint8		iModeShift;
	TUint8		iAutoShift;
	TUint8		_spare[2];
	};

static const TPllModeInfo KPllMode[] =
	{
		// iModeRegister		iAutoRegister			iModeShift	iAutoShift
		{ KCM_CLKEN_PLL_MPU,	KCM_AUTOIDLE_PLL_MPU,	0,			0 },
		{ KCM_CLKEN_PLL_IVA2,	KCM_AUTOIDLE_PLL_IVA2,	0,			0 },
		{ KCM_CLKEN_PLL,		KCM_AUTOIDLE_PLL,		0,			0 },
		{ KCM_CLKEN_PLL,		KCM_AUTOIDLE_PLL,		16,			3 },
		{ KCM_CLKEN2_PLL,		KCM_AUTOIDLE2_PLL,		0,			3 }
	};
__ASSERT_COMPILE( (sizeof(KPllMode) / sizeof( KPllMode[0] )) == Prcm::KSupportedPllCount );


// All dividers in the PRCM fall into one of these classes
// Some are unique to a particular peripheral but some
// are used by multiple peripherals so we can share that implementation
enum TDivType
	{
	EDivNotSupported,
	EDiv_1_2,
	EDivCore_1_2_4,
	EDivCore_3_4_6_96M,
	EDivPll_1_To_16,
	EDivPll_1_To_31,
	EDivUsimClk,
	EDivClkOut_1_2_4_8_16,
	};

struct TDividerInfo
	{
	TUint32		iRegister;
	TUint32		iMask;			// mask of bits to modify in register
	TDivType	iDivType : 8;
	TUint8		iShift;			// number of bits to shift to move divide value into position
	};

static const TDividerInfo KDividerInfo[] =
	{
		{ KCM_CLKSEL2_PLL_MPU,		0x1F,							EDivPll_1_To_16,	0 },	// EClkMpu,		///< DPLL1
		{ KCM_CLKSEL2_PLL_IVA2,		0x1F,							EDivPll_1_To_16,	0 },
		{ KCM_CLKSEL1_PLL,			0x1FU << 27,					EDivPll_1_To_31,	27 },	// EClkCore,		///< DPLL3
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkPeriph,		///< DPLL4
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkPeriph2,	///< DPLL5

		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkPrcmInterface,

		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkEmu,		///< Emulation clock
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkNeon,

		{ KCM_CLKSEL_CORE,			KBit0 | KBit1,					EDiv_1_2,			0 },	// EClkL3Domain,
		{ KCM_CLKSEL_CORE,			KBit2 | KBit3,					EDiv_1_2,			2 },	// EClkL4Domain,

		{ KCM_CLKSEL1_PLL_MPU,		KBit19 | KBit20 | KBit21,		EDivCore_1_2_4,		19 },	// EClkMpuPll_Bypass,	///< DPLL1 bypass frequency
		{ KCM_CLKSEL1_PLL_IVA2,		KBit19 | KBit20 | KBit21,		EDivCore_1_2_4,		19 },	// EClkIva2Pll_Bypass,	///< DPLL2 bypass frequency
		{ KCM_CLKSEL_WKUP,			KBit1 | KBit2,					EDiv_1_2,			1 },	// EClkRM_F,	///< Reset manager functional clock
		{ KCM_CLKSEL3_PLL,			0x1F,							EDivPll_1_To_16,	0 },	// EClk96M		///< 96MHz clock
		{ KCM_CLKSEL5_PLL,			0x1F,							EDivPll_1_To_16,	0 },	// EClk120M		///< 120MHz clock
		{ KCM_CLKOUT_CTRL,			KBit3 | KBit4 | KBit5,			EDivClkOut_1_2_4_8_16,	3 },	// EClkSysOut

		// Functional clocks
		{ KCM_CLKSEL_DSS,			0x1FU << 8,						EDivPll_1_To_16,	8 },	// EClkTv_F,
		{ KCM_CLKSEL_DSS,			0x1F,							EDivPll_1_To_16,	0 },	// EClkDss1_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkDss2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkCsi2_F,
		{ KCM_CLKSEL_CAM,			0x1F,							EDivPll_1_To_16,	0 },	// EClkCam_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkIva2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMmc1_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMmc2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMmc3_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMsPro_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkHdq_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp1_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp3_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp4_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp5_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcSpi1_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcSpi2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcSpi3_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcSpi4_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkI2c1_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkI2c2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkI2c3_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUart1_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUart2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUart3_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt1_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt3_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt4_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt5_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt6_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt7_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt8_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt9_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt10_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt11_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUsbTll_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkTs_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkCpeFuse_F,

		{ KCM_CLKSEL_SGX,	KBit0 | KBit1 | KBit2,					EDivCore_3_4_6_96M, 0 },	// EClkSgx_F,

		{ KCM_CLKSEL_WKUP,	KBit3 | KBit4 | KBit5 | KBit6,			EDivUsimClk,		3 },	// EClkUsim_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkSmartReflex2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkSmartReflex1_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkWdt2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkWdt3_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio1_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio2_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio3_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio4_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio5_F,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio6_F,

		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUsb120_F,		///< USB host 120MHz functional clock
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUsb48_F,		///< USB host 48MHz functional clock


	// Interface clocks
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkDss_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkCam_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkIcr_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMmc1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMmc2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMmc3_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMsPro_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkHdq_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkAes1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkAes2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkSha11_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkSha12_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkDes1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkDes2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp3_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp4_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcBsp5_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkI2c1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkI2c2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkI2c3_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUart1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUart2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUart3_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcSpi1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcSpi2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcSpi3_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMcSpi4_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt3_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt4_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt5_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt6_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt7_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt8_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt9_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt10_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt11_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpt12_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkMailboxes_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkOmapSCM_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkHsUsbOtg_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkSdrc_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkPka_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkRng_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUsbTll_I,

		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkSgx_I,

		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUsim_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkWdt1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkWdt2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkWdt3_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio1_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio2_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio3_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio4_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio5_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkGpio6_I,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClk32Sync_I,

		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkUsb_I,			///< USB host interface clock

		{ KDummy,					0,								EDivNotSupported,	0 },	// EClk48M
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClk12M

		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkSysClk,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkAltClk,
		{ KDummy,					0,								EDivNotSupported,	0 },	// EClkSysClk32k,
	};
__ASSERT_COMPILE( (sizeof(KDividerInfo) / sizeof( KDividerInfo[0] )) == Prcm::KSupportedClockCount );

// Special case divider and mux info for USIM
struct TUsimDivMuxInfo
	{
	Prcm::TClock	iClock : 8;		// source clock
	TUint8			iDivider;		// divider factor
	};
static const TUsimDivMuxInfo UsimDivMuxInfo[16] =
	{
		{ Prcm::EClkSysClk,		1 },	// 0x0
		{ Prcm::EClkSysClk,		1 },	// 0x1
		{ Prcm::EClkSysClk,		2 },	// 0x2
		{ Prcm::EClk96M,		2 },	// 0x3
		{ Prcm::EClk96M,		4 },	// 0x4
		{ Prcm::EClk96M,		8 },	// 0x5
		{ Prcm::EClk96M,		10 },	// 0x6
		{ Prcm::EClk120M,		4 },	// 0x7
		{ Prcm::EClk120M,		8 },	// 0x8
		{ Prcm::EClk120M,		16 },	// 0x9
		{ Prcm::EClk120M,		20 },	// 0xA
		{ Prcm::EClkSysClk,		1 },	// 0xB
		{ Prcm::EClkSysClk,		1 },	// 0xC
		{ Prcm::EClkSysClk,		1 },	// 0xD
		{ Prcm::EClkSysClk,		1 },	// 0xE
		{ Prcm::EClkSysClk,		1 }		// 0xF
	};

// Structure representing a register, mask and enable/disable values
struct TRegisterBitDef
	{
	TUint32	iRegister;
	TUint32	iMask;
	TUint32	iEnablePattern;
	TUint32	iDisablePattern;
	};

// Structure for holding information on clock enable and auto mode
struct TClockEnableAutoInfo
	{
	TRegisterBitDef	iGate;
	TRegisterBitDef	iAuto;
	};

const TUint32 KDummyReadAsDisabled = 1;
const TUint32 KDummyReadAsEnabled = 0;
const TUint32 KBit012	= KBit0 | KBit1 | KBit2;
const TUint32 KBit345	= KBit3 | KBit4 | KBit5;
const TUint32 KBit16_17_18 = KBit16 | KBit17 | KBit18;

// Table of bits to set to enable each clock
// Note where a function doesn't exist, use { KDummy, 0, V, 0 } which will cause a write to harmlessly write
// to __dummypoke and a read to find that the item is disabled if V==KDummyReadAsDisabled and enabled if V=KDummyReadAsEnabled
static const TClockEnableAutoInfo KClockControlTable[] =
	{
		{ { KDummy, 0, 0, 0 },						{ KCM_AUTOIDLE_PLL_MPU, KBit012, 1, 0 } },					// EClkMpu,
		{ { KCM_CLKEN_PLL_IVA2, KBit012, 7, 1 },	{ KCM_AUTOIDLE_PLL_IVA2, KBit0, 1, 0 } },					// EClkIva2Pll,
		{ { KCM_CLKEN_PLL, KBit012, 0x7, 0x5 },						{ KCM_AUTOIDLE_PLL, KBit012, 1, 0 } },		// EClkCore,		///< DPLL3
		{ { KCM_CLKEN_PLL, KBit16_17_18, KBit16_17_18, KBit16 },	{ KCM_AUTOIDLE_PLL, KBit345, KBit3, 0 } },	// EClkPeriph,		///< DPLL4
		{ { KCM_CLKEN2_PLL, KBit012, 0x7, 0x1 },					{ KCM_AUTOIDLE2_PLL, KBit012, 1, 0 } },		// EClkPeriph2,	///< DPLL5

		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClkPrcmInterface,
		{ { KDummy, 0, 0, 0 },							{ KCM_CLKSTCTRL_EMU, KBit0 | KBit1, 3, 2 } },		// EClkEmu,		///< Emulation clock
		{ { KCM_IDLEST_NEON, KBit0, 0, 1 },				{ KCM_CLKSTCTRL_NEON, KBit0 | KBit1, 3, 2 } },		// EClkNeon,

		{ { KDummy, 0, 0, 0 },							{ KCM_CLKSTCTRL_CORE, KBit0 | KBit1, KBit0 | KBit1, 0 } },		// EClkL3Domain,
		{ { KDummy, 0, 0, 0 },							{ KCM_CLKSTCTRL_CORE, KBit2 | KBit3, KBit2 | KBit3, 0 } },	// EClkL4Domain,

		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClkMpuPll_Bypass,	///< DPLL1 bypass frequency
		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClkIva2Pll_Bypass,	///< DPLL2 bypass frequency
		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClkRM_F,			///< Reset manager functional clock
		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClk96M,			///< 96MHz clock
		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClk120M,			///< 120MHz clock
		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkSysOut,

	// Functional clocks
		{ { KCM_FCLKEN_DSS, KBit2, KBit2, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkTv_F,
		{ { KCM_FCLKEN_DSS, KBit0, KBit0, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkDss1_F,
		{ { KCM_FCLKEN_DSS, KBit1, KBit1, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkDss2_F,
		{ { KCM_FCLKEN_CAM, KBit1, KBit1, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkCsi2_F,
		{ { KCM_FCLKEN_CAM, KBit0, KBit0, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkCam_F,
		{ { KCM_FCLKEN_IVA2, KBit0, KBit0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkIva2_F,
		{ { KCM_FCLKEN1_CORE, KBit24, KBit24, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMmc1_F,
		{ { KCM_FCLKEN1_CORE, KBit25, KBit25, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMmc2_F,
		{ { KCM_FCLKEN1_CORE, KBit30, KBit30, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMmc3_F,
		{ { KCM_FCLKEN1_CORE, KBit23, KBit23, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMsPro_F,
		{ { KCM_FCLKEN1_CORE, KBit22, KBit22, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkHdq_F,
		{ { KCM_FCLKEN1_CORE, KBit9, KBit9, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMcBSP1_F,
		{ { KCM_FCLKEN_PER, KBit0, KBit0, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMcBSP2_F,
		{ { KCM_FCLKEN_PER, KBit1, KBit1, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMcBSP3_F,
		{ { KCM_FCLKEN_PER, KBit2, KBit2, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMcBSP4_F,
		{ { KCM_FCLKEN1_CORE, KBit10, KBit10, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMcBSP5_F,
		{ { KCM_FCLKEN1_CORE, KBit18, KBit18, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMcSpi1_F,
		{ { KCM_FCLKEN1_CORE, KBit19, KBit19, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkMcSpi2_F,
		{ { KCM_FCLKEN1_CORE, KBit20, KBit20, 0 },	{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClkMcSpi3_F,
		{ { KCM_FCLKEN1_CORE, KBit21, KBit21, 0 },	{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClkMcSpi4_F,
		{ { KCM_FCLKEN1_CORE, KBit15, KBit15, 0},	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkI2c1_F,
		{ { KCM_FCLKEN1_CORE, KBit16, KBit16, 0},	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkI2c2_F,
		{ { KCM_FCLKEN1_CORE, KBit17, KBit17, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkI2c3_F,
		{ { KCM_FCLKEN1_CORE, KBit13, KBit13, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkUart1_F,
		{ { KCM_FCLKEN1_CORE, KBit14, KBit14, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkUart2_F,
		{ { KCM_FCLKEN_PER, KBit11, KBit11, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkUart3_F,
		{ { KCM_FCLKEN_WKUP, KBit0, KBit0, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt1_F,
		{ { KCM_FCLKEN_PER, KBit3, KBit3, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt2_F,
		{ { KCM_FCLKEN_PER, KBit4, KBit4, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt3_F,
		{ { KCM_FCLKEN_PER, KBit5, KBit5, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt4_F,
		{ { KCM_FCLKEN_PER, KBit6, KBit6, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt5_F,
		{ { KCM_FCLKEN_PER, KBit7, KBit7, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt6_F,
		{ { KCM_FCLKEN_PER, KBit8, KBit8, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt7_F,
		{ { KCM_FCLKEN_PER, KBit9, KBit9, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt8_F,
		{ { KCM_FCLKEN_PER, KBit10, KBit10, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt9_F,
		{ { KCM_FCLKEN1_CORE, KBit11, KBit11, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt10_F,
		{ { KCM_FCLKEN1_CORE, KBit12, KBit12, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpt11_F,
		{ { KCM_FCLKEN3_CORE, KBit2, KBit2, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkUsbTll_F,
		{ { KCM_FCLKEN3_CORE, KBit1, KBit1, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkTs_F,
		{ { KCM_FCLKEN3_CORE, KBit0, KBit0, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkCpeFuse_F,

		{ { KCM_FCLKEN_SGX, KBit1, KBit1, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkSgx_F,

		{ { KCM_FCLKEN_WKUP, KBit9, KBit9, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkUsim_F,
		{ { KCM_FCLKEN_WKUP, KBit7, KBit7, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkSmartReflex2_F,
		{ { KCM_FCLKEN_WKUP, KBit6, KBit6, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkSmartReflex1_F,
		{ { KCM_FCLKEN_WKUP, KBit5, KBit5, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkWdt2_F,
		{ { KCM_FCLKEN_PER, KBit12, KBit12, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkWdt3_F,
		{ { KCM_FCLKEN_WKUP, KBit3, KBit3, 0 },		{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpio1_F,
		{ { KCM_FCLKEN_PER, KBit13, KBit13, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpio2_F,
		{ { KCM_FCLKEN_PER, KBit14, KBit14, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpio3_F,
		{ { KCM_FCLKEN_PER, KBit15, KBit15, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpio4_F,
		{ { KCM_FCLKEN_PER, KBit16, KBit16, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpio5_F,
		{ { KCM_FCLKEN_PER, KBit17, KBit17, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkGpio6_F,

		{ { KCM_FCLKEN_USBHOST, KBit1, KBit1, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkUsb120_F,
		{ { KCM_FCLKEN_USBHOST, KBit0, KBit0, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },		// EClkUsb48_F,


	// Interface clocks
		{ { KCM_ICLKEN_DSS, KBit0, KBit0, 0 },		{ KCM_AUTOIDLE_DSS, KBit0, KBit0, 0 } },		// EClkDss_I,
		{ { KCM_ICLKEN_CAM, KBit0,KBit0, 0 },		{ KCM_AUTOIDLE_CAM, KBit0, KBit0, 0 } },		// EClkCam_I,
		{ { KCM_ICLKEN1_CORE, KBit29, KBit29, 0 },	{ KCM_AUTOIDLE1_CORE, KBit29, KBit29, 0 } },	// EClkIcr_I,
		{ { KCM_ICLKEN1_CORE, KBit24, KBit24, 0 },	{ KCM_AUTOIDLE1_CORE, KBit24, KBit24, 0 } },	// EClkMmc1_I,
		{ { KCM_ICLKEN1_CORE, KBit25, KBit25, 0 },	{ KCM_AUTOIDLE1_CORE, KBit25, KBit25, 0 } },	// EClkMmc2_I,
		{ { KCM_ICLKEN1_CORE, KBit30, KBit30, 0 },	{ KCM_AUTOIDLE1_CORE, KBit30, KBit30, 0 } },	// EClkMmc3_I,
		{ { KCM_ICLKEN1_CORE, KBit23, KBit23, 0 },	{ KCM_AUTOIDLE1_CORE, KBit23, KBit23, 0 } },	// EClkMsPro_I,
		{ { KCM_ICLKEN1_CORE, KBit22, KBit22, 0 },	{ KCM_AUTOIDLE1_CORE, KBit22, KBit22, 0 } },	// EClkHdq_I,
		{ { KCM_ICLKEN2_CORE, KBit3, KBit3, 0 },	{ KCM_AUTOIDLE2_CORE, KBit3, KBit3, 0 } },		// EClkAes1_I,
		{ { KCM_ICLKEN1_CORE, KBit28, KBit28, 0 },	{ KCM_AUTOIDLE1_CORE, KBit28, KBit28, 0 } },	// EClkAes2_I,
		{ { KCM_ICLKEN2_CORE, KBit1, KBit1, 0 },	{ KCM_AUTOIDLE2_CORE, KBit1, KBit1, 0 } },		// EClkSha11_I,
		{ { KCM_ICLKEN1_CORE, KBit28, KBit27, 0 },	{ KCM_AUTOIDLE1_CORE, KBit27, KBit27, 0 } },	// EClkSha12_I,
		{ { KCM_ICLKEN2_CORE, KBit0, KBit0, 0 },	{ KCM_AUTOIDLE2_CORE, KBit0, KBit0, 0 } },		// EClkDes1_I,
		{ { KCM_ICLKEN1_CORE, KBit26, KBit26, 0 },	{ KCM_AUTOIDLE1_CORE, KBit26, KBit26, 0 } },	// EClkDes2_I,
		{ { KCM_ICLKEN1_CORE, KBit9, KBit9, 0 },	{ KCM_AUTOIDLE1_CORE, KBit9, KBit9, 0 } },		// EClkMcBSP1_I,
		{ { KCM_ICLKEN_PER, KBit0, KBit0, 0},		{ KCM_AUTOIDLE_PER, KBit0, KBit0, 0 } },		// EClkMcBSP2_I,
		{ { KCM_ICLKEN_PER, KBit1, KBit1, 0 },		{ KCM_AUTOIDLE_PER, KBit1, KBit1, 0 } },		// EClkMcBSP3_I,
		{ { KCM_ICLKEN_PER, KBit2, KBit2, 0 },		{ KCM_AUTOIDLE_PER, KBit2, KBit2, 0 } },		// EClkMcBSP4_I,
		{ { KCM_ICLKEN1_CORE, KBit10, KBit10, 0 },	{ KCM_AUTOIDLE1_CORE, KBit10, KBit10, 0 } },	// EClkMcBSP5_I,
		{ { KCM_ICLKEN1_CORE, KBit15, KBit15, 0 },	{ KCM_AUTOIDLE1_CORE, KBit15, KBit15, 0 } },	// EClkI2c1_I,
		{ { KCM_ICLKEN1_CORE, KBit16, KBit16, 0 },	{ KCM_AUTOIDLE1_CORE, KBit16, KBit16, 0 } },	// EClkI2c2_I,
		{ { KCM_ICLKEN1_CORE, KBit17, KBit17, 0 },	{ KCM_AUTOIDLE1_CORE, KBit17, KBit17, 0 } },	// EClkI2c3_I,
		{ { KCM_ICLKEN1_CORE, KBit13, KBit13, 0 },	{ KCM_AUTOIDLE1_CORE, KBit13, KBit13, 0 } },	// EClkUart1_I,
		{ { KCM_ICLKEN1_CORE, KBit14, KBit14, 0 },	{ KCM_AUTOIDLE1_CORE, KBit14, KBit14, 0 } },	// EClkUart2_I,
		{ { KCM_ICLKEN_PER, KBit11, KBit11, 0 },	{ KCM_AUTOIDLE_PER, KBit11, KBit11, 0 } },		// EClkUart3_I,
		{ { KCM_ICLKEN1_CORE, KBit18, KBit18, 0 },	{ KCM_AUTOIDLE1_CORE, KBit18, KBit18, 0 } },	// EClkMcSpi1_I,
		{ { KCM_ICLKEN1_CORE, KBit19, KBit19, 0 },	{ KCM_AUTOIDLE1_CORE, KBit19, KBit19, 0 } },	// EClkMcSpi2_I,
		{ { KCM_ICLKEN1_CORE, KBit20, KBit20, 0 },	{ KCM_AUTOIDLE1_CORE, KBit20, KBit20, 0 } },	// EClkMcSpi3_I,
		{ { KCM_ICLKEN1_CORE, KBit21, KBit21, 0 },	{ KCM_AUTOIDLE1_CORE, KBit21, KBit21, 0 } },	// EClkMcSpi4_I,
		{ { KCM_ICLKEN_WKUP, KBit0, KBit0, 0 },		{ KCM_AUTOIDLE_WKUP, KBit0, KBit0, 0 } },		// EClkGpt1_I,
		{ { KCM_ICLKEN_PER, KBit3, KBit3, 0 },		{ KCM_AUTOIDLE_PER, KBit3, KBit3, 0 } },		// EClkGpt2_I,
		{ { KCM_ICLKEN_PER, KBit4, KBit4, 0 },		{ KCM_AUTOIDLE_PER, KBit4, KBit4, 0 } },		// EClkGpt3_I,
		{ { KCM_ICLKEN_PER, KBit5, KBit5, 0 },		{ KCM_AUTOIDLE_PER, KBit5, KBit5, 0 } },		// EClkGpt4_I,
		{ { KCM_ICLKEN_PER, KBit6, KBit6, 0 },		{ KCM_AUTOIDLE_PER, KBit6, KBit6, 0 } },		// EClkGpt5_I,
		{ { KCM_ICLKEN_PER, KBit7, KBit7, 0 },		{ KCM_AUTOIDLE_PER, KBit7, KBit7, 0 } },		// EClkGpt6_I,
		{ { KCM_ICLKEN_PER, KBit8, KBit8, 0 },		{ KCM_AUTOIDLE_PER, KBit8, KBit8, 0 } },		// EClkGpt7_I,
		{ { KCM_ICLKEN_PER, KBit9, KBit9, 0 },		{ KCM_AUTOIDLE_PER, KBit9, KBit9, 0 } },		// EClkGpt8_I,
		{ { KCM_ICLKEN_PER, KBit10, KBit10, 0 },	{ KCM_AUTOIDLE_PER, KBit10, KBit10, 0 } },		// EClkGpt9_I,
		{ { KCM_ICLKEN1_CORE, KBit11, KBit11, 0 },	{ KCM_AUTOIDLE1_CORE, KBit11, KBit11, 0 } },	// EClkGpt10_I,
		{ { KCM_ICLKEN1_CORE, KBit12, KBit12, 0 },	{ KCM_AUTOIDLE1_CORE, KBit12, KBit12, 0 } },	// EClkGpt11_I,
		{ { KDummy, 0, 0, 0 },						{ KDummy, 0, KDummyReadAsDisabled, 0 } },							// EClkGpt12_I,
		{ { KCM_ICLKEN1_CORE, KBit7, KBit7, 0 },	{ KCM_AUTOIDLE1_CORE, KBit7, KBit7, 0 } },		// EClkMailboxes_I,
		{ { KCM_ICLKEN1_CORE, KBit6, KBit6, 0 },	{ KCM_AUTOIDLE1_CORE, KBit6, KBit6, 0 } },		// EClkOmapSCM_I,
		{ { KCM_ICLKEN1_CORE, KBit4, KBit4, 0 },	{ KCM_AUTOIDLE1_CORE, KBit4, KBit4, 0 } },		// EClkHsUsbOtg_I,
		{ { KCM_ICLKEN1_CORE, KBit1, KBit1, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSdrc_I,
		{ { KCM_ICLKEN2_CORE, KBit4, KBit4, 0 },	{ KCM_AUTOIDLE2_CORE, KBit4, KBit4, 0 } },		// EClkPka_I,
		{ { KCM_ICLKEN2_CORE, KBit2, KBit2, 0 },	{ KCM_AUTOIDLE2_CORE, KBit2, KBit2, 0 } },		// EClkRng_I,
		{ { KCM_ICLKEN3_CORE, KBit2, KBit2, 0 },	{ KCM_AUTOIDLE3_CORE, KBit2, KBit2, 0 } },		// EClkUsbTll_I,

		{ { KCM_ICLKEN_SGX, KBit0, KBit0, 0 },		{ KCM_CLKSTCTRL_SGX, KBit0 | KBit1, 0x3, 0x0 } },	// EClkSgx_I,

		{ { KCM_ICLKEN_WKUP, KBit9, KBit9, 0 },		{ KCM_AUTOIDLE_WKUP, KBit9, KBit9, 0 } },		// EClkUsim_I,
		{ { KCM_ICLKEN_WKUP, KBit4, KBit4, 0 },		{ KCM_AUTOIDLE_WKUP, KBit4, KBit4, 0 } },		// EClkWdt1_I,
		{ { KCM_ICLKEN_WKUP, KBit5, KBit5, 0 },		{ KCM_AUTOIDLE_WKUP, KBit5, KBit5, 0 } },		// EClkWdt2_I,
		{ { KCM_ICLKEN_PER, KBit12, KBit12, 0 },	{ KCM_AUTOIDLE_PER, KBit12, KBit12, 0 } },		// EClkWdt3_I,
		{ { KCM_ICLKEN_WKUP, KBit3, KBit3, 0 },		{ KCM_AUTOIDLE_WKUP, KBit3, KBit3, 0 } },		// EClkGpio1_I,
		{ { KCM_ICLKEN_PER, KBit13, KBit13, 0 },	{ KCM_AUTOIDLE_PER, KBit13, KBit13, 0 } },		// EClkGpio2_I,
		{ { KCM_ICLKEN_PER, KBit14, KBit14, 0 },	{ KCM_AUTOIDLE_PER, KBit14, KBit14, 0 } },		// EClkGpio3_I,
		{ { KCM_ICLKEN_PER, KBit15, KBit15, 0 },	{ KCM_AUTOIDLE_PER, KBit15, KBit15, 0 } },		// EClkGpio4_I,
		{ { KCM_ICLKEN_PER, KBit16, KBit16, 0 },	{ KCM_AUTOIDLE_PER, KBit16, KBit16, 0 } },		// EClkGpio5_I,
		{ { KCM_ICLKEN_PER, KBit17, KBit17, 0 },	{ KCM_AUTOIDLE_PER, KBit17, KBit17, 0 } },		// EClkGpio6_I,
		{ { KCM_ICLKEN_WKUP, KBit2, KBit2, 0 },		{ KCM_AUTOIDLE_WKUP, KBit2, KBit2, 0 } },		// EClk32Sync_I,

		{ { KCM_ICLKEN_USBHOST, KBit0, KBit0, 0 }, { KCM_AUTOIDLE_USBHOST, KBit0, KBit0, 0 } },		// EClkUsb_I,			///< USB host interface clock

		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClk48M
		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClk12M

		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClkSysClk
		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClkAltClk
		{ { KDummy, 0, 0, 0 },							{ KDummy, 0, KDummyReadAsEnabled, 0 } },		// EClkSysClk32k
	};
__ASSERT_COMPILE( (sizeof(KClockControlTable) / sizeof( KClockControlTable[0] )) == Prcm::KSupportedClockCount );

static const TRegisterBitDef KClockWakeupTable[] =
	{
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMpu,		///< DPLL1
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkIva2Pll,	///< DPLL2
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkCore,		///< DPLL3
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkPeriph,		///< DPLL4
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkPeriph2,	///< DPLL5

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkPrcmInterface,

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkEmu,		///< Emulation clock
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkNeon,

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkL3Domain,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkL4Domain,

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMpuPll_Bypass,	///< DPLL1 bypass frequency
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkIva2Pll_Bypass,	///< DPLL2 bypass frequency
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkRM_F,			///< Reset manager functional clock
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClk96M,			///< 96MHz clock
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClk120M,			///< 120MHz clock
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkSysOut,

	// Functional clocks
	// NOTE - functional clocks aren't mapped to a wakeup event, these just clock the internals
	// Use the interface clocks to register a wakeup
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkTv_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkDss1_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkDss2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkCsi2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkCam_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkIva2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMmc1_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMmc2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMmc3_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMsPro_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkHdq_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMcBSP1_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMcBSP2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMcBSP3_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMcBSP4_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMcBSP5_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMcSpi1_F
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMcSpi2_F
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMcSpi3_F
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMcSpi4_F
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkI2c1_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkI2c2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkI2c3_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkUart1_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkUart2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkUart3_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt1_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt3_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt4_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt5_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt6_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt7_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt8_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt9_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt10_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpt11_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkUsbTll_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkTs_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkCpeFuse_F,

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkSgx_F,

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkUsim_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkSmartReflex2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkSmartReflex1_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkWdt2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkWdt3_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpio1_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpio2_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpio3_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpio4_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpio5_F,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkGpio6_F,

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkUsb120_F,		///< USB host 120MHz functional clock
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkUsb48_F,		///< USB host 48MHz functional clock


	// Interface clocks
		{ KPM_WKEN_DSS, KBit0, KBit0, 0 },	// EClkDss_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkCam_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkIcr_I,
		{ KPM_WKEN1_CORE, KBit24, KBit24, 0 },	// EClkMmc1_I,
		{ KPM_WKEN1_CORE, KBit25, KBit25, 0 },	// EClkMmc2_I,
		{ KPM_WKEN1_CORE, KBit30, KBit30, 0 },	// EClkMmc3_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMsPro_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkHdq_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkAes1_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkAes2_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkSha11_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkSha12_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkDes1_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkDes2_I,
		{ KPM_WKEN1_CORE, KBit9, KBit9, 0 },	// EClkMcBSP1_I,
		{ KPM_WKEN_PER, KBit0, KBit0, 0 },	// EClkMcBSP2_I,
		{ KPM_WKEN_PER, KBit1, KBit1, 0 },	// EClkMcBSP3_I,
		{ KPM_WKEN_PER, KBit2, KBit2, 0 },	// EClkMcBSP4_I,
		{ KPM_WKEN1_CORE, KBit10, KBit10, 0 },	// EClkMcBSP5_I,
		{ KPM_WKEN1_CORE, KBit15, KBit15, 0 },	// EClkI2c1_I,
		{ KPM_WKEN1_CORE, KBit16, KBit16, 0 },	// EClkI2c2_I,
		{ KPM_WKEN1_CORE, KBit17, KBit17, 0 },	// EClkI2c3_I,
		{ KPM_WKEN1_CORE, KBit13, KBit13, 0 },	// EClkUart1_I,
		{ KPM_WKEN1_CORE, KBit14, KBit14, 0 },	// EClkUart2_I,
		{ KPM_WKEN_PER, KBit11, KBit11, 0 },	// EClkUart3_I,
		{ KPM_WKEN1_CORE, KBit18, KBit18, 0 },	// EClkMcSpi1_I
		{ KPM_WKEN1_CORE, KBit19, KBit19, 0 },	// EClkMcSpi2_I
		{ KPM_WKEN1_CORE, KBit20, KBit20, 0 },	// EClkMcSpi3_I
		{ KPM_WKEN1_CORE, KBit21, KBit21, 0 },	// EClkMcSpi4_I
		{ KPM_WKEN_WKUP, KBit0, KBit0, 0 },	// EClkGpt1_I,
		{ KPM_WKEN_PER, KBit3, KBit3, 0 },	// EClkGpt2_I,
		{ KPM_WKEN_PER, KBit4, KBit4, 0 },	// EClkGpt3_I,
		{ KPM_WKEN_PER, KBit5, KBit5, 0 },	// EClkGpt4_I,
		{ KPM_WKEN_PER, KBit6, KBit6, 0 },	// EClkGpt5_I,
		{ KPM_WKEN_PER, KBit7, KBit7, 0 },	// EClkGpt6_I,
		{ KPM_WKEN_PER, KBit8, KBit8, 0 },	// EClkGpt7_I,
		{ KPM_WKEN_PER, KBit9, KBit9, 0 },	// EClkGpt8_I,
		{ KPM_WKEN_PER, KBit10, KBit10, 0 },	// EClkGpt9_I,
		{ KPM_WKEN1_CORE, KBit11, KBit11, 0 },	// EClkGpt10_I,
		{ KPM_WKEN1_CORE, KBit12, KBit12, 0 },	// EClkGpt11_I,
		{ KPM_WKEN_WKUP, KBit1, KBit1, 0 },	// EClkGpt12_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkMailboxes_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkOmapSCM_I,
		{ KPM_WKEN1_CORE, KBit4, KBit4, 0 },	// EClkHsUsbOtg_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkSdrc_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkPka_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkRng_I,
		{ KPM_WKEN3_CORE, KBit2, KBit2, 0 },	// EClkUsbTll_I,

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkSgx_I,

		{ KPM_WKEN_WKUP, KBit9, KBit9, 0 },	// EClkUsim_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkWdt1_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkWdt2_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkWdt3_I,
		{ KPM_WKEN_WKUP, KBit3, KBit3, 0 },	// EClkGpio1_I,
		{ KPM_WKEN_PER, KBit13, KBit13, 0 },	// EClkGpio2_I,
		{ KPM_WKEN_PER, KBit14, KBit14, 0 },	// EClkGpio3_I,
		{ KPM_WKEN_PER, KBit15, KBit15, 0 },	// EClkGpio4_I,
		{ KPM_WKEN_PER, KBit16, KBit16, 0 },	// EClkGpio5_I,
		{ KPM_WKEN_PER, KBit17, KBit17, 0 },	// EClkGpio6_I,
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClk32Sync_I,

		{ KPM_WKEN_USBHOST, KBit0, KBit0, 0 },	// EClkUsb_I,			///< USB host interface clock

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClk48M
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClk12M

		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkSysClk
		{ KDummy, 0, KDummyReadAsDisabled, 0 },	// EClkAltClk
		{ KDummy, 0, KDummyReadAsDisabled, 0 }	// EClkSysClk32k

	};
__ASSERT_COMPILE( (sizeof(KClockWakeupTable) / sizeof( KClockWakeupTable[0] )) == Prcm::KSupportedClockCount );


__ASSERT_COMPILE( Prcm::EWakeGroupMpu == 0 );
__ASSERT_COMPILE( Prcm::EWakeGroupIva2 == 1 );
static const TRegisterBitDef KClockWakeupGroupTable[ Prcm::KSupportedClockCount ][ Prcm::KSupportedWakeupGroupCount ] =
	{
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMpu,		///< DPLL1
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkIva2Pll,	///< DPLL2
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkCore,		///< DPLL3
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkPeriph,		///< DPLL4
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkPeriph2,	///< DPLL5

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkPrcmInterface,

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkEmu,		///< Emulation clock
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkNeon,

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkL3Domain,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkL4Domain,

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMpuPll_Bypass,	///< DPLL1 bypass frequency
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkIva2Pll_Bypass,	///< DPLL2 bypass frequency
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkRM_F,			///< Reset manager functional clock
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClk96M,			///< 96MHz clock
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClk120M,			///< 120MHz clock
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSysOut,

	// Functional clocks
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkTv_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkDss1_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkDss2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkCsi2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkCam_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkIva2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMmc1_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMmc2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMmc3_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMsPro_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkHdq_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMcBsp1_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMcBsp2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMcBsp3_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMcBsp4_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMcBsp5_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMcSpi1_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMcSpi2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMcSpi3_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMcSpi4_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkI2c1_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkI2c2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkI2c3_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkUart1_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkUart2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkUart3_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt1_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt3_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt4_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt5_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt6_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt7_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt8_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt9_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt10_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpt11_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkUsbTll_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkTs_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkCpeFuse_F,

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSgx_F,

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkUsim_F,
		{ { KPM_MPUGRPSEL_WKUP, KBit7, KBit7, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSmartReflex2_F,
		{ { KPM_MPUGRPSEL_WKUP, KBit6, KBit6, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSmartReflex1_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkWdt2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkWdt3_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpio1_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpio2_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpio3_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpio4_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpio5_F,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkGpio6_F,

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkUsb120_F,		///< USB host 120MHz functional clock
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkUsb48_F,		///< USB host 48MHz functional clock


	// Interface clocks
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkDss_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkCam_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkIcr_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit24, KBit24, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit24, KBit24, 0 } },			// EClkMmc1_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit25, KBit25, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit25, KBit25, 0 } },			// EClkMmc2_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit30, KBit30, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit30, KBit30, 0 } },			// EClkMmc3_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMsPro_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkHdq_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkAes1_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkAes2_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSha11_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSha12_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkDes1_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkDes2_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit9, KBit9, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit9, KBit9, 0 } },			// EClkMcBsp1_I,
		{ { KPM_MPUGRPSEL_PER, KBit0, KBit0, 0 },	{ KPM_IVA2GRPSEL_PER, KBit0, KBit0, 0 } },			// EClkMcBsp2_I,
		{ { KPM_MPUGRPSEL_PER, KBit1, KBit1, 0 },	{ KPM_IVA2GRPSEL_PER, KBit1, KBit1, 0 } },			// EClkMcBsp3_I,
		{ { KPM_MPUGRPSEL_PER, KBit2, KBit2, 0 },	{ KPM_IVA2GRPSEL_PER, KBit2, KBit2, 0 } },			// EClkMcBsp4_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit10, KBit10, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit10, KBit10, 0 } },			// EClkMcBsp5_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit15, KBit15, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit15, KBit15, 0 } },			// EClkI2c1_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit16, KBit16, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit16, KBit16, 0 } },			// EClkI2c2_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit17, KBit17, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit17, KBit17, 0 } },			// EClkI2c3_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit13, KBit13, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit13, KBit13, 0 } },			// EClkUart1_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit14, KBit14, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit14, KBit14, 0 } },			// EClkUart2_I,
		{ { KPM_MPUGRPSEL_PER, KBit11, KBit11, 0 },	{ KPM_IVA2GRPSEL_PER, KBit11, KBit11, 0 } },			// EClkUart3_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit18, KBit18, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit18, KBit18, 0 } },			// EClkMcSpi1_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit19, KBit19, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit19, KBit19, 0 } },			// EClkMcSpi2_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit20, KBit20, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit20, KBit20, 0 } },			// EClkMcSpi3_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit21, KBit21, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit21, KBit21, 0 } },			// EClkMcSpi4_I,
		{ { KPM_MPUGRPSEL_WKUP, KBit0, KBit0, 0 },	{ KPM_IVA2GRPSEL_WKUP, KBit0, KBit0, 0 } },			// EClkGpt1_I,
		{ { KPM_MPUGRPSEL_PER, KBit3, KBit3, 0 },	{ KPM_IVA2GRPSEL_PER, KBit3, KBit3, 0 } },			// EClkGpt2_I,
		{ { KPM_MPUGRPSEL_PER, KBit4, KBit4, 0 },	{ KPM_IVA2GRPSEL_PER, KBit4, KBit4, 0 } },			// EClkGpt3_I,
		{ { KPM_MPUGRPSEL_PER, KBit5, KBit5, 0 },	{ KPM_IVA2GRPSEL_PER, KBit5, KBit5, 0 } },			// EClkGpt4_I,
		{ { KPM_MPUGRPSEL_PER, KBit6, KBit6, 0 },	{ KPM_IVA2GRPSEL_PER, KBit6, KBit6, 0 } },			// EClkGpt5_I,
		{ { KPM_MPUGRPSEL_PER, KBit7, KBit7, 0 },	{ KPM_IVA2GRPSEL_PER, KBit7, KBit7, 0 } },			// EClkGpt6_I,
		{ { KPM_MPUGRPSEL_PER, KBit8, KBit9, 0 },	{ KPM_IVA2GRPSEL_PER, KBit8, KBit8, 0 } },			// EClkGpt7_I,
		{ { KPM_MPUGRPSEL_PER, KBit9, KBit9, 0 },	{ KPM_IVA2GRPSEL_PER, KBit9, KBit9, 0 } },			// EClkGpt8_I,
		{ { KPM_MPUGRPSEL_PER, KBit10, KBit10, 0 },	{ KPM_IVA2GRPSEL_PER, KBit10, KBit10, 0 } },			// EClkGpt9_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit11, KBit11, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit11, KBit11, 0 } },			// EClkGpt10_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit12, KBit12, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit12, KBit12, 0 } },			// EClkGpt11_I,
		{ { KPM_MPUGRPSEL_WKUP, KBit1, KBit1, 0 },	{ KPM_IVA2GRPSEL_WKUP, KBit1, KBit1, 0 } },			// EClkGpt12_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkMailboxes_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkOmapSCM_I,
		{ { KPM_MPUGRPSEL1_CORE, KBit4, KBit4, 0 },	{ KPM_IVA2GRPSEL1_CORE, KBit4, KBit4, 0 } },			// EClkHsUsbOtg_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSdrc_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkPka_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkRng_I,
		{ { KPM_MPUGRPSEL3_CORE, KBit2, KBit2, 0 },	{ KPM_IVA2GRPSEL3_CORE, KBit2, KBit2, 0 } },			// EClkUsbTll_I,

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSgx_I,

		{ { KPM_MPUGRPSEL_WKUP, KBit9, KBit9, 0 },	{ KPM_IVA2GRPSEL_WKUP, KBit9, KBit9, 0 } },			// EClkUsim_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkWdt1_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkWdt2_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkWdt3_I,
		{ { KPM_MPUGRPSEL_WKUP, KBit3, KBit3, 0 },	{ KPM_IVA2GRPSEL_WKUP, KBit3, KBit3, 0 } },			// EClkGpio1_I,
		{ { KPM_MPUGRPSEL_PER, KBit13, KBit13, 0 },	{ KPM_IVA2GRPSEL_PER, KBit13, KBit13, 0 } },			// EClkGpio2_I,
		{ { KPM_MPUGRPSEL_PER, KBit14, KBit14, 0 },	{ KPM_IVA2GRPSEL_PER, KBit14, KBit14, 0 } },			// EClkGpio3_I,
		{ { KPM_MPUGRPSEL_PER, KBit15, KBit15, 0 },	{ KPM_IVA2GRPSEL_PER, KBit15, KBit15, 0 } },			// EClkGpio4_I,
		{ { KPM_MPUGRPSEL_PER, KBit16, KBit16, 0 },	{ KPM_IVA2GRPSEL_PER, KBit16, KBit16, 0 } },			// EClkGpio5_I,
		{ { KPM_MPUGRPSEL_PER, KBit17, KBit17, 0 },	{ KPM_IVA2GRPSEL_PER, KBit17, KBit17, 0 } },			// EClkGpio6_I,
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClk32Sync_I,

		{ { KPM_MPUGRPSEL_USBHOST, KBit0, KBit0, 0 },	{ KPM_IVA2GRPSEL_USBHOST, KBit0, KBit0, 0 } },			// EClkUsb_I,			///< USB host interface clock

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClk48M
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClk12M

		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkSysClk
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } },			// EClkAltClk
		{ { KDummy, 0, KDummyReadAsDisabled, 0 },	{ KDummy, 0, KDummyReadAsDisabled, 0 } }			// EClkSysClk32k
	};

	__ASSERT_COMPILE( Prcm::EWakeDomainMpu == 0 );
	__ASSERT_COMPILE( Prcm::EWakeDomainCore == 1 );
	__ASSERT_COMPILE( Prcm::EWakeDomainIva2 == 2 );
	__ASSERT_COMPILE( Prcm::EWakeDomainPeripheral == 3 );
	__ASSERT_COMPILE( Prcm::EWakeDomainDss == 4 );
	__ASSERT_COMPILE( Prcm::EWakeDomainWakeup == 5 );
	__ASSERT_COMPILE( Prcm::KSupportedWakeupDomainCount == 6 );

struct TWakeupDomainInfo
	{
	// To save space, there's an assumption here that all domain dependency configuration for
	// a single clock is in one register, and a single bit defines the dependency,
	// 1 = dependant, 0 = independant
	// The bits are defined here by bit number rather than by mask
	TUint32		iRegister;
	TInt8		iBitNumber[ Prcm::KSupportedWakeupDomainCount ];	///< bit number to modify, -1 if not supported
	};

static const TWakeupDomainInfo KClockWakeupDomainTable[ Prcm::KSupportedClockCount ] =
	{
		// REGISTER			MPU		CORE	IVA2	PER		DSS		WAKE
		{ KPM_WKDEP_MPU,	{-1,		0,		2,		7,		5,		-1 } },		// EClkMpu,		///< DPLL1
		{ KPM_WKDEP_IVA2,	{1,			0,		-1,		7,		5,		4 } },		// EClkIva2Pll,	///< DPLL2
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkCore,		///< DPLL3
		{ KPM_WKDEP_PER,	{1,			0,		2,		-1,		-1,		4 } },		// EClkPeriph,		///< DPLL4
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkPeriph2,	///< DPLL5

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkPrcmInterface,

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkEmu,		///< Emulation clock
		{ KPM_WKDEP_NEON,	{1,			-1,		-1,		-1,		-1,		-1 } },		// EClkNeon,

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkL3Domain,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkL4Domain,

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMpuPll_Bypass,	///< DPLL1 bypass frequency
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkIva2Pll_Bypass,	///< DPLL2 bypass frequency
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkRM_F,			///< Reset manager functional clock
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClk96M,			///< 96MHz clock
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClk120M,			///< 120MHz clock
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkSysOut,

	// Functional clocks
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkTv_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkDss1_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkDss2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkCsi2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkCam_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkIva2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMmc1_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMmc2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMmc3_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMsPro_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkHdq_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp1_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp3_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp4_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp5_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcSpi1_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcSpi2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcSpi3_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcSpi4_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkI2c1_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkI2c2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkI2c3_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUart1_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUart2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUart3_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt1_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt3_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt4_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt5_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt6_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt7_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt8_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt9_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt10_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt11_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUsbTll_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkTs_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkCpeFuse_F,

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkSgx_F,

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUsim_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkSmartReflex2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkSmartReflex1_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkWdt2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkWdt3_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio1_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio2_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio3_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio4_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio5_F,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio6_F,

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUsb120_F,		///< USB host 120MHz functional clock
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUsb48_F,		///< USB host 48MHz functional clock


	// Interface clocks
		{ KPM_WKDEP_DSS,	{1,		-1,		2,		-1,		-1,		4 } },		// EClkDss_I,
		{ KPM_WKDEP_CAM,	{1,		-1,		2,		-1,		-1,		4 } },		// EClkCam_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkIcr_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMmc1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMmc2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMmc3_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMsPro_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkHdq_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkAes1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkAes2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkSha11_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkSha12_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkDes1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkDes2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp3_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp4_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcBsp5_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkI2c1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkI2c2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkI2c3_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUart1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUart2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUart3_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcSpi1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcSpi2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcSpi3_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMcSpi4_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt3_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt4_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt5_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt6_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt7_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt8_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt9_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt10_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt11_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpt12_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkMailboxes_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkOmapSCM_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkHsUsbOtg_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkSdrc_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkPka_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkRng_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUsbTll_I,

		{ KPM_WKDEP_SGX,	{1,		-1,		2,		-1,		-1,		4 } },		// EClkSgx_I,

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkUsim_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkWdt1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkWdt2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkWdt3_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio1_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio2_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio3_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio4_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio5_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkGpio6_I,
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClk32Sync_I,

		{ KPM_WKDEP_USBHOST,	{1,	0,		2,		-1,		-1,		4	} },		// EClkUsb_I,			///< USB host interface clock

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClk48M
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClk12M

		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkSysClk
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkAltClk
		{ KDummy,		{-1,		-1,		-1,		-1,		-1,		-1 } },		// EClkSysClk32k
		// REGISTER			MPU		CORE	IVA2	PER		DSS		WAKE
	};

struct TPowerDomainControl
	{
	TUint32		iRegister;
	TUint8		iShift;			///< shift to move bits into position
	TUint8		iAllowedMask;	///< mask of which modes are supported
	TUint8		__spare[2];
	};

const TUint8	KPowerAllowedOff		= 1 << Prcm::EPowerOff;
const TUint8	KPowerAllowedOn			= 1 << Prcm::EPowerOn;
const TUint8	KPowerAllowedRetention	= 1 << Prcm::EPowerRetention;
const TUint8	KPowerAllowedOnOffRetention	=	(KPowerAllowedOff bitor KPowerAllowedOn bitor KPowerAllowedRetention);
const TUint8	KPowerModeMask			= 0x3;

static const TPowerDomainControl KPowerDomainControl[] =
	{
		// iRegister			iShift	iAllowedMask
		{ KPM_PWSTCTRL_MPU,		0,		KPowerAllowedOnOffRetention	},	// EPowerDomainMpu,
		{ KPM_PWSTCTRL_IVA2,	0,		KPowerAllowedOnOffRetention	},	// EPowerDomainIva2,
		{ KPM_PWSTCTRL_NEON,	0,		KPowerAllowedOnOffRetention	},	// EPowerDomainNeon,
		{ KPM_PWSTCTRL_CORE,	0,		KPowerAllowedOnOffRetention },	// EPowerDomainCore,
		{ KPM_PWSTCTRL_SGX,		0,		KPowerAllowedOnOffRetention },	// EPowerDomainSgx,
		{ KPM_PWSTCTRL_DSS,		0,		KPowerAllowedOnOffRetention	},	// EPowerDomainDss,
		{ KPM_PWSTCTRL_CAM,		0,		KPowerAllowedOnOffRetention	},	// EPowerDomainCamera,
		{ KPM_PWSTCTRL_USBHOST,	0,		KPowerAllowedOnOffRetention	},	// EPowerDomainUsb,
		{ KPM_PWSTCTRL_PER,		0,		KPowerAllowedOnOffRetention	}	// EPowerDomainPer,
	};
__ASSERT_COMPILE( (sizeof(KPowerDomainControl) / sizeof( KPowerDomainControl[0] )) == Prcm::KSupportedPowerDomainCount );

struct TGptClkSelInfo
	{
	TUint32	iRegister;
	TUint32	iMask;
	};

static const TGptClkSelInfo KGptClockSourceInfo[ Prcm::KSupportedGptCount ] =
	{
		{ KCM_CLKSEL_WKUP, KBit0 },	//	EGpt1,
		{ KCM_CLKSEL_PER, KBit0 },	//	EGpt2,
		{ KCM_CLKSEL_PER, KBit1 },	//	EGpt3,
		{ KCM_CLKSEL_PER, KBit2 },	//	EGpt4,
		{ KCM_CLKSEL_PER, KBit3 },	//	EGpt5,
		{ KCM_CLKSEL_PER, KBit4 },	//	EGpt6,
		{ KCM_CLKSEL_PER, KBit5 },	//	EGpt7,
		{ KCM_CLKSEL_PER, KBit6 },	//	EGpt8,
		{ KCM_CLKSEL_PER, KBit7 },	//	EGpt9,
		{ KCM_CLKSEL_CORE, KBit6 },	//	EGpt10,
		{ KCM_CLKSEL_CORE, KBit7 },	//	EGpt11,
		{ KDummy, 0 },			//	EGpt12	- clocked from security block
	};

// This table is used to find the source clock for a given clock. That is, by looking up a
// specific clock in this table, you can find out which DPLL/divider it was derived from.
// Following the chain backwards to SYSCLK allows building of the total multiply and
// divide applied to SYSCLK to get the given clock
enum TClockSourceType
	{
	EIgnore,	// not implemented yet...
	EDpll,		// this clock is derived from a PLL
	EDivider,	// this clock is divied from a given clock
	EDivMux,	// divider fed by mux-selectable input clock
	EMux,		// fed by mux-selectable input clock
	EDuplicate,	// this clock is a duplicate of another clock
	E96MMux,	// 96MHz mux-selected clock source
	E54MMux,	// 54MHz mux-selected clock source
	E48MMux,	// 48MHz mux-selected clock source
	EDiv4,		// specified clock source divided by 4
	};

struct TClockSourceInfo
	{
	TClockSourceType	iType : 8;	// type of the source for this clock
	union	{
		Prcm::TClock	iClock : 8;		// the clock that feeds this divider, or which this is a duplicate of
		Prcm::TPll		iPll : 8;		// the PLL that generates this clock
		Prcm::TGpt		iGpt : 8;		// conversion to TGpt type for the clock we are interested in
		};
	};

static const TClockSourceInfo KClockSourceInfo[] =
	{
		{ EDpll,		(Prcm::TClock)Prcm::EDpll1 },			// EClkMpu,
		{ EDpll,		(Prcm::TClock)Prcm::EDpll2 },			// EClkIva2Pll,
		{ EDpll,		(Prcm::TClock)Prcm::EDpll3 },			// EClkCore,
		{ EDpll,		(Prcm::TClock)Prcm::EDpll4 },			// EClkPeriph,
		{ EDpll,		(Prcm::TClock)Prcm::EDpll5 },			// EClkPeriph2,
		{ EDuplicate,	Prcm::EClkSysClk },		// EClkPrcmInterface,
		{ EIgnore,		(Prcm::TClock)0 },		// EClkEmu,
		{ EDuplicate,	Prcm::EClkMpu },		// EClkNeon,
		{ EDivider,		Prcm::EClkCore },		// EClkL3Domain,
		{ EDivider,		Prcm::EClkL3Domain },	// EClkL4Domain,
		{ EDivider,		Prcm::EClkCore },		// EClkMpuPll_Bypass,
		{ EDivider,		Prcm::EClkCore },		// EClkIva2Pll_Bypass,
		{ EDivider,		Prcm::EClkL4Domain },	// EClkRM_F,
		{ E96MMux,		Prcm::EClkPeriph },		// EClk96M,
		{ EDivider,		Prcm::EClkPeriph2 },	// EClk120M,
		{ EDivMux,		(Prcm::TClock)0 },		// EClkSysOut,

	// Functional clocks
		{ E54MMux,		Prcm::EClkPeriph },
		{ EDivider,		Prcm::EClkPeriph },		// EClkDss1_F,
		{ EDuplicate,	Prcm::EClkSysClk },		// EClkDss2_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkCsi2_F,
		{ EDivider,		Prcm::EClkPeriph },		// EClkCam_F,
		{ EDuplicate,	Prcm::EClkIva2Pll },	// EClkIva2_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkMmc1_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkMmc2_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkMmc3_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkMsPro_F,
		{ EDuplicate,	Prcm::EClk12M },		// EClkHdq_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkMcBsp1_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkMcBsp2_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkMcBsp3_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkMcBsp4_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkMcBsp5_F,
		{ EDuplicate,	Prcm::EClk48M },		// EClkMcSpi1_F,
		{ EDuplicate,	Prcm::EClk48M },		// EClkMcSpi2_F,
		{ EDuplicate,	Prcm::EClk48M },		// EClkMcSpi3_F,
		{ EDuplicate,	Prcm::EClk48M },		// EClkMcSpi4_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkI2c1_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkI2c2_F,
		{ EDuplicate,	Prcm::EClk96M },		// EClkI2c3_F,
		{ EDuplicate,	Prcm::EClk48M },		// EClkUart1_F,
		{ EDuplicate,	Prcm::EClk48M },		// EClkUart2_F,
		{ EDuplicate,	Prcm::EClk48M },		// EClkUart3_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt1 },			// EClkGpt1_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt2 },			// EClkGpt2_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt3 },			// EClkGpt3_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt4 },			// EClkGpt4_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt5 },			// EClkGpt5_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt6 },			// EClkGpt6_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt7 },			// EClkGpt7_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt8 },			// EClkGpt8_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt9 },			// EClkGpt9_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt10 },			// EClkGpt10_F,
		{ EMux,			(Prcm::TClock)Prcm::EGpt11 },			// EClkGpt11_F,
		{ EDuplicate,	Prcm::EClk120M },		// EClkUsbTll_F,
		{ EDuplicate,	Prcm::EClkSysClk32k },	// EClkTs_F,
		{ EDuplicate,	Prcm::EClkSysClk },		// EClkCpeFuse_F,
		{ EDivMux,		(Prcm::TClock)0 },					// EClkSgx_F,
		{ EDivMux,		Prcm::EClkSysClk },		// EClkUsim_F,
		{ EDuplicate,	Prcm::EClkSysClk },		// EClkSmartReflex2_F,
		{ EDuplicate,	Prcm::EClkSysClk },		// EClkSmartReflex1_F,
		{ EDuplicate,	Prcm::EClkSysClk32k },					// EClkWdt2_F,
		{ EDuplicate,	Prcm::EClkSysClk32k },					// EClkWdt3_F,
		{ EDuplicate,	Prcm::EClkSysClk32k },					// EClkGpio1_F,
		{ EDuplicate,	Prcm::EClkSysClk32k },					// EClkGpio2_F,
		{ EDuplicate,	Prcm::EClkSysClk32k },					// EClkGpio3_F,
		{ EDuplicate,	Prcm::EClkSysClk32k },					// EClkGpio4_F,
		{ EDuplicate,	Prcm::EClkSysClk32k },					// EClkGpio5_F,
		{ EDuplicate,	Prcm::EClkSysClk32k },					// EClkGpio6_F,
		{ EDuplicate,	Prcm::EClk120M },		// EClkUsb120_F,
		{ EDuplicate,	Prcm::EClk48M },		// EClkUsb48_F,

	// Interface clocks
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkDss_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkCam_I,
		{ },					// EClkIcr_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMmc1_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMmc2_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMmc3_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMsPro_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkHdq_I,
		{ EDuplicate,	Prcm::EClkL4Domain},		// EClkAes1_I,
		{ EDuplicate,	Prcm::EClkL4Domain},		// EClkAes2_I,
		{ EDuplicate,	Prcm::EClkL4Domain},		// EClkSha11_I,
		{ EDuplicate,	Prcm::EClkL4Domain},		// EClkSha12_I,
		{ EDuplicate,	Prcm::EClkL4Domain},		// EClkDes1_I,
		{ EDuplicate,	Prcm::EClkL4Domain},		// EClkDes2_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMcBsp1_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMcBsp2_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMcBsp3_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMcBsp4_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMcBsp5_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkI2c1_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkI2c2_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkI2c3_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkUart1_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkUart2_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkUart3_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMcSpi1_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMcSpi2_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMcSpi3_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMcSpi4_I,
		{ EDuplicate,	Prcm::EClkSysClk },			// EClkGpt1_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt2_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt3_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt4_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt5_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt6_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt7_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt8_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt9_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt10_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt11_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpt12_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkMailboxes_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkOmapSCM_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkHsUsbOtg_I,
		{ EDuplicate,	Prcm::EClkL3Domain },		// EClkSdrc_I,
		{ EDuplicate,	Prcm::EClkL3Domain },		// EClkPka_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkRng_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkUsbTll_I,
		{ EDuplicate,	Prcm::EClkL3Domain },		// EClkSgx_I,
		{ EDuplicate,	Prcm::EClkSysClk },			// EClkUsim_I,
		{ EDuplicate,	Prcm::EClkSysClk },			// EClkWdt1_I,
		{ EDuplicate,	Prcm::EClkSysClk },			// EClkWdt2_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkWdt3_I,
		{ EDuplicate,	Prcm::EClkSysClk },			// EClkGpio1_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpio2_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpio3_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpio4_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpio5_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkGpio6_I,
		{ EDuplicate,	Prcm::EClkSysClk },			// EClk32Sync_I,
		{ EDuplicate,	Prcm::EClkL4Domain },		// EClkUsb_I,

		{ E48MMux,		Prcm::EClk96M },		// EClk48M,
		{ EDiv4,		Prcm::EClk48M },		// EClk12M,

		{ EDuplicate,	Prcm::EClkSysClk },		// EClkSysClk
		{ EDuplicate,	Prcm::EClkAltClk },		// EClkAltClk
		{ EDuplicate,	Prcm::EClkSysClk32k },	// EClkSysClk32k

	};

__ASSERT_COMPILE( sizeof( KClockSourceInfo ) / sizeof( KClockSourceInfo[0] ) == Prcm::KSupportedClockCount );


// Bit of hackery to enable creation of a const table of pointer to _LITs.
// Taking the address of a _LIT will cause the compiler to invoke its operator&()
// function, which forces the compiler to generate the table in code. But hiding
// it inside a dummy struct allows taking of the address of the struct instead,
// avoiding the operator&() problem.

template< TInt S >
struct THiddenLit8
	{
	TLitC8<S>	iLit;
	};

#define __PLIT8(name,s) const static THiddenLit8<sizeof(s)> name={{sizeof(s)-1,s}};

// List of identifer strings for each clock source - used for PRM
__PLIT8(KClkMpu,			"a.MPU" );
__PLIT8(KClkIva2Pll,		"a.IVA" );
__PLIT8(KClkCore,			"a.CORE" );
__PLIT8(KClkPeriph,			"a.PER" );
__PLIT8(KClkPeriph2,		"a.PER2" );
__PLIT8(KClkPrcmInterface,	"a.PRCM" );
__PLIT8(KClkEmu,			"a.EMU" );
__PLIT8(KClkNeon,			"a.NEON" );
__PLIT8(KClkL3Domain,		"a.L3" );
__PLIT8(KClkL4Domain,		"a.L4" );
__PLIT8(KClkMpuPll_Bypass,	"a.MPUB" );
__PLIT8(KClkIva2Pll_Bypass,	"a.IVAB" );
__PLIT8(KClkRM_F,			"a.RMf" );
__PLIT8(KClk96M,			"a.96" );
__PLIT8(KClk120M,			"a.120" );
__PLIT8(KClkSysOut,			"a.OUT" );
__PLIT8(KClkTv_F,			"a.TVf" );
__PLIT8(KClkDss1_F,			"a.DSS1f" );
__PLIT8(KClkDss2_F,			"a.DSS2f" );
__PLIT8(KClkCsi2_F,			"a.CSI2f" );
__PLIT8(KClkCam_F,			"a.CAMf" );
__PLIT8(KClkIva2_F,			"a.IVA2f" );
__PLIT8(KClkMmc1_F,			"a.MMC1f" );
__PLIT8(KClkMmc2_F,			"a.MMC2f" );
__PLIT8(KClkMmc3_F,			"a.MMC3f" );
__PLIT8(KClkMsPro_F,		"a.MSPf" );
__PLIT8(KClkHdq_F,			"a.HDQf" );
__PLIT8(KClkMcBsp1_F,		"a.BSP1f" );
__PLIT8(KClkMcBsp2_F,		"a.BSP2f" );
__PLIT8(KClkMcBsp3_F,		"a.BSP3f" );
__PLIT8(KClkMcBsp4_F,		"a.BSP4f" );
__PLIT8(KClkMcBsp5_F,		"a.BSP5f" );
__PLIT8(KClkMcSpi1_F,		"a.SPI1f" );
__PLIT8(KClkMcSpi2_F,		"a.SPI2f" );
__PLIT8(KClkMcSpi3_F,		"a.SPI3f" );
__PLIT8(KClkMcSpi4_F,		"a.SPI4f" );
__PLIT8(KClkI2c1_F,			"a.I2C1f" );
__PLIT8(KClkI2c2_F,			"a.I2C2f" );
__PLIT8(KClkI2c3_F,			"a.I2C3f" );
__PLIT8(KClkUart1_F,		"a.UART1f" );
__PLIT8(KClkUart2_F,		"a.UART2f" );
__PLIT8(KClkUart3_F,		"a.UART3f" );
__PLIT8(KClkGpt1_F,			"a.GPT1f" );
__PLIT8(KClkGpt2_F,			"a.GPT2f" );
__PLIT8(KClkGpt3_F,			"a.GPT3f" );
__PLIT8(KClkGpt4_F,			"a.GPT4f" );
__PLIT8(KClkGpt5_F,			"a.GPT5f" );
__PLIT8(KClkGpt6_F,			"a.GPT6f" );
__PLIT8(KClkGpt7_F,			"a.GPT7f" );
__PLIT8(KClkGpt8_F,			"a.GPT8f" );
__PLIT8(KClkGpt9_F,			"a.GPT9f" );
__PLIT8(KClkGpt10_F,		"a.GPTAf" );
__PLIT8(KClkGpt11_F,		"a.GPTBf" );
__PLIT8(KClkUsbTll_F,		"a.UTLLf" );
__PLIT8(KClkTs_F,			"a.TSf" );
__PLIT8(KClkCpeFuse_F,		"a.FUSEf" );
__PLIT8(KClkSgx_F,			"a.SGXf" );
__PLIT8(KClkUsim_F,			"a.USIMf" );
__PLIT8(KClkSmartReflex2_F,	"a.SMRF2f" );
__PLIT8(KClkSmartReflex1_F,	"a.SMRF1f" );
__PLIT8(KClkWdt2_F,			"a.WDT2f" );
__PLIT8(KClkWdt3_F,			"a.WDT3f" );
__PLIT8(KClkGpio1_F,		"a.GPIO1f" );
__PLIT8(KClkGpio2_F,		"a.GPIO2f" );
__PLIT8(KClkGpio3_F,		"a.GPIO3f" );
__PLIT8(KClkGpio4_F,		"a.GPIO4f" );
__PLIT8(KClkGpio5_F,		"a.GPIO5f" );
__PLIT8(KClkGpio6_F,		"a.GPIO6f" );
__PLIT8(KClkUsb120_F,		"a.U120f" );
__PLIT8(KClkUsb48_F,		"a.U48f" );
__PLIT8(KClkDss_I,			"a.DSSi" );
__PLIT8(KClkCam_I,			"a.CAMi" );
__PLIT8(KClkIcr_I,			"a.ICRi" );
__PLIT8(KClkMmc1_I,			"a.MMC1i" );
__PLIT8(KClkMmc2_I,			"a.MMC2i" );
__PLIT8(KClkMmc3_I,			"a.MMC3i" );
__PLIT8(KClkMsPro_I,		"a.MSi" );
__PLIT8(KClkHdq_I,			"a.HDQi" );
__PLIT8(KClkAes1_I,			"a.AES1i" );
__PLIT8(KClkAes2_I,			"a.AES2i" );
__PLIT8(KClkSha11_I,		"a.SHA1i" );
__PLIT8(KClkSha12_I,		"a.SHA2i" );
__PLIT8(KClkDes1_I,			"a.DES1i" );
__PLIT8(KClkDes2_I,			"a.DES2i" );
__PLIT8(KClkMcBsp1_I,		"a.BSP1i" );
__PLIT8(KClkMcBsp2_I,		"a.BSP2i" );
__PLIT8(KClkMcBsp3_I,		"a.BSP3i" );
__PLIT8(KClkMcBsp4_I,		"a.BSP4i" );
__PLIT8(KClkMcBsp5_I,		"a.BSP5i" );
__PLIT8(KClkI2c1_I,			"a.I2C1i" );
__PLIT8(KClkI2c2_I,			"a.I2C2i" );
__PLIT8(KClkI2c3_I,			"a.I2C3i" );
__PLIT8(KClkUart1_I,		"a.UART1i" );
__PLIT8(KClkUart2_I,		"a.UART2i" );
__PLIT8(KClkUart3_I,		"a.UART3i" );
__PLIT8(KClkMcSpi1_I,		"a.SPI1i" );
__PLIT8(KClkMcSpi2_I,		"a.SPI2i" );
__PLIT8(KClkMcSpi3_I,		"a.SPI3i" );
__PLIT8(KClkMcSpi4_I,		"a.SPI4i" );
__PLIT8(KClkGpt1_I,			"a.GPT1i" );
__PLIT8(KClkGpt2_I,			"a.GPT2i" );
__PLIT8(KClkGpt3_I,			"a.GPT3i" );
__PLIT8(KClkGpt4_I,			"a.GPT4i" );
__PLIT8(KClkGpt5_I,			"a.GPT5i" );
__PLIT8(KClkGpt6_I,			"a.GPT6i" );
__PLIT8(KClkGpt7_I,			"a.GPT7i" );
__PLIT8(KClkGpt8_I,			"a.GPT8i" );
__PLIT8(KClkGpt9_I,			"a.GPT9i" );
__PLIT8(KClkGpt10_I,		"a.GPTAi" );
__PLIT8(KClkGpt11_I,		"a.GPTBi" );
__PLIT8(KClkGpt12_I,		"a.GPTCi" );
__PLIT8(KClkMailboxes_I,	"a.MBi" );
__PLIT8(KClkOmapSCM_I,		"a.SCMi" );
__PLIT8(KClkHsUsbOtg_I,		"a.OTGi" );
__PLIT8(KClkSdrc_I,			"a.SDRCi" );
__PLIT8(KClkPka_I,			"a.PKAi" );
__PLIT8(KClkRng_I,			"a.RNGi" );
__PLIT8(KClkUsbTll_I,		"a.TLLi" );
__PLIT8(KClkSgx_I,			"a.SGXi" );
__PLIT8(KClkUsim_I,			"a.USIMi" );
__PLIT8(KClkWdt1_I,			"a.WDT1i" );
__PLIT8(KClkWdt2_I,			"a.WDT2i" );
__PLIT8(KClkWdt3_I,			"a.WDT3i" );
__PLIT8(KClkGpio1_I,		"a.GPIO1i" );
__PLIT8(KClkGpio2_I,		"a.GPIO2i" );
__PLIT8(KClkGpio3_I,		"a.GPIO3i" );
__PLIT8(KClkGpio4_I,		"a.GPIO4i" );
__PLIT8(KClkGpio5_I,		"a.GPIO5i" );
__PLIT8(KClkGpio6_I,		"a.GPIO6i" );
__PLIT8(KClk32Sync_I,		"a.32SYNi" );
__PLIT8(KClkUsb_I,			"a.USBi" );
__PLIT8(KClk48M,			"a.48" );
__PLIT8(KClk12M,			"a.12" );
__PLIT8(KClkSysClk,			"a.SYSCLK" );
__PLIT8(KClkAltClk,			"a.ALTCLK" );
__PLIT8(KClkSysClk32k,		"a.SYS32K" );


// Table converting clock sources to string identifiers for PRM
static const TDesC8* const KNames[] =
	{
	(const TDesC8*)( &KClkMpu ),				// EClkMpu
	(const TDesC8*)( &KClkIva2Pll ),			// EClkIva2Pll
	(const TDesC8*)( &KClkCore ),				// EClkCore
	(const TDesC8*)( &KClkPeriph ),			// EClkPeriph
	(const TDesC8*)( &KClkPeriph2 ),			// EClkPeriph2
	(const TDesC8*)( &KClkPrcmInterface ),		// EClkPrcmInterface
	(const TDesC8*)( &KClkEmu ),				// EClkEmu
	(const TDesC8*)( &KClkNeon ),				// EClkNeon
	(const TDesC8*)( &KClkL3Domain ),			// EClkL3Domain
	(const TDesC8*)( &KClkL4Domain ),			// EClkL4Domain
	(const TDesC8*)( &KClkMpuPll_Bypass ),		// EClkMpuPll_Bypass
	(const TDesC8*)( &KClkIva2Pll_Bypass ),	// EClkIva2Pll_Bypass
	(const TDesC8*)( &KClkRM_F ),				// EClkRM_F
	(const TDesC8*)( &KClk96M ),				// EClk96M
	(const TDesC8*)( &KClk120M ),				// EClk120M
	(const TDesC8*)( &KClkSysOut ),			// EClkSysOut
	(const TDesC8*)( &KClkTv_F ),				// EClkTv_F
	(const TDesC8*)( &KClkDss1_F ),			// EClkDss1_F
	(const TDesC8*)( &KClkDss2_F ),			// EClkDss2_F
	(const TDesC8*)( &KClkCsi2_F ),			// EClkCsi2_F
	(const TDesC8*)( &KClkCam_F ),				// EClkCam_F
	(const TDesC8*)( &KClkIva2_F ),			// EClkIva2_F
	(const TDesC8*)( &KClkMmc1_F ),			// EClkMmc1_F
	(const TDesC8*)( &KClkMmc2_F ),			// EClkMmc2_F
	(const TDesC8*)( &KClkMmc3_F ),			// EClkMmc3_F
	(const TDesC8*)( &KClkMsPro_F ),			// EClkMsPro_F
	(const TDesC8*)( &KClkHdq_F ),				// EClkHdq_F
	(const TDesC8*)( &KClkMcBsp1_F ),			// EClkMcBsp1_F
	(const TDesC8*)( &KClkMcBsp2_F ),			// EClkMcBsp2_F
	(const TDesC8*)( &KClkMcBsp3_F ),			// EClkMcBsp3_F
	(const TDesC8*)( &KClkMcBsp4_F ),			// EClkMcBsp4_F
	(const TDesC8*)( &KClkMcBsp5_F ),			// EClkMcBsp5_F
	(const TDesC8*)( &KClkMcSpi1_F ),			// EClkMcSpi1_F
	(const TDesC8*)( &KClkMcSpi2_F ),			// EClkMcSpi2_F
	(const TDesC8*)( &KClkMcSpi3_F ),			// EClkMcSpi3_F
	(const TDesC8*)( &KClkMcSpi4_F ),			// EClkMcSpi4_F
	(const TDesC8*)( &KClkI2c1_F ),			// EClkI2c1_F
	(const TDesC8*)( &KClkI2c2_F ),			// EClkI2c2_F
	(const TDesC8*)( &KClkI2c3_F ),			// EClkI2c3_F
	(const TDesC8*)( &KClkUart1_F ),			// EClkUart1_F
	(const TDesC8*)( &KClkUart2_F ),			// EClkUart2_F
	(const TDesC8*)( &KClkUart3_F ),			// EClkUart3_F
	(const TDesC8*)( &KClkGpt1_F ),			// EClkGpt1_F
	(const TDesC8*)( &KClkGpt2_F ),			// EClkGpt2_F
	(const TDesC8*)( &KClkGpt3_F ),			// EClkGpt3_F
	(const TDesC8*)( &KClkGpt4_F ),			// EClkGpt4_F
	(const TDesC8*)( &KClkGpt5_F ),			// EClkGpt5_F
	(const TDesC8*)( &KClkGpt6_F ),			// EClkGpt6_F
	(const TDesC8*)( &KClkGpt7_F ),			// EClkGpt7_F
	(const TDesC8*)( &KClkGpt8_F ),			// EClkGpt8_F
	(const TDesC8*)( &KClkGpt9_F ),			// EClkGpt9_F
	(const TDesC8*)( &KClkGpt10_F ),			// EClkGpt10_F
	(const TDesC8*)( &KClkGpt11_F ),			// EClkGpt11_F
	(const TDesC8*)( &KClkUsbTll_F ),			// EClkUsbTll_F
	(const TDesC8*)( &KClkTs_F ),				// EClkTs_F
	(const TDesC8*)( &KClkCpeFuse_F ),			// EClkCpeFuse_F
	(const TDesC8*)( &KClkSgx_F ),				// EClkSgx_F
	(const TDesC8*)( &KClkUsim_F ),			// EClkUsim_F
	(const TDesC8*)( &KClkSmartReflex2_F ),	// EClkSmartReflex2_F
	(const TDesC8*)( &KClkSmartReflex1_F ),	// EClkSmartReflex1_F
	(const TDesC8*)( &KClkWdt2_F ),			// EClkWdt2_F
	(const TDesC8*)( &KClkWdt3_F ),			// EClkWdt3_F
	(const TDesC8*)( &KClkGpio1_F ),			// EClkGpio1_F
	(const TDesC8*)( &KClkGpio2_F ),			// EClkGpio2_F
	(const TDesC8*)( &KClkGpio3_F ),			// EClkGpio3_F
	(const TDesC8*)( &KClkGpio4_F ),			// EClkGpio4_F
	(const TDesC8*)( &KClkGpio5_F ),			// EClkGpio5_F
	(const TDesC8*)( &KClkGpio6_F ),			// EClkGpio6_F
	(const TDesC8*)( &KClkUsb120_F ),			// EClkUsb120_F
	(const TDesC8*)( &KClkUsb48_F ),			// EClkUsb48_F
	(const TDesC8*)( &KClkDss_I ),				// EClkDss_I
	(const TDesC8*)( &KClkCam_I ),				// EClkCam_I
	(const TDesC8*)( &KClkIcr_I ),				// EClkIcr_I
	(const TDesC8*)( &KClkMmc1_I ),			// EClkMmc1_I
	(const TDesC8*)( &KClkMmc2_I ),			// EClkMmc2_I
	(const TDesC8*)( &KClkMmc3_I ),			// EClkMmc3_I
	(const TDesC8*)( &KClkMsPro_I ),			// EClkMsPro_I
	(const TDesC8*)( &KClkHdq_I ),				// EClkHdq_I
	(const TDesC8*)( &KClkAes1_I ),			// EClkAes1_I
	(const TDesC8*)( &KClkAes2_I ),			// EClkAes2_I
	(const TDesC8*)( &KClkSha11_I ),			// EClkSha11_I
	(const TDesC8*)( &KClkSha12_I ),			// EClkSha12_I
	(const TDesC8*)( &KClkDes1_I ),			// EClkDes1_I
	(const TDesC8*)( &KClkDes2_I ),			// EClkDes2_I
	(const TDesC8*)( &KClkMcBsp1_I ),			// EClkMcBsp1_I
	(const TDesC8*)( &KClkMcBsp2_I ),			// EClkMcBsp2_I
	(const TDesC8*)( &KClkMcBsp3_I ),			// EClkMcBsp3_I
	(const TDesC8*)( &KClkMcBsp4_I ),			// EClkMcBsp4_I
	(const TDesC8*)( &KClkMcBsp5_I ),			// EClkMcBsp5_I
	(const TDesC8*)( &KClkI2c1_I ),			// EClkI2c1_I
	(const TDesC8*)( &KClkI2c2_I ),			// EClkI2c2_I
	(const TDesC8*)( &KClkI2c3_I ),			// EClkI2c3_I
	(const TDesC8*)( &KClkUart1_I ),			// EClkUart1_I
	(const TDesC8*)( &KClkUart2_I ),			// EClkUart2_I
	(const TDesC8*)( &KClkUart3_I ),			// EClkUart3_I
	(const TDesC8*)( &KClkMcSpi1_I ),			// EClkMcSpi1_I
	(const TDesC8*)( &KClkMcSpi2_I ),			// EClkMcSpi2_I
	(const TDesC8*)( &KClkMcSpi3_I ),			// EClkMcSpi3_I
	(const TDesC8*)( &KClkMcSpi4_I ),			// EClkMcSpi4_I
	(const TDesC8*)( &KClkGpt1_I ),			// EClkGpt1_I
	(const TDesC8*)( &KClkGpt2_I ),			// EClkGpt2_I
	(const TDesC8*)( &KClkGpt3_I ),			// EClkGpt3_I
	(const TDesC8*)( &KClkGpt4_I ),			// EClkGpt4_I
	(const TDesC8*)( &KClkGpt5_I ),			// EClkGpt5_I
	(const TDesC8*)( &KClkGpt6_I ),			// EClkGpt6_I
	(const TDesC8*)( &KClkGpt7_I ),			// EClkGpt7_I
	(const TDesC8*)( &KClkGpt8_I ),			// EClkGpt8_I
	(const TDesC8*)( &KClkGpt9_I ),			// EClkGpt9_I
	(const TDesC8*)( &KClkGpt10_I ),			// EClkGpt10_I
	(const TDesC8*)( &KClkGpt11_I ),			// EClkGpt11_I
	(const TDesC8*)( &KClkGpt12_I ),			// EClkGpt12_I
	(const TDesC8*)( &KClkMailboxes_I ),		// EClkMailboxes_I
	(const TDesC8*)( &KClkOmapSCM_I ),			// EClkOmapSCM_I
	(const TDesC8*)( &KClkHsUsbOtg_I ),		// EClkHsUsbOtg_I
	(const TDesC8*)( &KClkSdrc_I ),			// EClkSdrc_I
	(const TDesC8*)( &KClkPka_I ),				// EClkPka_I
	(const TDesC8*)( &KClkRng_I ),				// EClkRng_I
	(const TDesC8*)( &KClkUsbTll_I ),			// EClkUsbTll_I
	(const TDesC8*)( &KClkSgx_I ),				// EClkSgx_I
	(const TDesC8*)( &KClkUsim_I ),			// EClkUsim_I
	(const TDesC8*)( &KClkWdt1_I ),			// EClkWdt1_I
	(const TDesC8*)( &KClkWdt2_I ),			// EClkWdt2_I
	(const TDesC8*)( &KClkWdt3_I ),			// EClkWdt3_I
	(const TDesC8*)( &KClkGpio1_I ),			// EClkGpio1_I
	(const TDesC8*)( &KClkGpio2_I ),			// EClkGpio2_I
	(const TDesC8*)( &KClkGpio3_I ),			// EClkGpio3_I
	(const TDesC8*)( &KClkGpio4_I ),			// EClkGpio4_I
	(const TDesC8*)( &KClkGpio5_I ),			// EClkGpio5_I
	(const TDesC8*)( &KClkGpio6_I ),			// EClkGpio6_I
	(const TDesC8*)( &KClk32Sync_I ),			// EClk32Sync_I
	(const TDesC8*)( &KClkUsb_I ),				// EClkUsb_I
	(const TDesC8*)( &KClk48M ),				// EClk48M
	(const TDesC8*)( &KClk12M ),				// EClk12M
	(const TDesC8*)( &KClkSysClk ),				// EClkSysClk
	(const TDesC8*)( &KClkAltClk ),				// EClkAltClk
	(const TDesC8*)( &KClkSysClk32k ),			// EClkSysClk32k
	};

__ASSERT_COMPILE( (sizeof( KNames ) / sizeof( KNames[0] )) == Prcm::KSupportedClockCount );

#endif /* PRCM_H_ */
