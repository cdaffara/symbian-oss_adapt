/* Cypress West Bridge API source file (cyasintr.c)
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

#include "cyashal.h"
#include "cyasdevice.h"
#include "cyasregs.h"
#include "cyaserr.h"

extern void CyAsMailBoxInterruptHandler(CyAsDevice *) ;

void
CyAsMcuInterruptHandler(CyAsDevice *dev_p)
{
    /* Read and clear the interrupt. */
    uint16_t v ;

    v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_MCU_STAT) ;
    v = v ;
}

void
CyAsPowerManagementInterruptHandler(CyAsDevice *dev_p)
{
    uint16_t v ;

    v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_PWR_MAGT_STAT) ;
    v = v ;
}

void
CyAsPllLockLossInterruptHandler(CyAsDevice *dev_p)
{
    uint16_t v ;

    v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_PLL_LOCK_LOSS_STAT) ;
    v = v ;
}

uint32_t CyAsIntrStart(CyAsDevice *dev_p, CyBool dmaintr)
{
    uint16_t v ;

    CyAsHalAssert(dev_p->sig == CY_AS_DEVICE_HANDLE_SIGNATURE) ;

    if (CyAsDeviceIsIntrRunning(dev_p) != 0)
        return CY_AS_ERROR_ALREADY_RUNNING ;

    v = CY_AS_MEM_P0_INT_MASK_REG_MMCUINT |
        CY_AS_MEM_P0_INT_MASK_REG_MMBINT |
        CY_AS_MEM_P0_INT_MASK_REG_MPMINT ;

    if (dmaintr)
        v |= CY_AS_MEM_P0_INT_MASK_REG_MDRQINT ;

    /* Enable the interrupts of interest */
    CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_INT_MASK_REG, v) ;

    /* Mark the interrupt module as initialized */
    CyAsDeviceSetIntrRunning(dev_p) ;

    return CY_AS_ERROR_SUCCESS ;
}

uint32_t CyAsIntrStop(CyAsDevice *dev_p)
{
    CyAsHalAssert(dev_p->sig == CY_AS_DEVICE_HANDLE_SIGNATURE) ;

    if (CyAsDeviceIsIntrRunning(dev_p) == 0)
        return CY_AS_ERROR_NOT_RUNNING ;

    CyAsHalWriteRegister(dev_p->tag, CY_AS_MEM_P0_INT_MASK_REG, 0) ;
    CyAsDeviceSetIntrStopped(dev_p) ;

    return CY_AS_ERROR_SUCCESS ;
}

void CyAsIntrServiceInterrupt(CyAsHalDeviceTag tag)
{
    uint16_t v ;
    CyAsDevice *dev_p ;

    dev_p = CyAsDeviceFindFromTag(tag) ;

    /*
       Only power management interrupts can occur before the Antioch API setup is complete.
       If this is a PM interrupt, handle it here; otherwise output a warning message.
     */
    if (dev_p == 0)
    {
        v = CyAsHalReadRegister(tag, CY_AS_MEM_P0_INTR_REG) ;
        if (v == CY_AS_MEM_P0_INTR_REG_PMINT)
        {
            /* Read the PWR_MAGT_STAT register to clear this interrupt. */
            v = CyAsHalReadRegister(tag, CY_AS_MEM_PWR_MAGT_STAT) ;
        }
        else
            ;//CyAsHalPrintMessage("Stray Antioch interrupt detected, tag not associated with any created device.") ;
        return ;
    }

    /* Make sure we got a valid object from CyAsDeviceFindFromTag */
    CyAsHalAssert(dev_p->sig == CY_AS_DEVICE_HANDLE_SIGNATURE) ;

    v = CyAsHalReadRegister(dev_p->tag, CY_AS_MEM_P0_INTR_REG) ;

    if (v & CY_AS_MEM_P0_INTR_REG_MCUINT)
        CyAsMcuInterruptHandler(dev_p) ;

    if (v & CY_AS_MEM_P0_INTR_REG_PMINT)
        CyAsPowerManagementInterruptHandler(dev_p) ;

    if (v & CY_AS_MEM_P0_INTR_REG_PLLLOCKINT)
        CyAsPllLockLossInterruptHandler(dev_p) ;

    /* If the interrupt module is not running, no mailbox interrupts are expected
     * from the Antioch. */
    if (CyAsDeviceIsIntrRunning(dev_p) == 0)
        return ;

    if (v & CY_AS_MEM_P0_INTR_REG_MBINT)
        CyAsMailBoxInterruptHandler(dev_p) ;
}

