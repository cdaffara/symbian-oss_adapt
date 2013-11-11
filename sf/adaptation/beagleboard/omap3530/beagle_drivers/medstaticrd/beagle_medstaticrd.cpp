/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - contribution.
*
* Contributors:
* Gareth Long - Symbian Foundation
*
* Description: Minimalistic non volatile memory driver
*
*/


#include "locmedia.h"
#include "platform.h"
#include "\sf\adaptation\beagleboard\omap3530\beagleboard\inc\variantmediadef.h"
#include "beagle_medstaticrd.h"
//#include "syborg.h"

#define NVMEM1_DRIVENAME "STATICRD"
#define NVMEM1_DRIVECOUNT 1
#define NVMEM1_DRIVELIST 1
#define NVMEM1_NUMMEDIA 1

_LIT(KNVMemPddName, "Media.MEDTATICRD");
_LIT(KNVMemDriveName,NVMEM1_DRIVENAME);

class DPhysicalDeviceMediaStaticRD : public DPhysicalDevice
	{
public:
	DPhysicalDeviceMediaStaticRD();
public:
	// Implementing the interface
	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DBase*& aChannel, TInt aMediaId, const TDesC8* anInfo, const TVersion& aVer);
	virtual TInt Validate(TInt aDeviceType, const TDesC8* anInfo, const TVersion& aVer);
	virtual TInt Info(TInt aFunction, TAny* a1);
	};
								
class DMediaDriverStaticRD : public DMediaDriver
	{
public:
	DMediaDriverStaticRD(TInt aMediaId);
public:
	// Implementing the interface
	virtual TInt Request(TLocDrvRequest& aRequest);
	virtual void Disconnect(DLocalDrive* aLocalDrive, TThreadMessage* aMsg);
	virtual TInt PartitionInfo(TPartitionInfo &anInfo);
	virtual void NotifyPowerDown();
	virtual void NotifyEmergencyPowerDown();
public:
	void CompleteRequest(TInt aReason);
	TInt DoCreate(TInt aMediaId);
	TInt Caps(TLocDrvRequest& aRequest);
	TInt Read();
	TInt Write();
	TInt Format();
	static void TransactionLaunchDfc(TAny* aMediaDriver);
	void DoTransactionLaunchDfc();
	static void SessionEndDfc(TAny* aMediaDriver);
	void DoSessionEndDfc();
	TUint32 GetNVMemSize( void );

public:
	TUint32 iLatestTransferSectorCount;
	TDfc iSessionEndDfc;

private:
	TInt ContinueTransaction( TUint32 aTransactionSectorOffset, TUint32 aTransactionSectorCount, TUint32 aDirection );
	static void Isr(TAny* aPtr);

private:
	TLocDrvRequest* iCurrentRequest;				// Current Request
	Int64 iTotalLength;
	Int64 iProsessedLength;
	Int64 iPos;
	TPhysAddr iTransferBufferPhys;
	TUint8* iTransferBufferLin;
	TUint8* iDiscBufferLin;
	TUint32 iHead;
	TUint32 iTail;
	TUint32 iSplitted;
	TUint32 iAlignmentOverhead;
	TBool iReadModifyWrite;
	TDfc iTransactionLaunchDfc;
	};

DPhysicalDeviceMediaStaticRD::DPhysicalDeviceMediaStaticRD()
//
// Constructor
//
	{
	__DEBUG_PRINT(">DPhysicalDeviceMediaStaticRD::DPhysicalDeviceMediaStaticRD");
	iUnitsMask=0x1;
	iVersion=TVersion(KMediaDriverInterfaceMajorVersion,KMediaDriverInterfaceMinorVersion,KMediaDriverInterfaceBuildVersion);
	}

TInt DPhysicalDeviceMediaStaticRD::Install()
//
// Install the Internal NVMem PDD.
//
	{
	__DEBUG_PRINT("DPhysicalDeviceMediaStaticRD::Install()");
	return SetName(&KNVMemPddName);
	}

void DPhysicalDeviceMediaStaticRD::GetCaps(TDes8& /*aDes*/) const
//
// Return the media drivers capabilities.
//
	{
	__DEBUG_PRINT("DPhysicalDeviceMediaStaticRD::GetCaps()");
	}

TInt DPhysicalDeviceMediaStaticRD::Create(DBase*& aChannel, TInt aMediaId, const TDesC8* /* anInfo */,const TVersion &aVer)
//
// Create an Internal Ram media driver.
//
	{
	__DEBUG_PRINT("DPhysicalDeviceMediaStaticRD::Create()");
	if (!Kern::QueryVersionSupported(iVersion,aVer))
		return KErrNotSupported;
	TInt r=KErrNoMemory;
	DMediaDriverStaticRD* pD=new DMediaDriverStaticRD(aMediaId);
	aChannel=pD;
	if (pD)
		{
		r=pD->DoCreate(aMediaId);
		}
	if( r==KErrNone )
		{
		pD->OpenMediaDriverComplete(KErrNone);
		}

	return r;
	}

TInt DPhysicalDeviceMediaStaticRD::Validate(TInt aDeviceType, const TDesC8* /*anInfo*/, const TVersion& aVer)
	{
	__DEBUG_PRINT("DPhysicalDeviceMediaStaticRD::Validate()");
	if (!Kern::QueryVersionSupported(iVersion,aVer))
		return KErrNotSupported;
	if (aDeviceType!=EFixedMedia1)
		return KErrNotSupported;
	return KErrNone;
	}

TInt DPhysicalDeviceMediaStaticRD::Info(TInt aFunction, TAny*)
//
// Return the priority of this media driver
//
	{
	__DEBUG_PRINT("DPhysicalDeviceMediaStaticRD::Info()");
	if (aFunction==EPriority)
		return KMediaDriverPriorityNormal;
	return KErrNotSupported;
	}

DMediaDriverStaticRD::DMediaDriverStaticRD(TInt aMediaId)
//
// Constructor.
//
	:	DMediaDriver(aMediaId),
		iSessionEndDfc(DMediaDriverStaticRD::SessionEndDfc, this, 1),
		iTransferBufferPhys(0),
		iTransactionLaunchDfc(DMediaDriverStaticRD::TransactionLaunchDfc, this, KMaxDfcPriority)
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::DMediaDriverStaticRD()");
	}

TInt DMediaDriverStaticRD::DoCreate(TInt /*aMediaId*/)
//
// Create the media driver.
//
	{
	__DEBUG_PRINT(">DMediaDriverStaticRD::DoCreate");
	TInt r = KErrNone; 
	// Inform our size
	Int64 size=GetNVMemSize()<<KDiskSectorShift;
	if( size<=0 )
		{
		Kern::Fault("DMediaDriverStaticRD zero size nv memory array", 0);
		}
	SetTotalSizeInBytes(size);
	// Some dfc initialization
	if( r==KErrNone )
		{
		iSessionEndDfc.SetDfcQ( this->iPrimaryMedia->iDfcQ );
		iTransactionLaunchDfc.SetDfcQ( this->iPrimaryMedia->iDfcQ );
		}
	// Create our piece of physically contiguous transfer buffer. 
	r = Epoc::AllocPhysicalRam( KNVMemTransferBufferSize, iTransferBufferPhys );
	if( r != KErrNone )
		{
		Kern::Fault("DMediaDriverStaticRD Allocate Ram %d",r);
		}

    DPlatChunkHw* bufChunk = NULL;
    DPlatChunkHw* bufChunk2 = NULL;

    // Create HW Memory Chunk
    r = DPlatChunkHw::New( bufChunk, iTransferBufferPhys, KNVMemTransferBufferSize, EMapAttrUserRw | EMapAttrFullyBlocking );

    if( r != KErrNone )
        {
        // Check Physical Memory
        if( iTransferBufferPhys )
			{
            // Free Physical Memory
            Epoc::FreePhysicalRam( iTransferBufferPhys, KNVMemTransferBufferSize );
			}
		Kern::Fault("DMediaDriverStaticRD error in creating transfer buffer", r);
        }
    

    // Set Main Buffer Pointer
    iTransferBufferLin = reinterpret_cast<TUint8*>(bufChunk->LinearAddress());

    
    
    // Create HW Memory Chunk for drive image
       r = DPlatChunkHw::New( bufChunk2, 0x81000000, 0x1000000, EMapAttrUserRw | EMapAttrFullyBlocking );
       if( r != KErrNone )
              {
			   Kern::Fault("DMediaDriverStaticRD error in creating disc image linear address", r);
              }
            
           
       // Set Main Buffer Pointer
       iDiscBufferLin = reinterpret_cast<TUint8*>(bufChunk2->LinearAddress());

       
  	// Inform "hardware" about the shared memory
  	//WriteReg( KHwNVMemoryDevice, R_NVMEM_SHARED_MEMORY_BASE, iTransferBufferPhys );
  	//WriteReg( KHwNVMemoryDevice, R_NVMEM_SHARED_MEMORY_SIZE, KNVMemTransferBufferSize );
  	//WriteReg( KHwNVMemoryDevice, R_NVMEM_ENABLE, 1 );

	// Set up interrupt service
  	//r = Interrupt::Bind( EIntNVMemoryDevice, Isr, this );
  	//Interrupt::Enable( EIntNVMemoryDevice );


	__DEBUG_PRINT("<DMediaDriverStaticRD::DoCreate %d", r);
	return(r);
	}

TInt DMediaDriverStaticRD::Request(TLocDrvRequest& m)
	{
	TInt request=m.Id();
	__DEBUG_PRINT(">DMediaDriverStaticRD::Request %d",request);
	TInt r=KErrNone;
	
	// Requests that can be handled synchronously
	if( request == DLocalDrive::ECaps ) 
		{
		r=Caps(m);
		return r;
		}

	// All other requests must be deferred if a request is currently in progress
	if (iCurrentRequest)
		{
		// a request is already in progress, so hold on to this one
		r = KMediaDriverDeferRequest;
		}
	else
		{

		iCurrentRequest=&m;
		iProsessedLength = 0;
		iHead = 0;
		iTail = 0;
		iSplitted = 0;
		iAlignmentOverhead = 0;

		iPos = m.Pos();
		iTotalLength = m.Length();


		__DEBUG_PRINT(">DMediaDriverStaticRD::Request pos:0x%lx len:0x%lx", iPos, iTotalLength);
		if( iTotalLength<0 || iPos<0 )
			{
			Kern::Fault("DMediaDriverStaticRD::Request: illegal access!", 0);
			}
		// Handle unaligned operations
		iHead = iPos % KDiskSectorSize;
		TUint32 tailAlignment = ((iHead + iTotalLength) % KDiskSectorSize);
		if( tailAlignment )
			{
			iTail = KDiskSectorSize - tailAlignment;
			}

		iSplitted = (iTotalLength + iHead + iTail) / KNVMemTransferBufferSize;

		__DEBUG_PRINT(">DMediaDriverStaticRD::Request head: %d tail: %d splitted: %d\n", iHead, iTail, iSplitted );
		__DEBUG_PRINT(">DMediaDriverStaticRD::Request partitionlen: %lx", iCurrentRequest->Drive()->iPartitionLen );

		if( (iTotalLength + iPos) > iCurrentRequest->Drive()->iPartitionLen )
			{
			Kern::Fault("DMediaDriverStaticRD::Request: Access over partition boundary!", 0);
			}
		if( iTotalLength > KMaxTInt32 )
			{
			Kern::Fault("DMediaDriverStaticRD::Request: Access length overflow!", 0);
			}
		switch (request)
			{
			case DLocalDrive::ERead:
			case DLocalDrive::EWrite:
			case DLocalDrive::EFormat:
				__DEBUG_PRINT("DMediaDriverStaticRD::Request iTransactionLaunchDfc.Enque()>");
				iTransactionLaunchDfc.Enque();
				__DEBUG_PRINT("DMediaDriverStaticRD::Request iTransactionLaunchDfc.Enque()<");
			break;
			case DLocalDrive::EEnlarge:
			case DLocalDrive::EReduce:
			default:
				r=KErrNotSupported;
			break;
			}
		}

	__DEBUG_PRINT("<DMediaDriverStaticRD::Request %d",r);
	return r;
	}

void DMediaDriverStaticRD::CompleteRequest(TInt aReason)
//
// completes the request which is being currently processed
//
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::CompleteRequest() reason: %d", aReason);
	TLocDrvRequest* pR=iCurrentRequest;
	if (pR)
		{
		iCurrentRequest=NULL;
		DMediaDriver::Complete(*pR,aReason);
		}
	}

void DMediaDriverStaticRD::Disconnect( DLocalDrive* aLocalDrive, TThreadMessage* aMsg )
	{
	__DEBUG_PRINT(">DMediaDriverStaticRD::Disconnect()");
	// Complete using the default implementation
	DMediaDriver::Disconnect(aLocalDrive, aMsg);
	__DEBUG_PRINT("<DMediaDriverStaticRD::Disconnect()");
	}

void DMediaDriverStaticRD::NotifyPowerDown()
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::NotifyPowerDown()");
	// no action required
	}

void DMediaDriverStaticRD::NotifyEmergencyPowerDown()
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::NotifyEmergencyPowerDown()");
	// no action required
	}

TInt DMediaDriverStaticRD::Caps(TLocDrvRequest& m)
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::Caps()");
	TLocalDriveCapsV6& caps=*(TLocalDriveCapsV6*)m.RemoteDes();
	caps.iType=EMediaHardDisk;
	caps.iConnectionBusType=EConnectionBusInternal;
	caps.iDriveAtt=KDriveAttLocal|KDriveAttInternal;
	caps.iMediaAtt=KMediaAttFormattable;
	caps.iFileSystemId=KDriveFileSysFAT;
	caps.iPartitionType=KPartitionTypeFAT16;
	caps.iSize=m.Drive()->iPartitionLen;
	caps.iHiddenSectors=0;
	caps.iEraseBlockSize = KNVMemTransferBufferSize; 
	caps.iBlockSize=KDiskSectorSize;
	caps.iMaxBytesPerFormat = KNVMemTransferBufferSize;

	return KErrCompletion;									
	}

TInt DMediaDriverStaticRD::Read()
	{
	__DEBUG_PRINT(">DMediaDriverStaticRD::Read() pos: %lx, size: %lx", iPos, iTotalLength);
	// Set our sector offset
	TUint32 transactionSectorOffset = (TUint32)(iPos / KDiskSectorSize); 
	TUint32 transactionLength = 0;
	TUint32 transactionDirection = NVMEM_TRANSACTION_READ;
	// Do we have an operation longer than our shared memory?
	if( iSplitted > 0 )
		{
		transactionLength = KNVMemTransferBufferSize;
		}
	else
		{
		// Do the whole operation in one go since we have enough room in our memory
		transactionLength = I64LOW(iTotalLength);
		// Read the "broken" tail sector
		if( iTail )
			{
			transactionLength += iTail;
			iAlignmentOverhead += iTail;
			}
		}
	// Read the "broken" head sector
	if( iHead > 0 )
		{
		transactionLength += iHead;
		iAlignmentOverhead += iHead;
		}

	// We should be ok to continue
	ContinueTransaction( transactionSectorOffset, transactionLength/KDiskSectorSize, transactionDirection );
	__DEBUG_PRINT("<DMediaDriverStaticRD::Read()");
	return KErrNone;
	}

TInt DMediaDriverStaticRD::Write()
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::Write() pos: 0x%lx, size: 0x%lx", iPos, iTotalLength);
	TInt r = KErrNone;
	// Set our sector offset
	TUint32 transactionSectorOffset = (TUint32)(iPos / KDiskSectorSize); 
	TUint32 transactionLength = 0;
	TUint32 transactionDirection = NVMEM_TRANSACTION_WRITE;
	// Do we have an operation longer than our shared memory?
	if( iSplitted > 0 )
		{
		transactionLength = KNVMemTransferBufferSize;
		}
	else
		{
		// Do the whole operation in one go since we have enough room in our memory
		transactionLength = I64LOW(iTotalLength);
		if( iTail )
			{
			iReadModifyWrite = ETrue;
			// Read the "broken" tail sector
			transactionLength += iTail;
			iAlignmentOverhead += iTail;
			}
		}
	// Is there a need to read modify write the "broken" head sector of the operation
	if( iHead > 0 )
		{
		iReadModifyWrite = ETrue;
		// If splitted operation we only need the broken sector
		if( iSplitted > 0 )
			{
			transactionLength = KDiskSectorSize;
			iAlignmentOverhead += iHead;
			}
		else
			{
			// Read the "broken" head sector in addition to everything else
			transactionLength += iHead;
			iAlignmentOverhead += iHead;
			}
		}
	
	// Was there a need to read-modify before writing
	if( iReadModifyWrite )
		{
		transactionDirection = NVMEM_TRANSACTION_READ;
		}
	else
		{
		// Handle format here
		if( iCurrentRequest->Id() == DLocalDrive::EFormat )
			{
			// Not much handling just flow through since we have filled the shared memory with zeroes already
			}
		else
			{
			// Read from client
			TPtr8 targetDescriptor(iTransferBufferLin, transactionLength);
			r = iCurrentRequest->ReadRemote(&targetDescriptor,0);
			}
		}
	
	// We should be ok to continue
	ContinueTransaction( transactionSectorOffset, transactionLength/KDiskSectorSize, transactionDirection );

	return r;
	}


TInt DMediaDriverStaticRD::Format()
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::Format() pos: 0x%lx, size: 0x%lx", iPos, iTotalLength);
	memset( iTransferBufferLin, 0x00, KNVMemTransferBufferSize );
	// Stop the nonsense here. Write operations should be used for partial sector data removal operations
	if( iHead > 0 || iTail > 0 )
		{
		Kern::Fault("DMediaDriverStaticRD::Format: alignment violation!", 0);
		}
	Write();
//	DoTransaction( m, NVMEM_TRANSACTION_WRITE );
	return KErrNone;
	}

TInt DMediaDriverStaticRD::ContinueTransaction( TUint32 aTransactionSectorOffset, TUint32 aTransactionSectorCount, TUint32 aTransactionDirection )
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::ContinueTransaction() sectoroffset: %d, sectorcount: %d, direction: %d", aTransactionSectorOffset, aTransactionSectorCount, aTransactionDirection);
	if( aTransactionDirection != NVMEM_TRANSACTION_UNDEFINED )
		{
	  	//WriteReg( KHwNVMemoryDevice, R_NVMEM_TRANSACTION_OFFSET, aTransactionSectorOffset );
	  	//WriteReg( KHwNVMemoryDevice, R_NVMEM_TRANSACTION_SIZE, aTransactionSectorCount );
	  	//WriteReg( KHwNVMemoryDevice, R_NVMEM_TRANSACTION_DIRECTION, aTransactionDirection );
	  	//WriteReg( KHwNVMemoryDevice, R_NVMEM_TRANSACTION_EXECUTE, aTransactionDirection );
		if ( aTransactionDirection == NVMEM_TRANSACTION_WRITE )
		{
			memcpy( (TAny *)(iDiscBufferLin+(aTransactionSectorOffset<<9)), iTransferBufferLin, aTransactionSectorCount*512 );
		}
		else if ( aTransactionDirection == NVMEM_TRANSACTION_READ )
		{
			memcpy( iTransferBufferLin, (TAny *)(iDiscBufferLin+(aTransactionSectorOffset<<9)), aTransactionSectorCount*512 );
		}
		iLatestTransferSectorCount = aTransactionSectorCount;
		Isr(this); // terrible hack, we've yransferred all the sectors and now we pretend to generate an interrupt
		}
	else
		{
		Kern::Fault("DMediaDriverStaticRD::ContinueTransaction: Undefined transaction!", 0);
		}
	return KErrNone;
	}


TInt DMediaDriverStaticRD::PartitionInfo(TPartitionInfo& anInfo)
//
// Return partition information on the media.
//
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::PartitionInfo()");
	anInfo.iPartitionCount=1;
	anInfo.iEntry[0].iPartitionBaseAddr=0;
	anInfo.iEntry[0].iPartitionLen=anInfo.iMediaSizeInBytes=TotalSizeInBytes();
	anInfo.iEntry[0].iPartitionType=KPartitionTypeFAT16;
	return KErrCompletion;
	}

void DMediaDriverStaticRD::TransactionLaunchDfc(TAny* aMediaDriver)
	{
	static_cast<DMediaDriverStaticRD*>(aMediaDriver)->DoTransactionLaunchDfc();
	}

void DMediaDriverStaticRD::DoTransactionLaunchDfc()
	{
	__DEBUG_PRINT(">DMediaDriverStaticRD::DoTransactionLaunchDfc()");
	TInt request = iCurrentRequest->Id();
	TInt r(KErrNone);
	switch (request)
		{
		case DLocalDrive::ERead:
			r=Read();
		break;
		case DLocalDrive::EWrite:
			r=Write();
		break;
		case DLocalDrive::EFormat:
			r=Format();
		break;
		case DLocalDrive::EEnlarge:
		case DLocalDrive::EReduce:
		default:
			r=KErrNotSupported;
		break;
		}
	if( r != KErrNone )
		{
		// TODO some proper error handling here
		}
	__DEBUG_PRINT("<MediaDriverStaticRD::DoTransactionLaunchDfc %d",r);
	}

void DMediaDriverStaticRD::SessionEndDfc(TAny* aMediaDriver)
	{
	static_cast<DMediaDriverStaticRD*>(aMediaDriver)->DoSessionEndDfc();
	}

void DMediaDriverStaticRD::DoSessionEndDfc()
	{
	__DEBUG_PRINT(">DMediaDriverStaticRD::DoSessionEndDfc()");
	TInt r = KErrNone;
	// Check that we have a request in process
	if( iCurrentRequest )
		{
		// Transaction variables
		TUint32 transactionSectorOffset(0);
		TUint32 transactionLength(0);
		TUint32 transactionDirection( NVMEM_TRANSACTION_UNDEFINED );
		// How much did we actually transfer? 
		TUint32 latestTransferSize = (iLatestTransferSectorCount * KDiskSectorSize);
		__DEBUG_PRINT("DMediaDriverStaticRD::DoSessionEndDfc() latestTransferSize: %d", latestTransferSize );
		// Subtract alignment overhead
		latestTransferSize = latestTransferSize - iAlignmentOverhead;
		// For decision whether the buffer is ready for operation already
		TBool bufferReady(EFalse);
		// For decision whether we have finished the latest request
		TBool sessionComplete(EFalse);

		// Was there a read-modify-write (RWM) for which we need to do some buffer manipulation before proceeding?
		// Note that in case of format we triggered to alignment violation in earlier method already and can not enter to following 
		// condition when there is a format operation going on
		if( iReadModifyWrite )
			{
			bufferReady = ETrue;
			iReadModifyWrite = EFalse;
			// Was it a splitted operation for which we only need to take care of the broken head sector.
			if( iSplitted > 0 )
				{
				// We have a sector here here filled with data from mass memory. Modify with client data.
				__DEBUG_PRINT("DMediaDriverStaticRD::DoSessionEndDfc() readremote splitted: %d head: %d", latestTransferSize, iHead );
				TPtr8 targetDescriptor(&iTransferBufferLin[iHead], KNVMemTransferBufferSize - iHead);
				r = iCurrentRequest->ReadRemote(&targetDescriptor,0);
				}
			// Else we need to take care of both head and tail
			else
				{
				// We have a piece of data read from mass memory. Modify with client data.
				__DEBUG_PRINT("DMediaDriverStaticRD::DoSessionEndDfc() readremote: %d head: %d", I64LOW(iTotalLength - iProsessedLength), iHead );
				TPtr8 targetDescriptor(&iTransferBufferLin[iHead], I64LOW(iTotalLength - iProsessedLength));
				r = iCurrentRequest->ReadRemote(&targetDescriptor,0);
//				latestTransferSize -= (KDiskSectorSize - iTail);
				iTail = 0;
				}
			}
		else
			{
			// Overhead is processed when we enter here
			iAlignmentOverhead = 0;
			// Update position
			iPos += latestTransferSize;
			// Save the information on how many bytes we transferred already
			iProsessedLength += latestTransferSize; 
			// Update the splitted information. We don't take head into account here anymore since it is already taken care of
			iSplitted = (iTotalLength - iProsessedLength + iTail) / KNVMemTransferBufferSize;
			// Check if we have done already
			if( iProsessedLength >= iTotalLength )
				{
				// If this was the final transfer for this request let's take tail into account as well (if not taken already)
				// iProsessedLength -= iTail;
				// latestTransferSize -= iTail;
				if( iProsessedLength > iTotalLength )
					{
					Kern::Fault("DMediaDriverStaticRD: Illegal transfer operation!", 0);
					}
				sessionComplete = ETrue;
				}
			}

		TInt request = iCurrentRequest->Id();

		// Set our sector offset
		transactionSectorOffset = (TUint32)(iPos / KDiskSectorSize); 
		
		if( bufferReady )
			{
			// Write as much as we read in RMW operation
			transactionLength = (iLatestTransferSectorCount * KDiskSectorSize);
			}
		else
			{
			// Do we have an operation longer than our shared memory?
			if( iSplitted > 0 )
				{
				transactionLength = KNVMemTransferBufferSize;
				}
			else
				{
				// Do the whole operation in one go since we have enough room in our memory
				transactionLength = I64LOW(iTotalLength - iProsessedLength);
				// Read the "broken" tail sector
				if( iTail > 0 && request == DLocalDrive::EWrite )
					{
					iReadModifyWrite = ETrue;
					// Read the "broken" tail sector
					transactionLength += iTail;
	   				iAlignmentOverhead = iTail;
	   				}
				}
			}
		
		// Was there a need to read-modify before writing
		if( iReadModifyWrite )
			{
			transactionDirection = NVMEM_TRANSACTION_READ;
			}
		else
			{
			if( request == DLocalDrive::ERead )
				{
				transactionDirection = NVMEM_TRANSACTION_READ;
				// Write to client
				__DEBUG_PRINT("DMediaDriverStaticRD::DoSessionEndDfc() WriteRemote: %d head: %d", latestTransferSize, iHead );
				TPtrC8 sourceDescriptor(&iTransferBufferLin[iHead], latestTransferSize);
				r = iCurrentRequest->WriteRemote( &sourceDescriptor, 0 );
				}
			// Head is processed
			iHead = 0;
			if( request == DLocalDrive::EWrite && !sessionComplete )
				{
				transactionDirection = NVMEM_TRANSACTION_WRITE;
				if( bufferReady )
					{
					// Actually no need for any actions here
					}
				else
					{
					// Prepare a buffer for transfer
					__DEBUG_PRINT("DMediaDriverStaticRD::DoSessionEndDfc() ReadRemote: %d head: %d", latestTransferSize, iHead );
					TPtr8 targetDescriptor(iTransferBufferLin, transactionLength);
					r = iCurrentRequest->ReadRemote(&targetDescriptor,0);
					}
				}
			if( request == DLocalDrive::EFormat )
				{
				transactionDirection = NVMEM_TRANSACTION_WRITE;
				}
			}
		if( sessionComplete )
			{
			CompleteRequest( r );
			}
		else
			{
			ContinueTransaction( transactionSectorOffset, transactionLength/KDiskSectorSize, transactionDirection );
			}
		}
	else
		{
		// Let's just flow through for now
		}

	__DEBUG_PRINT("<DMediaDriverStaticRD::DoSessionEndDfc()" );
	}


TUint32  DMediaDriverStaticRD::GetNVMemSize( void )
	{
	__DEBUG_PRINT("DMediaDriverStaticRD::GetNVMemSize()");
	TUint32 sizeInSectors = 32768;//ReadReg( KHwNVMemoryDevice, R_NVMEM_NV_MEMORY_SIZE );
	return sizeInSectors; 
	}

void DMediaDriverStaticRD::Isr(TAny* aPtr)
	{
	__DEBUG_PRINT(">DMediaDriverStaticRD::Isr");

    DMediaDriverStaticRD* nvMem = reinterpret_cast<DMediaDriverStaticRD*>(aPtr);

	// Save the amount of transferred sectors. This clears the interrupt from HW as well
	//nvMem->iLatestTransferSectorCount = ReadReg( KHwNVMemoryDevice, R_NVMEM_STATUS );

	// Clear from framework
	//Interrupt::Clear( EIntNVMemoryDevice );

	nvMem->iSessionEndDfc.Add();
	}


DECLARE_EXTENSION_PDD()
	{
	__DEBUG_PRINT(">MediaDriverStaticRD create device");
	return new DPhysicalDeviceMediaStaticRD;
	}

static const TInt NVMemDriveNumbers[NVMEM1_DRIVECOUNT]={NVMEM1_DRIVELIST};	

DECLARE_STANDARD_EXTENSION()
	{
	__DEBUG_PRINT("Registering STATICRD drive");
	TInt r=KErrNoMemory;

	DPrimaryMediaBase* pM=new DPrimaryMediaBase;

	TDynamicDfcQue* NVMemoryDfcQ;
	 
	r = Kern::DynamicDfcQCreate( NVMemoryDfcQ, KNVMemDfcThreadPriority, KNVMemDriveName );

	if( r == KErrNone )
		{
		pM->iDfcQ = NVMemoryDfcQ;
		}
	else
		{
		__DEBUG_PRINT("NVMEM DFCQ initialization failed");
		}

	if (pM)

		{
		r=LocDrv::RegisterMediaDevice(EFixedMedia1,NVMEM1_DRIVECOUNT,&NVMemDriveNumbers[0],pM,NVMEM1_NUMMEDIA,KNVMemDriveName);
		}

	pM->iMsgQ.Receive();

	__DEBUG_PRINT("Registering NVMEM drive - return %d",r);

	return r;
	}
