
#include <kern_priv.h>
//#include <beagle/beagle_gpio.h>
#include <beagle/variant.h>
//#include <assp/omap3530_assp/omap3530_assp_priv.h>
//#include <assp/omap3530_assp/omap3530_irqmap.h> // GPIO interrupts
//#include <assp/omap3530_assp/omap3530_gpio.h>

//#include <assp.h> // Required for definition of TIsr

#include <cyastoria.h>
#include <cyasmtp.h>
#include <cyasusbinit.h>
//#include <cyasusbdescs.h>
#include "CyAsSymbianStorageDriver.h"

extern TmtpAstDev g_AstDevice;
extern TmtpAstDev * g_pAstDevice;

_LIT(KLitWbAPIMutex,"WBAPIMutex");
static DMutex	*WbApiMutex;
const TUint8 KMutexOrdWB			= 0x11; /**< @internalComponent */

TInt64	iMediaTotalSize;
EXPORT_C int  CyAsSymbianStorageDriver::Open(void)
{
	//CyAsHalDeviceTag          tag;
	CyAsDeviceHandle          h;
	CyAsReturnStatus_t ret ;
	char *media_name = "SD";
	uint32_t count = 0 ;

	Kern::MutexCreate(WbApiMutex, KLitWbAPIMutex, KMutexOrdWB);
	//tag = g_pAstDevice->astHalTag ;
	h = g_pAstDevice->astDevHandle ;
	
    Kern::Printf("*** CyAsStorageStart...\n") ;
    ret = CyAsStorageStart(h, 0, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        Kern::Printf("CyAsSymbianStorageOpen: CyAsStorageStart returned error code %d\n", ret) ;
        return 0 ;
    }

	Kern::Printf("*** CyAsStorageQueryMedia...\n") ;
	ret = CyAsStorageQueryMedia(h, CyAsMediaSDFlash, &count, 0, 0) ;
	if (ret != CY_AS_ERROR_SUCCESS)
	{
	  Kern::Printf("CyAsSymbianStorageOpen: Cannot query %s device count - Reason code %d\n", media_name, ret) ;
	  return 0 ;
	}
	Kern::Printf("%d %s device(s) found\n", count, media_name) ;

	Kern::Printf("*** CyAsStorageClaim...\n") ;
	ret = CyAsStorageClaim(h, CyAsBus_1, 0, 0, 0) ;
	if (ret != CY_AS_ERROR_SUCCESS)
	{
	  Kern::Printf("CyAsSymbianStorageOpen: Cannot claim %s media - Reason code %d\n", media_name, ret) ;
	  return 0;
	}

	/* We know that there is only one */
	g_pAstDevice->dev_data.bus = CyAsBus_1 ;
	g_pAstDevice->dev_data.device = 0 ;
	ret = CyAsStorageQueryDevice(h, &(g_pAstDevice->dev_data), 0, 0) ;
	if (ret != CY_AS_ERROR_SUCCESS)
	{
	  Kern::Printf("CyAsSymbianStorageOpen: Cannot query %s device - Reason code %d\n", media_name, ret) ;
	  return 0 ;
	}

	g_pAstDevice->unit_data.bus = CyAsBus_1 ;
	g_pAstDevice->unit_data.device = 0 ;
	g_pAstDevice->unit_data.unit = 0 ;
	
	/* We know that there is only one */
	ret = CyAsStorageQueryUnit(h, &(g_pAstDevice->unit_data), 0, 0) ;
	if (ret != CY_AS_ERROR_SUCCESS)
	{
	  Kern::Printf("CyAsSymbianStorageOpen: Cannot query %s device unit - Reason code %d\n", media_name, ret) ;
	  return 0 ;
	}
														
	iMediaTotalSize = (TInt64)g_pAstDevice->dev_data.desc_p.block_size * (TInt64)g_pAstDevice->unit_data.desc_p.unit_size;

#if 0
	Kern::Printf("========================================================") ;
	Kern::Printf("WBStorageInfo.type - %d", g_pAstDevice->dev_data.desc_p.type) ;
	Kern::Printf("WBStorageInfo.removable - %d", g_pAstDevice->dev_data.desc_p.removable) ;
	Kern::Printf("WBStorageInfo.writeable - %d", g_pAstDevice->dev_data.desc_p.writeable) ;
	Kern::Printf("WBStorageInfo.block_size - %d Bytes", g_pAstDevice->dev_data.desc_p.block_size) ;
	Kern::Printf("WBStorageInfo.number_units - %d", g_pAstDevice->dev_data.desc_p.number_units) ;
	Kern::Printf("WBStorageInfo.locked - %d", g_pAstDevice->dev_data.desc_p.locked) ;
	Kern::Printf("WBStorageInfo.erase_unit_size - %d Bytes", g_pAstDevice->dev_data.desc_p.erase_unit_size) ;
	Kern::Printf("WBStorageInfo.unit_size - %d", g_pAstDevice->unit_data.desc_p.unit_size) ;
	Kern::Printf("WBStorageInfo.start_block - %d", g_pAstDevice->unit_data.desc_p.start_block) ;
	Kern::Printf("WBStorageInfo.totalsize - %d Bytes", iMediaTotalSize) ;
	Kern::Printf("========================================================\n") ;
#endif
	return 1;
}
EXPORT_C int CyAsSymbianStorageDriver::Close(void)
{
	CyAsDeviceHandle          h;
	CyAsReturnStatus_t ret ;
	
	//tag = g_pAstDevice->astHalTag ;
	h = g_pAstDevice->astDevHandle ;
    Kern::Printf("*** CyAsStorageStop...\n") ;
    ret = CyAsStorageStop(h, 0, 0) ;
    if (ret != CY_AS_ERROR_SUCCESS)
    {
        Kern::Printf("CyAsStorageStop: CyAsStorageStop returned error code %d\n", ret) ;
        return 0 ;
    }
	return 1;
}

EXPORT_C int CyAsSymbianStorageDriver::Read(int	pos, int num_block, void* cybuf)
{
	//CyAsHalDeviceTag          tag;
	CyAsDeviceHandle          h;
	CyAsReturnStatus_t ret ;
	char *tmpbuf;

	Kern::MutexWait(*WbApiMutex); 

	tmpbuf = (char *)cybuf;
	//tag = g_pAstDevice->astHalTag ;
	h = g_pAstDevice->astDevHandle ;
	//CyAsHalPrintMessage("Read SD card\n");
	ret = CyAsStorageRead(	h, 1, 0, 0, pos, tmpbuf, num_block);
	if (ret != CY_AS_ERROR_SUCCESS)
	{
		Kern::Printf("CyAsSymbianStorageRead: Cannot read sector %d (%d@%d)\n", ret, num_block, pos) ;
		return 0 ;
	}
	Kern::MutexSignal(*WbApiMutex); 
	
	return 1;
}


EXPORT_C int CyAsSymbianStorageDriver::Write(int pos, int num_block, void* cybuf)
{
	//CyAsHalDeviceTag          tag;
	CyAsDeviceHandle          h;
	CyAsReturnStatus_t ret ;
	char *tmpbuf;

	Kern::MutexWait(*WbApiMutex); 

	tmpbuf = (char *)cybuf;
	//tag = g_pAstDevice->astHalTag ;
	h = g_pAstDevice->astDevHandle ;

	ret = CyAsStorageWrite(h, 1, 0, 0, pos, tmpbuf, num_block);
	if ( ret != CY_AS_ERROR_SUCCESS )
	{
		Kern::Printf("CyAsSymbianStorageWrite: Cannot write to first sector of SD card\n") ;
		return 0 ;
	}
	Kern::MutexSignal(*WbApiMutex); 

	return 1;
}

EXPORT_C TInt CyAsSymbianStorageDriver::GetMediaType(void)
{
	/* Type of device */
	return (TInt)g_pAstDevice->dev_data.desc_p.type;
}

EXPORT_C TInt CyAsSymbianStorageDriver::GetIsRemovable(void)
{
	/* Is the device removable */
	return g_pAstDevice->dev_data.desc_p.removable;
}

EXPORT_C TInt CyAsSymbianStorageDriver::GetIsWriteable(void)
{
	/* Is the device writeable */
	return g_pAstDevice->dev_data.desc_p.writeable;
}

EXPORT_C TUint16 CyAsSymbianStorageDriver::GetBlockSize(void)
{
	/* Basic block size for device */
	return g_pAstDevice->dev_data.desc_p.block_size;
}

EXPORT_C TUint CyAsSymbianStorageDriver::GetNumberOfUnits(void)
{
	/* Number of LUNs on the device */
	return g_pAstDevice->dev_data.desc_p.number_units;
}

EXPORT_C TInt CyAsSymbianStorageDriver::GetIsLocked(void)
{
	/* Is the device password locked */
	return g_pAstDevice->dev_data.desc_p.locked;
}

EXPORT_C TUint CyAsSymbianStorageDriver::GetEraseBlockSize(void)
{
		/* Size in bytes of an Erase Unit. Block erase operation is only supported for SD storage, 
                    and the erase_unit_size is invalid for all other kinds of storage. */
	return g_pAstDevice->dev_data.desc_p.erase_unit_size;
}

EXPORT_C TUint CyAsSymbianStorageDriver::GetUnitSize(void)
{
	/* Number of blocks in the LUN */
	return g_pAstDevice->unit_data.desc_p.unit_size;
}

EXPORT_C TUint CyAsSymbianStorageDriver::GetStartBlock(void)
{
	/* Physical start block for LUN */
	return g_pAstDevice->unit_data.desc_p.start_block;
}

EXPORT_C TInt64 CyAsSymbianStorageDriver::GetMediaSize(void)
{

	/*(Basic block size for device x  Number of LUNs on the device)  
	g_pAstDevice->dev_data.desc_p.block_size * g_pAstDevice->dev_data.desc_p.number_units */
	return iMediaTotalSize;
}




