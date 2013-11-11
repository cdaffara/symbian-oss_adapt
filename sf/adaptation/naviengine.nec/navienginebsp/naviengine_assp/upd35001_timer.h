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
*
*/





#ifndef __UPD35001_TIMER_H__
#define __UPD35001_TIMER_H__
#include <naviengine.h>

struct NETimer
	{
	static inline NETimer& Timer(TUint a) { return *(NETimer*)(KHwTimersBase + (a<<10)); }
	volatile TUint32 iTimerCount;				/* counter register */
	volatile TUint32 iTimerCtrl;				/* control register */
	volatile TUint32 iTimerReset;				/* counter resets to zero upon reaching this value */
	volatile TUint32 iGTOPulseStart;			/* output pulse starts at this value */
	volatile TUint32 iGTOPulseEnd;				/* output pulse ends at this value */
	volatile TUint32 iGTICtrl;					/* input capture mode control */
	volatile TUint32 iGTIRisingEdgeCapture;		/* count captured at rising edge */
	volatile TUint32 iGTIFallingEdgeCapture;	/* count captured at falling edge */
	volatile TUint32 iGTInterrupt;				/* interrupt flags */
	volatile TUint32 iGTInterruptEnable;		/* interrupt enable mask */
	volatile TUint32 iPrescaler;				/* prescaler control */
	};

const TUint32	KNETimerCtrl_CAE			= 0x00000001u;	/* Count up enable (held static until set) */
const TUint32	KNETimerCtrl_CE				= 0x00000002u;	/* Counter enable (held at 0 until set) */
const TUint32	KNETimerCtrl_GTOEnable		= 0x00000004u;	/* GTO output enable */

const TUint32	KNETimerGTICtrl_FAL_M		= 0x00000001u;
const TUint32	KNETimerGTICtrl_FAL_C		= 0x00000002u;
const TUint32	KNETimerGTICtrl_RIS_M		= 0x00000004u;
const TUint32	KNETimerGTICtrl_RIS_C		= 0x00000008u;
const TUint32	KNETimerGTICtrl_FilterMask	= 0xfffffff0u;
const TUint32	KNETimerGTICtrl_FilterShift	= 4;

const TUint32	KNETimerGTIInt_GTIFI		= 0x00000001u;	/* interrupt detected on falling edge of GTI */
const TUint32	KNETimerGTIInt_GTIRI		= 0x00000002u;	/* interrupt detected on rising edge of GTI */
const TUint32	KNETimerGTIInt_GTOFI		= 0x00000004u;	/* interrupt detected on falling edge of GTO */
const TUint32	KNETimerGTIInt_GTORI		= 0x00000008u;	/* interrupt detected on rising edge of GTO */
const TUint32	KNETimerGTIInt_TCI			= 0x00000010u;	/* interrupt detected on timer counter reset */
const TUint32	KNETimerGTIInt_All			= 0x0000001fu;	/* mask to clear all pending interrupts */

const TUint32	KNETimerGTIIntE_GTIFE		= 0x00000001u;	/* interrupt enabled on falling edge of GTI */
const TUint32	KNETimerGTIIntE_GTIRE		= 0x00000002u;	/* interrupt enabled on rising edge of GTI */
const TUint32	KNETimerGTIIntE_GTOFE		= 0x00000004u;	/* interrupt enabled on falling edge of GTO */
const TUint32	KNETimerGTIIntE_GTORE		= 0x00000008u;	/* interrupt enabled on rising edge of GTO */
const TUint32	KNETimerGTIIntE_TCE			= 0x00000010u;	/* interrupt enabled on timer counter reset */

const TUint32	KNETimerPrescaleBy1			= 0x00000001u;
const TUint32	KNETimerPrescaleBy2			= 0x00000002u;
const TUint32	KNETimerPrescaleBy4			= 0x00000004u;
const TUint32	KNETimerPrescaleBy8			= 0x00000008u;
const TUint32	KNETimerPrescaleBy16		= 0x00000010u;
const TUint32	KNETimerPrescaleBy32		= 0x00000020u;

#endif


