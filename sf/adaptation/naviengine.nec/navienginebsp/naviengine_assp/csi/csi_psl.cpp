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


#ifdef STANDALONE_CHANNEL
#include <drivers/iic_transaction.h>
#else
#include <drivers/iic.h>
#endif

#include <drivers/iic_channel.h>
#include <assp/naviengine/naviengine.h>
#include "csi_psl.h"

#ifdef MASTER_MODE
#include "csi_master.h"
#endif
#ifdef SLAVE_MODE
#include "csi_slave.h"
#endif

#include "hcrconfig.h"
#include "hcrconfig_csi.h"

// table of pointers to the channels - to be filled with pointers to channels
// and used to register these channels with the Bus Controller
// (definition)
DIicBusChannel** ChannelPtrArray;

// EXTENSION DLL ENTRY POINT
DECLARE_EXTENSION_WITH_PRIORITY(BUS_IMPLMENTATION_PRIORITY)
	{
	TInt r = KErrNone;
#ifndef STANDALONE_CHANNEL

	HCR::TSettingId settingId;
	settingId.iCat = KHcrCat_HWServ_CSI;
	settingId.iKey = KHcrKey_CSI_NumOfChannels;
	TInt32 csiNumOfChannels;

	r = HCR::GetInt(settingId, csiNumOfChannels);
	if(r != KErrNone) 
		{
		__KTRACE_OPT(KIIC, Kern::Printf("Failed to read HCR setting for category = %d and key = %d\n", settingId.iCat, settingId.iKey));
		return r;
		}
	ChannelPtrArray = new DIicBusChannel*[csiNumOfChannels];

#ifdef MASTER_MODE
#ifndef SLAVE_MODE
	// If only MASTER_MODE is declared - Create all as DCsiChannelMaster channels
	__KTRACE_OPT(KIIC, Kern::Printf("\n\nCreating DCsiChannelMaster only"));

	DIicBusChannel* chan = NULL;
	for (TInt i = 0; i < csiNumOfChannels; ++i)
		{
		chan = DCsiChannelMaster::New(i, DIicBusChannel::ESpi, DIicBusChannel::EFullDuplex);
		if (!chan)
			{
			return KErrNoMemory;
			}
		ChannelPtrArray[i] = chan;
		}

#else /*SLAVE_MODE*/
	// both - Master and Slave channels.. or MasterSlaveChannels..
	// for now, create channel 0 as Master, and channel 1 as a Slave.
	__KTRACE_OPT(KIIC, Kern::Printf("\n\nCreating Master and Slave"));

	DIicBusChannel* chan = NULL;
	// create channel 0 - as master..
	chan = DCsiChannelMaster::New(0, DIicBusChannel::ESpi, DIicBusChannel::EFullDuplex);
	if (!chan)
		{
		return KErrNoMemory;
		}
	ChannelPtrArray[0] = chan;

	// and created channel 1 - as slave..
	chan = DCsiChannelSlave::New(1, DIicBusChannel::ESpi, DIicBusChannel::EFullDuplex);
	if (!chan)
		{
		return KErrNoMemory;
		}
	ChannelPtrArray[1] = chan;

#endif /*SLAVE_MODE*/
#else /*MASTER_MODE*/

#ifdef SLAVE_MODE
	// If only SLAVE_MODE is declared - Create all as DCsiChannelMaster channels
	__KTRACE_OPT(KIIC, Kern::Printf("\n\nCreating DCsiChannelSlave only"));

	DIicBusChannel* chan = NULL;
	for (TInt i = 0; i < csiNumOfChannels; ++i)
		{
		chan = DCsiChannelSlave::New(i, DIicBusChannel::ESpi, DIicBusChannel::EFullDuplex);
		if (!chan)
			{
			return KErrNoMemory;
			}
		ChannelPtrArray[i] = chan;
		}

#endif
#endif /*MASTER_MODE*/

	// Register them with the Bus Controller
	r = DIicBusController::RegisterChannels(ChannelPtrArray, csiNumOfChannels);
#endif/*STANDALONE_CHANNEL*/

	return r;
	}

// helper function - to dump the supplied configuration
#ifdef _DEBUG
void DumpConfiguration(const TConfigSpiBufV01& spiHeader, TInt aCsPin)
	{
#ifdef DUMP_CONFIG
	__KTRACE_OPT(KIIC, Kern::Printf("new configuration:\n"));
	__KTRACE_OPT(KIIC, Kern::Printf("iWordWidth %d", spiHeader().iWordWidth));
	__KTRACE_OPT(KIIC, Kern::Printf("iClkSpeedHz: %d", spiHeader().iClkSpeedHz));
	__KTRACE_OPT(KIIC, Kern::Printf("iClkMode %d", spiHeader().iClkMode));
	__KTRACE_OPT(KIIC, Kern::Printf("iTimeoutPeriod %d", spiHeader().iTimeoutPeriod));
	__KTRACE_OPT(KIIC, Kern::Printf("iBitOrder %d", spiHeader().iBitOrder));
	__KTRACE_OPT(KIIC, Kern::Printf("iTransactionWaitCycles %d", spiHeader().iTransactionWaitCycles));
	__KTRACE_OPT(KIIC, Kern::Printf("iSSPinActiveMode %d", spiHeader().iSSPinActiveMode));
	__KTRACE_OPT(KIIC, Kern::Printf("csPin %d\n", aCsPin));
#endif
	}
#endif

