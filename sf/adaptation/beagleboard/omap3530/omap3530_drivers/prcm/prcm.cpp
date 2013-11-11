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
// \omap3530\omap3530_assp\prcm.cpp
// Access to PRCM. And implimentation of device driver's power and clock control API
// This file is part of the Beagle Base port
//

#include <e32cmn.h>
#include <assp/omap3530_assp/omap3530_prcm.h>
#include <assp/omap3530_assp/omap3530_ktrace.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>
#include <assp/omap3530_assp/omap3530_hardware_base.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <nkern.h>

#include "prcm_regs.h"
#include "prcm.h"



namespace Prcm
{
TSpinLock iLock(/*TSpinLock::EOrderGenericIrqLow0*/); // prevents concurrent access to the prcm hardware registers

void Panic( TPanic aPanic )
	{
	Kern::Fault( "PRCM", aPanic );
	}

void InternalPanic( TInt aLine )
	{
	Kern::Fault( "PRCMINT", aLine );
	}

FORCE_INLINE void _BitClearSet( TUint32 aRegister, TUint32 aClearMask, TUint32 aSetMask )
	{
	volatile TUint32* pR = (volatile TUint32*)aRegister;
	*pR = (*pR & ~aClearMask) | aSetMask;
	}

FORCE_INLINE void _LockedBitClearSet( TUint32 aRegister, TUint32 aClearMask, TUint32 aSetMask )
	{
	volatile TUint32* pR = (volatile TUint32*)aRegister;
	TInt irq = __SPIN_LOCK_IRQSAVE(iLock);
	*pR = (*pR & ~aClearMask) | aSetMask;
	__SPIN_UNLOCK_IRQRESTORE(iLock, irq);
	}


EXPORT_C void SetPllConfig( TPll aPll, const TPllConfiguration& aConfig  )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetPllConfig(%x)", aPll ) );

	__ASSERT_DEBUG( (TUint)aPll < KSupportedPllCount, Panic( ESetPllConfigBadPll ) );

	const TPllControlInfo& inf = KPllControlInfo[ aPll ];

	__ASSERT_DEBUG( aConfig.iDivider <= KPllMaximumDivider,		Panic( ESetPllConfigBadDivider ) );
	__ASSERT_DEBUG( aConfig.iMultiplier <= KPllMaximumMultiplier,	Panic( ESetPllConfigBadMultiplier ) );
	__ASSERT_DEBUG( ((TUint)aConfig.iFreqRange <= EPllRange_1750_2100)
				&&  ((TUint)aConfig.iFreqRange >= EPllRange_075_100),	Panic( ESetPllConfigBadFreqRange ) );
	__ASSERT_DEBUG( ((TUint)aConfig.iRamp <= EPllRamp40us),	Panic( ESetPllConfigBadRamp ) );
	__ASSERT_DEBUG( (TUint)aConfig.iDrift <= EPllDriftGuardEnabled,	Panic( ESetPllConfigBadDrift ) );

	TUint	mult = (aConfig.iMultiplier bitand KPllMultiplierMask) << inf.iMultShift;
	TUint	div = ((aConfig.iDivider - 1) bitand KPllDividerMask) << inf.iDivShift;
	TUint	range = (aConfig.iFreqRange bitand KPllFreqRangeMask) << inf.iFreqSelShift;
	TUint	ramp = (aConfig.iRamp bitand KPllRampMask) << inf.iRampShift;
	TUint	drift = (aConfig.iDrift == EPllDriftGuardEnabled) ? (1 << inf.iDriftShift) : 0;

	TInt irq = __SPIN_LOCK_IRQSAVE(iLock);
	// We must apply frequency range setting before new multuplier and divider
	TUint clearMaskConfig =			(KPllFreqRangeMask << inf.iFreqSelShift)
							bitor	(KPllRampMask << inf.iRampShift)
							bitor	(1 << inf.iDriftShift);
	_BitClearSet( inf.iConfigRegister, clearMaskConfig, range | ramp | drift );

	TUint clearMaskMulDiv =	(KPllMultiplierMask << inf.iMultShift) bitor (KPllDividerMask << inf.iDivShift);
	_BitClearSet( inf.iMulDivRegister, clearMaskMulDiv, mult | div );
	__SPIN_UNLOCK_IRQRESTORE(iLock, irq);
	}

EXPORT_C void PllConfig( TPll aPll, TPllConfiguration& aConfigResult )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::PllConfig(%x)", aPll ) );

	__ASSERT_DEBUG( (TUint)aPll < KSupportedPllCount, Panic( EGetPllConfigBadPll ) );

	const TPllControlInfo& inf = KPllControlInfo[ aPll ];

	TUint32 config = AsspRegister::Read32( inf.iConfigRegister );
	TUint32 muldiv = AsspRegister::Read32( inf.iMulDivRegister );

	aConfigResult.iMultiplier =	(muldiv >> inf.iMultShift) bitand KPllMultiplierMask;
	aConfigResult.iDivider =	1 + ((muldiv >> inf.iDivShift) bitand KPllDividerMask);
	aConfigResult.iFreqRange =	static_cast<TPllFrequencyRange>((config >> inf.iFreqSelShift) bitand KPllFreqRangeMask);
	aConfigResult.iRamp =		static_cast<TPllRamp>((config >> inf.iRampShift ) bitand KPllRampMask);
	aConfigResult.iDrift =		(config >> inf.iDriftShift ) bitand 1 ? EPllDriftGuardEnabled : EPllDriftGuardDisabled;

	__KTRACE_OPT( KPRCM, Kern::Printf( "DPLL%d: m=%d, d=%d, fr=%d, r=%d, dr=%d",
						aPll + 1,
						aConfigResult.iMultiplier,
						aConfigResult.iDivider,
						aConfigResult.iFreqRange,
						aConfigResult.iRamp,
						aConfigResult.iDrift ) );
	}

EXPORT_C void SetPllLp( TPll aPll, TLpMode aLpMode )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetPllLp(%x)", aPll ) );

	__ASSERT_DEBUG( (TUint)aPll < KSupportedPllCount, Panic( ESetPllLpBadPll ) );
	__ASSERT_DEBUG( (aLpMode == ENormalMode)
					|| (aLpMode == ELpMode), Panic( ESetPllLpBadMode ) );

	const TPllControlInfo& inf = KPllControlInfo[ aPll ];

	TUint32 clear = 1 << inf.iLpShift;
	TUint32 set = 0;

	if( ELpMode == aLpMode )
		{
		set = clear;
		clear = 0;
		}

	_LockedBitClearSet( inf.iConfigRegister, clear, set );
	}

EXPORT_C TLpMode PllLp( TPll aPll )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::PllLp(%x)", aPll ) );

	__ASSERT_DEBUG( (TUint)aPll < KSupportedPllCount, Panic( EGetPllLpBadPll ) );

	const TPllControlInfo& inf = KPllControlInfo[ aPll ];

	TUint32 config = AsspRegister::Read32( inf.iConfigRegister );
	if( 0 == ((config >> inf.iLpShift) bitand 1) )
		{
		return ENormalMode;
		}
	else
		{
		return ELpMode;
		}
	}


EXPORT_C void SetPllMode( TPll aPll, TPllMode aPllMode )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetPllMode(%x;%x)", aPll, aPllMode ) );
	__ASSERT_DEBUG( (TUint)aPll <= EDpll5, Panic( ESetPllModeBadClock ) );

	TUint32 newMode;
	TUint32 newAuto = KPllAutoOff;

	switch( aPllMode )
		{
		default:
			__DEBUG_ONLY( Panic( ESetPllModeBadMode ) );
			return;

		case EPllStop:
			newMode = KPllModeStop;
			break;

		case EPllBypass:
			newMode = KPllModeBypass;
			break;

		case EPllAuto:
			newAuto = KPllAutoOn;
			// fall through...

		case EPllRun:
			newMode = KPllModeLock;
			break;

		case EPllFastRelock:
			newMode = KPllModeFastRelock;
			break;
		}

	TInt irq = __SPIN_LOCK_IRQSAVE(iLock);

	_BitClearSet(	KPllMode[ aPll ].iModeRegister,
					KPllModeMask << KPllMode[ aPll ].iModeShift,
					newMode << KPllMode[ aPll ].iModeShift );

	_BitClearSet(	KPllMode[ aPll ].iAutoRegister,
					KPllAutoMask << KPllMode[ aPll ].iAutoShift,
					newAuto << KPllMode[ aPll ].iAutoShift );

	__SPIN_UNLOCK_IRQRESTORE(iLock, irq);
	}


EXPORT_C TPllMode PllMode( TPll aPll )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::PllMode(%x)", aPll ) );
	__ASSERT_DEBUG( (TUint)aPll <= EDpll5, Panic( ESetPllModeBadClock ) );

	TUint32 mode = (AsspRegister::Read32( KPllMode[ aPll ].iModeRegister ) >> KPllMode[ aPll ].iModeShift) bitand KPllModeMask;
	TUint32 autoSet = (AsspRegister::Read32( KPllMode[ aPll ].iAutoRegister ) >> KPllMode[ aPll ].iAutoShift) bitand KPllAutoMask;

	static const TPllMode modeTable[8][2] =
		{	// auto disabled	auto enabled
			{ EPllStop,			EPllStop },	// not possible
			{ EPllStop,			EPllStop },
			{ EPllStop,			EPllStop },	// not possible
			{ EPllStop,			EPllStop },	// not possible
			{ EPllStop,			EPllStop },	// not possible
			{ EPllBypass,		EPllBypass },
			{ EPllFastRelock,	EPllAuto },
			{ EPllRun,			EPllAuto },
		};
	return modeTable[ mode ][ (KPllAutoOff == autoSet) ? 0 : 1 ];
	}

EXPORT_C void CalcPllFrequencyRange( TPll aPll, TPllConfiguration& aConfig )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::CalcPllFrequencyRange(%x)", aPll ) );

	struct TFreqSelRange
		{
		TUint	iMin;
		TUint	iMax;
		TPllFrequencyRange	iSetting;
		};

	const TFreqSelRange KRanges[] =
		{
			{ 750000,	1000000,	EPllRange_075_100 },
			{ 1000001,	1250000,	EPllRange_100_125 },
			{ 1250001,	1500000,	EPllRange_125_150 },
			{ 1500001,	1750000,	EPllRange_150_175 },
			{ 1750001,	2100000,	EPllRange_175_210 },
			{ 7500000,	10000000,	EPllRange_750_1000 },
			{ 10000001,	12500000,	EPllRange_1000_1250 },
			{ 12500001,	15000000,	EPllRange_1250_1500 },
			{ 15000001,	17500000,	EPllRange_1500_1750 },
			{ 17500001,	21000000,	EPllRange_1750_2100 },
			{ 0,		0,			EPllRange_1750_2100	}
		};

	// We have to work out the internal frequency from the source clock frequency and the
	// divider factor N

	const TUint32 divider =	aConfig.iDivider;

	TInt found = -1;

	if( divider > 0 )
		{
		TUint fInternal = ClockFrequency( EClkSysClk ) / divider;

		// Find an appropriate range
		for( TInt i = 0; KRanges[i].iMax > 0; ++i )
			{
			if( fInternal < KRanges[i].iMin )
				{
				// We've passed all possible ranges, work out whether current or previous is nearest
				__DEBUG_ONLY( Panic( EPllInternalFrequencyOutOfRange ) );

				if( i > 0 )
					{
					// How near are we to minimum of current range?
					TUint currentDiff = KRanges[i].iMin - fInternal;

					// How near are we to maximum of previous range?
					TUint prevDiff = fInternal - KRanges[i - 1].iMax;

					found = (prevDiff < currentDiff) ? i - 1 : i;
					}
				else
					{
					// it's below minimum, so use minimum range
					found = 0;
					}
				break;
				}
			else if( (KRanges[i].iMin <= fInternal) && (KRanges[i].iMax >= fInternal) )
				{
				found = i;
				break;
				}
			}

		}
	// If we've fallen off end of list, use maximum setting
	__ASSERT_DEBUG( found >= 0, Panic( EPllInternalFrequencyOutOfRange ) );
	aConfig.iFreqRange = (found >= 0) ? KRanges[ found ].iSetting : EPllRange_1750_2100;
	}


EXPORT_C void AutoSetPllLpMode( TPll aPll )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::PllMode(%x)", aPll ) );
	__ASSERT_DEBUG( (TUint)aPll <= EDpll5, Panic( ESetPllModeBadClock ) );

	const TUint32 reg = KPllControlInfo[ aPll ].iConfigRegister;
	const TUint shift = KPllControlInfo[ aPll ].iLpShift;

	TUint freq = ClockFrequency( KPllToClock[ aPll ] );
	TUint32 clear = 1 << shift;
	TUint32 set = 0;
	if( freq <= KPllLpModeMaximumFrequency )
		{
		// LP mode can be enabled
		set = clear;
		clear = 0;
		}
	_LockedBitClearSet( reg, clear, set );
	}

EXPORT_C TBool PllIsLocked( TPll aPll )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::PllIsLocked(%x)", aPll ) );
	__ASSERT_DEBUG( (TUint)aPll <= EDpll5, Panic( EPllIsLockedBadPll ) );

	TUint32 reg = KPllControlInfo[ aPll ].iStatusRegister;
	TUint32 lockMask = 1 << KPllControlInfo[ aPll ].iLockBit;

	return ( 0 != (AsspRegister::Read32( reg ) bitand lockMask) );
	}

EXPORT_C void WaitForPllLock( TPll aPll )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::WaitForPllLock(%x)", aPll ) );
	__ASSERT_DEBUG( (TUint)aPll <= EDpll5, Panic( EWaitForPllLockBadPll ) );

	TUint32 reg = KPllControlInfo[ aPll ].iStatusRegister;
	TUint32 lockMask = 1 << KPllControlInfo[ aPll ].iLockBit;

	while( 0 == (AsspRegister::Read32( reg ) bitand lockMask) );
	}

EXPORT_C void SetPllBypassDivider( TPll aPll, TBypassDivider aDivider )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetPllBypassDivider(%x;%x)", aPll, aDivider ) );
	__ASSERT_DEBUG( (TUint)aPll <= EDpll5, Panic( ESetPllBypassDividerBadPll ) );
	__ASSERT_DEBUG( (TUint)aDivider <= EBypassDiv4, Panic( ESetPllBypassDividerBadDivider ) );

	static const TUint8 KLookupTable[] =
		{
		1,	// EBypassDiv1
		2,	// EBypassDiv2
		4.	// EBypassDiv4
		};

	TUint32 div = KLookupTable[ aDivider ];

	switch( aPll )
		{
		case EDpll1:
			_LockedBitClearSet( KCM_CLKSEL1_PLL_MPU, KBit19 | KBit20 | KBit21, div << 19 );
			break;

		case EDpll2:
			_LockedBitClearSet( KCM_CLKSEL1_PLL_IVA2, KBit19 | KBit20 | KBit21, div << 19 );
			break;

		default:
			break;
		}
	}

EXPORT_C TBypassDivider PllBypassDivider( TPll aPll )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::PllBypassDivider(%x)", aPll ) );
	__ASSERT_DEBUG( (TUint)aPll <= EDpll5, Panic( EPllBypassDividerBadPll ) );

	TUint div = 1;

	switch( aPll )
		{
		case EDpll1:
			div = (AsspRegister::Read32( KCM_CLKSEL1_PLL_MPU ) >> 19) bitand 0x7;
			break;

		case EDpll2:
			div = (AsspRegister::Read32( KCM_CLKSEL1_PLL_IVA2 ) >> 19) bitand 0x7;
			break;

		default:
			break;
		}

	TBypassDivider result = EBypassDiv1;

	if( 2 == div )
		{
		result = EBypassDiv2;
		}
	else if( 4 == div )
		{
		result = EBypassDiv4;
		}

	return result;
	}

EXPORT_C void SetDivider( TClock aClock, TUint aDivide )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetDivider(%x;%x)", aClock, aDivide ) );

	__ASSERT_DEBUG( (TUint)aClock < KSupportedClockCount, Panic( ESetDividerBadClock ) );

	const TDividerInfo&	inf = KDividerInfo[ aClock ];

	TUint32 div = aDivide;	// most common case, special cases handled below

	switch( inf.iDivType )
		{
		case EDivUsimClk:
			// Special case, not suppored by this function - use SetUsimClockDivider()
			return;

		default:
		case EDivNotSupported:
			Panic( ESetDividerUnsupportedClock );
			return;

		case EDiv_1_2:
			if( (1 != aDivide ) && (2 != aDivide ) )
				{
				__DEBUG_ONLY( Panic( ESetDividerBadDivider ) );
				return;
				}
			break;

		case EDivCore_1_2_4:
			if( (1 != aDivide ) && (2 != aDivide ) && (3 != aDivide) )
				{
				__DEBUG_ONLY( Panic( ESetDividerBadDivider ) );
				return;
				}
			break;

		case EDivCore_3_4_6_96M:
			{
			switch( aDivide )
				{
				default:
					__DEBUG_ONLY( Panic( ESetDividerBadDivider ) );
					return;

				case 3:
					div = 0;
					break;

				case 4:
					div = 1;
					break;

				case 6:
					div = 2;
					break;

				case 0:
					// Special-case, use 96MHz clock
					div = 3;
					break;
				}
			break;
			}

		case EDivPll_1_To_16:
			if( (aDivide < 1) || (aDivide > 16) )
				{
				__DEBUG_ONLY( Panic( ESetDividerBadDivider ) );
				return;
				}
			break;

		case EDivPll_1_To_31:
			if( (aDivide < 1) || (aDivide > 16) )
				{
				__DEBUG_ONLY( Panic( ESetDividerBadDivider ) );
				return;
				}
			break;



		case EDivClkOut_1_2_4_8_16:
			{
			switch( aDivide )
				{
				default:
					__DEBUG_ONLY( Panic( ESetDividerBadDivider ) );
					return;

				case 1:
					div = 0;
					break;

				case 2:
					div = 1;
					break;

				case 4:
					div = 2;
					break;

				case 8:
					div = 3;
					break;

				case 16:
					div = 4;
					break;
				}
			break;
			}
		}

	// if we get here, we have a valid divider value

	_LockedBitClearSet( inf.iRegister, inf.iMask, div << inf.iShift );
	}

EXPORT_C TUint Divider( TClock aClock )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::Divider(%x)", aClock ) );

	__ASSERT_DEBUG( (TUint)aClock < KSupportedClockCount, Panic( EGetDividerBadClock ) );

	const TDividerInfo&	inf = KDividerInfo[ aClock ];

	TUint32 div = ( AsspRegister::Read32( inf.iRegister ) bitand inf.iMask ) >> inf.iShift;
	TUint result = div;	// most common case

	switch( inf.iDivType )
		{
		case EDivUsimClk:
			return UsimDivider();

		default:
		case EDivNotSupported:
			Panic( ESetDividerUnsupportedClock );
			return 0xFFFFFFFF;

		// These are all the standard case, where value in register is divide factor
		case EDiv_1_2:
		case EDivCore_1_2_4:
		case EDivPll_1_To_16:
		case EDivPll_1_To_31:
			break;

		case EDivCore_3_4_6_96M:
			{
			switch( div )
				{
				default:
					// hardware value has unknown meaning
					result = 0xFFFFFFFF;

				case 0:
					result = 3;
					break;

				case 1:
					result = 4;
					break;

				case 2:
					result = 6;
					break;

				case 3:
					result = 0;
					break;
				}
			break;
			}

		case EDivClkOut_1_2_4_8_16:
			{
			switch( div )
				{
				default:
					// hardware value has unknown meaning
					result = 0xFFFFFFFF;

				case 0:
					result = 1;
					break;

				case 1:
					result = 2;
					break;

				case 2:
					result = 4;
					break;

				case 3:
					result = 8;
					break;

				case 4:
					result = 16;
					break;
				}
			break;
			}
		}

	return result;
	}

EXPORT_C void SetPowerDomainMode( TPowerDomain aDomain, TPowerDomainMode aMode )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetPowerDomainMode(%x;%x)", aDomain, aMode ) );
	__ASSERT_DEBUG( (TUint)aDomain < KSupportedPowerDomainCount, Panic( ESetDomainModeBadDomain ) );
	__ASSERT_DEBUG( (TUint)aMode <= EPowerOn, Panic( ESetDomainModeBadMode ) );

	__ASSERT_DEBUG( 0 != (KPowerDomainControl[ aDomain ].iAllowedMask bitand (1 << aMode)), Panic( ESetDomainModeUnsupportedMode ) );

	TUint shift = KPowerDomainControl[ aDomain ].iShift;

	_LockedBitClearSet( KPowerDomainControl[ aDomain ].iRegister,
						KPowerModeMask << shift,
						aMode << shift );
	}

EXPORT_C TPowerDomainMode PowerDomainMode( TPowerDomain aDomain )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::PowerDomainMode(%x)", aDomain ) );
	__ASSERT_DEBUG( (TUint)aDomain < KSupportedPowerDomainCount, Panic( EGetDomainModeBadDomain ) );

	TUint32 m = (AsspRegister::Read32( KPowerDomainControl[ aDomain ].iRegister ) >> KPowerDomainControl[ aDomain ].iShift) bitand KPowerModeMask;
	return static_cast< TPowerDomainMode >( m );
	}

EXPORT_C void SetClockState( TClock aClock, TClockState aState )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetClockState(%x;%x)", aClock, aState ) );

	__ASSERT_DEBUG( (TUint)aClock < KSupportedClockCount, Panic( ESetStateBadClock ) );

	const TClockEnableAutoInfo& def = KClockControlTable[ aClock ];

	TUint32 reg = def.iGate.iRegister;
	TUint32 mask = def.iGate.iMask;
	TUint32 autoReg = def.iAuto.iRegister;
	TUint32 autoMask = def.iAuto.iMask;

	TInt irq = __SPIN_LOCK_IRQSAVE(iLock);

	if( EClkOn == aState )
		{
		_BitClearSet( reg, mask, def.iGate.iEnablePattern );
		_BitClearSet( autoReg, autoMask, def.iAuto.iDisablePattern );
		}
	else if( EClkOff == aState )
		{
		_BitClearSet( reg, mask, def.iGate.iDisablePattern );
		_BitClearSet( autoReg, autoMask, def.iAuto.iDisablePattern );
		}
	else if( EClkAuto == aState )
		{
		_BitClearSet( autoReg, autoMask, def.iAuto.iEnablePattern );
		_BitClearSet( reg, mask, def.iGate.iEnablePattern );
		}

	__SPIN_UNLOCK_IRQRESTORE(iLock, irq);
	}

EXPORT_C TClockState ClockState( TClock aClock )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "+Prcm::ClockState(%x)", aClock ) );

	__ASSERT_DEBUG( (TUint)aClock < KSupportedClockCount, Panic( EGetStateBadClock ) );

	const TClockEnableAutoInfo& def = KClockControlTable[ aClock ];

	TUint32 reg = def.iGate.iRegister;
	TUint32 mask = def.iGate.iMask;
	TUint32 autoReg = def.iAuto.iRegister;
	TUint32 autoMask = def.iAuto.iMask;

	TUint32 enable = AsspRegister::Read32( reg ) bitand mask;
	TUint32 autoClock = AsspRegister::Read32( autoReg ) bitand autoMask;

	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::ClockState(%x):e:%x a:%x", aClock, enable, autoClock ) );

	TClockState state = EClkAuto;

	// OFF = OFF
	// ON + AUTO = AUTO
	// ON + !AUTO = ON
	if( def.iGate.iEnablePattern != enable )
		{
		state = EClkOff;
		}
	else if( def.iAuto.iEnablePattern != autoClock )
		{
		state = EClkOn;
		}

	__KTRACE_OPT( KPRCM, Kern::Printf( "-Prcm::ClockState(%x):%d", aClock, state ) );

	return state;
	}

EXPORT_C void SetWakeupMode( TClock aClock, TWakeupMode aMode )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetWakeupMode(%x;%x)", aClock, aMode ) );

	__ASSERT_DEBUG( (TUint)aClock < KSupportedClockCount, Panic( ESetWakeupBadClock ) );

	const TRegisterBitDef& def = KClockWakeupTable[ aClock ];

	TUint32 reg = def.iRegister;
	TUint32 mask = def.iMask;

	TInt irq = __SPIN_LOCK_IRQSAVE(iLock);

	if( EWakeupEnabled == aMode )
		{
		_BitClearSet( reg, mask, def.iEnablePattern );
		}
	else
		{
		_BitClearSet( reg, mask, def.iDisablePattern );
		}

	__SPIN_UNLOCK_IRQRESTORE(iLock, irq);
	}

EXPORT_C TWakeupMode WakeupMode( TClock aClock )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::WakeupMode(%x)", aClock ) );

	__ASSERT_DEBUG( (TUint)aClock < KSupportedClockCount, Panic( EGetWakeupBadClock ) );

	const TRegisterBitDef& def = KClockWakeupTable[ aClock ];

	TUint32 reg = def.iRegister;
	TUint32 mask = def.iMask;

	if( def.iEnablePattern == (AsspRegister::Read32( reg ) bitand mask) )
		{
		return EWakeupEnabled;
		}
	else
		{
		return EWakeupDisabled;
		}
	}

EXPORT_C void AddToWakeupGroup( TClock aClock, TWakeupGroup aGroup )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::AddToWakeupGroup(%x;%x)", aClock, aGroup ) );

	__ASSERT_DEBUG( (TUint)aClock < KSupportedClockCount, Panic( EAddWakeupGroupBadClock ) );
	__ASSERT_DEBUG( (TUint)aGroup < KSupportedWakeupGroupCount, Panic( EAddWakeupGroupBadGroup ) );

	const TRegisterBitDef& def = KClockWakeupGroupTable[ aClock ][ aGroup ];

	TUint32 reg = def.iRegister;
	TUint32 mask = def.iMask;

	_LockedBitClearSet( reg, mask, def.iEnablePattern );
	}

EXPORT_C void RemoveFromWakeupGroup( TClock aClock, TWakeupGroup aGroup )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::RemoveFromWakeupGroup(%x;%x)", aClock, aGroup ) );

	__ASSERT_DEBUG( (TUint)aClock < KSupportedClockCount, Panic( ERemoveWakeupGroupBadClock ) );
	__ASSERT_DEBUG( (TUint)aGroup < KSupportedWakeupGroupCount, Panic( ERemoveWakeupGroupBadGroup ) );

	const TRegisterBitDef& def = KClockWakeupGroupTable[ aClock ][ aGroup ];

	TUint32 reg = def.iRegister;
	TUint32 mask = def.iMask;

	_LockedBitClearSet( reg, mask, def.iDisablePattern );
	}

EXPORT_C TBool IsInWakeupGroup( TClock aClock, TWakeupGroup aGroup )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::IsInWakeupGroup(%x)", aClock ) );

	__ASSERT_DEBUG( (TUint)aClock < KSupportedClockCount, Panic( EGetWakeupGroupBadClock ) );
	__ASSERT_DEBUG( (TUint)aGroup < KSupportedWakeupGroupCount, Panic( EGetWakeupGroupBadGroup ) );

	const TRegisterBitDef& def = KClockWakeupGroupTable[ aClock ][ aGroup ];

	TUint32 reg = def.iRegister;
	TUint32 mask = def.iMask;

	return( def.iEnablePattern == (AsspRegister::Read32( reg ) bitand mask) );
	}


EXPORT_C void AddToWakeupDomain( TClock aClock, TWakeupDomain aDomain )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::AddToWakeupDomain(%x;%x)", aClock, aDomain ) );

	__ASSERT_DEBUG( (TUint)aClock <= (TUint)KSupportedClockCount, Panic( EAddDomainBadClock ) );
	__ASSERT_DEBUG( (TUint)aDomain <= (TUint)KSupportedWakeupDomainCount, Panic( EAddDomainBadDomain ) );

	const TWakeupDomainInfo& inf = KClockWakeupDomainTable[ aClock ];
	TUint32 mask = 1 << (TUint)inf.iBitNumber[ aDomain ];	// unsupported bit numbers will result in a mask of 0x00000000

	_LockedBitClearSet( inf.iRegister, KClearNone, mask );
	}

EXPORT_C void RemoveFromWakeupDomain( TClock aClock, TWakeupDomain aDomain )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::RemoveFromWakeupDomain(%x;%x)", aClock, aDomain ) );

	__ASSERT_DEBUG( (TUint)aClock <= (TUint)KSupportedClockCount, Panic( ERemoveDomainBadClock ) );
	__ASSERT_DEBUG( (TUint)aDomain <= (TUint)KSupportedWakeupDomainCount, Panic( ERemoveDomainBadDomain ) );

	const TWakeupDomainInfo& inf = KClockWakeupDomainTable[ aClock ];
	TUint32 mask = 1 << (TUint)inf.iBitNumber[ aDomain ];	// unsupported bit numbers will result in a mask of 0x00000000

	_LockedBitClearSet( inf.iRegister, mask, KSetNone );
	}

EXPORT_C TBool IsInWakeupDomain( TClock aClock, TWakeupDomain aDomain )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::IsInWakeupDomain(%x;%x)", aClock, aDomain ) );

	__ASSERT_DEBUG( (TUint)aClock <= (TUint)KSupportedClockCount, Panic( ECheckDomainBadClock ) );
	__ASSERT_DEBUG( (TUint)aDomain <= (TUint)KSupportedWakeupDomainCount, Panic( ECheckDomainBadDomain ) );

	const TWakeupDomainInfo& inf = KClockWakeupDomainTable[ aClock ];
	TUint32 mask = 1 << (TUint)inf.iBitNumber[ aDomain ];	// unsupported bit numbers will result in a mask of 0x00000000

	return ( 0 != (AsspRegister::Read32( inf.iRegister ) bitand mask) );
	}

EXPORT_C void SetGptClockSource( TGpt aGpt, TGptClockSource aSource )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetGptClockSource(%x;%x)", aGpt, aSource ) );

	__ASSERT_DEBUG( (TUint)aGpt <= (TUint)EGpt12, Panic( ESetGptClockBadGpt ) );


	TUint32 reg = KGptClockSourceInfo[ aGpt ].iRegister;
	TUint32 mask = KGptClockSourceInfo[ aGpt ].iMask;
	TUint32 setPattern = (EGptClockSysClk == aSource ) ? mask : 0;

	_LockedBitClearSet( reg, mask, setPattern );
	}

EXPORT_C TGptClockSource GptClockSource( TGpt aGpt )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::GptClockSource(%x)", aGpt ) );

	__ASSERT_DEBUG( (TUint)aGpt <= (TUint)EGpt12, Panic( ESetGptClockBadGpt ) );

	TUint32 reg = KGptClockSourceInfo[ aGpt ].iRegister;
	TUint32 mask = KGptClockSourceInfo[ aGpt ].iMask;

	if( 0 == (AsspRegister::Read32( reg ) bitand mask) )
		{
		return EGptClock32k;
		}
	else
		{
		return EGptClockSysClk;
		}
	}

EXPORT_C TUint UsimDivider()
	{
	const TDividerInfo& info = KDividerInfo[ EClkUsim_F ];
	TUint divmux = (AsspRegister::Read32( info.iRegister ) bitand info.iMask ) >> info.iShift;
	return UsimDivMuxInfo[ divmux ].iDivider;
	}

EXPORT_C TClock UsimClockSource()
	{
	const TDividerInfo& info = KDividerInfo[ EClkUsim_F ];
	TUint divmux = (AsspRegister::Read32( info.iRegister ) bitand info.iMask ) >> info.iShift;
	return UsimDivMuxInfo[ divmux ].iClock;
	}

EXPORT_C void SetClockMux( TClock aClock, TClock aSource )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::SetClockMux(%x;%x)", aClock, aSource ) );

	switch( aClock )
		{
		case EClk96M:
			{
			TUint set = KBit6;
			TUint clear = 0;

			switch( aSource )
				{
				case EClkPeriph:
					clear = KBit6;
					set = 0;
					// fall through...

				case EClkSysClk:
					_LockedBitClearSet( KCM_CLKSEL1_PLL, clear, set );
					break;

				default:
					Panic( ESetClockMuxBadSource );
				}
			break;
			}

		case EClkSysOut:
			{
			TUint set;
			switch( aSource )
				{
				case EClkCore:
					set = 0;
					break;

				case EClkSysClk:
					set = 1;
					break;

				case EClkPeriph:
					set = 2;
					break;

				case EClkTv_F:
					set = 3;
					break;

				default:
					Panic( ESetClockMuxBadSource );
					return;
				}

			_LockedBitClearSet( KCM_CLKOUT_CTRL, KBit1 | KBit0, set );
			break;
			}

		case EClkTv_F:
			{
			TUint set = KBit5;
			TUint clear = 0;

			switch( aSource )
				{
				case EClkPeriph:
					clear = KBit5;
					set = 0;
					// fall through...

				case EClkAltClk:
					_LockedBitClearSet( KCM_CLKSEL1_PLL, clear, set );
					break;

				default:
					Panic( ESetClockMuxBadSource );
					return;
				}
			break;
			}

		case EClkGpt1_F:
		case EClkGpt2_F:
		case EClkGpt3_F:
		case EClkGpt4_F:
		case EClkGpt5_F:
		case EClkGpt6_F:
		case EClkGpt7_F:
		case EClkGpt8_F:
		case EClkGpt9_F:
			{
			TGptClockSource src = EGptClock32k;

			switch( aSource )
				{
				case EClkSysClk:
					src = EGptClockSysClk;
				case EClkSysClk32k:
					break;
				default:
					Panic( ESetClockMuxBadSource );
					return;
				}

			SetGptClockSource( KClockSourceInfo[ aClock ].iGpt, src );
			break;
			}

		case EClkSgx_F:
			switch( aSource )
				{
				case EClk96M:
					SetDivider( EClkSgx_F, 0 );
					break;

				case EClkCore:
					// Unfortunately the combined divider/mux means that switching from
					// CORE t 96M loses the old divider values
					if( 0 != Divider( EClkSgx_F ) )
						{
						// Not currently CORE, switch to default maximum divider
						SetDivider( EClkSgx_F, 6 );
						}
					break;

				default:
					Panic( ESetClockMuxBadSource );
					return;
				}
			break;


		case EClk48M:
			{
			TUint set = KBit3;
			TUint clear = 0;

			switch( aSource )
				{
				case EClkPeriph:
					clear = KBit3;
					set = 0;
					// fall through...

				case EClkAltClk:
					_LockedBitClearSet( KCM_CLKSEL1_PLL, clear, set );
					break;

				default:
					Panic( ESetClockMuxBadSource );
					return;
				}
			break;
			}

		default:
			Panic( ESetClockMuxBadClock );
			return;
		}
	}

EXPORT_C TClock ClockMux( TClock aClock )
	{
	__KTRACE_OPT( KPRCM, Kern::Printf( "Prcm::ClockMux(%x)", aClock ) );

	TClock result;

	switch( aClock )
		{
		case EClk96M:
			if( 0 == (AsspRegister::Read32( KCM_CLKSEL1_PLL ) bitand KBit6 ) )
				{
				result = EClkPeriph;
				}
			else
				{
				result = EClkSysClk;
				}
			break;

		case EClkSysOut:
			switch( AsspRegister::Read32( KCM_CLKOUT_CTRL ) bitand (KBit1 | KBit0) )
				{
				default:
				case 0:
					result = EClkCore;
					break;

				case 1:
					result = EClkSysClk;
					break;

				case 2:
					result = EClkPeriph;
					break;

				case 3:
					result = EClkTv_F;		// same as 54MHz clock
					break;
				}
			break;

		case EClkTv_F:
			if( 0 == (AsspRegister::Read32( KCM_CLKSEL1_PLL ) bitand KBit5 ) )
				{
				result = EClkPeriph;
				}
			else
				{
				result = EClkAltClk;
				}
			break;

		case EClkGpt1_F:
		case EClkGpt2_F:
		case EClkGpt3_F:
		case EClkGpt4_F:
		case EClkGpt5_F:
		case EClkGpt6_F:
		case EClkGpt7_F:
		case EClkGpt8_F:
		case EClkGpt9_F:
		case EClkGpt10_F:
		case EClkGpt11_F:
			// Redirect these to GptClockSource()
			if( EGptClockSysClk == GptClockSource( KClockSourceInfo[ aClock ].iGpt ) )
				{
				result = EClkSysClk;
				}
			else
				{
				result = EClkSysClk32k;
				}
			break;

		case EClkSgx_F:
			if( Divider( EClkSgx_F ) == 0 )
				{
				result = EClk96M;
				}
			else
				{
				result = EClkCore;
				}
			break;

		case EClkUsim_F:
			result = UsimClockSource();
			break;

		case EClk48M:
			if( 0 == (AsspRegister::Read32( KCM_CLKSEL1_PLL ) bitand KBit3 ) )
				{
				result = EClk96M;
				}
			else
				{
				result = EClkAltClk;
				}
			break;

		default:
			Panic( EGetClockMuxBadClock );
			return EClkAltClk;	// dumy to stop compiler warning
		}

	return result;
	}

EXPORT_C TUint ClockFrequency( TClock aClock )
	{
	// Works out the frequency by traversing backwards through the clock chain
	// assumulating a multply and divide factor until SYSCLK or SYSCLK32 is reached
	// Reaching a DPLL implicitly means SYSCLK has been reached

	TUint mul = 1;
	TUint div = 1;
	TClock currentClock = aClock;
	__ASSERT_ALWAYS( currentClock < Prcm::KSupportedClockCount, Panic( EClockFrequencyBadClock ) );

	// Ensure assumption that root clock range is >=EClkSysClk
	__ASSERT_COMPILE( EClkSysClk < EClkAltClk );
	__ASSERT_COMPILE( EClkAltClk < EClkSysClk32k );
	__ASSERT_COMPILE( (TUint)EClkSysClk32k == (TUint)KSupportedClockCount - 1 );

	while( currentClock < EClkSysClk )
		{
		// Get previous clock in chain
		TClock prevClock = KClockSourceInfo[ currentClock ].iClock;

		switch( KClockSourceInfo[ currentClock ].iType )
			{
			case EIgnore:
				return 0;	// unsupported clock

			case EDpll:
				{
				TPll pll = KClockSourceInfo[ currentClock ].iPll;

				if( PllMode( pll ) == EPllBypass )
					{
					if( EDpll1 == pll )
						{
						prevClock = Prcm::EClkMpuPll_Bypass;
						}
					else if( EDpll2 == pll )
						{
						prevClock = Prcm::EClkIva2Pll_Bypass;
						}
					else
						{
						// for all other DPLL1 the bypass clock is the input clock SYSCLK
						prevClock = EClkSysClk;
						}
					}
				else
					{
					TPllConfiguration pllCfg;
					PllConfig( pll, pllCfg );
					mul *= pllCfg.iMultiplier;
					div *= pllCfg.iDivider;
					if( EDpll4 == pll )
						{
						// Output is multiplied by 2 for DPLL4
						mul *= 2;
						}
					prevClock = EClkSysClk;
					}
				break;
				}

			case EMux:
				prevClock = ClockMux( currentClock );
				break;

			case EDivMux:
				// need to find what clock the divider is fed from
				prevClock = ClockMux( currentClock );
				// fall through to get divider..

			case EDivider:
				{
				TUint selectedDiv = Divider( currentClock );
				// Special case for SGX - ignore a return of 0
				if( 0 != selectedDiv )
					{
					div *= selectedDiv;
					}
				break;
				}

			case EDuplicate:
				// Nothing to do, we just follow to the next clock
				break;

			case E48MMux:
				prevClock = ClockMux( currentClock );
				if( prevClock != EClkAltClk )
					{
					div *= 2;
					}
				break;

			case E54MMux:
				prevClock = ClockMux( currentClock );
				if( prevClock != EClkAltClk )
					{
					div *= Divider( currentClock );
					}
				break;

			case E96MMux:
				prevClock = ClockMux( currentClock );
				if( prevClock != EClkSysClk )
					{
					div *= Divider( currentClock );
					}
				break;

			case EDiv4:
				div *= 4;
				break;
			}

		currentClock = prevClock;
		}	// end do

	// When we reach here we have worked back to the origin clock

	TUint64 fSrc;
	const Omap3530Assp* variant = (Omap3530Assp*)Arch::TheAsic();

	if( EClkSysClk == currentClock )
		{
		// input OSC_SYSCLK is always divided by 2 before being fed to SYS_CLK
		fSrc = variant->SysClkFrequency() / 2;
		}
	else if( EClkSysClk32k == currentClock )
		{
		fSrc = variant->SysClk32kFrequency();
		}
	else
		{
		fSrc = variant->AltClkFrequency();
		}

	if( div == 0 )
		{
		// to account for any registers set at illegal values
		return 0;
		}
	else
		{
		return (TUint)((fSrc * mul) / div);
		}
	}

EXPORT_C void SetSysClkFrequency( TSysClkFrequency aFrequency )
	{
	static const TUint8 KConfigValues[] =
		{
		0,	// ESysClk12MHz
		1,	// ESysClk13MHz
		5,	// ESysClk16_8MHz
		2,	// ESysClk19_2MHz
		3,	// ESysClk26MHz
		4	// ESysClk38_4MHz
		};

	_LockedBitClearSet( KPRM_CLKSEL, KBit0 | KBit1 | KBit2, KConfigValues[ aFrequency ] );
	}

/** Get the currently configured SysClk frequency */
EXPORT_C TSysClkFrequency SysClkFrequency()
	{

	switch( AsspRegister::Read32( KPRM_CLKSEL ) bitand (KBit0 | KBit1 | KBit2) )
		{
		case 0:
			return ESysClk12MHz;
		case 1:
			return ESysClk13MHz;
		case 2:
			return ESysClk19_2MHz;
		case 3:
			return ESysClk26MHz;
		case 4:
			return ESysClk38_4MHz;
		case 5:
			return ESysClk16_8MHz;
		default:
			__DEBUG_ONLY( InternalPanic( __LINE__ ) );
			return ESysClk13MHz;
		}
	}


EXPORT_C const TDesC& PrmName( TClock aClock )
	{
	__ASSERT_DEBUG( (TUint)aClock <= KSupportedClockCount, Panic( EGetNameBadClock ) );
	__ASSERT_DEBUG( KNames[ aClock ] != NULL, Kern::Fault( "PrmName", aClock ) );

	return *KNames[ aClock ];
	}

EXPORT_C void Init3()
	{
	// Enable LP mode if possible on MPU and CORE PLLs.
	// Don't enable on PERIPHERAL, IVA2 or USB because LP mode introduces jitter
	AutoSetPllLpMode( EDpll1 );
	AutoSetPllLpMode( EDpll3 );

	TInt irq = __SPIN_LOCK_IRQSAVE(iLock);
	TUint32 r;

	// IVA2
//	Not yet mapped! const TUint32 KPDCCMD = Omap3530HwBase::TVirtual<0x01810000>::Value;
//	r = AsspRegister::Read32(KPDCCMD);
//	AsspRegister::Modify32(KPDCCMD, 0, 1 << 16);
//	Set(KCM_FCLKEN_IVA2, 1 << 0, 0);
	// CAM
	const TUint32 KISP_CTRL = Omap3530HwBase::TVirtual<0x480BC040>::Value;
	r = AsspRegister::Read32(KISP_CTRL);
	_BitClearSet(KISP_CTRL, 0xf << 10, 0);
	_BitClearSet(KCM_FCLKEN_CAM, 1 << 0, 0);
	_BitClearSet(KCM_ICLKEN_CAM, 1 << 0, 0);

	// MMC
	r = AsspRegister::Read32(KMMCHS1_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkMmc1_F, EClkOff );
	SetClockState( EClkMmc1_I, EClkOff );
	r = AsspRegister::Read32(KMMCHS2_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkMmc2_F, EClkOff );
	SetClockState( EClkMmc2_I, EClkOff );
/* There is no MMC3 on the beagle board
	r = AsspRegister::Read32(KMMCHS3_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
*/
	SetClockState( EClkMmc3_F, EClkOff );
	SetClockState( EClkMmc3_I, EClkOff );

	// McBSP
	r = AsspRegister::Read32(KMCBSPLP1_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkMcBsp1_F, EClkOff );
	SetClockState( EClkMcBsp1_I, EClkOff );
	const TUint32 KMCBSPLP2_SPCR1 = Omap3530HwBase::TVirtual<0x49022014>::Value;
	_BitClearSet(KMCBSPLP2_SPCR1, 1 << 0, 0); // RRST := 0
	const TUint32 KMCBSPLP2_SPCR2 = Omap3530HwBase::TVirtual<0x49022010>::Value;
	_BitClearSet(KMCBSPLP2_SPCR2, 1 << 7 | 1 << 0, 0); // FRST, XRST := 0
	_BitClearSet(KMCBSPLP2_SYSCONFIG, 0x3 << 8 | 0x3 << 3, 0); // CLOCKACTIVITY := can be switched off, SIDLEMODE := force idle
	r = AsspRegister::Read32(KMCBSPLP2_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkMcBsp2_F, EClkOff );
	SetClockState( EClkMcBsp2_I, EClkOff );
	r = AsspRegister::Read32(KMCBSPLP3_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkMcBsp3_F, EClkOff );
	SetClockState( EClkMcBsp3_I, EClkOff );
	r = AsspRegister::Read32(KMCBSPLP4_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkMcBsp4_F, EClkOff );
	SetClockState( EClkMcBsp4_I, EClkOff );
	r = AsspRegister::Read32(KMCBSPLP5_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkMcBsp5_F, EClkOff );
	SetClockState( EClkMcBsp5_I, EClkOff );

	// McSPI
	r = AsspRegister::Read32(KMCSPI1_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkMcSpi1_F, EClkOff );
	SetClockState( EClkMcSpi1_I, EClkOff );
	r = AsspRegister::Read32(KMCSPI2_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkMcSpi2_F, EClkOff );
	SetClockState( EClkMcSpi2_I, EClkOff );
	r = AsspRegister::Read32(KMCSPI3_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	/* nxz enable SPI 3
	SetClockState( EClkMcSpi3_F, EClkOff );
	SetClockState( EClkMcSpi3_I, EClkOff );*/
	SetClockState( EClkMcSpi3_F, EClkOn );
	SetClockState( EClkMcSpi3_I, EClkOn );
	r = AsspRegister::Read32(KMCSPI4_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	/* nxz enable SPI 4
	SetClockState( EClkMcSpi4_F, EClkOff );
	SetClockState( EClkMcSpi4_I, EClkOff );*/
	SetClockState( EClkMcSpi4_F, EClkOn );
	SetClockState( EClkMcSpi4_I, EClkOn );

    /* BUG 3612 - We do not want to dissable all other UARTS
	// UART
	TInt debugport = Kern::SuperPage().iDebugPort;
	if( debugport != 0 )
		{
		r = AsspRegister::Read32(KUART1_SYSC);
		__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
		SetClockState( EClkUart1_F, EClkOff );
		SetClockState( EClkUart1_I, EClkOff );
		}
	if( debugport != 1 )
		{
		r = AsspRegister::Read32(KUART2_SYSC);
		__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
		SetClockState( EClkUart2_F, EClkOff );
		SetClockState( EClkUart2_I, EClkOff );
		}
	if( debugport != 2 )
		{
		r = AsspRegister::Read32(KUART3_SYSC);
		__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
		SetClockState( EClkUart3_F, EClkOff );
		SetClockState( EClkUart3_I, EClkOff );
		}
     */

	// I2C KI2C1_SYSC
	r = AsspRegister::Read32(KI2C1_SYSC);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkI2c1_F, EClkOff );
	SetClockState( EClkI2c1_I, EClkOff );
	r = AsspRegister::Read32(KI2C2_SYSC);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkI2c2_F, EClkOff );
	SetClockState( EClkI2c2_I, EClkOff );
	r = AsspRegister::Read32(KI2C3_SYSC);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkI2c3_F, EClkOff );
	SetClockState( EClkI2c3_I, EClkOff );

	// GPT
	r = AsspRegister::Read32(KTI1OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt1_F, EClkOff );
	SetClockState( EClkGpt1_I, EClkOff );
	r = AsspRegister::Read32(KTI2OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt2_F, EClkOff );
	SetClockState( EClkGpt2_I, EClkOff );
	r = AsspRegister::Read32(KTI3OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt3_F, EClkOff );
	SetClockState( EClkGpt3_I, EClkOff );
	r = AsspRegister::Read32(KTI4OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt4_F, EClkOff );
	SetClockState( EClkGpt4_I, EClkOff );
	r = AsspRegister::Read32(KTI5OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt5_F, EClkOff );
	SetClockState( EClkGpt5_I, EClkOff );
	r = AsspRegister::Read32(KTI6OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt6_F, EClkOff );
	SetClockState( EClkGpt6_I, EClkOff );
	r = AsspRegister::Read32(KTI7OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt7_F, EClkOff );
	SetClockState( EClkGpt7_I, EClkOff );
	r = AsspRegister::Read32(KTI8OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt8_F, EClkOff );
	SetClockState( EClkGpt8_I, EClkOff );
	r = AsspRegister::Read32(KTI9OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt9_F, EClkOff );
	SetClockState( EClkGpt9_I, EClkOff );
	r = AsspRegister::Read32(KTI10OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt10_F, EClkOff );
	SetClockState( EClkGpt10_I, EClkOff );
	r = AsspRegister::Read32(KTI11OCP_CFG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkGpt11_F, EClkOff );
	SetClockState( EClkGpt11_I, EClkOff );

	// WDT
	r = AsspRegister::Read32(KWD2_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkWdt2_F, EClkOff );
	SetClockState( EClkWdt2_I, EClkOff );
	r = AsspRegister::Read32(KWD3_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	SetClockState( EClkWdt3_F, EClkOff );
	SetClockState( EClkWdt3_I, EClkOff );

	// GPIO
	/*
	r = AsspRegister::Read32(KGPIO1_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	_BitClearSet(KCM_FCLKEN_WKUP, 1 << 3, 0);
	_BitClearSet(KCM_ICLKEN_WKUP, 1 << 3, 0);

	//r = AsspRegister::Read32(KGPIO2_SYSCONFIG);
	//__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	//__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	//_BitClearSet(KCM_FCLKEN_PER, 1 << 13, 0);
	//_BitClearSet(KCM_ICLKEN_PER, 1 << 13, 0);
	r = AsspRegister::Read32(KGPIO3_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	_BitClearSet(KCM_FCLKEN_PER, 1 << 14, 0);
	_BitClearSet(KCM_ICLKEN_PER, 1 << 14, 0);
	r = AsspRegister::Read32(KGPIO4_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	_BitClearSet(KCM_FCLKEN_PER, 1 << 15, 0);
	_BitClearSet(KCM_ICLKEN_PER, 1 << 15, 0);
	r = AsspRegister::Read32(KGPIO5_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	_BitClearSet(KCM_FCLKEN_PER, 1 << 16, 0);
	_BitClearSet(KCM_ICLKEN_PER, 1 << 16, 0);
	r = AsspRegister::Read32(KGPIO6_SYSCONFIG);
	__NK_ASSERT_ALWAYS((r & 1 << 3) == 0);
	__NK_ASSERT_ALWAYS((r & 1 << 8) == 0);
	_BitClearSet(KCM_FCLKEN_PER, 1 << 17, 0);
	_BitClearSet(KCM_ICLKEN_PER, 1 << 17, 0);
	*/
	__SPIN_UNLOCK_IRQRESTORE(iLock, irq);
	}

} // end namespace Prcm


NONSHARABLE_CLASS( TPrcmInterruptDispatch ): public MInterruptDispatcher
	{
	public:
		TInt Init();
		virtual TInt Bind(TInt aId, TIsr anIsr, TAny* aPtr) ;
		virtual TInt Unbind(TInt aId);
		virtual TInt Enable(TInt aId);
		virtual TInt Disable(TInt aId);
		virtual TInt Clear(TInt aId);
		virtual TInt SetPriority(TInt aId, TInt aPriority);

	private:
		static void Spurious( TAny* aId );
		static void Dispatch( TAny* aParam );
	};

SInterruptHandler Handlers[ Prcm::KInterruptCount ];
TInt TheRootInterruptEnable = 0;
TSpinLock iIntLock(/*TSpinLock::EOrderGenericIrqLow0*/);
TPrcmInterruptDispatch TheIntDispatcher;

void TPrcmInterruptDispatch::Spurious( TAny* aId )
	{
	Kern::Fault( "PRCM:Spurious", (TInt)aId );
	}

void TPrcmInterruptDispatch::Dispatch( TAny* /*aParam*/ )
	{
	TUint32 status = AsspRegister::Read32( KPRM_IRQSTATUS_MPU )
					bitand AsspRegister::Read32( KPRM_IRQENABLE_MPU );

	for( TInt i = 0; (status) && (i < Prcm::KInterruptCount); ++i )
		{
		if( status bitand 1 )
			{
			(*Handlers[i].iIsr)( Handlers[i].iPtr );
			}
		status >>= 1;
		}
	}

TInt TPrcmInterruptDispatch::Init()
	{
	// Disable all interrupts
	AsspRegister::Write32( KPRM_IRQENABLE_MPU, 0 );
	AsspRegister::Write32( KPRM_IRQSTATUS_MPU, KSetAll );

	// Bind all to spurious handler
	for( TInt i = 0; i < Prcm::KInterruptCount; ++i )
		{
		Handlers[i].iIsr = TPrcmInterruptDispatch::Spurious;
		Handlers[i].iPtr = (TAny*)(i + (EIrqRangeBasePrcm << KIrqRangeIndexShift));
		}

	TInt r = Interrupt::Bind( EOmap3530_IRQ11_PRCM_MPU_IRQ, TPrcmInterruptDispatch::Dispatch, this );
	if( KErrNone == r )
		{
		Register( EIrqRangeBasePrcm );
		}
	return r;
	}

TInt TPrcmInterruptDispatch::Bind(TInt aId, TIsr aIsr, TAny* aPtr)
	{
	TUint id = aId bitand KIrqNumberMask;
	TInt r;

	if( id < Prcm::KInterruptCount )
		{
		if( Handlers[ id ].iIsr != TPrcmInterruptDispatch::Spurious )
			{
			r = KErrInUse;
			}
		else
			{
			Handlers[ id ].iIsr = aIsr;
			Handlers[ id ].iPtr = aPtr;
			r = KErrNone;
			}
		}
	else
		{
		r = KErrArgument;
		}
	return r;
	}

TInt TPrcmInterruptDispatch::Unbind(TInt aId)
	{
	TUint id = aId bitand KIrqNumberMask;
	TInt r;

	if( id < Prcm::KInterruptCount )
		{
		if( Handlers[ id ].iIsr == TPrcmInterruptDispatch::Spurious )
			{
			r = KErrGeneral;
			}
		else
			{
			Handlers[ id ].iIsr = TPrcmInterruptDispatch::Spurious;
			r = KErrNone;
			}
		}
	else
		{
		r = KErrArgument;
		}
	return r;
	}

TInt TPrcmInterruptDispatch::Enable(TInt aId)
	{
	TUint id = aId bitand KIrqNumberMask;

	if( id < Prcm::KInterruptCount )
		{
		TInt irq = __SPIN_LOCK_IRQSAVE(iIntLock);
		if( ++TheRootInterruptEnable == 1 )
			{
			Interrupt::Enable( EOmap3530_IRQ11_PRCM_MPU_IRQ );
			}
		Prcm::_BitClearSet( KPRM_IRQENABLE_MPU, KClearNone, 1 << id );
		__SPIN_UNLOCK_IRQRESTORE(iIntLock, irq);
		return KErrNone;
		}
	else
		{
		return KErrArgument;
		}
	}

TInt TPrcmInterruptDispatch::Disable(TInt aId)
	{
	TUint id = aId bitand KIrqNumberMask;

	if( id < Prcm::KInterruptCount )
		{
		TInt irq = __SPIN_LOCK_IRQSAVE(iIntLock);
		if( --TheRootInterruptEnable == 0 )
			{
			Interrupt::Disable( EOmap3530_IRQ11_PRCM_MPU_IRQ );
			}
		Prcm::_BitClearSet( KPRM_IRQENABLE_MPU, 1 << id, KSetNone );
		__SPIN_UNLOCK_IRQRESTORE(iIntLock, irq);
		return KErrNone;
		}
	else
		{
		return KErrArgument;
		}
	}

TInt TPrcmInterruptDispatch::Clear(TInt aId)
	{
	TUint id = aId bitand KIrqNumberMask;
	TInt r;

	if( id < Prcm::KInterruptCount )
		{
		AsspRegister::Write32( KPRM_IRQSTATUS_MPU, 1 << id );
		r = KErrNone;
		}
	else
		{
		r = KErrArgument;
		}
	return r;
	}

TInt TPrcmInterruptDispatch::SetPriority(TInt anId, TInt aPriority)
	{
	return KErrNotSupported;
	}



DECLARE_STANDARD_EXTENSION()
	{
	return TheIntDispatcher.Init();
	}





