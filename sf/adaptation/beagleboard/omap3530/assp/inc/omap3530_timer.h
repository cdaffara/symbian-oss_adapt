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
// /omap3530/assp/inc/omap3530_timer.h
//

#ifndef HEADER_OMAP3530_TIMER_H_INCLUDED
#	define HEADER_OMAP3530_TIMER_H_INCLUDED

/**
@file
	omap3530_timer.h header file
This file provides timer handling for the omap3530 timers
@publishedAll
@released
*/

#include <assp/omap3530_assp/omap3530_hardware_base.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>

namespace TexasInstruments
	{

	namespace Omap3530
		{


		namespace GPTimer
			{

			namespace TIOCP_CFG
				{
				/**
					0 AUTOIDLE Internal L4 interface clock gating strategy 0
					0x0: L4 interface clock is free-running.
					0x1: Automatic L4 interface clock gating strategy isapplied, based on the L4 interface activity.
				*/
				typedef TSingleBitField<0>	T_AUTOIDLE ;

				/**
					1 SOFTRESET Software reset. This bit is automatically reset by the RW 0 hardware. During reads, it always returns 0.
					0x0: Normal mode
					0x1: The module is reset.
				*/
				typedef TSingleBitField<1>	T_SOFTRESET ;

				/**
					2 ENAWAKEUP Wake-up feature global control RW 0
					0x0: No wake-up line assertion in idle mode
					0x1: Wake-up line assertion enabled in smart-idle mode
				*/
				typedef TSingleBitField<2>	T_ENAWAKEUP ;

				/**
					4:3 IDLEMODE Power management, req/ack control RW 0x0
					0x0: Force-idle. An idle request is acknowledged unconditionally.
					0x1: No-idle. An idle request is never acknowledged.
					0x2: Smart-idle. Acknowledgement to an idle request is given based on the internal activity of the module.
					0x3: Reserved. Do not use.
				*/
				class T_IDLEMODE : public TBitField<3, 2>
					{
				  public :
					enum TConstants
						{
						KForceIdle		= TConstVal<0>::KValue,
						KNoIdle			= TConstVal<1>::KValue,
						KSmartIdle		= TConstVal<2>::KValue
						} ;
					} ;

				/**
					5 EMUFREE Emulation mode RW 0
					0x0: Timer counter frozen in emulation
					0x1: Timer counter free-running in emulation
				*/
				typedef TSingleBitField<5>	T_EMUFREE ;

				/**
					9:8 CLOCKACTIVITY Clock activity during wakeup mode period: RW 0x0
					0x0: L4 interface and Functional clocks can be switched off.
					0x1: L4 interface clock is maintained during wake-up period; Functional clock can be switched off.
					0x2: L4 interface clock can be switched off; Functional clock is maintained during wake-up period.
					0x3: L4 interface and Functional clocks are maintained during wake-up period.
				*/
				class T_CLOCKACTIVITY : public TBitField<8, 2>
					{
				  public :
					enum TConstants
						{
						KMaintainNeither	= TConstVal<0>::KValue,
						KMaintainIfClock	= TConstVal<1>::KValue,
						KMaintainFuncClock	= TConstVal<2>::KValue,
						KMaintainBoth		= TConstVal<3>::KValue
						} ;
					} ;

				} ;	// namespace TIOCP_CFG

			namespace TISTAT
				{
				/**
				0 RESETDONE Internal reset monitoring R 0
				0x0: Internal module reset is ongoing.
				0x1: Reset completed
				*/
				typedef TSingleBitField<0>	T_RESETDONE ;

				} ;	// namespace TISTAT

			namespace TISR
				{
				/**
				0 MAT_IT_FLAG Pending match interrupt status RW 0
				Read 0x0: No match interrupt pending
				Write 0x0: Status unchanged
				Read 0x1: Match interrupt pending
				Write 0x1: Status bit cleared
				*/
				typedef TSingleBitField<0>	T_MAT_IT_FLAG ;

				/**
				1 OVF_IT_FLAG Pending overflow interrupt status RW 0
				Read 0x0: No overflow interrupt pending
				Write 0x0: Status unchanged
				Read 0x1: Overflow interrupt pending
				Write 0x1: Status bit cleared
				*/
				typedef TSingleBitField<1>	T_OVF_IT_FLAG ;

				/**
				2 TCAR_IT_FLAG Pending capture interrupt status RW 0
				Read 0x0: No capture interrupt event pending
				Write 0x0: Status unchanged
				Read 0x1: Capture interrupt event pending
				Write 0x1: Status bit cleared
				*/
				typedef TSingleBitField<2>	T_TCAR_IT_FLAG ;

				} ;	// namespace TISR

			namespace TIER
				{
				/**
				0 MAT_IT_ENA Enable match interrupt RW 0
				0x0: Disable match interrupt.
				0x1: Enable match interrupt.
				*/
				typedef TSingleBitField<0>	T_MAT_IT_ENA ;

				/**
				1 OVF_IT_ENA Enable overflow interrupt RW 0
				0x0: Disable overflow interrupt.
				0x1: Enable overflow interrupt.
				*/
				typedef TSingleBitField<1>	T_OVF_IT_ENA ;

				/**
				2 TCAR_IT_ENA Enable capture interrupt RW 0
				0x0: Disable capture interrupt.
				0x1: Enable capture interrupt.
				*/
				typedef TSingleBitField<2>	T_TCAR_IT_ENA ;

				} ;	// namespace TIER

			namespace TWER
				{
				/**
				0 MAT_WUP_ENA Enable match wake-up RW 0
				0x0: Disable match wake-up.
				0x1: Enable match wake-up.
				*/
				typedef TSingleBitField<0>	T_MAT_WUP_ENA ;

				/**
				1 OVF_WUP_ENA Enable overflow wake-up RW 0
				0x0: Disable overflow wake-up.
				0x1: Enable overflow wake-up.
				*/
				typedef TSingleBitField<1>	T_OVF_WUP_ENA ;

				/**
				2 TCAR_WUP_ENA Enable capture wake-up RW 0
				0x0: Disable capture wake-up.
				0x1: Enable capture wake-up.
				*/
				typedef TSingleBitField<2>	T_TCAR_WUP_ENA ;

				} ;	// namespace TWER

			namespace TCLR
				{
				/**
				0 ST Start/stop timer control RW 0
				0x0: Stop the timer
				0x1: Start the timer
				*/
				typedef TSingleBitField<0>	T_ST ;

				/**
				1 AR Autoreload mode RW 0
				0x0: One-shot mode overflow
				0x1: Autoreload mode overflow
				*/
				typedef TSingleBitField<1>	T_AR ;

				/**
				4:2 PTV Trigger output mode
				0x0: The timer counter is prescaled with the value: RW 0x0
				2(PTV+1). Example: PTV = 3, counter increases value (if started) after 16 functional clock periods.
				*/
				class T_PTV : public TBitField<2, 3>
					{
				  public :
					enum TConstants
						{
						KPS_2		= TConstVal<0>::KValue,
						KPS_4		= TConstVal<1>::KValue,
						KPS_8		= TConstVal<2>::KValue,
						KPS_16		= TConstVal<3>::KValue,
						KPS_32		= TConstVal<4>::KValue,
						KPS_64		= TConstVal<5>::KValue,
						KPS_128		= TConstVal<6>::KValue,
						KPS_256		= TConstVal<7>::KValue
						} ;
					} ;

				/**
				5 PRE Prescaler enable RW 0
				0x0: Prescaler disabled
				0x1: Prescaler enabled
				*/
				typedef TSingleBitField<5>	T_PRE ;

				/**
				6 CE Compare enable RW 0
				0x0: Compare disabled
				0x1: Compare enabled
				*/
				typedef TSingleBitField<6>	T_CE ;

				/**
				7 SCPWM Pulse-width-modulation output pin default setting when RW 0
				counter is stopped or trigger output mode is set to no trigger.
				0x0: Default value of PWM_out output: 0
				0x1: Default value of PWM_out output: 1
				*/
				typedef TSingleBitField<7>	T_SCPWM ;

				/**
				9:8 TCM Transition capture mode RW 0x0
				0x0: No capture
				0x1: Capture on rising edges of EVENT_CAPTURE pin.
				0x2: Capture on falling edges of EVENT_CAPTURE pin.
				0x3: Capture on both edges of EVENT_CAPTURE pin.
				*/
				class T_TCM : public TBitField<8, 2>
					{
				  public :
					enum TConstants
						{
						KNoCapture			= TConstVal<0>::KValue,
						KRisingEdge			= TConstVal<1>::KValue,
						KFallingEdge		= TConstVal<2>::KValue,
						KBothEdges			= TConstVal<3>::KValue
						} ;
					} ;

				/**
				11:10 TRG Trigger output mode RW 0x0
				0x0: No trigger
				0x1: Overflow trigger
				0x2: Overflow and match trigger
				0x3: Reserved
				*/
				class T_IDLEMODE : public TBitField<10, 2>
					{
				  public :
					enum TConstants
						{
						KNoTrigger				= TConstVal<0>::KValue,
						KOverflow				= TConstVal<1>::KValue,
						KOverflowAndMatch		= TConstVal<2>::KValue
						} ;
					} ;

				/**
				12 PT Pulse or toggle select bit RW 0
				0x0: Pulse modulation
				0x1: Toggle modulation
				*/
				typedef TSingleBitField<12>	T_PT ;

				/**
				13 CAPT_MODE Capture mode select bit (first/second) RW 0
				0x0: Capture the first enabled capture event in TCAR1.
				0x1: Capture the second enabled capture event in TCAR2.
				*/
				typedef TSingleBitField<13>	T_CAPT_MODE ;

				/**
				14 GPO_CFG PWM output/event detection input pin direction control: RW 0
				0x0: Configures the pin as an output (needed when PWM mode is required)
				0x1: Configures the pin as an input (needed when capture mode is required)
				*/
				typedef TSingleBitField<14>	T_GPO_CFG ;

				} ;	// namespace TCLR

			namespace TWPS
				{
				/**
				0 W_PEND_TCLR Write pending for register GPT_TCLR R 0
				0x0: Control register write not pending
				0x1: Control register write pending
				*/
				typedef TSingleBitField<0>	T_W_PEND_TCLR ;

				/**
				1 W_PEND_TCRR Write pending for register GPT_TCRR R 0
				0x0: Counter register write not pending
				0x1: Counter register write pending
				*/
				typedef TSingleBitField<1>	T_W_PEND_TCRR ;

				/**
				2 W_PEND_TLDR Write pending for register GPT_TLDR R 0
				0x0: Load register write not pending
				0x1: Load register write pending
				*/
				typedef TSingleBitField<2>	T_W_PEND_TLDR ;

				/**
				3 W_PEND_TTGR Write pending for register GPT_TTGR R 0
				0x0: Trigger register write not pending
				0x1: Trigger register write pending
				*/
				typedef TSingleBitField<3>	T_W_PEND_TTGR ;

				/**
				4 W_PEND_TMAR Write pending for register GPT_TMAR R 0
				0x0: Match register write not pending
				0x1: Match register write pending
				*/
				typedef TSingleBitField<4>	T_W_PEND_TMAR;

				/**
				5 W_PEND_TPIR Write pending for register GPT_TPIR R 0
				Reserved for instances 3, 4, 5, 6, 7, 8, 9, 11, 12 Read returns reset value. R 0
				0x0: Positive increment register write not pending
				0x1: Positive increment register write pending
				*/
				typedef TSingleBitField<5>	T_W_PEND_TPIR ;

				/**
				6 W_PEND_TNIR Write pending for register GPT_TNIR R 0
				0x0: Negative increment register write not pending
				0x1: Negative increment register write pending
				Reserved for instances 3, 4, 5, 6, 7, 8, 9, 11, 12 Read returns reset value. R 0
				*/
				typedef TSingleBitField<6>	T_W_PEND_TNIR ;

				/**
				7 W_PEND_TCVR Write pending for register GPT_TCVR R 0
				0x0: Counter value register write not pending
				0x1: Counter value register write pending
				Reserved for instances 3, 4, 5, 6, 7, 8, 9, 11, 12 Read returns reset value. R 0
				*/
				typedef TSingleBitField<7>	T_W_PEND_TCVR ;

				/**
				8 W_PEND_TOCR Write pending for register GPT_TOCR R 0
				0x0: Overflow counter register write not pending
				0x1: Overflow counter register write pending
				Reserved for instances 3, 4, 5, 6, 7, 8, 9, 11, 12 Read returns reset value. R 0
				*/
				typedef TSingleBitField<8>	T_W_PEND_TOCR ;

				/**
				9 W_PEND_TOWR Write pending for register GPT_TOWR R 0
				0x0: Overflow wrapping register write not pending
				0x1: Overflow wrapping register write pending
				Reserved for instances 3, 4, 5, 6, 7, 8, 9, 11, 12 Read returns reset value. R 0
				*/
				typedef TSingleBitField<9>	T_W_PEND_TOWR ;

				} ;	// namespace TWPS

			namespace TSICR
				{
				/**
				1 SFT Reset software functional registers. This bit is automatically reset RW 0
				by the hardware. During reads, it always returns 0.
				0x0: Normal functional mode
				0x1: The functional registers are reset.
				*/
				typedef TSingleBitField<1>	T_SFT ;

				/**
				2 POSTED Posted mode selection RW 1
				0x0: Non-posted mode selected
				0x1: Posted mode selected
				*/
				typedef TSingleBitField<2>	T_POSTED ;

				} ;	// namespace TSICR

			namespace TOCR
				{
				/**
				23:0 OVF_COUNTER_VALUE The number of overflow events. RW 0x00000000
				*/
				class T_OVF_COUNTER_VALUE : public TBitField<0, 24>
					{
				  public :
					enum TConstants
						{
						} ;
					} ;

				} ;	// namespace TOCR

			namespace TOWR
				{
				/**
				23:0 OVF_WRAPPING_VALUE The number of masked interrupts. RW 0x00000000
				*/
				class T_OVF_WRAPPING_VALUE : public TBitField<0, 24>
					{
				  public :
					enum TConstants
						{
						} ;
					} ;

				} ;	// namespace TOWR

			enum TBaseAddress
				{
				KGPTIMER1_Base			= TVirtual<0x48318000>::Value,
				KGPTIMER2_Base			= TVirtual<0x49032000>::Value,
				KGPTIMER3_Base			= TVirtual<0x49034000>::Value,
				KGPTIMER4_Base			= TVirtual<0x49036000>::Value,
				KGPTIMER5_Base			= TVirtual<0x49038000>::Value,
				KGPTIMER6_Base			= TVirtual<0x4903A000>::Value,
				KGPTIMER7_Base			= TVirtual<0x4903C000>::Value,
				KGPTIMER8_Base			= TVirtual<0x4903E000>::Value,
				KGPTIMER9_Base			= TVirtual<0x49040000>::Value,
				KGPTIMER10_Base			= TVirtual<0x48086000>::Value,
				KGPTIMER11_Base			= TVirtual<0x48088000>::Value,
				KGPTIMER12_Base			= TVirtual<0x48304000>::Value,
				} ;

			enum TTimerNumber
				{
				EGpTimer1,
				EGpTimer2,
				EGpTimer3,
				EGpTimer4,
				EGpTimer5,
				EGpTimer6,
				EGpTimer7,
				EGpTimer8,
				EGpTimer9,
				EGpTimer10,
				EGpTimer11,
				EGpTimer12
				};

			typedef void (*TTimerIsr)(TAny*) ;

			template<const TTimerNumber tImEr>
			struct TTimerTraits
				{
				} ;

			template<>
			struct TTimerTraits<EGpTimer1>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER1_Base,
					KIrq			= EOmap3530_IRQ37_GPT1_IRQ,
					KClockSelMask	= TSingleBitField<7>::KMask,
					KClockSelValue	= TSingleBitField<7>::KOn,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer2>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER2_Base,
					KIrq	= EOmap3530_IRQ38_GPT2_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer3>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER3_Base,
					KIrq	= EOmap3530_IRQ39_GPT3_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer4>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER4_Base,
					KIrq	= EOmap3530_IRQ40_GPT4_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer5>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER5_Base,
					KIrq	= EOmap3530_IRQ41_GPT5_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer6>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER6_Base,
					KIrq	= EOmap3530_IRQ42_GPT6_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer7>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER7_Base,
					KIrq	= EOmap3530_IRQ43_GPT7_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer8>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER8_Base,
					KIrq	= EOmap3530_IRQ44_GPT8_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer9>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER9_Base,
					KIrq			= EOmap3530_IRQ45_GPT9_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer10>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER10_Base,
					KIrq	= EOmap3530_IRQ46_GPT10_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer11>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER11_Base,
					KIrq			= EOmap3530_IRQ47_GPT11_IRQ,
					} ;
				} ;

			template<>
			struct TTimerTraits<EGpTimer12>
				{
				enum	TraitValues
					{
					KBaseAddress	= KGPTIMER12_Base,
					KIrq			= EOmap3530_IRQ95_GPT12_IRQ,
					KClockSelReg	= 0,
					KClockSelMask	= 0,
					KClockSel32K	= 0,
					KClockSelSys	= 0,
					KClockSelValue	= KClockSel32K
					} ;
				} ;

			/**
			An interface template for OMAP3530 General Purpose timer functionality.
			*/
			template <const TTimerNumber tImEr>
			class TGPT
				{
			  protected :
				enum TRegisterOffsets
					{
					KTIOCP_CFG_Offset	= 0x010,
					KTISTAT_Offset		= 0x014,
					KTISR_Offset		= 0x018,
					KTIER_Offset		= 0x01C,
					KTWER_Offset		= 0x020,
					KTCLR_Offset		= 0x024,
					KTCRR_Offset		= 0x028,
					KTLDR_Offset		= 0x02C,
					KTTGR_Offset		= 0x030,
					KTWPS_Offset		= 0x034,
					KTMAR_Offset		= 0x038,
					KTCAR1_Offset		= 0x03C,
					KTSICR_Offset		= 0x040,
					KTCAR2_Offset		= 0x044
					} ;
				enum TConstants
					{
					KHz						= 1000,
					KClockInputFrequency	= 32768,
					} ;

			  public :
				static inline TOmap3530_IRQ Irq()
					{
					return TOmap3530_IRQ(TTimerTraits<tImEr>::KIrq) ;
					}
				static inline TBool CanWriteTCLR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TCLR::KOn)) ;
					}
				static inline TBool CanWriteTCRR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TCRR::KOn)) ;
					}
				static inline TBool CanWriteTLDR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TLDR::KOn)) ;
					}
				static inline TBool CanWriteTTGR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TTGR::KOn)) ;
					}
				static inline TBool CanWriteTMAR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TMAR::KOn)) ;
					}
				static inline void Reset()
					{
					iTIOCP_CFG.Write(TIOCP_CFG::T_SOFTRESET::KOn) ;
					}
				static inline TBool ResetComplete()
					{
					return (TISTAT::T_RESETDONE::KOn & iTISTAT.Read()) ;
					}
				static inline TBool WriteOutstanding()
					{
					return (iTWPS.Read()) ;
					}

			  public :
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTIOCP_CFG_Offset)>	iTIOCP_CFG ;
				static TReg32_R<(TTimerTraits<tImEr>::KBaseAddress  + KTISTAT_Offset)>		iTISTAT ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTISR_Offset)>		iTISR ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTIER_Offset)>		iTIER ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTWER_Offset)>		iTWER ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTCLR_Offset)>		iTCLR ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTCRR_Offset)>		iTCRR ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTLDR_Offset)>		iTLDR ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTTGR_Offset)>		iTTGR ;
				static TReg32_R<(TTimerTraits<tImEr>::KBaseAddress  + KTWPS_Offset)>		iTWPS ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTMAR_Offset)>		iTMAR ;
				static TReg32_R<(TTimerTraits<tImEr>::KBaseAddress  + KTCAR1_Offset)>		iTCAR1 ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTSICR_Offset)>		iTSICR ;
				static TReg32_R<(TTimerTraits<tImEr>::KBaseAddress  + KTCAR2_Offset)>		iTCAR2 ;
				} ;	// class TGPTi

			/**

			An interface template for OMAP3530 Microsecond aligned timer functionality.
			Encapsulates the extra registers provided for timers 1, 2 and 10.
			*/
			template <const TTimerNumber tImEr>
			class TMsSyncTimer : public TGPT<tImEr>
				{
				using TGPT<tImEr>::iTWPS ;
				using TGPT<tImEr>::iTLDR ;
			  
			protected :
				enum TRegisterOffsets
					{
					KTPIR_Offset		= 0x048,
					KTNIR_Offset		= 0x04C,
					KTCVR_Offset		= 0x050,
					KTOCR_Offset		= 0x054,
					KTOWR_Offset		= 0x058
					} ;

			  public :
				enum TRegisterValues
					{
					KInitialLoad	= 0xFFFFFFE0,
					KInitialPIR		= 0x38A40,
					KInitialNIR		= 0xFFF44800
					} ;

				static inline TBool CanWriteTPIR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TPIR::KOn)) ;
					}
				static inline TBool CanWriteTNIR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TNIR::KOn)) ;
					}
				static inline TBool CanWriteTCVR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TCVR::KOn)) ;
					}
				static inline TBool CanWriteTOCR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TOCR::KOn)) ;
					}
				static inline TBool CanWriteTOWR()
					{
					return (0 == (iTWPS.Read() & TWPS::T_W_PEND_TOWR::KOn)) ;
					}

				static inline void ConfigureFor1Ms()
					{
					iTLDR.Write( KInitialLoad );
					iTPIR.Write( KInitialPIR );
					iTNIR.Write( KInitialNIR );
					}

			  public :
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTPIR_Offset)>		iTPIR ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTNIR_Offset)>		iTNIR ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTCVR_Offset)>		iTCVR ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTOCR_Offset)>		iTOCR ;
				static TReg32_RW<(TTimerTraits<tImEr>::KBaseAddress + KTOWR_Offset)>		iTOWR ;
				} ;	// class TMsSyncTimer


			}	// namespage GPTimer

		typedef GPTimer::TMsSyncTimer<GPTimer::EGpTimer1>		TGpTimer1 ;
		typedef GPTimer::TMsSyncTimer<GPTimer::EGpTimer2>		TGpTimer2 ;
		typedef GPTimer::TGPT<GPTimer::EGpTimer3>				TGpTimer3 ;
		typedef GPTimer::TGPT<GPTimer::EGpTimer4>				TGpTimer4 ;
		typedef GPTimer::TGPT<GPTimer::EGpTimer5>				TGpTimer5 ;
		typedef GPTimer::TGPT<GPTimer::EGpTimer6>				TGpTimer6 ;
		typedef GPTimer::TGPT<GPTimer::EGpTimer7>				TGpTimer7 ;
		typedef GPTimer::TGPT<GPTimer::EGpTimer8>				TGpTimer8 ;
		typedef GPTimer::TGPT<GPTimer::EGpTimer9>				TGpTimer9 ;
		typedef GPTimer::TMsSyncTimer<GPTimer::EGpTimer10>		TGpTimer10 ;
		typedef GPTimer::TGPT<GPTimer::EGpTimer11>				TGpTimer11 ;
		typedef GPTimer::TGPT<GPTimer::EGpTimer12>				TGpTimer12 ;


		/**
		An interface template for OMAP3530 32-KHz aligned timer functionality.
		*/
		class T32KhzSyncTimer
			{
		  protected :
			enum TRegisterAddress
				{
				KREG_32KSYNCNT_SYSCONFIG	= TVirtual<0x48320004>::Value,
				KREG_32KSYNCNT_CR			= TVirtual<0x48320010>::Value
				} ;

		  public :
			static TReg32_RW<KREG_32KSYNCNT_SYSCONFIG>		iSysConfig ;
			static TReg32_R<KREG_32KSYNCNT_CR>				iCR ;

		  private :
			} ;	// class TMsSyncTimer

		}	// namespace Omap3530

	}	// namespace TexasInstruments


// **** TEST CODE ****
//#	define HEADER_OMAP3530_TIMER_H_DO_COMPILE_TIME_CHECK_TESTS 1
#	ifdef HEADER_OMAP3530_TIMER_H_DO_COMPILE_TIME_CHECK_TESTS
		inline void CompileTimeChecks(void)
			{
			__ASSERT_COMPILE((TI::Omap3530::GPTimer::TIOCP_CFG::T_IDLEMODE::KSmartIdle == (2 << 3))) ;
			__ASSERT_COMPILE((TI::Omap3530::GPTimer::TIOCP_CFG::T_CLOCKACTIVITY::KMaintainIfClock == (1 << 8))) ;
			__ASSERT_COMPILE((TI::Omap3530::GPTimer::KGPTIMER1_Base == (0xC6318000))) ;
			__ASSERT_COMPILE((0)) ;		// Prove that testing is happening
			}
#	endif
#endif	/* ndef HEADER_OMAP3530_TIMER_H_INCLUDED */
