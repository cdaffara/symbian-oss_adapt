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
// Access to PRCM.
// This file is part of the Beagle Base port
//

#ifndef PRCM_H__
#define PRCM_H__

#include <e32cmn.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>

namespace Prcm
{
enum TPanic
	{
	ESetPllConfigBadPll,	///< bad PLL ID in SetPllConfiguration()
	EGetPllConfigBadPll,	///< bad PLL ID in PllConfiguration()
	ESetPllConfigBadFreqRange,	///< bad PLL frequency range in SetPllConfiguration()
	ESetPllConfigBadRamp,		///< bad PLL ramp setting in SetPllConfiguration()
	ESetPllConfigBadDrift,		///< bad PLL drift setting in SetPllConfiguration()
	ESetPllConfigBadDivider,	///< bad divider setting in SetPllConfiguration()
	ESetPllConfigBadMultiplier,	///< bad divider setting in SetPllConfiguration()
	ESetPllLpBadPll,		///< bad PLL ID in SetPllLp()
	EGetPllLpBadPll,		///< bad PLL ID in PllLp()
	ESetPllLpBadMode,		///< bad PLL LP mode in SetPllLp()
	ESetDividerBadClock,	///< bad clock ID in SetDivider()
	EGetDividerBadClock,	///< bad clock ID in Divider()
	ESetStateBadClock,		///< bad clock ID in SetClockState()
	ESetWakeupBadClock,		///< bad clock ID in SetWakeupMode()
	ESetPllModeBadClock,
	ESetPllModeBadMode,
	EGetStateBadClock,		///< bad clock ID in ClockState()
	EGetWakeupBadClock,		///< bad clock ID in WakeupMode()
	ESetGptClockBadGpt,		///< bad GPT ID in SetGptClockSource()
	EGetWakeupGroupBadClock,
	EGetWakeupGroupBadGroup,
	EAddWakeupGroupBadClock,
	EAddWakeupGroupBadGroup,
	ERemoveWakeupGroupBadClock,
	ERemoveWakeupGroupBadGroup,
	EAddDomainBadClock,		///< bad clock in call to AddToWakeupDomain()
	ERemoveDomainBadClock,	///< bad clock in call to RemoveFromWakeupDomain()
	ECheckDomainBadClock,	///< bad clock in call to IsInWakeupDomain()
	EAddDomainBadDomain,	///< bad domain in call to AddToWakeupDomain()
	ERemoveDomainBadDomain,	///< bad domain in call to RemoveFromWakeupDomain()
	ECheckDomainBadDomain,	///< bad domain in call to IsInWakeupDomain()
	ESetDividerUnsupportedClock,	///< attempt to set divider on clock that does not have divider
	ESetDividerBadDivider,	///< bad divider value in SetDivider()
	EGetNameBadClock,		///< bad clock ID in PrmName()
	EClockFrequencyBadClock,	///< bad clock ID in ClockFrequency()
	ESetClockMuxBadClock,	///< bad clock ID in SetClockMux()
	ESetClockMuxBadSource,	///< bad source clock ID in SetClockMux()
	EGetClockMuxBadClock,	///< bad clock ID in ClockMux()
	ESetDomainModeBadDomain,	///< bad domain in SetPowerDomainMode()
	ESetDomainModeBadMode,		///< bad mode in SetPowerDomainMode()
	EGetDomainModeBadDomain,	///< bad domain in PowerDomainMode()
	ESetDomainModeUnsupportedMode,	///< mode requested in SetPowerDomainMode() not supported by that domain
	EPllIsLockedBadPll,			///< bad PLL ID in PllIsLocked()
	EWaitForPllLockBadPll,		///< bad PLL ID in WaitForPllLocked()
	ESetPllBypassDividerBadPll,	///< bad PLL ID in SetPllBypassDivider()
	EPllBypassDividerBadPll,		///< bad PLL ID in PllBypassDivider()
	ESetPllBypassDividerBadDivider,	///< bad dividier value in SetPllBypassDivider()
	EPllInternalFrequencyOutOfRange	///< PLL internal frequency out of range in AutoSetPllFrequencyRange()
	};

enum TClock
	{
	EClkMpu,		///< DPLL1
	EClkIva2Pll,	///< DPLL2
	EClkCore,		///< DPLL3
	EClkPeriph,		///< DPLL4
	EClkPeriph2,	///< DPLL5

	EClkPrcmInterface,

	EClkEmu,		///< Emulation clock
	EClkNeon,

	EClkL3Domain,
	EClkL4Domain,

	EClkMpuPll_Bypass,	///< DPLL1 bypass frequency
	EClkIva2Pll_Bypass,	///< DPLL2 bypass frequency
	EClkRM_F,			///< Reset manager functional clock	
	EClk96M,			///< 96MHz clock
	EClk120M,			///< 120MHz clock
	EClkSysOut,

	// Functional clocks
	EClkTv_F,			///< TV functional clock, same as 54MHz FCLK
	EClkDss1_F,
	EClkDss2_F,
	EClkCsi2_F,
	EClkCam_F,
	EClkIva2_F,
	EClkMmc1_F,
	EClkMmc2_F,
	EClkMmc3_F,
	EClkMsPro_F,
	EClkHdq_F,
	EClkMcBsp1_F,
	EClkMcBsp2_F,
	EClkMcBsp3_F,
	EClkMcBsp4_F,
	EClkMcBsp5_F,
	EClkMcSpi1_F,
	EClkMcSpi2_F,
	EClkMcSpi3_F,
	EClkMcSpi4_F,
	EClkI2c1_F,
	EClkI2c2_F,
	EClkI2c3_F,
	EClkUart1_F,
	EClkUart2_F,
	EClkUart3_F,
	EClkGpt1_F,
	EClkGpt2_F,
	EClkGpt3_F,
	EClkGpt4_F,
	EClkGpt5_F,
	EClkGpt6_F,
	EClkGpt7_F,
	EClkGpt8_F,
	EClkGpt9_F,
	EClkGpt10_F,
	EClkGpt11_F,
	EClkUsbTll_F,
	EClkTs_F,
	EClkCpeFuse_F,

	EClkSgx_F,

	EClkUsim_F,
	EClkSmartReflex2_F,
	EClkSmartReflex1_F,
	EClkWdt2_F,
	EClkWdt3_F,
	EClkGpio1_F,
	EClkGpio2_F,
	EClkGpio3_F,
	EClkGpio4_F,
	EClkGpio5_F,
	EClkGpio6_F,

	EClkUsb120_F,		///< USB host 120MHz functional clock
	EClkUsb48_F,		///< USB host 48MHz functional clock


	// Interface clocks
	EClkDss_I,
	EClkCam_I,
	EClkIcr_I,
	EClkMmc1_I,
	EClkMmc2_I,
	EClkMmc3_I,
	EClkMsPro_I,
	EClkHdq_I,
	EClkAes1_I,
	EClkAes2_I,
	EClkSha11_I,
	EClkSha12_I,
	EClkDes1_I,
	EClkDes2_I,
	EClkMcBsp1_I,
	EClkMcBsp2_I,
	EClkMcBsp3_I,
	EClkMcBsp4_I,
	EClkMcBsp5_I,
	EClkI2c1_I,
	EClkI2c2_I,
	EClkI2c3_I,
	EClkUart1_I,
	EClkUart2_I,
	EClkUart3_I,
	EClkMcSpi1_I,
	EClkMcSpi2_I,
	EClkMcSpi3_I,
	EClkMcSpi4_I,
	EClkGpt1_I,
	EClkGpt2_I,
	EClkGpt3_I,
	EClkGpt4_I,
	EClkGpt5_I,
	EClkGpt6_I,
	EClkGpt7_I,
	EClkGpt8_I,
	EClkGpt9_I,
	EClkGpt10_I,
	EClkGpt11_I,
	EClkGpt12_I,
	EClkMailboxes_I,
	EClkOmapSCM_I,
	EClkHsUsbOtg_I,
	EClkSdrc_I,
	EClkPka_I,
	EClkRng_I,
	EClkUsbTll_I,

	EClkSgx_I,

	EClkUsim_I,
	EClkWdt1_I,
	EClkWdt2_I,
	EClkWdt3_I,
	EClkGpio1_I,
	EClkGpio2_I,
	EClkGpio3_I,
	EClkGpio4_I,
	EClkGpio5_I,
	EClkGpio6_I,
	EClk32Sync_I,

	EClkUsb_I,			///< USB host interface clock

	EClk48M,			///< 48MHz clock
	EClk12M,			///< 12MHz clock

	// These cannot be modified, they just represent the input clocks
	// They must remain last in the table
	EClkSysClk,			///< SYSCLK input clock
	EClkAltClk,			///< SYSCLK32k input clock
	EClkSysClk32k,		///< ALTCLK input clock

	KSupportedClockCount
	};

enum TInterruptIds
	{
	EIntWkUp	= (EIrqRangeBasePrcm << KIrqRangeIndexShift),
	EIntUnused1,
	EIntEvGenOn,
	EIntEvGenOff,
	EIntTransition,
	EIntCoreDpll,
	EIntPeriphDpll,
	EIntMpuDpll,
	EIntIvaDpll,
	EIntIo,
	EIntVp1OpChangeDone,
	EIntVp1MinVdd,
	EIntVp1MaxVdd,
	EIntVp1NoSmpsAck,
	EIntVp1EqValue,
	EIntVp1TranDone,
	EIntVp2OpChangeDone,
	EIntVp2MinVdd,
	EIntVp2MaxVdd,
	EIntVp2NoSmpsAck,
	EIntVp2EqValue,
	EIntVp2TranDone,
	EIntVcSaErr,
	EIntVcRaErr,
	EIntVcTimeoutErr,
	EIntSndPeriphDpll,

	KPrcmLastInterruptPlusOne
	};
const TInt KInterruptCount = KPrcmLastInterruptPlusOne - EIrqRangeBasePrcm;


/** GPT reference enumeration */
enum TGpt
	{
	EGpt1,
	EGpt2,
	EGpt3,
	EGpt4,
	EGpt5,
	EGpt6,
	EGpt7,
	EGpt8,
	EGpt9, 
	EGpt10,
	EGpt11,
	EGpt12,

	KSupportedGptCount
	};

/** Enumeration of supported PLLs */
enum TPll
	{
	EDpll1,
	EDpll2,
	EDpll3,
	EDpll4,
	EDpll5,

	KSupportedPllCount
	};

enum TGptClockSource
	{
	EGptClockSysClk,
	EGptClock32k
	};

enum TClockState
	{
	EClkOff,		///< clock is disabled
	EClkOn,		///< clock is enabled
	EClkAuto		///< clock is in auto mode (enabled when required)
	};

enum TWakeupMode
	{
	EWakeupDisabled,
	EWakeupEnabled,
	};

enum TLpMode
	{
	ENormalMode,
	ELpMode
	};

enum TPowerSaveMode
	{
	EPowerSaveOff,
	EPowerSaveIdle,
	EPowerSaveStandby
	};

enum TPllMode
	{
	EPllStop,
	EPllBypass,
	EPllRun,
	EPllFastRelock,
	EPllAuto
	};

enum TBypassDivider
	{
	EBypassDiv1,
	EBypassDiv2,
	EBypassDiv4
	};

enum TPowerDomainMode
	{
	EPowerOff,
	EPowerRetention,
	EPowerReserved,
	EPowerOn,
	};

enum TPowerDomain
	{
	EPowerDomainMpu,
	EPowerDomainIva2,
	EPowerDomainNeon,
	EPowerDomainCore,
	EPowerDomainSgx,
	EPowerDomainDss,
	EPowerDomainCamera,
	EPowerDomainUsb,
	EPowerDomainPer,

	KSupportedPowerDomainCount
	};

enum TWakeupDomain
	{
	EWakeDomainMpu,
	EWakeDomainCore,
	EWakeDomainIva2,
	EWakeDomainPeripheral,
	EWakeDomainDss,
	EWakeDomainWakeup,

	KSupportedWakeupDomainCount
	};

enum TWakeupGroup
	{
	EWakeGroupMpu,
	EWakeGroupIva2,

	KSupportedWakeupGroupCount
	};

/** Indicates how to handle a request to set a clock frequency */
enum TClockRoundMode
	{
	EExactOnly,		///< only set clock if requested frequency can be set exactly
	ENearest,		///< always set clock to nearest possible frequency higher or lower than requested
	ENearestLower,	///< set to nearest frequency <=requested, fail if no frequency <= requested is possible
	ENearestHigher,	///< set to nearest frequency >=requested, fail if no frequency >= requested is possible
	};

/** Enumeration of valid Pll frequency ranges */
enum TPllFrequencyRange
	{
	EPllRange_075_100		= 0x3,	///<	0.75 - 1.0 MHz
	EPllRange_100_125		= 0x4,	///<	<1.0 MHz - 1.25 MHz
	EPllRange_125_150		= 0x5,	///<	<1.25 MHz - 1.5 MHz
	EPllRange_150_175		= 0x6,	///<	<1.5 MHz - 1.75 MHz
	EPllRange_175_210		= 0x7,	///<	<1.75 MHz - 2.1 MHz
	EPllRange_750_1000		= 0xB,	///<	<7.5 MHz - 10 MHz
	EPllRange_1000_1250		= 0xC,	///<	<10 MHz - 12.5 MHz
	EPllRange_1250_1500		= 0xD,	///<	<12.5 MHz - 15 MHz
	EPllRange_1500_1750		= 0xE,	///<	<15 MHz - 17.5 MHz
	EPllRange_1750_2100		= 0xF	///<	<17.5 MHz - 21 MHz
	};

/** Enumeration of valid PLL ramp settings */
enum TPllRamp
	{
	EPllRampDisabled = 0x0,
	EPllRamp4us		= 0x1,
	EPllRam20us		= 0x2,
	EPllRamp40us	= 0x3
	};

/** Enumeration of vali PLL driftguard settings */
enum TPllDrift
	{
	EPllDriftGuardDisabled,
	EPllDriftGuardEnabled
	};

/** Structure containing configuration for a PLL */
struct TPllConfiguration
	{
	TUint		iMultiplier;		///< Multiple value
	TUint		iDivider;			///< Divider value (this is actual divider, hardware is programmed with iDivider-1)
	TPllFrequencyRange	iFreqRange : 8;
	TPllRamp	iRamp : 8;
	TPllDrift	iDrift : 8;
	TUint8		__spare;
	};

/** Enumeration of supported SysClk frequency configurations */
enum TSysClkFrequency
	{
	ESysClk12MHz,
	ESysClk13MHz,
	ESysClk16_8MHz,
	ESysClk19_2MHz,
	ESysClk26MHz,
	ESysClk38_4MHz
	};

// called during start-up
IMPORT_C void Init3(); // PRCM (disable every peripheral leaving DSS (and UART3 in debug) running)

IMPORT_C void SetPllConfig( TPll aPll, const TPllConfiguration& aConfig  );
IMPORT_C void PllConfig( TPll aPll, TPllConfiguration& aConfigResult );


/** Configure PLL frequency */
IMPORT_C void SetPllMode( TPll aPll, TPllMode aPllMode );

/** Return PLL frequency configuration */
IMPORT_C TPllMode PllMode( TPll aPll );

/** Test whether a PLL is locked */
IMPORT_C TBool PllIsLocked( TPll aPll );

/** Wait for a PLL to lock */
IMPORT_C void WaitForPllLock( TPll aPll );

/** Calculate the correct FreqRange setting for the given pll
 * Updates the iFreqRange parameter of the given TPllConfiguration
 */
IMPORT_C void CalcPllFrequencyRange( TPll aPll, TPllConfiguration& aConfig );

/** Enable LP mode on a DLL if it is within LP frequency range */
IMPORT_C void AutoSetPllLpMode( TPll aPll );

/** Enable or disable PLL LP mode */
IMPORT_C void SetPllLp( TPll aPll, TLpMode aLpMode );

/** Get LP mode setting for a PLL */
IMPORT_C TLpMode PllLp( TPll aPll );

/** Set the bypass divider for a PLL */
IMPORT_C void SetPllBypassDivider( TPll aPll, TBypassDivider aDivider );

/** Get the current bypass divider for a PLL */
IMPORT_C TBypassDivider PllBypassDivider( TPll aPll );

/** Set the divider value for the given clock 
 * aDivider is the required divide value - e.g. to divide by 4
 * aDivider=4.
 *
 * Note that not all clocks support division by any number, and
 * only some clocks have a divider. Attempting to set a divider
 * on a clock without a divider will have no effect in UREL and
 * will panic in UDEB with ESetDividerUnsupportedClock.
 * Attempting to set a divider value not supported by the clock
 * will have no effect in UREL and will panic in UDEB with
 * ESetDividerBadDivider.
 *
 * Note 1: for EClkSgx_F the value valued of aDivide are 0, 3, 4, 6.
 * 0 sets the clock to be the 96MHz clock
 * 3, 4, 6 set it to be CORE_CLK divided by 3, 4, or 6
 *
 * Note 2: you cannot use this function to set EClkUsim_F, use
 * SetUsimClockDivider().
 */
IMPORT_C void SetDivider( TClock aClock, TUint aDivide );

/** Get the current divider value of the given clock */
IMPORT_C TUint Divider( TClock aClock );

//IMPORT_C void SetUsimClockDivider( TUint TUsimDivideMode aMode );
//IMPORT_C TUsimDivideMode UsimClockDivider();

/** Controls power to a power domain */
IMPORT_C void SetPowerDomainMode( TPowerDomain aDomain, TPowerDomainMode aMode );

/** Gets the current mode of a power domain power control */
IMPORT_C TPowerDomainMode PowerDomainMode( TPowerDomain aDomain );

//IMPORT_C void SetPowerSaveMode( TClock aClock, TPowerSaveMode aMode );
//IMPORT_C TPowerSaveMode PowerSaveMode( TClock aClock );

//IMPORT_C TBool DomainClockActive( TClock aClock );

// Set clock enable/disable
/** Set the clock state of a given clock */
IMPORT_C void SetClockState( TClock aClock, TClockState aState );

/** Get the configured clock state of a given clock */
IMPORT_C TClockState ClockState( TClock aClock );

// Configure wakeup mode for clocks
/** Configure wakeup mode for a clock
 * Note - for peripheral blocks with an interface and functional clock, it is
 * the interface clock which is configured for wakeup. Attempting to configure
 * wakeup on the functional clock has no effect
 */
IMPORT_C void SetWakeupMode( TClock aClock, TWakeupMode aMode );

/** Get configured wakeup mode for a clock */
IMPORT_C TWakeupMode WakeupMode( TClock aClock );

/** Add a peripheral interface clock to the specified wakeup group */
IMPORT_C void AddToWakeupGroup( TClock aClock, TWakeupGroup aGroup );

/** Remove a peripheral interface clock from the specified wakeup group */
IMPORT_C void RemoveFromWakeupGroup( TClock aClock, TWakeupGroup aGroup );

/** Test whether a peripheral interface clock is in the specified wakeup group */
IMPORT_C TBool IsInWakeupGroup( TClock aClock, TWakeupGroup aGroup );

/** Add a clock to the given wakeup domain */
IMPORT_C void AddToWakeupDomain( TClock aClock, TWakeupDomain aDomain );

/** Remove a clock from the given wakeup domain */
IMPORT_C void RemoveFromWakeupDomain( TClock aClock, TWakeupDomain aDomain );

/** Test whether a clock is in the specified wakeup domain */
IMPORT_C TBool IsInWakeupDomain( TClock aClock, TWakeupDomain aDomain );


// Functions for configuring clock sources

/** Set the clock source for a GPT timer */
IMPORT_C void SetGptClockSource( TGpt aGpt, TGptClockSource aSource );

/** Get the current clock source of a GPT */
IMPORT_C TGptClockSource GptClockSource( TGpt aGpt );

/** Get the USIM divider factor */
IMPORT_C TUint UsimDivider();

/** Get the USIM source clock */
IMPORT_C TClock UsimClockSource();

/** Sets the current input clock into the clock mux for the specified clock
 * aClock must refer to a clock that has a mux for selecting input clock
 * and aSource must be a possible input clock for aClock
 */
IMPORT_C void SetClockMux( TClock aClock, TClock aSource );


/** Gets the current input clock into the clock mux for the specified clock
 * aClock must refer to a clock that has a mux for selecting input clock
 */
IMPORT_C TClock ClockMux( TClock aClock );

/** Get the currently configured frequency of the specified clock
 * Note that this is regardless of whether the clock is currently running.
 * That is, if a clock is configured to run at 8MHz, then this function
 * will return 8000000 whether the clock is currently enabled or disabled.
 *
 * @param	aClock	clock required
 * @return	Frequency in Hz
 */
IMPORT_C TUint ClockFrequency( TClock aClock );

/** Set the correct SysClock frequency */
IMPORT_C void SetSysClkFrequency( TSysClkFrequency aFrequency );

/** Get the currently configured SysClk frequency */
IMPORT_C TSysClkFrequency SysClkFrequency();

/** Function to get the name to be passed to the Power Resource Manager
 * to refer to the given clock source
 */
IMPORT_C const TDesC& PrmName( TClock aClock );

}

#endif // !defined PRCM_H__
