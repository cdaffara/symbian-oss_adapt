/* Cypress West Bridge HAL for beagleboard SPI Symbian header file (cyashalbeagleboard_spi.h)
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

/*
 * This file contains the defintion of the hardware abstraction layer on beagleboard
 * talking to the West Bridge device through SPI interface
 */


#ifndef _INCLUDED_CYASHALBEAGLEBOARD_H_
#define _INCLUDED_CYASHALBEAGLEBOARD_H_

typedef struct CyAsHalSleepChannel_t {
    /* TODO:
    wait_queue_head_t wq ;*/
    int wq ;
} CyAsHalSleepChannel ;
typedef CyAsHalSleepChannel CyAnHalSleepChannel;

#include "cyastypes.h"
#include "cyas_cplus_start.h"

typedef enum CyAsHalInterfaceMode
{
    CyAsHalCRAMMode         = 0,
    CyAsHalSRAMMode         = 1,
    CyAsHalADMMode          = 2,
    CyAsHalPNANDLBD8Mode    = 3,
    CyAsHalPNANDLBD16Mode   = 4,
    CyAsHalPNANDSBD8Mode    = 5,
    CyAsHalPNANDSBD16Mode   = 6,
    CyAsHalSPIMode          = 7,
    CyAsHalLNAMode          = 8,
    CyAsHalUnknownMode      = 9
} CyAsHalInterfaceMode;

/*************************************************************************************************
 *
 * Below are the data structures that must be defined by the HAL layer
 *
 *************************************************************************************************/

/*
 * The HAL layer must define a TAG for identifying a specific West Bridge device in the system.
 * In this case the tag is a void * which is really an OMAP device pointer
 */
typedef void * CyAsHalDeviceTag ;
typedef CyAsHalDeviceTag CyAnHalDeviceTag;

/* This must be included after the CyAsHalDeviceTag type is defined */
#include "cyashalcb.h"

/*************************************************************************************************
 *
 * Below are the functions that communicate with the West Bridge device.  These are system dependent
 * and must be defined by the HAL layer for a given system.
 *
 *************************************************************************************************/

/*
 * This function must be defined to write a register within the West Bridge
 * device.  The addr value is the address of the register to write with
 * respect to the base address of the West Bridge device.
 */
extern void
CyAsHalWriteRegister(CyAsHalDeviceTag tag, uint16_t addr, uint16_t data) ;
#define CyAnHalWriteRegister(tag, addr, data) CyAsHalWriteRegister((CyAsHalDeviceTag)(tag), (addr), (data))

/*
 * This function must be defined to read a register from the West Bridge
 * device.  The addr value is the address of the register to read with
 * respect to the base address of the West Bridge device.
 */
uint16_t
CyAsHalReadRegister(CyAsHalDeviceTag tag, uint16_t addr) ;
#define CyAnHalReadRegister(tag, addr) CyAsHalReadRegister((CyAsHalDeviceTag)(tag), (addr))

/*
 * This function must be defined to transfer a block of data to the West Bridge device.  This
 * function can use the burst write (DMA) capabilities of West Bridge to do this, or it can
 * just copy the data using writes.
 */
extern void
CyAsHalDmaSetupWrite(CyAsHalDeviceTag tag, uint8_t ep, void *buf, uint32_t size, uint16_t maxsize) ;
#define CyAnHalDmaSetupWrite(tag, ep, buf, sz, max)     \
    CyAsHalDmaSetupWrite((CyAsHalDeviceTag)(tag), (ep), (buf), (sz), (max))

/*
 * This function must be defined to transfer a block of data from the West Bridge device.  This
 * function can use the burst read (DMA) capabilities of West Bridge to do this, or it can just
 * copy the data using reads.
 */
extern void
CyAsHalDmaSetupRead(CyAsHalDeviceTag tag, uint8_t ep, void *buf, uint32_t size, uint16_t maxsize) ;
#define CyAnHalDmaSetupRead(tag, ep, buf, sz, max)      \
    CyAsHalDmaSetupRead((CyAsHalDeviceTag)(tag), (ep), (buf), (sz), (max))

/*
 * This function must be defined to cancel any pending DMA request.
 */
extern void
CyAsHalDmaCancelRequest(CyAsHalDeviceTag tag, uint8_t ep) ;
#define CyAnHalDmaCancelRequest(tag, ep) CyAsHalDmaCancelRequest((CyAsHalDeviceTag)(tag), (ep))

/*
 * This function must be defined to allow the West Bridge API to register a callback function that is
 * called when a DMA transfer is complete.
 */
extern void
CyAsHalDmaRegisterCallback(CyAsHalDeviceTag tag, CyAsHalDmaCompleteCallback cb) ;
#define CyAnHalDmaRegisterCallback(tag, cb)     \
    CyAsHalDmaRegisterCallback((CyAsHalDeviceTag)(tag), (CyAsHalDmaRegisterCallback)(cb))

/*
 * This function must be defined to return the maximum size of DMA request that can be handled
 * on the given endpoint.  The return value should be the maximum size in bytes that the DMA
 * module can handle.
 */
extern uint32_t
CyAsHalDmaMaxRequestSize(CyAsHalDeviceTag tag, CyAsEndPointNumber_t ep) ;
#define CyAnHalDmaMaxRequestSize(tag, ep)       \
    CyAsHalDmaMaxRequestSize((CyAsHalDeviceTag)(tag), (CyAsEndPointNumber_t)(ep))

/*
 * This function must be defined to set the state of the WAKEUP pin on the West Bridge device.  Generally
 * this is done via a GPIO of some type.
 */
extern CyBool
CyAsHalSetWakeupPin(CyAsHalDeviceTag tag, CyBool state) ;
#define CyAnHalSetWakeupPin(tag, state) CyAsHalSetWakeupPin((CyAsHalDeviceTag)(tag), (state))

/*
 * This function is called when the West Bridge PLL loses lock, because
 * of a problem in the supply voltage or the input clock.
 */
extern void
CyAsHalPllLockLossHandler(CyAsHalDeviceTag tag) ;
#define CyAnHalPllLockLossHandler(tag) CyAsHalPllLockLossHandler((CyAsHalDeviceTag)(tag))

/*
 * Function to re-synchronise connection between Processor and West Bridge.
 * This is a no-operation for this HAL.
 */
extern CyBool
CyAsHalSyncDeviceClocks(CyAsHalDeviceTag tag) ;
#define CyAsHalSyncDeviceClocks(tag)    (CyTrue)

/*
 * Function to re-initialize West Bridge register when waking up from standby.
 * This is a no-operation for this HAL.
 */
extern void
CyAsHalInitDevRegisters(CyAsHalDeviceTag tag, CyBool is_standby_wakeup ) ;


/*
 * Function to read and save register values before Astoria is put into standby mode.
 */
extern void
CyAsHalReadRegsBeforeStandby (
        CyAsHalDeviceTag tag) ;

/*
 * Function to restore original values to registers after Astoria wakes up from standby mode.
 */
extern void
CyAsHalRestoreRegsAfterStandby (
        CyAsHalDeviceTag tag) ;

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
CyAsHalAlloc(uint32_t cnt) ;
#define CyAnHalAlloc(cnt) CyAsHalAlloc(cnt)

/*
 * This function is required by the API to free memory allocated with CyAsHalAlloc().  This function is
 * expected to work exacly like free().
 */
void
CyAsHalFree(void *mem_p) ;
#define CyAnHalFree(ptr) CyAsHalFree(ptr)

/*
 * This function is required by the API to allocate memory during a callback.  This function must be able to
 * provide storage at inturupt time.
 */
void *
CyAsHalCBAlloc(uint32_t cnt) ;
#define CyAnHalCBAlloc(cnt) CyAsHalCBAlloc(cnt)

/*
 * This function is required by the API to free memory allocated with CyAsHalCBAlloc.
 */
void
CyAsHalCBFree(void *mem_p) ;
#define CyAsHalCBFree(ptr) CyAsHalFree(ptr)
#define CyAnHalCBFree(ptr) CyAsHalFree(ptr)

/*
 * This function is required to set a block of memory to a specific value.  This function is
 * expected to work exactly like memset()
 */
void
CyAsHalMemSet(void *ptr, uint8_t value, uint32_t cnt) ;
#define CyAnHalMemSet(ptr, value, cnt) CyAsHalMemSet((ptr), (value), (cnt))

/*
 * This function is expected to create a sleep channel.  The data structure that represents the
 * sleep channel is given by the pointer in the argument.
 */
CyBool
CyAsHalCreateSleepChannel(CyAsHalSleepChannel *channel) ;
#define CyAnHalCreateSleepChannel(ch) CyAsHalCreateSleepChannel((CyAsHalSleepChannel *)(ch))

/*
 * This function is expected to destroy a sleep channel.  The data structure that represents the
 * sleep channel is given by the pointer in the argument.
 */
CyBool
CyAsHalDestroySleepChannel(CyAsHalSleepChannel *channel) ;
#define CyAnHalDestroySleepChannel(ch) CyAsHalDestroySleepChannel((CyAsHalSleepChannel *)(ch))

CyBool
CyAsHalSleepOn(CyAsHalSleepChannel *channel, uint32_t ms) ;
#define CyAnHalSleepOn(ch, ms) CyAsHalSleepOn((CyAsHalSleepChannel *)(ch), (ms))

CyBool
CyAsHalWake(CyAsHalSleepChannel *channel) ;
#define CyAnHalWake(ch) CyAsHalWake((CyAsHalSleepChannel *)(ch))

uint32_t
CyAsHalDisableInterrupts(void) ;
#define CyAnHalDisableInterrupts() CyAsHalDisableInterrupts()

void
CyAsHalEnableInterrupts(uint32_t);
#define CyAnHalEnableInterrupts(msk) CyAsHalEnableInterrupts(msk)

void
CyAsHalSleep150(void) ;
#define CyAnHalSleep150() CyAsHalSleep150()

void
CyAsHalSleep(uint32_t ms) ;
#define CyAnHalSleep(ms) CyAsHalSleep(ms)

#define CyAsHalIsPolling()	        (CyFalse)
#define CyAsHalDisablePolling()
#define CyAsHalEnablePolling()

#define CyAnHalIsPolling()	        (CyFalse)
#define CyAnHalDisablePolling()
#define CyAnHalEnablePolling()

// #define PRINT_DEBUG_INFO // uncomment for debug info..
#ifdef PRINT_DEBUG_INFO
#define CyAsHalPrintMessage(...) Kern::Printf(__VA_ARGS__)
#else
#define CyAsHalPrintMessage(...)
#endif
#define CyAnHalPrintMessage CyAsHalPrintMessage (...)

void
CyAsHalPrintMessage2(const char* msg);

#ifdef PRINT_DEBUG_INFO
#define CyAsHalAssert(cond)	if (!(cond)) { CyAsHalPrintMessage ("Assertion failed at %s:%d\n", __FILE__, __LINE__); }
#else
#define CyAsHalAssert(cond)
#endif

#define CyAnHalAssert(cond) CyAsHalAssert(cond)

/*
   As this is a completely interrupt driven HAL, there is no
   need to implement the following functions.
 */

/*
 * These are the functions that are not part of the HAL layer, but are required to be called
 * for this HAL.
 */
int StartBeagleboardHal(const char *pgm, CyAsHalDeviceTag *tag, CyBool debug) ;
int StopBeagleboardHal(const char *pgm, CyAsHalDeviceTag tag) ;

/*
 * Constants
 */
#define CY_AS_OMAP_KERNEL_HAL_SIG	    (0x1441)

#define CYAS_DEV_MAX_ADDR   (0xFF)
#define CYAS_DEV_ADDR_RANGE (CYAS_DEV_MAX_ADDR << 16)

/* The base address is added in the kernel module code. */
#define CYAS_DEV_CALC_ADDR(addr) (addr << 16)

/*
 * Data structures
 */
typedef struct CyAsOmapDevKernel
{
    /* This is the signature for this data structure */
    unsigned int m_sig ;

    /* Address base of West Bridge Device */
    void *m_addr_base ;

    /* This is a pointer to the next West Bridge device in the system */
    struct CyAsOmapDevKernel *m_next_p ;

    /* This is for thread sync */
    /* TODO: struct completion thread_complete ;*/

    /* This is for thread to wait for interrupts */
    CyAsHalSleepChannel thread_sc ;

    /* This is for thread to exit upon StopOmapKernel */
    int thread_flag ; /* set 1 to exit */

} CyAsOmapDevKernel ;

extern int
CyAsHalAstoriaInit(void);

#include "cyas_cplus_end.h"

#endif
