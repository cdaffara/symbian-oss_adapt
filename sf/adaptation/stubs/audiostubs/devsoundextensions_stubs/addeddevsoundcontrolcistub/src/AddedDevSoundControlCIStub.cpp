/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  Custom Interface stub implementation for AddedDevSoundControl.
*
*/


// INCLUDE FILES
#include <e32debug.h>
#include "AddedDevSoundControlCIStub.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

/**
 * CAddedDevSoundControlCI::CAddedDevSoundControlCI
 * C++ default constructor can NOT contain any code, that might leave.
 */
CAddedDevSoundControlCI::CAddedDevSoundControlCI()
    {
    }

/**
 * CAddedDevSoundControlCI::ConstructL
 * Symbian 2nd phase constructor can leave.
 */
void CAddedDevSoundControlCI::ConstructL()
    {
    }

/**
 * CAddedDevSoundControlCI::NewL
 * Two-phased constructor.
 */
EXPORT_C CAddedDevSoundControlCI* CAddedDevSoundControlCI::NewL()
    {
	CAddedDevSoundControlCI* self = new (ELeave)CAddedDevSoundControlCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

/**
 * Destructor
 */
EXPORT_C CAddedDevSoundControlCI::~CAddedDevSoundControlCI()
    {
    }


// From CAddedDevSoundControl

/**
 * Handles DevSound observer’s request to alter DevSound's behavior
 * for Pause. Can be called after DevSound creation. Must be called
 * prior to calling Pause to take effect. When value is True, the
 * Pause will halt the resource and keep all buffers sent to the
 * DevSound interface intact for playing upon resuming. When the value
 * is False, Pause will cause buffers to be flushed and any associated
 * resources freed. Should return a KErrNotSupported if the DevSound
 * behavior cannot be altered.
 *
 * (other items defined in the header)
 */
EXPORT_C TInt CAddedDevSoundControlCI::SetHwAwareness(TBool aHwAware)
    {
    iHwAware = aHwAware;

#ifdef _DEBUG
    RDebug::Print(_L("CAddedDevSoundControlCI::SetHwAwareness [%d]"), iHwAware);
#endif //_DEBUG

    return KErrNone;
    }

/**
 * Handles DevSound observer’s request to pause the audio resources
 * and explicitly flush the buffers. Must be in the Playing state in
 * order to function properly. Should return KErrNotSupported if the
 * feature is not available.
 *
 * (other items defined in the header)
 */
EXPORT_C TInt CAddedDevSoundControlCI::PauseAndFlush()
    {
#ifdef _DEBUG
    RDebug::Print(_L("CAddedDevSoundControlCI::PauseAndFlush [OK]"));
#endif //_DEBUG

    return KErrNone;
    }


// End of file
