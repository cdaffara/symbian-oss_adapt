/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  Custom Interface for eAAC+ decoder.
*
*/


#ifndef EAACPLUSDECODERINTFCCI_H
#define EAACPLUSDECODERINTFCCI_H

// INCLUDES
#include <e32base.h>
#include <EAacPlusDecoderIntfc.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Custom Interface for eAAC+ decoder.
*
*  @lib EAacPlusDecoderIntfcCI_Stub.lib
*  @since S60 3.0
*/
class CEAacPlusDecoderIntfcCI : public CEAacPlusDecoderIntfc
	{
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
		IMPORT_C static CEAacPlusDecoderIntfcCI* NewL();

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CEAacPlusDecoderIntfcCI();

    public: // New functions

    public: // Functions from CEAacPlusDecoderIntfc

        /**
        * Configures decoder's profile defined by TAudioObjectType.
        * @since S60 3.0
        * @param TAudioObjectType - Supported AAC profile.
        * @return void
        */
		IMPORT_C virtual void SetAudioObjectType(
		                      TAudioObjectType aAudioObjectType);

        /**
        * Configures decoder's input sampling frequency.
        * @since S60 3.0
        * @param TUint - Decoder's input sampling frequency value.
        * @return void
        */
		IMPORT_C virtual void SetInputSamplingFrequency(
		                      TUint aInputSamplingFrequency);

        /**
        * Configures the number of channels to be used by the decoder.
        * @since S60 3.0
        * @param TUint - Number of channels (stereo/mono).
        * @return void
        */
		IMPORT_C virtual void SetNumOfChannels(TUint aNumOfChannels);

        /**
        * Configures decoder's spectral bandwidth replication (SBR).
        * @since S60 3.0
        * @param TBool - Toggle SBR.
        * @return void
        */
		IMPORT_C virtual void SetSbr(TBool aSbrEnabled);

        /**
        * Configures decoder's down-sampled mode (DSM).
        * @since S60 3.0
        * @param TBool - Toggle DSM.
        * @return void
        */
		IMPORT_C virtual void SetDownSampledMode(TBool aDsmEnabled);

        /**
        * Commits configuration settings to the decoder.
        * @since S60 3.0
        * @return TInt - Status
        */
		IMPORT_C virtual TInt ApplyConfig();

        /**
        * Returns decoder's AAC profile setting.
        * @since S60 3.0
        * @param TAudioObjectType& - Reference to AAC profile.
        * @return TInt - Status
        */
		IMPORT_C virtual TInt GetAudioObjectType(
		                      TAudioObjectType& aAudioObjectType);

        /**
        * Returns decoder's input sampling frequency setting.
        * @since S60 3.0
        * @param TUint& - Reference to input sampling frequency.
        * @return TInt - Status
        */
		IMPORT_C virtual TInt GetInputSamplingFrequency(
		                      TUint& aInputSamplingFrequency);

        /**
        * Returns decoder's channel setting.
        * @since S60 3.0
        * @param TUint& - Reference to the number of channels.
        * @return TInt - Status
        */
		IMPORT_C virtual TInt GetNumOfChannels(TUint& aNumOfChannels);

        /**
        * Returns decoder's SBR setting.
        * @since S60 3.0
        * @param TBool& - Reference to the SBR setting.
        * @return TInt - Status
        */
		IMPORT_C virtual TInt GetSbr(TBool& aSbrEnabled);

        /**
        * Returns decoder's DSM setting.
        * @since S60 3.0
        * @param TBool& - Reference to the DSM setting.
        * @return TInt - Status
        */
		IMPORT_C virtual TInt GetDownSampledMode(TBool& aDsmEnabled);


    protected:  // New functions

    protected:  // Functions from base classes

	private:

        /**
        * C++ default constructor.
        */
	    CEAacPlusDecoderIntfcCI();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

    public:     // Data
    protected:  // Data
    private:    // Data

        // Working config before ApplyConfig()
        TAudioObjectType  iAudioObjectType;
        TUint             iInputSamplingFrequency;
        TUint             iNumOfChannels;
        TBool             iSbrEnabled;
        TBool             iDsmEnabled;

        // Current config after ApplyConfig()
        TAudioObjectType  iCurrentAudioObjectType;
        TUint             iCurrentInputSamplingFrequency;
        TUint             iCurrentNumOfChannels;
        TBool             iCurrentSbrEnabled;
        TBool             iCurrentDsmEnabled;

	};

#endif      // EAACPLUSDECODERINTFCCI_H

// End of File
