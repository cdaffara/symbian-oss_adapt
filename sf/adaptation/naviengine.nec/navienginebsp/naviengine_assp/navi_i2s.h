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
* bsp\hwip_nec_navienegine\navienegine_assp\i2s.h
*
*/



#include <e32cmn.h>
#include <e32def.h>
#include <naviengine_priv.h>
#include <i2s.h>


//
// each channel should implemment the platform specific interface 

class DI2sChannelBase
	{
public:
	static TInt CreateChannels(DI2sChannelBase*& aChannel, TInt aInterfaceId);   
	
	virtual TInt ConfigureInterface(TDes8* aConfig) = 0;
	virtual TInt GetInterfaceConfiguration(TDes8& aConfig) = 0;
	virtual TInt SetSamplingRate(TInt aSamplingRate) = 0;
	virtual TInt GetSamplingRate(TInt& aSamplingRate) = 0;
	virtual TInt SetFrameLengthAndFormat(TInt aFrameLength, TInt aLeftFramePhaseLength) = 0;
	virtual TInt GetFrameFormat(TInt& aLeftFramePhaseLength, TInt& aRightFramePhaseLength) = 0;
	virtual TInt SetSampleLength(TInt aFramePhase, TInt aSampleLength) = 0;
	virtual TInt GetSampleLength(TInt aFramePhase, TInt& aSampleLength) = 0;
	virtual TInt SetDelayCycles(TInt aFramePhase, TInt aDelayCycles) = 0;
	virtual TInt GetDelayCycles(TInt aFramePhase, TInt& aDelayCycles) = 0;
	virtual TInt ReadReceiveRegister(TInt aFramePhase, TInt& aData) = 0;
	virtual TInt WriteTransmitRegister(TInt aFramePhase, TInt aData) = 0;
	virtual TInt ReadTransmitRegister(TInt aFramePhase, TInt& aData) = 0;
	virtual TInt ReadRegisterModeStatus(TInt aFramePhase, TInt& aFlags) = 0;
	virtual TInt EnableRegisterInterrupts(TInt aFramePhase, TInt aInterrupt) = 0;
	virtual TInt DisableRegisterInterrupts(TInt aFramePhase, TInt aInterrupt) = 0;
	virtual TInt IsRegisterInterruptEnabled(TInt aFramePhase, TInt& aEnabled) = 0;
	virtual TInt EnableFIFO(TInt aFramePhase, TInt aFifoMask) = 0;
	virtual TInt DisableFIFO(TInt aFramePhase, TInt aFifoMask) = 0;
	virtual TInt IsFIFOEnabled(TInt aFramePhase, TInt& aEnabled) = 0;
	virtual TInt SetFIFOThreshold(TInt aFramePhase, TInt aDirection, TInt aThreshold) = 0;
	virtual TInt ReadFIFOModeStatus(TInt aFramePhase, TInt& aFlags) = 0;
	virtual TInt EnableFIFOInterrupts(TInt aFramePhase, TInt aInterrupt) = 0;
	virtual TInt DisableFIFOInterrupts(TInt aFramePhase, TInt aInterrupt) = 0;
	virtual TInt IsFIFOInterruptEnabled(TInt aFramePhase, TInt& aEnabled) = 0;
	virtual TInt ReadFIFOLevel(TInt aFramePhase, TInt aDirection, TInt& aLevel) = 0;
	virtual TInt EnableDMA(TInt aFifoMask) = 0;
	virtual TInt DisableDMA(TInt aFifoMask) = 0;
	virtual TInt IsDMAEnabled(TInt& aEnabled) = 0;
	virtual TInt Start(TInt aDirection) = 0;
	virtual TInt Stop(TInt aDirection) = 0;
	virtual TInt IsStarted(TInt aDirection, TBool& aStarted) = 0;	
	
protected:
	TInt iInterfaceId;
	TI2sConfigV01 iConfig;
	};

/**
The management class for all I2S channels
*/
class TI2sManager
	{
public:
	// initialisation
	TInt DoCreate();
public:
	static DI2sChannelBase **iChannels;	// array of pointers to channels (populated in DoCreate); indexed by the channel number portion of interface ID (bottom 16 bits)
	static TInt iChannelsNum; 			// to store number of channels present on the system (obtained from ConfRep and set in DoCreate)
	};

