/*
* Copyright (c) 2004-2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Macros for debugging.
*
*/



#ifndef __STARTUPADAPTATIONSTUBDEBUG_H__
#define __STARTUPADAPTATIONSTUBDEBUG_H__

// MACROS
#ifdef _DEBUG
    #include <e32svr.h>
    #define ERROR_DEBUG(X)           RDebug::Print(X)
    #define ERROR_DEBUG_1(X,Y1)      RDebug::Print(X,Y1)
    #define ERROR_DEBUG_2(X,Y1,Y2)   RDebug::Print(X,Y1,Y2)
#else
    #define ERROR_DEBUG(X)
    #define ERROR_DEBUG_1(X,Y)
    #define ERROR_DEBUG_2(X,Y1,Y2)
#endif

#if defined _DEBUG && defined __TRACE_ALL__
    #include <e32svr.h>
    #define RDEBUG(X)           RDebug::Print(X)
    #define RDEBUG_1(X,Y1)      RDebug::Print(X,Y1)
    #define RDEBUG_2(X,Y1,Y2)   RDebug::Print(X,Y1,Y2)
#else
    #define RDEBUG(X)
    #define RDEBUG_1(X,Y)
    #define RDEBUG_2(X,Y1,Y2)
#endif

#endif // __STARTUPADAPTATIONSTUBDEBUG_H__

// End of File
