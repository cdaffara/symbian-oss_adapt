/*  Cypress West Bridge API header file (cyanstorage.h)
 ## Header for backward compatibility with previous releases of Antioch SDK.
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

#ifndef _INCLUDED_CYANSTORAGE_H_
#define _INCLUDED_CYANSTORAGE_H_
#ifndef __doxygen__

#include "cyanmedia.h"
#include "cyanmisc.h"
#include "cyasstorage.h"
#include "cyas_cplus_start.h"

#define CY_AN_LUN_PHYSICAL_DEVICE				(CY_AS_LUN_PHYSICAL_DEVICE)
#define CY_AN_STORAGE_EP_SIZE                                   (CY_AS_STORAGE_EP_SIZE)

#define	CyAnStorageAntioch	CyAsStorageAntioch
#define	CyAnStorageProcessor	CyAsStorageProcessor
#define	CyAnStorageRemoved	CyAsStorageRemoved
#define	CyAnStorageInserted	CyAsStorageInserted
#define	CyAnSdioInterrupt	CyAsSdioInterrupt
typedef CyAsStorageEvent    CyAnStorageEvent;

#define	CyAnOpRead	 CyAsOpRead
#define	CyAnOpWrite	 CyAsOpWrite
typedef CyAsOperType CyAnOperType;

typedef CyAsDeviceDesc CyAnDeviceDesc;

typedef CyAsUnitDesc CyAnUnitDesc;

typedef CyAsStorageCallback_dep CyAnStorageCallback;

typedef CyAsStorageEventCallback_dep CyAnStorageEventCallback;

#define	CyAnSDReg_OCR CyAsSDReg_OCR
#define	CyAnSDReg_CID CyAsSDReg_CID
#define	CyAnSDReg_CSD CyAsSDReg_CSD
typedef CyAsSDCardRegType CyAnSDCardRegType;

typedef CyAsStorageQueryDeviceData_dep CyAnStorageQueryDeviceData ;

typedef CyAsStorageQueryUnitData_dep CyAnStorageQueryUnitData ;

typedef CyAsStorageSDRegReadData CyAnStorageSDRegReadData;

#define CY_AN_SD_REG_OCR_LENGTH         (CY_AS_SD_REG_OCR_LENGTH)
#define CY_AN_SD_REG_CID_LENGTH         (CY_AS_SD_REG_CID_LENGTH)
#define CY_AN_SD_REG_CSD_LENGTH         (CY_AS_SD_REG_CSD_LENGTH)
#define CY_AN_SD_REG_MAX_RESP_LENGTH    (CY_AS_SD_REG_MAX_RESP_LENGTH)

/**** API Functions ******/

/* Sync version of Storage Start */
EXTERN CyAnReturnStatus_t
CyAnStorageStart(
	CyAnDeviceHandle		handle
	) ;
#define CyAnStorageStart(handle) CyAsStorageStart((CyAsDeviceHandle)(handle), 0, 0)

/* Async version of Storage Start */
EXTERN CyAnReturnStatus_t
CyAnStorageStartEX(
	CyAnDeviceHandle	handle,
	CyAnFunctionCallback	cb,
	uint32_t		client
	) ;
#define CyAnStorageStartEX(h, cb, client)		\
    CyAsStorageStart((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Storage Stop */
EXTERN CyAnReturnStatus_t
CyAnStorageStop(
	CyAnDeviceHandle		handle
	) ;
#define CyAnStorageStop(handle) CyAsStorageStop((CyAsDeviceHandle)(handle), 0, 0)

/* Async version of Storage Stop */
EXTERN CyAnReturnStatus_t
CyAnStorageStopEX(
	CyAnDeviceHandle	handle,
	CyAnFunctionCallback	cb,
	uint32_t		client
	) ;
#define CyAnStorageStopEX(h, cb, client)		\
    CyAsStorageStop((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/* Register Call back api */
EXTERN CyAnReturnStatus_t
CyAnStorageRegisterCallback(
	CyAnDeviceHandle	        handle,
	CyAnStorageEventCallback	callback
	) ;
#define CyAnStorageRegisterCallback(h, cb)		\
    CyAsStorageRegisterCallback_dep((CyAsDeviceHandle)(h), (CyAsStorageEventCallback_dep)(cb))

/* Sync version of Storage Claim */
EXTERN CyAnReturnStatus_t
CyAnStorageClaim(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type
	) ;
#define CyAnStorageClaim(h, type)			\
    CyAsStorageClaim_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(type))

/* Async version of Storage Claim */
EXTERN CyAnReturnStatus_t
CyAnStorageClaimEX(
	CyAnDeviceHandle		handle,
	CyAnMediaType*			type,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnStorageClaimEX(h, type_p, cb, client)		\
    CyAsStorageClaim_dep_EX((CyAsDeviceHandle)(h), (CyAsMediaType *)(type_p), (CyAsFunctionCallback)(cb), (client))

/* Sync Version of Storage Release */
EXTERN CyAnReturnStatus_t
CyAnStorageRelease(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type
	) ;
#define CyAnStorageRelease(h, type)			\
    CyAsStorageRelease_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(type))

/* Async Version of Storage Release */
EXTERN CyAnReturnStatus_t
CyAnStorageReleaseEX(
	CyAnDeviceHandle		handle,
	CyAnMediaType*			type,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnStorageReleaseEX(h, type_p, cb, client)	\
    CyAsStorageRelease_dep_EX((CyAsDeviceHandle)(h), (CyAsMediaType *)(type_p), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Query Media */
EXTERN CyAnReturnStatus_t
CyAnStorageQueryMedia(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type,
	uint32_t *			count
	) ;
#define CyAnStorageQueryMedia(handle, type, count) \
    CyAsStorageQueryMedia((CyAsDeviceHandle)(handle), (CyAsMediaType)(type), (count), 0, 0)

/* Async version of Query Media */
EXTERN CyAnReturnStatus_t
CyAnStorageQueryMediaEX(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type,
	uint32_t *			count,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnStorageQueryMediaEX(h, type, count, cb, client)	\
    CyAsStorageQueryMedia((CyAsDeviceHandle)(h), (CyAsMediaType)(type), (count), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Query device */
EXTERN CyAnReturnStatus_t
CyAnStorageQueryDevice(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type,
	uint32_t			device,
	CyAnDeviceDesc *		desc_p
	) ;
#define CyAnStorageQueryDevice(h, type, device, desc_p)		\
    CyAsStorageQueryDevice_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(type), (device), (CyAsDeviceDesc *)(desc_p))

/* Async version of Query device */
EXTERN CyAnReturnStatus_t
CyAnStorageQueryDeviceEX(
	CyAnDeviceHandle		handle,
	CyAnStorageQueryDeviceData*	data,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnStorageQueryDeviceEX(h, data, cb, client)							\
    CyAsStorageQueryDevice_dep_EX((CyAsDeviceHandle)(h), (CyAsStorageQueryDeviceData_dep *)(data),	\
	    (CyAsFunctionCallback)(cb), (client))

/* Sync version of Query Unit */
EXTERN CyAnReturnStatus_t
CyAnStorageQueryUnit(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type,
	uint32_t			device,
	uint32_t			unit,
	CyAnUnitDesc *			desc_p
	) ;
#define CyAnStorageQueryUnit(h, type, device, unit, desc_p)	\
    CyAsStorageQueryUnit_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(type), (device), (unit), (CyAsUnitDesc *)(desc_p))

/* Async version of Query Unit */
EXTERN CyAnReturnStatus_t
CyAnStorageQueryUnitEX(
	CyAnDeviceHandle		handle,
	CyAnStorageQueryUnitData*	data_p,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnStorageQueryUnitEX(h, data_p, cb, client)							\
    CyAsStorageQueryUnit_dep_EX((CyAsDeviceHandle)(h), (CyAsStorageQueryUnitData_dep *)(data_p),	\
	    (CyAsFunctionCallback)(cb), (client))

/* Sync version of device control */
EXTERN CyAnReturnStatus_t
CyAnStorageDeviceControl(
        CyAnDeviceHandle                handle,
        CyBool                          card_detect_en,
        CyBool                          write_prot_en
        ) ;
#define CyAnStorageDeviceControl(handle, card_detect_en, write_prot_en) \
    CyAsStorageDeviceControl_dep((CyAsDeviceHandle)(handle), (card_detect_en), (write_prot_en), 0, 0)

/* Async version of device control */
EXTERN CyAnReturnStatus_t
CyAnStorageDeviceControlEX(
        CyAnDeviceHandle                handle,
        CyBool                          card_detect_en,
        CyBool                          write_prot_en,
	CyAnFunctionCallback		cb,
	uint32_t			client
        ) ;
#define CyAnStorageDeviceControlEX(h, det_en, prot_en, cb, client)	\
    CyAsStorageDeviceControl_dep((CyAsDeviceHandle)(h), (det_en), (prot_en), (CyAsFunctionCallback)(cb), (client))

/* Sync Read */
EXTERN CyAnReturnStatus_t
CyAnStorageRead(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type,
	uint32_t			device,
	uint32_t			unit,
	uint32_t			block,
	void *				data_p,
	uint16_t			num_blocks
	) ;
#define CyAnStorageRead(h, type, device, unit, block, data_p, nblks)	\
    CyAsStorageRead_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(type), (device), (unit), (block), (data_p), (nblks))

/* Async Read */
EXTERN CyAnReturnStatus_t
CyAnStorageReadAsync(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type,
	uint32_t			device,
	uint32_t			unit,
	uint32_t			block,
	void *				data_p,
	uint16_t			num_blocks,
	CyAnStorageCallback		callback
	) ;
#define CyAnStorageReadAsync(h, type, device, unit, block, data_p, nblks, cb)				\
    CyAsStorageReadAsync_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(type), (device), (unit), (block),	\
	    (data_p), (nblks), (CyAsStorageCallback_dep)(cb))

/* Sync Write */
EXTERN CyAnReturnStatus_t
CyAnStorageWrite(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type,
	uint32_t			device,
	uint32_t			unit,
	uint32_t			block,
	void *				data_p,
	uint16_t			num_blocks
	) ;
#define CyAnStorageWrite(h, type, device, unit, block, data_p, nblks)	\
    CyAsStorageWrite_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(type), (device), (unit), (block), (data_p), (nblks))

/* Async Write */
EXTERN CyAnReturnStatus_t
CyAnStorageWriteAsync(
	CyAnDeviceHandle		handle,
	CyAnMediaType			type,
	uint32_t			device,
	uint32_t			unit,
	uint32_t			block,
	void *				data_p,
	uint16_t			num_blocks,
	CyAnStorageCallback		callback
	) ;
#define CyAnStorageWriteAsync(h, type, device, unit, block, data_p, nblks, cb)				\
    CyAsStorageWriteAsync_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(type), (device), (unit), (block),	\
	    (data_p), (nblks), (CyAsStorageCallback_dep)(cb))

/* Cancel Async */
EXTERN CyAnReturnStatus_t
CyAnStorageCancelAsync(
	CyAnDeviceHandle		handle
	) ;
#define CyAnStorageCancelAsync(h) CyAsStorageCancelAsync((CyAsDeviceHandle)(h))

/* Sync SD Register Read*/
EXTERN CyAnReturnStatus_t
CyAnStorageSDRegisterRead(
        CyAnDeviceHandle          handle,
	CyAnMediaType		  type,
	uint8_t		          device,
	CyAnSDCardRegType         regType,
	uint8_t                   readLen,
	uint8_t                  *data_p
        ) ;
#define CyAnStorageSDRegisterRead(h, type, device, regType, len, data_p)			\
    CyAsStorageSDRegisterRead_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(type), (device), 	\
	    (CyAsSDCardRegType)(regType), (len), (data_p))

/*Async SD Register Read*/
EXTERN CyAnReturnStatus_t
CyAnStorageSDRegisterReadEX(
        CyAnDeviceHandle          handle,
	CyAnMediaType		  type,
	uint8_t		          device,
	CyAnSDCardRegType         regType,
	CyAnStorageSDRegReadData *data_p,
	CyAnFunctionCallback      cb,
	uint32_t                  client
        ) ;
#define CyAnStorageSDRegisterReadEX(h, type, device, regType, data_p, cb, client)		\
    CyAsStorageSDRegisterRead_dep_EX((CyAsDeviceHandle)(h), (CyAsMediaType)(type), (device),	\
	    (CyAsSDCardRegType)(regType), (CyAsStorageSDRegReadData *)(data_p),			\
	    (CyAsFunctionCallback)(cb), (client))

/* Create partition on storage device */
EXTERN CyAnReturnStatus_t
CyAnStorageCreatePPartition(
        CyAnDeviceHandle     handle,
        CyAnMediaType        media,
        uint32_t             device,
        uint32_t             size,
        CyAnFunctionCallback cb,
        uint32_t             client) ;
#define CyAnStorageCreatePPartition(h, media, dev, size, cb, client)                            \
    CyAsStorageCreatePPartition_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(media), (dev),       \
            (size), (CyAsFunctionCallback)(cb), (client))

/* Remove partition on storage device */
EXTERN CyAnReturnStatus_t
CyAnStorageRemovePPartition(
        CyAnDeviceHandle        handle,
        CyAnMediaType           media,
        uint32_t                device,
        CyAnFunctionCallback    cb,
        uint32_t                client) ;
#define CyAnStorageRemovePPartition(h, media, dev, cb, client)                                  \
    CyAsStorageRemovePPartition_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(media), (dev),       \
            (CyAsFunctionCallback)(cb), (client))

#include "cyas_cplus_end.h"
#endif /*__doxygen__ */

#endif
