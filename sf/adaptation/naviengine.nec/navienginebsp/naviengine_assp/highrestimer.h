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
* Defintions for High resolution timer
*
*/





#ifndef __HIGHRESTIMER_H__
#define __HIGHRESTIMER_H__

/**
 * Macro indicating that a high resolution timer is supported.
 */
#define HAS_HIGH_RES_TIMER

#ifdef __MEMMODEL_DIRECT__
#define HIGH_RES_COUNT_HW_ADDR 0x18036800	/* address of timer 2 counter */
#else
#define HIGH_RES_COUNT_HW_ADDR 0xC6003800	/* address of timer 2 counter */
#endif

/**
 * Assembler macro to get the the current value of the high res timer and place
 * it in the specified register.
 */
#define GET_HIGH_RES_TICK_COUNT(Rd) \
	asm("ldr	"#Rd", = %a0" : : "i" (HIGH_RES_COUNT_HW_ADDR)); \
	asm("ldr	"#Rd", ["#Rd"]")

/**
 * The frequency of the timer in Hz.
 */
const TInt KHighResTimerFrequency = 2083333; // 200MHz/(3*32)

/**
 * Macro indicating that the timer counts up if defined.
 */
#define HIGH_RES_TIMER_COUNTS_UP

#endif
