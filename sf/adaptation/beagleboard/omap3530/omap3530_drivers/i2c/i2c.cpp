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
// omap3530/omap3530_drivers/i2c/i2c.cpp
// I2C Driver
// Main interface, I2c, is declared in omap3530_i2c.h
// A more restricted register orientated interface, I2cReg, is declared in omap3530_i2creg.h
// This file is part of the Beagle Base port
//

#include <assp/omap3530_assp/omap3530_i2creg.h>

#include <assp/omap3530_assp/omap3530_irqmap.h>
#include <assp/omap3530_assp/omap3530_hardware_base.h>
#include <assp/omap3530_assp/omap3530_ktrace.h>
//#include <assp/omap3530_assp/omap3530_prm.h>
#include <assp/omap3530_assp/omap3530_prcm.h>
#include <nk_priv.h>
#include <nklib.h>
//#include <resourceman.h>

#ifdef USE_SYMBIAN_PRM
_LIT(KDfcName, "I2C_DFC"); // Not used by the I2c dfc!
#endif

DECLARE_STANDARD_EXTENSION()
	{
	return KErrNone;
	}

namespace I2c
{
const TInt KMaxDevicesPerUnit = 8; // arbitary - change if required
const TInt KNumUnits = E3 + 1;

// Each unit has KMaxDevicesPerUnit of these structures. At least one for each slave device on it's bus.
struct TDeviceControl
	{
	TDeviceAddress iAddress; // the slave devices address; 7 or 10 bits
	TDfcQue* iDfcQueue; // calling driver's DFC thread
	NFastSemaphore iSyncSem; // used to block the calling thread during synchronous transfers
	};

// There are three instances of this structure - one for each I2C bus on the OMAP3530
struct TUnitControl
	{
	TUnitControl();

	TSpinLock iLock; // prevents concurrent access to the request queue
	DMutex*	iOpenMutex;

	enum
		{
		EIdle,
		ERead,
		EWrite
		} iState;

	// Configuration stored and checked during Open()
	TRole iRole;
	TMode iMode;
	void* iExclusiveClient;
	TRate iRate;
	TDeviceAddress iOwnAddress;

	// The DFC for this unit - it runs on the thread associated with the active transfer
	TDfc iDfc;


	// the slave devices on this unit's bus
	TDeviceControl iDevice[KMaxDevicesPerUnit];
	TInt iNumDevices;

	// The queue of requested transfers - the active transfer is the head of the queue
	TTransferPb* iTransferQ;
	TTransferPb* iTransferQTail; // the last transfer on the queue

	// the current phase of the sctive transfer
	TTransferPb* iCurrentPhase;
	};

// The OMAP3530 register address
const TUint KI2C_IE[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070004>::Value, Omap3530HwBase::TVirtual<0x48072004>::Value, Omap3530HwBase::TVirtual<0x48060004>::Value};
const TUint KI2C_STAT[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070008>::Value, Omap3530HwBase::TVirtual<0x48072008>::Value, Omap3530HwBase::TVirtual<0x48060008>::Value};
//const TUint KI2C_WE[KNumUnits] =
//	{Omap3530HwBase::TVirtual<0x4807000C>::Value, Omap3530HwBase::TVirtual<0x4807200C>::Value, Omap3530HwBase::TVirtual<0x4806000C>::Value};
const TUint KI2C_SYSS[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070010>::Value, Omap3530HwBase::TVirtual<0x48072010>::Value, Omap3530HwBase::TVirtual<0x48060010>::Value};
const TUint KI2C_BUF[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070014>::Value, Omap3530HwBase::TVirtual<0x48072014>::Value, Omap3530HwBase::TVirtual<0x48060014>::Value};
const TUint KI2C_CNT[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070018>::Value, Omap3530HwBase::TVirtual<0x48072018>::Value, Omap3530HwBase::TVirtual<0x48060018>::Value};
const TUint KI2C_DATA[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x4807001C>::Value, Omap3530HwBase::TVirtual<0x4807201C>::Value, Omap3530HwBase::TVirtual<0x4806001C>::Value};
const TUint KI2C_SYSC[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070020>::Value, Omap3530HwBase::TVirtual<0x48072020>::Value, Omap3530HwBase::TVirtual<0x48060020>::Value};
const TUint KI2C_CON[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070024>::Value, Omap3530HwBase::TVirtual<0x48072024>::Value, Omap3530HwBase::TVirtual<0x48060024>::Value};
//const TUint KI2C_OA0[KNumUnits] =
//	{Omap3530HwBase::TVirtual<0x48070028>::Value, Omap3530HwBase::TVirtual<0x48072028>::Value, Omap3530HwBase::TVirtual<0x48060028>::Value};
const TUint KI2C_SA[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x4807002C>::Value, Omap3530HwBase::TVirtual<0x4807202C>::Value, Omap3530HwBase::TVirtual<0x4806002C>::Value};
const TUint KI2C_PSC[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070030>::Value, Omap3530HwBase::TVirtual<0x48072030>::Value, Omap3530HwBase::TVirtual<0x48060030>::Value};
const TUint KI2C_SCLL[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070034>::Value, Omap3530HwBase::TVirtual<0x48072034>::Value, Omap3530HwBase::TVirtual<0x48060034>::Value};
const TUint KI2C_SCLH[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070038>::Value, Omap3530HwBase::TVirtual<0x48072038>::Value, Omap3530HwBase::TVirtual<0x48060038>::Value};
//const TUint KI2C_SYSTEST[KNumUnits] =
//	{Omap3530HwBase::TVirtual<0x4807003C>::Value, Omap3530HwBase::TVirtual<0x4807203C>::Value, Omap3530HwBase::TVirtual<0x4806003C>::Value};
const TUint KI2C_BUFSTAT[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070040>::Value, Omap3530HwBase::TVirtual<0x48072040>::Value, Omap3530HwBase::TVirtual<0x48060040>::Value};
const TUint KI2C_OA1[KNumUnits] =
	{Omap3530HwBase::TVirtual<0x48070044>::Value, Omap3530HwBase::TVirtual<0x48072044>::Value, Omap3530HwBase::TVirtual<0x48060044>::Value};
//const TUint KI2C_OA2[KNumUnits] =
//	{Omap3530HwBase::TVirtual<0x48070048>::Value, Omap3530HwBase::TVirtual<0x48072048>::Value, Omap3530HwBase::TVirtual<0x48060048>::Value};
//const TUint KI2C_OA3[KNumUnits] =
//	{Omap3530HwBase::TVirtual<0x4807004C>::Value, Omap3530HwBase::TVirtual<0x4807204C>::Value, Omap3530HwBase::TVirtual<0x4806004C>::Value};
//const TUint KI2C_ACTOA[KNumUnits] =
//	{Omap3530HwBase::TVirtual<0x48070050>::Value, Omap3530HwBase::TVirtual<0x48072050>::Value, Omap3530HwBase::TVirtual<0x48060050>::Value};
//const TUint KI2C_SBLOCK[KNumUnits] =
//	{Omap3530HwBase::TVirtual<0x48070054>::Value, Omap3530HwBase::TVirtual<0x48072054>::Value, Omap3530HwBase::TVirtual<0x48060054>::Value};
const TUint KCM_ICLKEN1_CORE = Omap3530HwBase::TVirtual<0x48004A10>::Value;
const TUint KCM_FCLKEN1_CORE = Omap3530HwBase::TVirtual<0x48004A00>::Value;

// the Id's used when binding the interrupts
const TOmap3530_IRQ KIrqId[KNumUnits] = {EOmap3530_IRQ56_I2C1_IRQ, EOmap3530_IRQ57_I2C2_IRQ, EOmap3530_IRQ61_I2C3_IRQ};

// The three unit control blocks; one for each unit
TUnitControl gUcb[KNumUnits];
//TUint prmClientId;

TUnit RawUnit(THandle aHandle);
TUnit Unit(THandle aHandle);
TUnitControl& UnitCb(THandle aHandle);
TDeviceAddress Device(THandle aHandle);
TDeviceControl& DeviceCb(THandle aHandle);
THandle Handle(TUnit aUnit, TDeviceAddress aDeviceAddress);
void Complete(TUnitControl& aUnit, TInt aResult);
void Configure(TUnit); // reset and configure an I2C unit
void Deconfigure(TUnit);
void TheIsr(void*);
void TheDfc(TAny* aUnit);

EXPORT_C TConfigPb::TConfigPb() :
	iUnit((TUnit)-1), // ensure that an un-initialised cpb will return KErrArgument from Open()
	iExclusiveClient(0),
	iDeviceAddress(1)
	{}

EXPORT_C TTransferPb::TTransferPb() :
	iNextPhase(0)
	{}

EXPORT_C THandle Open(const TConfigPb& aConfig)
	{
#ifdef USE_SYMBIAN_PRM
	//TInt r = PowerResourceManager::RegisterClient( prmClientId, KDfcName );
	//__NK_ASSERT_ALWAYS(r==KErrNone);
#error FIXME: the DFC, along with the associated dfcq have to be created (e.g. in DLL entry point?)
#endif
	THandle h;
	__NK_ASSERT_ALWAYS(aConfig.iVersion == I2C_VERSION);
	if (aConfig.iUnit >= E1 && aConfig.iUnit <= E3)
		{
		TUnitControl& unit = gUcb[aConfig.iUnit];
		Kern::MutexWait( *unit.iOpenMutex );

		if (unit.iNumDevices == 0)
			{
			if (aConfig.iRole == EMaster &&
				aConfig.iMode == E7Bit &&
				aConfig.iRate >= E100K && aConfig.iRate <= E400K)
				{
				unit.iRole = aConfig.iRole;
				unit.iMode = aConfig.iMode;
				unit.iExclusiveClient = aConfig.iExclusiveClient;
				unit.iRate = aConfig.iRate;
				unit.iDevice[unit.iNumDevices].iAddress = aConfig.iDeviceAddress;
				unit.iDevice[unit.iNumDevices++].iDfcQueue = aConfig.iDfcQueue;
				h = Handle(aConfig.iUnit, aConfig.iDeviceAddress);
				Configure(aConfig.iUnit);
				TInt r = Interrupt::Bind(KIrqId[aConfig.iUnit], TheIsr, (void*) aConfig.iUnit);
				__NK_ASSERT_DEBUG(r == KErrNone);
				}
			else
				{
				h = KErrArgument;
				}
			}
		else // unit is already open
			{
			if (unit.iNumDevices < KMaxDevicesPerUnit)
				{
				if (unit.iRole == aConfig.iRole &&
						unit.iMode == aConfig.iMode &&
						unit.iExclusiveClient == aConfig.iExclusiveClient &&
						unit.iRate == aConfig.iRate)
					{
					h = 0;
					for (TInt i = 0; i < unit.iNumDevices; i++)
						{
						if (unit.iDevice[i].iAddress == aConfig.iDeviceAddress)
							{
							h = KErrInUse;
							break;
							}
						}
					if (h == 0)
						{
						unit.iDevice[unit.iNumDevices].iAddress = aConfig.iDeviceAddress;
						unit.iDevice[unit.iNumDevices++].iDfcQueue = aConfig.iDfcQueue;
						h = Handle(aConfig.iUnit, aConfig.iDeviceAddress);
						}
					}
				else
					{
					h = KErrInUse;
					}
				}
			else
				{
				h = KErrTooBig;
				}
			}
		Kern::MutexSignal( *unit.iOpenMutex );
		}
	else
		{
		h = KErrArgument;
		}
	return h;
	}

EXPORT_C void Close(THandle& aHandle)
	{
	TUnit unitI = RawUnit(aHandle);

	if (unitI >= E1 && unitI <= E3)
		{
		TUnitControl& unit = gUcb[unitI];
		Kern::MutexWait( *unit.iOpenMutex );

		TInt i = 0;
		for (; i < unit.iNumDevices; i++)
			{
			if (unit.iDevice[i].iAddress == Device(aHandle))
				{
				unit.iNumDevices--;
				break;
				}
			}
		for (; i < unit.iNumDevices; i++)
			{
			unit.iDevice[i] = unit.iDevice[i + 1];
			}

		if (unit.iNumDevices == 0)
			{
			(void) Interrupt::Unbind(KIrqId[unitI]);
			Deconfigure(TUnit(unitI));
			}
		Kern::MutexSignal( *unit.iOpenMutex );
		}
	aHandle = -1;
	//PowerResourceManager::DeRegisterClient(prmClientId);
	//prmClientId=0;
	}

void AddToQueue( TUnitControl& aUnit, TDeviceControl& aDcb, TTransferPb& aWcb )
	{
	TInt irq = __SPIN_LOCK_IRQSAVE(aUnit.iLock);

	if (aUnit.iTransferQ == 0)
		{
		__NK_ASSERT_DEBUG(aUnit.iState == TUnitControl::EIdle);
		aUnit.iTransferQ = &aWcb;
		aUnit.iCurrentPhase = &aWcb;
		aUnit.iTransferQTail = &aWcb;
		aUnit.iDfc.SetDfcQ(aDcb.iDfcQueue);
		aUnit.iDfc.Enque();
		}
	else
		{
		__NK_ASSERT_DEBUG(aUnit.iTransferQTail->iNextTransfer == 0);
		aUnit.iTransferQTail->iNextTransfer = &aWcb;
		aUnit.iTransferQTail = &aWcb;
		}
	__SPIN_UNLOCK_IRQRESTORE(unit.iLock, irq);
	}


EXPORT_C TInt TransferS(THandle aHandle, TTransferPb& aWcb)
	{
	__KTRACE_OPT(KI2C, __KTRACE_OPT(KI2C, Kern::Printf("+I2C:TransferS")));

	CHECK_PRECONDITIONS(MASK_NOT_ISR, "I2c::TransferS");

	aWcb.iNextTransfer = 0;
	aWcb.iCompletionDfc = 0; // indicate that it is a sync transfer and the FSM needs to Signal the semaphore
	TDeviceControl& dcb = DeviceCb(aHandle);
	aWcb.iDcb = &dcb;
	aWcb.iResult = (TInt)&dcb.iSyncSem; // use the async tranfer result member to store the semaphore // Todo: store ptr to dcb in aWcb 

	NKern::FSSetOwner(&dcb.iSyncSem, 0);

	TUnitControl& unit = UnitCb(aHandle);

	AddToQueue( unit, dcb, aWcb );

	NKern::FSWait(&dcb.iSyncSem);

	__KTRACE_OPT(KI2C, __KTRACE_OPT(KI2C, Kern::Printf("-I2C:TransferS:%d", aWcb.iResult)));

	return aWcb.iResult;
	}

EXPORT_C void TransferA(THandle aHandle, TTransferPb& aWcb)
	{
	__KTRACE_OPT(KI2C, __KTRACE_OPT(KI2C, Kern::Printf("+I2C:TransferA")));

	CHECK_PRECONDITIONS(MASK_NOT_ISR, "I2c::TransferA");

	aWcb.iNextTransfer = 0;
	TDeviceControl& dcb = DeviceCb(aHandle);
	aWcb.iDcb = &dcb;
	TUnitControl& unit = UnitCb(aHandle);

	AddToQueue( unit, dcb, aWcb );

	__KTRACE_OPT(KI2C, __KTRACE_OPT(KI2C, Kern::Printf("-I2C:TransferA")));
	}

EXPORT_C void CancelATransfer(THandle)
	{
	}

inline TBool BitSet(TUint32 aWord, TUint32 aMask)
	{
	return (aWord & aMask) != 0;
	}
const TUint32 KStatBb = 1 << 12;
const TUint32 KStatNack = 1 << 1;
const TUint32 KStatAl = 1 << 0;
const TUint32 KStatArdy = 1 << 2;
const TUint32 KStatRdr = 1 << 13;
const TUint32 KStatRRdy = 1 << 3;
const TUint32 KStatXdr = 1 << 14;
const TUint32 KStatXrdy = 1 << 4;
const TUint32 KStatBf = 1 << 8;
const TUint32 KStatInterupts = KStatXdr | KStatRdr | KStatBf | KStatXrdy | KStatRRdy | KStatArdy | KStatNack | KStatAl;

const TUint32 KConMst = 1 << 10;
const TUint32 KConI2cEn = 1 << 15;
const TUint32 KConTrx = 1 << 9;
const TUint32 KConStp = 1 << 1;
const TUint32 KConStt = 1 << 0;

void TheDfc(TAny* aUnit)
	{
	TUnit unitI = (TUnit)(TInt)aUnit;
	TUnitControl& unit = gUcb[unitI];

	__KTRACE_OPT(KI2C, __KTRACE_OPT(KI2C, Kern::Printf("I2C:DFC:S%d", unit.iState)) );

	switch (unit.iState)
		{
	case TUnitControl::EIdle:
		{
// 18.5.1.1.2
// 1
		TTransferPb& tpb = *unit.iTransferQ;
		TTransferPb& ppb = *unit.iCurrentPhase;

		TUint32 con = KConI2cEn | KConMst;
		if (ppb.iType == TTransferPb::EWrite)
			{
			con |= KConTrx;
			}
		AsspRegister::Write16(KI2C_CON[unitI], con);
// 18.5.1.1.3
		TUint32 sa = AsspRegister::Read16(KI2C_SA[unitI]);
		__KTRACE_OPT(KI2C, Kern::Printf("I2C:SA[%d]: 0x%04x<-0x%04x", unitI, sa, tpb.iDcb->iAddress));
		AsspRegister::Write16(KI2C_SA[unitI], tpb.iDcb->iAddress);
		TUint32 cnt = AsspRegister::Read16(KI2C_CNT[unitI]);
		__KTRACE_OPT(KI2C, Kern::Printf("I2C:CNT[%d]: 0x%04x<-0x%04x", unitI, cnt, ppb.iLength));
		AsspRegister::Write16(KI2C_CNT[unitI], ppb.iLength);
// 18.5.1.1.4
		if (ppb.iNextPhase == 0) // last phase
			{
			con |= KConStp; // STP
			}
		con |= KConStt; // STT			
		if (&tpb == &ppb) // first phase
			{
			TInt im = NKern::DisableAllInterrupts(); // ensure that the transaction is started while the bus is free
			TUint32 stat = AsspRegister::Read16(KI2C_STAT[unitI]);
			__KTRACE_OPT(KI2C, Kern::Printf("I2C:STAT[%d]: 0x%04x", unitI, stat));
			__NK_ASSERT_ALWAYS(!BitSet(stat, KStatBb)); // if multi-master then need a polling state with a timeout
			AsspRegister::Write16(KI2C_CON[unitI], con);
			NKern::RestoreInterrupts(im);
			}
		else // a follow on phase
			{
			AsspRegister::Write16(KI2C_CON[unitI], con);
			}
		__KTRACE_OPT(KI2C, Kern::Printf("I2C:CON[%d]: 0x%04x", unitI, con));
__KTRACE_OPT(KI2C, Kern::Printf("I2C:..CNT[%d]: 0x%04x", unitI, AsspRegister::Read16(KI2C_CNT[unitI])));

		if (ppb.iType == TTransferPb::ERead)
			{
			unit.iState = TUnitControl::ERead;
			}
		else
			{
			unit.iState = TUnitControl::EWrite;
			}
		}
		break;
	case TUnitControl::ERead:
	case TUnitControl::EWrite:
		{
		TTransferPb& ppb = *unit.iCurrentPhase;
		TUint32 stat = AsspRegister::Read16(KI2C_STAT[unitI]);
		__KTRACE_OPT(KI2C, Kern::Printf("I2C:STAT[%d]: 0x%04x", unitI, stat));
		do
			{
			if (BitSet(stat, KStatNack))
				{
				__KTRACE_OPT(KI2C, Kern::Printf("I2C:N"));
				Configure(unitI); // reset the whole unit. Need more testing to determine the correct behavior.
				__KTRACE_OPT(KI2C, Kern::Printf("I2C:CON|STAT[%d]: 0x%04x|0x%04x", unitI, AsspRegister::Read16(KI2C_CON[unitI]), AsspRegister::Read16(KI2C_STAT[unitI])));
				Complete(unit, KErrGeneral);
				return;
				}
			if (BitSet(stat, KStatAl))
				{
				__KTRACE_OPT(KI2C, Kern::Printf("I2C:A"));
				AsspRegister::Write16(KI2C_STAT[unitI], KStatAl);
				
				if((AsspRegister::Read16(KI2C_CON[unitI]) & (KConMst | KConStp)) == 0)
					{
					AsspRegister::Modify16(KI2C_CON[unitI], KClearNone, KConStp);
					Complete(unit, KErrGeneral);
					return;
					}
				}
			if (BitSet(stat, KStatArdy))
				{
				__KTRACE_OPT(KI2C, Kern::Printf("I2C:Y"));
				AsspRegister::Write16(KI2C_STAT[unitI], KStatArdy);

				if (ppb.iNextPhase != 0)
					{
					unit.iCurrentPhase = ppb.iNextPhase;
					unit.iState = TUnitControl::EIdle;
					unit.iDfc.Enque();
					return;
					}
				else
					{
					Complete(unit, KErrNone);
					return;
					}
				}
			if (BitSet(stat, KStatRdr))
				{
				__NK_ASSERT_DEBUG(unit.iState == TUnitControl::ERead);
				TUint32 rxstat = AsspRegister::Read16(KI2C_BUFSTAT[unitI]) >> 8;
				rxstat &= 0x3f;
				__KTRACE_OPT(KI2C, Kern::Printf("I2C:R%d", rxstat));
				for (TUint i = 0; i < rxstat; i++)
					{
					TUint8* d = const_cast<TUint8*>(ppb.iData++);
					*d = (TUint8) AsspRegister::Read16(KI2C_DATA[unitI]);
					}
				AsspRegister::Write16(KI2C_STAT[unitI], KStatRdr);
				}
			else if (BitSet(stat, KStatRRdy))
				{
				__KTRACE_OPT(KI2C, Kern::Printf("I2C:..BUF:%x BUFSTAT:%x", AsspRegister::Read16(KI2C_BUF[unitI]), AsspRegister::Read16(KI2C_BUFSTAT[unitI])));
				__NK_ASSERT_DEBUG(unit.iState == TUnitControl::ERead);
				TUint32 rtrsh = AsspRegister::Read16(KI2C_BUF[unitI]) >> 8;
				rtrsh &= 0x3f;
				__KTRACE_OPT(KI2C, Kern::Printf("I2C:RD%d", rtrsh + 1));
				for (TUint i = 0; i < rtrsh + 1; i++)
					{
					TUint8* d = const_cast<TUint8*>(ppb.iData++);
					*d = (TUint8) AsspRegister::Read16(KI2C_DATA[unitI]);
					}
				AsspRegister::Write16(KI2C_STAT[unitI], KStatRRdy);
				}
			if (BitSet(stat, KStatXdr))
				{
				__NK_ASSERT_DEBUG(unit.iState == TUnitControl::EWrite);
				TUint32 txstat = AsspRegister::Read16(KI2C_BUFSTAT[unitI]);
				txstat &= 0x3f;
				__KTRACE_OPT(KI2C, Kern::Printf("I2C:W%d", txstat));
				for (TUint i = 0; i < txstat; i++)
					{
					AsspRegister::Write16(KI2C_DATA[unitI], *ppb.iData++);
					}
				AsspRegister::Write16(KI2C_STAT[unitI], KStatXdr);
				}
			else if (BitSet(stat, KStatXrdy))
				{
				__NK_ASSERT_DEBUG(unit.iState == TUnitControl::EWrite);
				TUint32 xtrsh = AsspRegister::Read16(KI2C_BUF[unitI]);
				xtrsh &= 0x3f;
				__KTRACE_OPT(KI2C, Kern::Printf("I2C:WD%d", xtrsh + 1));
				for (TUint i = 0; i < xtrsh + 1; i++)
					{
					AsspRegister::Write16(KI2C_DATA[unitI], *ppb.iData++);
					}
				AsspRegister::Write16(KI2C_STAT[unitI], KStatXrdy);
				}
/*			if (stat == KStatBf)
				{
				__KTRACE_OPT(KI2C, Kern::Printf("F"));
				__NK_ASSERT_ALWAYS(ppb.iNextPhase == 0);
				AsspRegister::Write16(KI2C_STAT[unitI], KStatBf);
				Complete(unit, KErrNone);
				return;
				}
*/			stat = AsspRegister::Read16(KI2C_STAT[unitI]);
			__KTRACE_OPT(KI2C, Kern::Printf("I2C:STAT[%d]: 0x%04x", unitI, stat));
			} while (BitSet(stat, KStatInterupts));
		}
		break;
		}
	Interrupt::Enable(KIrqId[unitI]);
	}

TUnitControl::TUnitControl() :
	iLock(TSpinLock::EOrderGenericIrqLow1),
	iDfc(TheDfc, 0, 1),
	iNumDevices(0),
	iTransferQ(0)
	{
	iDfc.iPtr = (void*)(this - gUcb); // unit index
	__ASSERT_ALWAYS( Kern::MutexCreate( iOpenMutex, KNullDesC, KMutexOrdGeneral0 ) == KErrNone, Kern::Fault( "I2C", __LINE__ ) );
	}

void TheIsr(void* aUnit)
	{
	TUnit unitI = (TUnit)(TInt)aUnit;
	Interrupt::Disable(KIrqId[unitI]);

	TUnitControl& unit = gUcb[unitI];
	__KTRACE_OPT(KI2C, __KTRACE_OPT(KI2C, Kern::Printf("=I2C:DFC:u%x", &unit )));
	unit.iDfc.Add();
	}

void Configure(TUnit aUnitI)
	{
	__ASSERT_NO_FAST_MUTEX;
	__NK_ASSERT_ALWAYS(aUnitI<3);
// 18.5.1.1.1
// 1
	//TInt r = PowerResourceManager::ChangeResourceState( prmClientId, Omap3530Prm::EPrmClkI2c1_F+aUnitI, Prcm::EClkOn );
	//r = PowerResourceManager::ChangeResourceState( prmClientId, Omap3530Prm::EPrmClkI2c1_I+aUnitI, Prcm::EClkOn );
	TUint32 iClkEn = AsspRegister::Read16(KCM_ICLKEN1_CORE);
	TUint32 fClkEn = AsspRegister::Read16(KCM_FCLKEN1_CORE);
	__KTRACE_OPT(KI2C, Kern::Printf("I2C:CM_I|FCLKEN1[%d]: 0x%04x|0x%04x", aUnitI, iClkEn, fClkEn));
	AsspRegister::Modify32(KCM_ICLKEN1_CORE, 0, 1 << 15 + aUnitI);
	AsspRegister::Modify32(KCM_FCLKEN1_CORE, 0, 1 << 15 + aUnitI);
// Reset
	AsspRegister::Write16(KI2C_SYSC[aUnitI], 0x0002);

	if (gUcb[aUnitI].iRate == E100K)
		{
// 2
		AsspRegister::Write16(KI2C_PSC[aUnitI], 23);
// 3 + 4
		AsspRegister::Write16(KI2C_SCLL[aUnitI], 0x000d); // 100kHz F/S, 400kHz HS
		AsspRegister::Write16(KI2C_SCLH[aUnitI], 0x000f);
		}
	else if (gUcb[aUnitI].iRate == E400K)
		{
// 2
		AsspRegister::Write16(KI2C_PSC[aUnitI], 9);
// 3 + 4
		AsspRegister::Write16(KI2C_SCLL[aUnitI], 0x0005); // 400kHz F/S, 400kHz HS
		AsspRegister::Write16(KI2C_SCLH[aUnitI], 0x0007);
		}
// 6
	AsspRegister::Write16(KI2C_OA1[aUnitI], gUcb[aUnitI].iOwnAddress);
// 7
	TUint32 buf = AsspRegister::Read16(KI2C_BUF[aUnitI]);
	__KTRACE_OPT(KI2C, Kern::Printf("I2C:I2C_BUF[%d]: 0x%04x", aUnitI, buf));
// 8
	TUint32 con = AsspRegister::Read16(KI2C_CON[aUnitI]);
	__KTRACE_OPT(KI2C, Kern::Printf("I2C:I2C_CON[%d]: 0x%04x<-0x%04x", aUnitI, con, con | 1 << 15));
	AsspRegister::Modify16(KI2C_CON[aUnitI], 0, 1 << 15);

	TUint32 syss = AsspRegister::Read16(KI2C_SYSS[aUnitI]);
	__NK_ASSERT_DEBUG(syss == 0x1);

	// set-up interrupts
	TUint32 ie = AsspRegister::Read16(KI2C_IE[aUnitI]);
	__KTRACE_OPT(KI2C, Kern::Printf("I2C:IE[%d]: 0x%04x<-0x%04x", aUnitI, ie, KStatInterupts));
	AsspRegister::Write16(KI2C_IE[aUnitI], KStatInterupts);
	}

void Deconfigure(TUnit aUnitI)
	{
	__ASSERT_NO_FAST_MUTEX;
	__NK_ASSERT_ALWAYS(aUnitI<3);
	//TInt r = PowerResourceManager::ChangeResourceState( prmClientId, Omap3530Prm::EPrmClkI2c1_F+aUnitI, Prcm::EClkOff  );
	//__KTRACE_OPT(KBOOT, Kern::Printf("EPrmClkI2c%d_F DIS %d", aUnitI, r));
	//r = PowerResourceManager::ChangeResourceState( prmClientId, Omap3530Prm::EPrmClkI2c1_I+aUnitI, Prcm::EClkOff );
	//__KTRACE_OPT(KBOOT, Kern::Printf("EPrmClkI2c%d_I DIS %d", aUnitI, r));
	AsspRegister::Modify32(KCM_ICLKEN1_CORE, 1 << 15 + aUnitI, 0);
	AsspRegister::Modify32(KCM_FCLKEN1_CORE, 1 << 15 + aUnitI, 0);
	}

THandle Handle(TUnit aUnit, TDeviceAddress aDeviceAddress)
	{
	return THandle(aUnit << 16 | aDeviceAddress);
	}

TUnit RawUnit(THandle aHandle)
	{
	TUnit r = TUnit(aHandle >> 16);
	return r;
	}

TUnit Unit(THandle aHandle)
	{
	TUnit r = RawUnit(aHandle);
	if (r < E1 || r > E3)
		{
		__KTRACE_OPT(KI2C, Kern::Printf("I2C Unit out of range: %d", r));
		r = E1;
		}
	return r;
	}

TUnitControl& UnitCb(THandle aHandle)
	{
	return gUcb[Unit(aHandle)];
	}

TDeviceAddress Device(THandle aHandle)
	{
	TDeviceAddress r = TDeviceAddress(aHandle & 0x0000ffff);
	if (r < 0 || r > 1023)
		{
		__KTRACE_OPT(KI2C, Kern::Printf("I2C Device out of range: %d", r));
		}
	return r;
	}

TDeviceControl& DeviceCb(THandle aHandle)
	{
	TUnitControl& unit = UnitCb(aHandle);
	TDeviceAddress device = Device(aHandle);
	TInt i = 0;
	for (; i < unit.iNumDevices; i++)
		{
		if (unit.iDevice[i].iAddress == device)
			{
			break;
			}
		}
	return unit.iDevice[i];
	}

void Complete(TUnitControl& aUnit, TInt aResult)
	{
	aUnit.iTransferQ->iResult = aResult;
	aUnit.iState = TUnitControl::EIdle;

	TInt irq = __SPIN_LOCK_IRQSAVE(aUnit.iLock);
	TTransferPb& tpb = *aUnit.iTransferQ;
	aUnit.iTransferQ = aUnit.iTransferQ->iNextTransfer;
	__SPIN_UNLOCK_IRQRESTORE(aUnit.iLock, irq);

	if (tpb.iCompletionDfc == 0)
		{
		NKern::FSSignal(&tpb.iDcb->iSyncSem);
		}
	else
		{
		tpb.iCompletionDfc->Enque();
		}	
	}

} // namespace I2c

namespace I2cReg
{
EXPORT_C TUint8 ReadB(I2c::THandle aH, TUint8 aAddr)
	{
	const TUint8 KAddress = aAddr;
	I2c::TTransferPb addressPhase;
	addressPhase.iType = I2c::TTransferPb::EWrite;
	addressPhase.iLength = 1;
	addressPhase.iData = &KAddress;

	TUint8 readData;
	I2c::TTransferPb dataPhase;
	dataPhase.iType = I2c::TTransferPb::ERead;
	dataPhase.iLength = 1;
	dataPhase.iData = &readData;

	addressPhase.iNextPhase = &dataPhase; // link into a two phase transfer

	TInt r = KErrNone;
	TInt retryCount = 0;

	do
		{
		r=I2c::TransferS(aH, addressPhase);
		retryCount++;
		}
	while (r != KErrNone && retryCount < 5);
	
	__NK_ASSERT_ALWAYS(r == KErrNone);
	
	return readData;
	}

EXPORT_C void WriteB(I2c::THandle aH, TUint8 aAddr, TUint8 aData)
	{
	const TUint8 KAddrData[2] = {aAddr, aData};
	I2c::TTransferPb fullTransfer;
	fullTransfer.iType = I2c::TTransferPb::EWrite;
	fullTransfer.iLength = 2;
	fullTransfer.iData = KAddrData;
	
	TInt r = KErrNone;
	TInt retryCount = 0;

	do
		{
		r=I2c::TransferS(aH, fullTransfer);
		retryCount++;
		}
	while (r != KErrNone && retryCount < 5);

	__NK_ASSERT_ALWAYS(r == KErrNone);
	}
} // namespace I2cReg
