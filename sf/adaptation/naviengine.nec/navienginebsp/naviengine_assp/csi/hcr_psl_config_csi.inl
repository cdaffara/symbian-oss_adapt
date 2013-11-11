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
* Hardware Configuration Respoitory compiled repository in-line cpp.
*
*/


/** 
@file hcr_psl_config_csi.inl
File provides setting definitions for the CSI setting values applicable to the 
NEC NaviEngine base port.

@internalTechnology
*/


#ifndef HCR_PSL_CONFIG_CSI_INL
#define HCR_PSL_CONFIG_CSI_INL


// SSettingC gSettingsList[] = 
//     {
//     


/**
HCR Settings values for CSI driver service config. 

*/
    { { { KHcrCat_HWServ_CSI, KHcrKey_CSI_NumOfChannels},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA},		{ {HCR_WVALUE(2)}}},
	{ { { KHcrCat_HWServ_CSI, KHcrKey_CSI_Timeout},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA},		{ {HCR_WVALUE(20)}}},
	{ { { KHcrCat_HWServ_CSI, KHcrKey_CSI_FifoRxTrigerLvl},		ETypeUInt8,	HCR_FLAGS_NONE, HCR_LEN_NA},		{ {HCR_WVALUE(0)}}},
	{ { { KHcrCat_HWServ_CSI, KHcrKey_CSI_FifoTxTrigerLvl},		ETypeUInt8, HCR_FLAGS_NONE, HCR_LEN_NA},		{ {HCR_WVALUE(4)}}}


//
// Last entry must not end in a ',' as it is present in the enclosing file.
// 		};


#endif // HCR_PSL_CONFIG_CSI_INL


