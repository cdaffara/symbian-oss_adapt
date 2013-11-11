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
* naviengine_assp\naviengine_priv.h
* NE1_TBVariant ASSP architecture private header file
*
*/



#ifndef __NAVIENGINE_PRIV_H__
#define __NAVIENGINE_PRIV_H__
#include <e32const.h>
#include <arm.h>
#include <assp.h>
#include <assp/naviengine/naviengine.h>


//----------------------------------------------------------------------------
// NaviEngine class
//----------------------------------------------------------------------------
class TNaviEngine
	{
	/**
	 * Accessor functions to hardware resources managed by ASSP (ASIC). Auxiliary and information functions which
	 * are commonly used by Device Drivers or ASSP/Variant code.
	 * Some examples below. These examples assume that the hardware blocks they access (e.g. Interrupt controller
	 * RTC, Clock Control Module, UART, etc) are part of the ASSP.
	 */
public:
	/**
	 * Phase 1 initialisation
	 */
	static void Init1();
	/**
	 * Phase 3 initialisation
	 */
	static void Init3();
	/**
	 * Active waiting loop (not to be used after System Tick timer has been set up - Init3()
	 * @param aDuration A wait time in milliseconds
	 */	
	IMPORT_C static void BootWaitMilliSeconds(TInt aDuration);
	/**
	 * Read and return the Startup reason of the Hardware
	 * @return A TMachineStartupType enumerated value
	 */	
	IMPORT_C static TMachineStartupType StartupReason();
	/**
	 * Read and return the the CPU ID
	 * @return An integer containing the CPU ID string read off the hardware
	 */	
	IMPORT_C static TInt CpuVersionId();
	/**
	 * Read Linear base address of debug UART (as selected in obey file or with eshell debugport command).
	 * @return An integer containing the Linear address of debug Serial Port
	 */	
	IMPORT_C static TUint DebugPortAddr();
	/**
	 * Read CPU clock period in picoseconds
	 * @return An integer containing the CPU clock period in picoseconds
	 */	
	IMPORT_C static TUint ProcessorPeriodInPs();
	/**
	 * Read the current time of the RTC
	 * @return A value that is the real time as given by a RTC
	 */	
	IMPORT_C static TUint RtcData();
	/**
	 * Set the RTC time 
	 * @param aValue The real time to set the RTC
	 */	
	IMPORT_C static void SetRtcData(TUint aValue);
	/**
	 * Obtain the physical start address of Video Buffer
	 * @return the physical start address of Video Buffer
	 */	
	IMPORT_C static TPhysAddr VideoRamPhys();
	static void InitDebugOutput();
	static void DoDebugOutput(TUint aLetter);
	
private:
	/** Assp-specific implementation for Kern::NanoWait function  */
	static void NanoWait(TUint32 aInterval);
	};

/** All ASSP interrupt souces. */
enum TNaviEngineAsspInterruptId
	{
	// The list of core interrupts.
	KIntIdDigitiser,            // Not a valid value.                                     ///< Internal, clients get value from HCR
	KIntIdSound,                // Not a valid value.                                     ///< Internal, clients get value from HCR
	KIntIdTimer1,               // Not a valid value. (Used in template media driver)     ///< Internal, clients get value from HCR
	KIntIdExpansion,            // Not a valid value. (Second level interrupt controller) ///< Internal, clients get value from HCR
	EAsspIntIdUsb=11,           // Not a valid value.                                     ///< Internal, clients get value from HCR

	KIntCsi0 = 34,                                                                        ///< Internal, clients get value from HCR 
	KIntCsi1 = 35,                                                                        ///< Internal, clients get value from HCR 
	
	KIntIdOstMatchMsTimer = 36, // SoC Timer0 interrupt                                   ///< Internal, clients get value from HCR
	KIntId1stMatchMsTimer = 37, // SoC Timer1 interrupt                                   ///< Internal, clients get value from HCR
	KIntId2stMatchMsTimer = 38, // SoC Timer2 interrupt                                   ///< Internal, clients get value from HCR
	KIntId3stMatchMsTimer = 39, // SoC Timer3 interrupt                                   ///< Internal, clients get value from HCR
	KIntId4stMatchMsTimer = 41, // SoC Timer4 interrupt                                   ///< Internal, clients get value from HCR
	KIntId5stMatchMsTimer = 42, // SoC Timer5 interrupt                                   ///< Internal, clients get value from HCR
	
	EIntSd0 = 43,               // SD #0 : OXMNIRQ                                        ///< Internal, clients get value from HCR
	EIntSd1 = 44,               // SD #1 : OXASIOIRQ //SDIO                               ///< Internal, clients get value from HCR
	
	EIntNandCtrl = 46,          // Nand Controller                                        ///< Internal, clients get value from HCR

	EIntDisp0 = 50,             // DISP 0                                                 ///< Internal, clients get value from HCR

	KIntI2S0 = 56,              // I2S 0 Interrupts                                       ///< Internal, clients get value from HCR

	EIntPciInt = 65,            // PCI Int                                                ///< Internal, clients get value from HCR
	EIntPciSErrB = 66,          // PCI Systerm Error                                      ///< Internal, clients get value from HCR
	EIntPciPErrB = 67,          // PCI Parity Error                                       ///< Internal, clients get value from HCR
	EIntUsbHIntA = 71,          // USB Host Int A                                         ///< Internal, clients get value from HCR
	EIntUsbHIntB = 72,          // USB Host Int B                                         ///< Internal, clients get value from HCR
	EIntUsbHSmi = 73,           // USB Host System Management Interrupt                   ///< Internal, clients get value from HCR
	EIntUsbHPme = 74,           // USB Host Power Management Event                        ///< Internal, clients get value from HCR
	KIntDMAC32_0_End = 76,                                                                ///< Internal, clients get value from HCR
	KIntDMAC32_0_Err = 77,                                                                ///< Internal, clients get value from HCR
	KIntDMAC32_1_End = 78,                                                                ///< Internal, clients get value from HCR
	KIntDMAC32_1_Err = 79,                                                                ///< Internal, clients get value from HCR
	KIntDMAC32_2_End = 80,                                                                ///< Internal, clients get value from HCR
	KIntDMAC32_2_Err = 81,                                                                ///< Internal, clients get value from HCR
	KIntDMAC32_3_End = 82,                                                                ///< Internal, clients get value from HCR
	KIntDMAC32_3_Err = 83,                                                                ///< Internal, clients get value from HCR
	KIntDMAC32_4_End = 84,                                                                ///< Internal, clients get value from HCR
	KIntDMAC32_4_Err = 85,                                                                ///< Internal, clients get value from HCR

	KIntIdUart0 = 86,           // SoC Uart #0                                            ///< Internal, clients get value from HCR
	KIntIdUart1 = 87,           // SoC Uart #1                                            ///< Internal, clients get value from HCR
	KIntIdUart2 = 88,           // SoC Uart #2                                            ///< Internal, clients get value from HCR

	KIntIdGpio       = 94,       // gpio                                                  ///< Internal, clients get value from HCR
	KIntDMAC64_End   = 97,                                                                ///< Internal, clients get value from HCR
	KIntDMAC64_Err   = 98,                                                                ///< Internal, clients get value from HCR
	
	KIntCpuProfilingDefaultInterruptBase   = 99, //Interrupt used by sampling profilers. 
                                                 //Each CPU_i is interrupted by interrupt number ECpuProfilingInterrupt + i.
                                                 //Therefore we need 1 interrupt per CPU, 99, 100, 101, 102.

	KIntDMAExBus_End = 124,                                                               ///< Internal, clients get value from HCR 
	KIntDMAExBus_Err = 125,                                                               ///< Internal, clients get value from HCR 

	KNumNaviEngineInts    = 128, // Must be >= then the highest interrupt number in use
	KNumNaviEngineMaxInts = 256  // The number of interrupt sources in processors.
	};


class NaviEngineInterrupt : public Interrupt
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

#ifdef __SMP__
	/**
	 * IRQ/FIQ dispatchers
	 */
	static TUint32 IrqDispatch(TUint32 aVector);
	static void FiqDispatch();
#endif
#ifndef __SMP__
	/**
	 * IRQ/FIQ dispatchers
	 */
	static void IrqDispatch();
	static void FiqDispatch();
	/**
	 * Empty interrupt handler
	 */
	static void Spurious(TAny* anId);

	static SInterruptHandler Handlers[KNumNaviEngineInts];
#endif
	};

class NaviEngineAssp : public Asic
	{
public:
	IMPORT_C NaviEngineAssp();

public:
	/**
	 * These are the mandatory Asic class functions which are implemented here rather than in the Variant.
	 * It makes sense having an ASSP class when there is functionality at Variant/Core level which is common
	 * to a group of devices and is provided by an IP block(s) which is likely to be used in future generations
	 * of the same family of devices.
	 * In general the common functionality includes first-level Interrupt controllers, Power and Reset controllers,
	 * and timing functions
	 */

	/**
	 * initialisation
	 */
	IMPORT_C virtual void Init1();
	IMPORT_C virtual void Init3();
	/**
	 * Read and return the Startup reason of the Super Page (set up by Bootstrap)
	 * @return A TMachineStartupType enumerated value
	 * @see TMachineStartupType
	 */
	IMPORT_C virtual TMachineStartupType StartupReason();

	/**
	 * timing functions
	 */

	/**
	 * Obtain the period of System Tick timer in microseconds
	 * @return Period of System Tick timer in microseconds
	 */
	IMPORT_C virtual TInt MsTickPeriod();
	/**
	 * Obtain System Time from the RTC
	 * @return System Time in seconds from 00:00 hours of 1/1/2000
	 */
	IMPORT_C virtual TInt SystemTimeInSecondsFrom2000(TInt& aTime);
	/**
	 * Obtain Adjust the RTC with new System Time (from 00:00 hours of 1/1/2000)
	 * @return System wide error code
	 */
	IMPORT_C virtual TInt SetSystemTimeInSecondsFrom2000(TInt aTime);
	/**
	 * Obtain the time it takes to execute two processor instructions
	 * @return Time in nanoseconds it takes two execute 2 instructions at the processor clock speed
	 */
	IMPORT_C virtual TUint32 NanoWaitCalibration();
	/**
	 * @param aChar Character to be output by debug serial port
	 */
	IMPORT_C virtual void DebugOutput(TUint aChar);

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

	/**
	 * miscellaneous
	 */
	virtual TInt VideoRamSize()=0;
 	
public:
	static NaviEngineAssp* Variant;
	static TPhysAddr VideoRamPhys;
	NTimerQ* iTimerQ;
	TBool iDebugInitialised;
	};

#endif
