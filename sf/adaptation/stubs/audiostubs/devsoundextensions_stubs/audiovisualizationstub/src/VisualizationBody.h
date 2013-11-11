/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  This is the definition of the audio visualization body class.
*
*
*/


#ifndef CVISUALIZATIONBODY_H
#define CVISUALIZATIONBODY_H

// INCLUDES
#include <Visualization.h>

// FORWARD DECLARATION
class MVisualizationObserver;

// CLASS DECLARATION

/**
*  This is the base class for audio visualization.
*
*  @lib Visualization.lib
*  @since 3.0
*/

class CVisualization::CBody: public CBase
	{

	public: // Constructors and destructor

	    /**
        * Factory function for creating the volume object.
        * @since 3.0
        * @param aObserver A visualization observer.
        * @return pointer to CVisualization object
        */
		static CBody* NewL( MVisualizationObserver& aObserver);

		/**
        *	Destructor
        */
		virtual ~CBody();

    public: // New functions

		/**
        * Get the data rate.
        * @since 3.0
        * @return data rate in Hz.
        */
		TUint32 DataRate() const;

		/**
        * Get the upper and lower limits of the supported data rate range
        * @since 3.0
        * @param aMin The lower limit of the data rate range
        * @param aMax The upper limit of the data rate range
        */
		void DataRateRange( TUint32& aMin, TUint32& aMax );

		/**
        * Used by application enable or disable frequency data.
        * @since 3.0
        * @param aIndicator Set to ETrue to enable frequency, EFalse to disable frequency data.
        */
		 void EnableFrequencyData( TBool aIndicator );

		/**
        * Used by application to enable or disable waveform data.
        * @since 3.0
        * @param aIndicator Set to ETrue to enable waveform data, EFalse to disable waveform data.
        */
		 void EnableWaveformData( TBool aIndicator );

		/**
        * Get frequency band count.
        * @since 3.0
        * @return frequency band count.
        */
		 TUint32 FreqencyBandCount( ) const;

		/**
        * Get frequency band count range.
        * @since 3.0
        * @param aMin Lower limit of the frequency band count range
        * @param aMax Upper limit of the frequency band count range
        */
		 void FrequencyBandCountRange( TUint32& aMin, TUint32& aMax );

		/**
        * Determine whether frequency data is enabled.
        * @since 3.0
        * @return ETrue, if frequency data is enabled. EFalse, if frequency data is disabled.
        */
		 TBool IsFrequencyDataEnabled( ) const;

		/**
        * Determine whether waveform data is enabled.
        * @since 3.0
        * @return ETrue, if waveform data is enabled. EFalse, if waveform data is disabled.
        */
		 TBool IsWaveformDataEnabled( ) const;

		/**
        * Get the sampling rate.
        * @since 3.0
        * @return The sampling rate.
        */
		 TUint32 SamplingRate() const;

		/**
        * Set the data rate.
        * @since 3.0
        * @param aDataRate The data rate.
        */
		 void SetDataRateL( TUint32 aDataRate );

		/**
        * Sets the frequency band count. Results in a call to
        * MVisualizationObserver::FrequencyBandCountChanged().
        * @since 3.0
        * @param aFrequencyBandCount The frequency band count.
        */
		 void SetFrequencyBandCountL( TUint32 aFrequencyBandCount );

		/**
        * Sets the waveform band count. Results in a call to
        * MVisualizationObserver::WaveformDataLengthChanged().
        * @since 3.0
        * @param aWaveformDataLength The waveform data length in samples.
        */
		 void SetWaveformDataLengthL( TUint32 aWaveformDataLength );

		/**
        * Starts visualization. Results in a call to MVisualizationObserver::VisualizationStarted()
        * @since 3.0
        */
		 void StartL( );

		/**
        * Stops visualization. Results in a call to MVisualizationObserver::VisualizationStopped()
        * @since 3.0
        */
		 void StopL( );

		/**
        * Get the waveform data length.
        * @since 3.0
        * @return The waveform data length in samples.
        */
		 TUint32 WaveformDataLength() const;

		/**
        * Get the waveform data length range in samples.
        * @since 3.0
        * @param aMin The lower limit of the waveform data length range
        * @param aMax The upper limit of the waveform data length range
        */
		 void WaveformDataLengthRange( TUint32& aMin, TUint32& aMax );

	private:

		/**
		* Private C++ constructor for this class.
        * @since 3.0
        */
		CBody();

        /**
		* Second Phase Costructor
		* @since 3.0
		* @param aObserver Visualization Observer
		*/
        void ConstructL(MVisualizationObserver& aObserver );

	private: // Data

		// Indicate whether frequency data is enabled or not
		TBool iFrequencyDataEnabled;

		// Indicate whether waveform data is enabled or not
		TBool iWaveFormDataEnabled;

		// Data rate
		TUint32 iDataRate;
		TUint32 iDataRateMin;
		TUint32 iDataRateMax;

		// Frequency Data
		TUint32 iFreqencyBandCount;
		TUint32 iFreqencyBandCountMin;
		TUint32 iFreqencyBandCountMax;

		// Sampling rate
		TUint32 iSamplingRate;

		// Waveform Data
		TUint32 iWaveformDataLength;
		TUint32 iWaveformDataLengthMin;
		TUint32 iWaveformDataLengthMax;

		RArray<TInt16> iFrequencyData;
		RArray<TInt16> iWaveFormData;

		// Pointer to Observer
		MVisualizationObserver* iObserver;
	};

#endif	// of CVISUALIZATIONCBODY_H

// End of File
