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



#ifndef __CSI_SLAVE_H__
#define __CSI_SLAVE_H__

#include <assp/naviengine/naviengine.h>
#include <drivers/iic.h>
#include <drivers/iic_channel.h>
#ifdef MASTER_MODE
#include "csi_master.h"
#endif
class DCsiChannelSlave: public DIicBusChannelSlave
	{

public:
#ifdef STANDALONE_CHANNEL
	IMPORT_C
#endif
	static DCsiChannelSlave* New(TInt aChannelNumber, const TBusType aBusType,
					const TChannelDuplex aChanDuplex);
	// Gateway function for PSL implementation
	virtual TInt DoRequest(TInt aOperation);

	DCsiChannelSlave(TInt aChannelNumber, const TBusType aBusType,
	                 const TChannelDuplex aChanDuplex);

protected:
	// overriders for base pure-virtual methods..
	virtual TInt DoCreate(); // second-stage construction,
	virtual TInt CheckHdr(TDes8* aHdrBuff);
	virtual void ProcessData(TInt aTrigger, TIicBusSlaveCallback* aCb);

private:
	// some internal methods..
	TBool TransConfigDiffersFromPrev();
	TInt ConfigureInterface();
	TInt InitTransfer();

	// ISR handler and other static methods..
	static void CsiIsr(TAny* aPtr);
	static void TimeoutCallback(TAny* aPtr);
	static inline void NotifyClientEnd(DCsiChannelSlave* aPtr);

	// as implementation is common/generic for both channels
	// store base register address and interruptID
	TUint iChannelBase;
	TInt iInterruptId;

	// to store SS pin active mode (1-active on high, 0 - active on low)
	TInt8 iSSPinActiveMode;
	TUint8 iTrigger;

	// granularity..
	TInt8 iWordSize;

	// flags indicating transmission/operation competition
	TInt8 iInProgress;

	// pointers to buffers used to transfer data from/to...
	TInt8 *iTxData;
	TInt8 *iRxData;
	TInt8 *iTxDataEnd;
	TInt8 *iRxDataEnd;

	// Timer to guard 'while' loops..
	NTimer iHwGuardTimer;
	// guard-timer timeout value (in ms)
	TInt32 iHwTimeoutValue;

	// status of the transaction
	volatile TInt iTransactionStatus;

#ifdef MASTER_MODE
public:
	DCsiChannelMaster *iMasterAddress;
#endif
	};



#endif /*__CSI_SLAVE_H__*/
