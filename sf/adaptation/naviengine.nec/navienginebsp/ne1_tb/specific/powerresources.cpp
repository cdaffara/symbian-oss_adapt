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
* ne1_tb\specific\powerresources.cpp
*
*/



#include "resmanpsl.h"
#include <naviengine_priv.h>

/** Constructor for board power resource */
DNE1_TBBoardPowerResource::DNE1_TBBoardPowerResource():DStaticPowerResource(KBoardPower, E_ON)
	{
	iFlags = 0; //Binary synchronous single user positive sense resource
	}

/** This function updates the resource information for board power resource. Call default implementation to update
	generic information about the resource. */
TInt DNE1_TBBoardPowerResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = E_OFF;
	buf1->iMaxLevel = E_ON;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for board power resource.
	This is called from PIL on response to change or read resource state. 
	*/
TInt DNE1_TBBoardPowerResource::DoRequest(TPowerRequest& aRequest)
	{
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		aRequest.Level() = E_ON; //should be ON
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		//Change to level specified. If the state is OFF then board will be switched off 
		AsspRegister::Write32(KHwFPGABase+KHoSystemPowerDown, aRequest.Level());
		}
	else
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for display DCLK resource */
DNE1_TBDisplayDclkResource::DNE1_TBDisplayDclkResource():DStaticPowerResource(KDisplayDclk, EDisplayDclk66500KHz)
	{
	iFlags = EMultilevel; //Multilevel instantaneous single user positive sense resource
	}

/** This function updates the resource information for display DCLK resource. Call default implementation to 
	update generic information about the resource. */
TInt DNE1_TBDisplayDclkResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = EDisplayDclk19950KHz;
	buf1->iMaxLevel = EDisplayDclk79800KHz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read /write) for display DCLK. 
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBDisplayDclkResource::DoRequest(TPowerRequest& aRequest)
	{
	TInt level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUDisplayDCLKCtrl);
		level = ~(level & 0xF); //Need to convert from divider setting to frequency
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set the default level 
		AsspRegister::Write32(KHwSystemCtrlBase+KHoSCUDisplayDCLKCtrl, ~iDefaultLevel);
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		//Select internal clock 
		AsspRegister::Write32(KHwSystemCtrlBase+KHoSCUDisplayDCLKCtrl, ~aRequest.Level());
		}
	else 
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for LCD power resource */
DNE1_TBLcdResource::DNE1_TBLcdResource():DStaticPowerResource(KLcdPower, E_ON)
	{
	iFlags = 0; //Binary instantaneous single user positive sense resource
	}

/** This function updates the resource information for LCD power resource. Call default implementation to 
	update generic information about the resource. */
TInt DNE1_TBLcdResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = E_OFF;
	buf1->iMaxLevel = E_ON;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for LCD resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBLcdResource::DoRequest(TPowerRequest& aRequest)
	{
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		aRequest.Level() = AsspRegister::Read32(KHwFPGABase+KHoLCDControl) & 0x1;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		AsspRegister::Modify32(KHwFPGABase+KHoLCDControl, KClearBit0, iDefaultLevel);
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		//Set to requested level
		AsspRegister::Modify32(KHwFPGABase+KHoLCDControl, KClearBit0, aRequest.Level());
		}
	else 
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for CSI 0 clock resource */
DNE1_TBCSI0ClockResource::DNE1_TBCSI0ClockResource():DStaticPowerResource(KCSI0Clock, ECSIClkSck1)
	{
	iFlags = EMultilevel; //Mulitlevel instantaneous single user positive sense resource
	}

/** This function updates the resource information for CSI 0 clock resource. Call default implementation to 
	update generic information about the resource.
	*/
TInt DNE1_TBCSI0ClockResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = ECSIClkSck1;
	buf1->iMaxLevel = ECSIClk16670KHz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read /write) for CSI 0 clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBCSI0ClockResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwBaseCSI0+KHoCSIClockSelect);
		aRequest.Level() = ~(level & 0x7); //Convert from divider to frequency
		return KErrNone;
		}
	//Check whether CSIRST is 1, if not return KErrNotsupported
	level = AsspRegister::Read32(KHwBaseCSI0+KHoCSIModeControl);
	if(level & CSI_UNIT_ENABLE)
		{
		return KErrNotSupported;
		}
	//Check whether CSIE is 0 , if not return KErrNotSupported
	level = AsspRegister::Read32(KHwBaseCSI0+KHoCSIControl);
	if(!(level & CSI_RESET))
		{
		return KErrNotSupported;
		}
	if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwBaseCSI0+KHoCSIClockSelect, KClearBit0_2, ~iDefaultLevel);
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		AsspRegister::Modify32(KHwBaseCSI0+KHoCSIClockSelect, KClearBit0_2, ~aRequest.Level());
		}
	else 
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for CSI 1 clock resource */
DNE1_TBCSI1ClockResource::DNE1_TBCSI1ClockResource():DStaticPowerResource(KCSI1Clock, ECSIClkSck1)
	{
	iFlags = EMultilevel; //Multilevel instantaneous single user positive sense resource
	}

/** This function updates the resource information for CSI 1 clock resource. Call default implementation to 
	update generic information about the resource. 
	*/
TInt DNE1_TBCSI1ClockResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = ECSIClkSck1;
	buf1->iMaxLevel = ECSIClk16670KHz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read/write) for CSI 1 clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBCSI1ClockResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwBaseCSI1+KHoCSIClockSelect);
		level = ~(level & 0x7);
		return KErrNone;
		}
	//Check whether CSIRST is 1, if not return KErrNotsupported
	level = AsspRegister::Read32(KHwBaseCSI1+KHoCSIModeControl);
	if(level & CSI_UNIT_ENABLE)
		{
		return KErrNotSupported;
		}	
	//Check whether CSIE is 0 , if not return KErrNotSupported
	level = AsspRegister::Read32(KHwBaseCSI1+KHoCSIControl);
	if(!(level & CSI_RESET))
		{
		return KErrNotSupported;
		}
	if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwBaseCSI1+KHoCSIClockSelect, KClearBit0_2, ~iDefaultLevel);
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		//Change the resource to the requested level
		AsspRegister::Modify32(KHwBaseCSI1+KHoCSIClockSelect, KClearBit0_2, ~aRequest.Level());
		}
	else
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for I2S 0 MCLK divider resource */
DNE1_TBI2S0MclkResource::DNE1_TBI2S0MclkResource():DStaticPowerResource(KI2S0Mclk, EI2SMclk36864KHz)
	{
	//This resource also takes care of masking the DCLK clock and therefore classified as multiproperty
	iFlags = EMultiProperty; 
	}

/** This function updates the resource information for I2S 0 clock resource. Call default implementation to
	update generic information about the resource
	*/
TInt DNE1_TBI2S0MclkResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = EI2SMclkMask;
	buf1->iMaxLevel = EI2SMclk16934KHz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for I2S 0 MCLK resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBI2S0MclkResource::DoRequest(TPowerRequest& aRequest)
	{
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		aRequest.Level() = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl) & 0xF;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit0_3, iDefaultLevel);
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		if(aRequest.Level() == EI2SMclkMask) //Enable DCLK Mask
			{
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUClockMaskCtrl, KClearBit18, KSetBit18);
			}
		else
			{
			//Set the request frequency
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit0_3, aRequest.Level());
			//Disable the clock mask
			TUint level = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl);
			if(!(level & KSetBit18))
				{
				AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit18, KClearBit18);
				}
			}
		}
	else 
		{
		return KErrNotSupported;
		}	
	return KErrNone;
	}

/** Constructor for I2S 1 MCLK divider resource */
DNE1_TBI2S1MclkResource::DNE1_TBI2S1MclkResource():DStaticPowerResource(KI2S1Mclk, EI2SMclk36864KHz)
	{
	iFlags = EMultiProperty;
	}

/** This function updates the resource information for I2S 1 clock resource. Call default implementation to
	update generic information about the resource
	*/
TInt DNE1_TBI2S1MclkResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = EI2SMclkMask;
	buf1->iMaxLevel = EI2SMclk16934KHz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for I2S 1 MCLK clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBI2S1MclkResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl);
		aRequest.Level() = (level >> SHIFT_BY_8) & 0xF;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit8_11, (iDefaultLevel << SHIFT_BY_8));
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		if(aRequest.Level() == EI2SMclkMask) //Enable DCLK Mask
			{
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUClockMaskCtrl, KClearBit19, KSetBit19);
			}
		else
			{
			//Set the request frequency
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit8_11, (aRequest.Level() << SHIFT_BY_8));
			//Disable the clock mask
			level = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl);
			if(!(level & KSetBit19))
				{
				AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit19, KClearBit19);
				}
			}
		}
	else 
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for I2S 2 MCLK divider resource */
DNE1_TBI2S2MclkResource::DNE1_TBI2S2MclkResource():DStaticPowerResource(KI2S2Mclk, EI2SMclk36864KHz)
	{
	iFlags = EMultiProperty;
	}

/** This function updates the resource information for I2S 2 MCLK clock resource. Call default implementation to
	update generic information about the resource
	*/
TInt DNE1_TBI2S2MclkResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = EI2SMclkMask;
	buf1->iMaxLevel = EI2SMclk16934KHz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for I2S 2 MCLK clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBI2S2MclkResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl);
		aRequest.Level() = (level >> SHIFT_BY_16) & 0xF;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit16_19, (iDefaultLevel << SHIFT_BY_16));
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		if(aRequest.Level() == EI2SMclkMask) //Enable DCLK Mask
			{
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUClockMaskCtrl, KClearBit20, KSetBit20);
			}
		else
			{
			//Set the request frequency
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit16_19, 
																		(aRequest.Level() << SHIFT_BY_16));
			//Disable the clock mask
			level = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl);
			if(!(level & KSetBit20))
				{
				AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit20, KClearBit20);
				}
			}
		}
	else 
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for I2S 3 MCLK divider resource */
DNE1_TBI2S3MclkResource::DNE1_TBI2S3MclkResource():DStaticPowerResource(KI2S3Mclk, EI2SMclk36864KHz)
	{
	iFlags = EMultiProperty;
	}

/** This function updates the resource information for I2S 3 MCLK clock resource. Call default implementation to
	update generic information about the resource
	*/
TInt DNE1_TBI2S3MclkResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = EI2SMclkMask;
	buf1->iMaxLevel = EI2SMclk16934KHz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for I2S 3 MCLK clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBI2S3MclkResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl);
		aRequest.Level() = (level >> SHIFT_BY_24) & 0xF;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit24_27, (iDefaultLevel << SHIFT_BY_24));
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		if(aRequest.Level() == EI2SMclkMask) //Enable DCLK Mask
			{
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUClockMaskCtrl, KClearBit21, KSetBit21);
			}
		else
			{
			//Set the request frequency
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit24_27, 
																			(aRequest.Level() << SHIFT_BY_24));
			//Disable the clock mask
			level = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl);
			if(!(level & KSetBit21))
				{
				AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit21, KClearBit21);
				}
			}
		}
	else 
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for I2S 0 SCLK resource */
DNE1_TBI2S0SclkResource::DNE1_TBI2S0SclkResource():DStaticPowerResource(KI2S0Sclk, EI2SSclk8000Hz)
	{
	iFlags = EMultilevel;
	}

/** This function updates the resource information for I2S 0 SCLK clock resource. Call default implementation to
	update generic information about the resource
	*/
TInt DNE1_TBI2S0SclkResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = EI2SSclk8000Hz;
	buf1->iMaxLevel = EI2SSclk44100Hz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for I2S 0 SCLK clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBI2S0SclkResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwBaseI2S0+KHoI2SCtrl);
		aRequest.Level() = (level >> SHIFT_BY_16) & 0xF;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwBaseI2S0+KHoI2SCtrl, KClearBit16_19, (iDefaultLevel << SHIFT_BY_16));
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		//Set to request frequency
		AsspRegister::Modify32(KHwBaseI2S0+KHoI2SCtrl, KClearBit16_19, (aRequest.Level() << SHIFT_BY_16));
		}
	else
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for I2S 1 SCLK resource */
DNE1_TBI2S1SclkResource::DNE1_TBI2S1SclkResource():DStaticPowerResource(KI2S1Sclk, EI2SSclk8000Hz)
	{
	iFlags = EMultilevel;
	}

/** This function updates the resource information for I2S 1 SCLK clock resource. Call default implementation to
	update generic information about the resource
	*/
TInt DNE1_TBI2S1SclkResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = EI2SSclk8000Hz;
	buf1->iMaxLevel = EI2SSclk44100Hz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for I2S 1 SCLK clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBI2S1SclkResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwBaseI2S1+KHoI2SCtrl);
		aRequest.Level() = (level >> SHIFT_BY_16) & 0xF;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwBaseI2S1+KHoI2SCtrl, KClearBit16_19, (iDefaultLevel << SHIFT_BY_16));
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		//Set to request frequency
		AsspRegister::Modify32(KHwBaseI2S1+KHoI2SCtrl, KClearBit16_19, (aRequest.Level() << SHIFT_BY_16));
		}
	else
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for I2S 2 SCLK resource */
DNE1_TBI2S2SclkResource::DNE1_TBI2S2SclkResource():DStaticPowerResource(KI2S2Sclk, EI2SSclk8000Hz)
	{
	iFlags = EMultilevel;
	}

/** This function updates the resource information for I2S 2 SCLK clock resource. Call default implementation to
	update generic information about the resource
	*/
TInt DNE1_TBI2S2SclkResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = EI2SSclk8000Hz;
	buf1->iMaxLevel = EI2SSclk44100Hz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for I2S 2 SCLK clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBI2S2SclkResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwBaseI2S2+KHoI2SCtrl);
		aRequest.Level() = (level >> SHIFT_BY_16) & 0xF;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwBaseI2S2+KHoI2SCtrl, KClearBit16_19, (iDefaultLevel << SHIFT_BY_16));
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		//Set to request frequency
		AsspRegister::Modify32(KHwBaseI2S2+KHoI2SCtrl, KClearBit16_19, (aRequest.Level() << SHIFT_BY_16));
		}
	else
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for I2S 3 SCLK resource */
DNE1_TBI2S3SclkResource::DNE1_TBI2S3SclkResource():DStaticPowerResource(KI2S3Sclk, EI2SSclk8000Hz)
	{
	iFlags = EMultilevel;
	}

/** This function updates the resource information for I2S 3 SCLK clock resource. Call default implementation to
	update generic information about the resource
	*/
TInt DNE1_TBI2S3SclkResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = EI2SSclk8000Hz;
	buf1->iMaxLevel = EI2SSclk44100Hz;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for I2S 3 SCLK clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBI2S3SclkResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwBaseI2S3+KHoI2SCtrl);
		aRequest.Level() = (level >> SHIFT_BY_16) & 0xF;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwBaseI2S3+KHoI2SCtrl, KClearBit16_19, (iDefaultLevel << SHIFT_BY_16));
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		//Set to request frequency
		AsspRegister::Modify32(KHwBaseI2S3+KHoI2SCtrl, KClearBit16_19, (aRequest.Level() << SHIFT_BY_16));
		}
	else
		{	
		return KErrNotSupported;
		}
	return KErrNone;
	}

/** Constructor for PCI Clock resource */
DNE1_TBPCIClockResource::DNE1_TBPCIClockResource():DStaticPowerResource(KPCIClk, E_OFF)
	{
    iFlags = 0;
	}

/** This function updates the resource information for PCI mask clock resource. Call default implementation to
	update generic information about the resource
	*/
TInt DNE1_TBPCIClockResource::GetInfo(TDes8* aInfo)const
	{
	DStaticPowerResource::GetInfo((TDes8*)aInfo);
	TPowerResourceInfoV01 *buf1 = (TPowerResourceInfoV01*)aInfo;
	buf1->iMinLevel = E_OFF;
	buf1->iMaxLevel = E_ON;
	return KErrNone;
	}

/** This function takes care of the requested resource operation (read / write) for PCI clock resource.
	This is called from PIL on response to change or read resource state.
	*/
TInt DNE1_TBPCIClockResource::DoRequest(TPowerRequest& aRequest)
	{
	TUint level;
	if(aRequest.ReqType() == TPowerRequest::EGet)
		{
		level = AsspRegister::Read32(KHwSystemCtrlBase+KHoSCUClockMaskCtrl);
		aRequest.Level() = (level >> SHIFT_BY_9) & 0x1;
		}
	else if(aRequest.ReqType() == TPowerRequest::ESetDefaultLevel)
		{
		//Set to default level
		AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUDivideI2SCLKCtrl, KClearBit09, KSetBit09);
		aRequest.Level() = iDefaultLevel;
		}
	else if(aRequest.ReqType() == TPowerRequest::EChange)
		{
		if(aRequest.Level() == E_OFF)
			{//Mask the clock
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUClockMaskCtrl, KClearBit09, KSetBit09);
			}
		else
			{//Enable the clock
			AsspRegister::Modify32(KHwSystemCtrlBase+KHoSCUClockMaskCtrl, KClearBit09, KClearBit09);
			}
		}
	else
		{
		return KErrNotSupported;
		}
	return KErrNone;
	}







