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



#include <pci.h>
#include <naviengine.h>
#include <naviengine_priv.h>

#include "pci_priv.h"

//
// TAddrSpace class
//

TAddrSpace::TAddrSpace(TUint aSize, const TDesC& aName)
	:iSize(aSize), iName(aName)
	{
	}

EXPORT_C TUint TAddrSpace::Size()
	{
	return iSize;
	}

/**
Check access is aligned and in range
*/
void TAddrSpace::CheckAccess(TNumberOfBytes aNumberOfBytes, TUint aByteOffset)
	{
	if(aByteOffset > iSize-aNumberOfBytes)
		{
		Kern::Printf("PCI %S: Access out of range", &iName);
		FAULT();
		}
	if(aByteOffset%aNumberOfBytes)
		{
		Kern::Printf("PCI %S: Access missaligned", &iName);
		FAULT();
		}
	}

//
// DPciBridge class
//
DPciBridge::DPciBridge()
	{
	}

DPciBridge::~DPciBridge()
	{
	}


/**
Register this bridge with the system wide Pci manager.
*/
TInt DPciBridge::Register()
	{
	return Pci::AddBridge(this);
	}


//
// Pci class
//

TBool Pci::iEnumerated = EFalse;

RPointerArray<DPciBridge> Pci::iPciBridges(2);

/**
Search all PCI busses for PCI functions matching the specified vendor and
device ids.

@param aVid Vendor Id
@param aDid Device Id
@param aListOfFunctions On KErrNone will be filled with indices of matching PCI functions
@return
	- KErrNone - Matching functions were found
	- KErrNotFound - No matches were found.
	- KErrArgument - aListOfFunctions was not empty at start.
*/
EXPORT_C TInt Pci::Probe(RArray<TInt>& aListOfFunctions, TPciVendorId aVid, TDeviceId aDid)
	{
	if(aListOfFunctions.Count()>0)
		return KErrArgument;

	const TInt functionCount=iFunctions.Count();
	TInt r=KErrNotFound;
	for(TInt i=0; i<functionCount;++i)
		{
		TPciFunction* func=iFunctions[i];
		if(func->VendorId()==aVid && func->DeviceId()==aDid)
			{
			r = aListOfFunctions.Append(i);
			if(r!=KErrNone)
				return r; //some error when appending
			}
		}

	if(aListOfFunctions.Count()>0)
		r=KErrNone;

	return r;
	}

/**
Gets a pointer to one of the specified function's memory
spaces. A function may have a memory space for each of its
6 BARS. If either of the supplied indices are invalid NULL
will be returned.

@return NULL if error, otherwise a pointer the requested memory space.
@param aFunction Index of the PCI function.
@param aBarIndex Index from 0 to 5
*/
EXPORT_C TAddrSpace* Pci::GetMemorySpace(TInt aFunction, TInt aBarIndex)
	{
	if(!Rng(0, aFunction, iFunctions.Count()-1 ))
		return NULL;
	return iFunctions[aFunction]->GetMemorySpace(aBarIndex);
	}

/**
Get a pointer to the specified function's configuration space.

@return NULL if aFunction is invalid or some other error
@param aFunction Index of the PCI function.
*/
EXPORT_C TAddrSpace* Pci::GetConfigSpace(TInt aFunction)
	{
	if(!Rng(0, aFunction, iFunctions.Count()-1 ))
		return NULL;

	return iFunctions[aFunction]->GetConfigSpace();
	}

/**
Create a DMA'able DPlatChunkHw chunk with memory allocated which will be accessable by the PCI function aFunction (and other PCI devices
on the same bridge). The allocated chunk must only be destroyed with Pci::RemoveChunk. For simplicity
it is recommended to use the alternate CreateChunk to create a DChunk, since this will be a able to clean itself up automatically on
closure.
@return
	- KErrNone on success
	- KErrInUse - aChunk was not NULL
	- KErrNotFound - All of Bridge's BARS have been used up. Free one with RemoveChunk
	- KErrArgument - aSize was less than 1.
@param aFunction Index of the PCI function.
@param aChunk Must be NULL, on success will point to the created chunk.
@param aSize of chunk required.
This will be rounded up to the next power of 2 in kilobytes
.eg 1K, 2K, 4K etc. So requesting a 600Kb buffer will result in 1Mb being allocated.
@param aAttributes Attribute mask made up of OR'd values in TMappingAttributes
@param aPciAddress On success will be set to the PCI address with which devices may access the allocated memory.
*/
EXPORT_C TInt Pci::CreateChunk(TInt aFunction, DPlatChunkHw*& aChunk, TInt aSize, TUint aAttributes, TUint32& aPciAddress)
	{
	if(!Rng(0, aFunction, iFunctions.Count()-1 ))
		return KErrArgument;

	return iFunctions[aFunction]->GetBridge().CreateChunk(aChunk, aSize, aAttributes, aPciAddress);
	}

/**
Create a DMA'able chunk with memory allocated which will be accessable by the PCI function aFunction (and other PCI devices
on the same bridge). When the final reference to the chunk is closed it will be unmapped from PCI memory and free its physical ram.
@return
	- KErrNone on success
	- KErrInUse - aChunk was not NULL
	- KErrNotFound - All of Bridge's BARS have been used up. Free one by calling RemoveMapping, RemoveChunk or closing all
	  references to a DChunk previously allocated with this function.
	- KErrArgument - aSize was less than 1.
@param aFunction Index of the PCI function.
@param aChunk Must be NULL, on success will point to the created chunk.
@param aAttributes Contains creation parameters for DChunk. These may be adjusted by the driver and can be viewed on return
@param aOffset The number of bytes into the chunk's virtual address range at which memory should be commited.
@param aSize of chunk required.
This will be rounded up to the next power of 2 in kilobytes
.eg 1K, 2K, 4K etc. So requesting a 600Kb buffer will result in 1Mb being allocated. 
@param aPciAddress On success will be set to the PCI address with which devices may access the allocated memory.
*/
EXPORT_C TInt Pci::CreateChunk(TInt aFunction, DChunk*& aChunk, TChunkCreateInfo &aAttributes, TUint aOffset, TUint aSize, TUint32& aPciAddress)
	{
	if(!Rng(0, aFunction, iFunctions.Count()-1 ))
		return KErrArgument;

	return iFunctions[aFunction]->GetBridge().CreateChunk(aChunk, aAttributes, aOffset, aSize, aPciAddress);
	}

RPointerArray<TPciFunction> Pci::iFunctions;


/**
Remove mapping from PCI memory space to chunk.
Free the memory chunk's memory and chunk object itself.

@param aFunction The PCI function wth which the chunk was associated.
@param aChunk Pointer to the chunk to be deleted
@return
	- KErrNone on success
	- KErrArgument aFunction was invalid
	- KErrNotFound aChunk was not found
*/
EXPORT_C TInt Pci::RemoveChunk(TInt aFunction, DPlatChunkHw* aChunk)
	{
	if(!Rng(0, aFunction, iFunctions.Count()-1 ))
		return KErrArgument;

	return iFunctions[aFunction]->GetBridge().RemoveChunk(aChunk);
	}

EXPORT_C void Pci::ChunkCleanupCallback(TChunkCleanup* aCleanup)
	{
	__KTRACE_OPT(KPCI, Kern::Printf("Pci::ChunkCleanupCallback aCleanup = 0x%08x", aCleanup));
	aCleanup->Destroy();
	delete aCleanup;	
	}

EXPORT_C TInt Pci::CreateMapping(TInt aFunction, TUint32 aPhysicalAddress, TInt aSize, TUint32& aPciAddress)
	{
	if(!Rng(0, aFunction, iFunctions.Count()-1 ))
		return KErrArgument;
	__KTRACE_OPT(KPCI, Kern::Printf("Pci::CreateMapping() requested 0x%X bytes", aSize));
	return iFunctions[aFunction]->GetBridge().CreateMapping(aPhysicalAddress, aSize, aPciAddress);
	}

EXPORT_C TInt Pci::RemoveMapping(TInt aFunction, TUint32 aPhysicalAddress)
	{
	if(!Rng(0, aFunction, iFunctions.Count()-1 ))
		return KErrArgument;

	return iFunctions[aFunction]->GetBridge().RemoveMapping(aPhysicalAddress);
	}

/**
Returns the PCI address corresponding to a given physical address.
@param aFunction
@param aAddress The physical address, on success, will have been converted to the
equivilant PCI address.
@return
	- KErrNone
	- KErrNotFound The physical address has no corresponding PCI address
*/
EXPORT_C TInt Pci::GetPciAddress(TInt aFunction, TUint32& aAddress)
	{
	if(!Rng(0, aFunction, iFunctions.Count()-1 ))
		return KErrArgument;

	return iFunctions[aFunction]->GetBridge().GetPciAddress(aAddress, aAddress);
	}

/**
Returns the physical address corresponding to a given PCI address.
@param aFunction
@param aAddress The PCI address, on success, will have been converted to the
equivilant physical address.
@return
	- KErrNone
	- KErrNotFound The PCI address has no corresponding physical address
*/
EXPORT_C TInt Pci::GetPhysicalAddress(TInt aFunction, TUint32& aAddress)
	{
	if(!Rng(0, aFunction, iFunctions.Count()-1 ))
		return KErrArgument;

	return iFunctions[aFunction]->GetBridge().GetPhysicalAddress(aAddress,aAddress);
	}


/**
Add bridges before calling Enumerate.

@param aBridge A constructed bridge object.
@return An error value
*/
TInt Pci::AddBridge(DPciBridge* aBridge)
	{
	return iPciBridges.Append(aBridge);
	}

/**
Query all host bridges on the system for their PCI functions and
populate global function list.

@return
	- KErrNone - Success
	- KErrNotSupported - No bridges have been registered
	- KErrAccessDenied - Enumerate has already been called
*/
TInt Pci::Enumerate()
	{
	__KTRACE_OPT(KPCI, Kern::Printf("Pci::Enumerate"));

	//only call once, from the extension
	if(iEnumerated)
		return KErrAccessDenied;
	iEnumerated = ETrue;

	const TInt numberOfPciBridges= iPciBridges.Count();
	if(0 == numberOfPciBridges)
		return KErrNotSupported;

	TInt r=KErrNone;
	for(TInt bridge=0; bridge<numberOfPciBridges; ++bridge)
		{
		for(TInt bus=0; bus<KPciMaxBusses; ++bus)
			{
			for(TInt device=0; device<KPciMaxDevices; ++device)
				{
				for(TInt function=0; function<KPciMaxFunctions; ++function)
					{
					TPciFunction* func = iPciBridges[bridge]->Function(bus, device, function);
					if(func==NULL)
						continue;
					if(func->IsBridge())
						{
						//will call bridge fix up code here when supported.
						//don't add a bridge into global list as we do not want to
						//grant access to it to the client programmer.
						__KTRACE_OPT(KPCI, Kern::Printf(  "PCI-PCI bridge unsupported"));
						delete func;
						func = NULL;
						continue;
						}
					else
						{
						r=iFunctions.Append(func);
						__KTRACE_OPT(KPCI, Kern::Printf("  Adding function %d:%d:%d:%d, vid=0x%04x, did=0x%04x"
									, bridge, bus, device, function, func->VendorId(), func->DeviceId()));
						}
					if(r!=KErrNone)
						return r;

					//a multifunction device can indicate the last function
					//implemented by setting its multi function bit to 0
					//so this is checked for all function numbers
					if(!func->IsMultiFunc())
						{
						break;
						}
					}
				}
			}
		iPciBridges[bridge]->ConfigurationComplete();
		}
	return r;
	}

TPciFunction::TPciFunction(TInt aBus, TInt aDevice, TInt aFunction,
		TPciVendorId aVid, TDeviceId aDid, DPciBridge& aBridge)
	:iBus(aBus), iDevice(aDevice), iFunction(aFunction), iVid(aVid), iDid(aDid),
		iBridge(aBridge), iConfigSpace(NULL), iMemorySpaces(KPciNumberOfBars),
		iIsBridge(EFalse), iIsMultiFunc(EFalse)
	{
	__ASSERT_DEBUG(iBus<256, Kern::Printf("TConfigSpace: Bus id too big"));
	__ASSERT_DEBUG(iDevice<32, Kern::Printf("TConfigSpace: Device id too big"));
	__ASSERT_DEBUG(iFunction<8, Kern::Printf("TConfigSpace: Function id too big"));

	//Array has an element for each of this PCI function's BARs
	//but an unused one will be NULL
	for(TInt i=0; i<KPciNumberOfBars;++i)
		{
		TInt r=iMemorySpaces.Append(NULL);
		__NK_ASSERT_ALWAYS(KErrNone==r);
		}

	iConfigSpace = new TConfigSpace(*this, iBridge);
	__NK_ASSERT_ALWAYS(iConfigSpace!=NULL);

	const TUint8 headerTypeByte = iConfigSpace->Read8(KPciHeaderType);
	iIsMultiFunc = (headerTypeByte & HeaderType::KHtMultiFunction);

	const TInt headerType = headerTypeByte & (~HeaderType::KHtMultiFunction);
	_LIT(KHeaderError, "Unknown PCI header type");
	switch(headerType)
		{
	case 0:
		iIsBridge=EFalse; break;
	case 1:
		iIsBridge=ETrue; break;
	default:
		Kern::PanicCurrentThread(KHeaderError, 0);
		}
	}

TPciFunction::~TPciFunction()
	{
	iMemorySpaces.ResetAndDestroy();

	delete iConfigSpace;
	}

/**
@param aBarIndex The Bar which this memory space is asssociated with
*/
TInt TPciFunction::AddMemorySpace(TUint32 aSize, TUint32 aAddress, TInt aBarIndex)
	{
	TMemorySpace* space=new TMemorySpace(aAddress, aSize);
	if(NULL==space)
		{
		return KErrNoMemory;
		}
	else
		{
		iMemorySpaces[aBarIndex]=space;
		return KErrNone;
		}
	}

EXPORT_C TAddrSpace* TPciFunction::GetConfigSpace()
	{
	return iConfigSpace;
	}

EXPORT_C TAddrSpace* TPciFunction::GetMemorySpace(TInt aBar)
	{
	if(!Rng(0,aBar, iMemorySpaces.Count()-1))
		return NULL;

	return iMemorySpaces[aBar];
	}
//
// TMemory Space
//

#define READ_TRACE(N) __KTRACE_OPT(KPCI, Kern::Printf( "TMemorySpace::READ" #N " Offset=0x%x, Absolute=0x%x (Base=0x%x), Value=0x%x",\
			aOffset, iBaseAddress+aOffset, iBaseAddress, value ))

#define WRITE_TRACE(N) __KTRACE_OPT(KPCI, Kern::Printf( "TMemorySpace::Write" #N " Offset=0x%x, Absolute=0x%x (Base=0x%x), Value=0x%x",\
			aOffset, iBaseAddress+aOffset, iBaseAddress, aValue ))

#define MOD_TRACE(N) __KTRACE_OPT(KPCI, Kern::Printf( "TMemorySpace::Modify" #N " Offset=0x%x, Absolute=0x%x (Base=0x%x), ClearMask=0x%x, SetMask=0x%x",\
			aOffset, iBaseAddress+aOffset, iBaseAddress, aClearMask, aSetMask ))

_LIT(KTMemorySpace, "TMemorySpace");
TMemorySpace::TMemorySpace(TUint32 aBaseAddress, TUint aSize)
	:TAddrSpace(aSize, KTMemorySpace), iBaseAddress(aBaseAddress)
	{
	}

TUint32 TMemorySpace::Read32(TUint32 aOffset)
	{
	CheckAccess(E4Byte, aOffset);
	return AsspRegister::Read32(iBaseAddress+aOffset);
	}

void TMemorySpace::Write32(TUint32 aOffset, TUint32 aValue)
	{
	WRITE_TRACE(32);
	CheckAccess(E4Byte, aOffset);
	AsspRegister::Write32(iBaseAddress+aOffset, aValue);
	}

void TMemorySpace::Modify32(TUint32 aOffset, TUint32 aClearMask, TUint32 aSetMask)
	{
	MOD_TRACE(32);
	CheckAccess(E4Byte, aOffset);
	AsspRegister::Modify32(iBaseAddress+aOffset, aClearMask, aSetMask);
	}

TUint16 TMemorySpace::Read16(TUint32 aOffset)
	{
	CheckAccess(E2Byte, aOffset);
	return AsspRegister::Read16(iBaseAddress+aOffset);
	}

void TMemorySpace::Write16(TUint32 aOffset, TUint16 aValue)
	{
	WRITE_TRACE(16);
	CheckAccess(E2Byte, aOffset);
	AsspRegister::Write16(iBaseAddress+aOffset, aValue);
	}

void TMemorySpace::Modify16(TUint32 aOffset, TUint16 aClearMask, TUint16 aSetMask)
	{
	MOD_TRACE(16);
	CheckAccess(E2Byte, aOffset);
	AsspRegister::Modify16(iBaseAddress+aOffset, aClearMask, aSetMask);
	}

TUint8 TMemorySpace::Read8(TUint32 aOffset)
	{
	CheckAccess(E1Byte, aOffset);
	return AsspRegister::Read8(iBaseAddress+aOffset);
	}

void TMemorySpace::Write8(TUint32 aOffset, TUint8 aValue)
	{
	WRITE_TRACE(8);
	CheckAccess(E1Byte, aOffset);
	AsspRegister::Write8(iBaseAddress+aOffset, aValue);
	}

void TMemorySpace::Modify8(TUint32 aOffset, TUint8 aClearMask, TUint8 aSetMask)
	{
	MOD_TRACE(8);
	CheckAccess(E1Byte, aOffset);
	AsspRegister::Modify8(iBaseAddress+aOffset, aClearMask, aSetMask);
	}

//
// TChunkCleanup
//

