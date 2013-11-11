/* Cypress West Bridge API header file (cyaslowlevel.h)
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

#ifndef _INCLUDED_CYASLOWLEVEL_H_
#define _INCLUDED_CYASLOWLEVEL_H_

/*@@Low Level Communications

    Summary
    The low level communications module is responsible for communications between
    the West Bridge device and the P port processor.  Communications is organized as
    a series of requests and subsequent responses.  For each request there is a
    one and only one response.  Requests may go from the West Bridge device to the P port
    processor, or from the P Port processor to the West Bridge device.

        Description
    Requests are issued across what is called a context.  A context is a single
    channel of communications from one processor to another processor.  There can
    be only a single request outstanding on a context at a given time.  Contexts
    are used to identify subsystems that can only process a single request at a
    time, but are independent of other contexts in the system.  For instance, there
    is a context for communicating storage commands from the P port processor to the
    West Bridge device.  There is also a context for communicating USB commands from 
    the P port processor to the West Bridge device.

    Requests and responses are identical with the exception of the type bit in the
    request/response header.  If the type bit is one, the packet is a request.  If
    this bit is zero, the packet is a response.  Also encoded within the header of
    the request/response is the code.  The code is a command code for a request, or
    a response code for a response.  For a request, the code is a function of the
    context.  The code 0 has one meaning for the storage context and a different 
    meaning for the USB context.  The code is treated differently in the response.
    If the code in the response is less than 16, then the meaning of the response is
    global across all contexts.  If the response is greater than or equal to 16, 
    then the response is specific to the associated context.

    Requests and responses are transferred between processors through the mailbox
    registers.  It may take one or more cycles to transmit a complete request or
    response.  The context is encoded into each cycle of the transfer to insure the
    receiving processor can route the data to the appropriate context for processing.
    In this way, the traffic from multiple contexts can be multiplexed into a single
    data stream through the mailbox registers by the sending processor, and 
    demultiplexed from the mailbox registers by the receiving processor.

    * Firmware Assumptions *
    The firmware assumes that mailbox contents will be consumed immediately.  Therefore
    for multi-cycle packets, the data is sent in a tight polling loop from the firmware.
    This implies that the data must be read from the mailbox register on the P port side
    and processed immediately or performance of the firmware will suffer.  In order to
    insure this is the case, the data from the mailboxes is read and stored immediately
    in a per context buffer.  This occurs until the entire packet is received at which
    time the request packet is processed.  Since the protocol is designed to allow for 
    only one outstanding packet at a time, the firmware can never be in a position of
    waiting on the mailbox registers while the P port is processing a request.  Only after
    the response to the previous request is sent will another request be sent.
*/

#include "cyashal.h"
#include "cyasdevice.h"

#include "cyas_cplus_start.h"

/*
 * Constants
 */
#define CY_AS_REQUEST_RESPONSE_CODE_MASK                                (0x00ff)
#define CY_AS_REQUEST_RESPONSE_CONTEXT_MASK                             (0x0F00)
#define CY_AS_REQUEST_RESPONSE_CONTEXT_SHIFT                            (8)
#define CY_AS_REQUEST_RESPONSE_TYPE_MASK                                (0x4000)
#define CY_AS_REQUEST_RESPONSE_LAST_MASK                                (0x8000)
#define CY_AS_REQUEST_RESPONSE_CLEAR_STR_FLAG                           (0x1000)

/*
 * These macros extract the data from a 16 bit value
 */
#define CyAsMboxGetCode(c) ((uint8_t)((c) & CY_AS_REQUEST_RESPONSE_CODE_MASK))
#define CyAsMboxGetContext(c) ((uint8_t)(((c) & CY_AS_REQUEST_RESPONSE_CONTEXT_MASK) >> CY_AS_REQUEST_RESPONSE_CONTEXT_SHIFT))
#define CyAsMboxIsLast(c) ((c) & CY_AS_REQUEST_RESPONSE_LAST_MASK)
#define CyAsMboxIsRequest(c) (((c) & CY_AS_REQUEST_RESPONSE_TYPE_MASK) != 0)
#define CyAsMboxIsResponse(c) (((c) & CY_AS_REQUEST_RESPONSE_TYPE_MASK) == 0)

/*
 * These macros (not yet written) pack data into or extract data 
 * from the m_box0 field of the request or response
 */
#define CyAsLLRequestResponse_SetCode(req, code) \
        ((req)->box0 = ((req)->box0 & ~CY_AS_REQUEST_RESPONSE_CODE_MASK) | \
                        (code & CY_AS_REQUEST_RESPONSE_CODE_MASK))

#define CyAsLLRequestResponse_GetCode(req) CyAsMboxGetCode((req)->box0)

#define CyAsLLRequestResponse_SetContext(req, context) \
        ((req)->box0 |= ((context) << CY_AS_REQUEST_RESPONSE_CONTEXT_SHIFT))

#define CyAsLLRequestResponse_SetClearStorageFlag(req) \
        ((req)->box0 |= CY_AS_REQUEST_RESPONSE_CLEAR_STR_FLAG)

#define CyAsLLRequestResponse_GetContext(req) CyAsMboxGetContext((req)->box0)

#define CyAsLLRequestResponse_IsLast(req) CyAsMboxIsLast((req)->box0)

#define CYAnLLRequestResponse__SetLast(req) \
        ((req)->box0 |= CY_AS_REQUEST_RESPONSE_LAST_MASK)

#define CyAsLLRequestResponse_IsRequest(req) CyAsMboxIsRequest((req)->box0)

#define CyAsLLRequestResponse_SetRequest(req) \
        ((req)->box0 |= CY_AS_REQUEST_RESPONSE_TYPE_MASK)

#define CyAsLLRequestResponse_SetResponse(req) \
        ((req)->box0 &= ~CY_AS_REQUEST_RESPONSE_TYPE_MASK)

#define CyAsLLRequestResponse_IsResponse(req) CyAsMboxIsResponse((req)->box0)

#define CyAsLLRequestResponse_GetWord(req, offset) ((req)->data[(offset)])

#define CyAsLLRequestResponse_SetWord(req, offset, value) ((req)->data[(offset)] = value)

typedef enum CyAsRemoveRequestResult_t
{
    CyAsRemoveRequestSucessful,
    CyAsRemoveRequestInTransit,
    CyAsRemoveRequestNotFound
} CyAsRemoveRequestResult_t ;

/* Summary
   Start the low level communications module

   Description
*/
CyAsReturnStatus_t
CyAsLLStart(
        CyAsDevice *dev_p
        ) ;

CyAsReturnStatus_t
CyAsLLStop(
   CyAsDevice *dev_p
   ) ;


CyAsLLRequestResponse *
CyAsLLCreateRequest(
        CyAsDevice *dev_p,
        uint16_t code, 
        uint8_t context, 
        uint16_t                                length          /* Length of the request in 16 bit words */
        ) ;

void 
CyAsLLInitRequest(
    CyAsLLRequestResponse *req_p, 
    uint16_t code, 
    uint16_t context, 
    uint16_t length) ;

void
CyAsLLInitResponse(
    CyAsLLRequestResponse *req_p, 
    uint16_t length) ;

void
CyAsLLDestroyRequest(
        CyAsDevice *dev_p,
        CyAsLLRequestResponse *) ;

CyAsLLRequestResponse *
CyAsLLCreateResponse(
        CyAsDevice *dev_p,
        uint16_t                                length  /* Length of the request in 16 bit words */
        ) ;

CyAsRemoveRequestResult_t
CyAsLLRemoveRequest(
        CyAsDevice *dev_p,
        CyAsContext *ctxt_p,
        CyAsLLRequestResponse *req_p,
        CyBool force
        ) ;
void
CyAsLLRemoveAllRequests(CyAsDevice *dev_p,
                        CyAsContext *ctxt_p) ;

void
CyAsLLDestroyResponse(
    CyAsDevice *dev_p,
    CyAsLLRequestResponse *) ;

CyAsReturnStatus_t 
CyAsLLSendRequest(
    CyAsDevice *                    dev_p,              /* The West Bridge device */
    CyAsLLRequestResponse *         req,                /* The request to send */
    CyAsLLRequestResponse *         resp,               /* Storage for a reply, must be sure it is of sufficient size */
    CyBool                          sync,               /* If true, this is a sync request */
    CyAsResponseCallback            cb                  /* Callback to call when reply is received */
) ;

CyAsReturnStatus_t
CyAsLLSendRequestWaitReply(
    CyAsDevice *                        dev_p,          /* The West Bridge device */
    CyAsLLRequestResponse *     req,            /* The request to send */
    CyAsLLRequestResponse *     resp            /* Storage for a reply, must be sure it is of sufficient size */
) ;

/* Summary
   This function registers a callback function to be called when a request arrives on a given
   context.

   Description

   Returns
   * CY_AS_ERROR_SUCCESS
*/
extern CyAsReturnStatus_t
CyAsLLRegisterRequestCallback(
        CyAsDevice *dev_p,
        uint8_t context,
        CyAsResponseCallback cb
        ) ;

/* Summary
   This function packs a set of bytes given by the data_p pointer into a request, reply
   structure.
*/
extern void
CyAsLLRequestResponse_Pack(
        CyAsLLRequestResponse *req,                     /* The destintation request or response */
        uint32_t offset,                                        /* The offset of where to pack the data */
        uint32_t length,                                        /* The length of the data to pack in bytes */
        void *data_p                                            /* The data to pack */
        ) ;

/* Summary
   This function unpacks a set of bytes from a request/reply structure into a segment of memory given
   by the data_p pointer.
*/
extern void 
CyAsLLRequestResponse_Unpack(
        CyAsLLRequestResponse *req,                     /* The source of the data to unpack */
        uint32_t offset,                                        /* The offset of the data to unpack */
        uint32_t length,                                        /* The length of the data to unpack in bytes */
        void *data_p                                            /* The destination of the unpack operation */
        ) ;

/* Summary
   This function sends a status response back to the West Bridge device in response to a 
   previously send request
*/
extern CyAsReturnStatus_t
CyAsLLSendStatusResponse(
        CyAsDevice *dev_p,                                      /* The West Bridge device */
        uint8_t context,                                        /* The context to send the response on */
        uint16_t code,                                          /* The success/failure code to send */
        uint8_t clear_storage) ;                                /* Flag to clear wait on storage context */

/* Summary
   This function sends a response back to the West Bridge device.

   Description
   This function sends a response back to the West Bridge device.  The response is sent on the
   context given by the 'context' variable.  The code for the response is given by the 'code'
   argument.  The data for the response is given by the data and length arguments.
*/
extern CyAsReturnStatus_t
CyAsLLSendDataResponse(
    CyAsDevice *dev_p,                                  /* The West Bridge device */
    uint8_t context,                                    /* The context to send the response on */
    uint16_t code,                                      /* The response code to use */
    uint16_t length,                                    /* The length of the data for the response */
    void *data                                          /* The data for the response */
) ;

/* Summary
   This function removes any requests of the given type from the given context.

   Description
   This function removes requests of a given type from the context given via the
   context number.
*/
extern CyAsReturnStatus_t
CyAsLLRemoveEpDataRequests(
    CyAsDevice *dev_p,                                  /* The West Bridge device */
    CyAsEndPointNumber_t ep
    ) ;

#include "cyas_cplus_end.h"

#endif                  /* _INCLUDED_CYASLOWLEVEL_H_ */
