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
* ne1_tb\nktest\interrupts.cpp
* NaviEngine interrupt control and dispatch
*
*/



#if 0
#include <x86.h>
#include <x86pc.h>
#include <interrupts.h>

#undef EXPORT_C
#define EXPORT_C /* */

extern "C" void CheckPoint();

#define __CHECKPOINT()	CheckPoint()

#include "../specific/interrupts.cpp"
#include "../specific/ioapic.cpp"

#endif
