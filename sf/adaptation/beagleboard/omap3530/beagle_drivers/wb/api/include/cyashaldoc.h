/* Cypress West Bridge API header file (cyashaldoc.h)
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

#ifndef _INCLUDED_CYASHALDOC_H_
#define _INCLUDED_CYASHALDOC_H_

#include "cyashaldef.h"

/*@@Hardware Abstraction Layer (HAL)
    Summary
    This software module is supplied by the user of the West Bridge API.  This module contains the
    software that is specific to the hardware implementation or operating system of the client
    system.

    * Sleep Channels *
    A sleep channel is a operating system object that provides that capability for one
    thread or process to sleep while waiting on the completion of some hardware event. The
    hardware event is usually processed by a hardware interrupt and the interrupt handler
    then wakes the thread or process that is sleeping.
	
    A sleep channel provides the mechanism for this operation.  A sleep channel is created and 
    initialized during the API initialization. When the API needs to wait for the hardware, the 
    API performs a SleepOn() operation on the sleep channel.  When hardware event occurs, an 
    interrupt handler processes the event and then performs a Wake() operation on the sleep channel 
    to wake the sleeping process or thread.

    * DMA Model *
    When the West Bridge API needs to transfer USB or storage data to/from the West Bridge device, this is 
    done using a "DMA" operation.  In this context the term DMA is used loosely as the West Bridge 
    API does not really care if the data is transferred using a burst read or write operation,
    or if the data is transferred using programmed I/O operations.  When a "DMA" operation is
    needed, the West Bridge API calls either CyAsHalDmaSetupRead() or CyAsHalDmaSetupWrite() depending
    on the direction of the data flow.  The West Bridge API expects the "DMA" operation requested in
    the call to be completed and the registered "DMA complete" callback to be called.

    The West Bridge API looks at several factors to determine the size of the "DMA" request to
    pass to the HAL layer.  First the West Bridge API calls CyAsHalDmaMaxRequestSize() to determine
    the maximum amount of data the HAL layer can accept for a "DMA" operation on the requested
    endpoint.  The West Bridge API will never exceed this value in a "DMA" request to the HAL
    layer.  The West Bridge API also sends the maximum amount of data the West Bridge device can
    accept as part of the "DMA" request.  If the amount of data in the "DMA" request to the HAL
    layer exceeds the amount of data the West Bridge device can accept, it is expected that the
    HAL layer has the ability to break the request into multiple operations.

    If the HAL implementation requires the API to handle the size of the "DMA" requests for
    one or more endpoints, the value CY_AS_DMA_MAX_SIZE_HW_SIZE can be returned from the
    CyAsHalDmaMaxRequestSize() call.  In this case, the API assumes that the maximum size of
    each "DMA" request should be limited to the maximum that can be accepted by the endpoint
    in question.

    Notes
    See the <install>/api/hal/scm_kernel/cyashalscm_kernel.c file for an example of how 
    the DMA request size can be managed by the HAL implementation.

    * Interrupt Handling *
    The HAL implementation is required to handle interrupts arriving from the West Bridge device,
    and call the appropriate handlers.  If the interrupt arriving is one of PLLLOCKINT,
    PMINT, MBINT or MCUINT, the CyAsIntrServiceInterrupt API should be called to service the
    interrupt.  If the interrupt arriving is DRQINT, the HAL should identify the endpoint
    corresponding to which the DRQ is being generated and perform the read/write transfer
    from the West Bridge. See the <install>/api/hal/scm_kernel/cyashalscm_kernel.c or
    <install>/api/hal/fpga/cyashalfpga.c reference HAL implementations for examples.

    The HAL implementation can choose to poll the West Bridge interrupt status register instead
    of using interrupts.  In this case, the polling has to be performed from a different
    thread/task than the one running the APIs.  This is required because there are API calls
    that block on the reception of data from the West Bridge, which is delivered only through
    the interrupt handlers.

    * Required Functions *
    This section defines the types and functions that must be supplied in order to provide a
    complete HAL layer for the West Bridge API.

    Types that must be supplied:
    * CyAsHalSleepChannel

    Hardware functions that must be supplied:
    * CyAsHalWriteRegister
    * CyAsHalReadRegister
    * CyAsHalDmaSetupWrite
    * CyAsHalDmaSetupRead
    * CyAsHalDmaCancelRequest
    * CyAsHalDmaRegisterCallback
    * CyAsHalDmaMaxRequestSize
    * CyAsHalSetWakeupPin
    * CyAsHalSyncDeviceClocks
    * CyAsHalInitDevRegisters
    * CyAsHalReadRegsBeforeStandby
    * CyAsHalRestoreRegsAfterStandby

    Operating system functions that must be supplied:
    * CyAsHalAlloc
    * CyAsHalFree
    * CyAsHalCBAlloc
    * CyAsHalCBFree
    * CyAsHalMemSet
    * CyAsHalCreateSleepChannel
    * CyAsHalDestroySleepChannel
    * CyAsHalSleepOn
    * CyAsHalWake
    * CyAsHalDisableInterrupts
    * CyAsHalEnableInterrupts
    * CyAsHalSleep150
    * CyAsHalSleep
    * CyAsHalAssert
    * CyAsHalPrintMessage
    * CyAsHalIsPolling
*/

/* Summary
   This is the type that represents a sleep channel

   Description
   A sleep channel is an operating system object that, when a thread of control waits on the
   sleep channel, the thread sleeps until another thread signals the sleep object.  This object
   is generally used when a high level API is called and must wait for a response that is 
   supplied in an interrupt handler.  The thread calling the API is put into a sleep state and
   when the reply arrives via the interrupt handler, the interrupt handler wakes the sleeping
   thread to indicate that the expect reply is available.
*/
typedef struct CyAsHalSleepChannel
{
    int					m_channel ;	/* This structure is filled in with OS specific information
							   to implementat a sleep channel */
} CyAsHalSleepChannel ;

/* Summary
   This function is called to write a register value

   Description
   This function is called to write a specific register to a specific value.  The tag identifies the
   device of interest.  The address is relative to the base address of the West Bridge device. 

   Returns
   Nothing

   See Also
   * CyAsHalDeviceTag
   * CyAsHalReadRegister
*/
EXTERN void
CyAsHalWriteRegister(
    CyAsHalDeviceTag			tag,		/* The tag to ID a specific West Bridge device */
    uint16_t				addr,		/* The address we are writing to */
    uint16_t				value		/* The value to write to the register */
    ) ;

/* Summary
   This function is called to read a register value

   Description
   This function is called to read the contents of a specific register.  The tag identifies the
   device of interest.  The address is relative to the base address of the West Bridge device.

   Returns
   Contents of the register

   See Also
   * CyAsHalDeviceTag
   * CyAsHalWriteRegister
*/
EXTERN uint16_t
CyAsHalReadRegister(
    CyAsHalDeviceTag			tag,		/* The tag to ID a specific West Bridge device */
    uint16_t				addr		/* The address we are writing to */
    ) ;

/* Summary
   This function initiates a DMA write operation to write to West Bridge

   Description
   This function initiates a DMA write operation.  The request size will not exceed the value the HAL
   layer returned via CyAsHalDmaMaxRequestSize().  This request size may exceed the size of what the
   West Bridge device will accept as one packet and the HAL layer may need to divide the request into
   multiple hardware DMA operations.

   Returns
   None

   See Also
   * CyAsHalDmaSetupRead
   * CyAsHalDmaMaxRequestSize
*/
EXTERN void
CyAsHalDmaSetupWrite(
    CyAsHalDeviceTag			tag,		/* The tag to ID a specific West Bridge device */
    CyAsEndPointNumber_t		ep,		/* The endpoint we are writing to */
    void *				buf_p,		/* The data to write via DMA */
    uint32_t				size,		/* The size of the data at buf_p */
    uint16_t				maxsize		/* The maximum amount of data that the endpoint can accept as one packet */
    ) ;

/* Summary
   This function initiates a DMA read operation from West Bridge

   Description
   This function initiates a DMA read operation.  The request size will not exceed the value the 
   HAL layer returned via CyAsHalDmaMaxRequestSize().  This request size may exceed the size of what
   the Anitoch will accept as one packet and the HAL layer may need to divide the request into multiple
   hardware DMA operations.

   Returns
   None

   See Also
   * CyAsHalDmaSetupRead
   * CyAsHalDmaMaxRequestSize
*/
EXTERN void
CyAsHalDmaSetupRead(
    CyAsHalDeviceTag			tag,		/* The tag to ID a specific West Bridge device */
    CyAsEndPointNumber_t		ep,		/* The endpoint we are reading from */
    void *				buf_p,		/* The buffer to read data into */
    uint32_t				size,		/* The amount of data to read */
    uint16_t				maxsize		/* The maximum amount of data that the endpoint can provide in one DMA operation */
    ) ;

/* Summary
   This function cancels a pending DMA request

   Description
   This function cancels a pending DMA request that has been passed down to the hardware.  The HAL layer
   can elect to physically cancel the request if possible, or just ignore the results of the request if it
   is not possible.

   Returns
   None
*/
EXTERN void
CyAsHalDmaCancelRequest(
    CyAsHalDeviceTag			tag,		/* The tag to ID a specific West Bridge device */
    CyAsEndPointNumber_t		ep		/* The endpoint we are reading from */
    ) ;

/* Summary
   This function registers a callback function to be called when a DMA request is completed

   Description
   This function registers a callback that is called when a request issued via CyAsHalDmaSetupWrite() or
   CyAsHalDmaSetupRead() has completed.

   Returns
   None

   See Also
   * CyAsHalDmaSetupWrite
   * CyAsHalDmaSetupRead
*/
EXTERN void
CyAsHalDmaRegisterCallback(
    CyAsHalDeviceTag			tag,		/* The tag to ID a specific West Bridge device */
    CyAsHalDmaCompleteCallback		cb		/* The callback to call when a request has completed */
    ) ;

/* Summary
   This function returns the maximum size of a DMA request that can be handled by the HAL.

   Description
   When DMA requests are passed to the HAL layer for processing, the HAL layer may have a limit
   on the size of the request that can be handled.  This function is called by the DMA manager
   for an endpoint when DMA is enabled to get the maximum size of data the HAL layer can handle.
   The DMA manager insures that a request is never sent to the HAL layer that exceeds the size
   returned by this function.

   Returns
   the maximum size of DMA request the HAL layer can handle
*/
EXTERN uint32_t
CyAsHalDmaMaxRequestSize(
    CyAsHalDeviceTag			tag,		/* The tag to ID a specific West Bridge device */
    CyAsEndPointNumber_t		ep		/* The endpoint of interest */
    ) ;

/* Summary
   This function sets the WAKEUP pin to a specific state on the West Bridge device.

   Description
   In order to enter the standby mode, the WAKEUP pin must be de-asserted.  In order to resume from
   standby mode, the WAKEUP pin must be asserted.  This function provides the mechanism to do
   this.

   Returns
   1 if the pin was changed, 0 if the HAL layer does not support changing this pin
*/
EXTERN uint32_t
CyAsHalSetWakeupPin(
    CyAsHalDeviceTag			tag,		/* The tag to ID a specific West Bridge device */
    CyBool				state		/* The desired state of the wakeup pin */
    ) ;

/* Summary
   Synchronise the West Bridge device clocks to re-establish device connectivity.

   Description
   When the Astoria bridge device is working in SPI mode, a long period of inactivity
   can cause a loss of serial synchronisation between the processor and Astoria.  This
   function is called by the API when it detects such a condition, and is expected to take
   the action required to re-establish clock synchronisation between the devices.

   Returns
   CyTrue if the attempt to re-synchronise is successful, CyFalse if not.
 */
EXTERN CyBool
CyAsHalSyncDeviceClocks(
    CyAsHalDeviceTag                    tag,            /* The tag to ID a specific West Bridge device */
    ) ;

/* Summary
   Initialize West Bridge device registers that may have been modified while the device
   was in standby.

   Description
   The content of some West Bridge registers may be lost when the device is placed in
   standby mode.  This function restores these register contents so that the device can
   continue to function normally after it wakes up from standby mode.

   This function is required to perform operations only when the API is being used with
   the Astoria device in one of the PNAND modes or in the PSPI mode.  It can be a
   no-operation in all other cases.

   Returns
   None
 */
EXTERN void
CyAsHalInitDevRegisters(
    CyAsHalDeviceTag                    tag,                    /* The tag to ID a specific West Bridge device */
    CyBool                              is_standby_wakeup       /* Indicates whether this is a wake-up from standby. */
    ) ;

/* Summary
   This function reads a set of P-port accessible device registers and stores their value
   for later use.

   Description
   The West Bridge Astoria device silicon has a known problem when operating in SPI mode
   on the P-port, where some of the device registers lose their value when the device
   goes in and out of standby mode.  The suggested work-around is to reset the Astoria
   device as part of the wakeup procedure from standby.
   
   This requires that the values of some of the P-port accessible registers be restored to
   their pre-standby values after it has been reset.  This HAL function can be used to read
   and store the values of these registers at the point where the device is being placed
   in standby mode.

   Returns
   None

   See Also
   * CyAsHalRestoreRegsAfterStandby
 */
EXTERN void
CyAsHalReadRegsBeforeStandby(
    CyAsHalDeviceTag                    tag                     /* The tag to ID a specific West Bridge device */
    ) ;

/* Summary
   This function restores the old values to a set of P-port accessible device registers.

   Description
   This function is part of the work-around to a known West Bridge Astoria device error when
   operating in SPI mode on the P-port.  This function is used to restore a set of P-port
   accessible registers to the values they had before the device was placed in standby
   mode.

   Returns
   None

   See Also
   * CyAsHalRestoreRegsAfterStandby
 */
EXTERN void
CyAsHalRestoreRegsAfterStandby(
    CyAsHalDeviceTag                    tag                     /* The tag to ID a specific West Bridge device */
    ) ;

/*
 * The functions below this comment are part of the HAL layer, as the HAL layer consists of the
 * abstraction to both the hardware platform and the operating system.  However; the functions below
 * this comment all relate to the operating environment and not specifically to the hardware platform
 * or specific device.
 */

/* Summary
   This function allocates a block of memory

   Description
   This is the HAL layer equivalent of the malloc() function.

   Returns
   a pointer to a block of memory

   See Also
   * CyAsHalFree
*/
EXTERN void *
CyAsHalAlloc(
    uint32_t				size		/* The size of the memory block to allocate */
    ) ;

/* Summary
   This function frees a previously allocated block of memory

   Description
   This is the HAL layer equivalent of the free() function.

   Returns
   None

   See Also
   * CyAsHalAlloc
*/
EXTERN void
CyAsHalFree(
    void *				ptr		/* Pointer to a memory block to free */
    ) ;

/* Summary
   This function is a malloc equivalent that can be used from an
   interrupt context.

   Description
   This function is a malloc equivalent that will be called from the
   API in callbacks. This function is required to be able to provide
   memory in interrupt context.

   Notes
   For platforms where it is not possible to allocate memory in interrupt
   context, we provide a reference allocator that takes memory during
   initialization and implements malloc/free using this memory.
   See the <install>/api/hal/fpga/cyashalblkalloc.[ch] files for the
   implementation, and the <install>/api/hal/fpga/cyashalfpga.c file
   for an example of the use of this allocator.

   Returns
   A pointer to the allocated block of memory

   See Also
   * CyAsHalCBFree
   * CyAsHalAlloc
*/
EXTERN void *
CyAsHalCBAlloc(
    uint32_t                            size            /* The size of the memory block to allocate */
    ) ;

/* Summary
   This function frees the memory allocated through the CyAsHalCBAlloc
   call.

   Description
   This function frees memory allocated through the CyAsHalCBAlloc
   call, and is also required to support calls from interrupt
   context.

   Returns
   None

   See Also
   * CyAsHalCBAlloc
   * CyAsHalFree
*/
EXTERN void
CyAsHalCBFree(
    void *                              ptr             /* Pointer to the memory block to be freed */
    ) ;

/* Summary
   This function sets a block of memory to a specific value

   Description
   This function is the HAL layer equivalent of the memset() function.

   Returns
   None
*/
EXTERN void
CyAsMemSet(
    void *				ptr,		/* A pointer to a block of memory to set */
    uint8_t				value,		/* The value to set the memory to */
    uint32_t				cnt		/* The number of bytes to set */
    ) ;

/* Summary
   This function creates or initializes a sleep channel

   Description
   This function creates or initializes a sleep channel.  The sleep channel defined using
   the HAL data structure CyAsHalSleepChannel.

   Returns
   CyTrue is the initialization was sucessful, and CyFalse otherwise

   See Also
   * CyAsHalSleepChannel
   * CyAsHalDestroySleepChannel
   * CyAsHalSleepOn
   * CyAsHalWake
*/
EXTERN CyBool
CyAsHalCreateSleepChannel(
	CyAsHalSleepChannel		*chan		/* Pointer to the sleep channel to create/initialize */
	) ;

/* Summary
   This function destroys an existing sleep channel

   Description
   This function destroys an existing sleep channel.  The sleep channel is of type
   CyAsHalSleepChannel.

   Returns
   CyTrue if the channel was destroyed, and CyFalse otherwise

   See Also
   * CyAsHalSleepChannel
   * CyAsHalCreateSleepChannel
   * CyAsHalSleepOn
   * CyAsHalWake
*/
EXTERN CyBool
CyAsHalDestroySleepChannel(
	CyAsHalSleepChannel		chan		/* The sleep channel to destroy */
	) ;

/* Summary
   This function causes the calling process or thread to sleep until CyAsHalWake() is called

   Description
   This function causes the calling process or threadvto sleep.  When CyAsHalWake() is called 
   on the same sleep channel, this processes or thread is then wakened and allowed to run

   Returns
   CyTrue if the thread or process is asleep, and CyFalse otherwise

   See Also
   * CyAsHalSleepChannel
   * CyAsHalWake
*/
EXTERN CyBool
CyAsHalSleepOn(
	CyAsHalSleepChannel		chan,		/* The sleep channel to sleep on */
	uint32_t				ms			/* The maximum time to sleep in milli-seconds */
	) ;

/* Summary
   This function casues the process or thread sleeping on the given sleep channel to wake

   Description
   This function causes the process or thread sleeping on the given sleep channel to wake.  The
   channel

   Returns
   CyTrue if the thread or process is awake, and CyFalse otherwise

   See Also
   * CyAsHalSleepChannel
   * CyAsHalSleepOn
*/
EXTERN CyBool
CyAsHalWake(
	CyAsHalSleepChannel		chan		/* The sleep channel to wake */
	) ;

/* Summary
   This function disables interrupts, insuring that short bursts of code can be run without danger of interrupt
   handlers running.

   Description
   There are cases within the API when lists must be manipulated by both the API and the associated
   interrupt handlers.  In these cases, interrupts must be disabled to insure the integrity of the list during the 
   modification.  This function is used to disable interrupts during the short intervals where these lists are being
   changed.

   The HAL must have the ability to nest calls to CyAsHalDisableInterrupts and CyAsHalEnableInterrupts.

   Returns
   Any interrupt related state value which will be passed back into the subsequent CyAsHalEnableInterrupts call.

   See Also
   * CyAsHalEnableInterrupts
*/
EXTERN uint32_t
CyAsHalDisableInterrupts() ;

/* Summary
   This function re-enables interrupts after a critical section of code in the API has been completed.

   Description
   There are cases within the API when lists must be manipulated by both the API and the associated
   interrupt handlers.  In these cases, interrupts must be disabled to insure the integrity of the list during the 
   modification.  This function is used to enable interrupts after the short intervals where these lists are being
   changed.

   See Also
   * CyAsHalDisableInterrupts
*/
EXTERN void
CyAsHalEnableInterrupts(
        uint32_t value                          /* Value returned by the previous CyAsHalDisableInterrupts call. */
        ) ;

/* Summary
   This function sleeps for 150 ns.

   Description
   This function sleeps for 150 ns before allowing the calling function to continue.  This function is used for a
   specific purpose and the sleep required is at least 150 ns.
*/
EXTERN void
CyAsHalSleep150(
        ) ;

/* Summary
   This function sleeps for the given number of milliseconds

   Description
   This function sleeps for at least the given number of milliseonds
*/
EXTERN void
CyAsHalSleep(
        uint32_t ms
        ) ;

/* Summary
   This function asserts when the condition evaluates to zero

   Description
   Within the API there are conditions which are checked to insure the integrity of the code.  These conditions are
   checked only within a DEBUG build.  This function is used to check the condition and if the result evaluates to
   zero, it should be considered a fatal error that should be reported to Cypress.
*/
EXTERN void
CyAsHalAssert(
	CyBool					cond		/* The condition to evaluate */
	) ;

/* Summary
   This function prints a message from the API to a human readable device

   Description
   There are places within the West Bridge API where printing a message is useful to the debug process.  This function
   provides the mechanism to print a message.

   Returns
   NONE
*/
EXTERN void
CyAsHalPrintMessage(
	const char *			fmt_p,		/* The message to print */
	...						/* Variable arguments */
	) ;

/* Summary
   This function reports whether the HAL implementation uses polling to service
   data coming from the West Bridge.

   Description
   This function reports whether the HAL implementation uses polling to service
   data coming from the West Bridge.

   Returns
   CyTrue if the HAL polls the West Bridge Interrupt Status registers to complete operations,
   CyFalse if the HAL is interrupt driven.
 */
EXTERN CyBool
CyAsHalIsPolling (
        void) ;

#endif
