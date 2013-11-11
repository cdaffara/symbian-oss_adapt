#ifndef USBDESCS_H
#define USBDESCS_H

#include <cyastoria.h>
#include <cyasmtp.h>

char* UsbStrings[] =
{
    "Cypress Semiconductor",            /* Manufacturer (device) - index 1 */
#ifdef DEBUG_ZERO
	#ifdef DEBUG_MSC
		"Astoria MSC Device Symbian",			/* 2 */
	#else
		"Astoria MTP Device Symbian",			/* 2 */
	#endif
#else
    "Astoria TMTP Device Symbian",		/* 2 */
#endif
    "2222222222",				       	/* 3 */
    "Loopback Configuration",			/* 4 */
    "Loopback Interface"                /* 5 */
} ;

CyCh9DeviceQualifierDesc device_qualifier =
{
    sizeof(CyCh9DeviceQualifierDesc),
    CY_CH9_GD_DEVICE_QUALIFIER,		/* DEVICE QUALIFIER TYPE */
    0x200,				/* USB VERSION */
    0x00,				/* DEVICE CLASS */
    0x00,				/* DEVICE SUBCLASS */
    0x00,				/* PROTOCOL */
    64,					/* EP0 PACKET SIZE */
    1,					/* NUMBER OF CONFIGURATIONS */
    0
} ;

/*
* This is the basic device descriptor for this West Bridge test bench.  This descriptor is returned
* by this software for P port based enumeration.
*/
CyCh9DeviceDesc pport_device_desc =
{
    sizeof(CyCh9DeviceDesc),
    CY_CH9_GD_DEVICE,		/* DESCRIPTOR TYPE */
    0x200,			/* USB VERSION */
    0x00,			/* DEVICE CLASS */
    0x00,			/* DEVICE SUBCLASS */
    0x00,			/* PROTOCOL */
    64,				/* EP0 packet size */
#ifdef DEBUG_ZERO
    0x2,
    0x0,
#else
    0x5,			/* CYPRESS VENDOR ID */
    0x5,			/* West Bridge (MADEUP) for composite device */
#endif
    0x2,			/* DEVICE VERSION */
    0x01,			/* STRING ID FOR MANUF. */
    0x02,			/* STRING ID FOR PRODUCT */
    0x03,			/* STRING ID FOR SERIAL NUMBER */
    0x01			/* NUMBER OF CONFIGURATIONS */
} ;

CyCh9ConfigurationDesc ZeroDesc =
{
    sizeof(CyCh9ConfigurationDesc),		/* Descriptor Length */
    CY_CH9_GD_CONFIGURATION,			/* Descriptor Type */
    sizeof(CyCh9ConfigurationDesc),
    0,						/* Number of Interfaces */
    1,						/* Configuration Number */
    0,						/* Configuration String */
    CY_CH9_CONFIG_ATTR_RESERVED_BIT7,		/* Configuration Attributes */
    50						/* Power Requirements */
} ;

#ifndef DEBUG_ZERO
CyCh9ConfigurationDesc ConfigHSDesc =
{
    sizeof(CyCh9ConfigurationDesc),		/* Descriptor Length */
    CY_CH9_GD_CONFIGURATION,			/* Descriptor Type */
    sizeof(CyCh9ConfigurationDesc),
    0,
    1,									/* Configuration Number */
    0,									/* Configuration String */
    CY_CH9_CONFIG_ATTR_RESERVED_BIT7,	/* Configuration attributes */
    50									/* Power requirements */
} ;

CyCh9ConfigurationDesc ConfigFSDesc =
{
    sizeof(CyCh9ConfigurationDesc),		/* Descriptor Length */
    CY_CH9_GD_CONFIGURATION,			/* Descriptor Type */
    sizeof(CyCh9ConfigurationDesc),
    0,
    1,									/* Configuration Number */
    0,									/* Configuration String */
    CY_CH9_CONFIG_ATTR_RESERVED_BIT7,	/* Configuration attributes */
    50									/* Power requirements */
} ;
#else


#ifdef DEBUG_MSC

CyCh9ConfigurationDesc ConfigHSDesc =
{
    sizeof(CyCh9ConfigurationDesc),		/* Descriptor Length */
    CY_CH9_GD_CONFIGURATION,			/* Descriptor Type */
    sizeof(CyCh9ConfigurationDesc),
    0,
    1,									/* Configuration Number */
    0,									/* Configuration String */
    CY_CH9_CONFIG_ATTR_RESERVED_BIT7,	/* Configuration attributes */
    50									/* Power requirements */
} ;

CyCh9ConfigurationDesc ConfigFSDesc =
{
    sizeof(CyCh9ConfigurationDesc),		/* Descriptor Length */
    CY_CH9_GD_CONFIGURATION,			/* Descriptor Type */
    sizeof(CyCh9ConfigurationDesc),
    0,
    1,									/* Configuration Number */
    0,									/* Configuration String */
    CY_CH9_CONFIG_ATTR_RESERVED_BIT7,	/* Configuration attributes */
    50									/* Power requirements */
} ;

#else

#pragma pack(push,1)
typedef struct MyConfigDesc
{
	CyCh9ConfigurationDesc m_config ;
	CyCh9InterfaceDesc     m_interface ;
	/*nxz-debug-z CyCh9EndpointDesc      m_endpoints[2] ;*/
	CyCh9EndpointDesc      m_endpoints[3] ;

} MyConfigDesc ;
#pragma pack(pop)

static MyConfigDesc ConfigFSDesc =
{
	/*Configuration Descriptor*/
	{
		sizeof(CyCh9ConfigurationDesc),	/* Desc Length */
		CY_CH9_GD_CONFIGURATION,	/* Desc Type */
		/* nxz-debug-z sizeof(CyCh9ConfigurationDesc)+ sizeof(CyCh9InterfaceDesc) + 2 * sizeof(CyCh9EndpointDesc),*/
		sizeof(CyCh9ConfigurationDesc)+ sizeof(CyCh9InterfaceDesc) + 3 * sizeof(CyCh9EndpointDesc),
		1,				/* Num of Interface */
		1,				/* Configuration Value */
		251,				/* Configuration */
		CY_CH9_CONFIG_ATTR_RESERVED_BIT7|CY_CH9_CONFIG_ATTR_SELF_POWERED, /* Configuration Attributes */
		50
	}
	,
	/* Interface Descriptor */
	{
		sizeof(CyCh9InterfaceDesc),	/* Desc Length */
		CY_CH9_GD_INTERFACE,		/* Desc Type */
		0,				/* Interface Num */
		0, 				/* Alternate Interface */
		/*nxz-debug-z 2,*/				/* Number of Endpoints */
		3,				/* Number of Endpoints */
		0xff,				/* IF class */
		0x00,				/* IF sub-class */
		0x00,				/* IF protocol */
		251				/* IF string */
	}
	,
	{
		/* EP3_OUT */
		{
			sizeof(CyCh9EndpointDesc),	/* Desc Length */
			CY_CH9_GD_ENDPOINT,		/* Desc Type */
			CY_CH9_MK_EP_ADDR(CY_CH9_ENDPOINT_DIR_OUT, 3),
			CY_CH9_ENDPOINT_ATTR_BULK,
			64,
			255
		},
		/* EP5_IN */
		{
			sizeof(CyCh9EndpointDesc),	/* Desc Length */
			CY_CH9_GD_ENDPOINT,		/* Desc Type */
			CY_CH9_MK_EP_ADDR(CY_CH9_ENDPOINT_DIR_IN, 5),
			CY_CH9_ENDPOINT_ATTR_BULK,
			64,
			255
		},
		/* nxz-debug-z EP7_INT */
		{
			sizeof(CyCh9EndpointDesc),	/* Desc Length */
			CY_CH9_GD_ENDPOINT,		/* Desc Type */
			CY_CH9_MK_EP_ADDR(CY_CH9_ENDPOINT_DIR_IN, 7),
			CY_CH9_ENDPOINT_ATTR_INTERRUPT,
			64,
			0
		}

	}
};

static MyConfigDesc ConfigHSDesc =
{
	/*Configuration Descriptor*/
	{
		sizeof(CyCh9ConfigurationDesc),	/* Desc Length */
		CY_CH9_GD_CONFIGURATION,	/* Desc Type */
		/*nxz-debug-z sizeof(CyCh9ConfigurationDesc)+ sizeof(CyCh9InterfaceDesc) + 2 * sizeof(CyCh9EndpointDesc),*/
		sizeof(CyCh9ConfigurationDesc)+ sizeof(CyCh9InterfaceDesc) + 3 * sizeof(CyCh9EndpointDesc),
		1,				/* Num of Interface */
		1,				/* Configuration Value */
		251,				/* Configuration */
		CY_CH9_CONFIG_ATTR_RESERVED_BIT7, /* Configuration Attributes */
		50
	}
	,
	/* Interface Descriptor */
	{
		sizeof(CyCh9InterfaceDesc),	/* Desc Length */
		CY_CH9_GD_INTERFACE,		/* Desc Type */
		0,				/* Interface Num */
		0, 				/* Alternate Interface */
		/*nxz-debug-z2,	*/			/* Number of Endpoints */
		3,				/* Number of Endpoints */
		0xff,				/* IF class */
		0x00,				/* IF sub-class */
		0x00,				/* IF protocol */
		251				/* IF string */
	}
	,
	{
		/* EP3_OUT */
		{
			sizeof(CyCh9EndpointDesc),	/* Desc Length */
			CY_CH9_GD_ENDPOINT,		/* Desc Type */
			CY_CH9_MK_EP_ADDR(CY_CH9_ENDPOINT_DIR_OUT, 3),
			CY_CH9_ENDPOINT_ATTR_BULK,
			512,
			0
		},
		/* EP5_IN */
		{
			sizeof(CyCh9EndpointDesc),	/* Desc Length */
			CY_CH9_GD_ENDPOINT,		/* Desc Type */
			CY_CH9_MK_EP_ADDR(CY_CH9_ENDPOINT_DIR_IN, 5),
			CY_CH9_ENDPOINT_ATTR_BULK,
			512,
			0
		},
		/* nxz-debug-z EP7_INT */
		{
			sizeof(CyCh9EndpointDesc),	/* Desc Length */
			CY_CH9_GD_ENDPOINT,		/* Desc Type */
			CY_CH9_MK_EP_ADDR(CY_CH9_ENDPOINT_DIR_IN, 7),
			CY_CH9_ENDPOINT_ATTR_INTERRUPT,
			64,
			0
		}
	}
};
#endif
#endif /*DEBUG_ZERO*/

#endif


