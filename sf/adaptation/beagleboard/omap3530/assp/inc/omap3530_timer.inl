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
// omap3530/assp/inc/omap3530_timer.inl
//


#ifndef __OMAP3530_TIMER_INL__
#	define _OMAP3530_TIMER_INL__

/**
@file
	omap3530_timer.inl header file
This file provides timer handling for the omap3530 timers
@publishedAll
@released
*/

#	include <assp/omap3530_assp/omap3530_prcm.h>

inline void CompileTimeChecks(void) ;

namespace OMAP3530
	{

	namespace GPTimer
		{

		template <const TLinAddr bAsEaDdReSs>
		inline TOmap3530_IRQ TGPTi<bAsEaDdReSs>::Irq()
			{
			return TOmap3530_IRQ(TTimerTraits<bAsEaDdReSs>::KIrq) ;
			}
			
		template <const TLinAddr bAsEaDdReSs>
		inline void TGPTi<bAsEaDdReSs>::SelectClock()
			{
			typename TTimerTraits<bAsEaDdReSs>::TClockSelReg		reg ;
			reg.Modify(!TTimerTraits<bAsEaDdReSs>::KClockSelMask,
					   TTimerTraits<bAsEaDdReSs>::KClockSelValue) ;
			}
			
		template <const TLinAddr bAsEaDdReSs>
		inline TBool TGPTi<bAsEaDdReSs>::CanWriteTCLR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TCLR::KOn)) ;
			}
			
		template <const TLinAddr bAsEaDdReSs>
		inline TBool TGPTi<bAsEaDdReSs>::CanWriteTCRR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TCRR::KOn)) ;
			}
			
		template <const TLinAddr bAsEaDdReSs>
		inline TBool TGPTi<bAsEaDdReSs>::CanWriteTLDR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TLDR::KOn)) ;
			}
			
		template <const TLinAddr bAsEaDdReSs>
		inline TBool TGPTi<bAsEaDdReSs>::CanWriteTTGR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TTGR::KOn)) ;
			}
			
		template <const TLinAddr bAsEaDdReSs>
		inline TBool TGPTi<bAsEaDdReSs>::CanWriteTMAR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TMAR::KOn)) ;
			}
			
		template <const TLinAddr bAsEaDdReSs>
		inline void TGPTi<bAsEaDdReSs>::Reset()
			{
			iTIOCP_CFG.Write(TIOCP_CFG::T_SOFTRESET::KOn) ;
			}
			
		template <const TLinAddr bAsEaDdReSs>
		inline TBool TGPTi<bAsEaDdReSs>::ResetComplete()
			{
			return (TISTAT::T_RESETDONE::KOn & iTISTAT.Read()) ;
			}
			
		template <const TLinAddr bAsEaDdReSs>
		inline ::TBool TGPTi<bAsEaDdReSs>::WriteOutstanding()
			{
			return (iTWPS.Read()) ;
			}
			

		template <TLinAddr bAsEaDdReSs>
		inline TBool TMsSyncTimer<bAsEaDdReSs>::CanWriteTPIR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TPIR::KOn)) ;
			}
			
		template <TLinAddr bAsEaDdReSs>
		inline TBool TMsSyncTimer<bAsEaDdReSs>::CanWriteTNIR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TNIR::KOn)) ;
			}
			
		template <TLinAddr bAsEaDdReSs>
		inline TBool TMsSyncTimer<bAsEaDdReSs>::CanWriteTCVR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TCVR::KOn)) ;
			}
			
		template <TLinAddr bAsEaDdReSs>
		inline TBool TMsSyncTimer<bAsEaDdReSs>::CanWriteTOCR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TOCR::KOn)) ;
			}
			
		template <TLinAddr bAsEaDdReSs>
		inline TBool TMsSyncTimer<bAsEaDdReSs>::CanWriteTOWR()
			{
			return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TOWR::KOn)) ;
			}
			
		}	// namespace GPTimer
		
	}	// namespace OMAP3530

#endif	/* ndef __OMAP3530_TIMER_INL__ */
