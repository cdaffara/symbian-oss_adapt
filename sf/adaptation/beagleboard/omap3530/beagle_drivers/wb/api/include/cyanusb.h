/*  Cypress West Bridge API header file (cyanusb.h)
 ## Header for backward compatibility with previous Antioch SDK releases.
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

#ifndef _INCLUDED_CYANUSB_H_
#define _INCLUDED_CYANUSB_H_

#if !defined(__doxygen__)

#include "cyanmisc.h"
#include "cyasusb.h"
#include "cyas_cplus_start.h"

#define CY_AN_MAX_USB_DESCRIPTOR_SIZE				(CY_AS_MAX_USB_DESCRIPTOR_SIZE)

typedef CyAsUsbInquiryData_dep CyAnUsbInquiryData;
typedef CyAsUsbUnknownCommandData_dep CyAnUsbUnknownCommandData ;
typedef CyAsUsbStartStopData_dep CyAnUsbStartStopData ;
typedef CyAsMSCProgressData CyAnMSCProgressData ;

#define CyAnUsbNandEnum         CyAsUsbNandEnum
#define CyAnUsbSDEnum           CyAsUsbSDEnum
#define CyAnUsbMMCEnum          CyAsUsbMMCEnum
#define CyAnUsbCEATAEnum        CyAsUsbCEATAEnum
typedef CyAsUsbMassStorageEnum  CyAnUsbMassStorageEnum;

#define CyAnUsbDescDevice		CyAsUsbDescDevice
#define CyAnUsbDescDeviceQual		CyAsUsbDescDeviceQual
#define CyAnUsbDescFSConfiguration	CyAsUsbDescFSConfiguration
#define CyAnUsbDescHSConfiguration	CyAsUsbDescHSConfiguration
#define CyAnUsbDescString               CyAsUsbDescString
typedef CyAsUsbDescType CyAnUsbDescType ;

#define CyAnUsbIn	CyAsUsbIn
#define CyAnUsbOut	CyAsUsbOut
#define CyAnUsbInOut	CyAsUsbInOut
typedef CyAsUsbEndPointDir CyAnUsbEndPointDir ;


#define CyAnUsbControl  CyAsUsbControl
#define CyAnUsbIso      CyAsUsbIso
#define CyAnUsbBulk     CyAsUsbBulk
#define CyAnUsbInt      CyAsUsbInt
typedef CyAsUsbEndPointType CyAnUsbEndPointType ;


typedef CyAsUsbEnumControl_dep CyAnUsbEnumControl ;
typedef CyAsUsbEndPointConfig CyAnUsbEndPointConfig ;

#define CyAnUsbMSUnit0                  CyAsUsbMSUnit0
#define CyAnUsbMSUnit1                  CyAsUsbMSUnit1
#define CyAnUsbMSBoth                   CyAsUsbMSBoth
typedef CyAsUsbMSType_t CyAnUsbMSType_t ;

#define CyAnEventUsbSuspend		CyAsEventUsbSuspend
#define CyAnEventUsbResume		CyAsEventUsbResume
#define CyAnEventUsbReset		CyAsEventUsbReset
#define CyAnEventUsbSetConfig		CyAsEventUsbSetConfig
#define CyAnEventUsbSpeedChange		CyAsEventUsbSpeedChange
#define CyAnEventUsbSetupPacket		CyAsEventUsbSetupPacket
#define CyAnEventUsbStatusPacket	CyAsEventUsbStatusPacket
#define CyAnEventUsbInquiryBefore	CyAsEventUsbInquiryBefore
#define CyAnEventUsbInquiryAfter        CyAsEventUsbInquiryAfter
#define CyAnEventUsbStartStop           CyAsEventUsbStartStop
#define CyAnEventUsbUnknownStorage	CyAsEventUsbUnknownStorage
#define CyAnEventUsbMSCProgress         CyAsEventUsbMSCProgress
typedef CyAsUsbEvent CyAnUsbEvent;

typedef CyAsUsbEventCallback_dep CyAnUsbEventCallback ;

typedef CyAsUsbIoCallback CyAnUsbIoCallback;
typedef CyAsUsbFunctionCallback CyAnUsbFunctionCallback;

/******* USB Functions ********************/

/* Sync Usb Start */
extern CyAnReturnStatus_t
CyAnUsbStart(
	CyAnDeviceHandle		handle
	) ;
#define CyAnUsbStart(handle) CyAsUsbStart((CyAsDeviceHandle)(handle), 0, 0)

/*Async Usb Start */
extern CyAnReturnStatus_t
CyAnUsbStartEX(
	CyAnDeviceHandle		handle,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbStartEX(h, cb, client) CyAsUsbStart((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/* Sync Usb Stop */
extern CyAnReturnStatus_t
CyAnUsbStop(
	CyAnDeviceHandle		handle
	) ;
#define CyAnUsbStop(handle) CyAsUsbStop((CyAsDeviceHandle)(handle), 0, 0)

/*Async Usb Stop */
extern CyAnReturnStatus_t
CyAnUsbStopEX(
	CyAnDeviceHandle		handle,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbStopEX(h, cb, client) CyAsUsbStop((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/* Register USB event callback */
EXTERN CyAnReturnStatus_t
CyAnUsbRegisterCallback(
	CyAnDeviceHandle		handle,
	CyAnUsbEventCallback		callback
	) ;
#define CyAnUsbRegisterCallback(h, cb) 		\
    CyAsUsbRegisterCallback_dep((CyAsDeviceHandle)(h), (CyAsUsbEventCallback_dep)(cb))

/*Sync Usb connect */
EXTERN CyAnReturnStatus_t
CyAnUsbConnect(
	CyAnDeviceHandle		handle
	) ;
#define CyAnUsbConnect(handle) CyAsUsbConnect((CyAsDeviceHandle)(handle), 0, 0)

/*Async Usb connect */
extern CyAnReturnStatus_t
CyAnUsbConnectEX(
	CyAnDeviceHandle		handle,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbConnectEX(h, cb, client)		\
    CyAsUsbConnect((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/*Sync Usb disconnect */
EXTERN CyAnReturnStatus_t
CyAnUsbDisconnect(
	CyAnDeviceHandle		handle
	) ;
#define CyAnUsbDisconnect(handle) CyAsUsbDisconnect((CyAsDeviceHandle)(handle), 0, 0)

/*Async Usb disconnect */
extern CyAnReturnStatus_t
CyAnUsbDisconnectEX(
	CyAnDeviceHandle		handle,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbDisconnectEX(h, cb, client)	\
    CyAsUsbDisconnect((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/* Sync version of set enum config */
EXTERN CyAnReturnStatus_t
CyAnUsbSetEnumConfig(
	CyAnDeviceHandle	handle,
	CyAnUsbEnumControl *	config_p
	) ;
#define CyAnUsbSetEnumConfig(handle, config_p) \
    CyAsUsbSetEnumConfig_dep((CyAsDeviceHandle)(handle), (CyAsUsbEnumControl_dep *)(config_p), 0, 0)

/* Async version of set enum config */
extern CyAnReturnStatus_t
CyAnUsbSetEnumConfigEX(
	CyAnDeviceHandle		handle,
	CyAnUsbEnumControl *		config_p,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbSetEnumConfigEX(h, config_p, cb, client)						\
    CyAsUsbSetEnumConfig_dep((CyAsDeviceHandle)(h), (CyAsUsbEnumControl_dep *)(config_p), 	\
	    (CyAsFunctionCallback)(cb), (client))

/* Sync version of get enum config */
EXTERN CyAnReturnStatus_t
CyAnUsbGetEnumConfig(
	CyAnDeviceHandle		handle,
	CyAnUsbEnumControl *	config_p
	) ;
#define CyAnUsbGetEnumConfig(handle, config_p) \
    CyAsUsbGetEnumConfig_dep((CyAsDeviceHandle)(handle), (CyAsUsbEnumControl_dep *)(config_p), 0, 0)

/* Async version of get enum config */
extern CyAnReturnStatus_t
CyAnUsbGetEnumConfigEX(
	CyAnDeviceHandle		handle,
	CyAnUsbEnumControl *		config_p,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbGetEnumConfigEX(h, config_p, cb, client)						\
    CyAsUsbGetEnumConfig_dep((CyAsDeviceHandle)(h), (CyAsUsbEnumControl_dep *)(config_p), 	\
	    (CyAsFunctionCallback)(cb), (client))

/* Sync Version of Set descriptor */
EXTERN CyAnReturnStatus_t
CyAnUsbSetDescriptor(
	CyAnDeviceHandle		handle,
	CyAnUsbDescType			type,
	uint8_t				index,
	void *				desc_p,
	uint16_t			length
	) ;
#define CyAnUsbSetDescriptor(handle, type, index, desc_p, length) \
    CyAsUsbSetDescriptor((CyAsDeviceHandle)(handle), (CyAsUsbDescType)(type), (index), (desc_p), (length), 0, 0)

/* Async Version of Set descriptor */
extern CyAnReturnStatus_t
CyAnUsbSetDescriptorEX(
	CyAnDeviceHandle		handle,
	CyAnUsbDescType			type,
	uint8_t				index,
	void *				desc_p,
	uint16_t			length,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbSetDescriptorEX(h, type, index, desc_p, length, cb, client)				\
    CyAsUsbSetDescriptor((CyAsDeviceHandle)(h), (CyAsUsbDescType)(type), (index), (desc_p), (length),	\
	    (CyAsFunctionCallback)(cb), (client))

/* Only version of clear descriptors */
EXTERN CyAnReturnStatus_t
CyAnUsbClearDescriptors(
	CyAnDeviceHandle		handle,
        CyAnFunctionCallback            cb,
        uint32_t                        client
	) ;
#define CyAnUsbClearDescriptors(h, cb, client)                                                          \
    CyAsUsbClearDescriptors((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/* Sync version of get descriptor*/
EXTERN CyAnReturnStatus_t
CyAnUsbGetDescriptor(
	CyAnDeviceHandle	handle,
	CyAnUsbDescType		type,
	uint8_t			index,
	void *			desc_p,
	uint32_t *		length_p
	) ;
#define CyAnUsbGetDescriptor(h, type, index, desc_p, length_p)	\
    CyAsUsbGetDescriptor_dep((CyAsDeviceHandle)(h), (CyAsUsbDescType)(type), (index), (desc_p), (length_p))

typedef CyAsGetDescriptorData CyAnGetDescriptorData ;

/* Async version of get descriptor */
extern CyAnReturnStatus_t
CyAnUsbGetDescriptorEX(
	CyAnDeviceHandle		handle,
	CyAnUsbDescType			type,
	uint8_t				index,
	CyAnGetDescriptorData *		data,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbGetDescriptorEX(h, type, index, data, cb, client)						   \
    CyAsUsbGetDescriptor((CyAsDeviceHandle)(h), (CyAsUsbDescType)(type), (index), (CyAsGetDescriptorData *)(data), \
	    (CyAsFunctionCallback)(cb), (client))

EXTERN CyAnReturnStatus_t
CyAnUsbSetPhysicalConfiguration(
    CyAnDeviceHandle		handle,
    uint8_t			config
    ) ;
#define CyAnUsbSetPhysicalConfiguration(h, config)	\
    CyAsUsbSetPhysicalConfiguration((CyAsDeviceHandle)(h), (config))

EXTERN CyAnReturnStatus_t
CyAnUsbSetEndPointConfig(
    CyAnDeviceHandle			handle,
    CyAnEndPointNumber_t		ep,
    CyAnUsbEndPointConfig *		config_p
	) ;
#define CyAnUsbSetEndPointConfig(h, ep, config_p)	\
    CyAsUsbSetEndPointConfig((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsUsbEndPointConfig *)(config_p))

EXTERN CyAnReturnStatus_t
CyAnUsbGetEndPointConfig(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyAnUsbEndPointConfig *		config_p
	) ;
#define CyAnUsbGetEndPointConfig(h, ep, config_p)	\
    CyAsUsbGetEndPointConfig((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsUsbEndPointConfig *)(config_p))

/* Sync version of commit */
EXTERN CyAnReturnStatus_t
CyAnUsbCommitConfig(
	CyAnDeviceHandle		handle
	) ;
#define CyAnUsbCommitConfig(handle) CyAsUsbCommitConfig((CyAsDeviceHandle)(handle), 0, 0)

/* Async version of commit */
extern CyAnReturnStatus_t
CyAnUsbCommitConfigEX(
	CyAnDeviceHandle		handle,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbCommitConfigEX(h, cb, client)	\
    CyAsUsbCommitConfig((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

EXTERN CyAnReturnStatus_t
CyAnUsbReadData(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyBool				pktread,
	uint32_t			dsize,
	uint32_t *			dataread,
	void *				data
	) ;
#define CyAnUsbReadData(h, ep, pkt, dsize, dataread, data_p)	\
    CyAsUsbReadData((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (pkt), (dsize), (dataread), (data_p))

EXTERN CyAnReturnStatus_t
CyAnUsbReadDataAsync(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyBool				pktread,
	uint32_t			dsize,
	void *				data,
	CyAnUsbIoCallback		callback
	) ;
#define CyAnUsbReadDataAsync(h, ep, pkt, dsize, data_p, cb)						\
    CyAsUsbReadDataAsync((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (pkt), (dsize), (data_p),	\
	    (CyAsUsbIoCallback)(cb))

EXTERN CyAnReturnStatus_t
CyAnUsbWriteData(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	uint32_t			dsize,
	void *				data
	) ;
#define CyAnUsbWriteData(h, ep, dsize, data_p)		\
    CyAsUsbWriteData((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (dsize), (data_p))

EXTERN CyAnReturnStatus_t
CyAnUsbWriteDataAsync(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	uint32_t			dsize,
	void *				data,
	CyBool				spacket,
	CyAnUsbIoCallback		callback
	) ;
#define CyAnUsbWriteDataAsync(h, ep, dsize, data_p, spacket, cb)						\
    CyAsUsbWriteDataAsync((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (dsize), (data_p), (spacket),	\
	    (CyAsUsbIoCallback)(cb))

EXTERN CyAnReturnStatus_t
CyAnUsbCancelAsync(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep
	) ;
#define CyAnUsbCancelAsync(h, ep) CyAsUsbCancelAsync((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep))

/* Sync version of set stall */
EXTERN CyAnReturnStatus_t
CyAnUsbSetStall(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyAnUsbFunctionCallback		cb,
	uint32_t			client
) ;
#define CyAnUsbSetStall(h, ep, cb, client)	\
    CyAsUsbSetStall_dep((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsUsbFunctionCallback)(cb), (client))

/* Async version of set stall */
extern CyAnReturnStatus_t
CyAnUsbSetStallEX(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyAnFunctionCallback		cb,
	uint32_t			client
) ;
#define CyAnUsbSetStallEX(h, ep, cb, client)	\
    CyAsUsbSetStall((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsFunctionCallback)(cb), (client))

/*Sync version of clear stall */
EXTERN CyAnReturnStatus_t
CyAnUsbClearStall(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyAnUsbFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbClearStall(h, ep, cb, client)	\
    CyAsUsbClearStall_dep((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsUsbFunctionCallback)(cb), (client))

/*Sync version of clear stall */
extern CyAnReturnStatus_t
CyAnUsbClearStallEX(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbClearStallEX(h, ep, cb, client)	\
    CyAsUsbClearStall((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsFunctionCallback)(cb), (client))

/* Sync get stall */
EXTERN CyAnReturnStatus_t
CyAnUsbGetStall(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyBool *			stall_p
	) ;
#define CyAnUsbGetStall(handle, ep, stall_p) \
    CyAsUsbGetStall((CyAsDeviceHandle)(handle), (CyAsEndPointNumber_t)(ep), (stall_p), 0, 0)

/* Async get stall */
extern CyAnReturnStatus_t
CyAnUsbGetStallEX(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyBool *			stall_p,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbGetStallEX(h, ep, stall_p, cb, client)	\
    CyAsUsbGetStall((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (stall_p), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Set Nak */
EXTERN CyAnReturnStatus_t
CyAnUsbSetNak(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyAnUsbFunctionCallback		cb,
	uint32_t			client
) ;

#define CyAnUsbSetNak(h, ep, cb, client)		\
    CyAsUsbSetNak_dep((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsUsbFunctionCallback)(cb), (client))

/* Async version of Set Nak */
extern CyAnReturnStatus_t
CyAnUsbSetNakEX(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyAnFunctionCallback		cb,
	uint32_t			client
) ;
#define CyAnUsbSetNakEX(h, ep, cb, client)		\
    CyAsUsbSetNak((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsFunctionCallback)(cb), (client))

/* Sync version of clear nak */
EXTERN CyAnReturnStatus_t
CyAnUsbClearNak(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyAnUsbFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbClearNak(h, ep, cb, client)		\
    CyAsUsbClearNak_dep((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsUsbFunctionCallback)(cb), (client))

/* Sync version of clear nak */
extern CyAnReturnStatus_t
CyAnUsbClearNakEX(
	CyAnDeviceHandle		handle,
	CyAnEndPointNumber_t		ep,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnUsbClearNakEX(h, ep, cb, client)		\
    CyAsUsbClearNak((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (CyAsFunctionCallback)(cb), (client))

/* Sync Get NAK */
EXTERN CyAnReturnStatus_t
CyAnUsbGetNak(
    CyAnDeviceHandle		    handle,
    CyAnEndPointNumber_t	    ep,
    CyBool *			    nak_p
) ;
#define CyAnUsbGetNak(handle, ep, nak_p) \
    CyAsUsbGetNak((CyAsDeviceHandle)(handle), (CyAsEndPointNumber_t)(ep), (nak_p), 0, 0)

/* Async Get NAK */
EXTERN CyAnReturnStatus_t
CyAnUsbGetNakEX(
    CyAnDeviceHandle		    handle,
    CyAnEndPointNumber_t	    ep,
    CyBool *			    nak_p,
    CyAnFunctionCallback	    cb,
    uint32_t			    client
) ;
#define CyAnUsbGetNakEX(h, ep, nak_p, cb, client)	\
    CyAsUsbGetNak((CyAsDeviceHandle)(h), (CyAsEndPointNumber_t)(ep), (nak_p), (CyAsFunctionCallback)(cb), (client))

/* Sync remote wakup */
EXTERN CyAnReturnStatus_t
CyAnUsbSignalRemoteWakeup(
        CyAnDeviceHandle            handle
        ) ;
#define CyAnUsbSignalRemoteWakeup(handle) CyAsUsbSignalRemoteWakeup((CyAsDeviceHandle)(handle), 0, 0)

/* Async remote wakup */
EXTERN CyAnReturnStatus_t
CyAnUsbSignalRemoteWakeupEX(
        CyAnDeviceHandle            handle,
        CyAnFunctionCallback        cb,
        uint32_t                    client
        ) ;
#define CyAnUsbSignalRemoteWakeupEX(h, cb, client)	\
    CyAsUsbSignalRemoteWakeup((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/* Only version of SetMSReportThreshold */
EXTERN CyAnReturnStatus_t
CyAnUsbSetMSReportThreshold(
        CyAnDeviceHandle            handle,
        uint32_t                    wr_sectors,
        uint32_t                    rd_sectors,
        CyAnFunctionCallback        cb,
        uint32_t                    client
        ) ;
#define CyAnUsbSetMSReportThreshold(h, wr_cnt, rd_cnt, cb, client)              \
    CyAsUsbSetMSReportThreshold((CyAsDeviceHandle)(h), wr_cnt, rd_cnt, (CyAsFunctionCallback)(cb), (client))

/* Select storage partitions to be enumerated. */
EXTERN CyAnReturnStatus_t
CyAnUsbSelectMSPartitions (
        CyAnDeviceHandle                handle,
        CyAnMediaType                   media,
        uint32_t                        device,
        CyAnUsbMSType_t                 type,
        CyAnFunctionCallback            cb,
        uint32_t                        client
        ) ;
#define CyAnUsbSelectMSPartitions(h, media, dev, type, cb, client)                              \
    CyAsUsbSelectMSPartitions_dep((CyAsDeviceHandle)(h), (CyAsMediaType)(media), (dev),        \
                (CyAsUsbMSType_t)(type), (CyAsFunctionCallback)(cb), (client))

#include "cyas_cplus_end.h"
#endif /*__doxygen__*/
#endif	/*_INCLUDED_CYANUSB_H_*/
