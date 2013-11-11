// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/beagleboard/inc/gpio.h
//

#ifndef __BEAGLE_GPIO_H__
#define __BEAGLE_GPIO_H__

#include <assp/omap3530_assp/omap3530_gpio.h>

const TUint KGPIOINT_UserButton = EGPIOIRQ_PIN_7;
const TUint KGPIOINT_LED0 = EGPIOIRQ_PIN_150;
const TUint KGPIOINT_LED1 = EGPIOIRQ_PIN_149;
const TUint KGPIOINT_TFP410_POWERDOWN = EGPIOIRQ_PIN_170;//DVI_PUP Controls the DVI-D interface. A Hi = DVI-D enabled.
const TUint KGPIOINT_MMC1_WP = EGPIOIRQ_PIN_29; // I MMC1_WP SD/MMC card slot Write protect

const TUint KGPIO_UserButton = 7;
const TUint KGPIO_LED0 = 150;
const TUint KGPIO_LED1 = 149;

#endif //__BEAGLE_GPIO_H__
