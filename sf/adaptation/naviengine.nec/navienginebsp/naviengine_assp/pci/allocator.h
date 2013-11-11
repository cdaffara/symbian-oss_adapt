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



#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <e32cmn.h>

class DMutex;

/**
Keeps track of what address ranges are available
Address ranges will be size aligned. ie. 4k range will be aligned to 4k
boundary, 1k range will be aligned to 1k boundary
*/
class TAddressAllocator
	{
public:
	TAddressAllocator(TLinAddr aMaxSize);
	~TAddressAllocator();
	TInt Allocate(TLinAddr& aAddress, TLinAddr aSize);
	TInt DeAllocate(TLinAddr aAddress);
private:
	/**
	A contiguous address region.
	*/
	struct TRange
		{
		TRange(TLinAddr aStart, TLinAddr aLength, TBool aAllocated)
			:iAllocated(aAllocated), iStart(aStart), iLength(aLength)
			{
			}
		inline TLinAddr End() const
			{ return iStart+iLength-1;}

		static TInt OrderByStart(const TRange& aKeyRange, const TRange& aRange);

		TBool iAllocated;
		TLinAddr iStart;
		TLinAddr iLength;
		};

	void DoAllocate(TLinAddr aAddress, TLinAddr aSize, TInt aIndex);
	void Remove(TInt aIndex);
#if defined (_DEBUG)
	void Print();
	TBool InvariantCheck();
#endif


	const TLinAddr iMaxSize;
	RArray<TRange> iRangeList; //a list of contiguous, non-overlapping address regions.
	DMutex* iMutex;
	};

#endif //ALLOCATOR_H
