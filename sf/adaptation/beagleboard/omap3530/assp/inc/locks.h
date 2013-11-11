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
//

#ifndef __95_locks_h__
#define __95_locks_h__

/**
@publishedPartner
@prototype
*/
#define __SPIN_LOCK_IRQ_R(lock)					((void)NKern::DisableAllInterrupts())

/**
@publishedPartner
@prototype
*/
#define __SPIN_UNLOCK_IRQ_R(lock)				(NKern::EnableAllInterrupts())

/**
@publishedPartner
@prototype
*/
#define __SPIN_FLASH_IRQ_R(lock)				(NKern::EnableAllInterrupts(),(void)NKern::DisableAllInterrupts(),((TBool)TRUE))

/**
@publishedPartner
@prototype
*/
#define __SPIN_LOCK_IRQ_W(lock)					((void)NKern::DisableAllInterrupts())

/**
@publishedPartner
@prototype
*/
#define __SPIN_UNLOCK_IRQ_W(lock)				(NKern::EnableAllInterrupts())

/**
@publishedPartner
@prototype
*/
#define __SPIN_FLASH_IRQ_W(lock)				(NKern::EnableAllInterrupts(),(void)NKern::DisableAllInterrupts(),((TBool)TRUE))


/**
@publishedPartner
@prototype
*/
#define __SPIN_LOCK_R(lock)						

/**
@publishedPartner
@prototype
*/
#define __SPIN_UNLOCK_R(lock)					

/**
@internalComponent
*/
#define __SPIN_FLASH_R(lock)					((TBool)FALSE)

/**
@publishedPartner
@prototype
*/
#define __SPIN_LOCK_W(lock)						

/**
@publishedPartner
@prototype
*/
#define __SPIN_UNLOCK_W(lock)					

/**
@internalComponent
*/
#define __SPIN_FLASH_W(lock)					((TBool)FALSE)


/**
@publishedPartner
@prototype
*/
#define __SPIN_LOCK_IRQSAVE_R(lock)				(NKern::DisableAllInterrupts())

/**
@publishedPartner
@prototype
*/
#define __SPIN_UNLOCK_IRQRESTORE_R(lock,irq)	(NKern::RestoreInterrupts(irq))

/**
@publishedPartner
@prototype
*/
#define __SPIN_FLASH_IRQRESTORE_R(lock,irq)		(NKern::RestoreInterrupts(irq),((void)NKern::DisableAllInterrupts()),((TBool)TRUE))

/**
@publishedPartner
@prototype
*/
#define __SPIN_LOCK_IRQSAVE_W(lock)				(NKern::DisableAllInterrupts())

/**
@publishedPartner
@prototype
*/
#define __SPIN_UNLOCK_IRQRESTORE_W(lock,irq)	(NKern::RestoreInterrupts(irq))

/**
@publishedPartner
@prototype
*/
#define __SPIN_FLASH_IRQRESTORE_W(lock,irq)		(NKern::RestoreInterrupts(irq),((void)NKern::DisableAllInterrupts()),((TBool)TRUE))


/**
@publishedPartner
@prototype
*/
#define __SPIN_FLASH_PREEMPT_R(lock)			((TBool)NKern::PreemptionPoint())

/**
@publishedPartner
@prototype
*/
#define __SPIN_FLASH_PREEMPT_W(lock)			((TBool)NKern::PreemptionPoint())

#endif