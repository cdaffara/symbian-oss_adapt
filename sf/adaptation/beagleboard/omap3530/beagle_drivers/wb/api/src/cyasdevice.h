/* Cypress West Bridge API header file (cyasdevice.h)
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

#ifndef __INCLUDED_CYASDEVICE_H__
#define __INCLUDED_CYASDEVICE_H__

#include "cyashal.h"
#include "cyasprotocol.h"
#include "cyasusb.h"
#include "cyasstorage.h"
#include "cyasmtp.h"
#include "cyas_cplus_start.h"

/*****************************************************************************
 * West Bridge Constants
 ****************************************************************************/

/* The endpoints used by West Bridge for the P port to S port path */
#define CY_AS_P2S_WRITE_ENDPOINT                        (0x04)
#define CY_AS_P2S_READ_ENDPOINT                         (0x08)

/* The endpoint to use for firmware download */
#define CY_AS_FIRMWARE_ENDPOINT                         (0x02)

/* The maximum size of the firmware image West Bridge can accept */
#define CY_AS_MAXIMUM_FIRMWARE_SIZE                     (24 * 1024)

/* The maximum size of a write for EP0 and EP1 */
#define CY_AS_EP0_MAX_WRITE_SIZE                        (128)
#define CY_AS_EP1_MAX_WRITE_SIZE                        (64)

/* The bitfields for the device state value */
#define CY_AS_DEVICE_STATE_PIN_STANDBY                  (0x00000001)            /* The device is in StandBy mode */
#define CY_AS_DEVICE_STATE_CONFIGURED                   (0x00000002)            /* The device has been configured */
#define CY_AS_DEVICE_STATE_FIRMWARE_LOADED              (0x00000004)            /* The firmware has been loaded into the device */
#define CY_AS_DEVICE_STATE_LOWLEVEL_MODULE              (0x00000008)            /* The interrupt module has been initialized */
#define CY_AS_DEVICE_STATE_DMA_MODULE                   (0x00000010)            /* The DMA module has been initialized */
#define CY_AS_DEVICE_STATE_INTR_MODULE                  (0x00000020)            /* The interrupt module has been initialized */
#define CY_AS_DEVICE_STATE_STORAGE_MODULE               (0x00000040)            /* The storage module has been initialized */
#define CY_AS_DEVICE_STATE_USB_MODULE                   (0x00000080)            /* The USB module has been initialized */
#define CY_AS_DEVICE_STATE_STORAGE_SCSIMSG              (0x00000100)            /* If set, the API wants SCSI messages */
#define CY_AS_DEVICE_STATE_STORAGE_ASYNC_PENDING        (0x00000200)            /* If set, an ASYNC storage operation is pending */
#define CY_AS_DEVICE_STATE_USB_CONNECTED                (0x00000400)            /* If set, the USB port is connected */
#define CY_AS_DEVICE_STATE_USB_HIGHSPEED                (0x00000800)            /* If set and USB is connected, it is high speed */
#define CY_AS_DEVICE_STATE_IN_CALLBACK                  (0x00001000)            /* If set, we are in a callback */
#define CY_AS_DEVICE_STATE_IN_SETUP_PACKET              (0x00004000)            /* If set, we are processing a setup packet */
#define CY_AS_DEVICE_STATE_REGISTER_STANDBY             (0x00008000)            /* The device was placed in standby via register */
#define CY_AS_DEVICE_STATE_CRYSTAL                      (0x00010000)            /* If set, the device is using a crystal */
#define CY_AS_DEVICE_STATE_WAKING                       (0x00020000)            /* If set, wakeup has been called */
#define CY_AS_DEVICE_STATE_EP0_STALLED                  (0x00040000)            /* If set, EP0 has been stalled. */
#define CY_AS_DEVICE_STATE_SUSPEND                      (0x00080000)            /* If set, device is in suspend mode. */
#define CY_AS_DEVICE_STATE_RESETP                       (0x00100000)            /* If set, device is a reset is pending. */
#define CY_AS_DEVICE_STATE_STANDP                       (0x00200000)            /* If set, device is a standby is pending. */
#define CY_AS_DEVICE_STATE_SSSP                         (0x00400000)            /* If set, device has a storage start or stop pending. */
#define CY_AS_DEVICE_STATE_USSP                         (0x00800000)            /* If set, device has a usb start or stop pending. */
#define CY_AS_DEVICE_STATE_MSSP                         (0x01000000)            /* If set, device has a mtp start or stop pending. */


/* The bitfields for the endpoint state value */
#define CY_AS_DMA_ENDPOINT_STATE_ENABLED                (0x0001)                /* DMA requests are accepted into the queue */
#define CY_AS_DMA_ENDPOINT_STATE_SLEEPING               (0x0002)                /* The endpoint has a sleeping client, waiting on a queue drain */
#define CY_AS_DMA_ENDPOINT_STATE_DMA_RUNNING            (0x0004)                /* The DMA backend to hardware is running */
#define CY_AS_DMA_ENDPOINT_STATE_IN_TRANSIT             (0x0008)                /* There is an outstanding DMA entry deployed to the HAL */
#define CY_AS_DMA_ENDPOINT_STATE_DIRECTION              (0x0010)                /* 0 = OUT (West Bridge -> P Port), 1 = IN (P Port -> West Bridge) */

/* The state values for the request list */
#define CY_AS_REQUEST_LIST_STATE_MASK                   (0x0f)                  /* Mask for getting the state information */
#define CY_AS_REQUEST_LIST_STATE_QUEUED                 (0x00)                  /* The request is queued, nothing further */
#define CY_AS_REQUEST_LIST_STATE_WAITING                (0x01)                  /* The request is sent, waiting for response */
#define CY_AS_REQUEST_LIST_STATE_RECEIVED               (0x02)                  /* The response has been received, processing reponse */
#define CY_AS_REQUEST_LIST_STATE_CANCELING              (0x03)                  /* The request/response is being canceled */
#define CY_AS_REQUEST_LIST_STATE_SYNC                   (0x80)                  /* The request is synchronous */

/* The flag values for a LL RequestResponse */
#define CY_AS_REQUEST_RESPONSE_DELAY_ACK                (0x01)                  /* This request requires an ACK to be sent after it is completed */
#define CY_AS_REQUEST_RESPONSE_EX                       (0x02)                  /* This request originated from a version V1.1 function call */
#define CY_AS_REQUEST_RESPONSE_MS                       (0x04)                  /* This request originated from a version V1.2 function call */


#define CY_AS_DEVICE_HANDLE_SIGNATURE                   (0x01211219)

/*
 * This macro returns the endpoint pointer given the device pointer and an endpoint number
 */
#define CY_AS_NUM_EP(dev_p, num) ((dev_p)->endp[(num)])

/*****************************************************************************
 * West Bridge Data Structures
 ****************************************************************************/

typedef struct CyAsDevice CyAsDevice ;

/* Summary
   This type defines a callback function that will be called on completion of a DMA operation.

   Description
   This function definition is for a function that is called when the DMA operation is complete.  This
   function is called with the endpoint number, operation type, buffer pointer and size.

   See Also
   * CyAsDmaOper
   * CyAsDmaQueueWrite
 */
typedef void (*CyAsDmaCallback)(
        CyAsDevice *                dev_p,                  /* The device that completed DMA */
        CyAsEndPointNumber_t        ep,                     /* The endpoint that completed DMA */
        void *                      mem_p,                  /* The pointer to the buffer that completed DMA */
        uint32_t                    size,                   /* The amount of data transferred */
        CyAsReturnStatus_t          error                   /* The error code for this DMA xfer */
        ) ;

/* Summary
   This structure defines a DMA request that is queued

   Description
   This structure contains the information about a DMA request that is queued and is to
   be sent when possible.
*/
typedef struct CyAsDmaQueueEntry
{
    void *                          buf_p ;                 /* Pointer to memory buffer for this request */
    uint32_t                        size ;                  /* Size of the memory buffer for DMA operation */
    uint32_t                        offset ;                /* Offset into memory buffer for next DMA operation */
    CyBool                          packet ;                /* If TRUE and IN request */
    CyBool                          readreq ;               /* If TRUE, this is a read request */
    CyAsDmaCallback                 cb ;                    /* Callback function for when DMA is complete */
    struct CyAsDmaQueueEntry *      next_p ;                /* Pointer to next entry in queue */
} CyAsDmaQueueEntry ;

/* Summary
   This structure defines the endpoint data for a given

   Description
   This structure defines all of the information required to manage DMA for a given
   endpoint.
*/
typedef struct CyAsDmaEndPoint
{
    CyAsEndPointNumber_t            ep ;                    /* The endpoint number */
    uint8_t                         state ;                 /* The state of this endpoint */
    uint16_t                        maxhwdata ;             /* The maximum amount of data accepted in a packet by the hw */
    uint32_t                        maxhaldata ;            /* The maximum amount of data accepted by the HAL layer */
    CyAsDmaQueueEntry *             queue_p ;               /* The queue for DMA operations */
    CyAsDmaQueueEntry *             last_p ;                /* The last entry in the DMA queue */
    CyAsHalSleepChannel             channel ;               /* This sleep channel is used to wait while the DMA queue drains for a given endpoint */
} CyAsDmaEndPoint ;

#define CyAsEndPointNumberIsUsb(n) ((n) != 2 && (n) != 4 && (n) != 6 && (n) != 8)
#define CyAsEndPointNumberIsStorage(n) ((n) == 2 || (n) == 4 || (n) == 6 || (n) == 8)

#define CyAsDmaEndPointIsEnabled(ep) ((ep)->state & CY_AS_DMA_ENDPOINT_STATE_ENABLED)
#define CyAsDmaEndPointEnable(ep) ((ep)->state |= CY_AS_DMA_ENDPOINT_STATE_ENABLED)
#define CyAsDmaEndPointDisable(ep) ((ep)->state &= ~CY_AS_DMA_ENDPOINT_STATE_ENABLED)

#define CyAsDmaEndPointIsSleeping(ep) ((ep)->state & CY_AS_DMA_ENDPOINT_STATE_SLEEPING)
#define CyAsDmaEndPointSetSleepState(ep) ((ep)->state |= CY_AS_DMA_ENDPOINT_STATE_SLEEPING)
#define CyAsDmaEndPointSetWakeState(ep) ((ep)->state &= ~CY_AS_DMA_ENDPOINT_STATE_SLEEPING)

#define CyAsDmaEndPointIsRunning(ep) ((ep)->state & CY_AS_DMA_ENDPOINT_STATE_DMA_RUNNING)
#define CyAsDmaEndPointSetRunning(ep) ((ep)->state |= CY_AS_DMA_ENDPOINT_STATE_DMA_RUNNING)
#define CyAsDmaEndPointSetStopped(ep) ((ep)->state &= ~CY_AS_DMA_ENDPOINT_STATE_DMA_RUNNING)

#define CyAsDmaEndPointInTransit(ep) ((ep)->state & CY_AS_DMA_ENDPOINT_STATE_IN_TRANSIT)
#define CyAsDmaEndPointSetInTransit(ep) ((ep)->state |= CY_AS_DMA_ENDPOINT_STATE_IN_TRANSIT)
#define CyAsDmaEndPointClearInTransit(ep) ((ep)->state &= ~CY_AS_DMA_ENDPOINT_STATE_IN_TRANSIT)

#define CyAsDmaEndPointIsDirectionIn(ep) (((ep)->state & CY_AS_DMA_ENDPOINT_STATE_DIRECTION) == CY_AS_DMA_ENDPOINT_STATE_DIRECTION)
#define CyAsDmaEndPointIsDirectionOut(ep) (((ep)->state & CY_AS_DMA_ENDPOINT_STATE_DIRECTION) == 0)
#define CyAsDmaEndPointSetDirectionIn(ep) ((ep)->state |= CY_AS_DMA_ENDPOINT_STATE_DIRECTION)
#define CyAsDmaEndPointSetDirectionOut(ep) ((ep)->state &= ~CY_AS_DMA_ENDPOINT_STATE_DIRECTION)

#define CyAsDmaEndPointIsUsb(p) CyAsEndPointNumberIsUsb((p)->ep)
#define CyAsDmaEndPointIsStorage(p) CyAsEndPointNumberIsStorage((p)->ep)

typedef struct CyAsLLRequestResponse
{
    uint16_t                        box0 ;                  /* The mbox[0] contents - see low level comm section of API doc */
    uint16_t                        stored ;                /* The amount of data stored in this request/response in bytes */
    uint16_t                        length ;                /* Length of this request in words */
    uint16_t                        flags ;                 /* Additional status information about the request */
    uint16_t                        data[1] ;               /* Note: This is over indexed and contains the request/response data */
} CyAsLLRequestResponse ;

/*
 * The callback function for responses
 */
typedef void (*CyAsResponseCallback)(
    CyAsDevice *                    dev_p,                  /* The device that had the response */
    uint8_t                         context,                /* The context receiving a response */
    CyAsLLRequestResponse *         rqt,                    /* The request data */
    CyAsLLRequestResponse *         resp,                   /* The response data */
    CyAsReturnStatus_t              status                  /* The status of the request */
    ) ;

typedef struct CyAsLLRequestListNode
{
    CyAsLLRequestResponse *         rqt ;                   /* The request to send */
    CyAsLLRequestResponse *         resp ;                  /* The associated response for the request */
    uint16_t                        length ;                /* Length of the response */
    CyAsResponseCallback            callback ;              /* The callback to call when done */
    uint8_t                         state ;                 /* The state of the request */
    struct CyAsLLRequestListNode *  next ;                  /* The next request in the list */
} CyAsLLRequestListNode ;

#define CyAsRequestGetNodeState(node_p) ((node_p)->state & CY_AS_REQUEST_LIST_STATE_MASK)
#define CyAsRequestSetNodeState(node_p, st) ((node_p)->state = ((node_p)->state & ~CY_AS_REQUEST_LIST_STATE_MASK) | (st))

#define CyAsRequestNodeIsSync(node_p) ((node_p)->state & CY_AS_REQUEST_LIST_STATE_SYNC)
#define CyAsRequestNodeSetSync(node_p) ((node_p)->state |= CY_AS_REQUEST_LIST_STATE_SYNC)
#define CyAsRequestNodeClearSync(node_p) ((node_p)->state &= ~CY_AS_REQUEST_LIST_STATE_SYNC)

#ifndef __doxygen__
typedef enum CyAsCBNodeType
{
    CYAS_INVALID,
    CYAS_USB_FUNC_CB,
    CYAS_USB_IO_CB,
    CYAS_STORAGE_IO_CB,
    CYAS_FUNC_CB
} CyAsCBNodeType ;

typedef struct CyAsFuncCBNode
{
    CyAsCBNodeType              nodeType ;
    CyAsFunctionCallback        cb_p ;
    uint32_t                    client_data ;
    /*CyAsFunctCBType             dataType ; nxz */
    uint32_t					dataType ;
    void*                       data ;
    struct CyAsFuncCBNode *     next_p ;
} CyAsFuncCBNode;

extern CyAsFuncCBNode*
CyAsCreateFuncCBNodeData(CyAsFunctionCallback cb, uint32_t client, CyAsFunctCBType type, void* data) ;

extern CyAsFuncCBNode*
CyAsCreateFuncCBNode(CyAsFunctionCallback cb, uint32_t client) ;

extern void
CyAsDestroyFuncCBNode(CyAsFuncCBNode* node) ;

typedef struct CyAsMTPFuncCBNode
{
    CyAsCBNodeType              type ;
    CyAsMTPFunctionCallback     cb_p ;
    uint32_t                    client_data;
    struct CyAsMTPFuncCBNode *  next_p ;
} CyAsMTPFuncCBNode;

extern CyAsMTPFuncCBNode*
CyAsCreateMTPFuncCBNode(CyAsMTPFunctionCallback cb, uint32_t client) ;

extern void
CyAsDestroyMTPFuncCBNode(CyAsMTPFuncCBNode* node) ;

typedef struct CyAsUsbFuncCBNode
{
    CyAsCBNodeType              type ;
    CyAsUsbFunctionCallback     cb_p ;
    uint32_t                    client_data;
    struct CyAsUsbFuncCBNode *  next_p ;
} CyAsUsbFuncCBNode;

extern CyAsUsbFuncCBNode*
CyAsCreateUsbFuncCBNode(CyAsUsbFunctionCallback cb, uint32_t client) ;

extern void
CyAsDestroyUsbFuncCBNode(CyAsUsbFuncCBNode* node) ;

typedef struct CyAsUsbIoCBNode
{
    CyAsCBNodeType              type ;
    CyAsUsbIoCallback           cb_p ;
    struct CyAsUsbIoCBNode *    next_p ;
} CyAsUsbIoCBNode;

extern CyAsUsbIoCBNode*
CyAsCreateUsbIoCBNode(CyAsUsbIoCallback cb) ;

extern void
CyAsDestroyUsbIoCBNode(CyAsUsbIoCBNode* node) ;

typedef struct CyAsStorageIoCBNode
{
    CyAsCBNodeType              type ;
    CyAsStorageCallback         cb_p ;
    CyAsMediaType               media ;             /* The media for the currently outstanding async storage request */
    uint32_t                    device_index ;      /* The device index for the currently outstanding async storage request */
    uint32_t                    unit ;              /* The unit index for the currently outstanding async storage request */
    uint32_t                    block_addr ;        /* The block address for the currently outstanding async storage request */
    CyAsOperType                oper ;              /* The operation for the currently outstanding async storage request */
    CyAsLLRequestResponse*      req_p ;
    CyAsLLRequestResponse*      reply_p ;
    struct CyAsStorageIoCBNode* next_p ;
} CyAsStorageIoCBNode;

extern CyAsStorageIoCBNode*
CyAsCreateStorageIoCBNode(CyAsStorageCallback cb, CyAsMediaType media, uint32_t device_index,
                          uint32_t unit, uint32_t block_addr, CyAsOperType oper,
                          CyAsLLRequestResponse* req_p, CyAsLLRequestResponse* reply_p) ;

extern void
CyAsDestroyStorageIoCBNode(CyAsStorageIoCBNode* node) ;

typedef struct CyAsCBQueue
{
    void *              head_p;
    void *              tail_p;
    uint32_t            count ;
    CyAsCBNodeType      type ;
} CyAsCBQueue ;

extern CyAsCBQueue *
CyAsCreateCBQueue(CyAsCBNodeType type) ;

extern void
CyAsDestroyCBQueue(CyAsCBQueue* queue) ;

/* Allocates a new CyAsCBNode */
extern void
CyAsInsertCBNode(CyAsCBQueue * queue_p, void* cbnode) ;

/* Removes the first CyAsCBNode from the queue and frees it */
extern void
CyAsRemoveCBNode(CyAsCBQueue * queue_p) ;

/* Remove the last CyAsCBNode from the queue and frees it */
extern void
CyAsRemoveCBTailNode(CyAsCBQueue *queue_p) ;

/* Removes and frees all pending callbacks */
extern void
CyAsClearCBQueue(CyAsCBQueue * queue_p) ;

extern CyAsReturnStatus_t
CyAsMiscSendRequest(CyAsDevice* dev_p,
                      CyAsFunctionCallback cb,
                      uint32_t client,
                      CyAsFunctCBType type,
                      void* data,
                      CyAsCBQueue* queue,
                      uint16_t req_type,
                      CyAsLLRequestResponse *req_p,
                      CyAsLLRequestResponse *reply_p,
                      CyAsResponseCallback rcb) ;

extern void
CyAsMiscCancelExRequests(CyAsDevice* dev_p) ;

/* Summary
   Free all memory allocated by and zero all structures initialized
   by CyAsUsbStart.
 */
extern void
CyAsUsbCleanup (
        CyAsDevice *dev_p) ;

/* Summary
   Free all memory allocated and zero all structures initialized
   by CyAsStorageStart.
 */
extern void
CyAsStorageCleanup (
        CyAsDevice *dev_p) ;
#endif

/* Summary
   This structure defines the data structure to support a given command context

   Description
   All commands send to the West Bridge device via the mailbox registers are sent via a context.
   Each context is independent and there can be a parallel stream of requests and responses on
   each context.  This structure is used to manage a single context.
*/
typedef struct CyAsContext
{
    uint8_t                         number ;                /* The context number for this context */
    CyAsHalSleepChannel             channel ;               /* This sleep channel is used to sleep while waiting on a response from the
                                                               West Bridge device for a request. */
    CyAsLLRequestResponse *         req_p ;                 /* The buffer for received requests */
    uint16_t                        request_length ;        /* The length of the request being received */
    CyAsResponseCallback            request_callback ;      /* The callback for the next request received */

    CyAsLLRequestListNode *         request_queue_p ;       /* A list of low level requests to go to the firmware */
    CyAsLLRequestListNode *         last_node_p ;           /* The list node in the request queue */

    uint16_t                        queue_index ;           /* Index upto which data is stored. */
    uint16_t                        rqt_index ;             /* Index to the next request in the queue. */
    uint16_t                        data_queue[128] ;       /* Queue of data stored */

} CyAsContext ;

#define CyAsContextIsWaiting(ctxt) ((ctxt)->state & CY_AS_CTXT_STATE_WAITING_RESPONSE)
#define CyAsContextSetWaiting(ctxt) ((ctxt)->state |= CY_AS_CTXT_STATE_WAITING_RESPONSE)
#define CyAsContextClearWaiting(ctxt) ((ctxt)->state &= ~CY_AS_CTXT_STATE_WAITING_RESPONSE)



/* Summary
   This data structure stores SDIO function parameters for a SDIO card

   Description
*/
typedef struct CyAsSDIODevice
{
    uint8_t     function_init_map; /* Keeps track of IO functions initialized*/
    uint8_t     function_suspended_map;
    CyAsSDIOCard card;            /* Function 0 (Card Common) properties*/
    CyAsSDIOFunc function[7];   /* Function 1-7 (Mapped to array element 0-6) properties.*/

}CyAsSDIODevice;

/* Summary
Macros to access the SDIO card properties
*/
//GetFunction Code
#define CyAsSdioGetFunctionCode(handle,bus,i)       ((CyAsDevice *)handle)->sdiocard[bus].function[i-1].function_code

//Get Function Extended Code
#define CyAsSdioGetFunctionExtCode(handle,bus,i)    ((CyAsDevice *)handle)->sdiocard[bus].function[i-1].extended_func_code

//Get Function Product Serial number
#define CyAsSdioGetFunctionPSN(handle,bus,i)        ((CyAsDevice *)handle)->sdiocard[bus].function[i-1].card_psn

//Get Function Block Size
#define CyAsSdioGetFunctionBlocksize(handle,bus,i)  ((CyAsDevice *)handle)->sdiocard[bus].function[i-1].blocksize

//Get Function Max Block Size
#define CyAsSdioGetFunctionMaxBlocksize(handle,bus,i)  ((CyAsDevice *)handle)->sdiocard[bus].function[i-1].maxblocksize

//Get Function CSA support
#define CyAsSdioGetFunctionCsaSupport(handle,bus,i) ((CyAsDevice *)handle)->sdiocard[bus].function[i-1].csa_bits

//Get Function Wakeup Support
#define CyAsSdioGetFunctionWakeupSupport(handle,bus,i) ((CyAsDevice *)handle)->sdiocard[bus].function[i-1]. wakeup_support

#define CyAsSdioSetFunctionBlockSize(handle,bus,i,blocksize) (((CyAsDevice *)handle)->sdiocard[bus].function[i-1].blocksize = blocksize)

//Get Number of funtions on card
#define CyAsSdioGetCardNumFunctions(handle,bus)     ((CyAsDevice *)handle)->sdiocard[bus].card.num_functions

//Check if memory is present on the card
#define CyAsSdioGetCardMemPresent(handle,bus)       ((CyAsDevice *)handle)->sdiocard[bus].card.memory_present

//Get Card manufaturer ID
#define CyAsSdioGetCardManfId(handle,bus)           ((CyAsDevice *)handle)->sdiocard[bus].card.manufacturer_Id

//Get Card manufacturer Information
#define CyAsSdioGetCardManfInfo(handle,bus)         ((CyAsDevice *)handle)->sdiocard[bus].card.manufacturer_info

//Get Card Block Size
#define CyAsSdioGetCardBlocksize(handle,bus)        ((CyAsDevice *)handle)->sdiocard[bus].card.blocksize

//Get Card max Block Size
#define CyAsSdioGetCardMaxBlocksize(handle,bus)        ((CyAsDevice *)handle)->sdiocard[bus].card.maxblocksize

//Get SDIO version supported by card
#define CyAsSdioGetCardSDIOVersion(handle,bus)      ((CyAsDevice *)handle)->sdiocard[bus].card.sdio_version

//Get Card capabillity register
#define CyAsSdioGetCardCapability(handle,bus)       ((CyAsDevice *)handle)->sdiocard[bus].card.card_capability

//Get function initialization map
#define CyAsSdioGetFunctionInitMap(handle,bus)      ((CyAsDevice *)handle)->sdiocard[bus].function_init_map

//Check if function i has been initialized
#define CyAsSdioCheckFunctionInitialized(handle,bus,i) (((CyAsSdioGetFunctionInitMap(handle,bus))& (0x01<<i))?1:0)

//Set the Card functio 0 block size
#define CyAsSdioSetCardBlockSize(handle,bus,blocksize) (((CyAsDevice *)handle)->sdiocard[bus].card.blocksize = blocksize)

//Check if the Card supports Bus suspend.
#define CyAsSdioCheckSupportBusSuspend(handle,bus)      ((CyAsSdioGetCardCapability(handle,bus) & CY_SDIO_SBS)?1:0)

//Check if a fuction is in suspended state
#define CyAsSdioCheckFunctionSuspended(handle,bus,i)    ((((CyAsDevice *)handle)->sdiocard[bus].function_suspended_map & (0x01<<i))?1:0)

//Set a function state to suspended
#define CyAsSdioSetFunctionSuspended(handle,bus,i)      ((((CyAsDevice *)handle)->sdiocard[bus].function_suspended_map) |= (0x01<<i))

//Clear a function suspended state
#define CyAsSdioClearFunctionSuspended(handle,bus,i)      ((((CyAsDevice *)handle)->sdiocard[bus].function_suspended_map) &= (~(0x01<<i)))

/* Summary
   This data structure represents a single device.

   Description
*/
struct CyAsDevice
{
    /* General stuff */
    uint32_t                    sig ;                       /* A signature to insure we have a valid handle */
    uint16_t                    silicon_id ;                /* The ID of the silicon */
    struct CyAsDevice *         next_p ;                    /* Pointer to the next device */
    CyAsHalDeviceTag            tag ;                       /* This is the client specific tag for this device */
    uint32_t                    state ;                     /* This contains various state information about the device */
    CyBool                      use_int_drq ;               /* Flag indicating whether INT# pin is used for DRQ */

    /* DMA related */
    CyAsDmaEndPoint     *       endp[16] ;                  /* The endpoint pointers associated with this device */
    CyAsDmaQueueEntry *         dma_freelist_p ;            /* List of queue entries that can be used for xfers */

    /* Low level comm related */
    CyAsContext *               context[CY_RQT_CONTEXT_COUNT] ; /* The contexts available in this device */
    CyAsReturnStatus_t          ll_error ;                  /* The low level error returned from sending an async request */
    CyBool                      ll_sending_rqt ;            /* A request is currently being sent to West Bridge. */
    CyBool                      ll_abort_curr_rqt ;         /* The current mailbox request should be aborted. */
    CyBool                      ll_queued_data ;            /* Indicates that the LL layer has queued mailbox data. */

    /* MISC API related */
    CyAsMiscEventCallback       misc_event_cb ;             /* Misc callback */

    /* Storage Related */
    uint32_t                    storage_count ;             /* The reference count for the Storage API */
    CyAsStorageEventCallback_dep    storage_event_cb ;      /* Callback for storage events */
    CyAsStorageEventCallback  storage_event_cb_ms ;         /* V1.2+ callback for storage events */
    CyAsReturnStatus_t          storage_error ;             /* The error for a sleeping storage operation */
    CyBool                      storage_wait ;              /* Flag indicating that the storage stack is waiting for an operation */
    CyAsLLRequestResponse *     storage_rw_req_p ;          /* Request used for storage read/writes. */
    CyAsLLRequestResponse *     storage_rw_resp_p ;         /* Response used for storage read/writes. */
    CyAsStorageCallback_dep     storage_cb ;                /* The storage callback */
    CyAsStorageCallback         storage_cb_ms ;             /* The V1.2+ storage callback */
    CyAsBusNumber_t             storage_bus_index ;         /* The bus index for the currently outstanding async storage request */
    uint32_t                    storage_device_index ;      /* The device index for the currently outstanding async storage request */
    uint32_t                    storage_unit ;              /* The unit index for the currently outstanding async storage request */
    uint32_t                    storage_block_addr ;        /* The block address for the currently outstanding async storage request */
    CyAsOperType                storage_oper ;              /* The operation for the currently outstanding async storage request */
    CyAsEndPointNumber_t        storage_read_endpoint ;     /* The endpoint used to read Storage data */
    CyAsEndPointNumber_t        storage_write_endpoint ;    /* The endpoint used to write endpoint data */
    CyAsDeviceDesc              storage_device_info[CY_AS_MAX_BUSES][CY_AS_MAX_STORAGE_DEVICES] ;
    /* The information on each device on each bus */

    /* USB Related */
    uint16_t                    epasync ;                   /* This conatins the endpoint async state */
    uint32_t                    usb_count ;                 /* The reference count for the USB API */
    uint8_t                     usb_phy_config ;            /* The physical endpoint configuration */
    CyAsCBQueue *               usb_func_cbs ;              /* The callbacks for async func calls */
    CyAsUsbEndPointConfig       usb_config[16] ;            /* Endpoint configuration information */
    CyAsUsbEventCallback_dep    usb_event_cb ;              /* The USB callback */
    CyAsUsbEventCallback        usb_event_cb_ms ;           /* The V1.2+ USB callback */
    CyAsReturnStatus_t          usb_error ;                 /* The error for a sleeping usb operation */
    CyAsUsbIoCallback           usb_cb[16] ;                /* The USB callback for a pending storage operation */
    void *                      usb_pending_buffer ;        /* The buffer pending from a USB operation */
    uint32_t                    usb_pending_size ;          /* The size of the buffer pending from a USB operation */
    CyBool                      usb_spacket[16] ;           /* If true, send a short packet */
    uint32_t                    usb_actual_cnt ;            /* The amount of data actually xferred */
    uint8_t                     usb_ep1cfg[2] ;             /* EP1OUT and EP1IN config register contents */
    uint16_t                    usb_lepcfg[10] ;            /* LEP config register contents */
    uint16_t                    usb_pepcfg[4] ;             /* PEP config register contents */
    uint8_t *                   usb_ep_data ;               /* Buffer for EP0 and EP1 data sent via mailboxes */
    uint32_t                    usb_delay_ack_count ;       /* Used to track how many ack requests are pending */
    uint32_t                    usb_max_tx_size ;           /* Maximum transfer size for USB endpoints. */

    CyAsLLRequestResponse *     usb_ep0_dma_req ;           /* Request for sending EP0 data to West Bridge */
    CyAsLLRequestResponse *     usb_ep0_dma_resp ;          /* Response for EP0 data sent to West Bridge */
    CyAsLLRequestResponse *     usb_ep1_dma_req ;           /* Request for sending EP1 data to West Bridge */
    CyAsLLRequestResponse *     usb_ep1_dma_resp ;          /* Response for EP1 data sent to West Bridge */

    CyAsLLRequestResponse *     usb_ep0_dma_req_save ;
    CyAsLLRequestResponse *     usb_ep0_dma_resp_save ;

    /* MTP Related */
    uint32_t                    mtp_count ;                 /* The reference count for the MTP API */
    CyAsMTPEventCallback        mtp_event_cb ;              /* The MTP event callback supplied by the client */
    CyAsMTPBlockTable*          mtp_blk_tbl ;               /* The current block table to be transfered */

    CyAsCBQueue *               func_cbs_mtp ;
    CyAsCBQueue *               func_cbs_usb ;
    CyAsCBQueue *               func_cbs_stor ;
    CyAsCBQueue *               func_cbs_misc ;
    CyAsCBQueue *               func_cbs_res ;

    CyAsUsbEvent                usb_last_event ;                     /* The last USB event that was received */
    uint8_t                     media_supported[CY_AS_MAX_BUSES] ;   /* Types of storage media supported by the firmware */

    CyAsSDIODevice   sdiocard[CY_AS_MAX_BUSES];                      /* SDIO card parameters*/
    CyBool                      is_mtp_firmware ;                    /* if true, MTP enabled Firmware. */
    CyBool                      is_mtp_data_pending ;                /* if true, mailbox message has come already */
    CyBool                      mtp_turbo_active ;                   /* True between the time an Init was called and the complete event is generated */
    uint16_t                    mtp_data_len ;                       /* mbox reported EP 2 data len */
    CyAsReturnStatus_t          mtp_error ;                          /* The error for mtp EP4 write operation */
    CyAsFunctionCallback        mtp_cb ;                             /* mtp send/get operation callback */
    uint32_t                    mtp_client ;                         /* mtp send/get operation client id */
    CyAsFunctCBType             mtp_op ;                             /* mtp operation type. To be used in callback */

    CyBool                      is_storage_only_mode ;               /* Firmware is running in P2S only mode. */
    uint32_t                    stby_int_mask ;                      /* Interrupt mask value during device standby. */
} ;

#define CyAsDeviceIsConfigured(dp) ((dp)->state & CY_AS_DEVICE_STATE_CONFIGURED)
#define CyAsDeviceSetConfigured(dp) ((dp)->state |= CY_AS_DEVICE_STATE_CONFIGURED)
#define CyAsDeviceSetUnconfigured(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_CONFIGURED)

#define CyAsDeviceIsDmaRunning(dp)      ((dp)->state & CY_AS_DEVICE_STATE_DMA_MODULE)
#define CyAsDeviceSetDmaRunning(dp)     ((dp)->state |= CY_AS_DEVICE_STATE_DMA_MODULE)
#define CyAsDeviceSetDmaStopped(dp)     ((dp)->state &= ~CY_AS_DEVICE_STATE_DMA_MODULE)

#define CyAsDeviceIsLowLevelRunning(dp) ((dp)->state & CY_AS_DEVICE_STATE_LOWLEVEL_MODULE)
#define CyAsDeviceSetLowLevelRunning(dp) ((dp)->state |= CY_AS_DEVICE_STATE_LOWLEVEL_MODULE)
#define CyAsDeviceSetLowLevelStopped(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_LOWLEVEL_MODULE)

#define CyAsDeviceIsIntrRunning(dp) ((dp)->state & CY_AS_DEVICE_STATE_INTR_MODULE)
#define CyAsDeviceSetIntrRunning(dp) ((dp)->state |= CY_AS_DEVICE_STATE_INTR_MODULE)
#define CyAsDeviceSetIntrStopped(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_INTR_MODULE)

#define CyAsDeviceIsFirmwareLoaded(dp) ((dp)->state & CY_AS_DEVICE_STATE_FIRMWARE_LOADED)
#define CyAsDeviceSetFirmwareLoaded(dp) ((dp)->state |= CY_AS_DEVICE_STATE_FIRMWARE_LOADED)
#define CyAsDeviceSetFirmwareNotLoaded(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_FIRMWARE_LOADED)

#define CyAsDeviceIsStorageRunning(dp) ((dp)->state & CY_AS_DEVICE_STATE_STORAGE_MODULE)
#define CyAsDeviceSetStorageRunning(dp) ((dp)->state |= CY_AS_DEVICE_STATE_STORAGE_MODULE)
#define CyAsDeviceSetStorageStopped(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_STORAGE_MODULE)

#define CyAsDeviceIsUsbRunning(dp) ((dp)->state & CY_AS_DEVICE_STATE_USB_MODULE)
#define CyAsDeviceSetUsbRunning(dp) ((dp)->state |= CY_AS_DEVICE_STATE_USB_MODULE)
#define CyAsDeviceSetUsbStopped(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_USB_MODULE)

#define CyAsDeviceWantsScsiMessages(dp) (((dp)->state & CY_AS_DEVICE_STATE_STORAGE_SCSIMSG) ? CyTrue : CyFalse)
#define CyAsDeviceSetScsiMessages(dp) ((dp)->state |= CY_AS_DEVICE_STATE_STORAGE_SCSIMSG)
#define CyAsDeviceClearScsiMessages(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_STORAGE_SCSIMSG)

#define CyAsDeviceIsStorageAsyncPending(dp) ((dp)->state & CY_AS_DEVICE_STATE_STORAGE_ASYNC_PENDING)
#define CyAsDeviceSetStorageAsyncPending(dp) ((dp)->state |= CY_AS_DEVICE_STATE_STORAGE_ASYNC_PENDING)
#define CyAsDeviceClearStorageAsyncPending(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_STORAGE_ASYNC_PENDING)

#define CyAsDeviceIsUsbConnected(dp) ((dp)->state & CY_AS_DEVICE_STATE_USB_CONNECTED)
#define CyAsDeviceSetUsbConnected(dp) ((dp)->state |= CY_AS_DEVICE_STATE_USB_CONNECTED)
#define CyAsDeviceClearUsbConnected(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_USB_CONNECTED)

#define CyAsDeviceIsUsbHighSpeed(dp) ((dp)->state & CY_AS_DEVICE_STATE_USB_HIGHSPEED)
#define CyAsDeviceSetUsbHighSpeed(dp) ((dp)->state |= CY_AS_DEVICE_STATE_USB_HIGHSPEED)
#define CyAsDeviceClearUsbHighSpeed(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_USB_HIGHSPEED)

#define CyAsDeviceIsInCallback(dp) ((dp)->state & CY_AS_DEVICE_STATE_IN_CALLBACK)
#define CyAsDeviceSetInCallback(dp) ((dp)->state |= CY_AS_DEVICE_STATE_IN_CALLBACK)
#define CyAsDeviceClearInCallback(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_IN_CALLBACK)

#define CyAsDeviceIsSetupIOPerformed(dp) ((dp)->state & CY_AS_DEVICE_STATE_SETUP_IO_PERFORMED)
#define CyAsDeviceSetSetupIOPerformed(dp) ((dp)->state |= CY_AS_DEVICE_STATE_SETUP_IO_PERFORMED)
#define CyAsDeviceClearSetupIOPerformed(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_SETUP_IO_PERFORMED)

#define CyAsDeviceIsAckDelayed(dp) ((dp)->usb_delay_ack_count > 0)
#define CyAsDeviceSetAckDelayed(dp) ((dp)->usb_delay_ack_count++)
#define CyAsDeviceRemAckDelayed(dp) ((dp)->usb_delay_ack_count--)
#define CyAsDeviceClearAckDelayed(dp) ((dp)->usb_delay_ack_count = 0)

#define CyAsDeviceIsSetupPacket(dp) ((dp)->state & CY_AS_DEVICE_STATE_IN_SETUP_PACKET)
#define CyAsDeviceSetSetupPacket(dp) ((dp)->state |= CY_AS_DEVICE_STATE_IN_SETUP_PACKET)
#define CyAsDeviceClearSetupPacket(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_IN_SETUP_PACKET)

#define CyAsDeviceIsEp0Stalled(dp) ((dp)->state & CY_AS_DEVICE_STATE_EP0_STALLED)
#define CyAsDeviceSetEp0Stalled(dp) ((dp)->state |= CY_AS_DEVICE_STATE_EP0_STALLED)
#define CyAsDeviceClearEp0Stalled(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_EP0_STALLED)

#define CyAsDeviceIsRegisterStandby(dp) ((dp)->state & CY_AS_DEVICE_STATE_REGISTER_STANDBY)
#define CyAsDeviceSetRegisterStandby(dp) ((dp)->state |= CY_AS_DEVICE_STATE_REGISTER_STANDBY)
#define CyAsDeviceClearRegisterStandby(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_REGISTER_STANDBY)

#define CyAsDeviceIsPinStandby(dp) ((dp)->state & CY_AS_DEVICE_STATE_PIN_STANDBY)
#define CyAsDeviceSetPinStandby(dp) ((dp)->state |= CY_AS_DEVICE_STATE_PIN_STANDBY)
#define CyAsDeviceClearPinStandby(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_PIN_STANDBY)

#define CyAsDeviceIsCrystal(dp) ((dp)->state & CY_AS_DEVICE_STATE_CRYSTAL)
#define CyAsDeviceIsExternalClock(dp) (!((dp)->state & CY_AS_DEVICE_STATE_CRYSTAL))
#define CyAsDeviceSetCrystal(dp) ((dp)->state |= CY_AS_DEVICE_STATE_CRYSTAL)
#define CyAsDeviceSetExternalClock(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_CRYSTAL)

#define CyAsDeviceIsWaking(dp) ((dp)->state & CY_AS_DEVICE_STATE_WAKING)
#define CyAsDeviceSetWaking(dp) ((dp)->state |= CY_AS_DEVICE_STATE_WAKING)
#define CyAsDeviceClearWaking(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_WAKING)

#define CyAsDeviceIsInSuspendMode(dp) ((dp)->state & CY_AS_DEVICE_STATE_SUSPEND)
#define CyAsDeviceSetSuspendMode(dp) ((dp)->state |= CY_AS_DEVICE_STATE_SUSPEND)
#define CyAsDeviceClearSuspendMode(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_SUSPEND)

#define CyAsDeviceIsResetPending(dp) ((dp)->state & CY_AS_DEVICE_STATE_RESETP)
#define CyAsDeviceSetResetPending(dp) ((dp)->state |= CY_AS_DEVICE_STATE_RESETP)
#define CyAsDeviceClearResetPending(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_RESETP)

#define CyAsDeviceIsStandbyPending(dp) ((dp)->state & CY_AS_DEVICE_STATE_STANDP)
#define CyAsDeviceSetStandbyPending(dp) ((dp)->state |= CY_AS_DEVICE_STATE_STANDP)
#define CyAsDeviceClearStandbyPending(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_STANDP)

#define CyAsDeviceIsSSSPending(dp) ((dp)->state & CY_AS_DEVICE_STATE_SSSP)
#define CyAsDeviceSetSSSPending(dp) ((dp)->state |= CY_AS_DEVICE_STATE_SSSP)
#define CyAsDeviceClearSSSPending(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_SSSP)

#define CyAsDeviceIsUSSPending(dp) ((dp)->state & CY_AS_DEVICE_STATE_USSP)
#define CyAsDeviceSetUSSPending(dp) ((dp)->state |= CY_AS_DEVICE_STATE_USSP)
#define CyAsDeviceClearUSSPending(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_USSP)

#define CyAsDeviceIsMSSPending(dp) ((dp)->state & CY_AS_DEVICE_STATE_MSSP)
#define CyAsDeviceSetMSSPending(dp) ((dp)->state |= CY_AS_DEVICE_STATE_MSSP)
#define CyAsDeviceClearMSSPending(dp) ((dp)->state &= ~CY_AS_DEVICE_STATE_MSSP)

#define CyAsDeviceIsUsbAsyncPending(dp, ep) ((dp)->epasync & (1 << ep))
#define CyAsDeviceSetUsbAsyncPending(dp, ep) ((dp)->epasync |= (1 << ep))
#define CyAsDeviceClearUsbAsyncPending(dp, ep) ((dp)->epasync &= ~(1 << ep))

#define CyAsDeviceIsNandStorageSupported(dp) ((dp)->media_supported[0] & 1)

/* Macros to check the type of West Bridge device. */
#define CyAsDeviceIsAstoriaDev(dp) (((dp)->silicon_id == CY_AS_MEM_CM_WB_CFG_ID_HDID_ASTORIA_VALUE) || \
        ((dp)->silicon_id == CY_AS_MEM_CM_WB_CFG_ID_HDID_ASTORIA_FPGA_VALUE))
#define CyAsDeviceIsAntiochDev(dp) ((dp)->silicon_id == CY_AS_MEM_CM_WB_CFG_ID_HDID_ANTIOCH_VALUE)

#ifdef CY_AS_LOG_SUPPORT
extern void CyAsLogDebugMessage(int value, const char *msg) ;
#else
#define CyAsLogDebugMessage(value, msg)
#endif

/* Summary
   This function finds the device object given the HAL tag

   Description
   The user associats a device TAG with each West Bridge device created.  This tag is passed from the
   API functions to and HAL functions that need to ID a specific West Bridge device.  This tag is also
   passed in from the user back into the API via interrupt functions.  This function allows the API
   to find the device structure associated with a given tag.

   Notes
   This function does a simple linear search for the device based on the TAG.  This function is
   called each time an West Bridge interrupt handler is called.  Therefore this works fine for a small
   number of West Bridge devices (e.g. less than five).  Anything more than this and this methodology
   will need to be updated.

   Returns
   Pointer to a CyAsDevice associated with the tag
*/
extern CyAsDevice *
CyAsDeviceFindFromTag(
        CyAsHalDeviceTag tag
        ) ;

#include "cyas_cplus_end.h"

#endif          /* __INCLUDED_CYASDEVICE_H__ */
