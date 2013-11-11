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
// omap3530/omap3530_drivers/gpio/gpio_interrupts.cpp
//

#include <e32cmn.h>
#include <nk_priv.h>
#include <assp/omap3530_assp/omap3530_gpio.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>
#include <assp/omap3530_assp/omap3530_ktrace.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/locks.h>

NONSHARABLE_CLASS( TGpioDispatcher ) : public MInterruptDispatcher
	{
	public:
		TGpioDispatcher();

		TInt Init();

		virtual TInt Bind(TInt aId, TIsr aIsr, TAny* aPtr);
		virtual TInt Unbind(TInt aId);
		virtual TInt Enable(TInt aId);
		virtual TInt Disable(TInt aId);
		virtual TInt Clear(TInt aId);
		virtual TInt SetPriority(TInt aId, TInt aPriority);

	private:
		static void Spurious( TAny* aParam );
		static void DispatchIsr( TAny* aParam );

		static TInt GetGPIOPin( TInt aId,GpioPin *aPin );
		static TInt SetGPIOPin(TInt aId,GpioPin *aPin);

		static FORCE_INLINE TBool IsValidId( TInt aId );
	};

TSpinLock GPIOpinsDescLock(/*TSpinLock::EOrderGenericIrqLow0*/);
static GpioPin	GpioPins[KHwGpioPinMax];


TGpioDispatcher::TGpioDispatcher()
	{
	for (TInt32 i = 0; i < KHwGpioPinMax; i++)
		{
		GpioPins[i].iMode = GPIO::EIdle;
		GpioPins[i].irq.iIsr = Spurious;
		GpioPins[i].irq.iPtr = &GpioPins[i];	
		GpioPins[i].iBankAddr = GPIO_BASE_ADDRESS(i);
		GpioPins[i].iBank = i / KHwGpioPinsPerBank;
		GpioPins[i].iIrqVector =  EOmap3530_IRQ29_GPIO1_MPU_IRQ +GpioPins[i].iBank;
		}
	}


TInt TGpioDispatcher::Init()
	{
	for(TInt i=0; i < KHwGpioBanks; i++)
		{		
		TInt r = Interrupt::Bind(EOmap3530_IRQ29_GPIO1_MPU_IRQ+i,DispatchIsr,(TAny*) i); 
		// Pass in a pointer to the first pin in the bank - shouldn't use addition on the constant here	
		__ASSERT_ALWAYS(r==KErrNone,Kern::Fault("ExternalInterrupt::%s Cant Bind to %d",EOmap3530_IRQ29_GPIO1_MPU_IRQ+i));	
		}

	Register( EIrqRangeBaseGpio );

	return KErrNone;
	}



TInt TGpioDispatcher::GetGPIOPin(TInt aId,GpioPin *aPin)
	{
	
	if(! ( aId >= 0 && aId <  KHwGpioPinMax)  )
			return KErrArgument;
	
	TInt irq = /*NKern::DisableAllInterrupts();*/__SPIN_LOCK_IRQSAVE_R(GPIOpinsDescLock);
	memcpy(aPin,&GpioPins[aId],sizeof(GpioPin));
	/*NKern::RestoreInterrupts(irq);*/__SPIN_UNLOCK_IRQRESTORE_R(GPIOpinsDescLock,irq);	
	
	return KErrNone;	
	}

TInt TGpioDispatcher::SetGPIOPin(TInt aId,GpioPin *aPin)
	{
		
	if(! ( aId >= 0 && aId <  KHwGpioPinMax)  )
			return KErrArgument;
	
	TInt irq = /*NKern::DisableAllInterrupts();*/__SPIN_LOCK_IRQSAVE_W(GPIOpinsDescLock);				
	memcpy(&GpioPins[aId],aPin,sizeof(GpioPin ));
	/*NKern::RestoreInterrupts(irq);*/__SPIN_UNLOCK_IRQRESTORE_W(GPIOpinsDescLock,irq);
	
	return KErrNone;
	}

FORCE_INLINE TBool TGpioDispatcher::IsValidId( TInt aId )
	{
	return ((TUint)aId < EGPIOIRQ_END && (TUint)aId>=EGPIOIRQ_FIRST);
	}


void TGpioDispatcher::Spurious(TAny* aId)
	{
	Kern::Fault("SpuriousExtInt",(TInt)aId);
	}


void TGpioDispatcher::DispatchIsr(TAny *aPtr)
	{
	Interrupt::Disable(EOmap3530_IRQ29_GPIO1_MPU_IRQ + (TInt) aPtr);
	
	//need to spinlock the gpio here.....
	TUint32	highVectors = AsspRegister::Read32(KGPIO_BASE_ADDRESSES[(TInt) aPtr] + KGPIO_IRQSTATUS1);
  	AsspRegister::Write32(KGPIO_BASE_ADDRESSES[(TInt) aPtr] + KGPIO_IRQSTATUS1, highVectors);
	
	GpioPin pin;
	for (TInt i =  0; i < KHwGpioPinsPerBank ; i++,highVectors >>=1)
		{
		if(highVectors & 0x1)
			{		
			GetGPIOPin(i+(TInt)aPtr*KHwGpioPinsPerBank, &pin);
			(*pin.irq.iIsr)(pin.irq.iPtr); // dispatch this pin's ISR
			}
		}
	Interrupt::Enable(EOmap3530_IRQ29_GPIO1_MPU_IRQ + (TInt)aPtr);
	}


TInt TGpioDispatcher::Bind(TInt anId, TIsr anIsr, TAny* aPtr)
	{
	if(IsValidId(anId))
		{
		//we want to bind the callers isrfunc to the pin dispatch here
		GpioPin pin;
		GetGPIOPin(anId- EGPIOIRQ_FIRST,&pin);
		pin.irq.iIsr = anIsr;
		pin.irq.iPtr = aPtr;
		SetGPIOPin(anId- EGPIOIRQ_FIRST,&pin);
		return KErrNone;
		} 
	else
		{
		return KErrArgument;
		}
	}

TInt TGpioDispatcher::Unbind(TInt anId)
	{
	__KTRACE_OPT(KGPIO,Kern::Printf("GPIO:%s id=%d",__FUNCTION__,anId));

	if(IsValidId(anId))
		{
		GpioPin pin;
		TInt pinNr = anId - EGPIOIRQ_FIRST;
		GetGPIOPin(pinNr,&pin);
		pin.irq.iIsr=Spurious;
		pin.irq.iPtr=NULL;
		SetGPIOPin(pinNr,&pin);
		return KErrNone;
		}
	else
		{
		return KErrArgument;
		}
	}
	

TInt TGpioDispatcher::Enable(TInt anId)
	{
	__KTRACE_OPT(KGPIO,Kern::Printf("GPIO:%s id=%d +",__FUNCTION__,anId));
	CHECK_PRECONDITIONS(MASK_NO_FAST_MUTEX,"GPIO::InterruptEnable Cant Hold Mutex in Blocking function");			
	
	if(IsValidId(anId))
		{
		GpioPin pin;
		TInt pinNr = anId - EGPIOIRQ_FIRST;	
		GetGPIOPin(pinNr,&pin);
		
		if(Spurious == pin.irq.iIsr)
			{
			
			__KTRACE_OPT(KGPIO,Kern::Printf("GPIO:%s id=%d NOT BOUND",__FUNCTION__,anId));
			return KErrNotReady;
			}
		AsspRegister::Write32(pin.iBankAddr+KGPIO_SETIRQENABLE1,GPIO_PIN_OFFSET( pinNr));
		
		if(!Omap3530Interrupt::IsInterruptEnabled(pin.iIrqVector))
			Interrupt::Enable(pin.iIrqVector);
		
		return KErrNone;
		}
	else
		{
		return KErrArgument; 
		}
	}

TInt TGpioDispatcher::Disable(TInt anId)
	{
	
	__KTRACE_OPT(KGPIO,Kern::Printf("GPIO:%s id=%d",__FUNCTION__,anId));
	CHECK_PRECONDITIONS(MASK_NO_FAST_MUTEX,"GPIO::InterruptDisable Cant Hold Mutex in Blocking function");
	if(IsValidId(anId))
		{		
		TInt pinNr = anId- EGPIOIRQ_FIRST;
		GpioPin pin;
		GetGPIOPin(pinNr, &pin);
		
		AsspRegister::Write32(pin.iBankAddr+KGPIO_CLEARIRQENABLE1, GPIO_PIN_OFFSET(pinNr));
		
		//is this the last one for this bank ? then unmap
		if(0x00 == AsspRegister::Read32(pin.iBankAddr+KGPIO_IRQENABLE1))
			{
			Interrupt::Disable(pin.iIrqVector);
			}
		return KErrNone;
		}
	else
		{
		return KErrArgument; 
		}
	}

TInt TGpioDispatcher::Clear(TInt anId)
	{
	
	__KTRACE_OPT(KGPIO,Kern::Printf("GPIO:%s id=%d",__FUNCTION__,anId));
	CHECK_PRECONDITIONS(MASK_NO_FAST_MUTEX,"GPIO::InterruptDisable Cant Hold Mutex in Blocking function");
		
	if(IsValidId(anId))
		{
		TInt pinNr = anId- EGPIOIRQ_FIRST;
		GpioPin myPin;
		GetGPIOPin(pinNr, &myPin);
		
		AsspRegister::Write32((myPin.iBankAddr+KGPIO_IRQSTATUS1),  GPIO_PIN_OFFSET(pinNr));
		//if that was the only high bit clear the mainline as well
		if(0 == AsspRegister::Read32(myPin.iBankAddr+KGPIO_IRQSTATUS1))
			{
			Interrupt::Clear(myPin.iIrqVector);
			}
		}
	return KErrNone;
	}

TInt TGpioDispatcher::SetPriority(TInt aId, TInt aPriority)
	{
	return KErrNotSupported;
	}


GLDEF_C TInt InitGpioInterrupts()
	{
	TInt r = KErrNoMemory;

	TGpioDispatcher* dispatcher = new TGpioDispatcher;
	if( dispatcher )
		{
		r = dispatcher->Init();
		}
	return r;
	}
