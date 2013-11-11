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
* Description: Audio Stubs -
*
*/



#ifndef MMFHWDEVSTUB_H
#define MMFHWDEVSTUB_H

//  INCLUDES
#include <e32base.h>
#include <mmfhwdevice.h>

// CLASS DECLARATION

/**
*  This class provides an stub interface for CMMFHwDevice - intended
*  to provide non audio rendering support for MMFDevSoundAdaptation Stub
*
*  @lib MmfDevSoundAdaptation_Stub.lib
*  @since Series 60 3.0
*/
class CMMFHwDeviceStub : public CActive
    {
    public:

        /**
        * Constructs, and returns a pointer to, a new CMMFHwDeviceStub object.
        * Leaves on failure..
        * @since Series 60 3.0
        * @return CMMFHwDeviceStub* A pointer to newly created object.
        */
        static CMMFHwDeviceStub* NewL();

        /**
        * Destructor.
        */
        ~CMMFHwDeviceStub();

    public:
        // Methods from CMMFHwDevice

        /**
        * Starts playback/record based on aFuncCmd.
        * @since Series 60 3.0
        * @param TDeviceFunc aFuncCmd A function indicating decode or encode
        * @param TDeviceFlow aFlowCmd Flow command, not used here
        * @return TInt Error code. KErrNone if success.
        */
        TInt Start(TDeviceFunc aFuncCmd, TDeviceFlow aFlowCmd);

        /**
        * Stops current operation.
        * @since Series 60 3.0
        * @return TInt Error code. KErrNone if success.
        */
        TInt Stop();

        /**
        * Pauses current operation.
        * @since Series 60 3.0
        * @return TInt Error code. KErrNone if success.
        */
        TInt Pause();

        /**
        * Initializes CMMFHwDevice.
        * @since Series 60 3.0
        * @param THwDeviceInitParams& aDevInfo A reference to device init
        *        params.
        * @return TInt Error code. KErrNone if success.
        */
        TInt Init(THwDeviceInitParams& aDevInfo);

        /**
        * Returns a pointer to CustomInterface.
        * @since Series 60 3.0
        * @param TUid aInterfaceId A uid that uniquely identifies the custom
        *        interface definition.
        * @return TAny* NULL if no custom interface found, else object
        *        implementing custom interface
        */
        TAny* CustomInterface(TUid aInterfaceId);

        /**
        * Called by client when data is available during playback.
        * @since Series 60 3.0
        * @param CMMFBuffer& aFillBufferPtr A pointer to buffer that was sent
        *        to client during callback FillThisHwBuffer()
        * @return TInt Error code. KErrNone if success.
        */
        TInt ThisHwBufferFilled(CMMFBuffer& aFillBufferPtr);

        /**
        * Called by client when data is available during recording.
        * @since Series 60 3.0
        * @param CMMFBuffer& aFillBufferPtr A pointer to buffer that was sent
        *        to client during callback EmptyThisHwBuffer()
        * @return TInt Error code. KErrNone if success.
        */
        TInt ThisHwBufferEmptied(CMMFBuffer& aEmptyBufferPtr);

        /**
        * Configures CMMFHwDevice.
        * @since Series 60 3.0
        * @param TTaskConfig& aConfig A reference to configuration objec.
        * @return TInt Error code. KErrNone if success.
        */
        TInt SetConfig(TTaskConfig& aConfig);

        /**
        * Stops and deletes codec.
        * @since Series 60 3.0
        * @return TInt Error code. KErrNone if success.
        */
        TInt StopAndDeleteCodec();

        /**
        * Deletes codec.
        * @since Series 60 3.0
        * @return TInt Error code. KErrNone if success.
        */
        TInt DeleteCodec();

    public:// From CActive

        /**
        * Called by CActive object framework when local timer times out.
        * @since Series 60 3.0
        * @return void
        */
        virtual void RunL();

        /**
        * Called by CActive object framework when client cancels active object.
        * @since Series 60 3.0
        * @return void
        */
        virtual void DoCancel();

        /**
        * Called by CActive object framework when RunL leaves.
        * @since Series 60 3.0
        * @param TInt aError Error code.
        * @return KErrNone
        */
        virtual TInt Error(TInt aError);

    private:

        /**
        * C++ default constructor.
        */
        CMMFHwDeviceStub();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Initialize play buffer.
        * Leaves on failure.
        * @since Series 60 3.0
        * @return void
        */
        void InitializePlayBufferL();

        /**
        * Initialize record buffer
        * Leaves on failure.
        * @since Series 60 3.0
        * @return void
        */
        void InitializeRecdBufferL();

        /**
        * Read data from the input file to the buffer
        * @since Series 60 3.0
        * @return void
        */
        void SetActiveRecdBufferL();

        void SetActivePlayBufferL();

    private:
        // Pointer reference to HwDevice observer.
        MMMFHwDeviceObserver*       iObserver;

        // States this object can be at any time.
        enum THwDeviceStubState
            {
            EHWDeviceIdle,
            EHWDevicePlay,
            EHWDeviceRecord
            };
        // HwDevice state
        THwDeviceStubState          iHwDeviceState;
        // Local timer object
        RTimer                      iTimer;
        // Buffers used for playback
        CMMFDataBuffer              *iPlayBuffer1;
        CMMFDataBuffer              *iPlayBuffer2;
        CMMFDataBuffer              *iCurPlayBuffer;
        // Buffers used for recording
        CMMFDataBuffer              *iRecdBuffer1;
        CMMFDataBuffer              *iRecdBuffer2;
        CMMFDataBuffer              *iCurRecdBuffer;
        // ETrue if last buffer is received, else EFalse
        TBool                       iLastBufferReceived;
        // Count
        TInt                        iCount;
        // Index to the position within input file
        TInt                        iPosition;
};

#endif      // MMFHWDEVSTUB_H

// End of File
