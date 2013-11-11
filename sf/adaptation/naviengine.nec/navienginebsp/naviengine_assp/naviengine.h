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
* naviengine_assp\naviengine.h
* Definitions for NE1_TBVariant ASSP
*
*/



#ifndef __A32NAVIENGINE_H__
#define __A32NAVIENGINE_H__
#include <e32const.h>
#include <e32hal.h>

//-------------------------------------------------------------------
// Constant conventions:
//-------------------------------------------------------------------

// KH       Hardware definition
// KHw      4-byte word definition prefix
// KHb      Byte definition prefix
// KHt      Bit definition prefix
// KHm      Mask definition prefix
// KHs      Shift definition prefix
// KHo      Offset definition prefix
// KHwRo    Read-only register
// KHwWo    Write-only register
// KHwRw    Read/write register
// KHwBase  Base address within memory map
// _i       Input suffix
// _o       Output suffix
// _b       Input/output suffix
//
// ALL ASSP specific hardware registers should be defined in this file, not
// in local files.  The register belongs to the ASSP, not the driver.

//----------------------------------------------------------------------------
// Memory map: physical addresses
//----------------------------------------------------------------------------

// These details are taken directly from the NaviEngine SoC TRM "S18599EJ1V0UM00.pdf"
// Table 4-1 Address Map
const TUint KHwBaseMPcorePrivatePhys = 0xc0000000; // Private MPcore region for SCU and Interrupt controller (8k address space)
const TUint KHwDDR2RamBasePhys       = 0x80000000; // 256MB of DDR2 RAM sits here (in a 1GB address space)
const TUint KHwPCIPhys               = 0x20000000; // PCI controller (128MB address space)
const TUint KHwInternal              = 0x18000000; // Internal SoC peripherals reside in this area (16MB address space)

// Table 4-2 AXI Address Map
const TUint KHwAXI64IC2Phys          = 0x80000000; // AXI64 bus base address
const TUint KHwSATAPhys              = 0x18016000; // SATA Controller
const TUint KHwAXI64DMACPhys         = 0x18015000; // DMA Controller for the AXI64 bus
const TUint KHwVideoPhys             = 0x18014000; // External Video Controller
const TUint KHwDispPhys              = 0x18010000; // LCD/VGA display controller
const TUint KHwSGXPhys               = 0x18000000; // SGX display controller

// Table 4-3 AHB Address Map
const TUint KHwCFWindow2Phys         = 0x18070000;
const TUint KHwCFWindow1Phys         = 0x18060000;
const TUint KHwCFWindow0Phys         = 0x18050000;
const TUint KHwATA6_CS1Phys          = 0x18029000;
const TUint KHwATA6_CS0Phys          = 0x18028000;
const TUint KHwUSBHPhys              = 0x18024000; // Internal PCI controller memory window
const TUint KHwAHB32PCI_USBPhys      = 0x18023000; // PCI controller for the internal USB controller
const TUint KHwAHB32PCI_ExtPhys      = 0x18022000; // PCI controller for the external PCI bus
const TUint KHwDDR2RegPhys           = 0x18021000; // DDR2 memory control registers
const TUint KHwAHB0DMAC4Phys         = 0x18020000; // AHB bus DMA Controller 4
const TUint KHwAHB0DMAC3Phys         = 0x1801f000; // AHB bus DMA Controller 3
const TUint KHwAHB0DMAC2Phys         = 0x1801e000; // AHB bus DMA Controller 2
const TUint KHwAHB0DMAC1Phys         = 0x1801d000; // AHB bus DMA Controller 1
const TUint KHwAHB0DMAC0Phys         = 0x1801c000; // AHB bus DMA Controller 0
const TUint KHwAHBEXDMACPhys         = 0x1801b000; // AHB bus DMA Controller on external bus
const TUint KHwEXBUSPhys             = 0x18018000; // External Bus / NAND / CF

// Table 4-4 APB Address Map
const TUint KHwDTVIfPhys             = 0x1804C000; // Digital TV interface
const TUint KHwAPB1Phys              = 0x18030000; // AHB32APB bus base address

const TUint KHwPWM7Phys              = 0x1803AC00;
const TUint KHwPWM6Phys              = 0x1803A800;
const TUint KHwPWM5Phys              = 0x1803A400;
const TUint KHwPWM4Phys              = 0x1803A000;
const TUint KHwPWM3Phys              = 0x18039C00;
const TUint KHwPWM2Phys              = 0x18039800;
const TUint KHwPWM1Phys              = 0x18039400;
const TUint KHwPWM0Phys              = 0x18039000;
const TUint KHwGPIOPhys              = 0x18038000; // GPIO module
const TUint KHwSYSCTRLPhys           = 0x18037C00; // System Control Unit
const TUint KHweWDTPhys              = 0x18037800; // External Watchdog Timer
const TUint KHwTimer5Phys            = 0x18037400; // Timers
const TUint KHwTimer4Phys            = 0x18037000;
const TUint KHwTimer3Phys            = 0x18036C00;
const TUint KHwTimer2Phys            = 0x18036800;
const TUint KHwTimer1Phys            = 0x18036400;
const TUint KHwTimer0Phys            = 0x18036000;
const TUint KHwUART7Phys             = 0x18035C00; // UARTs
const TUint KHwUART6Phys             = 0x18035800;
const TUint KHwUART5Phys             = 0x18035400;
const TUint KHwUART4Phys             = 0x18035000;
const TUint KHwUART3Phys             = 0x18034C00;
const TUint KHwUART2Phys             = 0x18034800;
const TUint KHwUART1Phys             = 0x18034400;
const TUint KHwUART0Phys             = 0x18034000;
const TUint KHwI2S3Phys              = 0x18033000; // I2S
const TUint KHwI2S2Phys              = 0x18032C00;
const TUint KHwI2S1Phys              = 0x18032800;
const TUint KHwI2S0Phys              = 0x18032400;
const TUint KHwI2CPhys               = 0x18032000; // I2C
const TUint KHwCSI1Phys              = 0x18031800; // CSI
const TUint KHwCSI0Phys              = 0x18031400;
const TUint KHwSPDIFPhys             = 0x18031000; // SPDIF
const TUint KHwSDPhys                = 0x18030000; // SD

// Taken from the NE1-TB Hardware Specification "NE1-TB_HW-Spec_R1p0_20071221.pdf"
// Section 5
const TUint KHwFPGAPhys              = 0x04010000; // FPGA registers

// Taken from an email Section 4 and email exchange with NEC
const TUint KHwLANPhys               = 0x04000000; // SMCS 9118 LAN Controller


//----------------------------------------------------------------------------
// Memory map: linear addresses
// This has to be consistent with HwBanks mappings in bootrom (see, ne1_tb.s).
//----------------------------------------------------------------------------
#ifdef __MEMMODEL_DIRECT__

const TUint KHwBaseUart0            = KHwUART0Phys;              // serial port #0
const TUint KHwBaseMPCorePrivate    = KHwBaseMPcorePrivatePhys;  // 4KB of private MPcore region for SCU and Interrupt controller
const TUint KHwTimersBase           = KHwTimer0Phys;             // six SoC timers
const TUint KHwDisplayBase          = KHwDispPhys;               // SoC display controller
const TUint KHwBaseI2C              = KHwI2CPhys;                // I2C
const TUint KHwFPGABase             = KHwFPGAPhys;               // FPGA registers
const TUint KHwSPDIFBase            = KHwSPDIFPhys;              // SPDIF
const TUint KHwBaseSDCtrl           = KHwSDPhys;                 // SD
const TUint KDMACExBusBase          = KHwAHBEXDMACPhys;          // DMAC(4C) 4KB
const TUint KDMAC32Base             = KHwAHB0DMAC0Phys;          // DMAC(8C) 20KB
const TUint KHwDMAC64Base           = KHwAXI64DMACPhys;          // DMAC(4C) 4KB on AXI bus - 64bit
const TUint KHwBaseEthernet         = KHwLANPhys;                // Ethernet
const TUint KHwGPIOBase             = KHwGPIOPhys;               // GPIO
const TUint KHwPciBase              = KHwAHB32PCI_ExtPhys;       // PCI Bridges
const TUint KHwUsbHWindow           = KHwUSBHPhys;               // Internal PCI window
//const TUint KHw xxx Base          = KHw xxx Phys;              // xxx

#else

const TLinAddr KHWIOBase            = 0xC6000000u;               // Base address of virtual address range used for HW mappings

const TUint KHwBaseUart0            = KHWIOBase;                 //Bank # 0, ofset      0,  size 4K         // serial port #0                                                ///< Internal, clients get value from HCR
const TUint KHwBaseMPCorePrivate    = KHWIOBase + 0x01000;       //Bank # 1, offset  1000h, size 2*4K       // 4KB of private MPcore region for SCU and Interrupt controller ///< Internal, clients get value from HCR
const TUint KHwTimersBase           = KHWIOBase + 0x03000;       //Bank # 2, offset  3000h, size 2*K        // six SoC timers                                                ///< Internal, clients get value from HCR
const TUint KHwDisplayBase          = KHWIOBase + 0x05000;       //Bank # 3, offset  4000h, size 4*K        // SoC display controller                                        ///< Internal, clients get value from HCR
const TUint KHwBaseI2C              = KHWIOBase + 0x09000;       //Bank # 4, offset  9000h, size 2*4K       // I2C                                                           ///< Internal, clients get value from HCR
const TUint KHwFPGABase             = KHWIOBase + 0x0B000;       //Bank # 5, offset  B000h, size 4K         // FPGA registers                                                ///< Internal, clients get value from HCR
const TUint KHwSPDIFBase            = KHWIOBase + 0x0C000;       //Bank # 6, offset  C000h, size 4K         // SPDIF                                                         ///< Internal, clients get value from HCR
const TUint KHwBaseSDCtrl           = KHWIOBase + 0x0D000;       //Bank # 7, offset  D000h, size 4K         // SD                                                            ///< Internal, clients get value from HCR
const TUint KDMACExBusBase          = KHWIOBase + 0x0E000;       //Bank # 8, offset  D000h, size 6*4K = 24K // DMAC(4C) 4KB                                                  ///< Internal, clients get value from HCR
const TUint KDMAC32Base             = KHWIOBase + 0x0F000;       //Bank # 9, offset  D000h, size 6*4K = 24K // DMAC(8C) 20KB                                                 ///< Internal, clients get value from HCR
const TUint KHwBaseEthernet         = KHWIOBase + 0x14000;       //Bank #10, offset 14000h, size 4K         // Ethernet                                                      ///< Internal, clients get value from HCR
const TUint KHwGPIOBase             = KHWIOBase + 0x15000;       //Bank #11, offset 15000h, size 4K         // GPIO                                                          ///< Internal, clients get value from HCR
const TUint KHwPciBase              = KHWIOBase + 0x16000;       //Bank #12, offset 16000h, size 2*4K       // PCI Bridges                                                   ///< Internal, clients get value from HCR
const TUint KHwUsbHWindow           = KHWIOBase + 0x18000;       //Bank #13, offset 18000h, size 2*4K       // Internal PCI window                                           ///< Internal, clients get value from HCR
const TUint KHwDMAC64Base           = KHWIOBase + 0x1A000;       //Bank #14, offset 1A000h, size 4k         // DMAC(4C) 4KB on AXI bus - 64bit                               ///< Internal, clients get value from HCR
//const TUint KHw xxx Base            = KHWIOBase + 0x1B000;     //Bank #15, offset 1B000h, size xK         // xxx                                                           ///< Internal, clients get value from HCR
#endif




const TUint KHwBaseUart1            = KHwBaseUart0         + 0x400;  // serial port #1               ///< Internal, clients get value from HCR
const TUint KHwBaseUart2            = KHwBaseUart0         + 0x800;  // serial port #2               ///< Internal, clients get value from HCR
const TUint KHwBaseSCU              = KHwBaseMPCorePrivate;          // Snoop Control Unit           ///< Internal, clients get value from HCR
const TUint KHwBaseIntIf            = KHwBaseMPCorePrivate + 0x100;  // CPU interrupt interface      ///< Internal, clients get value from HCR
const TUint KHwBaseGlobalIntDist    = KHwBaseMPCorePrivate + 0x1000; // Global Interrupt Distributer ///< Internal, clients get value from HCR
const TUint KHwWatchdog             = KHwTimersBase        + 0x1800; // eWDT (Watchdog Timer)        ///< Internal, clients get value from HCR
const TUint KHwSystemCtrlBase       = KHwTimersBase        + 0x1c00; // system control unit          ///< Internal, clients get value from HCR
const TUint KHwBaseI2S0             = KHwBaseI2C           + 0x400;  // I2S0                         ///< Internal, clients get value from HCR
const TUint KHwBaseI2S1             = KHwBaseI2S0          + 0x400;  // I2S1                         ///< Internal, clients get value from HCR
const TUint KHwBaseI2S2             = KHwBaseI2S0          + 0x800;  // I2S2                         ///< Internal, clients get value from HCR
const TUint KHwBaseI2S3             = KHwBaseI2S0          + 0xC00;  // I2S3                         ///< Internal, clients get value from HCR
const TUint KHwBaseCSI0             = KHwSPDIFBase         + 0x400;  // CSI0                         ///< Internal, clients get value from HCR
const TUint KHwBaseCSI1             = KHwBaseCSI0          + 0x400;  // CSI1                         ///< Internal, clients get value from HCR
const TUint KHwLCDDispBase          = KHwFPGABase          + 0x0400; // LCD display                  ///< Internal, clients get value from HCR
const TUint KHwTSPBase              = KHwFPGABase          + 0x0600; // Digitiser                    ///< Internal, clients get value from HCR
const TUint KHwPciBridgeExtern      = KHwPciBase;                    // External PCI Bridge          ///< Internal, clients get value from HCR
const TUint KHwPciBridgeUsb         = KHwPciBase           + 0x1000; // UsbHost dedicated PCI Bridge ///< Internal, clients get value from HCR



//----------------------------------------------------------------------------
// CPU interrupt interface registers' addresses.
//----------------------------------------------------------------------------
const TUint KHwCIIControl     = KHwBaseIntIf + 0x00; // Control
const TUint KHwCIIPrioMask    = KHwBaseIntIf + 0x04; // Priority mask
const TUint KHwCIIBinPoint    = KHwBaseIntIf + 0x08; // Binary point
const TUint KHwCIIIntAck      = KHwBaseIntIf + 0x0C; // Interrupt acknowledge RO
const TUint KHwCIIEndOfInt    = KHwBaseIntIf + 0x10; // End of interrupt WO
const TUint KHwCIIPrioRun     = KHwBaseIntIf + 0x14; // Running Interrupt RO
const TUint KHwCIIHighPend    = KHwBaseIntIf + 0x18; // Highest pending interrupt RO

const TUint KHoCIIIntAck      = 0x0C;                // Interrupt acknowledge RO
const TUint KHoCIIEndOfInt    = 0x10;                // End of interrupt WO

//----------------------------------------------------------------------------
// Global Interrupt Distributor registers' addresses
//----------------------------------------------------------------------------
const TUint KHoGidControl         = KHwBaseGlobalIntDist + 0x000; // Control Register. Holds enable bit.
const TUint KHoGidType            = KHwBaseGlobalIntDist + 0x004; // Controller type. Read only register
const TUint KHoGidIntEnableBase   = KHwBaseGlobalIntDist + 0x100; // Writing into these registers will enable appropriate interrupt(s), 1 bit per interrupt
const TUint KHoGidIntDisableBase  = KHwBaseGlobalIntDist + 0x180; // Writing into these registers will disable appropriate interrupt(s), 1 bit per interrupt
const TUint KHoGidPendSetBase     = KHwBaseGlobalIntDist + 0x200; // Writing into these registers will return appropriate interrupt(s) from Pending to Inactive state. Active state is not modified, 1 bit per interrupt
const TUint KHoGidPendClearBase   = KHwBaseGlobalIntDist + 0x280; // Writing into these registers will put appropriate interrupt(s) into  Pending state, 1 bit per interrupt
const TUint KHoGidActiveBase      = KHwBaseGlobalIntDist + 0x300; // Indicates if corresponding interrupt is active. Ints 0-31 (the first reg.) are aliased for each CPU, 1 bit per interrupt
const TUint KHoGidPriorityBase    = KHwBaseGlobalIntDist + 0x400; // Priority of the interrupts, 8 bits per int, only 4 used
const TUint KHoGidTargetBase      = KHwBaseGlobalIntDist + 0x800; // Interrupt CPU targets, 8 bits per int, only 4 used
const TUint KHoGidConfigBase      = KHwBaseGlobalIntDist + 0xC00; // Interrupt configuration register: edge or leveled; 1-N or N-N software model,  2 bits per interrupt
const TUint KHoGidSoftwareTrigger = KHwBaseGlobalIntDist + 0xF00; // Software interrupt Trigger Register. A single register for all ints.

//----------------------------------------------------------------------------
// SystemControlUnit registers' addresses
//----------------------------------------------------------------------------
const TUint KHoSCUClockMaskCtrl                     = 0x80; // Mask control
const TUint KHoSCUDisplayDCLKCtrl                   = 0x8c; // Display DCLK control
const TUint KHw60HzDisplay                          = 11;

// Divide I2S CLK Control Register
const TUint KHoSCUDivideI2SCLKCtrl = 0x94;
const TUint KHtSCUDivI2SCLKCtrl_I2S0MCLK_SEL        = (1<<4); // Selects the I2S0 MCLK source: 1-SPDIF, 0-external input.
const TUint KHsSCUDivI2SCLKCtrl_I2S0MCLK_FREQ       = 0;      // shift for the MCK clock frequency for I2S0 unit
const TUint KHsSCUDivI2SCLKCtrl_I2S1MCLK_FREQ       = 8;      // shift for the MCK clock frequency for I2S1 unit
const TUint KHsSCUDivI2SCLKCtrl_I2S2MCLK_FREQ       = 16;     // shift for the MCK clock frequency for I2S2 unit
const TUint KHsSCUDivI2SCLKCtrl_I2S3MCLK_FREQ       = 24;     // shift for the MCK clock frequency for I2S3 unit
// values for MCK clock frequency
const TUint KHSCUDivI2SCLKCtrl_I2SMCLK_FREQ36_864M  = 0;
const TUint KHSCUDivI2SCLKCtrl_I2SMCLK_FREQ24_576M  = 1;
const TUint KHSCUDivI2SCLKCtrl_I2SMCLK_FREQ18_432M  = 2;
const TUint KHSCUDivI2SCLKCtrl_I2SMCLK_FREQ33_8688M = 4;
const TUint KHSCUDivI2SCLKCtrl_I2SMCLK_FREQ22_5792M = 5;
const TUint KHSCUDivI2SCLKCtrl_I2SMCLK_FREQ16_9344M = 6;

//----------------------------------------------------------------------------
// I2S Register Addresses
//----------------------------------------------------------------------------
const TUint KHoI2SCtrl              = 0x00;    // I2S Control register offset
const TUint KHtI2SCtrl_MSMODE       = (1<<28); // master/slave mode (1-master, 0-slave)
const TUint KHtI2SCtrl_TEN          = (1<<25); // transmission enable (1-enable, 0-disable)
const TUint KHtI2SCtrl_REN          = (1<<24); // reception (1-enable, 0-disable)
const TUint KHsI2SCtrl_FCKLKSEL     = 16;      // [3:0] clock select mask (Sampling fq)
const TUint KHsI2SCtrl_FSMODE       = 12;      // [1:0] number of SCLK cycles in one frame (1WS cycle)
const TUint KHsI2SCtrl_FORMAT       = 8;       // [2:0] transfer format (I2S = 100b)
const TUint KHtI2SCtrl_INVALID      = (1<<6);  // Invalid data format(1 - outputs "1" as invalid data)
const TUint KHsI2SCtrl_DLENGTH      = 0;       // [4:0] sampling data length

const TUint KHoI2SFifoCtrl          = 0x04;    // I2S Fifo Control register offset
const TUint KHtI2SFifoCtrl_TDMAEN   = (1<<23); // Enables DMA req on transmit side
const TUint KHtI2SFifoCtrl_TFIFOCLR = (1<<22); // transmit FIFO initialisation
const TUint KHsI2SFifoCtrl_TFIFOLT  = 19;      // [2:0] L-ch transmit FIFO trigger level (0-3: 2-16 word space avl)
const TUint KHsI2SFifoCtrl_TFIFORT  = 16;      // [2:0] R-ch transmit FIFO trigger level (0-3: 2-16 word space avl)
const TUint KHtI2SFifoCtrl_RDMAEN   = (1<<7);  // Enables DMA req on receive side
const TUint KHtI2SFifoCtrl_RFIFOCLR = (1<<6);  // receive FIFO initialisation
const TUint KHsI2SFifoCtrl_RFIFOLT  = 3;       // [2:0] L-ch receive FIFO-full trigger level (0-3: 2-16 word space avl)
const TUint KHsI2SFifoCtrl_RFIFORT  = 0;       // [2:0] R-ch receive FIFO-full trigger level (0-3: 2-16 word space avl)

const TUint KHoI2SFifoSts           = 0x08;    // I2S Fifo status register offset
const TUint KHtI2SFifoSts_TFL_FULL	= (1<<17); // L-ch transmit FIFO full (32 words)
const TUint KHtI2SFifoSts_TFR_FULL	= (1<<16); // R-ch transmit FIFO full (32 words)
const TUint KHtI2SFifoSts_RFL_EMPTY = (1<<1);  // L-ch receive FIFO empty
const TUint KHtI2SFifoSts_RFR_EMPTY = (1<<0);  // R-ch receive FIFO empty

const TUint KHoI2SIntFlg            = 0x0C;    // I2S interrupt flag register offset
const TUint KHtI2SIntFlg_TFLURINT	= (1<<19); // L-ch transmit FIFO underrun
const TUint KHtI2SIntFlg_TFLEINT    = (1<<18); // L-ch transmit FIFO reached the empty trigger level
const TUint KHtI2SIntFlg_TFRURINT   = (1<<17); // R-ch transmit FIFO underrun
const TUint KHtI2SIntFlg_TFREINT    = (1<<16); // R-ch transmit FIFO reached the empty trigger level
const TUint KHtI2SIntFlg_RFLORINT   = (1<<3);  // L-ch receive FIFO overrun
const TUint KHtI2SIntFlg_RFLFINT    = (1<<2);  // L-ch receive FIFO reached the full trigger level
const TUint KHtI2SIntFlg_RFRORINT   = (1<<1);  // R-ch receive FIFO overrun
const TUint KHtI2SIntFlg_RFRFINT    = (1<<0);  // R-ch receive FIFO reached the full trigger level

const TUint KHoI2SIntMask           = 0x10;    // I2S interrupt mask register offset
const TUint KHtI2SIntMask_TFLURINT  = (1<<19); // L-ch transmit FIFO underrun int enable
const TUint KHtI2SIntMask_TFLEINT   = (1<<18); // L-ch transmit FIFO reached the empty trigger level int enable
const TUint KHtI2SIntMask_TFRURINT  = (1<<17); // R-ch transmit FIFO underrun int enable
const TUint KHtI2SIntMask_TFREINT   = (1<<16); // R-ch transmit FIFO reached the empty trigger level int enable
const TUint KHtI2SIntMask_RFLORINT  = (1<<3);  // L-ch receive FIFO overrun int enable
const TUint KHtI2SIntMask_RFLFINT   = (1<<2);  // L-ch receive FIFO reached the full trigger level int enable
const TUint KHtI2SIntMask_RFRORINT  = (1<<1);  // R-ch receive FIFO overrun int enable
const TUint KHtI2SIntMask_RFRFINT   = (1<<0);  // R-ch receive FIFO reached the full trigger level int enable
const TUint KHmI2SIntMask_ALL       = 0xF000F; // All interrupts mask

const TUint KHoI2SRx                = 0x20;    // I2S receive data FIFO register offset
const TUint KHoI2STx                = 0x30;    // I2S transmit data FIFO register offset

//----------------------------------------------------------------------------
//FPGA Register Addresses
//----------------------------------------------------------------------------
const TUint KHoLCDControl      = 0x400;
const TUint KHoSystemPowerDown = 0xF10;

const TUint KHwIDMODE          = KHwFPGABase+0x0810;
const TUint KHmUserSwitches    = 0x3C00;
const TUint KHsUserSwitches    = 10;
const TUint KHmLcdSwitches     = 0x3000;
const TUint KHsLcdSwitches     = 12;
const TUint KHmKeyConfigSwitch = 0x800;
const TUint KHsKeyConfigSwitch = 11;

// There are 4 red LEDs that can be controlled by the FPGA
// Each LED has a bit in the register, where 0==off and 1==on
const TUint KHoFpgaLeds        = 0x0A06;
const TUint KHwFpgaLeds        = KHwFPGABase+KHoFpgaLeds;

const TUint KHmFpgaLeds        = 0xF;

const TUint KHsFpgaLed0        = 0x0;
const TUint KHsFpgaLed1        = 0x1;
const TUint KHsFpgaLed2        = 0x2;
const TUint KHsFpgaLed3        = 0x3;

const TUint KHtFpgaLed0        = 1 << KHsFpgaLed0;
const TUint KHtFpgaLed1        = 1 << KHsFpgaLed1;
const TUint KHtFpgaLed2        = 1 << KHsFpgaLed2;
const TUint KHtFpgaLed3        = 1 << KHsFpgaLed3;

//----------------------------------------------------------------------------
//CSI Register Addresses
//----------------------------------------------------------------------------
const TUint KHoCSIModeControl       = 0x00;  // CSI Mode Control Register
const TUint KHsCSIModeTWait         = 16;    // CSI waiting time for transaction shift
const TUint KHtCSIModeEnable        = 1<<7;  // CSI enable
const TUint KHtCSIModeTrEnable      = 1<<6;  // CSI transmission and reception mode select
const TUint KHtCSIModeDataLen       = 1<<5;  // CSI Data length select: 0-8bits, 1-16bits
const TUint KHtCSIModeTransferDir   = 1<<4;  // CSI Transfer direction mode: 0-MSB first, 1-LSB first
const TUint KHtCSIModeTransferState = 1<<0;  // CSI Transfer state indication flag: 0-idle first, 1-transmission

const TUint KHoCSIClockSelect       = 0x04;  // CSI Clock Select Register
const TUint KHtCSIClockSelectSSE    = 1<<9;  // SS pin enable
const TUint KHtCSIClockSelectSSPol  = 1<<8;  // SS pin polarity select (0: SS pin low active,  1: SS pin high active)
const TUint KHtCSIClockSelectCKP    = 1<<4;  // Clock polarity select
const TUint KHtCSIClockSelectDAP    = 1<<3;  // Clock phase select

const TUint KHsCSIClockSelect       = 0;     // Communication clock select shift CKS[2:0]
const TUint KHCSIClockValPCLKdiv4   = 0;     //  1/4 PCLK (master mode)   16.67 MHz
const TUint KHCSIClockValPCLKdiv16  = 1;     //  1/16 PCLK (master mode)	4.17 MHz
const TUint KHCSIClockValPCLKdiv32  = 2;     //  1/32 PCLK (master mode)	2.08 MHz
const TUint KHCSIClockValPCLKdiv64  = 3;     //  1/64 PCLK (master mode)	1.04 MHz
const TUint KHCSIClockValPCLKdiv128 = 4;     //  1/128 PCLK (master mode)	 521 kHz
const TUint KHCSIClockValPCLKdiv256 = 5;     //  1/256 PCLK (master mode)	 260 kHz
const TUint KHCSIClockValPCLKdiv512 = 6;     //  1/512 PCLK (master mode)	 130 kHz
const TUint KHCSIClockValSlave      = 7;     //  SCKI (slave mode)

const TUint KHoCSIControl           = 0x08;  // CSI Control Register
const TUint KHtCSIControlCSIRst     = 1<<28; // CSI unit reset
const TUint KHtCSIControlTxTrgEn    = 1<<27; // Permission of CSI_FIFOTRG.bit10-8(T_TRG[2:0]) operation
const TUint KHtCSIControlTxFifoFull = 1<<26; // State of Tx FIFO buffer
const TUint KHtCSIControlTxDMAE     = 1<<24; // Tx DMA mode
const TUint KHtCSIControlSSMon      = 1<<21; // SS signal monitor
const TUint KHtCSIControlRxTrgEn    = 1<<19; // Permission of CSI_FIFOTRG.bit2-0(R_TRG[2:0]) operation
const TUint KHtCSIControlRxFifoFull = 1<<18; // State of Rx FIFO buffer
const TUint KHtCSIControlRxDMAE     = 1<<16; // Rx DMA mode

const TUint KHtCSIControlSSDnIE     = 1<<15; // SS signal negative-edge interrupt (CSI_INT.bit15(SS_DN)) enable
const TUint KHtCSIControlSSUpIE     = 1<<14; // SS signal positive-edge interrupt (CSI_INT.bit14(SS_UP)) enable
const TUint KHtCSIControlTxUndIE    = 1<<13; // Tx FIFO buffer under-run error interrupt (CSI_INT.bit13(UNDER)) enable
const TUint KHtCSIControlRxOvfIE    = 1<<12; // Rx FIFO buffer overflow error interrupt (CSI_INT.bit12(OVERF)) enable
const TUint KHtCSIControlTEndIE     = 1<<8;  // Transmission end interrupt (CSI_INT.bit8(CSIEND)) enable
const TUint KHtCSIControlTxTrgIE    = 1<<4;  // Tx trigger level interrupt (CSI_INT.bit4(T_TRGR)) enable
const TUint KHtCSIControlRxTrgIE    = 1<<0;  // Rx trigger level interrupt (CSI_INT.bit0(R_TRGR bit) enable

const TUint KHoCSIIntStatus   = 0x0C;  // CSI Interrupt Status Register
const TUint KHtCSIIntStatusSSDn    = 1<<15;  // SS signal negative-edge interrupt
const TUint KHtCSIIntStatusSSUp    = 1<<14;  // SS signal positive-edge interrupt
const TUint KHtCSIIntStatusTxUnd   = 1<<13;  // Tx FIFO buffer under-run error interrupt
const TUint KHtCSIIntStatusRxOvf   = 1<<12;  // Rx FIFO buffer overflow error interrupt
const TUint KHtCSIIntStatusTEnd    = 1<<8;   // Transmission end interrupt
const TUint KHtCSIIntStatusTxTrgIE = 1<<4;   // Tx trigger level interrupt
const TUint KHtCSIIntStatusRxTrgIE = 1<<0;   // Rx trigger level interrupt

const TUint KHoCSIIFifoL           = 0x10;   // CSI Receive FIFO level indicate Register
const TUint KHoCSIOFifoL           = 0x14;   // CSI Transmit FIFO level indicate Register
const TUint KHoCSIIFifo            = 0x18;   // CSI Receive FIFO Window Register
const TUint KHoCSIOFifo            = 0x1C;   // CSI Transmit FIFO Window Register
const TUint KHwCSIFifoLMax         = 32;     // maximum amount of data in the CSI FIFO

const TUint KHoCSIFifoTrgLvl       = 0x20;   // CSI FIFO Trigger Level Register
const TUint KHsCSITxFifoTrgLvl     = 8;      // transmit FIFO trigger level shift [10:8]
const TUint KHsCSIRxFifoTrgLvl     = 0;      // receive FIFO trigger level shift [2:0]

//----------------------------------------------------------------------------
// GPIO Register Addresses
//----------------------------------------------------------------------------
const TUint KHwRwGpio_Port_Control_Enable   = KHwGPIOBase + 0x00;
const TUint KHwWoGpio_Port_Control_Disable  = KHwGPIOBase + 0x04;
const TUint KHwWoGpio_Port_Set_Clear_Hi     = KHwGPIOBase + 0x08;
const TUint KHwWoGpio_Port_Set_Clear_Lo     = KHwGPIOBase + 0x0c;
const TUint KHwRoGpio_Port_Value            = KHwGPIOBase + 0x10;
const TUint KHwRwGpio_Int                   = KHwGPIOBase + 0x14;
const TUint KHwRwGpio_Int_Enable            = KHwGPIOBase + 0x18;
const TUint KHwWoGpio_Int_Disable           = KHwGPIOBase + 0x1c;
const TUint KHwRwGpio_Int_Hold              = KHwGPIOBase + 0x20;
const TUint KHwRwGpio_Int_Mode0             = KHwGPIOBase + 0x24;
const TUint KHwRwGpio_Int_Mode1             = KHwGPIOBase + 0x28;
const TUint KHwRwGpio_Int_Mode2             = KHwGPIOBase + 0x2c;
const TUint KHwRwGpio_Int_Mode3             = KHwGPIOBase + 0x30;
const TUint KHwRwGpi_Polarity_Invert        = KHwGPIOBase + 0x38;
const TUint KHwWoGpi_Polarity_Reset         = KHwGPIOBase + 0x3c;
const TUint KHwRwGpo_Polarity_Invert        = KHwGPIOBase + 0x40;
const TUint KHwWoGpo_Polarity_Reset         = KHwGPIOBase + 0x44;

const TUint KGpio_Ethernet_Int_Pin          = 20;

//----------------------------------------------------------------------------
// eWDT Register Addresses
//----------------------------------------------------------------------------
const TUint KHwWatchdog_WDTCNT (KHwWatchdog +  0); // Control register
const TUint KHwWatchdog_WDTSET (KHwWatchdog +  4); // Period setting register
const TUint KHwWatchdog_WDTTIM (KHwWatchdog +  8); // Lapsed time register
const TUint KHwWatchdog_WDTINT (KHwWatchdog + 12); // Interrupt register

//----------------------------------------------------------------------------
// Baud Rate Divisor values
//----------------------------------------------------------------------------
const TUint KBaudRateDiv_50      = 0;
const TUint KBaudRateDiv_75      = 0;
const TUint KBaudRateDiv_110     = 0;
const TUint KBaudRateDiv_134     = 0;
const TUint KBaudRateDiv_150     = 55417;
const TUint KBaudRateDiv_300     = 27708;
const TUint KBaudRateDiv_600     = 13854;
const TUint KBaudRateDiv_1200    = 6927;
const TUint KBaudRateDiv_1800    = 4618;
const TUint KBaudRateDiv_2000    = 4156;
const TUint KBaudRateDiv_2400    = 3464;
const TUint KBaudRateDiv_3600    = 2309;
const TUint KBaudRateDiv_4800    = 1732;
const TUint KBaudRateDiv_7200    = 1155;
const TUint KBaudRateDiv_9600    = 866;
const TUint KBaudRateDiv_19200   = 433;
const TUint KBaudRateDiv_38400   = 216;
const TUint KBaudRateDiv_57600   = 144;
const TUint KBaudRateDiv_115200  = 72;
const TUint KBaudRateDiv_230400  = 36;

const TUint KBaudRateDiv_default = 72; // Set to KBaudRateDiv_115200 but h2inc.pl doesn't support token replacement.

//----------------------------------------------------------------------------
// Memory Layout addresses for BootLoader / bootstrap interaction
//----------------------------------------------------------------------------

const TUint KRamTargetAddr                = 0x88000000;      // Phys. addr. of the image to be started by bootloader.
const TUint KCoreLoaderAddress            = 0x8D000000;      // base of ram + 208MB

const TUint xKMega                        = (1024*1024);     // we can't use KMega because h2inc won't resolve tokens from other includes
const TLinAddr KNORFlashTargetAddr        = 0x00000000;      // Onboard NOR flash starts at phys addr 0x0
const TLinAddr KNORFlashTargetSize        = ( 64 * xKMega);  // and is 64MB is size
const TLinAddr KRamTargetSize             = (128 * xKMega);  // Reserved RAM starts at phys addr 0x88000000 and is 128MB is size

// The layout of the onboard NOR is
// [bootloader][configblock][flashedimage][reserved]
const TLinAddr KNORFlashMaxBootloaderSize = (5 * xKMega);
const TLinAddr KNORFlashMaxImageSize      = KNORFlashTargetSize - KNORFlashMaxBootloaderSize;

//----------------------------------------------------------------------------
// Restart Reason Codes
//----------------------------------------------------------------------------

// Restart types (bits 31-29)
const TUint KmRestartReasonsActions            = 0xE0000000; // Bits 31 to 29

// These three bits are indications for the bootloader to perform some activity
const TUint KtRestartReasonHardRestart         = 0x80000000; // Bit31 back to bootloader
const TUint KtRestartReasonSoftRestart         = 0x40000000; // Bit30 back to same image (will need image location too)
const TUint KtRestartReasonBootRestart         = 0x20000000; // Bit29 back into new image (will need image location too)

// This bit is an indicator for ane image to detect it is being warm booted
const TUint KtRestartReasonWarmBoot            = 0x10000000; // Bit28 this boot is "warm"

// Image locations (bits 27-20)
const TUint KmRestartImageLocations            = 0x0FF00000; // Bits 27 to 20
const TUint KtRestartReasonRAMImage            = 0x08000000; // Bit27
const TUint KtRestartReasonNORImage            = 0x04000000; // Bit26
const TUint KtRestartReasonNANDImage           = 0x02000000; // Bit25
const TUint KtRestartReasonONENANDImage        = 0x01000000; // Bit24
const TUint KtRestartReasonSIBLEYImage         = 0x00800000; // Bit23
const TUint KtRestartReasonOTAUpgrade          = 0x00400000; // Bit22
const TUint KtRestartReasonCoreLdr             = 0x00200000; // Bit21  This platform specific reason introduced to start SMP enabled CoreLdr

// Restart startup Mode (bits 19-16)
const TUint KmRestartStartupModes              = 0x000F0000; // Bits 19 to 16
const TUint KsRestartStartupModes              = 16;
const TUint KRestartStartupModesSize           = 4;          // size in bits

// Custom restart reasons (bits 15-8)
const TUint KmRestartCustomRestartReasons      = 0x0000FF00; // Bits 15 to 8
const TUint KsRestartCustomRestartReasons      = 8;
const TUint KRestartCustomRestartReasonsSize   = 8;          // size in bits

// Define USB loader restart (after USB link dropped)
const TUint KtRestartCustomRestartUSBLoader    = 0x00008000; // Bit 15
const TUint KtRestartCustomRestartMemCheck     = 0x00004000; // Bit 14
const TUint KtRestartCustomRestartMemCheckPass = 0x00002000; // Bit 13

// Mask of all the bits used during the memory test (but not the start)
const TUint KmRestartCustomRestartMemCheckBits = 0x00003F00; // Bits 13 to 8


// Gap (bits 7-0)

#endif
