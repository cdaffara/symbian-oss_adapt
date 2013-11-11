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
@file hcr_psl_config_mha.inl
File provides setting definitions for the MHA setting values applicable to the 
NEC NaviEngine base port. These definitions also contain the setting value where
the setting value is no larger than a 32-bit integer. The values for
larger settings can be found in the peer file hcr_psl_config_mha_lsd.inl.

@internalTechnology
*/


#ifndef HCR_PSL_CONFIG_MHA_INL
#define HCR_PSL_CONFIG_MHA_INL


// SSettingC gSettingsList[] = 
//     {
//     


/**
HCR Settings identifing the assignment of Interrrupt lines. 

These element keys are used along with the KHcrCat_MHA_Interrupt category UID
to identify and read the HCR setting.
*/
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_ExBus},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(32)}}},

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_I2C},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(33)}}},
 
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_CSI0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntCsi0)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_CSI1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntCsi1)}}},		

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_Timer0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntIdOstMatchMsTimer)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_Timer1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntId1stMatchMsTimer)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_Timer2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntId2stMatchMsTimer)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_Timer3},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntId3stMatchMsTimer)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_Timer4},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntId4stMatchMsTimer)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_Timer5},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntId5stMatchMsTimer)}}},	

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PWM},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(42)}}},	
 
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_SD0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntSd0)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_SD1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntSd1)}}},		

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_CF},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(45)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_NAND},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntNandCtrl)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_MIF},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(47)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DTV},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(48)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_SGX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(49)}}},		
 
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DISP0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntDisp0)}}}, 	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DISP1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(51)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DISP2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(52)}}},	

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_Video},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(53)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_SPDIF0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(54)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_SPDIF1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(55)}}},	

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_I2S0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntI2S0)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_I2S1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(57)}}}, 	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_I2S2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(58)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_I2S3},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(59)}}},		

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_APB},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(60)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_AHB0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(61)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_AHB1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(62)}}},		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_AHB2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(63)}}},		

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_AXI},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(64)}}},		
 
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PCIint},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntPciInt)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PCIserrb},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntPciSErrB)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PCIperrb},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntPciPErrB)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PCIExInt},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(68)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PCIExSerrb},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(69)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PCIExPerrb},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(70)}}},	
                                                                
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_USBHintA},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntUsbHIntA)}}}, 	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_USBHintB},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntUsbHIntB)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_USBHsmi},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntUsbHSmi)}}},	     
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_USBHpme},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EIntUsbHPme)}}},	

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_ATA6},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(75)}}},		
 
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_0end},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_0_End)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_0err},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_0_End)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_1end},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_1_End)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_1err},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_1_Err)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_2end},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_2_End)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_2err},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_2_Err)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_3end},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_3_End)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_3err},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_3_Err)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_4end},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_4_End)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMAC32_4err},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC32_4_Err)}}},
		
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_UART0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntIdUart0)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_UART1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntIdUart1)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_UART2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntIdUart2)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_UART3},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(89)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_UART4},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(90)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_UART5},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(91)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_UART6},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(92)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_UART7},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(93)}}},	
 
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_GPIO},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntIdGpio)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_eWDT},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC64_End)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_SATA},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAC64_Err)}}},	
 
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMACaxi_End},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(97)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMACaxi_Err},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(98)}}},
 
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(100)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(101)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(102)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG3},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(103)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG4},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(104)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG5},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(105)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG6},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(106)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG7},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(107)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG8},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(108)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG9},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(109)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG10},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(110)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PMUIRG11},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(111)}}},	

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_COMMRX0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(112)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_COMMRX1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(113)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_COMMRX2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(114)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_COMMRX3},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(115)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_COMMTX0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(116)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_COMMTX1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(117)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_COMMTX2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(118)}}},	
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_COMMTX3},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(119)}}},	

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PWRCTLO0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(120)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PWRCTLO1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(121)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PWRCTLO2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(122)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_PWRCTLO3},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(123)}}},

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMACexbus_End},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAExBus_End)}}},
    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_DMACexbus_Err},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KIntDMAExBus_Err)}}},

    { { { KHcrCat_MHA_Interrupt, KHcrKey_Interrupt_AHBbridge3},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(126)}}},


/**
HCR Settings identifing the GPIO pin assignments.

These element keys are used along with the KHcrCat_MHA_GPIO category UID
to identify and read the HCR setting.
*/
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_UART1_DSR},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(8)}}},
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_UART1_DTR},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(9)}}},
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_UART1_DCD},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(10)}}},
	
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_CKERST},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(11)}}},

    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_CSI_CS42L51},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(12)}}},
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_CSIa},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(13)}}}, 
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_CSIb},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(14)}}},
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_CSIc},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(15)}}},

    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_PCIa},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(18)}}},
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_PCIb},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(17)}}},
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_PCIc},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(16)}}},
 	
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_RGB},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(20)}}},

    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_eTRON},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(21)}}},

    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_LCDl},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(22)}}},

    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_RTC},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(23)}}},

    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_LINT20},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(24)}}},

    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_AUDIO_RESET},	ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(25)}}},

    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_SWb0},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(27)}}},
    { { {KHcrCat_MHA_GPIO, KHcrKey_GpioPin_SWb1},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(28)}}},


/**
HCR Settings identifing the I2S Bus Channels. 

These element keys are used along with the KHcrCat_MHA_I2S category UID
to identify and read the HCR setting.
*/

    { { {KHcrCat_MHA_I2S, KHcrKey_I2S_CS42L51},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(0)}}},
    { { {KHcrCat_MHA_I2S, KHcrKey_I2S_Reserved1},		ETypeUndefined, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(0xFFFFFFFF)}}}, 
    { { {KHcrCat_MHA_I2S, KHcrKey_I2S_Reserved2},		ETypeUndefined, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(0xFFFFFFFF)}}}, 
    { { {KHcrCat_MHA_I2S, KHcrKey_I2S_Reserved3},		ETypeUndefined, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(0xFFFFFFFF)}}}, 


/**
HCR Settings identifing the I2C Bus IDs. 

These element keys are used along with the KHcrCat_MHA_I2C category UID
to identify and read the HCR setting.
*/

    { { {KHcrCat_MHA_I2C, KHcrKey_I2C_ReservedA},		ETypeUndefined, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(0xFFFFFFFF)}}}, 
    { { {KHcrCat_MHA_I2C, KHcrKey_I2C_ReservedB},		ETypeUndefined, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(0xFFFFFFFF)}}}, 


/**
HCR Settings identifing the SPI/CSI Bus Channels. 

These element keys are used along with the KHcrCat_MHA_SPI_CSI category UID
to identify and read the HCR setting.
*/
    { { {KHcrCat_MHA_SPICSI, KHcrKey_CSI_CS42L51},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(0x0010000C)}}},
    { { {KHcrCat_MHA_SPICSI, KHcrKey_CSI_ReservedB},		ETypeUndefined, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(0xFFFFFFFF)}}}



//
// Last entry must not end in a ',' as it is present in the enclosing file.
// 		};


#endif // HCR_PSL_CONFIG_MHA_INL


