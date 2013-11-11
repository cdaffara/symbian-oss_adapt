/*
* Copyright (c) 2009 Aricent and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Aricent - initial contribution.
*
* Contributors:
*
* Description:
* This file defines the constants used in H264 encoder plugin.
*
*/

#ifndef ARIH264HWDEVICECONSTANTS_H
#define ARIH264HWDEVICECONSTANTS_H

// Compressed H264 Video Formats Supported
_LIT8(KH264MimeType, "video/H264");
_LIT8(KH264MimeType2, "video/H264; profile-level-id=42800A");
_LIT8(KH264MimeType3, "video/H264; profile-level-id=42800B");
_LIT8(KH264MimeType4, "video/H264; profile-level-id=42800C");
_LIT8(KH264MimeType5, "video/H264; profile-level-id=42800D");
_LIT8(KH264MimeType6, "video/H264; profile-level-id=428014");
_LIT8(KH264MimeType7, "video/H264; profile-level-id=428015");
_LIT8(KH264MimeType8, "video/H264; profile-level-id=428016");
_LIT8(KH264MimeType9, "video/H264; profile-level-id=42801E");
_LIT8(KH264MimeType10, "video/H264; profile-level-id=42900B");


_LIT(KH264EncManufacturer,"Aricent");
_LIT(KH264EncIdentifier, "ARM H264 Video Encoder Hw Device");

// 1 SPS and 1 PPS
const TInt KNumOfConfigParams		= 2;
// 1 SPS, 1 PPS and 1 SEI
const TInt KNumOfConfigParams_SEI	= 3;
// 1 SPS, 1 PPS, 4SEI
const TInt KNumOfNAL_SEI	        = 6;
const TInt KWordLength				= 4;
const TInt KDoubleWordLength		= 8;

const TInt KH264ENCIMPL_MAJOR_VERSION =  1;
const TInt KH264ENCIMPL_MINOR_VERSION =  0;
const TInt KH264ENCIMPL_BUILD_VERSION =  0;

// Size - width,height related
const TInt KH264ENCIMPL_SQCIF_WIDTH  		 = 128;
const TInt KH264ENCIMPL_SQCIF_HEIGHT 		 = 96;
const TInt KH264ENCIMPL_QCIF_WIDTH   		 = 176;
const TInt KH264ENCIMPL_QCIF_HEIGHT  		 = 144;
const TInt KH264ENCIMPL_QVGA_WIDTH   		 = 320;
const TInt KH264ENCIMPL_QVGA_HEIGHT  		 = 240;
const TInt KH264ENCIMPL_CIF_WIDTH    		 = 352;
const TInt KH264ENCIMPL_CIF_HEIGHT   		 = 288;
const TInt KH264ENCIMPL_VGA_WIDTH   		 = 640;
const TInt KH264ENCIMPL_VGA_HEIGHT   		 = 480;
const TInt KH264ENCIMPL_SDTV_WIDTH   		 = 720;
const TInt KH264ENCIMPL_SDTV_HEIGHT_PAL      = 576;
const TInt KH264ENCIMPL_SDTV_HEIGHT_NTSC     = 480;
const TInt KH264ENCIMPL_HALFSDTV_WIDTH       = 360;
const TInt KH264ENCIMPL_HALFSDTV_HEIGHT_PAL  = 288;
const TInt KH264ENCIMPL_HALFSDTV_HEIGHT_NTSC = 240;
const TInt KH264ENCIMPL_720P_HEIGHT          = 720;
const TInt KH264ENCIMPL_720P_WIDTH           = 1280;

// Bitrate related (bits per second)
const TInt KH264ENCIMPL_DEFAULT_BITRATE   =  64000;
const TInt KH264ENCIMPL_BITRATE_LEVEL_1b  =  128000;
const TInt KH264ENCIMPL_BITRATE_LEVEL_1   =  64000;
const TInt KH264ENCIMPL_BITRATE_LEVEL_11  =  192000;
const TInt KH264ENCIMPL_BITRATE_LEVEL_12  =  384000;
const TInt KH264ENCIMPL_BITRATE_LEVEL_13  =  768000;
const TInt KH264ENCIMPL_BITRATE_LEVEL_2   =  2000000;
const TInt KH264ENCIMPL_BITRATE_LEVEL_21  =  4000000;
const TInt KH264ENCIMPL_BITRATE_LEVEL_22  =  4000000;
const TInt KH264ENCIMPL_BITRATE_LEVEL_3   =  10000000;

// Level related mapping
const TInt KH264ENCIMPL_LEVEL_UNKNOWN = 0;
const TInt KH264ENCIMPL_LEVEL_1b      = 9;
const TInt KH264ENCIMPL_LEVEL_1       = 10;
const TInt KH264ENCIMPL_LEVEL_11      = 11;
const TInt KH264ENCIMPL_LEVEL_12      = 12;
const TInt KH264ENCIMPL_LEVEL_13      = 13;
const TInt KH264ENCIMPL_LEVEL_2       = 20;
const TInt KH264ENCIMPL_LEVEL_21      = 21;
const TInt KH264ENCIMPL_LEVEL_22      = 22;
const TInt KH264ENCIMPL_LEVEL_3       = 30;

// o/p buffer related
const TInt KH264ENCIMPL_MAXNUM_OUTPUTBUFFERS = 2;
const TInt KH264ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS = 2;
const TInt KH264ENCIMPL_MAXNUM_SEGMENTBUFFERS = 30;
const TInt KH264ENCIMPL_FACTOR_FOR_OUTPUTBUFFERSIZE_IN_PACKETMODE = 2;

// o/p buffer size related
const TInt KH264ENCIMPL_DEFAULT_SEGMENTSIZE = 256;
const TInt KH264ENCIMPL_MIN_SEGMENTSIZE     = 192;
const TInt KH264ENCIMPL_MAX_SEGMENTSIZE     = 300;

// Max coded picture size in case of Level 1b
const TInt KH264ENCIMPL_LEVEL1b_MAXCPBSIZE  = 39375;
// Max coded picture size in case of Level 1.1
const TInt KH264ENCIMPL_LEVEL11_MAXCPBSIZE  = 56250;

// picture rate related
const TReal KH264ENCIMPL_MAX_PICTURERATE     = 30.0;
const TReal KH264ENCIMPL_DEFAULT_PICTURERATE = 15.0;
const TReal KH264ENCIMPL_PICTURERATE_12_5    = 12.5;
const TReal KH264ENCIMPL_PICTURERATE_25      = 25.0;

const TInt KH264ENCIMPL_NUM_COMPLEXITYLEVELS     = 3;
const TInt KH264ENCIMPL_ENCODEAHEAD              = 0;
const TInt KH264ENCIMPL_MAXNUM_REFERENCEPICTURES = 16;

// Default random access rate - 1 per 3seconds ie 1 after 45 frames at 15 fps
const TInt KH264ENCIMPL_DEFAULT_RANDOMACCESSRATE =  0.3333333;

// Minimum size of slice in coded picture format.
const TInt KH264ENCIMPL_MIN_SLICESIZE = 200;

const TInt KPOLLINGINTERVAL  = 100000;
const TUint KMAXSUPPBITRATE  = 10000000;

#endif //ARIH264HWDEVICECONSTANTS_H
