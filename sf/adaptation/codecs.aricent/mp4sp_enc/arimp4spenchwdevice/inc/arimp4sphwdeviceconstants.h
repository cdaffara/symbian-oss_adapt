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
* This file defines the constants used in Mpeg4SP/H263 encoder plugin.
*
*/


#ifndef ARIMP4SPENCHWDEVICECONSTANTS_H
#define ARIMP4SPENCHWDEVICECONSTANTS_H

// Compressed Mpeg4 Video Formats Supported
_LIT8(KMPEG4MimeType, "video/mp4v-es");
_LIT8(KMPEG4VTMimeType, "video/MP4V-ES");
_LIT8(KMPEG4MimeTypeLevel0, "video/mp4v-es; profile-level-id=8");
_LIT8(KMPEG4MimeTypeLevel0B, "video/mp4v-es; profile-level-id=9");
_LIT8(KMPEG4MimeTypeLevel1, "video/mp4v-es; profile-level-id=1");
_LIT8(KMPEG4MimeTypeLevel2, "video/mp4v-es; profile-level-id=2");
_LIT8(KMPEG4MimeTypeLevel3, "video/mp4v-es; profile-level-id=3");
_LIT8(KMPEG4MimeTypeLevel4, "video/mp4v-es; profile-level-id=4");
_LIT8(KMPEG4MimeTypeLevel5, "video/mp4v-es; profile-level-id=5");
_LIT8(KMPEG4MimeTypeLevel6, "video/mp4v-es; profile-level-id=6");
_LIT8(KMPEG4MimeTypeLevel7, "video/mp4v-es; profile-level-id=7");


// Compressed H263 Video Formats Supported
_LIT8(KH263MimeType, "video/H263-2000");
_LIT8(KH263MimeTypeProfile0, "video/H263-2000; profile=0");
_LIT8(KH263MimeTypeLevel10, "video/H263-2000; profile=0; level=10");
_LIT8(KH263MimeTypeLevel20, "video/H263-2000; profile=0; level=20");
_LIT8(KH263MimeTypeLevel30, "video/H263-2000; profile=0; level=30");
_LIT8(KH263MimeTypeLevel40, "video/H263-2000; profile=0; level=40");
_LIT8(KH263MimeTypeLevel45, "video/H263-2000; profile=0; level=45");


// Mpeg4 and H263 Levels
#define KMPEG4_LEVEL_UNKNOWN   200
#define KMPEG4_LEVEL_0         0
#define KMPEG4_LEVEL_0B        10
#define KMPEG4_LEVEL_1         1
#define KMPEG4_LEVEL_2         2
#define KMPEG4_LEVEL_3         3
#define KMPEG4_LEVEL_4         4
#define KMPEG4_LEVEL_5         5
#define KMPEG4_LEVEL_6         6
#define KMPEG4_LEVEL_7         7

#define KH263_LEVEL_UNKNOWN    300
#define KH263_LEVEL_10         10
#define KH263_LEVEL_20         20
#define KH263_LEVEL_30         30
#define KH263_LEVEL_40         40
#define KH263_LEVEL_45         45


_LIT(KMPEG4H263EncManufacturer,"Aricent");
_LIT(KMPEG4H263EncIdentifier, "ARM MPEG4 H263 Video Encoder Hw Device");

// 1 SPS and 1 PPS
const TInt KWordLength				= 4;
const TInt KDoubleWordLength		= 8;


const TInt KMPEG4H263ENCIMPL_MAJOR_VERSION = 1;
const TInt KMPEG4H263ENCIMPL_MINOR_VERSION = 0;
const TInt KMPEG4H263ENCIMPL_BUILD_VERSION = 0;

// Size - width,height related

const TInt KMPEG4H263ENCIMPL_CIF_WIDTH    = 352;
const TInt KMPEG4H263ENCIMPL_CIF_HEIGHT   = 288;

const TInt KMPEG4H263ENCIMPL_QCIF_WIDTH   = 176;
const TInt KMPEG4H263ENCIMPL_QCIF_HEIGHT  = 144;

const TInt KMPEG4H263ENCIMPL_SQCIF_WIDTH  = 128;
const TInt KMPEG4H263ENCIMPL_SQCIF_HEIGHT = 96;

const TInt KMPEG4H263ENCIMPL_QVGA_WIDTH   = 320;
const TInt KMPEG4H263ENCIMPL_QVGA_HEIGHT  = 240;

const TInt KMPEG4H263ENCIMPL_VGA_WIDTH    = 640;
const TInt KMPEG4H263ENCIMPL_VGA_HEIGHT   = 480;

const TInt KMPEG4H263ENCIMPL_720P_WIDTH   = 1280;
const TInt KMPEG4H263ENCIMPL_720P_HEIGHT  = 720;

//Level for H263
const TInt KH263ENCIMPL_LEVEL10 = 10;
const TInt KH263ENCIMPL_LEVEL20 = 20;
const TInt KH263ENCIMPL_LEVEL30 = 30;
const TInt KH263ENCIMPL_LEVEL40 = 40;
const TInt KH263ENCIMPL_LEVEL45 = 45;

//Level for MPEG4
const TInt KMPEG4ENCIMPL_LEVEL_0  = 0;
const TInt KMPEG4ENCIMPL_LEVEL_1  = 1;
const TInt KMPEG4ENCIMPL_LEVEL_2  = 2;
const TInt KMPEG4ENCIMPL_LEVEL_3  = 3;
const TInt KMPEG4ENCIMPL_LEVEL_4A = 4;
const TInt KMPEG4ENCIMPL_LEVEL_5  = 5;
const TInt KMPEG4ENCIMPL_LEVEL_6  = 6;
const TInt KMPEG4ENCIMPL_LEVEL_7  = 7;
const TInt KMPEG4ENCIMPL_LEVEL_0B = 10;

// Bitrate related (bits per second) for H263
const TInt KH263ENCIMPL_DEFAULT_BITRATE = 64000;
const TInt KH263ENCIMPL_BITRATE_LEVEL_10 = 64000;
const TInt KH263ENCIMPL_BITRATE_LEVEL_20 = 128000;
const TInt KH263ENCIMPL_BITRATE_LEVEL_30 = 384000;
const TInt KH263ENCIMPL_BITRATE_LEVEL_40 = 2048000;
const TInt KH263ENCIMPL_BITRATE_LEVEL_45 = 128000;

// Bitrate (bits per second) for MPEG4
const TInt KMPEG4ENCIMPL_BITRATE_LEVEL_0  = 64000;
const TInt KMPEG4ENCIMPL_BITRATE_LEVEL_1  = 64000;
const TInt KMPEG4ENCIMPL_BITRATE_LEVEL_2  = 128000;
const TInt KMPEG4ENCIMPL_BITRATE_LEVEL_3  = 384000;
const TInt KMPEG4ENCIMPL_BITRATE_LEVEL_4A = 4000000;
const TInt KMPEG4ENCIMPL_BITRATE_LEVEL_5  = 8000000;
const TInt KMPEG4ENCIMPL_BITRATE_LEVEL_6  = 12000000;
const TInt KMPEG4ENCIMPL_BITRATE_LEVEL_7  = 38400000;
const TInt KMPEG4ENCIMPL_BITRATE_LEVEL_0B = 128000;

// Level related mapping
const TInt KH263ENCIMPL_LEVEL_UNKNOWN = 0;

// o/p buffer related
const TInt KMPEG4H263ENCIMPL_MAXNUM_OUTPUTBUFFERS                      = 2;
const TInt KMPEG4H263ENCIMPL_MAXNUM_TEMPOUTPUTBUFFERS                  = 2;
const TInt KMPEG4H263ENCIMPL_MAXNUM_SEGMENTBUFFERS                     = 30;
const TInt KMPEG4H263ENCIMPL_FACTOR_FOR_OUTPUTBUFFERSIZE_IN_PACKETMODE = 2;

// o/p buffer size related
const TInt KMPEG4H263ENCIMPL_DEFAULT_SEGMENTSIZE = 256;
const TInt KMPEG4H263ENCIMPL_MIN_SEGMENTSIZE     = 192;
const TInt KMPEG4H263ENCIMPL_MAX_SEGMENTSIZE     = 300;

// Max coded picture size in case of Level 1b
const TInt KH263ENCIMPL_LEVEL1b_MAXCPBSIZE = 39375;

// Max coded picture size in case of Level 1.1
const TInt KH263ENCIMPL_LEVEL11_MAXCPBSIZE = 56250;

// picture rate related
const TReal KMPEG4H263ENCIMPL_MAX_PICTURERATE     = 30.0;
const TReal KMPEG4H263ENCIMPL_DEFAULT_PICTURERATE = 15.0;

const TInt KMPEG4H263ENCIMPL_NUM_COMPLEXITYLEVELS     = 4;
const TInt KMPEG4H263ENCIMPL_MAXNUM_REFERENCEPICTURES = 1;

// Default random access rate- 1 per 3 seconds i.e.1 after 45 frames at 15 fps
const TReal KMPEG4H263ENCIMPL_DEFAULT_RANDOMACCESSRATE = 0.3333333;

const TInt KMPEG4SUPPMIMEARRAYMAXINDEXVALUE = 10;
const TInt KH263SUPPMIMEARRAYMAXINDEXVALUE  = 17;

const TUint KMaxSupportedBitRate = 38400000;

const TUint KDefaultTimerResolution  = 30000;

#endif //ARIMP4SPENCHWDEVICECONSTANTS_H
