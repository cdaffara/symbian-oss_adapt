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
#include <naviengine.h>
#include <kernel/kern_priv.h>

//
// TChunkManager
//

TChunkManager::TChunkRecord::TChunkRecord(DPlatChunkHw* aChunk, TInt32 aSize)
	:iChunk(aChunk), iSize(aSize)
	{
	}

/**
A utitlity function so that we can find pci chunks structs in iChunks array
*/
TBool TChunkManager::TChunkRecord::ChunkComparator(const TChunkRecord& aKeyRecord, const TChunkRecord& aRecord)
	{
	return (aKeyRecord.iChunk==aRecord.iChunk);
	}

TChunkManager::TChunkManager(TMappingManager& aMapMan)
	:iMapMan(aMapMan), iChunks(), iSharedChunks(), iMutex(NULL)
	{
	_LIT(KChunkManMutex, "PCI_Chunk_Man_Mutex");
	TInt r = Kern::MutexCreate(iMutex, KChunkManMutex, KMutexOrdGeneral2);
	__NK_ASSERT_ALWAYS(KErrNone==r);
	}

TChunkManager::~TChunkManager()
	{
	iChunks.Reset();
	iSharedChunks.Reset();
	iMutex->Close(NULL);
	}


/**
@param aChunk a NULL pointer. Will be set to new chunk on success.
@param aPciAddress On success will be set to address of chunk buffer in PCI address space
@param aSize The size of the chunk to allocate
@return An errror code.
	- KErrNotFound All of Bridges BARS have been used up.
*/
TInt TChunkManager::AddChunk(DPlatChunkHw*& aChunk, TInt& aSize, TUint aAttributes, TUint32& aPciAddress)
	{
	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	DPlatChunkHw* chunk=NULL;
	TUint32 pciAddress=NULL;
	TInt32 size=aSize;

	TInt r=DoAddChunk(chunk, aSize, aAttributes, pciAddress);
	if(r==KErrNone)
		{
		aChunk=chunk;
		aPciAddress=pciAddress;
		aSize=size;
		}

	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();

	return r;
	}

/**
@param aPciAddress On success will be set to address of chunk buffer in PCI address space
@param aSize The size of the chunk to allocate
@return An errror code.
	- KErrNotFound All of Bridges BARS have been used up.
*/
TInt TChunkManager::DoAddChunk(DPlatChunkHw*& aChunk, TInt aSize, TUint aAttributes, TUint32& aPciAddress)
	{
	
	#ifdef _DEBUG	
	__ASSERT_CRITICAL;	
	__ASSERT_MUTEX(iMutex); 
	#endif

	TInt requestedSize=Clp2(aSize);
	//Although the client has specified aSize, the actual chunk we get may be larger as it will be rounded to a page size.
	//CreateChunk may adjust its size parameter to be the actual size allocated.
	TInt r = CreateChunk(aChunk, aSize, aAttributes);
	if(r!=KErrNone)
		return r;

	const TUint32 physicalAddress=aChunk->PhysicalAddress();
	
	//even if physical memory block was 4K, it is ok to have a smaller
	//pci mapping as long as its size is power of 2
	TInt mapSize = Min(requestedSize, aSize);
	r = iMapMan.CreateMapping(physicalAddress, mapSize, aPciAddress);

	//no point having chunk if no mapping in pci space.
	if(r!=KErrNone)
		{
		const TInt ret=DeleteChunk(aChunk, aSize);
		__NK_ASSERT_ALWAYS(KErrNone==ret);
		return r;
		}

	//remember the actual size of memory allocated to chunk
	const TChunkRecord newRecord(aChunk, aSize);
	r = iChunks.Append(newRecord);

	// delete chunk if a record of it can't be kept.
	if(r!=KErrNone)
		{
		const TInt ret=DeleteChunk(aChunk, aSize);
		__NK_ASSERT_ALWAYS(KErrNone==ret);
		return r;
		}

	return r;
	}

/**
Creates a chunk of specified size, but rounded up to at least a page.
@param aChunk On success will be set to the new chunk.
@param aSize Size of chunk required, on return will have modfied if size was rounded up.
@param aAttributes A bit mask of TMappingAttributes values.
@return
	- KErrNone
	- KErrNoMemory
*/
TInt TChunkManager::CreateChunk(DPlatChunkHw*& aChunk, TInt& aSize, TUint aAttributes)
	{
	aSize = Kern::RoundToPageSize(aSize);
	aSize = Clp2(aSize);
	
	NKern::ThreadEnterCS();

	TPhysAddr physicalAddress=NULL;
	TInt r=Epoc::AllocPhysicalRam(aSize, physicalAddress, Log2(aSize));
	if(r!=KErrNone)
		{
		__KTRACE_OPT(KPCI, Kern::Printf("TChunkManager::CreateChunk(): Phys memory alloc failed with error=%d, size=%d (0x%X)", r, aSize, aSize));
		NKern::ThreadLeaveCS();
		return r;
		}

	r = DPlatChunkHw::New(aChunk,physicalAddress, aSize, aAttributes);
	if(r!=KErrNone)
		{
		__KTRACE_OPT(KPCI, Kern::Printf("TChunkManager::CreateChunk(): Chunk creation failed with status %d, phys addr=0x%08x, size=%d (0x%X)", r, physicalAddress, aSize, aSize));
		const TInt ret=Epoc::FreePhysicalRam(physicalAddress, aSize);
		__NK_ASSERT_ALWAYS(KErrNone==ret);
		NKern::ThreadLeaveCS();
		return r;
		}

	NKern::ThreadLeaveCS();

	__KTRACE_OPT(KPCI, Kern::Printf("TChunkManager::CreateChunk(): Chunk created at 0x%08x, phys addr=0x%08x, size=%d (0x%X)", aChunk, physicalAddress, aSize, aSize));
	return r;
	}


/**
Delete and remove PCI mapping for a chunk previously created by this class

@param aChunk pointer to a chunk to remove.
@return
	- KErrNone
	- KErrNotFound aChunk was not allocated by this class
*/
TInt TChunkManager::RemoveChunk(DPlatChunkHw* aChunk)
	{
	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	__KTRACE_OPT(KPCI, Kern::Printf("TChunkManager::RemoveChunk(): Removing chunk at 0x%08X", aChunk));
	const TChunkRecord key(aChunk, 0);
	TInt index=iChunks.Find(key, TIdentityRelation<TChunkRecord>(TChunkRecord::ChunkComparator) );
	TInt r=KErrNone;
	if(index!=KErrNotFound)
		{
		r = iMapMan.RemoveMapping(iChunks[index].iChunk->PhysicalAddress() );
		__NK_ASSERT_ALWAYS(KErrNone==r);
		r = DeleteChunk(iChunks[index].iChunk, iChunks[index].iSize);
		__NK_ASSERT_ALWAYS(r==KErrNone);

		iChunks.Remove(index);
#ifdef _DEBUG
		//free space when entry removed
		iChunks.Compress();
#endif
		}
	else
		r=index;

	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();

	return r;
	}

/**
Delete chunk and remove aSize of physical RAM
*/
TInt TChunkManager::DeleteChunk(DPlatChunkHw* aChunk, TInt aSize)
	{
	const TPhysAddr address = aChunk->PhysicalAddress();
	NKern::ThreadEnterCS();
	__KTRACE_OPT(KPCI, Kern::Printf("TChunkManager::DeleteChunk(): Freeing physical RAM: %d (0x%X) bytes at 0x%X ", aSize, aSize, address));
	Kern::SafeClose(reinterpret_cast<DObject*&>(aChunk), NULL);
	TInt r=Epoc::FreePhysicalRam(address,aSize);
	NKern::ThreadLeaveCS();
	return r;
	}


//
// Shared Chunk methods
//

TChunkManager::TSCRecord::TSCRecord(DChunk* aChunk, TUint32 aPhysAddr)
	:iChunk(aChunk), iPhysAddr(aPhysAddr), iMapped(EFalse), iSize(0)
	{
	}


TBool TChunkManager::TSCRecord::ChunkComparator(const TSCRecord& aKeyRecord, const TSCRecord& aRecord)
	{
	return (aKeyRecord.iPhysAddr==aRecord.iPhysAddr);
	}

TInt TChunkManager::AddChunk(DChunk*& aChunk, TChunkCreateInfo& aAttributes, TUint aOffset, TUint& aSize, TUint32& aPciAddress)
	{
	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	DChunk* chunk=NULL;
	TUint32 pciAddress;

	TInt r=DoAddChunk(chunk, aAttributes, aOffset, aSize, pciAddress);
	if(r==KErrNone)
		{
		aChunk=chunk;
		aPciAddress=pciAddress;
		}
	
	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();

	return r;
	}

void TChunkManager::RemoveChunk(TUint32 aPhysicalAddress)
	{
	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	const TSCRecord key(NULL, aPhysicalAddress);

	TInt index=iSharedChunks.Find(key, TIdentityRelation<TSCRecord>(TSCRecord::ChunkComparator) );
	__NK_ASSERT_ALWAYS(index!=KErrNotFound);

	TSCRecord& record(iSharedChunks[index]);
	__NK_ASSERT_DEBUG(record.iPhysAddr==aPhysicalAddress);

	// We will always have to free RAM
	const TInt size=record.iSize;
	__KTRACE_OPT(KPCI, Kern::Printf("TChunkManager::RemoveChunk(TUint32) Freeing physical RAM: %d (0x%X) bytes at 0x%X ", size, size, aPhysicalAddress));
	NKern::ThreadEnterCS();
	TInt r = Epoc::FreePhysicalRam(aPhysicalAddress, size);
	NKern::ThreadLeaveCS();
	__NK_ASSERT_ALWAYS(KErrNone==r);

	if(record.iMapped)
		{
		__KTRACE_OPT(KPCI,
			Kern::Printf("TChunkManager::RemoveChunk(TUint32) Removing PCI mapping for RAM: %d (0x%X) bytes at 0x%X ",
				size, size, record.iPhysAddr));

		TInt r=iMapMan.RemoveMapping(record.iPhysAddr);
		__NK_ASSERT_ALWAYS(KErrNone==r);
		}
	else
		{
		__KTRACE_OPT(KPCI, Kern::Printf("TChunkManager::RemoveChunk(TUint32) No PCI mapping for memory")); 
		}

	iSharedChunks.Remove(index);
#ifdef _DEBUG
	//free space when entry removed
	iSharedChunks.Compress();
#endif

	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();
	}

TInt TChunkManager::DoAddChunk(DChunk*& aChunk, TChunkCreateInfo& aAttributes, TUint aOffset, TUint& aSize, TUint32& aPciAddress)
	{
	TInt r = iSharedChunks.Append(TSCRecord(NULL, NULL));
	if(r!=KErrNone)
		{
		return r;
		}

	const TInt count = iSharedChunks.Count();
	TSCRecord& newRecord(iSharedChunks[count-1]);
	//Although the client has specified aSize, the actual chunk we get may be larger as it will be rounded to a page size.
	//CreateChunk may adjust its size parameter to be the actual size allocated.
	r = CreateChunk(aChunk, aAttributes, aOffset, aSize, newRecord);
	if(r!=KErrNone)
		{
		//If newRecord is not populated we must remove it.
		//If it has been populated, the chunk record will
		//be removed by the chunk's cleanup DFC and we must leave
		//it alone.
		if(newRecord.iChunk==NULL)
			{
			iSharedChunks.Remove(count-1);
#ifdef _DEBUG
			//free space when entry removed
			iSharedChunks.Compress();
#endif
			}

		return r;
		}

	//map all of the commited memory, which may exceed what the client requested
	//The minumum memory allocation is 4k
	//And memory must also be mapped in powers of 2 eg. A request for 11k would create
	//and map 16K


	//need CS since if we succeed in creating a mapping we
	//must be allowed to toggle the iMapped flag in the record array.
	NKern::ThreadEnterCS();
	TInt size=aSize;
	r = iMapMan.CreateMapping(newRecord.iPhysAddr, size, aPciAddress);

	//no point having chunk if no mapping in pci space.
	if(r!=KErrNone)
		{
		//chunk will proceed to cleanup memory only
		TBool destructionPending=Kern::ChunkClose(aChunk);
		NKern::ThreadLeaveCS();
		__NK_ASSERT_ALWAYS(destructionPending);
		return r;
		}
	newRecord.iMapped=ETrue;
	NKern::ThreadLeaveCS();

	__NK_ASSERT_ALWAYS(r==KErrNone); //we reserved space for this

	return r;
	}

TInt TChunkManager::CreateChunk(DChunk*& aChunk, TChunkCreateInfo& aAttributes, TUint aOffset, TUint& aSize, TSCRecord& aRecord)
	{
	//natuarally align aSize.
	const TInt size=Clp2(aSize);
	const TInt align=Log2(size);	

	TPhysAddr physicalAddress=NULL;

	NKern::ThreadEnterCS();
	TInt r = Epoc::AllocPhysicalRam(size, physicalAddress, align);	
    if(r != KErrNone)
	    {
		__KTRACE_OPT(KPCI, Kern::Printf("TChunkManager::CreateChunk(DChunk*): Phys memory alloc failed with error=%d, size=%d (0x%X)", r, size, size));
		NKern::ThreadLeaveCS();
		return r; 
	    }
	
	//now that we know the physical address of our memory
	//we can create the cleanup object
	TNaviEngineChunkCleanup* cleanup= new TNaviEngineChunkCleanup(*this, physicalAddress);
	if(cleanup == NULL)
		{
		//free physical ram
		r = Epoc::FreePhysicalRam(physicalAddress, size);
		__NK_ASSERT_ALWAYS(KErrNone==r);
		NKern::ThreadLeaveCS();
		return KErrNoMemory;
		}

	//Since we are mapping in memory we alloc'd
	//the chunk is not responsible for freeing it.
	aAttributes.iOwnsMemory=EFalse;

	//ensure that max size is large enough to contain the rounded physical block plus specified guard offsets
	//at each end
	aAttributes.iMaxSize=Max(aAttributes.iMaxSize, size+(2*aOffset));

	aAttributes.iDestroyedDfc = cleanup;

	TLinAddr kernAddr;
	TUint32 attribs;
	r=Kern::ChunkCreate(aAttributes, aChunk, kernAddr, attribs);
    if(r != KErrNone)
	    {
		//free physical ram
		TInt err = Epoc::FreePhysicalRam(physicalAddress, size);
		__NK_ASSERT_ALWAYS(KErrNone==err);

		delete cleanup;
		NKern::ThreadLeaveCS();
		return r;
	    }
	//At this point, the cleanup object will look after its own destruction
	//when the chunk is closed
	cleanup=NULL;
	
	//these will be requird in order to free physical memory if we have to close the chunk
	aRecord.iPhysAddr=physicalAddress;
	aRecord.iChunk=aChunk;	
	aRecord.iSize=size;
		
	r=Kern::ChunkCommitPhysical(aChunk, aOffset, size, physicalAddress);  

	if(r!=KErrNone)
		{
		TBool destructionPending=Kern::ChunkClose(aChunk);
		__NK_ASSERT_ALWAYS(destructionPending);
		NKern::ThreadLeaveCS();
		return r;
		}

	//report back size commited
	aSize=size;
	__KTRACE_OPT(KPCI, Kern::Printf("TChunkManager::CreateChunk(DChunk*): Chunk created at 0x%08x, phys addr=0x%08x, size=%d (0x%X)",
		aChunk, physicalAddress, aSize, aSize));

	NKern::ThreadLeaveCS();
	return r;
	}


