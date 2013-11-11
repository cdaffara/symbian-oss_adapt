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
* This is a header file, which is required for
* enabling / disabling debug prints
*
*/


#ifndef ARIPRINT_H__
#define ARIPRINT_H__

#include <e32debug.h>


#define LEVEL_LOW		2
#define LEVEL_HIGH		1
#define LEVEL_CRITICAL	0
#define LEVEL_NONE		-1

#ifdef LOGLEVEL_LOW
#undef LOGLEVEL_HIGH
#undef LOGLEVEL_CRITICAL
#undef LOGLEVEL_NONE
#define PRINT(x, y)			{if(x <= LEVEL_LOW) RDebug::Printf y;}
#endif

#ifdef LOGLEVEL_HIGH
#undef LOGLEVEL_LOW
#undef LOGLEVEL_CRITICAL
#undef LOGLEVEL_NONE
#define PRINT(x, y)			{if(x <= LEVEL_HIGH) RDebug::Printf y;}
#endif

#ifdef LOGLEVEL_CRITICAL
#undef LOGLEVEL_HIGH
#undef LOGLEVEL_LOW
#undef LOGLEVEL_NONE
#define PRINT(x, y)			{if(x <= LEVEL_CRITICAL) RDebug::Printf y;}
#endif

#ifdef LOGLEVEL_NONE
#undef LOGLEVEL_HIGH
#undef LOGLEVEL_CRITICAL
#undef LOGLEVEL_LOW
#define PRINT(x, y)			{;}
#endif


#define PRINT_ENTRY			PRINT(LEVEL_LOW, ("--> %s", __PRETTY_FUNCTION__))
#define PRINT_EXIT			PRINT(LEVEL_LOW, ("<-- %s", __PRETTY_FUNCTION__))
#define PRINT_ERR(y)		PRINT(LEVEL_CRITICAL, ("ERROR: %s, Ln: %d, %s", __PRETTY_FUNCTION__, __LINE__, y))
#define PRINT_MSG(x, y)		PRINT(x, y)


#endif	// ARIPRINT_H__
