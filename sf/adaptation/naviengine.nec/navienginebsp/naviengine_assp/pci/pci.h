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



/**
 @file
 @publishedPartner
 @test
*/

#ifndef _PCI_H
#define _PCI_H

#include <e32cmn.h>

typedef TUint16 TPciVendorId;
typedef TUint16 TDeviceId;
typedef TUint16 TSubSysId;
typedef TUint16 TSubSysVendorId;

const TUint KCfgSpaceSize = 256; // bytes

const TInt KPciMaxBusses = 1; //This can be 256, but this driver doesn't need to support subordinate busses yet
const TInt KPciMaxDevices = 32;
const TInt KPciMaxFunctions = 8;

const TInt KPciNumberOfBars=6;
const TUint KPciBar0 = 0x10;
const TUint KPciHeaderType = 0xE;

/**
Bitmasks for standard PCI Base address register.
*/
namespace Bar
	{
	const TUint32 KHtMemSpaceType	= KBit0; // Mem or Io address space
	const TUint32 KHmType			= KBit1|KBit2; //32 bit or 64 bit bar
	const TUint32 KHtPreFetchable	= KBit3; // Set if memspace is prefetchable
	}

/**
Bitmask for standard PCI header register
*/
namespace HeaderType
	{
	const TUint32 KHtMultiFunction	= KBit7; //True if the device is multifunction
	}

class TAddrSpace;
class TPciFunction;
class TChunkCleanup;
class DPciBridge;
class DChunk;
class DPlatChunkHw;
struct TChunkCreateInfo;

/**
A static class initialised in the kernel extension
to enumerate the system's PCI buses and grant access to
PCI functions.
*/
class Pci
	{
public:
	IMPORT_C static TInt Probe(RArray<TInt>& aListOfFunctions, TPciVendorId aVid, TDeviceId aDid);
	IMPORT_C static TAddrSpace* GetMemorySpace(TInt aFunction, TInt aBarIndex=0);
	IMPORT_C static TAddrSpace* GetConfigSpace(TInt aFunction);
	IMPORT_C static TInt CreateChunk(TInt aFunction, DPlatChunkHw*& aChunk, TInt aSize, 
		TUint aAttributes, TUint32& aPciAddress);
	IMPORT_C static TInt CreateChunk(TInt aFunction, DChunk*& aChunk, TChunkCreateInfo &aAttributes, 
		TUint aOffset, TUint aSize, TUint32& aPciAddress);
	IMPORT_C static TInt RemoveChunk(TInt aFunction, DPlatChunkHw* aChunk);
	IMPORT_C static TInt CreateMapping(TInt aFunction, TUint32 aPhysicalAddress, TInt aSize, 
		TUint32& aPciAddress);
	IMPORT_C static TInt RemoveMapping(TInt aFunction, TUint32 aPhysicalAddress);
	IMPORT_C static TInt GetPciAddress(TInt aFunction, TUint32& aAddress);
	IMPORT_C static TInt GetPhysicalAddress(TInt aFunction, TUint32& aAddress);


	static TInt Enumerate();
protected:
	friend class DPciBridge;
	static TInt AddBridge(DPciBridge* aBridge);

	friend class TChunkCleanup; //!< Allow Cleanup object to use callback
	IMPORT_C static void ChunkCleanupCallback(TChunkCleanup* aCleanup);
	
	static TBool iEnumerated;						//!< Is toggled after enummeration
	static RPointerArray<DPciBridge> iPciBridges;			//!< All the bridges on the system
	static RPointerArray<TPciFunction> iFunctions;	//!< A system wide list of available PCI functions
	};


#define READ(n) virtual TUint##n Read##n(TUint32 aOffset)
#define WRITE(n) virtual void Write##n(TUint32 aOffset, TUint##n aValue)
#define MODIFY(n) virtual void Modify##n(TUint32 aOffset, TUint##n aClearMask, TUint##n aSetMask)
/**
Allows reads and writes to a section of address space.
*/
class TAddrSpace
	{
public:
	IMPORT_C TUint Size();

	IMPORT_C READ(8) =0;
	IMPORT_C READ(16) =0;
	IMPORT_C READ(32) =0;

	IMPORT_C WRITE(8) =0;
	IMPORT_C WRITE(16) =0;
	IMPORT_C WRITE(32) =0;

	IMPORT_C MODIFY(8) =0;
	IMPORT_C MODIFY(16) =0;
	IMPORT_C MODIFY(32) =0;
protected:
	enum TNumberOfBytes {E1Byte=1, E2Byte=2, E4Byte=4};
	void CheckAccess(TNumberOfBytes aNumberOfBytes, TUint aByteOffset);

	TAddrSpace(TUint aSize, const TDesC& aName);
	const TUint iSize; //!< Size of address space in bytes
	const TDesC& iName; //!< Used in trace messages
	};
#endif //_PCI_H
