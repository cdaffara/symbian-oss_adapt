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



#ifndef __CHUNKMAN_H__
#define __CHUNKMAN_H__

#include <e32cmn.h>

class TAddressAllocator;
class DPlatChunkHw;
class DChunk;
class TChunkCreateInfo;
class DMutex;


/**
Responsible for allocating and deallocating chunks which
are accessible from the PCI bus.

At present there can be only 1 chunk associated with each BAR.
It would be possible to access more than one chunk per BAR, providing
the chunks were physically contiguous with each other,
*/
class TChunkManager
	{
public:
	TChunkManager(TMappingManager& aMapMan);
	~TChunkManager();
	TInt AddChunk(DPlatChunkHw*& aChunk, TInt& aSize, TUint aAttributes, TUint32& aPciAddress);
	TInt RemoveChunk(DPlatChunkHw* aChunk);

	TInt AddChunk(DChunk*& aChunk, TChunkCreateInfo& aAttributes, TUint aOffset, TUint& aSize, TUint32& aPciAddress);
	void RemoveChunk(TUint32 aPhysicalAddress);

private:
	/**
	Used to keep a record of an allocated chunk
	*/
	struct TChunkRecord
		{
		TChunkRecord(DPlatChunkHw* aChunk, TInt32 aSize);
		static TBool ChunkComparator(const TChunkRecord& aKeyRecord, const TChunkRecord& aRecord);

		DPlatChunkHw* iChunk;
		// DPlayChunkHw do not know the size which has been allocated to them.
		TInt32 iSize;
		};

	/**
	Used to keep a record of a shared chunk
	*/
	struct TSCRecord
		{
		TSCRecord(DChunk* aChunk, TUint32 aPhysAddr);
		static TBool ChunkComparator(const TSCRecord& aKeyRecord, const TSCRecord& aRecord);

		DChunk* iChunk;
		TUint32 iPhysAddr; //we need to remember this as it allows PCI mapping to be freed
		TBool iMapped; //says whether this chunk is mapped into PCI
		TInt32 iSize;
		};

	TInt DoAddChunk(DPlatChunkHw*& aChunk, TInt aSize, TUint aAttributes, TUint32& aPciAddress);
	TInt CreateChunk(DPlatChunkHw*& aChunk, TInt& aSize, TUint aAttributes);
	TInt DeleteChunk(DPlatChunkHw* aChunk, TInt aSize);

	TInt DoAddChunk(DChunk*& aChunk, TChunkCreateInfo& aAttributes, TUint aOffset, TUint& aSize, TUint32& aPciAddress);
	TInt CreateChunk(DChunk*& aChunk, TChunkCreateInfo& aAttributes, TUint aOffset, TUint& aSize, TSCRecord& aRecord);

	typedef DPlatChunkHw* DPlatChunkHwP;

	TMappingManager& iMapMan;

	RArray<TChunkRecord> iChunks;
	RArray<TSCRecord> iSharedChunks;
	DMutex* iMutex;


	};
#endif // __CHUNKMAN_H__
