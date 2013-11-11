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




#ifndef __CSI_MASTER_H_
#define __CSI_MASTER_H_

#include <assp/naviengine/naviengine.h>
#include <drivers/gpio.h>
#include <drivers/iic_channel.h>
#include "csi_psl.h"

_LIT(KSpiThreadName,"SpiChannelThread_");

const TInt KCsiDfcPriority = 0; // Arbitrary, can be 0-7, 7 highest
const TInt KSpiThreadPriority = 24;

// DCsiChannelMaster class declaration:
class DCsiChannelMaster: public DIicBusChannelMaster
	{
public:
#ifdef STANDALONE_CHANNEL
	IMPORT_C
#endif
	static DCsiChannelMaster* New(TInt aChannelNumber, const TBusType aBusType, const TChannelDuplex aChanDuplex);

	// gateway function for PSL implementation
	virtual TInt DoRequest(TIicBusTransaction* aTransaction);

	// overloaded constructor..
	DCsiChannelMaster(TInt aChannelNumber, const TBusType aBusType, const TChannelDuplex aChanDuplex);
#ifdef STANDALONE_CHANNEL
	~DCsiChannelMaster();
#endif
protected:
	// overriders for base pure-virtual methods..
	virtual TInt DoCreate(); // 2nd stage construction.
	virtual TInt CheckHdr(TDes8* aHdr);
	virtual TInt HandleSlaveTimeout();

private:
	// other internal methods
	TInt ConfigureInterface();
	TBool TransConfigDiffersFromPrev();
	TInt DoTransfer(TInt8 *aBuff, TUint aNumOfBytes);
	TInt StartTransfer(TIicBusTransfer* aTransferPtr, TUint8 aType);
	TInt ProcessNextTransfers();
	void ExitComplete(TInt aErr, TBool aComplete = ETrue);

	// Dfc/timeout Callback functions
	static void TransferEndDfc(TAny* aPtr);
	static void TransferTimeoutDfc(TAny* aPtr);
	static void TimeoutCallback(TAny* aPtr);

	// ISR handler.
	static void CsiIsr(TAny* aPtr);

	// DFc for Handling Transfer complete
	TDfc iTransferEndDfc;

	// flags indicating transmission/operation
	TCsiOperationType iOperation;

	// granularity..
	TUint8 iWordSize;

	// to store currently used CS pin number
	TUint8 iSSPin;

	// current state
	enum TMyState
		{
		EIdle,
		EBusy
		};
	TMyState iState;

	// as implementation is common/generic for both channels
	// store base register address and interruptID
	TUint iChannelBase;
	TInt iInterruptId;

	// pointers used to store current transfers information
	TIicBusTransfer* iHalfDTransfer;
	TIicBusTransfer* iFullDTransfer;

	// and current transaction.
	TIicBusTransaction* iCurrTransaction;

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

	// to store current configuration (for current transaction)
	TConfigSpiV01 iSpiHeader;

	// and its active/inactive state (can be active on high or low)
	GPIO::TGpioState iSSPinActiveStateOff;
	GPIO::TGpioState iSSPinActiveStateOn;

	// if the Master is a part of DIicBusChannelMasterSlave object,
	// we allow the friend class to unbind master's ISR - and register its own
	// for the time the channel is captured by the Slave.
	friend class DCsiChannelSlave;
	};

#endif //__CSI_MASTER_H_
