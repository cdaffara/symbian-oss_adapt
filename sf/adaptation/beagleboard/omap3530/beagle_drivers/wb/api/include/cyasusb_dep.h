/* Cypress West Bridge API header file (cyasusb_dep.h)
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
 * This header will contain Antioch specific declaration
 * of the APIs that are deprecated in Astoria SDK. This is
 * for maintaining backward compatibility.
 */

#ifndef __INCLUDED_CYASUSB_DEP_H__
#define __INCLUDED_CYASUSB_DEP_H__

#ifndef __doxygen__

/* 
   This data structure is the data passed via the evdata paramater on a usb event
   callback for the inquiry request.
*/

typedef struct CyAsUsbInquiryData_dep
{
    CyAsMediaType media ;	/* The media for the event */
    uint8_t evpd ;		    /* The EVPD bit from the SCSI INQUIRY request */
    uint8_t codepage ;		/* The codepage in the inquiry request */
    CyBool updated ;		/* This bool must be set to CyTrue indicate that the inquiry
				               data was changed */
    uint16_t length ;		/* The length of the data */
    void *data ;		    /* The inquiry data */
} CyAsUsbInquiryData_dep ;


typedef struct CyAsUsbUnknownCommandData_dep
{
    CyAsMediaType media ;	/* The media for the event */
    uint16_t reqlen ;		/* The length of the requst (should be 16 bytes) */
    void *request ;		/* The request */

    uint8_t status ;		/* The returned status value for the command */
    uint8_t key ;		/* If status is failed, the sense key */
    uint8_t asc ;		/* If status is failed, the additional sense code */
    uint8_t ascq ;		/* If status if failed, the additional sense code qualifier */
} CyAsUsbUnknownCommandData_dep ;


typedef struct CyAsUsbStartStopData_dep
{
    CyAsMediaType media ;	/* The media type for the event */
    CyBool start ;		/* CyTrue means start request, CyFalse means stop request */
    CyBool loej ;		/* CyTrue means LoEj bit set, otherwise false */
} CyAsUsbStartStopData_dep ;


typedef struct CyAsUsbEnumControl_dep
{
    uint8_t enum_mass_storage ;		/* The bits in this member determine which mass storage devices
							        are enumerated.  See CyAsUsbMassStorageEnum for more details. */
    CyBool antioch_enumeration ;	/* If true, West Bridge will control enumeration.  If this is false the
							        Pport controls enumeration.  If the P Port is controlling 
							        enumeration, traffic will be received via endpoint zero. */
    uint8_t mass_storage_interface ;/* This is the interface # to use for the mass storage interface, 
							        if mass storage is enumerated.  If mass storage is not enumerated 
							        this value should be zero. */
    CyBool mass_storage_callbacks ;	/* If true, Inquiry, START/STOP, and unknown mass storage
							        requests cause a callback to occur for handling by the
							        baseband processor. */
} CyAsUsbEnumControl_dep ;


typedef void (*CyAsUsbEventCallback_dep)(
    CyAsDeviceHandle			handle,		/* Handle to the device to configure */
    CyAsUsbEvent			ev,		/* The event type being reported */
    void *				evdata		/* The data assocaited with the event being reported */
) ;



/* Register Callback api */
EXTERN CyAsReturnStatus_t
CyAsUsbRegisterCallback_dep(
	CyAsDeviceHandle		        handle,		/* Handle to the West Bridge device */
	CyAsUsbEventCallback_dep		callback	/* The function to call */
	) ;


extern CyAsReturnStatus_t
CyAsUsbSetEnumConfig_dep(
	CyAsDeviceHandle		    handle,		/* Handle to the West Bridge device */
	CyAsUsbEnumControl_dep *	config_p,	/* The USB configuration information */
	CyAsFunctionCallback		cb,         /* The callback if async call */
	uint32_t			        client      /* Client supplied data */
	) ;


extern CyAsReturnStatus_t
CyAsUsbGetEnumConfig_dep(
	CyAsDeviceHandle		    handle,		/* Handle to the West Bridge device */
	CyAsUsbEnumControl_dep*		config_p,	/* The return value for USB congifuration information */
	CyAsFunctionCallback		cb,         /* The callback if async call */
	uint32_t			        client      /* Client supplied data */
	) ;

extern CyAsReturnStatus_t
CyAsUsbGetDescriptor_dep(
	CyAsDeviceHandle	handle,		/* Handle to the West Bridge device */
	CyAsUsbDescType		type,		/* The type of descriptor */
	uint8_t				index,		/* Index for string descriptor */
	void *				desc_p,		/* The buffer to hold the returned descriptor */
	uint32_t *			length_p	/* This is an input and output parameter.  Before the code this pointer
							        points to a uint32_t that contains the length of the buffer.  After
							        the call, this value contains the amount of data actually returned. */
	) ;

extern CyAsReturnStatus_t
CyAsUsbSetStall_dep(
	CyAsDeviceHandle		handle,		/* Handle to the West Bridge device */
	CyAsEndPointNumber_t		ep,		/* The endpoint of interest */
	CyAsUsbFunctionCallback		cb,		/* The callback if async call */
	uint32_t			client		/* Client supplied data */
) ;

EXTERN CyAsReturnStatus_t 
CyAsUsbClearStall_dep(
	CyAsDeviceHandle		handle,		/* Handle to the West Bridge device */
	CyAsEndPointNumber_t		ep,		/* The endpoint of interest */
	CyAsUsbFunctionCallback		cb,		/* The callback if async call */
	uint32_t			client		/* Client supplied data */
	) ;

EXTERN CyAsReturnStatus_t
CyAsUsbSetNak_dep(
	CyAsDeviceHandle		handle,		/* Handle to the West Bridge device */
	CyAsEndPointNumber_t		ep,		/* The endpoint of interest */
	CyAsUsbFunctionCallback		cb,		/* The callback if async call */
	uint32_t			client		/* Client supplied data */
) ;

EXTERN CyAsReturnStatus_t 
CyAsUsbClearNak_dep(
	CyAsDeviceHandle		handle,		/* Handle to the West Bridge device */
	CyAsEndPointNumber_t		ep,		/* The endpoint of interest */
	CyAsUsbFunctionCallback		cb,		/* The callback if async call */
	uint32_t			client		/* Client supplied data */
	) ;

EXTERN CyAsReturnStatus_t
CyAsUsbSelectMSPartitions_dep (
        CyAsDeviceHandle                handle,
        CyAsMediaType                   media,
        uint32_t                        device,
        CyAsUsbMSType_t                 type,
        CyAsFunctionCallback            cb,
        uint32_t                        client
        ) ;

#endif /*__doxygen*/

#endif /*__INCLUDED_CYANSTORAGE_DEP_H__*/
