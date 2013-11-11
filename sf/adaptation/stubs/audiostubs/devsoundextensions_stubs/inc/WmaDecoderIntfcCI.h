/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef WmaDECODERINTFCCIIMPL_H
#define WmaDECODERINTFCCIIMPL_H

// INCLUDES
#include <e32base.h>
#include <WmaDecoderIntfc.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  ?one_line_short_description.
*  ?other_description_lines
*
*  @lib ?library
*  @since Series ?XX ?SeriesXX_version
*/
class CWmaDecoderIntfcCI : public CWmaDecoderIntfc
	{
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
		IMPORT_C static CWmaDecoderIntfcCI* NewL();

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CWmaDecoderIntfcCI();

    public: // New functions

    public: // Functions from base classes

    public: // Functions from base classes

        // From CWmaDecoderIntfc

        /**
        * Identifies the compressed audio format
        * @since S60 3.2
        * @param aFormat - the audio format.
        * @return void
        */
		IMPORT_C virtual void SetFormat(TFormat aFormat)  ;

        /**
        * Specifies the number of bits per sample
        * @since S60 3.2
        * @param aFormat - the audio format.
        * @return void
        */
		IMPORT_C virtual void SetBitsPerSampleIn(TUint aBitsPerSample)  ;

        /**
        * Specifies the number of audio channels
        * @since S60 3.2
        * @param aNumOfChannels: The number of audio channels.
        * @return void
        */
		IMPORT_C virtual void SetNumChannelsIn (TUint aNumChannelsIn)  ;

        /**
        * Specifies the sampling rate
        * @since S60 3.2
        * @param aSamplesPerSec: Sample rate value in Hertz.
        * @return void
        */
		IMPORT_C virtual void SetSamplesPerSec (TUint aSamplesPerSec)  ;

        /**
        * Specifies the average number of bytes per second
        * @since S60 3.2
        * @param aAvgBytesPerSec: Average bytes per second.
        * @return void
        */
		IMPORT_C virtual void SetAvgBytesPerSec (TUint aAvgBytesPerSec)  ;

        /**
        * Specifies the block size in bytes
        * @since S60 3.2
        * @param aBlockAlign: Block size in bytes
        * @return void
        */
		IMPORT_C virtual void SetBlockAlign (TUint aBlockAlign)  ;

        /**
        * Configuration information specific to the WMA decoder
        * @since S60 3.2
        * @param aEncodeOptions: The encode options
        * @return void
        */
		IMPORT_C virtual void SetEncodeOptions (TUint aEncodeOptions)  ;

        /**
        * Returns the configuration state of the advanced encode options.
        * @since S60 3.2
        * @return aAdvEncodeOpts: The encode options
        */
		IMPORT_C virtual TInt GetEncodeOptions1 (TUint& aEncodeOpts1);

        /**
        * Returns the configuration state of the advanced encode options.
        * @since S60 3.2
        * @return aAdvEncodeOpts: The encode options
        */
		IMPORT_C virtual TInt GetEncodeOptions2 (TUint& aEncodeOpts2);

        /**
        * Configures the decoder for the channel data arrangement
        * @since S60 3.2
        * @param aChannelMask: Channel mask
        * @return void
        */
		IMPORT_C virtual void SetChannelMaskIn (TUint aChannelMask)  ;

        /**
        * Commits configuration settings to the decoder.
        * @since S60 3.2
        * @return TInt - Status
        */
		IMPORT_C virtual TInt ApplyConfig()  ;

        /**
        * Returns the configuration state of audio format.
        * @since S60 3.2
        * @return aFormat: The audio format
        */
		IMPORT_C virtual TInt GetFormat (TFormat& aFormat)  ;

        /**
        * Returns the configuration state of number of bits per sample.
        * @since S60 3.2
        * @return aBitsPerSample: Number of bits per sample
        */
		IMPORT_C virtual TInt GetBitsPerSampleIn(TUint& aBitsPerSample)  ;

        /**
        * Returns the configuration state of the number of channels.
        * @since S60 3.2
        * @return aNumOfChannels: Number of channels
        */
		IMPORT_C virtual TInt GetNumOfChannelsIn (TUint& aNumOfChannels)  ;

        /**
        * Returns the configuration state of the sample rate.
        * @since S60 3.2
        * @return aSamplesPerSec: Sample rate value in Hertz
        */
		IMPORT_C virtual TInt GetSamplesPerSec (TUint& aSamplesPerSec)  ;

        /**
        * Returns the configuration state of average bytes per second.
        * @since S60 3.2
        * @return aAvgBytesPerSec: Average bytes per second
        */
		IMPORT_C virtual TInt GetAvgBytesPerSec (TUint& aAvgBytesPerSec)  ;

        /**
        * Returns the configuration state of block alignment.
        * @since S60 3.2
        * @return aBlockAlign: Block size in bytes
        */
		IMPORT_C virtual TInt GetBlockAlign (TUint& aBlockAlign)  ;

        /**
        * Returns the configuration state of the encode options.
        * @since S60 3.2
        * @return aEncodeOpts: The encode options.
        */
		IMPORT_C virtual TInt GetEncodeOptions (TUint& aEncodeOpts)  ;


        /**
        * Returns the configuration state of the input channel mask.
        * @since S60 3.2
        * @return aChannelMask: Channel mask.
        */
		IMPORT_C virtual TInt GetChannelMaskIn (TUint& aChannelMask)  ;

        /**
        * Gets a list of formats supported by the decoder.
        * @since S60 3.2
        * @return aSupportedFormats: An array of the formats, as defined in  TFormat
        */
		IMPORT_C virtual TInt GetSupportedFormats(RArray<TFormat>& aSupportedFormats)  ;

        /**
        * Gets a list of tools supported by the decoder.
        * @since S60 3.2
        * @return aSupportedTools: An array of the tools supported by the decoder
        */
		IMPORT_C virtual TInt GetSupportedTools(RArray<TTool>& aSupportedTools)  ;

        /**
        * Get the maximum number of input channels supported by the decoder.
        * @since S60 3.2
        * @return aSupportedMaxChannelsIn: number of channels.
        */
		IMPORT_C virtual TInt GetSupportedMaxChannelsIn(TUint& aSupportedMaxChannelsIn)  ;

        /**
        * Get the maximum bitrate supported by the decoder.
        * @since S60 3.2
        * @return aSupportedMaxBitrate: maximum bitrate
        */
		IMPORT_C virtual TInt GetSupportedMaxBitrate(TUint& aSupportedMaxBitrate)  ;

        /**
        * Get the maximum sample rate supported by the decoder.
        * @since S60 3.2
        * @return aSupportedMaxSampleRate: maximum sample rate
        */
		IMPORT_C virtual TInt GetSupportedMaxSampleRate(TUint& aSupportedMaxSampleRate)  ;

        /**
        * Gets a list of the tools that are controllable through this interface.
        * @since S60 3.2
        * @return aControllableTools: An array of the tools that are controllable through this interface.
        */
		IMPORT_C virtual TInt GetControllableTools(RArray<TTool>& aControllableTools)  ;

        /**
        * Enables the tool specified.
        * @since S60 3.2
        * @return aTool: Specifies the tool to enable.
        */
		IMPORT_C virtual void EnableTool(TTool aTool)  ;

        /**
        * Disables the tool specified.
        * @since S60 3.2
        * @return aTool: Specifies the tool to disable.
        */
		IMPORT_C virtual void DisableTool(TTool aTool)  ;

        /**
        * Returns the state of the specified tool
        * @since S60 3.2
        * @return aTool: Specifies the tool for which the state is requested
        * @       aEnabled : Indicates the state of the tool specified in aTool
        */
		IMPORT_C virtual TInt GetTool(TTool aTool, TBool& aEnabled)  ;

         /**
        * Configuration information specific to the WMA decoder
        * @since S60 3.2
        * @param aEncodeOptions: The encode options
        * @return void
        */
		IMPORT_C virtual void SetEncodeOptions1 (TUint aEncodeOptions1);

        /**
        * Configuration information specific to the WMA decoder
        * @since S60 3.2
        * @param aEncodeOptions: The encode options
        * @return void
        */
		IMPORT_C virtual void SetEncodeOptions2 (TUint aEncodeOptions2);

        /**
        * Configures the decoder for the channel data arrangement
        * @since S60 3.2
        * @param aChannelMask: Channel mask
        * @return void
        */


    protected:  // New functions

    protected:  // Functions from base classes

	private:

        /**
        * C++ default constructor.
        */
	    CWmaDecoderIntfcCI();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

    public:     // Data
    protected:  // Data
    private:    // Data
        // Working config before ApplyConfig()
  		TFormat iFormat;
        TUint iBitsPerSample;
        TUint iNumChannelsIn;
        TUint iSamplesPerSec;
        TUint iAvgBytesPerSec;
        TUint iBlockAlign;
        TUint iEncodeOptions;
		TUint iEncodeOptions1;
		TUint iEncodeOptions2;

        TUint iChannelMask;
        RArray<TFormat> iSupportedFormats;
        RArray<TTool> iSupportedTools;
        RArray<TTool> iControllableTools;
        TUint iSupportedMaxSampleRate;
        TUint iSupportedMaxChannelsIn;
        TUint iSupportedMaxBitrate;
        TTool iEnable;
        TTool iDisable;


        TBool iToolOutPut32Bit;
        TBool iToolDownMixToStereo;
        TBool iToolLostDataConcealment;


		TFormat iCurrentFormat;
        TUint iCurrentBitsPerSample;
        TUint iCurrentNumChannelsIn;
        TUint iCurrentSamplesPerSec;
        TUint iCurrentAvgBytesPerSec;
        TUint iCurrentBlockAlign;
        TUint iCurrentEncodeOptions;
		TUint iCurrentEncodeOptions1;
		TUint iCurrentEncodeOptions2;

        TUint iCurrentChannelMask;
        RArray<TFormat>iCurrentSupportedFormats;
        RArray<TTool> iCurrentSupportedTools;
        RArray<TTool> iCurrentControllableTools;
        TUint iCurrentSupportedMaxSampleRate;
        TUint iCurrentSupportedMaxChannelsIn;
        TUint iCurrentSupportedMaxBitrate;

        TBool iCurrentToolOutPut32Bit;
        TBool iCurrentToolDownMixToStereo;
        TBool iCurrentToolLostDataConcealment;

        TTool iCurrentEnable;
        TTool iCurrentDisable;

		// Flag indicating readiness of the data to be applied
		TBool iHasBeenApplied;
        // returned status of function ApplyConfigL
        TInt iApplyConfStatus;


    public:     // Friend classes
    protected:  // Friend classes
    private:    // Friend classes


	};

#endif      // WMADECODERINTFCCIIMPL_H

// End of File
