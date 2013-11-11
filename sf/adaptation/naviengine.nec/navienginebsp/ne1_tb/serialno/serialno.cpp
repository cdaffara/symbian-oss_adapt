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
* @file
* @brief determine board serial number.
* This code is intended to read the board serial number and set it in the
* variant config for later retrieval via HAL.
* Currently, on NaviEngine, the Ethernet driver will set the HAL serial number
* to the lowest 4 bytes of the MAC address, so this is a stub DLL for now.
*
*/



#include <kernel/kernel.h>
#include "variant.h"

/**
 * DLL entry point function.
 */
DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("SerialNo: setting board's serial number"));
	return NE1_TBVariant::SetSerialNumber(0);
	}
