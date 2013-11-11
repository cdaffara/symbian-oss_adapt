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
* naviengine_assp\interrupts.cpp
* NE1_TBVariant ASSP interrupt control and dispatch
*
*/



#include <naviengine_priv.h>

const TUint8 IntIsEdge[96] =
	{
	0, 1, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 1,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 1,		1, 0, 0, 0,		1, 1, 1, 1,
	1, 1, 1, 1,		1, 1, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 1, 1, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0
	};

#ifdef __SMP__
#include <arm_gic.h>

#ifndef __STANDALONE_NANOKERNEL__
#undef	DEBUGPRINT
#define	DEBUGPRINT	Kern::Printf
#endif

TUint32 NaviEngineInterrupt::IrqDispatch(TUint32 aVector)
	{
	if (aVector<32 || aVector>127)
		{
		GicCpuIfc& g = *(GicCpuIfc*)KHwBaseIntIf;
		g.iEoi = aVector;
		*(TInt*)0xdeaddead = 0;
		return aVector;
		}
	NKern::Interrupt(aVector - 32);
	return aVector;
	}

void NaviEngineInterrupt::DisableAndClearAll()
	{
	// NKern does all the GIC stuff
	}

void NaviEngineInterrupt::Init1()
	{
	__KTRACE_OPT(KBOOT, DEBUGPRINT(">Interrupt_Init1()"));
	DisableAndClearAll();
	Arm::SetIrqHandler((TLinAddr)&IrqDispatch);
	Arm::SetFiqHandler((TLinAddr)&FiqDispatch);
	TInt i;
	for (i=32; i<128; ++i)
		{
		TBool edge = IntIsEdge[i-32];
		TUint32 flags = 0;
		if (i>=36 && i<42)
			flags |= NKern::EIrqInit_Count;	// timers count all interrupts
		if (edge)
			flags |= NKern::EIrqInit_RisingEdge;
		else
			flags |= NKern::EIrqInit_LevelHigh;
		TInt r = NKern::InterruptInit(i-32, flags, i, i);
		__KTRACE_OPT(KBOOT, DEBUGPRINT("InterruptInit %d(%02x) -> %d", i-32, i, r));
		__NK_ASSERT_ALWAYS(r==KErrNone);
		}

	__KTRACE_OPT(KBOOT, DEBUGPRINT("<Interrupt_Init1()"));
	}

//extern "C" void Interrupt_Init2AP()
//	{
//	}

void NaviEngineInterrupt::Init3()
	{
	//
	// Any further initialisation of the Hardware Interrupt Controller
	// Note This is not called at the moment. Should be deleted.
	}

EXPORT_C TInt Interrupt::Bind(TInt aId, TIsr aIsr, TAny* aPtr)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Bind id=%d func=%08x ptr=%08x", aId, aIsr, aPtr));
	TUint id = (TUint)aId;
	if (id<32 || id>=(TUint)KNumNaviEngineInts)
		return KErrArgument;
	TUint32 flags = 0;
	if (id>=36 && id<=41)
		{
		flags |= NKern::EIrqBind_Count;
		}
	return NKern::InterruptBind(id-32, aIsr, aPtr, flags, 0);
	}

EXPORT_C TInt Interrupt::Unbind(TInt aId)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Unbind id=%08x",aId));
	if (aId >= 0x10000)
		return NKern::InterruptUnbind(aId);			// aId is a handle not a specific ID
	if (TUint(aId) < TUint(KNumNaviEngineMaxInts))
		return NKern::InterruptUnbind(aId - 32);	// Vector number to nanokernel interrupt number
	return KErrArgument;
	}

EXPORT_C TInt Interrupt::Enable(TInt aId)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Enable id=%08x",aId));
	if (aId >= 0x10000)
		return NKern::InterruptEnable(aId);			// aId is a handle not a specific ID
	if (TUint(aId) < TUint(KNumNaviEngineMaxInts))
		return NKern::InterruptEnable(aId - 32);	// Vector number to nanokernel interrupt number
	return KErrArgument;
	}

EXPORT_C TInt Interrupt::Disable(TInt aId)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Disable id=%08x",aId));
	if (aId >= 0x10000)
		return NKern::InterruptDisable(aId);		// aId is a handle not a specific ID
	if (TUint(aId) < TUint(KNumNaviEngineMaxInts))
		return NKern::InterruptDisable(aId - 32);	// Vector number to nanokernel interrupt number
	return KErrArgument;
	}

EXPORT_C TInt Interrupt::Clear(TInt aId)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Clear id=%08x",aId));
	if (aId >= 0x10000)
		return NKern::InterruptClear(aId);			// aId is a handle not a specific ID
	if (TUint(aId) < TUint(KNumNaviEngineMaxInts))
		return NKern::InterruptClear(aId - 32);		// Vector number to nanokernel interrupt number
	return KErrArgument;
	}

EXPORT_C TInt Interrupt::SetPriority(TInt /*anId*/, TInt /*aPriority*/)
	{
	return KErrNotSupported;
	}

#else
SInterruptHandler NaviEngineInterrupt::Handlers[KNumNaviEngineInts];

void NaviEngineInterrupt::DisableAndClearAll()
	{
    //Disable all interrupts
    for(TUint i=0; i<=((KNumNaviEngineInts-1)/32); i++)
    	AsspRegister::Write32(KHoGidIntDisableBase+i*4, 0xffffffff);
    //Clear all interrupts
    for(TUint i=0; i<=((KNumNaviEngineInts-1)/32); i++)
    	AsspRegister::Write32(KHoGidPendClearBase+i*4, 0xffffffff);

	}

void NaviEngineInterrupt::Init1()
	{
	//
	// need to hook the ARM IRQ and FIQ handlers as early as possible and disable and clear all interrupt sources
	//
	__KTRACE_OPT(KBOOT,Kern::Printf("NaviEngineInterrupt::Init1()"));
	TInt i;
	for (i=0; i<KNumNaviEngineInts; i++)
		{
		Handlers[i].iPtr=(TAny*)i;
		Handlers[i].iIsr=Spurious;
		}
	DisableAndClearAll();
	Arm::SetIrqHandler((TLinAddr)NaviEngineInterrupt::IrqDispatch);
	Arm::SetFiqHandler((TLinAddr)NaviEngineInterrupt::FiqDispatch);

    // All interrupts are N-N (LSB = 1)
    // Both edge (MSB = 1) and level triggered (MSB = 0)
	// are present
	for (TInt i=0; i<KNumNaviEngineInts; ++i)
		{
		TUint8 isEdge = 0x0;
		if(i>=32 && i<128)
			{
			isEdge = IntIsEdge[i-32];
			}
		const TUint8 isNN = 0x1;
		const TUint8 irqConfig = (isEdge << 1) | isNN;

		const TInt byteOffset = i/4;
		const TInt shiftForIrq = i%4;
		TUint8 setMask = irqConfig << shiftForIrq;
		AsspRegister::Modify8(KHoGidConfigBase + byteOffset, 0, setMask);
		}

	// Set interrupts starting from 32 to this CPU only
    for(TUint i=0; i<=((KNumNaviEngineInts-1)/4); i++)
    	AsspRegister::Write32(KHoGidTargetBase+i*4, 0x0f0f0f0f);

	// Set priority on all interrupts
    for(TUint i=0; i<=((KNumNaviEngineInts-1)/4); i++)
    	AsspRegister::Write32(KHoGidPriorityBase+i*4, 0xA0A0A0A0);

    // Set binary point. No-preemption, all bits used
    AsspRegister::Write32(KHwCIIBinPoint, 7);

    // Set priority mask
    AsspRegister::Write32(KHwCIIPrioMask, 0xF0);

    // Enable Distributor and CPU Interface for IRQ
    AsspRegister::Write32(KHoGidControl, 1);
    AsspRegister::Write32(KHwCIIControl, 1);
	}

void NaviEngineInterrupt::Init3()
	{
	//
	// Any further initialisation of the Hardware Interrupt Controller
	// Note This is not called at the moment. Should be deleted.
	}

void NaviEngineInterrupt::Spurious(TAny* anId)
	{
	Kern::Fault("SpuriousInt", (TInt)anId);
	}

//
// The APIs below assume ther is a second level Interrupt controller located at Variant level which handles
// interrupts generated by hardware at that level.
//

EXPORT_C TInt Interrupt::Bind(TInt anId, TIsr anIsr, TAny* aPtr)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Bind id=%d func=%08x ptr=%08x",anId,anIsr,aPtr));
	TInt r = anId;
	// if ID indicates a chained interrupt, call variant...
//	if (anId<0 && ((((TUint)anId)>>16)&0x7fff)<(TUint)KNumNaviEngineInts)
//		r=NaviEngineAssp::Variant->InterruptBind(anId,anIsr,aPtr);
	__NK_ASSERT_ALWAYS(anId>=0);
	if ((TUint)anId >= (TUint)KNumNaviEngineInts)
		r=KErrArgument;
	else
		{
		SInterruptHandler& h=NaviEngineInterrupt::Handlers[anId];
		TInt irq=NKern::DisableAllInterrupts();
		if (h.iIsr != NaviEngineInterrupt::Spurious)
			r=KErrInUse;
		else
			{
			h.iPtr=aPtr;
			h.iIsr=anIsr;
			}
		NKern::RestoreInterrupts(irq);
		}
	return r;
	}

EXPORT_C TInt Interrupt::Unbind(TInt anId)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Unbind id=%d",anId));
	TInt r=KErrNone;
	// if ID indicates a chained interrupt, call variant...
	if (anId<0 && ((((TUint)anId)>>16)&0x7fff)<(TUint)KNumNaviEngineInts)
		r=NaviEngineAssp::Variant->InterruptUnbind(anId);
	else if ((TUint)anId >= (TUint)KNumNaviEngineInts)
		r=KErrArgument;
	else
		{
		SInterruptHandler& h=NaviEngineInterrupt::Handlers[anId];
		TInt irq=NKern::DisableAllInterrupts();
		if (h.iIsr == NaviEngineInterrupt::Spurious)
			r=KErrGeneral;
		else
			{
			h.iPtr=(TAny*)anId;
			h.iIsr=NaviEngineInterrupt::Spurious;
			Disable(anId);
			}
		NKern::RestoreInterrupts(irq);
		}
	return r;
	}

EXPORT_C TInt Interrupt::Enable(TInt anId)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Enable id=%d",anId));
	TInt r=KErrNone;
	// if ID indicates a chained interrupt, call variant...
	if (anId<0 && ((((TUint)anId)>>16)&0x7fff)<(TUint)KNumNaviEngineInts)
		r=NaviEngineAssp::Variant->InterruptEnable(anId);
	else if ((TUint)anId>=(TUint)KNumNaviEngineInts)
		r=KErrArgument;
	else if (NaviEngineInterrupt::Handlers[anId].iIsr==NaviEngineInterrupt::Spurious)
		r=KErrNotReady;
	else
		{
		TUint reg = anId/32;
		TUint mask = 1 << (anId - 32*reg);
	    AsspRegister::Write32(KHoGidIntEnableBase+reg*4, mask);
		}
	return r;
	}

EXPORT_C TInt Interrupt::Disable(TInt anId)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Disable id=%d",anId));
	TInt r=KErrNone;
	// if ID indicates a chained interrupt, call variant...
	if (anId<0 && ((((TUint)anId)>>16)&0x7fff)<(TUint)KNumNaviEngineInts)
		r=NaviEngineAssp::Variant->InterruptDisable(anId);
	else if ((TUint)anId>=(TUint)KNumNaviEngineInts)
		r=KErrArgument;
	else
		{
		TUint reg = anId/32;
		TUint mask = 1 << (anId - 32*reg);
	    AsspRegister::Write32(KHoGidIntDisableBase+reg*4, mask);
		}
	return r;
	}

EXPORT_C TInt Interrupt::Clear(TInt anId)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Interrupt::Clear id=%d",anId));
	TInt r=KErrNone;
	// if ID indicates a chained interrupt, call variant...
	if (anId<0 && ((((TUint)anId)>>16)&0x7fff)<(TUint)KNumNaviEngineInts)
		r=NaviEngineAssp::Variant->InterruptClear(anId);
	else if ((TUint)anId>=(TUint)KNumNaviEngineInts)
		r=KErrArgument;
	else
		{
		TUint reg = anId/32;
		TUint mask = 1 << (anId - 32*reg);
	    AsspRegister::Write32(KHoGidPendClearBase+reg*4, mask);
		}
	return r;
	}

EXPORT_C TInt Interrupt::SetPriority(TInt /*anId*/, TInt /*aPriority*/)
	{
	return KErrNotSupported;
	}
#endif
