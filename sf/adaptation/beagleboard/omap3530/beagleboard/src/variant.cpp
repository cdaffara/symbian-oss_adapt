// Copyright (c) 1994-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/beaglboard/src/variant.cpp
//

#include <kernel.h>
#include <beagle/variant.h>
#include <beagle/mconf.h>
#include <beagle/beagle_gpio.h>
#include <assp/omap3530_assp/omap3530_uart.h>
#include <assp/omap3530_assp/omap3530_gpio.h>
#include <assp/omap3530_shared/tps65950.h>

#define ENABLE_WFI
#define IDLE_TICK_SUPPRESSION


#ifdef IDLE_TICK_SUPPRESSION
#include <assp/omap3530_shared/omap3_mstick.h>
#endif


GLREF_C void ArmWaitForInterrupt();


//These constants define Custom Restart Reasons in SuperPage::iHwStartupReason
const TUint KHtCustomRestartMax	  = 0xff;
const TUint KHtCustomRestartShift = 8;
const TUint KHtCustomRestartMask  = KHtCustomRestartMax << KHtCustomRestartShift; 

//TODO: unncomment when referenced
const TUint KHtRestartStartupModesMax = 0xf; // Variable, platform dependant 
//const TUint KHtRestartStartupModesShift = 16; // Variable, platform dependant 
//const TUint KHtRestartStartupModesMask = KHtRestartStartupModesMax << KHtRestartStartupModesShift;

void BeagleVariantFault(TInt aLine)
	{
	Kern::Fault("BeagleVariant",aLine);	
	}

#define V_FAULT()	BeagleVariantFault(__LINE__)

// Debug output
#define XON								17
#define XOFF							19
#define DEBUG_XON_XOFF					0		// Non-zero if we want XON-XOFF handshaking

GLDEF_D Beagle TheVariant;
TUint32 Variant::iBaseAddress=0;





EXPORT_C Asic* VariantInitialise()
	{
	return &TheVariant;
	}

Beagle::Beagle()
	{
	iDebugInitialised=EFalse;
	}

//
// TO DO: (optional)
//
// Specify the RAM zone configuration.
//
// The lowest addressed zone must have the highest preference as the bootstrap 
// will always allocate from the lowest address up.  Once the kernel has initialised
// then the zone preferences will decide from which RAM zone memory is allocated.
//
// 	const TUint KVariantRamZoneCount = ?;
//	static const SRamZone KRamZoneConfig[KVariantRamZoneCount+1] = 
//				 			iBase      iSize   		iID	iPref	iFlags
//				{
//				__SRAM_ZONE(0x????????, 0x???????, 	?,	?, 		?), 
//				...
//				__SRAM_ZONE(0x????????, 0x???????, 	?, 	?, 		?),
//				__SRAM_ZONE_END, // end of zone list
//				};
//

TInt Beagle::RamZoneCallback(TRamZoneOp aOp, TAny* aId, const TAny* aMasks)
	{
	//
	// TO DO: (optional)
	//
	// Handle RAM zone operations requested by the kernel.
	//
	return TheVariant.DoRamZoneCallback(aOp, (TUint)aId, (const TUint*)aMasks);
	}


TInt Beagle::DoRamZoneCallback(TRamZoneOp aOp, TUint aId, const TUint* aMasks)
	{
	//
	// TO DO: (optional)
	//
	// Handle RAM zone operations requested by the kernel.
	//
	// Three types of operation need to be supported:
	//	ERamZoneOp_Init:		Update power state of the RAM zones after the
	//							kernel has initialised.
	//	ERamZoneOp_PowerUp:		A RAM zone changing from used to empty.
	//	ERamZoneOp_PowerDown:	A RAM zone changing from empty to used.
	//
 
	switch (aOp)
		{
		case ERamZoneOp_Init:	
			break;
		case ERamZoneOp_PowerUp:
			break;
		case ERamZoneOp_PowerDown:
			break;
		default:
			return KErrNotSupported;
		}
	return KErrNone;
	}

TUint Beagle::SysClkFrequency() const
	{
	return 26000000;
	}

TUint Beagle::SysClk32kFrequency() const
	{
	return 32768;
	}

TUint Beagle::AltClkFrequency() const
	{
	// Doesn't appear to be connected on Beagle
	return 0;
	}

void Beagle::Init1()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("Beagle::Init1()"));
	Omap3530Assp::Init1();
	}


EXPORT_C TInt Variant::GetMsTickPeriod()
	{
	return TheVariant.MsTickPeriod();
	
	}

void Beagle::Init3()
	{
	__KTRACE_OPT(KBOOT,Kern::Printf("Beagle::Init3()"));
	
	Omap3530Assp::Init3();

	Variant::Init3();
	}

void Variant::Init3()
//
// Phase 3 initialisation
//
    {
	__KTRACE_OPT(KHARDWARE, Kern::Printf(">Variant::Init3"));
	}

EXPORT_C TUint Variant::BaseLinAddress()
	{
	return((TUint)iBaseAddress);
	}

EXPORT_C void Variant::MarkDebugPortOff()
	{
	TheVariant.iDebugInitialised=EFalse;
	}

EXPORT_C void Variant::UartInit()
	{
	if (!TheVariant.iDebugInitialised)
		{
		const Omap3530Uart::TUartNumber portNumber( Omap3530Assp::DebugPortNumber() );

		if( portNumber >= 0 )
			{
			Omap3530Uart::TUart uart( portNumber );
		
			uart.Init();
			uart.DefineMode( Omap3530Uart::TUart::EUart );
			uart.SetBaud( Omap3530Uart::TUart::E115200 );
			uart.SetDataFormat( Omap3530Uart::TUart::E8Data, Omap3530Uart::TUart::E1Stop, Omap3530Uart::TUart::ENone );
			uart.Enable();
			
			TheVariant.iDebugInitialised=ETrue;
			}
		}
	}

void Beagle::DebugInit()
	{
	Variant::UartInit();
	iDebugInitialised = ETrue;
	}

void Beagle::DebugOutput(TUint aLetter)
//
// Output a character to the debug port
//
    {
	const Omap3530Uart::TUartNumber portNumber( Omap3530Assp::DebugPortNumber() );

	if( portNumber >= 0 )
		{
		if (!iDebugInitialised)
			{
			DebugInit();
			}
		Omap3530Uart::TUart uart( portNumber );
		// If the FIFO is full we need to wait..
		while( uart.TxFifoFull() );
		uart.Write( aLetter );
		}
    }

void Beagle::Idle()
//
// The NULL thread idle loop
//
	{
	// Idle the CPU, suppressing the system tick if possible

	//
	// TO DO: (optional)
	//
	// Idle Tick supression: 
	// 1- obtain the number of idle Ticks before the next NTimer expiration (NTimerQ::IdleTime())
	// 2- if the number of Ticks is large enough (criteria to be defined) reset the Hardware Timer
	//    to only interrupt again when the corresponding time has expired.
	//   2.1- the calculation of the new value to program the Hardware Timer with should take in 
	//		  consideration the rounding value (NTimerQ::iRounding)
	//  3- call the low level Sleep function (e'g. Bootstrap: address in iIdleFunction)
	//  4- on coming back from Idle need to read the Hardware Timer and determine if woken up due to 
	//     timer expiration (system time for new match<=current system time<system time for new match-tick period)
	//     or some other Interrupt.
	//	 4.1- if timer expiration, adjust System Time by adding the number of Ticks suppressed to NTimerQ::iMsCount
	//   4.2- if other interrupt, calculate the number of Ticks skipped until woken up and adjust the System Time as
	//		  above
	//
	// Support for different Sleep Modes:
	// Often the Sleep mode a platform can go to depends on how many resources such as clocks/voltages can be 
	// turned Off or lowered to a suitable level. If different Sleep modes are supported this code may need 
	// to be able to find out what power resources are On or Off or used to what level. This could be achieved by
	// enquiring the Resource Manager (see \beagle_variant\inc\beagle_power.h).
	// Then a decision could be made to what Sleep level we go to.
	//
	// Example calls:
	// Obtain the number of Idle Ticks before the next NTimer expiration
	// TInt aTicksLeft = NTimerQ::IdleTime();
	// ... 
	// Find out the deepest Sleep mode available for current resource usage and sleeping time
	// TemplateResourceManager* aManager = TTemplatePowerController::ResourceManager();
	// TemplateResourceManager::TSleepModes aMode = aManager -> MapSleepMode(aTicksLeft*MsTickPeriod());
	// ...
	// Find out the state of some particular resources
	// TBool aResourceState = aManager -> GetResourceState(TemplateResourceManager::AsynchBinResourceUsedByZOnly);
	// TUint aResourceLevel = aManager -> GetResourceLevel(TemplateResourceManager::SynchMlResourceUsedByXOnly);
	// ...

#ifdef ENABLE_WFI
	TInt irq = NKern::DisableAllInterrupts();

# ifdef IDLE_TICK_SUPPRESSION
	TInt maxSleepTicks = NTimerQ::IdleTime();
	TInt suppressedTicks = Omap3::MsTick::SuppressIdleTicks( maxSleepTicks );
# endif

	ArmWaitForInterrupt();


# ifdef IDLE_TICK_SUPPRESSION
	if( suppressedTicks > 0 )
		{
		suppressedTicks = Omap3::MsTick::EndIdleTickSuppression( suppressedTicks );
		if( suppressedTicks > 0 )
			{
			NTimerQ::Advance( suppressedTicks );
			}
		}
# endif

	NKern::RestoreInterrupts( irq );
#endif // ifdef ENABLE_WFI
	}

TInt Beagle::VariantHal(TInt aFunction, TAny* a1, TAny* a2)
	{
	TInt r=KErrNone;
	switch(aFunction)
		{
		case EVariantHalVariantInfo:
			{
			TVariantInfoV01Buf infoBuf;
			TVariantInfoV01& info=infoBuf();
			info.iRomVersion=Epoc::RomHeader().iVersion;

			//
			// TO DO: (mandatory)
			//
			// Fill in the TVariantInfoV01 info structure
			//	info.iMachineUniqueId=;
			//	info.iLedCapabilities=;
			//	info.iProcessorClockInKHz=;
			//	info.iSpeedFactor=;
			//
			Kern::InfoCopy(*(TDes8*)a1,infoBuf);
			break;
			}
		case EVariantHalDebugPortSet:
			{
			//
			// TO DO: (mandatory)
			//
			// Write the iDebugPort field of the SuperPage, as in the following EXAMPLE ONLY:
			//
			TUint32 thePort = (TUint32)a1;
			switch(thePort)
				{
				case 1:
				case 2:
				case 3:
					TheVariant.iDebugInitialised=EFalse;
				case (TUint32)KNullDebugPort:
					Kern::SuperPage().iDebugPort = thePort;
					break;
				default:
					r=KErrNotSupported;
				}
			break;
			}
		case EVariantHalDebugPortGet:
			{
			kumemput32(a1, &Kern::SuperPage().iDebugPort, sizeof(TUint32));
			break;
			}
		case EVariantHalSwitches:
			{
			//
			// TO DO: (optional)
			//
			// Read the state of any switches, as in the following EXAMPLE ONLY:
			//
			TUint32 x = Variant::Switches();
			kumemput32(a1, &x, sizeof(x));
			break;
			}
		case EVariantHalLedMaskSet:
			{
			//
			// TO DO: (optional)
			//
			// Set the state of any on-board LEDs, e.g:
			// TUint32 aLedMask=(TUint32)a1;
			// Variant::ModifyLedState(~aLedMask,aLedMask);
			//
			break;
			}
		case EVariantHalLedMaskGet:
			{
			//
			// TO DO: (optional)
			//
			// Read the state of any on-board LEDs, e.g:
			// TUint32 x = Variant::LedState();
			// kumemput32(a1, &x, sizeof(x));
			//
			break;
			}

		case EVariantHalCustomRestartReason:
			{
			//Restart reason is stored in super page
			TInt x = (Kern::SuperPage().iHwStartupReason & KHtCustomRestartMask) >> KHtCustomRestartShift ;
			kumemput32(a1, &x, sizeof(TInt));
			break;
			}

		case EVariantHalCustomRestart:
			{
			if(!Kern::CurrentThreadHasCapability(ECapabilityPowerMgmt,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EVariantHalCustomRestart")))
				return KErrPermissionDenied;
			if ((TUint)a1 > KHtCustomRestartMax)
				return KErrArgument;
			Kern::Restart((TInt)a1 << KHtCustomRestartShift);
			}
			break;

		case EVariantHalCaseState:
			{
			//
			// TO DO: (optional)
			//
			// Read the state of the case, e.g:
			// TUint32 x = Variant::CaseState();
			// kumemput32(a1, &x, sizeof(x));
			//
			break;
			}

		case EVariantHalPersistStartupMode:
			{
			if (!Kern::CurrentThreadHasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetBacklightOn")))
				return KErrPermissionDenied;

			if ((TUint)a1 > KHtRestartStartupModesMax ) // Restart startup mode max value
				return KErrArgument;
			//
			// TO DO: (optional)
			//
			// Store the restart reason locally,
			// which will eventually be picked up by
			// the power controller, e.g:
			// iCustomRestartReason = (TUint)a1;
			break;
			}


		case EVariantHalGetPersistedStartupMode:
			{
			//
			// TO DO: (optional)
			//
			// Read the restart startup mode, e.g:
			// TInt startup = (Kern::SuperPage().iHwStartupReason & KHtRestartStartupModesMask) >> KHtRestartStartupModesShift;
			// kumemput32(a1, &startup, sizeof(TInt));
			break; 			
			}

		case EVariantHalGetMaximumCustomRestartReasons:
			{
			//
			// TO DO: (optional)
			//
			// Read the maximum custom restart reason, e.g:
			// kumemput32(a1, &KHtCustomRestartMax, sizeof(TUint));
			break;
			}


		case EVariantHalGetMaximumRestartStartupModes:
			{
			//
			// TO DO: (optional)
			//
			// Read the maximum restart startup mode, e.g:
			// kumemput32(a1, &KHtRestartStartupModesMax, sizeof(TUint));
			break;
			}
		

		default:
			r=KErrNotSupported;
			break;
		}
	return r;
	}

TPtr8 Beagle::MachineConfiguration()
	{
	return TPtr8((TUint8*)&Kern::MachineConfig(),sizeof(TActualMachineConfig),sizeof(TActualMachineConfig));
	}


EXPORT_C void Variant::PowerReset()
	{
	//
	// TO DO: (optional)
	//
	// Reset all power supplies
	//
	}

EXPORT_C TUint Variant::Switches()
	{
	//
	// TO DO: (optional)
	//
	// Read the state of on-board switches
	//
	return 0;		// EXAMPLE ONLY
	}

// USB Client controller

TBool Beagle::UsbClientConnectorDetectable()
	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf("Beagle::UsbClientConnectorDetectable"));

	// TO DO: The return value should reflect the actual situation.
	return ETrue;
	}


TBool Beagle::UsbClientConnectorInserted()
 	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf("Beagle::UsbClientConnectorInserted"));

	// TO DO: Query cable status here. The return value should reflect the actual current state.
	return ETrue;
	}


TInt Beagle::RegisterUsbClientConnectorCallback(TInt (*aCallback)(TAny*), TAny* aPtr)
	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf("Beagle::RegisterUsbClientConnectorCallback"));

	iUsbClientConnectorCallback = aCallback;
	iUsbClientConnectorCallbackArg = aPtr;

	// TO DO: Register and enable the interrupt(s) for detecting USB cable insertion/removal here.
	// (Register UsbClientConnectorIsr.)

	// TO DO: The return value should reflect the actual situation.
	return KErrNone;
	}


void Beagle::UnregisterUsbClientConnectorCallback()
	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf("Beagle::UnregisterUsbClientConnectorCallback"));

	// TO DO: Disable and unbind the interrupt(s) for detecting USB cable insertion/removal here.

	iUsbClientConnectorCallback = NULL;
	iUsbClientConnectorCallbackArg = NULL;
	}


TBool Beagle::UsbSoftwareConnectable()
	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf("Beagle::UsbSoftwareConnectable"));

	// TO DO: The return value should reflect the actual situation.
	return ETrue;
	}


TInt Beagle::UsbConnect()
	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf("Beagle::UsbConnect"));

	// TO DO: Do here whatever is necessary for the UDC to appear on the bus (and thus to the host).

	return KErrNone;
	}


TInt Beagle::UsbDisconnect()
	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf("Beagle::UsbDisconnect"));

	// TO DO: Do here whatever is necessary for the UDC to appear disconnected from the bus (and thus from the
	// host).

	return KErrNone;
	}


void Beagle::UsbClientConnectorIsr(TAny *aPtr)
//
// Services the USB cable interrupt.
//
	{
	__KTRACE_OPT(KHARDWARE, Kern::Printf("Beagle::UsbClientConnectorIsr()"));

	Beagle* tm = static_cast<Beagle*>(aPtr);

	// TO DO: Service interrupt here: determmine cause, clear condition flag (if applicable), etc.

	if (tm->UsbClientConnectorInserted())
		{
		__KTRACE_OPT(KHARDWARE, Kern::Printf(" > USB cable now inserted."));
		}
	else
		{
		__KTRACE_OPT(KHARDWARE, Kern::Printf(" > USB cable now removed."));
		}

	// Important: Inform the USB stack.
	if (tm->iUsbClientConnectorCallback)
		{
		(*tm->iUsbClientConnectorCallback)(tm->iUsbClientConnectorCallbackArg);
		}
	}

// Used to convert time to BCD and vice-versa
const TInt KSecsPerMin = 60;
const TInt KSecsPerHour = 60*KSecsPerMin;
const TInt KSecsPerDay = 24*KSecsPerHour;
//const TInt KSecsPerLeapYr = 366*KSecsPerDay;
const TInt KSecsPerYr = 365*KSecsPerDay;
const TInt KSecsDaysPer4Years = (3*KSecsPerYr)+ 366*KSecsPerDay;

//#define BCDTONUM0_3_4_7(a) ((a&0xf)+(((a)>>4)*10))
#define BCDTONUM0_3_4_6(a) ((a&0xf)+((((a)>>4)&7)*10))

// Days in each month
LOCAL_D const TInt8 mTab[2][12]=
    {
    {31,28,31,30,31,30,31,31,30,31,30,31}, // 28 days in Feb
    {31,29,31,30,31,30,31,31,30,31,30,31}  // 29 days in Feb
    };

void GetMonthData(TInt aDayInYear, TBool aLeap, TUint8& aMonth, TUint8& aDay )
/**
 Work out day of the month and month
 @param aDayInYear	Day of the year
 @param aLeap		True if it is a leap year
 @param aMonth		Return month (range 01-12)
 @param aDay		Return day of the month (range 01-31)
 */
{
	TInt i;
	TInt runtot=0;
	for (i=0; i<12; i++)
	{
		if ((aDayInYear>=runtot) && (aDayInYear < mTab[aLeap][i]+runtot))
		{
			// Month and day of the month both start from 1, rather than
			// zero (hence the +1)
			aMonth=i+1;
			aDay=aDayInYear-runtot+1;
			break;
		}
		runtot+=mTab[aLeap][i];
	}
}

LOCAL_C void SecondsToYMD( const TInt aTime, TUint8& aYear, TUint8& aMonth, TUint8& aDay )
/**
 Work out year, day of the month and month
 @param aTime	Time in secs from year 2000
 @param aYear	Return year number
 @param aMonth	Return month (range 01-12)
 @param aDay	Return day of the month (range 01-31)
 */
{
	// Work out year within 4 years first
	aYear = (aTime / KSecsDaysPer4Years)*4;
	aDay=0;
	aMonth=0;
	TInt adjyear = aTime % KSecsDaysPer4Years;
	
	
	if (adjyear<KSecsPerYr + KSecsPerDay)
		{
		GetMonthData(adjyear/KSecsPerDay, ETrue, aMonth, aDay);
		}
	else
		{
		adjyear-=(KSecsPerYr + KSecsPerDay);
		aYear+=(adjyear/KSecsPerYr)+1;
		GetMonthData((adjyear%KSecsPerYr)/KSecsPerDay, EFalse, aMonth, aDay);
		}
}


TInt  Beagle::SystemTimeInSecondsFrom2000(TInt& aTime)
	{
	
	if(!TPS65950::Initialized())
		{
		return KErrNotSupported;
		}

	TPS65950::TRtcTime  time;
	TPS65950::GetRtcData( time );
		 
	aTime = time.iSecond;
	aTime += time.iMinute * KSecsPerMin;
	aTime += time.iHour * KSecsPerHour;

	// Careful - day starts from 1
	aTime += (time.iDay-1) * KSecsPerDay;

	// Determine whether it is a leap year, for the purpose of this chip
	// years run from 2000 onwards and the driver won't care beyond
	// yr2000 + 0x7fffffff(secs) (60 odd years). In brief, we can just divide
	// by 4 and ignore the problem of years divisible by 100
	TInt yrs= time.iYear;
	TUint yrsMod4 = yrs%4;
	TBool isLeap = (yrsMod4) ? EFalse : ETrue;

	// Careful - month starts from 1
	for (TInt i=0; i < time.iMonth-1; i++)
		{
		aTime += mTab[isLeap][i] * KSecsPerDay;
		}
	
	aTime += (yrs/4) * KSecsDaysPer4Years;
	
	if ( isLeap )
		{
		// Add KSecsPerDay, because first year is always a leap year
		aTime+=(KSecsPerYr*(yrsMod4))+KSecsPerDay;
		}
	return KErrNone;
	}

TInt Beagle::SetSystemTimeInSecondsFrom2000(TInt aTime)
	{
	if(!TPS65950::Initialized())
		{
		return KErrNotSupported;
		}
		
	TPS65950::TRtcTime  rtc;	
	TInt secs = aTime % KSecsPerMin;
	TInt mins_insecs = (aTime % KSecsPerHour) - secs;
	TInt hours_insecs = (aTime % KSecsPerDay) - mins_insecs - secs;

	rtc.iSecond = secs;
	rtc.iMinute = mins_insecs/KSecsPerMin;
	rtc.iHour = hours_insecs/KSecsPerHour;

	SecondsToYMD( aTime, rtc.iYear, rtc.iMonth, rtc.iDay);
	
	TPS65950::SetRtcData( rtc );

	return KErrNone;
	}


TInt Beagle::IsExternalInterrupt(TInt /*anId*/)
	{
	return EFalse;
	}

TInt Beagle::InterruptBind(TInt /*anId*/, TIsr /*anIsr*/, TAny* /*aPtr*/)
	{
	return KErrNotSupported;
	}

TInt Beagle::InterruptUnbind(TInt /*anId*/)
	{
	return KErrNotSupported;
	}

TInt Beagle::InterruptEnable(TInt /*anId*/)
	{
	return KErrNotSupported;
	}

TInt Beagle::InterruptDisable(TInt /*anId*/)
	{
	return KErrNotSupported;
	}

TInt Beagle::InterruptClear(TInt /*anId*/)
	{
	return KErrNotSupported;
	}






//---eof
