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
* Description: Audio Stubs -  Utility timer object used for playing tone sequence.
*
*/



#ifndef TONEPLAYCOMPLETETIMER_H
#define TONEPLAYCOMPLETETIMER_H

//  INCLUDES
#include <e32base.h>
#include <sounddevice.h>

// CLASS DECLARATION

/**
*  Utility timer object used for playing tone sequence.
*
*  @lib MmfDevSoundAdaptation_Stub.lib
*  @since Series 60 3.0
*/
class CTonePlayCompleteTimer : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Constructs, and returns a pointer to, a new CTonePlayCompleteTimer
        * object.
        * Leaves on failure..
        * @since Series 60 3.0
        * @return CTonePlayCompleteTimer* A pointer to newly created object.
        */
        static CTonePlayCompleteTimer* NewL(MDevSoundAdaptationObserver&
                                            aDevSoundObserver);

        /**
        * Destructor.
        */
        ~CTonePlayCompleteTimer();

    public: // New functions

        /**
        * Sets the timeout duration.
        * @since Series 60 3.0
        * @return void
        */
        void SetTimeOut(TTimeIntervalMicroSeconds32& aTimeOut);

        /**
        * Starts the timer for timeout duration set by SetTimeOut().
        * @since Series 60 3.0
        * @return void
        */
        void Start();

    protected:  // Functions from base classes

        /**
        * Called by CActive object framework when local timer times out.
        * @since Series 60 3.0
        * @return void
        */
        void RunL();

        /**
        * Called by CActive object framework when client cancels active object.
        * @since Series 60 3.0
        * @return void
        */
        void DoCancel();

        /**
        * Called by CActive object framework when RunL leaves.
        * @since Series 60 3.0
        * @param TInt aError Error code.
        * @return KErrNone
        */
        TInt RunError(TInt aError);

    private:

        /**
        * C++ default constructor.
        */
        CTonePlayCompleteTimer(MDevSoundAdaptationObserver&
                               aDevSoundObserver);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
        // Pointer reference to HwDevice observer.
        MDevSoundAdaptationObserver&    iObserver;
        // Time out duration
        TTimeIntervalMicroSeconds32     iTimeOut;
        // Local timer object
        RTimer                          iTimer;

    };

#endif      // TONEPLAYCOMPLETETIMER_H

// End of File
