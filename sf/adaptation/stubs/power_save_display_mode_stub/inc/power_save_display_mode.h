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

#ifndef __POWER_SAVE_DISPLAY_MODE_H__
#define __POWER_SAVE_DISPLAY_MODE_H__

// INCLUDES
#include <e32base.h>

// CLASS DECLARATION

/**
* Adaptation API for power save display mode use
*
* @since Series 60 3.0
*/
class CPowerSaveDisplayMode : public CBase
    {
public:
    /**
	* Power save levels define how much power should be saved.
    * Note: higher settings result in lower image quality.
	*/
    enum TPowerSaveLevel
        {
        ENone   = 0, //!< No power saving allowed
        ELow    = 1, //!< Minimal saving with high quality (default)
        EMedium = 2, //!< High saving with medium quality
        EHigh   = 3, //!< Maximum saving with low quality
        };

    /// Destructor
    IMPORT_C ~CPowerSaveDisplayMode();

    /**
    * Creates object and connects to display.
    * @param  aDisplayNumber Which display to connect to, default is primary display.
    * @return new CPowerSaveDisplayMode, NULL on error
    */
    IMPORT_C static CPowerSaveDisplayMode* NewL(TInt aDisplayNumber = 0);

    /**
    * Enables power save mode. Sets lines between given parameters visible,
    * other display lines are turned off and not shown.
    * @pre Image to display must be drawn in videobuffer before calling Set().
    * @param  aStartRow   First visible line, starting from 0
    * @param  aEndRow     Last visible line
    * @param  aFullColors Full color mode if ETrue and hw supports that
    * Note: full color mode consumes more power
    * @return KErrNone on success
    */
    IMPORT_C TInt Set(TUint aStartRow, TUint aEndRow, TBool aFullColors = EFalse);

    /**
    * Enables power save mode. Sets lines between given parameters visible,
    * other display lines are turned off and not shown.
    * @pre Image to display must be drawn in videobuffer before calling Set().
    * @param  aStartRow   First visible line, starting from 0
    * @param  aEndRow     Last visible line
    * @param  aPixels     Pointer to array of color of each pixel
    * @param  aFullColors Full color mode if ETrue and hw supports that
    * Note: full color mode consumes more power
    * @return KErrNone on success
    */
    IMPORT_C TInt Set(TUint aStartRow, TUint aEndRow, TUint16* aPixels, TBool aFullColors = EFalse);

    /**
    * Returns max number of lines that display supports in power save mode
    * (e.g.50 lines or full screen).
    * @return  Number of supported power save mode lines.
    */
    IMPORT_C TUint MaxLines();

    /**
    * Returns status of power save mode.
    * @return  ETrue = Enabled (on), EFalse = disabled (off, normal display)
    */
    IMPORT_C TBool Status();

    /** Turns off power save mode. All lines on display will be shown normally.
    * @return KErrNone on success.
    */
    IMPORT_C TInt Exit();

    /**
	* Changes the current power save level.
    * @param  aVal  New power save level
    * @return KErrNone operation succeeded,
    *         KErrNotSupported feature is not available.
	*/
    IMPORT_C TInt SetPowerSaveLevel(CPowerSaveDisplayMode::TPowerSaveLevel aVal);

    /**
	* Return the current power save level.
    * @return TPowerSaveLevel
	*/
    IMPORT_C CPowerSaveDisplayMode::TPowerSaveLevel GetPowerSaveLevel();

private:
    /// Default constructor
    CPowerSaveDisplayMode();
    
    /// Second phase constructor
    void ConstructL(TInt aDisplayNumber);

private:
    /// Display driver pointer
    TAny* iDisplayDriver;
    };

#endif // POWER_SAVE_DISPLAY_MODE_H

// End of File
