// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/shared/mstick/mstick.cpp
//

#include <nk_priv.h>
#include <assp/omap3530_assp/omap3530_timer.h>
#include <assp/omap3530_assp/omap3530_prcm.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>
#include <assp/omap3530_assp/omap3530_hardware_base.h>

using namespace TexasInstruments::Omap3530 ;
using namespace TexasInstruments::Omap3530::GPTimer ;

const TUint32 KMaxIdleTicks = 0xFFFFFF;
const TUint KMinSuppressTickCount = 2;
static const Omap3530HwBase::TRegValue	KTickInterruptMask = TISR::T_OVF_IT_FLAG::KOn;


namespace Omap3
{
namespace MsTick
{

void MsTickIsr(TAny* aPtr )
	{
	// Clear interrupts
	TGpTimer1::iTISR.Write( KTickInterruptMask );
	TGpTimer1::iTOCR.Write( 0 );
	reinterpret_cast< NTimerQ* >( aPtr )->Tick();
	}


EXPORT_C TInt Start()
	{
	 __KTRACE_OPT(KBOOT,Kern::Printf("+Omap3:MsTick:Start"));

	 //	Enable GPT1 and set it to run from the 32kHz sysclk
	Prcm::SetGptClockSource( Prcm::EGpt1, Prcm::EGptClock32k );
	Prcm::SetClockState( Prcm::EClkGpt1_F, Prcm::EClkOn );
	Prcm::SetClockState( Prcm::EClkGpt1_I, Prcm::EClkAuto );

	__NK_ASSERT_DEBUG(TGpTimer1::iTISTAT.Read() == 0x1);

//	Prcm::AddToWakeupGroup( Prcm::EClkGpt1_I, Prcm::EWakeGroupMpu );

	//Set GPT1 to highest priority
	Interrupt::SetPriority(EOmap3530_IRQ37_GPT1_IRQ,KOmap3530MaxIntPriority);

	 // Bind to GPTimer 1 interrupt
	TInt r = Interrupt::Bind( TGpTimer1::Irq(), MsTickIsr, NTimerQ::TimerAddress() );

	if( KErrNone == r )
		{
		TGpTimer1::Reset();
		while ( !TGpTimer1::ResetComplete()) {}

		const TRegValue startOcp = ( TIOCP_CFG::T_AUTOIDLE::KOff
								   | TIOCP_CFG::T_SOFTRESET::KOff
								   | TIOCP_CFG::T_ENAWAKEUP::KOn
								   | TIOCP_CFG::T_IDLEMODE::KSmartIdle
								   | TIOCP_CFG::T_EMUFREE::KOff
								   | TIOCP_CFG::T_CLOCKACTIVITY::KMaintainFuncClock);
		TGpTimer1::iTIOCP_CFG.Write(startOcp) ;
		TGpTimer1::iTIOCP_CFG.Modify(TIOCP_CFG::T_SOFTRESET::KOn, KClear32);
		TGpTimer1::iTIOCP_CFG.Write(startOcp);
		
		// Enable timer to generate wakeups
		TGpTimer1::iTWER.Write(TWER::T_MAT_WUP_ENA::KOff | TWER::T_OVF_WUP_ENA::KOn | TWER::T_TCAR_WUP_ENA::KOff);

		TGpTimer1::ConfigureFor1Ms();

		TGpTimer1::iTIER.Write(TIER::T_MAT_IT_ENA::KOff | TIER::T_OVF_IT_ENA::KOn | TIER::T_TCAR_IT_ENA::KOff);
		TGpTimer1::iTISR.Write(TISR::T_MAT_IT_FLAG::KOn | TISR::T_OVF_IT_FLAG::KOn | TISR::T_TCAR_IT_FLAG::KOff);

		TGpTimer1::iTTGR.Write(1);

		TGpTimer1::iTOWR.Write( 0 );

		while (TGpTimer1::WriteOutstanding()) {}

		// Start the timer in auto-reload mode
		TGpTimer1::iTCLR.Modify(KClear32, (TCLR::T_ST::KOn | TCLR::T_AR::KOn | TCLR::T_IDLEMODE::KOverflow));

		// Ensure Timer Control Reg write is completed
		while(TGpTimer1::WriteOutstanding());

		Interrupt::Enable(TGpTimer1::Irq());
		}

	__KTRACE_OPT(KBOOT,Kern::Printf("-Omap3:MsTick:Start:%d", r ));
	return r;
	}

EXPORT_C TInt SuppressIdleTicks( TInt aMaxSleepTicks )
	{
	TUint32 targetSleepTicks = (aMaxSleepTicks >= KMaxIdleTicks) ? KMaxIdleTicks : aMaxSleepTicks; 

	if( targetSleepTicks >= KMinSuppressTickCount )
		{
		while(TGpTimer1::WriteOutstanding());

		// Mask out the next <targetSleepTicks> number of overflow events
		// Don't clear TOCR - we want to include any pending expiries that happened
		// while we were setting up into the sleep count
		TGpTimer1::iTOWR.Write( targetSleepTicks );

		// Clear any pending interrupt so we don't wake up immediately
		TGpTimer1::iTISR.Write( KTickInterruptMask );
		while(TGpTimer1::WriteOutstanding());
		}
	else
		{
		targetSleepTicks = 0;
		}

	return targetSleepTicks;
	}

EXPORT_C TInt EndIdleTickSuppression( TInt aMaxSleepTicks )
	{
	TUint actualSleepTicks = 0;

	if( aMaxSleepTicks >= KMinSuppressTickCount )
		{
		// Get counter values immediately. TCRR must be read first so we can check for
		// overflow while we are executing this code
		TUint32 tcrr = TGpTimer1::iTCRR.Read();
		TUint32 tisr = TGpTimer1::iTISR.Read() bitand KTickInterruptMask;
		TUint32 tocr = TGpTimer1::iTOCR.Read();

		TUint32 targetSleepTicks = (aMaxSleepTicks >= KMaxIdleTicks) ? KMaxIdleTicks : aMaxSleepTicks; 

		// Initial assumption is number of ticks missed == overflow count
		actualSleepTicks = tocr;

		if( tisr  )
			{
			// If maximum time has expired TOCR will be reset to zero
			// we want to handle the pending tick interrupt immediately
			// in the normal tick ISR so don't include it in the count of
			// ticks slept
			actualSleepTicks = targetSleepTicks - 1;
			}

		// Set timer back to normal mode
		// Dont' clear interrupts - we want any pending timer interrupt handled in the tick ISR
		TGpTimer1::iTOWR.Write( 0 );
		TGpTimer1::iTOCR.Write( 0 );
		while(TGpTimer1::WriteOutstanding());

		// Check whether another tick has expired while we were doing this
		if( TGpTimer1::iTCRR.Read() < tcrr )
			{
			// it's overflowed since we first checked
			if( tisr )
				{
				// if there was already a pending interrupt to be handled by the tick ISR
				// we need to include this new expiry in the sleep count
				++actualSleepTicks;
				}
			// else if there wasn't already a pending interrupt, this overflow will have generated
			// one which will be handled by the tick ISR
			}
		}
		
	return actualSleepTicks;
	}

}	// namespace MsTick
}	// namespace Omap3


DECLARE_STANDARD_EXTENSION()
	{
	return KErrNone;
	}

