/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
@file hcr_psl_config_assp.inl
File provides setting definitions for the ASSP setting values applicable to the 
NEC NaviEngine base port. These definitions also contain the setting value where
the setting value is no larger than a 32-bit integer. The values for
larger settings can be found in the peer file hcr_psl_config_assp_lsd.inl.

@internalTechnology
*/


#ifndef HCR_PSL_CONFIG_ASSP_INL
#define HCR_PSL_CONFIG_ASSP_INL


// SSettingC gSettingsList[] = 
//     {
//     

/**
HCR Setting values for ASSP Hardware Block base virtual addresses for FMM and MMM. 
*/

	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_Uart0},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseUart0)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_Uart1},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseUart1)} } },	
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_Uart2},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseUart2)} } },			
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_MPCorePrivate}, ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseMPCorePrivate)} } },	
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_SCU},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseSCU)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_IntIf},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseIntIf)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_IntDist},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseGlobalIntDist)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_Timers},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwTimersBase)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_Watchdog},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwWatchdog)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_SystemCtrl},	ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwSystemCtrlBase)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_Display},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwDisplayBase)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_I2C},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseI2C)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_I2S0},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseI2S0)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_I2S1},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseI2S1)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_I2S2},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseI2S2)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_I2S3},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseI2S3)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_FPGA},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwFPGABase)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_CDDisp},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseUart0)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_TSP},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseUart0)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_SPDIF},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwSPDIFBase)} } },

	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_CSI0},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseCSI0)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_CSI1},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseCSI1)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_SDCtrl},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseSDCtrl)} } },

	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_KDMACExBus},	ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KDMACExBusBase)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_KDMAC32},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KDMAC32Base)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_Ethernet},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseEthernet)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_GPIOBase},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwGPIOBase)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_PciBase},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPciBase)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_PciBridgeExtern}, ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPciBridgeExtern)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_PciBridgeUsb},	ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPciBridgeUsb)} } },
                                  
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwBase_UsbHWindow},	ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUsbHWindow)} } },

/*
HCR Setting values for ASSP Physical addresses. 
*/
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_BaseMPcorePrivate},		ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwBaseMPcorePrivatePhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_DDR2RamBase},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwDDR2RamBasePhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_PCI},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPCIPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_Internal},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwInternal)} } },

	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AXI64IC2},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAXI64IC2Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_SATA},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwSATAPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AXI64DMAC},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAXI64DMACPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_Video},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwVideoPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_Disp},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwDispPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_SGX},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwSGXPhys)} } },

	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_CFWindow2},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwCFWindow2Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_CFWindow1},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwCFWindow1Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_CFWindow0},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwCFWindow0Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_ATA6CS1},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwATA6_CS1Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_ATA6CS0},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwATA6_CS0Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_USBH},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUSBHPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AHB32PCIUSB},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAHB32PCI_USBPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AHB32PCIExt},			ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAHB32PCI_ExtPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_DDR2Reg},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwDDR2RegPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AHB0DMAC4},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAHB0DMAC4Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AHB0DMAC3},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAHB0DMAC3Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AHB0DMAC2},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAHB0DMAC2Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AHB0DMAC1},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAHB0DMAC1Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AHB0DMAC0},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAHB0DMAC0Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_AHBEXDMAC},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAHBEXDMACPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_EXBUS},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwEXBUSPhys)} } },

	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_DTVIf},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwDTVIfPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_APB1},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwAPB1Phys)} } },

	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_PWM7},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPWM7Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_PWM6},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPWM6Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_PWM5},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPWM5Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_PWM4},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPWM4Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_PWM3},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPWM3Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_PWM2},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPWM2Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_PWM1},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPWM1Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_PWM0},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwPWM0Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_GPIO},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwGPIOPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_SYSCTRL},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwSYSCTRLPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_eWDT},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHweWDTPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_Timer5},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwTimer5Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_Timer4},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwTimer4Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_Timer3},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwTimer3Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_Timer2},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwTimer2Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_Timer1},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwTimer1Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_Timer0},				ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwTimer0Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_UART7},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUART7Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_UART6},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUART6Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_UART5},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUART5Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_UART4},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUART4Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_UART3},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUART3Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_UART2},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUART2Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_UART1},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUART1Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_UART0},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwUART0Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_I2S3},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwI2S3Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_I2S2},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwI2S2Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_I2S1},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwI2S1Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_I2S0},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwI2S0Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_I2C},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwI2CPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_CSI1},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwCSI1Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_CSI0},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwCSI0Phys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_SPDIF},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwSPDIFPhys)} } },
	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_SD},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwSDPhys)} } },

	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_FPGA},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwFPGAPhys)} } },

	{ { {KHcrCat_MHA_HWBASE, KHcrKey_HwPhys_LAN},					ETypeUInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(KHwLANPhys)} } },

/*
HCR Setting values for DMA logical channel identifiers. 
*/

	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_SD0},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelSD0)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_SD1},			ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelSD1)} } },

	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_I2S0RX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelI2S0RX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_I2S0TX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelI2S0TX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_I2S1RX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelI2S1RX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_I2S1TX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelI2S1TX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_I2S2RX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelI2S2RX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_I2S2TX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelI2S2TX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_I2S3RX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelI2S3RX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_I2S3TX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelI2S3TX)} } },
                          
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_UART0RX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelUART0RX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_UART0TX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelUART0TX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_UART1RX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelUART1RX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_UART1TX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelUART1TX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_UART2RX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelUART2RX)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_UART2TX},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDMAChannelUART2TX)} } },
                          
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_MemToMem0},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDmaMemToMem0)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_MemToMem1},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDmaMemToMem1)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_MemToMem2},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDmaMemToMem2)} } },
	{ { {KHcrCat_MHA_DMA, KHcrKey_DmaCh_MemToMem3},		ETypeInt32, HCR_FLAGS_NONE, HCR_LEN_NA}, { {HCR_WVALUE(EDmaMemToMem3)} } }

//
// Last entry must not end in a ',' as it is present in the enclosing file.
// 		};


#endif // HCR_PSL_CONFIG_ASSP_INL


