// Copyright (c) 1998-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/assp/src/interrupts.cpp
// Template ASSP interrupt control and dispatch
//

#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>
#include <assp/omap3530_assp/locks.h>

SInterruptHandler Omap3530Interrupt::Handlers[KNumOmap3530Ints];
MInterruptDispatcher*	TheDispatchers[ KIrqRangeCount ];

#define OMAP3530_INTERRUPTS_INC

#ifndef OMAP3530_INTERRUPTS_INC
//TODO: Implement the asm IRQ & FIQ dispatcher
#include "interrupts.cia"

#else

void IrqDispatch()
{
	CHECK_PRECONDITIONS( MASK_INTERRUPTS_DISABLED  ,Kern::Fault("IRQS ENABLED While reading IVT",__FILE__)); 
	
	TUint irqVector  =  AsspRegister::Read32(INTCPS_SIR_IRQ) &INTCPS_PENDING_MASK;
	
	TUint irqFlags = __SPIN_LOCK_IRQSAVE_R(Omap3530IVTLock);
	SInterruptHandler handler = Omap3530Interrupt::Handlers[irqVector];
	__SPIN_UNLOCK_IRQRESTORE_R(Omap3530IVTLock,irqFlags);
	
	//may be useful in the interim but dont want a print in every dispatch
	//__KTRACE_OPT(KHARDWARE,Kern::Printf("IRQDISPATCH V %x isr %x ptr %x",irqVector,handler.iIsr,handler.iPtr));
	//execute handler now
	((TIsr)*handler.iIsr)(handler.iPtr);

	AsspRegister::Write32(INTCPS_CONTROL,INTCPS_CONTROL_IRQ_CLEAR);
}

void FiqDispatch()
{
	Omap3530Interrupt::Spurious((TAny*)KErrNotFound);
}

#endif

#ifndef _DEBUG

void Omap3530Interrupt::dumpINTCState(){}
void Omap3530Interrupt::dumpIVTState(){}
void TestHandler(TAny * wibble){}
EXPORT_C void TestInterrupts(TInt id,TIsr func){}
void TestExternalInterrupts(){}
EXPORT_C void ClearAndDisableTestInterrupt(TInt anId){}
void TestPriorities(){}
#else

void Omap3530Interrupt::dumpINTCState()
{
	Kern::Printf("INTCPS_SYSCONFIG %x",AsspRegister::Read32(INTCPS_SYSCONFIG));
	Kern::Printf("INTCPS_SYSSTATUS %x",AsspRegister::Read32(INTCPS_SYSSTATUS));
	Kern::Printf("INTCPS_SIR_IRQ %x",AsspRegister::Read32(INTCPS_SIR_IRQ));
	Kern::Printf("INTCPS_SIR_FIQ %x",AsspRegister::Read32(INTCPS_SIR_FIQ));
	Kern::Printf("INTCPS_CONTROL %x",AsspRegister::Read32(INTCPS_CONTROL));
	Kern::Printf("INTCPS_PROTECTION %x",AsspRegister::Read32(INTCPS_PROTECTION));
	Kern::Printf("INTCPS_IDLE %x",AsspRegister::Read32(INTCPS_IDLE));
	Kern::Printf("INTCPS_IRQ_PRIORITY %x",AsspRegister::Read32(INTCPS_IRQ_PRIORITY));
	Kern::Printf("INTCPS_FIQ_PRIORITY %x",AsspRegister::Read32(INTCPS_FIQ_PRIORITY));
	Kern::Printf("INTCPS_THRESHOLD %x",AsspRegister::Read32(INTCPS_THRESHOLD));
	Kern::Printf("INTCPS_ITR0 %x",AsspRegister::Read32(INTCPS_ITR(0)));
	Kern::Printf("INTCPS_ITR1 %x",AsspRegister::Read32(INTCPS_ITR(1)));
	Kern::Printf("INTCPS_ITR 2%x",AsspRegister::Read32(INTCPS_ITR(2)));
	Kern::Printf("INTCPS_MIR0 %x",AsspRegister::Read32(INTCPS_MIRn(0)));
	Kern::Printf("INTCPS_MIR1 %x",AsspRegister::Read32(INTCPS_MIRn(1)));
	Kern::Printf("INTCPS_MIR2 %x",AsspRegister::Read32(INTCPS_MIRn(2)));
	Kern::Printf("INTCPS_PENDING_IRQ0 %x",AsspRegister::Read32(INTCPS_PENDING_IRQ(0)));
	Kern::Printf("INTCPS_PENDING_IRQ1 %x",AsspRegister::Read32(INTCPS_PENDING_IRQ(1)));
	Kern::Printf("INTCPS_PENDING_IRQ2 %x",AsspRegister::Read32(INTCPS_PENDING_IRQ(2)));
	Kern::Printf("INTCPS_PENDING_FIQ1 %x",AsspRegister::Read32(INTCPS_PENDING_FIQ(0)));
	Kern::Printf("INTCPS_PENDING_FIQ0 %x",AsspRegister::Read32(INTCPS_PENDING_FIQ(1)));
	Kern::Printf("INTCPS_PENDING_FIQ2 %x",AsspRegister::Read32(INTCPS_PENDING_FIQ(2)));
	Kern::Printf("INTCPS_ILR0 %x",AsspRegister::Read32(INTCPS_ILRM(0)));
	Kern::Printf("INTCPS_ILR1 %x",AsspRegister::Read32(INTCPS_ILRM(1)));
	Kern::Printf("INTCPS_ILR2 %x",AsspRegister::Read32(INTCPS_ILRM(2)));
	Kern::Printf("INTCPS_ISRSET0 %x", AsspRegister::Read32(INTCPS_ISRSET(0)));
	Kern::Printf("INTCPS_ISRSET1 %x", AsspRegister::Read32(INTCPS_ISRSET(1)));
	Kern::Printf("INTCPS_ISRSET2 %x", AsspRegister::Read32(INTCPS_ISRSET(2)));
}


void Omap3530Interrupt::dumpIVTState()
{
	//NOTE NOT THREAD SAFE ! 
	TInt reg;
	TInt bit;
	
	__KTRACE_OPT(KHARDWARE,Kern::Printf("Omap3530Interrupt::dumpIVTState"));
	
	for(TInt i=0;i<KNumOmap3530Ints;i++)
	{
		GetRegisterAndBitOffset(i,reg,bit);
		TUint val = AsspRegister::Read32(INTCPS_MIRn(reg));
		TUint priVal = AsspRegister::Read32(INTCPS_ILRM(i));
		val &= (0x1 << bit ); 
		
		Kern::Printf("INT_VECT %d F %x P %x MASK %d PRIORITY %d",i,Handlers[i].iIsr,Handlers[i].iPtr,(val >> bit),priVal >> 2);
	}	
}

void TestHandler(TAny * wibble)
{
	TInt irq = (TInt)wibble;
	__KTRACE_OPT(KHARDWARE,Kern::Printf("TestHandler IN IRQ %d",irq));
	Omap3530Interrupt::dumpINTCState();
	ClearAndDisableTestInterrupt(irq);
	Omap3530Interrupt::dumpINTCState();
	__KTRACE_OPT(KHARDWARE,Kern::Printf("TestHandler Interrupts OK ",irq));
}



void TestPriHandler(TAny * wibble)
{
	TInt irq = (TInt)wibble;
	__KTRACE_OPT(KHARDWARE,Kern::Printf("TestHandler PRI IN IRQ %d",irq));
	Omap3530Interrupt::dumpINTCState();
	ClearAndDisableTestInterrupt(irq);
	Omap3530Interrupt::dumpINTCState();
	
	__KTRACE_OPT(KHARDWARE,Kern::Printf("TestHandler Interrupts OK ",irq));
}


EXPORT_C void ClearAndDisableTestInterrupt(TInt anId)
{
	TInt reg,bit;
	TInt irq = (TInt)anId;
	__KTRACE_OPT(KHARDWARE,Kern::Printf("ClearAndDisableTestInterrupt IN IRQ %d",irq));
	
	Omap3530Interrupt::GetRegisterAndBitOffset(irq,reg,bit);
	
	AsspRegister::Write32(INTCPS_ISR_CLEAR(reg),1 << bit);
	
	Interrupt::Clear(irq);
	
	Interrupt::Disable(irq);
	
	Interrupt::Unbind(irq);

	
	
}

void TestPriorities()
{
	__KTRACE_OPT(KHARDWARE,Kern::Printf("Priorities"));
	Interrupt::Bind(EOmap3530_IRQ4_MCBSP2_ST_IRQ,TestPriHandler,(TAny*)EOmap3530_IRQ4_MCBSP2_ST_IRQ);
	Interrupt::Bind(EOmap3530_IRQ5_MCBSP3_ST_IRQ,TestPriHandler,(TAny*)EOmap3530_IRQ5_MCBSP3_ST_IRQ);
	
	
	TInt r = Interrupt::SetPriority(EOmap3530_IRQ4_MCBSP2_ST_IRQ,0);
	if(r != KErrNone)
	{
		__KTRACE_OPT(KHARDWARE,Kern::Printf("%s SP1 r %d ",__FUNCTION__,r));
	}
			
	r = Interrupt::SetPriority(EOmap3530_IRQ5_MCBSP3_ST_IRQ,0);
	if(r != KErrNone)
	{
		__KTRACE_OPT(KHARDWARE,Kern::Printf("%s SP2 r %d ",__FUNCTION__,r));
	}
	
	Omap3530Interrupt::dumpINTCState();
	
	Interrupt::Enable(EOmap3530_IRQ4_MCBSP2_ST_IRQ);
	Interrupt::Enable(EOmap3530_IRQ5_MCBSP3_ST_IRQ);
	
	
	Omap3530Interrupt::dumpIVTState();
	TInt reg,bit,bit1;
	Omap3530Interrupt::GetRegisterAndBitOffset(EOmap3530_IRQ5_MCBSP3_ST_IRQ,reg,bit);
	Omap3530Interrupt::GetRegisterAndBitOffset(EOmap3530_IRQ4_MCBSP2_ST_IRQ,reg,bit1);
	
	AsspRegister::Write32(INTCPS_ISRSET(reg),((1 << bit) | ( 1 << bit1)));
}

EXPORT_C void TestInterrupts(TInt id,TIsr func)
{
	__KTRACE_OPT(KHARDWARE,Kern::Printf("TestInterrupts"));
	Interrupt::Bind(id,func,(TAny*)id);
	
	Omap3530Interrupt::dumpIVTState();
	
	Interrupt::Enable(id);
	Omap3530Interrupt::dumpIVTState();
	
	TInt reg,bit;
	Omap3530Interrupt::GetRegisterAndBitOffset(id,reg,bit);
	
	AsspRegister::Write32(INTCPS_ISRSET(reg),1 << bit);
	Omap3530Interrupt::dumpINTCState();
}



#endif


void Omap3530Interrupt::DisableAndClearAll()
{
	// Disable then clear all Hardware Interrupt sources
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Interrupt::DisableAndClearAll 0 %x 1 %x 2 %x",
			INTCPS_MIR_SETn(0),
			INTCPS_MIR_SETn(1),
			INTCPS_MIR_SETn(2)));
	
	//first we mask all vectors
	AsspRegister::Write32(INTCPS_MIR_SETn(0),INTCPS_MIR_ALL_SET);
	AsspRegister::Write32(INTCPS_MIR_SETn(1),INTCPS_MIR_ALL_SET);
	AsspRegister::Write32(INTCPS_MIR_SETn(2),INTCPS_MIR_ALL_SET);

	AsspRegister::Write32(INTCPS_ISR_CLEAR(0),0xffffffff);
	AsspRegister::Write32(INTCPS_ISR_CLEAR(1),0xffffffff);
	AsspRegister::Write32(INTCPS_ISR_CLEAR(2),0xffffffff);
	
	
	AsspRegister::Write32(INTCPS_CONTROL,INTCPS_CONTROL_IRQ_CLEAR|INTCPS_CONTROL_FIQ_CLEAR); 
		
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Interrupt::DisableAndClearAll INTCPS_SIR_IRQ %x at %x",AsspRegister::Read32(INTCPS_SIR_IRQ),INTCPS_BASE ));
	
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Interrupt::DisableAndClearAll OUT"));
}

void Omap3530Interrupt::Init1()
{
	
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Interrupt::Init1()"));
	
	//make sure everything is off first
	
	DisableAndClearAll();
	
	//Initialise the IVT to the spurious handler
	//interrupts are not enabled yet but take mutex on the IVT anyway
	
	TUint irq = __SPIN_LOCK_IRQSAVE_W(Omap3530IVTLock);
	for (TInt i=0; i<KNumOmap3530Ints; i++)
	{
		Handlers[i].iPtr=(TAny*)i;
		Handlers[i].iIsr=Spurious;
	} 
	__SPIN_UNLOCK_IRQRESTORE_W(Omap3530IVTLock,irq);
	

	Arm::SetIrqHandler((TLinAddr)IrqDispatch);
	Arm::SetFiqHandler((TLinAddr)FiqDispatch);
	
	
	//set the low power mode
	//TODO: these registers are outside the mapped addressable ranged - and may be reserved as according to 
	// part of the TRM.  So we will omit for now.
	//AsspRegister::Write32(INTC_INIT_REGISTER1,INTCPS_INIT_RG_LOW_PWR);
	//AsspRegister::Write32(INTC_INIT_REGISTER2,INTCPS_INIT_RG_LOW_PWR);
		
	//enable the clock sources ?
	
	//program the INTC to initial state.
	AsspRegister::Write32(INTCPS_SYSCONFIG,INTCPS_SYSCONFIG_AUTOIDLE);
	AsspRegister::Write32(INTCPS_IDLE,INTCPS_IDLE_TURBO);
	
	//TODO do we really want flat priorities ? 
	for(TInt i=0;i<KNumOmap3530Ints;i++)
	{
		AsspRegister::Write32(INTCPS_ILRM(i), (INTCPS_ILRM_ROUTE_IRQ | (KOmap3530DefIntPriority <<2) ));
	}
	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Interrupt::Init1() OUT "));

}

void Omap3530Interrupt::Init3()
	{

	__KTRACE_OPT(KBOOT,Kern::Printf("Omap3530Interrupt::Init3"));
		
	}

void Omap3530Interrupt::Spurious(TAny* anId)
{
	// handle an unexpected interrupt
	dumpIVTState();
	Kern::Fault("Omap3530Interrupt::Spurious",(TInt)anId);
}

EXPORT_C TInt Omap3530Interrupt::IsInterruptEnabled(TInt anId)
	{
	TInt isrBitOffset=0;
	TInt reg=0;
	Omap3530Interrupt::GetRegisterAndBitOffset(anId,reg,isrBitOffset);
				
	TUint val = AsspRegister::Read32(INTCPS_MIRn(reg));
	val &=  (~(0x1 << isrBitOffset )); 
	return (val & 0x1 << isrBitOffset) ?
		1:
	 	0;
	}

TInt Omap3530Interrupt::GetRegisterAndBitOffset(TInt anId,TInt &aReg,TInt &aOffset)
{
	if ((TUint)anId >= (TUint)KNumOmap3530Ints)
	{
		return KErrArgument;
	}
	else
	{
		aOffset  = (anId % 32);		
		aReg = (anId  / 32);
	}
	return KErrNone;
	
}
//
// The APIs below assume ther is a second level Interrupt controller located at Omap3530Assp::Variant level which handles
// interrupts generated by hardware at that level.
//

EXPORT_C TInt Interrupt::Bind(TInt aId, TIsr aIsr, TAny* aPtr)
	{
	__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Bind id=%d func=%08x ptr=%08x Var %x",aId,aIsr,aPtr,Omap3530Assp::Omap3530Assp::Variant));
	TInt r;
	// if ID indicates a chained interrupt, call Omap3530Assp::Variant...	

	TInt index = aId >> KIrqRangeIndexShift;
	if( index == 0 )
		{
		 if ((TUint)aId >= (TUint)KNumOmap3530Ints)
			{
			__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Bind id OOB %d MAX %d",aId,KNumOmap3530Ints)); 
			r = KErrArgument;
			}
		else
			{
			TUint irq = __SPIN_LOCK_IRQSAVE_R(Omap3530IVTLock);
			SInterruptHandler& h=Omap3530Interrupt::Handlers[aId];
			
			if (h.iIsr != Omap3530Interrupt::Spurious)
				{
				r=KErrInUse;
				__SPIN_UNLOCK_IRQRESTORE_R(Omap3530IVTLock,irq);
				__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Bind Cant Bind irq (IN_USE %d",aId));
				}
			else
				{	
				__SPIN_UNLOCK_IRQRESTORE_R(Omap3530IVTLock,irq);
				TUint irq = __SPIN_LOCK_IRQSAVE_W(Omap3530Interrupt::Omap3530IVTLock);
				h.iPtr=aPtr;
				h.iIsr=aIsr;
				__SPIN_UNLOCK_IRQRESTORE_W(Omap3530Interrupt::Omap3530IVTLock,irq);
				__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Bind BOUND %d",aId));
				r = KErrNone;
				}
			__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Bind OUT"));
			}
		}
	else if( index > 0 )
		{
		r = TheDispatchers[ index ]->Bind( aId, aIsr, aPtr );
		}
	else if (Omap3530Assp::Variant->IsExternalInterrupt(aId))
		{
		__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Bind extint %d",aId));
		return Omap3530Assp::Variant->InterruptBind(aId,aIsr,aPtr);
		}
	else
		{
		r = KErrArgument;
		}
	
	return r;
	}

EXPORT_C TInt Interrupt::Unbind(TInt aId)
	{
	__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Unbind id=%d",aId));
	TInt r;
	TInt index = aId >> KIrqRangeIndexShift;

	if( index == 0 )
		{
		if ((TUint)aId >= (TUint)KNumOmap3530Ints)
			{
			r=KErrArgument;
			}
		else	
			{
			TUint irq = __SPIN_LOCK_IRQSAVE_R(Omap3530IVTLock);
			SInterruptHandler& h=Omap3530Interrupt::Handlers[aId];
			__SPIN_UNLOCK_IRQRESTORE_R(Omap3530IVTLock,irq);
			
			if (h.iIsr == Omap3530Interrupt::Spurious)
				{
				r=KErrGeneral;
				}
			else
				{
				TInt isrBitOffset=0;
				TInt reg=0;
				Omap3530Interrupt::GetRegisterAndBitOffset(aId,reg,isrBitOffset);
#ifdef _DEBUG
				TUint val = AsspRegister::Read32(INTCPS_MIRn(reg));	 
				if( ! (val & (0x1 << isrBitOffset)))
					{
					__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Unbind THIS IRQ IS STILL ENABLED - update your code"));
					}
				
				TUint irqVector  =  AsspRegister::Read32(INTCPS_SIR_IRQ) &INTCPS_PENDING_MASK;
				if(irqVector == aId  )
					{		
					__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Unbind THIS IRQ IS STILL PENDING - update your code"));
					}
#endif			
				TUint irq = __SPIN_LOCK_IRQSAVE_W(Omap3530Interrupt::Omap3530INTCLock);
				h.iPtr=(TAny*)aId;
				h.iIsr=Omap3530Interrupt::Spurious;
				__SPIN_UNLOCK_IRQRESTORE_W(Omap3530Interrupt::Omap3530INTCLock,irq);
					
				//calculate the register and bit offset for this id
				//and disable the corresponding Hardware Interrupt source
				
				AsspRegister::Write32(INTCPS_MIR_SETn(reg),(1 << isrBitOffset));			
				r = KErrNone;
				}
			}
		}
	else if( index > 0 )
		{
		r = TheDispatchers[ index ]->Unbind( aId );
		}
	else if (Omap3530Assp::Variant->IsExternalInterrupt(aId))
		{
		r = Omap3530Assp::Variant->InterruptUnbind(aId);
		}
	else
		{
		r = KErrArgument;
		}

	return r;
	}

EXPORT_C TInt Interrupt::Enable(TInt aId)
{
	__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Enable id=%d",aId));
	TInt r;
	TInt index = aId >> KIrqRangeIndexShift;

	if( index == 0 )
		{
		if ((TUint)aId>=(TUint)KNumOmap3530Ints)
			{	
			r=KErrArgument;
			}
		else if (Omap3530Interrupt::Handlers[aId].iIsr==Omap3530Interrupt::Spurious)
			{
			r=KErrNotReady;
			}
		else
			{
			__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Enable %d",aId));
			// Enable the corresponding Hardware Interrupt source
			TInt isrBitOffset=0;
			TInt reg=0;
			Omap3530Interrupt::GetRegisterAndBitOffset(aId,reg,isrBitOffset);
			AsspRegister::Write32(INTCPS_MIR_CLEARn(reg),(1 << isrBitOffset));
			r = KErrNone;
			}		
		}
	else if( index > 0 )
		{
		r = TheDispatchers[ index ]->Enable( aId );
		}
	else if (Omap3530Assp::Variant->IsExternalInterrupt(aId))
		{
		r = Omap3530Assp::Variant->InterruptEnable(aId);
		}
	else 
		{
		r = KErrArgument;
		}
	return r;
	}

EXPORT_C TInt Interrupt::Disable(TInt aId)
{
	__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Disable id=%d",aId));
	TInt r;
	TInt index = aId >> KIrqRangeIndexShift;

	if( index == 0 )
		{
		if ((TUint)aId >= (TUint)KNumOmap3530Ints)
			{
			r=KErrArgument;
			}
		else
			{	
			// Disable the corresponding Hardware Interrupt source
			TInt isrBitOffset=0;
			TInt reg=0;
			Omap3530Interrupt::GetRegisterAndBitOffset(aId,reg,isrBitOffset);
			AsspRegister::Write32(INTCPS_MIR_SETn(reg),(1 << isrBitOffset));
			r = KErrNone;
			}
		}
	else if( index > 0 )
		{
		r = TheDispatchers[ index ]->Disable( aId );
		}
	else if (Omap3530Assp::Variant->IsExternalInterrupt(aId))
		{
		r = Omap3530Assp::Variant->InterruptDisable(aId);
		}
	else
		{
		r = KErrArgument;
		}

	return r;
	}

EXPORT_C TInt Interrupt::Clear(TInt aId)
	{
	//__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Clear id=%d",aId));
	TInt r = KErrUnknown;
	TInt index = aId >> KIrqRangeIndexShift;

	if( index == 0 )
		{
		if ((TUint)aId >= (TUint)KNumOmap3530Ints)
			{
			r=KErrArgument;
			}
		else
			{
			TInt curVector = AsspRegister::Read32(INTCPS_SIR_IRQ);
			// Clear the corresponding Hardware Interrupt source
			if(curVector == aId)
				{
				//TODO:  determine whether we are dealing with a FIQ or IRQ source
				// for now assuming all are IRQS
				//__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::Clear id=%d x %x",aId,curVector ));
				AsspRegister::Write32(INTCPS_CONTROL,INTCPS_CONTROL_IRQ_CLEAR);
				}
			}
		}
	else if( index > 0 )
		{
		r = TheDispatchers[ index ]->Clear( aId );
		}
	else if (Omap3530Assp::Variant->IsExternalInterrupt(aId))
		{
		r = Omap3530Assp::Variant->InterruptClear(aId);
		}
	else
		{
		r = KErrArgument;
		}

	return r;
	}

EXPORT_C TInt Interrupt::SetPriority(TInt aId, TInt aPriority)
	{
	// If Interrupt priorities are supported the dispatchers need to take this in consideration
	
	// (IrqDispatch/FiqDispatch)
	__KTRACE_OPT(KHARDWARE,Kern::Printf("Interrupt::SetPriority id=%d pri=%d",aId,aPriority));
	
	TInt r;
	TInt index = aId >> KIrqRangeIndexShift;
	
	if( index == 0 )
		{
		if ((TUint)aId >= (TUint)KNumOmap3530Ints)
			{
			r = KErrArgument;
			}
		else
			{
			TUint irq = __SPIN_LOCK_IRQSAVE_W(Omap3530Interrupt::Omap3530INTCLock);
			TUint curIRLMi = AsspRegister::Read32(INTCPS_ILRM(aId));
			curIRLMi = ( curIRLMi & 0x000000003) |  (aPriority << 0x2) ; 
			AsspRegister::Write32(INTCPS_ILRM(aId),curIRLMi);
			__SPIN_UNLOCK_IRQRESTORE_W(Omap3530Interrupt::Omap3530INTCLock,irq);
			r = KErrNone;
			}
		}
	else if( index > 0 )
		{
		r = TheDispatchers[ index ]->SetPriority( aId, aPriority );
		}
	else if (Omap3530Assp::Variant->IsExternalInterrupt(aId))
		{
		r = KErrNotSupported;
		}
	else
		{
		r = KErrArgument;
		}

	return r;
	}


EXPORT_C void MInterruptDispatcher::Register( TIrqRangeIndex aIndex )
	{
	__ASSERT_ALWAYS( TheDispatchers[ aIndex ] == NULL, Kern::Fault( "interrupts.cpp", __LINE__ ) );
	TheDispatchers[ aIndex ] = this;
	}

