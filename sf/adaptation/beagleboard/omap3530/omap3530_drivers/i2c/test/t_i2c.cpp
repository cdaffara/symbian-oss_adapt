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
// omap3530/assp/test/t_i2c.cpp
// Test code for I2C Driver
// This file is part of the Beagle Base port
//

#include <assp/omap3530_assp/omap3530_i2creg.h>

#include <kernel.h>
#include <nk_priv.h>
#include <assp.h>

DECLARE_STANDARD_EXTENSION()
	{
	_LIT(K, "T_I2C");

	TDynamicDfcQue* dfcQueue;
	TInt r = Kern::DynamicDfcQCreate(dfcQueue, 24, K);
	__NK_ASSERT_ALWAYS(r == KErrNone);

	I2c::TConfigPb ccb;
	ccb.iUnit = I2c::E1;
	ccb.iRole = I2c::EMaster;
	ccb.iMode = I2c::E7Bit;
	ccb.iExclusiveClient = (void*) InitExtension; 
	ccb.iRate = I2c::E400K;
	ccb.iOwnAddress = 0x01;
	ccb.iDfcQueue = dfcQueue;
	ccb.iDeviceAddress = 0x4b;
	I2c::THandle h4b = I2c::Open(ccb);
	__NK_ASSERT_ALWAYS(h4b >= KErrNone);

	ccb.iDeviceAddress = 0x49;
	I2c::THandle h49 = I2c::Open(ccb);
	__NK_ASSERT_ALWAYS(h49 >= KErrNone);

	ccb.iUnit = I2c::E3;
	ccb.iRate = I2c::E100K;
	ccb.iDeviceAddress = 0x50;
	I2c::THandle hEdid = I2c::Open(ccb);
	__NK_ASSERT_ALWAYS(hEdid >= KErrNone);

	const TUint8 KTotalRead = 128;
	const TUint8 KReadPerTransfer = 16;
	
	I2c::TTransferPb addressPhase;
	addressPhase.iType = I2c::TTransferPb::EWrite;
	addressPhase.iLength = 1;

	I2c::TTransferPb dataPhase;
	dataPhase.iType = I2c::TTransferPb::ERead;
	dataPhase.iLength = KReadPerTransfer;

	addressPhase.iNextPhase = &dataPhase; // a two phase transfer

	TUint8 data[128];

	for (TUint8 address = 0; address < KTotalRead; address += KReadPerTransfer)
		{
		addressPhase.iData = &address;
		dataPhase.iData = &data[address];

		r = I2c::TransferS(hEdid, addressPhase);
		if (r != KErrNone)
			{
			Kern::Printf("*** Check that the DVI cable is connected ***");
			break;
			}
		}

	I2c::Close(hEdid);

	for (TInt i = 0; i < KTotalRead; i += 16)
		{
		Kern::Printf("%02x: %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x",
			i, data[i + 0], data[i + 1], data[i + 2], data[i + 3], data[i + 4], data[i + 5], data[i + 6], data[i + 7],
				data[i + 8], data[i + 9], data[i + 10], data[i + 11], data[i + 12], data[i + 13], data[i + 14], data[i + 15]);
		}

	TUint8 rd = I2cReg::ReadB(h4b, 0x7a); Kern::Printf("expect 0x20:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x7a); Kern::Printf("expect 0x03:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x8E); Kern::Printf("expect 0xE0:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x91); Kern::Printf("expect 0x05:%d", rd);
	rd = I2cReg::ReadB(h49, 0x01); Kern::Printf("expect 0x03:%d", rd);
	rd = I2cReg::ReadB(h49, 0x02); Kern::Printf("expect 0xc0:%d", rd);
	rd = I2cReg::ReadB(h49, 0x03); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x04); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x05); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x06); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x07); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x08); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x09); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x0a); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x0b); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x0c); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x0d); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x0e); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x0f); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x10); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x11); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x12); Kern::Printf("expect 0x6c:%d", rd);
	rd = I2cReg::ReadB(h49, 0x13); Kern::Printf("expect 0x6c:%d", rd);
	rd = I2cReg::ReadB(h49, 0x14); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x15); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x16); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x17); Kern::Printf("expect 0x0c:%d", rd);
	rd = I2cReg::ReadB(h49, 0x18); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x19); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x1a); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x1b); Kern::Printf("expect 0x2b:%d", rd);
	rd = I2cReg::ReadB(h49, 0x1c); Kern::Printf("expect 0x2b:%d", rd);
	rd = I2cReg::ReadB(h49, 0x1d); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x1e); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x1f); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x20); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x21); Kern::Printf("expect 0x00:%d", rd);
	rd = I2cReg::ReadB(h49, 0x22); Kern::Printf("expect 0x24:%d", rd);
	rd = I2cReg::ReadB(h49, 0x23); Kern::Printf("expect 0x0a:%d", rd);
	rd = I2cReg::ReadB(h49, 0x24); Kern::Printf("expect 0x42:%d", rd);
	rd = I2cReg::ReadB(h49, 0x25); Kern::Printf("expect 0x00:%d", rd);

	rd = I2cReg::ReadB(h4b, 0x29); Kern::Printf("RTC_CTRL_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2a); Kern::Printf("RTC_STATUS_REG:%d", rd);
	I2cReg::WriteB(h4b, 0x2a, 0x80);
	rd = I2cReg::ReadB(h4b, 0x2a); Kern::Printf("RTC_STATUS_REG:%d", rd);

	rd = I2cReg::ReadB(h4b, 0x29); Kern::Printf("RTC_CTRL_REG:%d", rd);
	I2cReg::WriteB(h4b, 0x29, 0x01);
	rd = I2cReg::ReadB(h4b, 0x3f); Kern::Printf("CFG_PWRANA2:%d", rd);

	Kern::Printf("RTC_CTRL_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2a); Kern::Printf("RTC_STATUS_REG:%d", rd);

	rd = I2cReg::ReadB(h4b, 0x1c); Kern::Printf("SECONDS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x1d); Kern::Printf("MINUTES_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x1e); Kern::Printf("HOURS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x1f); Kern::Printf("DAYS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x20); Kern::Printf("MONTHS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x21); Kern::Printf("YEARS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x22); Kern::Printf("WEEKS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x23); Kern::Printf("ALARM_SECONDS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x24); Kern::Printf("ALARM_MINUTES_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x25); Kern::Printf("ALARM_HOURS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x26); Kern::Printf("ALARM_DAYS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x27); Kern::Printf("ALARM_MONTHS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x28); Kern::Printf("ALARM_YEARS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x29); Kern::Printf("RTC_CTRL_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2a); Kern::Printf("RTC_STATUS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2b); Kern::Printf("RTC_INTERRUPTS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2c); Kern::Printf("RTC_COMP_LSB_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2d); Kern::Printf("RTC_COMP_MSB_REG:%d", rd);

	TUint8 ctrl = I2cReg::ReadB(h4b, 0x29);
	ctrl &= ~0x01;
	I2cReg::WriteB(h4b, 0x29, ctrl);
	ctrl = I2cReg::ReadB(h4b, 0x29); Kern::Printf("RTC_CTRL_REG:%d", ctrl);

	I2cReg::WriteB(h4b, 0x28, 0x08);
	I2cReg::WriteB(h4b, 0x27, 0x12);
	I2cReg::WriteB(h4b, 0x26, 0x17);
	I2cReg::WriteB(h4b, 0x25, 0x10);
	I2cReg::WriteB(h4b, 0x24, 0x00);
	I2cReg::WriteB(h4b, 0x23, 0x00);
	I2cReg::WriteB(h4b, 0x22, 0x00);
	I2cReg::WriteB(h4b, 0x21, 0x08);
	I2cReg::WriteB(h4b, 0x20, 0x12);
	I2cReg::WriteB(h4b, 0x1f, 0x16);
	I2cReg::WriteB(h4b, 0x1e, 0x10);
	I2cReg::WriteB(h4b, 0x1d, 0x17);
	I2cReg::WriteB(h4b, 0x1c, 0x30);

	ctrl |= 0x01;
	I2cReg::WriteB(h4b, 0x29, ctrl);
	ctrl = I2cReg::ReadB(h4b, 0x29); Kern::Printf("RTC_CTRL_REG:%d", ctrl);

	NKern::Sleep(2000);

	rd = I2cReg::ReadB(h4b, 0x2a); Kern::Printf("RTC_STATUS_REG:%d", rd);

	ctrl |= 0x40;
	I2cReg::WriteB(h4b, 0x29, ctrl);
	ctrl = I2cReg::ReadB(h4b, 0x29); Kern::Printf("RTC_CTRL_REG:%d", ctrl);

	rd = I2cReg::ReadB(h4b, 0x1c); Kern::Printf("SECONDS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x1d); Kern::Printf("MINUTES_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x1e); Kern::Printf("HOURS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x1f); Kern::Printf("DAYS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x20); Kern::Printf("MONTHS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x21); Kern::Printf("YEARS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x22); Kern::Printf("WEEKS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x23); Kern::Printf("ALARM_SECONDS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x24); Kern::Printf("ALARM_MINUTES_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x25); Kern::Printf("ALARM_HOURS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x26); Kern::Printf("ALARM_DAYS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x27); Kern::Printf("ALARM_MONTHS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x28); Kern::Printf("ALARM_YEARS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x29); Kern::Printf("RTC_CTRL_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2a); Kern::Printf("RTC_STATUS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2b); Kern::Printf("RTC_INTERRUPTS_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2c); Kern::Printf("RTC_COMP_LSB_REG:%d", rd);
	rd = I2cReg::ReadB(h4b, 0x2d); Kern::Printf("RTC_COMP_MSB_REG:%d", rd);

	I2c::Close(h4b);
	I2c::Close(h49);
	dfcQueue->Destroy();

	return KErrNone;
	}
