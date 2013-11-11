#include <kern_priv.h>
#include <beagle/beagle_gpio.h>
#include <beagle/variant.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_irqmap.h> // GPIO interrupts
#include <assp/omap3530_assp/omap3530_gpio.h>

#include <assp.h> // Required for definition of TIsr

#include <cyastoria.h>
#include <cyasmtp.h>
#include <cyasusbinit.h>
#include <cyasusbdescs.h>

TmtpAstDev g_AstDevice;
TmtpAstDev * g_pAstDevice = &g_AstDevice ;

static uint16_t replybuf[512] ;

static uint8_t *
GetReplyArea(void)
{
    return (uint8_t *) replybuf ;
}


/* Globals */
/*static uint8_t pktbuffer3[512] ;*/

#ifdef CY_AS_USB_TB_FOUR
static uint8_t pktbuffer7[512] ;
#endif

#ifdef CY_AS_USB_TB_SIX
static uint8_t pktbuffer11[512] ;
#endif

/*static uint8_t turbopktbuffer[512] ;*/

static volatile CyBool gSetConfig = CyFalse ;
static volatile CyBool gAsyncStallDone = CyFalse ;

static volatile CyBool gStorageReleaseBus0 = CyFalse ;
static volatile CyBool gStorageReleaseBus1 = CyFalse ;

static volatile CyAsHalDeviceTag g_tag ;

static uint8_t MyConfiguration = 0 ;
static CyCh9ConfigurationDesc *desc_p = 0 ;
static CyCh9ConfigurationDesc *other_p = 0 ;

static volatile uint8_t gAsyncStallStale = 0;

/* Forward declarations */
static int SetupUSBPPort(CyAsDeviceHandle h, uint8_t media, CyBool isTurbo) ;
static void MyUsbEventCallbackMS(CyAsDeviceHandle h, CyAsUsbEvent ev, void *evdata) ;
static void PrintData(const char *name, uint8_t *data, uint16_t size) ;

static void
StallCallback(CyAsDeviceHandle h, CyAsReturnStatus_t status, uint32_t tag, CyAsFunctCBType cbtype, void *cbdata)
{
    (void)h ;
    (void)cbtype ;
    (void)cbdata ;

    if (tag == 1)
        CyAsHalPrintMessage("*** Nak callback - status = %d\n", status) ;
    else
        CyAsHalPrintMessage("*** Stall callback - status = %d\n", status) ;
}

static void
StallCallbackEX(CyAsDeviceHandle h,
                CyAsReturnStatus_t status,
                uint32_t tag,
                CyAsFunctCBType type,
                void*   data)
{
    (void)h ;
    (void)type ;
    (void)data ;
    (void)status ;

    if(tag == 18)
    {
        CyAsReturnStatus_t ret = CyAsUsbClearStall(h, 3, StallCallbackEX, 21);
        CyAsHalAssert(ret == CY_AS_ERROR_SUCCESS) ;
    }
    else
        gAsyncStallDone = CyTrue ;
}

static void
StallCallbackAsync(CyAsDeviceHandle h, CyAsReturnStatus_t status, uint32_t tag, CyAsFunctCBType cbtype, void *cbdata)
{
#ifdef PRINT_DEBUG_INFO
	CyAsReturnStatus_t ret ;
#endif
    (void)cbtype ;
    (void)cbdata ;
    (void)tag ;
    (void)status ;

    if(gAsyncStallStale == 0)
    {
        gAsyncStallStale++;
#ifdef PRINT_DEBUG_INFO
        ret =
#endif
			CyAsUsbClearStall(h, 3, StallCallbackAsync, 21);
        CyAsHalAssert(ret == CY_AS_ERROR_SUCCESS) ;
    }
    else
    {
        gAsyncStallDone = CyTrue ;
    }
}

static void
MyStorageEventCBMS(CyAsDeviceHandle h, CyAsBusNumber_t bus, uint32_t device, CyAsStorageEvent evtype, void *evdata)
{
    (void)h ;
    (void)evdata ;

    switch (evtype)
    {
    case CyAsStorageAntioch:
        CyAsHalPrintMessage("CyAsStorageAntioch Event: bus=%d, device=%d\n", bus, device) ;
        switch (bus)
        {
        case 0:
            gStorageReleaseBus0 = CyTrue ;
            break;
        case 1:
            gStorageReleaseBus1 = CyTrue ;
            break;
        default:
            break;
        }
        break;

    case CyAsStorageProcessor:
        CyAsHalPrintMessage("CyAsStorageProcessor Event: bus=%d, device %d\n", bus, device) ;
        break;

    case CyAsStorageRemoved:
        CyAsHalPrintMessage("Bus %d, device %d has been removed\n", bus, device) ;
        break;

    case CyAsStorageInserted:
        CyAsHalPrintMessage("Bus %d, device %d has been inserted\n", bus, device) ;
        break;

    default:
        break;
    }
}


/*
* This function exercises the USB module
*/
int CyAsAPIUsbInit(const char *pgm, CyAsDeviceHandle h, CyAsHalDeviceTag tag)
{
    CyAsReturnStatus_t ret ;
    /*char buffer[16] ;*/

	g_tag = tag ;
	memset(g_pAstDevice,0, sizeof(g_AstDevice));
	g_pAstDevice->astHalTag = tag ;
	g_pAstDevice->astDevHandle = h ;

    /*
    * Give a delay to allow any equipment to be ready (e.g. CATC)
    */
    /*CyAsHalPrintMessage("Press enter to begin USB operation (%s): ", "P Port Enumeration") ;
    fgets(buffer, sizeof(buffer), stdin) ;*/

	CyAsHalPrintMessage("*** CyAsStorageStart...\n") ;
    ret = CyAsStorageStart(h, 0, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("%s: CyAsStorageStart returned error code %d\n", pgm, ret) ;
        return 0 ;
    }
    CyAsHalPrintMessage("*** CyAsStorageStart...Done\n") ;
    /*
    * Register a storage event call-back so that the USB attached storage can be
    * release when the USB connection has been made.
    */
    CyAsHalPrintMessage("*** CyAsStorageRegisterCallback...\n") ;
    ret = CyAsStorageRegisterCallback(h, MyStorageEventCBMS) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("%s: CyAsStorageRegisterCallbackMS returned error code %d\n", pgm, ret) ;
        return 0 ;
    }
    CyAsHalPrintMessage("*** CyAsStorageRegisterCallback...Done\n") ;

	ret = CyAsStorageRelease(h, 1, 0, 0, 0) ;
	if (ret != CY_AS_ERROR_SUCCESS)
	{
		CyAsHalPrintMessage("CyAsStorageReleaseMS returned error code %d\n", ret) ;
		return 0 ;
	}
	gStorageReleaseBus1 = CyFalse ;

    /*
     * Use the 24 MHz operating frequency, if the SD card is a low speed one.
     */
    ret = CyAsMiscSetLowSpeedSDFreq(h, CY_AS_SD_RATED_FREQ, 0, 0) ;
    if ((ret != CY_AS_ERROR_SUCCESS) && (ret != CY_AS_ERROR_INVALID_RESPONSE))
    {
        CyAsHalPrintMessage("%s: CyAsMiscSetLowSpeedSDFreq returned error code %d\n", pgm, ret) ;
        return 0 ;
    }

    /*
    * We are using P Port based enumeration
    */
#ifdef DEBUG_ZERO
    if (!SetupUSBPPort(h, 2, 0))
#else
    if (!SetupUSBPPort(h, 2, 1))
#endif
        return 0 ;
    /*
    * Now we let the enumeration process happen via callbacks.  When the set configuration
    * request is processed, we are done with enumeration and ready to perform our function.
    */
    while (!gSetConfig)
        CyAsHalSleep(100) ;

    CyAsHalPrintMessage("*** Configuration complete, starting echo function\n") ;

    return 1 ;
}

static void
MyCyAsMTPEventCallback(
    CyAsDeviceHandle handle,
    CyAsMTPEvent evtype,
    void* evdata)
{
	(void) handle;
	switch(evtype)
	{
	case CyAsMTPSendObjectComplete:
		{
			CyAsMTPSendObjectCompleteData* sendObjData = (CyAsMTPSendObjectCompleteData*) evdata ;
			CyAsHalPrintMessage("MTP EVENT: SendObjectComplete\n");
			CyAsHalPrintMessage("Bytes sent = %d\nSend status = %d",sendObjData->byte_count,sendObjData->status);
			g_pAstDevice->tmtpSendCompleteData.byte_count = sendObjData->byte_count;
			g_pAstDevice->tmtpSendCompleteData.status = sendObjData->status;
			g_pAstDevice->tmtpSendCompleteData.transaction_id = sendObjData->transaction_id ;
			g_pAstDevice->tmtpSendComplete = CyTrue ;
			break;
		}
	case CyAsMTPGetObjectComplete:
		{
			CyAsMTPGetObjectCompleteData*  getObjData = (CyAsMTPGetObjectCompleteData*) evdata ;
			CyAsHalPrintMessage("MTP EVENT: GetObjectComplete\n");
			CyAsHalPrintMessage("Bytes got = %d\nGet status = %d",getObjData->byte_count,getObjData->status);
			g_pAstDevice->tmtpGetCompleteData.byte_count = getObjData->byte_count;
			g_pAstDevice->tmtpGetCompleteData.status = getObjData->status ;
			g_pAstDevice->tmtpGetComplete = CyTrue ;
			break;
		}
	case CyAsMTPBlockTableNeeded:
		g_pAstDevice->tmtpNeedNewBlkTbl = CyTrue ;
		break;
	default:
		;
	}

}
/*
* This function is responsible for initializing the USB function within West Bridge.  This
* function initializes West Bridge for P port based enumeration.
*/
int SetupUSBPPort(CyAsDeviceHandle h, uint8_t bus, CyBool isTurbo)
{
    CyAsReturnStatus_t ret ;
    CyAsUsbEnumControl config ;
#ifdef DEBUG_ZERO
    CyAsUsbEndPointConfig epconfig ;
    (void)epconfig;
#endif
    uint32_t count = 0 ;

    CyAsHalPrintMessage("*** SetupUSBPPort...\n") ;
    /*
    * Intialize the primary descriptor to be the full speed descriptor and the
    * other descriptor to by the high speed descriptor.  This will swap if we see a
    * high speed event.
    */
#ifdef DEBUG_ZERO
    desc_p = (CyCh9ConfigurationDesc *)&ConfigFSDesc ;
    other_p = (CyCh9ConfigurationDesc *)&ConfigHSDesc ;
#else
    desc_p = (CyCh9ConfigurationDesc *)&ZeroDesc ;
    other_p = (CyCh9ConfigurationDesc *)&ZeroDesc ;
#endif
    /* Step 1: Release the USB D+ and D- pins
    *
    * This code releases control of the D+ and D- pins if they have been previously
    * acquired by the P Port processor.  The physical D+ and D- pins are controlled either by
    * West Bridge, or by some other hardware external to West Bridge.  If external hardware is using
    * these pins, West Bridge must put these pins in a high impedence state in order to insure there
    * is no coflict over the use of the pins.  Before we can initialize the USB capabilities of
    * West Bridge, we must be sure West Bridge has ownership of the D+ and D- signals.  West Bridge will take
    * ownership of these pins as long as the P port processor has released them.  This call
    * releases control of these pins.  Before calling the CyAsMiscReleaseResource(), the P port API
    * must configure the hardware to release control of the D+ and D- pins by any external hardware.
    *
    * Note that this call can be made anywhere in the intialization sequence as long as it is done
    * before the call to CyAsUsbConnect().  If not, when the CyAsUsbConnect() call is made, West Bridge
    * will detect that it does not own the D+ and D- pins and the call to CyAsUsbConnect will fail.
    */
    ret = CyAsMiscReleaseResource(h, CyAsBusUSB) ;
    if (ret != CY_AS_ERROR_SUCCESS && ret != CY_AS_ERROR_RESOURCE_NOT_OWNED)
    {
        CyAsHalPrintMessage("Cannot Release USB reousrce: CyAsMiscReleaseResourceMS failed with error code %d\n", ret) ;
        return 0 ;
    }

    /*
    * Step 2: Start the USB stack
    *
    * This code initializes the USB stack.  It takes a handle to an West Bridge device
    * previously created with a call to CyAsMiscCreateDevice().
    */
    ret = CyAsUsbStart(h, 0, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("CyAsUsbStart failed with error code %d\n", ret) ;
        return 0 ;
    }

    /*
    * Step 3: Register a callback
    *
    * This code registers a callback to handle USB events.  This callback function will handle
    * all setup packets during enumeration as well as other USB events (SUSPEND, RESUME, CONNECT,
    * DISCONNECT, etc.)
    */
    ret = CyAsUsbRegisterCallback(h, MyUsbEventCallbackMS) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("CyAsUsbRegisterCallbackMS failed with error code %d\n", ret) ;
        return 0 ;
    }

#ifdef DEBUG_ZERO
    if ( isTurbo )
#endif
    {
		/*
		 * The SD/MMC resource needs to be released before the device
		 * can be successfully initialized by the firmware.
		 */
		ret = CyAsMiscReleaseResource(h, CyAsBus_1) ;
		if (ret != CY_AS_ERROR_SUCCESS && ret != CY_AS_ERROR_RESOURCE_NOT_OWNED)
		{
			CyAsHalPrintMessage("CyAsMtpApp: CyAsMiscReleaseResource failed with error code %d\n", ret) ;
			return -ret;
		}

		ret = CyAsStorageStart(h, 0, 0) ;
		if (ret != CY_AS_ERROR_SUCCESS)
		{
			CyAsHalPrintMessage("CyAsMtpApp: CyAsStorageStart failed with error code %d\n", ret) ;
			return -ret;
		}

		ret = CyAsStorageQueryMedia(h, CyAsMediaSDFlash, &count, 0, 0) ;
		if (ret != CY_AS_ERROR_SUCCESS)
		{
		  CyAsHalPrintMessage("CyAsMtpApp: Cannot query SD device count - Reason code %d\n", ret) ;
		  return -ret ;
		}
		CyAsHalPrintMessage("CyAsMtpApp: %d %s device(s) found\n", count, media_name) ;

		ret = CyAsStorageClaim(h, CyAsBus_1, 0, 0, 0) ;
		if (ret != CY_AS_ERROR_SUCCESS)
		{
		  CyAsHalPrintMessage("CyAsMtpApp: Cannot claim SD media - Reason code %d\n", ret) ;
		  return -ret;
		}

		/* We know that there is only one */
		g_pAstDevice->dev_data.bus = CyAsBus_1 ;
		g_pAstDevice->dev_data.device = 0 ;
		ret = CyAsStorageQueryDevice(h, &(g_pAstDevice->dev_data), 0, 0) ;
		if (ret != CY_AS_ERROR_SUCCESS)
		{
		  CyAsHalPrintMessage("CyAsMtpApp: Cannot query SD device - Reason code %d\n", ret) ;
		  return -ret ;
		}
		CyAsHalPrintMessage("CyAsMtpApp: blocksize %d, %d units found\n",
			g_pAstDevice->dev_data.desc_p.block_size, g_pAstDevice->dev_data.desc_p.number_units) ;

		g_pAstDevice->unit_data.bus = CyAsBus_1 ;
		g_pAstDevice->unit_data.device = 0 ;
		g_pAstDevice->unit_data.unit = 0 ;
		/* We know that there is only one */
		ret = CyAsStorageQueryUnit(h, &(g_pAstDevice->unit_data), 0, 0) ;
		if (ret != CY_AS_ERROR_SUCCESS)
		{
		  CyAsHalPrintMessage("CyAsMtpApp: Cannot query SD device unit - Reason code %d\n", ret) ;
		  return -ret ;
		}
		CyAsHalPrintMessage("CyAsMtpApp: blocksize %d, %d Block(s) found\n",
			g_pAstDevice->unit_data.desc_p.block_size, g_pAstDevice->unit_data.desc_p.unit_size) ;

			CyAsHalPrintMessage("CyAsMtpApp: Starting TMTP...\n");
		ret = CyAsMTPStart(h, MyCyAsMTPEventCallback, 0, 0) ;
		if (ret != CY_AS_ERROR_SUCCESS)
		{
			CyAsHalPrintMessage("CyAsMTPStart failed with error code %d\n", ret) ;
			return 0 ;
		}
    }

    /*
    * Step 4: Setup the enumeration mode
    *
    * This code tells the West Bridge API how enumeration will be done.  Specifically in this
    * example we are configuring the API for P Port processor based enumeraton.  This will cause
    * all setup packets to be relayed to the P port processor via the USB event callback.  See
    * the function CyAsUsbRegisterEventCallback() for more information about this callback.
    */
    config.antioch_enumeration = CyFalse ;                      /* P port will do enumeration, not West Bridge */

    /* Set the media to enumerate through USB */
#ifdef DEBUG_ZERO
    config.devices_to_enumerate[0][0] = CyFalse;
    config.devices_to_enumerate[1][0] = CyFalse;
#else
    config.devices_to_enumerate[0][0] = (bus & CY_TEST_BUS_0) ? CyTrue : CyFalse;
    config.devices_to_enumerate[1][0] = (bus & CY_TEST_BUS_1) ? CyTrue : CyFalse;
#endif

    if (isTurbo)
    {
        /* Force SD bus */
        config.devices_to_enumerate[0][0] = CyFalse;
        config.devices_to_enumerate[1][0] =  CyTrue ;
        /* No MSC in Turbo */
        config.mass_storage_interface = 0 ;
        config.mtp_interface = 1 ;
    }
    else
    {
        /* not Turbo here */
        config.mtp_interface = 0 ;

#ifndef DEBUG_MSC
        config.mass_storage_interface = 0 ;
        config.mass_storage_callbacks = CyFalse ;
#else
        /* Force SD bus */
        config.devices_to_enumerate[1][0] =  CyTrue ;
        config.mass_storage_interface = 1 ;
        config.mass_storage_callbacks = CyTrue ;
#endif
    }
    ret = CyAsUsbSetEnumConfig(h, &config, 0, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("CyAsUsbSetEnumConfigMS failed with error code %d\n", ret) ;
        return 0 ;
    }

#ifndef DEBUG_MSC
    /*
     * Step 5: set physical configuration
     */
#ifdef DEBUG_ZERO
    /*nxz-debug-z ret = CyAsUsbSetPhysicalConfiguration(h, 5) ;*/
    ret = CyAsUsbSetPhysicalConfiguration(h, 1) ;
#else
    ret = CyAsUsbSetPhysicalConfiguration(h, 5) ;
#endif
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("CyAsUsbSetPhysicalConfiguration failed with error code %d\n", ret) ;
        return 0 ;
    }

    /*
    * Step 5: Commit the endpoint configuration
    */
#ifdef DEBUG_ZERO
    epconfig.enabled = CyTrue ;
    epconfig.dir = CyAsUsbOut ;
    epconfig.type = CyAsUsbBulk ;
    epconfig.size = 0 ;
    epconfig.physical = 1 ;
    ret = CyAsUsbSetEndPointConfig(h, 3, &epconfig) ;
    if ( ret != CY_AS_ERROR_SUCCESS )
    {
	CyAsHalPrintMessage("CyAsUsbSetEndPointConfig failed with error code %d\n", ret) ;
        return 0 ;
    }

    epconfig.enabled = CyTrue ;
    epconfig.dir = CyAsUsbIn ;
    epconfig.type = CyAsUsbBulk ;
    epconfig.size = 0 ;
    epconfig.physical = 3 ;
    ret = CyAsUsbSetEndPointConfig(h, 5, &epconfig) ;
    if ( ret != CY_AS_ERROR_SUCCESS )
    {
	CyAsHalPrintMessage("CyAsUsbSetEndPointConfig failed with error code %d\n", ret) ;
        return 0 ;
    }

    /*nxz-debug-z */
    epconfig.enabled = CyTrue ;
    epconfig.dir = CyAsUsbIn ;
    epconfig.type = CyAsUsbInt ;
    epconfig.size = 64 ;
    epconfig.physical = 2 ;
    ret = CyAsUsbSetEndPointConfig(h, 7, &epconfig) ;
    if ( ret != CY_AS_ERROR_SUCCESS )
    {
	CyAsHalPrintMessage("CyAsUsbSetEndPointConfig failed with error code %d\n", ret) ;
        return 0 ;
    }
#endif

    /*
    * This code commits the endpoint configuration to the West Bridge hardware.
    */
    ret = CyAsUsbCommitConfig(h, 0, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("CyAsUsbCommitConfig failed with error code %d\n", ret) ;
        return 0 ;
    }
#endif
    /*
    * Step 6: Connect to the USB host.
    *
    * This code actually connects the D+ and D- signals internal to West Bridge to the D+ and D- pins
    * on the device.  If the host is already physically connected, this will begin the enumeration
    * process.  Otherwise, the enumeration process will being when the host is connected.
    */
    ret = CyAsUsbConnect(h, 0, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("CyAsUsbConnect failed with error code %d\n", ret) ;
        return 0 ;
    }

    CyAsHalPrintMessage("*** SetupUSBPPort...Done\n") ;
    return 1 ;
}

/*
* Print a block of data, useful for displaying data during debug.
*/
static void PrintData(const char *name_p, uint8_t *data, uint16_t size)
{
    uint32_t i = 0 ;
    uint32_t linecnt = 0 ;

    while (i < size)
    {
        if (linecnt == 0)
            CyAsHalPrintMessage("%s @ %02x:", name_p, i) ;

        CyAsHalPrintMessage(" %02x", data[i]) ;

        linecnt++ ;
        i++ ;

        if (linecnt == 16)
        {
            CyAsHalPrintMessage("\n") ;
            linecnt = 0 ;
        }
    }

    if (linecnt != 0)
        CyAsHalPrintMessage("\n") ;
}

/*
* This is the write callback for writes that happen as part of the setup operation.
*/
static void SetupWriteCallback(CyAsDeviceHandle h, CyAsEndPointNumber_t ep, uint32_t count, void *buf_p, CyAsReturnStatus_t status)
{
    (void)count ;
    (void)h ;
    (void)buf_p ;

    if (status != CY_AS_ERROR_SUCCESS)
        CyAsHalPrintMessage("Error returned in SetupWriteCallback - %d\n", status) ;
}

static CyAsReturnStatus_t
SetupWrite(CyAsDeviceHandle h, uint32_t requested, uint32_t dsize, void *data)
{
    CyBool spacket = CyTrue ;

    if (requested == dsize)
        spacket = CyFalse ;

    return CyAsUsbWriteDataAsync(h, 0, dsize, data, spacket, SetupWriteCallback) ;
}

int mystrlen(char* str)
{
	int len = 0 ;

	while( str && (*str != '\0') )
	{
		len++;
		str++;
	}

	return len ;
}
/*
* Send the USB host a string descriptor.  If the index is zero, send the
* array of supported languages, otherwise send the string itself per the USB
* Ch9 specification.
*/
static void SendStringDescriptor(CyAsDeviceHandle h, uint8_t *data)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS;
    int i = data[2] ;
    int langid = data[4] | (data[5] << 8) ;
    uint16_t reqlen = data[6] | (data[7] << 8) ;

    CyAsHalPrintMessage("**** CY_CH9_GD_STRING - %d\n", i) ;
    if (i == 0)
    {
        uint8_t *reply ;

        reply = GetReplyArea() ;
        reply[0] = 4 ;
        reply[1] = CY_CH9_GD_STRING ;
        reply[2] = CY_CH9_LANGID_ENGLISH_UNITED_STATES & 0xff ;
        reply[3] = (CY_CH9_LANGID_ENGLISH_UNITED_STATES >> 8) & 0xff ;
        ret = SetupWrite(h, reqlen, 4, reply) ;
    }
    else if (i <= sizeof(UsbStrings)/sizeof(UsbStrings[0]) && langid == CY_CH9_LANGID_ENGLISH_UNITED_STATES)
    {
        uint8_t *reply ;
        uint16_t len = (uint16_t)mystrlen(UsbStrings[i - 1]) ;

        CyAsHalPrintMessage("*** Sending string '%s'\n", UsbStrings[i - 1]) ;

        reply = GetReplyArea() ;
        reply[0] = (uint8_t)(len * 2 + 2) ;
        reply[1] = CY_CH9_GD_STRING ;
        /* nxz-linux-port */
	/*memcpy(reply + 2, UsbStrings[i - 1], len ) ;
        ret = SetupWrite(h, reqlen, len  + 2, reply) ;*/
        {
            uint16_t index ;
            uint16_t *rpy = (uint16_t *)(reply + 2) ;
            for (index = 0; index < len; index++)
            {
                *rpy = (uint16_t)(UsbStrings[i - 1][index]) ;
                rpy++ ;
            }
        }
        ret = SetupWrite(h, reqlen, len * 2 + 2, reply) ;
    }
    else
    {
        /*
        * If the host asks for an invalid string, we must stall EP 0
        */
        ret = CyAsUsbSetStall(h, 0, StallCallback, 0) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            CyAsHalPrintMessage("**** cannot set stall state on EP 0\n") ;

        CyAsHalPrintMessage("Host asked for invalid string or langid, index = 0x%04x, langid = 0x%04x\n", i, langid) ;
    }

    if (ret != CY_AS_ERROR_SUCCESS)
        CyAsHalPrintMessage("****** ERROR WRITING USB DATA - %d\n", ret) ;
    else
        CyAsHalPrintMessage("** Write Sucessful\n") ;
}

static CyAsReturnStatus_t
SendSetupData(CyAsDeviceHandle h, uint32_t reqlen, uint32_t size, void *data_p)
{
    CyAsReturnStatus_t ret ;
    uint8_t *reply ;

    /*
    * Never send more data than was requested
    */
    if (size > reqlen)
        size = reqlen ;

    reply = GetReplyArea() ;

    memcpy(reply, data_p, size) ;
    ret = SetupWrite(h, reqlen, size, reply) ;

    return ret ;
}

/*
* This function processes the GET DESCRIPTOR usb request.
*/
static void ProcessGetDescriptorRequest(CyAsDeviceHandle h, uint8_t *data)
{
    CyAsReturnStatus_t ret ;
    uint16_t reqlen = data[6] | (data[7] << 8) ;

    if (data[3] == CY_CH9_GD_DEVICE)
    {
        /*
        * Return the device descriptor
        */
        CyAsHalPrintMessage("**** CY_CH9_GD_DEVICE (size = %d)\n", sizeof(pport_device_desc)) ;
        PrintData("DD", (uint8_t *)&pport_device_desc, sizeof(pport_device_desc)) ;
        ret = SendSetupData(h, reqlen, sizeof(pport_device_desc), &pport_device_desc) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            CyAsHalPrintMessage("****** ERROR WRITING USB DATA - %d\n", ret) ;
        else
            CyAsHalPrintMessage("** Write Sucessful\n") ;
    }
    else if (data[3] == CY_CH9_GD_DEVICE_QUALIFIER)
    {
        /*
        * Return the device descriptor
        */
        CyAsHalPrintMessage("**** CY_CH9_GD_DEVICE (size = %d)\n", sizeof(device_qualifier)) ;
        PrintData("DD", (uint8_t *)&device_qualifier, sizeof(device_qualifier)) ;
        ret = SendSetupData(h, reqlen, sizeof(device_qualifier), &device_qualifier) ;
        if (ret != CY_AS_ERROR_SUCCESS)
            CyAsHalPrintMessage("****** ERROR WRITING USB DATA - %d\n", ret) ;
        else
            CyAsHalPrintMessage("** Write Sucessful\n") ;
    }
    else if (data[3] == CY_CH9_GD_CONFIGURATION)
    {
#ifdef PRINT_DEBUG_INFO
		const char *desc_name_p = "UNKNOWN" ;
#endif
        uint16_t size = 0 ;

        /*
        * Return the CONFIGURATION descriptor.
        */
        if (desc_p == (CyCh9ConfigurationDesc *)&ConfigHSDesc)
        {
#ifdef PRINT_DEBUG_INFO
            desc_name_p = "HighSpeed" ;
#endif
            size = sizeof(ConfigHSDesc) ;
        }
        else if (desc_p == (CyCh9ConfigurationDesc *)&ConfigFSDesc)
        {
#ifdef PRINT_DEBUG_INFO
            desc_name_p = "FullSpeed" ;
#endif
            size = sizeof(ConfigFSDesc) ;
        }
        else if (desc_p == &ZeroDesc)
        {
#ifdef PRINT_DEBUG_INFO
            desc_name_p = "ZeroDesc" ;
#endif
            size = sizeof(ZeroDesc) ;
        }

        CyAsHalPrintMessage("**** CY_CH9_GD_CONFIGURATION - %s (size = %d)\n", desc_name_p, size) ;
        if (size > 0)
        {
            PrintData("CFG", (uint8_t *)desc_p, size) ;
            desc_p->bDescriptorType = CY_CH9_GD_CONFIGURATION;
            ret = SendSetupData(h, reqlen, size, desc_p) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                CyAsHalPrintMessage("****** ERROR WRITING USB DATA - %d\n", ret) ;
            else
                CyAsHalPrintMessage("** Write Sucessful\n") ;
        }
    }
    else if (data[3] == CY_CH9_GD_OTHER_SPEED_CONFIGURATION)
    {
#ifdef PRINT_DEBUG_INFO
		const char *desc_name_p = "UNKNOWN" ;
#endif
        uint16_t size = 0 ;

        /*
        * Return the CONFIGURATION descriptor.
        */
        if (other_p == (CyCh9ConfigurationDesc *)&ConfigHSDesc)
        {
#ifdef PRINT_DEBUG_INFO
            desc_name_p = "HighSpeed" ;
#endif
            size = sizeof(ConfigHSDesc) ;
        }
        else if (other_p == (CyCh9ConfigurationDesc *)&ConfigFSDesc)
        {
#ifdef PRINT_DEBUG_INFO
            desc_name_p = "FullSpeed" ;
#endif
            size = sizeof(ConfigFSDesc) ;
        }
        else if (other_p == &ZeroDesc)
        {
#ifdef PRINT_DEBUG_INFO
            desc_name_p = "ZeroDesc" ;
#endif
            size = sizeof(ZeroDesc) ;
        }

        CyAsHalPrintMessage("**** CY_CH9_GD_OTHER_SPEED_CONFIGURATION - %s (size = %d)\n", desc_name_p, size) ;
        if (size > 0)
        {
            PrintData("CFG", (uint8_t *)other_p, size) ;
            other_p->bDescriptorType = CY_CH9_GD_OTHER_SPEED_CONFIGURATION;
            ret = SendSetupData(h, reqlen, size, other_p) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                CyAsHalPrintMessage("****** ERROR WRITING USB DATA - %d\n", ret) ;
            else
                CyAsHalPrintMessage("** Write Sucessful\n") ;
        }
    }
    else if (data[3] == CY_CH9_GD_STRING)
    {
        SendStringDescriptor(h, data) ;
    }
    else if (data[3] == CY_CH9_GD_REPORT)
    {
        CyAsHalPrintMessage("**** CY_CH9_GD_REPORT\n") ;
    }
    else if (data[3] == CY_CH9_GD_HID)
    {
        CyAsHalPrintMessage("**** CY_CH9_GD_HID\n") ;
    }
    else
    {
        CyAsHalPrintMessage("**** Unknown Descriptor request\n") ;
    }
}

/*
static void EP0DataCallback(CyAsDeviceHandle h, CyAsEndPointNumber_t ep, uint32_t count, void *buf_p, CyAsReturnStatus_t status)
{
    (void)ep ;
    (void)h ;

    if (status == CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("Read data phase of setup packet from EP0\n") ;
        PrintData("SetupData", buf_p, (uint16_t)count) ;
    }
    else
    {
        CyAsHalPrintMessage("Error reading data from EP0\n") ;
    }
}
*/

static void
ProcessSetupPacketRequest(CyAsDeviceHandle h, uint8_t *data)
{
    CyAsReturnStatus_t ret ;
    uint16_t reqlen = data[6] | (data[7] << 8) ;
    (void)ret;

    if ((data[0] & CY_CH9_SETUP_TYPE_MASK) == CY_CH9_SETUP_STANDARD_REQUEST)
    {
        switch(data[1])
        {
        case CY_CH9_SC_GET_DESCRIPTOR:
            CyAsHalPrintMessage("USB EP0 : CY_CH9_SC_GET_DESCRIPTOR request\n") ;
            ProcessGetDescriptorRequest(h, data) ;
            break ;

        case CY_CH9_SC_GET_INTERFACE:
            {
                uint8_t *response = GetReplyArea() ;

                *response = 0 ;
                CyAsHalPrintMessage("************* USB EP0: CY_CH9_SC_GET_INTERFACE request - RETURNING ZERO\n") ;
                ret = SetupWrite(h, reqlen, 1, response) ;
                if (ret != CY_AS_ERROR_SUCCESS)
                {
                    CyAsHalPrintMessage("****** ERROR WRITING USB DATA - %d\n", ret) ;
                }
            }
            break ;

        case CY_CH9_SC_SET_INTERFACE:
            CyAsHalPrintMessage("USB EP0 : CY_CH9_SC_SET_INTERFACE request\n") ;
            break ;

        case CY_CH9_SC_SET_CONFIGURATION:
			{
				/*CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;*/

				CyAsHalPrintMessage("USB EP0 : CY_CH9_SC_SET_CONFIGURATION request (%02x)\n", data[2]) ;
				{
					gAsyncStallDone = CyFalse ;
					gAsyncStallStale = 0;
					CyAsUsbSetStall(h, 3, StallCallbackEX, 18);
				}
				gSetConfig = CyTrue ;
				g_pAstDevice->configDone = 1 ;
				MyConfiguration = data[2];

				/* Set configuration is the last step before host send MTP data to EP2 */
#if 0
				ret = CyAsUsbReadDataAsync(g_pAstDevice->astDevHandle,
						CY_AS_MTP_BULK_OUT_EP, CyFalse, 512,
						g_pAstDevice->astEPBuf, TmtpReadCallback) ;
				if(ret != CY_AS_ERROR_SUCCESS && ret != CY_AS_ERROR_ASYNC_PENDING)
				{
					/*handle error in reading*/
					CyAsHalPrintMessage("CyAsMtpApp: CyAsUsbReadDataAsync Failed. Reason code: %d\n",ret) ;
					return ;
				}
#endif
			}
            break ;

        case CY_CH9_SC_GET_CONFIGURATION:
            {
                uint8_t *response = GetReplyArea() ;

                *response = MyConfiguration ;
                CyAsHalPrintMessage("USB EP0 : CY_CH9_SC_GET_INTERFACE request\n") ;
                ret = SetupWrite(h, reqlen, 1, response) ;
                if (ret != CY_AS_ERROR_SUCCESS)
                {
                    CyAsHalPrintMessage("****** ERROR WRITING USB DATA - %d\n", ret) ;
                }
            }
            CyAsHalPrintMessage("USB EP0 : CY_CH9_SC_GET_CONFIGURATION request\n") ;
            break ;

        case CY_CH9_SC_GET_STATUS:
            {
                uint16_t *response = (uint16_t *)GetReplyArea() ;

                *response = 0 ;
                CyAsHalPrintMessage("USB EP0 : CY_CH9_SC_GET_STATUS request\n") ;
                ret = SetupWrite(h, reqlen, 2, response) ;
                if (ret != CY_AS_ERROR_SUCCESS)
                {
                    CyAsHalPrintMessage("****** ERROR WRITING USB DATA - %d\n", ret) ;
                }
            }
            break ;

        case CY_CH9_SC_CLEAR_FEATURE:
            {
                uint16_t feature = data[2] | (data[3] << 8) ;
                CyAsHalPrintMessage("USB EP0 : CY_CH9_SC_CLEAR_FEATURE request\n") ;

                if ((data[0] & CY_CH9_SETUP_DEST_MASK) == CY_CH9_SETUP_DEST_ENDPOINT && feature == 0)
                {
                    CyAsEndPointNumber_t ep = data[4] | (data[5] << 8) ;
                    /* This is a clear feature/endpoint halt on an endpoint */
                    CyAsHalPrintMessage("Calling ClearStall on EP %d\n", ep) ;
                    ret = CyAsUsbClearStall(h, ep, StallCallback, 0) ;
                    if (ret != CY_AS_ERROR_SUCCESS)
                    {
                        CyAsHalPrintMessage("******* ERROR SEND CLEAR STALL REQUEST - %d\n", ret) ;
                    }
                }
            }
            break ;

        case CY_CH9_SC_SET_FEATURE:
            {
                uint16_t feature = data[2] | (data[3] << 8) ;
                CyAsHalPrintMessage("USB EP0 : CY_CH9_SC_SET_FEATURE request\n") ;

                if ((data[0] & CY_CH9_SETUP_DEST_MASK) == CY_CH9_SETUP_DEST_ENDPOINT && feature == 0)
                {
                    CyAsEndPointNumber_t ep = data[4] | (data[5] << 8) ;
                    /* This is a clear feature/endpoint halt on an endpoint */
                    CyAsHalPrintMessage("Calling SetStall on EP %d\n", ep) ;
                    ret = CyAsUsbSetStall(h, ep, StallCallback, 0) ;
                    if (ret != CY_AS_ERROR_SUCCESS)
                    {
                        CyAsHalPrintMessage("******* ERROR SEND SET STALL REQUEST - %d\n", ret) ;
                    }
                }
            }
            break;
        }
    }
    else if ((data[0] & CY_CH9_SETUP_TYPE_MASK) == CY_CH9_SETUP_CLASS_REQUEST)
    {
        /*
        * Handle class requests other than Mass Storage
        */
        ret = CyAsUsbSetStall(h, 0, StallCallback, 0) ;
        CyAsHalPrintMessage("Sending stall request\n") ;
        if (ret != CY_AS_ERROR_SUCCESS)
            CyAsHalPrintMessage("**** cannot set stall state on EP 0\n") ;
    }
    else
    {
        static char buf[1024] ;

        if ((data[0] & 0x80) == 0)
        {
            if (reqlen != 0)
            {
				CyAsHalPrintMessage("OUT setup request with additional data\n") ;
                /* This is an OUT setup request, with additional data to follow */
                /*ret = CyAsUsbReadDataAsync(h, 0, CyFalse, reqlen, buf, EP0DataCallback) ;
				if(ret != CY_AS_ERROR_SUCCESS && ret != CY_AS_ERROR_ASYNC_PENDING)
				{
					CyAsHalPrintMessage("CyAsMtpApp: CyAsUsbReadDataAsync Failed. Reason code: %d\n",ret) ;
				}*/
            }
            else
            {
                CyAsHalPrintMessage("Call setnak\n") ;
                ret = CyAsUsbSetNak(h, 3, StallCallback, 1) ;
                if (ret != CY_AS_ERROR_SUCCESS)
                {
                    CyAsHalPrintMessage("Error in CyAsUsbSetNak - %d\n", ret) ;
                }
            }
        }
        else
        {
            if (reqlen != 0)
            {
                /*
                * This is an unknown setup packet, probably some type of generated packet or a class
                * packet we do not understand.  We just send back some data.
                */
                CyAsHalMemSet(buf, 0x44, sizeof(buf)) ;
                ret = SendSetupData(h, reqlen, reqlen, buf) ;
                if (ret != CY_AS_ERROR_SUCCESS)
                {
                    CyAsHalPrintMessage("Error ending setup data in response to unknown packet\n") ;
                }
                else
                {
                    CyAsHalPrintMessage("Sent setup data associated with the unknown setup packet\n") ;
                }
            }
            else
            {
                CyAsHalPrintMessage("Call setnak\n") ;
                ret = CyAsUsbSetNak(h, 3, StallCallback, 1) ;
                if (ret != CY_AS_ERROR_SUCCESS)
                {
                    CyAsHalPrintMessage("Error in CyAsUsbSetNak - %d\n", ret) ;
                }
            }
        }
    }
}

static void
MyUsbEventCallbackMS(CyAsDeviceHandle h, CyAsUsbEvent ev, void *evdata)
{
    CyAsHalPrintMessage("------------------------------ IN -------------------------------------\n") ;
    switch(ev)
    {
    case CyAsEventUsbSuspend:
        CyAsHalPrintMessage("CyAsEventUsbSuspend received\n") ;
        break ;
    case CyAsEventUsbResume:
		{
			/*CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS;*/
			CyAsHalPrintMessage("CyAsEventUsbResume received\n") ;
			/*ret = CyAsUsbReadDataAsync(g_pAstDevice->astDevHandle, CY_AS_MTP_BULK_OUT_EP,
						CyFalse, 512, g_pAstDevice->astEPBuf,
						TmtpReadCallback) ;

			if(ret != CY_AS_ERROR_SUCCESS && ret != CY_AS_ERROR_ASYNC_PENDING)
			{
				CyAsHalPrintMessage("CyAsMtpApp: CyAsUsbReadDataAsync Failed. Reason code: %d\n",ret) ;
				return ;
			}*/
		}
        break ;
    case CyAsEventUsbReset:
#ifdef DEBUG_ZERO
	desc_p = (CyCh9ConfigurationDesc *)&ConfigFSDesc ;
        other_p = (CyCh9ConfigurationDesc *)&ConfigHSDesc ;
#else
        desc_p = (CyCh9ConfigurationDesc *)&ZeroDesc ;
        other_p = (CyCh9ConfigurationDesc *)&ZeroDesc ;
#endif
        CyAsHalPrintMessage("CyAsEventUsbReset received\n") ;
        break ;
    case CyAsEventUsbSpeedChange:

#ifdef DEBUG_ZERO
	desc_p = (CyCh9ConfigurationDesc *)&ConfigHSDesc ;
        other_p = (CyCh9ConfigurationDesc *)&ConfigFSDesc ;
#else
        desc_p = (CyCh9ConfigurationDesc *)&ZeroDesc ;
        other_p = (CyCh9ConfigurationDesc *)&ZeroDesc ;
#endif
        CyAsHalPrintMessage("CyAsEventUsbSpeedChange received\n") ;
        break ;
    case CyAsEventUsbSetConfig:
        CyAsHalPrintMessage("CyAsEventUsbSetConfig received\n") ;
        gSetConfig = CyTrue ;
        break ;
    case CyAsEventUsbSetupPacket:
        PrintData("CyAsEventUsbSetupPacket received: ", (uint8_t*)evdata, 8) ;
        ProcessSetupPacketRequest(h, (uint8_t *)evdata) ;
        break ;
    case CyAsEventUsbStatusPacket:
        CyAsHalPrintMessage("CyAsEventUsbStatusPacket received\n") ;
        break ;
    case CyAsEventUsbInquiryBefore:
        CyAsHalPrintMessage("CyAsEventUsbInquiryBefore received\n") ;
        {
            CyAsUsbInquiryData *data = (CyAsUsbInquiryData *)evdata ;
            data->updated = CyTrue ;
            data = data ;
        }
        break ;
    case CyAsEventUsbInquiryAfter:
        CyAsHalPrintMessage("CyAsEventUsbInquiryAfter received\n") ;
        break ;
    case CyAsEventUsbStartStop:
        CyAsHalPrintMessage("CyAsEventUsbStartStop received\n") ;
        {
            CyAsUsbStartStopData *data = (CyAsUsbStartStopData *)evdata ;
            data = data ;
        }
        break ;
    default:
        break;
    }
    CyAsHalPrintMessage("------------------------------ OUT -------------------------------------\n") ;
}


int CyAsSymbianStorageTest(const char *pgm, CyAsDeviceHandle h, CyAsHalDeviceTag tag)
{
    CyAsReturnStatus_t ret ;
	uint32_t count = 0 ;

	g_tag = tag ;
	memset(g_pAstDevice,0, sizeof(g_AstDevice));
	g_pAstDevice->astHalTag = tag ;
	g_pAstDevice->astDevHandle = h ;

    /*
    * Give a delay to allow any equipment to be ready (e.g. CATC)
    */
    /*CyAsHalPrintMessage("Press enter to begin USB operation (%s): ", "P Port Enumeration") ;
    fgets(buffer, sizeof(buffer), stdin) ;*/

    CyAsHalPrintMessage("*** CyAsStorageStart...\n") ;
    ret = CyAsStorageStart(h, 0, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        CyAsHalPrintMessage("%s: CyAsStorageStart returned error code %d\n", pgm, ret) ;
        return 0 ;
    }

	CyAsHalPrintMessage("*** CyAsStorageQueryMedia...\n") ;
	ret = CyAsStorageQueryMedia(h, CyAsMediaSDFlash, &count, 0, 0) ;
	if (ret != CY_AS_ERROR_SUCCESS)
	{
	  CyAsHalPrintMessage("%s: Cannot query SD device count - Reason code %d\n", pgm, ret) ;
	  return 0 ;
	}
	CyAsHalPrintMessage("%d %s device(s) found\n", count, media_name) ;

	CyAsHalPrintMessage("*** CyAsStorageClaim...\n") ;
	ret = CyAsStorageClaim(h, CyAsBus_1, 0, 0, 0) ;
	if (ret != CY_AS_ERROR_SUCCESS)
	{
	  CyAsHalPrintMessage("%s: Cannot claim SD media - Reason code %d\n", pgm, ret) ;
	  return 0;
	}

	/* We know that there is only one */
	g_pAstDevice->dev_data.bus = CyAsBus_1 ;
	g_pAstDevice->dev_data.device = 0 ;
	ret = CyAsStorageQueryDevice(h, &(g_pAstDevice->dev_data), 0, 0) ;
	if (ret != CY_AS_ERROR_SUCCESS)
	{
	  CyAsHalPrintMessage("%s: Cannot query SD device - Reason code %d\n", pgm, ret) ;
	  return 0 ;
	}
	CyAsHalPrintMessage("blocksize %d, %d units found\n",
		g_pAstDevice->dev_data.desc_p.block_size, g_pAstDevice->dev_data.desc_p.number_units) ;

	g_pAstDevice->unit_data.bus = CyAsBus_1 ;
	g_pAstDevice->unit_data.device = 0 ;
	g_pAstDevice->unit_data.unit = 0 ;
	/* We know that there is only one */
	ret = CyAsStorageQueryUnit(h, &(g_pAstDevice->unit_data), 0, 0) ;
	if (ret != CY_AS_ERROR_SUCCESS)
	{
	  CyAsHalPrintMessage("%s: Cannot query SD device unit - Reason code %d\n", pgm, ret) ;
	  return 0 ;
	}
	CyAsHalPrintMessage("blocksize %d, %d Block(s) found\n",
		g_pAstDevice->unit_data.desc_p.block_size, g_pAstDevice->unit_data.desc_p.unit_size) ;


	{
		int i = 0 , j;
		char buf[512] = {0} ;
		char expBuf[512] = {0} ;
		memset(buf, 0xa5, 512);
		memset(expBuf, 0xa5, 512);

		CyAsHalPrintMessage("Read SD card\n");
		ret = CyAsStorageRead(	h, 1, 0, 0, 0, buf, 1);
		if (ret != CY_AS_ERROR_SUCCESS)
		{
			CyAsHalPrintMessage("%s: Cannot read first sector of SD card\n", pgm) ;
			return 0 ;
		}
		CyAsHalPrintMessage("Read Result\n");
		for ( i = 0 ; i < 512; i++ )
		{
			if ( (i != 0 ) && (i % 16 == 0))
			{

				CyAsHalPrintMessage("\t");
				for ( j = 0 ; j < 16 ; j++ )
				{
					if ( (buf[i+j] > 32) && (buf[i+j] < 127))
					{
						CyAsHalPrintMessage("%c",buf[i+j]);
					}
					else
					{
						CyAsHalPrintMessage(".");
					}
				}
				CyAsHalPrintMessage("\n");
			}
			CyAsHalPrintMessage("%02x ",buf[i]);
		}
		CyAsHalPrintMessage("\n");

		ret = CyAsStorageWrite(h, 1, 0, 0, 0, expBuf, 1);
		if ( ret != CY_AS_ERROR_SUCCESS )
		{
			CyAsHalPrintMessage("%s: Cannot write to first sector of SD card\n",pgm) ;
			return 0 ;
		}

		memset(buf, 0xa5, 512);
		ret = CyAsStorageRead(h, 1, 0, 0, 0, buf, 1);
		if (ret != CY_AS_ERROR_SUCCESS)
		{
			CyAsHalPrintMessage("%s: Cannot read first sector of SD card\n",pgm) ;
			return 0 ;
		}

		for ( i = 0 ; i < 512; i++ )
		{
			if ( buf[i] != expBuf[i] )
			{

				CyAsHalPrintMessage("EXP[%d]: 0x%02x",i,expBuf[i]);
				CyAsHalPrintMessage("ACT[%d]: 0x%02x",i,buf[i]);
			}
		}
	}


	return 1 ;
}

int CyAsAPIGetHandle( CyAsDeviceHandle h, CyAsHalDeviceTag tag)
{
	g_tag = tag ;
	memset(g_pAstDevice,0, sizeof(g_AstDevice));
	g_pAstDevice->astHalTag = tag ;
	g_pAstDevice->astDevHandle = h ;

	return 1 ;
}

