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
* Description:
* 		This header specifies the implementation of CFMRadioTunerControl.
*
*/



#ifndef FMRADIOTUNERCONTROLSTUB_H
#define FMRADIOTUNERCONTROLSTUB_H

//  INCLUDES
#include <e32base.h>
#include <FMRadioTunerControl.h>
#include <RadioRdsControl.h>

// FORWARD DECLARATIONS
class CTuner;
class CRdsGenerator;
class CAfSwitcher;

// CLASS DECLARATION

/**
*  ?one_line_short_description.
*  ?other_description_lines
*
*  @lib FMRadioTunerControlStub.lib
*  @since Series 60 3.0
*/
class CAdaptation : public CFMRadioTunerControl
#ifdef RD_FM_RADIO_ENHANCEMENTS
					,public MRdsControl
#endif	// RD_FM_RADIO_ENHANCEMENTS
    {
    public: // New functions
		static CAdaptation* NewL( MFMRadioTunerControlObserver& aObserver );
		virtual ~CAdaptation();

	public: // From base class CFMRadioTunerControl
		void TunerOn( TFMRadioFrequencyRange aRange, TInt aFrequency );
		void CancelTunerOn();
		void TunerOff();
		void CancelTunerOff();
		void SetFrequencyRange( TFMRadioFrequencyRange aRange );
		void CancelSetFrequencyRange();
		void SetFrequency( TInt aFrequency );
		void CancelSetFrequency();
		void StationSeek( TFMRadioSeekDirection aDirection );
		void CancelStationSeek();
		void AudioMode();
		void CancelAudioMode();
		void SetAudioMode( TFMRadioAudioMode aMode );
		void CancelSetAudioMode();
		void MaxSignalStrength();
		void CancelMaxSignalStrength();
		void SignalStrength();
		void CancelSignalStrength();
		void Squelch();
		void CancelSquelch();
		void SetSquelch( TBool aEnabled );
		void CancelSetSquelch();
		void BufferToBeFilled( TDes8& aBuffer );
		TFMRadioCapabilities Capabilities();

#ifdef RD_FM_RADIO_ENHANCEMENTS
        MRdsControl* RdsControl( MRdsControlObserver& aObserver ) ;
        TInt GetFrequencyRange( TFMRadioFrequencyRange& aRange, TInt& aMinFreq, TInt& aMaxFreq ) const;

	public: // From base class MRdsControl
	    TInt GetCapabilities( TRdsCapabilities& aCaps ) const;
        TInt GetRdsSignalStatus( TBool& aRdsSignal ) const;
        TInt NotifyRdsDataChange( TRdsData aRdsData );
        void CancelNotifyRdsDataChange();
        TInt SetAutomaticSwitching( TBool aAuto );
        TInt GetAutomaticSwitching( TBool& aAuto );
        void CancelAFSearch();
        TInt SetAutomaticTrafficAnnouncement( TBool aAuto );
        TInt GetAutomaticTrafficAnnouncement( TBool& aAuto );
        void StationSeekByPTY( TRdsProgrammeType aPty, TBool aSeekUp );
        void StationSeekByTA( TBool aSeekUp );
        void StationSeekByTP( TBool aSeekUp );
        void CancelRdsStationSeek();
        void GetFreqByPTY( TRdsProgrammeType aPty );
        void CancelGetFreqByPTY();
        void GetFreqByTA();
        void CancelGetFreqByTA();
        void GetPSByPTY( TRdsProgrammeType aPty );
        void CancelGetPSByPTY();
        void GetPSByTA();
        void CancelGetPSByTA();
        TInt GetProgrammeIdentification( TInt& aPi );
        TInt GetProgrammeType( TRdsProgrammeType& aPty );
        TInt GetProgrammeService( TRdsPSName& aPs );
        TInt GetRadioText( TRdsRadioText& aRt, RArray<TRdsRTplusTag>& aRTplusTags );
        TInt GetClockTime( TDateTime& aCt );
        TInt GetTrafficAnnouncementStatus( TBool& aTaStatus );
        TInt GetTrafficProgrammeStatus( TBool& aTpStatus );

#endif	// RD_FM_RADIO_ENHANCEMENTS

    private:

		TBool ValidFrequency( TFMRadioFrequencyRange aRange, TInt aFrequency );
		void SetFrequencySettings( TInt aFrequency );

        /**
        * C++ default constructor.
        */
        CAdaptation();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( MFMRadioTunerControlObserver& aObserver );

    private:    // Data
    	// Tuner
    	CTuner* iTuner;
    	CRdsGenerator* iRds;
    	CAfSwitcher* iRdsAf;
    };

#endif      // FMRADIOTUNERCONTROLSTUB_H

// End of File
