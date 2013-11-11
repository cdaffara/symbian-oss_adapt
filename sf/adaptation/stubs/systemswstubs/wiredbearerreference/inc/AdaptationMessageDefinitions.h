/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Used as an reference implementation.
*
*/


#ifndef ADAPTATIONMESSAGEDEFINITIONS_H
#define ADAPTATIONMESSAGEDEFINITIONS_H

//  INCLUDES
#include <RemConKeyEventData.h>                    // TRemConKeyEventData from accessoryremotecontrol (S60).

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

const TUint32 KBearerMessageGroupAccessoryEvent    = 0x00010002;  /* Message group for Accessory events */
const TUint32 KBearerMessageGroupAccessoryKeyEvent = 0x00010014;  /* Message group for Accessory key events */

// RemCon event definitions
// -----------------------------------------------------------------------------
const TUint32 KBearerMsgAccessoryKeyEvent                         = 0x02000001;
typedef TPckgBuf< TRemConKeyEventData > TBearerMsgAccessoryKeyEventBuf;
// -----------------------------------------------------------------------------


#endif      // ADAPTATIONMESSAGEDEFINITIONS_H
            
// End of File
