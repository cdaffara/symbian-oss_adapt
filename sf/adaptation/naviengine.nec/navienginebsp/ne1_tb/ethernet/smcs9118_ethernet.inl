/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


inline TInt32 DEthernetSMCS9118Pdd::IsReady()
	{
	return iReady;
	}

/**
 * see data sheet section 6.1, Host Interface Timing
 * "dummy" reads of the BYTE_TEST register will
 * guarantee the minimum write-to-read timing restrictions
 * as listed in Table 6.1
 */
inline void DEthernetSMCS9118Pdd::ByteTestDelay(TUint32 aCount)
	{
	TUint32 i;

	for (i = 0; i< aCount; i++)
		{
		AsspRegister::Read32(SMCS9118_BYTE_TEST);
		}
	}

/**
 * Read a 32bit register
 */
inline TUint32 DEthernetSMCS9118Pdd::Read32(TUint32 aReg)
	{
	return AsspRegister::Read32(aReg);
	}

/**
 * Write a 32bit register
 */
inline void DEthernetSMCS9118Pdd::Write32(TUint32 aReg, TUint32 aVal)
	{
	AsspRegister::Write32(aReg, aVal);
	}

/**
 * Interrupt handling
 */
inline void DEthernetSMCS9118Pdd::ClearInterrupt(TInt aId)
	{
	GPIO::ClearInterrupt(aId);
	}

inline void DEthernetSMCS9118Pdd::UnbindInterrupt(TInt aId)
	{
	GPIO::UnbindInterrupt(aId);
	}

inline TInt DEthernetSMCS9118Pdd::BindInterrupt(TInt aId, TGpioIsr aIsr, TAny *aPtr)
	{
	return GPIO::BindInterrupt(aId, aIsr, aPtr);
	}

inline TInt DEthernetSMCS9118Pdd::EnableInterrupt(TInt aId)
	{
	return GPIO::EnableInterrupt(aId);
	}

inline TInt DEthernetSMCS9118Pdd::DisableInterrupt(TInt aId)
	{
	return GPIO::DisableInterrupt(aId);
	}

/**
 * lock handling
 */
inline TInt DEthernetSMCS9118Pdd::DriverLock()
	{
	return __SPIN_LOCK_IRQSAVE(*iDriverLock); 
	}

inline void DEthernetSMCS9118Pdd::DriverUnlock(TInt irq)
	{
	__SPIN_UNLOCK_IRQRESTORE(*iDriverLock, irq); 
	}
