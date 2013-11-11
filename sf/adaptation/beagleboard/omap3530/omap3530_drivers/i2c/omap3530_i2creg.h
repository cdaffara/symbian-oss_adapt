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
// omap3530/omap3530_drivers/i2c/omap3530_i2creg.h
// I2C register interface - uses the I2C driver main interface
// This file is part of the Beagle Base port
//

#ifndef OMAP3530_I2CREG_H_
#define OMAP3530_I2CREG_H_

#include <assp/omap3530_assp/omap3530_i2c.h> // for I2c::THandle
namespace I2cReg
{
IMPORT_C TUint8 ReadB(I2c::THandle, TUint8 aAddr); // Synchronous read of a single byte at address aAddr
IMPORT_C void WriteB(I2c::THandle, TUint8 aAddr, TUint8 aData); // Synchronous write of a single byte, aData, at address aAddr
}

#endif // !OMAP3530_I2CREG_H_
