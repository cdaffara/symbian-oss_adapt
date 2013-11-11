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
* \bsp\hwip_nec_naviengine\ne1_tb\soundsc\soundsc_channel.cpp
* Implementation of the NE1_TBVariant playback and record shared chunk sound physical device driver (PDD).
* This file is part of the NE1_TBVariant Base port
* Unit is identified by the time of its creation by the DSoundScPddNE1_TB - by specifying TSoundDirection
* (either ESoundDirRecord or ESoundDirPlayback) as the constructor parameter.
*
*/



/**
 @file
*/
#include <navienginedma.h>
#include <i2s.h>
#include <nkern.h>
#include "soundsc_plat.h"

#if _DEBUG
static const char KSoundPDDPanicCat[] = "SOUNDSC PDD, line:";
#endif

// physical address of the I2S channel 0 Tx register
const TUint32 KHwI2S0TxPhys = KHwI2S0Phys + KHoI2STx;

// physical address of the I2S channel 0 Rx register
const TUint32 KHwI2S0RxPhys = KHwI2S0Phys + KHoI2SRx;

/**
Constructor for the NE1_TBVariant playback shared chunk sound driver physical device driver (PDD).
*/
DNE1_TBSoundScPddChannel::DNE1_TBSoundScPddChannel(TSoundDirection aSoundDirection) :
	iPowerUpDfc(PowerUpCallback, this, 0)
	{
	// The data transfer direction for this unit is specified as the constuctor parameter
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::DNE1_TBSoundScPddChannel()", aSoundDirection));

	iCaps.iDirection = aSoundDirection;

	// store direction for I2s calls
	if(aSoundDirection == ESoundDirRecord)
		{
		iI2sDirection = I2s::ERx;
		}
	else
		{
		iI2sDirection = I2s::ETx;
		}
	}

/**
Destructor for the NE1_TBVariant playback shared chunk sound driver physical device driver (PDD).
*/
DNE1_TBSoundScPddChannel::~DNE1_TBSoundScPddChannel()
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::~DNE1_TBSoundScPddChannel()", iCaps.iDirection));

	// Delete the DMA request objects
	for (TInt i=0; i<KMaxDmaRequests; i++)
		{
		delete iDmaRequest[i];
		}

	// Close the DMA channel.
	if (iDmaChannel)
		{
		iDmaChannel->Close();
		}
	}

/**
Second stage constructor for the NE1_TBVariant playback shared chunk sound driver physical device driver (PDD).
Note that this constructor is called before the second stage constructor for the LDD so it is not
possible to call methods on the LDD here.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DNE1_TBSoundScPddChannel::DoCreate()
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::DoCreate", iCaps.iDirection));
	TInt r = KErrNone;

	// Setup the capabilities of this device.
	SetCaps();

	if (iCaps.iDirection == ESoundDirRecord)
		{
		iPowerUpDfc.SetDfcQ(DfcQ(KSoundScRxUnit0));
		}
	else
		{
		iPowerUpDfc.SetDfcQ(DfcQ(KSoundScTxUnit0));
		}

	// Setup the DMA channel information for this channel.
	// note, that the channel type (Playback/Record) is stored in the iDirection
	// and the I2s::TI2sDirection in iI2sDirection.
	TDmaChannel::SCreateInfo info;
	if (iCaps.iDirection == ESoundDirRecord)
		{
		info.iCookie = EDMAChannelI2S0RX;
		}
	else
		{
		info.iCookie = EDMAChannelI2S0TX;
		}

	if (iCaps.iDirection == ESoundDirRecord)
		{
		info.iDfcQ = DfcQ(KSoundScRxUnit0);
		}
	else
		{
		info.iDfcQ = DfcQ(KSoundScTxUnit0);
		}

	info.iDfcPriority = 0; // and set priority to 0 (the same as for RX channel)
	info.iDesCount = KMaxDmaRequests;

	// Try to open the DMA channel for a given direction (Playback or Record specified in iCookie)
	// If this channel was already opened at this point - the DMA framework will return KErrInUse.
	r = TDmaChannel::Open(info, iDmaChannel);
	if (r != KErrNone)
		{
		return r;
		}

	// Create the DMA request objects for use with the DMA channel.
	for (TInt i = 0; i < KMaxDmaRequests; i++)
		{
		iDmaRequest[i] = new DNE1_TBSoundScDmaRequest(*iDmaChannel,this, 0);
		if (iDmaRequest[i] == NULL)
			{
			return KErrNoMemory;
			}

		r = iDmaRequest[i]->CreateMonoBuffer();
		if (r != KErrNone)
			{
			return r;
			}
		}

	// initialize the hardware FIFO of the I2S bus for this particular direction (iI2sDIrection).
	// Because on this bus - both channels' (left and right) FIFO can't be enabled separately
	// it is enough to call EnableFIFO for either of them (I2s::ELeft in this case).
	r = I2s::EnableFIFO(KI2sChanNum, I2s::ELeft, iI2sDirection);
	if (r != KErrNone)
		{
		return r;
		}

	// set the I2S bus hardware FIFO threshold for each channel of a given direction
	r = I2s::SetFIFOThreshold(KI2sChanNum, I2s::ELeft, iI2sDirection, KFifoThreshold);
	if (r != KErrNone)
		{
		return r;
		}

	r = I2s::SetFIFOThreshold(KI2sChanNum, I2s::ERight, iI2sDirection, KFifoThreshold);
	if (r != KErrNone)
		{
		return r;
		}


	return KErrNone;
	}

/**
Return the DFC queue to be used by this playback device.
@return The DFC queue to use.
*/
TDfcQue* DNE1_TBSoundScPddChannel::DfcQ(TInt /*aUnit*/)
	{
	return(iPhysicalDevice->iDfcQ);
	}

/**
Called from the LDD to return the shared chunk create information to be used by this play device.
@param aChunkCreateInfo A chunk create info. object to be to be filled with the settings
						required for this device.
*/
void DNE1_TBSoundScPddChannel::GetChunkCreateInfo(TChunkCreateInfo& aChunkCreateInfo)
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::GetChunkCreateInfo", iCaps.iDirection));

	// Setup the shared chunk create information in aChunkCreateInfo for this play device.
	aChunkCreateInfo.iType = TChunkCreateInfo::ESharedKernelMultiple;
	aChunkCreateInfo.iMapAttr = EMapAttrFullyBlocking | EMapAttrWriteUser; // not cached, user writable
	aChunkCreateInfo.iOwnsMemory = ETrue; 	// Using RAM pages.
	aChunkCreateInfo.iDestroyedDfc = NULL; 	// No chunk destroy DFC.
	}

/**
Called from the LDD to return the capabilities of this device.
@param aCapsBuf A packaged TSoundFormatsSupportedV02 object to be filled with the play
				capabilities of this device. This descriptor is in kernel memory and can be accessed directly.
@see TSoundFormatsSupportedV02.
*/
void DNE1_TBSoundScPddChannel::Caps(TDes8& aCapsBuf) const
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::Caps", iCaps.iDirection));

	// Copy iCaps back.
	TPtrC8 ptr((const TUint8*)&iCaps,sizeof(iCaps));
	aCapsBuf.FillZ(aCapsBuf.MaxLength());
	aCapsBuf=ptr.Left(Min(ptr.Length(),aCapsBuf.MaxLength()));
	}

/**
Called from the LDD to return the maximum transfer length in bytes that this device can support in a single data transfer.
@return The maximum transfer length in bytes.
*/
TInt DNE1_TBSoundScPddChannel::MaxTransferLen() const
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::MaxTransferLen() %x (%d)", iCaps.iDirection,KMaxDmaTransferLen,KMaxDmaTransferLen));
	return(KMaxDmaTransferLen);
	}

/**
Called from the LDD to configure or reconfigure the device using the the configuration supplied.
@param aConfigBuf A packaged TCurrentSoundFormatV02 object which contains the new configuration settings.
				  This descriptor is in kernel memory and can be accessed directly.
@return KErrNone if successful, otherwise one of the other system wide error codes.
@see TCurrentSoundFormatV02.
*/
TInt DNE1_TBSoundScPddChannel::SetConfig(const TDesC8& aConfigBuf)
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::SetConfig", iCaps.iDirection));
	TInt r=KErrNone;

	// Read the new configuration from the LDD.
	TCurrentSoundFormatV02 config;
	TPtr8 ptr((TUint8*)&config,sizeof(config));
	Kern::InfoCopy(ptr,aConfigBuf);

	// Set the I2S interface as bidirectional and master
	TI2sConfigV01 i2sconfig = {I2s::EMaster, I2s::EBidirectional};
	TPckgBuf<TI2sConfigV01> i2sconf(i2sconfig);

	r = I2s::ConfigureInterface(KI2sChanNum, &i2sconf);
	if(r != KErrNone)
		{
		return r;
		}

	// Apply the specified audio configuration to the audio device.
	if(config.iChannels > 2)
		{
		return KErrNotSupported;
		}

	r = I2s::EnableDMA(KI2sChanNum, iI2sDirection);
	if(r != KErrNone)
		{
		return r;
		}

	switch (config.iEncoding)
		{
		case ESoundEncoding16BitPCM:
			r = I2s::SetSampleLength(KI2sChanNum, I2s::ELeft, I2s::ESample16Bit);
			if(r!=KErrNone)
				{
				break;
				}
			r = I2s::SetFrameLengthAndFormat(KI2sChanNum, I2s::EFrame48Bit,  16);
			break;
		default:
			r =  KErrNotSupported;
		}

	// might be also be'KErrInUse' here - so we shouldn't continue..
	if(r!=KErrNone)
		{
		return r;
		}

	// BPS = rate * bytes_per_sample * num_of_channels
	switch(config.iRate)
		{
		case ESoundRate11025Hz:
			r = I2s::SetSamplingRate(KI2sChanNum, I2s::E11_025KHz);
			break;

		case ESoundRate22050Hz:
			r = I2s::SetSamplingRate(KI2sChanNum, I2s::E22_05KHz);
			break;

		case ESoundRate44100Hz:
			r = I2s::SetSamplingRate(KI2sChanNum, I2s::E44_1KHz);
			break;

		default:
			r = KErrNotSupported;
		}

	// if we support it - copy the new configuration
	if(r == KErrNone)
		{
		iCurrentConfig = config;
		}
	return(r);
	}

/**
Called from the LDD to set the play volume.
@param aVolume The play volume to be set - a value in the range 0 to 255. The value 255 equates
	to the maximum volume and each value below this equates to a 0.5dB step below it.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DNE1_TBSoundScPddChannel::SetVolume(TInt aVolume)
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::SetVolume", iCaps.iDirection));
	TInt r;
	// Set the specified play volume on the audio device.
	if (iCaps.iDirection == ESoundDirRecord)
		{
		r = iPhysicalDevice->iCodec->SetRecordVolume(aVolume);
		}
	else
		{
		r = iPhysicalDevice->iCodec->SetPlayVolume(aVolume);
		}
	return(r);
	}

/**
Called from the LDD to prepare the audio device for playback.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/

TInt DNE1_TBSoundScPddChannel::StartTransfer()
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::StartTransfer", iCaps.iDirection));
	TInt r = I2s::Start(KI2sChanNum, iI2sDirection);
	return(r);
	}

/**
Called from the LDD to initiate the playback of a portion of data to the audio device.
When the transfer is complete, the PDD signals this event using the LDD function PlayCallback().
@param aTransferID A value assigned by the LDD to allow it to uniquely identify a particular transfer fragment.
@param aLinAddr The linear address within the shared chunk of the start of the data to be played.
@param aPhysAddr The physical address within the shared chunk of the start of the data to be played.
@param aNumBytes The number of bytes to be played.
@return KErrNone if the transfer has been initiated successfully;
  		KErrNotReady if the device is unable to accept the transfer for the moment;
		otherwise one of the other system-wide error codes.
*/
TInt DNE1_TBSoundScPddChannel::TransferData(TUint aTransferID,TLinAddr aLinAddr,TPhysAddr /*aPhysAddr*/,TInt aNumBytes)
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::TransferData(ID:%xH,Addr:%xH,Len:%d)",iCaps.iDirection,aTransferID,aLinAddr,aNumBytes));
	TInt r = KErrNone;

	// Check that we can accept the request
	if (iPendingPlay >= KMaxDmaRequests)
		{
		return KErrNotReady;
		}
	else
		{
		// Set the DMA transfer..
		// DSoundScDmaRequest, as a friend class checks iChannels and iDirection of the transfer
		r = iDmaRequest[iFlag]->SetDmaTransfer(aTransferID, aLinAddr, aNumBytes);
		if (r != KErrNone)
			{
			__KTRACE_SND(Kern::Printf("DMA Fragment error (%d), r= %d", iCaps.iDirection, r));
			return r;
			}
		else
			{
			iDmaRequest[iFlag]->Queue();
			iPendingPlay++;
			if ((++iFlag) >= KMaxDmaRequests)
				iFlag = 0;
			}
		}
	return KErrNone;
	}

/**
Called from the LDD to terminate the playback of a data to the device and to release any resources necessary for playback.
This is called soon after the last pending play request from the client has been completed. Once this function had been
called, the LDD will not issue any further TransferData() commands without first issueing a StartTransfer() command.
*/
void DNE1_TBSoundScPddChannel::StopTransfer()
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::StopTransfer", iCaps.iDirection));

	// Stop the DMA channel.
#ifdef _DEBUG
	TInt r = I2s::Stop(KI2sChanNum, iI2sDirection);
	__ASSERT_DEBUG(r == KErrNone, Kern::Fault(KSoundPDDPanicCat, __LINE__));
#else
	I2s::Stop(KI2sChanNum, iI2sDirection);
#endif

	iDmaChannel->CancelAll();
	iFlag = 0;
	iPendingPlay = 0;
	}

/**
Called from the LDD to halt the playback of data to the sound device but not to release any resources necessary for
playback.
If possible, any active transfer should be suspended in such a way that it can be resumed later - starting from next
sample following the one last played.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DNE1_TBSoundScPddChannel::PauseTransfer()
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::PauseTransfer, pending %d", iCaps.iDirection, iPendingPlay));

	// Halt transfer on the audio device.
	TInt r = I2s::Stop(KI2sChanNum, iI2sDirection);
	if(r != KErrNone)
		{
		return r;
		}

	if (iCaps.iDirection == ESoundDirRecord)
		{
	    // for Record, we need to figure out how much data was actually
		// transfered and provide this to the LDD..
		if (iPendingPlay)
			{
			iDmaChannel->CancelAll();
			TInt byteCount = 0; // Unless dma API is extended..
			Ldd()->RecordCallback(0,KErrNone, byteCount);	// We can use a NULL transfer ID when pausing.
			iPendingPlay=0;
			}
	    iFlag=0;
		}

	return(r);
	}

/**
Called from the LDD to resume the playback of data to the sound device following a request to halt playback.
If possible, any transfer which was active when the device was halted should be resumed - starting from next sample
following the one last played. Once complete, it should be reported using PlayCallback()
as normal.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DNE1_TBSoundScPddChannel::ResumeTransfer()
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::ResumeTransfer, pending %d", iCaps.iDirection, iPendingPlay));

	// Resume playback on the audio device.
	TInt r = I2s::Start(KI2sChanNum, iI2sDirection);
	return(r);
	}


NFastSemaphore DNE1_TBSoundScPddChannel::iFastSem;

void DNE1_TBSoundScPddChannel::PowerUpCallback (TAny *aArg)
	{
	DNE1_TBSoundScPddChannel *a= (DNE1_TBSoundScPddChannel*)aArg;
	__KTRACE_SND(Kern::Printf("powerUpCallback(%d)", a->iCaps.iDirection));

	// PowerUp the Codec
	a->iPowerUpStatus = RCS42AudioCodec::Open(a->iPhysicalDevice->iCodec);

	// signal will unblock the thread blocked in call to PowerUp() method.
	NKern::FSSignal(&a->iFastSem);
	}

/**
Called from the LDD to power up the sound device when the channel
is first opened and if ever the phone is brought out of standby mode.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DNE1_TBSoundScPddChannel::PowerUp()
	{
	// Power up the audio device.
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::PowerUp", iCaps.iDirection));

	// need to power up the device in the context of the driver's thread
	// (blocking the calling thread)
	TDfcQue* dfcQ;
	if (iCaps.iDirection == ESoundDirRecord)
		{
		dfcQ = DfcQ(KSoundScRxUnit0);
		}
	else
		{
		dfcQ = DfcQ(KSoundScTxUnit0);
		}

	if(dfcQ->iThread != NKern::CurrentThread())
		{
		iPowerUpDfc.Enque();
		iFastSem.iOwningThread = NKern::CurrentThread();
		NKern::FSWait(&iFastSem);
		}
	else
		{
		iPowerUpStatus = RCS42AudioCodec::Open(iPhysicalDevice->iCodec);
		}

	return iPowerUpStatus;
	}

/**
Called from the LDD in the context of the driver thread to power down the sound device when the
channel is closed and just before the phone powers down when being turned off or going into standby.
*/
void DNE1_TBSoundScPddChannel::PowerDown()
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::PowerDown", iCaps.iDirection));
	// Power down the audio device.
	// note, that reference-counting Codec will be powered-down if this call closes the last instance
	RCS42AudioCodec::Close(iPhysicalDevice->iCodec);
	}

/**
Called from the LDD to handle a custom configuration request.
@param aFunction A number identifying the request.
@param aParam A 32-bit value passed to the driver. Its meaning depends on the request.
@return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DNE1_TBSoundScPddChannel::CustomConfig(TInt /*aFunction*/,TAny* /*aParam*/)
	{
	return(KErrNotSupported);
	}

/**
Called each time a playback DMA transfer completes - from the DMA callback function in the sound thread's DFC context.
@param aTransferID The transfer ID of the DMA transfer.
@param aTransferResult The result of the DMA transfer.
@param aBytesTransferred The number of bytes transferred.
*/
void DNE1_TBSoundScPddChannel::PlayCallback(TUint aTransferID,TInt aTransferResult,TInt aBytesTransferred)
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::PlayCallback, ID %x, result %d, pending: %d", iCaps.iDirection, aTransferID, aTransferResult, iPendingPlay));

	--iPendingPlay;

	if(iCaps.iDirection == ESoundDirRecord)
		{
		Ldd()->RecordCallback(aTransferID,aTransferResult,aBytesTransferred);
		}
	else
		{
		Ldd()->PlayCallback(aTransferID,aTransferResult,aBytesTransferred);
		}
	}

/**
Initialise the data member DNE1_TBSoundScPddChannel::iCaps with the play capabilities of this audio playback device.
*/
void DNE1_TBSoundScPddChannel::SetCaps()
	{
	__KTRACE_SND(Kern::Printf("DNE1_TBSoundScPddChannel(%d)::SetCaps", iCaps.iDirection));

	// The audio channel configurations supported by this unit
	// This unit supports both mono and stereo
	iCaps.iChannels = (KSoundMonoChannel | KSoundStereoChannel);

	// This unit supports only some of the sample rates offered by Symbian OS
	iCaps.iRates = (KSoundRate11025Hz | KSoundRate22050Hz | KSoundRate44100Hz);

	// The encoding formats supported
	// until we'll be able to set the transfer source/dest lengths for DMA transfers
	// only support for this one
	iCaps.iEncodings = KSoundEncoding16BitPCM;

	// This unit only supports interleaved data format when playing stereo;  that is, a PCM data
	// stream where the left and right channel samples are interleaved as L-R-L-R-L-R etc.
	iCaps.iDataFormats = KSoundDataFormatInterleaved;

	// The minimum request size that the device can support. All requests to play or record data must be of a
	//	length that is a multiple of this value.
	iCaps.iRequestMinSize = 4;

	// The logarithm to base 2 of the alignment required for request arguments. All requests to play or
	//	record data must specify locations in the shared chunk which conform to this alignment.
	iCaps.iRequestAlignment = 2;

	// Indicates whether this unit is capable of detecting changes in its hardware configuration.
	iCaps.iHwConfigNotificationSupport = EFalse;
	}

/**
Constructor for a shared chunk sound driver playback DMA request.
*/
DNE1_TBSoundScDmaRequest::DNE1_TBSoundScDmaRequest(TDmaChannel& aChannel, DNE1_TBSoundScPddChannel* aPdd, TInt aMaxTransferSize)
	: DDmaRequest(aChannel,DNE1_TBSoundScDmaRequest::DmaService,this,aMaxTransferSize),
	  iPdd(aPdd)
	{
	}

DNE1_TBSoundScDmaRequest::~DNE1_TBSoundScDmaRequest()
	{
	// release buffer used for mono playback
	if (iChunk)
		{
		iChunk->Close(NULL);
		}

	if (iBuffPhys)
		{
		Epoc::FreePhysicalRam(iBuffPhys, KMaxDmaTransferLen*2);
		}
	}

TInt DNE1_TBSoundScDmaRequest::CreateMonoBuffer()
	{
	// alloc memory for buffer - we might need to play mono samples..
	TInt r = Epoc::AllocPhysicalRam(KMaxDmaTransferLen*2, iBuffPhys);
	if(r != KErrNone)
		{
		return r;
		}

	// map this buffer as non-cachable and writtable only by supervisor.
	r = DPlatChunkHw::New(iChunk, iBuffPhys, KMaxDmaTransferLen*2,
							  EMapAttrSupRw | EMapAttrFullyBlocking);
	if (r != KErrNone)
		{
		return r;
		}

	iBufLin = iChunk->LinearAddress();

	return KErrNone;
	}

//
TInt DNE1_TBSoundScDmaRequest::SetDmaTransfer(TUint aTransferID, TLinAddr aLinAddr, TInt aNumBytes)
	{
	__ASSERT_DEBUG(iBufLin != NULL, Kern::Fault(KSoundPDDPanicCat, __LINE__));
	TInt r = KErrNone;

	// store TransferID
	iTransferID = aTransferID;

	if (iPdd->iCurrentConfig.iChannels == 1)
		{
		// Set the DMA source information - local buffer, which is always
		// twice as big for mono transfers..
		iTransferSize = aNumBytes*2;

		// Store the original address of the data supplied.. this will be used
		// as the destination address for recorded mono data..
		if (iPdd->iCaps.iDirection == ESoundDirRecord)
			{
			// store address of the orginal buffer,
			// where we need to copy the recorded data back - after the transfer has finished
			iAddrLinOrig = aLinAddr;

			r = Fragment(KHwI2S0RxPhys, iBufLin, iTransferSize,
					KDmaMemDest | KDmaIncDest | KDmaPhysAddrSrc,
					(TUint32)this);
			}
		else // this is a Play (Tx) unit
		// This is a mono transfer request so we need to copy data to the internal buffer
		// and transfer it as interleaved stereo - since this is the only format supported
		// by the I2S bus.
			{
			TInt16 *src = (TInt16*)aLinAddr;
			TInt32 *dst = (TInt32*)iBufLin;

			// copy data to the local buffer (2 bytes at the time) -to play mono in both channels
			for (TInt i = 0; i < aNumBytes/2; i++)
				{
				*dst++ = TInt32((*src) << 16) | (*src & 0xffff);
				src++;
				}

			r = Fragment(iBufLin, KHwI2S0TxPhys, iTransferSize,
					KDmaMemSrc | KDmaIncSrc | KDmaPhysAddrDest,
					(TUint32)this);
			}
		}
	else // it's stereo, interleaved data, which can be transferred directly
		{
		// Supply the DMA source information - original data in the shared chunk
		iTransferSize = aNumBytes;

		if (iPdd->iCaps.iDirection == ESoundDirRecord)
			{
			r = Fragment(KHwI2S0RxPhys, aLinAddr, iTransferSize,
					KDmaMemDest | KDmaIncDest | KDmaPhysAddrSrc,
					(TUint32)this);
			}
		else // this is a Play (Tx) unit
			{
			r = Fragment(aLinAddr, KHwI2S0TxPhys, iTransferSize,
					KDmaMemSrc | KDmaIncSrc | KDmaPhysAddrDest,
					(TUint32)this);
			}
		}
	return r;
	}

/**
DMA tx service routine. Called in the sound thread's DFC context by the s/w DMA controller.
@param aResult Status of DMA transfer.
@param aArg Argument passed to DMA controller.
*/
void DNE1_TBSoundScDmaRequest::DmaService(TResult aResult, TAny* aArg)
	{
	DNE1_TBSoundScDmaRequest& req = *(DNE1_TBSoundScDmaRequest*)aArg;
	__KTRACE_SND( Kern::Printf("DmaService(%d) %d",req.iPdd->iCaps.iDirection, aResult));

	TInt res = KErrNone;
	TInt bytesTransferred = req.iTransferSize;
	if (aResult!=DDmaRequest::EOk)
		{
		res = KErrCorrupt;
		bytesTransferred = 0;
		}

	// if this was mono transfered as stereo..
	if (req.iPdd->iCurrentConfig.iChannels == 1)
		{
		// adjust back the number of bytes transfered
		bytesTransferred /= 2;

		// if this request is a part of record unit
		// copy data to back to the shared chunk provided by the LDD
		if (req.iPdd->iCaps.iDirection == ESoundDirRecord)
			{
			TInt32 *src = (TInt32*)req.iBufLin;
			TInt16 *dst = (TInt16*)req.iAddrLinOrig;

			for (TInt i = 0; i < bytesTransferred; i+=2)
				{
				*dst++ = TInt16(*src++);
				}
			}
		}

	// Inform the LDD of the result of the transfer.
	req.iPdd->PlayCallback(req.iTransferID,res,bytesTransferred);

	return;
	}

