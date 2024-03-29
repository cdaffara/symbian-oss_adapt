/*  Cypress West Bridge API header file (cyanregs.h)
 ## Register and field definitions for the Antioch device.
 ## ===========================
 ##
 ##  Copyright Cypress Semiconductor Corporation, 2006-2009,
 ##  All Rights Reserved
 ##  UNPUBLISHED, LICENSED SOFTWARE.
 ##
 ##  CONFIDENTIAL AND PROPRIETARY INFORMATION
 ##  WHICH IS THE PROPERTY OF CYPRESS.
 ##
 ##  Use of this file is governed
 ##  by the license agreement included in the file
 ##
 ##     <install>/license/license.txt
 ##
 ##  where <install> is the Cypress software
 ##  installation root directory path.
 ##
 ## ===========================
*/

#ifndef _INCLUDED_CYANREG_H_
#define _INCLUDED_CYANREG_H_

#if !defined(__doxygen__)

#define CY_AN_MEM_CM_WB_CFG_ID				(0x80)
#define		CY_AN_MEM_CM_WB_CFG_ID_VER_MASK			(0x000F)
#define		CY_AN_MEM_CM_WB_CFG_ID_HDID_MASK		(0xFFF0)
#define		CY_AN_MEM_CM_WB_CFG_ID_HDID_ANTIOCH_VALUE	(0xA100)
#define		CY_AN_MEM_CM_WB_CFG_ID_HDID_ASTORIA_FPGA_VALUE  (0x6800)
#define		CY_AN_MEM_CM_WB_CFG_ID_HDID_ASTORIA_VALUE       (0xA200)


#define CY_AN_MEM_RST_CTRL_REG				(0x81)
#define		CY_AN_MEM_RST_CTRL_REG_HARD			(0x0003)
#define		CY_AN_MEM_RST_CTRL_REG_SOFT			(0x0001)
#define		CY_AN_MEM_RST_RSTCMPT				(0x0004)

#define CY_AN_MEM_P0_ENDIAN				(0x82)
#define		CY_AN_LITTLE_ENDIAN				(0x0000)
#define		CY_AN_BIG_ENDIAN				(0x0101)

#define CY_AN_MEM_P0_VM_SET				(0x83)
#define		CY_AN_MEM_P0_VM_SET_VMTYPE_MASK			(0x0007)
#define		CY_AN_MEM_P0_VM_SET_VMTYPE_RAM			(0x0005)
#define		CY_AN_MEM_P0_VM_SET_VMTYPE_VMWIDTH		(0x0008)
#define		CY_AN_MEM_P0_VM_SET_VMTYPE_FLOWCTRL		(0x0010)
#define		CY_AN_MEM_P0_VM_SET_IFMODE			(0x0020)
#define		CY_AN_MEM_P0_VM_SET_CFGMODE			(0x0040)
#define		CY_AN_MEM_P0_VM_SET_DACKEOB			(0x0080)
#define		CY_AN_MEM_P0_VM_SET_OVERRIDE			(0x0100)
#define		CY_AN_MEM_P0_VM_SET_INTOVERD			(0x0200)
#define		CY_AN_MEM_P0_VM_SET_DRQOVERD			(0x0400)
#define		CY_AN_MEM_P0_VM_SET_DRQPOL			(0x0800)
#define		CY_AN_MEM_P0_VM_SET_DACKPOL			(0x1000)


#define CY_AN_MEM_P0_NV_SET				(0x84)
#define		CY_AN_MEM_P0_NV_SET_WPSWEN			(0x0001)
#define		CY_AN_MEM_P0_NV_SET_WPPOLAR			(0x0002)

#define CY_AN_MEM_PMU_UPDATE				(0x85)
#define		CY_AN_MEM_PMU_UPDATE_UVALID			(0x0001)
#define		CY_AN_MEM_PMU_UPDATE_USBUPDATE			(0x0002)
#define		CY_AN_MEM_PMU_UPDATE_SDIOUPDATE			(0x0004)

#define CY_AN_MEM_P0_INTR_REG				(0x90)
#define		CY_AN_MEM_P0_INTR_REG_MCUINT			(0x0020)
#define		CY_AN_MEM_P0_INTR_REG_DRQINT			(0x0800)
#define		CY_AN_MEM_P0_INTR_REG_MBINT			(0x1000)
#define		CY_AN_MEM_P0_INTR_REG_PMINT			(0x2000)
#define         CY_AN_MEM_P0_INTR_REG_PLLLOCKINT                (0x4000)

#define CY_AN_MEM_P0_INT_MASK_REG			(0x91)
#define		CY_AN_MEM_P0_INT_MASK_REG_MMCUINT		(0x0020)
#define		CY_AN_MEM_P0_INT_MASK_REG_MDRQINT		(0x0800)
#define		CY_AN_MEM_P0_INT_MASK_REG_MMBINT		(0x1000)
#define		CY_AN_MEM_P0_INT_MASK_REG_MPMINT		(0x2000)
#define         CY_AN_MEM_P0_INT_MASK_REG_MPLLLOCKINT           (0x4000)

#define CY_AN_MEM_MCU_MB_STAT				(0x92)
#define		CY_AN_MEM_P0_MCU_MBNOTRD			(0x0001)

#define CY_AN_MEM_P0_MCU_STAT				(0x94)
#define		CY_AN_MEM_P0_MCU_STAT_CARDINS			(0x0001)
#define		CY_AN_MEM_P0_MCU_STAT_CARDREM			(0x0002)

#define CY_AN_MEM_PWR_MAGT_STAT				(0x95)
#define		CY_AN_MEM_PWR_MAGT_STAT_WAKEUP			(0x0001)

#define CY_AN_MEM_P0_RSE_ALLOCATE			(0x98)
#define		CY_AN_MEM_P0_RSE_ALLOCATE_SDIOAVI		(0x0001)
#define		CY_AN_MEM_P0_RSE_ALLOCATE_SDIOALLO		(0x0002)
#define		CY_AN_MEM_P0_RSE_ALLOCATE_NANDAVI		(0x0004)
#define		CY_AN_MEM_P0_RSE_ALLOCATE_NANDALLO		(0x0008)
#define		CY_AN_MEM_P0_RSE_ALLOCATE_USBAVI		(0x0010)
#define		CY_AN_MEM_P0_RSE_ALLOCATE_USBALLO		(0x0020)

#define CY_AN_MEM_P0_RSE_MASK				(0x9A)
#define		CY_AN_MEM_P0_RSE_MASK_MSDIOBUS_RW		(0x0003)
#define		CY_AN_MEM_P0_RSE_MASK_MNANDBUS_RW		(0x00C0)
#define		CY_AN_MEM_P0_RSE_MASK_MUSBBUS_RW		(0x0030)

#define CY_AN_MEM_P0_DRQ				(0xA0)
#define		CY_AN_MEM_P0_DRQ_EP2DRQ				(0x0004)
#define		CY_AN_MEM_P0_DRQ_EP3DRQ				(0x0008)
#define		CY_AN_MEM_P0_DRQ_EP4DRQ				(0x0010)
#define		CY_AN_MEM_P0_DRQ_EP5DRQ				(0x0020)
#define		CY_AN_MEM_P0_DRQ_EP6DRQ				(0x0040)
#define		CY_AN_MEM_P0_DRQ_EP7DRQ				(0x0080)
#define		CY_AN_MEM_P0_DRQ_EP8DRQ				(0x0100)
#define		CY_AN_MEM_P0_DRQ_EP9DRQ				(0x0200)
#define		CY_AN_MEM_P0_DRQ_EP10DRQ			(0x0400)
#define		CY_AN_MEM_P0_DRQ_EP11DRQ			(0x0800)
#define		CY_AN_MEM_P0_DRQ_EP12DRQ			(0x1000)
#define		CY_AN_MEM_P0_DRQ_EP13DRQ			(0x2000)
#define		CY_AN_MEM_P0_DRQ_EP14DRQ			(0x4000)
#define		CY_AN_MEM_P0_DRQ_EP15DRQ			(0x8000)

#define CY_AN_MEM_P0_DRQ_MASK				(0xA1)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP2DRQ			(0x0004)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP3DRQ			(0x0008)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP4DRQ			(0x0010)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP5DRQ			(0x0020)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP6DRQ			(0x0040)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP7DRQ			(0x0080)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP8DRQ			(0x0100)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP9DRQ			(0x0200)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP10DRQ			(0x0400)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP11DRQ			(0x0800)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP12DRQ			(0x1000)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP13DRQ			(0x2000)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP14DRQ			(0x4000)
#define		CY_AN_MEM_P0_DRQ_MASK_MEP15DRQ			(0x8000)

#define CY_AN_MEM_P0_EP2_DMA_REG			(0xA2)
#define		CY_AN_MEM_P0_EPn_DMA_REG_COUNT_MASK		(0x7FF)
#define		CY_AN_MEM_P0_EPn_DMA_REG_DMAVAL			(1 << 12)
#define CY_AN_MEM_P0_EP3_DMA_REG			(0xA3)
#define CY_AN_MEM_P0_EP4_DMA_REG			(0xA4)
#define CY_AN_MEM_P0_EP5_DMA_REG			(0xA5)
#define CY_AN_MEM_P0_EP6_DMA_REG			(0xA6)
#define CY_AN_MEM_P0_EP7_DMA_REG			(0xA7)
#define CY_AN_MEM_P0_EP8_DMA_REG			(0xA8)
#define CY_AN_MEM_P0_EP9_DMA_REG			(0xA9)
#define CY_AN_MEM_P0_EP10_DMA_REG			(0xAA)
#define CY_AN_MEM_P0_EP11_DMA_REG			(0xAB)
#define CY_AN_MEM_P0_EP12_DMA_REG			(0xAC)
#define CY_AN_MEM_P0_EP13_DMA_REG			(0xAD)
#define CY_AN_MEM_P0_EP14_DMA_REG			(0xAE)
#define CY_AN_MEM_P0_EP15_DMA_REG			(0xAF)

#define CY_AN_MEM_IROS_IO_CFG				(0xC1)
#define 	CY_AN_MEM_IROS_IO_CFG_GPIODRVST_MASK		(0x0003)
#define		CY_AN_MEM_IROS_IO_CFG_GPIOSLEW_MASK		(0x0004)
#define 	CY_AN_MEM_IROS_IO_CFG_PPIODRVST_MASK		(0x0018)
#define 	CY_AN_MEM_IROS_IO_CFG_PPIOSLEW_MASK		(0x0020)
#define		CY_AN_MEM_IROS_IO_CFG_SSIODRVST_MASK		(0x0300)
#define 	CY_AN_MEM_IROS_IO_CFG_SSIOSLEW_MASK		(0x0400)
#define 	CY_AN_MEM_IROS_IO_CFG_SNIODRVST_MASK		(0x1800)
#define		CY_AN_MEM_IROS_IO_CFG_SNIOSLEW_MASK		(0x2000)

#define CY_AN_MEM_PLL_LOCK_LOSS_STAT                    (0xC4)
#define         CY_AN_MEM_PLL_LOCK_LOSS_STAT_PLLSTAT            (0x0800)

#define CY_AN_MEM_P0_MAILBOX0				(0xF0)
#define CY_AN_MEM_P0_MAILBOX1				(0xF1)
#define CY_AN_MEM_P0_MAILBOX2				(0xF2)
#define CY_AN_MEM_P0_MAILBOX3				(0xF3)

#define CY_AN_MEM_MCU_MAILBOX0				(0xF8)
#define CY_AN_MEM_MCU_MAILBOX1				(0xF9)
#define CY_AN_MEM_MCU_MAILBOX2				(0xFA)
#define CY_AN_MEM_MCU_MAILBOX3				(0xFB)

#endif				/* !defined(__doxygen__) */

#endif				/* _INCLUDED_CYANREG_H_ */
