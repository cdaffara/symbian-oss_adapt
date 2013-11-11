/* Cypress West Bridge API header file (cyanstorage_dep.h)
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

/* This header will contain Antioch specific declaration
 * of the APIs that are deprecated in Astoria SDK. This is
 * for maintaining backward compatibility
 */
#ifndef __INCLUDED_CYANSTORAGE_DEP_H__
#define __INCLUDED_CYANSTORAGE_DEP_H__

#ifndef __doxygen__

typedef void (*CyAsStorageCallback_dep)(
	CyAsDeviceHandle handle,			/* Handle to the device completing the storage operation */
	CyAsMediaType type,					/* The media type completing the operation */
	uint32_t device,					/* The device completing the operation */
	uint32_t unit,						/* The unit completing the operation */
	uint32_t block_number,				/* The block number of the completed operation */
	CyAsOperType op,					/* The type of operation */
	CyAsReturnStatus_t status			/* The error status */
	) ;

typedef void (*CyAsStorageEventCallback_dep)(
	CyAsDeviceHandle handle,			/* Handle to the device sending the event notification */
	CyAsMediaType type,					/* The media type */
	CyAsStorageEvent evtype,			/* The event type */
	void *evdata						/* Event related data */
	) ;

typedef struct CyAsStorageQueryDeviceData_dep
{
    CyAsMediaType	type ;		/* The type of media to query */
    uint32_t		device ;	/* The logical device number to query */
    CyAsDeviceDesc 	desc_p ;	/* The return value for the device descriptor */
} CyAsStorageQueryDeviceData_dep ;

typedef struct CyAsStorageQueryUnitData_dep
{
	CyAsMediaType	type ;		/* The type of media to query */
	uint32_t	device ;	/* The logical device number to query */
	uint32_t	unit ;		/* The unit to query on the device */
	CyAsUnitDesc 	desc_p ;	/* The return value for the unit descriptor */
} CyAsStorageQueryUnitData_dep ;


/************ FUNCTIONS *********************/

EXTERN CyAsReturnStatus_t
CyAsStorageRegisterCallback_dep(
	CyAsDeviceHandle	        handle,		/* Handle to the device of interest */
	CyAsStorageEventCallback_dep	callback	/* The callback function to call for async storage events */
	) ;

EXTERN CyAsReturnStatus_t 
CyAsStorageClaim_dep(CyAsDeviceHandle handle, 
		   CyAsMediaType type
           );

EXTERN CyAsReturnStatus_t
CyAsStorageClaim_dep_EX(
	CyAsDeviceHandle		handle,		/* Handle to the device of interest */
	CyAsMediaType*			type,		/* The type of media to claim */
	CyAsFunctionCallback	cb,         /* Callback to be called when the operation is complete */
	uint32_t			    client      /* Client data to be passed to the callback */
	) ;

EXTERN CyAsReturnStatus_t 
CyAsStorageRelease_dep(CyAsDeviceHandle handle, 
		     CyAsMediaType type
             );

EXTERN CyAsReturnStatus_t
CyAsStorageRelease_dep_EX(
	CyAsDeviceHandle		handle,		/* Handle to the device of interest */
	CyAsMediaType*			type,		/* The type of media to release */
	CyAsFunctionCallback		cb,             /* Callback to be called when the operation is complete */
	uint32_t			client          /* Client data to be passed to the callback */
	) ;

EXTERN CyAsReturnStatus_t
CyAsStorageQueryDevice_dep(
			CyAsDeviceHandle handle, 
			 CyAsMediaType media, 
			 uint32_t device, 
			 CyAsDeviceDesc *desc_p
             );

EXTERN CyAsReturnStatus_t
CyAsStorageQueryDevice_dep_EX(
	CyAsDeviceHandle		handle,		/* Handle to the device of interest */
	CyAsStorageQueryDeviceData_dep*	data,           /* Parameters and return value for the query call */
	CyAsFunctionCallback		cb,             /* Callback to be called when the operation is complete */
	uint32_t			client          /* Client data to be passed to the callback */
	) ;

EXTERN CyAsReturnStatus_t
CyAsStorageQueryUnit_dep(
	CyAsDeviceHandle	handle,		/* Handle to the device of interest */
	CyAsMediaType		type,		/* The type of media to query */
	uint32_t			device,		/* The logical device number to query */
	uint32_t			unit,		/* The unit to query on the device */
	CyAsUnitDesc *		unit_p		/* The return value for the unit descriptor */
	) ;

EXTERN CyAsReturnStatus_t
CyAsStorageQueryUnit_dep_EX(
	CyAsDeviceHandle		handle,		    /* Handle to the device of interest */
	CyAsStorageQueryUnitData_dep*	data_p, /* Parameters and return value for the query call */
	CyAsFunctionCallback		cb,         /* Callback to be called when the operation is complete */
	uint32_t			client              /* Client data to be passed to the callback */
    ) ;

EXTERN CyAsReturnStatus_t
CyAsStorageDeviceControl_dep(
        CyAsDeviceHandle       handle,         /* Handle to the West Bridge device */
        CyBool                 card_detect_en, /* Enable/disable control for card detection */
        CyBool                 write_prot_en,  /* Enable/disable control for write protect handling */
	    CyAsFunctionCallback   cb,             /* Callback to be called when the operation is complete */
	    uint32_t			   client          /* Client data to be passed to the callback */
        ) ;


EXTERN CyAsReturnStatus_t
CyAsStorageRead_dep(
	CyAsDeviceHandle		handle,		/* Handle to the device of interest */
	CyAsMediaType			type,		/* The type of media to access */
	uint32_t			device,		/* The device to access */
	uint32_t			unit,		/* The unit to access */
	uint32_t			block,		/* The first block to access */
	void *				data_p,		/* The buffer where data will be placed */
	uint16_t			num_blocks	/* The number of blocks to be read */
	) ;

EXTERN CyAsReturnStatus_t
CyAsStorageReadAsync_dep(
	CyAsDeviceHandle		handle,		/* Handle to the device of interest */
	CyAsMediaType			type,		/* The type of media to access */
	uint32_t			device,		    /* The device to access */
	uint32_t			unit,		    /* The unit to access */
	uint32_t			block,		    /* The first block to access */
	void *				data_p,		    /* The buffer where data will be placed */
	uint16_t			num_blocks,	    /* The number of blocks to be read */
	CyAsStorageCallback_dep		callback/* The function to call when the read is complete
										or an error occurs */
	) ;
EXTERN CyAsReturnStatus_t
CyAsStorageWrite_dep(
	CyAsDeviceHandle		handle,		/* Handle to the device of interest */
	CyAsMediaType			type,		/* The type of media to access */
	uint32_t			device,		/* The device to access */
	uint32_t			unit,		/* The unit to access */
	uint32_t			block,		/* The first block to access */
	void *				data_p,		/* The buffer containing the data to be written */
	uint16_t			num_blocks	/* The number of blocks to be written */
	) ;

EXTERN CyAsReturnStatus_t
CyAsStorageWriteAsync_dep(
	CyAsDeviceHandle		handle,		/* Handle to the device of interest */
	CyAsMediaType			type,		/* The type of media to access */
	uint32_t			device,		/* The device to access */
	uint32_t			unit,		/* The unit to access */
	uint32_t			block,		/* The first block to access */
	void *				data_p,		/* The buffer where the data to be written is stored */
	uint16_t			num_blocks,	/* The number of blocks to be written */
	CyAsStorageCallback_dep	        callback	/* The function to call when the write is complete
										   or an error occurs */
	) ;

EXTERN CyAsReturnStatus_t
CyAsStorageSDRegisterRead_dep(
        CyAsDeviceHandle        handle,
        CyAsMediaType           type,
        uint8_t                 device,
        CyAsSDCardRegType       regType,
        uint8_t                 readLen,
        uint8_t                 *data_p
        );

EXTERN CyAsReturnStatus_t
CyAsStorageSDRegisterRead_dep_EX(
        CyAsDeviceHandle                handle,     /* Handle to the West Bridge device. */
        CyAsMediaType			type,       /* The type of media to query */
        uint8_t		                device,     /* The device to query */
        CyAsSDCardRegType               regType,    /* The type of register to read. */
        CyAsStorageSDRegReadData       *data_p,     /* Output data buffer and length. */
        CyAsFunctionCallback            cb,         /* Callback function to call when done. */
        uint32_t                        client      /* Call context to send to the cb function. */
        ) ;

EXTERN CyAsReturnStatus_t
CyAsStorageCreatePPartition_dep(
        CyAsDeviceHandle     handle,
        CyAsMediaType        media,
        uint32_t             device,
        uint32_t             size,
        CyAsFunctionCallback cb,
        uint32_t             client) ;

EXTERN CyAsReturnStatus_t
CyAsStorageRemovePPartition_dep(
        CyAsDeviceHandle        handle,
        CyAsMediaType           media,
        uint32_t                device,
        CyAsFunctionCallback    cb,
        uint32_t                client) ;

#endif /*__doxygen*/

#endif /*__INCLUDED_CYANSTORAGE_DEP_H__*/
