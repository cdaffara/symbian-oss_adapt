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
* Hardware Configuration Respoitory Platform Specific Layer (PSL) 
*
*/


/** 
@file hcr_psl_config.h
File provides NE1 base port configuration for HCR service. It represents a 
wrapper that brings togther all the ASSP, BSP, Driver, Service setting data
for the compiled repository.

@internalTechnology
*/



// -- HCR INCLUDES ------------------------------------------------------------

#include <hcrconfig.h>

#include "hcr_hai.h"
#include "hcr_uids.h"

using namespace HCR;



// -- HCR BSP SETTING INCLUDES ------------------------------------------------

// Includes for setting published by base port ASSP/SOC interconnections
#include "../../naviengine_assp/hcr/hcr_psl_config_assp_inc.inl"
#include "hcr_psl_config_mha_inc.inl"

// Includes for MHA Hardware Service internal configuration settings
// e.g. #include "../hws/hcr_psl_config_hsw_inc.inl"
#include "../../naviengine_assp/csi/hcr_psl_config_csi_inc.inl"

// Includes for Physical Device Driver internal configuration settings
// e.g. #include "../hws/hcr_psl_config_pdd_inc.inl"



// -- HCR BSP LARGE SETTING DATA (LSD) VALUES ---------------------------------

// Includes for setting published by base port ASSP/SOC interconnections
#include "../../naviengine_assp/hcr/hcr_psl_config_assp_lsd.inl"
#include "hcr_psl_config_mha_lsd.inl"

// Includes for MHA Hardware Service internal configuration settings
// e.g. #include "../hws/hcr_psl_config_hsw_lsd.inl"

// Includes for Physical Device Driver internal configuration settings
// e.g. #include "../pdd/hcr_psl_config_pdd_lsd.inl"




// -- HCR BSP SETTINGS LIST ---------------------------------------------------

SSettingC gSettingsList[] = 
    {
    
// Includes for setting published by base port ASSP/SOC interconnections

// const HCR::TCategoryUid KHcrCat_MHA_HWBASE 	= 0x20029482; 		//< HCR Category for MHA ASSP Hardware Block Base Addresses
// const HCR::TCategoryUid KHcrCat_MHA_DMA 		= 0x20029483; 		//< HCR Category for MHA ASSP DMA EndPoints
#include "../../naviengine_assp/hcr/hcr_psl_config_assp.inl"
,
//const HCR::TCategoryUid KHcrCat_MHA_Interrupt = 0x20029484; 		//< HCR Category for MHA SoC Interrupt Source IDs
//const HCR::TCategoryUid KHcrCat_MHA_GPIO 		= 0x20029485; 		//< HCR Category for MHA SoC GPIO Pin IDs
//const HCR::TCategoryUid KHcrCat_MHA_I2S 		= 0x20029486; 		//< HCR Category for MHA SoC I2S Bus Channels
//const HCR::TCategoryUid KHcrCat_MHA_I2C 		= 0x20029487; 		//< HCR Category for MHA SoC I2C Bus IDs
//const HCR::TCategoryUid KHcrCat_MHA_SPICSI  	= 0x20029488;		//< HCR Category for MHA SoC SPI/CSI Bus Channels
#include "hcr_psl_config_mha.inl"
,

// Includes for MHA Hardware Service internal configuration settings
// e.g. #include "../hws/hcr_psl_config_hsw.inl"
// ,
// Includes for CSI Device Driver internal configuration settings
#include "../../naviengine_assp/csi/hcr_psl_config_csi.inl"
,

// Includes for Physical Device Driver internal configuration settings
// e.g. #include "../pdd/hcr_psl_config_mypdd.inl"
// ,

	HCR_LAST_SETTING
	};
	
	
SRepositoryBase gHeader = 
    {
    HCR_FINGER_PRINT, 
    EReposCompiled, 
    KRepositoryFirstVersion,
    EReposReadOnly,
    HCR_SETTING_COUNT(gSettingsList)
    };


GLDEF_C SRepositoryCompiled gRepository = 
    { 
    &gHeader, 
    gSettingsList 
    };

