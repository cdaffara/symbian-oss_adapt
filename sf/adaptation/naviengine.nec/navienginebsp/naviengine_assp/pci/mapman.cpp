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
#include <naviengine_priv.h>

//
// Global helper functions
//


/**
Integer log base 2 for power of 2
*/
TUint32 Log2(TUint32 aNumber)
	{
	__NK_ASSERT_ALWAYS(aNumber>0);
	__NK_ASSERT_DEBUG((aNumber&(aNumber-1))==0); //assert that size is power of 2
	const TUint32 b[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000};

	TUint32 r = (aNumber & b[0]) != 0;
	r |= ((aNumber & b[4]) != 0) << 4;
	r |= ((aNumber & b[3]) != 0) << 3;
	r |= ((aNumber & b[2]) != 0) << 2;
	r |= ((aNumber & b[1]) != 0) << 1;
	return r;
	}

/**
Ceiling power 2. Round up aNumber to the next power of 2.
*/
TUint32 Clp2(TUint32 aNumber)
	{
	aNumber -= 1;
	aNumber |= aNumber >> 1;
	aNumber |= aNumber >> 2;
	aNumber |= aNumber >> 4;
	aNumber |= aNumber >> 8;
	aNumber |= aNumber >> 16;
	return aNumber+1;
	}


//
// TMappingManager
//

/**
Just make sure members are zeroed
*/
TMappingManager::TMapping::TMapping()
	:iPciAddr(NULL), iPhysAddr(NULL), iUsed(EFalse)
	{}

/**
A compare function so that we can find used/unused mappings in RArray.
*/
TBool TMappingManager::TMapping::UsedCompare(const TMapping& aKeyMapping, const TMapping& aMapping)
	{
	return aKeyMapping.iUsed==aMapping.iUsed;
	}

/**
A compare function so that we can find mappings to specific phys addr in RArray.
*/
TBool TMappingManager::TMapping::PhysAddrCompare(const TUint32* aPhysAddr, const TMapping& aMapping)
	{
	return *aPhysAddr==aMapping.iPhysAddr;
	}

TBool TMappingManager::TMapping::ContainsPhysical(const TUint32* aPhysAddr, const TMapping& aMapping)
	{
	return aMapping.iUsed && Rng(aMapping.iPhysAddr, *aPhysAddr, aMapping.iPhysAddr+aMapping.iSize-1);
	}

TBool TMappingManager::TMapping::ContainsPci(const TUint32* aPciAddr, const TMapping& aMapping)
	{
	return aMapping.iUsed && Rng(aMapping.iPciAddr, *aPciAddr, aMapping.iPciAddr+aMapping.iSize-1);
	}

TUint32 TMappingManager::TMapping::PciAddress(TUint32 aPhysAddress)
	{
	__NK_ASSERT_ALWAYS(Rng(iPhysAddr, aPhysAddress, iPhysAddr+iSize-1));
	return iPciAddr+(aPhysAddress-iPhysAddr);
	}

TUint32 TMappingManager::TMapping::PhysAddress(TUint32 aPciAddress)
	{
	__NK_ASSERT_ALWAYS(Rng(iPciAddr, aPciAddress, iPciAddr+iSize-1));
	return iPhysAddr+(aPciAddress-iPciAddr);
	}

TMappingManager::TMappingManager(TInt aNumOfBars, TAddressAllocator& aAllocator, TUint32 aBaseAddress)
	: iAllocator(aAllocator), iNumOfBars(aNumOfBars), iBaseAddress(aBaseAddress), iMappings(iNumOfBars), iMutex(NULL)
	{
	_LIT(KChunkManMutex, "PCI_Map_Man_Mutex");
	TInt r=Kern::MutexCreate(iMutex, KChunkManMutex, KMutexOrdGeneral1);
	__NK_ASSERT_ALWAYS(KErrNone==r);

	TMapping emptyRecord;
	for(TInt i=0; i<iNumOfBars; ++i)
		{
		r = iMappings.Append(emptyRecord);
		__NK_ASSERT_ALWAYS(r==KErrNone);
		}
	}

TMappingManager::~TMappingManager()
	{
	iMutex->Close(NULL);
	iMappings.Reset();
	}

/**
Obtain a block of memory from PCI memory space, and map it to the supplied aPhysicalAddress.
@param aPhysicalAddress The system memory address to which window points.
Must be naturally aligned in accord with aSize. Eg. a 1MB memory block must be alligned to a 1MB boundary.
@param aSize Size of mapping. Must be a power of 2 and greater than 1KB.
@param On return Will be set to the PCI address of the window.
@return
	- KErrNone Success
	- KErrNotSupported aPhysicalAddress was not aligned correctly or aSize was not a power of 2
*/
TInt TMappingManager::CreateMapping(TUint32 aPhysicalAddress, TInt aSize, TUint32& aPciAddress)
	{
	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	if(aSize<KMinOutboundWindow)
		aSize=KMinOutboundWindow;
	else
		aSize=Clp2(aSize); //round up to next Power of 2.

	__NK_ASSERT_ALWAYS(aSize>=KMinOutboundWindow); //assert that size is greater than minimum window
	__NK_ASSERT_ALWAYS((aSize&(aSize-1))==0); //assert that size is power of 2

	TInt r=KErrNone;
	const TMapping unusedKey;
	const TInt barIndex = iMappings.Find(unusedKey, TIdentityRelation<TMapping>(TMapping::UsedCompare));
	if(KErrNotFound==barIndex)
		{
		r=barIndex;
		goto End;
		}

	//ensure that aPhysicalAddress is aligned correctly for the
	//window size
	if(aPhysicalAddress&(aSize-1))
		{
		r=KErrNotSupported;
		goto End;
		}

	r = iAllocator.Allocate(aPciAddress, aSize);
	if(r!=KErrNone)
		{
		goto End;
		}

	DoCreateMapping(barIndex, aPhysicalAddress, aSize, aPciAddress);

End:
	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();
	return r;
	}

void TMappingManager::DoCreateMapping(TInt aBarIndex, TUint32 aPhysicalAddress, TInt& aSize, TUint32& aPciAddress)
	{
	__KTRACE_OPT(KPCI, Kern::Printf("Create PCI window mapping: phys=0x%08x, pci=0x%08x, size=0x%08x, BarIndex=%d ", aPhysicalAddress, aPciAddress, aSize, aBarIndex));
	//The bar mask field in the
	//ACR determines how many of the upper bits
	//of an accessed PCI address should be converted
	//before being forwarded to AHB.
	//This is equivalent to 32-log2(size)
	const TUint32 log2Size=Log2(aSize);
	const TUint32 acrBarMask = ((32-log2Size)<<4)&KHmAcr_BarMask;
	const TUint32 acrValue = aPhysicalAddress|acrBarMask|KHtAcr_P2Ace;
	const TUint32 barValue = aPciAddress;
	__KTRACE_OPT(KPCI, Kern::Printf(" calculated bar mask = 0x%08x", acrBarMask));
	__KTRACE_OPT(KPCI, Kern::Printf(" writing acrValue = 0x%08x @ os 0x%08x", acrValue, KHoAcr[aBarIndex]));
	__KTRACE_OPT(KPCI, Kern::Printf(" writing barValue = 0x%08x @ os 0x%08x", barValue, KHoBar[aBarIndex]));

	AsspRegister::Write32(iBaseAddress+KHoAcr[aBarIndex],acrValue);
	AsspRegister::Write32(iBaseAddress+KHoBar[aBarIndex],barValue);

	//set bit for this bar in the bar-enable register
	AsspRegister::Modify32(iBaseAddress+KHoBarEnable, NULL, (1<<aBarIndex));
	//__KTRACE_OPT(KPCI, Kern::Printf("New bar enable register: 0x%08x", AsspRegister::Read32(iBaseAddress+KHoBarEnable)));

	TMapping& newMapping = iMappings[aBarIndex];
	newMapping.iPciAddr=aPciAddress;
	newMapping.iPhysAddr=aPhysicalAddress;
	newMapping.iSize=aSize;
	newMapping.iUsed=ETrue;
	}

TInt TMappingManager::RemoveMapping(TUint32 aPhysicalAddress)
	{
	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	TInt r=KErrNone;
	TInt barIndex=iMappings.Find(aPhysicalAddress, TMapping::PhysAddrCompare);

	if(KErrNotFound==barIndex)
		{
		r=barIndex;
		}
	else
		{
		//clear the bits in the BarMask field of the ACR to invalidate access to Bar.
		AsspRegister::Modify32(iBaseAddress+KHoAcr[barIndex], KHmAcr_BarMask, NULL);

		//clear bit for this bar in the bar enable register
		AsspRegister::Modify32(iBaseAddress+KHoBarEnable, (1<<barIndex), NULL);

		TMapping& oldMapping = iMappings[barIndex];
		r = iAllocator.DeAllocate(oldMapping.iPciAddr);
		__NK_ASSERT_ALWAYS(KErrNone==r);

		oldMapping.iUsed=EFalse;
		}

	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();
	return r;
	}

TInt TMappingManager::GetPciAddress(TUint32 aPhysicalAddress, TUint32& aPciAddress)
	{
	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	const TInt barIndex=iMappings.Find(aPhysicalAddress, TMapping::ContainsPhysical);
	TInt r= KErrNotFound;

	if(barIndex==KErrNotFound)
		r = KErrNotFound;
	else
		{
		aPciAddress=iMappings[barIndex].PciAddress(aPhysicalAddress);
		r = KErrNone;
		}

	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();
	return r;
	}

TInt TMappingManager::GetPhysicalAddress(TUint32 aPciAddress, TUint32& aPhysicalAddress)
	{
	NKern::ThreadEnterCS();
	Kern::MutexWait(*iMutex);

	const TInt barIndex=iMappings.Find(aPciAddress, TMapping::ContainsPci);
	TInt r= KErrNotFound;

	if(barIndex==KErrNotFound)
		r = KErrNotFound;
	else
		{
		aPhysicalAddress=iMappings[barIndex].PhysAddress(aPciAddress);
		r = KErrNone;
		}

	Kern::MutexSignal(*iMutex);
	NKern::ThreadLeaveCS();
	return r;
	}

