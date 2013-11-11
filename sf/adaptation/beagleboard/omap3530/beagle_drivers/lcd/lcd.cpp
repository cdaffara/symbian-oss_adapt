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
// iwanj@users.sourceforge.net added NGA support based on Syborg display PDD

// Description:
// omap3530/beagle_drivers/lcd/lcd.cpp
// Implementation of an LCD driver. 
// This file is part of the Beagle Base port
// N.B. This sample code assumes that the display supports setting the backlight on or off, 
// as well as adjusting the contrast and the brightness.
//

#include <videodriver.h>
#include <platform.h>
#include <nkern.h>
#include <kernel.h>
#include <kern_priv.h>
#include <kpower.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_hardware_base.h>
#include <assp/omap3530_assp/omap3530_prcm.h>

//#undef __KTRACE_OPT
//#define __KTRACE_OPT(c,p)	p

#define DSS_SYSCONFIG				0x48050010
#define DISPC_SYSSTATUS				0x48050414

#define DISPC_SYSCONFIG				0x48050410
#define DISPC_CONFIG				0x48050444
#define DISPC_DEFAULT_COLOR0		0x4805044c
#define DISPC_TRANS_COLOR0			0x48050454

#define DISPC_TIMING_H				0x48050464
#define DISPC_TIMING_V				0x48050468
#define DISPC_POL_FREQ				0x4805046c
#define DISPC_DIVISOR				0x48050470
#define DISPC_SIZE_LCD				0x4805047c

#define DISPC_GFX_BA1				0x48050480
#define DISPC_GFX_BA2				0x48050484
#define DISPC_GFX_POSITION			0x48050488
#define DISPC_GFX_SIZE				0x4805048c
#define DISPC_GFX_ATTRIBUTES		0x480504a0

#define DISPC_GFX_FIFO_THRESHOLD	0x480504a4
#define DISPC_GFX_FIFO_SIZE_STATUS	0x480504a8
#define DISPC_GFX_ROW_INC			0x480504ac
#define DISPC_GFX_PIXEL_INC			0x480504b0
#define DISPC_GFX_WINDOW_SKIP		0x480504b4
#define DISPC_GFX_TABLE_BA			0x480504b8

#define DISPC_CONTROL				0x48050440

#define GET_REGISTER(Reg)		*( (TUint *) Omap3530HwBase::TVirtual<Reg>::Value )
#define SET_REGISTER(Reg,Val)	*( (TUint *) Omap3530HwBase::TVirtual<Reg>::Value ) = Val

#define _MODE_1280x1024_
//#define _MODE_1024x768_

#ifdef _MODE_800x600_
// ModeLine       "800x600@60" 40.0 800 840 968 1056 600 601 605 628 +hsync +vsync
// Decoded by: http://www.tkk.fi/Misc/Electronics/faq/vga2rgb/calc.html
#	define PIXEL_CLK	40000
#	define H_DISP		800
#	define H_FPORCH	40
#	define H_SYNC		128
#	define H_BPORCH	88
#	define H_SYNC_POL	1
#	define V_DISP		600
#	define V_FPORCH	1
#	define V_SYNC		4
#	define V_BPORCH	23
#	define V_SYNC_POL	1
#	define INTERLACE_ENABLE	0
#endif
#ifdef _MODE_1024x768_
// ModeLine       "1024x768@60" 65.0 1024 1048 1184 1344 768 771 777 806 -hsync -vsync
// Decoded by: http://www.tkk.fi/Misc/Electronics/faq/vga2rgb/calc.html
#	define PIXEL_CLK	65000
#	define H_DISP		1024
#	define H_FPORCH	24
#	define H_SYNC		136
#	define H_BPORCH	160
#	define H_SYNC_POL	0
#	define V_DISP		768
#	define V_FPORCH	3
#	define V_SYNC		6
#	define V_BPORCH	29
#	define V_SYNC_POL	0
#	define INTERLACE_ENABLE	0
#endif
#ifdef _MODE_1280x1024_
// ModeLine "1280x1024@60" 108.0 1280 1328 1440 1688 1024 1025 1028 1066 +hsync +vsync
// Decoded by: http://www.tkk.fi/Misc/Electronics/faq/vga2rgb/calc.html
#	define PIXEL_CLK	108000
#	define H_DISP		1280
#	define H_FPORCH	48
#	define H_SYNC		112
#	define H_BPORCH	248
#	define H_SYNC_POL	1
#	define V_DISP		1024
#	define V_FPORCH	1
#	define V_SYNC		3
#	define V_BPORCH	38
#	define V_SYNC_POL	1
#	define INTERLACE_ENABLE	0
#endif



// TO DO: (mandatory)
// If the display supports Contrast and/or Brightness control then supply the following defines:
// This is only example code... you may need to modify it for your hardware
const TInt KConfigInitialDisplayContrast	= 128;
const TInt KConfigLcdMinDisplayContrast		= 1;
const TInt KConfigLcdMaxDisplayContrast		= 255;
const TInt KConfigInitialDisplayBrightness	= 128;
const TInt KConfigLcdMinDisplayBrightness	= 1;
const TInt KConfigLcdMaxDisplayBrightness	= 255;

// TO DO: (mandatory)
// define a macro to calculate the screen buffer size
// This is only example code... you may need to modify it for your hardware
// aBpp is the number of bits-per-pixel, aPpl is the number of pixels per line and 
// aLpp number of lines per panel
#define FRAME_BUFFER_SIZE(aBpp,aPpl,aLpp)	(aBpp*aPpl*aLpp)/8	
																

// TO DO: (mandatory)
// define the physical screen dimensions
// This is only example code... you need to modify it for your hardware
const TUint	KConfigLcdWidth					= 360;//640;		// 640 pixels per line
const TUint	KConfigLcdHeight				= 640;//480;		// 480 lines per panel

// TO DO: (mandatory)
// define the characteristics of the LCD display
// This is only example code... you need to modify it for your hardware
const TBool	KConfigLcdIsMono				= EFalse;
const TBool	KConfigLcdPixelOrderLandscape	= ETrue;
const TBool	KConfigLcdPixelOrderRGB			= ETrue;
const TInt	KConfigLcdMaxDisplayColors		= 16777216;//65536;	//24bit: 16777216;


// TO DO: (mandatory)
// define the display dimensions in TWIPs
// A TWIP is a 20th of a point.  A point is a 72nd of an inch
// Therefore a TWIP is a 1440th of an inch
// This is only example code... you need to modify it for your hardware
const TInt	KConfigLcdWidthInTwips			= 2670;		// = 6.69 inches	//15*1440;
const TInt	KConfigLcdHeightInTwips			= 3550;		//5616;		// = 5.11 inches	//12*1440;

// TO DO: (mandatory)
// define the available display modes
// This is only example code... you need to modify it for your hardware
const TInt  KConfigLcdNumberOfDisplayModes	= 1;
const TInt  KConfigLcdInitialDisplayMode	= 0;
struct SLcdConfig
	{
	TInt iMode;
	TInt iOffsetToFirstVideoBuffer;
	TInt iLenghtOfVideoBufferInBytes;
	TInt iOffsetBetweenLines;
	TBool iIsPalettized;
	TInt iBitsPerPixel;
	};
static const SLcdConfig Lcd_Mode_Config[KConfigLcdNumberOfDisplayModes]=
	{
		{
		0,								// iMode
		0,								// iOffsetToFirstVideoBuffer
		FRAME_BUFFER_SIZE(32/*16*/, KConfigLcdWidth, KConfigLcdHeight),	// iLenghtOfVideoBufferInBytes
		KConfigLcdWidth*4,//2,				// iOffsetBetweenLines
		EFalse,							// iIsPalettized
		32,//16								// iBitsPerPixel
		}
	};	



_LIT(KLitLcd,"LCD");

//
// TO DO: (optional)
//
// Add any private functions and data you require
//
NONSHARABLE_CLASS(DLcdPowerHandler) : public DPowerHandler
	{
public: 
	DLcdPowerHandler();
	
	// from DPowerHandler
	void PowerDown(TPowerState);
	void PowerUp();

	void PowerUpDfc();
	void PowerDownDfc();

	TInt Create();
	void DisplayOn();
	void DisplayOff();
	TInt HalFunction(TInt aFunction, TAny* a1, TAny* a2);

	void PowerUpLcd(TBool aSecure);
	void PowerDownLcd();

	void ScreenInfo(TScreenInfoV01& aInfo);
	void WsSwitchOnScreen();
	void WsSwitchOffScreen();
	void HandleMsg();
	void SwitchDisplay(TBool aSecure);

	void SetBacklightState(TBool aState);
	void BacklightOn();
	void BacklightOff();
	TInt SetContrast(TInt aContrast);
	TInt SetBrightness(TInt aBrightness);

#ifdef ENABLE_GCE_MODE
	void ChangeFrameBufferAddress(TUint32 aFbAddr);
#endif

private:
	TInt SetPaletteEntry(TInt aEntry, TInt aColor);
	TInt GetPaletteEntry(TInt aEntry, TInt* aColor);
	TInt NumberOfPaletteEntries();
	TInt GetCurrentDisplayModeInfo(TVideoInfoV01& aInfo, TBool aSecure);
	TInt GetSpecifiedDisplayModeInfo(TInt aMode, TVideoInfoV01& aInfo);
	TInt SetDisplayMode(TInt aMode);
	void SplashScreen();
	TInt GetDisplayColors(TInt* aColors);

#ifdef ENABLE_GCE_MODE
public:
	static DLcdPowerHandler* pLcd;
	TInt iSize;
	TPhysAddr iCompositionPhysical;
	TVideoInfoV01 iVideoInfo;
	TDfcQue* iDfcQ;
	TPhysAddr ivRamPhys;
#endif

private:
	TBool iIsPalettized;
	TBool iDisplayOn;				// to prevent a race condition with WServer trying to power up/down at the same time
	DPlatChunkHw* iChunk;
	DPlatChunkHw* iSecureChunk;
	TBool iWsSwitchOnScreen;
 	TBool iSecureDisplay;
	TMessageQue iMsgQ;
	TDfc iPowerUpDfc;
	TDfc iPowerDownDfc;	

#ifndef ENABLE_GCE_MODE
	TDfcQue* iDfcQ;
	TVideoInfoV01 iVideoInfo;
	TPhysAddr ivRamPhys;
#endif

	TVideoInfoV01 iSecureVideoInfo;
	NFastMutex iLock;				// protects against being preempted whilst manipulating iVideoInfo/iSecureVideoInfo
	TPhysAddr iSecurevRamPhys;
	
	TBool iBacklightOn;
	TInt iContrast;
	TInt iBrightness;
	};


/**
HAL handler function

@param	aPtr a pointer to an instance of DLcdPowerHandler
@param	aFunction the function number
@param	a1 an arbitrary parameter
@param	a2 an arbitrary parameter
*/
LOCAL_C TInt halFunction(TAny* aPtr, TInt aFunction, TAny* a1, TAny* a2)
	{
	DLcdPowerHandler* pH=(DLcdPowerHandler*)aPtr;
	return pH->HalFunction(aFunction,a1,a2);
	}

/**
DFC for receiving messages from the power handler
@param	aPtr a pointer to an instance of DLcdPowerHandler
*/
void rxMsg(TAny* aPtr)
	{
	DLcdPowerHandler& h=*(DLcdPowerHandler*)aPtr;
	h.HandleMsg();
	}

/**
DFC for powering up the device

@param aPtr	aPtr a pointer to an instance of DLcdPowerHandler
*/
void power_up_dfc(TAny* aPtr)
	{
	((DLcdPowerHandler*)aPtr)->PowerUpDfc();
	}

/**
DFC for powering down the device

@param aPtr	aPtr a pointer to an instance of DLcdPowerHandler
*/
void power_down_dfc(TAny* aPtr)
	{
	((DLcdPowerHandler*)aPtr)->PowerDownDfc();
	}


/**
Default constructor
*/
DLcdPowerHandler::DLcdPowerHandler() :
		DPowerHandler(KLitLcd),
		iMsgQ(rxMsg,this,NULL,1),
		iPowerUpDfc(&power_up_dfc,this,6),
		iPowerDownDfc(&power_down_dfc,this,7),
		iBacklightOn(EFalse),
		iContrast(KConfigInitialDisplayContrast),
		iBrightness(KConfigInitialDisplayBrightness)
	{
	}


/**
Second-phase constructor 

Called by factory function at ordinal 0
*/
TInt DLcdPowerHandler::Create()
	{
#ifdef ENABLE_GCE_MODE
	pLcd = this;
#endif

	iDfcQ=Kern::DfcQue0();	// use low priority DFC queue for this driver 

	// map the video RAM
	
	//TPhysAddr videoRamPhys;
	TInt vSize = Lcd_Mode_Config[KConfigLcdInitialDisplayMode].iLenghtOfVideoBufferInBytes; //KConfigLcdWidth*KConfigLcdHeight*3; //VideoRamSize();
	TInt r = Epoc::AllocPhysicalRam( 2*vSize, ivRamPhys );
	if ( r!=KErrNone )
		{
		Kern::Fault( "AllocVRam", r );
		}
	
	//TInt vSize = ((Omap3530BoardAssp*)Arch::TheAsic())->VideoRamSize();
	//ivRamPhys = TOmap3530Assp::VideoRamPhys();				// EXAMPLE ONLY: assume TOmap3530Assp interface class
	r = DPlatChunkHw::New(iChunk,ivRamPhys,vSize,EMapAttrUserRw|EMapAttrBufferedC);
	if (r != KErrNone)
		return r;	
	
	//create "secure" screen immediately after normal one
	iSecurevRamPhys =  ivRamPhys + vSize;
	TInt r2 = DPlatChunkHw::New(iSecureChunk,iSecurevRamPhys,vSize,EMapAttrUserRw|EMapAttrBufferedC);
	if (r2 != KErrNone)
		return r2;
	
	TUint* pV=(TUint*)iChunk->LinearAddress();	
	
	__KTRACE_OPT(KEXTENSION,Kern::Printf("DLcdPowerHandler::Create: VideoRamSize=%x, VideoRamPhys=%08x, VideoRamLin=%08x",vSize,ivRamPhys,pV));
	
	// TO DO: (mandatory)
	// initialise the palette for the initial display mode
	// NOTE: the palette could either be a buffer allocated in system RAM (usually contiguous to Video buffer)
	//		 or could be offered as part of the hardware block that implemenst the lcd control
	//

	TUint* pV2=(TUint*)iSecureChunk->LinearAddress();

	__KTRACE_OPT(KEXTENSION,Kern::Printf("DLcdPowerHandler::Create: Secure display VideoRamSize=%x, VideoRamPhys=%08x, VideoRamLin=%08x",vSize,iSecurevRamPhys,pV2));

	// TO DO: (mandatory)
	// initialise the secure screen's palette for the initial display mode
	//
	
	// setup the video info structure, this'll be used to remember the video settings
	iVideoInfo.iDisplayMode = KConfigLcdInitialDisplayMode;
	iVideoInfo.iOffsetToFirstPixel = Lcd_Mode_Config[KConfigLcdInitialDisplayMode].iOffsetToFirstVideoBuffer;
	iVideoInfo.iIsPalettized = Lcd_Mode_Config[KConfigLcdInitialDisplayMode].iIsPalettized;
	iVideoInfo.iOffsetBetweenLines = Lcd_Mode_Config[KConfigLcdInitialDisplayMode].iOffsetBetweenLines;
	iVideoInfo.iBitsPerPixel = Lcd_Mode_Config[KConfigLcdInitialDisplayMode].iBitsPerPixel;

	iVideoInfo.iSizeInPixels.iWidth = KConfigLcdWidth;
	iVideoInfo.iSizeInPixels.iHeight = KConfigLcdHeight;
	iVideoInfo.iSizeInTwips.iWidth = KConfigLcdWidthInTwips;
	iVideoInfo.iSizeInTwips.iHeight = KConfigLcdHeightInTwips;
	iVideoInfo.iIsMono = KConfigLcdIsMono;
	iVideoInfo.iVideoAddress=(TInt)pV;
	iVideoInfo.iIsPixelOrderLandscape = KConfigLcdPixelOrderLandscape;
	iVideoInfo.iIsPixelOrderRGB = KConfigLcdPixelOrderRGB;

	iSecureVideoInfo = iVideoInfo;
	iSecureVideoInfo.iVideoAddress = (TInt)pV2;

	iDisplayOn = EFalse;
	iSecureDisplay = EFalse;

#ifdef ENABLE_GCE_MODE
	// Alloc Physical RAM for the Composition Buffers used by the GCE
	iSize = Lcd_Mode_Config[KConfigLcdInitialDisplayMode].iLenghtOfVideoBufferInBytes;
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DLcdPowerHandler.iSize  = %d", iSize));

	// double and round the page size
	TUint round = 2*Kern::RoundToPageSize(iSize);
	r = Epoc::AllocPhysicalRam(round , iCompositionPhysical);
	if(r != KErrNone)
		{
		__KTRACE_OPT(KEXTENSION, Kern::Printf("Failed to allocate physical RAM for composition buffer %d", r));
		return r;
		}
#endif

	// install the HAL function
	r=Kern::AddHalEntry(EHalGroupDisplay, halFunction, this);
	if (r!=KErrNone)
		return r;

	iPowerUpDfc.SetDfcQ(iDfcQ);
	iPowerDownDfc.SetDfcQ(iDfcQ);
	iMsgQ.SetDfcQ(iDfcQ);
	iMsgQ.Receive();

	// install the power handler
	// power up the screen
	Add();
	DisplayOn();
	
	SplashScreen();
	
	return KErrNone;
	}

/**
Turn the display on
May be called as a result of a power transition or from the HAL
If called from HAL, then the display may be already be on (iDisplayOn == ETrue)
*/
void DLcdPowerHandler::DisplayOn()
	{
	__KTRACE_OPT(KBOOT, Kern::Printf("DisplayOn %d", iDisplayOn));
	if (!iDisplayOn)				// may have been powered up already
		{
		iDisplayOn = ETrue;
		PowerUpLcd(iSecureDisplay);
		SetContrast(iContrast);
		SetBrightness(iBrightness);
		}
	}

/**
Turn the display off
May be called as a result of a power transition or from the HAL
If called from Power Manager, then the display may be already be off (iDisplayOn == EFalse)
if the platform is in silent running mode
*/
void DLcdPowerHandler::DisplayOff()
	{
	__KTRACE_OPT(KBOOT, Kern::Printf("DisplayOff %d", iDisplayOn));
	if (iDisplayOn)
		{
		iDisplayOn = EFalse;
		PowerDownLcd();
		}
	}

/**
Switch between secure and non-secure displays

@param aSecure ETrue if switching to secure display
*/
void DLcdPowerHandler::SwitchDisplay(TBool aSecure)
 	{
 	if (aSecure)
 		{
 		if (!iSecureDisplay)
 			{
 			//switch to secure display
 			DisplayOff();
 			iSecureDisplay = ETrue;
 			DisplayOn();
 			}
 		}
 	else
 		{
 		if (iSecureDisplay)
 			{
 			//switch from secure display
 			DisplayOff();
 			iSecureDisplay = EFalse;
 			DisplayOn();
 			}
 		}
 	}

/**
DFC to power up the display
*/
void DLcdPowerHandler::PowerUpDfc()
	{
	__KTRACE_OPT(KPOWER, Kern::Printf("PowerUpDfc"));
	DisplayOn();

	PowerUpDone();				// must be called from a different thread than PowerUp()
	}

/**
DFC to power down the display
*/
void DLcdPowerHandler::PowerDownDfc()
	{
	__KTRACE_OPT(KPOWER, Kern::Printf("PowerDownDfc"));
	DisplayOff();
	PowerDownDone();			// must be called from a different thread than PowerUp()
	}

/**
Schedule the power-down DFC
*/
void DLcdPowerHandler::PowerDown(TPowerState)
	{
	iPowerDownDfc.Enque();		// schedules DFC to execute on this driver's thread
	}

/**
Schedule the power-up DFC
*/
void DLcdPowerHandler::PowerUp()
	{
	iPowerUpDfc.Enque();		// schedules DFC to execute on this driver's thread
	}

/**
Power up the display

@param aSecure ETrue if powering up the secure display
*/
void DLcdPowerHandler::PowerUpLcd(TBool aSecure)
    {
	
	TUint32 l = 0x0;
	
	// Set up the Display Subsystem to control a DVI monitor
	
    // The following four lines need to be replaced by a call to the GPIO driver which should call the PowerClock driver
//	PowerClock::GpioActive(0, PowerClock::E1s, PowerClock::ECpu10, PowerClock::EBus10);
//	PowerClock::GpioAccess(0, PowerClock::EAuto);
	Prcm::SetClockState( Prcm::EClkGpio1_F, Prcm::EClkOn );
	Prcm::SetClockState( Prcm::EClkGpio1_I, Prcm::EClkAuto );
	*( (TUint *) Omap3530HwBase::TVirtual<0x48310034>::Value ) = 0xfefffedf;		//GPIO1 output enable	p3336
	*( (TUint *) Omap3530HwBase::TVirtual<0x48310094>::Value ) = 0x01000120;		//GPIO1 set data out	p3336
//	const TUint KCM_CLKSEL_DSS = Omap3530HwBase::TVirtual<0x48004E40>::Value;
//	Prcm::Set(KCM_CLKSEL_DSS, 0xffffffffu, 0x00001006);

	Prcm::SetDivider( Prcm::EClkDss1_F, 2 );
	Prcm::SetDivider( Prcm::EClkTv_F, 1 );
	
	
	SET_REGISTER( DSS_SYSCONFIG, 0x00000010 );			// Display Subsystem reset
	while ( !( GET_REGISTER( DISPC_SYSSTATUS ) & 1 ) );	// Spin until reset complete
	
	TInt8 MIDLEMODE		= 0x2;	// Smart Standby. MStandby is asserted based on the internal activity of the module.
	TInt8 CLOCKACTIVITY = 0x0;	// interface and functional clocks can be switched off.
	TInt8 SIDLEMODE		= 0x2;	// Smart idle. Idle request is acknowledged based on the internal activity of the module
	TInt8 ENWAKEUP		= 0x1;	// Wakeup is enabled.
	TInt8 SOFTRESET		= 0x0;	// Normal mode
	TInt8 AUTOIDLE		= 0x1;	// Automatic L3 and L4 interface clock gating strategy is applied based on interface activity
	l = MIDLEMODE<<12 | CLOCKACTIVITY<<8 | SIDLEMODE<<3 | ENWAKEUP<<2 | SOFTRESET<<1 | AUTOIDLE;
	SET_REGISTER( DISPC_SYSCONFIG, l );
	
	TInt8 LOADMOAD = 0x2; 		//Frame data only loaded every frame
	l = LOADMOAD<<1;
	SET_REGISTER( DISPC_CONFIG, l );
	
	SET_REGISTER( DISPC_DEFAULT_COLOR0, 0xFFFFFFFF );
	SET_REGISTER( DISPC_TRANS_COLOR0, 0x00000000 );
	
	TUint8 hbp = H_BPORCH - 1;	// Horizontal Back Porch
	TUint8 hfp = H_FPORCH - 1;	// Horizontal front porch
	TUint8 hsw = H_SYNC - 1;	// Horizontal synchronization pulse width
	if ( hsw > 63 )
		{
		hsw = 63;
		Kern::Printf("[LCD] H_SYNC too big");
		}
	l = hbp<<20 | hfp<<8 | hsw;
	SET_REGISTER( DISPC_TIMING_H, l );
	
	TUint8 vbp = V_BPORCH;		// Vertical back porch
	TUint8 vfp = V_FPORCH;		// Vertical front porch
	TUint8 vsw = V_SYNC;		// Vertical synchronization pulse width
	__ASSERT_ALWAYS( vbp<=255, Kern::Fault("LCD", 1) );
	__ASSERT_ALWAYS( vfp<=255, Kern::Fault("LCD", 1) );
	__ASSERT_ALWAYS( vsw>=1 && vsw<=255, Kern::Fault("LCD", 1) );
	l = vbp<<20 | vfp<<8 | vsw;
	SET_REGISTER( DISPC_TIMING_V, l );
	
	TUint8 onoff= 0;
	TUint8 rf	= 0;
	TUint8 ieo 	= 0;
	TUint8 ipc	= 1;			// Invert Pixel Clock
	TUint8 ihs	= H_SYNC_POL ? 0 : 1;	// Invert HSYNC (0: Positive Sync polarity, 1: Negative Sync polarity)
	TUint8 ivs	= V_SYNC_POL ? 0 : 1;	// Invert VSYNC (0: Positive Sync polarity, 1: Negative Sync polarity)
	TUint8 acbi	= 0;
	TUint16 acb	= 0x28;			// AC-bias pin frequency
	l = onoff<<17 | rf<<16 | ieo<<15 | ipc<<14 | ihs<<13 | ivs<<12 | acbi<<8 | acb;
	SET_REGISTER( DISPC_POL_FREQ, l );
	
	TUint8 lcd = 1;				// Display Controller Logic Clock Divisor
	TUint8 pcd = ( 432000 + (PIXEL_CLK - 1) ) / PIXEL_CLK; // Pixel Clock Divisor - add (PIXEL_CLK - 1) to avoid rounding error
	__ASSERT_ALWAYS( lcd>=1 && lcd<=255, Kern::Fault("LCD", 1) );
	__ASSERT_ALWAYS( pcd>=2 && pcd<=255, Kern::Fault("LCD", 1) );
	l = lcd<<16 | pcd;
	SET_REGISTER( DISPC_DIVISOR, l );
	
	TUint16 ppl = H_DISP - 1;	// Pixels per line
	TUint16 llp = V_DISP - 1;	// Lines per panel
	__ASSERT_ALWAYS( ppl>=1 && ppl<=2048, Kern::Fault("LCD", 1) );
	__ASSERT_ALWAYS( llp>=1 && llp<=2048, Kern::Fault("LCD", 1) );
	l = llp<<16 | ppl;
	SET_REGISTER( DISPC_SIZE_LCD, l );
	
	
	// Setup a graphics region (GFX)
	
	// Set GFX frame buffer
	SET_REGISTER( DISPC_GFX_BA1, ivRamPhys );
	
	// Center the GFX
	TInt16 gfxposy	= ( V_DISP - KConfigLcdHeight ) / 2;
	TInt16 gfxposx	= ( H_DISP - KConfigLcdWidth ) / 2;
	l = ( gfxposy << 16 ) | gfxposx;
	SET_REGISTER( DISPC_GFX_POSITION, l );
	
	// Set the GFX dimensions
	TInt16 gfxsizey = KConfigLcdHeight - 1;
	TInt16 gfxsizex = KConfigLcdWidth - 1;
	l = gfxsizey<<16 | gfxsizex;
	SET_REGISTER( DISPC_GFX_SIZE, l );
	
	TInt8 GFXSELFREFRESH		= 0x0;
	TInt8 GFXARBITRATION		= 0x0;
	TInt8 GFXROTATION			= 0x0;
	TInt8 GFXFIFOPRELOAD		= 0x0;
	TInt8 GFXENDIANNESS			= 0x0;
	TInt8 GFXNIBBLEMODE			= 0x0;
	TInt8 GFXCHANNELOUT			= 0x0;
	TInt8 GFXBURSTSIZE			= 0x2;	// 16x32bit bursts
	TInt8 GFXREPLICATIONENABLE	= 0x0;	// Disable Graphics replication logic
	TInt8 GFXFORMAT				= 0x8;//0x6;	// RGB16=0x6, RGB24-unpacked=0x8, RGB24-packed=0x9
	TInt8 GFXENABLE				= 0x1;	// Graphics enabled
	l = GFXSELFREFRESH<<15 | GFXARBITRATION<<14 | GFXROTATION<<12 | GFXFIFOPRELOAD<<11 | GFXENDIANNESS<<10 | GFXNIBBLEMODE<<9 | GFXCHANNELOUT<8 | GFXBURSTSIZE<<6 | GFXREPLICATIONENABLE<<5 | GFXFORMAT<<1 | GFXENABLE;
	SET_REGISTER( DISPC_GFX_ATTRIBUTES, l );
	
	TInt16 GFXFIFOHIGHTHRESHOLD	= 0x3fc;	// Graphics FIFO High Threshold
	TInt16 GFXFIFOLOWTHRESHOLD	= 0x3BC;	// Graphics FIFO Low Threshold
	l = GFXFIFOHIGHTHRESHOLD<<16 | GFXFIFOLOWTHRESHOLD;
	SET_REGISTER(DISPC_GFX_FIFO_THRESHOLD, l);
	
	TInt16 GFXFIFOSIZE = 0x400;	// Number of bytes defining the FIFO value
	l = GFXFIFOSIZE;
	SET_REGISTER(DISPC_GFX_FIFO_SIZE_STATUS, l);
	
	TInt32 GFXROWINC	= 0x1;
	l = GFXROWINC;
	SET_REGISTER(DISPC_GFX_ROW_INC, l);
	
	TInt16	GFXPIXELINC	= 0x1;
	l = GFXPIXELINC;
	SET_REGISTER(DISPC_GFX_PIXEL_INC, l);
	
	TInt32 GFXWINDOWSKIP = 0x0;
	l = GFXWINDOWSKIP;
	SET_REGISTER(DISPC_GFX_WINDOW_SKIP, l);
	
	// TO DO: Sort out the Gamma table + pallets
	TInt32 GFXTABLEBA	= 0x807ff000;
	l = GFXTABLEBA;
	SET_REGISTER(DISPC_GFX_TABLE_BA, l);
	
	
	// Propigate all the shadowed registers 
	
	TInt8 SPATIALTEMPORALDITHERINGFRAMES	= 0;
	TInt8 LCDENABLEPOL			= 0;
	TInt8 LCDENABLESIGNAL		= 0;
	TInt8 PCKFREEENABLE			= 0;
	TInt8 TDMUNUSEDBITS			= 0;
	TInt8 TDMCYCLEFORMAT		= 0;
	TInt8 TDMPARALLELMODE		= 0;
	TInt8 TDMENABLE				= 0;
	TInt8 HT					= 0;
	TInt8 GPOUT1				= 1;
	TInt8 GPOUT0				= 1;
	TInt8 GPIN1					= 0;
	TInt8 GPIN0					= 0;
	TInt8 OVERLAYOPTIMIZATION	= 0;
	TInt8 RFBIMODE				= 0;
	TInt8 SECURE				= 0;
	TInt8 TFTDATALINES			= 0x3;
	TInt8 STDITHERENABLE		= 0;
	TInt8 GODIGITAL				= 1;
	TInt8 GOLCD					= 1;
	TInt8 M8B					= 0;
	TInt8 STNTFT				= 1;
	TInt8 MONOCOLOR				= 0;
	TInt8 DIGITALENABLE			= 1;
	TInt8 LCDENABLE				= 1;	
	l = SPATIALTEMPORALDITHERINGFRAMES<<30 | LCDENABLEPOL<<29 | LCDENABLESIGNAL<<28 | PCKFREEENABLE<<27 | 
		TDMUNUSEDBITS<<25 | TDMCYCLEFORMAT<<23 | TDMPARALLELMODE<<21 | TDMENABLE<<20 | HT<<17 | GPOUT1<<16 | 
			GPOUT0<<15 | GPIN1<<14 | GPIN0<<13 | OVERLAYOPTIMIZATION<<12 | 	RFBIMODE<<11 | SECURE<<10 |
				TFTDATALINES<<8 | STDITHERENABLE<<7 | GODIGITAL<<6 | GOLCD<<5 | M8B<<4 | STNTFT<<3 |
					MONOCOLOR<<2 | DIGITALENABLE<<1 | LCDENABLE;
	NKern::Sleep(1);
	SET_REGISTER(DISPC_CONTROL, l);
	NKern::Sleep(1);
	
    }


/**
Power down the display and the backlight
*/
void DLcdPowerHandler::PowerDownLcd()
    {
	SetBacklightState(EFalse);

	// TO DO: (mandatory)
	// Power down the display & disable LCD DMA.
	// May need to wait until the current frame has been output
	//
	
	SET_REGISTER(DISPC_CONTROL, 0);
	
    }

/**
Set the Lcd contrast

@param aValue the contrast setting
*/
TInt DLcdPowerHandler::SetContrast(TInt aValue)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetContrast(%d)", aValue));

	if (aValue >= KConfigLcdMinDisplayContrast && aValue <= KConfigLcdMaxDisplayContrast)
		{
		iContrast=aValue;
		
		// TO DO: (mandatory)
		// set the contrast
		//
		return KErrNone;
		}

	return KErrArgument;
	}

/**
Set the Lcd brightness

@param aValue the brightness setting
*/
TInt DLcdPowerHandler::SetBrightness(TInt aValue)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetBrightness(%d)", aValue));

	if (aValue >= KConfigLcdMinDisplayBrightness && aValue <= KConfigLcdMaxDisplayBrightness)
		{
		iBrightness=aValue;

		// TO DO: (mandatory)
		// set the brightness
		//
		return KErrNone;
		}
	return KErrArgument;
	}

/**
Turn the backlight on
*/
void DLcdPowerHandler::BacklightOn()
    {
	// TO DO: (mandatory)
	// turn the backlight on
	//
    }

/**
Turn the backlight off
*/
void DLcdPowerHandler::BacklightOff()
    {
	// TO DO: (mandatory)
	// turn the backlight off
	//
    }

/**
Set the state of the backlight

@param aState ETrue if setting the backlight on
*/
void DLcdPowerHandler::SetBacklightState(TBool aState)
	{
	iBacklightOn=aState;
	if (iBacklightOn)
		BacklightOn();
	else
		BacklightOff();
	}

void DLcdPowerHandler::ScreenInfo(TScreenInfoV01& anInfo)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("DLcdPowerHandler::ScreenInfo"));
	anInfo.iWindowHandleValid=EFalse;
	anInfo.iWindowHandle=NULL;
	anInfo.iScreenAddressValid=ETrue;
	anInfo.iScreenAddress=(TAny *)(iChunk->LinearAddress());
	anInfo.iScreenSize.iWidth=KConfigLcdWidth;
	anInfo.iScreenSize.iHeight=KConfigLcdHeight;
	}

/**
Handle a message from the power handler
*/
void DLcdPowerHandler::HandleMsg(void)
	{
	
	TMessageBase* msg = iMsgQ.iMessage;
	if (msg == NULL)
		return;

	if (msg->iValue)
		DisplayOn();
	else
		DisplayOff();
	msg->Complete(KErrNone,ETrue);
	}

/**
Send a message to the power-handler message queue to turn the display on
*/
void DLcdPowerHandler::WsSwitchOnScreen()
	{
	TThreadMessage& m=Kern::Message();
	m.iValue = ETrue;
	m.SendReceive(&iMsgQ);		// send a message and block Client thread until keyboard has been powered up
	}

/**
Send a message to the power-handler message queue to turn the display off
*/
void DLcdPowerHandler::WsSwitchOffScreen()
	{
	TThreadMessage& m=Kern::Message();
	m.iValue = EFalse;
	m.SendReceive(&iMsgQ);		// send a message and block Client thread until keyboard has been powered down
	}

/**
Return information about the current display mode

@param	aInfo a structure supplied by the caller to be filled by this function.
@param	aSecure ETrue if requesting information about the secure display
@return	KErrNone if successful
*/
TInt DLcdPowerHandler::GetCurrentDisplayModeInfo(TVideoInfoV01& aInfo, TBool aSecure)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("GetCurrentDisplayModeInfo"));
	NKern::FMWait(&iLock);
	if (aSecure)
 		aInfo = iSecureVideoInfo;
 	else
 		aInfo = iVideoInfo;
	NKern::FMSignal(&iLock);
	return KErrNone;
	}

/**
Return information about the specified display mode

@param	aMode the display mode to query
@param	aInfo a structure supplied by the caller to be filled by this function.
@return	KErrNone if successful
*/
TInt DLcdPowerHandler::GetSpecifiedDisplayModeInfo(TInt aMode, TVideoInfoV01& aInfo)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("GetSpecifiedDisplayModeInfo mode is %d",aMode));

	if (aMode < 0 || aMode >= KConfigLcdNumberOfDisplayModes)
		return KErrArgument;

	NKern::FMWait(&iLock);
	aInfo = iVideoInfo;
	NKern::FMSignal(&iLock);

	if (aMode != aInfo.iDisplayMode)
		{
		aInfo.iOffsetToFirstPixel=Lcd_Mode_Config[aMode].iOffsetToFirstVideoBuffer;
		aInfo.iIsPalettized = Lcd_Mode_Config[aMode].iIsPalettized;
		aInfo.iOffsetBetweenLines=Lcd_Mode_Config[aMode].iOffsetBetweenLines;
		aInfo.iBitsPerPixel = Lcd_Mode_Config[aMode].iBitsPerPixel;
		}
	return KErrNone;
	}

/**
Set the display mode

@param	aMode the display mode to set
*/
TInt DLcdPowerHandler::SetDisplayMode(TInt aMode)
	{

	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetDisplayMode = %d", aMode));

	if (aMode < 0 || aMode >= KConfigLcdNumberOfDisplayModes)
		return KErrArgument;

	NKern::FMWait(&iLock);

	// store the current mode
	iVideoInfo.iDisplayMode = aMode;
	iVideoInfo.iOffsetToFirstPixel = Lcd_Mode_Config[aMode].iOffsetToFirstVideoBuffer;
	iVideoInfo.iIsPalettized = Lcd_Mode_Config[aMode].iIsPalettized;
	iVideoInfo.iOffsetBetweenLines = Lcd_Mode_Config[aMode].iOffsetBetweenLines;
	iVideoInfo.iBitsPerPixel = Lcd_Mode_Config[aMode].iBitsPerPixel;

	// store the current mode for secure screen
	iSecureVideoInfo.iDisplayMode = aMode;
	iSecureVideoInfo.iOffsetToFirstPixel = Lcd_Mode_Config[aMode].iOffsetToFirstVideoBuffer;
	iSecureVideoInfo.iIsPalettized = Lcd_Mode_Config[aMode].iIsPalettized;
	iSecureVideoInfo.iOffsetBetweenLines = Lcd_Mode_Config[aMode].iOffsetBetweenLines;
	iSecureVideoInfo.iBitsPerPixel = Lcd_Mode_Config[aMode].iBitsPerPixel;
	
	// TO DO: (mandatory)
	// set bits per pixel on hardware
	// May need to reconfigure DMA if video buffer size and location have changed
	//
	NKern::FMSignal(&iLock);

	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetDisplayMode mode = %d, otfp = %d, palettized = %d, bpp = %d, obl = %d",
		aMode, iVideoInfo.iOffsetToFirstPixel, iVideoInfo.iIsPalettized, iVideoInfo.iBitsPerPixel, iVideoInfo.iOffsetBetweenLines));

	return KErrNone;
	}

/**
Fill the video memory with an initial pattern or image
This will be displayed on boot-up
*/
void DLcdPowerHandler::SplashScreen()
	{
	// TO DO: (optional)
	// replace the example code below to display a different spash screen
	
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Splash SCreen +"));
	TUint* pV=(TUint*)(iVideoInfo.iVideoAddress + iVideoInfo.iOffsetToFirstPixel);
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Splash SCreen FB @ %x",pV));
	
	//Fill the framebuffer with bars
	
	for (TInt y = 0; y<KConfigLcdHeight; ++y)
		{
		for(TInt x = 0; x<KConfigLcdHeight; ++x)
			{
			TUint8 r = 0x00;
			TUint8 g = 0x00;
			TUint8 b = 0x00;
			TUint16 rgb = ((r&0xf8) << 8) | ((g&0xfc) << 3) | ((b&0xf8) >> 3);
			
			TUint16* px = reinterpret_cast<TUint16*>(pV) + y*KConfigLcdWidth + x;
			*px = rgb;
			}
		}
	
	__KTRACE_OPT(KEXTENSION,Kern::Printf("Splash SCreen -"));
	}


/**
Get the size of the pallete

@return	the number of pallete entries
*/
TInt DLcdPowerHandler::NumberOfPaletteEntries()		//only call when holding mutex
	{
	// TO DO: (mandatory)
	// Calculate the number of Palette entries - this is normally 
	// calculated from the bits per-pixel.
	// This is only example code... you may need to modify it for your hardware
	//
	TInt num = iVideoInfo.iIsPalettized ? 1<<iVideoInfo.iBitsPerPixel : 0;

	__KTRACE_OPT(KEXTENSION,Kern::Printf("NumberOfPaletteEntries = %d", num));

	return num;
	}


/** 
Retrieve the palette entry at a particular offset

@param	aEntry the palette index
@param	aColor a caller-supplied pointer to a location where the returned RGB color is to be stored
@return	KErrNone if successful
		KErrNotSupported if the current vide mode does not support a palette
		KErrArgument if aEntry is out of range
*/
TInt DLcdPowerHandler::GetPaletteEntry(TInt aEntry, TInt* aColor)
	{
	NKern::FMWait(&iLock);
	if (!iVideoInfo.iIsPalettized)
		{
		NKern::FMSignal(&iLock);
		return KErrNotSupported;
		}

	if ((aEntry < 0) || (aEntry >= NumberOfPaletteEntries()))
		{
		NKern::FMSignal(&iLock);
		return KErrArgument;
		}

	// TO DO: (mandatory)
	// read the RGB value of the palette entry into aColor
	// NOTE: the palette could either be a buffer allocated in system RAM (usually contiguous to Video buffer)
	//		 or could be offered as part of the hardware block that implemenst the lcd control
	//
	NKern::FMSignal(&iLock);

	__KTRACE_OPT(KEXTENSION,Kern::Printf("GetPaletteEntry %d color 0x%x", aEntry, aColor));

	return KErrNone;
	}

/** 
Set the palette entry at a particular offset

@param	aEntry the palette index
@param	aColor the RGB color to store
@return	KErrNone if successful
		KErrNotSupported if the current vide mode does not support a palette
		KErrArgument if aEntry is out of range
*/
TInt DLcdPowerHandler::SetPaletteEntry(TInt aEntry, TInt aColor)
	{

	NKern::FMWait(&iLock);
	if (!iVideoInfo.iIsPalettized)
		{
		NKern::FMSignal(&iLock);
		return KErrNotSupported;
		}

	if ((aEntry < 0) || (aEntry >= NumberOfPaletteEntries()))	//check entry in range
		{
		NKern::FMSignal(&iLock);
		return KErrArgument;
		}

	// TO DO: (mandatory)
	// update the palette entry for the secure and non-secure screen
	// NOTE: the palette could either be a buffer allocated in system RAM (usually contiguous to Video buffer)
	//		 or could be offered as part of the hardware block that implemenst the lcd control
	//
	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetPaletteEntry %d to 0x%x", aEntry, aColor ));

	return KErrNone;
	}

/**
a HAL entry handling function for HAL group attribute EHalGroupDisplay

@param	a1 an arbitrary argument
@param	a2 an arbitrary argument
@return	KErrNone if successful
*/
TInt DLcdPowerHandler::HalFunction(TInt aFunction, TAny* a1, TAny* a2)
	{
	TInt r=KErrNone;
	switch(aFunction)
		{
		case EDisplayHalScreenInfo:
			{
			TPckgBuf<TScreenInfoV01> vPckg;
			ScreenInfo(vPckg());
			Kern::InfoCopy(*(TDes8*)a1,vPckg);
			break;
			}

		case EDisplayHalWsRegisterSwitchOnScreenHandling:
			iWsSwitchOnScreen=(TBool)a1;
			break;
		
		case EDisplayHalWsSwitchOnScreen:
			WsSwitchOnScreen();
			break;

		case EDisplayHalMaxDisplayContrast:
			{
			TInt mc=KConfigLcdMaxDisplayContrast;
			kumemput32(a1,&mc,sizeof(mc));
			break;
			}
		case EDisplayHalSetDisplayContrast:
			__KTRACE_OPT(KEXTENSION,Kern::Printf("EDisplayHalSetDisplayContrast"));
			if(!Kern::CurrentThreadHasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetDisplayContrast")))
				return KErrPermissionDenied;
			r=SetContrast(TInt(a1));
			break;
		
		case EDisplayHalDisplayContrast:
			kumemput32(a1,&iContrast,sizeof(iContrast));
			break;

		case EDisplayHalMaxDisplayBrightness:
			{
			TInt mc=KConfigLcdMaxDisplayBrightness;
			kumemput32(a1,&mc,sizeof(mc));
			break;
			}
		
		case EDisplayHalSetDisplayBrightness:
			__KTRACE_OPT(KEXTENSION,Kern::Printf("EDisplayHalSetDisplayBrightness"));
			if(!Kern::CurrentThreadHasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetDisplayBrightness")))
				return KErrPermissionDenied;
			r=SetBrightness(TInt(a1));
			break;
		
		case EDisplayHalDisplayBrightness:
			kumemput32(a1,&iBrightness,sizeof(iBrightness));
			break;
		
		case EDisplayHalSetBacklightOn:
			if(!Kern::CurrentThreadHasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetBacklightOn")))
				return KErrPermissionDenied;
			if (Kern::MachinePowerStatus()<ELow)
				r=KErrBadPower;
			else
				SetBacklightState(TBool(a1));
			break;
		
		case EDisplayHalBacklightOn:
			kumemput32(a1,&iBacklightOn,sizeof(TInt));
			break;

		case EDisplayHalModeCount:
			{
			TInt ndm = KConfigLcdNumberOfDisplayModes;
			kumemput32(a1, &ndm, sizeof(ndm));
			break;
			}
		
		case EDisplayHalSetMode:
			if(!Kern::CurrentThreadHasCapability(ECapabilityMultimediaDD,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetMode")))
				return KErrPermissionDenied;
			r = SetDisplayMode((TInt)a1);
			break;
		
		case EDisplayHalMode:
			kumemput32(a1, &iVideoInfo.iDisplayMode, sizeof(iVideoInfo.iDisplayMode));
			break;

		case EDisplayHalSetPaletteEntry:
			if(!Kern::CurrentThreadHasCapability(ECapabilityMultimediaDD,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetPaletteEntry")))
				return KErrPermissionDenied;
			r = SetPaletteEntry((TInt)a1, (TInt)a2);
			break;
		
		case EDisplayHalPaletteEntry:
			{
			TInt entry;
			kumemget32(&entry, a1, sizeof(TInt));
			TInt x;
			r = GetPaletteEntry(entry, &x);
			if (r == KErrNone)
				kumemput32(a2, &x, sizeof(x));
			break;
			}
		
		case EDisplayHalSetState:
			{
			if(!Kern::CurrentThreadHasCapability(ECapabilityPowerMgmt,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetState")))
				return KErrPermissionDenied;
			if ((TBool)a1)
				{
				WsSwitchOnScreen();
				}
			else
				{
				WsSwitchOffScreen();
				}
			break;
			}

		case EDisplayHalState:
			kumemput32(a1, &iDisplayOn, sizeof(TBool));
			break;

		case EDisplayHalColors:
			{
			TInt mdc = KConfigLcdMaxDisplayColors;
			kumemput32(a1, &mdc, sizeof(mdc));
			break;
			}

		case EDisplayHalCurrentModeInfo:
			{
			TPckgBuf<TVideoInfoV01> vPckg;
			r = GetCurrentDisplayModeInfo(vPckg(), (TBool)a2);
			if (KErrNone == r)
				Kern::InfoCopy(*(TDes8*)a1,vPckg);
			}
			break;

		case EDisplayHalSpecifiedModeInfo:
			{
			TPckgBuf<TVideoInfoV01> vPckg;
			TInt mode;
			kumemget32(&mode, a1, sizeof(mode));
			r = GetSpecifiedDisplayModeInfo(mode, vPckg());
			if (KErrNone == r)
				Kern::InfoCopy(*(TDes8*)a2,vPckg);
			}
			break;
			
		case EDisplayHalSecure:
			kumemput32(a1, &iSecureDisplay, sizeof(TBool));
			break;

		case EDisplayHalSetSecure:
			{
			if(!Kern::CurrentThreadHasCapability(ECapabilityMultimediaDD,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetSecure")))
				return KErrPermissionDenied;
			SwitchDisplay((TBool)a1);
			}
			break;

		default:
			r=KErrNotSupported;
			break;
		}
	return r;
	}


#ifdef ENABLE_GCE_MODE
DLcdPowerHandler* DLcdPowerHandler::pLcd = NULL;

void DLcdPowerHandler::ChangeFrameBufferAddress(TUint32 aFbAddr)
	{
	//TODO: this is guess work
	//find out the correct sequence to change LCD DMA address
	//
	const TInt8 DISPC_GODIGITAL_BITSHIFT = 6;
	const TInt8 DISPC_GOLCD_BITSHIFT = 5;
	const TUint32 goFlags = (1 << DISPC_GODIGITAL_BITSHIFT) | (1 << DISPC_GOLCD_BITSHIFT);

	const TUint32 ctl = GET_REGISTER(DISPC_CONTROL);
	SET_REGISTER(DISPC_GFX_BA1, aFbAddr);
	SET_REGISTER(DISPC_CONTROL, ctl | goFlags);
	}

#include <display.h>

class DDisplayPddBeagle : public DDisplayPdd
	{
public:
	DDisplayPddBeagle();
	~DDisplayPddBeagle();
    virtual TInt  SetLegacyMode();
    virtual TInt  SetGceMode();
    virtual TInt  SetRotation(RDisplayChannel::TDisplayRotation aRotation);
	virtual TInt  PostUserBuffer(TBufferNode* aNode);
	virtual TInt  PostCompositionBuffer(TBufferNode* aNode);
    virtual TInt  PostLegacyBuffer();
    virtual TInt  CloseMsg();
    virtual TInt  CreateChannelSetup(TInt aUnit);
   	virtual TBool  PostPending();
    virtual TDfcQue* DfcQ(TInt  aUnit);    
            
public:
	static void VSyncDfcFn(TAny* aChannel);

private:
	TDfcQue* 			iDfcQ;
	TVideoInfoV01    	iScreenInfo;
    TBufferNode*     	iPendingBuffer;
    TBufferNode*     	iActiveBuffer;
    DChunk* 		 	iChunk;

public:
	TDfc 		     	iVSyncDfc;
	};

class DDisplayPddFactory : public DPhysicalDevice
	{
public:
	DDisplayPddFactory();

	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DBase*& aChannel, TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);
	virtual TInt Validate(TInt aDeviceType, const TDesC8* anInfo, const TVersion& aVer);
	};

const TInt KVSyncDfcPriority = 7 ;   //priority of DFC within the queue (0 to 7, where 7 is highest)

DDisplayPddBeagle::DDisplayPddBeagle():
	iPendingBuffer(NULL),
	iActiveBuffer(NULL),
	iChunk(NULL),
	iVSyncDfc(&VSyncDfcFn, this, KVSyncDfcPriority)
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::DDisplayPddBeagle"));

	iPostFlag = EFalse;
	}

DDisplayPddBeagle::~DDisplayPddBeagle()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::~DDisplayPddBeagle()"));

	//The DFC Queue is owned by DLcdPowerHandler so we shouldn't call Destroy() at this point.
	if (iDfcQ)
		{
		iDfcQ=NULL;
		}

	DChunk* chunk = (DChunk*) __e32_atomic_swp_ord_ptr(&iChunk, 0);

	if(chunk)
		{
		Kern::ChunkClose(chunk);
		}

	}

TInt DDisplayPddBeagle::SetLegacyMode()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::SetLegacyMode()"));
    return KErrNone;
	}

TInt DDisplayPddBeagle::SetGceMode()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::SetGceMode()"));
    PostCompositionBuffer(&iLdd->iCompositionBuffer[0]);
    return KErrNone;
	}

TInt DDisplayPddBeagle::SetRotation(RDisplayChannel::TDisplayRotation aDegOfRot)
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::SetRotation()"));
	return KErrNone;
	}

TInt DDisplayPddBeagle::PostUserBuffer(TBufferNode* aNode)
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::PostUserBuffer :  aNode->iAddress = %08x\n", aNode->iAddress));
	if(iPendingBuffer)
		{
		iPendingBuffer->iState = EBufferFree;
		if (!(iPendingBuffer->iType == EBufferTypeUser) )
			{
			iPendingBuffer->iFree  = ETrue;
			}
		}
	aNode->iState   = EBufferPending;
	iPendingBuffer	= aNode;
	iPostFlag		= ETrue;
	
  	// Activate the posted buffer
	TUint32 physicalAddress = Epoc::LinearToPhysical( aNode->iAddress );
	DLcdPowerHandler::pLcd->ChangeFrameBufferAddress(physicalAddress);

	/* Queue a DFC to complete the request*/
	iVSyncDfc.Enque();

	return KErrNone;
	}

TInt DDisplayPddBeagle::PostCompositionBuffer(TBufferNode* aNode)
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::PostCompositionBuffer :  aNode->iAddress = %08x\n", aNode->iAddress));

	if(iPendingBuffer)
		{
		iPendingBuffer->iState = EBufferFree;
		if (iPendingBuffer->iType == EBufferTypeUser)
			{
			RequestComplete(RDisplayChannel::EReqPostUserBuffer, KErrCancel);
			}
		else
			{
			iPendingBuffer->iFree  = ETrue;
			}
		}

	aNode->iState	= EBufferPending;
	aNode->iFree	= EFalse;
	iPendingBuffer	= aNode;
	iPostFlag		= ETrue;

  	// Activate the posted buffer
	TUint32 physicalAddress = Epoc::LinearToPhysical( aNode->iAddress );
	DLcdPowerHandler::pLcd->ChangeFrameBufferAddress(physicalAddress);
	
	/* Queue a DFC to complete the request*/
	iVSyncDfc.Enque();

	return KErrNone;
	}

TInt DDisplayPddBeagle::PostLegacyBuffer()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::PostLegacyBuffer()"));

	if(iPendingBuffer)
		{
		iPendingBuffer->iState = EBufferFree;
		if (iPendingBuffer->iType == EBufferTypeUser)
			{

			RequestComplete(RDisplayChannel::EReqPostUserBuffer, KErrCancel);
			}
		else
			{
			iPendingBuffer->iFree  = ETrue;
			}
		}

	iLdd->iLegacyBuffer[0].iState		= EBufferPending;
	iLdd->iLegacyBuffer[0].iFree		= EFalse;
	iPendingBuffer						= &iLdd->iLegacyBuffer[0];
	iPostFlag		= ETrue;

  	// Activate the posted buffer
	DLcdPowerHandler::pLcd->ChangeFrameBufferAddress(DLcdPowerHandler::pLcd->ivRamPhys);

	/* Queue a DFC to complete the request*/
	iVSyncDfc.Enque();

	return KErrNone;
	}

TInt DDisplayPddBeagle::CloseMsg()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::CloseMsg()"));

	iPendingBuffer  = NULL;
	iActiveBuffer	= NULL;
	iVSyncDfc.Cancel();
    return KErrNone;
	}

TInt DDisplayPddBeagle::CreateChannelSetup(TInt aUnit)
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::CreateChannelSetup()"));

	iScreenInfo = DLcdPowerHandler::pLcd->iVideoInfo;
	iLdd->iUnit = aUnit;

	iLdd->iDisplayInfo.iAvailableRotations			= RDisplayChannel::ERotationNormal;
	iLdd->iDisplayInfo.iNormal.iOffsetBetweenLines	= iScreenInfo.iOffsetBetweenLines;
	iLdd->iDisplayInfo.iNormal.iHeight				= iScreenInfo.iSizeInPixels.iHeight;
	iLdd->iDisplayInfo.iNormal.iWidth				= iScreenInfo.iSizeInPixels.iWidth;
	iLdd->iDisplayInfo.iNumCompositionBuffers		= KDisplayCBMax;
	iLdd->iDisplayInfo.iBitsPerPixel				= iScreenInfo.iBitsPerPixel;
    iLdd->iDisplayInfo.iRefreshRateHz = 60;


	switch (iScreenInfo.iBitsPerPixel)
		{
		case 16:
			iLdd->iDisplayInfo.iPixelFormat = EUidPixelFormatRGB_565;
			break;
		case 24:
			iLdd->iDisplayInfo.iPixelFormat = EUidPixelFormatRGB_888;
			break;
		case 32:
			iLdd->iDisplayInfo.iPixelFormat = EUidPixelFormatXRGB_8888;
			break;
		default:
			iLdd->iDisplayInfo.iPixelFormat = EUidPixelFormatUnknown;
			break;
		}

	iLdd->iCurrentRotation = RDisplayChannel::ERotationNormal;

	// Open shared chunk to the composition framebuffer

	DChunk* chunk = 0;
	TLinAddr chunkKernelAddr  = 0;
	TUint32 chunkMapAttr = 0;

	// round to twice the page size
	TUint round  =  2*Kern::RoundToPageSize(DLcdPowerHandler::pLcd->iSize);

	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::CreateChannelSetup DLcdPowerHandler::pLcd->iSize  = %d\n", DLcdPowerHandler::pLcd->iSize));

	TChunkCreateInfo info;
	info.iType					 = TChunkCreateInfo::ESharedKernelMultiple;
	info.iMaxSize				 = round;
	info.iMapAttr				 = EMapAttrFullyBlocking;
	info.iOwnsMemory			 = EFalse;
	info.iDestroyedDfc			 = 0;

	TInt r = Kern::ChunkCreate(info, chunk, chunkKernelAddr, chunkMapAttr);

	__KTRACE_OPT(KEXTENSION, Kern::Printf("CreateChannelSetup:ChunkCreate called for composition chunk. Set iChunkKernelAddr  = %08x\n", chunkKernelAddr));

	if( r == KErrNone)
		{
		// map our chunk
		r = Kern::ChunkCommitPhysical(chunk, 0,round , DLcdPowerHandler::pLcd->iCompositionPhysical);
		__KTRACE_OPT(KEXTENSION, Kern::Printf("Mapping chunk %d", r));
		if(r != KErrNone)
			{
			Kern::ChunkClose(chunk);
			}
		}

	if ( r!= KErrNone)
		{
		return r;
		}

	iChunk	= chunk;

	// init CB 0
	iLdd->iCompositionBuffer[0].iType			= EBufferTypeComposition;
	iLdd->iCompositionBuffer[0].iBufferId		= 0;
	iLdd->iCompositionBuffer[0].iFree			= ETrue;
	iLdd->iCompositionBuffer[0].iState			= EBufferFree;
	iLdd->iCompositionBuffer[0].iAddress		= chunkKernelAddr;
	iLdd->iCompositionBuffer[0].iPhysicalAddress = Epoc::LinearToPhysical(chunkKernelAddr);
	iLdd->iCompositionBuffer[0].iChunk			= chunk;
	iLdd->iCompositionBuffer[0].iHandle			= 0;
	iLdd->iCompositionBuffer[0].iOffset			= 0;
	iLdd->iCompositionBuffer[0].iSize			= DLcdPowerHandler::pLcd->iSize;
	iLdd->iCompositionBuffer[0].iPendingRequest = 0;

	// init CB 1
	iLdd->iCompositionBuffer[1].iType			= EBufferTypeComposition;
	iLdd->iCompositionBuffer[1].iBufferId		= 1;
	iLdd->iCompositionBuffer[1].iFree			= ETrue;
	iLdd->iCompositionBuffer[1].iState			= EBufferFree;
	iLdd->iCompositionBuffer[1].iAddress		= chunkKernelAddr + DLcdPowerHandler::pLcd->iSize;
	iLdd->iCompositionBuffer[1].iPhysicalAddress = Epoc::LinearToPhysical(chunkKernelAddr + DLcdPowerHandler::pLcd->iSize);
	iLdd->iCompositionBuffer[1].iChunk			= chunk;
	iLdd->iCompositionBuffer[1].iHandle			= 0;
	iLdd->iCompositionBuffer[1].iOffset			= DLcdPowerHandler::pLcd->iSize;
	iLdd->iCompositionBuffer[1].iSize			= DLcdPowerHandler::pLcd->iSize;
	iLdd->iCompositionBuffer[1].iPendingRequest = 0;

	iLdd->iCompositionBuffIdx					= 0;
	//Use the same DFC queue created by the DLcdPowerHandler so all hardware accesses are executed under the same DFC thread.
	iDfcQ= DLcdPowerHandler::pLcd->iDfcQ;

	// Set the Post DFC.
	iVSyncDfc.SetDfcQ(iDfcQ);


	return KErrNone;
	}

TBool DDisplayPddBeagle::PostPending()
	{
	return (iPendingBuffer != NULL);
	}

TDfcQue * DDisplayPddBeagle::DfcQ(TInt aUnit)
	{
	return iDfcQ;
	}

void DDisplayPddBeagle::VSyncDfcFn(TAny* aChannel)
	{
	DDisplayPddBeagle * channel =(DDisplayPddBeagle*)aChannel;

	if (channel->iPostFlag)
		{
		 channel->iPostFlag = EFalse;

		if (channel->iActiveBuffer)
			{
			//When a User buffer is registered its iFree member becomes EFalse and Deregister sets it
			//back to ETrue. Composition and Legacy buffers are not free when they are in the pending or
			//active state.
			if (channel->iActiveBuffer->iType == EBufferTypeUser)
				{
				channel->RequestComplete(RDisplayChannel::EReqPostUserBuffer, KErrNone);
				}
			else
				{
				channel->iActiveBuffer->iFree	= ETrue;
				}

			channel->iActiveBuffer->iState		= EBufferFree;


			//If no buffer was available during a call to GetCompositionBuffer the active buffer has
			//been returned as the next available one, so we must set the buffer to the proper state before we
			//send the notification.
			TInt pendingIndex = channel->iLdd->iPendingIndex[RDisplayChannel::EReqGetCompositionBuffer];
			if(channel->iLdd->iPendingReq[RDisplayChannel::EReqGetCompositionBuffer][pendingIndex].iTClientReq)
				{
				if(channel->iLdd->iPendingReq[RDisplayChannel::EReqGetCompositionBuffer][pendingIndex].iTClientReq->IsReady())
					{
					channel->iActiveBuffer->iState = EBufferCompose;
				    channel->RequestComplete(RDisplayChannel::EReqGetCompositionBuffer,KErrNone);
				    }
			    }

			channel->iActiveBuffer				= NULL;
			}

		if (channel->iPendingBuffer)
			{
			__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddBeagle::VSyncDfcFn moving pending buffer at address %08x to the active state\n", channel->iPendingBuffer->iAddress));
			channel->iActiveBuffer			= channel->iPendingBuffer;
			channel->iActiveBuffer->iState	= EBufferActive;
			channel->iPendingBuffer			= NULL;

			channel->RequestComplete(RDisplayChannel::EReqWaitForPost,  KErrNone);
			}
		}
	}

DDisplayPddFactory::DDisplayPddFactory()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddFactory::DDisplayPddFactory()"));

	iVersion		= TVersion(KDisplayChMajorVersionNumber,
                      KDisplayChMinorVersionNumber,
                      KDisplayChBuildVersionNumber);
	}

TInt DDisplayPddFactory::Create(DBase*& aChannel, TInt aUnit, const TDesC8* aInfo, const TVersion& aVer)
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddFactory::Create()"));

	DDisplayPddBeagle *device= new DDisplayPddBeagle() ;
	aChannel=device;
	if (!device)
		{
		return KErrNoMemory;
		}
	return KErrNone;
	}

TInt DDisplayPddFactory::Install()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddFactory::Install()"));

	TBuf<32> name(RDisplayChannel::Name());
	_LIT(KPddExtension,".pdd");
	name.Append(KPddExtension);
	return SetName(&name);
	}


void DDisplayPddFactory::GetCaps(TDes8& /*aDes*/) const
	{
	//Not supported
	}

TInt DDisplayPddFactory::Validate(TInt aUnit, const TDesC8* /*anInfo*/, const TVersion& aVer)
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("DDisplayPddFactory::Validate()"));

	if (!Kern::QueryVersionSupported(iVersion,aVer))
		{
		return KErrNotSupported;
		}

	if (aUnit != 0)
		{
		return KErrNotSupported;
		}

	return KErrNone;
	}

#endif //ENABLE_GCE_MODE

DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("Creating DLcdPowerHandler"));

	TInt r = KErrNoMemory;
	DLcdPowerHandler* pH=new DLcdPowerHandler;
	if (!pH)
		{
		__KTRACE_OPT(KEXTENSION, Kern::Printf("Failed to create DLcdPowerHandler %d", r));
		return r;
		}

	r = pH->Create();
	if (r != KErrNone)
		{
		__KTRACE_OPT(KEXTENSION, Kern::Printf("Failed to create DLcdPowerHandler %d", r));
		return r;
		}

#ifdef ENABLE_GCE_MODE
	__KTRACE_OPT(KEXTENSION, Kern::Printf("Creating DDisplayPddFactory"));
	r = KErrNoMemory;
	DDisplayPddFactory * device = new DDisplayPddFactory;
	if (!device)
		{
		__KTRACE_OPT(KEXTENSION, Kern::Printf("Failed to create DLcdPowerHandler %d", r));
		return r;
		}

	__KTRACE_OPT(KEXTENSION, Kern::Printf("Installing DDisplayPddFactory"));
	r = Kern::InstallPhysicalDevice(device);
	if (r != KErrNone)
		{
		__KTRACE_OPT(KEXTENSION, Kern::Printf("Failed to install DDisplayPddFactory %d", r));
		}
#endif

	return r;
	}

