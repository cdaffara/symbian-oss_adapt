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
* This file is part of the NE1_TB Variant Base Port
* Hardware Configuration Respoitory Published Setting IDs header. 
*
*/



/** 
@file hcrconfig_assp.h
File provides const uint definitions for the published set of HCR settings
identifiers applicable to the NEC NaviEngine base port.
The settings defined in the HCR are examples for "device configuration" for
the platform and not an exhustive list. The HCR is not intended to hold
"feature configuration" which if varied would required significant code 
changes in the hardware consumer component. 

===============================================================
 ____            _        _                    
|  _ \ _ __ ___ | |_ ___ | |_ _   _ _ __   ___ 
| |_) | '__/ _ \| __/ _ \| __| | | | '_ \ / _ \
|  __/| | | (_) | || (_) | |_| |_| | |_) |  __/
|_|   |_|  \___/ \__\___/ \__|\__, | .__/ \___|
                              |___/|_|         

This API and component are in an early release form. As such
this component, it's API/HAI interfaces and internal design 
are not fixed and may be updated/changed at any time before 
final release.

===============================================================

@publishedPartner
@prototype
*/

#ifndef HCR_CONFIG_H
#define HCR_CONFIG_H



// -- INCLUDES ----------------------------------------------------------------

#include  <drivers/hcr.h>



// -- CATEGORY UIDs -----------------------------------------------------------

const HCR::TCategoryUid KHcrCat_MHA_Base 	= 0x20029482;			//< Lowest UID in SymSign allocated UID range for HCR MHA Settings
const HCR::TCategoryUid KHcrCat_MHA_Max 	= 0x20029491;			//< Highest UID in SymSign allocated UID range for HCR MHA Settings

const HCR::TCategoryUid KHcrCat_MHA_HWBASE 	= 0x20029482; 			//< HCR Category for MHA ASSP Hardware Block Base Addresses
const HCR::TCategoryUid KHcrCat_MHA_DMA 	= 0x20029483; 			//< HCR Category for MHA ASSP DMA EndPoints
const HCR::TCategoryUid KHcrCat_MHA_Interrupt = 0x20029484; 		//< HCR Category for MHA SoC Interrupt Source IDs
const HCR::TCategoryUid KHcrCat_MHA_GPIO 	= 0x20029485; 			//< HCR Category for MHA SoC GPIO Pin IDs
const HCR::TCategoryUid KHcrCat_MHA_I2S 	= 0x20029486; 			//< HCR Category for MHA SoC I2S Bus Channels
const HCR::TCategoryUid KHcrCat_MHA_I2C 	= 0x20029487; 			//< HCR Category for MHA SoC I2C Bus IDs
const HCR::TCategoryUid KHcrCat_MHA_SPICSI  = 0x20029488; 			//< HCR Category for MHA SoC SPI/CSI Bus Channels



const HCR::TCategoryUid KHcrCat_HWServ_Base = 0x20029D1D;			//< Lowest UID in SymSign allocated UID range for HCR Hardware Service provider configuration
const HCR::TCategoryUid KHcrCat_HWServ_Max 	= 0x20029D24;			//< Highest UID in SymSign allocated UID range for HCR Hardware Service provider configuration

const HCR::TCategoryUid KHcrCat_HWServ_CSI 	= 0x20029D1D;			//< Private HCR UID allocated for CSI configuration
	
	
	
const HCR::TCategoryUid KHcrCat_DevDrvier_Base 	= 0x20029D25;		//< Lowest UID in SymSign allocated UID range for HCR NE1
const HCR::TCategoryUid KHcrCat_DevDriver_Max 	= 0x20029D2C;		//< Highest UID in SymSign allocated UID range for HCR NE1


	
// -- ASSP SETTING KEYS -------------------------------------------------------

#include <assp/naviengine/hcrconfig_assp.h>




// -- MHA SETTING KEYS --------------------------------------------------------

#include <ne1_tb/hcrconfig_mha.h>



#endif // HCR_CONFIG_H




