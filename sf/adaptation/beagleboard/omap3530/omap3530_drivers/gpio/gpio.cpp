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
// omap3530/omap3530_drivers/gpio/gpio.cpp
//

#include <kern_priv.h>
#include <assp/omap3530_assp/omap3530_gpio.h>
#include <assp/omap3530_assp/omap3530_hardware_base.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>

#include <assp.h>
//#include <nkern.h>
#include <assp/omap3530_assp/gpio.h>

GLREF_C TInt InitGpioInterrupts();


TSpinLock GPIOModeLock(/*TSpinLock::EOrderNone*/);
TSpinLock GPIOWakeLock(/*TSpinLock::EOrderNone*/);
TSpinLock GPIOLevelLock(/*TSpinLock::EOrderNone*/);

GPIO::TGpioMode	ThePinMode[ KHwGpioPinMax ];
TUint32 ThePinIsEnabled[ KHwGpioBanks ];
__ASSERT_COMPILE( KHwGpioPinsPerBank <= 32 );

#if 0
static void dumpGpioBank(TUint aBankAddr)
	{
		Kern::Printf("GPIO_SYSCONFIG at %x is  %x",aBankAddr +KGPIO_SYSCONFIG,AsspRegister::Read32(aBankAddr +KGPIO_SYSCONFIG) );
		Kern::Printf("GPIO_SYSSTATUS at %x is  %x",aBankAddr +KGPIO_SYSSTATUS,AsspRegister::Read32(aBankAddr +KGPIO_SYSSTATUS));
		Kern::Printf("GPIO_IRQSTATUS1 at %x is  %x",aBankAddr +KGPIO_IRQSTATUS1,AsspRegister::Read32(aBankAddr +KGPIO_IRQSTATUS1) );
		Kern::Printf("GPIO_IRQENABLE1 at %x is  %x",aBankAddr +KGPIO_IRQENABLE1,AsspRegister::Read32(aBankAddr +KGPIO_IRQENABLE1) );
		Kern::Printf("GPIO_WAKEUPENABLE at %x is  %x",aBankAddr +KGPIO_WAKEUPENABLE,AsspRegister::Read32(aBankAddr +KGPIO_WAKEUPENABLE) );		
		Kern::Printf("GPIO_CTRL at %x is  %x",aBankAddr +KGPIO_CTRL,AsspRegister::Read32(aBankAddr +KGPIO_CTRL) );
		Kern::Printf("GPIO_OE at %x is  %x",aBankAddr +KGPIO_CTRL,AsspRegister::Read32(aBankAddr +KGPIO_OE) );
	}
#endif


EXPORT_C TInt GPIO::SetPinMode(TInt aId, TGpioMode aMode)
	{	
	TInt irq = __SPIN_LOCK_IRQSAVE(GPIOModeLock);
	if(ThePinMode[ aId ] != aMode)
		{
		ThePinMode[ aId ] = aMode;
		TUint bank = GPIO_PIN_BANK( aId );
		TUint pinMask = 1 << GPIO_PIN_OFFSET( aId );

		if( aMode == GPIO::EEnabled)
			{
			if( 0 == ThePinIsEnabled[ bank ] )
				{
				// First enabled pin in bank
				AsspRegister::Modify32(GPIO_BASE_ADDRESS( aId ) +KGPIO_CTRL,KClearNone,0x1);
				}

			ThePinIsEnabled[ bank ] |= pinMask;
			}
		else
			{
			ThePinIsEnabled[ bank ] &= ~pinMask;
			
			if( 0 == ThePinIsEnabled[ bank ] )
				{
				// Bank can be disabled
				AsspRegister::Modify32(GPIO_BASE_ADDRESS( aId ) +KGPIO_CTRL,0x1, KSetNone);
				}
			}
		}
	__SPIN_UNLOCK_IRQRESTORE(GPIOModeLock,irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetPinMode(TInt aId, TGpioMode & aMode)
	{
	aMode = ThePinMode[ aId ];
	return KErrNone;
	}

EXPORT_C TInt GPIO::SetPinDirection(TInt aId, TGpioDirection aDirection)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::SetPinBias OOB ",KErrArgument));
	
	if (aDirection == ETriStated)
		{
		return KErrNotSupported;
		}
			
    if (aDirection == EInput)
        {
        AsspRegister::Modify32(GPIO_BASE_ADDRESS(aId)+KGPIO_OE, KClearNone, GPIO_PIN_OFFSET(aId));
        }
    else
        {
        AsspRegister::Modify32(GPIO_BASE_ADDRESS(aId)+KGPIO_OE, GPIO_PIN_OFFSET(aId), KSetNone);
        }
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetPinDirection(TInt aId, TGpioDirection& aDirection)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::GetPinDirection OOB ",KErrArgument));
	
	if(AsspRegister::Read32(GPIO_BASE_ADDRESS(aId)+KGPIO_OE) & GPIO_PIN_OFFSET(aId))
		aDirection=EInput;	
	else
		aDirection=EOutput;
	
	return KErrNone;
	}

EXPORT_C TInt GPIO::SetPinBias(TInt aId, TGpioBias aBias)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::SetPinBias OOB ",KErrArgument));
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::GetPinBias(TInt aId, TGpioBias& aBias)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::GetPinBias OOB ",KErrArgument));
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::SetPinIdleConfigurationAndState(TInt aId, TInt /*aConf*/)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::SetPinIdleConfigurationAndState OOB ",KErrArgument));
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::GetPinIdleConfigurationAndState(TInt aId, TInt& aBias)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::GetPinIdleConfigurationAndState OOB ",KErrArgument));
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::BindInterrupt(TInt aId,	TGpioIsr aIsr, TAny* aPtr)
	{
	return Interrupt::Bind( EGPIOIRQ_FIRST + aId,aIsr,aPtr);
	}

EXPORT_C TInt GPIO::UnbindInterrupt(TInt aId)
	{
	return Interrupt::Unbind( EGPIOIRQ_FIRST + aId );
	}

EXPORT_C TInt GPIO::EnableInterrupt(TInt aId)
	{
	return Interrupt::Enable( EGPIOIRQ_FIRST + aId );	
	}

EXPORT_C TInt GPIO::DisableInterrupt(TInt aId)
	{
	return Interrupt::Disable( EGPIOIRQ_FIRST + aId );
	}

EXPORT_C TInt GPIO::ClearInterrupt(TInt aId)
	{
	return Interrupt::Clear( EGPIOIRQ_FIRST + aId );
	}

EXPORT_C TInt GPIO::IsInterruptEnabled(TInt aId, TBool& aEnable)
	{
	aEnable = AsspRegister::Read32(GPIO_BASE_ADDRESS( aId ) + KGPIO_IRQENABLE1) & GPIO_PIN_OFFSET(aId);	
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetMaskedInterruptState(TInt aId, TBool& aActive)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::GetMaskedInterruptState OOB ",KErrArgument));
	aActive = AsspRegister::Read32(GPIO_BASE_ADDRESS(aId)+KGPIO_IRQSTATUS1) & GPIO_PIN_OFFSET(aId);
	
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetRawInterruptState(TInt aId, TBool& aActive)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::GetRawInterruptState OOB ",KErrArgument));
	
	aActive = AsspRegister::Read32(GPIO_BASE_ADDRESS(aId)+KGPIO_IRQSTATUS1) & GPIO_PIN_OFFSET(aId);

	return KErrNone;
	}

EXPORT_C TInt GPIO::SetInterruptTrigger(TInt aId, TGpioDetectionTrigger aTrigger)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::SetInterruptTrigger OOB ",KErrArgument));
	
	TInt baseAddr = GPIO_BASE_ADDRESS(aId);
	TUint irqFlags=0;
	//first we clear the current trigger(s)
	//then set the new for each case
	switch (aTrigger)
		{
		case ELevelLow:

			irqFlags = NKern::DisableAllInterrupts();//__SPIN_LOCK_IRQSAVE_W(GPIOLevelSpinLock);
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT1, GPIO_PIN_OFFSET(aId),KSetNone);
			AsspRegister::Modify32(baseAddr+KGPIO_FALLINGDETECT, GPIO_PIN_OFFSET(aId),KSetNone);
			AsspRegister::Modify32(baseAddr+KGPIO_RISINGDETECT, GPIO_PIN_OFFSET(aId),KSetNone);
			
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT0, KClearNone, GPIO_PIN_OFFSET(aId));
			NKern::RestoreInterrupts(irqFlags);//__SPIN_UNLOCK_IRQRESTORE_W(GPIOLevelSpinLock,irqFlags);	
			break;
		case ELevelHigh:
			
			irqFlags = NKern::DisableAllInterrupts();//__SPIN_LOCK_IRQSAVE_W(GPIOLevelSpinLock);
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT0, GPIO_PIN_OFFSET(aId),KSetNone);
			AsspRegister::Modify32(baseAddr+KGPIO_FALLINGDETECT, GPIO_PIN_OFFSET(aId),KSetNone);
			AsspRegister::Modify32(baseAddr+KGPIO_RISINGDETECT, GPIO_PIN_OFFSET(aId),KSetNone);
				
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT1, KClearNone, GPIO_PIN_OFFSET(aId));
			NKern::RestoreInterrupts(irqFlags);//__SPIN_UNLOCK_IRQRESTORE_W(GPIOLevelSpinLock,irqFlags);
				
			break;
		case EEdgeFalling:

			irqFlags = NKern::DisableAllInterrupts();//__SPIN_LOCK_IRQSAVE_W(GPIOLevelSpinLock);
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT0, GPIO_PIN_OFFSET(aId),KSetNone);
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT1, GPIO_PIN_OFFSET(aId),KSetNone);
			AsspRegister::Modify32(baseAddr+KGPIO_RISINGDETECT, GPIO_PIN_OFFSET(aId),KSetNone);
		
			AsspRegister::Modify32(baseAddr+KGPIO_FALLINGDETECT, KClearNone, GPIO_PIN_OFFSET(aId));
			NKern::RestoreInterrupts(irqFlags);//__SPIN_UNLOCK_IRQRESTORE_W(GPIOLevelSpinLock,irqFlags);
			break;
		case EEdgeRising:
			
			irqFlags = NKern::DisableAllInterrupts();//__SPIN_LOCK_IRQSAVE_W(GPIOLevelSpinLock);
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT0, GPIO_PIN_OFFSET(aId),KSetNone);
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT1, GPIO_PIN_OFFSET(aId),KSetNone);
			AsspRegister::Modify32(baseAddr+KGPIO_FALLINGDETECT, GPIO_PIN_OFFSET(aId),KSetNone);
			
			AsspRegister::Modify32(baseAddr+KGPIO_RISINGDETECT, KClearNone, GPIO_PIN_OFFSET(aId));
			NKern::RestoreInterrupts(irqFlags);//__SPIN_UNLOCK_IRQRESTORE_W(GPIOLevelSpinLock,irqFlags);			
			break;
		case EEdgeBoth:
				
			irqFlags = NKern::DisableAllInterrupts();//__SPIN_LOCK_IRQSAVE_W(GPIOLevelSpinLock);
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT0, GPIO_PIN_OFFSET(aId),KSetNone);
			AsspRegister::Modify32(baseAddr+KGPIO_LEVELDETECT1, GPIO_PIN_OFFSET(aId),KSetNone);
			
			AsspRegister::Modify32(baseAddr+KGPIO_FALLINGDETECT, KClearNone, GPIO_PIN_OFFSET(aId));
			AsspRegister::Modify32(baseAddr+KGPIO_RISINGDETECT, KClearNone, GPIO_PIN_OFFSET(aId));
			NKern::RestoreInterrupts(irqFlags);//__SPIN_UNLOCK_IRQRESTORE_W(GPIOLevelSpinLock,irqFlags);
			
		break;
		default:
			return KErrArgument;
		}

	return KErrNone;
	}


EXPORT_C TInt GPIO::EnableWakeup(TInt aId)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::EnableWakeup OOB ",KErrArgument));
	
	TInt baseAddr = GPIO_BASE_ADDRESS(aId);
	
	TInt irq = __SPIN_LOCK_IRQSAVE(GPIOWakeLock);
	AsspRegister::Modify32(baseAddr+KGPIO_SYSCONFIG,KClearNone, 1 << 2 );
	AsspRegister::Modify32(baseAddr+KGPIO_SETWKUENA,KClearNone ,GPIO_PIN_OFFSET(aId));
	__SPIN_UNLOCK_IRQRESTORE(GPIOWakeLock,irq);
	
	return KErrNone;
	}

EXPORT_C TInt GPIO::DisableWakeup(TInt aId)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::DisableWakeup OOB ",KErrArgument));
	
	TInt baseAddr = GPIO_BASE_ADDRESS(aId);
	
	TInt irq = __SPIN_LOCK_IRQSAVE(GPIOWakeLock);
	AsspRegister::Modify32(baseAddr+KGPIO_SYSCONFIG,1 << 2 ,KSetNone);
	AsspRegister::Modify32(baseAddr+KGPIO_CLEARWKUENA,KClearNone,GPIO_PIN_OFFSET(aId));
	__SPIN_UNLOCK_IRQRESTORE(GPIOWakeLock,irq);
	return KErrNone;
	}

EXPORT_C TInt GPIO::IsWakeupEnabled(TInt aId, TBool& aEnable)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::IsWakeupEnabled OOB ",KErrArgument));
	
	aEnable = AsspRegister::Read32(GPIO_BASE_ADDRESS(aId)+KGPIO_WAKEUPENABLE) & GPIO_PIN_OFFSET(aId);
	return KErrNone;
	}

EXPORT_C TInt GPIO::SetWakeupTrigger(TInt aId, TGpioDetectionTrigger aTrigger)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::SetWakeupTrigger OOB ",KErrArgument));
	return KErrNotSupported;
	}

// WARNING: Changing debouncing time will change it for all pins in the bank
EXPORT_C TInt GPIO::SetDebounceTime(TInt aId, TInt aTime)
	{
	TGpioDirection direction;
	GetPinDirection(aId, direction);
	
	if(direction==EOutput)
		{
		// The pin must be configured as input.	
		return KErrNotSupported;
		}
	
	//convert the Ms input time into units of 31us 
	TInt timeSteps =  aTime / 31;
	if(timeSteps>127)
		{
#ifdef _DEBUG
		Kern::Printf("Warning: Tried to set the GPIO debounce time to %dus, \
						which is greater than the maximum supported 3937us.", aTime);
#endif
		timeSteps=127; // The maximum debounce value.
		}
	
	TInt baseAddr = GPIO_BASE_ADDRESS(aId);
	
	TUint irqFlags = __SPIN_LOCK_IRQSAVE(gpio::GPIODebounceSpinLock);
	AsspRegister::Write32(baseAddr+KGPIO_DEBOUNCINGTIME, timeSteps & KGPIO_DEBOUNCE_TIME_MASK);
	AsspRegister::Modify32(baseAddr+KGPIO_DEBOUNCENABLE, KClearNone, GPIO_PIN_OFFSET(aId));
	__SPIN_UNLOCK_IRQRESTORE(gpio::GPIODebounceSpinLock,irqFlags);

	return KErrNone;
	}

EXPORT_C TInt GPIO::GetDebounceTime(TInt aId, TInt& aTime)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::GetDebounceTime OOB ",KErrArgument));
	aTime=AsspRegister::Read32(GPIO_BASE_ADDRESS(aId)+KGPIO_DEBOUNCINGTIME); // The time in in multiples of 31 microseconds. We should probably use a nicer unit..
		
	return KErrNone;
	}

EXPORT_C TInt GPIO::SetOutputState(TInt aId, TGpioState aState)
	{
	if(aState==GPIO::ELow)
		{
		AsspRegister::Modify32(GPIO_BASE_ADDRESS(aId) + KGPIO_DATAOUT, GPIO_PIN_OFFSET(aId), KSetNone);
		}
	else
		{
		AsspRegister::Modify32(GPIO_BASE_ADDRESS(aId) + KGPIO_DATAOUT, KClearNone, GPIO_PIN_OFFSET(aId));
		}
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetInputState(TInt aId,	TGpioState& aState)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::GetInputState OOB ",KErrArgument));

	aState= ( AsspRegister::Read32(GPIO_BASE_ADDRESS(aId) + KGPIO_DATAIN) & GPIO_PIN_OFFSET(aId) ? 
			GPIO::EHigh:
			GPIO::ELow);

	return KErrNone;
	}

EXPORT_C TInt GPIO::GetOutputState(TInt aId, TGpioState& aState)
	{
	__ASSERT_ALWAYS(GPIO_PIN_BOUNDS(aId),Kern::Fault(" GPIO::GetOutputState OOB ",KErrArgument));
		
	aState = (AsspRegister::Read32(GPIO_BASE_ADDRESS(aId)+KGPIO_DATAOUT)& GPIO_PIN_OFFSET(aId) ?
				aState=GPIO::EHigh:
				aState=GPIO::ELow);
	return KErrNone;
	}

EXPORT_C TInt GPIO::GetInputState(TInt aId, TGpioCallback* /*aCb*/)
	{
	return KErrNotSupported;
	}

EXPORT_C TInt GPIO::SetOutputState(TInt aId, TGpioState	aState, TGpioCallback* /*aCb*/)
	{
	return KErrNotSupported;
	}


DECLARE_STANDARD_EXTENSION()
	{
	
	TInt i=0;
	for(;i<KHwGpioBanks;i++)
		{	
		//spins here
		AsspRegister::Write32(GPIO_BASE_ADDRESS(i*KHwGpioPinsPerBank)+KGPIO_SYSCONFIG,0x01 | 1 <<2);
		AsspRegister::Write32(GPIO_BASE_ADDRESS(i*KHwGpioPinsPerBank)+KGPIO_CTRL,0x00);
		ThePinIsEnabled[i]=0;
		}

	for(i=0; i<KHwGpioPinMax; i++)
		{
		//ThePinMode[ i ]=GPIO::EIdle;
		}
	return InitGpioInterrupts();
	}
