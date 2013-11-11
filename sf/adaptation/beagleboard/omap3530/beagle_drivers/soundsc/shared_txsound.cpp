/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
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

#include "shared_sound.h"
#include "variant_sound.h"

void TimerCallback(TAny* aData)
	{
	DDriverBeagleSoundScPdd * soundscpdd = (DDriverBeagleSoundScPdd*) aData;
		
	soundscpdd->Callback(soundscpdd->iTransferArray[0].iTransferID, KErrNone, soundscpdd->iTransferArray[0].iNumBytes);
	
	}


DDriverBeagleSoundScPdd::DDriverBeagleSoundScPdd() : iTimer(TimerCallback,this)
	{

	}

DDriverBeagleSoundScPdd::~DDriverBeagleSoundScPdd()
	{
	iTimer.Cancel();
	}


TInt DDriverBeagleSoundScPdd::DoCreate()
	{

	SetCaps();

	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::DoCreate TxPdd");
	
	return KErrNone;
	}

void DDriverBeagleSoundScPdd::GetChunkCreateInfo(TChunkCreateInfo& aChunkCreateInfo)
	{
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::GetChunkCreateInfo TxPdd");
	
	aChunkCreateInfo.iType = TChunkCreateInfo::ESharedKernelMultiple;
	aChunkCreateInfo.iMapAttr = EMapAttrFullyBlocking; 	// No caching
	aChunkCreateInfo.iOwnsMemory = ETrue; 				// Using RAM pages
	aChunkCreateInfo.iDestroyedDfc = NULL; 				// No chunk destroy DFC
	}

void DDriverBeagleSoundScPdd::Caps(TDes8& aCapsBuf) const
	{

	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::Caps TxPdd");
	
	// Fill the structure with zeros in case it is a newer version than we know about
	aCapsBuf.FillZ(aCapsBuf.MaxLength());

	// And copy the capabilities into the packaged structure
	TPtrC8 ptr((const TUint8*) &iCaps, sizeof(iCaps));
	aCapsBuf = ptr.Left(Min(ptr.Length(), aCapsBuf.MaxLength()));
	}

TInt DDriverBeagleSoundScPdd::SetConfig(const TDesC8& aConfigBuf)
	{

	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::SetConfig TxPdd");
	
	// Read the new configuration from the LDD
	TCurrentSoundFormatV02 config;
	TPtr8 ptr((TUint8*) &config, sizeof(config));
	Kern::InfoCopy(ptr, aConfigBuf);

	iConfig = config;
	
	return KErrNone;
	}


TInt DDriverBeagleSoundScPdd::SetVolume(TInt aVolume)
	{
	
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::Setvolume TxPdd");
	
	return KErrNone;
	}


TInt DDriverBeagleSoundScPdd::StartTransfer()
	{
	
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::starttransfer TxPdd");
	
	//Prepare for transfer
	return KErrNone;
	
	}

TInt DDriverBeagleSoundScPdd::CalculateBufferTime(TInt aNumBytes)
	{
	
	TUint samplerate=0;

	// Let the compiler perform an integer division of rates
	switch(iConfig.iRate)
		{
		case ESoundRate7350Hz: 	samplerate = 7350; break;
		case ESoundRate8000Hz: 	samplerate = 8000; break;
		case ESoundRate8820Hz: 	samplerate = 8820; break;
		case ESoundRate9600Hz: 	samplerate = 9600; break;
		case ESoundRate11025Hz: samplerate = 11025; break;
		case ESoundRate12000Hz: samplerate = 12000; break;
		case ESoundRate14700Hz:	samplerate = 14700; break;
		case ESoundRate16000Hz: samplerate = 16000; break;
		case ESoundRate22050Hz: samplerate = 22050; break;
		case ESoundRate24000Hz: samplerate = 24000; break;
		case ESoundRate29400Hz: samplerate = 29400; break;
		case ESoundRate32000Hz: samplerate = 32000; break;
		case ESoundRate44100Hz: samplerate = 44100; break;
		case ESoundRate48000Hz: samplerate = 48000; break;
		}


	// integer division by number of channels
	aNumBytes /= iConfig.iChannels;

	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::iChannels =%d", iConfig.iChannels);
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::iEncoding =%d", iConfig.iEncoding);
	
	// integer division by bytes per sample
	switch(iConfig.iEncoding)
		{
		case ESoundEncoding8BitPCM: break;
		case ESoundEncoding16BitPCM: aNumBytes /= 2; break;
		case ESoundEncoding24BitPCM: aNumBytes /= 3; break;
		}

	return (aNumBytes * 1000) / samplerate; //return time in milliseconds
	

	}

TInt DDriverBeagleSoundScPdd::TransferData(TUint aTransferID, TLinAddr aLinAddr, TPhysAddr /*aPhysAddr*/, TInt aNumBytes)
	{

	//function wil get called multiple times while transfer is in progress therefore keep fifo queue of requests
	TTransferArrayInfo transfer;	
		
	transfer.iTransferID = aTransferID;
	transfer.iLinAddr = aLinAddr;
	transfer.iNumBytes = aNumBytes;
	
	//calculate the amount of time required to play/record buffer
	TInt buffer_play_time = CalculateBufferTime(aNumBytes);
	TInt timerticks = NKern::TimerTicks(buffer_play_time);
	transfer.iPlayTime = timerticks;
	
	iTransferArray.Append(transfer);
	
	//Timer will callback when correct time has elapsed, will return KErrInUse if transfer
	//already active, this is ok becuase will be started again in callback
	TInt err = iTimer.OneShot(timerticks, ETrue);
	
	
	return KErrNone;
	}

void DDriverBeagleSoundScPdd::StopTransfer()
	{
	// Stop transfer
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::stoptransfer TxPdd");
	
	//If timer is currently active then cancel it and call back buffer
	if(iTimer.Cancel())
		{
		Callback(iTransferArray[0].iTransferID, KErrNone, iTransferArray[0].iNumBytes);
		}
		

	}


TInt DDriverBeagleSoundScPdd::PauseTransfer()
	{
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::pausetransfer TxPdd");
	//Pause Transfer
	
	return KErrNone;
	}


TInt DDriverBeagleSoundScPdd::ResumeTransfer()
	{
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::resumetransfer TxPdd");
	//Resume Transfer
	
	return KErrNone;
	}

TInt DDriverBeagleSoundScPdd::PowerUp()
	{
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::PowerUp TxPdd");
	return KErrNone;
	}

void DDriverBeagleSoundScPdd::PowerDown()
	{
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::Powerdown TxPdd");
	}

TInt DDriverBeagleSoundScPdd::CustomConfig(TInt /*aFunction*/,TAny* /*aParam*/)
	{
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::customconfig TxPdd");
	return KErrNotSupported;
	}


void DDriverBeagleSoundScPdd::Callback(TUint aTransferID, TInt aTransferResult, TInt aBytesTransferred)
	{
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::playcallback TxPdd");
	//Callback when Transfer completes or is stopped
	
	iTransferArray.Remove(0);
	
	if(iUnitType == KSoundScTxUnit0)
		{
		Ldd()->PlayCallback(aTransferID, aTransferResult, aBytesTransferred);
		}
	else if(iUnitType == KSoundScRxUnit0)
		{
		Ldd()->RecordCallback(aTransferID, aTransferResult, aBytesTransferred);
		}
	
	if(	iTransferArray.Count()>0)
		{
		iTimer.OneShot(iTransferArray[0].iPlayTime, ETrue);
		}
	
	}

TDfcQue*DDriverBeagleSoundScPdd::DfcQ(TInt /* aUnit*/ )
        {
        return this->DfcQ();
        }

TDfcQue*DDriverBeagleSoundScPdd::DfcQ()
        {
        return iPhysicalDevice->iDfcQ;
        }

TInt DDriverBeagleSoundScPdd::MaxTransferLen() const
	{
	
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::MaxTransferLen TxPdd");
	
	TInt maxlength = 200*1024;
	return maxlength;
	}


void DDriverBeagleSoundScPdd::SetCaps()
	{
	BEAGLE_SOUND_DEBUG("DDriverBeagleSoundScPdd::SetCaps TxPdd");
	
	if(iUnitType == KSoundScTxUnit0)
		{
		// The data transfer direction for this unit is play
		iCaps.iDirection = ESoundDirPlayback;
		}
	else if(iUnitType == KSoundScRxUnit0)
		{
		// The data transfer direction for this unit is record 
		iCaps.iDirection = ESoundDirRecord;
		}
	
	// This unit supports both mono and stereo
	iCaps.iChannels = (KSoundMonoChannel | KSoundStereoChannel);

	// This unit supports only some of the sample rates offered by Symbian OS
	iCaps.iRates = (KSoundRate8000Hz | KSoundRate11025Hz | KSoundRate12000Hz | KSoundRate16000Hz |
					KSoundRate22050Hz | KSoundRate24000Hz | KSoundRate32000Hz | KSoundRate44100Hz |
					KSoundRate48000Hz);

	// This unit only supports 16bit PCM encoding
	iCaps.iEncodings = KSoundEncoding16BitPCM;

	// This unit only supports interleaved data format when playing stereo;  that is, a PCM data
	// stream where the left and right channel samples are interleaved as L-R-L-R-L-R etc.
	iCaps.iDataFormats = KSoundDataFormatInterleaved;

	// The iRequestMinSize member is named badly.  It is actually the value of which the length samples
	// must be a multiple of.  ie.  The sample length % iRequestMinSize must == 0.  This value must always
	// be a power of 2
	iCaps.iRequestMinSize = 4;

	// The logarithm to base 2 of the alignment required for request arguments.  DMA requests must be
	// aligned to a 32 bit boundary
	iCaps.iRequestAlignment = 2;

	// This unit is not capable of detecting changes in hardware configuration
	iCaps.iHwConfigNotificationSupport = EFalse;
	}




