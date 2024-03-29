/* Cypress West Bridge API header file (cyasdma.h)
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

#ifndef _INCLUDED_CYASDMA_H_
#define _INCLUDED_CYASDMA_H_

#include "cyashal.h"
#include "cyasdevice.h"

#include "cyas_cplus_start.h"


/*@@DMA Overview
    This module manages the DMA operations to/from the West Bridge device.  The 
    DMA module maintains a DMA queue for each endpoint so multiple DMA requests
    may be queued and they will complete at some future time.

    The DMA module must be started before it can be used.  It is started by
    calling CyAsDmaStart().  This function intializes all of the endpoint
    data structures.

    In order to perform DMA on a particular endpoint, the endpoint must be 
    enabled by calling CyAsDmaEnableEndPoint().  In addition to enabling or
    disabling the endpoint, this function also sets the direction for a 
    given endpoint.  Direction is given in USB terms.  For P port to West Bridge
    traffic, the endpoint is a CyAsDirectionIn endpoint.  For West Bridge to P
    port traffic, the endpoint is a CyAsDirectionOut endpoint.

    Once DMA is started and an endpoint is enabled, DMA requests are issued by
    calling CyAsDmaQueueRequest().  This function queue either a DMA read or
    DMA write request.  The callback associated with the request is called once
    the request has been fulfilled.

    See Also
    * CyAsDmaStart
    * CyAsDmaEnableEndPoint
    * CyAsDmaDirection
    * CyAsDmaQueueRequest
 */

/*****************************************************************************
 * West Bridge Constants
 ****************************************************************************/
#define CY_AS_DMA_MAX_SIZE_HW_SIZE                              (0xffffffff)

/*****************************************************************************
 * West Bridge Data Structures
 ****************************************************************************/

/* Summary
   This type specifies the direction of an endpoint to the CyAsDmaEnableEndPoint function.

   Description
   When an endpoint is enabled, the direction of the endpoint can also be set. 
   This type is used to specify the endpoint type.  Note that the direction is specified in
   USB terms.  Therefore, if the DMA is from the P port to West Bridge, the direction is IN.

   See Also
   * CyAsDmaEnableEndPoint
*/
typedef enum CyAsDmaDirection
{
    CyAsDirectionIn = 0,                                /* Set the endpoint to type IN (P -> West Bridge) */
    CyAsDirectionOut = 1,                               /* Set the endpoint to type OUT (West Bridge -> P) */
    CyAsDirectionInOut = 2,                             /* Only valid for EP 0 */
    CyAsDirectionDontChange = 3                 /* Do no change the endpoint type */
} CyAsDmaDirection ;

/*****************************************************************************
 * West Bridge Functions
 ****************************************************************************/

/* Summary
   Initialize the DMA module and ready the module for receiving data

   Description
   This function initializes the DMA module by initializing all of the endpoint data 
   structures associated with the device given.  This function also register a DMA
   complete callback with the HAL DMA code.  This callback is called whenever the
   HAL DMA subsystem completes a requested DMA operation.

   Returns
   CY_AS_ERROR_SUCCESS - the module initialized sucessfully
   CY_AS_ERROR_OUT_OF_MEMORY - memory allocation failed during initialization
   CY_AS_ERROR_ALREADY_RUNNING - the DMA module was already running

   See Also
   * CyAsDmaStop
*/
extern CyAsReturnStatus_t
CyAsDmaStart(
    CyAsDevice *                        dev_p           /* The device to start */
    ) ;

/* Summary
   Shutdown the DMA module

   Description
   This function shuts down the DMA module for this device by canceling any DMA requests
   associated with each endpoint and then freeing the resources associated with each DMA
   endpoint.

   Returns
   CY_AS_ERROR_SUCCESS - the module shutdown sucessfully
   CY_AS_ERROR_NOT_RUNNING - the DMA module was not running

   See Also
   * CyAsDmaStart
   * CyAsDmaCancel
*/
extern CyAsReturnStatus_t
CyAsDmaStop(
    CyAsDevice *                        dev_p           /* The device to stop */
    ) ;

/* Summary
   This function cancels all outstanding DMA requests on a given endpoint
   
   Description
   This function cancels any DMA requests outstanding on a given endpoint by
   disabling the transfer of DMA requests from the queue to the HAL layer and
   then removing any pending DMA requests from the queue.  The callback associated
   with any DMA requests that are being removed is called with an error code of
   CY_AS_ERROR_CANCELED.

   Notes
   If a request has already been sent to the HAL layer it will be completed and
   not canceled.  Only requests that have not been sent to the HAL layer will be
   canceled.

   Returns
   CY_AS_ERROR_SUCCESS - the traffic on the endpoint is canceled sucessfully

   See Also
*/
extern CyAsReturnStatus_t 
CyAsDmaCancel(
    CyAsDevice *                        dev_p,          /* The device of interest */
    CyAsEndPointNumber_t                ep,             /* The endpoint to cancel */
    CyAsReturnStatus_t                  err
    ) ;

/* Summary
   This function enables a single endpoint for DMA operations

   Description
   In order to enable the queuing of DMA requests on a given endpoint, the endpoint
   must be enabled for DMA.  This function enables a given endpoint.  In addition, this
   function sets the direction of the DMA operation.

   Returns
   * CY_AS_ERROR_INVALID_ENDPOINT - invalid endpoint number
   * CY_AS_ERROR_SUCCESS - endpoint was enabled or disabled successfully

   See Also
   * CyAsDmaQueueRequest
*/
extern CyAsReturnStatus_t
CyAsDmaEnableEndPoint(
    CyAsDevice *                        dev_p,                  /* The device of interest */
    CyAsEndPointNumber_t                ep,                     /* The endpoint to enable or disable */
    CyBool                              enable,                 /* CyTrue to enable, CyFalse to disable */
    CyAsDmaDirection                    dir                     /* The direction of the endpoint */
    ) ;

/* Summary
   This function queue a DMA request for a given endpoint

   Description
   When an West Bridge API module wishes to do a DMA operation, this function is called on the
   associated endpoint to queue a DMA request.  When the DMA request has been fulfilled, the
   callback associated with the DMA operation is called.

   Notes
   The buffer associated with the DMA request, must remain valid until after the callback 
   function is calld.

   Returns
   * CY_AS_ERROR_SUCCESS - the DMA operation was queued successfully
   * CY_AS_ERROR_INVALID_ENDPOINT - the endpoint number was invalid
   * CY_AS_ERROR_ENDPOINT_DISABLED - the endpoint was disabled
   * CY_AS_ERROR_OUT_OF_MEMORY - out of memory processing the request

   See Also
   * CyAsDmaEnableEndPoint
   * CyAsDmaCancel
*/
extern CyAsReturnStatus_t
CyAsDmaQueueRequest(
    CyAsDevice *                        dev_p,          /* The device of interest */
    CyAsEndPointNumber_t                ep,             /* The endpoint to receive a new request */
    void *                              mem_p,          /* The memory buffer for the DMA request - must be valid until after the callback has been called */
    uint32_t                            size,           /* The size of the DMA request in bytes */
    CyBool                              packet,         /* If true and a DMA read request, return the next packet regardless of size */
    CyBool                              readreq,        /* If true, this is a read request, otherwise it is a write request */
    CyAsDmaCallback                     cb              /* The callback to call when the DMA request is complete, either successfully or via an error */
    ) ;

/* Summary
   This function waits until all DMA requests on a given endpoint have been processed and then return

   Description
   There are times when a module in the West Bridge API needs to wait until the DMA operations have been
   queued.  This function sleeps until all DMA requests have been fulfilled and only then returns to the caller.

   Notes
   I don't think we will need a list of sleeping clients to support multiple parallel client modules
   sleeping on a single endpoint, but if we do instead of having a single sleep channel in the endpoint, each
   client will have to supply a sleep channel and we will have to maintain a list of sleep channels to wake.

   Returns
   * CY_AS_ERROR_SUCCESS - the queue has drained sucessfully
   * CY_AS_ERROR_INVALID_ENDPOINT - the endpoint given is not valid
   * CY_AS_ERROR_NESTED_SLEEP - CyAsDmaQueueRequest() was requested on an endpoint where CyAsDmaQueueRequest was already called
*/
extern CyAsReturnStatus_t
CyAsDmaDrainQueue(
    CyAsDevice *                        dev_p,          /* The device of interest */
    CyAsEndPointNumber_t                ep,             /* The endpoint to drain */
    CyBool                              kickstart       /* If CyTrue, call kickstart to start the DMA process,
                                                           If CyFalse, West Bridge will start the DMA process */
    ) ;

/* Summary
   Sets the maximum amount of data West Bridge can accept in a single DMA Operation for the given endpoint

   Description
   Depending on the configuration of the West Bridge device endpoint, the amount of data that can be
   accepted varies.  This function sets the maximum amount of data West Bridge can accept in a single
   DMA operation.  The value is stored with the endpoint and passed to the HAL layer in the 
   CyAsHalDmaSetupWrite() and CyAsHalDmaSetupRead() functoins.

   Returns
   * CY_AS_ERROR_SUCCESS - the value was set sucessfully
   * CY_AS_ERROR_INVALID_SIZE - the size value was not valid
*/
extern CyAsReturnStatus_t
CyAsDmaSetMaxDmaSize(
    CyAsDevice *                        dev_p,          /* The device of interest */
    CyAsEndPointNumber_t                ep,             /* The endpoint to change */
    uint32_t                            size            /* The max size of this endpoint in bytes */
    ) ;

/* Summary
   This function starts the DMA process on a given channel.

   Description
   When transferring data from the P port processor to West Bridge, the DMA operation must be initiated
   P Port software for the first transfer.  Subsequent transferrs will be handled at the interrupt level.

   Returns
   * CY_AS_ERROR_SUCCESS
*/
extern CyAsReturnStatus_t
CyAsDmaKickStart(
    CyAsDevice *                        dev_p,          /* The device of interest */
    CyAsEndPointNumber_t                ep              /* The endpoint to change */
    ) ;

/* Summary
   This function receives endpoint data from a request.

   Description
   For endpoint 0 and 1 the endpoint data is transferred from the West Bridge device to the DMA via a lowlevel
   requests (via the mailbox registers).

   Returns
   * CY_AS_ERROR_SUCCESS
*/
extern CyAsReturnStatus_t
CyAsDmaReceivedData(
    CyAsDevice *                        dev_p,          /* The device of interest */
    CyAsEndPointNumber_t                ep,             /* The endpoint that received data */
    uint32_t                            dsize,          /* The data size */
    void *                              data            /* The data buffer */
    );

/* Summary
   This function is called when the DMA operation on an endpoint has been completed.

   Returns
   * void
 */
extern void
CyAsDmaCompletedCallback (
        CyAsHalDeviceTag     tag,                       /* Tag to HAL completing the DMA operation. */
        CyAsEndPointNumber_t ep,                        /* Endpoint on which DMA has been completed. */
        uint32_t             length,                    /* Length of data received. */
        CyAsReturnStatus_t   status                     /* Status of DMA operation. */
        ) ;

#include "cyas_cplus_end.h"

#endif          /* _INCLUDED_CYASDMA_H_ */
