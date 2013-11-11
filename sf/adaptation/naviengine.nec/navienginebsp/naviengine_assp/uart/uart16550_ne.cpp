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
* naviengine_assp\uart\uart16550.cpp
* PDD for 16550 UART
*
*/



#include <drivers/comm.h>
#include <assp.h>
#include "../naviengine_priv.h"
#include "../naviengine.h"
#include "uart16550_ne.h"
#include <e32hal.h>

_LIT(KPddName,"Comm.16550_ne");

#define __COMMS_MACHINE_CODED__
#ifdef __COMMS_MACHINE_CODED__
#define DBASE_VPTR_OFFSET		 4
#define RX_ISR_VT_OFFSET		 0x24
#define CHK_TXB_VT_OFFSET		 0x28
#define STATE_ISR_VT_OFFSET		 0x2C
#endif

// needs ldd version..
const TInt KMinimumLddMajorVersion=1;
const TInt KMinimumLddMinorVersion=1;
const TInt KMinimumLddBuild=122;

// The following defines when enabled can help debugging uart:

// RX_WORKAROUND - The basic workaround. There is a small window where data loss could occur
// #define RX_WORKAROUND

// RX_WORKAROUND_SAVE - A more advanced workaround that notifies the LDD that there
// has been a read error. May cause new reporting of errors
// #define RX_WORKAROUND_SAVE

// TRACK_WORKAROUND - Records the number activations of the workaround
// and the state of the last one. printf's the values on on deconstruct
// #define TRACK_WORKAROUND

// TRACK_DISTRIBUTION - Records the number of bytes read at a time to
// demonstrate operation of fifo mode. printf's the values on on deconstruct
// #define TRACK_DISTRIBUTION

// DISABLE_RX_FLUSH - Disables RX Flush in uart configuration procedure.
// #define DISABLE_RX_FLUSH

// TEMPORARY_SMP_FIX reduces the number of bytes pumped into the Tx FIFO from 16 to 8
// which works around a hardware problem not fully understood.
#define TEMPORARY_SMP_FIX

// configuration data
static const TUint16 BaudRateDivisor[] =
	{
	KBaudRateDiv_50,
	KBaudRateDiv_75,
	KBaudRateDiv_110,
	KBaudRateDiv_134,
	KBaudRateDiv_150,
	KBaudRateDiv_300,
	KBaudRateDiv_600,
	KBaudRateDiv_1200,
	KBaudRateDiv_1800,
	KBaudRateDiv_2000,
	KBaudRateDiv_2400,
	KBaudRateDiv_3600,
	KBaudRateDiv_4800,
	KBaudRateDiv_7200,
	KBaudRateDiv_9600,
	KBaudRateDiv_19200,
	KBaudRateDiv_38400,
	KBaudRateDiv_57600,
	KBaudRateDiv_115200,
	KBaudRateDiv_230400,
	};

class DDriverComm : public DPhysicalDevice
	{
public:
	DDriverComm();
	~DDriverComm();
	virtual TInt Install();
	virtual void GetCaps(TDes8 &aDes) const;
	virtual TInt Create(DBase*& aChannel, TInt aUnit, const TDesC8* anInfo, const TVersion &aVer);
	virtual TInt Validate(TInt aUnit, const TDesC8* anInfo, const TVersion &aVer);
public:
	TDynamicDfcQue* iDfcQ;
	};

class DComm16550 : public DComm
	{
public:
	DComm16550();
	~DComm16550();
	TInt DoCreate(TInt aUnit, const TDesC8* anInfo, TDynamicDfcQue* aDfcQ);
public:
	virtual TInt Start();
	virtual void Stop(TStopMode aMode);
	virtual void Break(TBool aState);
	virtual void EnableTransmit();
	virtual TUint Signals() const;
	virtual void SetSignals(TUint aSetMask,TUint aClearMask);
	virtual TInt ValidateConfig(const TCommConfigV01 &aConfig) const;
	virtual void Configure(TCommConfigV01 &aConfig);
	virtual void Caps(TDes8 &aCaps) const;
	virtual TInt DisableIrqs();
	virtual void RestoreIrqs(TInt aIrq);
	virtual TDfcQue* DfcQ(TInt aUnit);
	virtual void CheckConfig(TCommConfigV01& aConfig);
public:
	static void Isr(TAny* aPtr);
public:
	TInt iInterruptId;
	TInt iUnit;
	T16550Uart* iUart;
	TDynamicDfcQue* iDfcQ;

#ifdef TRACK_WORKAROUND
	TInt iWorkaround;		 		 		 // how many times did the workaround fire
	TInt iWorkaroundLSR;
	TInt iWorkaroundISR;
#endif

#ifdef TRACK_DISTRIBUTION
	TUint iRead[33];		 // array of how many bytes reported back to the ldd
#endif
	};


DDriverComm::DDriverComm()
//
// Constructor
//
	{
	iUnitsMask=~(0xffffffffu<<KNum16550Uarts);
	iVersion=TVersion(KCommsMajorVersionNumber,KCommsMinorVersionNumber,KCommsBuildVersionNumber);
	}


TInt DDriverComm::Install()
//
// Install the driver
//
	{
	// Allocate a kernel thread to run the DFC
	TInt r = Kern::DynamicDfcQCreate(iDfcQ, KUart16550ThreadPriority, KUar16550tDriverThread);
	if (r == KErrNone)
		{
#ifdef CPU_AFFINITY_ANY
	NKern::ThreadSetCpuAffinity((NThread*)(iDfcQ->iThread), KCpuAffinityAny);
#endif
		r = SetName(&KPddName);
		}
	return r;
	}

/**
  Destructor
*/
DDriverComm::~DDriverComm()
	{
	if (iDfcQ)
		iDfcQ->Destroy();
	}

void Get16550CommsCaps(TDes8& aCaps, TInt aUnit)
	{
	TCommCaps3 capsBuf;
	TCommCapsV03 &c=capsBuf();
	c.iRate =
		 KCapsBps150
		|KCapsBps300
		|KCapsBps600
		|KCapsBps1200
		|KCapsBps1800
		|KCapsBps2000
		|KCapsBps2400
		|KCapsBps3600
		|KCapsBps4800
		|KCapsBps7200
		|KCapsBps9600
		|KCapsBps19200
		|KCapsBps38400
		|KCapsBps57600
		|KCapsBps115200;
	if (aUnit != 2)
		{
		c.iRate |= KCapsBps230400;
		}

	c.iDataBits =
		 KCapsData5
		|KCapsData6
		|KCapsData7
		|KCapsData8;
	c.iStopBits =
		 KCapsStop1
		|KCapsStop2;
	c.iParity =
		 KCapsParityNone
		|KCapsParityEven
		|KCapsParityOdd
		|KCapsParityMark
		|KCapsParitySpace;
	c.iSIR = 0;
	switch (aUnit)
		{
	case 0:
		c.iHandshake =
			 KCapsObeyXoffSupported
			|KCapsSendXoffSupported
			|KCapsObeyCTSSupported
			|KCapsFailCTSSupported
			|KCapsFreeRTSSupported;
		c.iSignals =
			KCapsSignalCTSSupported
			|KCapsSignalRTSSupported;
		break;
	case 1:
		c.iHandshake =
			 KCapsObeyXoffSupported
			|KCapsSendXoffSupported
			|KCapsObeyCTSSupported
			|KCapsFailCTSSupported
			|KCapsObeyDSRSupported
			|KCapsFailDSRSupported
			|KCapsObeyDCDSupported
			|KCapsFailDCDSupported
			|KCapsFreeRTSSupported
			|KCapsFreeDTRSupported;
		c.iSignals =
			 KCapsSignalCTSSupported
			|KCapsSignalRTSSupported
			|KCapsSignalDTRSupported
			|KCapsSignalDSRSupported
			|KCapsSignalDCDSupported;
		break;
	default:
	case 2:
		c.iHandshake =
			 KCapsObeyXoffSupported
			|KCapsSendXoffSupported;
		c.iSignals = 0;
		break;
		}
	c.iFifo=KCapsHasFifo;
	c.iNotificationCaps=
	     KNotifyDataAvailableSupported
	    |KNotifySignalsChangeSupported;
	c.iRoleCaps        = 0;
	c.iFlowControlCaps = 0;
	c.iBreakSupported  = ETrue;
	aCaps.FillZ(aCaps.MaxLength());
	aCaps=capsBuf.Left(Min(capsBuf.Length(),aCaps.MaxLength()));
	}

void DDriverComm::GetCaps(TDes8 &aDes) const
//
// Return the drivers capabilities
//
	{
	Get16550CommsCaps(aDes, 0);
	}

TInt DDriverComm::Create(DBase*& aChannel, TInt aUnit, const TDesC8* anInfo, const TVersion& aVer)
//
// Create a driver
//
	{
	DComm16550* pD=new DComm16550;
	aChannel=pD;
	TInt r=KErrNoMemory;
	if (pD)
		r=pD->DoCreate(aUnit,anInfo, iDfcQ);
	return r;
	}

TInt DDriverComm::Validate(TInt aUnit, const TDesC8* /*anInfo*/, const TVersion& aVer)
//
//		 Validate the requested configuration
//
	{
	if ((!Kern::QueryVersionSupported(iVersion,aVer)) || (!Kern::QueryVersionSupported(aVer,TVersion(KMinimumLddMajorVersion,KMinimumLddMinorVersion,KMinimumLddBuild))))
		return KErrNotSupported;
	if (TUint(aUnit)>=TUint(KNum16550Uarts))
		return KErrNotSupported;
	return KErrNone;
	}

DComm16550::DComm16550()
//
// Constructor
//
	{
//		 iTransmitting=EFalse;
	iInterruptId=-1;		 		 // -1 means not bound
	}

DComm16550::~DComm16550()
//
// Destructor
//
	{
	if (iUart)
		delete iUart;
	if (iInterruptId>=0)
		Interrupt::Unbind(iInterruptId);

#ifdef TRACK_WORKAROUND
	if (iWorkaround)
		{
		Kern::Printf("########################################");
		Kern::Printf("####%d iWorkaround %d LSR 0x%x ISR 0x%x", iUnit, iWorkaround, iWorkaroundLSR, iWorkaroundISR);
		Kern::Printf("########################################");
		}
	else
		{
		Kern::Printf("####%d iWorkaround %d LSR 0x%x ISR 0x%x", iUnit, iWorkaround, iWorkaroundLSR, iWorkaroundISR);
		}
#endif
#ifdef TRACK_DISTRIBUTION
	Kern::Printf("####%d iRead: %d, %d %d %d %d %d %d %d %d, %d %d %d %d %d %d %d %d, %d %d %d %d %d %d %d %d, %d %d %d %d %d %d %d",
	             iUnit,
	             iRead[0],
	             iRead[1], iRead[2], iRead[3], iRead[4], iRead[5], iRead[6], iRead[7], iRead[8],
	             iRead[9], iRead[10], iRead[11], iRead[12], iRead[13], iRead[14], iRead[15], iRead[16],
	             iRead[17], iRead[18], iRead[19], iRead[20], iRead[21], iRead[22], iRead[23], iRead[24],
	             iRead[25], iRead[26], iRead[27], iRead[28], iRead[29], iRead[30], iRead[31], iRead[32]);
#endif
	}

TInt DComm16550::DoCreate(TInt aUnit, const TDesC8* /*anInfo*/,  TDynamicDfcQue* aDfcQ)
//
// Sets up the PDD
//
	{
	TUint base;

	switch (aUnit)
		{
	case 0:
		base=KHwBaseUart0;
		break;
	case 1:
		base=KHwBaseUart1;
		break;
	case 2:
		base=KHwBaseUart2;
		break;
	default:
		return KErrNotSupported;
		}

	iUnit=aUnit;
	iDfcQ = aDfcQ;
	TInt irq=KIntIdUart0+aUnit;

	// bind to UART interrupt
	TInt r=Interrupt::Bind(irq,Isr,this);
	if (r>=0)
		{
		iInterruptId = r;
		r = KErrNone;

		iUart = new T16550Uart;
		if (iUart)
			{
			iUart->iBase = (TUint8*)base;
			iUart->SetIER(0);
			iUart->SetLCR(0);
			iUart->SetFCR(0);
			iUart->SetMCR(0);
			}
		else
			{
			Interrupt::Unbind(iInterruptId);
			r = KErrNoMemory;
			}
		}

#ifdef TRACK_WORKAROUND
	iWorkaround = iWorkaroundLSR = iWorkaroundISR = 0;
#endif
	return r;
	}

TDfcQue* DComm16550::DfcQ(TInt /*aUnit*/)
//
// Return the DFC queue to be used for this device
// For PC cards, use the PC card controller thread for the socket in question.
//
	{
	return iDfcQ;
	}

TInt DComm16550::Start()
//
// Start receiving characters
//
	{
	// if EnableTransmit() called before Start()
	iTransmitting=EFalse;
	iUart->SetIER(K16550IER_RDAI|K16550IER_RLSI|K16550IER_MSI);
	iLdd->UpdateSignals(Signals());
	Interrupt::Enable(iInterruptId);
	return KErrNone;
	}

TBool FinishedTransmitting(TAny* aPtr)
	{
	DComm16550& d=*(DComm16550*)aPtr;
	return d.iUart->TestLSR(K16550LSR_TxIdle);
	}

void DComm16550::Stop(TStopMode aMode)
//
// Stop receiving characters
//
	{
	switch (aMode)
		{
	case EStopNormal:
	case EStopPwrDown:
		iUart->SetIER(0);
		Interrupt::Disable(iInterruptId);
		iTransmitting=EFalse;

		// wait for uart to stop tranmitting
		Kern::PollingWait(FinishedTransmitting,this,3,100);
		break;
	case  EStopEmergency:
		iUart->SetIER(0);
		Interrupt::Disable(iInterruptId);
		iTransmitting=EFalse;
		break;
		}
	}

void DComm16550::Break(TBool aState)
//
// Start or stop the uart breaking
//
	{
	if (aState)
		iUart->ModifyLCR(0,K16550LCR_TxBreak);
	else
		iUart->ModifyLCR(K16550LCR_TxBreak,0);
	}

void DComm16550::EnableTransmit()
//
// Start sending characters.
//
	{
	TBool tx = (TBool)__e32_atomic_swp_ord32(&iTransmitting, 1);
	if (tx)
		return;
	iUart->ModifyIER(0,K16550IER_THREI);
	}

TUint DComm16550::Signals() const
//
// Read and translate the modem lines
//
	{
	TUint msr=iUart->MSR();
	msr=((msr>>4)&0x0f);		 		 		 // true input signals
	TUint sig=msr & 3;		 		 		 		 // CTS,DSR OK
	if (msr & 4)
		sig|=KSignalRNG;		 		 		 // swap DCD,RNG
	if (msr & 8)
		sig|=KSignalDCD;

	return sig;
	}

void DComm16550::SetSignals(TUint aSetMask, TUint aClearMask)
//
// Set signals.
//
	{
	TUint set=0;
	TUint clear=0;
	if (aSetMask & KSignalRTS)
		set|=K16550MCR_RTS;
	if (aSetMask & KSignalDTR)
		set|=K16550MCR_DTR;
	if (aClearMask & KSignalRTS)
		clear|=K16550MCR_RTS;
	if (aClearMask & KSignalDTR)
		clear|=K16550MCR_DTR;
	iUart->ModifyMCR(clear,set);
	}

TInt DComm16550::ValidateConfig(const TCommConfigV01 &aConfig) const
//
// Check a config structure.
//
	{
	if (aConfig.iSIREnable==ESIREnable)
		return KErrNotSupported;
	switch (aConfig.iParity)
		{
	case EParityNone:
	case EParityOdd:
	case EParityEven:
	case EParityMark:
	case EParitySpace:
		break;
	default:
		return KErrNotSupported;
		}
//	if (TUint(aConfig.iRate)>TUint(EBps115200))
//		return KErrNotSupported;
	return KErrNone;
	}

void DComm16550::CheckConfig(TCommConfigV01& aConfig)
	{
	// do nothing
	}

TInt DComm16550::DisableIrqs()
//
// Disable normal interrupts
//
	{

	return NKern::DisableInterrupts(1);
	}

void DComm16550::RestoreIrqs(TInt aLevel)
//
// Restore normal interrupts
//
	{

	NKern::RestoreInterrupts(aLevel);
	}

void DComm16550::Configure(TCommConfigV01 &aConfig)
//
// Set up the Uart
//
	{
	// wait for uart to stop tranmitting
	Kern::PollingWait(FinishedTransmitting,this,3,100);

	TUint lcr=0;
	switch (aConfig.iDataBits)
		{
	case EData5:
		lcr=K16550LCR_Data5;
		break;
	case EData6:
		lcr=K16550LCR_Data6;
		break;
	case EData7:
		lcr=K16550LCR_Data7;
		break;
	case EData8:
		lcr=K16550LCR_Data8;
		break;
		}
	switch (aConfig.iStopBits)
		{
	case EStop1:
		break;
	case EStop2:
		lcr|=K16550LCR_Stop2;
		break;
		}
	switch (aConfig.iParity)
		{
	case EParityNone:
		break;
	case EParityEven:
		lcr|=K16550LCR_ParityEnable|K16550LCR_ParityEven;
		break;
	case EParityOdd:
		lcr|=K16550LCR_ParityEnable;
		break;
	case EParityMark:
		lcr|=K16550LCR_ParityEnable|K16550LCR_ParityMark;
		break;
	case EParitySpace:
		lcr|=K16550LCR_ParityEnable|K16550LCR_ParitySpace;
		break;
		}

	/*
	S18599EJ1V5UM.pdf page 746 uart configuration
	   Caution: Must not change the baud rate while any data transfers are executing.
	   If the baud rate is necessary to changed, please apply following sequence.
	   ... (inlined with code) ...
	 */

	// 1) Put UART in Local loop-back mode (write 1 to MCR4 bit).
	iUart->ModifyMCR(0,K16550MCR_LocalLoop);

	// 2) Clear transmit FIFO (write 1 to FCR2 bit).
	iUart->SetFCR(K16550FCR_TxReset);

	// 3) Wait 1 character period (ex. 100µs at 115kbps)
	NKern::Sleep(1);		 // sleep 1ms (for, 9600 bump up to 2ms?)

	// 4) Clear receive buffer (read RBR register until LSR0 bit becomes 0).
	TInt maxdrain = 1000; // arbitrary count (fifo depth is 16 bytes)
	while (iUart->TestLSR(K16550LSR_RxReady) && --maxdrain)
		iUart->RxData(); // read character from RBR

	// 5) Clear receive FIFO (write 1 to FCR1 bit).
	iUart->SetFCR(K16550FCR_RxReset);

	// 6) Change the baud rate (write 1 to LCR7, write DLL and DLH, and then write 0 to LCR7).
	iUart->SetLCR(lcr|K16550LCR_DLAB);
	iUart->SetBaudRateDivisor(BaudRateDivisor[(TInt)aConfig.iRate]);
	iUart->SetLCR(lcr);

	// 7) Go back to normal mode (write 0 to MCR4 bit).
	iUart->ModifyMCR(K16550MCR_LocalLoop, 0);

	// Enable fifo mode
	//iUart->SetFCR(K16550FCR_Enable | K16550FCR_TxRxRdy | K16550FCR_RxTrig8);
	iUart->SetFCR(K16550FCR_Enable | K16550FCR_RxTrig8);

#ifndef	 DISABLE_RX_FLUSH
	// Read characters from RBR; this isn't part of the recommended baud rate change code
	// but this has fixed the problem with ISR running indefinitely.
	TInt j;
	for (j=0;j<16;j++)	 // Fifo buffer is 16 bytes deep
		iUart->RxData();
#endif

#ifdef TRACK_DISTRIBUTION
	// blank an array of 33 integers 0 to 32
	TInt i=0;
	for (i=0;i<33;i++)
		{
		iRead[i]=0;
		}
#endif
	}

void DComm16550::Caps(TDes8 &aCaps) const
//
// return our caps
//
	{
	Get16550CommsCaps(aCaps,iUnit);
	}

void DComm16550::Isr(TAny* aPtr)
//
// Service the UART interrupt
//
	{
	DComm16550& d=*(DComm16550*)aPtr;
	T16550Uart& u=*d.iUart;
	TUint rx[32];
	TUint xon=d.iLdd->iRxXonChar;
	TUint xoff=d.iLdd->iRxXoffChar;

	TUint isr=u.ISR();
	if (isr & K16550ISR_NotPending)
		return;
	isr&=K16550ISR_IntIdMask;

	// if receive data available or line status interrupt
	if (isr==K16550ISR_RDAI || isr==K16550ISR_RLSI)
		{
		TInt rxi=0;
		TInt x=0;

#if defined(RX_WORKAROUND) || defined(RX_WORKAROUND_SAVE)
		// It has been observed that the hardware can assert RDAI without asserting RxReady
		// This is nasty but it reads from the head of the fifo, ORs in a faked error
		// and saves it in the buffer, later on in the loop it will be passed to the ldd
		// as data with an error
		if (!(u.LSR() & K16550LSR_RxReady))
			{
#ifdef TRACK_WORKAROUND
			++d.iWorkaround;
			d.iWorkaroundLSR=u.LSR();
			d.iWorkaroundISR=u.ISR();
#endif

#ifdef RX_WORKAROUND_SAVE
			// save the character and flag error, this is a bit risky as it could report errors
			rx[rxi++]=(u.RxData() | KReceiveIsrFrameError);
#else // or just discard it (normal workaround)
			u.RxData();
#endif
			}
#endif
		while (u.TestLSR(K16550LSR_RxReady|K16550LSR_RxParityErr|K16550LSR_RxOverrun|K16550LSR_RxFrameErr|K16550LSR_RxBreak) && Kern::PowerGood())
			{
			TUint lsr=0;
			// checks for EIF flag
			if (isr==K16550ISR_RLSI)
				lsr=u.LSR()&(K16550LSR_RxParityErr|K16550LSR_RxOverrun|K16550LSR_RxFrameErr);
			TUint ch=u.RxData();
			// if error in this character
			if (lsr)
				{
				if (lsr & K16550LSR_RxParityErr)
					ch|=KReceiveIsrParityError;
				if (lsr & K16550LSR_RxBreak)
					ch|=KReceiveIsrBreakError;
				if (lsr & K16550LSR_RxFrameErr)
					ch|=KReceiveIsrFrameError;
				if (lsr & K16550LSR_RxOverrun)
					ch|=KReceiveIsrOverrunError;
				}
			if (ch==xon)
				x=1;
			else if (ch==xoff)
				x=-1;
			else
				rx[rxi++]=ch;
			}
#ifdef TRACK_DISTRIBUTION
		d.iRead[rxi]++;		 		 // record how many bytes we're reporting
#endif
		d.ReceiveIsr(rx,rxi,x);
		return;
		}
	// if TFS flag and TIE
	if (isr==K16550ISR_THREI)
		{
		TInt n;
#ifdef TEMPORARY_SMP_FIX
		for (n=0; n<8; ++n)
#else
		for (n=0; n<16; ++n)
#endif
			{
			TInt r=d.TransmitIsr();
			if (r<0)
				{
				//no more to send
				// Disable the TX interrupt
				u.ModifyIER(K16550IER_THREI,0);
				d.iTransmitting=EFalse;
				break;
				}
			u.SetTxData(r);
			}
		d.CheckTxBuffer();
		return;
		}
	// must be signal change
	d.StateIsr(d.Signals());
	}

DECLARE_STANDARD_PDD()
	{
	return new DDriverComm;
	}

