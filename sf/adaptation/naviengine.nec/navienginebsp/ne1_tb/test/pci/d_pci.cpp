/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Device-driver for naviEngine PCI testing 
*
*/


#include <kernel/kernel.h>
#include <kernel/kern_priv.h>
#include <pci.h>
#include <naviengine.h>
#include <kernel/cache.h>
#include "allocator.h"
#include "pci-ne.h"
#include "pci_priv.h"
#include <platform.h>
#include "t_pci.h"
#include "../../naviengine_assp/naviengine_pci.h"

#define TEST(X) __NK_ASSERT_ALWAYS(X)
#define TEST_KERRNONE(X) if((X) !=KErrNone) {\
								  Kern::Printf("Assertion Failed X=%d", (X)); FAULT();}

#define FUNC_LOG() __KTRACE_OPT(KPCI, Kern::Printf(__PRETTY_FUNCTION__))

 
void TestAllocator();

/**
So that the test app can get notification
when PCI DChunks' cleanup operation has run
we will replace their cleanup object with this
one, which will call the original object's destroy
function as well completing a notification
*/
class TPciCleanupWrapper : public TChunkCleanup
	{
public:

	~TPciCleanupWrapper()
		{
		delete iOriginal;
		Kern::DestroyClientRequest(iClientRequest);
		iClient->Close(NULL);
		}

	static TPciCleanupWrapper* Create(TRequestStatus* aRequestStatus)
		{
		__KTRACE_OPT(KPCI, Kern::Printf("TPciCleanupWrapper::Create aRequestStatus=0x%08x", aRequestStatus));
		TClientRequest* request = NULL;
		TInt r = Kern::CreateClientRequest(request);
		if(r != KErrNone)
			{
			__KTRACE_OPT(KPCI, Kern::Printf("TPciCleanupWrapper::Create Failed to create client request r=%d", r));
			return NULL;
			}

		r = request->SetStatus(aRequestStatus);
		if(r != KErrNone)
			{
			__KTRACE_OPT(KPCI, Kern::Printf("TPciCleanupWrapper::Create Failed to set status r=%d", r));
			Kern::DestroyClientRequest(request);
			return NULL;
			}

		return new TPciCleanupWrapper(request);
		}

	/**
	Insert the cleanup object into aChunk, remembering
	the original one

	@param aChunk a chunk known to have TChunkCleanup derived cleanup object
	*/
	void Insert(DChunk* aChunk)
		{
		__KTRACE_OPT(KPCI, Kern::Printf("TPciCleanupWrapper::Insert aChunk=0x%08x", aChunk));
		__NK_ASSERT_DEBUG(aChunk);
		__KTRACE_OPT(KPCI, Kern::Printf("TPciCleanupWrapper replace 0x%08x with 0x%08x", aChunk->iDestroyedDfc, this));
		iOriginal = static_cast<TChunkCleanup*>(aChunk->iDestroyedDfc);

		__NK_ASSERT_DEBUG(iOriginal);
		aChunk->iDestroyedDfc = this;
		}

	/**
	Run the original object's destroy method
	then notify client
	*/
	void Destroy()
		{
		__KTRACE_OPT(KPCI, Kern::Printf("TPciCleanupWrapper::Destroy\n"));
		iOriginal->Destroy();

		__NK_ASSERT_ALWAYS(iClientRequest->IsReady());
		Kern::QueueRequestComplete(iClient, iClientRequest, KErrNone);
		}

private:
	TPciCleanupWrapper(TClientRequest* aRequest)
		:TChunkCleanup(), iOriginal(NULL), iClientRequest(aRequest), iClient(&Kern::CurrentThread())
		{
		__ASSERT_CRITICAL;
		iClient->Open(); //don't allow thread object to be destroyed before we signal
		}


	TChunkCleanup* iOriginal;
	TClientRequest* iClientRequest;
	DThread* const iClient;
	};




//This information will come from the pci driver
//if this code is ever made generic
TPciTestInfo KTestInfo =
	{
	TPciDevice(0x1033, 0x35, 0),
	TPciTestInfo::TAddrSpaceTest(0, 0x00351033, 0),
	TPciTestInfo::TAddrSpaceTest(KPciBar0, 0, 0xFFF),
	0,
	TPciTestInfo::TAddrSpaceTest(0x34, 0x2EDF, 0),
	TPciTestInfo::TAddrSpaceTest(0x20, 0, 0xF),
	KNeBridgeNumberOfBars
	};

/**
Class for a DChunk to remove a DPlatHwChunk chunk
*/
class TPciPlatChunkCleanup : public TChunkCleanup
	{
public:
	TPciPlatChunkCleanup(TInt aPciFunction, DPlatChunkHw* aPciPlatChunk);
	virtual void Destroy();
public:
	TInt iPciFunction;
	DPlatChunkHw* iPciChunk;
	};

TPciPlatChunkCleanup::TPciPlatChunkCleanup(TInt aPciFunction,  DPlatChunkHw* aPciPlatChunk)
	: TChunkCleanup(), iPciFunction(aPciFunction), iPciChunk(aPciPlatChunk)
	{
	}

void TPciPlatChunkCleanup::Destroy()
	{	
	__KTRACE_OPT(KPCI, Kern::Printf("SHAREDCHUNK ChunkDestroyed DFC\n"));
	TInt r = Pci::RemoveChunk(iPciFunction, iPciChunk);
	__NK_ASSERT_ALWAYS(r==KErrNone);
	}

/**
Cleanup class to remove the mapping for an externally
mapped chunk
*/
class TPciMappedChunkCleanup : public TChunkCleanup
	{
public:
	TPciMappedChunkCleanup (TInt aPciFunction,TUint32 aPhysicalAddress);
	virtual void Destroy();
public:
	TInt iPciFunction;
	TUint32 iPhysicalAddress;
	};

TPciMappedChunkCleanup::TPciMappedChunkCleanup(TInt aPciFunction,TUint32 aPhysicalAddress)
	: TChunkCleanup(), iPciFunction(aPciFunction),iPhysicalAddress(aPhysicalAddress)
	{	
	}

void TPciMappedChunkCleanup::Destroy()
	{	
	//remove mapping
	TInt r = Pci::RemoveMapping(iPciFunction, iPhysicalAddress);
	__NK_ASSERT_ALWAYS(r==KErrNone);	
	__KTRACE_OPT(KPCI, Kern::Printf("MAPPING REMOVED ChunkDestroyed DFC\n"));
	}

class DPciTestChannel : public DLogicalChannelBase
	{
public:
	DPciTestChannel();
	virtual ~DPciTestChannel();
	TInt DoCreate(TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);
	virtual TInt Request(TInt aFunction, TAny* a1, TAny* a2);

private:
	TInt OpenPciDChunk(TUint32& aPciAddr,TInt aPciChunkSize, TRequestStatus* aStatus);		
	TInt OpenPciPlatHwChunk(TUint32& aPciAddr,TInt aPciChunkSize, TRequestStatus* aStatus);	
	TInt OpenPciMappedChunk(TUint32& aPciAddr,TInt aPciChunkSize, TRequestStatus* aStatus);	 
	TInt CreateSharedChunk(TInt aPciChunkSize, TUint32 aAttributes, DChunk*& aChunk, TLinAddr& aVirt, TPhysAddr& aPhysicalAddress);
	TInt OpenPciWindowChunk();
	void RunUnitTests();

private:
	const TPciTestInfo& iTestInfo;
	TInt iFunction; ///< PCI function number this channel is associated with
	};

DPciTestChannel::DPciTestChannel()
	: iTestInfo(KTestInfo), iFunction(-1)
	{
	FUNC_LOG();
	}

DPciTestChannel::~DPciTestChannel()
	{
	FUNC_LOG();
	}

TInt DPciTestChannel::DoCreate(TInt aUnit, const TDesC8* aInfo, const TVersion& aVer)
	{
	if(aInfo == NULL)
		return KErrNone; //Not a device specific channel

	TPciDevice dev;
	TPckg<TPciDevice> devPckg(dev);
	TInt r = Kern::ThreadDesRead(&Kern::CurrentThread(), aInfo, devPckg, 0, KChunkShiftBy0);
	if(r != KErrNone)
		return r;

	NKern::ThreadEnterCS();
	RArray<TInt> indicies;
	r = Pci::Probe(indicies, dev.iVendorId, dev.iDeviceId);
	
	if((KErrNone == r) && (dev.iInstance < indicies.Count()))
		{
		iFunction = indicies[dev.iInstance];
		}
	else
		{
		r = KErrNotFound;
		}
	
	indicies.Close();
	NKern::ThreadLeaveCS();
	return r;
	}

TInt DPciTestChannel::Request(TInt aFunction, TAny* a1, TAny* a2)
	{
	switch (aFunction)
		{
		case EGetTestInfo:
			{
			TDes8& dest(*(TDes8*)a1);
			TPckgC<TPciTestInfo> info(iTestInfo);
			Kern::KUDesPut(dest, info);
			return KErrNone;
			}
		case EAccessConfigSpace:
			{
			TPckgBuf<TUserConfigSpace> pckg;
			Kern::KUDesGet(pckg, *reinterpret_cast<TDes8*>(a1));
		
			TAddrSpace* configSpace = Pci::GetConfigSpace(iFunction);
			if(configSpace == NULL)
				{
				Kern::PanicCurrentThread(KPciTest, KErrGeneral);
				return KErrGeneral;
				}
			return pckg().KRun(*configSpace);
			}
		case EAccessMemorySpace:
			{
			TPckgBuf<TUserMemorySpace> pckg;
			Kern::KUDesGet(pckg, *reinterpret_cast<TDes8*>(a1));

			TAddrSpace* memSpace = Pci::GetMemorySpace(iFunction, pckg().BarIndex());
			if(memSpace == NULL)
				{
				Kern::PanicCurrentThread(KPciTest, KErrGeneral);
				return KErrGeneral;
				}
			return pckg().KRun(*memSpace);
			}
		case EOpenPciWindowChunk:
			{				
			TInt rHandle = 0;				
			rHandle = OpenPciWindowChunk();		
			return rHandle;			
			}
		case EOpenPciDChunk: //Fall-through
		case EOpenPciPlatHwChunk:
		case EOpenPciMappedChunk:
			{				
				TPckgBuf<TPciChunkCreateInfo> pckg;				
				Kern::KUDesGet(pckg, *reinterpret_cast<TDes8*>(a1));

				TUint32 pciAddr;				
				TInt rHandle = 0;				
				switch (aFunction)
					{
				case EOpenPciDChunk:
						{						
						rHandle = OpenPciDChunk(pciAddr, pckg().iSize, pckg().iStatus);						
						break;
						}
				case EOpenPciPlatHwChunk:
						{						
						rHandle = OpenPciPlatHwChunk(pciAddr, pckg().iSize, pckg().iStatus);				
						break;
						}
				case EOpenPciMappedChunk:
						{
						rHandle = OpenPciMappedChunk(pciAddr, pckg().iSize, pckg().iStatus);					
						break;
						}
				default:
						{
						FAULT();
						}
					}
				//write back PCI address to user
				umemput(pckg().iPciAddress,&pciAddr,sizeof(pciAddr));				
				return rHandle;			
			}
		case ERunUnitTests:
			{					
			RunUnitTests();					
			return KErrNone;		
			}
		default:
			return KErrNotSupported;
		}
	}

/**
This function runs tests for the address allocator
*/
void DPciTestChannel::RunUnitTests()
{
	// Enter critical section 
   	NKern::ThreadEnterCS();		
	
	TestAllocator();

 	// Finished
	NKern::ThreadLeaveCS();
}


/**
This function creates and opens a PCI DChunk and returns the PCI addresss
@param aPciAddr on return contains the pci address
@param aPciChunkSize contains the size of the PCI DChunk which is to be created
*/
TInt DPciTestChannel::OpenPciDChunk(TUint32& aPciAddr,TInt aPciChunkSize, TRequestStatus* aStatus)
{
	//Chunk Attributes 
	TChunkCreateInfo aInfo;
	aInfo.iType = TChunkCreateInfo::ESharedKernelMultiple;
	aInfo.iMapAttr = EMapAttrSupRw|EMapAttrFullyBlocking;
	aInfo.iOwnsMemory = EFalse; // We'll be using our own devices memory
	
	DChunk* pciChunk;
	pciChunk=NULL;	

    // Enter critical section 
	NKern::ThreadEnterCS();
	
	//Create DChunk
	TInt r = Pci::CreateChunk(iFunction, pciChunk, aInfo,0,aPciChunkSize,aPciAddr);
	if(r!=KErrNone)
	{
		// Failed to create DChunk	
		__KTRACE_OPT(KPCI,Kern::Printf("Failed to create DChunk: Error code is=%d", r) );
		NKern::ThreadLeaveCS(); // Finished
		return r;		
	}
	else
	{
		TInt rHandle = KErrGeneral;
		if(aStatus)
			{
			TPciCleanupWrapper* wrapper = TPciCleanupWrapper::Create(aStatus);
			if(wrapper == NULL)
				{
				__KTRACE_OPT(KPCI,Kern::Printf("Creation of TPciCleanupWrapper failed"));
				goto End;
				}
			wrapper->Insert(pciChunk);
			}

		__KTRACE_OPT(KPCI,Kern::Printf("Created DChunk: PCI_ADDRESS=0x%08x",aPciAddr));
		rHandle = Kern::MakeHandleAndOpen(NULL, pciChunk);//Get DChunk handle	

End:
		pciChunk->Close(NULL);		// Close DChunk		
		NKern::ThreadLeaveCS(); 	// Finished
		return rHandle;
	}
}

/**
This function creates and opens a PCI DPlatChunk and returns the PCI addresss.
A DPlatChunk is intially created and then a DChunk is set to point to the same 
memory as the DPlatChunk.This is done so that it can be accessed on the user side. 
@param aPciAddr on return contains the pci address
@param aPciChunkSize contains the size of the PCI PlatHwChunk which is to be created
*/
TInt DPciTestChannel::OpenPciPlatHwChunk(TUint32& aPciAddr,TInt aPciChunkSize, TRequestStatus* aStatus)
{
	TUint32 pciPhysicalAddr;
	TUint32 pciChunkMapAttr;	
	TLinAddr pciChunkKernelAddr;

	DPlatChunkHw* pciPlatChunk;
	pciPlatChunk=NULL;	

    // Enter critical section 
	NKern::ThreadEnterCS();
	
	//Create DPlatChunkHw
	TInt r = Pci::CreateChunk(iFunction,pciPlatChunk,aPciChunkSize,(EMapAttrSupRw|EMapAttrFullyBlocking),aPciAddr);
	if(r!=KErrNone)
	{
		// Failed to create DPlatChunkHw	
		__KTRACE_OPT(KPCI,Kern::Printf("Failed to create DPlatChunkHw chunk: Error code is=%d", r));
		NKern::ThreadLeaveCS(); // Finished
		return r;	
	}

	//Get physical addresss
	pciPhysicalAddr	= pciPlatChunk->PhysicalAddress();
	
	// Create DChunk cleanup object
	TPciPlatChunkCleanup* cleanup = new TPciPlatChunkCleanup(iFunction, pciPlatChunk);
	if(!cleanup)
    {
		pciPlatChunk->Close(NULL); //close pciPlatChunk
		NKern::ThreadLeaveCS();
		return KErrNoMemory;
    }
	
	//Chunk Attributes  for DChunk
	TChunkCreateInfo chunkinfo;
	chunkinfo.iType         = TChunkCreateInfo::ESharedKernelMultiple;    
	chunkinfo.iMaxSize      = 0x4000;
	chunkinfo.iMapAttr      = EMapAttrSupRw|EMapAttrFullyBlocking; // No caching
	chunkinfo.iOwnsMemory   = EFalse; // Use memory from system's free pool
	chunkinfo.iDestroyedDfc = cleanup;

	DChunk* pciDChunk;
  
	//Create DChunk
	r = Kern::ChunkCreate(chunkinfo, pciDChunk, pciChunkKernelAddr, pciChunkMapAttr);
	if(r!=KErrNone)
    {
		pciPlatChunk->Close(NULL); //close pciPlatChunk
		delete cleanup;
		NKern::ThreadLeaveCS();
		return r;
	}

	pciPlatChunk=NULL; // pciDChunk now owns chunk        	

	if(aStatus)
		{
		TPciCleanupWrapper* wrapper = TPciCleanupWrapper::Create(aStatus);
		if(wrapper == NULL)
			{
			pciDChunk->Close(NULL);	// Close pciDChunk	
			NKern::ThreadLeaveCS(); // Finished
			return KErrGeneral;
			}
		wrapper->Insert(pciDChunk);
		}

	//Commit memory to a DChunk using DPlatChunkHw physical address
	r = Kern::ChunkCommitPhysical(pciDChunk,0,aPciChunkSize,pciPhysicalAddr);
	if(r!=KErrNone)
	{
		// Failed to commit memory
		Kern::Printf("Commit failed: Error code is=%d", r);
		__KTRACE_OPT(KPCI,Kern::Printf("Commit failed: Error code is=%d", r));

		// Close chunk, which will then get deleted at some point
		Kern::ChunkClose(pciDChunk);
		NKern::ThreadLeaveCS();
		return r;
	}	
	
	//Close pciPlatChunk using pciDChunk as pciDChunk now owns it
	const TInt rHandle = Kern::MakeHandleAndOpen(NULL, pciDChunk); //Get DChunk handle
	pciDChunk->Close(NULL);	// Close pciDChunk	
	NKern::ThreadLeaveCS(); // Finished
	return rHandle;   	
}

/**
This function creates and opens a PCI mapped DChunk and returns the PCI addresss
@param aPciAddr on return contains the pci address
@param aPciChunkSize contains the size of the PCI DChunk which is to be created
*/
TInt DPciTestChannel::OpenPciMappedChunk(TUint32& aPciAddr,TInt aPciChunkSize, TRequestStatus* aStatus)
{
	TLinAddr virt=NULL;
	TPhysAddr physicalAddress=NULL;
	DChunk* pciChunk=NULL;
	TUint32 pciAttributes=EMapAttrSupRw|EMapAttrFullyBlocking;

	// Enter critical section 
	NKern::ThreadEnterCS();	

	//create DChunk
	TInt r = CreateSharedChunk(aPciChunkSize, pciAttributes, pciChunk, virt, physicalAddress);
	if(r!=KErrNone)
	{
	__KTRACE_OPT(KPCI,Kern::Printf("Create shared Chunk failed: Error code is=%d", r));
	return r;
	}
	
	__NK_ASSERT_ALWAYS(pciChunk);

	//create mapping
	r=Pci::CreateMapping(iFunction, physicalAddress, aPciChunkSize, aPciAddr);
	if(r!=KErrNone)
	{
	pciChunk->Close(NULL);
	__KTRACE_OPT(KPCI,Kern::Printf("Create mapping failed: Error code is=%d", r));
	return r;
	}	


	// Create DChunk cleanup object
	TPciMappedChunkCleanup* cleanup = new TPciMappedChunkCleanup(iFunction, physicalAddress);
	if(!cleanup)
		{
			pciChunk->Close(NULL);
			NKern::ThreadLeaveCS();
			return KErrNoMemory;
		}

	//must add the cleanup dfc to the chunk after creation
	//since the cleanup parameters aren't known
	//till after creating it and allocating memory to it
	pciChunk->iDestroyedDfc = cleanup;

	if(aStatus)
		{
		TPciCleanupWrapper* wrapper = TPciCleanupWrapper::Create(aStatus);
		if(wrapper == NULL)
			{
			pciChunk->Close(NULL);	// Close pciDChunk	
			NKern::ThreadLeaveCS();
			return KErrGeneral;
			}
		wrapper->Insert(pciChunk);
		}
	
	//Get DChunk handle
	const TInt rHandle = Kern::MakeHandleAndOpen(NULL, pciChunk);	
	
	// Close DChunk		
	pciChunk->Close(NULL);		    
	
	// Finished
	NKern::ThreadLeaveCS(); 	
	return rHandle;
}

/**
This function creates and opens a PCI Window Chunk and returns the PCI Window addresss
@param aPciChunkSize contains the size of the PCI Window DChunk which is to be created
*/
TInt DPciTestChannel::OpenPciWindowChunk()
{
	TUint32 pciChunkMapAttr;	
	TLinAddr pciChunkKernelAddr=NULL;
	DChunk* pciWindowChunk=NULL;	

	//Chunk Attributes  for DChunk
	TChunkCreateInfo chunkinfo;
	chunkinfo.iType         = TChunkCreateInfo::ESharedKernelMultiple;    
	chunkinfo.iMaxSize      = 0x2000;
	chunkinfo.iMapAttr      = EMapAttrSupRw|EMapAttrFullyBlocking; // No caching
	chunkinfo.iOwnsMemory   = EFalse; // Use memory from system's free pool

	 // Enter critical section 
	NKern::ThreadEnterCS();
	
	//Create shared chunk for PCI window
	TInt r = Kern::ChunkCreate(chunkinfo, pciWindowChunk, pciChunkKernelAddr, pciChunkMapAttr);
	if(r!=KErrNone)
    {
		// Failed to create DChunk	
		__KTRACE_OPT(KPCI,Kern::Printf("Failed to create DChunk: Error code is=%d", r) );		
		NKern::ThreadLeaveCS();
		return r;
	}

	//This address is PSL specific. This will have to be changed
	//if d_pci.cpp is ever made generic
	TUint32 pciPhysicalAddr = KHwUSBHPhys; // Internal PCI window address

	//Commit memory to a DChunk using  Internal PCI window address
	r = Kern::ChunkCommitPhysical(pciWindowChunk,0,KHwUSBHInternalPciWindowSize, pciPhysicalAddr);
	if(r!=KErrNone)
	{
		// Failed to commit memory
		Kern::Printf("Commit failed: Error code is=%d", r);
		__KTRACE_OPT(KPCI,Kern::Printf("Commit failed: Error code is=%d", r));

		// Close chunk, which will then get deleted at some point
		Kern::ChunkClose(pciWindowChunk);
		NKern::ThreadLeaveCS();
		return r;
	}		

	//Close pciPlatChunk using pciDChunk as pciDChunk now owns it
	const TInt rHandle = Kern::MakeHandleAndOpen(NULL, pciWindowChunk); //Get PCI Window DChunk handle
	pciWindowChunk->Close(NULL);	// Close pci window chunk	
	NKern::ThreadLeaveCS(); // Finished
	return rHandle;   	
}

/**
This function creates and opens a shared chunk. The chunk is then commited to a contiguous memory
@param aPciChunkSize contains the size of the PCI DChunk which is to be created
@param aAttributes  on return, this is set to the mmu mapping attributes used for the chunk
@param aChunk on return, a reference to the shared chunk
@param aVirt on return, this is set to the virtual address shared chunk
@param aPhysicalAddress on return, this is set to the physical address of the first page of memory
	   which was committed.
*/
TInt DPciTestChannel::CreateSharedChunk(TInt aPciChunkSize, TUint32 aAttributes, DChunk*& aChunk, TLinAddr& aVirt, TPhysAddr& aPhysicalAddress)
{
	__NK_ASSERT_DEBUG(aChunk==NULL);
	aPciChunkSize = Kern::RoundToPageSize(aPciChunkSize);
	DChunk* pC=NULL;

	// Enter critical section 
	NKern::ThreadEnterCS();

	//Chunk Attributes  for DChunk
	TChunkCreateInfo info;
	info.iType=TChunkCreateInfo::ESharedKernelSingle;
	info.iMaxSize=aPciChunkSize;
	info.iMapAttr=aAttributes;
	info.iOwnsMemory=ETrue;

	//Create DChunk
	TInt r=Kern::ChunkCreate(info, pC, aVirt, aAttributes);
	if(r!=KErrNone)
		{
		NKern::ThreadLeaveCS();
		return r;
		}
	//Commit DChunk to Contiguous memory
	r = Kern::ChunkCommitContiguous(pC, 0, aPciChunkSize, aPhysicalAddress);
	if(r==KErrNone)
		{
			aChunk=pC;		
		}
	else
		{
		Kern::ChunkClose(pC);
		__KTRACE_OPT(KPCI,Kern::Printf("Commit DChunk to Contiguous memory Failed : Error code is=%d",r));
		return r;
		}

	NKern::ThreadLeaveCS(); // Finished
	__KTRACE_OPT(KPCI, Kern::Printf("Created SC: size=0x%08x, virtual= 0x%08x, phys=0x%08x", aPciChunkSize, aVirt, aPhysicalAddress));	
	return r;
}

class DPciDevice : public DLogicalDevice
	{
public:
	DPciDevice();
	~DPciDevice();
	TInt Install();
	void GetCaps(TDes8& aDes) const;
	TInt Create(DLogicalChannelBase*& aChannel);
	};

DPciDevice::DPciDevice()
	{
	FUNC_LOG();
	}

DPciDevice::~DPciDevice()
	{
	FUNC_LOG();
	}

TInt DPciDevice::Install()
	{
	return SetName(&KPciLddFactory);
	}

void DPciDevice::GetCaps(TDes8&) const
	{
	}

TInt DPciDevice::Create(DLogicalChannelBase*& aChannel)
	{
	aChannel = new DPciTestChannel;
	return aChannel ? KErrNone : KErrNoMemory;
	}

/****************************************
TUserPciSpace
*/

/**
Decides what action to run based on contents of class
*/
TUint TUserPciSpace::KRun(TAddrSpace& aAddrSp)
	{

	//this could be reworked as a function pointer
	//table, but this might be clearer
	switch(iBitWidth)
		{
	case 8:
			{
			switch(iOperation)
				{
			case ERead:
					{
					return aAddrSp.Read8(iOffset);
					}
			case EWrite:
					{
					aAddrSp.Write8(iOffset, iWriteValue);
					return KErrNone;
					}
			case EModify:
					{
					aAddrSp.Modify8(iOffset, iClearMask, iSetMask);
					return KErrNone;
					}
			default:
					{
					Kern::PanicCurrentThread(KPciTest, KErrNotReady);
					}
				}
			}
	case 16:
			{
			switch(iOperation)
				{
			case ERead:
					{
					return aAddrSp.Read16(iOffset);
					}
			case EWrite:
					{
					aAddrSp.Write16(iOffset, iWriteValue);
					return KErrNone;
					}
			case EModify:
					{
					aAddrSp.Modify16(iOffset, iClearMask, iSetMask);
					return KErrNone;
					}
			default:
					{
					Kern::PanicCurrentThread(KPciTest, KErrNotReady);
					}
				}
			}
	case 32:
			{
			switch(iOperation)
				{
			case ERead:
					{
					return aAddrSp.Read32(iOffset);
					}
			case EWrite:
					{
					aAddrSp.Write32(iOffset, iWriteValue);
					return KErrNone;
					}
			case EModify:
					{
					aAddrSp.Modify32(iOffset, iClearMask, iSetMask);
					return KErrNone;
					}
			default:
					{
					Kern::PanicCurrentThread(KPciTest, KErrNotReady);
					}
				}
			}
	default:
			{
			Kern::PanicCurrentThread(KPciTest, KErrArgument);
			}
			
		}

	//unreachable return
	return KMaxTUint;
	}

//stub implementation for kernel side
TUint TUserConfigSpace::Call()
		{
		FAULT();
		return 0;
		} 

TUserPciSpace* TUserConfigSpace::Clone() const
		{
		FAULT();
		return 0;
		} 

//stub implementation for kernel side
TUint TUserMemorySpace::Call()
		{
		FAULT();
		return 0;
		} 

TUserPciSpace* TUserMemorySpace::Clone() const
		{
		FAULT();
		return 0;
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


DECLARE_STANDARD_LDD()
	{
	return new DPciDevice;
	}
