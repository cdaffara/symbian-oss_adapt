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
* The frequency of the system timestamp counter in Hz
* The timestamp counter is a 64 bit value which is required to increment at a
* constant rate of at least 1MHz and not to wrap in any reasonable amount of time.
*
*/





const TUint32 KTimestampFrequency = 66666667u;

#ifdef __DEFINE_NKERN_TIMESTAMP_ASM__
/* If NKern::Timestamp() is written in assembler define it here
*/

EXPORT_C __NAKED__ TUint64 NKern::Timestamp()
	{
	// Return a 64 bit high resolution timestamp count
	// Timer 1 counts from 00000000 to FFFFFEFF and then wraps back to 0
	// Timer 2 counts from 00000000 to FFFFFFFF and then wraps back to 0
	// Both timers increment at 66.667MHz (15ns period)
	// Algorithm:
	//		volatile TUint32 t2 = Timer2Count;	// must read t2 first
	//		volatile TUint32 t1 = Timer1Count;
	//		TUint32 n = (t1-t2)>>8;		// number of times T1 has wrapped
	//		return t1 + n * 0xFFFFFF00;

	asm("ldr	r3, 1f ");				// r3 = address of T1 counter
	asm("mrs	r12, cpsr ");
	__ASM_CLI();						// interrupts off
	asm("ldr	r1, [r3, #0x400] ");	// r1 = t2
	asm("ldr	r0, [r3] ");			// r0 = t1
	asm("msr	cpsr, r12 ");			// restore interrupts
	asm("sub	r1, r0, r1 ");			// t1-t2
	asm("mov	r1, r1, lsr #8 ");		// n = (t1-t2)>>8, now have r1:r0 = 2^32*n + t1
	asm("subs	r0, r0, r1, lsl #8 ");	// subtract 256*n
	asm("sbcs	r1, r1, #0 ");			// propagate borrow
	__JUMP(,lr);

	asm("1: ");
#ifdef __MEMMODEL_DIRECT__
	asm(".word 0x18036400 ");				// address of timer 1 counter
#else
	asm(".word 0xC6003400 ");				// address of timer 1 counter
#endif
	}

#endif

#ifdef __DEFINE_NKERN_TIMESTAMP_CPP__
/* If NKern::Timestamp() is written in C++ define it here
*/
#endif

