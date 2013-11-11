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
* Description: Audio Stubs -  Custom Interface stub object for configuring the SBC encoder.
*
*/


#ifndef CSBCENCODERINTFCCI_H
#define CSBCENCODERINTFCCI_H

// INCLUDES
#include <e32base.h>
#include <SbcEncoderIntfc.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
 *  Custom Interface stub for configuring the SBC encoder operating modes.
 *  This class processes requests received from the CSbcEncoderIntfcProxy.
 *
 *  @lib SbcEncoderIntfcCI_Stub.lib
 *  @since S60 3.0
 */
class CSbcEncoderIntfcCI : public CSbcEncoderIntfc
	{
    public:  // Constructors and destructor

        /**
         * Two-phased constructor.
         */
		IMPORT_C static CSbcEncoderIntfcCI* NewL();

        /**
         * Destructor.
         */
        IMPORT_C virtual ~CSbcEncoderIntfcCI();

    public: // New functions

    public: // Functions from CSbcEncoderInfc base class

        /**
         * Retrieves the sampling frequencies supported by the encoder.
         * @since S60 3.0
         * @param RArray<TUint>& - reference to the array of sampling freq.
         * @return TInt status
         */
		IMPORT_C virtual TInt GetSupportedSamplingFrequencies(
		                      RArray<TUint>& aSupportedSamplingFrequencies);

        /**
         * Retrieves the channel modes supported by the encoder.
         * @since S60 3.0
         * @param RArray<TUint>& - reference to the array of channel modes.
         * @return TInt status
         */
		IMPORT_C virtual TInt GetSupportedChannelModes(
		                      RArray<TSbcChannelMode>& aSupportedChannelModes);

        /**
         * Retrieves the blocks supported by the encoder.
         * @since S60 3.0
         * @param RArray<TUint>& - reference to the array of blocks.
         * @return TInt status
         */
		IMPORT_C virtual TInt GetSupportedNumOfBlocks(
		                      RArray<TUint>& aSupportedNumOfBlocks);

        /**
         * Retrieves the subbands supported by the encoder.
         * @since S60 3.0
         * @param RArray<TUint>& - reference to the array of subbands.
         * @return TInt status
         */
		IMPORT_C virtual TInt GetSupportedNumOfSubbands(
		                      RArray<TUint>& aSupportedNumOfSubbands);

        /**
         * Retrieves the allocation methods supported by the encoder.
         * @since S60 3.0
         * @param RArray<TUint>& - reference to the array of alloc. methods.
         * @return TInt status
         */
		IMPORT_C virtual TInt GetSupportedAllocationMethods(
		                      RArray<TSbcAllocationMethod>&
		                      aSupportedAllocationMethods);

        /**
         * Retrieves the bitpool size supported by the encoder.
         * @since S60 3.0
         * @param TUint& - reference to the min bitpool size.
         * @param TUint& - reference to the max bitpool size.
         * @return TInt status
         */
		IMPORT_C virtual TInt GetSupportedBitpoolRange(
		                      TUint& aMinSupportedBitpoolSize,
		                      TUint& aMaxSupportedBitpoolSize);

        /**
         * Sets new sampling frequency.
         * @since S60 3.0
         * @param TUint - sampling frequency to be set.
         * @return void
         */
		IMPORT_C virtual void SetSamplingFrequency(TUint aSamplingFrequency);

        /**
         * Returns current sampling frequency.
         * @since S60 3.0
         * @param TUint& - current sampling frequency.
         * @return TInt - status
         */
		IMPORT_C virtual TInt GetSamplingFrequency(TUint& aSamplingFrequency);

        /**
         * Sets new channel mode.
         * @since S60 3.0
         * @param TSbcChannelMode - channel mode to be set.
         * @return void
         */
		IMPORT_C virtual void SetChannelMode(TSbcChannelMode aChannelMode);

        /**
         * Returns current channel mode.
         * @since S60 3.0
         * @param TSbcChannelMode& - current channel mode.
         * @return TInt - status
         */
		IMPORT_C virtual TInt GetChannelMode(TSbcChannelMode& aChannelMode);

        /**
          * Sets new number of subbands.
          * @since S60 3.0
          * @param TSbcChannelMode - number of subbands to be set.
          * @return void
          */
		IMPORT_C virtual void SetNumOfSubbands(TUint aNumOfSubbands);

        /**
          * Returns current number of subbands.
          * @since S60 3.0
          * @param TUint& - current subbands.
          * @return TInt - status
          */
		IMPORT_C virtual TInt GetNumOfSubbands(TUint& aNumOfSubbands);

        /**
         * Sets new number of blocks.
         * @since S60 3.0
         * @param TSbcChannelMode - number of blocks to be set.
         * @return void
         */
		IMPORT_C virtual void SetNumOfBlocks(TUint aNumOfBlocks);

        /**
         * Returns current number of blocks.
         * @since S60 3.0
         * @param TUint& - current number of blocks.
         * @return TInt - status
         */
		IMPORT_C virtual TInt GetNumOfBlocks(TUint& aNumOfBlocks);

        /**
         * Sets new allocation method.
         * @since S60 3.0
         * @param TSbcAllocationMethod - allocation method to be set.
         * @return void
         */
		IMPORT_C virtual void SetAllocationMethod(
		                      TSbcAllocationMethod aAllocationMethod);

        /**
         * Returns current allocation method.
         * @since S60 3.0
         * @param TUint& - current allocation method.
         * @return TInt - status
         */
		IMPORT_C virtual TInt GetAllocationMethod(
		                      TSbcAllocationMethod& aAllocationMethod);

        /**
         * Sets new bitpool size.
         * @since S60 3.0
         * @param TUint - bitpool size to be set.
         * @return void
         */
		IMPORT_C virtual void SetBitpoolSize(TUint aBitpoolSize);

        /**
         * Returns current bitpool size.
         * @since S60 3.0
         * @param TUint& - current bitpool size.
         * @return TInt - status
         */
		IMPORT_C virtual TInt GetBitpoolSize(TUint& aBitpoolSize);

        /**
         * Commits new configuration settings.
         * @since S60 3.0
         * @return TInt - status
         */
		IMPORT_C virtual TInt ApplyConfig();


    protected:  // New functions

    protected:  // Functions from base classes

	private:

        /**
         * C++ default constructor.
         */
        CSbcEncoderIntfcCI();

        /**
         * Symbian 2nd phase constructor.
         */
        void ConstructL();

    public:     // Data
    protected:  // Data
    private:    // Data

    	// Working config before ApplyConfig()
    	TUint                iSamplingFrequency;
    	TSbcChannelMode      iChannelMode;
    	TUint                iNumOfSubbands;
    	TUint                iNumOfBlocks;
    	TSbcAllocationMethod iAllocationMethod;
    	TUint                iBitpoolSize;

    	// Current config after ApplyConfig()
    	TUint                iCurrentSamplingFrequency;
    	TSbcChannelMode      iCurrentChannelMode;
    	TUint                iCurrentNumOfSubbands;
    	TUint                iCurrentNumOfBlocks;
    	TSbcAllocationMethod iCurrentAllocationMethod;
    	TUint                iCurrentBitpoolSize;

	};

#endif      // CSBCENCODERINTFCCI_H

// End of File
