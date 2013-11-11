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
* Description: Audio Stubs -  Class that provides API to list ECOM plugin implementation IDs
*
*/



#ifndef MMFDEVSOUNDUTILITY_H
#define MMFDEVSOUNDUTILITY_H

//  INCLUDES

#include <ecom.h>
#include <badesca.h>

#include "mmfutilities.h" // For TFourCC

#define KFOURCCLENGTH 4

// CLASS DECLARATION

/**
*  Utility class.
*
*  @lib MmfDevSoundAdaptation.lib
*  @since S60 3.0
*/
class TMMFRawPackage
    {
    public:

        /**
        * Returns a descriptor reference.
        * @since S60 3.0
        * @return TPtr8& A descriptor reference
        */
        inline TPtr8& Package();

        /**
        * Returns a constant descriptor reference.
        * @since S60 3.0
        * @return TPtr8& A constant descriptor reference
        */
        inline const TPtr8& Package() const;

    protected:

        /**
        * Constructor.
        * @since S60 3.0
        * @param TInt aDerivedSize Size of the descriptor.
        */
        TMMFRawPackage(TInt aDerivedSize);

        /**
        * Sets the size of the descriptor.
        * @since S60 3.0
        * @param TInt aDerivedSize New descriptor size.
        * @return void
        */
        inline void SetSize(TInt aDerivedSize);

    protected:
        TPtr8 iThis;
    };

// CLASS DECLARATION

/**
*  Utlilty class used for Fixed Sequences.
*
*  @lib MmfDevSoundAdaptation.lib
*  @since S60 3.0
*/
class TMMFToneFixedSequenceNames : public TMMFRawPackage
    {
    public:

        /**
        * Constructor.
        * @since S60 3.0
        */
        inline TMMFToneFixedSequenceNames();

    public:
        TInt iCount;
    };

// CLASS DECLARATION

/**
*  Utlilty class used for Fixed Sequences.
*
*  @lib MmfDevSoundAdaptation.lib
*  @since S60 3.0
*/
class HMMFToneFixedSequenceNames : public TMMFToneFixedSequenceNames
    {
    public:

        /**
        * Constructor.
        * @since S60 3.0
        */
        HMMFToneFixedSequenceNames();

        /**
        * Adds name to this object.
        * @since S60 3.0
        * @param const TDesC& aName A descriptor containing name to be added.
        * @return HMMFToneFixedSequenceNames* A pointer to this object.
        */
        HMMFToneFixedSequenceNames* AddNameL(const TDesC& aName);
    };


// CLASS DECLARATION

/**
*  Utility class.used by DevSound.
*
*  @lib MmfDevSoundAdaptation.lib
*  @since S60 3.0
*/
class CMMFDevSoundUtility : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Constructs, and returns a pointer to, a new CMMFDevSound object.
        * Leaves on failure.
        * @return CMMFDevSoundUtility* A pointer to newly created utlitly object.
        */
        static CMMFDevSoundUtility* NewL();

        /**
        * Constructs, leaves object on the cleanup stack, and returns a pointer
        * to, a new CMMFDevSound object.
        * Leaves on failure.
        * @return CMMFDevSoundUtility* A pointer to newly created utlitly object.
        */
        static CMMFDevSoundUtility* NewLC();

        /**
        * Destructor.
        */
        ~CMMFDevSoundUtility();

    public: // New functions

        /**
        * Finds the ECom plugins based on FourCC.
        * @since S60 3.0
        * @param TUid aInterfaceUid The interface Uid where to look for the plugin.
        * @param RImplInfoPtrArray& aPlugInArray The array of ECom plugins Uids
        *        that will be filled in by this function.
        * @param const TFourCC& aSrcDataType The source data type.
        * @param const TFourCC& aDstDataType The destination data type.
        * @return void
        */
        void SeekUsingFourCCLC(TUid aInterfaceUid,
                               RImplInfoPtrArray& aPlugInArray,
                               const TFourCC& aSrcDataType,
                               const TFourCC& aDstDataType,
                               const TDesC& aPreferredSupplier);

        /**
        * local function to disable items which do not match the preferred
        * supplier. Note that at least one enabled item is returned (if there
        * was an enabled item to begin with) which may not match the preferred
        * supplier.
        * @since S60 3.0
        * @param RImplInfoPtrArray& aPlugInArray An array containing the list of
        *        ECom plugin Uids.
        * @param const TDesC &aPreferredSupplier Additional resolution criteria
        *        when searching for ECom plugin. If this is provided, the list
        *        of matching plugins will be further searched for the latest
        *        version of a plugin supplied by supplier named. Note that the
        *        display name field is parsed for a match.
        * @return void
        */
        void SelectByPreference(RImplInfoPtrArray& aPlugInArray, const TDesC& aPreferredSupplier);

        /*
        *  This method looks for hwDevicePlugins that support the state given in
        * aState which must be either EMMFStatePlaying or EMMFStateRecording for
        * each HwDevice plugin found the datatype as indicated by its FourCC
        * code from the default_data field in the resource file is added to the
        * array of aSupportedDataTypes
        * Leaves on failure.
        * @since S60 3.0
        * @param RArray<TFourCC>& aSupportedDataTypes An array of fourCC codes
        *        that has a fourCC code added to for each hardware device found.
        * @param TMMFState aState EMMFStatePlaying if seeking HwDevice plugins
        *        that support play and EMMFStateRecording if seeking HwDevice
        *        plugins that support record        *
        * @return void
        */
        void SeekHwDevicePluginsL(RArray<TFourCC>& aSupportedDataTypes, TMMFState aState);

        /**
        * Initializes the names of Fixed Sequences.
        * Leaves on failure.
        * @since S60 3.0
        * @param CPtrC8Array** aFixedSequences A pointer to a pointer reference
        *        which will be populated with Fixed Sequence names when this
        *        funtion returns.
        * @return void
        */
        void InitializeFixedSequenceL(CPtrC8Array** aFixedSequences);

        /**
        * Recognizes tone sequence.
        * @since S60 3.0
        * @param const TDesC8& aData A reference to descriptor containing tone
        *        sequence.
        * @return ETrue of the sequence starts with 'SQNC' else EFalse.
        */
        TBool RecognizeSequence(const TDesC8& aData);

        /**
        * Returns a descriptor reference containing name of the fixed sequence.
        * @since S60 3.0
        * @param TInt aSequenceNumber Sequence number.
        * @return A constant reference to descriptor.
        */
        const TDesC& FixedSequenceName(TInt aSequenceNumber);

    private:

        /*
        * This method takes a given resource entry from a hardware device and
        * determines whether the hwdevice plugin is a data type match for
        * playing or recording depending on the setting of aState. The method
        * matchs the default_data field from the hw device resource entry
        * matching it with the aHwMatchFourCC code.
        * @since S60 3.0
        * @param CImplementationInformation aHwDeviceResourceEntry The hw device
        *        resource entry that is to be checked whether it can be used to
        *        play or record
        * @param TDesC8& aHwMatchFourCC The data type FourCC code to match to
        *        that the hardware device that must convert to for playing and
        *        convert from for recording - for the reference DevSound this
        *        is always ' P16' ie pcm16
        * @param TMMFState aState EMMFStatePlaying for playing and
        *        EMMFStateRecording for recording
        * @return ETrue if a match for play or record else EFalse.
        */
        TBool IsDataTypeMatch(CImplementationInformation* hwDeviceResourceEntry,
                              const TDesC8& fourCCPtrPCM16,
                              TMMFState aState);


    private:

        /**
        * C++ default constructor.
        */
        CMMFDevSoundUtility();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
        //data_declaration;

        HMMFToneFixedSequenceNames* iFixedSequenceNames;
        HBufC8* iInfo;

    };

#endif // MMFDEVSOUNDUTILITY_H

// End of File
