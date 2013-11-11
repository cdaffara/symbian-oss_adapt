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
* ne1_tb\specific\resmanpsl.cpp
*
*/


#include "resmanpsl.h"

static DNE1_TBPowerResourceController TheController;
/** Entry point of resource controller */
DECLARE_RESOURCE_MANAGER_EXTENSION(TheController)
	{
	__KTRACE_OPT(KBOOT, Kern::Printf("CreateController called"));
    new(&TheController) DNE1_TBPowerResourceController;
    return;
	}

/** Constructor */
DNE1_TBPowerResourceController::DNE1_TBPowerResourceController() : DPowerResourceController()
	{
	__KTRACE_OPT(KRESMANAGER, Kern::Printf("DNE1_TBPowerResourceController Called\n"));
	}

/** This function is called by PIL during its creation phase.
	It creates a DFC queue and then invokes setDfcQ function of PIL to set the queue.
	It also initialises the pools with appropriate size.
	*/
TInt DNE1_TBPowerResourceController::DoInitController()
	{
	TInt r = KErrNone;
	__KTRACE_OPT(KRESMANAGER, Kern::Printf(">DNE1_TBPowerResourceController::DoInitController()"));
	//Create a DFC queue
	r = Kern::DfcQCreate(iDfcQ, KDfcQThreadPriority, &KResmanName);
	if(r != KErrNone)
		{
		Kern::Printf("DFC Queue creation failed");
		return r;
		}

#ifdef CPU_AFFINITY_ANY
	NKern::ThreadSetCpuAffinity((NThread*)(iDfcQ->iThread), KCpuAffinityAny);
#endif

	//Call the resource controller to set the DFCQ
	SetDfcQ(iDfcQ);
	//Init pools
	r = InitPools(KERNEL_CLIENTS, USER_CLIENTS, CLIENT_LEVELS, REQUESTS);
	return r;
	}

/** This function is called by PIL to register the static resources. 
    It creates an array to hold the static resource and also creates the resources and updates 
	in the array. 
	*/
//C TSAI: this is to fit new virtual function prototype which uses RPointerArray
TInt DNE1_TBPowerResourceController::DoRegisterStaticResources(RPointerArray<DStaticPowerResource>& aStaticResourceArray)
	{
	__KTRACE_OPT(KRESMANAGER, Kern::Printf("DNE1_TBPowerResourceController::DoRegisterStaticResources [SF4 new virtual function prototype]"));

/** Macro definition to register resource */
#define REGISTER_RESOURCE_RPOINTERARRAY(resource, resourceArray)						\
	{																					\
	pR = new resource();																\
	if(!pR)																				\
		{																				\
		TUint resourceCount = (TUint) resourceArray.Count();							\
		for(TUint count = 0; count < resourceCount; count++)							\
			{																			\
			delete resourceArray[count];												\
			}																			\
		resourceArray.Reset();															\
		return KErrNoMemory;															\
		}																				\
	resourceArray.Append(pR);												\
	}

	DStaticPowerResource* pR = NULL;

	//Note: RPointerArray itself only deal with pointers and doesn't delete objects when Close() or Reset()

	/** Create I2S0 MCLK resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBI2S0MclkResource, aStaticResourceArray);
	/** Create I2S1 MCLK resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBI2S1MclkResource, aStaticResourceArray);
	/** Create I2S2 MCLK resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBI2S2MclkResource, aStaticResourceArray);
	/** Create I2S3 MCLK resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBI2S3MclkResource, aStaticResourceArray);
	/** Create I2S0 SCLK resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBI2S0SclkResource, aStaticResourceArray);
	/** Create I2S1 SCLK resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBI2S1SclkResource, aStaticResourceArray);
	/** Create I2S2 SCLK resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBI2S2SclkResource, aStaticResourceArray);
	/** Create I2S3 SCLK resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBI2S3SclkResource, aStaticResourceArray);
	/** Create CSI0 clock resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBCSI0ClockResource, aStaticResourceArray);
	/** Create CSI1 clock resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBCSI1ClockResource, aStaticResourceArray);
	/** Create Display DCLK resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBDisplayDclkResource, aStaticResourceArray);
	/** Create LCD resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBLcdResource, aStaticResourceArray);
	/** Create Board Power resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBBoardPowerResource, aStaticResourceArray);
	/** Create PCI clock mask enable resource and add to the static resource array */
	REGISTER_RESOURCE_RPOINTERARRAY(DNE1_TBPCIClockResource, aStaticResourceArray);

	return KErrNone;
	}

#if 0	//C TSAI: This is older function implementation (older virtual function prototype)
TInt DNE1_TBPowerResourceController::DoRegisterStaticResources(DStaticPowerResource**& aStaticResourceArray, 
																                 TUint16& aStaticResourceCount)
	{
	__KTRACE_OPT(KRESMANAGER, Kern::Printf("DNE1_TBPowerResourceController::DoRegisterStaticResources"));
	/** Create the static resource array */
	aStaticResourceArray = (DStaticPowerResource**)new(DStaticPowerResource*[EMaxResourceCount]);
	if(!aStaticResourceArray)
		return KErrNoMemory;

	DStaticPowerResource* pR = NULL;

	/** Create I2S0 MCLK resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBI2S0MclkResource, aStaticResourceArray, aStaticResourceCount);

	/** Create I2S1 MCLK resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBI2S1MclkResource, aStaticResourceArray, aStaticResourceCount);
	
	/** Create I2S2 MCLK resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBI2S2MclkResource, aStaticResourceArray, aStaticResourceCount);

	/** Create I2S3 MCLK resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBI2S3MclkResource, aStaticResourceArray, aStaticResourceCount);

	/** Create I2S0 SCLK resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBI2S0SclkResource, aStaticResourceArray, aStaticResourceCount);

	/** Create I2S1 SCLK resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBI2S1SclkResource, aStaticResourceArray, aStaticResourceCount);

	/** Create I2S2 SCLK resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBI2S2SclkResource, aStaticResourceArray, aStaticResourceCount);

	/** Create I2S3 SCLK resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBI2S3SclkResource, aStaticResourceArray, aStaticResourceCount);

	/** Create CSI0 clock resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBCSI0ClockResource, aStaticResourceArray, aStaticResourceCount);

	/** Create CSI1 clock resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBCSI1ClockResource, aStaticResourceArray, aStaticResourceCount);

	/** Create Display DCLK resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBDisplayDclkResource, aStaticResourceArray, aStaticResourceCount);

	/** Create LCD resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBLcdResource, aStaticResourceArray, aStaticResourceCount);

	/** Create Board Power resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBBoardPowerResource, aStaticResourceArray, aStaticResourceCount);

	/** Create PCI clock mask enable resource and add to the static resource array */
	REGISTER_RESOURCE(DNE1_TBPCIClockResource, aStaticResourceArray, aStaticResourceCount);

	return KErrNone;
	}

#endif


