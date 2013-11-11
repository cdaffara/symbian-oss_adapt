// Copyright (c) 1994-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/beagle_drivers/led/led.cpp
//

#include <kern_priv.h>
#include <beagle/beagle_gpio.h>
#include <beagle/variant.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_irqmap.h> // GPIO interrupts
#include <assp/omap3530_assp/omap3530_gpio.h>

#include <assp.h> // Required for definition of TIsr

#include <cyashalbeagleboard.h>

DECLARE_STANDARD_EXTENSION()
	{

		CyAsHalAstoriaInit();
		return KErrNone;
	}




