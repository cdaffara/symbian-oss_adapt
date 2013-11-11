/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  This file defines logging macros for DSY plugin.
*
*/


#ifndef DSYDEBUG_H
#define DSYDEBUG_H

#include <f32file.h>


#ifdef _DEBUG

// Enable this to enable memory tracing
// for BT Phone Host Interface
//#define MEMTRACE

#ifdef __WINS__
#define __FLOGGING__ // File logging for WINS
#else
#define __CLOGGING__ // Logging with RDebug for target HW
#endif //__WINS__

#endif // _DEBUG


// File logging
//
#if  defined(__FLOGGING__)

#include <e32std.h>

_LIT(KLogFile,"doslog.txt");
_LIT(KLogDirFullName,"c:\\logs\\bt\\");
_LIT(KLogDir,"BT");

#include <f32file.h>
#include <flogger.h>

#define FLOG(a) { FPrint(a); } 
#define FTRACE(a) { a; }

// Declare the FPrint function
inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list);

#ifdef MEMTRACE // If memory tracing is activated.
	TInt size;
	User::Heap().AllocSize(size);	
	RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, _L("[TBTPHI]\t MEM\tMemory usage: %d high: %d"), size, User::Heap().Size());
#endif
    }


// RDebug logging
//
#elif defined(__CLOGGING__)

#include <e32svr.h>

#define FLOG(a) { RDebug::Print(a);  }
#define FLOGHEX(a)
#define FTRACE(a) { a; }

// Declare the FPrint function
inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    TInt tmpInt = VA_ARG(list, TInt);
    TInt tmpInt2 = VA_ARG(list, TInt);
    TInt tmpInt3 = VA_ARG(list, TInt);
    VA_END(list);
    RDebug::Print(aFmt, tmpInt, tmpInt2, tmpInt3);
    }


// No loggings --> Reduced binary size
//
#else   
#define FLOG(a)
#define FLOGHEX(a)
#define FTRACE(a)
#endif

#endif // DSYDEBUG_H

// End of File
