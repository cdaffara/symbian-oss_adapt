/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  Common definitions
*
*/

#ifndef DEVSOUNDEXTENSIONSTUBS_COMMON_H_
#define DEVSOUNDEXTENSIONSTUBS_COMMON_H_

#define KUidDevsoundExtensionStubsInterface     0x10207C49

#define RET_ERR_IF_ERR(err) if ( err != KErrNone) return err;

#ifdef _DEBUG
#include "e32debug.h"

#define DB_IN         RDebug::Printf("%s>ENTER", __PRETTY_FUNCTION__)
#define DB_OUT        RDebug::Printf("%s>EXIT", __PRETTY_FUNCTION__)
#define DB_IF_ERR(err) if (err != KErrNone) RDebug::Printf( "%s>BREAK[%d]", __PRETTY_FUNCTION__, err )

#define DB0(str)              RDebug::Print(str, this)
#define DB1(str, val1)        RDebug::Print(str, this, val1)
#define DB2(str, val1, val2)  RDebug::Print(str, this, val1, val2)

#define DB_CUSTOM0(str)             RDebug::Print(str)
#define DB_CUSTOM1(str, val1)       RDebug::Print(str, val1)
#define DB_CUSTOM2(str, val1, val2) RDebug::Print(str, val1, val2)

#else

#define DB_IN
#define DB_OUT
#define DB_IF_ERR(err)
#define DB0(str)
#define DB1(str, val1)
#define DB2(str, val1, val2)
#define DB_CUSTOM0(str)
#define DB_CUSTOM1(str, val1)
#define DB_CUSTOM2(str, val1, val2)
#endif //_DEBUG


#endif /*DEVSOUNDEXTENSIONSTUBS_COMMON_H_*/
