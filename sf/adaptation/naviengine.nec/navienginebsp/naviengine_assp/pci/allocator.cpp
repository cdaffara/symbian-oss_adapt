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



#include "allocator.h"
#include <kernel/kern_priv.h>

TAddressAllocator::TAddressAllocator(TLinAddr aMaxSize)
	:iMaxSize(aMaxSize)
	{
	TInt r = iRangeList.Append(TRange(0, iMaxSize, EFalse));
	__NK_ASSERT_ALWAYS(KErrNone==r);

	_LIT(KAllocatorMutex, "PCIAllocatorMutex");
	r = Kern::MutexCreate(iMutex, KAllocatorMutex, KMutexOrdGeneral0);
	__NK_ASSERT_ALWAYS(KErrNone==r);
	}

TAddressAllocator::~TAddressAllocator()
	{
	__NK_ASSERT_DEBUG(iRangeList.Count()==1);
	iRangeList.Reset();
	iMutex->Close(NULL);
	iMutex=NULL;
	}

/**
@return Indicates if alloc succeeded
@param aAddress This will be set to the starting address of the range if allocation succeeded.
@param aSize Size of memory range required in bytes. This must be a power of 2.
*/
TInt TAddressAllocator::Allocate(TLinAddr& aAddress, TLinAddr aSize)
	{
	//check size alignment
	__NK_ASSERT_ALWAYS( (aSize & (aSize - 1)) == 0 ); //assert aSize is power of 2.
	TInt r = KErrNotFound;

	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	const TInt count = iRangeList.Count();
	for(TInt i = 0; i < count; ++i)
		{
		TRange& range(iRangeList[i]);
		if(range.iAllocated == EFalse)
			{
			//amount which must be added to start address to align to aSize
			//(aSize must be a power of 2)
			const TLinAddr alignedOffset = (-range.iStart) & (aSize - 1);

			if((alignedOffset + aSize) <= range.iLength)
				{
				const TLinAddr alignedStart = range.iStart + alignedOffset;
				DoAllocate(alignedStart, aSize, i);
				aAddress = alignedStart;
				r = KErrNone;
				break;
				}

			}
		}
	__KTRACE_OPT(KPCI, Kern::Printf("TAddressAllocator::Allocate(): Allocated %d (0x%X) bytes at 0x%X", 
		aSize, aSize, aAddress));
	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();
	return r;
	}

/**
Mark region as allocated, with the assumption that region is unallocated.

@param aAddress The start address requested for new region
@param aSize Length of new region
@param aIndex Position to modify in iRangeList
*/
void TAddressAllocator::DoAllocate(TLinAddr aAddress, TLinAddr aSize, TInt aIndex)
	{
	__NK_ASSERT_DEBUG(InvariantCheck());
	//check that new region will lie within existing range
	__NK_ASSERT_ALWAYS(
			(aAddress >= iRangeList[aIndex].iStart) &&
			((aAddress + aSize) <= (iRangeList[aIndex].End() + 1))
			);

	//allocating at start of unallocated region
	if(iRangeList[aIndex].iStart == aAddress)
		{
		//will there be space left at end of region?
		if((iRangeList[aIndex].iLength - aSize) > 0)
			{
			const TRange newFreeRange(iRangeList[aIndex].iStart + aSize, iRangeList[aIndex].iLength - aSize, EFalse);
			const TInt r = iRangeList.Insert(newFreeRange, aIndex + 1);
			__NK_ASSERT_ALWAYS(KErrNone == r);

			iRangeList[aIndex].iLength = aSize;
			}

		iRangeList[aIndex].iAllocated = ETrue;
		}
	else
		{
		//allocating from middle of an unallocated region
		const TRange newAllocRange(aAddress, aSize, ETrue);
		TInt r = iRangeList.Insert(newAllocRange, aIndex + 1);
		__NK_ASSERT_ALWAYS(KErrNone == r);

		//is there an unallocated gap after the newly allocated range?
		TLinAddr diff = iRangeList[aIndex].End() - newAllocRange.End();
		if( diff > 0 )
			{
			const TRange newFreeRange(newAllocRange.End() + 1, diff, EFalse);
			r=iRangeList.Insert(newFreeRange, aIndex + 2);
			__NK_ASSERT_ALWAYS(KErrNone == r);
			}

		iRangeList[aIndex].iLength = aAddress - iRangeList[aIndex].iStart;
		}
	//calculate invariant: run through array to check that ranges are contiguous
	__NK_ASSERT_DEBUG(InvariantCheck());
	}

#if defined (_DEBUG)
void TAddressAllocator::Print()
	{
	__KTRACE_OPT(KPCI, Kern::Printf("TAddressAllocator::Print(): Printing ranges"));
	const TInt count = iRangeList.Count();
	for(TInt i=0; i<count; ++i)
		{
		const TRange& r(iRangeList[i]);
		__KTRACE_OPT(KPCI, Kern::Printf("  index:%d, start:%x, length:%x, end:%x, allocated:%d",
				i, r.iStart, r.iLength, r.End(), r.iAllocated));
		}
	}

TBool TAddressAllocator::InvariantCheck()
	{
	//Print();
	const TInt count = iRangeList.Count();
	__NK_ASSERT_ALWAYS(count>0);
	__NK_ASSERT_ALWAYS(iRangeList[0].iStart==0);
	__NK_ASSERT_ALWAYS(iRangeList[count-1].End()==iMaxSize-1);

	for(TInt i=1; i<count; ++i)
		{
		const TRange& prev = iRangeList[i-1];
		const TRange& curr = iRangeList[i];
		__NK_ASSERT_ALWAYS(prev.End()==curr.iStart-1);
		__NK_ASSERT_ALWAYS(curr.iLength>0);

		//check that free spaces are always consolidated
		if(!prev.iAllocated)
			{
			__NK_ASSERT_ALWAYS(curr.iAllocated);
			}
		}
	return ETrue;
	}
#endif

TInt TAddressAllocator::TRange::OrderByStart(const TRange& aKeyRange, const TRange& aRange)
	{
	if(aKeyRange.iStart > aRange.iStart)
		return 1;
	else if(aKeyRange.iStart < aRange.iStart)
		return -1;
	else
		return 0;
	}

/**
Make a region of PCI memory available again.

@param aAddress Start address of an allocated PCI range to deallocate.
@return
	- KErrNone If the region was deallocated
	- KErrNotFound if aAddress is not the beginning of a previously
	  allocated range.
*/
TInt TAddressAllocator::DeAllocate(TLinAddr aAddress)
	{
	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	TInt r=KErrNone;

	TRange key(aAddress, 0, EFalse);
	TLinearOrder<TRange> startOrder(TRange::OrderByStart);
	TInt index = iRangeList.FindInOrder(key, startOrder);

	if(index==KErrNotFound || (!iRangeList[index].iAllocated))
		{
		r=KErrNotFound;
		}
	else
		{
		Remove(index);
		}

	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();
	return r;
	}

/**
Remove the allocated region and adjust remaining regions
to consolidate free space.
*/
void TAddressAllocator::Remove(TInt aIndex)
	{
	__KTRACE_OPT(KPCI, Kern::Printf("TAddressAllocator::Remove(): Removing %d (0x%X) bytes at 0x%X", 
		iRangeList[aIndex].iLength, iRangeList[aIndex].iLength, iRangeList[aIndex].iStart));
	__NK_ASSERT_DEBUG(InvariantCheck());
	const TInt count = iRangeList.Count();
	//range is above an unallocated range
	if(aIndex > 0 && (!iRangeList[aIndex - 1].iAllocated) )
		{
		//range is below unallocated range
		if(aIndex < (count - 1) && !(iRangeList[aIndex + 1].iAllocated))
			{
			iRangeList[aIndex - 1].iLength = iRangeList[aIndex + 1].End() + 1 -iRangeList[aIndex - 1].iStart;
			iRangeList.Remove(aIndex);
			iRangeList.Remove(aIndex);
#ifdef _DEBUG
			//The PCI kernel extension contains several persistent dynamic arrays and by default these RArrays 
			//do not free heap for every item that is removed. In order to ensure that  kernel heap checking 
			//succeeds  Compress() calls are needed to force this excess memory to be freed.  This calls are only
			//made in UDEB builds only as KHEAP checks are only performed in UDEB mode.
			iRangeList.Compress();
#endif
			}
		else
			{
			iRangeList[aIndex - 1].iLength = iRangeList[aIndex].End() + 1 - iRangeList[aIndex - 1].iStart;
			iRangeList.Remove(aIndex);
#ifdef _DEBUG			
			//The PCI kernel extension contains several persistent dynamic arrays and by default these RArrays 
			//do not free heap for every item that is removed. In order to ensure that  kernel heap checking 
			//succeeds  Compress() calls are needed to force this excess memory to be freed.  This calls are only
			//made in UDEB builds only as KHEAP checks are only performed in UDEB mode.
			iRangeList.Compress();
#endif
			}
		}
	//range is above allocated range
	else
		{
		//range is below unallocated range
		if(aIndex < (count - 1) && !(iRangeList[aIndex + 1].iAllocated))
			{
			iRangeList[aIndex].iAllocated=EFalse;
			iRangeList[aIndex].iLength=iRangeList[aIndex + 1].End() + 1 - iRangeList[aIndex].iStart;
			iRangeList.Remove(aIndex + 1);
#ifdef _DEBUG
			//The PCI kernel extension contains several persistent dynamic arrays and by default these RArrays 
			//do not free heap for every item that is removed. In order to ensure that  kernel heap checking 
			//succeeds  Compress() calls are needed to force this excess memory to be freed.  This calls are only
			//made in UDEB builds only as KHEAP checks are only performed in UDEB mode.
			iRangeList.Compress();
#endif
			}
		else
			{
			iRangeList[aIndex].iAllocated = EFalse;
			}
		}
	//calculate invariant: run through array to check that ranges are contiguous
	__NK_ASSERT_DEBUG(InvariantCheck());
	}

