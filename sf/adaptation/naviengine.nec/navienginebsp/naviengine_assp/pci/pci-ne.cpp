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
*
*/



#include "pci-ne.h"
#include "../naviengine_pci.h"
#include <naviengine_priv.h>
#include <naviengine.h>


_LIT(KTConfigSpace, "TConfigSpace");
TConfigSpace::TConfigSpace(TPciFunction& aFunction, DPciBridge& aBridge)
	:TAddrSpace(KCfgSpaceSize, KTConfigSpace), iFunction(aFunction), iBridge(aBridge)
	{
	}

/**
Read 1 byte from config space. Config space is 256 bytes long
out of range access will fault the kernel.
*/
EXPORT_C TUint8 TConfigSpace::Read8(TUint32 aOffset)
	{
	CheckAccess(E1Byte, aOffset);

	return iBridge.ReadConfig8(
			iFunction.Bus(),
			iFunction.Device(),
			iFunction.Function(),
			aOffset
			);
	}

EXPORT_C TUint16 TConfigSpace::Read16(TUint32 aOffset)
	{
	CheckAccess(E2Byte, aOffset);

	return iBridge.ReadConfig16(
			iFunction.Bus(),
			iFunction.Device(),
			iFunction.Function(),
			aOffset
			);
	}

EXPORT_C TUint32 TConfigSpace::Read32(TUint32 aOffset)
	{
	CheckAccess(E4Byte, aOffset);

	return iBridge.ReadConfig32(
			iFunction.Bus(),
			iFunction.Device(),
			iFunction.Function(),
			aOffset
			);
	}

EXPORT_C void TConfigSpace::Write8(TUint32 aOffset, TUint8 aValue)
	{
	CheckAccess(E1Byte, aOffset);

	iBridge.WriteConfig8(
			iFunction.Bus(),
			iFunction.Device(),
			iFunction.Function(),
			aOffset,
			aValue
			);
	}

EXPORT_C void TConfigSpace::Write16(TUint32 aOffset, TUint16 aValue)
	{
	CheckAccess(E2Byte, aOffset);

	iBridge.WriteConfig16(
			iFunction.Bus(),
			iFunction.Device(),
			iFunction.Function(),
			aOffset,
			aValue
			);
	}

EXPORT_C void TConfigSpace::Write32(TUint32 aOffset, TUint32 aValue)
	{
	CheckAccess(E4Byte, aOffset);

	iBridge.WriteConfig32(
			iFunction.Bus(),
			iFunction.Device(),
			iFunction.Function(),
			aOffset,
			aValue
			);
	}

EXPORT_C void TConfigSpace::Modify8(TUint32 aOffset, TUint8 aClearMask, TUint8 aSetMask)
	{
	CheckAccess(E1Byte, aOffset);

	iBridge.ModifyConfig8(
			iFunction.Bus(),
			iFunction.Device(),
			iFunction.Function(),
			aOffset,
			aClearMask,
			aSetMask
			);
	}

EXPORT_C void TConfigSpace::Modify16(TUint32 aOffset, TUint16 aClearMask, TUint16 aSetMask)
	{
	CheckAccess(E2Byte, aOffset);

	iBridge.ModifyConfig16(
			iFunction.Bus(),
			iFunction.Device(),
			iFunction.Function(),
			aOffset,
			aClearMask,
			aSetMask
			);
	}

EXPORT_C void TConfigSpace::Modify32(TUint32 aOffset, TUint32 aClearMask, TUint32 aSetMask)
	{
	CheckAccess(E4Byte, aOffset);

	iBridge.ModifyConfig32(
			iFunction.Bus(),
			iFunction.Device(),
			iFunction.Function(),
			aOffset,
			aClearMask,
			aSetMask
			);
	}

DNaviEnginePciBridge::DNaviEnginePciBridge(TUint aBaseAddress, TUint32 aVirtualWindow)
	:DPciBridge(), iBaseAddr(aBaseAddress),
	iVirtualWindow(aVirtualWindow),
	iAllocator(KPciAddressSpaceSize),
	iMapMan(KNeBridgeNumberOfBars, iAllocator, iBaseAddr),
	iChunkMan(iMapMan),
	iVid(AsspRegister::Read16(iBaseAddr+KHoPciVid)),
	iDid(AsspRegister::Read16(iBaseAddr+KHoPciDid))
	{
	}

TInt DNaviEnginePciBridge::Initialise()
	{
	__KTRACE_OPT(KPCI, Kern::Printf("Initialising Naviengine bridge: Base address: %x", iBaseAddr)) ;
	InitialiseRegisters();
	return SetupInterrupts();
	}

DNaviEnginePciBridge::~DNaviEnginePciBridge()
	{
	}

/**
Create a DPlatChunkHw which will be accessble to PCI devices under this bridge.

@param aSize Amount of memory visible from PCI in bytes.
This will be rounded up to the next power of 2 in kilobytes
.eg 1K, 2K, 4K etc. @note The actual amount of memory allocated
will always be a multiple of the page size (4K), but any excess
will not be visible to the PCI bus.
@param aPciAddress On success, will be a PCI address by which the chunk may be accessed
@return
	- KErrNone - On Success
	- KErrInUse - aChunk was not NULL
	- KErrNotFound - All of Bridge's BARS have been used up. Free one with RemoveChunk
	- KErrArgument - aSize was less than 1.
*/
TInt DNaviEnginePciBridge::CreateChunk(DPlatChunkHw*& aChunk, TInt aSize, TUint aAttributes, TUint32& aPciAddress)
	{
	if(aChunk!=NULL)
		return KErrInUse;
	if(aSize<1)
		return KErrArgument;

	return iChunkMan.AddChunk(aChunk, aSize, aAttributes, aPciAddress);
	}

/**
Create a DChunk which will be accessble to PCI devices under this bridge.

@param aSize Amount of memory to be allocated to chunk.
This will be rounded up to at least the next power of 2 in kilobytes
.eg 1K, 2K, 4K, 8k etc.
@param aPciAddress On success, will be a PCI address by which the chunk may be accessed
@return
	- KErrNone - On Success
	- KErrInUse - aChunk was not NULL
	- KErrNotFound - All of Bridge's BARS have been used up. Free one with RemoveChunk
	- KErrArgument - aSize was less than 1.
*/
TInt DNaviEnginePciBridge::CreateChunk(DChunk*& aChunk, TChunkCreateInfo &aAttributes, TUint aOffset, TUint aSize, TUint32& aPciAddress)
	{
	if(aChunk!=NULL)
		return KErrInUse;
	if(aSize<1)
		return KErrArgument;

	return iChunkMan.AddChunk(aChunk, aAttributes, aOffset, aSize, aPciAddress);
	}


TInt DNaviEnginePciBridge::RemoveChunk(DPlatChunkHw* aChunk)
	{
	return iChunkMan.RemoveChunk(aChunk);
	}

TInt DNaviEnginePciBridge::CreateMapping(TUint32 aPhysicalAddress, TInt aSize, TUint32& aPciAddress)
	{
	if(aSize<1)
		return KErrArgument;

	return iMapMan.CreateMapping(aPhysicalAddress, aSize, aPciAddress);
	}

TInt DNaviEnginePciBridge::RemoveMapping(TUint32 aPhysicalAddress)
	{
	return iMapMan.RemoveMapping(aPhysicalAddress);
	}

TInt DNaviEnginePciBridge::GetPciAddress(TUint32 aPhysicalAddress, TUint32& aPciAddress)
	{
	return iMapMan.GetPciAddress(aPhysicalAddress, aPciAddress);
	}

TInt DNaviEnginePciBridge::GetPhysicalAddress(TUint32 aPciAddress, TUint32& aPhysicalAddress)
	{
	return iMapMan.GetPhysicalAddress(aPciAddress, aPhysicalAddress);
	}

/**
Attempt to access function at this location. If it exists then scan its
bars to create required PCI memory space.
Create and return a complete TPciFunction
*/
TPciFunction* DNaviEnginePciBridge::Function(TInt aBus, TInt aDevice, TInt aFunction)
	{
	const TUint32 val = ReadConfig32(aBus, aDevice, aFunction, 0x0);

	//the function does not exist.
	if(val == 0xFFFFFFFF)
		return NULL;

	const TInt16 vid=val&0xFFFF;
	TInt16 did=(val>>16)&0xFFFF;

	//don't give access to the bridge its self
	if(vid==iVid && did==iDid)
		return NULL;

	TPciFunction* func= new TPciFunction(aBus, aDevice, aFunction, vid, did, *this);
	if(NULL==func)
		return func;

	TAddrSpace& configSpace=*func->GetConfigSpace();

	TInt r=KErrNone;
	//scan each of the bars
	for(TInt i=0; i<KPciNumberOfBars; ++i)
		{
		const TInt barOffset=KPciBar0+(4*i);
		const TUint32 size= ProbeBar(configSpace, barOffset);

		if(NULL==size)
			continue;

		//allocate pci address range
		TUint32 pciAddress=0;
		r=iAllocator.Allocate(pciAddress, size);
		if(r!=KErrNone)
			break;

		//tell the function what its address is.
		r = func->AddMemorySpace(size, pciAddress+iVirtualWindow, i);
		if(r!=KErrNone)
			break;

		//modify bar.
		configSpace.Write32(barOffset, pciAddress);
		}

	if(r==KErrNone)
		{
		configSpace.Modify16(KHoPciCmd, NULL, KHtPcicmd_Memen);
		return func;
		}
	else
		{
		delete func;
		return NULL;
		}

	}

void DNaviEnginePciBridge::ConfigurationComplete()
	{
	//state that config is complete and allow subsequent master aborts
	//to trigger the error interrupt
	AsspRegister::Modify32(iBaseAddr+KHoPciCtrlH, NULL, KHtPciCtrlH_CnfigDone| KHtPciCtrlH_Mase);
	Interrupt::Enable(EIntPciInt);
	}


void DNaviEnginePciBridge::ErrorPrint()
	{
	Kern::Printf("Pci Errors:");

	Kern::Printf(" Status Register");
	volatile TUint16 status=AsspRegister::Read16(iBaseAddr+KHoPciStatus);
	if(status & KHtPciStatus_ParityError)
		Kern::Printf("  Parity Error");
	if(status & KHtPciStatus_SystemError)
		Kern::Printf("  System Error");
	if(status & KHtPciStatus_MasterAbrtRcvd)
		Kern::Printf("  MasterAbrtRcvd");
	if(status & KHtPciStatus_TargetAbrtRcvd)
		Kern::Printf("  TargetAbrtRcvd");
	if(status & KHtPciStatus_DPErrorAsserted)
		Kern::Printf("  PERR# asserted");

	Kern::Printf(" Err1 Register:");
	volatile TUint32 error=AsspRegister::Read32(iBaseAddr+KHoError1);
	if(error & KHtError1_SystemError)
		Kern::Printf("  System Error");
	if(error & KHtError1_AMEr)
		Kern::Printf("  AHB master error");

	Kern::Printf(" PciCtrlHi Register");

	volatile TUint32 pciCtrlH=AsspRegister::Read32(iBaseAddr+KHoPciCtrlH);
	if(pciCtrlH & KHtPciCtrlH_Aper)
		Kern::Printf("  Address Parity error");
	if(pciCtrlH & KHtPciCtrlH_Dtep)
		Kern::Printf("  Discard time out");
	if(pciCtrlH & KHtPciCtrlH_Dper)
		Kern::Printf("  Data parity error");
	if(pciCtrlH & KHtPciCtrlH_Rlex)
		Kern::Printf("  Retry limit exceeded");
	if(pciCtrlH & KHtPciCtrlH_Mabo)
		Kern::Printf("  Master abort");
	if(pciCtrlH & KHtPciCtrlH_Tabo)
		Kern::Printf("  Target abort");

	}
/**
@param aOffset A DWord index (32 bit)
@return A value sutiable for writing to the bridge's CNFIG_ADDR register
*/
TCnfgAddr DNaviEnginePciBridge::MakeConfigAddress(TInt aBus, TInt aDevice, TInt aFunction, TUint aDwordOffset)
	{
	using namespace ConfigAddress;
	const TUint32 bus=(aBus<<KHsBus);
	const TUint32 device=(aDevice<<KHsDevice);
	const TUint32 function=(aFunction<<KHsFunction);
	const TUint32 offset=(aDwordOffset<<KHsOffset);

	__NK_ASSERT_DEBUG( (bus & (~KHmBus)) == NULL);
	__NK_ASSERT_DEBUG( (device & (~KHmDevice)) == NULL);
	__NK_ASSERT_DEBUG( (function & (~KHmFunction)) == NULL);
	__NK_ASSERT_DEBUG( (offset & (~KHmOffset)) == NULL);

	return (KHtCnfigEnable|
			(bus & KHmBus)|
			(device & KHmDevice)|
			(function & KHmFunction)|
			(offset & KHmOffset)
		   );
	}

/**
Probes the specified bar to see whether it is implemented, if so
then return the amount of memory space required.

@note Driver does not currently support devices which request IO-space
or 64-bit bars.
*/
TUint DNaviEnginePciBridge::ProbeBar(TAddrSpace& aCs, TUint32 aBarOffset)
	{
	__KTRACE_OPT(KPCI, Kern::Printf("DNaviEnginePciBridge::ProbeBar Probing BAR at Offset %d", aBarOffset));

	//ignore any writable bits in positions [0:2]
	//they shouldn't be writable but are for the NaviEngine host
	//bridge
	const TUint32 KHmIgnore= Bar::KHtMemSpaceType|Bar::KHmType|Bar::KHtPreFetchable;

	aCs.Write32(aBarOffset, 0x00000000);
	TUint32 initial= aCs.Read32(aBarOffset);
	aCs.Write32(aBarOffset, KMaxTUint32);
	TUint32 bar = aCs.Read32(aBarOffset);

	//reset after probing
	aCs.Write32(aBarOffset, 0x00000000);

	if( (bar&(~KHmIgnore)) == (initial&(~KHmIgnore)) )
		{
		__KTRACE_OPT(KPCI, Kern::Printf(" Function doesn't implement BAR"));
		return NULL;
		}
	if(bar & Bar::KHtMemSpaceType)
		{
		__KTRACE_OPT(KPCI, Kern::Printf(" IOSpace is not supported") );
		return NULL;
		}

	if(bar & Bar::KHmType )
		{
		__KTRACE_OPT(KPCI, Kern::Printf(" Only support 32 bit address space") );
		return NULL;
		}

	TUint size= (bar<<1)^(bar);
	__KTRACE_OPT(KPCI, Kern::Printf(" Address space: %08x bytes", size));
	return size;
	}



TUint8 DNaviEnginePciBridge::ReadConfig8(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset) const
	{
	const TUint dwordOffset = aOffset>>2; //divide by 4
	const TUint byteOffset = aOffset%4;

	const TCnfgAddr location = MakeConfigAddress(aBus, aDevice, aFunction, dwordOffset);

	Wait();
	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, location);
	volatile TUint8 value = AsspRegister::Read8(iBaseAddr+KHoCnfig_data+byteOffset);
	Signal();
	return value;
	}

TUint16 DNaviEnginePciBridge::ReadConfig16(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset) const
	{
	const TUint dwordOffset = aOffset>>2; //divide by 4
	const TUint byteOffset = aOffset%4;

	const TCnfgAddr location = MakeConfigAddress(aBus, aDevice, aFunction, dwordOffset);

	Wait();
	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, location);
	volatile TUint16 value = AsspRegister::Read16(iBaseAddr+KHoCnfig_data+byteOffset);
	Signal();
	return value;
	}

/**
@param aOffset A byte index
*/
TUint32 DNaviEnginePciBridge::ReadConfig32(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset) const
	{
	TUint dwordOffset = aOffset>>2; //divide by 4

	const TCnfgAddr location = MakeConfigAddress(aBus, aDevice, aFunction, dwordOffset);

	Wait();
	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, location);
	volatile TInt32 value = AsspRegister::Read32(KHoCnfig_data+iBaseAddr);
	Signal();
	return value;
	}

void DNaviEnginePciBridge::WriteConfig8(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint8 aValue)
	{
	TUint dwordOffset = aOffset>>2; //divide by 4
	const TUint byteOffset = aOffset%4;

	const TCnfgAddr location = MakeConfigAddress(aBus, aDevice, aFunction, dwordOffset);

	Wait();
	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, location);
	AsspRegister::Write8(KHoCnfig_data+iBaseAddr+byteOffset, aValue);
	Signal();
	}

void DNaviEnginePciBridge::WriteConfig16(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint16 aValue)
	{
	TUint dwordOffset = aOffset>>2; //divide by 4
	const TUint byteOffset = aOffset%4;

	const TCnfgAddr location = MakeConfigAddress(aBus, aDevice, aFunction, dwordOffset);

	Wait();
	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, location);
	AsspRegister::Write16(KHoCnfig_data+iBaseAddr+byteOffset, aValue);
	Signal();
	}

/**
@param aOffset A byte index
*/
void DNaviEnginePciBridge::WriteConfig32(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint32 aValue)
	{
	//__KTRACE_OPT(KPCI, Kern::Printf("PCI: WriteConfig32: %x:%x:%x os=%x, val=%x ", aBus,aDevice, aFunction, aOffset, aValue));
	TUint dwordOffset = aOffset>>2; //divide by 4
	const TCnfgAddr location = MakeConfigAddress(aBus, aDevice, aFunction, dwordOffset);

	Wait();
	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, location);
	AsspRegister::Write32(KHoCnfig_data+iBaseAddr, aValue);
	Signal();
	}

void DNaviEnginePciBridge::ModifyConfig8(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint8 aClearMask, TUint8 aSetMask)
	{
	//__KTRACE_OPT(KPCI, Kern::Printf("PCI: ModifyConfig32: %x:%x:%x os=%x, clear=%x, set=%x ",aBus,aDevice, aFunction, aOffset, aClearMask, aSetMask));
	TUint dwordOffset = aOffset>>2; //divide by 4 (and trunctate)
	const TUint byteOffset = aOffset%4;

	const TCnfgAddr location = MakeConfigAddress(aBus, aDevice, aFunction, dwordOffset);

	Wait();
	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, location);
	AsspRegister::Modify8(KHoCnfig_data+iBaseAddr+byteOffset, aClearMask, aSetMask);
	Signal();
	}

void DNaviEnginePciBridge::ModifyConfig16(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint16 aClearMask, TUint16 aSetMask)
	{
	//__KTRACE_OPT(KPCI, Kern::Printf("PCI: ModifyConfig32: %x:%x:%x os=%x, clear=%x, set=%x ",aBus,aDevice, aFunction, aOffset, aClearMask, aSetMask));
	TUint dwordOffset = aOffset>>2; //divide by 4 (and trunctate)
	const TUint byteOffset = aOffset%4;

	const TCnfgAddr location = MakeConfigAddress(aBus, aDevice, aFunction, dwordOffset);

	Wait();
	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, location);
	AsspRegister::Modify16(KHoCnfig_data+iBaseAddr+byteOffset, aClearMask, aSetMask);
	Signal();
	}

void DNaviEnginePciBridge::ModifyConfig32(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint32 aClearMask, TUint32 aSetMask)
	{
	//__KTRACE_OPT(KPCI, Kern::Printf("PCI: ModifyConfig32: %x:%x:%x os=%x, clear=%x, set=%x ",aBus,aDevice, aFunction, aOffset, aClearMask, aSetMask));
	TUint dwordOffset = aOffset>>2; //divide by 4 (and trunctate)

	const TCnfgAddr location = MakeConfigAddress(aBus, aDevice, aFunction, dwordOffset);

	Wait();
	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, location);
	AsspRegister::Modify32(KHoCnfig_data+iBaseAddr, aClearMask, aSetMask);
	Signal();
	}

void DNaviEnginePciBridge::InitialiseRegisters()
	{
	ClearRegisters();

	//make the bridge a PCI master and respond as a target
	AsspRegister::Modify16(iBaseAddr+KHoPciCmd, 0x0, KHtPcicmd_Bmasen|KHtPcicmd_Memen|KHtPcicmd_Peren|KHtPcicmd_Seren);

	//set up inbound access to PCI through window 1.
	const TUint32 window1Register =
		0| //window points to address 0 of PCI
		(0x13<<Initiator::KHsA2PCAMask)| //modify incomming AHB address above 8k
		(0x3<<Initiator::KHsType)| //incomming accesses will perform memory read/memory write
		0x1; //enable incomming address conversion
	AsspRegister::Write32(iBaseAddr+Initiator::KHoReg1, window1Register);

	AsspRegister::Write32(iBaseAddr+KHoBarEnable, 0); //disable all BARs, don't yet want access from PCI to AHB

	AsspRegister::Modify32(iBaseAddr+KHoPciCtrlH ,NULL,
			KHtPciCtrlH_Aerse|
			KHtPciCtrlH_Dtimse|
			KHtPciCtrlH_Perse|
			KHtPciCtrlH_Rtyse|
			KHtPciCtrlH_Tase
			);
	//report errors from PCI bus to AHB64PCI_ERR pin
	//report errror on AHB to AHB64PCI_ERR pin
	AsspRegister::Modify32(iBaseAddr+KHoError1, NULL,
			KHtError1_PEEn|
			KHtError1_AMEn
			);

	}

void DNaviEnginePciBridge::ClearRegisters()
	{
	ClearErrors();

	AsspRegister::Modify16(iBaseAddr+KHoPciCmd,
			KHtPcicmd_Bmasen|
			KHtPcicmd_Memen|
			KHtPcicmd_Peren|
			KHtPcicmd_Seren,
			NULL);

	AsspRegister::Modify32(iBaseAddr+KHoPciCtrlH ,
			KHtPciCtrlH_CnfigDone|
			KHtPciCtrlH_Aerse|
			KHtPciCtrlH_Dtimse|
			KHtPciCtrlH_Perse|
			KHtPciCtrlH_Rtyse|
			KHtPciCtrlH_Mase|
			KHtPciCtrlH_Tase,
			NULL
			);

	AsspRegister::Modify32(iBaseAddr+KHoError1,
			KHtError1_PEEn|
			KHtError1_AMEn,
			NULL
			);

	AsspRegister::Write32(KHoCnfig_addr+iBaseAddr, 0x0);

	AsspRegister::Write32(iBaseAddr+Initiator::KHoReg1, 0x0);
	AsspRegister::Write32(iBaseAddr+Initiator::KHoReg2, 0x0);
	}

//Clear latched bits by wriring 1 to them
void DNaviEnginePciBridge::ClearErrors()
	{
	AsspRegister::Modify16(iBaseAddr+KHoPciStatus, NULL,
		KHtPciStatus_ParityError|
		KHtPciStatus_SystemError|
		KHtPciStatus_MasterAbrtRcvd|
		KHtPciStatus_TargetAbrtRcvd|
		KHtPciStatus_DPErrorAsserted
		);

	AsspRegister::Modify32(iBaseAddr+KHoError1, NULL,
			KHtError1_SystemError|
			KHtError1_AMEr
			);

	AsspRegister::Modify32(iBaseAddr+KHoPciCtrlH, NULL,
			KHtPciCtrlH_Aper|
			KHtPciCtrlH_Dtep|
			KHtPciCtrlH_Dper|
			KHtPciCtrlH_Rlex|
			KHtPciCtrlH_Mabo|
			KHtPciCtrlH_Tabo
			);

	}


TInt DNaviEnginePciBridge::SetupInterrupts()
	{
	__KTRACE_OPT(KPCI, Kern::Printf("DNaviEnginePciBridge: SetupInterrupts()"));
	TInt r = KErrNone;

	__KTRACE_OPT(KPCI, Kern::Printf("  Binding EIntPciPErrB"));
	r = Interrupt::Bind(EIntPciPErrB, ParityErrorISR, this);
	__KTRACE_OPT(KPCI, Kern::Printf("  r=%d", r));

	__KTRACE_OPT(KPCI, Kern::Printf("  Binding EIntPciSErrB"));
	r = Interrupt::Bind(EIntPciSErrB, SystemErrorISR, this);
	__KTRACE_OPT(KPCI, Kern::Printf("  r=%d", r));

	__KTRACE_OPT(KPCI, Kern::Printf("  Binding EIntPciInt"));
	r = Interrupt::Bind(EIntPciInt, PciISR, this);
	__KTRACE_OPT(KPCI, Kern::Printf("  r=%d", r));

	__KTRACE_OPT(KPCI, Kern::Printf("  Enabling EIntPciPErrB"));
	r = Interrupt::Enable(EIntPciPErrB);
	__KTRACE_OPT(KPCI, Kern::Printf("  r=%d", r));

	__KTRACE_OPT(KPCI, Kern::Printf("  Enabling EIntPciSErrB"));
	r = Interrupt::Enable(EIntPciSErrB);
	__KTRACE_OPT(KPCI, Kern::Printf("  r=%d", r));

	__KTRACE_OPT(KPCI, Kern::Printf("  Enabling EIntPciInt"));
	Interrupt::Clear(EIntPciInt);
	//r = Interrupt::Enable(EIntPciInt);
	__KTRACE_OPT(KPCI, Kern::Printf("  r=%d", r));
	return r;
	}

void DNaviEnginePciBridge::ParityErrorISR(void* aP)
	{
	Interrupt::Clear(EIntPciPErrB);
	Kern::Fault("PCI Parity error",0);
	}

void DNaviEnginePciBridge::SystemErrorISR(void* aP)
	{
	Interrupt::Clear(EIntPciSErrB);
	Kern::Fault("PCI System error",0);
	}

/**
This interrupt is raised when a bus error has occured, if the
appropriate bit has been set (KHtPciCtrlH_Mase).
It will also be raised if a peripheral uses one of the
native PCI interrupt lines - but this not yet supported.
*/
void DNaviEnginePciBridge::PciISR(void* aP)
	{
	Interrupt::Clear(EIntPciInt);
	DNaviEnginePciBridge* bridge = static_cast<DNaviEnginePciBridge*>(aP);
	__KTRACE_OPT(KPCI,
			Kern::Printf("Pci interrupt line: Bridge Base address 0x%08x",bridge->iBaseAddr);
			bridge->ErrorPrint();
			);

	volatile TUint16 status=AsspRegister::Read16((bridge->iBaseAddr)+KHoPciStatus);
	if(status & KHtPciStatus_SystemError)
		{
		Kern::Fault("PCI System Error: Fatal",0);
		}
	bridge->ClearErrors();
	}

//
// TNaviEngineChunkCleanup //
//

TNaviEngineChunkCleanup::TNaviEngineChunkCleanup(TChunkManager& aChunkMan, TUint32 aPhysicalAddress)
	:TChunkCleanup(), iChunkMan(aChunkMan), iPhysicalAddress(aPhysicalAddress)
	{
	}

TNaviEngineChunkCleanup::~TNaviEngineChunkCleanup()
	{
	}

void TNaviEngineChunkCleanup::Destroy()
	{
	iChunkMan.RemoveChunk(iPhysicalAddress);
	}


DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("Pci Extension starting..."));

	DPciBridge* internBridge = new DNaviEnginePciBridge(KHwPciBridgeUsb, KHwUsbHWindow);

	if(internBridge == NULL)
		return KErrNoMemory;

	TInt r = internBridge->Initialise();
	if(r !=KErrNone)
		return r;

	r = internBridge->Register();
	if(r !=KErrNone)
		{
		delete internBridge;
		return r;
		}

	r = Pci::Enumerate();
	return r;
	}





