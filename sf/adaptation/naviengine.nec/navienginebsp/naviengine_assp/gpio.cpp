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
* naviengine_assp\gpio.cpp
* NaviEngine implementation of the MHA GPIO class
*
*/



#include <naviengine_priv.h>
#include <gpio.h>

const TInt32		KHwGpioPinMax		= 31;
const TInt32		KHwGpioEdgeRising	= 0x0;
const TInt32		KHwGpioEdgeFalling	= 0x1;
const TInt32		KHwGpioEdgeBoth		= 0x2;
const TInt32		KHwGpioLevelLow		= 0x3;
const TInt32		KHwGpioLevelHigh	= 0x4;

const  TInt32		KGpioDebounceInterval = 1; // (in ticks)
const  TInt32		KGpioMaxRetries = 8;

class GpioPin
	{
public:
	GPIO::TGpioMode	iMode;
   	TGpioIsr		iIsr;
	TAny *			iPtr;
	TInt			iDebounce;
	};

static GpioPin	GpioPins[KHwGpioPinMax+1];
static TInt32	GpioInterruptId;
#ifdef __SMP__
const  TInt32		KGpioLockOrder = TSpinLock::EOrderGenericIrqLow2;
static TSpinLock	GpioSpinLock(KGpioLockOrder);
#endif

inline TInt GpioLock();
inline void GpioUnlock(TInt irq);

/**
Calculate 16-bit device pin Id from 32-bit pin Id. Use DeviceId() to 
get device Id.
@param   aId         32-bit pin Id
@return  16-bit device specific pin Id
*/
static inline TUint16 DevicePinId(TInt aId)
    {return static_cast<TUint16>(aId & 0x0000FFFF);}


//Commented out to satisfy compiler(as method is not used in the code) but can  
//be usefull later
/**
Calculate and return GPIO device Id(either SOC or one of the extenders)
defined in TGpioBaseId from the 32-bit pin Id
@param   aId         32-bit pin Id
@return
   - EInternalId              SOC GPIO 
   - EExtender0-15            GPIO extenders from 0-15

static inline GPIO::TGpioBaseId ExtenderId(TInt aId)
    {return static_cast<GPIO::TGpioBaseId>((aId & 0xFFFF0000));}
*/

//Commented out to satisfy compiler(as method is not used in the code) but can  
//be usefull later
/**
Generate 32-bit pin Id from the device Id and device specific 16-bit 
pin Id.
@param   aExtenderId     Device Id is defined in TGpioBaseId
@param   aPinId          16-bit device pin Id
return   32-bit pin Id  

static inline TInt Id(GPIO::TGpioBaseId aExtenderId, TUint16 aPinId)
    {return static_cast<TInt>(aExtenderId |aPinId);}
*/

//Commented out to satisfy compiler(as method is not used in the code) but can  
//be usefull later
/**
Find index in extender GPIO device table.
@param   aExtenderId     Extender Id is defined in TGpioBaseId
@return  singned 32-bit integer index device, possible value 
        from 0 to 15

static TInt DeviceIndex(GPIO::TGpioBaseId aExtenderId)
    {
    TUint16 val = (TUint16)((aExtenderId & 0xFFFF0000) >> 16);
    if(val == 0) return GPIO::EInternalId;

    //The algorithm steps througth the value until first non-zero bit is
    //found.
    //
    TInt index = 0;
    if(val & 0xFF00) {index  = 8; val = val >> 8;} // 2 x 8-bits
    if(val & 0x00F0) {index += 4; val = val >> 4;} // 2 x 4-bits
    if(val & 0x000C) {index += 2; val = val >> 2;} // 2 x 2 bits
    if(val & 0x0002) {index += 1; val = val >> 1;} // 2 x 1 bits

    return index;
    }
*/

//Commented out to satisfy compiler(as method is not used in the code) but can  
//be usefull later
/**
Find index in extender GPIO device table.
@param   aId    32-bit GPIO pin Id
@return  singned 32-bit integer index device, possible value 
         from 0 to 15

static TInt DeviceIndex(TInt aId){return DeviceIndex(ExtenderId(aId));}
*/



/**
GPIO interrupt handler
Takes a generic argument (TAny*)
*/
void GpioIsrDispatch(TAny *aPtr)
	{
	GpioPin	*pins = (GpioPin *)aPtr;
	TInt irq = GpioLock();
	TUint32 interrupt = AsspRegister::Read32(KHwRwGpio_Int);
	TUint32 enabled = AsspRegister::Read32(KHwRwGpio_Int_Enable);
	TUint32	masked = interrupt & enabled;
	for (TInt i = 0; i <= KHwGpioPinMax; i++)
		{
		if ((masked & 0x1) && (pins[i].iIsr != NULL))
			{
			(*pins[i].iIsr)(pins[i].iPtr);
			}
		masked >>= 1;
		}
	Interrupt::Clear(GpioInterruptId);
	GpioUnlock(irq);
	}

#include "gpio.inl"

//
// work out debounced state
//
TInt GetDebouncedState(TInt aId, TUint32 &aVal)
	{

    TUint16   pinId = DevicePinId(aId);
	TInt	debounceCount = GpioPins[pinId].iDebounce/KGpioDebounceInterval;
	TInt	count = 0;
	TInt	retries = debounceCount*KGpioMaxRetries;
	TUint	lastState = AsspRegister::Read32(KHwRoGpio_Port_Value);
	TUint	state = lastState;

	for (count = 0; count < debounceCount && retries > 0; count ++)
		{
		NKern::Sleep(KGpioDebounceInterval);
		state = AsspRegister::Read32(KHwRoGpio_Port_Value);

		if ((state & 1<<pinId) != (lastState & 1<<pinId))
			{
			// state has changed so reset count and lastState
			lastState = AsspRegister::Read32(KHwRoGpio_Port_Value);
			count = 0;
			retries--;
			}
		}

	if (retries == 0)
		{
		// too many retries
		return KErrTimedOut;
		}

	aVal = state;
	return KErrNone;
	}

EXPORT_C TInt GPIO::SetPinMode
	(
	TInt      aId,
   	TGpioMode aMode
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// the chip doesn't support modes, so just store it
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}

	__e32_atomic_store_rel32(&GpioPins[pinId].iMode, aMode);
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetPinMode
	(
	TInt        aId,
   	TGpioMode & aMode
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// the chip doesn't support modes, so just return what we stored earlier
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	aMode = (TGpioMode) __e32_atomic_load_acq32(&GpioPins[pinId].iMode);
	return KErrNone;
	}

EXPORT_C TInt GPIO::SetPinDirection
	(
	TInt           aId,
   	TGpioDirection aDirection
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// tristate not supported
	if (pinId > KHwGpioPinMax || aDirection == ETriStated)
		{
		return KErrArgument;
		}
	// port enabled means output, disabled means input
	if (aDirection == EInput)
		{
		AsspRegister::Write32(KHwWoGpio_Port_Control_Disable, 1<<pinId);
		}
	else
		{
		AsspRegister::Write32(KHwRwGpio_Port_Control_Enable, 1<<pinId);
		}
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetPinDirection
	(
	TInt             aId,
   	TGpioDirection & aDirection
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	// port enabled means output, disabled means input
	TUint enabled = AsspRegister::Read32(KHwRwGpio_Port_Control_Enable);
	if (enabled & 1<<pinId)
		{
		aDirection = EOutput;
		}
	else
		{
		aDirection = EInput;
		}
	return KErrNone;
	}

EXPORT_C TInt GPIO::SetPinBias
	(
	TInt      aId,
   	TGpioBias /*aBias*/
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// pin bias not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::GetPinBias
	(
	TInt        aId,
   	TGpioBias& /*aBias*/
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// pin bias not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::SetPinIdleConfigurationAndState
	(
	TInt        aId,
   	TInt		/*aConf*/
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// pin idle configuration and state not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::GetPinIdleConfigurationAndState
	(
	TInt        aId,
   	TInt	  & /*aBias*/
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// pin idle configuration and state not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::BindInterrupt
	(
	TInt     aId,
   	TGpioIsr aIsr,
   	TAny *   aPtr
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax || aIsr == NULL)
		{
		return KErrArgument;
		}
	TInt irq = GpioLock();
	if (GpioPins[pinId].iIsr != NULL)
		{
		GpioUnlock(irq);
		// already bound
		return KErrInUse;
		}
	GpioPins[pinId].iIsr = aIsr;
	GpioPins[pinId].iPtr = aPtr;
	GpioUnlock(irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::UnbindInterrupt
	(
	TInt aId
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	TInt irq = GpioLock();
	if (GpioPins[pinId].iIsr == NULL)
		{
		GpioUnlock(irq);
		// nothing bound
		return KErrGeneral;
		}
	GpioPins[pinId].iIsr = NULL;
	GpioPins[pinId].iPtr = NULL;
	GpioUnlock(irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::EnableInterrupt
	(
	TInt aId
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	TInt irq = GpioLock();
	if (GpioPins[pinId].iIsr == NULL)
		{
		GpioUnlock(irq);
		// nothing bound
		return KErrGeneral;
		}

	// hold value so it can be read after triggering
	TUint32 held = AsspRegister::Read32(KHwRwGpio_Int_Hold);
	AsspRegister::Write32(KHwRwGpio_Int_Hold, held | 1<<pinId);
	// enable interrupt
	AsspRegister::Write32(KHwRwGpio_Int_Enable, 1<<pinId);
	GpioUnlock(irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::DisableInterrupt
	(
	TInt aId
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	TInt irq = GpioLock();
	if (GpioPins[pinId].iIsr == NULL)
		{
		GpioUnlock(irq);
		// nothing bound
		return KErrGeneral;
		}
	// disable interrupt
	AsspRegister::Write32(KHwWoGpio_Int_Disable, 1<<pinId);
	// disable hold
	TUint32 held = AsspRegister::Read32(KHwRwGpio_Int_Hold);
	AsspRegister::Write32(KHwRwGpio_Int_Hold, held & !(1<<pinId));
	GpioUnlock(irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::IsInterruptEnabled
	(
	TInt    aId,
   	TBool & aEnable
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	TInt irq = GpioLock();
	if (GpioPins[pinId].iIsr == NULL)
		{
		GpioUnlock(irq);
		// nothing bound
		return KErrGeneral;
		}
	aEnable = AsspRegister::Read32(KHwRwGpio_Int_Enable) & (1<<pinId);
	GpioUnlock(irq);

	return KErrNone;
	}

EXPORT_C TInt GPIO::ClearInterrupt
	(
	TInt aId
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	// clear pin interrupt status
	AsspRegister::Write32(KHwRwGpio_Int, 1<<pinId);
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetMaskedInterruptState
	(
	TInt    aId,
   	TBool & aActive
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	TInt irq = GpioLock();
	aActive = AsspRegister::Read32(KHwRwGpio_Int_Enable) & AsspRegister::Read32(KHwRwGpio_Int) & (1<<pinId);
	GpioUnlock(irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetRawInterruptState
	(
	TInt    aId,
   	TBool & aActive
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	aActive = AsspRegister::Read32(KHwRwGpio_Int) & (1<<pinId);
	return KErrNone;
	}

EXPORT_C TInt GPIO::SetInterruptTrigger
	(
	TInt                  aId,
   	TGpioDetectionTrigger aTrigger
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	TUint modeRegister = KHwRwGpio_Int_Mode0 + ((pinId >> 3) << 2);
	TUint modeShift = pinId & 0x7;
	TUint mode;
	switch (aTrigger)
		{
	case ELevelLow:
		mode = KHwGpioLevelLow;
		break;
	case ELevelHigh:
		mode = KHwGpioLevelHigh;
		break;
	case EEdgeFalling:
		mode = KHwGpioEdgeFalling;
		break;
	case EEdgeRising:
		mode = KHwGpioEdgeRising;
		break;
	case EEdgeBoth:
		mode = KHwGpioEdgeBoth;
		break;
	default:
		return KErrArgument;
		}

	TInt irq = GpioLock();
	TUint currentMode = AsspRegister::Read32(modeRegister) & !(0xf << modeShift);
	AsspRegister::Write32(modeRegister, (mode << modeShift) | currentMode);
	GpioUnlock(irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::EnableWakeup
	(
	TInt aId
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// wakeup not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::DisableWakeup
	(
	TInt aId
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// wakeup not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::IsWakeupEnabled
	(
	TInt    aId,
   	TBool & /*aEnable*/
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// wakeup not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::SetWakeupTrigger
	(
	TInt                  aId,
   	TGpioDetectionTrigger /*aTrigger*/
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// wakeup not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::SetDebounceTime
	(
	TInt aId,
   	TInt aTime
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	TInt irq = GpioLock();
	GpioPins[pinId].iDebounce = NKern::TimerTicks(aTime/1000);
	GpioUnlock(irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetDebounceTime
	(
	TInt   aId,
   	TInt & aTime
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	TInt irq = GpioLock();
	aTime = (GpioPins[pinId].iDebounce*1000)/NKern::TimerTicks(1);
	GpioUnlock(irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetInputState
	(
	TInt         aId,
   	TGpioState & aState
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	// check it is enabled and is an input port
	TInt irq = GpioLock();
	if (GpioPins[pinId].iMode != EEnabled || (AsspRegister::Read32(KHwRwGpio_Port_Control_Enable) & 1<<pinId)) // reg will have 0 for input 1 for output
		{
		GpioUnlock(irq);
		return KErrGeneral;
		}
	GpioUnlock(irq);
	TUint32 state;
	if (GpioPins[pinId].iDebounce == 0)
		{
		// just read it if we are not debouncing
		state = AsspRegister::Read32(KHwRoGpio_Port_Value);
		}
	else
		{
		// work out debounced state
		TInt err = GetDebouncedState(aId, state);
		if (err != KErrNone)
			{
			return err;
			}
		}

	if (state & (1<<pinId))
		{
		aState = EHigh;
		}
	else
		{
		aState = ELow;
		}

	return KErrNone;
	}

EXPORT_C TInt GPIO::SetOutputState
	(
	TInt       aId,
   	TGpioState aState
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	TInt irq = GpioLock();
	if (GpioPins[pinId].iMode != EEnabled)
		{
		GpioUnlock(irq);
		return KErrGeneral;
		}
	TUint outputReg, value;
	if (pinId >= 16)
		{
		outputReg = KHwWoGpio_Port_Set_Clear_Hi;
		pinId = pinId - 16;
		}
	else
		{
		outputReg = KHwWoGpio_Port_Set_Clear_Lo;
		}
	if (aState == EHigh)
		{
		value = 1 << pinId;   	// LSW - set..
		}
	else
		{
		value = 1 << (pinId + 16);  // MSW - clear..
		}

	AsspRegister::Write32(outputReg, value);
	GpioUnlock(irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetOutputState
	(
	TInt         aId,
   	TGpioState & aState
	)
	{
    TUint16 pinId = DevicePinId(aId);
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	// atomic - no lock required
	TUint32 state = AsspRegister::Read32(KHwRoGpio_Port_Value);
	if(state & (1<<pinId))
		{
		aState = EHigh;
		}
	else
		{
		aState = ELow;
		}
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetInputState
	(
	TInt                  aId,
   	TGpioCallback		* /*aCb*/
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// asynch calls not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::SetOutputState
	(
	TInt                  aId,
	TGpioState			  /*aState*/,
   	TGpioCallback		* /*aCb*/
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// asynch calls not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}

#ifndef __USE_GPIO_STATIC_EXTENSION__
EXPORT_C TInt GPIO::StaticExtension
	(
	TInt	  aId,
	TInt	  /*aCmd*/,
	TAny	* /*aArg1*/,
	TAny	* /*aArg2*/
	)
	{
    TUint16 pinId = DevicePinId(aId);
	// static extensions not supported
	if (pinId > KHwGpioPinMax)
		{
		return KErrArgument;
		}
	return KErrNotSupported;
	}
#endif

DECLARE_STANDARD_EXTENSION()
	{
	TInt irq = GpioLock();
	// initialise GPIO pins array
	for (TInt32 i = 0; i <= KHwGpioPinMax; i++)
		{
		GpioPins[i].iMode = GPIO::EIdle;
		GpioPins[i].iIsr = NULL;
		GpioPins[i].iPtr = NULL;
		GpioPins[i].iDebounce = 0;
		}
	GpioUnlock(irq);
	TInt r = Interrupt::Bind(KIntIdGpio, GpioIsrDispatch, &GpioPins[0]);
	if (r < 0)
		{
		return r;
		}
	GpioInterruptId = r;
	Interrupt::Clear(GpioInterruptId);
	Interrupt::Enable(GpioInterruptId);
	return KErrNone;
	}
