/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
* ne1_tb\inc\variantmediadef.h
* Media definitions for NE1_TBVariant variant.
* Each Media Driver requires the following definitions
* DRIVECOUNT - The total number of local drive object to be assigned to the Media Driver (1-KMaxLocalDrives)
* DRIVELIST - A list of the local drive numbers (each separated with a comma) to be assigned to the Media Driver.
* Each in the range 0 - (KMaxLocalDrives-1). Total number of drive numbers must equal the value
* of DRIVECOUNT.
* NUMMEDIA - The total number of media objects to be assigned to the Media Driver.
* DRIVENAME - A name for the drive group.
* For the complete set of media definitions
* - The total number of local drive objects assigned should not exceed KMaxLocalDrives.
* - Each Media Driver should be assigned a unique set of drive numbers - no conflicts between Media Drivers.
* - The total number of media objects assigned should not exceed KMaxLocalDrives.
*
*/


 
#ifndef __VARIANTMEDIADEF_H__
#define __VARIANTMEDIADEF_H__

// Variant parameters for IRAM Media Driver (MEDINT.PDD)
#define IRAM_DRIVECOUNT 1
#define IRAM_DRIVELIST 0
#define IRAM_NUMMEDIA 1	
#define IRAM_DRIVENAME "IRam"

// Variant parameters for LFFS Media Driver (MEDLFS.PDD)
#define LFFS_DRIVECOUNT 1
#define LFFS_DRIVELIST 8
#define LFFS_NUMMEDIA 1	
#define LFFS_DRIVENAME "Flash"

// Variant parameters for the MMC Controller (EPBUSMMC.DLL)
#define MMC0_DRIVECOUNT 1
#define MMC0_DRIVELIST 1
#define MMC0_NUMMEDIA 1	
#define MMC0_DRIVENAME "MultiMediaCard0"

// Variant parameters for the SDIO Controller (EPBUSSDIO.DLL)
#define SDIO0_DRIVECOUNT 1
#define SDIO0_DRIVELIST 4
#define SDIO0_NUMMEDIA 1
#define SDIO0_DRIVENAME "SecureDiskCard0"

// Variant parameters for the NAND media driver (MEDNAND.PDD)
#define NAND_DRIVECOUNT 10
#define NAND_DRIVELIST 2,3,5,6,7,9,10,11,12,13

#define NAND_NUMMEDIA 1	
#define NAND_DRIVENAME "Nand"


#ifdef __NAND_DEMAND_PAGING__

#ifdef __NAND_DATA_PAGING__
	#define PAGING_TYPE				DPagingDevice::ERom | DPagingDevice::ECode | DPagingDevice::EData
#else
	#define PAGING_TYPE				DPagingDevice::ERom | DPagingDevice::ECode
#endif	// __NAND_DATA_PAGING__

#define NAND_PAGEDRIVELIST		2,3,5,6,7,9,10,11,12,13	// code paging from all drives.
#define	NAND_PAGEDRIVECOUNT		10


#define NUM_PAGES				2			// defines the size of fragment
#endif // __DEMAND_PAGING__

#define NFE_DRIVECOUNT		3
#define NFE_DRIVELIST		1 ,2 ,13
#define NFE_DRIVELETTERLIST 3 ,8 ,-1		// EDRive? IDs of the each instance. (EDriveD,EDriveI,UNMAPPED from estartnandtestpaged.txt)

#define NFE_PAGEDRIVECOUNT	1
#define NFE_PAGEDRIVELIST	2				// code paging on first writable partition.in slot #2

#define NFE_NUM_PAGES 2						// the number of pages for paging. i.e. the largest block size of any attached media

#define NFE_INSTANCE_COUNT 2				// the number of NFE media driver instances
#define NFE_INSTANCE_DRIVE_COUNTS 1,2		// the number of drives in NFE_DRIVELIST for each instance of the driver
#define NFE_INSTANCE_PAGEDRIVE_COUNTS 0,1	// the number of drives in NFE_PAGEDRIVELIST for each instance of the driver
#define NFE_INSTANCE_NUM_PAGES 0,4			// the number of pages for each instance of the driver
#define NFE_INSTANCE_PAGING_TYPE 0,DPagingDevice::ECode | DPagingDevice::EData	// the paging type for each instance of the driver 

#endif
