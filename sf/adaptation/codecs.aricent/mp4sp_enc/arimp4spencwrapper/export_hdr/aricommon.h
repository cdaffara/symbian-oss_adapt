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
* Common header used by plugin, wrapper and codec.
*/

#ifndef ARICOMMON_H
#define ARICOMMON_H

typedef signed char          int8;
typedef unsigned char        uint8;
typedef short int            int16;
typedef unsigned short int   uint16;
typedef int                  int32;
typedef unsigned int         uint32;

typedef float                flt32;
typedef double               flt64;

typedef unsigned char        tBool;
typedef signed int           tError;


enum {I_VOP, P_VOP, B_VOP, ACCESS_VOP, SKIP_VOP};
enum {LOW, MEDIUM, HIGH, COMPLEX};
enum {CBR, VBR, HQVBR};
enum {H263,MPEG4};

#define E_TRUE               1
#define E_FALSE              0

#define E_ON                 1
#define E_OFF                0

#define E_DEBUG              1
#define E_RELEASE            0

#define E_SUCCESS            0
#define E_FAILURE           (-1)
#define E_OUT_OF_MEMORY     (-2)
#define E_OUT_OF_RANGE      (-3)
#define E_FILE_CREATE_FAIL  (-4)
#define E_UNDEFINED_FLAG    (-5)
#define E_FILE_READ_FAIL    (-6)
#define E_FILE_OPEN_FAIL    (-7)
#define E_END_OF_FILE		(-8)
#define E_NOT_COMPLETE		(-9)
#define E_OUTBUF_OVERFLOW	(-10)
#define E_USER_ERROR_BASE   (-1000)
#define mIsSuccess(code)    ((code)>=0)
#define mIsFailure(code)    ((code)<0)

#if defined(__MARM_ARM4__) || defined(__MARM_ARMI__) || defined(__MARM_ARM5__) || defined(__MARM_ARMV5__) || defined(__MARM_ARMV6__)

	#ifndef USE_UNOPTIMIZED
		#define ARM9TDMI_INLINE
	#endif
#endif

#endif  // ARICOMMON_H
