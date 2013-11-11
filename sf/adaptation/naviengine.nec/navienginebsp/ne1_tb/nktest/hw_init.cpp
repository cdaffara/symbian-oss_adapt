/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* ne1_tb\nktest\hw_init.cpp
*
*/



#include <arm.h>
#include <nkutils.h>
#include <diag.h>
#include "kernboot.h"
//#include <naviengine.h>

#ifdef __SMP__
#include <arm_gic.h>
#endif


const TUint KHwBaseMPcorePrivatePhys = 0xC0000000u;

extern void DumpExcInfo(TArmExcInfo&);
extern void DumpFullRegSet(SFullArmRegSet& a);

extern "C" void Interrupt_Init1();
extern "C" void Interrupt_Init2AP();
extern "C" void Interrupt_Init3();

extern "C" TLinAddr DebugPortBase();

extern "C" TUint KernCoreStats_EnterIdle(TUint);
extern "C" void KernCoreStats_LeaveIdle(TInt,TUint);

extern "C" {
extern TLinAddr RomHeaderAddress;
extern TLinAddr SuperPageAddress;
}

#ifdef __SMP__
TSpinLock DbgSpinLock(TSpinLock::EOrderGenericIrqLow1);
#endif


struct NETimer
	{
	static inline NETimer& Timer(TUint a) { return *(NETimer*)(0x18036000u + (a<<10)); }
	volatile TUint32 iTimerCount;
	volatile TUint32 iTimerCtrl;
	volatile TUint32 iTimerReset;
	volatile TUint32 iGTOPulseStart;
	volatile TUint32 iGTOPulseEnd;
	volatile TUint32 iGTICtrl;
	volatile TUint32 iGTIRisingEdgeCapture;
	volatile TUint32 iGTIFallingEdgeCapture;
	volatile TUint32 iGTInterrupt;
	volatile TUint32 iGTInterruptEnable;
	volatile TUint32 iPrescaler;
	};

class ArmGic
	{
public:
	static void Dump();
	static void DumpCpuIfc();
	};

#ifdef __SMP__

extern "C" void ApMainGeneric(volatile SAPBootInfo* aInfo);

extern "C" {
SVariantInterfaceBlock TheVIB;

SVariantInterfaceBlock* InitVIB()
	{
	SVariantInterfaceBlock* v = &TheVIB;
	v->iVer = 0;
	v->iSize = sizeof(TheVIB);
	v->iMaxCpuClock = UI64LIT(400000000);	// 400MHz
	v->iMaxTimerClock = 200000000u;			// 200MHz = CPU CLK / 2
	v->iScuAddr = KHwBaseMPcorePrivatePhys + 0x0;
	v->iGicDistAddr = KHwBaseMPcorePrivatePhys + 0x1000;
	v->iGicCpuIfcAddr = KHwBaseMPcorePrivatePhys + 0x100;
	v->iLocalTimerAddr = KHwBaseMPcorePrivatePhys + 0x600;
	return v;
	}
}

#endif

static TInt SystemTimerInterruptHandle = -1;

extern "C" {

void TimerIsr(TAny* aPtr)
	{
	NETimer& NET = NETimer::Timer(0);
	NET.iGTInterrupt = 0x1fu;
	__e32_io_completion_barrier();
	((NTimerQ*)aPtr)->Tick();
	}

void StartSystemTimer()
	{
	__KTRACE_OPT(KBOOT,DEBUGPRINT(">StartSystemTimer()"));

	NETimer& NET = NETimer::Timer(0);
	NET.iTimerCtrl = 0;
	NET.iGTICtrl = 0;
	__e32_io_completion_barrier();
	NET.iTimerCtrl = 2;
	__e32_io_completion_barrier();
	NET.iTimerReset = 66666;
//	NET.iTimerReset = 66666666;
	__e32_io_completion_barrier();
	NET.iGTInterrupt = 0x1fu;
	__e32_io_completion_barrier();

	NTimerQ& m=*(NTimerQ*)NTimerQ::TimerAddress();
	TUint32 flags = NKern::EIrqBind_Count;
	TInt r = NKern::InterruptBind(36-32, &TimerIsr, &m, flags, 0);
	DEBUGPRINT("r=%08x", r);
	__NK_ASSERT_ALWAYS(r>=0);
	SystemTimerInterruptHandle = r;

	NKern::InterruptEnable(r);
	DEBUGPRINT("r=%08x", r);

	NET.iGTInterruptEnable = 0x10u;
	__e32_io_completion_barrier();
	NET.iTimerCtrl = 3;
	__e32_io_completion_barrier();

//	NTimerQ& m=*(NTimerQ*)NTimerQ::TimerAddress();
//	TInt r=Interrupt::Bind(EIntIdTimer,&MsTimerTick,&m);
//	__NK_ASSERT_ALWAYS(r>=0);
//	initTimer(PIT_COUNT_FOR_1MS);
//	r=Interrupt::Enable(r);
//	KPrintf("r=%d",r);
//	__NK_ASSERT_ALWAYS(r>=0);
	__KTRACE_OPT(KBOOT,DEBUGPRINT("<StartSystemTimer()"));

	TInt i;
	for (i=0; i<50; ++i)
		{
		DEBUGPRINT("Count %8d IRQ %02x", NET.iTimerCount, NET.iGTInterrupt);
		}

	ArmGic::Dump();
	ArmGic::DumpCpuIfc();
	}

void HijackSystemTimer(NSchedulable* aTieTo)
	{
	TInt r = NKern::InterruptUnbind(SystemTimerInterruptHandle);
	// need to accept KErrArgument because if the tied thread/group
	// has gone away the interrupt will be unbound already and the
	// handle is not valid
	__NK_ASSERT_ALWAYS(r==KErrNone || r==KErrArgument);

	NTimerQ& m=*(NTimerQ*)NTimerQ::TimerAddress();
	TUint32 flags = NKern::EIrqBind_Count;
	if (aTieTo)
		flags |= NKern::EIrqBind_Tied;
	r = NKern::InterruptBind(36-32, &TimerIsr, &m, flags, aTieTo);
	__NK_ASSERT_ALWAYS(r>=0);
	SystemTimerInterruptHandle = r;
	NKern::InterruptEnable(r);
	}

static int debug_uart_data_available()
	{
	TUint32 base = DebugPortBase();
	volatile TUint8& LSR = *(volatile TUint8*)(base + 0x14);
	return LSR & 0x01;
	}

static int debug_uart_poll()
	{
	TUint32 base = DebugPortBase();
	volatile TUint8& LSR = *(volatile TUint8*)(base + 0x14);
	volatile TUint8& RXHR = *(volatile TUint8*)(base + 0x00);
	if (LSR & 0x01)
		return RXHR;
	return -1;
	}

static void write_debug_uart(char aChar)
	{
	TUint32 base = DebugPortBase();
	volatile TUint8& LSR = *(volatile TUint8*)(base + 0x14);
	volatile TUint8& TXHR = *(volatile TUint8*)(base + 0x00);

	while (!(LSR & 0x20))
		{}

	TXHR = (TUint8)aChar;
	}

const DiagIO DebugUartIO =
	{
	&debug_uart_data_available,
	&debug_uart_poll,
	&write_debug_uart
	};

static void init_debug_uart()
	{
	write_debug_uart('*');
	TheIoFunctions = &DebugUartIO;
	}

// have DFAR DFSR IFSR R13 R14 CPSR ExcCode R5-R11 R0-R4 R12 PC saved
struct X
	{
	TUint32	iDFAR;
	TUint32	iDFSR;
	TUint32	iIFSR;
	TUint32	iR13;
	TUint32	iR14;
	TUint32	iCPSR;
	TUint32	iExcCode;
	TUint32	iR5;
	TUint32	iR6;
	TUint32	iR7;
	TUint32	iR8;
	TUint32	iR9;
	TUint32	iR10;
	TUint32	iR11;
	TUint32	iR0;
	TUint32	iR1;
	TUint32	iR2;
	TUint32	iR3;
	TUint32	iR4;
	TUint32	iR12;
	TUint32	iR15;
	};

extern "C" {
void hw_init_exc(TUint32* a)
	{
	X& x = *(X*)a;
	TInt irq = DbgSpinLock.LockIrqSave();
	DumpStruct(
		"-------------------------------------\n"
		"DFAR %w DFSR %w IFSR %w\n"
		"R13  %w R14  %w CPSR %w ExcCode %w\n"
		"R5   %w R6   %w R7   %w R8   %w\n"
		"R9   %w R10  %w R11  %w\n"
		"R0   %w R1   %w R2   %w R3   %w\n"
		"R4   %w R12  %w PC   %w\n",
		a);
	if (x.iExcCode==2)
		{
		TUint32* p = (TUint32*)x.iR15;
		TUint32 inst = *p;
		if (inst>=0xe7ffdef0u && inst<0xe7ffdeffu)
			{
			PrtHex8(inst);
			NewLine();
			x.iR15 += 4;
			DbgSpinLock.UnlockIrqRestore(irq);
			return;
			}
		}
	RunCrashDebugger();
	}
}

extern "C" void __DebugMsgGlobalCtor(TUint addr, TUint cpsr)
	{
	PrtHex8(cpsr); PutSpc(); PrtHex8(addr); NewLine();
	}

extern "C" TUint64 fast_counter_x(TUint32*);
extern "C" void HwInit0()
	{
	init_debug_uart();

	NETimer& T1 = NETimer::Timer(1);
	NETimer& T2 = NETimer::Timer(2);

	T1.iTimerCtrl = 0;	// stop and reset timer 1
	T1.iGTICtrl = 0;	// disable timer 1 capture modes
	T2.iTimerCtrl = 0;	// stop and reset timer 2
	T2.iGTICtrl = 0;	// disable timer 2 capture modes
	__e32_io_completion_barrier();
	T1.iPrescaler = 1;	// Timer 1 prescaled by 1 (=66.667MHz)
	T2.iPrescaler = 1;	// Timer 2 prescaled by 1
//	T1.iPrescaler = 4;	// Timer 1 prescaled by 4 (=16.667MHz)
//	T2.iPrescaler = 4;	// Timer 2 prescaled by 4
	__e32_io_completion_barrier();
	T1.iGTInterruptEnable = 0;
	T2.iGTInterruptEnable = 0;
	__e32_io_completion_barrier();
	T1.iGTInterrupt = 0x1f;
	T2.iGTInterrupt = 0x1f;
	__e32_io_completion_barrier();
	T1.iTimerCtrl = 2;	// deassert reset for timer 1, count still stopped
	T2.iTimerCtrl = 2;	// deassert reset for timer 2, count still stopped
	__e32_io_completion_barrier();
	T1.iTimerReset = 0xfffffeffu;	// timer 1 wraps after 2^32-256 counts
	T2.iTimerReset = 0xffffffffu;	// timer 2 wraps after 2^32 counts
	__e32_io_completion_barrier();
	T1.iTimerCtrl = 3;	// start timer 1
	__e32_io_completion_barrier();
	T2.iTimerCtrl = 3;	// start timer 2
	__e32_io_completion_barrier();

	// Each time T1 wraps, (T1-T2) increases by 256 after starting at 0
	// t1=T1; t2=T2; n=(t1-t2)>>8; time = t1 + n * (2^32-256)

	TUint32 t[2];
	TUint64 x = fast_counter_x(t);
	DEBUGPRINT("t1=%08x t2=%08x result %08x %08x", t[0], t[1], I64HIGH(x), I64LOW(x));
	}

void Hw_Init1()
	{
	__CHECKPOINT();

#ifdef __SMP__
	NKern::Init0(InitVIB());
#else
	NKern::Init0(0);
#endif
	Interrupt_Init1();
	__CHECKPOINT();
	Arm::Init1Interrupts();
	__CHECKPOINT();
	}

#ifdef __SMP__
extern "C" void Hw_InitAPs()
	{
	TSubScheduler& ss = SubScheduler();
	SSuperPageBase& spg = *(SSuperPageBase*)::SuperPageAddress;
	TInt ncpus = 4;
	TInt cpu;
	for (cpu=1; cpu<ncpus; ++cpu)
		{
		TAny* stack = 0;
		NThread* thread = 0;
		stack = malloc(4096);
		__NK_ASSERT_ALWAYS(stack);
		memset(stack, (0xe1|(cpu<<1)), 4096);
		thread = new NThread;
		__NK_ASSERT_ALWAYS(thread);

		SArmAPBootInfo info;
		memclr(&info,sizeof(info));
		info.iCpu = cpu;
		info.iInitStackSize = 4096;
		info.iInitStackBase = (TLinAddr)stack;
		info.iMain = &ApMainGeneric;
		info.iArgs[0] = (TAny*)thread;
		info.iAPBootLin = spg.iAPBootPageLin;
		info.iAPBootPhys = spg.iAPBootPagePhys;
		info.iAPBootCodeLin = ::RomHeaderAddress;
		info.iAPBootCodePhys = spg.iRomHeaderPhys;
		info.iAPBootPageDirPhys = spg.iAPBootPageDirPhys;
		TUint32 delta = cpu*0x2000;
		info.iInitR13Fiq = TLinAddr(ss.iSSX.iFiqStackTop) + delta;
		info.iInitR13Irq = TLinAddr(ss.iSSX.iIrqStackTop) + delta;
		info.iInitR13Abt = TLinAddr(ss.iSSX.iAbtStackTop) + delta;
		info.iInitR13Und = TLinAddr(ss.iSSX.iUndStackTop) + delta;
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iCpu=%08x", info.iCpu));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iInitStackSize=%08x", info.iInitStackSize));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iInitStackBase=%08x", info.iInitStackBase));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iMain=%08x", info.iMain));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iArgs=%08x %08x %08x %08x", info.iArgs[0], info.iArgs[1], info.iArgs[2], info.iArgs[3]));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iAPBootLin=%08x", info.iAPBootLin));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iAPBootPhys=%08x", info.iAPBootPhys));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iAPBootCodeLin=%08x", info.iAPBootCodeLin));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iAPBootCodePhys=%08x", info.iAPBootCodePhys));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iAPBootPageDirPhys=%08x", info.iAPBootPageDirPhys));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iInitR13Fiq=%08x", info.iInitR13Fiq));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iInitR13Irq=%08x", info.iInitR13Irq));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iInitR13Abt=%08x", info.iInitR13Abt));
		__KTRACE_OPT(KBOOT,DEBUGPRINT("iInitR13Und=%08x", info.iInitR13Und));
		TInt r = NKern::BootAP(&info);
		__KTRACE_OPT(KBOOT,DEBUGPRINT("ret %d", r));
		if (r==KErrNone)
			{
			while (__e32_atomic_load_acq32(&info.iArgs[1])==0)
				__cpu_yield();
			__KTRACE_OPT(KBOOT,DEBUGPRINT("CPU %d: OK NullThread=%08x InitialStack=%08x", cpu, thread, stack));
			stack = 0;
			thread = 0;
			}
		__NK_ASSERT_ALWAYS(r==KErrNone);
		if (stack)
			free(stack);
		if (thread)
			free(thread);
		}
	}
#endif

void Hw_Init3()
	{
	Interrupt_Init3();
	StartSystemTimer();
	}
}

extern "C" void NKCrashHandler(TInt aPhase, const TAny*, TInt)
	{
	if (aPhase==0)
		{
		return;
		}
	__finish();
	}

extern "C" void ExcFault(void* aExcInfo)
	{
#ifdef __SMP__
	SubScheduler().iSSX.iExcInfo = aExcInfo;
	SFullArmRegSet& a = *SubScheduler().iSSX.iRegs;
#else
	TheScheduler.i_ExcInfo = aExcInfo;
	SFullArmRegSet& a = *(SFullArmRegSet*)TheScheduler.i_Regs;
#endif
	if (aExcInfo)
		{
		Arm::SaveState(a);
		Arm::UpdateState(a, *(TArmExcInfo*)aExcInfo);
		}
	DumpFullRegSet(a);
	NKern::NotifyCrash(0,0);
	}

/**
Faults the system, noting file name and line number.

Used from nanokernel code and in various __ASSERT macros.

@param	file	The file name as a C string (__FILE__).
@param	line	The line number (__LINE__).

@see Kern::Fault()
*/
extern "C" void NKFault(const char* file, TInt line)
	{
	KPrintf("FAULT at line %d file %s", line, file);
	NKern::NotifyCrash(0,0);
	}


void DebugPrint(const char* s, int l)
	{
	TInt i;
#ifdef __SMP__
	TInt irq=0;
	if (!NKern::Crashed())
		irq = DbgSpinLock.LockIrqSave();
#endif
	for (i=0; i<l; ++i)
		PutC(*s++);
#ifdef __SMP__
	if (!NKern::Crashed())
		DbgSpinLock.UnlockIrqRestore(irq);
#endif
	}

TInt __timer_period()
	{
	return 1000;
	}

#ifdef __SMP__
TInt __microseconds_to_timeslice_ticks(TInt us)
	{
	return NKern::TimesliceTicks(us);
	}

TInt __fast_counter_to_timeslice_ticks(TUint64 aFCdelta)
	{
	// fast counter freq = 400MHz/6
	// timeslice freq = 400MHz/128
	aFCdelta*=3;
	aFCdelta>>=6;
	return (TInt)aFCdelta;
	}
#else
TInt __microseconds_to_timeslice_ticks(TInt us)
	{
	return (us+999)/1000;
	}

TInt __fast_counter_to_timeslice_ticks(TUint64 aFCdelta)
	{
	TUint64 fcf = fast_counter_freq();
	TUint64 x = (aFCdelta * 1000 + (fcf - 1)) / fcf;
	return (TInt)x;
	}
#endif

extern "C" {
extern TLinAddr RomHeaderAddress;
void __finish()
	{
	RunCrashDebugger();

//	TLinAddr f = RomHeaderAddress + 124;
//	(*(void (*)(TInt))f)(0x80000000);

	}
}

extern "C" void NKIdle(TUint32 aStage)
	{
/*
	SCpuIdleHandler* cih = NKern::CpuIdleHandler();
#ifdef __SMP__
	TSubScheduler& ss = SubScheduler();
	if (cih && cih->iHandler)
		(*cih->iHandler)(cih->iPtr, aStage, ss.iUncached);
#else
	if (cih && cih->iHandler)
		(*cih->iHandler)(cih->iPtr, aStage);
#endif
	else if (K::PowerModel)
		K::PowerModel->CpuIdle();
	else
		Arm::TheAsic->Idle();
*/
	__cpu_idle();
	}


extern "C" TUint32 IrqDispatch(TUint32 aVector)
	{
	if (aVector<32 || aVector>127)
		{
		GIC_CPU_IFC.iEoi = aVector;
		*(TInt*)0xdeaddead = 0;
		return aVector;
		}
	NKern::Interrupt(aVector - 32);
	return aVector;
	}

const TUint8 IntIsEdge[96] =
	{
	0, 1, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 1,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 1,		1, 0, 0, 0,		0, 0, 1, 1,
	1, 1, 1, 1,		1, 1, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 1, 1, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,
	0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0,		0, 0, 0, 0
	};

extern "C" void Interrupt_Init1()
	{
	__KTRACE_OPT(KBOOT, DEBUGPRINT(">Interrupt_Init1()"));
	Arm::SetIrqHandler((TLinAddr)&IrqDispatch);
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

extern "C" void Interrupt_Init2AP()
	{
	}

extern "C" void Interrupt_Init3()
	{
	}


#include <e32rom.h>

extern "C" TLinAddr DebugPortBase()
	{
	const TRomHeader& romHdr = *(const TRomHeader*)RomHeaderAddress;
	if (romHdr.iDebugPort & 1)
		return 0x18034400u;
	return 0x18034000u;
	}


extern "C" TUint KernCoreStats_EnterIdle(TUint)
	{
	return (TUint) EFalse;
	}

extern "C" void KernCoreStats_LeaveIdle(TInt,TUint)
	{
	}
