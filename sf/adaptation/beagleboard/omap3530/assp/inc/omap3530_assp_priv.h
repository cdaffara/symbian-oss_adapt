// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// omap3530/assp/inc/omap3530_assp_priv.h
//

#ifndef __OMAP3530_PRIV_H__
#define __OMAP3530_PRIV_H__

#include <e32const.h>
#include <arm.h>
#include <assp.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>
#include <assp/omap3530_assp/omap3530_timer.h>
#include <assp/omap3530_assp/omap3530_uart.h>

/**
* IRQ/FIQ dispatchers
*/
static void IrqDispatch();
static void FiqDispatch();

class TArmCpuId {
public:
	TInt Implementor;		// Indicates the implementor, ARM:
	TInt Variant; 			//Indicates the variant number, or major revision, of the processor:
	TInt Architecture; 		//Indicates that the architecture is given in the feature registers:
	TInt Primary; 			//part number Indicates the part number, Cortex-A8:
	TInt Revision;
};

class Omap3530Interrupt : public Interrupt
	{
public:
	/**
	 * These functions are required to initialise the Interrupt controller,or perform housekeeping
	 * functions, or dispatch the incoming IRQ or FIQ interrupts.
	 */

	/**
	 * initialisation
	 */
	static void Init1();
	static void Init3();

	/**
	 * Housekeeping (disable and clear all hardware interrupt sources)
	 */
	static void DisableAndClearAll();
	IMPORT_C static TInt IsInterruptEnabled(TInt anId);
	static void dumpINTCState();	
	static void dumpIVTState();
	static TInt GetRegisterAndBitOffset(TInt anId,TInt &aReg,TInt &aOffset);
	static void Spurious(TAny* anId);

	static SInterruptHandler Handlers[KNumOmap3530Ints];

	TSpinLock Omap3530INTCLock;
	TSpinLock Omap3530IVTLock;
private:


	};



class Omap3530Assp : public Asic
	{
public:
	IMPORT_C Omap3530Assp();

public:
	IMPORT_C virtual TInt MsTickPeriod();
	/**
	 * Obtain System Time from the RTC
	 * @return System Time in seconds from 00:00 hours of 1/1/2000
	 */
	virtual TInt SystemTimeInSecondsFrom2000(TInt& aTime)=0;
	/**
	 * Obtain Adjust the RTC with new System Time (from 00:00 hours of 1/1/2000)
	 * @return System wide error code
	 */
	virtual TInt SetSystemTimeInSecondsFrom2000(TInt aTime)=0;
	/**
	 * Obtain the time it takes to execute two processor instructions
	 * @return Time in nanoseconds it takes two execute 2 instructions at the processor clock speed
	 */
	IMPORT_C virtual TUint32 NanoWaitCalibration();
	
	/**
	 * initialisation
	 */
	IMPORT_C virtual void Init1();
	IMPORT_C virtual void Init3();
	/**
	 * Active waiting loop (not to be used after System Tick timer has been set up - Init3()
	 * @param aDuration A wait time in milliseconds
	 */	
	IMPORT_C static void BootWaitMilliSeconds(TInt aDuration);
	/**
	 * Read and return the Startup reason of the Hardware
	 * @return A TMachineStartupType enumerated value
	 */	
	IMPORT_C virtual TMachineStartupType StartupReason();
	/**
	 * Read and return the the CPU ID
	 * @return An integer containing the CPU ID string read off the hardware
	 */	
	IMPORT_C static void ArmCpuVersionId(TArmCpuId &id);
	/**
	 * Get debug port number enumeration
	 * @return An integer containing the Omap3530Uart::TUartNumber value, with value ENone if debug port isn't a UART
	 */	
	IMPORT_C static Omap3530Uart::TUartNumber DebugPortNumber();
	/**
	 * Read CPU clock period in picoseconds
	 * @return An integer containing the CPU clock period in picoseconds
	 */	
	IMPORT_C static TUint ProcessorPeriodInPs();
	/**
	 * Read the current time of the RTC
	 * @return A value that is the real time as given by a RTC
	 */	
	//IMPORT_C static TUint RtcData();
	/**
	 * Set the RTC time 
	 * @param aValue The real time to set the RTC
	 */	
	//IMPORT_C static void SetRtcData(TUint aValue);
	/**
	 * Obtain the physical start address of Video Buffer
	 * @return the physical start address of Video Buffer
	 */	
					 
	IMPORT_C static  TUint64 GetXtalFrequency();

private:
	/**
	 * Assp-specific implementation for Kern::NanoWait function
	 */
	static void NanoWait(TUint32 aInterval);
	TInt StartMsTick();
	static void MsTickIsr(TAny* aPtr);
	

public:
	/**
	 * for derivation by Variant
	 */

	/**
	 * external interrupt handling
	 * used by second-level interrupt controllers at Variant level
	 */
	virtual TInt InterruptBind(TInt anId, TIsr anIsr, TAny* aPtr)=0;
	virtual TInt InterruptUnbind(TInt anId)=0;
	virtual TInt InterruptEnable(TInt anId)=0;
	virtual TInt InterruptDisable(TInt anId)=0;
	virtual TInt InterruptClear(TInt anId)=0;
	virtual TInt IsExternalInterrupt(TInt anId)=0;

	/**
	 * USB client controller - Some example functions for the case that USB cable detection and
	 * UDC connect/disconnect functionality are part of the Variant.
	 * Pure virtual functions called by the USB PSL, to be implemented by the Variant (derived class).
	 * If this functionality is part of the ASSP then these functions can be removed and calls to them
	 * in the PSL (./pa_usbc.cpp) replaced by the appropriate internal operations.
	 */
	virtual TBool UsbClientConnectorDetectable()=0;
	virtual TBool UsbClientConnectorInserted()=0;
	virtual TInt RegisterUsbClientConnectorCallback(TInt (*aCallback)(TAny*), TAny* aPtr)=0;
	virtual void UnregisterUsbClientConnectorCallback()=0;
	virtual TBool UsbSoftwareConnectable()=0;
	virtual TInt UsbConnect()=0;
	virtual TInt UsbDisconnect()=0;

	/** Return the frequency in Hz of the SYSCLK source clock */
	virtual TUint SysClkFrequency() const = 0;

	/** Return the frequency in Hz of the SYSCLK32K source clock */
	virtual TUint SysClk32kFrequency() const = 0;

	/** Return the frequency in Hz of the ALTCLK source clock */
	virtual TUint AltClkFrequency() const = 0;

public:
	static Omap3530Assp * 											Variant;
	TBool															iDebugInitialised;

private:
	};

#endif
