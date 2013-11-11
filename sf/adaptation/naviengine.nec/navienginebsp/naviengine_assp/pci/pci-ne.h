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



#ifndef _PCI_NE_H
#define _PCI_NE_H

#include "allocator.h"
#include "mapman.h"
#include "chunkman.h"
#include <pci.h>
#include "pci_priv.h"

typedef TUint32 TCnfgAddr;

const TUint32 KPciAddressSpaceSize = 0x80000000; //2GB

const TInt32 KMinOutboundWindow = 0x400;

const TUint16 KNecVendorId=0x1033;
const TUint16 KInternalPciBridgeId=0x0175;
const TUint16 KExternalPciBridgeId=0x0174;

class TNaviEngineChunkCleanup : public TChunkCleanup
	{
public:
	TNaviEngineChunkCleanup(TChunkManager& aChunkMan, TUint32 aPhysicalAddress);
	~TNaviEngineChunkCleanup();
	void Destroy();
	
private:
	TChunkManager& iChunkMan; //< The chunk manager used by the NaviEngine host bridge
	TUint32 iPhysicalAddress; //< Required to free physical memory and unmap memory from Pci
	};



/**
This represents a PCI host bridge controller on the NaviEngine, there are
two identical ones, one for external peripherals and one dedicated
to the OHCI and EHCI usb host controllers.

The main job of the class is to manage the address mappings which control
access across the bridge. For access to a PCI address from the AHB bus, there
is a window region in AHB space for which accesses will be forwarded to the bridge.
The bridge can then convert that AHB address before accessing the PCI bus.

It is also possible for devices on the PCI side to access addresses on the AHB side (DMA).
This is done by configuring the BARs on the bridge device to respond to selected PCI
addresses and forward these accesses on to the AHB bus. This is functionallity
is accessed with the CreateChunk method.
*/
class DNaviEnginePciBridge : public DPciBridge
	{
public:
	DNaviEnginePciBridge(TUint aBaseAddress, TUint32 aVirtualWindow);
	TInt Initialise();
	~DNaviEnginePciBridge();

	TPciFunction* Function(TInt aBus, TInt aDevice, TInt aFunction);
	void ConfigurationComplete();
	TInt CreateChunk(DPlatChunkHw*& aChunk, TInt aSize, TUint aAttributes, TUint32& aPciAddress);
	TInt CreateChunk(DChunk*& aChunk, TChunkCreateInfo &aAttributes, TUint aOffset, TUint aSize, TUint32& aPciAddress);
	TInt RemoveChunk(DPlatChunkHw* aChunk);

	TInt CreateMapping(TUint32 aPhysicalAddress, TInt aSize, TUint32& aPciAddress);
	TInt RemoveMapping(TUint32 aPhysicalAddress);
	TInt GetPciAddress(TUint32 aPhysicalAddress, TUint32& aPciAddress);
	TInt GetPhysicalAddress(TUint32 aPciAddress, TUint32& aPhysicalAddress);

	void ErrorPrint();

	TUint8 ReadConfig8(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset) const;
	TUint16 ReadConfig16(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset) const;
	TUint32 ReadConfig32(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset) const;

	void WriteConfig8(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint8 aValue);
	void WriteConfig16(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint16 aValue);
	void WriteConfig32(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint32 aValue);

	void ModifyConfig8(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint8 aClearMask, TUint8 aSetMask);
	void ModifyConfig16(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint16 aClearMask, TUint16 aSetMask);
	void ModifyConfig32(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint32 aClearMask, TUint32 aSetMask);

private:
	void InitialiseRegisters();
	void ClearRegisters();
	void ClearErrors();

	TInt SetupInterrupts();
	static TCnfgAddr MakeConfigAddress(TInt aBus, TInt aDevice, TInt aFunction, TUint aDwordOffset);
	inline void Wait() const
		{NKern::FMWait(&iConfigLock);}
	inline void Signal() const
		{NKern::FMSignal(&iConfigLock);}

	TUint ProbeBar(TAddrSpace& aCs, TUint32 aBarOffset);


	// ISRs //
	static void PciISR(void* aP);
	static void ParityErrorISR(void* aP);
	static void SystemErrorISR(void* aP);

	const TUint32 iBaseAddr;
	const TUint32 iVirtualWindow; //The kernel-side virtual address which is used to access PCI bus

	mutable NFastMutex iConfigLock; //make access to config addrss and data ports atomic

	TAddressAllocator iAllocator;
	TMappingManager iMapMan;
	TChunkManager iChunkMan;
	const TUint16 iVid;
	const TUint16 iDid;
	};
#endif //_PCI_NE_H
