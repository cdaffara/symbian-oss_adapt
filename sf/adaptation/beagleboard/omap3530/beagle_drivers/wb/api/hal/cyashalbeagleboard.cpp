// Copyright (c) 1994-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// omap3530/beagle_drivers/wb/cyashalbeagleboard_spi.cpp
//

#include <kern_priv.h>
#include <beagle/beagle_gpio.h>
#include <beagle/variant.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_irqmap.h> // GPIO interrupts
#include <assp/omap3530_assp/omap3530_gpio.h>

#include <assp.h> // Required for definition of TIsr

#include <cyasregs.h> // Astoria register definitions

#ifdef __CY_ASTORIA_BEAGLEBOARD_SPI__HAL__

#include "cyashalbeagleboard_spi.h"
#include "cyaserr.h"
#include "cyasregs.h"
#include "cyasdma.h"
#include "cyasintr.h"
#ifdef FIRMWARE_NOPPORT

#ifdef OVERCLOCK_SD
#include "cyastfw_sd_mmc_rel_nopport_Ast121_68.h"
#else
#include "cyastfw_sd_mmc_rel_nopport.h"
#endif

#else

#ifdef OVERCLOCK_SD
#include "cyastfw_sd_mmc_rel_silicon_Ast121_68.h"
#else
#include "cyastfw_sd_mmc_rel_silicon.h"
#endif

#endif

#include "cyasusbinit.h"

/*
 * For performance reasons, we handle storage endpoint transfers upto 4 KB
 * within the HAL itself.
 */
#define CYASSTORAGE_WRITE_EP_NUM	(4)
#define CYASSTORAGE_READ_EP_NUM		(8)
#define CYASSTORAGE_MAX_XFER_SIZE	(2*32768)

/*#define MONITOR_THREAD 1*/
#define INT_DE 1

/* DFC queue */
TDfc* gpDfc;

/*
 * The type of DMA operation, per endpoint
 */
typedef enum CyAsHalDmaType
{
    CyAsHalRead,
    CyAsHalWrite,
    CyAsHalNone
} CyAsHalDmaType ;

typedef struct CyAsHalEndpointDma
{
    CyBool buffer_valid ;
    uint16_t *data_p ;
    uint32_t size ;
    /*struct scatterlist* sg_p ;
    uint16_t scatter_index ;*/
    uint32_t bytes_xfered ;
    uint16_t transfer_cnt ;
    CyAsHalDmaType type ;
    CyBool pending ;
} CyAsHalEndpointDma ;

/*
 * The list of OMAP devices (should be one)
 */
static CyAsOmapDevKernel *m_omap_list_p = 0 ;

/*
 * The callback to call after DMA operations are complete
 */
static CyAsHalDmaCompleteCallback callback = 0 ;

/*
 * Pending data size for the endpoints
 */
static CyAsHalEndpointDma EndPoints[16] ;

/* Forward declaration */
static void CyHandleDRQInterrupt(CyAsOmapDevKernel *dev_p) ;

static volatile uint32_t Intr_Disabled = 0 ;
static volatile CyAsHalDeviceTag gDevTag = 0 ;

#define CYAS_INT_MASK (CY_AS_MEM_P0_INTR_REG_MCUINT | CY_AS_MEM_P0_INTR_REG_MBINT | \
                       CY_AS_MEM_P0_INTR_REG_PMINT | CY_AS_MEM_P0_INTR_REG_PLLLOCKINT)
#define CYAS_DRQ_MASK (CY_AS_MEM_P0_INTR_REG_DRQINT)


static void
CyAstoriaISR (
    void *dev_id)
{
#ifdef SPI_DEBUG_LOG
	Kern::Printf("CyAstoriaISR...");
#endif
	gpDfc->Add();

#ifdef SPI_DEBUG_LOG
	Kern::Printf("Disable interrupt");
#endif
	/* Disable Interrupt Here, it will be re-enabled by DFCs */
	GPIO::DisableInterrupt(KGPIO_INT) ;
}

static void
CyAstoriaIntHandler_DFC (
    TAny *aPtr)
{
    CyAsOmapDevKernel *dev_p = (CyAsOmapDevKernel *)aPtr ;
    uint16_t          read_val = 0 ;
#ifdef SPI_DEBUG_LOG
	Kern::Printf("CyAstoriaIntHandler called...");
#endif
    {
		read_val = CyAsHalReadRegister((CyAsHalDeviceTag)dev_p, CY_AS_MEM_P0_INTR_REG) ;
		if (read_val & CYAS_INT_MASK)
		{
			CyAsIntrServiceInterrupt((CyAsHalDeviceTag)dev_p) ;
		}

		if (read_val & CYAS_DRQ_MASK)
		{
			CyHandleDRQInterrupt(dev_p) ;
		}
    }
	GPIO::EnableInterrupt(KGPIO_INT) ;
#ifdef SPI_DEBUG_LOG
	Kern::Printf("Enable interrupt\n");
#endif
}

extern TmtpAstDev * g_pAstDevice ;

static int
CyAsHalConfigureInterrupts (
    void *dev_p,
	void *handler)
{

    return CyAsHalBeagleBoard__SetupISR(handler, dev_p) ;
}
//nxz-debug
void
MyOmapStartIntr(CyAsHalDeviceTag tag)
{
    CyAsOmapDevKernel *dev_p = (CyAsOmapDevKernel *)tag ;
	int ret ;

    const uint16_t mask = CY_AS_MEM_P0_INTR_REG_DRQINT | CY_AS_MEM_P0_INTR_REG_MBINT ;

	/* Setup DFC */
	gpDfc = new TDfc( CyAstoriaIntHandler_DFC, tag, Kern::DfcQue0(), 1 ) ;

    /* Register for interrupts */
    ret = CyAsHalConfigureInterrupts (dev_p,(void*)CyAstoriaISR) ;
	if ( ret != 0 )
		Kern::Printf("ERROR: CyAsHalConfigureInterrupts failed\n");

    /* enable only MBox & DRQ interrupts for now */
    CyAsHalWriteRegister((CyAsHalDeviceTag)dev_p, CY_AS_MEM_P0_INT_MASK_REG, mask) ;
}

volatile CyBool interrupt_fired = CyFalse;

//This is the Diagnostics Interrupt Handler
void
MyIntTestHandler(
	void *dev_id)
{
    CyAsOmapDevKernel * dev_tag = (CyAsOmapDevKernel*) dev_id ;

	Kern::Printf("Diag: CyAsDiagIntHandler called\n");

	if(interrupt_fired == CyFalse)
	{
			Kern::Printf("Diag: CyAsDiagIntHandler, first instance of INT# \n");
			interrupt_fired = CyTrue;

			//return INT to high state, only called for override testing
			CyAsHalWriteRegister(dev_tag, CY_AS_MEM_P0_VM_SET, 0x0745);
	}
	//return INT to high state, only called for override testing
	CyAsHalWriteRegister(dev_tag, CY_AS_MEM_P0_VM_SET, 0x0745);

}



/*
* These are the functions that are not part of the HAL layer, but are required to be called
* for this HAL.
*/
int StartOMAPKernel(const char *pgm, CyAsHalDeviceTag *tag, CyBool debug)
{
    CyAsOmapDevKernel *dev_p ;
    CyAsHalSleepChannel channel ;
    int i;

    (void)debug ; /* No debug mode support as of now */

    Kern::Printf ("<1>startOMAPKernel called\n");

    /*
     * Initialize the HAL level endpoint DMA data.
     */
    for(i = 0 ; i < sizeof(EndPoints)/sizeof(EndPoints[0]) ; i++)
    {
        EndPoints[i].data_p = 0 ;
        EndPoints[i].pending = CyFalse ;
        EndPoints[i].size = 0 ;
        EndPoints[i].type = CyAsHalNone ;
		EndPoints[i].bytes_xfered = 0 ;
		EndPoints[i].transfer_cnt = 0 ;
    }

    dev_p = (CyAsOmapDevKernel *)CyAsHalAlloc(sizeof(CyAsOmapDevKernel)) ;
    if (dev_p == 0)
    {
		Kern::Printf("<1>%s: out of memory allocating OMAP device structure\n", pgm) ;
		return 0 ;
    }
    dev_p->m_sig = CY_AS_OMAP_KERNEL_HAL_SIG ;
    dev_p->m_addr_base = 0 ;

    CyAsHalBeagleBoard__ConfigureSPI();

#ifdef HW_TEST
{
    uint16_t readVal = 0 ;
	uint16_t timeOutCounter = 0 ;
	uint16_t errCnt = 0 ;

	Kern::Printf( "<1> Regsiter Test ------------->\n") ;

	readVal = CyAsHalReadRegister(dev_p, CY_AS_MEM_CM_WB_CFG_ID) ;
	Kern::Printf("ID reg 0x%04x\n",readVal);
        if ( readVal != 0xa200 )
	{
		Kern::Printf("ERROR: Wrong Antioch ID reg value\n");
		Kern::Printf("ERROR: Exp:  0x%04x\n",0xa200);
		Kern::Printf("ERROR: Act:  0x%04x\n",readVal);
		errCnt++;
	}

	readVal = CyAsHalReadRegister(dev_p, CY_AS_MEM_P0_VM_SET) ;
	Kern::Printf("P0_VM_SET 0x%04x\n",readVal);
	if ( readVal != 0x45 )
	{
		Kern::Printf("ERROR: Wrong Antioch P0_VM_SET reg value\n");
		Kern::Printf("ERROR: Exp:  0x%04x\n",0x45);
		Kern::Printf("ERROR: Act:  0x%04x\n",readVal);
		errCnt++;
	}

	readVal = CyAsHalReadRegister(dev_p, CY_AS_MEM_P0_RSE_ALLOCATE) ;
	Kern::Printf("P0_RSE_ALLOCATE 0x%04x\n",readVal);
	if ( readVal != 0x26 )
	{
		Kern::Printf("ERROR: Wrong Antioch P0_RSE_ALLOCATE reg value\n");
		Kern::Printf("ERROR: Exp:  0x%04x\n",0x26);
		Kern::Printf("ERROR: Act:  0x%04x\n",readVal);
		errCnt++;
	}

	CyAsHalWriteRegister(dev_p, CY_AS_MEM_MCU_MAILBOX0, 0x0);
	CyAsHalWriteRegister(dev_p, CY_AS_MEM_MCU_MAILBOX1, 0xFFFF);
	CyAsHalWriteRegister(dev_p, CY_AS_MEM_MCU_MAILBOX2, 0xAAAA);
	CyAsHalWriteRegister(dev_p, CY_AS_MEM_MCU_MAILBOX3, 0x5555);

	readVal = CyAsHalReadRegister(dev_p, CY_AS_MEM_MCU_MAILBOX0) ;
	Kern::Printf("mailbox0 0x%04x\n",readVal);
	if ( readVal != 0x0 )
	{
		Kern::Printf("ERROR: Wrong Antioch MAILBOX0 reg value\n");
		Kern::Printf("ERROR: Exp:  0x%04x\n",0x0);
		Kern::Printf("ERROR: Act:  0x%04x\n",readVal);
		errCnt++;
	}

	readVal = CyAsHalReadRegister(dev_p, CY_AS_MEM_MCU_MAILBOX1) ;
	Kern::Printf("mailbox1 0x%04x\n",readVal);
	if ( readVal != 0xffff )
	{
		Kern::Printf("ERROR: Wrong Antioch MAILBOX1 reg value\n");
		Kern::Printf("ERROR: Exp:  0x%04x\n",0xffff);
		Kern::Printf("ERROR: Act:  0x%04x\n",readVal);
		errCnt++;
	}

	readVal = CyAsHalReadRegister(dev_p, CY_AS_MEM_MCU_MAILBOX2) ;
    Kern::Printf("mailbox2 0x%04x\n",readVal);
	if ( readVal != 0xaaaa )
	{
		Kern::Printf("ERROR: Wrong Antioch MAILBOX2 reg value\n");
		Kern::Printf("ERROR: Exp:  0x%04x\n",0xaaaa);
		Kern::Printf("ERROR: Act:  0x%04x\n",readVal);
		errCnt++;
	}

	readVal = CyAsHalReadRegister(dev_p, CY_AS_MEM_MCU_MAILBOX3) ;
    Kern::Printf("mailbox3 0x%04x\n",readVal);
	if ( readVal != 0x5555 )
	{
		Kern::Printf("ERROR: Wrong Antioch MAILBOX3 reg value\n");
		Kern::Printf("ERROR: Exp:  0x%04x\n",0x5555);
		Kern::Printf("ERROR: Act:  0x%04x\n",readVal);
		errCnt++;
	}

	Kern::Printf( "<1> Interrupt Test ------------->\n") ;
	Kern::Printf("Checking that INT# can execute Procesor ISR:\n");

	CyAsHalConfigureInterrupts (dev_p,(void*)MyIntTestHandler) ;

	//overrid default INT# value, cause Astoria to assert INT#
	CyAsHalWriteRegister(dev_p, CY_AS_MEM_P0_VM_SET, 0x0545);

	do {
		timeOutCounter++;
	}while((interrupt_fired == CyFalse) && (timeOutCounter < 255));

	if(interrupt_fired == CyTrue)
	{
		Kern::Printf("INT# fired Processor ISR\n");
	}
	else
	{
		Kern::Printf("ERROR: INT# did not fire processor ISR %i\n", interrupt_fired);
		errCnt++;
	}

	if ( errCnt == 0 )
	{
		Kern::Printf("HW test passed!!\n") ;
	}
	else
		Kern::Printf("HW test failed (%d)\n",errCnt) ;

	Kern::Printf("Reset Astoria\n") ;
	CyAsHalWriteRegister(dev_p, CY_AS_MEM_RST_CTRL_REG, CY_AS_MEM_RST_CTRL_REG_HARD) ;
	return 0 ;
}
#endif

    /*
     * Now perform a hard reset of the device to have the new settings take
     * effect
     */
    Kern::Printf("<1>West Bridge Device Enters hard reset\n") ;
    CyAsHalWriteRegister(dev_p, CY_AS_MEM_RST_CTRL_REG, CY_AS_MEM_RST_CTRL_REG_HARD) ;

    /*
     * Sleep for 100 ms to be sure reset is complete.
     */
    Kern::Printf("<1>Sleep 100ms\n") ;
    CyAsHalCreateSleepChannel (&channel) ;
    CyAsHalSleepOn(&channel, 100) ;
    CyAsHalDestroySleepChannel(&channel) ;
	Kern::Printf("<1>MyOmapStartIntr\n") ;
    MyOmapStartIntr(dev_p);

    dev_p->m_next_p = m_omap_list_p ;
    m_omap_list_p = dev_p ;

    *tag = dev_p ;
    gDevTag = dev_p ;

    return 1 ;
}

int StopOMAPKernel(const char *pgm, CyAsHalDeviceTag tag)
{
    CyAsOmapDevKernel *dev_p = (CyAsOmapDevKernel *)tag ;

    if (0 == dev_p)
		return 1 ;

    Kern::Printf ("<1>StopOMAPKernel called\n");
    if (dev_p->m_sig != CY_AS_OMAP_KERNEL_HAL_SIG)
    {
		Kern::Printf("<1>%s: %s: bad TAG parameter passed to function\n", pgm, __FUNCTION__) ;
		return 1 ;
    }

    Kern::Printf ("<1>West Bridge OMAP Kernel: Done cleaning thread\n") ;
	/* TODO: */

    CyAsHalFree(dev_p) ;

    return 1 ;
}


/*************************************************************************************************
*
* Below are the functions that communicate with the West Bridge device.  These are system dependent
* and must be defined by the HAL layer for a given system.
*
*************************************************************************************************/

void
CyAsHalWriteEP(CyAsHalDeviceTag tag, uint16_t addr, uint8_t* data, uint16_t size)
{
    CyAsOmapDevKernel *dev_p = (CyAsOmapDevKernel *)tag ;
GPIO::DisableInterrupt(KGPIO_INT) ;

    /* Do additional error checks while in debug mode. */
    if (dev_p->m_sig != CY_AS_OMAP_KERNEL_HAL_SIG)
    {
		Kern::Printf("<1>%s: bad TAG parameter passed to function\n",  __FUNCTION__) ;
		return ;
    }

    if (addr & 0x80)
    {
		Kern::Printf ("<1>West Bridge write EP address 0x%x out of range\n", addr);
		return ;
    }

	CyAsHalBeagleBoardMcSPI4Ch0_WriteEP(addr, data, size) ;
GPIO::EnableInterrupt(KGPIO_INT) ;
    return ;
}

void
CyAsHalReadEP(CyAsHalDeviceTag tag, uint16_t addr, uint8_t* data, uint16_t size)
{
    CyAsOmapDevKernel *dev_p = (CyAsOmapDevKernel *)tag ;
GPIO::DisableInterrupt(KGPIO_INT) ;

    /* Do additional error checks while in debug mode. */
    if (dev_p->m_sig != CY_AS_OMAP_KERNEL_HAL_SIG)
    {
		Kern::Printf("<1>%s: bad TAG parameter passed to function\n",  __FUNCTION__) ;
		return ;
    }

    if (addr & 0x80)
    {
		Kern::Printf ("<1>West Bridge write EP address 0x%x out of range\n", addr);
		return ;
    }

	CyAsHalBeagleBoardMcSPI4Ch0_ReadEP(addr, data, size) ;
GPIO::EnableInterrupt(KGPIO_INT) ;
    return ;
}

/*
* This function must be defined to write a register within the West Bridge
* device.  The addr value is the address of the register to write with
* respect to the base address of the West Bridge device.
*/
void
CyAsHalWriteRegister(CyAsHalDeviceTag tag, uint16_t addr, uint16_t data)
{
    CyAsOmapDevKernel *dev_p = (CyAsOmapDevKernel *)tag ;
GPIO::DisableInterrupt(KGPIO_INT) ;

    /* Do additional error checks while in debug mode. */
    if (dev_p->m_sig != CY_AS_OMAP_KERNEL_HAL_SIG)
    {
		Kern::Printf("<1>%s: bad TAG parameter passed to function\n",  __FUNCTION__) ;
		return ;
    }

    if (addr > CYAS_DEV_MAX_ADDR)
    {
		Kern::Printf ("<1>West Bridge write address 0x%x out of range\n", addr);
		return ;
    }

	CyAsHalBeagleBoardMcSPI4Ch0_WriteReg((TUint32) addr, (TUint16) data) ;
#ifdef SPI_DEBUG_LOG
	Kern::Printf("CyAsHalWriteRegister(0x%02x,0x%04x)\n",addr,data) ;
#endif
GPIO::EnableInterrupt(KGPIO_INT) ;
    return ;

}

/*
* This function must be defined to read a register from the West Bridge
* device.  The addr value is the address of the register to read with
* respect to the base address of the West Bridge device.
*/
uint16_t
CyAsHalReadRegister(CyAsHalDeviceTag tag, uint16_t addr)
{
    uint16_t data  = 0 ;
    CyAsOmapDevKernel *dev_p = (CyAsOmapDevKernel *)tag ;
GPIO::DisableInterrupt(KGPIO_INT) ;

    if (dev_p->m_sig != CY_AS_OMAP_KERNEL_HAL_SIG)
    {
		Kern::Printf("<1>%s: bad TAG parameter passed to function\n",  __FUNCTION__) ;
		return 0 ;
    }

    if (addr > CYAS_DEV_MAX_ADDR)
    {
		Kern::Printf("<1>West Bridge read address 0x%x out of range\n", addr) ;
		return 0 ;
    }

    CyAsHalBeagleBoardMcSPI4Ch0_ReadReg((TUint32) addr, (TUint16*) &data) ;
#ifdef SPI_DEBUG_LOG
    Kern::Printf("CyAsHalReadRegister(0x%02x,0x%04x)\n",addr,data) ;
#endif
GPIO::EnableInterrupt(KGPIO_INT) ;
    return data ;
}

static void
CyServiceEPDmaReadRequest(CyAsOmapDevKernel *dev_p, uint8_t ep)
{
    CyAsHalDeviceTag tag = (CyAsHalDeviceTag)dev_p ;
    uint16_t  v ;
    uint32_t  size ;
    uint16_t  addr = CY_AS_MEM_P0_EP2_DMA_REG + ep - 2 ;
    uint8_t *dptr = (uint8_t*)EndPoints[ep].data_p ;


#ifdef SPI_DEBUG_LOG
	Kern::Printf("CyServiceEPDmaReadRequest...");
#endif
    /* Read the amount of data available */
    v = CyAsHalReadRegister(tag, addr) ;
    size =  v & CY_AS_MEM_P0_EPn_DMA_REG_COUNT_MASK ;

	CyAsHalReadEP(tag, ep, dptr, size) ;

    /*
     * Now, reset the DMAVAL bit indicating that the data has been read
     */

    CyAsHalWriteRegister(tag, addr, 0) ;

    EndPoints[ep].pending      = CyFalse ;
    EndPoints[ep].type         = CyAsHalNone ;
    EndPoints[ep].buffer_valid = CyFalse ;

    if (callback)
		callback(tag, ep, size, CY_AS_ERROR_SUCCESS) ;
}

static void
CyServiceEPDmaWriteRequest(CyAsOmapDevKernel *dev_p, uint8_t ep)
{
    CyAsHalDeviceTag tag = (CyAsHalDeviceTag)dev_p ;
    uint16_t  addr = CY_AS_MEM_P0_EP2_DMA_REG + ep - 2 ;
    uint8_t *dptr = (uint8_t *)EndPoints[ep].data_p ;
    uint32_t  size ;


#ifdef SPI_DEBUG_LOG
	Kern::Printf("CyServiceEPDmaWriteRequest...");
#endif

    if ( ep == CYASSTORAGE_WRITE_EP_NUM )
    {
		if ( EndPoints[ep].size == 0x180 )
		{
			size = 0x180 ;
		}
		else
			size = 0x200 ;
    }
    else
		size = EndPoints[ep].size ;
#ifdef SPI_DEBUG_LOG
    Kern::Printf("Service DMA write request EP%d, size=%d, ep.size=%d\n",ep,size,EndPoints[ep].size);
#endif
	#if 1
	CyAsHalWriteEP(tag, ep, dptr, size) ;
	#else
	{
		uint16_t v ;
		int i ;
 		for(i = 0 ; i < size / 2 ; i++)
		{
			v = dptr[0] | (dptr[1] << 8) ;
			CyAsHalWriteRegister(tag, ep, v) ;
			dptr++ ;
			dptr++ ;
		}
	}
	#endif

    /*
     * Now, write the DMAVAL bit to indicate we are done transferring data and that the data
     * can now be sent via USB to the USB host, sent to storage, or used internally.
     */
    CyAsHalWriteRegister(tag, addr, size) ;

    /* We have sent the data, mark it as false */
    EndPoints[ep].pending = CyFalse ;
    EndPoints[ep].type     = CyAsHalNone ;
    EndPoints[ep].buffer_valid = CyFalse ;

    /*
     * Finally, tell the USB subsystem that the data is gone and we can accept the
     * next request if one exists.
     */
    if (callback)
		callback(tag, ep, size, CY_AS_ERROR_SUCCESS) ;
}

static void
CyHandleDRQInterrupt(CyAsOmapDevKernel *dev_p)
{
    uint16_t v ;
    static uint8_t service_ep = 2 ;
    static CyBool indrq = CyFalse ;

    /*Kern::Printf("<1>DRQ interrupt detected\n") ;*/
    if (indrq)
    {
#ifndef NDEBUG

	Kern::Printf("<1>+++++++++  Nested DRQ interrupt detected\n") ;
        v = CyAsHalReadRegister((CyAsHalDeviceTag)dev_p, CY_AS_MEM_P0_INTR_REG) ;
	Kern::Printf("<1>+++++++++ INTR_REG = %04x\n",v) ;
    	v = CyAsHalReadRegister((CyAsHalDeviceTag)dev_p, CY_AS_MEM_P0_DRQ) ;
	Kern::Printf("<1>+++++++++ DRQ_REG = %04x\n",v) ;
#endif
	return ;
    }

    indrq = CyTrue ;

    /*
     * We have received a DMA interrupt
     */
    v = CyAsHalReadRegister((CyAsHalDeviceTag)dev_p, CY_AS_MEM_P0_DRQ) ;
    if (v == 0)
    {
#ifndef NDEBUG
	/*Stray DRQ is possible because we will check DRQ register before exit handler*/
	Kern::Printf("<1>+++++++++  Stray DRQ interrupt detected\n") ;
#endif
	indrq = CyFalse;
	return ;
    }

    /*
     * Now, pick a given DMA request to handle, for now, we just go round robin.  Each bit
     * position in the service_mask represents an endpoint from EP2 to EP15.  We rotate through
     * each of the endpoints to find one that needs to be serviced.
     */
    while ((v & (1 << service_ep)) == 0)
    {
	if (service_ep == 15)
	    service_ep = 2 ;
	else
	    service_ep++ ;
    }

    if ((v & (1 << service_ep)) == 0)
    {
	Kern::Printf("<1>+++++++++  Internal error, this should not happen\n") ;
	indrq = CyFalse;
	return ;
    }

    if (EndPoints[service_ep].type == CyAsHalWrite)
		CyServiceEPDmaWriteRequest(dev_p, service_ep) ;
    else if (EndPoints[service_ep].type == CyAsHalRead)
		CyServiceEPDmaReadRequest(dev_p, service_ep) ;

#ifndef NDEBUG
    else
    {
        Kern::Printf("+++++++ Interrupt occurred, but there is no DMA operation pending - check DRQ_MASK logic\n") ;
    }
#endif

    /* Now bump the EP ahead, so other endpoints get a shot before the one we just serviced */
    if (EndPoints[service_ep].type == CyAsHalNone)
    {
	if (service_ep == 15)
	    service_ep = 2 ;
	else
	    service_ep++ ;
    }

    indrq = CyFalse ;
}

void
CyAsHalDmaCancelRequest(CyAsHalDeviceTag tag, uint8_t ep)
{
    if (EndPoints[ep].pending)
        CyAsHalWriteRegister(tag, CY_AS_MEM_P0_EP2_DMA_REG + ep - 2, 0) ;

    EndPoints[ep].buffer_valid = CyFalse ;
}

/*
* This function must be defined to transfer a block of data to the West Bridge device.  This
* function can use the burst write (DMA) capabilities of West Bridge to do this, or it can
* just copy the data using writes.
*/
void
CyAsHalDmaSetupWrite(CyAsHalDeviceTag tag, uint8_t ep, void *buf, uint32_t size, uint16_t maxsize)
{
    uint32_t addr ;
    uint16_t v ;
#ifdef SPI_DEBUG_LOG
	Kern::Printf("CyAsHalDmaSetupWrite (%d,%d)",size,maxsize);
#endif
    /* No EP0 or EP1 traffic should get here */
    CyAsHalAssert(ep != 0 && ep != 1) ;

    /*
     * If this asserts, we have an ordering problem.  Another DMA request is coming down
     * before the previous one has completed.
     */
    CyAsHalAssert(EndPoints[ep].buffer_valid == CyFalse) ;

    /*
     * Store the data for the interrupt service routine
     */
    EndPoints[ep].buffer_valid = CyTrue ;
    EndPoints[ep].data_p = (uint16_t*)buf ;
    EndPoints[ep].size = size ;
    EndPoints[ep].type = CyAsHalWrite ;

    /*
     * Tell West Bridge we are ready to send data on the given endpoint
     */
    {
		addr = CY_AS_MEM_P0_EP2_DMA_REG + ep - 2 ;
		v = (size & CY_AS_MEM_P0_EPn_DMA_REG_COUNT_MASK) | CY_AS_MEM_P0_EPn_DMA_REG_DMAVAL ;
    }

    CyAsHalWriteRegister(tag, addr, v) ;

}

/*
* This function must be defined to transfer a block of data from the West Bridge device.  This
* function can use the burst read (DMA) capabilities of West Bridge to do this, or it can just
* copy the data using reads.
*/
void
CyAsHalDmaSetupRead(CyAsHalDeviceTag tag, uint8_t ep, void *buf, uint32_t size, uint16_t maxsize)
{
    uint32_t addr ;
    uint16_t v ;
#ifdef SPI_DEBUG_LOG
	Kern::Printf("CyAsHalDmaSetupRead (%d,%d)",size,maxsize);
#endif
    /* No EP0 or EP1 traffic should get here */
    CyAsHalAssert(ep != 0 && ep != 1) ;
    CyAsHalAssert(EndPoints[ep].buffer_valid == CyFalse) ;

    EndPoints[ep].buffer_valid = CyTrue ;
    EndPoints[ep].data_p = (uint16_t*)buf ;
    EndPoints[ep].size = size ;
    EndPoints[ep].type = CyAsHalRead ;

    /*
     * Program the EP DMA register for Storage endpoints only.
     */
    if (ep == 2)
    {
		if (size == 1)
		  size = 2 ;

		addr = CY_AS_MEM_P0_EP2_DMA_REG + ep - 2 ;
		v = (size & CY_AS_MEM_P0_EPn_DMA_REG_COUNT_MASK) | CY_AS_MEM_P0_EPn_DMA_REG_DMAVAL;
		CyAsHalWriteRegister(tag, addr, v );
    }
    else if (ep == CYASSTORAGE_READ_EP_NUM)
    {
        addr = CY_AS_MEM_P0_EP8_DMA_REG ;

		/* This transfer is always done in 512 byte chunks. */
        v = 0x200 | CY_AS_MEM_P0_EPn_DMA_REG_DMAVAL ;
        CyAsHalWriteRegister(tag, addr, v) ;
	}
}

/*
* This function must be defined to allow the West Bridge API to register a callback function that is
* called when a DMA transfer is complete.
*/
void
CyAsHalDmaRegisterCallback(CyAsHalDeviceTag tag, CyAsHalDmaCompleteCallback cb)
{
    callback = cb ;
}

/*
* This function must be defined to return the maximum size of DMA request that can be handled
* on the given endpoint.  The return value should be the maximum size in bytes that the DMA
* module can handle.
*/
uint32_t
CyAsHalDmaMaxRequestSize(CyAsHalDeviceTag tag, CyAsEndPointNumber_t ep)
{
    /* Storage reads and writes are always done in 512 byte blocks. So, we do the count
       handling within the HAL, and save on some of the data transfer delay.
     */
    if ((ep == CYASSTORAGE_READ_EP_NUM) || (ep == CYASSTORAGE_WRITE_EP_NUM))
	return CYASSTORAGE_MAX_XFER_SIZE;
    else
	/*
	 * For the USB - Processor endpoints, the maximum transfer size depends on
	 * the speed of USB operation. So, we use the following constant to
	 * indicate to the API that splitting of the data into chunks less than
	 * or equal to the max transfer size should be handled internally.
	 */
	return CY_AS_DMA_MAX_SIZE_HW_SIZE;
}

/*
 * This function must be defined to set the state of the WAKEUP pin on the West Bridge device.  Generally
 * this is done via a GPIO of some type.
 */
CyBool
CyAsHalSetWakeupPin(CyAsHalDeviceTag tag, CyBool state)
{
    /* Not supported as of now. */
    return CyFalse ;
}

void
CyAsHalPllLockLossHandler(CyAsHalDeviceTag tag)
{
    Kern::Printf("Error: West Bridge PLL has lost lock\n") ;
    Kern::Printf("Please check the input voltage levels and clock, and restart the system\n") ;
}

#define CYASHAL_REGISTERS_TO_SAVE_COUNT         (12)
static uint16_t gAstoriaRegCache[CYASHAL_REGISTERS_TO_SAVE_COUNT][2] = {
    {CY_AS_MEM_P0_ENDIAN,        0x0000},
    {CY_AS_MEM_PMU_UPDATE,       0x0000},
    {CY_AS_MEM_P0_VM_SET,        0x0000},
    {CY_AS_MEM_P0_INT_MASK_REG,  0x0000},
    {CY_AS_MEM_P0_RSE_ALLOCATE,  0x0000},
    {CY_AS_MEM_P0_RSE_MASK,      0x0000},
    {CY_AS_MEM_P0_DRQ_MASK,      0x0000},
    {CY_AS_MEM_IROS_SLB_DATARET, 0x0000},
    {CY_AS_MEM_IROS_IO_CFG,      0x0000},
    {CY_AS_MEM_IROS_PLL_CFG,     0x0000},
    {CY_AS_MEM_IROS_PXB_DATARET, 0x0000},
    {CY_AS_MEM_IROS_SLEEP_CFG,   0x0000}
};

void
CyAsHalReadRegsBeforeStandby (
        CyAsHalDeviceTag tag)
{
    CyAsOmapDevKernel *dev_p = (CyAsOmapDevKernel *)tag;
    int i, pos;

    if (dev_p->m_sig != CY_AS_OMAP_KERNEL_HAL_SIG)
    {
        Kern::Printf("%s: bad TAG parameter passed to function\n", __FUNCTION__);
        return;
    }

    {
        for (i = 0; i < CYASHAL_REGISTERS_TO_SAVE_COUNT; i++)
        {
            gAstoriaRegCache[i][1] = CyAsHalReadRegister (tag, gAstoriaRegCache[i][0]);

            /* Special handling required for the RSE_ALLOCATE register, so as to
               ensure that the SD_IO pin ownership is set correctly.
             */
            if (gAstoriaRegCache[i][0] == CY_AS_MEM_P0_RSE_ALLOCATE)
            {
                /* For each of the 4 two bits fields in the register, set it to 'b01 if the
                 * resource is currently allocated to the P port, and 'b10 if it is currently
                 * allocated to Astoria.
                 */
                for (pos = 0; pos < 8; pos+= 2)
                {
                    if (((gAstoriaRegCache[i][1] >> pos) & 0x03) == 0x03)
                        gAstoriaRegCache[i][1] = (gAstoriaRegCache[i][1] & (0xFF ^ (0x03 << pos))) | (0x01 << pos);
                    else
                        gAstoriaRegCache[i][1] = (gAstoriaRegCache[i][1] & (0xFF ^ (0x03 << pos))) | (0x02 << pos);
                }
            }
        }
    }
}

void
CyAsHalRestoreRegsAfterStandby (
        CyAsHalDeviceTag tag)
{
    CyAsOmapDevKernel *dev_p = (CyAsOmapDevKernel *)tag;
    int i;

    if (dev_p->m_sig != CY_AS_OMAP_KERNEL_HAL_SIG)
    {
        Kern::Printf("%s: bad TAG parameter passed to function\n", __FUNCTION__);
        return;
    }

    {
        /* Write 0xFF into the mask register so that all fields can be updated.
           The mask will get updated to its proper value later.
         */
        CyAsHalWriteRegister (tag, CY_AS_MEM_P0_RSE_MASK, 0xFF);
        for (i = 0; i < CYASHAL_REGISTERS_TO_SAVE_COUNT; i++)
        {
            CyAsHalWriteRegister (tag, gAstoriaRegCache[i][0], gAstoriaRegCache[i][1]);
        }
    }
}

void
CyAsHalInitDevRegisters(
	CyAsHalDeviceTag tag,
	CyBool           is_standby_wakeup)
{
	(void)tag;
	(void)is_standby_wakeup;
	Kern::Printf("CyAsHalInitDevRegisters...");
	return ;
}


void
CyAsHalPrintMessage2(const char* msg)
{
	Kern::Printf("%s",msg);
}

/*************************************************************************************************
*
* Below are the functions that must be defined to provide the basic operating system services
* required by the API.
*
*************************************************************************************************/

/*
 * This function is required by the API to allocate memory.  This function is expected to work
 * exactly like malloc().
 */
void *
CyAsHalAlloc(uint32_t cnt)
{
    void *ret_p ;
    /*ret_p = kmalloc(cnt, GFP_KERNEL) ;*/
    ret_p = (void*) new TUint8[cnt];
    return ret_p ;
}

/*
 * This function is required by the API to free memory allocated with CyAsHalAlloc().  This function is
 * expected to work exacly like free().
 */
void
CyAsHalFree(void *mem_p)
{
    delete [] (mem_p) ;
}

/*
 * Allocator that can be used in interrupt context. We have to ensure that the kmalloc
 * call does not sleep in this case.
 */
void *
CyAsHalCBAlloc(uint32_t cnt)
{
    void *ret_p ;

	/* TODO:
    ret_p = kmalloc(cnt, GFP_ATOMIC) ;*/
	ret_p = (void*) new TUint8[cnt];
    return ret_p ;
}

/*
 * This function is required to set a block of memory to a specific value.  This function is
 * expected to work exactly like memset()
 */
void
CyAsHalMemSet(void *ptr, uint8_t value, uint32_t cnt)
{
	/*uint32_t i ;
	uint8_t* b = (uint8_t*) ptr ;
	for ( i = 0 ; i < cnt ; i++ )
		*b = value ;
	 TODO: */
    memset(ptr, value, cnt) ;
}


/*
 * This function is expected to create a sleep channel.  The data structure that represents the
 * sleep channel is given by the pointer in the argument.
 */
CyBool
CyAsHalCreateSleepChannel(CyAsHalSleepChannel *channel)
{
    /* TODO:
    init_waitqueue_head(&channel->wq) ;
    return CyTrue ;*/
    if (channel)
    	channel->wq = 0 ;
    return CyTrue ;
}

/*
 * This function is expected to destroy a sleep channel.  The data structure that represents the
 * sleep channel is given by the pointer in the argument.
 */
CyBool
CyAsHalDestroySleepChannel(CyAsHalSleepChannel *channel)
{
	/* TODO:
	return CyTrue ;*/
	channel->wq =1 ;
    return CyTrue ;
}

CyBool
CyAsHalSleepOn(CyAsHalSleepChannel *channel, uint32_t ms)
{
    /* TODO:
    interruptible_sleep_on_timeout (&channel->wq, (ms * HZ/1000)) ;
    return CyTrue ;*/

    NKern::Sleep(ms);
    return CyTrue;
}

CyBool
CyAsHalWake(CyAsHalSleepChannel *channel)
{

    /* TODO:
    wake_up (&channel->wq) ;
    return CyTrue ;*/
    channel->wq = 1 ;
    return CyTrue ;

}

uint32_t
CyAsHalDisableInterrupts()
{
    uint16_t v = CyAsHalReadRegister(gDevTag,CY_AS_MEM_P0_INT_MASK_REG);
    if ( !Intr_Disabled )
    {
        CyAsHalWriteRegister(gDevTag,CY_AS_MEM_P0_INT_MASK_REG,0);
    }
    Intr_Disabled++ ;
	return (uint32_t)v ;
}

void
CyAsHalEnableInterrupts(uint32_t val)
{
    Intr_Disabled-- ;
    if ( !Intr_Disabled )
    {
        val = CYAS_INT_MASK | CYAS_DRQ_MASK ;
        CyAsHalWriteRegister(gDevTag,CY_AS_MEM_P0_INT_MASK_REG,(uint16_t)val);
        /*CyAsHalWriteRegister(gDevTag,CY_AS_MEM_P0_INT_MASK_REG,0x1800);*/
    }
}

void
CyAsHalSleep150 (void) /* Sleep atleast 150ns */
{
    uint32_t i, j;

    j = 0;
    for (i = 0; i < 100; i++)
	j += (~i);
}

int j = 0;
void
CyAsHalSleep(uint32_t ms)
{
    int i;

    while (ms--)
    {
	i = 60000;
	while (i--)
	{
	    j += ((i * ~i) + 3 * i + 79);
	}
    }
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
int gInitComplete = 0 ;

/*
 * This callback function is called for events from the MISC module.  The primary event
 * processed by this function is the CyAsEventMiscInitialized which is sent once the
 * West Bridge device is initialized and ready to accept requests.
 */
static void
CyMiscCallback(CyAsDeviceHandle h, CyAsMiscEventType evtype, void *evdata)
{
    (void)h ;
    (void)evdata ;

    switch(evtype)
    {
    case CyAsEventMiscInitialized:
    	Kern::Printf("Firmware initialized");
        gInitComplete = CyTrue ;
        break ;
    default:
        break ;
    }
}

int
CyAsHalAstoriaInit(void)
{
 	uint32_t ret = 0 ;
 	char* pgm = "Beagleboard HAL";
    CyAsDeviceHandle dev_handle ;
    CyAsDeviceConfig config ;
    CyAsHalDeviceTag tag ;

    if (!StartOMAPKernel(pgm, &tag, CyFalse))
    {
#ifndef HW_TEST
		Kern::Printf("ERROR: Cannot start OMAPKernel\n") ;
#endif
		return 1 ;
	}

    /*
     * Create a handle to a West Bridge device.  It tag is used to identifiy the specific hardware
     * we are talking to.
     */
    Kern::Printf("** Create API device\n") ;
    ret = CyAsMiscCreateDevice(&dev_handle, tag) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        Kern::Printf("%s: cannot initailize the West Bridge API - code %d\n", pgm, ret) ;
        return 1 ;
    }
    Kern::Printf("** API device created\n") ;

    /*
     * Configure the physical bus so we can talk to the West Bridge device.  This is the first required
     * API call after creating a device.
     */
    Kern::Printf("** API device configuring...\n");
    memset(&config, 0, sizeof(config)) ;
    ret = CyAsMiscConfigureDevice(dev_handle, &config) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        Kern::Printf("%s: cannot configure the West Bridge device - code %d\n", pgm, ret) ;
        return 1 ;
    }
    Kern::Printf("** API device configured\n") ;

    /*
     * Register a callback to process events from the MISC module.
     */
    Kern::Printf("** Register Callback...\n") ;
    ret = CyAsMiscRegisterCallback(dev_handle, CyMiscCallback) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        Kern::Printf("%s: cannot configure the West Bridge device - code %d\n", pgm, ret) ;
        return 1 ;
    }
	Kern::Printf("** Register Callback done\n") ;
	/*
	 * Download the firmware to the device.  Until the firmware has been downloaded, the West Bridge
	 * device cannot response to requests from the P port processor.
	 */
	Kern::Printf("** Download firmware...\n") ;
#ifdef FIRMWARE_NOPPORT

#ifdef OVERCLOCK_SD
	ret = CyAsMiscDownloadFirmware(dev_handle, CyAnFirmware.fw_image,
			(uint16_t)CYANFW_SIZE, 0, 0) ;
#else
	ret = CyAsMiscDownloadFirmware(dev_handle, cyastfw_sd_mmc_rel_nopport_array.fw_image,
			(uint16_t)CYASTFW_SD_MMC_REL_NOPPORT_SIZE, 0, 0) ;
#endif

#else

#ifdef OVERCLOCK_SD
	ret = CyAsMiscDownloadFirmware(dev_handle, CyAnFirmware.fw_image,
			(uint16_t)CYANFW_SIZE, 0, 0) ;
#else
	ret = CyAsMiscDownloadFirmware(dev_handle, cyastfw_sd_mmc_rel_silicon_array.fw_image,
			(uint16_t)CYASTFW_SD_MMC_REL_SILICON_SIZE, 0, 0) ;
#endif

#endif
	if (ret != CY_AS_ERROR_SUCCESS)
	{
		Kern::Printf("%s: cannot download the antioch firmware - code %d\n", pgm, ret) ;
		return 1 ;
	}
	Kern::Printf("** API device loaded firmware\n") ;

    /*
     * Note, if a firmware image is not provided, the firmware can still be loaded if the West Bridge device
     * is in DEBUG mode.  In debug mode, the firmware can be loaded via USB.  In general however, the firmware
     * should be loaded via the P Port and therefore a firmware file name should be provided.
     */

    /*
     * Wait for the initialization event telling me that the firmware
     * has completed initialization and is ready to go.  The sleep of 100 ms
     * is used to insure that we do not burn too many CPU cycles while waiting on
     * the firmware initialization to finish.
     */
#ifndef FIRMWARE_NOPPORT
    while (!gInitComplete)
    {
		NKern::Sleep(1000);
	}

	Kern::Printf("Firmware donwloaded successfully!!") ;
#if 0
#ifdef STORAGE_TEST
	ret = CyAsSymbianStorageTest("Symbian WB Test",dev_handle, tag);
	if ( !ret )
	{
		Kern::Printf("%s: CyAsSymbianStorageTest failed - code %d\n", pgm, ret) ;
		return 1 ;
	}
	Kern::Printf("** CyAsSymbianStorageTest done\n") ;
#else
	ret = CyAsAPIUsbInit("Symbian WB Test",dev_handle, tag);
	if ( !ret )
	{
		Kern::Printf("%s: CyAsAPIUsbInit failed - code %d\n", pgm, ret) ;
		return 1 ;
	}
	Kern::Printf("** CyAsAPIUsbInit done\n") ;
#endif
#endif

#endif

	ret = CyAsAPIGetHandle(dev_handle, tag);
	Kern::Printf("** CyAsAPIGetHandle done\n") ;

	return 0 ;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#else
/*
* Some compilers do not like empty C files, so if the OMAP hal is not being
* compiled, we compile this single function.  We do this so that for a given target
* HAL there are not multiple sources for the HAL functions.
*/
void MyOMAPKernelHalDummyFunction(
	void)
{
}

#endif



