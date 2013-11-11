
#ifndef ASTORIAUSBINIT_H
#define ASTORIAUSBINIT_H

/* The BUS bits */
#define CY_TEST_BUS_0               (0x01)
#define CY_TEST_BUS_1               (0x02)

typedef struct TmtpAstDev
{
	CyAsHalDeviceTag          astHalTag;
	CyAsDeviceHandle          astDevHandle;

	/* EP related stats */
	volatile uint8_t      	  astEPDataAvail;
    uint32_t                  astEPBuflength;
    uint8_t                   astEPBuf[512];

    /* Current Transaction Id */
    uint32_t tId ;

    /* Data members to be used by user-implemented MTPEventCallback() via the relevant interface methods */
    volatile CyBool tmtpSendComplete;
    volatile CyBool tmtpGetComplete;
    volatile CyBool tmtpNeedNewBlkTbl;
	CyAsStorageQueryDeviceData dev_data;
	CyAsStorageQueryUnitData unit_data;
    /* Data member used to store the SendObjectComplete event data */
    CyAsMTPSendObjectCompleteData tmtpSendCompleteData;

    /* Data member used to store the GetObjectComplete event data */
    CyAsMTPGetObjectCompleteData tmtpGetCompleteData;

    uint8_t configDone ;
} TmtpAstDev;


int
CyAsAPIUsbInit(
	const char *pgm,
	CyAsDeviceHandle h,
	CyAsHalDeviceTag tag);

int
CyAsSymbianStorageTest(
	const char *pgm,
	CyAsDeviceHandle h,
	CyAsHalDeviceTag tag);

int
CyAsAPIGetHandle(
	CyAsDeviceHandle h,
	CyAsHalDeviceTag tag);


#endif
