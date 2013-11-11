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
File provides definitions for the published set of HCR settings
identifiers applicable to the MHA Services on the NEC NaviEngine 
base port.

@publishedPartner
@prototype
*/

#ifndef HCRCONFIG_MHA_H
#define HCRCONFIG_MHA_H



// -- INCLUDES ----------------------------------------------------------------

#include  <drivers/hcr.h>



// -- CATEGORY UIDs -----------------------------------------------------------



// -- KEYS --------------------------------------------------------------------

/**
HCR Settings identifing the assignment of Interrrupt lines. 

These element keys are used along with the KHcrCat_MHA_Interrupt category UID
to identify and read the HCR setting.
*/
#define INT_SRC 0x00010000
const HCR::TElementId KHcrKey_Interrupt_ExBus			= INT_SRC +  11; //< 

const HCR::TElementId KHcrKey_Interrupt_I2C				= INT_SRC +  12; //<
 
const HCR::TElementId KHcrKey_Interrupt_CSI0			= INT_SRC +  13; //< 
const HCR::TElementId KHcrKey_Interrupt_CSI1			= INT_SRC +  14; //< 

const HCR::TElementId KHcrKey_Interrupt_Timer0			= INT_SRC +  21; //< SoC Timer0 interrupt
const HCR::TElementId KHcrKey_Interrupt_Timer1			= INT_SRC +  22; //< SoC Timer1 interrupt
const HCR::TElementId KHcrKey_Interrupt_Timer2			= INT_SRC +  23; //< SoC Timer2 interrupt
const HCR::TElementId KHcrKey_Interrupt_Timer3			= INT_SRC +  24; //< SoC Timer3 interrupt
const HCR::TElementId KHcrKey_Interrupt_Timer4			= INT_SRC +  25; //< SoC Timer4 interrupt
const HCR::TElementId KHcrKey_Interrupt_Timer5			= INT_SRC +  26; //< SoC Timer5 interrupt

const HCR::TElementId KHcrKey_Interrupt_PWM 			= INT_SRC +  31; //<
 
const HCR::TElementId KHcrKey_Interrupt_SD0				= INT_SRC +  32; //< SD #0 : OXMNIRQ
const HCR::TElementId KHcrKey_Interrupt_SD1				= INT_SRC +  33; //< SD #1 : OXASIOIRQ //SDIO

const HCR::TElementId KHcrKey_Interrupt_CF				= INT_SRC +  41; //< 
const HCR::TElementId KHcrKey_Interrupt_NAND			= INT_SRC +  42; //<  Nand Controller
const HCR::TElementId KHcrKey_Interrupt_MIF				= INT_SRC +  43; //< 
const HCR::TElementId KHcrKey_Interrupt_DTV				= INT_SRC +  44; //< 
const HCR::TElementId KHcrKey_Interrupt_SGX				= INT_SRC +  45; //<
 
const HCR::TElementId KHcrKey_Interrupt_DISP0 			= INT_SRC +  51; //< DISP 0
const HCR::TElementId KHcrKey_Interrupt_DISP1			= INT_SRC +  52; //< 
const HCR::TElementId KHcrKey_Interrupt_DISP2			= INT_SRC +  53; //< 

const HCR::TElementId KHcrKey_Interrupt_Video			= INT_SRC +  61; //< 
const HCR::TElementId KHcrKey_Interrupt_SPDIF0			= INT_SRC +  62; //< 
const HCR::TElementId KHcrKey_Interrupt_SPDIF1			= INT_SRC +  63; //< 

const HCR::TElementId KHcrKey_Interrupt_I2S0			= INT_SRC +  71; //< I2S 0 Interrupts
const HCR::TElementId KHcrKey_Interrupt_I2S1   		  	= INT_SRC +  72; //< 
const HCR::TElementId KHcrKey_Interrupt_I2S2			= INT_SRC +  73; //< 
const HCR::TElementId KHcrKey_Interrupt_I2S3			= INT_SRC +  74; //<

const HCR::TElementId KHcrKey_Interrupt_APB				= INT_SRC +  81; //< 
const HCR::TElementId KHcrKey_Interrupt_AHB0			= INT_SRC +  82; //< 
const HCR::TElementId KHcrKey_Interrupt_AHB1			= INT_SRC +  83; //< 
const HCR::TElementId KHcrKey_Interrupt_AHB2			= INT_SRC +  84; //<

const HCR::TElementId KHcrKey_Interrupt_AXI				= INT_SRC +  91; //<
 
const HCR::TElementId KHcrKey_Interrupt_PCIint			= INT_SRC + 101; //< PCI Int                              
const HCR::TElementId KHcrKey_Interrupt_PCIserrb		= INT_SRC + 102; //< PCI Systerm Error                     
const HCR::TElementId KHcrKey_Interrupt_PCIperrb		= INT_SRC + 103; //< PCI Parity Error                      
const HCR::TElementId KHcrKey_Interrupt_PCIExInt		= INT_SRC + 104; //<                   
const HCR::TElementId KHcrKey_Interrupt_PCIExSerrb		= INT_SRC + 105; //<
const HCR::TElementId KHcrKey_Interrupt_PCIExPerrb		= INT_SRC + 106; //<
                                                                             
const HCR::TElementId KHcrKey_Interrupt_USBHintA 		= INT_SRC + 111; //< USB Host Int A   
const HCR::TElementId KHcrKey_Interrupt_USBHintB		= INT_SRC + 112; //< USB Host Int B       
const HCR::TElementId KHcrKey_Interrupt_USBHsmi			= INT_SRC + 113; //< USB Host System Management Interrupt       
const HCR::TElementId KHcrKey_Interrupt_USBHpme			= INT_SRC + 114; //< USB Host Power Management Event

const HCR::TElementId KHcrKey_Interrupt_ATA6			= INT_SRC + 121; //<
 
const HCR::TElementId KHcrKey_Interrupt_DMAC32_0end		= INT_SRC + 131; //< 
const HCR::TElementId KHcrKey_Interrupt_DMAC32_0err		= INT_SRC + 132; //< 
const HCR::TElementId KHcrKey_Interrupt_DMAC32_1end		= INT_SRC + 133; //< 
const HCR::TElementId KHcrKey_Interrupt_DMAC32_1err		= INT_SRC + 134; //< 
const HCR::TElementId KHcrKey_Interrupt_DMAC32_2end		= INT_SRC + 135; //< 
const HCR::TElementId KHcrKey_Interrupt_DMAC32_2err		= INT_SRC + 136; //< 
const HCR::TElementId KHcrKey_Interrupt_DMAC32_3end		= INT_SRC + 137; //< 
const HCR::TElementId KHcrKey_Interrupt_DMAC32_3err		= INT_SRC + 138; //<
const HCR::TElementId KHcrKey_Interrupt_DMAC32_4end		= INT_SRC + 139; //< 
const HCR::TElementId KHcrKey_Interrupt_DMAC32_4err		= INT_SRC + 140; //<

const HCR::TElementId KHcrKey_Interrupt_UART0			= INT_SRC + 151; //< SoC Uart #0 
const HCR::TElementId KHcrKey_Interrupt_UART1			= INT_SRC + 152; //< SoC Uart #1 
const HCR::TElementId KHcrKey_Interrupt_UART2			= INT_SRC + 153; //< SoC Uart #2 
const HCR::TElementId KHcrKey_Interrupt_UART3			= INT_SRC + 154; //< 
const HCR::TElementId KHcrKey_Interrupt_UART4			= INT_SRC + 155; //< 
const HCR::TElementId KHcrKey_Interrupt_UART5			= INT_SRC + 156; //< 
const HCR::TElementId KHcrKey_Interrupt_UART6			= INT_SRC + 157; //< 
const HCR::TElementId KHcrKey_Interrupt_UART7			= INT_SRC + 158; //<
 
const HCR::TElementId KHcrKey_Interrupt_GPIO			= INT_SRC + 161; //< 
const HCR::TElementId KHcrKey_Interrupt_eWDT			= INT_SRC + 162; //<
const HCR::TElementId KHcrKey_Interrupt_SATA			= INT_SRC + 163; //<
 
const HCR::TElementId KHcrKey_Interrupt_DMACaxi_End		= INT_SRC + 171; //< 
const HCR::TElementId KHcrKey_Interrupt_DMACaxi_Err		= INT_SRC + 172; //<
 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG0  		= INT_SRC + 181; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG1			= INT_SRC + 182; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG2			= INT_SRC + 183; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG3			= INT_SRC + 184; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG4			= INT_SRC + 185; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG5			= INT_SRC + 186; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG6			= INT_SRC + 187; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG7			= INT_SRC + 188; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG8			= INT_SRC + 189; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG9			= INT_SRC + 190; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG10		= INT_SRC + 191; //< 
const HCR::TElementId KHcrKey_Interrupt_PMUIRG11		= INT_SRC + 192; //< 

const HCR::TElementId KHcrKey_Interrupt_COMMRX0			= INT_SRC + 201; //< 
const HCR::TElementId KHcrKey_Interrupt_COMMRX1			= INT_SRC + 202; //< 
const HCR::TElementId KHcrKey_Interrupt_COMMRX2			= INT_SRC + 203; //< 
const HCR::TElementId KHcrKey_Interrupt_COMMRX3			= INT_SRC + 204; //< 
const HCR::TElementId KHcrKey_Interrupt_COMMTX0			= INT_SRC + 205; //< 
const HCR::TElementId KHcrKey_Interrupt_COMMTX1			= INT_SRC + 206; //< 
const HCR::TElementId KHcrKey_Interrupt_COMMTX2			= INT_SRC + 207; //< 
const HCR::TElementId KHcrKey_Interrupt_COMMTX3			= INT_SRC + 208; //<

const HCR::TElementId KHcrKey_Interrupt_PWRCTLO0		= INT_SRC + 211; //< 
const HCR::TElementId KHcrKey_Interrupt_PWRCTLO1		= INT_SRC + 212; //< 
const HCR::TElementId KHcrKey_Interrupt_PWRCTLO2		= INT_SRC + 213; //< 
const HCR::TElementId KHcrKey_Interrupt_PWRCTLO3		= INT_SRC + 214; //< 

const HCR::TElementId KHcrKey_Interrupt_DMACexbus_End	= INT_SRC + 221; //< 
const HCR::TElementId KHcrKey_Interrupt_DMACexbus_Err	= INT_SRC + 222; //< 
const HCR::TElementId KHcrKey_Interrupt_AHBbridge3		= INT_SRC + 223; //< 

#undef INT_SRC


/**
HCR Settings identifing the GPIO pin assignments.

These element keys are used along with the KHcrCat_MHA_GPIO category UID
to identify and read the HCR setting.
*/
#define GPIO_PIN 0x00020000
const HCR::TElementId KHcrKey_GpioPin_UART1_DSR		= GPIO_PIN +  11; //< Serial DSR on RGB board
const HCR::TElementId KHcrKey_GpioPin_UART1_DTR		= GPIO_PIN +  12; //< Serial DTR on RGB board
const HCR::TElementId KHcrKey_GpioPin_UART1_DCD		= GPIO_PIN +  13; //< Serial DCD on RGB board
	
const HCR::TElementId KHcrKey_GpioPin_CKERST		= GPIO_PIN +  21; //< Line to FPGA

const HCR::TElementId KHcrKey_GpioPin_CSI_CS42L51	= GPIO_PIN +  22; //< Line to CS42L51
const HCR::TElementId KHcrKey_GpioPin_CSIa 			= GPIO_PIN +  23; //< Line to Test Pin Header
const HCR::TElementId KHcrKey_GpioPin_CSIb			= GPIO_PIN +  24; //< Line to Test Pin Header
const HCR::TElementId KHcrKey_GpioPin_CSIc			= GPIO_PIN +  25; //< Line to Test Pin Header

const HCR::TElementId KHcrKey_GpioPin_PCIa			= GPIO_PIN +  26; //< ExtBus Connector
const HCR::TElementId KHcrKey_GpioPin_PCIb			= GPIO_PIN +  27; //< ExtBus Connector
const HCR::TElementId KHcrKey_GpioPin_PCIc			= GPIO_PIN +  28; //< ExtBus Connector
	
const HCR::TElementId KHcrKey_GpioPin_RGB			= GPIO_PIN +  41; //< Line to FPGA

const HCR::TElementId KHcrKey_GpioPin_eTRON		 	= GPIO_PIN +  42; //< Line to FPGA

const HCR::TElementId KHcrKey_GpioPin_LCDl			= GPIO_PIN +  43; //< Line to FPGA

const HCR::TElementId KHcrKey_GpioPin_RTC			= GPIO_PIN +  44; //< Line to FPGA

const HCR::TElementId KHcrKey_GpioPin_LINT20		= GPIO_PIN +  45; //< Line to FPGA

const HCR::TElementId KHcrKey_GpioPin_AUDIO_RESET	= GPIO_PIN +  46; //< CS42L51 codec reset line

const HCR::TElementId KHcrKey_GpioPin_SWb0			= GPIO_PIN +  61; //< Line to FPGA
const HCR::TElementId KHcrKey_GpioPin_SWb1			= GPIO_PIN +  62; //< Line to FPGA
#undef GPIO_PIN


/**
HCR Settings identifing the I2S Bus Channels. 

These element keys are used along with the KHcrCat_MHA_I2S category UID
to identify and read the HCR setting.
*/
#define I2S_CHAN 0x00030000
const HCR::TElementId KHcrKey_I2S_CS42L51			= I2S_CHAN +  11; //< Channel used with the Audio Codec
const HCR::TElementId KHcrKey_I2S_Reserved1			= I2S_CHAN +  12; //< 
const HCR::TElementId KHcrKey_I2S_Reserved2			= I2S_CHAN +  13; //< 
const HCR::TElementId KHcrKey_I2S_Reserved3			= I2S_CHAN +  14; //< 
#undef I2S_CHAN


/**
HCR Settings identifing the I2C Bus IDs. 

These element keys are used along with the KHcrCat_MHA_I2C category UID
to identify and read the HCR setting.
*/
#define I2C_BUS 0x00040000
const HCR::TElementId KHcrKey_I2C_ReservedA			= I2C_BUS +  11; //< 
const HCR::TElementId KHcrKey_I2C_ReservedB			= I2C_BUS +  12; //< 
#undef I2S_BUS


/**
HCR Settings identifing the SPI/CSI Bus Channels. 

These element keys are used along with the KHcrCat_MHA_SPI_CSI category UID
to identify and read the HCR setting.
*/
#define CSI_CHAN 0x00050000
const HCR::TElementId KHcrKey_CSI_CS42L51			= CSI_CHAN +  11; //< 
const HCR::TElementId KHcrKey_CSI_ReservedB			= CSI_CHAN +  12; //< 
#undef CSI_CHAN


#endif HCRCONFIG_MHA_H

