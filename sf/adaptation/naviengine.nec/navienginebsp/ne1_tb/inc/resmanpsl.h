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
* ne1_tb\inc\resmanpsl.h
*
*/



#ifndef __RESOURCECONTROLPSL_H__
#define __RESOURCECONTROLPSL_H__

#include <naviengine.h>
#include "powerresources.h"
#include <resourcecontrol.h>

#define SHIFT_BY_8 8
#define SHIFT_BY_9 9
#define SHIFT_BY_16 16
#define SHIFT_BY_24 24

#define CSI_UNIT_ENABLE 0x80
#define CSI_RESET 0x10000000

const TUint KClearBit0 = 0x1;
const TUint KClearBit0_2 = 0x7;
const TUint KClearBit0_3 = 0xF;
const TUint KClearBit09 = 0x200;
const TUint KSetBit09 = 0x200;
const TUint KClearBit18 = 0x40000;
const TUint KSetBit18 = 0x40000;
const TUint KClearBit8_11 = 0xF00;
const TUint KClearBit19 = 0x80000;
const TUint KSetBit19 = 0x80000;
const TUint KClearBit16_19 = 0xF0000;
const TUint KClearBit20 = 0x100000;
const TUint KSetBit20 = 0x100000;
const TUint KClearBit24_27 = 0xF000000;
const TUint KClearBit21 = 0x200000;
const TUint KSetBit21 = 0x200000;

//Initial resource controller pool definition
#define KERNEL_CLIENTS  14
#define USER_CLIENTS	1
#define CLIENT_LEVELS	14
#define REQUESTS		0

const TUint KDfcQThreadPriority = 28;
_LIT(KResmanName, "NE1_TBResMan");

/** Macro definition to register resource */
#define REGISTER_RESOURCE(resource, resourceArray, resourceCount)						\
	{																					\
	pR = new resource();																\
	if(!pR)																				\
		{																				\
		for(TUint count = 0; count < resourceCount; count++)							\
			{																			\
			delete resourceArray[count];												\
			}																			\
		delete[] resourceArray;															\
		return KErrNoMemory;															\
		}																				\
	resourceArray[resourceCount++] = pR;												\
	}

//Power Resource Controller derived class for naviengine.
class DNE1_TBPowerResourceController : public DPowerResourceController
	{
public:
	DNE1_TBPowerResourceController();
	TInt DoInitController(); 

	//C TSAI: from SF4 PDK 1022, this virtual function prototype has changed, and need to re-write this function 
	//to match the new function prototype
	//TInt DoRegisterStaticResources(DStaticPowerResource**& aStaticResourceArray, TUint16& aStaticResourceCount);
	TInt DoRegisterStaticResources(RPointerArray<DStaticPowerResource>& aStaticResourceArray);
	};


// Resource definition for I2S0 MCLK frequency divider setting
class DNE1_TBI2S0MclkResource : public DStaticPowerResource
	{
public:
	DNE1_TBI2S0MclkResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for I2S1 MCLK frequency divider setting
class DNE1_TBI2S1MclkResource : public DStaticPowerResource
	{
public:
	DNE1_TBI2S1MclkResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for I2S2 MCLK frequency divider setting
class DNE1_TBI2S2MclkResource : public DStaticPowerResource
	{
public:
	DNE1_TBI2S2MclkResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for I2S3 MCLK frequency divider setting
class DNE1_TBI2S3MclkResource : public DStaticPowerResource
	{
public:
	DNE1_TBI2S3MclkResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for I2S0 Sampling frequency setting
class DNE1_TBI2S0SclkResource : public DStaticPowerResource
	{
public:
	DNE1_TBI2S0SclkResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for I2S1 sampling frequency setting
class DNE1_TBI2S1SclkResource : public DStaticPowerResource
	{
public:
	DNE1_TBI2S1SclkResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for I2S2 sampling frequency setting
class DNE1_TBI2S2SclkResource : public DStaticPowerResource
	{
public:
	DNE1_TBI2S2SclkResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for I2S3 sampling frequency setting
class DNE1_TBI2S3SclkResource : public DStaticPowerResource	
	{
public:
	DNE1_TBI2S3SclkResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for Display clock (DCLK) setting
class DNE1_TBDisplayDclkResource : public DStaticPowerResource
	{
public:
	DNE1_TBDisplayDclkResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for LCD power
class DNE1_TBLcdResource : public DStaticPowerResource
	{
public:
	DNE1_TBLcdResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for board power
class DNE1_TBBoardPowerResource : public DStaticPowerResource
	{
public:
	DNE1_TBBoardPowerResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for CSI0 clock select
class DNE1_TBCSI0ClockResource : public DStaticPowerResource
	{
public:
	DNE1_TBCSI0ClockResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};	

//Resource definition for CSI1 clock select
class DNE1_TBCSI1ClockResource : public DStaticPowerResource
	{
public:
	DNE1_TBCSI1ClockResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for PCL clock enable
class DNE1_TBPCIClockResource : public DStaticPowerResource
	{
public:
	DNE1_TBPCIClockResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

//Resource definition for PCL clock enable
class DNE1_WakeupLatencyResource : public DStaticPowerResource
	{
public:
	DNE1_WakeupLatencyResource();
	TInt GetInfo(TDes8* aInfo)const;
	TInt DoRequest(TPowerRequest& aRequest);
	};

#endif //__RESOURCECONTROLPSL_H__

