// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/omap3530_drivers/usbcc/pa_usbc.cpp
//

#include <usbc.h>
//#include <resourceman.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>
#include <assp/omap3530_assp/omap3530_usbc.h>
//#include <assp/omap3530_assp/omap3530_prm.h>
#include <assp/omap3530_assp/omap3530_prcm.h>


// Debug support
#ifdef _DEBUG
static const char KUsbPanicCat[] = "USB PSL";
#endif

_LIT(KDfcName, "USB_DFC");

// Register definitions - move to a seperate header file at some point..

const TUint KCM_ICLKEN1_CORE = Omap3530HwBase::TVirtual<0x48004A10>::Value;
	const TUint KENHOSTOTGUSB_BIT = KBit4;
const TUint KCM_AUTOIDLE1_CORE = Omap3530HwBase::TVirtual<0x48004A30>::Value;
	const TUint KAUTO_HOSTOTGUSB_BIT = KBit4;


const TInt KSetupPacketSize = 8;
const TInt KMaxPayload = 0x400;
const TInt KUsbDfcPriority = 45;
const TUint KUSBBase = Omap3530HwBase::TVirtual<0x480AB000>::Value;

// USB registers - need the slave clock enabled to access most of these
const TUint KFADDR_REG = 0x0;
	const TUint KADDRESS_MSK = 0x7F;
const TUint KPOWER_REG  = 0x1;
	const TUint KSOFTCONNECT_BIT = KBit6;
	const TUint KSUSPENDM_BIT = KBit1;
	const TUint KRESUME_BIT = KBit2;
	const TUint KHSEN_BIT = KBit5;
//	const TUint KRESET_BIT = KBit3;
const TUint K_INTRTX_REG =0x2;
const TUint K_INTRRX_REG =0x4;
const TUint K_INTRTXE_REG =0x6;
const TUint K_INTRRXE_REG =0x8;
const TUint K_INTRUSB_REG = 0xA;
const TUint K_INTRUSBE_REG = 0xB;
	const TUint K_INT_RESET = KBit2;
	const TUint K_INT_RESUME = KBit1;
	const TUint K_INT_SUSPEND = KBit0;
//const TUint K_DEVCTRL_REG = 0x60;
const TUint K_FIFO0_REG = 0x20;
const TUint K_FIFO_OFFSET = 0x4;
const TUint K_COUNT0_REG = 0x18;
const TUint K_RXCOUNT_REG = 0x18;
const TUint K_CONFIGDATA_REG = 0x1F;
	const TUint K_MPRXE = KBit7;
	const TUint K_MPTXE = KBit6;
	const TUint K_DYNFIFO = KBit2;
	const TUint K_SOFTCONNECT = KBit1;
const TUint K_INDEX_REG = 0xE;
const TUint K_PERI_CSR0_REG = 0x12;
	const TUint K_EP0_FLUSHFIFO = KBit8;
	const TUint K_EP0_SERV_SETUPEND = KBit7;
	const TUint K_EP0_SERV_RXPKTRDY = KBit6;
	const TUint K_EP0_SETUPEND = KBit4;
	const TUint K_EP0_SENDSTALL = KBit5;
	const TUint K_EP0_DATAEND = KBit3;
	const TUint K_EP0_SENTSTALL = KBit2;
	const TUint K_EP0_TXPKTRDY = KBit1;
	const TUint K_EP0_RXPKTRDY = KBit0;
const TUint K_TXMAXP_REG = 0x10;
const TUint K_RXMAXP_REG = 0x14;
const TUint K_PERI_TXCSR_REG =  0x12;
	const TUint K_TX_ISO = KBit14;
	const TUint K_TX_DMAEN = KBit12;
	const TUint K_TX_DMAMODE = KBit10;
	const TUint K_TX_CLRDATATOG = KBit6;
	const TUint K_TX_SENTSTALL = KBit5;
	const TUint K_TX_SENDSTALL = KBit4;
	const TUint K_TX_FLUSHFIFO = KBit3;
	const TUint K_TX_UNDERRUN = KBit2;
//	const TUint K_TX_FIFONOTEMPTY = KBit1;
	const TUint K_TX_TXPKTRDY = KBit0;
const TUint K_PERI_RXCSR_REG =  0x16;
	const TUint K_RX_ISO = KBit14;
	const TUint K_RX_DMAEN = KBit13;
	const TUint K_RX_DISNYET = KBit12;
	const TUint K_RX_CLRDATATOG = KBit7;
	const TUint K_RX_SENTSTALL = KBit6;
	const TUint K_RX_SENDSTALL = KBit5;
	const TUint K_RX_FLUSHFIFO = KBit4;
	const TUint K_RX_OVERRUN = KBit2;
	const TUint K_RX_RXPKTRDY = KBit0;	
const TUint K_TXFIFOSZ_REG = 0x62;
const TUint K_RXFIFOSZ_REG = 0x63;
const TUint K_TXFIFOADDR_REG = 0x64;
const TUint K_RXFIFOADDR_REG = 0x66;
const TUint K_OTG_SYSCONFIG_REG = 0x404;
	const TUint K_ENABLEWAKEUP = KBit2;
//const TUint K_OTG_SYSSTATUS_REG = 0x408;

// End of Register definitions

// Define USB_SUPPORTS_PREMATURE_STATUS_IN to enable proper handling of a premature STATUS_IN stage, i.e. a
// situation where the host sends less data than first announced and instead of more data (OUT) will send an
// IN token to start the status stage. What we do in order to implement this here is to prime the TX fifo with
// a ZLP immediately when we find out that we're dealing with a DATA_OUT request. This way, as soon as the
// premature IN token is received, we complete the transaction by sending off the ZLP. If we don't prime the
// TX fifo then there is no way for us to recognise a premature status because the IN token itself doesn't
// raise an interrupt. We would simply wait forever for more data, or rather we would time out and the host
// would move on and send the next Setup packet.
// The reason why we would not want to implement the proper behaviour is this: After having primed the TX fifo
// with a ZLP, it is impossible for a user to reject such a (class/vendor specific) Setup request, basically
// because the successful status stage happens automatically. At the time the user has received and decoded
// the Setup request there's for her no way to stall Ep0 in order to show to the host that this Setup packet
// is invalid or inappropriate or whatever, because she cannot prevent the status stage from happening.
// (All this is strictly true only if the amount of data in the data stage is less than or equal to Ep0's max
//	packet size. However this is almost always the case.)
//#define USB_SUPPORTS_PREMATURE_STATUS_IN


static const TUsbcEndpointCaps DeviceEndpoints[KUsbTotalEndpoints] =
	{
	//                                                      Hardware #    iEndpoints index
	{KEp0MaxPktSzMask,	(KUsbEpTypeControl	   | KUsbEpDirOut)}, //	 0 -  0
	{KEp0MaxPktSzMask,	(KUsbEpTypeControl	   | KUsbEpDirIn )}, //	 0 -  1
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 1 -  2
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 2 -  3
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 3 -  4
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 4 -  5
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 5 -  6
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 6 -  7
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 7 -  8
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 8 -  9
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 9 -  10
	{KEp0MaxPktSzMask,	(KUsbEpTypeBulk	   	   | KUsbEpDirIn )}, //	 10 -  11
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 11 -  12
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 12 -  13
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 13 -  14
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 14 -  15
	// Disabled due to limited FIFO space
	/*{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 15 -  16
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 16 -  17
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 17 -  18
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 18 -  19
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 19 -  20
	{KEp0MaxPktSzMask,	(KUsbEpTypeBulk	   	   | KUsbEpDirIn )}, //	 20 -  21
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 21 -  22
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 22 -  23
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 23 -  24
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 24 -  25
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 25 -  26
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 26 -  27
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 27 -  28
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirIn )}, //	 28 -  29
	{KBlkMaxPktSzMask,	(KUsbEpTypeBulk		   | KUsbEpDirOut)}, //	 29 -  30
	{KIntMaxPktSzMask,	(KUsbEpTypeInterrupt   | KUsbEpDirIn )}  //	 30-   31*/
	};


// --- TEndpoint --------------------------------------------------------------

TEndpoint::TEndpoint()
//
// Constructor
//
	: iRxBuf(NULL), iReceived(0), iLength(0), iZlpReqd(EFalse), iNoBuffer(EFalse), iDisabled(EFalse),
	  iPackets(0), iLastError(KErrNone), iRequest(NULL), iRxTimer(RxTimerCallback, this),
	  iRxTimerSet(EFalse), iRxMoreDataRcvd(EFalse), iPacketIndex(NULL), iPacketSize(NULL)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("TEndpoint::TEndpoint"));
	}


void TEndpoint::RxTimerCallback(TAny* aPtr)
//
// (This function is static.)
//
	{
	TEndpoint* const ep = static_cast<TEndpoint*>(aPtr);
	if (!ep)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: !ep"));
		}
	else if (!ep->iRxTimerSet)
		{
		// Timer 'stop' substitute (instead of stopping it,
		// we just let it expire after clearing iRxTimerSet)
		__KTRACE_OPT(KUSB, Kern::Printf("!ep->iRxTimerSet - returning"));
		}
	else if (!ep->iRxBuf)
		{
		// Request already completed
		__KTRACE_OPT(KUSB, Kern::Printf("!ep->iRxBuf - returning"));
		}
	else if (ep->iRxMoreDataRcvd)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > rx timer cb: not yet completing..."));
		ep->iRxMoreDataRcvd = EFalse;
		ep->iRxTimer.Again(KRxTimerTimeout);
		}
	else
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > rx timer cb: completing now..."));
		*ep->iPacketSize = ep->iReceived;
		ep->iController->RxComplete(ep);
		}
	}


// --- DOmap3530Usbcc public ---------------------------------------------------

DOmap3530Usbcc::DOmap3530Usbcc()
//
// Constructor.
//
	: iCableConnected(ETrue), iBusIsPowered(EFalse),
	  iInitialized(EFalse), iUsbClientConnectorCallback(UsbClientConnectorCallback),
	  iAssp( static_cast<Omap3530Assp*>( Arch::TheAsic() ) ),
	  iEp0Configured(EFalse), iSuspendDfc(SuspendDfcFn, this, 7),
	  iResumeDfc(ResumeDfcFn, this, 7), iResetDfc(ResetDfcFn, this, 7)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::DOmap3530Usbcc"));
	
	TInt r = Kern::DfcQCreate(iDfcQueue, KUsbDfcPriority, &KDfcName);
	
	iSuspendDfc.SetDfcQ(iDfcQueue);	
	iResetDfc.SetDfcQ(iDfcQueue);
	iResumeDfc.SetDfcQ(iDfcQueue);
	
	iSoftwareConnectable = iAssp->UsbSoftwareConnectable();

	iCableDetectable = iAssp->UsbClientConnectorDetectable();

	if (iCableDetectable)
		{
		// Register our callback for detecting USB cable insertion/removal.
		// We ignore the error code: if the registration fails, we just won't get any events.
		// (Which of course is bad enough...)
		(void) iAssp->RegisterUsbClientConnectorCallback(iUsbClientConnectorCallback, this);
		// Call the callback straight away so we get the proper PIL state from the beginning.
		(void) UsbClientConnectorCallback(this);
		}

	for (TInt i = 0; i < KUsbTotalEndpoints; i++)
		{
		iEndpoints[i].iController = this;
		}
	
	__KTRACE_OPT(KUSB, Kern::Printf("-DOmap3530Usbcc::DOmap3530Usbcc"));
	}


TInt DOmap3530Usbcc::Construct()
//
// Construct.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Construct"));

	iPhy = MOmap3530UsbPhy::New();
	if( !iPhy )
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Failed to get pointer to USB PHY"));
		return KErrNoMemory;
		}

	//TInt r = PowerResourceManager::RegisterClient( iPrmClientId, KDfcName );
	//if( r != KErrNone )
	//	{
	//	__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Failed to connect to PRM"));
	//	return r;
	//	}


	TUsbcDeviceDescriptor* DeviceDesc = TUsbcDeviceDescriptor::New(
		0x00,												// aDeviceClass
		0x00,												// aDeviceSubClass
		0x00,												// aDeviceProtocol
		KEp0MaxPktSz,										// aMaxPacketSize0
		KUsbVendorId,										// aVendorId
		KUsbProductId,										// aProductId
		KUsbDevRelease,										// aDeviceRelease
		1);													// aNumConfigurations
	if (!DeviceDesc)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for dev desc failed."));
		return KErrGeneral;
		}

	TUsbcConfigDescriptor* ConfigDesc = TUsbcConfigDescriptor::New(
		1,													// aConfigurationValue
		ETrue,												// aSelfPowered (see 12.4.2 "Bus-Powered Devices")
		ETrue,												// aRemoteWakeup
		0);													// aMaxPower (mA)
	if (!ConfigDesc)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for config desc failed."));
		return KErrGeneral;
		}

	TUsbcLangIdDescriptor* StringDescLang = TUsbcLangIdDescriptor::New(KUsbLangId);
	if (!StringDescLang)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for lang id $ desc failed."));
		return KErrGeneral;
		}

	// ('sizeof(x) - 2' because 'wchar_t KStringXyz' created a wide string that ends in '\0\0'.)

	TUsbcStringDescriptor* StringDescManu =
		TUsbcStringDescriptor::New(TPtr8(
									   const_cast<TUint8*>(reinterpret_cast<const TUint8*>(KStringManufacturer)),
									   sizeof(KStringManufacturer) - 2, sizeof(KStringManufacturer) - 2));
	if (!StringDescManu)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for manufacturer $ desc failed."));
		return KErrGeneral;
		}

	TUsbcStringDescriptor* StringDescProd =
		TUsbcStringDescriptor::New(TPtr8(
									   const_cast<TUint8*>(reinterpret_cast<const TUint8*>(KStringProduct)),
									   sizeof(KStringProduct) - 2, sizeof(KStringProduct) - 2));
	if (!StringDescProd)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for product $ desc failed."));
		return KErrGeneral;
		}

	TUsbcStringDescriptor* StringDescSer =
		TUsbcStringDescriptor::New(TPtr8(
									   const_cast<TUint8*>(reinterpret_cast<const TUint8*>(KStringSerialNo)),
									   sizeof(KStringSerialNo) - 2, sizeof(KStringSerialNo) - 2));
	if (!StringDescSer)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for serial no $ desc failed."));
		return KErrGeneral;
		}

	TUsbcStringDescriptor* StringDescConf =
		TUsbcStringDescriptor::New(TPtr8(
									   const_cast<TUint8*>(reinterpret_cast<const TUint8*>(KStringConfig)),
									   sizeof(KStringConfig) - 2, sizeof(KStringConfig) - 2));
	if (!StringDescConf)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for config $ desc failed."));
		return KErrGeneral;
		}

	const TBool b =	InitialiseBaseClass(DeviceDesc,
										ConfigDesc,
										StringDescLang,
										StringDescManu,
										StringDescProd,
										StringDescSer,
										StringDescConf);
	if (!b)
		{ 
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: UsbClientController::InitialiseBaseClass failed."));
		return KErrGeneral;
		}
	
	return KErrNone;
	}


DOmap3530Usbcc::~DOmap3530Usbcc()
//
// Destructor.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::~DOmap3530Usbcc"));

	// Unregister our callback for detecting USB cable insertion/removal
	if (iCableDetectable)
		{
		iAssp->UnregisterUsbClientConnectorCallback();
		}
	if (iInitialized)
		{
		// (The explicit scope operator is used against Lint warning #1506.)
		DOmap3530Usbcc::StopUdc();
		}
	}


TBool DOmap3530Usbcc::DeviceStateChangeCaps() const
//
// Returns capability of hardware to accurately track the device state (Chapter 9 state).
//
	{
	return EFalse;
	}


TInt DOmap3530Usbcc::SignalRemoteWakeup()
//
// Forces the UDC into a non-idle state to perform a remote wakeup operation.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SignalRemoteWakeup"));
	Kern::Printf("DOmap3530Usbcc::SignalRemoteWakeup");
	// Resume signal
	
	TInt sysconfig = AsspRegister::Read32(KUSBBase+K_OTG_SYSCONFIG_REG );
	if(sysconfig&K_ENABLEWAKEUP && iRmWakeupStatus_Enabled)
		{
		AsspRegister::Modify8(KUSBBase+KPOWER_REG, KClearNone , KRESUME_BIT);
		Kern::NanoWait(10000000); // Wait 10ms - Use a callback instead!
		AsspRegister::Modify8(KUSBBase+KPOWER_REG, KRESUME_BIT, KSetNone);
		}
	return KErrNone;
	}


void DOmap3530Usbcc::DumpRegisters()
//
// Dumps the contents of a number of UDC registers to the screen (using Kern::Printf()).
// Rarely used, but might prove helpful when needed.
//
	{
	Kern::Printf("DOmap3530Usbcc::DumpRegisters:");
	}


TDfcQue* DOmap3530Usbcc::DfcQ(TInt /* aUnit */)
//
// Returns a pointer to the kernel DFC queue to be used buy the USB LDD.
//
	{
	return iDfcQueue;
	}


// --- DOmap3530Usbcc private virtual ------------------------------------------

TInt DOmap3530Usbcc::SetDeviceAddress(TInt aAddress)
//
// Sets the PIL-provided device address manually (if possible - otherwise do nothing).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SetDeviceAddress: %d", aAddress));

	AsspRegister::Write8(KUSBBase+KFADDR_REG, aAddress & KADDRESS_MSK);
		
	if (aAddress || GetDeviceStatus()==EUsbcDeviceStateAddress)
		{
		// Address can be zero.
		MoveToAddressState();
		}

	return KErrNone;
	}


TInt DOmap3530Usbcc::ConfigureEndpoint(TInt aRealEndpoint, const TUsbcEndpointInfo& aEndpointInfo)
//
// Prepares (enables) an endpoint (incl. Ep0) for data transmission or reception.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::ConfigureEndpoint(%d)", aRealEndpoint));
	
	const TInt n = ArrayIdx2TemplateEp(aRealEndpoint);
	if (n < 0)
		return KErrArgument;

	TEndpoint* const ep = &iEndpoints[aRealEndpoint];
	if (ep->iDisabled == EFalse)
		{
		EnableEndpointInterrupt(aRealEndpoint);
		if(n!=0)
			{
			AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
			if(aRealEndpoint%2==0)
				{

				AsspRegister::Write16(KUSBBase+K_PERI_RXCSR_REG, K_RX_CLRDATATOG | K_RX_DISNYET);
				}
			else
				{			
				AsspRegister::Write16(KUSBBase+K_PERI_TXCSR_REG, K_TX_CLRDATATOG);
				}
			}
		else
			{
			AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0x0);
			AsspRegister::Write16(KUSBBase+K_PERI_CSR0_REG, K_EP0_FLUSHFIFO); // FlushFifo;
			}
		}
	ep->iNoBuffer = EFalse;
	if (n == 0)
		iEp0Configured = ETrue;

	return KErrNone;
	}


TInt DOmap3530Usbcc::DeConfigureEndpoint(TInt aRealEndpoint)
//
// Disables an endpoint (incl. Ep0).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::DeConfigureEndpoint(%d)", aRealEndpoint));

	const TInt n = ArrayIdx2TemplateEp(aRealEndpoint);
	if (n < 0)
		return KErrArgument;

	DisableEndpointInterrupt(aRealEndpoint);
	if (n == 0)
		{
		iEp0Configured = EFalse;
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0);
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_FLUSHFIFO);
		}
	else
		{
		if(aRealEndpoint%2==0)
			{
			AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
			AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, KClearNone, K_RX_FLUSHFIFO);
			}
		else
			{		
			AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
			AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, KClearNone, K_TX_FLUSHFIFO);
			}
		}
	return KErrNone;
	}


TInt DOmap3530Usbcc::AllocateEndpointResource(TInt aRealEndpoint, TUsbcEndpointResource aResource)
//
// Puts the requested endpoint resource to use, if possible.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::AllocateEndpointResource(%d): %d",
									aRealEndpoint, aResource));

	// TO DO: Allocate endpoint resource here.

	return KErrNone;
	}


TInt DOmap3530Usbcc::DeAllocateEndpointResource(TInt aRealEndpoint, TUsbcEndpointResource aResource)
//
// Stops the use of the indicated endpoint resource, if beneficial.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::DeAllocateEndpointResource(%d): %d",
									aRealEndpoint, aResource));

	// TO DO: Deallocate endpoint resource here.

	return KErrNone;
	}


TBool DOmap3530Usbcc::QueryEndpointResource(TInt aRealEndpoint, TUsbcEndpointResource aResource) const
//
// Returns the status of the indicated resource and endpoint.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::QueryEndpointResource(%d): %d",
									aRealEndpoint, aResource));

	// TO DO: Query endpoint resource here. The return value should reflect the actual state.
	return ETrue;
	}


TInt DOmap3530Usbcc::OpenDmaChannel(TInt aRealEndpoint)
//
// Opens a DMA channel for this endpoint. This function is always called during the creation of an endpoint
// in the PIL. If DMA channels are a scarce resource, it's possible to do nothing here and wait for an
// AllocateEndpointResource call instead.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::OpenDmaChannel(%d)", aRealEndpoint));

	// TO DO (optional): Open DMA channel here.

	// An error should only  be returned in case of an actual DMA problem.
	return KErrNone;
	}


void DOmap3530Usbcc::CloseDmaChannel(TInt aRealEndpoint)
//
// Closes a DMA channel for this endpoint. This function is always called during the destruction of an
// endpoint in the PIL.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::CloseDmaChannel(%d)", aRealEndpoint));

	// TO DO (optional): Close DMA channel here (only if it was opened via OpenDmaChannel).
	}


TInt DOmap3530Usbcc::SetupEndpointRead(TInt aRealEndpoint, TUsbcRequestCallback& aCallback)
//
// Sets up a read request for an endpoint on behalf of the LDD.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SetupEndpointRead(%d)", aRealEndpoint));
	if (!IS_OUT_ENDPOINT(aRealEndpoint))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: !IS_OUT_ENDPOINT(%d)", aRealEndpoint));
		return KErrArgument;
		}
	TEndpoint* const ep = &iEndpoints[aRealEndpoint];
	if (ep->iRxBuf != NULL)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > WARNING: iEndpoints[%d].iRxBuf != NULL", aRealEndpoint));
		return KErrGeneral;
		}
	ep->iRxBuf = aCallback.iBufferStart;
	ep->iReceived = 0;
	ep->iLength = aCallback.iLength;

	// For Bulk reads we start out with the assumption of 1 packet (see BulkReceive for why):
	ep->iPackets = IS_BULK_OUT_ENDPOINT(aRealEndpoint) ? 1 : 0;
	ep->iRequest = &aCallback;
	ep->iPacketIndex = aCallback.iPacketIndex;
	if (IS_BULK_OUT_ENDPOINT(aRealEndpoint))
		*ep->iPacketIndex = 0;								// a one-off optimization
	ep->iPacketSize = aCallback.iPacketSize;

	if (ep->iDisabled)
		{
		ep->iDisabled = EFalse;
		EnableEndpointInterrupt(aRealEndpoint);
		}
	else if (ep->iNoBuffer)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > There had been no Rx buffer available: reading Rx FIFO now"));
		ep->iNoBuffer = EFalse;
		if (IS_BULK_OUT_ENDPOINT(aRealEndpoint))
			{
			BulkReadRxFifo(aRealEndpoint);
			}
		else if (IS_ISO_OUT_ENDPOINT(aRealEndpoint))
			{
			IsoReadRxFifo(aRealEndpoint);
			}
		else
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Endpoint not found"));
			}
		}

	return KErrNone;
	}


TInt DOmap3530Usbcc::SetupEndpointWrite(TInt aRealEndpoint, TUsbcRequestCallback& aCallback)
//
// Sets up a write request for an endpoint on behalf of the LDD.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SetupEndpointWrite(%d)", aRealEndpoint));
	
	if (!IS_IN_ENDPOINT(aRealEndpoint))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: !IS_IN_ENDPOINT(%d)", aRealEndpoint));
		return KErrArgument;
		}
	TEndpoint* const ep = &iEndpoints[aRealEndpoint];
	if (ep->iTxBuf != NULL)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: iEndpoints[%d].iTxBuf != NULL", aRealEndpoint));
		return KErrGeneral;
		}
	ep->iTxBuf = aCallback.iBufferStart;
	ep->iTransmitted = 0;
	ep->iLength = aCallback.iLength;
	ep->iPackets = 0;
	ep->iZlpReqd = aCallback.iZlpReqd;
	ep->iRequest = &aCallback;

	if (IS_BULK_IN_ENDPOINT(aRealEndpoint))
		{
		if (ep->iDisabled)
			{
			ep->iDisabled = EFalse;
			EnableEndpointInterrupt(aRealEndpoint);
			}
		BulkTransmit(aRealEndpoint);
		}
	else if (IS_ISO_IN_ENDPOINT(aRealEndpoint))
		{
		IsoTransmit(aRealEndpoint);
		}
	else if (IS_INT_IN_ENDPOINT(aRealEndpoint))
		{
		IntTransmit(aRealEndpoint);
		}
	else
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Endpoint not found"));
		}

	return KErrNone;
	}


TInt DOmap3530Usbcc::CancelEndpointRead(TInt aRealEndpoint)
//
// Cancels a read request for an endpoint on behalf of the LDD.
// No completion to the PIL occurs.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::CancelEndpointRead(%d)", aRealEndpoint));

	if (!IS_OUT_ENDPOINT(aRealEndpoint))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: !IS_OUT_ENDPOINT(%d)", aRealEndpoint));
		return KErrArgument;
		}
	TEndpoint* const ep = &iEndpoints[aRealEndpoint];
	if (ep->iRxBuf == NULL)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > WARNING: iEndpoints[%d].iRxBuf == NULL", aRealEndpoint));
		return KErrNone;
		}
		
	// : Flush the Ep's Rx FIFO here
	if(aRealEndpoint==KEp0_Out || aRealEndpoint==KEp0_In)
		{
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0);
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_FLUSHFIFO );
		}
	else
		{
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, KClearNone, K_RX_FLUSHFIFO );
		}
		
	ep->iRxBuf = NULL;
	ep->iReceived = 0;
	ep->iNoBuffer = EFalse;

	return KErrNone;
	}


TInt DOmap3530Usbcc::CancelEndpointWrite(TInt aRealEndpoint)
//
// Cancels a write request for an endpoint on behalf of the LDD.
// No completion to the PIL occurs.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::CancelEndpointWrite(%d)", aRealEndpoint));

	if (!IS_IN_ENDPOINT(aRealEndpoint))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: !IS_IN_ENDPOINT(%d)", aRealEndpoint));
		return KErrArgument;
		}
	TEndpoint* const ep = &iEndpoints[aRealEndpoint];
	if (ep->iTxBuf == NULL)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > WARNING: iEndpoints[%d].iTxBuf == NULL", aRealEndpoint));
		return KErrNone;
		}

	// TO DO (optional): Flush the Ep's Tx FIFO here, if possible.	
	if(aRealEndpoint==KEp0_Out || aRealEndpoint==KEp0_In)
		{
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0);
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_FLUSHFIFO );
		}
	else
		{
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, KClearNone, K_TX_FLUSHFIFO );
		}
			
	ep->iTxBuf = NULL;
	ep->iTransmitted = 0;
	ep->iNoBuffer = EFalse;

	return KErrNone;
	}


TInt DOmap3530Usbcc::SetupEndpointZeroRead()
//
// Sets up an Ep0 read request (own function due to Ep0's special status).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SetupEndpointZeroRead"));

	TEndpoint* const ep = &iEndpoints[KEp0_Out];
	if (ep->iRxBuf != NULL)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > WARNING: iEndpoints[%d].iRxBuf != NULL", KEp0_Out));
		return KErrGeneral;
		}
	ep->iRxBuf = iEp0_RxBuf;
	ep->iReceived = 0;

	return KErrNone;
	}


TInt DOmap3530Usbcc::SetupEndpointZeroWrite(const TUint8* aBuffer, TInt aLength, TBool aZlpReqd)
//
// Sets up an Ep0 write request (own function due to Ep0's special status).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SetupEndpointZeroWrite"));

	TEndpoint* const ep = &iEndpoints[KEp0_In];
	if (ep->iTxBuf != NULL)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: iEndpoints[%d].iTxBuf != NULL", KEp0_In));
		return KErrGeneral;
		}
	ep->iTxBuf = aBuffer;
	ep->iTransmitted = 0;
	ep->iLength = aLength;
	ep->iZlpReqd = aZlpReqd;
	ep->iRequest = NULL;
	Ep0Transmit();

	return KErrNone;
	}


TInt DOmap3530Usbcc::SendEp0ZeroByteStatusPacket()
//
// Sets up an Ep0 write request for zero bytes.
// This is a separate function because no data transfer is involved here.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SendEp0ZeroByteStatusPacket"));

	// This is possibly a bit tricky. When this function is called it just means that the higher layer wants a
	// ZLP to be sent. Whether we actually send one manually here depends on a number of factors, as the
	// current Ep0 state (i.e. the stage of the Ep0 Control transfer), and, in case the hardware handles some
	// ZLPs itself, whether it might already handle this one.

	// Here is an example of what the checking of the conditions might look like:

#ifndef USB_SUPPORTS_SET_DESCRIPTOR_REQUEST
	if ((!iEp0ReceivedNonStdRequest && iEp0State == EP0_IN_DATA_PHASE) ||
#else
	if ((!iEp0ReceivedNonStdRequest && iEp0State != EP0_IDLE) ||
#endif
#ifdef USB_SUPPORTS_PREMATURE_STATUS_IN
		(iEp0ReceivedNonStdRequest && iEp0State != EP0_OUT_DATA_PHASE))
#else
		(iEp0ReceivedNonStdRequest))
#endif

		{
		// TO DO: Arrange for the sending of a ZLP here.
		Kern::Printf("ZLP!");
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0x0);
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_TXPKTRDY | K_EP0_DATAEND );
		}

	return KErrNone;
	}


TInt DOmap3530Usbcc::StallEndpoint(TInt aRealEndpoint)
//
// Stalls an endpoint.
//
	{
	__KTRACE_OPT(KPANIC, Kern::Printf("DOmap3530Usbcc::StallEndpoint(%d)", aRealEndpoint));

	if (IS_ISO_ENDPOINT(aRealEndpoint))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Iso endpoint cannot be stalled"));
		return KErrArgument;
		}

	// Stall the endpoint here.
	if(aRealEndpoint==KEp0_Out || aRealEndpoint==KEp0_In)
		{
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0x0);
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_SENDSTALL);
		}
	else
	if(aRealEndpoint%2==0)
		{
		// RX stall
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, KClearNone, K_RX_SENDSTALL);
		}
	else
		{
		// TX stall
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, KClearNone, K_TX_SENDSTALL );
		}
	return KErrNone;
	}


TInt DOmap3530Usbcc::ClearStallEndpoint(TInt aRealEndpoint)
//
// Clears the stall condition of an endpoint.
//
	{
	__KTRACE_OPT(KPANIC, Kern::Printf("DOmap3530Usbcc::ClearStallEndpoint(%d)", aRealEndpoint));

	if (IS_ISO_ENDPOINT(aRealEndpoint))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Iso endpoint cannot be unstalled"));
		return KErrArgument;
		}

	// De-stall the endpoint here.
		
	if(aRealEndpoint==KEp0_Out || aRealEndpoint==KEp0_In)
		{
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0x0);
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, K_EP0_SENTSTALL, KSetNone );
		}
	else
	if(aRealEndpoint%2==0)
		{
		//Clear RX stall
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_SENDSTALL, KSetNone );
		}
	else
		{
		//Clear TX stall
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, K_TX_SENDSTALL, KSetNone );
		}

	return KErrNone;
	}


TInt DOmap3530Usbcc::EndpointStallStatus(TInt aRealEndpoint) const
//
// Reports the stall status of an endpoint.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::EndpointStallStatus(%d)", aRealEndpoint));
	if (IS_ISO_ENDPOINT(aRealEndpoint))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Iso endpoint has no stall status"));
		return KErrArgument;
		}		

	// Query endpoint stall status here. The return value should reflect the actual state.
	if(aRealEndpoint==KEp0_Out || aRealEndpoint==KEp0_In)
		{
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0x0);
		TInt status = AsspRegister::Read16(KUSBBase+K_PERI_CSR0_REG);
		return status & K_EP0_SENTSTALL;
		}
	else
	if(aRealEndpoint%2==0)
		{
		//Clear RX stall
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		TInt status = AsspRegister::Read16(KUSBBase+K_PERI_RXCSR_REG);
		return status & K_RX_SENDSTALL;
		}
	else
		{
		//Clear TX stall
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		TInt status = AsspRegister::Read16(KUSBBase+K_PERI_TXCSR_REG);
		return status & K_TX_SENDSTALL;
		
		}
	}


TInt DOmap3530Usbcc::EndpointErrorStatus(TInt aRealEndpoint) const
//
// Reports the error status of an endpoint.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::EndpointErrorStatus(%d)", aRealEndpoint));

	if (!IS_VALID_ENDPOINT(aRealEndpoint))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: !IS_VALID_ENDPOINT(%d)", aRealEndpoint));
		return KErrArgument;
		}

	// TO DO: Query endpoint error status here. The return value should reflect the actual state.
	// With some UDCs there is no way of inquiring the endpoint error status; say 'ETrue' in that case.

	// Bulk EP's don't have an error status
	return ETrue;
	}


TInt DOmap3530Usbcc::ResetDataToggle(TInt aRealEndpoint)
//
// Resets to zero the data toggle bit of an endpoint.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::ResetDataToggle(%d)", aRealEndpoint));

	// Reset the endpoint's data toggle bit here.
	// With some UDCs there is no way to individually reset the endpoint's toggle bits; just return KErrNone
	// in that case.
	
	if(aRealEndpoint==KEp0_Out || aRealEndpoint==KEp0_In)
		{
		// No way of setting data toggle for EP0
		}
	else
	if(aRealEndpoint%2==0)
		{
		//Clear RX stall
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, KClearNone, K_RX_CLRDATATOG);
		}
	else
		{
		//Clear TX stall
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aRealEndpoint/2));
		AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, KClearNone, K_TX_CLRDATATOG);	
		}

	return KErrNone;
	}


TInt DOmap3530Usbcc::SynchFrameNumber() const
//
// For use with isochronous endpoints only. Causes the SOF frame number to be returned.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SynchFrameNumber"));

	// TO DO: Query and return the SOF frame number here.
	return 0;
	}

void DOmap3530Usbcc::SetSynchFrameNumber(TInt aFrameNumber)
//
// For use with isochronous endpoints only. Causes the SOF frame number to be stored.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SetSynchFrameNumber(%d)", aFrameNumber));

	// We should actually store this number somewhere. But the PIL always sends '0x00'
	// in response to a SYNCH_FRAME request...
	// TO DO: Store the frame number. Alternatively (until SYNCH_FRAME request specification changes): Do
	// nothing.
	}

TInt DOmap3530Usbcc::StartUdc()
//
// Called to initialize the device controller hardware before any operation can be performed.
//
	{
	__KTRACE_OPT(KUSB,Kern::Printf("DOmap3530Usbcc::StartUdc"));

	if (iInitialized)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: UDC already initialised"));
		return KErrNone;
		}

	// Disable UDC (might also reset the entire design):
	UdcDisable();

	// Bind & enable the UDC interrupt
	if (SetupUdcInterrupt() != KErrNone)
		{
		return KErrGeneral;
		}
	// Enable the slave clock
	EnableSICLK();

	// Write meaningful values to some registers:
	InitialiseUdcRegisters();

	// Finally, turn on the UDC:
	UdcEnable();

	// and enable the PHY
	iPhy->StartPHY();
	iPhy->SetPHYMode(ENormal);

	// Even if only one USB feature has been enabled, we later need to undo it:
	iInitialized = ETrue;

	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc: UDC Enabled"));

	return KErrNone;
	}


TInt DOmap3530Usbcc::StopUdc()
//
// Basically, makes undone what happened in StartUdc.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::StopUdc"));

	if (!iInitialized)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: UDC not initialized"));
		return KErrNone;
		}

	// Disable UDC:
	UdcDisable();
	// Disable & unbind the UDC interrupt:
	ReleaseUdcInterrupt();
	iPhy->SetPHYMode(EUART);

	// Finally turn off slave clock
	DisableSICLK();
	
	// Only when all USB features have been disabled we'll call it a day:
	iInitialized = EFalse;

	return KErrNone;
	}


TInt DOmap3530Usbcc::UdcConnect()
//
// Connects the UDC to the bus under software control. How this is achieved depends on the UDC; the
// functionality might also be part of the Variant component (instead of the ASSP).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::UdcConnect"));

	//AsspRegister::Modify8(KUSBBase+KPOWER_REG , KClearNone, KSOFTCONNECT_BIT);
	AsspRegister::Write8(KUSBBase+KPOWER_REG , KSOFTCONNECT_BIT  | KHSEN_BIT);
	iPhy->EnablePHY();
	
	// Here: A call into the Variant-provided function.
	return iAssp->UsbConnect();
	}


TInt DOmap3530Usbcc::UdcDisconnect()
//
// Disconnects the UDC from the bus under software control. How this is achieved depends on the UDC; the
// functionality might also be part of the Variant component (instead of the ASSP).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::UdcDisconnect"));

	// Here: A call into the Variant-provided function.
	return iAssp->UsbDisconnect();
	}


TBool DOmap3530Usbcc::UsbConnectionStatus() const
//
// Returns a value showing the USB cable connection status of the device.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::UsbConnectionStatus"));

	return iCableConnected;
	}


TBool DOmap3530Usbcc::UsbPowerStatus() const
//
// Returns a truth value showing whether VBUS is currently powered or not.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::UsbPowerStatus"));

	return iBusIsPowered;
	}


TBool DOmap3530Usbcc::DeviceSelfPowered() const
//
// Returns a truth value showing whether the device is currently self-powered or not.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::DeviceSelfPowered"));

	// TO DO: Query and return self powered status here. The return value should reflect the actual state.
	// (This can be always 'ETrue' if the UDC does not support bus-powered devices.)
	return ETrue;
	}

const TUsbcEndpointCaps* DOmap3530Usbcc::DeviceEndpointCaps() const
//
// Returns a pointer to an array of elements, each of which describes the capabilities of one endpoint.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::DeviceEndpointCaps"));
	__KTRACE_OPT(KUSB, Kern::Printf(" > Ep: Sizes Mask, Types Mask"));
	__KTRACE_OPT(KUSB, Kern::Printf(" > --------------------------"));
	for (TInt i = 0; i < KUsbTotalEndpoints; ++i)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > %02d: 0x%08x, 0x%08x",
										i, DeviceEndpoints[i].iSizes, DeviceEndpoints[i].iTypesAndDir));
		}
	return DeviceEndpoints;
	}


TInt DOmap3530Usbcc::DeviceTotalEndpoints() const
//
// Returns the element number of the endpoints array a pointer to which is returned by DeviceEndpointCaps.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::DeviceTotalEndpoints"));

	return KUsbTotalEndpoints;
	}


TBool DOmap3530Usbcc::SoftConnectCaps() const
//
// Returns a truth value showing whether or not there is the capability to disconnect and re-connect the D+
// line under software control.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SoftConnectCaps"));

	return iSoftwareConnectable;
	}


void DOmap3530Usbcc::Suspend()
//
// Called by the PIL after a Suspend event has been reported (by us).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Suspend"));

	if (NKern::CurrentContext() == EThread)
		{
		iSuspendDfc.Enque();
		}
	else
		{
		iSuspendDfc.Add();
		}
	// TO DO (optional): Implement here anything the device might require after bus SUSPEND signalling.
	// Need to put the transceiver into suspend too. Can't do it here as it requries I2C and we are in an interrupt context.
	AsspRegister::Modify8(KUSBBase+KPOWER_REG , KClearNone, KSUSPENDM_BIT);
	}


void DOmap3530Usbcc::Resume()
//
// Called by the PIL after a Resume event has been reported (by us).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Resume"));
	if (NKern::CurrentContext() == EThread)
		{
		iResumeDfc.Enque();
		}
	else
		{
		iResumeDfc.Add();
		}

	// TO DO (optional): Implement here anything the device might require after bus RESUME signalling.
	// Need to put the transceiver into resume too. Can't do it here as it requries I2C and we are in an interrupt context.
	AsspRegister::Modify8(KUSBBase+KPOWER_REG, KClearNone , KRESUME_BIT);
	Kern::NanoWait(10000000); // Wait 10ms - Use a callback instead!
	AsspRegister::Modify8(KUSBBase+KPOWER_REG, KRESUME_BIT, KSetNone);
	}


void DOmap3530Usbcc::Reset()
//
// Called by the PIL after a Reset event has been reported (by us).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Reset"));

	// This does not really belong here, but has to do with the way the PIL sets
	// up Ep0 reads and writes.
	TEndpoint* ep = &iEndpoints[0];
	ep->iRxBuf = NULL;
	++ep;
	ep->iTxBuf = NULL;
	// Idle
	Ep0NextState(EP0_IDLE);
	// TO DO (optional): Implement here anything the device might require after bus RESET signalling.
	// Need to put the transceiver into reset too. Can't do it here as it requries I2C and we are in an interrupt context.
	if (NKern::CurrentContext() == EThread)
		{
		iResetDfc.Enque();
		}
	else
		{
		iResetDfc.Add();
		}
	
	// Write meaningful values to some registers
	InitialiseUdcRegisters();
	UdcEnable();
	if (iEp0Configured)
		EnableEndpointInterrupt(0);
	}


// --- DOmap3530Usbcc private --------------------------------------------------

void DOmap3530Usbcc::InitialiseUdcRegisters()
//
// Called after every USB Reset etc.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::InitialiseUdcRegisters"));

	AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0);
	AsspRegister::Write8(KUSBBase+K_CONFIGDATA_REG, K_SOFTCONNECT | K_DYNFIFO | K_MPTXE | K_MPRXE);// Dynamic FIFO
	
	// Configure FIFO's
	for(TUint n=1; n<KUsbTotalEndpoints; n++) // Fifo for EP 0 is fixed. Size 0x200 (512) for the ISO ep is wrong! FIXME!!!!!!!!!!!! Hacked to make all FIFO's 1024 bytes (ignore ep>16!)
		{
		AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)((n+1)/2));
		if(n%2==0)
			{
			AsspRegister::Write16(KUSBBase+K_TXMAXP_REG, KMaxPayload | 0x1<<11); // Not sure how many packets we want to split into. Use 2 because it is OK for Bulk and INT
			AsspRegister::Write8(KUSBBase+K_TXFIFOSZ_REG, 0x7); // No double buffering, FIFO size == 2^(7+3) = 1024
			AsspRegister::Write16(KUSBBase+K_TXFIFOADDR_REG, 128*((TInt)n/2)); // We have 16kb of memory and 16 endpoints. Start each fifo on a 1kb boundary	
			AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, K_TX_DMAMODE | K_TX_ISO | K_TX_DMAEN, K_TX_CLRDATATOG | K_TX_FLUSHFIFO);
			}
		else
			{
			AsspRegister::Write16(KUSBBase+K_RXMAXP_REG, KMaxPayload | 0x1<<11); // Not sure how many packets we want to split into. Use 2 because it is OK for Bulk and INT
			AsspRegister::Write8(KUSBBase+K_RXFIFOSZ_REG, 0x7); // No double buffering, FIFO size == 2^(7+3) = 1024
			AsspRegister::Write16(KUSBBase+K_RXFIFOADDR_REG, 128*((TInt)(n/2)+8)); // We have 16kb of memory and 16 endpoints. Start each fifo on a 1kb boundary			
			AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_ISO | K_RX_DMAEN, K_RX_CLRDATATOG | K_RX_FLUSHFIFO | K_RX_DISNYET);
			}
		}	

	// Disable interrupt requests for all endpoints
	AsspRegister::Modify16(KUSBBase+K_INTRTXE_REG, 0xFFFF, KSetNone);
	AsspRegister::Modify16(KUSBBase+K_INTRRXE_REG, 0XFFFE, KSetNone);
	
	AsspRegister::Modify32(KUSBBase+K_OTG_SYSCONFIG_REG, KClearNone, K_ENABLEWAKEUP);
	}


void DOmap3530Usbcc::UdcEnable()
//
// Enables the UDC for USB transmission or reception.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::UdcEnable"));
	EnableSICLK();
	// TO DO: Do whatever is necessary to enable the UDC here. This might include enabling (unmasking)
	// the USB Reset interrupt, setting a UDC enable bit, etc.
	AsspRegister::Read8(KUSBBase+K_INTRUSB_REG);	 // Reading this register clears it
	AsspRegister::Write8(KUSBBase+K_INTRUSBE_REG, K_INT_SUSPEND | K_INT_RESUME | K_INT_RESET);
	DisableSICLK();
	}


void DOmap3530Usbcc::UdcDisable()
//
// Disables the UDC.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::UdcDisable"));
	EnableSICLK();
	// TO DO: Do whatever is necessary to disable the UDC here. This might include disabling (masking)
	// the USB Reset interrupt, clearing a UDC enable bit, etc.
	AsspRegister::Write8(KUSBBase+K_INTRUSBE_REG, 0x0);
	AsspRegister::Read8(KUSBBase+K_INTRUSB_REG);	 // Reading this register clears it
	DisableSICLK();
	}


void DOmap3530Usbcc::EnableEndpointInterrupt(TInt aEndpoint)
//
// Enables interrupt requests for an endpoint.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::EnableEndpointInterrupt(%d)", aEndpoint));

	// Enable (unmask) interrupt requests for this endpoint:
	if(aEndpoint==0)
		{
		AsspRegister::Modify16(KUSBBase+K_INTRTXE_REG , KClearNone, 1<<(int)(aEndpoint/2));
		}
	else
		{
		if(aEndpoint%2==0)
			{
			AsspRegister::Modify16(KUSBBase+K_INTRRXE_REG  , KClearNone, 1<<(int)((aEndpoint)/2));
			}
		else
			{
			AsspRegister::Modify16(KUSBBase+K_INTRTXE_REG, KClearNone, 1<<(int)((aEndpoint)/2));
			}
		}
	}


void DOmap3530Usbcc::DisableEndpointInterrupt(TInt aEndpoint)
//
// Disables interrupt requests for an endpoint.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::DisableEndpointInterrupt(%d)", aEndpoint));

	// Disable (mask) interrupt requests for this endpoint:
	if(aEndpoint==0)
		{
		AsspRegister::Modify16(KUSBBase+K_INTRTXE_REG , 1<<(int)(aEndpoint/2), KSetNone);
		}
	else
		{
		if(aEndpoint%2==0)
			{
			AsspRegister::Modify16(KUSBBase+K_INTRRXE_REG , 1<<(int)((aEndpoint)/2), KSetNone);
			}
		else
			{
			AsspRegister::Modify16(KUSBBase+K_INTRTXE_REG, 1<<(int)((aEndpoint)/2), KSetNone);
			}
		}
	}


void DOmap3530Usbcc::ClearEndpointInterrupt(TInt aEndpoint)
//
// Clears a pending interrupt request for an endpoint.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::ClearEndpointInterrupt(%d)", aEndpoint));

	// Clear (reset) pending interrupt request for this endpoint:
	if(aEndpoint==0)
		{
		AsspRegister::Modify16(KUSBBase+K_INTRTX_REG , 1<<(int)(aEndpoint/2), KSetNone);
		}
	else
		{
		if(aEndpoint%2==0)
			{
			AsspRegister::Modify16(KUSBBase+K_INTRRX_REG , 1<<(int)((aEndpoint)/2), KSetNone);
			}
		else
			{
			AsspRegister::Modify16(KUSBBase+K_INTRTX_REG, 1<<(int)((aEndpoint)/2), KSetNone);
			}
		}
	}


void DOmap3530Usbcc::Ep0IntService()
//
// ISR for endpoint zero interrupt.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Ep0IntService"));
	Interrupt::Disable(EOmap3530_IRQ92_HSUSB_MC_NINT);
	
	//  Enquire about Ep0 status & the interrupt cause here. Depending on the event and the Ep0 state,

	AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0x0);
	TUint ep0 = AsspRegister::Read16(KUSBBase+K_PERI_CSR0_REG);
	
	if(ep0 & K_EP0_SETUPEND)
		{
		 // Setupend is set - A setup transaction ended unexpectedly
		Ep0Cancel();
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_SERV_SETUPEND);
		Ep0NextState(EP0_IDLE);
		}
	if(ep0&K_EP0_SENTSTALL)
		{
		// Stalled! Complete the stall handshake
		ClearStallEndpoint(0);
		}
	
	switch(iEp0State)
		{
			case EP0_END_XFER:
				Ep0EndXfer(); 
				break;
			case EP0_IDLE: 
				if(ep0&K_EP0_RXPKTRDY)
					{
					Ep0ReadSetupPkt();
					}
				else
					{
					Ep0StatusIn();
					} 
				break;
			case EP0_OUT_DATA_PHASE:
				Ep0Receive();
				break;
			case EP0_IN_DATA_PHASE:
				Ep0Transmit();
				break;
			default:
				break; // Do nothing
			}

	ClearEndpointInterrupt(0);
	Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
	}


void DOmap3530Usbcc::Ep0ReadSetupPkt()
//
// Called from the Ep0 ISR when a new Setup packet has been received.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Ep0ReadSetupPkt"));

	TEndpoint* const ep = &iEndpoints[KEp0_Out];
	TUint8* buf = ep->iRxBuf;
	if (!buf)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: No Ep0 Rx buffer available (1)"));
		StallEndpoint(KEp0_Out);
		return;
		}

	// Read Setup packet data from Rx FIFO into 'buf' here.
	// (In this function we don't need to use "ep->iReceived" since Setup packets
	// are always 8 bytes long.)
	for(TInt x=0; x<KSetupPacketSize; x++)
		{
		// Should try and check we aren't running out of FIFO!
		buf[x] = AsspRegister::Read8(KUSBBase+K_FIFO0_REG);
		}
	AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone,  K_EP0_SERV_RXPKTRDY); // The packet has been retrieved from the FIFO
	
	// Upcall into PIL to determine next Ep0 state:
	TUsbcEp0State state = EnquireEp0NextState(ep->iRxBuf);

	if (state == EEp0StateStatusIn)
		{
		Ep0NextState(EP0_IDLE);								// Ep0 No Data
		}
	else if (state == EEp0StateDataIn)
		{
		Ep0NextState(EP0_IN_DATA_PHASE);					// Ep0 Control Read
		}
	else
		{
		Ep0NextState(EP0_OUT_DATA_PHASE);					// Ep0 Control Write
		}

	ep->iRxBuf = NULL;
	const TInt r = Ep0RequestComplete(KEp0_Out, KSetupPacketSize, KErrNone);

	// Don't finish (proceed) if request completion returned 'KErrNotFound'!
	if (!(r == KErrNone || r == KErrGeneral))
		{
		DisableEndpointInterrupt(0);
		}
		
#ifdef USB_SUPPORTS_PREMATURE_STATUS_IN
	if (iEp0State == EP0_OUT_DATA_PHASE)
		{
		// Allow for a premature STATUS IN
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_TXPKTRDY | K_EP0_DATAEND); // TXPKTRDY, DATAEND
		}
#endif	
	}


void DOmap3530Usbcc::Ep0ReadSetupPktProceed()
//
// Called by the PIL to signal that it has finished processing a received Setup packet and that the PSL can
// now prepare itself for the next Ep0 reception (for instance by re-enabling the Ep0 interrupt).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Ep0ReadSetupPktProceed"));

	EnableEndpointInterrupt(0);
	}


void DOmap3530Usbcc::Ep0Receive()
//
// Called from the Ep0 ISR when a data packet has been received.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Ep0Receive"));
	AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0);
	TEndpoint* const ep = &iEndpoints[KEp0_Out];
	TUint8* buf = ep->iRxBuf;
	if (!buf)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: No Ep0 Rx buffer available (2)"));
		StallEndpoint(KEp0_Out);
		return;
		}

	TInt n = 0;
	// Read packet data from Rx FIFO into 'buf' and update 'n' (# of received bytes) here.
	TInt FIFOCount = AsspRegister::Read8(KUSBBase+K_COUNT0_REG);
	for(; n<FIFOCount; n++)
		{
		// Should try and check we aren't running out of FIFO!
		buf[n] = AsspRegister::Read8(KUSBBase+K_FIFO0_REG);
		}
	AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_SERV_RXPKTRDY); // The packet has been retrieved from the FIFO
	
	ep->iReceived = n;
	ep->iRxBuf = NULL;
	const TInt r = Ep0RequestComplete(KEp0_Out, n, KErrNone);

	// Don't finish (proceed) if request was 'KErrNotFound'!
	if (!(r == KErrNone || r == KErrGeneral))
		{
		DisableEndpointInterrupt(0);
		}
	
#ifdef USB_SUPPORTS_PREMATURE_STATUS_IN
	// Allow for a premature STATUS IN
	AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_TXPKTRDY | K_EP0_DATAEND); // TXPKTRDY, DATAEND
#endif
	}


void DOmap3530Usbcc::Ep0ReceiveProceed()
//
// Called by the PIL to signal that it has finished processing a received Ep0 data packet and that the PSL can
// now prepare itself for the next Ep0 reception (for instance by re-enabling the Ep0 Ep0ReadSetupPkt).
//
	{
	Ep0ReadSetupPktProceed();
	}


void DOmap3530Usbcc::Ep0Transmit()
//
// Called from either the Ep0 ISR or the PIL when a data packet has been or is to be transmitted.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Ep0Transmit"));
	AsspRegister::Write8(KUSBBase+K_INDEX_REG, 0);
	if (iEp0State != EP0_IN_DATA_PHASE)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > WARNING: Invalid Ep0 state when trying to handle EP0 IN (0x%x)", iEp0State));
		// TO DO (optional): Do something about this warning.
		}
	
	TEndpoint* const ep = &iEndpoints[KEp0_In];
	const TUint8* buf = ep->iTxBuf;
	if (!buf)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > No Tx buffer available: returning"));
		return;
		}
	const TInt t = ep->iTransmitted;						// already transmitted
	buf += t;
	TInt n = 0;												// now transmitted

	// Write packet data (if any) into Tx FIFO from 'buf' and update 'n' (# of tx'ed bytes) here.
	for(; n<ep->iLength-ep->iTransmitted && n<KEp0MaxPktSz; n++)
		{
		// Should try and check we aren't running out of FIFO!
		AsspRegister::Write8(KUSBBase+K_FIFO0_REG, buf[n]);
		}

	ep->iTransmitted += n;
	if (n == KEp0MaxPktSz)
		{
		if (ep->iTransmitted == ep->iLength && !(ep->iZlpReqd))
			{
			Ep0NextState(EP0_END_XFER);
			}
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_TXPKTRDY); // TXPKTY,
		}
	else if (n && n != KEp0MaxPktSz)
		{
		// Send off the data
		__ASSERT_DEBUG((ep->iTransmitted == ep->iLength),
					   Kern::Printf(" > ERROR: Short packet in mid-transfer"));
		Ep0NextState(EP0_END_XFER);
		//  Send off the data here.
		AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_TXPKTRDY); // TXPKTRDY,
		}
	else // if (n == 0)
		{
		__ASSERT_DEBUG((ep->iTransmitted == ep->iLength),
					   Kern::Printf(" > ERROR: Nothing transmitted but still not finished"));
		if (ep->iZlpReqd)
			{
			// Send a zero length packet
			ep->iZlpReqd = EFalse;
			Ep0NextState(EP0_END_XFER);
			// Arrange for the sending of a ZLP here.
			AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_TXPKTRDY | K_EP0_DATAEND); // TXPKTRDY, DATAEND
			}
		else
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: nothing transmitted & no ZLP req'd"));
			}
		}	
	}


void DOmap3530Usbcc::Ep0EndXfer()
//
// Called at the end of a Ep0 Control transfer.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Ep0EndXfer"));
	// Clear Ep0 Rx condition flags here.
	AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_SERV_RXPKTRDY | K_EP0_DATAEND); //  DATAEND
		
	Ep0NextState(EP0_IDLE);
	TEndpoint* const ep = &iEndpoints[KEp0_In];
	ep->iTxBuf = NULL;
	(void) Ep0RequestComplete(KEp0_In, ep->iTransmitted, KErrNone);
	}


void DOmap3530Usbcc::Ep0Cancel()
//
// Called when an ongoing Ep0 Control transfer has to be aborted prematurely (for instance when receiving a
// new Setup packet before the processing of the old one has completed).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Ep0Cancel"));

	Ep0NextState(EP0_IDLE);
	TEndpoint* const ep = &iEndpoints[KEp0_In];
	if (ep->iTxBuf)
		{
		ep->iTxBuf = NULL;
		const TInt err = (ep->iTransmitted == ep->iLength) ? KErrNone : KErrCancel;
		(void) Ep0RequestComplete(KEp0_In, ep->iTransmitted, err);
		}
	}


void DOmap3530Usbcc::Ep0PrematureStatusOut()
//
// Called when an ongoing Ep0 Control transfer encounters a premature Status OUT condition.
//
	{
	__KTRACE_OPT(KPANIC, Kern::Printf("DOmap3530Usbcc::Ep0PrematureStatusOut"));

	// Clear Ep0 Rx condition flags here.
	AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_SERV_RXPKTRDY | K_EP0_DATAEND); //  DATAEND
	Ep0NextState(EP0_IDLE);

	// Flush the Ep0 Tx FIFO here, if possible.
	AsspRegister::Modify16(KUSBBase+K_PERI_CSR0_REG, KClearNone, K_EP0_FLUSHFIFO);
		
	TEndpoint* const ep = &iEndpoints[KEp0_In];
	if (ep->iTxBuf)
		{
		ep->iTxBuf = NULL;
		(void) Ep0RequestComplete(KEp0_In, ep->iTransmitted, KErrPrematureEnd);
		}
	}


void DOmap3530Usbcc::Ep0StatusIn()
//
// Called when an ongoing Ep0 Control transfer moves to a Status IN stage.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Ep0StatusIn"));

	Ep0NextState(EP0_IDLE);
	}


void DOmap3530Usbcc::BulkTransmit(TInt aEndpoint)
//
// Endpoint 1 (BULK IN).
// Called from either the Ep ISR or the PIL when a data packet has been or is to be transmitted.
//
	{
	Interrupt::Disable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::BulkTransmit(%d)", aEndpoint));
	
	AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aEndpoint/2));
	
	TInt status = AsspRegister::Read16(KUSBBase+K_PERI_TXCSR_REG); 
	
	if(status & K_TX_UNDERRUN)
		{
		// TX UNDERRUN
		AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, K_TX_UNDERRUN, KSetNone); 
		}
	if(status & K_TX_SENTSTALL)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Stall Handshake"));
		// Complete stall handshake
		AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, K_TX_SENTSTALL, KSetNone); 
		Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
		return;
		}
	if(status & K_TX_SENDSTALL)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Stalled"));
		// We are stalled
		Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
		return;
		}
	
	TBool calledFromISR=AsspRegister::Read16(KUSBBase+K_INTRTX_REG) & 1<<(aEndpoint/2)==1;
	
	const TInt idx = aEndpoint;										// only in our special case of course!
	TEndpoint* const ep = &iEndpoints[idx];
	const TUint8* buf = ep->iTxBuf;
	
	if (!buf)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: No Tx buffer has been set up"));
		DisableEndpointInterrupt(aEndpoint);
		ep->iDisabled = ETrue;
		ClearEndpointInterrupt(aEndpoint);
		Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
		return;
		}
	const TInt t = ep->iTransmitted;						// already transmitted
	const TInt len = ep->iLength;							// to be sent in total
	// (len || ep->iPackets): Don't complete for a zero bytes request straight away.
	if (t >= len && (len || ep->iPackets))
		{
		if (ep->iZlpReqd)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf(" > 'Transmit Short Packet' explicitly"));
			//  Arrange for the sending of a ZLP here.
			AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, KClearNone, K_TX_TXPKTRDY); //  FIFO_NOT_EMPTY, TXPKTRDY
			ep->iZlpReqd = EFalse;
			}
		else
			{
			__KTRACE_OPT(KUSB, Kern::Printf(" > All data sent: %d --> completing", len));
			ep->iTxBuf = NULL;
			ep->iRequest->iTxBytes = ep->iTransmitted;
			ep->iRequest->iError = KErrNone;
			EndpointRequestComplete(ep->iRequest);
			ep->iRequest = NULL;
			}
		}
	else
		{
		buf += t;
		TInt left = len - t;								// left in total
		TInt n = (left >= KBlkMaxPktSz) ? KBlkMaxPktSz : left; // now to be transmitted
		__KTRACE_OPT(KUSB, Kern::Printf(" > About to send %d bytes (%d bytes left in total)", n, left));

		// Write data into Tx FIFO from 'buf' here...
		TInt x=0;
		TInt FIFOAddr = K_FIFO0_REG+K_FIFO_OFFSET*(TInt)((aEndpoint)/2);
		for(; x<n; x++) // While FIFO is not full...
			{
			// Should try and check we aren't running out of FIFO!
			AsspRegister::Write8(KUSBBase+FIFOAddr, buf[x]);
			}
		AsspRegister::Modify16(KUSBBase+K_PERI_TXCSR_REG, KClearNone, /*K_TX_FIFONOTEMPTY | */K_TX_TXPKTRDY); //  TXPKTRDY	
		ep->iTransmitted += x;
		ep->iPackets++;										// only used for (len == 0) case
		left -= n;											// (still) left in total

		// If double-buffering is available, it might be possible to stick a second packet
		// into the FIFO here.

		// TO DO (optional): Send another packet if possible (& available) here.
			
		}
	if(calledFromISR)
		{
		ClearEndpointInterrupt(aEndpoint);
		}
	Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
	}



void DOmap3530Usbcc::BulkReceive(TInt aEndpoint)
//
// Endpoint 2 (BULK OUT) (This one is called in an ISR.)
//
	{
	Interrupt::Disable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::BulkReceive(%d)", aEndpoint));
	
	AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aEndpoint/2));
	
	// Start NYETTING packets..
	AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_DISNYET,  KSetNone);

	TInt status = AsspRegister::Read16(KUSBBase+K_PERI_RXCSR_REG); 
	
	if(status & K_RX_OVERRUN)
		{
		// RX OVERRUN
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_OVERRUN, KSetNone); 
		}
	if(status & K_RX_SENTSTALL)
		{
		// Complete stall handshake
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_SENTSTALL,  K_RX_DISNYET); 
		Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
		return;
		}

	if(status & K_RX_SENDSTALL)
		{
		// We are stalled
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, KClearNone,  K_RX_DISNYET); 
		Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
		return;
		}

	TBool calledFromISR=AsspRegister::Read16(KUSBBase+K_INTRRX_REG) & 1<<(aEndpoint/2)==1;
	
	const TInt idx = aEndpoint;			// only in our special case of course!
	TEndpoint* const ep = &iEndpoints[idx];
	TUint8* buf = ep->iRxBuf;
	if (!buf)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > No Rx buffer available: setting iNoBuffer"));
		ep->iNoBuffer = ETrue;
		DisableEndpointInterrupt(aEndpoint);
		ep->iDisabled = ETrue;
		ClearEndpointInterrupt(aEndpoint);
		Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, KClearNone,  K_RX_DISNYET); 
		return;
		}
	TInt bytes = AsspRegister::Read16(KUSBBase+K_RXCOUNT_REG);
	const TInt r = ep->iReceived;							// already received
	// Check whether a ZLP was received here:
	if (bytes==0)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > received zero-length packet"));
		}
	else// if (status & 2)									// some other condition
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > Bulk received: %d bytes", bytes));
		if (r + bytes > ep->iLength)
			{
			__KTRACE_OPT(KUSB, Kern::Printf(" > not enough space in rx buffer: setting iNoBuffer"));
			ep->iNoBuffer = ETrue;
			StopRxTimer(ep);
			*ep->iPacketSize = ep->iReceived;
			RxComplete(ep);

			if(calledFromISR)
				{
				ClearEndpointInterrupt(aEndpoint);
				}
			AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, KClearNone,  K_RX_DISNYET); 
			Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
			return;
			}
		buf += r;											// set buffer pointer

		// Read 'bytes' bytes from Rx FIFO into 'buf' here.
		TInt FIFOAddr = K_FIFO0_REG+K_FIFO_OFFSET*(TInt)((aEndpoint)/2);
		for(TInt n=0; n<bytes; n++)
			{
			// Should try and check we aren't running out of FIFO!
			buf[n] = AsspRegister::Read8(KUSBBase+FIFOAddr);
			}
		
		ep->iReceived += bytes;
		}

	if (bytes == 0)
		{
		// ZLPs must be recorded separately
		const TInt i = ep->iReceived ? 1 : 0;
		ep->iPacketIndex[i] = r;
		ep->iPacketSize[i] = 0;
		// If there were data packets before: total packets reported 1 -> 2
		ep->iPackets += i;
		}

	if ((bytes < KBlkMaxPktSz) ||
		(ep->iReceived == ep->iLength))
		{
		StopRxTimer(ep);
		*ep->iPacketSize = ep->iReceived;
		RxComplete(ep);
		// since we have no buffer any longer we disable interrupts:
		DisableEndpointInterrupt(aEndpoint);
		ep->iDisabled = ETrue;
		}
	else
		{
		if (!ep->iRxTimerSet)
			{
			__KTRACE_OPT(KUSB, Kern::Printf(" > setting rx timer"));
			ep->iRxTimerSet = ETrue;
			ep->iRxTimer.OneShot(KRxTimerTimeout);
			}
		else
			{
			ep->iRxMoreDataRcvd = ETrue;
			}
		}
	if(calledFromISR)
		{
		ClearEndpointInterrupt(aEndpoint);
		}
	// Clear Ep Rx condition flags here.
	AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_RXPKTRDY,  K_RX_DISNYET); 
	Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
	}


void DOmap3530Usbcc::BulkReadRxFifo(TInt aEndpoint)
//
// Endpoint 2 (BULK OUT) (This one is called w/o interrupt to be served.)
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::BulkReadRxFifo(%d)", aEndpoint));
	Interrupt::Disable(EOmap3530_IRQ92_HSUSB_MC_NINT);	

	AsspRegister::Write8(KUSBBase+K_INDEX_REG, (TInt)(aEndpoint/2));
	
	// Start NYETTING packets..
	AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_DISNYET,  KSetNone);
	
	TInt status = AsspRegister::Read16(KUSBBase+K_PERI_RXCSR_REG); 
	if(status & K_RX_OVERRUN)
		{
		// RX OVERRUN
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_OVERRUN, KSetNone); 
		}
	if(status & K_RX_SENTSTALL)
		{
		// Complete stall handshake
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_SENTSTALL, K_RX_DISNYET); 		
		Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
		return;
		}
	if(status & K_RX_SENTSTALL)
		{
		// We are stalled
		AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, KClearNone,  K_RX_DISNYET); 
		Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
		return;
		}
	
	TBool calledFromISR=AsspRegister::Read16(KUSBBase+K_INTRRX_REG) & 1<<(aEndpoint/2)==1;

	const TInt idx = aEndpoint;				// only in our special case of course!
	TEndpoint* const ep = &iEndpoints[idx];
	TUint8* buf = ep->iRxBuf;
	if (!buf)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: No Rx buffer has been set up"));
		Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
		return;
		}
	TInt bytes =  AsspRegister::Read16(KUSBBase+K_RXCOUNT_REG);
	const TInt r = ep->iReceived;							// already received
	// Check whether a ZLP was received here:
	if (bytes==0)											// some condition
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > received zero-length packet"));
		}
	else //if (status & 2)									// some other condition
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > Bulk received: %d bytes", bytes));
		if (r + bytes > ep->iLength)
			{
			__KTRACE_OPT(KUSB, Kern::Printf(" > not enough space in rx buffer: setting iNoBuffer"));
			ep->iNoBuffer = ETrue;
			*ep->iPacketSize = ep->iReceived;
			RxComplete(ep);
			
			// Stop NYETting packets
			AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, KClearNone,  K_RX_DISNYET); 
			Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
			return;
			}
		buf += r;											// set buffer pointer

		// TO DO: Read 'bytes' bytes from Rx FIFO into 'buf' here.
		TInt FIFOAddr = K_FIFO0_REG+K_FIFO_OFFSET*(TInt)((aEndpoint)/2);
		for(TInt n=0; n<bytes; n++)
			{
			// Should try and check we aren't running out of FIFO!
			buf[n] = AsspRegister::Read8(KUSBBase+FIFOAddr);
			}
		ep->iReceived += bytes;
		}
	if (bytes == 0)
		{
		// ZLPs must be recorded separately
		const TInt i = ep->iReceived ? 1 : 0;
		ep->iPacketIndex[i] = r;
		ep->iPacketSize[i] = 0;
		// If there were data packets before: total packets reported 1 -> 2
		ep->iPackets += i;
		}

	if ((bytes < KBlkMaxPktSz) ||
		(ep->iReceived == ep->iLength))
		{
		*ep->iPacketSize = ep->iReceived;
		RxComplete(ep);
		}
	else
		{
		if (!ep->iRxTimerSet)
			{
			__KTRACE_OPT(KUSB, Kern::Printf(" > setting rx timer"));
			ep->iRxTimerSet = ETrue;
			ep->iRxTimer.OneShot(KRxTimerTimeout);
			}
		else
			{
			ep->iRxMoreDataRcvd = ETrue;
			}
		}

	if(calledFromISR)
		{
		ClearEndpointInterrupt(aEndpoint);
		}

	// Stop NYETting packets and Clear Ep Rx condition flags here.
	AsspRegister::Modify16(KUSBBase+K_PERI_RXCSR_REG, K_RX_RXPKTRDY, K_RX_DISNYET); 
	Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
	}


void DOmap3530Usbcc::IsoTransmit(TInt aEndpoint)
//
// Endpoint 3 (ISOCHRONOUS IN).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::IsoTransmit(%d)", aEndpoint));

	// TO DO: Write data to endpoint FIFO. Might be similar to BulkTransmit.

	}


void DOmap3530Usbcc::IsoReceive(TInt aEndpoint)
//
// Endpoint 4 (ISOCHRONOUS OUT) (This one is called in an ISR.)
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::IsoReceive(%d)", aEndpoint));

	// TO DO: Read data from endpoint FIFO. Might be similar to BulkReceive.
	}


void DOmap3530Usbcc::IsoReadRxFifo(TInt aEndpoint)
//
// Endpoint 4 (ISOCHRONOUS OUT) (This one is called w/o interrupt to be served.)
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::IsoReadRxFifo(%d)", aEndpoint));

	// TO DO: Read data from endpoint FIFO. Might be similar to BulkReadRxFifo.
	}


void DOmap3530Usbcc::IntTransmit(TInt aEndpoint)
//
// Endpoint 5 (INTERRUPT IN).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::IntTransmit(%d)", aEndpoint));

	// TO DO: Write data to endpoint FIFO. Might be similar to BulkTransmit.
	}


void DOmap3530Usbcc::RxComplete(TEndpoint* aEndpoint)
//
// Called at the end of an Rx (OUT) transfer to complete to the PIL.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::RxComplete"));
	TUsbcRequestCallback* const req = aEndpoint->iRequest;	

	__ASSERT_DEBUG((req != NULL), Kern::Fault(KUsbPanicCat, __LINE__));

	aEndpoint->iRxBuf = NULL;
	aEndpoint->iRxTimerSet = EFalse;
	aEndpoint->iRxMoreDataRcvd = EFalse;
	req->iRxPackets = aEndpoint->iPackets;
	req->iError = aEndpoint->iLastError;
	EndpointRequestComplete(req);
	aEndpoint->iRequest = NULL;
	}


void DOmap3530Usbcc::StopRxTimer(TEndpoint* aEndpoint)
//
// Stops (cancels) the Rx timer for an endpoint.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::StopRxTimer"));

	if (aEndpoint->iRxTimerSet)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > stopping rx timer"));
		aEndpoint->iRxTimer.Cancel();
		aEndpoint->iRxTimerSet = EFalse;
		}
	}


void DOmap3530Usbcc::EndpointIntService(TInt aEndpoint)
//
// ISR for endpoint interrupts.
// Note: the aEndpoint here is a "hardware endpoint", not a aRealEndpoint.
//
	{
	switch (aEndpoint)
		{
	case 0:
		Ep0IntService();
		break;
	case 3:
	case 5:
	case 7:
	case 9:
	case 11:
	case 13:
	case 15:
	case 17:
	case 19:
	case 21:
	case 23:
	case 25:
	case 27:
	case 29:
		BulkTransmit(aEndpoint);	
		break;
	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	case 14:
	case 16:
	case 18:
	case 20:
	case 22:
	case 24:
	case 26:
	case 28:
		BulkReceive(aEndpoint);			
		break;
	case 30:
		IntTransmit(aEndpoint);
		break;
	default:
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Endpoint not found"));
		break;
		}
	}


TInt DOmap3530Usbcc::ResetIntService()
//
// ISR for a USB Reset event interrupt.
// This function returns a value which can be used on the calling end to decide how to proceed.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::ResetIntService"));

	// Clear an interrupt:
	// TO DO: Clear reset interrupt flag here.

	// TO DO (optional): Enquire about special conditions and possibly return here.
	
	DeviceEventNotification(EUsbEventReset);

	return KErrNone;
	}


void DOmap3530Usbcc::SuspendIntService()
//
// ISR for a USB Suspend event interrupt.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SuspendIntService"));

	// Clear an interrupt:
	// TO DO: Clear suspend interrupt flag here.

	DeviceEventNotification(EUsbEventSuspend);
	}


void DOmap3530Usbcc::ResumeIntService()
//
// ISR for a USB Resume event interrupt.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::ResumeIntService"));

	// Clear an interrupt:
	// TO DO: Clear resume interrupt flag here.

	DeviceEventNotification(EUsbEventResume);
	}


void DOmap3530Usbcc::SofIntService()
//
// ISR for a USB Start-of-Frame event interrupt.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SofIntService"));

	// Clear an interrupt:
	// TO DO: Clear SOF interrupt flag here.
	
	// TO DO (optional): Do something about the SOF condition.
	}


void DOmap3530Usbcc::UdcInterruptService()
//
// Main UDC ISR - determines the cause of the interrupt, clears the condition, dispatches further for service.
//
	{
	Interrupt::Disable(EOmap3530_IRQ92_HSUSB_MC_NINT);	
	TUint status = AsspRegister::Read8(KUSBBase+K_INTRUSB_REG);	

	// Reset interrupt
	if (status &  K_INT_RESET)
		{
		ResetIntService();
		}

	// Resume interrupt
	if (status & K_INT_RESUME)
		{
		ResumeIntService();
		}

	// Endpoint interrupt
	TUint TxEpInt = AsspRegister::Read16(KUSBBase+K_INTRTX_REG);	

	TInt ep=0;
	for(TInt x=0; TxEpInt!=0 && x<16 ; x++)
		{
		if(TxEpInt&(1<<x))
			{
			EndpointIntService(ep);
			}
		if(ep==0) { ep++; } // TX EP's are odd numbered - numbers are array indicies so we start from 2
		ep+=2;
		}	
	TUint RxEpInt = AsspRegister::Read16(KUSBBase+K_INTRRX_REG);
	ep=2;
	for(TInt x=1; RxEpInt!=0 && x<16; x++)
		{
		if(RxEpInt&(1<<x))
			{
			EndpointIntService(ep);
			}
		ep+=2;
		}
		
	// Suspend interrupt should be serviced last
	if (status & K_INT_SUSPEND)
		{
		SuspendIntService();
		}
	
	Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);
	}


void DOmap3530Usbcc::Ep0NextState(TEp0State aNextState)
//
// Moves the Ep0 state to aNextState.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::Ep0NextState"));
	iEp0State = aNextState;
	}


void DOmap3530Usbcc::UdcIsr(TAny* aPtr)
//
// This is the static ASSP first-level UDC interrupt service routine. It dispatches the call to the
// actual controller's ISR.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::UdcIsr"));
	static_cast<DOmap3530Usbcc*>(aPtr)->UdcInterruptService();
	}


TInt DOmap3530Usbcc::UsbClientConnectorCallback(TAny* aPtr)
//
// This function is called in ISR context by the Variant's UsbClientConnectorInterruptService.
// (This function is static.)
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::UsbClientConnectorCallback"));

	DOmap3530Usbcc* const ptr = static_cast<DOmap3530Usbcc*>(aPtr);
	ptr->iCableConnected = ptr->iAssp->UsbClientConnectorInserted();
#ifdef _DEBUG
	_LIT(KIns, "inserted");
	_LIT(KRem, "removed");
	__KTRACE_OPT(KUSB, Kern::Printf(" > USB cable now %lS", ptr->iCableConnected ? &KIns : &KRem));
#endif
	if (ptr->iCableConnected)
		{
		ptr->DeviceEventNotification(EUsbEventCableInserted);
		}
	else
		{
		ptr->DeviceEventNotification(EUsbEventCableRemoved);
		}

	return KErrNone;
	}


TInt DOmap3530Usbcc::SetupUdcInterrupt()
//
// Registers and enables the UDC interrupt (ASSP first level interrupt).
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::SetupUdcInterrupt"));

	TInt error = Interrupt::Bind(EOmap3530_IRQ92_HSUSB_MC_NINT, UdcIsr, this);
	if (error != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Binding UDC interrupt failed"));
		return error;
		}
	Interrupt::Enable(EOmap3530_IRQ92_HSUSB_MC_NINT);
	return KErrNone;
	}


void DOmap3530Usbcc::ReleaseUdcInterrupt()
//
// Disables and unbinds the UDC interrupt.
//
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::ReleaseUdcInterrupt"));

	Interrupt::Disable(EOmap3530_IRQ92_HSUSB_MC_NINT);
	Interrupt::Unbind(EOmap3530_IRQ92_HSUSB_MC_NINT);
	}

	
void DOmap3530Usbcc::EnableSICLK()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::EnableSICLK"));
	if(iSICLKEnabled==0)
		{
		//TInt r = PowerResourceManager::ChangeResourceState( iPrmClientId, Omap3530Prm::EPrmClkHsUsbOtg_I, Prcm::EClkAuto );
		// What are we supposed to do with errors from PRM?
		
		
		AsspRegister::Modify32(KCM_ICLKEN1_CORE, KClearNone, KENHOSTOTGUSB_BIT);
		AsspRegister::Modify32(KCM_AUTOIDLE1_CORE, KClearNone, KAUTO_HOSTOTGUSB_BIT);

		
		__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc: SICLK Enabled"));
		}
	iSICLKEnabled++;
	}

void DOmap3530Usbcc::DisableSICLK()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::DisableSICLK"));
	if(iSICLKEnabled==1)
		{
		//TInt r = PowerResourceManager::ChangeResourceState( iPrmClientId, Omap3530Prm::EPrmClkHsUsbOtg_I, Prcm::EClkOff );
		// What are we supposed to do with errors from PRM?
		
		AsspRegister::Modify32(KCM_ICLKEN1_CORE, KENHOSTOTGUSB_BIT, KSetNone);
		AsspRegister::Modify32(KCM_AUTOIDLE1_CORE, KAUTO_HOSTOTGUSB_BIT, KSetNone);

		
		}
	if(iSICLKEnabled>0)
		{
		iSICLKEnabled--;
		}
	}

TBool DOmap3530Usbcc::CurrentlyUsingHighSpeed()
	{
	return ETrue;
	}

void DOmap3530Usbcc::SuspendDfcFn(TAny *aPtr)
	{

	}
	
void DOmap3530Usbcc::ResumeDfcFn(TAny *aPtr)
	{

	}
	
void DOmap3530Usbcc::ResetDfcFn(TAny *aPtr)
	{
	DOmap3530Usbcc* self = reinterpret_cast<DOmap3530Usbcc*>(aPtr);
	// Put the Transceiver into normal mode
	self->iPhy->EnablePHY();
	self->iPhy->SetPHYMode(ENormal);
	self->iPhy->DisablePHY();
	}
	
TBool DOmap3530Usbcc::DeviceHighSpeedCaps() const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DOmap3530Usbcc::DeviceHighSpeedCaps()"));
	return ETrue;
	}

	
//
// --- DLL Exported Function --------------------------------------------------
//

DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KUSB, Kern::Printf(" > Initializing USB client support (Udcc)..."));

	DOmap3530Usbcc* const usbcc = new DOmap3530Usbcc();
	if (!usbcc)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for DOmap3530Usbcc failed"));
		return KErrNoMemory;
		}
Kern::Printf( "$1" );
	TInt r;
	if ((r = usbcc->Construct()) != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Construction of DOmap3530Usbcc failed (%d)", r));
		delete usbcc;
		return r;
		}
Kern::Printf( "$2" );

	if (usbcc->RegisterUdc(0) == NULL)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: PIL registration of PSL failed"));
		delete usbcc;
		return KErrGeneral;
		}
	
	__KTRACE_OPT(KUSB, Kern::Printf(" > Initializing USB client support: Done"));

	return KErrNone;
	}


// --- EOF --------------------------------------------------------------------
