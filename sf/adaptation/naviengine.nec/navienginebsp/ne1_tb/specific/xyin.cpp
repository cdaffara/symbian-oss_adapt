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
* ne1_tb\Specific\xyin.cpp
* Implementation of a digitiser (touch-screen) driver.
* This code assumes that an interrupt is generated on pen-down and pen-up events.
* This file is part of the NE1_TBVariant Base port
* We use this driver to exemplify the usage of Resource Management for shared resources, Peripheral "Sleep"
* and detection and notification of Wakeup events
*
*/




#include <assp.h>
#include <naviengine.h>
#include <videodriver.h>
#include "mconf.h"
#include <xyin.h>
#include "lcdgce.h"

#ifdef USE_PEN_INTERRUPTS
#include <gpio.h>
const TUint KLcdInterruptPin = 22;
#else
const TUint KPenEventsPollTime = 30;
#endif

// digitiser origin & size in pixels
const TUint	KConfigXyOffsetX	= 0;		// digitiser origin - same as display area
const TUint	KConfigXyOffsetY	= 0;

// digitiser dimensions in digitiser co-ordinates
const TInt      KConfigXySpreadX = 3600; // maximum valid X spread
const TInt      KConfigXySpreadY = 3600; // maximum valid Y spread
const TInt		KConfigXyMinX    = 450;  // minimum valid X value
const TInt		KConfigXyMinY    = 450;  // minimum valid Y value
const TUint		KXYShift         = 4100;

const TInt KDigitiserThreadPriority = 26;
_LIT(KDigitiserDriverThreadName,"DigitizerThread");

// Define a 2x2 matrix and two constants Tx and Ty to convert digitiser co-ordinates
// to pixels such that
//
// (X<<16 Y<<16)	=	(x y)	x	(R11 R12)	+	(Tx Ty)
//									(R21 R22)
// or :
//
// X = (x*R11 + y*R21 + TX) >> 16;
// Y = (x*R12 + y*R22 + TY) >> 16;

//
// where x,y are digitiser coordinates, Tx,Ty are constant offsets and X,Y are screen
// coordinates. Left shifting by 16 bits is used so as not to lose precision.

// After taking a sample, wait for the specified number of nano-kernel ticks (normally 1 ms)
// before taking the next sample
const TInt		KInterSampleTime	= 1;

// After a group of samples has been processed by the DDigitiser::ProcessRawSample() DFC,
// wait for the specified number of nano-kernel ticks before taking the next sample
const TInt		KInterGroupTime		= 1;

// After a pen-down interrupt,
// wait for the specified number of nano-kernel ticks before taking the next sample
const TInt		KPenDownDelayTime	= 2;

// If powering up the device with the pen down,
// wait for the specified number of nano-kernel ticks before taking the next sample
const TInt		KPenUpPollTime		= 30;

// After a pen-up interrupt,
// wait for the specified number of nano-kernel ticks before calling PenUp()
const TInt		KPenUpDebounceTime	= 10;

// number of samples to discard on pen-down
const TInt		KConfigXyPenDownDiscard	= 1;

// number of samples to discard on pen-up
const TInt		KConfigXyPenUpDiscard	= 1;

// offset in pixels to cause movement in X direction
const TInt		KConfigXyAccThresholdX	= 12;

// offset in pixels to cause movement in Y direction
const TInt		KConfigXyAccThresholdY	= 12;

// number of samples to average - MUST be <= KMaxXYSamples
const TInt		KConfigXyNumXYSamples	= 4;

// disregard extremal values in each 4-sample group
const TBool		KConfigXyDisregardMinMax= ETrue;  //EFalse;

// Registers..
const TUint KHwPenInterruptRegister = KHwFPGABase + 0x0408; // LCD penInterrupt register
const TUint KHwTSPStart             = KHwFPGABase + 0x0608; // TSP Control Register..
const TUint KHwTSPXData             = KHwFPGABase + 0x0610; // TSP X Data Register..
const TUint KHwTSPYData             = KHwFPGABase + 0x0618; // TSP Y Data Register..

// obsolete constants :
const TInt KConfigXyDriveXRise = 0;
const TInt KConfigXyDriveYRise = 0;
const TInt KConfigXyMaxJumpX   = 0;
const TInt KConfigXyMaxJumpY   = 0;


struct SConfig
	{
	TUint iConfigXyWidth;
	TUint iConfigXyHeight;
	TBool iLandscape;

	// Calibration Values
	TInt iConfigXyR11;
	TInt iConfigXyR12;
	TInt iConfigXyR21;
	TInt iConfigXyR22;
	TInt iConfigXyTx;
	TInt iConfigXyTy;
	};

enum TTouchScreenMode
	{
	TOUCHSCREEN_MODE_NEC_WVGA     = 0,
	TOUCHSCREEN_MODE_HITACHI_VGA,
	TOUCHSCREEN_MODE_HITACHI_QVGA,

	TOUCHSCREEN_MODE_NONE = 255,
	};

static const SConfig Mode_Config[]=
	{
	// NEC WVGA - default mode
		{
		800,
		480,
		ETrue,

		// Calibration values; these are set manually using the TechView calibration app
		17722,      // R11
		1239,       // R12
		399,        // R21
		12352,      // R22
		-11623449,  // Tx
		-10468471,  // Ty
		},


	// LCD, Portrait VGA
		{
		480,
		640,
		EFalse,

		// Calibration values; these are set manually using the TechView calibration app
		11346,      // R11
		538,        // R12
		-682,       // R21
		14703,      // R22
		-5683258,   // Tx
		-9913475,   // Ty
		},

	// LCD, Portrait QVGA
		{
		240,
		320,
		EFalse,

		// Calibration values; these are set manually using the TechView calibration app
		5873,		//iConfigXyR11;
		651,		//iConfigXyR12;
		-250,		//iConfigXyR21;
		7366,		//iConfigXyR22;
		-3591097,	//iConfigXyTx;
		-5181791,	//iConfigXyTy;
		},
	};


/******************************************************
 * Main Digitiser Class
 ******************************************************/
NONSHARABLE_CLASS(DNE1_TBDigitiser) : public DDigitiser
	{
public:
	enum TState
		{
		E_HW_PowerUp,
		E_HW_PenUpDebounce,
		E_HW_CollectSample
		};

public:
	// from DDigitiser - initialisation
	DNE1_TBDigitiser();
	virtual TInt DoCreate();
	void SetDefaultConfig();

	// from DDigitiser - signals to hardware-dependent code
	virtual void WaitForPenDown();
	virtual void WaitForPenUp();
	virtual void WaitForPenUpDebounce();
	virtual void DigitiserOn();
	virtual void DigitiserOff();
	virtual void FilterPenMove(const TPoint& aPoint);
	virtual void ResetPenMoveFilter();

	// from DDigitiser - machine-configuration related things
	virtual TInt DigitiserToScreen(const TPoint& aDigitiserPoint, TPoint& aScreenPoint);
	virtual void ScreenToDigitiser(TInt& aX, TInt& aY);
	virtual TInt SetXYInputCalibration(const TDigitizerCalibration& aCalibration);
	virtual TInt CalibrationPoints(TDigitizerCalibration& aCalibration);
	virtual TInt SaveXYInputCalibration();
	virtual TInt RestoreXYInputCalibration(TDigitizerCalibrationType aType);
	virtual void DigitiserInfo(TDigitiserInfoV01& aInfo);

	// from DPowerHandler
	virtual void PowerDown(TPowerState);
	virtual void PowerUp();

	// implementation
	void TakeSample();
	void PenInterrupt();
	void DigitiserPowerUp();
	void PowerUpDfc();

	// callbacks
	static void TimerExpired(TAny* aPtr);
	static void TimerIntExpired(TAny* aPtr);
	static void PenIsr(TAny* aPtr);
	static void TakeReading(TAny* aPtr);
	static void PowerDown(TAny* aPtr);
	static void PowerUp(TAny* aPtr);

private:
	NTimer iTimer;
	NTimer iTimerInt;
	TDfc   iTakeReadingDfc;
	TDfc   iPowerDownDfc;
	TDfc   iPowerUpDfc;
	TInt   iSamplesCount;
	TState iState;
	TUint8 iPoweringDown;
	TSize  iScreenSize;
#ifndef USE_PEN_INTERRUPTS
	NTimer iPollingTimer;
#endif
	TActualMachineConfig& iMachineConfig;
	TTouchScreenMode      iCurrentMode;
	TInt                  iInterruptId;
	};

/******************************************************
 * Digitiser main code
 ******************************************************/
/**
Sample timer callback
Schedules a DFC to take a sample

@param aPtr	a pointer to DNE1_TBDigitiser
*/
void DNE1_TBDigitiser::TimerExpired(TAny* aPtr)
	{
	DNE1_TBDigitiser* pD=(DNE1_TBDigitiser*)aPtr;
	__KTRACE_OPT(KHARDWARE, Kern::Printf("TimerExpired"));
	pD->iTakeReadingDfc.Add();
	}

/**
Debounce timer callback
schedules a DFC to process a pen-down interrupt

@param aPtr	a pointer to DNE1_TBDigitiser
*/
void DNE1_TBDigitiser::TimerIntExpired(TAny* aPtr)
	{
	DNE1_TBDigitiser* pD=(DNE1_TBDigitiser*)aPtr;
	__KTRACE_OPT(KHARDWARE, Kern::Printf("TIntExpired"));
	pD->iTakeReadingDfc.Add();
	}

/**
Pen-up/down interrupt handler

@param aPtr	a pointer to DNE1_TBDigitiser
*/
void DNE1_TBDigitiser::PenIsr(TAny* aPtr)
	{
	DNE1_TBDigitiser* pD=(DNE1_TBDigitiser*)aPtr;

	//check the status of the interrupt:
	TUint16 status = AsspRegister::Read16(KHwPenInterruptRegister);

	if (!(status & 0x100)) // Pen is down..
		{
#ifdef USE_PEN_INTERRUPTS
		GPIO::DisableInterrupt(pD->iInterruptId);
#endif
		pD->PenInterrupt();
		}

#ifndef USE_PEN_INTERRUPTS
		// kick-off the timer again..
		pD->iPollingTimer.Again(KPenEventsPollTime);
#endif

	}

/**
DFC for taking a sample

@param aPtr	a pointer to DNE1_TBDigitiser
*/
void DNE1_TBDigitiser::TakeReading(TAny* aPtr)
	{
	DNE1_TBDigitiser* pD=(DNE1_TBDigitiser*)aPtr;
	pD->TakeSample();
	}

/**
DFC for powering down the device

@param aPtr	a pointer to DNE1_TBDigitiser
*/
void DNE1_TBDigitiser::PowerDown(TAny* aPtr)
	{
	DNE1_TBDigitiser* pD=(DNE1_TBDigitiser*)aPtr;
	pD->DigitiserOff();
	}

/**
DFC for powering up the device

@param aPtr	a pointer to DNE1_TBDigitiser
*/
void DNE1_TBDigitiser::PowerUp(TAny* aPtr)
	{
	DNE1_TBDigitiser* pD=(DNE1_TBDigitiser*)aPtr;
	pD->PowerUpDfc();
	}


/**
Creates a new instance of DDigitiser.
Called by extension entry point (PIL) to create a DDigitiser-derived object.

@return	a pointer to a DNE1_TBDigitiser object
*/
DDigitiser* DDigitiser::New()
	{
	return new DNE1_TBDigitiser;
	}

/**
Default constructor
*/
DNE1_TBDigitiser::DNE1_TBDigitiser() :
		iTimer(TimerExpired,this),
		iTimerInt(TimerIntExpired,this),
		iTakeReadingDfc(TakeReading,this,5),
		iPowerDownDfc(PowerDown,this,5),
		iPowerUpDfc(PowerUp,this,5),
#ifndef USE_PEN_INTERRUPTS
		iPollingTimer(PenIsr, this),
#endif
		iMachineConfig(TheActualMachineConfig())
	{
	}

/**
Perform hardware-dependent initialisation

Called by platform independent layer
*/
TInt DNE1_TBDigitiser::DoCreate()
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("DNE1_TBDigitiser::DoCreate"));
	if (Kern::ColdStart())
		{
		__KTRACE_OPT(KEXTENSION,Kern::Printf("Resetting digitiser calibration"));

		// Emergency digitiser calibration values
		TInt mode = ReadDipSwitchDisplayMode();
		switch (mode)
			{
			case DISPLAY_MODE_HITACHI_VGA: // Hitachi display in VGA
				iCurrentMode = TOUCHSCREEN_MODE_HITACHI_VGA;
				break;
			case DISPLAY_MODE_HITACHI_QVGA: // Hitachi display in QVGA
				iCurrentMode = TOUCHSCREEN_MODE_HITACHI_QVGA;
				break;
			case DISPLAY_MODE_NEC_WVGA:	// NEC display in WVGA
				iCurrentMode = TOUCHSCREEN_MODE_NEC_WVGA;
				break;

			// In all ANALOG modes - don't use the digitiser
			case DISPLAY_MODE_ANALOG_VGA:
			case DISPLAY_MODE_ANALOG_QVGA_LANDSCAPE_OLD:
			case DISPLAY_MODE_ANALOG_QVGA_PORTRAIT:
			case DISPLAY_MODE_ANALOG_QVGA_LANDSCAPE:
				iCurrentMode = TOUCHSCREEN_MODE_NONE;
				break;

			default:    // In all other modes, use the NEC WVGA settings
				iCurrentMode = TOUCHSCREEN_MODE_NEC_WVGA;
				break;

			}
		iMachineConfig.iCalibration.iR11 = Mode_Config[iCurrentMode].iConfigXyR11;
		iMachineConfig.iCalibration.iR12 = Mode_Config[iCurrentMode].iConfigXyR12;
		iMachineConfig.iCalibration.iR21 = Mode_Config[iCurrentMode].iConfigXyR21;
		iMachineConfig.iCalibration.iR22 = Mode_Config[iCurrentMode].iConfigXyR22;
		iMachineConfig.iCalibration.iTx  = Mode_Config[iCurrentMode].iConfigXyTx;
		iMachineConfig.iCalibration.iTy  = Mode_Config[iCurrentMode].iConfigXyTy;
		}

	TDynamicDfcQue* dfcq;
	// Create a dedicated DFC queue for this driver
	TInt r = Kern::DynamicDfcQCreate(dfcq, KDigitiserThreadPriority, KDigitiserDriverThreadName);
	if (r != KErrNone)
		{
		__KTRACE_OPT(KEXTENSION,Kern::Printf("Could not create a DFCQue, r %d", r));
		return r;
		}

#ifdef CPU_AFFINITY_ANY
	NKern::ThreadSetCpuAffinity((NThread*)(iDfcQ->iThread), KCpuAffinityAny);
#endif

	// and DFCs to use it..
	iDfcQ = dfcq;
	iTakeReadingDfc.SetDfcQ(iDfcQ);
	iPowerDownDfc.SetDfcQ(iDfcQ);
	iPowerUpDfc.SetDfcQ(iDfcQ);


	// stop at this point, if the LCD panel is not present
	if (iCurrentMode == TOUCHSCREEN_MODE_NONE)
		{
		__KTRACE_OPT(KEXTENSION,Kern::Printf("xyin.cpp: LCD panel not switched on...\nwill not start the driver"));
		return KErrNone;
		}

	// register power handler
	Add();
	DigitiserPowerUp();

#ifdef USE_PEN_INTERRUPTS
   // set up interrupts
	iInterruptId = KLcdInterruptPin;
	r = GPIO::BindInterrupt(iInterruptId, PenIsr, this);
	if (r != KErrNone)
		{
		__KTRACE_OPT(KEXTENSION,Kern::Printf("Error binding the interrupt: r %d", r));
		return r;
		}
#endif

	// set up the default configuration
	SetDefaultConfig();

	return KErrNone;
	}

/**
Initialise the DDigitiser::iCfg structure
*/
void DNE1_TBDigitiser::SetDefaultConfig()
	{
	iCfg.iPenDownDiscard  = KConfigXyPenDownDiscard;  // number of samples to discard on pen-down
	iCfg.iPenUpDiscard    = KConfigXyPenUpDiscard;    // number of samples to discard on pen-up
	iCfg.iDriveXRise      = KConfigXyDriveXRise;      // number of milliseconds to wait when driving horizontal edges
	iCfg.iDriveYRise      = KConfigXyDriveYRise;      // number of milliseconds to wait when driving vertical edges
	iCfg.iMinX            = KConfigXyMinX;            // minimum valid X value
	iCfg.iMaxX            = KConfigXySpreadX - 1;     // maximum valid X value
	iCfg.iSpreadX         = KConfigXySpreadX;         // maximum valid X spread
	iCfg.iMinY            = KConfigXyMinY;            // minimum valid Y value
	iCfg.iMaxY            = KConfigXySpreadY - 1;     // maximum valid Y value
	iCfg.iSpreadY         = KConfigXySpreadY;         // maximum valid Y spread
	iCfg.iMaxJumpX        = KConfigXyMaxJumpX;        // maximum X movement per sample (pixels)
	iCfg.iMaxJumpY        = KConfigXyMaxJumpY;        // maximum Y movement per sample (pixels)
	iCfg.iAccThresholdX   = KConfigXyAccThresholdX;   // offset in pixels to cause movement in X direction
	iCfg.iAccThresholdY   = KConfigXyAccThresholdY;   // offset in pixels to cause movement in Y direction
	iCfg.iNumXYSamples    = KConfigXyNumXYSamples;    // number of samples to average
	iCfg.iDisregardMinMax = KConfigXyDisregardMinMax; // disregard extremal values in each 4-sample group
	}

/**
Takes a sample from the digitiser.
Called in the context of a DFC thread.
*/
void DNE1_TBDigitiser::TakeSample()
	{
//	TNE1_TBPowerController::WakeupEvent();	// notify of pendown (wakeup event) and let the power controller sort
												// out if it needs propagation
	TBool penDown = EFalse;

    //check the touch panel interrupt state
    TUint16 status = AsspRegister::Read16(KHwPenInterruptRegister);
    if(!(status & 0x100))  //Panel touched
        penDown = ETrue;

	__KTRACE_OPT(KHARDWARE,Kern::Printf("TS: S%d PD%d Sp%d", (TInt)iState, penDown?1:0, iSamplesCount));

	if (iState==E_HW_PowerUp)
		{
		// waiting for pen to go up after switch on due to pen down or through the HAL
		if (!penDown)		// pen has gone up -> transition to new state
			{
			iState=E_HW_CollectSample;
			iSamplesCount=0;     // reset sample buffer
#ifdef USE_PEN_INTERRUPTS
			// clear and enable pen-down interrupt
			GPIO::EnableInterrupt(iInterruptId);
#endif
			}
		else	// pen is still down, wait a bit longer in this state
			{
			iTimer.OneShot(KPenUpPollTime);
			}
		return;
		}

	if (!penDown)
		{
		if (iState==E_HW_PenUpDebounce)
			{
			iState=E_HW_CollectSample;	// back to initial state, no samples collected
			iSamplesCount=0;			// reset sample buffer
			iPenUpDfc.Enque();
			}
		else							// iState=E_HW_CollectSample
			{
			iState=E_HW_PenUpDebounce;
			iTimer.OneShot(KPenUpDebounceTime);		// wait a bit to make sure pen still up
			}
		return;
		}
	else if (iState==E_HW_PenUpDebounce)	// pen down
		{
		// false alarm - pen is down again
		iState=E_HW_CollectSample;		// take a new set of samples
		iSamplesCount=0;				// reset sample buffer
		}
	// default: pen down and iState=E_HW_CollectSample
	// Read from appropriate hardware register to get the current digitiser coordinates
	// of the point that is being touched

    //start A/D conversion
    AsspRegister::Write16(KHwTSPStart, 1);

    // and wait for its completion(0x1 bit should be cleared)
    TUint timeout=100;
    while((AsspRegister::Read16(KHwPenInterruptRegister)&0x1) && --timeout);

    // Read values (Reading will clear interrupt status)
    TInt16 X = AsspRegister::Read16(KHwTSPXData);
    TInt16 Y = AsspRegister::Read16(KHwTSPYData);

	if(Mode_Config[iCurrentMode].iLandscape)
		{
		iX[iSamplesCount] = KXYShift - X;
		iY[iSamplesCount] = Y;
		}
	else
		{
		iX[iSamplesCount] =  KXYShift - X;
		iY[iSamplesCount] =  KXYShift - Y;
		}

	__KTRACE_OPT(KHARDWARE,Kern::Printf("Raw: X=%d Y=%d",iX[iSamplesCount],iY[iSamplesCount]));

	// count samples collected - if it's less than minimum,
	// schedule the reading of another sample
	if (++iSamplesCount < iCfg.iNumXYSamples)	// iX[] and iY[] are 4 levels deep in xyin.h...
		{
		if(KInterSampleTime > 0)
			{
			iTimer.OneShot(KInterSampleTime);	// haven't got a complete group yet, so queue timer to sample again
			}
		else
		    {
		    iTakeReadingDfc.Enque();
		    }
		return;
		}

	// Have a complete group of samples so pass up to processing layer (PIL)
	iSampleDfc.Enque(); // adds DFC

	}

/**
Request for an interrupt to be generated when the pen is next down
Called by PIL at startup or when pen leaves digitiser after pen-up event issued
*/
void DNE1_TBDigitiser::WaitForPenDown()
	{
	// Called at startup or when pen leaves digitiser after pen-up event issued
	__KTRACE_OPT(KHARDWARE,Kern::Printf("WD: PowerDownMask %x",iPoweringDown));
	if (iPoweringDown)
		{
		// powering down

		// Enable touch panel interrupt to allow the hardware
		// to detect when the digitiser panel is touched and wakes up the system if in standby
	    AsspRegister::Write16(KHwPenInterruptRegister, 0);

		// Relinquish request on power resources
        //...

		iPoweringDown = EFalse;
		PowerDownDone();
		}
	else
		{
		if (!iTimer.IsPending() &&
		    !iTimerInt.IsPending() &&
		    iCurrentMode != TOUCHSCREEN_MODE_NONE)
		    {
#ifndef USE_PEN_INTERRUPTS
		    iPollingTimer.OneShot(KPenEventsPollTime,ETrue);
#else
		    // enable pen-down interrupt
		    GPIO::EnableInterrupt(iInterruptId);
#endif
		    }
		}
	}

/**
Called by PIL after it has processed a group of raw samples while pen is down.
Used to indicate that the iX, iY buffers may be re-used
*/
void DNE1_TBDigitiser::WaitForPenUp()
	{
	__KTRACE_OPT(KHARDWARE,Kern::Printf("WU"));
	iState = E_HW_CollectSample;
	iSamplesCount = 0;					// reset sample buffer
	if(KInterGroupTime > 0)				// need to check this config param as it might be zero!
		iTimer.OneShot(KInterGroupTime);
	else
		iTakeReadingDfc.Enque();

	}

/**
Called by PIL if the group of samples collected is not good enough
Used to indicate that the iX, iY buffers may be re-used
*/
void DNE1_TBDigitiser::WaitForPenUpDebounce()
	{
	__KTRACE_OPT(KHARDWARE,Kern::Printf("WUDB"));
	iState = E_HW_CollectSample;
	iSamplesCount = 0;			// reset sample buffer
	if(KInterGroupTime > 0)					// need to check this config param as it might be zero!
		iTimer.OneShot(KInterGroupTime);
	else
		iTakeReadingDfc.Enque();
	}

/**
Pen up/down interrupt service routine (ISR)
*/
void DNE1_TBDigitiser::PenInterrupt()
    {
	__KTRACE_OPT(KHARDWARE, Kern::Printf("PenInterrupt"));

	if (KPenDownDelayTime>0)					// need to check this config param as it might be zero!
		iTimerInt.OneShot(KPenDownDelayTime);	// start a debounce timer which will queue a DFC to process the interrupt
	else
		{
        iTakeReadingDfc.Add();
		}
	}

/**
DPowerHandler pure virtual
*/
void DNE1_TBDigitiser::PowerUp()
	{
	iPowerUpDfc.Enque();			// queue a DFC in this driver's context
	}

/**
Called by power up DFC
*/
void DNE1_TBDigitiser::PowerUpDfc()
	{
	__KTRACE_OPT(KPOWER, Kern::Printf("DNE1_TBDigitiser::PowerUpDfc()"));
	DigitiserOn();
	PowerUpDone();			// must be called from a different thread than PowerUp()
	}

/**
Turn the digitiser on
May be called as a result of a power transition or from the HAL
If called from HAL, then the digitiser may be already be on (iPointerOn == ETrue)
*/
void DNE1_TBDigitiser::DigitiserOn()
	{
	__KTRACE_OPT(KPOWER,Kern::Printf("DNE1_TBDigitiser::DigitiserOn() iPointerOn=%d", iPointerOn));

	if (!iPointerOn)				// may have been powered up already
		DigitiserPowerUp();
	}

/**
Power-up the digitiser. Assumes digitiser is off.
*/
void DNE1_TBDigitiser::DigitiserPowerUp()
	{
	__KTRACE_OPT(KPOWER, Kern::Printf("DigitiserPowerUp"));
	iPointerOn = ETrue;		// now turned on

	// Re-assert request on power resources
	// This will move the peripheral hardware out of low power "Sleep" mode back to fully operational
    // ...

    // Select Clock by writing clk_div
    // (TSP_CLK = 30MHz/((clk_div+1)*2) (should be >=2)
    AsspRegister::Write16(KHwTSPBase, 2);

    // ensure, that touch-panel interrupt is enabled
    AsspRegister::Write16(KHwPenInterruptRegister, 0);

    iState = E_HW_PowerUp;	// so we wait for pen up if necessary
	iTakeReadingDfc.Enque();
	}

/**
DPowerHandler pure virtual

@param aPowerState the current power state
*/
void DNE1_TBDigitiser::PowerDown(TPowerState /*aPowerState*/)
	{
	iPoweringDown = ETrue;
	iPowerDownDfc.Enque();						// queue a DFC in this driver's context
	}

/**
Turn the digitiser off
May be called as a result of a power transition or from the HAL
If called from Power Manager, then the digitiser may be already be off (iPointerOn == EFalse)
if the platform is in silent running mode
*/
void DNE1_TBDigitiser::DigitiserOff()
	{
	__KTRACE_OPT(KPOWER,Kern::Printf("DNE1_TBDigitiser::DigitiserOff() iPointerOn=%d", iPointerOn));
	if (iPointerOn)		// can have been powered down from HAL
		{
		iPointerOn = EFalse;
#ifdef USE_PEN_INTERRUPTS
		GPIO::DisableInterrupt(iInterruptId);
#else
		iPollingTimer.Cancel();
#endif
		// disable the digitiser interrupt
		AsspRegister::Write16(KHwPenInterruptRegister, 1);

		iTimer.Cancel();
		iTimerInt.Cancel();
		iTakeReadingDfc.Cancel();
		if (iState != E_HW_CollectSample)
			{
#ifdef USE_PEN_INTERRUPTS
			iPenUpDfc.Add();
#else
			iPenUpDfc.Enque();
#endif
			}
		else
			{
			//
			// TO DO: (optional)
			//
			// Relinquish request on power resources as we are being powered down
			// This will place the peripheral hardware in a low power "Sleep" mode which is Interrupt detection capable
			// EXAMPLE ONLY
			//
            // ...

			if (iPoweringDown)			// came here through PowerDown
				{
				iPoweringDown = EFalse;
				PowerDownDone();
				}
			}
		}
	else	// already powered down (by HAL)
		{
			if (iPoweringDown)			// came here through PowerDown
				{
				iPoweringDown = EFalse;
				PowerDownDone();
				}
		}
	}


/**
Convert digitiser coordinates to screen coordinates

@param aDigitiserPoint the digitiser coordinates
@param aScreenPoint A TPoint supplied by the caller.
					On return, set to the converted screen coordinates in pixels.

@return KErrNone if successful
*/
TInt DNE1_TBDigitiser::DigitiserToScreen(const TPoint& aDigitiserPoint, TPoint& aScreenPoint)
	{
	NKern::LockSystem();
	TInt R11 = iMachineConfig.iCalibration.iR11;
	TInt R12 = iMachineConfig.iCalibration.iR12;
	TInt R21 = iMachineConfig.iCalibration.iR21;
	TInt R22 = iMachineConfig.iCalibration.iR22;
	TInt TX  = iMachineConfig.iCalibration.iTx;
	TInt TY  = iMachineConfig.iCalibration.iTy;
	NKern::UnlockSystem();
	TInt X = aDigitiserPoint.iX;
	TInt Y = aDigitiserPoint.iY;

	aScreenPoint.iX = (X*R11 + Y*R21 + TX) >> 16;
	aScreenPoint.iY = (X*R12 + Y*R22 + TY) >> 16;

	__KTRACE_OPT(KHARDWARE,	Kern::Printf("DtS: Dp.x %d, Dp.y %d, Sp.x %d, Sp.y %d", X,Y,aScreenPoint.iX,aScreenPoint.iY));

	return KErrNone;
	}

/**
Convert screen coordinates back into digitiser coordinates
using the current constants from the superpage

@param aX The screen X coordinate in pixels; On return, set to the digitiser X coordinate.
@param aY The screen Y coordinate in pixels; On return, set to the digitiser Y coordinate.
*/
void DNE1_TBDigitiser::ScreenToDigitiser(TInt& aX, TInt& aY)
	{

	NKern::LockSystem();
	Int64 R11 = iMachineConfig.iCalibration.iR11;
	Int64 R12 = iMachineConfig.iCalibration.iR12;
	Int64 R21 = iMachineConfig.iCalibration.iR21;
	Int64 R22 = iMachineConfig.iCalibration.iR22;
	Int64 TX  = iMachineConfig.iCalibration.iTx;
	Int64 TY  = iMachineConfig.iCalibration.iTy;
	NKern::UnlockSystem();
	Int64 X = aX;
	Int64 Y = aY;
	//
	// Xd=(Xs<<16)*R22-(Ys<<16)*R21-(TX*R22)+(TY*R21)
	//	  -------------------------------------------
	//				   (R22*R11)-(R21*R12)
	//
	//
	// Yd=(Xs<<16)*R12-(Ys<<16)*R11-(TX*R12)+(TY*R11)
	//	  -------------------------------------------
	//				   (R21*R12)-(R22*R11)
	//
	// where Xd and Yd are digitiser coordinates
	//		 Xs and Ys are supplied screen coordinates
	//
	X <<= 16;
	Y <<= 16;

	Int64 d = Int64(R21) * Int64(R12) - Int64(R22) * Int64(R11);

	Int64 r = (X*R12) - (Y*R11) - (TX*R12) + (TY*R11);

	if (d != 0)
		{
		r = r/d;
		}
	else
		{
		r = 0;
		}

	aY = (TInt)r;

	r = (X*R22)-(Y*R21)-(TX*R22)+(TY*R21);

	if (d != 0)
		{
		r = r/(-d);
		}
	else
		{
		r = 0;
		}

	aX=(TInt)r;
	}

/**
Calculate values for R11, R12, R21, R22, TX and TY

@param aCalibration the screen coordinates of points touched
@return KErrNone if successful
*/
TInt DNE1_TBDigitiser::SetXYInputCalibration(const TDigitizerCalibration& aCalibration)
	{
	TInt R11,R12,R21,R22,TX,TY;
	//
	// Get coords of expected points
	//
	TDigitizerCalibration cal;
	TInt ret=CalibrationPoints(cal);
	if (ret != KErrNone)
		return ret;

	TInt Xp1 = cal.iTl.iX;
	TInt Yp1 = cal.iTl.iY;
	TInt Xp2 = cal.iBl.iX;
	TInt Yp2 = cal.iBl.iY;
	TInt Xp3 = cal.iBr.iX;
	TInt Yp3 = cal.iBr.iY;

	//
	// Get coords of points touched in screen coordinates
	//
	TInt X1 = aCalibration.iTl.iX;
	TInt Y1 = aCalibration.iTl.iY;
	TInt X2 = aCalibration.iBl.iX;
	TInt Y2 = aCalibration.iBl.iY;
	TInt X3 = aCalibration.iBr.iX;
	TInt Y3 = aCalibration.iBr.iY;


	//
	// Convert back to raw digitiser coordinates
	//
	ScreenToDigitiser(X1,Y1);
	ScreenToDigitiser(X2,Y2);
	ScreenToDigitiser(X3,Y3);
	//
	// (Y1-Y2)(Xp1-Xp3) - (Y1-Y3)(Xp1-Xp2)
	// ----------------------------------- = R11
	// (Y1-Y2)(X1-X3)	- (Y1-Y3)(X1-X2)

	Int64 temp;      // temporary variable to prevent divide by zero faults
	Int64 r = ((Int64(Y1-Y2)*Int64(Xp1-Xp3))-(Int64(Y1-Y3)*Int64(Xp1-Xp2)));
	r <<= 16;
	temp = (Int64(Y1-Y2) * Int64(X1-X3) - Int64(Y1-Y3) * Int64(X1-X2));
	if (temp == 0)
		{
		r = 0;
		}
	else
		{
		r /= temp;
		}
	R11 = (TInt)r;
	//
	// (Y1-Y2)(Yp1-Yp3) - (Y1-Y3)(Yp1-Yp2)
	// ----------------------------------- = R12
	// (Y1-Y2)(X1-X3)	- (Y1-Y3)(X1-X2)
	//
	r = ((Int64(Y1-Y2) * Int64(Yp1-Yp3)) - (Int64(Y1-Y3) * Int64(Yp1-Yp2)));
	r <<= 16;
	temp = (Int64(Y1-Y2) * Int64(X1-X3) - Int64(Y1-Y3) * Int64(X1-X2));
	if (temp == 0)
		{
		r = 0;
		}
	else
		{
		r /= temp;
		}
	R12 = (TInt)r;
	//
	// (X1-X3)(Xp2-Xp3) - (X2-X3)(Xp1-Xp3)
	// ----------------------------------- = R21
	// (Y2-Y3)(X1-X3)	- (Y1-Y3)(X2-X3)
	//
	r = (((X1-X3) * (Xp2-Xp3)) - ((X2-X3) * (Xp1-Xp3)));
	r <<= 16;
	temp = (Int64(Y2-Y3) * Int64(X1-X3) - Int64(Y1-Y3) * Int64(X2-X3));
	if (temp == 0)
		{
		r = 0;
		}
	else
		{
		r /= temp;
		}
	R21=(TInt)r;
	//
	// (X1-X3)(Yp2-Yp3) - (X2-X3)(Yp1-Yp3)
	// ----------------------------------- = R22
	// (Y2-Y3)(X1-X3)	- (Y1-Y3)(X2-X3)
	//
	r = ((Int64(X1-X3) * Int64(Yp2-Yp3)) - (Int64(X2-X3) * Int64(Yp1-Yp3)));
	r <<= 16;
	temp = (Int64(Y2-Y3) * Int64(X1-X3) - Int64(Y1-Y3) * Int64(X2-X3));
	if (temp==0)
		{
		r = 0;
		}
	else
		{
		r/=temp;
		}
	R22 = (TInt)r;
	//
	// TX = Xp1 - X1*R11 - Y1*R21
	//
   TX = (Xp1<<16) - (X1*R11) - (Y1*R21);
	//
	// TY = Yp1 - X1*R12 - Y1*R22
	//
	TY = (Yp1<<16) - (X1*R12) - (Y1*R22);

	//
	// Write new values into the superpage
	//
	NKern::LockSystem();
	iMachineConfig.iCalibration.iR11 = R11;
	iMachineConfig.iCalibration.iR12 = R12;
	iMachineConfig.iCalibration.iR21 = R21;
	iMachineConfig.iCalibration.iR22 = R22;
	iMachineConfig.iCalibration.iTx  = TX;
	iMachineConfig.iCalibration.iTy  = TY;
	NKern::UnlockSystem();

	return(KErrNone);
	}

/**
Informs the user-side calibration application where to draw
the cross-hairs on the screen

@param aCalibration On return contains the for points on the screen (in screen coordinates)
					where the cross-hairs should be drawn
@return KErrNone if succcessful
*/
TInt DNE1_TBDigitiser::CalibrationPoints(TDigitizerCalibration& aCalibration)
	{
	TVideoInfoV01Buf buf;
	TVideoInfoV01& vidinfo = buf();
	TInt r = Kern::HalFunction(EHalGroupDisplay, EDisplayHalCurrentModeInfo, (TAny*)&buf, NULL);
	if (r != KErrNone)
		return r;
	iScreenSize=vidinfo.iSizeInPixels;

    aCalibration.iBl.iX = aCalibration.iTl.iX = iScreenSize.iWidth/10;
    aCalibration.iTr.iY = aCalibration.iTl.iY = iScreenSize.iHeight/10;
    aCalibration.iBr.iY = aCalibration.iBl.iY = iScreenSize.iHeight-iScreenSize.iHeight/10;
    aCalibration.iTr.iX = aCalibration.iBr.iX = iScreenSize.iWidth-iScreenSize.iWidth/10;
    return r;
	}
/**
Saves the digitiser calibration to the persistent machine configuration area
so that it can be restored after a power-down/up

@return KErrNone if succcessful
*/
TInt DNE1_TBDigitiser::SaveXYInputCalibration()
	{
	NKern::LockSystem();
	iMachineConfig.iCalibrationSaved = iMachineConfig.iCalibration;
	NKern::UnlockSystem();
	return(KErrNone);
	}

/**
Restores the digitiser calibration from the persistent machine configuration area
following a power-up

@param aType indicates whether to restore factory or saved settings
@return KErrNone if succcessful
*/
TInt DNE1_TBDigitiser::RestoreXYInputCalibration(TDigitizerCalibrationType aType)
	{
	TInt r=KErrNone;
	NKern::LockSystem();
	switch (aType)
		{
		case EFactory:
			iMachineConfig.iCalibration = iMachineConfig.iCalibrationFactory;
			break;
		case ESaved:
			iMachineConfig.iCalibration = iMachineConfig.iCalibrationSaved;
			break;
		default:
			r = KErrNotSupported;
			break;
		}
	NKern::UnlockSystem();
	return r;
	}

/**
Gets the digitiser configuration information

@param aInfo On return, contains information about the digitiser's dimensions etc.
*/
void DNE1_TBDigitiser::DigitiserInfo(TDigitiserInfoV01& aInfo)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("DNE1_TBDigitiser::DigitiserInfo"));

	if (iCurrentMode != TOUCHSCREEN_MODE_NONE)
		{
		aInfo.iDigitiserSize.iWidth  = Mode_Config[iCurrentMode].iConfigXyWidth;
		aInfo.iDigitiserSize.iHeight = Mode_Config[iCurrentMode].iConfigXyHeight;
		}
	else
		{
		aInfo.iDigitiserSize.iWidth  = 0;
		aInfo.iDigitiserSize.iHeight = 0;
		}

	aInfo.iOffsetToDisplay.iX    = KConfigXyOffsetX;
	aInfo.iOffsetToDisplay.iY    = KConfigXyOffsetY;
	}

/**
Issues a pen move event if the distance from the last point is greater than the threshold

@param aPoint the pen position in screen coordinates
*/
void DNE1_TBDigitiser::FilterPenMove(const TPoint& aPoint)
	{
	TPoint offset=aPoint;
	offset.iX -= iLastPos.iX;
	offset.iY -= iLastPos.iY;
	if (Abs(offset.iX)>=iCfg.iAccThresholdX || Abs(offset.iY)>=iCfg.iAccThresholdY)
		{
		iLastPos=aPoint;
		IssuePenMoveEvent(aPoint);
		}
	}

/**
Reset the pen move filter
*/
void DNE1_TBDigitiser::ResetPenMoveFilter()
	{
	}


