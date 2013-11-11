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
* naviengine_assp\gpio.inl
* NaviEngine implementation of the MHA GPIO class
*
*/



inline TInt GpioLock()
	{
	return __SPIN_LOCK_IRQSAVE(GpioSpinLock);
	}

inline void GpioUnlock(TInt irq)
	{
	__SPIN_UNLOCK_IRQRESTORE(GpioSpinLock, irq);
	}

