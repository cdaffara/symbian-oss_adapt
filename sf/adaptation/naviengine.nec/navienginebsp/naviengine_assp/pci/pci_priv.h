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

#ifndef _PCI_PRIV_H
#define _PCI_PRIV_H

#include <kernel/kernel.h>

/**
An object of this type will be passed in to the chunk cleanup
callback.
A concrete class will be derived by the PSL as the exact information
needed in order to cleanup may vary.
*/
class TChunkCleanup : public TDfc
	{
public:
	inline TChunkCleanup();
	inline virtual ~TChunkCleanup()
		{}
	inline virtual void Destroy()=0; 
private:
	inline static void ChunkDestroyed(TChunkCleanup* aCleanup);
	};


TChunkCleanup::TChunkCleanup()
	:TDfc(reinterpret_cast<TDfcFn>(ChunkDestroyed), this, Kern::DfcQue0(), 0)
	{
	}

/**
The static function called by the DFC
*/
void TChunkCleanup::ChunkDestroyed(TChunkCleanup* aCleanup)
	{
	Pci::ChunkCleanupCallback(aCleanup); 
	}

/**
Interface to PCI Host Bridge
*/
class DPciBridge : public DBase
	{
public:
	~DPciBridge();
	TInt Register();

	virtual TInt Initialise() =0;
	virtual TPciFunction* Function(TInt aBus, TInt aDevice, TInt aFunction) =0;
	virtual TInt CreateChunk(DPlatChunkHw*& aChunk, TInt aSize, TUint aAttributes, TUint32& aPciAddress)=0;
	virtual TInt CreateChunk(DChunk*& aChunk, TChunkCreateInfo &aAttributes, TUint aOffset, TUint aSize, TUint32& aPciAddress)=0;
	virtual TInt RemoveChunk(DPlatChunkHw* aChunk)=0;
	virtual TInt CreateMapping(TUint32 aPhysicalAddress, TInt aSize, TUint32& aPciAddress)=0;
	virtual TInt RemoveMapping(TUint32 aPhysicalAddress)=0;
	virtual TInt GetPciAddress(TUint32 aPhysicalAddress, TUint32& aPciAddress)=0;
	virtual TInt GetPhysicalAddress(TUint32 aPciAddress, TUint32& aPhysicalAddress)=0;

	virtual void ConfigurationComplete()=0;

	virtual TUint8 ReadConfig8(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset) const =0;
	virtual TUint16 ReadConfig16(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset) const =0;
	virtual TUint32 ReadConfig32(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset) const =0;

	virtual void WriteConfig8(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint8 aValue)=0;
	virtual void WriteConfig16(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint16 aValue)=0;
	virtual void WriteConfig32(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint32 aValue)=0;

	virtual void ModifyConfig8(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint8 aClearMask, TUint8 aSetMask)=0;
	virtual void ModifyConfig16(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint16 aClearMask, TUint16 aSetMask)=0;
	virtual void ModifyConfig32(TInt aBus, TInt aDevice, TInt aFunction, TUint aOffset, TUint32 aClearMask, TUint32 aSetMask)=0;
protected:
	DPciBridge();
	};


/**
Gives access to one of a PCI functions's memory spaces
*/
class TMemorySpace : public TAddrSpace
	{
public:
	TMemorySpace(TUint32 aBaseAddress, TUint aSize);

	READ(8);
	READ(16);
	READ(32);

	WRITE(8);
	WRITE(16);
	WRITE(32);

	MODIFY(8);
	MODIFY(16);
	MODIFY(32);
private:
	TUint32 iBaseAddress;
	};

class TPciFunction;
/**
Access to a PCI function's config space
*/
class TConfigSpace : public TAddrSpace
	{
public:
	TConfigSpace(TPciFunction& aFunction, DPciBridge& aBridge);

	READ(8);
	READ(16);
	READ(32);

	WRITE(8);
	WRITE(16);
	WRITE(32);

	MODIFY(8);
	MODIFY(16);
	MODIFY(32);

private:
	TPciFunction& iFunction; //!< The function which owns this config space
	DPciBridge& iBridge;	//!< The Host bridge which iFunction is on
	};

/**
Represents a single Function on a (possibly multi-function) PCI device.
*/
class TPciFunction
	{
public:
	TPciFunction(TInt aBus, TInt aDevice, TInt aFunction, TPciVendorId aVid, TDeviceId aDid, DPciBridge& aBridge);
	~TPciFunction();

	TInt AddMemorySpace(TUint32 aSize, TUint32 aAddress, TInt aBarIndex);
	IMPORT_C TAddrSpace* GetMemorySpace(TInt aBar=0);
	IMPORT_C TAddrSpace* GetConfigSpace();

	inline TInt Bus() {return iBus;}
	inline TInt Device() {return iDevice;}
	inline TInt Function() {return iFunction;}
	inline TPciVendorId VendorId() {return iVid;}
	inline TDeviceId DeviceId() {return iDid;}

	inline DPciBridge& GetBridge() {return iBridge;}

	inline TBool IsBridge() {return iIsBridge;}
	inline TBool IsMultiFunc() {return iIsMultiFunc;}

protected:
	const TInt iBus;
	const TInt iDevice;
	const TInt iFunction;

	const TPciVendorId iVid;
	const TDeviceId iDid;

	DPciBridge& iBridge;		//!< The Host bridge which this TPciFunction is on
	TAddrSpace* iConfigSpace;
	RPointerArray<TMemorySpace> iMemorySpaces;
	TBool iIsBridge;
	TBool iIsMultiFunc;
	};


#endif //_PCI_PRIV_H
