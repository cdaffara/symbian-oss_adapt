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



#ifndef __MAPMAN_H__
#define __MAPMAN_H__

#include <e32cmn.h>

//helper maths functions
TUint32 Clp2(TUint32 aNumber);
TUint32 Log2(TUint32 aNumber);

class TAddressAllocator;
class DMutex;

/**
Responsible for creating outbound mappings so that
PCI peripherals can access system memory.
*/
class TMappingManager
	{
public:
	TMappingManager(TInt aNumOfBars, TAddressAllocator& aAllocator, TUint32 aBaseAddress);
	~TMappingManager();
	TInt CreateMapping(TUint32 aPhysicalAddress, TInt aSize, TUint32& aPciAddress);
	TInt RemoveMapping(TUint32 aPhysicalAddress);

	TInt GetPciAddress(TUint32 aPhysicalAddress, TUint32& aPciAddress);
	TInt GetPhysicalAddress(TUint32 aPciAddress, TUint32& aPhysicalAddress);

private:
	/**
	Associates BAR with an allocated PCI address
	*/
	struct TMapping
		{
		TMapping();
		static TBool UsedCompare(const TMapping& aKeyMapping, const TMapping& aMapping);
		static TBool PhysAddrCompare(const TUint32*, const TMapping& aMapping);
		static TBool ContainsPhysical(const TUint32*, const TMapping& aMapping);
		static TBool ContainsPci(const TUint32*, const TMapping& aMapping);

		inline TUint32 PciAddress(TUint32 aPhysAddress);
		inline TUint32 PhysAddress(TUint32 aPciAddress);

		TUint32 iPciAddr;
		TUint32 iPhysAddr;
		TUint32 iSize;
		TBool iUsed;
		};

	void DoCreateMapping(TInt aBarIndex, TUint32 aPhysicalAddress, TInt& aSize, TUint32& aPciAddress);

	TAddressAllocator& iAllocator;
	const TInt iNumOfBars;
	const TUint32 iBaseAddress;

	RArray<TMapping> iMappings;
	DMutex* iMutex;
	};
#endif // __MAPMAN_H__
