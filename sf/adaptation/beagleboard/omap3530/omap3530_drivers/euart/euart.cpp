// This component and the accompanying materials are made available
// under the terms of the License "Symbian Foundation License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.symbianfoundation.org/legal/sfl-v10.html".
//
// Initial Contributors:
// Accenture
//
// Contributors:
//
// Description:
// \sf\adaptation\beagleboard\omap3530\omap3530_drivers\euart\euart.cpp
// pdd for serial ports
// assume Modem Control Signals change cause an interrupt
// 
//

#include <d32comm.h>
#include <beagle\variant.h>
#include "euart.h"

#define TX_POLLING_HACK

_LIT(KPddName,"Comm.Beagle");
using namespace Omap3530Uart;

DDriverComm::DDriverComm()
//
// Constructor
//
	{
	iUnitsMask=0x7;
	iVersion=TVersion(KCommsMajorVersionNumber,KCommsMinorVersionNumber,KCommsBuildVersionNumber);
	}

TInt DDriverComm::Install()
//
// Install the driver
//
	{
	return SetName(&KPddName);
	}

void GetTemplateCommsCaps(TDes8 &aCaps, TInt aUnit)
	{
	TCommCaps2 capsBuf;

	// Fill in the Caps structure with the relevant information for this Unit, e.g
	TCommCapsV02 &c=capsBuf();
	c.iRate=KCapsBps1200|KCapsBps2400|KCapsBps4800|KCapsBps9600|KCapsBps19200|KCapsBps38400|
			KCapsBps57600|KCapsBps115200|KCapsBps230400|KCapsBps460800|KCapsBps4000000;
			
	c.iDataBits=KCapsData5|KCapsData6|KCapsData7|KCapsData8;
	c.iStopBits=KCapsStop1|KCapsStop2;
	c.iParity=KCapsParityNone|KCapsParityEven|KCapsParityOdd|KCapsParityMark|KCapsParitySpace;
	c.iHandshake=0; //(OR in all KCapsObeyXXXSupported, KCapsSendXXXSupported, KCapsFailXXXSupported, KCapsFreeXXXSupported
				    // as required for this Unit's configuration);.
	c.iSignals=0;   //(OR in as many KCapsSignalXXXSupported as Modem control signals controllable by this Unit);
	c.iSIR=0; 		//(0 or OR in as many KCapsSIRXXX as IR bit rates supported);
	c.iNotificationCaps=0; //(OR in as many KNotifyXXXSupported as notifications supported by this Unit);
	c.iFifo=0;				//(0 or KCapsHasFifo);
	c.iRoleCaps=0;			//(0 or KCapsRoleSwitchSupported);
	c.iFlowControlCaps=0;	//(0 or KCapsFlowControlStatusSupported);
	/** @see TCommCapsV02 */

	aCaps=capsBuf.Left(Min(capsBuf.Length(),aCaps.MaxLength()));
	}

void DDriverComm::GetCaps(TDes8 &aDes) const
//
// Return the drivers capabilities
//
	{
	GetTemplateCommsCaps(aDes, 0);
	}

TInt DDriverComm::Create(DBase*& aChannel, TInt aUnit, const TDesC8* anInfo, const TVersion& aVer)
//
// Create a driver
//
	{
	DCommBeagle* pD=new DCommBeagle;
	aChannel=pD;
	TInt r=KErrNoMemory;
	if (pD)
		r=pD->DoCreate(aUnit,anInfo);
	return r;
	}

TInt DDriverComm::Validate(TInt aUnit, const TDesC8* /*anInfo*/, const TVersion& aVer)
//
//	Validate the requested configuration (Version and Unit)
//
	{
	if ((!Kern::QueryVersionSupported(iVersion,aVer)) || (!Kern::QueryVersionSupported(aVer,TVersion(KMinimumLddMajorVersion,KMinimumLddMinorVersion,KMinimumLddBuild))))
		return KErrNotSupported;
		
	if(aUnit<0 || aUnit >2)
		return KErrNotSupported;
		
	return KErrNone;
	}

DCommBeagle::DCommBeagle()
//
// Constructor
//
	{
	gData=0;
	iInterruptId=-1;		// -1 means not bound
	}

DCommBeagle::~DCommBeagle()
//
// Destructor
//
	{
	if (iInterruptId>=0)
		Interrupt::Unbind(iInterruptId);
	
	if (iDfcQ)
		{
		iDfcQ->Destroy();
		}
	}

const TInt KDCommBeagleDfcThreadPriority = 24;
_LIT(KDCommBeagleDfcThread,"DCommBeagleDfcThread");

TInt DCommBeagle::DoCreate(TInt aUnit, const TDesC8* /*anInfo*/)
//
// Sets up the PDD
//
	{
	iUnit=aUnit;

	//  Create own DFC queue
	TInt r = Kern::DynamicDfcQCreate(iDfcQ, KDCommBeagleDfcThreadPriority, KDCommBeagleDfcThread);

	if (r != KErrNone)
		return r; 	

	// Set iPortAddr and irq with the Linear Base address of the UART and the Interrupt ID coresponding to aUnit
	iUart = new TUart((TUartNumber)aUnit);
	Prcm::SetClockState(iUart->PrcmInterfaceClk(), Prcm::EClkOn);
	Prcm::SetClockState(iUart->PrcmFunctionClk(), Prcm::EClkOn);
	
	// bind to UART interrupt
	r=Interrupt::Bind(iUart->InterruptId(),Isr,this);
	if (r==KErrNone)
		iInterruptId=iUart->InterruptId();
	return r;
	}

TDfcQue* DCommBeagle::DfcQ(TInt aUnit)
//
// Return the DFC queue to be used for this device
// For UARTs just use the standard low priority DFC queue
// For Serial PC cards, use the PC card controller thread for the socket in question.
//
	{
	return aUnit==iUnit ? iDfcQ : NULL;
	}

TInt DCommBeagle::Start()
//
// Start receiving characters
//
	{
	if(iUnit == Kern::SuperPage().iDebugPort)
		Variant::MarkDebugPortOff();
	
	iTransmitting=EFalse;			// if EnableTransmit() called before Start()

	if(iUart==NULL)
		{
		return KErrNotReady;
		}
	iSignals=Signals();
	iLdd->UpdateSignals(iSignals);

	Interrupt::Enable(iInterruptId);
	iUart->EnableInterrupt(TUart::EIntRhr);
	return KErrNone;
	}

TBool FinishedTransmitting(TAny* aPtr)
	{
	DCommBeagle* self = (DCommBeagle*)aPtr;
	if(self->iUart==NULL)
		{
		return ETrue;
		}
		
	if(!self->iUart->TxFifoEmpty())
		{
		return EFalse;
		}
	return ETrue;
	}

void DCommBeagle::Stop(TStopMode aMode)
//
// Stop receiving characters
//
	{
	switch (aMode)
		{
		case EStopNormal:
		case EStopPwrDown:
			iUart->DisableInterrupt(TUart::EIntRhr);
			iUart->DisableInterrupt(TUart::EIntThr);
			Interrupt::Disable(iInterruptId);
			
			// wait for uart to stop tranmitting
			Kern::PollingWait(FinishedTransmitting,this,3,100);
			
			iTransmitting=EFalse;
			
			iUart->Disable();			
			Prcm::SetClockState(iUart->PrcmFunctionClk(), Prcm::EClkOff);
			break;
		case  EStopEmergency:
			iUart->DisableInterrupt(TUart::EIntRhr);			
			iUart->DisableInterrupt(TUart::EIntThr);
			Interrupt::Disable(iInterruptId);
			iTransmitting=EFalse;
			break;
		}
	}

void DCommBeagle::Break(TBool aState)
//
// Start or stop the uart breaking
//
	{
	if (aState)
		{
		//
		// TO DO: (mandatory)
		//
		// Enable sending a Break (space) condition
		//
		}
	else
		{
		//
		// TO DO: (mandatory)
		//
		// Stop sending a Break (space) condition
		//
		}
	}

void DCommBeagle::EnableTransmit()
//
// Start sending characters.
//
	{
	TBool tx=(TBool)NKern::SafeSwap((TAny*)ETrue,(TAny*&)iTransmitting);
	if (tx)
		{
		return;
		}
	TInt r = 0;
	#ifdef TX_POLLING_HACK
	while (Kern::PowerGood())
	#else
	while (Kern::PowerGood() && !iUart->TxFifoFull())
	#endif
		{	
		r=TransmitIsr();
		if(r<0)
			{
			//no more to send
			iTransmitting=EFalse;
			break;
			}
		#ifdef TX_POLLING_HACK
		while(iUart->TxFifoFull());
		#endif
		iUart->Write(r);
		}
	TInt irq=0;
	if (!iInInterrupt)					// CheckTxBuffer adds a Dfc: can only run from ISR or with NKernel locked
		{
		NKern::Lock();
		irq=NKern::DisableAllInterrupts();
		}
	CheckTxBuffer();
	if (!iInInterrupt)
		{
		NKern::RestoreInterrupts(irq);
		NKern::Unlock();
		}
	if (r>=0)											// only enable interrupt if there's more data to send
		{
		iUart->EnableInterrupt(TUart::EIntThr);
		}
	}

TUint DCommBeagle::Signals() const
//
// Read and translate the modem lines
//
	{
	TUint signals=0;
	//
	// TO DO: (mandatory)
	//
	// If the UART corresponding to iUnit supports Modem Control Signals, read them and return a bitmask with one or 
	// more of the following OR-ed in:
	// - KSignalDTR,
	// - KSignalRTS,
	// - KSignalDSR,
	// - KSignalCTS,
	// - KSignalDCD.
	// 
	return signals;
	}

void DCommBeagle::SetSignals(TUint aSetMask, TUint aClearMask)
//
// Set signals.
//
	{
	//
	// TO DO: (mandatory)
	//
	// If the UART corresponding to iUnit supports Modem Control Signals, converts the flags in aSetMask and aClearMask 
	// into hardware-specific bitmasks to write to the UART modem/handshake output register(s). 
	// aSetMask, aClearMask will have one or more of the following OR-ed in:
	// - KSignalDTR,
	// - KSignalRTS,
	//
	}

TInt DCommBeagle::ValidateConfig(const TCommConfigV01 &aConfig) const
//
// Check a config structure.
//
	{
	//
	// TO DO: (mandatory)
	//
	// Checks the the options in aConfig are supported by the UART corresponding to iUnit
	// May need to check:
	//  - aConfig.iParity (contains one of EParityXXX)
	/** @see TParity */
	//  - aConfig.iRate (contains one of EBpsXXX)
	/** @see TBps */
	//  - aConfig.iDataBits (contains one of EDataXXX)
	/** @see TDataBits */
	//  - aConfig.iStopBits (contains one of EStopXXX)
	/** @see TDataBits */
	//  - aConfig.iHandshake (contains one of KConfigObeyXXX or KConfigSendXXX or KConfigFailXXX or KConfigFreeXXX)
	//  - aConfig.iParityError (contains KConfigParityErrorFail or KConfigParityErrorIgnore or KConfigParityErrorReplaceChar)
	//  - aConfig.iFifo (contains ether EFifoEnable or EFifoDisable)
	/** @see TFifo */
	//  - aConfig.iSpecialRate (may contain a rate not listed under TBps)
	//  - aConfig.iTerminatorCount (conatains number of special characters used as terminators)
	//  - aConfig.iTerminator[] (contains a list of special characters which can be used as terminators)
	//  - aConfig.iXonChar (contains the character used as XON - software flow control)
	//  - aConfig.iXoffChar (contains the character used as XOFF - software flow control)
	//  - aConfig.iParityErrorChar (contains the character used to replace bytes received with a parity error)
	//  - aConfig.iSIREnable (contains either ESIREnable or ESIRDisable)
	/** @see TSir */
	//  - aConfig.iSIRSettings (contains one of KConfigSIRXXX)
	// and returns KErrNotSupported if the UART corresponding to iUnit does not support this configuration
	//
	return KErrNone;
	}

void DCommBeagle::CheckConfig(TCommConfigV01& aConfig)
	{
	//
	// TO DO: (optional)
	//
	// Validates the default configuration that is defined when a channel is first opened
	//
	}

TInt DCommBeagle::DisableIrqs()
//
// Disable normal interrupts
//
	{
	
	return NKern::DisableInterrupts(1);
	}

void DCommBeagle::RestoreIrqs(TInt aLevel)
//
// Restore normal interrupts
//
	{
	
	NKern::RestoreInterrupts(aLevel);
	}

void DCommBeagle::Configure(TCommConfigV01 &aConfig)
//
// Configure the UART from aConfig
//
	{
	Kern::PollingWait(FinishedTransmitting,this,3,100);	// wait for uart to stop tranmitting
	iUart->Disable();
	
	iUart->Init();
	iUart->DefineMode(TUart::EUart);

	switch(aConfig.iRate)
		{
		case EBps1200:
			iUart->SetBaud(TUart::E1200);
			break;
		case EBps2400:
			iUart->SetBaud(TUart::E2400);
			break;
		case EBps4800:
			iUart->SetBaud(TUart::E4800);
			break;
		case EBps9600:
			iUart->SetBaud(TUart::E9600);
			break;
		case EBps19200:
			iUart->SetBaud(TUart::E19200);
			break;
		case EBps38400:
			iUart->SetBaud(TUart::E38400);
			break;
		case EBps57600:
			iUart->SetBaud(TUart::E57600);
			break;
		case EBps460800:
			iUart->SetBaud(TUart::E460800);
			break;
		case EBps115200:
		default:
			iUart->SetBaud(TUart::E115200);
		}
	
	TUart::TDataBits databits = (TUart::TDataBits)aConfig.iDataBits;
	TUart::TStopBits stopbits;
	switch(aConfig.iStopBits)
		{
		case EStop2:
			stopbits=TUart::E2Stop;
			break;
		case EStop1:
		default:
			stopbits=TUart::E1Stop;		
		}

	TUart::TParity parity;	
	switch(aConfig.iParity)
		{
		case EParityEven:
			parity=TUart::EEven;
			break;
		case EParityOdd:
			parity=TUart::EOdd;
			break;
		case EParityMark:
			parity=TUart::EMark;
			break;
		case EParitySpace:
			parity=TUart::ESpace;
			break;
		case EParityNone:
		default:
			parity=TUart::ENone;
		}

	iUart->SetDataFormat(databits, stopbits, parity);
	iUart->EnableFifo(TUart::EEnabled, TUart::ETrigger8, TUart::ETrigger8);

	iUart->Enable();
	}

void DCommBeagle::Caps(TDes8 &aCaps) const
//
// return our caps
//
	{
	GetTemplateCommsCaps(aCaps,iUnit);
	}

void DCommBeagle::Isr(TAny* aPtr)
//
// Service the UART interrupt
//
	{
	DCommBeagle& d=*(DCommBeagle*)aPtr;
	d.iInInterrupt=1;										// going in...
	TUint xon=d.iLdd->iRxXonChar;
	TUint xoff=d.iLdd->iRxXoffChar;

	// Read the interrupt source register to determine if it is a Receive, Transmit or Modem Signals change interrupt.
	// If required also, clear interrupts at the source.
	// Then process the interrupt condition as in the following pseudo-code extract:
	//
	TUint interruptId = IIR::iMem.Read(*(d.iUart));
	TUint interruptEn = IER::iMem.Read(*(d.iUart));	
	
	if((interruptId & IIR::IT_TYPE::ETHR) && (interruptEn & IER::THR_IT::KMask) )
		{
		while(Kern::PowerGood() && !d.iUart->TxFifoFull())	
			{
			TInt r=d.TransmitIsr();
			if(r<0)
				{
				// Disable the Transmit Interrupt in Hardware
				d.iUart->DisableInterrupt(TUart::EIntThr);
				d.iTransmitting=EFalse;
				break;
				}
			d.iUart->Write(r);
			}
		d.CheckTxBuffer();
		}
		
	if((interruptId & IIR::IT_TYPE::ERHR) && (interruptEn & IER::RHR_IT::KMask) )
		{
		TUint rx[32];
		TInt rxi=0;
		TInt x=0;

		while(!d.iUart->RxFifoEmpty() && rxi<32)
			{
			TUint ch = d.iUart->Read();
			if (ch==xon)
				x=1;
			else if (ch==xoff)
				x=-1;
			else
				rx[rxi++]=ch;
			}
		d.ReceiveIsr(rx,rxi,x);
		}

	if((interruptId & IIR::IT_TYPE::EModem) && (interruptEn & IER::MODEM_STS_IT::KMask) )
		{
		TUint signals=d.Signals()&KDTEInputSignals;
		if (signals != d.iSignals)
			{
			d.iSignals=signals;
			d.iLdd->StateIsr(signals);
			}
		}
	d.iInInterrupt=0;										// going out...
	}

DECLARE_STANDARD_PDD()
	{
	return new DDriverComm;
	}
