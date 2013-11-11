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
// omap3530/omap3530_drivers/i2c/omap3530_i2c.h
// I2C driver interface.
// This file is part of the Beagle Base port
//

#ifndef OMAP3530_I2C_H_
#define OMAP3530_I2C_H_

#include <e32def.h>

#define I2C_VERSION 0 // update if TConfigPb or TTransferPb change

class TDfc;
class TDfcQue;

namespace I2c
{
class TDeviceControl;

enum TUnit
	{
	E1, // EMaster, ESlave
	E2, // EMaster, ESlave, ESccb
	E3 // EMaster, ESlave, ESccb
	};
enum TRole
	{
	EMaster
//	ESlave - TBI
	};
enum TMode
	{
	E7Bit
//	E10Bit  - TBI
//	EHs - TBI
//	ESccb - TBI
	};
enum TRate
	{
	E100K,
	E400K
//	E3M31 - TBI
	};
typedef TInt TDeviceAddress; // range 0..1023 or 0..128
struct TVersion
	{
	inline TVersion() : iVersion(I2C_VERSION){}
	TInt iVersion;
	};
struct TConfigPb : TVersion // the parameter block used by Open()
	{
	IMPORT_C TConfigPb();

	TUnit iUnit;
	TRole iRole;
	TMode iMode;
	void* iExclusiveClient; // Clients magic number - zero if the client doesn't require exclusive access otherwise use an owned object, code, stack or data address.
	TRate iRate;
	TDeviceAddress iOwnAddress;
	TDeviceAddress iDeviceAddress; // if role is master
	TDfcQue* iDfcQueue; // clients thread
	};
typedef TInt THandle; // returned from Open()
struct TTransferPb // the parameter block used by Transfer*()
	{
	IMPORT_C TTransferPb();

	enum
		{
		ERead,
		EWrite
		} iType;

	TInt iLength; // in bytes.
	const TUint8* iData; // only truly const for writes, i.e. *iData is modified bye Read transfers

	TTransferPb* iNextPhase; // zero if a full transfer or last phase of a multiphase transfer

	// only for asynchronous transfers i.e. WriteA() or ReadA();
	TDfc* iCompletionDfc;
	TInt iResult;

	// reserved for use by I2C driver
	TTransferPb* iNextTransfer;
	TDeviceControl* iDcb;
	};

// call Open() for each slave device.
// returns:
// KErrArgument if a combination of iUnit, iRole, iMode or iRate are not supported
// KErrInUse if already open exclusively, the configuration doesn't match or the device is already open.
// KErrTooBig if there are already KMaxDevicesPerUnit (currently 8) slave devices open
// a positive value if successful
IMPORT_C THandle Open(const TConfigPb&); 
IMPORT_C void Close(THandle&);
IMPORT_C TInt TransferS(THandle, TTransferPb&);
IMPORT_C void TransferA(THandle, TTransferPb&); // TBI
IMPORT_C void CancelATransfer(THandle); // TBI
}

#endif // !OMAP3530_I2C_H_
