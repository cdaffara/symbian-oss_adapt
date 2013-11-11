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

#ifndef __BEAGLESHARED_SOUND_H__
#define __BEAGLESHARED_SOUND_H__

#include <soundsc.h>

#ifdef _DEBUG
#define BEAGLE_SOUND_DEBUG(x...) Kern::Printf(x)
#else
#define BEAGLE_SOUND_DEBUG(x...)
#endif

class DDriverBeagleSoundScPddFactory;

class DDriverBeagleSoundScPdd : public DSoundScPdd
	{
public:

	DDriverBeagleSoundScPdd();
	~DDriverBeagleSoundScPdd();
	TInt DoCreate();
	void GetChunkCreateInfo(TChunkCreateInfo& aChunkCreateInfo);
	void Caps(TDes8& aCapsBuf) const;
	TInt MaxTransferLen() const;
	TInt SetConfig(const TDesC8& aConfigBuf);
	TInt SetVolume(TInt aVolume);
	TInt StartTransfer();
	TInt TransferData(TUint aTransferID, TLinAddr aLinAddr, TPhysAddr aPhysAddr, TInt aNumBytes);
	void StopTransfer();
	TInt PauseTransfer();
	TInt ResumeTransfer();
	TInt PowerUp();
	void PowerDown();
	TInt CustomConfig(TInt aFunction, TAny* aParam);
	void Callback(TUint aTransferID, TInt aTransferResult, TInt aBytesTransferred);

	void SetCaps();
        // There was a change in the signature for DfcQ() which
        // is a pure virtual method in the parent.
        //  for Symbian^2
        TDfcQue* DfcQ();
        //  for Symbian^3
	TDfcQue* DfcQ(TInt aUnit);
	
	TInt CalculateBufferTime(TInt aNumBytes);

public:
	
	DDriverBeagleSoundScPddFactory*	iPhysicalDevice;
	
	class TTransferArrayInfo{

public:
	TUint 						iTransferID;
	TLinAddr 					iLinAddr;
	TInt 						iNumBytes;
	TInt						iPlayTime;
	};
	
	RArray<TTransferArrayInfo> iTransferArray;
	
	NTimer						iTimer;
	
	TInt						iUnitType; //Play or Record
	
private:

	TSoundFormatsSupportedV02	iCaps;
	
	TCurrentSoundFormatV02		iConfig;
	


	};




#endif 
