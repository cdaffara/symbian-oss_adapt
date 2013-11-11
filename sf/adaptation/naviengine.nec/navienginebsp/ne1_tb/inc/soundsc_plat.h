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
* ne1_tb\inc\soundsc_plat.h
* Definitions for the NE1_TBVariant shared chunk sound physical device driver (PDD).
*
*/



/**
 @file
 @internalTechnology
 @prototype
*/

#ifndef __SOUNDSC_PLAT_H__
#define __SOUNDSC_PLAT_H__

#include <dma.h>
#include <soundsc.h>
#include <i2s.h>
#include "cs42l51.h"

// macro to print out thread context and CPU that thread is running on..
#define __THREAD_AND_CPU 	Kern::Printf("(Thread %T, CPU: %d)\n", NKern::CurrentThread(), NKern::CurrentCpu())
#define __KTRACE_SND(s) __KTRACE_OPT(KSOUND1, s)
//#define __KTRACE_SND(s) s

// Fill in the maximum number of requests that may be outstanding on the playback and record DMA channels for this device.
const TInt KMaxDmaRequests=2;

// Fill in the maximum transfer length supported  on the playback and record DMA channels for this device.
const TInt KMaxDmaTransferLen=0x2000;

// fifo threshold values for I2S transfers..
const TInt KFifoThreshold = 8;

// Forward declarations
class DNE1_TBSoundScDmaRequest;

// Define the I2S channel number used by this driver 
const TUint KI2sChanNum = 0;
/**
Factory class instantiated from ordinal 0.
The NE1_TBVariant physical device for the shared chunk sound driver used to create the DSoundScPdd-derived channel objects.
*/
class DSoundScPddNE1_TB : public DPhysicalDevice
	{
public:
	DSoundScPddNE1_TB();
	~DSoundScPddNE1_TB();
	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DBase*& aChannel, TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);
	virtual TInt Validate(TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);
private:
	// The DFC queue (used also by the LDD).
	TDynamicDfcQue* iDfcQ;

	// A pointer to the audio codec object
	RCS42AudioCodec* iCodec;

	friend class DNE1_TBSoundScPddChannel;
	};

/**
The NE1_TBVariant physical device driver (PDD) for the playback shared chunk sound driver.
*/
class DNE1_TBSoundScPddChannel : public DSoundScPdd
	{
public:
	explicit DNE1_TBSoundScPddChannel(TSoundDirection aSoundDirection);
	~DNE1_TBSoundScPddChannel();
	TInt DoCreate();

	// Implementations of the pure virtual functions inherited from DSoundScPdd (called by LDD).
	virtual TDfcQue* DfcQ(TInt aUnit);
	virtual void GetChunkCreateInfo(TChunkCreateInfo& aChunkCreateInfo);
	virtual void Caps(TDes8& aCapsBuf) const;
	virtual TInt MaxTransferLen() const;
	virtual TInt SetConfig(const TDesC8& aConfigBuf);
	virtual TInt SetVolume(TInt aVolume);
	virtual TInt StartTransfer();
	virtual TInt TransferData(TUint aTransferID,TLinAddr aLinAddr,TPhysAddr aPhysAddr,TInt aNumBytes);
	virtual void StopTransfer();
	virtual TInt PauseTransfer();
	virtual TInt ResumeTransfer();
	virtual TInt PowerUp();
	virtual void PowerDown();
	virtual TInt CustomConfig(TInt aFunction,TAny* aParam);
	void PlayCallback(TUint aTransferID,TInt aTransferResult,TInt aBytesTransferred);

private:
	void SetCaps();
	static void PowerUpCallback (TAny *aArg);

private:
	// A pointer to the PDD factory.
	DSoundScPddNE1_TB* iPhysicalDevice;
	// The capabilities of this device.
	TSoundFormatsSupportedV02 iCaps;
	// The playback DMA channel.
	TDmaChannel* iDmaChannel;
	// The DMA request structures used for transfers.
	DNE1_TBSoundScDmaRequest* iDmaRequest[KMaxDmaRequests];
	// The number of outstanding DMA play requests on the DMA channel.
	TInt iPendingPlay;
	// A flag selecting the next DMA request for transfer.
	TInt iFlag;
	// current configuration
	TCurrentSoundFormatV02 iCurrentConfig;

	// I2S direction (I2s::ETx or I2s::ERx) of this unit
	I2s::TI2sDirection iI2sDirection;

	// Dfc, semaphore and status -used to power up the Codec in the context of the driver thread
	TDfc iPowerUpDfc;
	static NFastSemaphore iFastSem;
	TInt iPowerUpStatus;
	
	friend class DSoundScPddNE1_TB;
	friend class DNE1_TBSoundScDmaRequest;
	};

/**
Wrapper function for a shared chunk sound driver DMA request.
*/
class DNE1_TBSoundScDmaRequest : public DDmaRequest
	{
public:
	DNE1_TBSoundScDmaRequest(TDmaChannel& aChannel,DNE1_TBSoundScPddChannel* aPdd,TInt aMaxTransferSize=0);
	~DNE1_TBSoundScDmaRequest();
	TInt CreateMonoBuffer();
	TInt SetDmaTransfer(TUint aTransferID, TLinAddr aLinAddr, TInt aNumBytes);
	static void DmaService(TResult aResult, TAny* aArg);
public:
	// Pointer back to the PDD.
	DNE1_TBSoundScPddChannel* iPdd;
	// The transfer ID for this DMA request - supplied by the LDD.
	TUint iTransferID;
	// The transfer sizes in progress.
	TUint iTransferSize;

private:
	// To facilitate mono support:
	// Linear start address of the data supplied by the client..
	TLinAddr iAddrLinOrig;

	// buffers needed to store mono samples
	TPhysAddr iBuffPhys;
	TLinAddr iBufLin;
	DPlatChunkHw* iChunk;
	};

#endif /* __SOUNDSC_PLAT_H__ */
