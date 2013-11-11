/* Cypress West Bridge API source file (cyasusb.c)
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

#include "cyashal.h"
#include "cyasusb.h"
#include "cyaserr.h"
#include "cyaslowlevel.h"
#include "cyasdma.h"

typedef enum CyAsPhysicalEndpointState
{
    CyAsEPFree,
    CyAsEPIn,
    CyAsEPOut,
    CyAsEPIsoIn,
    CyAsEPIsoOut
} CyAsPhysicalEndpointState;


/*
* This map is used to map an index between 1 and 10 to a logical endpoint number.  This
* is used to map LEP register indexes into actual EP numbers.
*/
static CyAsEndPointNumber_t EndPointMap[] = { 3, 5, 7, 9, 10, 11, 12, 13, 14, 15 } ;

#define CY_AS_EPCFG_1024            (1 << 3)
#define CY_AS_EPCFG_DBL             (0x02)
#define CY_AS_EPCFG_TRIPLE          (0x03)
#define CY_AS_EPCFG_QUAD            (0x00)

/*
 * NB: This table contains the register values for PEP1 and PEP3.  PEP2 and PEP4 only
 *     have a bit to change the direction of the PEP and therefre are not represented
 *     in this table.
 */
static uint8_t PepRegisterValues[12][4] =
{
    /* Bit 1:0 buffering, 0 = quad, 2 = double, 3 = triple */
    /* Bit 3 size, 0 = 512, 1 = 1024 */
    { 
        CY_AS_EPCFG_DBL, 
        CY_AS_EPCFG_DBL, 
    },      /* Config 1  - PEP1 (2 * 512), PEP2 (2 * 512), PEP3 (2 * 512), PEP4 (2 * 512) */
    { 
        CY_AS_EPCFG_DBL, 
        CY_AS_EPCFG_QUAD, 
    },      /* Config 2  - PEP1 (2 * 512), PEP2 (2 * 512), PEP3 (4 * 512), PEP4 (N/A) */
    { 
        CY_AS_EPCFG_DBL, 
        CY_AS_EPCFG_DBL | CY_AS_EPCFG_1024, 
    },      /* Config 3  - PEP1 (2 * 512), PEP2 (2 * 512), PEP3 (2 * 1024), PEP4(N/A) */
    { 
        CY_AS_EPCFG_QUAD, 
        CY_AS_EPCFG_DBL, 
    },      /* Config 4  - PEP1 (4 * 512), PEP2 (N/A), PEP3 (2 * 512), PEP4 (2 * 512) */
    { 
        CY_AS_EPCFG_QUAD, 
        CY_AS_EPCFG_QUAD, 
    },      /* Config 5  - PEP1 (4 * 512), PEP2 (N/A), PEP3 (4 * 512), PEP4 (N/A) */
    { 
        CY_AS_EPCFG_QUAD, 
        CY_AS_EPCFG_1024 | CY_AS_EPCFG_DBL, 
    },      /* Config 6  - PEP1 (4 * 512), PEP2 (N/A), PEP3 (2 * 1024), PEP4 (N/A) */
    { 
        CY_AS_EPCFG_1024 | CY_AS_EPCFG_DBL, 
        CY_AS_EPCFG_DBL, 
    },      /* Config 7  - PEP1 (2 * 1024), PEP2 (N/A), PEP3 (2 * 512), PEP4 (2 * 512) */
    { 
        CY_AS_EPCFG_1024 | CY_AS_EPCFG_DBL, 
        CY_AS_EPCFG_QUAD, 
    },      /* Config 8  - PEP1 (2 * 1024), PEP2 (N/A), PEP3 (4 * 512), PEP4 (N/A) */
    { 
        CY_AS_EPCFG_1024 | CY_AS_EPCFG_DBL, 
        CY_AS_EPCFG_1024 | CY_AS_EPCFG_DBL, 
    },      /* Config 9  - PEP1 (2 * 1024), PEP2 (N/A), PEP3 (2 * 1024), PEP4 (N/A)*/
    { 
        CY_AS_EPCFG_TRIPLE, 
        CY_AS_EPCFG_TRIPLE, 
    },      /* Config 10 - PEP1 (3 * 512), PEP2 (N/A), PEP3 (3 * 512), PEP4 (2 * 512)*/
    { 
        CY_AS_EPCFG_TRIPLE | CY_AS_EPCFG_1024, 
        CY_AS_EPCFG_DBL,
    },      /* Config 11 - PEP1 (3 * 1024), PEP2 (N/A), PEP3 (N/A), PEP4 (2 * 512) */
    { 
        CY_AS_EPCFG_QUAD | CY_AS_EPCFG_1024, 
        CY_AS_EPCFG_DBL,
    },      /* Config 12 - PEP1 (4 * 1024), PEP2 (N/A), PEP3 (N/A), PEP4 (N/A) */
} ;

static CyAsReturnStatus_t
FindEndpointDirections(CyAsDevice *dev_p, CyAsPhysicalEndpointState epstate[4])
{
    int i ;
    CyAsPhysicalEndpointState desired ;

    /*
     * Note, there is no error checking here becuase ISO error checking happens when
     * the API is called.
     */
    for(i = 0 ; i < 10 ; i++)
    {
        int epno = EndPointMap[i] ;
        if (dev_p->usb_config[epno].enabled)
        {
            int pep = dev_p->usb_config[epno].physical ;
            if (dev_p->usb_config[epno].type == CyAsUsbIso)
            {
                /*
                 * Marking this as an ISO endpoint, removes the physical EP from consideration when
                 * mapping the remaining EPs.
                 */
                if (dev_p->usb_config[epno].dir == CyAsUsbIn)
                    desired = CyAsEPIsoIn ;
                else
                    desired = CyAsEPIsoOut ;
            }
            else
            {
                if (dev_p->usb_config[epno].dir == CyAsUsbIn)
                    desired = CyAsEPIn ;
                else
                    desired = CyAsEPOut ;
            }

            /* NB: Note the API calls insure that an ISO endpoint has a physical and logical
             *     EP number that are the same, therefore this condition is not enforced here.
             */
            if (epstate[pep - 1] != CyAsEPFree && epstate[pep - 1] != desired)
                return CY_AS_ERROR_INVALID_CONFIGURATION ;

            epstate[pep - 1] = desired ;
        }
    }

    /*
     * Create the EP1 config values directly.
     * Both EP1OUT and EP1IN are invalid by default.
     */
    dev_p->usb_ep1cfg[0] = 0 ;
    dev_p->usb_ep1cfg[1] = 0 ;
    if (dev_p->usb_config[1].enabled)
    {
        if ((dev_p->usb_config[1].dir == CyAsUsbOut) || (dev_p->usb_config[1].dir == CyAsUsbInOut))
        {
            /* Set the valid bit and type field. */
            dev_p->usb_ep1cfg[0] = (1 << 7) ;
            if (dev_p->usb_config[1].type == CyAsUsbBulk)
                dev_p->usb_ep1cfg[0] |= (2 << 4) ;
            else
                dev_p->usb_ep1cfg[0] |= (3 << 4) ;
        }

        if ((dev_p->usb_config[1].dir == CyAsUsbIn) || (dev_p->usb_config[1].dir == CyAsUsbInOut))
        {
            /* Set the valid bit and type field. */
            dev_p->usb_ep1cfg[1] = (1 << 7) ;
            if (dev_p->usb_config[1].type == CyAsUsbBulk)
                dev_p->usb_ep1cfg[1] |= (2 << 4) ;
            else
                dev_p->usb_ep1cfg[1] |= (3 << 4) ;
        }
    }

    return CY_AS_ERROR_SUCCESS ;
}

static void
CreateRegisterSettings(CyAsDevice *dev_p, CyAsPhysicalEndpointState epstate[4])
{
    int i ;
    uint8_t v ;

    for(i = 0 ; i < 4 ; i++)
    {
        if (i == 0)
            dev_p->usb_pepcfg[i] = PepRegisterValues[dev_p->usb_phy_config - 1][0] ;    /* Start with the values that specify size */
        else if (i == 2)
            dev_p->usb_pepcfg[i] = PepRegisterValues[dev_p->usb_phy_config - 1][1] ;    /* Start with the values that specify size */
        else
            dev_p->usb_pepcfg[i] = 0 ;

        if (epstate[i] == CyAsEPIsoIn || epstate[i] == CyAsEPIn)
            dev_p->usb_pepcfg[i] |= (1 << 6) ;                                          /* Adjust direction if it is in */
    }

    /* Configure the logical EP registers */
    for(i = 0 ; i < 10 ; i++)
    {
        int val ;
        int epnum = EndPointMap[i] ;

        v = 0x10 ;      /* PEP 1, Bulk Endpoint, EP not valid */
        if (dev_p->usb_config[epnum].enabled)
        {
            v |= (1 << 7) ;                                             /* Enabled */

            val = dev_p->usb_config[epnum].physical - 1 ;
            CyAsHalAssert(val >= 0 && val <= 3) ;
            v |= (val << 5) ;

            switch(dev_p->usb_config[epnum].type)
            {
            case CyAsUsbBulk:
                val = 2 ;
                break ;
            case CyAsUsbInt:
                val = 3 ;
                break ;
            case CyAsUsbIso:
                val = 1 ;
                break ;
            default:
                CyAsHalAssert(CyFalse) ;
                break ;
            }
            v |= (val << 3) ;
        }

        dev_p->usb_lepcfg[i] = v ;
    }
}


CyAsReturnStatus_t
CyAsUsbMapLogical2Physical(CyAsDevice *dev_p)
{
    CyAsReturnStatus_t ret ;

    /* Physical EPs 3 5 7 9 respectively in the array */
    CyAsPhysicalEndpointState epstate[4] = { CyAsEPFree, CyAsEPFree, CyAsEPFree, CyAsEPFree } ;

    /* Find the direction for the endpoints */
    ret = FindEndpointDirections(dev_p, epstate) ;
    if (ret != CY_AS_ERROR_SUCCESS)
        return ret ;

    /*
     * Now create the register settings based on the given assigned of logical EPs to
     * physical endpoints.
     */
    CreateRegisterSettings(dev_p, epstate) ;

    return ret ;
}

static uint16_t
GetMaxDmaSize(CyAsDevice *dev_p, CyAsEndPointNumber_t ep)
{
    uint16_t size = dev_p->usb_config[ep].size ;

    if (size == 0)
    {
        switch(dev_p->usb_config[ep].type)
        {
        case CyAsUsbControl:
            size = 64 ;
            break ;

        case CyAsUsbBulk:
            size = CyAsDeviceIsUsbHighSpeed(dev_p) ? 512 : 64 ;
            break ;

        case CyAsUsbInt:
            size = CyAsDeviceIsUsbHighSpeed(dev_p) ? 1024 : 64 ;
            break ;

        case CyAsUsbIso:
            size = CyAsDeviceIsUsbHighSpeed(dev_p) ? 1024 : 1023 ;
            break ;
        }
    }

    return size ;
}

CyAsReturnStatus_t
CyAsUsbSetDmaSizes(CyAsDevice *dev_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint32_t i ;

    for(i = 0 ; i < 10 ; i++)
    {
        CyAsUsbEndPointConfig *config_p = &dev_p->usb_config[EndPointMap[i]] ;
        if (config_p->enabled)
        {
            ret = CyAsDmaSetMaxDmaSize(dev_p, EndPointMap[i], GetMaxDmaSize(dev_p, EndPointMap[i])) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                break ;
        }
    }

    return ret ;
}

CyAsReturnStatus_t
CyAsUsbSetupDma(CyAsDevice *dev_p)
{
    CyAsReturnStatus_t ret = CY_AS_ERROR_SUCCESS ;
    uint32_t i ;

    for(i = 0 ; i < 10 ; i++)
    {
        CyAsUsbEndPointConfig *config_p = &dev_p->usb_config[EndPointMap[i]] ;
        if (config_p->enabled)
        {
            /* Map the endpoint direction to the DMA direction */
            CyAsDmaDirection dir = CyAsDirectionOut ;
            if (config_p->dir == CyAsUsbIn)
                dir = CyAsDirectionIn ;

            ret = CyAsDmaEnableEndPoint(dev_p, EndPointMap[i], CyTrue, dir) ;
            if (ret != CY_AS_ERROR_SUCCESS)
                break ;
        }
    }

    return ret ;
}
