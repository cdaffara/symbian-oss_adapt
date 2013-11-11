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
* naviengine_assp\naviengine_lcd.h
*
*/



#ifndef __NAVIENGINE_LCD_H__
#define __NAVIENGINE_LCD_H__
#include <e32const.h>
#include <platform.h>
#include <e32hal.h>
#include <assp.h>
#include <kernel/kern_priv.h>



// Macro to calculate the screen buffer size
// aBpp is the number of bits-per-pixel, aPpl is the number of pixels per line and aLpp number of lines per panel
#define FRAME_BUFFER_SIZE(aBpp,aPpl,aLpp)	(aBpp*aPpl*aLpp)/8	
																

//----------------------------------------------------------------------------
//Display Controller registers
//----------------------------------------------------------------------------

// ------------------------------
// Interface control registers
// ------------------------------

const TUint KHoDisplayInterfaceControl 			= 0x00;			//!!!
const TUint KHwDisplayInterfaceControl 			= KHwDisplayBase + KHoDisplayInterfaceControl;
	//DE		- positive logic
	//EVSYNC	- negative logic
	//VSYNC		- negative logic
	//EHSYNC	- negative logic
	//HSYNC		- negative logic
	//Dot clock normal polarity
	//Switch on VSync, HSync, CSync, DEN
	const TUint KInterfaceControlValue 			= 0x04;
const TUint KHoDisplayInterruptStatus			= 0x08;
const TUint KHwDisplayInterruptStatus			= KHwDisplayBase + KHoDisplayInterruptStatus;
const TUint KHoDisplayInterruptClear 			= 0x10;
const TUint KHwDisplayInterruptClear 			= KHwDisplayBase + KHoDisplayInterruptClear;
const TUint KHoDisplayInterruptEnable 			= 0x18;
const TUint KHwDisplayInterruptEnable 			= KHwDisplayBase + KHoDisplayInterruptEnable;
const TUint KHoDisplayInterruptLineSettings 	= 0x20; 	//???
const TUint KHwDisplayInterruptLineSettings		= KHwDisplayBase + KHoDisplayInterruptLineSettings; 	//???
	// ic_al_h???
const TUint KHoDisplayInterruptEnableSelection	= 0x28;
const TUint KHwDisplayInterruptEnableSelection	= KHwDisplayBase + KHoDisplayInterruptEnableSelection;


// ------------------------------
// Display control registers
// ------------------------------

const TUint KHoDisplayResolutionAndCycle 		= 0x30;		//!!!
const TUint KHwDisplayResolutionAndCycle 		= KHwDisplayBase + KHoDisplayResolutionAndCycle;
	//ds_hr[10:0] - Sets the horizontal (width) resolution. The valid range is 320 (QVGA) to 1280 (WXGA).
	//ds_vr[25:16] - Sets the vertical (height) resolution. The valid range is 240 (QVGA) to 800 (WXGA).
	//ds_th[42:32] - Sets HSYNC in dot clock units. ds_hr + 3 to 7FEh (7FFh is prohibited).
	//ds_tv[57:48] - Sets VSYNC in line units. ds_vr + 3 to 3FEh (3FFh is prohibited).

	#define DISPLAY_RESOLUTION_AND_CYCLE(height, width, linesInFrame, pixelsInLine)	(((height<<16) + width) | ((TUint64) ((linesInFrame<<16) + (pixelsInLine)) <<32))


const TUint KHoDisplayPulseWidth 				= 0x38;		//!!!
const TUint KHwDisplayPulseWidth 				= KHwDisplayBase + KHoDisplayPulseWidth;
	//ds_thp[10:0] - Sets the HSYNC pulse width in dot clock units. Set these bits so that the total blank period is 3 clocks or more.
	//ds_tvp[25:16] - Sets the VSYNC pulse width in line units. Set these bits so that the total blank period is 3H or more.
	//ds_thb[42:32] - Sets the HSYNC back porch width in dot clock units.
	//ds_tvb[57:48] - Sets the VSYNC back porch width in line units.

	//settings for LCD QVGA mode
	//	const TUint KPulseWidthLow 				= (3<<16)+25;  // SW4-2 OFF 
	//	const TUint KPulseWidthHigh				= (3<<16)+38;  // SW4-2 OFF
	const TUint KPulseWidthLow 					= (2<<16)+25;  // SW4-2 ON -on some VGA monitors it doesn't show the whole area
	const TUint KPulseWidthHigh 				= (2<<16)+14;  // SW4-2 ON   
	const TUint64 KPulseWidth 					= KPulseWidthLow | ((TUint64)KPulseWidthHigh <<32);

	//settings for Hitachi LCD VGA mode
	const TUint KPulseWidthLowVgaLCD 			= (2<<16)+30;
	const TUint KPulseWidthHighVgaLCD 			= 0x00020030;
	const TUint64 KPulseWidthVgaLCD 			= KPulseWidthLowVgaLCD | ((TUint64)KPulseWidthHighVgaLCD <<32);	

	//settings for NEC LCD WVGA mode
	const TUint KPulseWidthLowWvga 				= (2<<16)+30;	// RMH - xxx - magic numbers!!!
	const TUint KPulseWidthHighWvga 			= 0x000A0000;	// RMH - xxx - magic numbers!!! 0a seems to be a vertical offset, with 02 being too small and 21 being too large; the last two digits, now 00, were 30 seem to be the horizintal offset...
	const TUint64 KPulseWidthWvga 				= KPulseWidthLowWvga | ((TUint64)KPulseWidthHighWvga <<32);	

	//settings for VGA mode on monitor (if LCD panel is not present)
	const TUint KPulseWidthLowVga 				= (2<<16)+96;
	const TUint KPulseWidthHighVga 				= 0x00210030;
	const TUint64 KPulseWidthVga 				= KPulseWidthLowVga | ((TUint64)KPulseWidthHighVga <<32);

const TUint KHoDisplayDisplaySettings 			= 0x40;	//!!!
const TUint KHwDisplaySettings 					= KHwDisplayBase + KHoDisplayDisplaySettings;
		//ds_oc[0] - Sets on/off of display output to LCD panel;  0 - Off (Black output if sync signal is output); 1 - On
		//gamma correction and dithering
	const TUint KDisplaySettingsValue 			= 0x1;


const TUint KHoDisplayDithering 				= 0x48;
const TUint KHwDisplayDithering 				= KHwDisplayBase + KHoDisplayDithering;
const TUint KHoDisplayBrightness 				= 0x50;		//!!!
const TUint KHwDisplayBrightness 				= KHwDisplayBase + KHoDisplayBrightness;

	//ds_bc[7:0] Sets the brightness coefficient for RGB data. The range is -128 to 127 (8 bits). To turn off brightness control, set to 0.
	const TUint KDisplayBrightnessVal 			= 0x00;		//!!!

// ------------------------------
// Memory Frame control registers
// ------------------------------

const TUint KHoDisplayEndianConversion 			= 0x58;		//!!!
const TUint KHwDisplayEndianConversion 			= KHwDisplayBase + KHoDisplayEndianConversion;

	//mf_es0[1::0] - endian conversion of data in VRAM - should be 00b ???
	//mf_lbs[25:24] = 0 -limits of layers and dimensions
	const TUint KEndianConversionValue 			= 0x00;		//!!!


//mf_pf0[3:0] Sets the pixel format of each memory frame stored in VRAM. 0h: ARGB0565
const TUint KHoDisplayPixelFormat 				= 0x60;			//!!!
const TUint KHwDisplayPixelFormat 				= KHwDisplayBase + KHoDisplayPixelFormat;

const TUint KPixelFormatValue16bpp 				= 0x0;	// ARGB0565
const TUint KPixelFormatValue32bpp 				= 0x4;	// ARGB8888
const TUint KPixelFormatValue					= KPixelFormatValue32bpp; // Retain keyword and default to bpp32


//mf_sa0[31:0]  - Sets the start address of the memory frame stored in VRAM, in byte units.
//mf_sa1[63:32] - Sets the start address of the memory frame stored in VRAM, in byte units.
const TUint KHoDisplayMemoryFrameAddress 		= 0x0070;			//!!!
const TUint KHwDisplayMemoryFrameAddress 		= KHwDisplayBase + KHoDisplayMemoryFrameAddress;


// physical address in VRAM
const TUint KHoDisplayColorPalletAddress 		= 0x90;
const TUint KHwDisplayColorPalletAddress		= KHwDisplayBase + KHoDisplayColorPalletAddress;

	
const TUint KHoDisplayColorPalletDirection 		= 0xA0;
const TUint KHwDisplayColorPalletDirection		= KHwDisplayBase + KHoDisplayColorPalletDirection;

const TUint KHoDisplayMemoryFrameSizeH 			= 0xA8;	//!!!
const TUint KHwDisplayMemoryFrameSizeH 			= KHwDisplayBase + KHoDisplayMemoryFrameSizeH;

	// mf_ws_h0[11:0] - horisontal(width) size in byte units. The valid setting range is 1 to 4,096 at 0 to 4,095.

	// Usually the width value - 1, eg for 640 pixels wide, use 639

const TUint KHoDisplayMemoryFrameSizeV 			= 0xB8;	//!!!
const TUint KHwDisplayMemoryFrameSizeV 			= KHwDisplayBase + KHoDisplayMemoryFrameSizeV;

	// mf_ws_v0[11:0] - vertical(height) size in line units

	// Usually the height value - 1, eg for 480 pixels high, use 479

const TUint KHoDisplayMemoryFrameStartPointX	= 0xc8;	//!!!
const TUint KHwDisplayMemoryFrameStartPointX	= KHwDisplayBase + KHoDisplayMemoryFrameStartPointX;
//mf_sp_x0[11:0] -	Sets the horizontal-direction start point coordinates of the
//					frame memory stored in VRAM, in pixel units

const TUint KHoDisplayMemoryFrameStartPointY	= 0xd8;	//!!!
const TUint KHwDisplayMemoryFrameStartPointY	= KHwDisplayBase + KHoDisplayMemoryFrameStartPointY;
//mf_sp_y0[11:0] -	Sets the vertical-direction start point coordinates of the
//					frame memory stored in VRAM, in line units.

const TUint KHoDisplayMemoryFrameStartFractionV		= 0xe8;
const TUint KHwDisplayMemoryFrameStartFractionV		= KHwDisplayBase + KHoDisplayMemoryFrameStartFractionV;

const TUint KHoDisplayMemoryFrameDisplayFrameSizeH 	= 0xf0;	//!!!
const TUint KHwDisplayMemoryFrameDisplayFrameSizeH	= KHwDisplayBase + KHoDisplayMemoryFrameDisplayFrameSizeH;

	//mf_ds_h0[11:0] - Sets display frame horizontal-direction size in the memory
	//					frame, in pixel units. The valid setting range is 1 to 1,280 at 0 to 1,279.

	// Usually the width value - 1, eg for 640 pixels wide, use 639


const TUint KHoDisplayMemoryFrameDisplayFrameSizeV	= 0x100;	//!!!
const TUint KHwDisplayMemoryFrameDisplayFrameSizeV	= KHwDisplayBase + KHoDisplayMemoryFrameDisplayFrameSizeV;

	//mf_ds_v0[11:0] - Sets display frame vertical-direction size in the memory
	//					frame, in line units. The valid setting range is 1 to 800 at 0 to 799.

	// Usually the height value - 1, eg for 480 pixels high, use 479
	
		

const TUint KHoDisplayBorderColourRGB	= 0x110;	//!!!
const TUint KHwDisplayBorderColourRGB	= KHwDisplayBase + KHoDisplayBorderColourRGB;	//!!!

	//!!!
	//mf_bc0[23:0]	- Sets the border color value. Correspondence of mf_bcX[23:0] is as shown in Figure 2-6-22.
	//mf_bc_s0[28] - Sets the border color format for the corresponding layer.
	//						0: Format set by mf_pf0 (memory frame pixel format)
	//						1: ARGB0888


// ------------------------------
// Display frame control registers	
// ------------------------------

// NOTE: KH*DisplayDisplay is done on purpose because KH*Display is the prefix and the next Display is part of the register name!

const TUint KHoDisplayDisplayFrameControl			= 0x130;	//!!!
const TUint KHwDisplayDisplayFrameControl			= KHwDisplayBase + KHoDisplayDisplayFrameControl;
	//df_dc0[3:0] - Sets whether a layer is to be displayed according to a priority.7 - is off
	//df_vs0[8] - Selects the contents of VRAM or register to be displayed.
	//				0: VRAM
	//				1: Register (Value of constant color (df_cc_r, df_cc_g, and df_cc_b) registers is displayed.)
	//df_lc0[12] - Sets layer on/off. (Stops access to VRAM.) Display can be turned off by df_dc0, but
	// 				access to VRAM is not stopped. To accurately turn off display, use this register

	// Values to set layers 0-3 frame control on and fix their priority.
	const TUint64 KDisplayFrameControlValue 		= (1<<12);
	const TUint64 KDisplayFrame1ControlValue 		= (1<<28) | (0x1<<16);
	const TUint64 KDisplayFrame2ControlValue 		= ((TInt64)1<<44) | ((TInt64)0x2<<32) ;
	const TUint64 KDisplayFrame3ControlValue 		= ((TInt64)1<<60) | ((TInt64)0x3<<48) ;  


const TUint KHoDisplayDisplayFrameStartPointX 		= 0x140;	//!!!
const TUint KHwDisplayDisplayFrameStartPointX 		= KHwDisplayBase + KHoDisplayDisplayFrameStartPointX;
	//df_sp_x0[11:0] - Sets the horizontal-direction start point coordinates to be
	//					displayed, in pixel units. The valid setting range is -2,048 to 2,047.

const TUint KHoDisplayDisplayFrameStartPointY		= 0x150;	//!!!
const TUint KHwDisplayDisplayFrameStartPointY		= KHwDisplayBase + KHoDisplayDisplayFrameStartPointY;
	//df_sp_y0[11:0] - Sets the vertical-direction start point coordinates to be
	//					displayed, in line units. The valid setting range is -2,048 to 2,047.

const TUint KHoDisplayDisplayFrameSizeH 			= 0x160;	//!!!
const TUint KHwDisplayDisplayFrameSizeH 			= KHwDisplayBase + KHoDisplayDisplayFrameSizeH;
	//df_ds_h0[10:0] - Sets the horizontal size to be displayed in pixel units.
	//					The valid setting range is 1 to 1,280 at 0 to 1,279

const TUint KHoDisplayDisplayFrameSizeV 			= 0x170;	//!!!
const TUint KHwDisplayDisplayFrameSizeV 			= KHwDisplayBase + KHoDisplayDisplayFrameSizeV;
	//df_ds_v0[10:0] - Sets the vertical size to be displayed in line units. The valid setting range is 1-800 at 0-799.
	//					Caution Ignored depending on the setting of mf_ms_v (memory frame, magnification select, V0, V2,V4)

const TUint KHoDisplayDisplayFrameExpansionRateH 	= 0x180;
const TUint KHwDisplayDisplayFrameExpansionRateH 	= KHwDisplayBase + KHoDisplayDisplayFrameExpansionRateH;
	// df_rc_h0[12:0] - set to 0

const TUint KHoDisplayDisplayFrameExpansionRateV 	= 0x190;
const TUint KHwDisplayDisplayFrameExpansionRateV 	= KHwDisplayBase + KHoDisplayDisplayFrameExpansionRateV;
	// df_rc_v0[12:0] - set to 0

const TUint KHoDisplayDisplayFramePixelTransmittance			= 0x1a0;
const TUint KHwDisplayDisplayPixelTransmittance					= KHwDisplayBase + KHoDisplayDisplayFramePixelTransmittance;
const TUint KHoDisplayDisplayFrameTranCtrlLayerTransmittance	= 0x1b0;
const TUint KHwDisplayDisplayTranCtrlLayerTransmittance			= KHwDisplayBase + KHoDisplayDisplayFrameTranCtrlLayerTransmittance;
	//Display frames 0 to 6 transmission control layer transmittance

const TUint KHoDisplayDisplayFrameColourKeyControl		= 0x1c0;
const TUint KHwDisplayDisplayColourKeyControl			= KHwDisplayBase + KHoDisplayDisplayFrameColourKeyControl;
	// Display frames color key control and color key R, G, B format instruction

const TUint KHoDisplayDisplayFrameColourKey				= 0x1c8;
const TUint KHwDisplayDisplayColourKey					= KHwDisplayBase + KHoDisplayDisplayFrameColourKey;

const TUint KHoDisplayDisplayFrameColourKeyControlRange = 0x1e8;
const TUint KHwDisplayDisplayColourKeyControlRange		= KHwDisplayBase + KHoDisplayDisplayFrameColourKeyControlRange;
	// Display frame color key control R, G, B permissible range

const TUint KHoDisplayDisplayFrameConstantColour		= 0x1f8;	//!!!
const TUint KHwDisplayDisplayFrameConstantColour		= KHwDisplayBase + KHoDisplayDisplayFrameConstantColour;
	//Sets the constant colours rgb 

const TUint KHoDisplayDisplayFrameBackgroundColour		= 0x218;	//!!!
const TUint KHwDisplayDisplayFrameBackgroundColour		= KHwDisplayBase + KHoDisplayDisplayFrameBackgroundColour;
	// df_bc_r[23:16], df_bc_g[15:8] & df_bc_b[7:0]


// 220 CDE

// ------------------------------
// (5) H/W cursor control register
// ------------------------------

//	...

// ------------------------------
// (6) V blank update control register
// ------------------------------


// If the display supports Contrast and/or Brightness control then supply the following defines:
// This is only example code... you may need to modify it for your hardware
const TInt KConfigInitialDisplayContrast	= 128;
const TInt KConfigLcdMinDisplayContrast		= 1;
const TInt KConfigLcdMaxDisplayContrast		= 255;
const TInt KConfigInitialDisplayBrightness	= 128;
const TInt KConfigLcdMinDisplayBrightness	= 1;
const TInt KConfigLcdMaxDisplayBrightness	= 255;

#endif //__NAVIENGINE_LCD_H__
