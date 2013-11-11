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
* ne1_tb\specific\powerresources.h
*
*/



#ifndef __POWERRESOURCES_H__
#define __POWERRESOURCES_H__

/** Enumeration for static resource Id */
enum ResourceId
	{
	/** Resource Id for I2S 0 MCLK Resource */
	ENE1_TBI2S0MclkResource = 1,
    /** Resource Id for I2S 1 MCLK Resource */
	ENE1_TBI2S1MclkResource = 2,
	/** Resource Id for I2S 2 MCLK Resource */
	ENE1_TBI2S2MclkResource = 3,
	/** Resource Id for  I2S 3 MCLK Resource */
	ENE1_TBI2S3MclkResource = 4,
	/** Resource Id for I2S 0 SCLK Resource */
	ENE1_TBI2S0SclkResource = 5,
	/** Resource Id for I2S 1 SCLK Resource */
	ENE1_TBI2S1SclkResource = 6,
	/** Resource Id for I2S 2 SCLK Resource */
	ENE1_TBI2S2SclkResource	= 7,
	/** Resource Id for I2S 3 SCLK Resource */
	ENE1_TBI2S3SclkResource = 8,
	/** Resource Id for CSI 0 clock Resource */
	ENE1_TBCSI0ClockResource = 9,
	/** Resource Id for CSI 1 clock Resource */
	ENE1_TBCSI1ClockResource = 10,
	/** Resource Id for Display DCLK Resource */
	ENE1_TBDisplayDclkResource = 11,
	/** Resource Id for LCD Resource */
	ENE1_TBLcdResource = 12,
	/** Resource Id for Board Power Resource */
	ENE1_TBBoardPowerResource = 13,
	/** Resource Id for PCI Clock enable resource */
	ENE1_TBPCIClkResource = 14,
	/** Add any new resource above this */
	EMaxResourceCount
	};


/** Resource name for board power.*/
_LIT(KBoardPower, "NE1_TBBoardPower");

/** Resource name for LCD Power. */
_LIT(KLcdPower, "NE1_TBLcdPower");

/** Resource name for PCI clock enable resource */
_LIT(KPCIClk, "NE1_TBPCIClk");

/** Enumeration for binary resource 
	Possible values for board power and lcd power resource.
*/
enum EBinaryResPower
	{
	/** Power OFF */
	E_OFF,
	/** Power ON */
	E_ON
	};

/** Resource name for display Dclk */
_LIT(KDisplayDclk, "NE1_TBDisplayDclk");

/** Enumeration of possible programmable divider for display DCLK */
enum TDisplayDclk
	{
	/** 399MHz divide by 20 */
	EDisplayDclk19950KHz = 0,
	/** 399MHz divide by 19 */
	EDisplayDclk21000KHz,
	/** 399MHz divide by 18 */
	EDisplayDclk22166KHz,
	/** 399MHz divide by 17 */
	EDisplayDclk23470KHz, 
	/** 399MHz divide by 16 */
	EDisplayDclk24937KHz,
	/** 399MHz divide by 15 */
	EDisplayDclk26600KHz,
	/** 399MHz divide by 14 */
	EDisplayDclk28500KHz,
	/** 399MHz divide by 13 */
	EDisplayDclk30692KHz,
	/** 399MHz divide by 12 */
	EDisplayDclk33250KHz,
	/** 399MHz divide by 11 */
	EDisplayDclk36272KHz,
	/** 399MHz divide by 10 */
	EDisplayDclk39900KHz,
	/** 399MHz divide by 9 */
	EDisplayDclk44333KHz,
	/** 399MHz divide by 8 */
	EDisplayDclk49875KHz,
	/** 399MHz divide by 7 */
	EDisplayDclk57000KHz,
	/** 399MHz divide by 6 */
	EDisplayDclk66500KHz,
	/** 399MHz divide by 5 */
	EDisplayDclk79800KHz
	};

/** Resource name for CSI 0 clock */
_LIT(KCSI0Clock, "NE1_TBCSI0Clock");		

/** Resource name for CSI 1 clock */
_LIT(KCSI1Clock, "NE1_TBCSI1Clock");

/** Enumeration of possible frequency for CSI clock */
enum TCSIClock
	{
	/** SCLK1 (slave mode) */
	ECSIClkSck1 = 0,
	/** 1/512 PCLK frequency for CSI clock (master mode) */
	ECSIClk130KHz,
	/** 1/256 PCLK frequency for CSI clock (master mode) */
	ECSIClk260KHz,
	/** 1/128 PCLK frequency for CSI clock (master mode) */
	ECSIClk521KHz,
	/** 1/64 PCLK frequency for CSI clock (master mode) */
	ECSIClk1040KHz,
	/** 1/32 PCLK frequency for CSI clock (master mode) */
	ECSIClk2080KHz,
	/** 1/16 PCLK frequency for CSI clock (master mode) */
	ECSIClk4170KHz,
	/** 1/4 PCLK frequency for CSI clock (master mode) */
	ECSIClk16670KHz
	};

/** Resource name for I2S0 MCLK */
_LIT(KI2S0Mclk, "NE1_TBI2S0Mclk");
/** Resource name for I2S1 MCLK */
_LIT(KI2S1Mclk, "NE1_TBI2S1Mclk");
/** Resource name for I2S2 MCLK */
_LIT(KI2S2Mclk, "NE1_TBI2S2Mclk");
/** Resource name for I2S3 MCLK */
_LIT(KI2S3Mclk, "NE1_TBI2S3Mclk");

/** Enumeration of possible frequency for I2S MCLK */
enum TI2SMclk 
	{
	/** Enable MCLK masking */
	EI2SMclkMask = -1,
	/** 36.864MHz MCLK frequency */
	EI2SMclk36864KHz = 0, 
	/** 24.576MHz MCLK frequency */
	EI2SMclk24576KHz = 1, 
	/** 18.432MHz MCLK frequency */
	EI2SMclk18432KHz = 2,
	/** 33.8688MHz MCLK frequency */
	EI2SMclk33868KHz = 4,
	/** 22.5792MHz MCLK frequency */
	EI2SMclk22579KHz = 5,
	/** 16.9344MHz MCLK frequency */
	EI2SMclk16934KHz = 6
	};

/** Resource name for I2S0 SCLK */
_LIT(KI2S0Sclk, "NE1_TBI2S0Sclk");
/** Resource name for I2S1 SCLK */
_LIT(KI2S1Sclk, "NE1_TBI2S1Sclk");
/** Resource name for I2S2 SCLK */
_LIT(KI2S2Sclk, "NE1_TBI2S2Sclk");
/** Resource name for I2S3 SCLK */
_LIT(KI2S3Sclk, "NE1_TBI2S3Sclk");

/** Enumeration of possible frequency for I2S SCLK */
enum TI2SSclk 
	{
	/** 8KHz Sampling frequency in master mode */
	EI2SSclk8000Hz = 0,
	/** 12KHz Sampling frequency in master mode */
	EI2SSclk12000Hz = 1,
	/** 16KHz Sampling frequency in master mode */
	EI2SSclk16000Hz = 2,
	/** 24KHz Sampling frequency in master mode */
	EI2SSclk24000Hz = 3,
	/** 32KHz Sampling frequency in master mode */
	EI2SSclk32000Hz = 4,
	/** 48KHz Sampling frequency in master mode */
	EI2SSclk48000Hz = 5,
	/** 11.025KHz Sampling frequency in master mode */
	EI2SSclk11025Hz = 8,
	/** 22.05KHz Sampling frequency in master mode */
	EI2SSclk22050Hz = 9,
	/** 44.1KHz Sampling frequency in master mode */
    EI2SSclk44100Hz = 10
	};

#endif //__POWERRESOURCES_H__

