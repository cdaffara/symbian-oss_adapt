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
identifiers applicable to the NEC NaviEngine base port ASSP part.

@publishedPartner
@prototype
*/

#ifndef HCRCONFIG_ASSP_H
#define HCRCONFIG_ASSP_H




// -- INCLUDES ----------------------------------------------------------------

#include  <drivers/hcr.h>



// -- CATEGORY ----------------------------------------------------------------



// -- KEYS --------------------------------------------------------------------


/**
HCR Setting Keys for ASSP Hardware Block base virtual addresses for FMM and MMM. 
*/
#define HW_BASE 0x00001000
const HCR::TElementId KHcrKey_HwBase_Uart0				= HW_BASE +  11; //< Serial port #0
const HCR::TElementId KHcrKey_HwBase_Uart1				= HW_BASE +  12; //< Serial port #1
const HCR::TElementId KHcrKey_HwBase_Uart2				= HW_BASE +  13; //< Serial port #2

const HCR::TElementId KHcrKey_HwBase_MPCorePrivate		= HW_BASE +  21; //< 4KB of private MPcore region for SCU and Interrupt controller
const HCR::TElementId KHcrKey_HwBase_SCU				= HW_BASE +  22; //< Snoop Control Unit
const HCR::TElementId KHcrKey_HwBase_IntIf				= HW_BASE +  23; //< CPU interrupt interface
const HCR::TElementId KHcrKey_HwBase_IntDist			= HW_BASE +  24; //< Global Interrupt Distributer

const HCR::TElementId KHcrKey_HwBase_Timers				= HW_BASE +  31; //< Six SoC timers
const HCR::TElementId KHcrKey_HwBase_Watchdog			= HW_BASE +  32; //< eWDT (Watchdog Timer)
const HCR::TElementId KHcrKey_HwBase_SystemCtrl			= HW_BASE +  33; //< System control unit

const HCR::TElementId KHcrKey_HwBase_Display			= HW_BASE +  41; //< SoC display controller

const HCR::TElementId KHcrKey_HwBase_I2C				= HW_BASE +  51; //< I2C
const HCR::TElementId KHcrKey_HwBase_I2S0				= HW_BASE +  52; //< I2S0
const HCR::TElementId KHcrKey_HwBase_I2S1				= HW_BASE +  53; //< I2S1
const HCR::TElementId KHcrKey_HwBase_I2S2				= HW_BASE +  54; //< I2S2
const HCR::TElementId KHcrKey_HwBase_I2S3				= HW_BASE +  55; //< I2S3

const HCR::TElementId KHcrKey_HwBase_FPGA				= HW_BASE +  61; //< FPGA registers
const HCR::TElementId KHcrKey_HwBase_CDDisp				= HW_BASE +  62; //< LCD display
const HCR::TElementId KHcrKey_HwBase_TSP				= HW_BASE +  63; //< Digitiser

const HCR::TElementId KHcrKey_HwBase_SPDIF				= HW_BASE +  64; //< SPDIF

const HCR::TElementId KHcrKey_HwBase_CSI0				= HW_BASE +  71; //< CSI0
const HCR::TElementId KHcrKey_HwBase_CSI1				= HW_BASE +  72; //< CSI1

const HCR::TElementId KHcrKey_HwBase_SDCtrl				= HW_BASE +  81; //< SD

const HCR::TElementId KHcrKey_HwBase_KDMACExBus			= HW_BASE +  91; //< DMAC(4C) 4KB
const HCR::TElementId KHcrKey_HwBase_KDMAC32			= HW_BASE +  92; //< DMAC(8C) 20KB

const HCR::TElementId KHcrKey_HwBase_Ethernet			= HW_BASE + 101; //< Ethernet0

const HCR::TElementId KHcrKey_HwBase_GPIOBase			= HW_BASE + 111; //< GPIO

const HCR::TElementId KHcrKey_HwBase_PciBase			= HW_BASE + 121; //< // PCI Bridges
const HCR::TElementId KHcrKey_HwBase_PciBridgeExtern	= HW_BASE + 122; //< External PCI Bridge
const HCR::TElementId KHcrKey_HwBase_PciBridgeUsb		= HW_BASE + 123; //< UsbHost dedicated PCI Bridge

const HCR::TElementId KHcrKey_HwBase_UsbHWindow			= HW_BASE + 131; //< Internal PCI window
#undef HW_BASE


/**
HCR Settings holding the DMA logical channel identifiers. 
*/
#define DMA_CH 0x00002000
const HCR::TElementId KHcrKey_DmaCh_SD0				= DMA_CH +  11; //<
const HCR::TElementId KHcrKey_DmaCh_SD1				= DMA_CH +  12; //<
	
const HCR::TElementId KHcrKey_DmaCh_I2S0RX			= DMA_CH +  21; //<
const HCR::TElementId KHcrKey_DmaCh_I2S0TX			= DMA_CH +  22; //<
const HCR::TElementId KHcrKey_DmaCh_I2S1RX			= DMA_CH +  23; //<
const HCR::TElementId KHcrKey_DmaCh_I2S1TX			= DMA_CH +  24; //<
const HCR::TElementId KHcrKey_DmaCh_I2S2RX			= DMA_CH +  25; //<
const HCR::TElementId KHcrKey_DmaCh_I2S2TX			= DMA_CH +  26; //<
const HCR::TElementId KHcrKey_DmaCh_I2S3RX			= DMA_CH +  27; //<
const HCR::TElementId KHcrKey_DmaCh_I2S3TX			= DMA_CH +  28; //<
	
const HCR::TElementId KHcrKey_DmaCh_UART0RX			= DMA_CH +  41; //<
const HCR::TElementId KHcrKey_DmaCh_UART0TX			= DMA_CH +  42; //<
const HCR::TElementId KHcrKey_DmaCh_UART1RX			= DMA_CH +  43; //<
const HCR::TElementId KHcrKey_DmaCh_UART1TX			= DMA_CH +  44; //<
const HCR::TElementId KHcrKey_DmaCh_UART2RX			= DMA_CH +  45; //<
const HCR::TElementId KHcrKey_DmaCh_UART2TX			= DMA_CH +  46; //<

const HCR::TElementId KHcrKey_DmaCh_MemToMem0		= DMA_CH +  61; //<
const HCR::TElementId KHcrKey_DmaCh_MemToMem1		= DMA_CH +  62; //<
const HCR::TElementId KHcrKey_DmaCh_MemToMem2		= DMA_CH +  63; //<
const HCR::TElementId KHcrKey_DmaCh_MemToMem3		= DMA_CH +  64; //<
#undef DMA_CH

/**
HCR Settings holding the memory map physical addresses identifiers. 
*/
#define HW_PHYS 0x00003000
const HCR::TElementId KHcrKey_HwPhys_BaseMPcorePrivate		= HW_PHYS +  11; //<
const HCR::TElementId KHcrKey_HwPhys_DDR2RamBase			= HW_PHYS +  12; //<
const HCR::TElementId KHcrKey_HwPhys_PCI					= HW_PHYS +  13; //<
const HCR::TElementId KHcrKey_HwPhys_Internal				= HW_PHYS +  14; //<

const HCR::TElementId KHcrKey_HwPhys_AXI64IC2				= HW_PHYS +  21; //<
const HCR::TElementId KHcrKey_HwPhys_SATA					= HW_PHYS +  22; //<
const HCR::TElementId KHcrKey_HwPhys_AXI64DMAC				= HW_PHYS +  23; //<
const HCR::TElementId KHcrKey_HwPhys_Video					= HW_PHYS +  24; //<
const HCR::TElementId KHcrKey_HwPhys_Disp					= HW_PHYS +  25; //<
const HCR::TElementId KHcrKey_HwPhys_SGX					= HW_PHYS +  26; //<

const HCR::TElementId KHcrKey_HwPhys_CFWindow2				= HW_PHYS +  31; //<
const HCR::TElementId KHcrKey_HwPhys_CFWindow1				= HW_PHYS +  32; //<
const HCR::TElementId KHcrKey_HwPhys_CFWindow0				= HW_PHYS +  33; //<
const HCR::TElementId KHcrKey_HwPhys_ATA6CS1				= HW_PHYS +  34; //<
const HCR::TElementId KHcrKey_HwPhys_ATA6CS0				= HW_PHYS +  35; //<
const HCR::TElementId KHcrKey_HwPhys_USBH					= HW_PHYS +  36; //<
const HCR::TElementId KHcrKey_HwPhys_AHB32PCIUSB			= HW_PHYS +  37; //<
const HCR::TElementId KHcrKey_HwPhys_AHB32PCIExt			= HW_PHYS +  38; //<
const HCR::TElementId KHcrKey_HwPhys_DDR2Reg				= HW_PHYS +  39; //<
const HCR::TElementId KHcrKey_HwPhys_AHB0DMAC4				= HW_PHYS +  40; //<
const HCR::TElementId KHcrKey_HwPhys_AHB0DMAC3				= HW_PHYS +  41; //<
const HCR::TElementId KHcrKey_HwPhys_AHB0DMAC2				= HW_PHYS +  42; //<
const HCR::TElementId KHcrKey_HwPhys_AHB0DMAC1				= HW_PHYS +  43; //<
const HCR::TElementId KHcrKey_HwPhys_AHB0DMAC0				= HW_PHYS +  44; //<
const HCR::TElementId KHcrKey_HwPhys_AHBEXDMAC				= HW_PHYS +  45; //<
const HCR::TElementId KHcrKey_HwPhys_EXBUS					= HW_PHYS +  46; //<

const HCR::TElementId KHcrKey_HwPhys_DTVIf					= HW_PHYS +  50; //<
const HCR::TElementId KHcrKey_HwPhys_APB1					= HW_PHYS +  51; //<

const HCR::TElementId KHcrKey_HwPhys_PWM7					= HW_PHYS +  60; //<
const HCR::TElementId KHcrKey_HwPhys_PWM6					= HW_PHYS +  61; //<
const HCR::TElementId KHcrKey_HwPhys_PWM5					= HW_PHYS +  62; //<
const HCR::TElementId KHcrKey_HwPhys_PWM4					= HW_PHYS +  63; //<
const HCR::TElementId KHcrKey_HwPhys_PWM3					= HW_PHYS +  64; //<
const HCR::TElementId KHcrKey_HwPhys_PWM2					= HW_PHYS +  65; //<
const HCR::TElementId KHcrKey_HwPhys_PWM1					= HW_PHYS +  66; //<
const HCR::TElementId KHcrKey_HwPhys_PWM0					= HW_PHYS +  67; //<
const HCR::TElementId KHcrKey_HwPhys_GPIO					= HW_PHYS +  68; //<
const HCR::TElementId KHcrKey_HwPhys_SYSCTRL				= HW_PHYS +  69; //<
const HCR::TElementId KHcrKey_HwPhys_eWDT					= HW_PHYS +  70; //<
const HCR::TElementId KHcrKey_HwPhys_Timer5					= HW_PHYS +  71; //<
const HCR::TElementId KHcrKey_HwPhys_Timer4					= HW_PHYS +  72; //<
const HCR::TElementId KHcrKey_HwPhys_Timer3					= HW_PHYS +  73; //<
const HCR::TElementId KHcrKey_HwPhys_Timer2					= HW_PHYS +  74; //<
const HCR::TElementId KHcrKey_HwPhys_Timer1					= HW_PHYS +  75; //<
const HCR::TElementId KHcrKey_HwPhys_Timer0					= HW_PHYS +  76; //<
const HCR::TElementId KHcrKey_HwPhys_UART7					= HW_PHYS +  77; //<
const HCR::TElementId KHcrKey_HwPhys_UART6					= HW_PHYS +  78; //<
const HCR::TElementId KHcrKey_HwPhys_UART5					= HW_PHYS +  79; //<
const HCR::TElementId KHcrKey_HwPhys_UART4					= HW_PHYS +  80; //<
const HCR::TElementId KHcrKey_HwPhys_UART3					= HW_PHYS +  81; //<
const HCR::TElementId KHcrKey_HwPhys_UART2					= HW_PHYS +  82; //<
const HCR::TElementId KHcrKey_HwPhys_UART1					= HW_PHYS +  83; //<
const HCR::TElementId KHcrKey_HwPhys_UART0					= HW_PHYS +  84; //<
const HCR::TElementId KHcrKey_HwPhys_I2S3					= HW_PHYS +  85; //<
const HCR::TElementId KHcrKey_HwPhys_I2S2					= HW_PHYS +  86; //<
const HCR::TElementId KHcrKey_HwPhys_I2S1					= HW_PHYS +  87; //<
const HCR::TElementId KHcrKey_HwPhys_I2S0					= HW_PHYS +  88; //<
const HCR::TElementId KHcrKey_HwPhys_I2C					= HW_PHYS +  89; //<
const HCR::TElementId KHcrKey_HwPhys_CSI1					= HW_PHYS +  90; //<
const HCR::TElementId KHcrKey_HwPhys_CSI0					= HW_PHYS +  91; //<
const HCR::TElementId KHcrKey_HwPhys_SPDIF					= HW_PHYS +  92; //<
const HCR::TElementId KHcrKey_HwPhys_SD						= HW_PHYS +  93; //<

const HCR::TElementId KHcrKey_HwPhys_FPGA					= HW_PHYS +  100; //<


const HCR::TElementId KHcrKey_HwPhys_LAN					= HW_PHYS +  110; //<
#undef HW_PHYS

#endif // HCRCONFIG_ASSP_H
