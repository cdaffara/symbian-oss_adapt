/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*     Defines an API used by Series 60 to access the power save display mode
*     features.
*      
*
*/



// INCLUDE FILES
#include "power_save_display_mode.h"
#include <e32std.h>

// CONSTANTS
// Stub uses a constant value for power save size 
const TInt KMaxPowerSaveLinesStub = 50;


// ============================ MEMBER FUNCTIONS ===============================

/**
 * @see power_save_display_mode.h
 */
CPowerSaveDisplayMode::CPowerSaveDisplayMode() :
    CBase(),
    iDisplayDriver(NULL)
    {
    }


/**
 * @see power_save_display_mode.h
 */
EXPORT_C CPowerSaveDisplayMode* CPowerSaveDisplayMode::NewL(TInt aDisplayNumber)
    {
    CPowerSaveDisplayMode* result = new(ELeave) CPowerSaveDisplayMode;

    CleanupStack::PushL(result);
    result->ConstructL(aDisplayNumber);
    CleanupStack::Pop();

    return result;
    }


/**
 * @see power_save_display_mode.h
 */
void CPowerSaveDisplayMode::ConstructL(TInt /* aDisplayNumber */)
    {
    /*
     * ADAPTATION IMPLEMENTATION NEEDED
     *
     * Create and initialize all the member variables that are needed
     * in order to complete the tasks assigned to this class here.
     * Note that this function may leave.
     */
    }

    
/**
 * @see power_save_display_mode.h
 */
EXPORT_C CPowerSaveDisplayMode::~CPowerSaveDisplayMode()
    {
    /*
     * ADAPTATION IMPLEMENTATION NEEDED
     *
     *  Destroy all member variables that have been created by
     *  ConstructL(). Close open connections.
     */
    }

    
/**
 * @see power_save_display_mode.h
 */
EXPORT_C TBool CPowerSaveDisplayMode::Status()
    {
    /*
     * ADAPTATION IMPLEMENTATION NEEDED
     *
     * Return ETrue if the power save mode is activated, otherwise EFalse
     */
    return EFalse;
    }


/**
 * @see power_save_display_mode.h
 */
EXPORT_C TInt CPowerSaveDisplayMode::Exit()
    {
    /*
     * ADAPTATION IMPLEMENTATION NEEDED
     *
     * Instruct the display driver to exit power save mode.
     * Return KErrNone on success, or an error code.
     */
    
    return KErrNone;
    }


/**
 * @see power_save_display_mode.h
 */
EXPORT_C TUint CPowerSaveDisplayMode::MaxLines()
    {
    /*
     * ADAPTATION IMPLEMENTATION NEEDED
     *
     * Return the maximum number of screen display lines supported in
     * power save mode. This should be queried from the display driver.
     */
    return KMaxPowerSaveLinesStub;
    }


/**
 * @see power_save_display_mode.h
 */
EXPORT_C TInt CPowerSaveDisplayMode::Set(
    TUint aStartRow,
    TUint aEndRow,
    TBool /* aFullColors */)
    {
    /*
     * ADAPTATION IMPLEMENTATION NEEDED
     *
     * Instruct the display driver to enter power save mode starting from
     * row "aStartRow" (0-based) and finishing at row "aEndRow". The image
     * to be displayed should already be drawn in display buffer.
     */
    
    // Sanity-check params
    if (aEndRow <= aStartRow)
        {
        return KErrArgument;
        }
    else
        {
        return KErrNone;
        }
    }


/**
 * @see power_save_display_mode.h
 */
EXPORT_C TInt CPowerSaveDisplayMode::Set(
    TUint aStartRow,
    TUint aEndRow,
    TUint16* aPixels,
    TBool /* aFullColors */)
    {
    /*
     * ADAPTATION IMPLEMENTATION NEEDED
     *
     * Instruct the display driver to enter power save mode starting from
     * row "aStartRow" (0-based) and finishing at row "aEndRow". The image
     * to be displayed should already be drawn in display buffer.
     */
    
    // Sanity-check params
    if (aEndRow <= aStartRow || aPixels == NULL)
        {
        return KErrArgument;
        }
    else
        {
        return KErrNone;
        }
    }


/**
 * @see power_save_display_mode.h
 */
EXPORT_C TInt CPowerSaveDisplayMode::SetPowerSaveLevel(CPowerSaveDisplayMode::TPowerSaveLevel aVal)
    {
    if ( aVal >= ENone && aVal <= EHigh )
        {
        return KErrNone;
        }
    else
        {
        return KErrNotSupported;
        }
    }


/**
 * @see power_save_display_mode.h
 */
EXPORT_C CPowerSaveDisplayMode::TPowerSaveLevel CPowerSaveDisplayMode::GetPowerSaveLevel()
    {
    return CPowerSaveDisplayMode::ELow;
    }

// End of File
