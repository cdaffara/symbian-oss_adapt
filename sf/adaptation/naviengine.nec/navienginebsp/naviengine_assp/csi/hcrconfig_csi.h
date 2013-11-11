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
* Hardware Configuration Respoitory Setting IDs header. 
*
*/



/** 
@file hcrconfig_csi.h
File provides definitions for the CSI set of HCR settings
identifiers applicable to the NEC NaviEngine base port.

@internalTechnology
@prototype
*/

#ifndef HCRCONFIG_CSI_H
#define HCRCONFIG_CSI_H



// -- INCLUDES ----------------------------------------------------------------

#include  <drivers/hcr.h>



// -- CATEGORY UIDs -----------------------------------------------------------



// -- KEYS --------------------------------------------------------------------

/**
HCR Setting for CSI Service configuration
*/
#define CSI_BASE 0x00010000
const HCR::TElementId KHcrKey_CSI_NumOfChannels			= CSI_BASE;		//< 
const HCR::TElementId KHcrKey_CSI_Timeout				= CSI_BASE + 1; //< 
const HCR::TElementId KHcrKey_CSI_FifoRxTrigerLvl		= CSI_BASE + 2; //< 
const HCR::TElementId KHcrKey_CSI_FifoTxTrigerLvl		= CSI_BASE + 3; //< 

#undef CSI_BASE

#endif //HCRCONFIG_CSI_H
