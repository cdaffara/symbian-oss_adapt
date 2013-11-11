/*  Cypress West Bridge API header file (cyanmisc.h)
 ## Version for backward compatibility with previous Antioch SDK releases.
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

#ifndef _INCLUDED_CYANMISC_H_
#define _INCLUDED_CYANMISC_H_

#include "cyantypes.h"
#include <cyasmisc.h>
#include "cyanmedia.h"
#include "cyas_cplus_start.h"

#define CY_AN_LEAVE_STANDBY_DELAY_CLOCK		(CY_AS_LEAVE_STANDBY_DELAY_CLOCK)
#define CY_AN_RESET_DELAY_CLOCK			(CY_AS_RESET_DELAY_CLOCK)

#define CY_AN_LEAVE_STANDBY_DELAY_CRYSTAL	(CY_AS_LEAVE_STANDBY_DELAY_CRYSTAL)
#define CY_AN_RESET_DELAY_CRYSTAL		(CY_AS_RESET_DELAY_CRYSTAL)

/* Defines to convert the old CyAn names to the new
 * CyAs names
 */
typedef CyAsDeviceHandle	    CyAnDeviceHandle;

#define CyAnDeviceDackAck	    CyAsDeviceDackAck
#define CyAnDeviceDackEob	    CyAsDeviceDackEob
typedef CyAsDeviceDackMode CyAnDeviceDackMode;

typedef CyAsDeviceConfig CyAnDeviceConfig;

#define CyAnResourceUSB		    CyAsBusUSB
#define CyAnResourceSDIO_MMC	    CyAsBus_1
#define CyAnResourceNand	    CyAsBus_0
typedef CyAsResourceType CyAnResourceType;

#define CyAnResetSoft		    CyAsResetSoft
#define CyAnResetHard		    CyAsResetHard
typedef CyAsResetType CyAnResetType;
typedef CyAsFunctCBType CyAnFunctCBType;
typedef CyAsFunctionCallback CyAnFunctionCallback;

#define CyAnEventMiscInitialized    CyAsEventMiscInitialized
#define CyAnEventMiscAwake	    CyAsEventMiscAwake
#define CyAnEventMiscHeartBeat      CyAsEventMiscHeartBeat
#define CyAnEventMiscWakeup         CyAsEventMiscWakeup
#define CyAnEventMiscDeviceMismatch CyAsEventMiscDeviceMismatch
typedef CyAsMiscEventType CyAnMiscEventType;
typedef CyAsMiscEventCallback CyAnMiscEventCallback;

#define CyAnMiscGpio_0		    CyAsMiscGpio_0
#define CyAnMiscGpio_1		    CyAsMiscGpio_1
#define CyAnMiscGpio_Nand_CE	    CyAsMiscGpio_Nand_CE
#define CyAnMiscGpio_Nand_CE2	    CyAsMiscGpio_Nand_CE2
#define CyAnMiscGpio_Nand_WP	    CyAsMiscGpio_Nand_WP
#define CyAnMiscGpio_Nand_CLE	    CyAsMiscGpio_Nand_CLE
#define CyAnMiscGpio_Nand_ALE	    CyAsMiscGpio_Nand_ALE
#define CyAnMiscGpio_UValid	    CyAsMiscGpio_UValid
#define CyAnMiscGpio_SD_POW         CyAsMiscGpio_SD_POW
typedef CyAsMiscGpio CyAnMiscGpio;

#define CY_AN_SD_DEFAULT_FREQ       CY_AS_SD_DEFAULT_FREQ
#define CY_AN_SD_RATED_FREQ         CY_AS_SD_RATED_FREQ
typedef CyAsLowSpeedSDFreq CyAnLowSpeedSDFreq;

#define CY_AN_HS_SD_FREQ_48         CY_AS_HS_SD_FREQ_48
#define CY_AN_HS_SD_FREQ_24         CY_AS_HS_SD_FREQ_24
typedef CyAsHighSpeedSDFreq CyAnHighSpeedSDFreq;

#define CyAnMiscActiveHigh          CyAsMiscActiveHigh
#define CyAnMiscActiveLow           CyAsMiscActiveLow
typedef CyAsMiscSignalPolarity CyAnMiscSignalPolarity;

typedef CyAsGetFirmwareVersionData CyAnGetFirmwareVersionData;

enum {
    CYAN_FW_TRACE_LOG_NONE = 0,
    CYAN_FW_TRACE_LOG_STATE,
    CYAN_FW_TRACE_LOG_CALLS,
    CYAN_FW_TRACE_LOG_STACK_TRACE,
    CYAN_FW_TRACE_MAX_LEVEL
};


/***********************************/
/***********************************/
/*    FUNCTIONS                    */
/***********************************/
/***********************************/


EXTERN CyAnReturnStatus_t
CyAnMiscCreateDevice(
        CyAnDeviceHandle *		handle_p,
        CyAnHalDeviceTag		tag
        ) ;
#define CyAnMiscCreateDevice(h, tag) CyAsMiscCreateDevice((CyAsDeviceHandle *)(h), (CyAsHalDeviceTag)(tag))

EXTERN CyAnReturnStatus_t
CyAnMiscDestroyDevice(
	CyAnDeviceHandle  handle
	) ;
#define CyAnMiscDestroyDevice(h) CyAsMiscDestroyDevice((CyAsDeviceHandle)(h))

EXTERN CyAnReturnStatus_t
CyAnMiscConfigureDevice(
        CyAnDeviceHandle		handle,
        CyAnDeviceConfig		*config_p
        ) ;
#define CyAnMiscConfigureDevice(h, cfg) CyAsMiscConfigureDevice((CyAsDeviceHandle)(h), (CyAsDeviceConfig *)(cfg))

EXTERN CyAnReturnStatus_t
CyAnMiscInStandby(
        CyAnDeviceHandle		handle,
        CyBool			        *standby
        ) ;
#define CyAnMiscInStandby(h, standby) CyAsMiscInStandby((CyAsDeviceHandle)(h), (standby))

/* Sync version of Download Firmware */
EXTERN CyAnReturnStatus_t
CyAnMiscDownloadFirmware(
        CyAnDeviceHandle		handle,
        const void 			*fw_p,
        uint16_t			size
        ) ;

#define CyAnMiscDownloadFirmware(handle, fw_p, size)            \
    CyAsMiscDownloadFirmware((CyAsDeviceHandle)(handle), (fw_p), (size), 0, 0)

/* Async version of Download Firmware */
EXTERN CyAnReturnStatus_t
CyAnMiscDownloadFirmwareEX(
        CyAnDeviceHandle		handle,
        const void 			*fw_p,
        uint16_t			size,
        CyAnFunctionCallback		cb,
        uint32_t			client
        ) ;

#define CyAnMiscDownloadFirmwareEX(h, fw_p, size, cb, client)   \
    CyAsMiscDownloadFirmware((CyAsDeviceHandle)(h), (fw_p), (size), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Get Firmware Version */
EXTERN CyAnReturnStatus_t
CyAsMiscGetFirmwareVersion_dep(CyAsDeviceHandle handle,
			     uint16_t *major,
			     uint16_t *minor,
			     uint16_t *build,
			     uint8_t *mediaType,
			     CyBool *isDebugMode);

#define CyAnMiscGetFirmwareVersion(h, major, minor, bld, type, mode)    \
    CyAsMiscGetFirmwareVersion_dep((CyAsDeviceHandle)(h), (major), (minor), (bld), (type), (mode))

/* Async version of Get Firmware Version*/
EXTERN CyAnReturnStatus_t
CyAnMiscGetFirmwareVersionEX(
        CyAnDeviceHandle		handle,
        CyAnGetFirmwareVersionData*	data,
        CyAnFunctionCallback	        cb,
        uint32_t			client
        ) ;
#define CyAnMiscGetFirmwareVersionEX(h, data, cb, client)       \
    CyAsMiscGetFirmwareVersion((CyAsDeviceHandle)(h), (data), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Read MCU Register*/
EXTERN CyAnReturnStatus_t
CyAnMiscReadMCURegister(
	CyAnDeviceHandle	handle,
	uint16_t			address,
	uint8_t				*value
	) ;

#define CyAnMiscReadMCURegister(handle, address, value) \
    CyAsMiscReadMCURegister((CyAsDeviceHandle)(handle), (address), (value), 0, 0)

/* Async version of Read MCU Register*/
EXTERN CyAnReturnStatus_t
CyAnMiscReadMCURegisterEX(
	CyAnDeviceHandle		handle,
	uint16_t			address,
	uint8_t				*value,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;

#define CyAnMiscReadMCURegisterEX(h, addr, val, cb, client)     \
    CyAsMiscReadMCURegister((CyAsDeviceHandle)(h), (addr), (val), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Write MCU Register*/
EXTERN CyAnReturnStatus_t
CyAnMiscWriteMCURegister(
        CyAnDeviceHandle    handle,
        uint16_t            address,
        uint8_t			    mask,
        uint8_t             value
        ) ;
#define CyAnMiscWriteMCURegister(handle, address, mask, value) \
    CyAsMiscWriteMCURegister((CyAsDeviceHandle)(handle), (address), (mask), (value), 0, 0)

/* Async version of Write MCU Register*/
EXTERN CyAnReturnStatus_t
CyAnMiscWriteMCURegisterEX(
        CyAnDeviceHandle     handle,
        uint16_t             address,
        uint8_t		     mask,
        uint8_t              value,
        CyAnFunctionCallback cb,
        uint32_t	     client
        ) ;
#define CyAnMiscWriteMCURegisterEX(h, addr, mask, val, cb, client)      \
    CyAsMiscWriteMCURegister((CyAsDeviceHandle)(h), (addr), (mask), (val), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Write MCU Register*/
EXTERN CyAnReturnStatus_t
CyAnMiscReset(
	CyAnDeviceHandle		handle,
	CyAnResetType			type,
	CyBool				flush
	) ;
#define CyAnMiscReset(handle, type, flush) \
    CyAsMiscReset((CyAsDeviceHandle)(handle), (type), (flush), 0, 0)

/* Async version of Write MCU Register*/
EXTERN CyAnReturnStatus_t
CyAnMiscResetEX(
	CyAnDeviceHandle	handle,
	CyAnResetType		type,
	CyBool		    	flush,
	CyAnFunctionCallback	cb,
	uint32_t		client
	) ;
#define CyAnMiscResetEX(h, type, flush, cb, client)	\
    CyAsMiscReset((CyAsDeviceHandle)(h), (CyAsResetType)(type), (flush), (CyAsFunctionCallback)(cb), (client))

/*  Synchronous version of CyAnMiscAcquireResource. */
EXTERN CyAnReturnStatus_t
CyAnMiscAcquireResource(
	CyAnDeviceHandle		handle,
	CyAnResourceType		type,
	CyBool				force
	) ;
#define CyAnMiscAcquireResource(h, type, force)		\
    CyAsMiscAcquireResource_dep((CyAsDeviceHandle)(h), (CyAsResourceType)(type), (force))

/* Asynchronous version of CyAnMiscAcquireResource. */
EXTERN CyAnReturnStatus_t
CyAnMiscAcquireResourceEX(
	CyAnDeviceHandle		handle,
	CyAnResourceType*		type,
	CyBool				force,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnMiscAcquireResourceEX(h, type_p, force, cb, client)	\
    CyAsMiscAcquireResource((CyAsDeviceHandle)(h), (CyAsResourceType *)(type_p), \
	    (force), (CyAsFunctionCallback)(cb), (client))

/* The one and only version of Release resource */
EXTERN CyAnReturnStatus_t
CyAnMiscReleaseResource(
	CyAnDeviceHandle		handle,
	CyAnResourceType		type
	) ;
#define CyAnMiscReleaseResource(h, type)		\
    CyAsMiscReleaseResource((CyAsDeviceHandle)(h), (CyAsResourceType)(type))

/* Synchronous version of CyAnMiscSetTraceLevel. */
EXTERN CyAnReturnStatus_t
CyAnMiscSetTraceLevel(
	CyAnDeviceHandle	handle,
	uint8_t			level,
	CyAnMediaType		media,
	uint32_t 		device,
	uint32_t		unit
	) ;

#define CyAnMiscSetTraceLevel(handle, level, media, device, unit) \
    CyAsMiscSetTraceLevel_dep((CyAsDeviceHandle)(handle), (level), (CyAsMediaType)(media), (device), (unit), 0, 0)

/* Asynchronous version of CyAnMiscSetTraceLevel. */
EXTERN CyAnReturnStatus_t
CyAnMiscSetTraceLevelEX(
	CyAnDeviceHandle		handle,
	uint8_t				level,
	CyAnMediaType			media,
	uint32_t			device,
	uint32_t			unit,
	CyAnFunctionCallback		cb,
	uint32_t			client
	) ;
#define CyAnMiscSetTraceLevelEX(h, level, media, device, unit, cb, client)				\
    CyAsMiscSetTraceLevel_dep((CyAsDeviceHandle)(h), (level), (CyAsMediaType)(media), (device), (unit),	\
	    (CyAsFunctionCallback)(cb), (client))

/* Synchronous version of CyAnMiscEnterStandby. */
EXTERN CyAnReturnStatus_t
CyAnMiscEnterStandby(
	CyAnDeviceHandle	handle,
	CyBool			pin
	) ;
#define CyAnMiscEnterStandby(handle, pin) \
    CyAsMiscEnterStandby((CyAsDeviceHandle)(handle), (pin), 0, 0)

/* Synchronous version of CyAnMiscEnterStandby. */
EXTERN CyAnReturnStatus_t
CyAnMiscEnterStandbyEX(
	CyAnDeviceHandle	handle,
	CyBool			pin,
	CyAnFunctionCallback	cb,
	uint32_t		client
	) ;
#define CyAnMiscEnterStandbyEX(h, pin, cb, client)		\
    CyAsMiscEnterStandby((CyAsDeviceHandle)(h), (pin), (CyAsFunctionCallback)(cb), (client))

/* Only one version of CyAnMiscLeaveStandby. */
EXTERN CyAnReturnStatus_t
CyAnMiscLeaveStandby(
	CyAnDeviceHandle		handle,
	CyAnResourceType		type
	) ;
#define CyAnMiscLeaveStandby(h, type) 				\
    CyAsMiscLeaveStandby((CyAsDeviceHandle)(h), (CyAsResourceType)(type))

/* The one version of Misc Register Callback */
EXTERN CyAnReturnStatus_t
CyAnMiscRegisterCallback(
	CyAnDeviceHandle	handle,
	CyAnMiscEventCallback	callback
	) ;
#define CyAnMiscRegisterCallback(h, cb)			\
    CyAsMiscRegisterCallback((CyAsDeviceHandle)(h), (CyAsMiscEventCallback)(cb))

/* The only version of SetLogLevel */
EXTERN void
CyAnMiscSetLogLevel(
	uint8_t	level
	) ;
#define CyAnMiscSetLogLevel(level) CyAsMiscSetLogLevel(level)

/* Sync version of Misc Storage Changed */
EXTERN CyAnReturnStatus_t
CyAnMiscStorageChanged(
	CyAnDeviceHandle		handle
	) ;
#define CyAnMiscStorageChanged(handle) \
    CyAsMiscStorageChanged((CyAsDeviceHandle)(handle), 0, 0)

/* Async version of Misc Storage Changed */
EXTERN CyAnReturnStatus_t
CyAnMiscStorageChangedEX(
	CyAnDeviceHandle	handle,
	CyAnFunctionCallback	cb,
	uint32_t		client
	) ;
#define CyAnMiscStorageChangedEX(h, cb, client)			\
    CyAsMiscStorageChanged((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Heartbeat control */
EXTERN CyAnReturnStatus_t
CyAnMiscHeartBeatControl(
        CyAnDeviceHandle                handle,
        CyBool                          enable
        ) ;
#define CyAnMiscHeartBeatControl(handle, enable) \
    CyAsMiscHeartBeatControl((CyAsDeviceHandle)(handle), (enable), 0, 0)

/* Async version of Heartbeat control */
EXTERN CyAnReturnStatus_t
CyAnMiscHeartBeatControlEX(
        CyAnDeviceHandle       	handle,
        CyBool                 	enable,
        CyAnFunctionCallback   	cb,
        uint32_t		client
        ) ;
#define CyAnMiscHeartBeatControlEX(h, enable, cb, client)	\
    CyAsMiscHeartBeatControl((CyAsDeviceHandle)(h), (enable), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Get Gpio */
EXTERN CyAnReturnStatus_t
CyAnMiscGetGpioValue(
        CyAnDeviceHandle                handle,
        CyAnMiscGpio                    pin,
        uint8_t                        *value
        ) ;
#define CyAnMiscGetGpioValue(handle, pin, value) \
    CyAsMiscGetGpioValue((CyAsDeviceHandle)(handle), (CyAsMiscGpio)(pin), (value), 0, 0)

/* Async version of Get Gpio */
EXTERN CyAnReturnStatus_t
CyAnMiscGetGpioValueEX(
        CyAnDeviceHandle                handle,
        CyAnMiscGpio                    pin,
        uint8_t                        *value,
        CyAnFunctionCallback            cb,
        uint32_t                        client
        ) ;
#define CyAnMiscGetGpioValueEX(h, pin, value, cb, client)	\
    CyAsMiscGetGpioValue((CyAsDeviceHandle)(h), (CyAsMiscGpio)(pin), (value), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Set Gpio */
EXTERN CyAnReturnStatus_t
CyAnMiscSetGpioValue(
        CyAnDeviceHandle handle,
        CyAnMiscGpio     pin,
        uint8_t          value
        ) ;
#define CyAnMiscSetGpioValue(handle, pin, value) \
    CyAsMiscSetGpioValue((CyAsDeviceHandle)(handle), (CyAsMiscGpio)(pin), (value), 0, 0)

/* Async version of Set Gpio */
EXTERN CyAnReturnStatus_t
CyAnMiscSetGpioValueEX(
        CyAnDeviceHandle                handle,
        CyAnMiscGpio                    pin,
        uint8_t                         value,
        CyAnFunctionCallback            cb,
        uint32_t                        client
        ) ;
#define CyAnMiscSetGpioValueEX(h, pin, value, cb, client)	\
    CyAsMiscSetGpioValue((CyAsDeviceHandle)(h), (CyAsMiscGpio)(pin), (value), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Enter suspend */
EXTERN CyAnReturnStatus_t
CyAnMiscEnterSuspend(
        CyAnDeviceHandle                handle,
        CyBool                          usb_wakeup_en,
        CyBool                          gpio_wakeup_en
        ) ;
#define CyAnMiscEnterSuspend(handle, usb_wakeup_en, gpio_wakeup_en) \
    CyAsMiscEnterSuspend((CyAsDeviceHandle)(handle), (usb_wakeup_en), (gpio_wakeup_en), 0, 0)

/* Async version of Enter suspend */
EXTERN CyAnReturnStatus_t
CyAnMiscEnterSuspendEX(
        CyAnDeviceHandle        handle,
        CyBool                  usb_wakeup_en,
        CyBool                  gpio_wakeup_en,
        CyAnFunctionCallback    cb,
        uint32_t                client
        ) ;
#define CyAnMiscEnterSuspendEX(h, usb_en, gpio_en, cb, client)	\
    CyAsMiscEnterSuspend((CyAsDeviceHandle)(h), (usb_en), (gpio_en), (CyAsFunctionCallback)(cb), (client))

/* Sync version of Enter suspend */
EXTERN CyAnReturnStatus_t
CyAnMiscLeaveSuspend(
        CyAnDeviceHandle                handle
        ) ;
#define CyAnMiscLeaveSuspend(handle) \
    CyAsMiscLeaveSuspend((CyAsDeviceHandle)(handle), 0, 0)

/* Async version of Enter suspend */
EXTERN CyAnReturnStatus_t
CyAnMiscLeaveSuspendEX(
        CyAnDeviceHandle                handle,
        CyAnFunctionCallback            cb,
        uint32_t                        client
        ) ;

#define CyAnMiscLeaveSuspendEX(h, cb, client)		\
    CyAsMiscLeaveSuspend((CyAsDeviceHandle)(h), (CyAsFunctionCallback)(cb), (client))

/* Sync version of SetLowSpeedSDFreq */
EXTERN CyAnReturnStatus_t
CyAnMiscSetLowSpeedSDFreq(
        CyAnDeviceHandle                handle,
        CyAnLowSpeedSDFreq              setting
        ) ;
#define CyAnMiscSetLowSpeedSDFreq(h, setting)           \
    CyAsMiscSetLowSpeedSDFreq((CyAsDeviceHandle)(h), (CyAsLowSpeedSDFreq)(setting), 0, 0)

/* Async version of SetLowSpeedSDFreq */
EXTERN CyAnReturnStatus_t
CyAnMiscSetLowSpeedSDFreqEX(
        CyAnDeviceHandle                handle,
        CyAnLowSpeedSDFreq              setting,
        CyAnFunctionCallback            cb,
        uint32_t                        client
        ) ;
#define CyAnMiscSetLowSpeedSDFreqEX(h, setting, cb, client)                             \
    CyAsMiscSetLowSpeedSDFreq((CyAsDeviceHandle)(h), (CyAsLowSpeedSDFreq)(setting),     \
            (CyAsFunctionCallback)(cb), (client))

/* SetHighSpeedSDFreq */
EXTERN CyAnReturnStatus_t
CyAnMiscSetHighSpeedSDFreq(
        CyAnDeviceHandle                handle,
        CyAnHighSpeedSDFreq             setting,
        CyAnFunctionCallback            cb,
        uint32_t                        client
        ) ;
#define CyAnMiscSetHighSpeedSDFreq(h, setting, cb, client)                              \
    CyAsMiscSetHighSpeedSDFreq((CyAsDeviceHandle)(h), (CyAsHighSpeedSDFreq)(setting),   \
            (CyAsFunctionCallback)(cb), (client))

/* ReserveLNABootArea */
EXTERN CyAnReturnStatus_t 
CyAnMiscReserveLNABootArea(
        CyAnDeviceHandle handle,
        uint8_t numzones,
        CyAnFunctionCallback cb, 
        uint32_t client);
#define CyAnMiscReserveLNABootArea(h, num, cb, client)                                  \
    CyAsMiscReserveLNABootArea((CyAsDeviceHandle)(h), num, (CyAsFunctionCallback)(cb),  \
            (client))

/* SetSDPowerPolarity */
EXTERN CyAnReturnStatus_t
CyAnMiscSetSDPowerPolarity(
        CyAnDeviceHandle       handle,
        CyAnMiscSignalPolarity polarity,
        CyAnFunctionCallback   cb,
        uint32_t               client);
#define CyAnMiscSetSDPowerPolarity(h, pol, cb, client)                                  \
    CyAsMiscSetSDPowerPolarity((CyAsDeviceHandle)(h), (CyAsMiscSignalPolarity)(pol),    \
            (CyAsFunctionCallback)(cb), (client))

#include "cyas_cplus_end.h"

#endif

