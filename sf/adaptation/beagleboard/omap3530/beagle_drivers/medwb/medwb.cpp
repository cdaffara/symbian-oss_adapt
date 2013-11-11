/* Cypress West Bridge API source file 
## ===========================
##
##  Copyright Cypress Semiconductor Corporation, 2006-2009,
##  All Rights Reserved
##  UNPUBLISHED, LICENSED SOFTWARE.
##
##  CONFIDENTIAL AND PROPRIETARY INFORMATION
##  WHICH IS THE PROPERTY OF CYPRESS.
##
##  Use of this file is governed
##  by the license agreement included in the file
##
##     <install>/license/license.txt
##
##  where <install> is the Cypress software
##  installation root directory path.
##
## ===========================
*/

#include "locmedia.h"
#include "platform.h"
//#include "variantmediadef.h"
#include <assp\omap3530_assp\CyAsSymbianStorageDriver.h>

//#define REGIST_MEDIA_USE_MMC
#define _MEDWB_DEBUG_1_
//#define _MEDWB_DEBUG_2_

//#define INTERVAL_FOR_WB 15 // 15 -- OK
#define WB_BUFFER_SIZE	2*(65536 + 512)
#define WB_RETRY_COUNT 2

//const TInt KStackNumber = 0;
const TInt KDiskSectorSize=512;
const TInt KDiskSectorShift=9;
//const TInt KIdleCurrentInMilliAmps = 1;

const TInt KMBRFirstPartitionEntry=0x1BE;

TUint8	ptrWBBuffer[WB_BUFFER_SIZE];


template <class T>
inline T UMin(T aLeft,T aRight)
	{return(aLeft<aRight ? aLeft : aRight);}


class DPhysicalDeviceMediaWB : public DPhysicalDevice
	{
public:
	DPhysicalDeviceMediaWB();

	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DBase*& aChannel, TInt aMediaId, const TDesC8* aInfo, const TVersion& aVer);
	virtual TInt Validate(TInt aDeviceType, const TDesC8* aInfo, const TVersion& aVer);
	virtual TInt Info(TInt aFunction, TAny* a1);
	};


class DMediaDriverWB : public DMediaDriver
	{
public:
	DMediaDriverWB(TInt aMediaId);
	~DMediaDriverWB();
	// ...from DMediaDriver
	virtual void Close();
	// replacing pure virtual
	virtual void Disconnect(DLocalDrive* aLocalDrive, TThreadMessage*);
	virtual TInt Request(TLocDrvRequest& aRequest);
	virtual TInt PartitionInfo(TPartitionInfo& anInfo);
	virtual void NotifyPowerDown();
	virtual void NotifyEmergencyPowerDown();
	// For creation by DPhysicalDeviceMediaMmcFlash
	TInt DoCreate(TInt aMediaId);

private:

private:
	// MMC device specific stuff
	TInt DoRead(TLocDrvRequest&);
	TInt DoWrite(TLocDrvRequest&);
	TInt DoFormat(TLocDrvRequest&);
	TInt Caps(TLocDrv& aDrive, TLocalDriveCapsV6& aInfo);


	TInt DecodePartitionInfo();
	TInt WritePartitionInfo();
	TInt GetDefaultPartitionInfo(TMBRPartitionEntry& aPartitionEntry);
	TInt CreateDefaultPartition();



	static void SetPartitionEntry(TPartitionEntry* aEntry, TUint aFirstSector, TUint aNumSectors);

	TInt CheckDevice(int aReqType);
	
	void Reset();

private:
	TInt iMediaId;
	TPartitionInfo* iPartitionInfo;
	TBool iMbrMissing;
	TUint iHiddenSectors;						// bootup / password

	TUint8* ptrWriteBuf;							// start of current buffer region
	TUint8* ptrReadBuf;							// start of current buffer region
	TInt	read_size;
	TInt	read_pos;

	TInt 	iUnitSize;
	TInt	iBlockSize;
	};
	
// ======== DPhysicalDeviceMediaMmcFlash ========


DPhysicalDeviceMediaWB::DPhysicalDeviceMediaWB()
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("=mmd:ctr");
	#endif
	iUnitsMask = 0x01;
	iVersion = TVersion(KMediaDriverInterfaceMajorVersion,KMediaDriverInterfaceMinorVersion,KMediaDriverInterfaceBuildVersion);
	}


TInt DPhysicalDeviceMediaWB::Install()
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("=mmd:ins");
	#endif
	_LIT(KDrvNm, "Media.WB");
	return SetName(&KDrvNm);
	}


void DPhysicalDeviceMediaWB::GetCaps(TDes8& /* aDes */) const
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("=mmd:cap");
	#endif
	}
								 
									 
TInt DPhysicalDeviceMediaWB::Info(TInt aFunction, TAny* /*a1*/)
//
// Return the priority of this media driver
//
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("=mmd:info");
	#endif
	if (aFunction==EPriority)
		return KMediaDriverPriorityNormal;
	// Don't close media driver when peripheral bus powers down. This avoids the need for Caps() to power up the stack.
	if (aFunction==EMediaDriverPersistent)
		return KErrNone;
	return KErrNotSupported;
	}
								 
TInt DPhysicalDeviceMediaWB::Validate(TInt aDeviceType, const TDesC8* /*aInfo*/, const TVersion& aVer)
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("=mmd:validate aDeviceType %d\n", aDeviceType);
	#endif
	if (!Kern::QueryVersionSupported(iVersion,aVer))
	{	
	#ifdef _MEDWB_DEBUG_3_
		Kern::Printf("Validate -> KErrNotSupported\n");
	#endif
		return KErrNotSupported;
	}
	if (aDeviceType!=MEDIA_DEVICE_MMC)
	{
	#ifdef _MEDWB_DEBUG_3_
		Kern::Printf("Validate -> Wrong DeviceType\n");
	#endif
		return KErrNotSupported;
	}
	return KErrNone;
	}
	
								 
TInt DPhysicalDeviceMediaWB::Create(DBase*& aChannel, TInt aMediaId, const TDesC8* /*aInfo*/, const TVersion& aVer)
//
// Create an MMC Card media driver.
//
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("=mmd:crt");
	#endif
	if (!Kern::QueryVersionSupported(iVersion,aVer))
		return KErrNotSupported;

	DMediaDriverWB* pD = new DMediaDriverWB(aMediaId);
	aChannel=pD;

	TInt r=KErrNoMemory;
	if (pD)
		r=pD->DoCreate(aMediaId);

	#ifdef REGIST_MEDIA_USE_MMC
	if (r==KErrNone)
		pD->OpenMediaDriverComplete(KErrNone);
	#endif
	__KTRACE_OPT(KPBUSDRV, Kern::Printf("<mmd:mdf"));
	return r;
	}



// Helper
template <class T>
inline T* KernAlloc(const TUint32 n)
	{ return static_cast<T*>(Kern::Alloc(n * sizeof(T))); }

// ---- ctor, open, close, dtor ----

#pragma warning( disable : 4355 )	// this used in initializer list
DMediaDriverWB::DMediaDriverWB(TInt aMediaId)
   :DMediaDriver(aMediaId),
	iMediaId(iPrimaryMedia->iNextMediaId)
	{
	#ifdef _MEDWB_DEBUG_2_
	Kern::Printf("=mmd:wb");
	// NB aMedia Id = the media ID of the primary media, iMediaId = the media ID of this media
	Kern::Printf("DMediaDriverWB(), iMediaId %d, aMediaId %d\n", iMediaId, aMediaId);
	#endif
	}

#pragma warning( default : 4355 )
TInt DMediaDriverWB::DoCreate(TInt /*aMediaId*/)
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf(">mmd:opn");
	#endif
	iUnitSize = CyAsSymbianStorageDriver::GetUnitSize();
	iBlockSize = CyAsSymbianStorageDriver::GetBlockSize();
	if( iBlockSize == 0 )
		iBlockSize = 1;
	
	read_pos = 0x7FFFFFFF;
	read_size = 0;

	// get card characteristics
	SetTotalSizeInBytes(CyAsSymbianStorageDriver::GetMediaSize());
	
	// get buffer memory from EPBUS
	ptrReadBuf = ptrWBBuffer;
	ptrWriteBuf = &ptrWBBuffer[65536+512];
	
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("<mmd:opn");
	#endif
	return(KErrNone);
	}

void DMediaDriverWB::Close()
//
// Close the media driver - also called on media change
//
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("=mmd:cls");
	#endif
	EndInCritical();
	//CompleteRequest(KErrNotReady);
	DMediaDriver::Close();
	}


DMediaDriverWB::~DMediaDriverWB()
	{

	}


// ---- media access ----

TInt DMediaDriverWB::DoRead(TLocDrvRequest& iCurrentReq)
//
// set up iReqStart, iReqEnd and iReqCur and launch first read.  Subsequent reads
// will be launched from the callback DFC.
//
	{
	Int64 pos=iCurrentReq.Pos();
	Int64 length=iCurrentReq.Length();
	TInt r;
	if (length<0 || pos<0 || (pos+length)>KMaxTInt)
		return KErrGeneral;
	TInt p=(TInt)pos;
	TInt l=(TInt)length;
	
	if (p+l>CyAsSymbianStorageDriver::GetMediaSize())
		return KErrGeneral;

	TInt pos_block = p / iBlockSize;
	TInt pos_offset =  p % iBlockSize;
	TInt size_block = l / iBlockSize; 
	TInt size_offset = l % iBlockSize; 
	
	TUint buf_offset = 0;
	TInt	local_pos;

	#ifdef _MEDWB_DEBUG_2_
	Kern::Printf("WB::Read> real (%d@%d)", l, p);
	Kern::Printf("WB::Read> pos_block - %d", pos_block) ;
	Kern::Printf("WB::Read> pos_offset - %d", pos_offset) ;
	Kern::Printf("WB::Read> size_block - %d", size_block) ;
	Kern::Printf("WB::Read> size_offset - %d", size_offset) ;
	#endif

	#ifdef INTERVAL_FOR_WB
	NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	if( pos_block == read_pos )
	{
		if( read_size < l )
		{
			buf_offset = read_size;
			size_block = (l-read_size) / iBlockSize;
			size_offset = (l-read_size) % iBlockSize;

			local_pos = pos_block + (read_size /iBlockSize);
		}
		else
		{
			TPtrC8 des((ptrReadBuf+pos_offset), l);
				
			r = iCurrentReq.WriteRemote(&des, 0);
			
			return r;
		}
	}
	#if 0
	else if( (read_pos + (read_size/iBlockSize)) > pos_block )
	{
		TInt adjust_offset;
		TInt adjust_size;
		adjust_offset = (read_pos - pos_block) * iBlockSize;
		adjust_size = read_size - adjust_offset;
		
		memcpy(ptrReadBuf, &ptrReadBuf[adjust_offset], adjust_size );

		read_pos = pos_block;
		read_size = adjust_size;
	
		if( read_size < l )
		{
			buf_offset = read_size;
			size_block = (l-read_size) / iBlockSize;
			size_offset = (l-read_size) % iBlockSize;

			local_pos = pos_block + (read_size /iBlockSize);
		}
		else
		{
			TPtrC8 des((ptrReadBuf+pos_offset), l);
				
			r = iCurrentReq.WriteRemote(&des, 0);
			
			return r;
		}
	}
	#endif
	else
		local_pos = read_pos = pos_block;
		
	#ifdef _MEDWB_DEBUG_2_
	Kern::Printf("WB::Read> local_pos - %d", local_pos) ;
	Kern::Printf("WB::Read> buf_offset - %d", buf_offset) ;
	#endif


	if( size_block )
	{
		CyAsSymbianStorageDriver::Read(local_pos, size_block, (void *)(ptrReadBuf+buf_offset));
		local_pos += size_block;
		buf_offset += (size_block*iBlockSize);
	#ifdef INTERVAL_FOR_WB
		NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	}
	
	if( pos_offset || size_offset )
	{
		CyAsSymbianStorageDriver::Read(local_pos, 1, (void *)(ptrReadBuf+buf_offset) );
		local_pos += size_block;
		buf_offset += iBlockSize;
	#ifdef INTERVAL_FOR_WB
		NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	}

	read_size = buf_offset;
		
	TPtrC8 des((ptrReadBuf+pos_offset), l);
		
	r = iCurrentReq.WriteRemote(&des, 0);
	
	return r;
	}


TInt DMediaDriverWB::DoWrite(TLocDrvRequest& iCurrentReq)
//
// set up iReqStart, iReqEnd, and iReqCur, and launch first write.  Any subsequent
// writes are launched from the session end DFC.  LaunchWrite() handles pre-reading
// any sectors that are only partially modified.
//
	{
	Int64 pos = iCurrentReq.Pos();
	Int64 length = iCurrentReq.Length();
	TInt r;
	if (length<0 || pos<0 || (pos+length)>KMaxTInt)
		return KErrGeneral;
	TInt p=(TInt)pos;
	TInt l=(TInt)length;

	if (p+l>CyAsSymbianStorageDriver::GetMediaSize())
		return KErrGeneral;

	#ifdef _MEDWB_DEBUG_2_
	Kern::Printf("WB::Write> (%d@%d)", l, p);
	#endif
	#ifdef INTERVAL_FOR_WB
	NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	
	TInt pos_block = p / iBlockSize;
	TInt pos_offset =  p % iBlockSize;
	TInt size_block = l / iBlockSize; 
	TInt size_offset = l % iBlockSize; 
	
	TUint buf_offset = 0;
	TInt	local_pos;
	TInt	w_block_size = 0;

	local_pos = pos_block;
	
	if( size_block )
	{
		CyAsSymbianStorageDriver::Read(local_pos, size_block, (void *)(ptrWriteBuf+buf_offset));
		local_pos += size_block;
		buf_offset += (size_block*iBlockSize);
		w_block_size += size_block;
	#ifdef INTERVAL_FOR_WB
		NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	}
	
	if( pos_offset || size_offset )
	{
		CyAsSymbianStorageDriver::Read(local_pos, 1, (void *)(ptrWriteBuf+buf_offset) );
		local_pos += size_block;
		buf_offset += iBlockSize;
		w_block_size ++;
	#ifdef INTERVAL_FOR_WB
		NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	}
	
	#ifdef _MEDWB_DEBUG_2_
	Kern::Printf("WB::Write> pos_block - %d", pos_block) ;
	Kern::Printf("WB::Write> pos_offset - %d", pos_offset) ;
	Kern::Printf("WB::Write> size_block - %d", size_block) ;
	Kern::Printf("WB::Write> size_offset - %d", size_offset) ;
	#endif

	TPtr8 des((ptrWriteBuf+pos_offset), l);
		
	if ( (r = iCurrentReq.ReadRemote(&des,0)) !=KErrNone)
	{
		return r;			
	}


	#ifdef _MEDWB_DEBUG_2_
	Kern::Printf("WB::Write> local_pos - %d", local_pos) ;
	Kern::Printf("WB::Write> w_block_size - %d", w_block_size) ;
	#endif
	
	CyAsSymbianStorageDriver::Write(local_pos, w_block_size, ptrWriteBuf);
	#ifdef INTERVAL_FOR_WB
	NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	return r;
	}


TInt DMediaDriverWB::DoFormat(TLocDrvRequest& iCurrentReq)
	{
	Int64 pos = iCurrentReq.Pos();
	Int64 length = iCurrentReq.Length();
	TInt r = KErrGeneral;
	if (length<0 || pos<0 || (pos+length)>KMaxTInt)
		return r;
	TInt p=(TInt)pos;
	TInt l=(TInt)length;

	if (p+l>CyAsSymbianStorageDriver::GetMediaSize())
		return KErrGeneral;

	#ifdef _MEDWB_DEBUG_1_
	Kern::Printf("WB::Format> (%d@%d)", l, p);
	#endif
	#ifdef INTERVAL_FOR_WB
	NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	TInt pos_block = p / iBlockSize;
	TInt pos_offset =  p % iBlockSize;
	TInt size_block = l / iBlockSize; 
	TInt size_offset = l % iBlockSize; 
	
	TUint buf_offset = 0;
	TInt	local_pos;
	TInt	w_block_size = 0;

	local_pos = pos_block;
	
	if( size_block )
	{
		CyAsSymbianStorageDriver::Read(local_pos, size_block, (void *)(ptrWriteBuf+buf_offset));
		local_pos += size_block;
		buf_offset += (size_block*iBlockSize);
		w_block_size += size_block;
	#ifdef INTERVAL_FOR_WB
		NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	}
	
	if( pos_offset || size_offset )
	{
		CyAsSymbianStorageDriver::Read(local_pos, 1, (void *)(ptrWriteBuf+buf_offset) );
		local_pos += size_block;
		buf_offset += iBlockSize;
		w_block_size ++;
	#ifdef INTERVAL_FOR_WB
		NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	}
	
	memclr(ptrWriteBuf + pos_offset, l);
	
	CyAsSymbianStorageDriver::Write(local_pos, w_block_size, ptrWriteBuf);
	#ifdef INTERVAL_FOR_WB
	NKern::Sleep(INTERVAL_FOR_WB);
	#endif
	return r;
	}


TInt DMediaDriverWB::PartitionInfo(TPartitionInfo& anInfo)
//
// Read the partition information for the media.  If the user supplied a password,
// then unlock the card before trying to read the first sector.
//
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf(">mmd:rpi");
	#endif
	iPartitionInfo = &anInfo;

	// Assume MBR will be present or is not required
	iMbrMissing = EFalse;

	TInt r = DecodePartitionInfo();


	if(r == KErrLocked)
		{
		// If the media is locked, we present a default partition entry to the local
		// media subsystem, which will be updated when the media is finally unlocked.
		r = CreateDefaultPartition();
		if (r != KErrNone)
			return r;
		return KErrLocked;
		}

	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("<mmd:rpi:%d", r);
	#endif
	// KErrNone indicates asynchronous completion
	return r;
	}





TInt DMediaDriverWB::DecodePartitionInfo()
//
// decode partition info that was read into internal buffer 
//
	{
	TInt partitionCount=iPartitionInfo->iPartitionCount=0;
	TInt defaultPartitionNumber=-1;
	TMBRPartitionEntry* pe;
	const TUint KMBRFirstPartitionOffsetAligned = KMBRFirstPartitionOffset & ~3;
	TInt i;

	CyAsSymbianStorageDriver::Read(0, 1, (void *)ptrReadBuf);
	read_pos = 0;
	read_size = 512;
	// Read of the first sector successful so check for a Master Boot Record
	if (*(TUint16*)(&ptrReadBuf[KMBRSignatureOffset])!=0xAA55)
		goto mbr_done;

	__ASSERT_COMPILE(KMBRFirstPartitionOffsetAligned + KMBRMaxPrimaryPartitions * sizeof(TMBRPartitionEntry) <= KMBRSignatureOffset);

	memmove(&ptrReadBuf[0], &ptrReadBuf[2],
		KMBRFirstPartitionOffsetAligned + KMBRMaxPrimaryPartitions * sizeof(TMBRPartitionEntry)); 


	for (i=0, pe = (TMBRPartitionEntry*)(&ptrReadBuf[KMBRFirstPartitionOffsetAligned]);
		pe->iPartitionType != 0 && i < KMBRMaxPrimaryPartitions;i++,pe++)
		{
		if (pe->IsDefaultBootPartition())
			{
			SetPartitionEntry(&iPartitionInfo->iEntry[0],pe->iFirstSector,pe->iNumSectors);
			defaultPartitionNumber=i;
			partitionCount++;
			break;
			}
		}

	// Now add any other partitions
	for (i=0, pe = (TMBRPartitionEntry*)(&ptrReadBuf[KMBRFirstPartitionOffsetAligned]);
		pe->iPartitionType != 0 && i < KMBRMaxPrimaryPartitions;i++,pe++)
		{
		TBool validPartition = ETrue;	// assume partition valid

		if (defaultPartitionNumber==i)
			{
			// Already sorted
			}

		// FAT partition ?
		else if (pe->IsValidDosPartition() || pe->IsValidFAT32Partition())
			{
			SetPartitionEntry(&iPartitionInfo->iEntry[partitionCount],pe->iFirstSector,pe->iNumSectors);
			#ifdef _MEDWB_DEBUG_1_
			Kern::Printf("WB: FAT partition found at sector #%u", pe->iFirstSector);
			#endif
			partitionCount++;
			}
		else
			{
			validPartition = EFalse;
			}
		
		if (validPartition && partitionCount == 1)
			iHiddenSectors = pe->iFirstSector;

		}

	// Check the validity of the partition address boundaries
	// If there is any
	if(partitionCount > 0)
		{
		const TInt64 deviceSize = CyAsSymbianStorageDriver::GetMediaSize();
		TPartitionEntry& part = iPartitionInfo->iEntry[partitionCount - 1];
		// Check that the card address space boundary is not exceeded by the last partition
		// In case of only 1 partition in the media check also it
		if(part.iPartitionBaseAddr + part.iPartitionLen > deviceSize)
			{
			Kern::Printf("WB: MBR partition exceeds card memory space");
			// Adjust the partition length to card address boundary
			part.iPartitionLen = (deviceSize - part.iPartitionBaseAddr);

			// Check that the base address contained valid information
			if(part.iPartitionLen <= 0)
				{
				Kern::Printf("WB: Invalid base address");
				// Invalid MBR - assume the boot sector is in the first sector
				defaultPartitionNumber =-1; 
				partitionCount=0;
				}
			}
		// More than one partition. Go through all of them
		if (partitionCount > 0)
			{
			for(i=partitionCount-1; i>0; i--)
				{
				const TPartitionEntry& curr = iPartitionInfo->iEntry[i];
				TPartitionEntry& prev = iPartitionInfo->iEntry[i-1];
				// Check if partitions overlap
				if(curr.iPartitionBaseAddr < (prev.iPartitionBaseAddr + prev.iPartitionLen))
					{
					Kern::Printf("WB: Overlapping partitions");
					// Adjust the partition length to not overlap the next partition
					prev.iPartitionLen = (curr.iPartitionBaseAddr - prev.iPartitionBaseAddr);

					// Check that the base address contained valid information
					if(prev.iPartitionLen <= 0)
						{
						Kern::Printf("WB: Invalid base address");
						// Invalid MBR - assume the boot sector is in the first sector
						defaultPartitionNumber=(-1); 
						partitionCount=0;
						}
					}
				}
			}
		}

mbr_done:
	if (defaultPartitionNumber==(-1) && partitionCount==0)
		{
		Kern::Printf("WB:PartitionInfo no MBR");
			{
			// Assume it has no MBR, and the Boot Sector is in the 1st sector
			SetPartitionEntry(&iPartitionInfo->iEntry[0],0,I64LOW(CyAsSymbianStorageDriver::GetMediaSize()>>KDiskSectorShift));
			iHiddenSectors=0;
			}
		partitionCount=1;
		}

	iPartitionInfo->iPartitionCount=partitionCount;
	iPartitionInfo->iMediaSizeInBytes=TotalSizeInBytes();
	#ifdef _MEDWB_DEBUG_1_

	Kern::Printf("<Mmc:PartitionInfo (C:%d)",iPartitionInfo->iPartitionCount);
	Kern::Printf("     Partition1 (B:%xH L:%xH)",I64LOW(iPartitionInfo->iEntry[0].iPartitionBaseAddr),I64LOW(iPartitionInfo->iEntry[0].iPartitionLen));
	Kern::Printf("     Partition2 (B:%xH L:%xH)",I64LOW(iPartitionInfo->iEntry[1].iPartitionBaseAddr),I64LOW(iPartitionInfo->iEntry[1].iPartitionLen));
	Kern::Printf("     Partition3 (B:%xH L:%xH)",I64LOW(iPartitionInfo->iEntry[2].iPartitionBaseAddr),I64LOW(iPartitionInfo->iEntry[2].iPartitionLen));
	Kern::Printf("     Partition4 (B:%xH L:%xH)",I64LOW(iPartitionInfo->iEntry[3].iPartitionBaseAddr),I64LOW(iPartitionInfo->iEntry[3].iPartitionLen));


	Kern::Printf("     iMediaSizeInBytes (%d)",iPartitionInfo->iMediaSizeInBytes);
	Kern::Printf("     iHiddenSectors (%d)",iHiddenSectors);
	#endif

	#ifdef _MEDWB_DEBUG_3_

	TMBRPartitionEntry cPe;
	if(GetDefaultPartitionInfo(cPe) == KErrNone)
		{
		pe = (TMBRPartitionEntry*)(&ptrReadBuf[0]);

		Kern::Printf("-------------------------------------------");
		Kern::Printf("-- Partition Entry Validation/Comparison --");
		Kern::Printf("-------------------------------------------");
		Kern::Printf("-- iX86BootIndicator [%02x:%02x] %c       -", pe->iX86BootIndicator, cPe.iX86BootIndicator, pe->iX86BootIndicator == cPe.iX86BootIndicator ? ' ' : 'X');
		Kern::Printf("--        iStartHead [%02x:%02x] %c       -", pe->iStartHead,        cPe.iStartHead,        pe->iStartHead        == cPe.iStartHead        ? ' ' : 'X');
		Kern::Printf("--      iStartSector [%02x:%02x] %c       -", pe->iStartSector,      cPe.iStartSector,      pe->iStartSector      == cPe.iStartSector      ? ' ' : 'X');
		Kern::Printf("--    iStartCylinder [%02x:%02x] %c       -", pe->iStartCylinder,    cPe.iStartCylinder,    pe->iStartCylinder    == cPe.iStartCylinder    ? ' ' : 'X');
		Kern::Printf("--    iPartitionType [%02x:%02x] %c       -", pe->iPartitionType,    cPe.iPartitionType,    pe->iPartitionType    == cPe.iPartitionType    ? ' ' : 'X');
		Kern::Printf("--          iEndHead [%02x:%02x] %c       -", pe->iEndHead,          cPe.iEndHead,          pe->iEndHead          == cPe.iEndHead          ? ' ' : 'X');
		Kern::Printf("--        iEndSector [%02x:%02x] %c       -", pe->iEndSector,        cPe.iEndSector,        pe->iEndSector        == cPe.iEndSector        ? ' ' : 'X');
		Kern::Printf("--      iEndCylinder [%02x:%02x] %c       -", pe->iEndCylinder,      cPe.iEndCylinder,      pe->iEndCylinder      == cPe.iEndCylinder      ? ' ' : 'X');
		Kern::Printf("--      iFirstSector [%08x:%08x] %c       -", pe->iFirstSector,      cPe.iFirstSector,      pe->iFirstSector      == cPe.iFirstSector      ? ' ' : 'X');
		Kern::Printf("--       iNumSectors [%08x:%08x] %c       -", pe->iNumSectors,       cPe.iNumSectors,       pe->iNumSectors       == cPe.iNumSectors       ? ' ' : 'X');
		Kern::Printf("-------------------------------------------");
		}
	#endif

	return(KErrNone);
	}


TInt DMediaDriverWB::WritePartitionInfo()
/**
	Write the default partition table to freshly formatted media
	@return Standard Symbian OS Error Code
 */
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf(">mmd:wpi");
	#endif
	TMBRPartitionEntry partitionEntry;
	TInt err = GetDefaultPartitionInfo(partitionEntry);
	if(err == KErrNone)
		{
		TUint8	*iPartitionBuf;
		
		iPartitionBuf = new TUint8[KDiskSectorSize];
		
		#ifdef _MEDWB_DEBUG_2_
		Kern::Printf("mmd:MBR/Partition Table");
		Kern::Printf("    Boot ID          : %02xh", partitionEntry.iX86BootIndicator);
		Kern::Printf("    Start Head       : %02xh", partitionEntry.iStartHead);
		Kern::Printf("    Start Sector     : %02xh", partitionEntry.iStartSector);
		Kern::Printf("    Start Cyclinder  : %02xh", partitionEntry.iStartCylinder);
		Kern::Printf("    System ID        : %02xh", partitionEntry.iPartitionType);
		Kern::Printf("    End Head         : %02xh", partitionEntry.iEndHead);
		Kern::Printf("    End Sector       : %02xh", partitionEntry.iEndSector);
		Kern::Printf("    End Cyclinder    : %02xh", partitionEntry.iEndCylinder);
		Kern::Printf("    Relative Sector  : %08xh", partitionEntry.iFirstSector);
		Kern::Printf("    Number of Sectors: %08xh", partitionEntry.iNumSectors);
		#endif
		//
		// Clear all other partition entries and align the partition info into the minor buffer for writing...
		//
		memclr(iPartitionBuf, KDiskSectorSize);
		memcpy(&iPartitionBuf[KMBRFirstPartitionEntry], &partitionEntry, sizeof(TMBRPartitionEntry));

		*(TUint16*)(&iPartitionBuf[KMBRSignatureOffset]) = 0xAA55;

		err = CyAsSymbianStorageDriver::Write(0, 1, iPartitionBuf);
		
		//
		// Write the partition table and engage the read to validate and complete the mount process
		//
		delete iPartitionBuf;
		iMbrMissing = EFalse;
		}

	#ifdef _MEDWB_DEBUG_3_
	__KTRACE_OPT(KPBUSDRV, Kern::Printf("<mmd:wpi:%d", err));
	#endif
	
	return(err);
	}


TInt DMediaDriverWB::CreateDefaultPartition()
	{
	TMBRPartitionEntry defPartition;
	TInt r = GetDefaultPartitionInfo(defPartition);
	if (r == KErrNone)
		{
		SetPartitionEntry(&iPartitionInfo->iEntry[0], defPartition.iFirstSector, defPartition.iNumSectors);
		iHiddenSectors = defPartition.iFirstSector;
		iPartitionInfo->iPartitionCount   = 1;
		iPartitionInfo->iMediaSizeInBytes = TotalSizeInBytes();
		}
	return r;
	}

TInt DMediaDriverWB::GetDefaultPartitionInfo(TMBRPartitionEntry& aPartitionEntry)
/**
	Calculates the default patition information for an specific card.
	@param aPartitionEntry The TMBRPartitionEntry to be filled in with the format parameters
	@return Standard Symbian OS Error Code
 */
	{
	memclr(&aPartitionEntry, sizeof(TMBRPartitionEntry));

	const TUint32 KTotalSectors = I64LOW(CyAsSymbianStorageDriver::GetMediaSize() >> KDiskSectorShift);

	aPartitionEntry.iFirstSector = (CyAsSymbianStorageDriver::GetEraseBlockSize()>> KDiskSectorShift);
	aPartitionEntry.iNumSectors  = KTotalSectors - aPartitionEntry.iFirstSector;
	aPartitionEntry.iX86BootIndicator = 0x00;

	if(aPartitionEntry.iNumSectors < 32680)
		{
		aPartitionEntry.iPartitionType = KPartitionTypeFAT12;
		}
	else if(aPartitionEntry.iNumSectors < 65536)
		{
		aPartitionEntry.iPartitionType = KPartitionTypeFAT16small;
		}
	else if (aPartitionEntry.iNumSectors < 1048576)
		{
		aPartitionEntry.iPartitionType = KPartitionTypeFAT16;
 		}
	else
		{
		aPartitionEntry.iPartitionType = KPartitionTypeWin95FAT32;
		}
	
	return(KErrNone);
	}

void DMediaDriverWB::SetPartitionEntry(TPartitionEntry* aEntry, TUint aFirstSector, TUint aNumSectors)
//
// auxiliary static function to record partition information in TPartitionEntry object
//
	{
	aEntry->iPartitionBaseAddr=aFirstSector;
	aEntry->iPartitionBaseAddr<<=KDiskSectorShift;
	aEntry->iPartitionLen=aNumSectors;
	aEntry->iPartitionLen<<=KDiskSectorShift;
	aEntry->iPartitionType=KPartitionTypeFAT12;	
	}

// ---- device status, callback DFC ----

TInt DMediaDriverWB::CheckDevice(int aReqType)
//
// Check the device before initiating a command
//
	{
	
	__KTRACE_OPT(KPBUSDRV, Kern::Printf(">wb:cd:%d",aReqType));

	TInt r=KErrNone;
#if 0
	if (!iCard->IsReady())
		r=KErrNotReady;

	// The card must be locked if attempting to unlock during RPI, and
	// unlocked at all other times.
	else if (aReqType!=EMReqTypeUnlockPswd && iCard->IsLocked())
		r=KErrLocked;
	// Don't perform Password setting for WriteProtected cards, 
	// unable to recover (ForcedErase) if password lost.
	else if (aReqType==EMReqTypeChangePswd)
		{
		if (iCard->MediaType()==EMultiMediaROM)
			{
			r=KErrAccessDenied;
			}
		}
	else if (iMbrMissing && aReqType==EMReqTypeNormalRd)
		r=KErrCorrupt;

	// Don't perform write operations when the mechanical write protect switch is set
	else if (aReqType==EMReqTypeNormalWr && iCard->IsWriteProtected())
		r=KErrAccessDenied;
	// Don't perform write/format operations on MMC ROM cards
	else if (iMediaType==EMultiMediaROM && aReqType == EMReqTypeNormalWr)
		r=KErrAccessDenied;
#endif
	__KTRACE_OPT(KPBUSDRV, Kern::Printf("<wb:cd:%d", r));
	return(r);
	}


// ---- request management ----

TInt DMediaDriverWB::Caps(TLocDrv& aDrive, TLocalDriveCapsV6& aInfo)
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf(">WB::Caps ");
	#endif
	// Fill buffer with current media caps.
	aInfo.iType = EMediaHardDisk;
	//aInfo.iBattery = EBatNotSupported;
	aInfo.iDriveAtt = KDriveAttLocal;
	aInfo.iMediaAtt	= KMediaAttFormattable;

	//if(CyAsSymbianStorageDriver::GetIsLocked())
	//	aInfo.iMediaAtt |= KMediaAttLockable;
	//if (iCard->HasPassword())
		//aInfo.iMediaAtt |= KMediaAttHasPassword;
		
	if ( !CyAsSymbianStorageDriver::GetIsWriteable())
		aInfo.iMediaAtt |= KMediaAttWriteProtected;
	if (CyAsSymbianStorageDriver::GetIsLocked())
		aInfo.iMediaAtt |= KMediaAttLocked;

	aInfo.iFileSystemId = KDriveFileSysFAT;

	// Format is performed in multiples of the erase sector (or multiple block) size
	aInfo.iMaxBytesPerFormat =  CyAsSymbianStorageDriver::GetBlockSize();

    // Set serial number to CID
    aInfo.iSerialNumLength = 16;
    for (TUint i=0; i<16; i++)
        aInfo.iSerialNum[i] = 0;
    
	// Get block size & erase block size to allow the file system to align first usable cluster correctly
	aInfo.iBlockSize = CyAsSymbianStorageDriver::GetBlockSize();
	aInfo.iEraseBlockSize = CyAsSymbianStorageDriver::GetEraseBlockSize();

	if ( CyAsSymbianStorageDriver::GetIsRemovable())
		aInfo.iDriveAtt|= KDriveAttRemovable;

	// Must return KErrCompletion to indicate that this 
	// is a synchronous version of the function
	return KErrNone;
	}


void DMediaDriverWB::NotifyPowerDown()
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf(">Mmc:NotifyPowerDown");
	#endif

	EndInCritical();

	//CompleteRequest(KErrNotReady);
	}

void DMediaDriverWB::NotifyEmergencyPowerDown()
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf(">Ata:NotifyEmergencyPowerDown");
	#endif
	//TInt r=KErrNotReady;
	//if (iCritical)
	//	r=KErrAbort;
	EndInCritical();

	// need to cancel the session as the stack doesn't take too kindly to having the same session engaged more than once.

	//CompleteRequest(r);
	}

TInt DMediaDriverWB::Request(TLocDrvRequest& aRequest)
	{
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("MmcMd:Req %08x id %d",&aRequest,aRequest.Id());
	#endif
	TInt r=KErrNotSupported;
	TInt id=aRequest.Id();

	NKern::ThreadEnterCS();

	TUint partitionType = aRequest.Drive()->iPartitionType;
	TBool readOnly = (partitionType == KPartitionTypeRofs || partitionType == KPartitionTypeROM);

	switch (id)
		{
		case DLocalDrive::ECaps:
			{
				TLocalDriveCapsV6& c = *(TLocalDriveCapsV6*)aRequest.RemoteDes();
				TLocDrv& drive = *aRequest.Drive();
				r = Caps(drive, c);
				c.iSize = drive.iPartitionLen;
				c.iPartitionType = drive.iPartitionType;	
				c.iHiddenSectors = (TUint) (drive.iPartitionBaseAddr >> KDiskSectorShift);
				#ifdef _MEDWB_DEBUG_2_
				Kern::Printf("caps : c.iSize = %d ", c.iSize);
				Kern::Printf("caps : c.iPartitionType = %d ", c.iPartitionType);
				Kern::Printf("caps : c.iHiddenSectors = %d ", c.iHiddenSectors);
				#endif
			}
			break;
		case DLocalDrive::EQueryDevice:
			Kern::Printf(">WB::EQueryDevice ");
			r = KErrNotSupported;
			break;

		case DLocalDrive::ERead:
			r=DoRead(aRequest);
			break;
		case DLocalDrive::EWrite:
			if (readOnly)
				return KErrNotSupported;
			r=DoWrite(aRequest);
			break;
		case DLocalDrive::EFormat:
			if (readOnly)
				return KErrNotSupported;
			r=DoFormat(aRequest);
			break;


		case DLocalDrive::EPasswordUnlock:
		case DLocalDrive::EPasswordLock:
		case DLocalDrive::EPasswordClear:
		case DLocalDrive::EPasswordErase:
		case DLocalDrive::EWritePasswordStore:
				Kern::Printf(">WB::EPassword ");
		break;
		case DLocalDrive::EEnlarge:
		case DLocalDrive::EReduce:
			Kern::Printf(">WB::EReduce ");
		default:
			Kern::Printf(">WB::default ");
			r=KErrNotSupported;
			break;
		}

	NKern::ThreadLeaveCS();
	#ifdef _MEDWB_DEBUG_3_
	Kern::Printf("MmcMd:Req %08x cmp %d",&aRequest,r);
	#endif
	return r;
	}

void DMediaDriverWB::Disconnect(DLocalDrive* aLocalDrive, TThreadMessage* aMsg)
	{
	// Complete using the default implementation
	DMediaDriver::Disconnect(aLocalDrive, aMsg);
	}


DECLARE_EXTENSION_PDD()
	{
	// NB if the media driver has been defined as a kernel extension in the .OBY/.IBY file 
	// i.e the "extension" keyword has been used rather than "device", then an instance of 
	// DPhysicalDeviceMediaMmcFlash will already have been created by InitExtension(). In this 
	// case the kernel will see that an object of the same name already exists and delete the 
	// new one.
	return new DPhysicalDeviceMediaWB;
	}
#ifdef REGIST_MEDIA_USE_MMC
DECLARE_STANDARD_EXTENSION()
	{	
	Kern::Printf("Creating WestBridge PDD");

	DPhysicalDeviceMediaWB* device = new DPhysicalDeviceMediaWB;

	TInt r;
	if (device==NULL)
		r=KErrNoMemory;
	else
		r=Kern::InstallPhysicalDevice(device);
	Kern::Printf("Installing WestBridge PDD in kernel returned %d",r);

	if( CyAsSymbianStorageDriver::Open() )
		Kern::Printf("**CyAsSymbianStorageDriver::Open() - Success");
	else
		Kern::Printf("**CyAsSymbianStorageDriver::Open() - Fail");

	Kern::Printf("WestBridge extension entry point drive returns %d",r);
	return r;
	}

#else
static const TInt WBDriveNumbers[1]={1};	
_LIT(KWBDriveName,"WestBridge");

DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("Registering WB drive"));
	#ifdef _MEDWB_DEBUG_2_
	Kern::Printf("Registering WB drive");
	#endif
	TInt r=KErrNoMemory;
	DPrimaryMediaBase* pM=new DPrimaryMediaBase;
	if (pM)
		{
		r=LocDrv::RegisterMediaDevice(MEDIA_DEVICE_MMC,1,&WBDriveNumbers[0],pM,1,KWBDriveName);
		}


	if( CyAsSymbianStorageDriver::Open() )
	{
		#ifdef _MEDWB_DEBUG_1_
		Kern::Printf("**CyAsSymbianStorageDriver::Open() - Success");
		#endif
	}
	else
	{
		#ifdef _MEDWB_DEBUG_1_
		Kern::Printf("**CyAsSymbianStorageDriver::Open() - Fail");
		#endif
	}
	
	__KTRACE_OPT(KBOOT,Kern::Printf("Registering WB drive - return %d",r));
	#ifdef _MEDWB_DEBUG_2_
	Kern::Printf("Registering WB drive - return %d",r);
	#endif
	
	return r;
	}


#endif

