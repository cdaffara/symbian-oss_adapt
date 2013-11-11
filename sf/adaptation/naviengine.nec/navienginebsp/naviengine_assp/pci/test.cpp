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
#include <naviengine_priv.h>
#include <naviengine.h>
#include <kernel/cache.h>
#include "allocator.h"
#include "pci-ne.h"

#define TEST(X) __NK_ASSERT_ALWAYS(X)
#define TEST_KERRNONE(X) TEST((X)==KErrNone)

/**
Make sure that I can read and write some values as expected
*/
void TestConfigAccess(TAddrSpace& aCfgSpc)
	{
	TEST(aCfgSpc.Size()==0x100);

	TEST(aCfgSpc.Read32(0x0)==0x00351033);

	TEST(aCfgSpc.Read16(0x0)==0x1033);

	TEST(aCfgSpc.Read16(0x2)==0x0035);

	TEST(aCfgSpc.Read8(0x0)==0x33);

	TEST(aCfgSpc.Read8(0x1)==0x10);

	TEST(aCfgSpc.Read8(0x02)==0x35);

	TEST(aCfgSpc.Read8(0x3)==0x00);

	//test writes (there aren't that many contiguous blocks of writable bits in config space, but the BAR will do)
	const TUint32 original= aCfgSpc.Read32(KPciBar0);

	aCfgSpc.Write32(KPciBar0,0xFFFFFFFF);
	const TUint32 filled = aCfgSpc.Read32(KPciBar0);
	TEST(filled==0xFFFFF000); //not all of register is writable

	aCfgSpc.Write8(KPciBar0+0x3,0xBA);
	TEST(aCfgSpc.Read8(KPciBar0+0x3)== 0xBA);
	TEST(aCfgSpc.Read32(KPciBar0)== 0xBAFFF000);

	aCfgSpc.Write8(KPciBar0+0x2,0x55);
	TEST(aCfgSpc.Read8(KPciBar0+0x2)== 0x55);
	TEST(aCfgSpc.Read32(KPciBar0)== 0xBA55F000);

	aCfgSpc.Write8(KPciBar0+0x1,0x42);
	TEST(aCfgSpc.Read8(KPciBar0+0x1)== 0x40); //lower nibble unwriteable
	TEST(aCfgSpc.Read32(KPciBar0)== 0xBA554000);

	aCfgSpc.Write16(KPciBar0+0x0,0x5000);
	TEST(aCfgSpc.Read16(KPciBar0+0x0)== 0x5000);
	TEST(aCfgSpc.Read32(KPciBar0)== 0xBA555000);

	aCfgSpc.Write16(KPciBar0+0x2,0xAAAA);
	TEST(aCfgSpc.Read16(KPciBar0+0x2)== 0xAAAA);
	TEST(aCfgSpc.Read32(KPciBar0)== 0xAAAA5000);


	//test modifies
	aCfgSpc.Modify8(KPciBar0+0x3, 0xFF, 0x3C);
	TEST(aCfgSpc.Read8(KPciBar0+0x3) == 0x3C);

	aCfgSpc.Modify8(KPciBar0+0x2, 0xFF, 0x3C);
	TEST(aCfgSpc.Read8(KPciBar0+0x2) == 0x3C);

	//restore original value.
	aCfgSpc.Write32(KPciBar0, original);
	TEST(aCfgSpc.Read32(KPciBar0)==original);
	}


TBool TestMemoryAccess(TAddrSpace& memSpace)
	{
	TEST(memSpace.Size()==0x1000);

	//try some writes to the HcControlHeadED register - bits 31:4 are writeable
	const TUint KReg=0x20;
	const TUint32 initial = memSpace.Read32(KReg);

	memSpace.Write32(KReg, 0x0);
	TEST(memSpace.Read32(KReg)==0x0);

	memSpace.Write32(KReg, 0xFFFFFFFF);
	TEST(memSpace.Read32(KReg)==0xFFFFFFF0); //nibble0 read-only

	memSpace.Write16(KReg+2, 0xDEAD);
	TEST(memSpace.Read32(KReg)==0xDEADFFF0);

	memSpace.Write16(KReg, 0xABCD);
	TEST(memSpace.Read16(KReg)==0xABC0);
	TEST(memSpace.Read16(KReg+2)==0xDEAD);

	memSpace.Modify32(KReg, 0x0000FFFF, 0x0F0F0000);
	TEST(memSpace.Read32(KReg)==0xDFAF0000);
	TEST(memSpace.Read8(KReg+3)==0xDF);

	memSpace.Write8(KReg+1,0x42);
	TEST(memSpace.Read8(KReg+0)==0x00);
	TEST(memSpace.Read8(KReg+1)==0x42);
	TEST(memSpace.Read8(KReg+2)==0xAF);
	TEST(memSpace.Read8(KReg+3)==0xDF);
	TEST(memSpace.Read32(KReg)==0xDFAF4200);

	memSpace.Modify8(KReg+3,0xFF,0x10);
	TEST(memSpace.Read32(KReg)==0x10AF4200);

	//reset to inital value
	memSpace.Write32(KReg, initial);
	TEST(memSpace.Read32(KReg)==initial);

	return ETrue;
	}

void TestAllocator()
	{
	__KTRACE_OPT(KPCI, Kern::Printf("Testing address allocator"));
	TAddressAllocator allocator(0x80000000); //2 GB
	TLinAddr rcvdAddr=NULL;
	TEST_KERRNONE(allocator.Allocate(rcvdAddr,0x10) );
	TEST(0x0 ==rcvdAddr);
	TEST_KERRNONE(allocator.Allocate(rcvdAddr,0x100) );
	TEST(0x100 ==rcvdAddr);
	TEST_KERRNONE(allocator.Allocate(rcvdAddr,0x10) );
	TEST(0x10 ==rcvdAddr);
	TEST_KERRNONE(allocator.Allocate(rcvdAddr,0x10) );
	TEST(0x20 ==rcvdAddr);
	//test deallocating
	TEST_KERRNONE(allocator.DeAllocate(0x0));
	TEST_KERRNONE(allocator.Allocate(rcvdAddr,0x10) );
	TEST(0x000 ==rcvdAddr);

	TEST_KERRNONE(allocator.DeAllocate(0x100));
	TEST_KERRNONE(allocator.Allocate(rcvdAddr,0x100) );
	TEST(0x100 ==rcvdAddr);

	TEST_KERRNONE(allocator.DeAllocate(0x10));
	TEST_KERRNONE(allocator.Allocate(rcvdAddr,0x10) );
	TEST(0x10 ==rcvdAddr);

	TEST_KERRNONE(allocator.DeAllocate(0x20));
	TEST_KERRNONE(allocator.Allocate(rcvdAddr,0x20) );
	TEST(0x20 ==rcvdAddr);

	TEST(allocator.DeAllocate(0x40)==KErrNotFound);
	TEST_KERRNONE(allocator.DeAllocate(0x100));
	TEST_KERRNONE(allocator.DeAllocate(0x20));
	TEST_KERRNONE(allocator.DeAllocate(0x0));
	TEST_KERRNONE(allocator.DeAllocate(0x10));
	}

/**
Wrapper to get chunk and its virtual address
*/
TInt CreatePciChunk(TInt aFunc, TInt& aSize, TUint32 aAttributes, DPlatChunkHw*& aChunk, TUint32& aPci, TLinAddr& aVirt)
	{
	TInt r = Pci::CreateChunk(aFunc, aChunk, aSize, aAttributes, aPci);
	if(r!=KErrNone)
		return r;

	aVirt=aChunk->LinearAddress();

	return r;
	}

/**
Wrapper to create chunk and append to array
*/
TInt CreatePciChunkAppend(TInt aFunc, TInt aSize, TUint32 aAttributes, RPointerArray<DPlatChunkHw>& aChunks)
	{
	DPlatChunkHw* chunk=NULL;
	TUint32 pci=NULL;
	TInt r = Pci::CreateChunk(aFunc, chunk, aSize, aAttributes, pci);
	if(KErrNone==r)
		{
		TEST_KERRNONE(aChunks.Append(chunk));
		}
	return r;
	}

TInt CreateSharedChunk(TInt aSize, TUint32& aAttributes, DChunk*& aChunk, TLinAddr& aVirt, TPhysAddr& aPhysicalAddress)
	{
	TEST(aChunk==NULL);
	aSize = Kern::RoundToPageSize(aSize);
	TChunkCreateInfo info;
	info.iType=TChunkCreateInfo::ESharedKernelSingle;
	info.iMaxSize=aSize;
	info.iMapAttr=aAttributes;
	info.iOwnsMemory=ETrue;

	DChunk* pC=NULL;

	NKern::ThreadEnterCS();
	TInt r=Kern::ChunkCreate(info, pC, aVirt, aAttributes);
	if(r!=KErrNone)
		{
		NKern::ThreadLeaveCS();
		return r;
		}

	r = Kern::ChunkCommitContiguous(pC, 0, aSize, aPhysicalAddress);

	if(r==KErrNone)
		{
		aChunk=pC;
		}
	else
		{
		Kern::ChunkClose(pC);
		}

	NKern::ThreadLeaveCS();
	__KTRACE_OPT(KPCI, Kern::Printf("Created SC: size=0x%08x, virtual= 0x%08x, phys=0x%08x", aSize, aVirt, aPhysicalAddress));
	return r;
	}

TInt UnmapAndCloseSC(TInt aFunc, DChunk* aChunk, TPhysAddr aPhysicalAddress)
	{
	TInt r = Pci::RemoveMapping(aFunc, aPhysicalAddress);
	TEST_KERRNONE(r);
	NKern::ThreadEnterCS();
	TEST(Kern::ChunkClose(aChunk)); //test that ref count has gone to zero
	NKern::ThreadLeaveCS();
	return r;
	}

TInt CreateAndMapSC(TInt aFunc, TInt aSize, TUint32 aAttributes, DChunk*& aChunk, TUint32& aPci, TLinAddr& aVirt, TPhysAddr& aPhysicalAddress)
	{
	TInt r = CreateSharedChunk(aSize, aAttributes, aChunk, aVirt, aPhysicalAddress);
	TEST_KERRNONE(r);
	TEST(aChunk);
	TEST_KERRNONE(r);
	r=Pci::CreateMapping(aFunc, aPhysicalAddress, aSize, aPci);
	TEST_KERRNONE(r);
	return r;
	}

void TestChunkAllocation(TInt func)
	{
	__KTRACE_OPT(KPCI, Kern::Printf("test allocating chunks"));
	const TUint32 attributes= EMapAttrSupRw|EMapAttrFullyBlocking;

	//keep track of the chunks so i can delete them afterwards
	RPointerArray<DPlatChunkHw> chunkList(12);

	TEST_KERRNONE(CreatePciChunkAppend(func, 0x1000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x4000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x1000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x2000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x8000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x10000, attributes, chunkList));

	TEST_KERRNONE(CreatePciChunkAppend(func, 0x8000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x1000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x4000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x1000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x2000, attributes, chunkList));
	TEST_KERRNONE(CreatePciChunkAppend(func, 0x80000, attributes, chunkList));

	//try to allocate more chunks than there are BARS
	TEST(CreatePciChunkAppend(func, 0x1000, attributes, chunkList)==KErrNotFound);
	//try to allocate chunk with size less than 1
	TEST(CreatePciChunkAppend(func, 0x0, attributes, chunkList)==KErrArgument);

	__KTRACE_OPT(KPCI, Kern::Printf("Delete all chunks"));
	const TInt count=chunkList.Count();
	for(TInt i=0; i<count; ++i)
		{
		TEST_KERRNONE(Pci::RemoveChunk(func, chunkList[i]));
		}
	chunkList.Close();
	}

void TestPciMemoryAccess(TLinAddr aVirt, TUint32 aPciAddress, TUint aSize, TUint32 aAttributes)
	{
	__KTRACE_OPT(KPCI, Kern::Printf("\nTest accessing memory via PCI: attrs=0x%08x\n", aAttributes));

	TEST(aPciAddress<=(0x2000-aSize)); //we need to fit within the first 8k of pci space or we'll be beyond the access window

	__KTRACE_OPT(KPCI, Kern::Printf("write chunk from software"));
	TUint8* const vByte=(TUint8*)aVirt;
	memset(vByte, 0x5A, aSize);

	__KTRACE_OPT(KPCI, Kern::Printf("confirm values"));
	for(TUint i=0; i<aSize; ++i)
		{
		const TUint8 result=vByte[i];
		TEST(result==0x5A);
		}

	__KTRACE_OPT(KPCI, Kern::Printf("read back chunk via PCI"));
	//A standard DMA write would read from memory and *write* to peripheral
	//in this case the PCI is doing a DMA read from memory.
	Cache::SyncMemoryBeforeDmaWrite(aVirt, aSize, aAttributes);
	TUint8* const pByte = (TUint8*)(KHwUsbHWindow+aPciAddress);
	TEST(memcompare(vByte, aSize, pByte, aSize)==0);

	//A standard DMA read would *read* from peripheral and write to memory.
	Cache::SyncMemoryBeforeDmaRead(aVirt, aSize, aAttributes);
	__KTRACE_OPT(KPCI, Kern::Printf("write chunk via PCI"));
	memset(pByte,0xA5,aSize);

	__KTRACE_OPT(KPCI, Kern::Printf("read back chunk via PCI"));
	for(TUint i=0; i<aSize; ++i)
		{
		const TUint8 result=pByte[i];
		TEST(result==0xA5);
		}
	Cache::SyncMemoryAfterDmaRead(aVirt, aSize);
	__KTRACE_OPT(KPCI, Kern::Printf("read back chunk from software"));
	TEST(memcompare(vByte, aSize, pByte, aSize)==0);
	}

void TestAddressConversion(TInt aFunc, TUint32 aPciAddr, TUint32 aPhysAddr, TInt32 aSize)
	{
	//round up size of region to what its equivilent pci region would be
	if(aSize<KMinOutboundWindow)
		aSize=KMinOutboundWindow;
	else
		aSize=Clp2(aSize); //round up to next Power of 2.


	__KTRACE_OPT(KPCI, Kern::Printf("testing Pci<-->Phys address conversions: func %d, pci 0x%08x, physical 0x%08x, size 0x%08x ", aFunc, aPciAddr, aPhysAddr, aSize));
	for(TInt offset=-1; offset<=aSize; ++offset)
		{
		const TUint32 expectedPci=aPciAddr+TInt(offset);
		TUint32 address=aPhysAddr+TInt(offset);
		TInt r=Pci::GetPciAddress(aFunc,address);
		//__KTRACE_OPT(KPCI, Kern::Printf("GetPciAddress: offset=0x%08x, r=%d, physical=0x%08x, pci=0x%08x,expectedPci=0x%08x",
		//		offset, r, aPhysAddr+offset, address, expectedPci));
		if(offset==-1||offset==aSize)
			{
			TEST(r==KErrNotFound); //test going beyond either side of the memory region
			}
		else
			{
			TEST_KERRNONE(r);
			TEST(address==expectedPci);
			}

		const TUint32 expectedPhys=aPhysAddr+offset;
		address=aPciAddr+offset;
		r=Pci::GetPhysicalAddress(aFunc, address);
		//__KTRACE_OPT(KPCI, Kern::Printf("GetPhysicalAddress: offset=0x%08x, r=%d, pci=0x%08x, physical=0x%08x, expectedPhysical=0x%08x",
		//		offset, r, aPciAddr+offset, address, expectedPhys));
		if(offset==-1||offset==aSize)
			{
			TEST(r==KErrNotFound);
			}
		else
			{
			TEST_KERRNONE(r);
			TEST(address==expectedPhys);
			}
		}
	}

void TestPciChunkAccesses(TInt aFunc)
	{
	const TInt sizes[] = {0x1, 0x400, 0x800};
	const TUint32 attrs[] = {EMapAttrSupRw|EMapAttrFullyBlocking, EMapAttrSupRw|EMapAttrCachedMax};

	__KTRACE_OPT(KPCI, Kern::Printf("Testing PCI chunk access"));
	for(TInt i=0; i<3; ++i)
		{
		for(TInt j=0; j<2; ++j)
			{
			__KTRACE_OPT(KPCI, Kern::Printf("\nSize=0x%08x, attrs=0x%08x", sizes[i], attrs[j]));
			TLinAddr virt=NULL;
			TUint32 pci=NULL;
			DPlatChunkHw* chunk=NULL;
			TInt size=sizes[i];//size may be altered by CreatePciChunk if it is rounded
			TInt r = CreatePciChunk(aFunc, size, attrs[j], chunk, pci, virt);
			TEST_KERRNONE(r);
			TestPciMemoryAccess(virt, pci, size, attrs[j]);
			TestAddressConversion(aFunc, pci, chunk->PhysicalAddress(), sizes[i]);

			r = Pci::RemoveChunk(aFunc, chunk);
			TEST_KERRNONE(r);

			}
		}
	}

void TestWindowAccesses(TInt aFunc)
	{
	const TInt sizes[] = {0x1, 0x400, 0x800};
	const TUint32 attrs[] = {EMapAttrSupRw|EMapAttrFullyBlocking, EMapAttrSupRw|EMapAttrCachedMax};

	__KTRACE_OPT(KPCI, Kern::Printf("Testing PCI access to externally allocated shared chunks"));
	for(TInt i=0; i<3; ++i)
		{
		for(TInt j=0; j<2; ++j)
			{
			__KTRACE_OPT(KPCI, Kern::Printf("\nSize=0x%08x, attrs=0x%08x", sizes[i], attrs[j]));
			TLinAddr virt=NULL;
			TUint32 pci=NULL;
			TPhysAddr phys=NULL;
			DChunk* chunk=NULL;
			TInt r = CreateAndMapSC(aFunc, sizes[i], attrs[j], chunk, pci, virt, phys);
			TEST_KERRNONE(r);
			TestPciMemoryAccess(virt, pci, sizes[i], attrs[j]);
			TestAddressConversion(aFunc, pci, phys, sizes[i]);

			r = UnmapAndCloseSC(aFunc, chunk, phys);
			TEST_KERRNONE(r);

			}
		}


	}

DECLARE_STANDARD_EXTENSION()
	{
	TInt r=KErrNone;
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Starting PCI test extension"));

	RArray<TInt> indicies;
	TEST(Pci::Probe(indicies, KNecVendorId, KInternalPciBridgeId) == KErrNotFound);
	TEST(Pci::Probe(indicies, KNecVendorId, KExternalPciBridgeId) == KErrNotFound);

	TEST(Pci::Probe(indicies,0x1033, 0x35) == KErrNone);
	TEST(indicies.Count()==1); //only expecting one match
	const TInt ohciFunc=indicies[0];
	indicies.Close();

	TAddrSpace& cs = *Pci::GetConfigSpace(ohciFunc);
	TAddrSpace& ms = *Pci::GetMemorySpace(ohciFunc);

	TestMemoryAccess(ms);
	TestConfigAccess(cs);
	TestAllocator();
	TestChunkAllocation(ohciFunc);
	TestPciChunkAccesses(ohciFunc);
	TestWindowAccesses(ohciFunc);

	__KTRACE_OPT(KPCI, Kern::Printf("DNaviEnginePci: Dumping OHCI config space"));
	TUint size=cs.Size();
	for(TUint i=0; i<size; i+=4)
		{
		__KTRACE_OPT(KPCI, Kern::Printf("%08x %02xH", cs.Read32(i), i));
		}

	__KTRACE_OPT(KPCI, Kern::Printf("DNaviEnginePci: Dumping OHCI memory space"));
	size=ms.Size();
	for(TUint i=0; i<size; i+=4)
		{
		__KTRACE_OPT(KPCI, Kern::Printf("%08x %02xH", ms.Read32(i), i));
		}

	//this will cause a master abort if it touches any unallocated PCI memory, which is likely.
#ifdef CAUSE_ERROR
	__KTRACE_OPT(KPCI, Kern::Printf("Dump all of PCI space"));
	TUint32* pciSpace=(TUint32*)KHwUsbHWindow;
	for(TInt i=0; i<0x2000; ++i)
		{
		__KTRACE_OPT(KPCI, Kern::Printf("%08x %02xH", pciSpace[i], i));
		}
#endif
	Kern::Printf("Dump PCI bridge registers");
	for(TInt i=0; i<0x100; i+=4)
		{
		__KTRACE_OPT(KPCI, Kern::Printf("%08x %02xH", reinterpret_cast<TUint32*>(KHwPciBridgeUsb)[i], i));
		}
	return r;
	}

