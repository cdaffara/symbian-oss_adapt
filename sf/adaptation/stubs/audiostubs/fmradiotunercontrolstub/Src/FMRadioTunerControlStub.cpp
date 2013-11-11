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
*			Contains implementation of the CAdaptation class.
*			Which holds RadioServer Console Test.
*
*/



// INCLUDE FILES
#include 	<e32svr.h>
#include    "FMRadioTunerControlStub.h"

// CONSTANTS
const TInt KReqTunerOn				= 1;
const TInt KReqTunerOff				= 2;
const TInt KReqSetFrequencyRange	= 3;
const TInt KReqSetFrequency			= 4;
const TInt KReqStationSeek			= 5;
const TInt KReqAudioMode			= 6;
const TInt KReqSetAudioMode			= 7;
const TInt KReqMaxSignalStrength	= 8;
const TInt KReqSignalStrength		= 9;
const TInt KReqSquelch				= 10;
const TInt KReqSetSquelch			= 11;
//---------------------------------------
const TInt KReqStationSeekByPTY		= 12;
const TInt KReqStationSeekByTA		= 13;
const TInt KReqStationSeekByTP		= 14;
const TInt KReqGetFreqByPTY			= 15;
const TInt KReqGetFreqByTA			= 16;
const TInt KReqGetPSByPTY			= 17;
const TInt KReqGetPSByTA			= 18;

const TInt KEuroAmericaMin	=  87500000;
const TInt KEuroAmericaMax	= 108000000;
const TInt KJapanMin		=  76000000;
const TInt KJapanMax		=  90000000;

/*
KCBI 	 90.9	PTY = KRdsPtyReligion
KLTY 	 94.9	PTY = KRdsPtyReligion
KVIL	103.7	PTY = KRdsPtyLightClassical
KHKS	106.1	PTY = KRdsPtyRockMusic
*/

const TInt KFrequency1	=	 90900000;	// TA, TP
const TInt KFrequency2	=	 94900000;	// Alternate Frequency of KFrequency1
const TInt KFrequency3	=	103700000;
const TInt KFrequency4	=	106100000;	// TP

// DATA TYPES
struct TRadioMessageRequestData
	{
	TSglQueLink iLink;
	TInt iRequest;
	TInt iError;
	};

// ============================= LOCAL FUNCTIONS ===============================

/**
*  Class CTuner
*  @lib FMRadioTunerControlStub.lib
*  @since Series 60 3.0
*/
class CTuner : public CTimer
    {
	public:
		static CTuner* NewL( MFMRadioTunerControlObserver& aObserver )
			{
			CTuner* self = new( ELeave ) CTuner(aObserver);
			CleanupStack::PushL( self );
		    self->ConstructL();
		    CleanupStack::Pop();
		    return self;
			};

		virtual ~CTuner()
			{
			iFreqList.Close();
			iPsList.Close();
			iRTplusTags.Close();
			if ( IsActive() )
				{
				Cancel();
				}
			TRadioMessageRequestData* data;
			while ( !iRequestQue.IsEmpty() )
				{
				data = iRequestQue.First();
				iRequestQue.Remove(*data);
				delete data;
				}
			};

		void CompleteRequest( TInt aRequest, TInt aError )
			{
			TRadioMessageRequestData *data = new (ELeave) TRadioMessageRequestData;
			data->iRequest = aRequest;
			data->iError = aError;
			// Add it to the queue
			iRequestQue.AddLast(*data);
			if ( !IsActive() )
				{
				After(500000);	// 0.5 sec by default
				}
			};

		void Reset()
			{
			iTunerOn = EFalse;
			iRange = EFMRadioFrequencyEuroAmerica;
			iFrequency = KEuroAmericaMin;
			iSigStrength = 0;
			iMode = EFMRadioAudioStereo;
			iSquelch = ETrue;
			ResetRds();
			};

		void ResetRds()
			{
			iNotifyOn = EFalse;
			iRdsSignal = EFalse;
			iRdsData = 0;
			iAutoSwitch = EFalse;
			iFreqList.Reset();
			iPsList.Reset();
			iPi = 0;
			iPty = KRdsPtyNone;
			iPs = _L8("");
			iRt = _L8("");
			iRTplusTags.Reset();
			// Hardcoded to 2007/11/10 7:35
			iCt.Set(2007,ENovember,10,7,35,0,0);
			};

		void SendRdsData()
			{
			// Make sure we are still in RDS channel
			if ( !iRdsSignal || !iNotifyOn )
				{
				return;
				}

			if( iRdsData & ERdsProgrammeIdentification )
				{
				iRdsObserver->RdsDataPI(iPi);
				}
			if( iRdsData & ERdsProgrammeType )
				{
				iRdsObserver->RdsDataPTY(iPty);
				}
			if( iRdsData & ERdsProgrammeService )
				{
				iRdsObserver->RdsDataPS(iPs);
				}
			if( iRdsData & ERdsRadioText )
				{
				iRdsObserver->RdsDataRT(iRt, iRTplusTags);
				}
			if( iRdsData & ERdsClockTime )
				{
				iRdsObserver->RdsDataCT(iCt);
				}
			};

		void SendRdsAf(TBool aBegin)
			{
			if (aBegin)
				{
				iRdsObserver->RdsSearchBeginAF();
				}
			else
				{
				iFrequency = KFrequency2;
				iSigStrength = 2;
				iPs = _L8("KLTY");
				iRdsObserver->RdsSearchEndAF(iError, iFrequency);
				}
			};


    private:
		void DoCancel()
			{};

		void RunL()
			{
			TRadioMessageRequestData* data;
			data = iRequestQue.First();
			iRequestQue.Remove(*data);
			iRequest = data->iRequest;
			iError = data->iError;
			delete data;

			switch (iRequest)
				{
				case KReqTunerOn:
					iObserver->TunerOnComplete(iError);
					break;
				case KReqTunerOff:
					iObserver->TunerOffComplete(iError);
					break;
				case KReqSetFrequencyRange:
					iObserver->SetFrequencyRangeComplete(iError);
					break;
				case KReqSetFrequency:
					iObserver->SetFrequencyComplete(iError);
					break;
				case KReqStationSeek:
					iObserver->StationSeekComplete(iError, iFrequency);
					break;
				case KReqAudioMode:
					iObserver->AudioModeComplete(iError, iMode);
					break;
				case KReqSetAudioMode:
					iObserver->SetAudioModeComplete(iError);
					break;
				case KReqMaxSignalStrength:
					iObserver->MaxSignalStrengthComplete(iError, 15);
					break;
				case KReqSignalStrength:
					iObserver->SignalStrengthComplete(iError, iSigStrength);
					break;
				case KReqSquelch:
					iObserver->SquelchComplete(iError, iSquelch);
					break;
				case KReqSetSquelch:
					iObserver->SetSquelchComplete(iError);
					break;
// -----------------------------------------------------------------------------
//		RDS
				case KReqStationSeekByPTY:
					iRdsObserver->StationSeekByPTYComplete(iError, iFrequency);
					break;
				case KReqStationSeekByTA:
					iRdsObserver->StationSeekByTAComplete(iError, iFrequency);
					break;
				case KReqStationSeekByTP:
					iRdsObserver->StationSeekByTPComplete(iError, iFrequency);
					break;
				case KReqGetFreqByPTY:
					iRdsObserver->GetFreqByPTYComplete(iError, iFreqList);
					break;
				case KReqGetFreqByTA:
					iRdsObserver->GetFreqByTAComplete(iError, iFreqList);
					break;
				case KReqGetPSByPTY:
					iRdsObserver->GetPSByPTYComplete(iError, iPsList);
					break;
				case KReqGetPSByTA:
					iRdsObserver->GetPSByTAComplete(iError, iPsList);
					break;
//		RDS
// -----------------------------------------------------------------------------
				default:
					break;
				}

			// Process the next item in the queue if any
			if ( !iRequestQue.IsEmpty() )
				{
				// there is at least one element in the linked list
				After(100000);	// 0.1 sec by default
				}
			};

    private:
        CTuner( MFMRadioTunerControlObserver& aObserver )
        	:	CTimer(EPriorityStandard),
        		iObserver(&aObserver),
        		iRdsObserver(NULL),
        		iRequestQue(_FOFF( TRadioMessageRequestData, iLink ))
        	{};

        void ConstructL()
        	{
        	CTimer::ConstructL();
			CActiveScheduler::Add(this);
			Reset();
			};

    private:    // Data
    	// Tuner observer
    	MFMRadioTunerControlObserver* iObserver;
		// Rds observer
		MRdsControlObserver* iRdsObserver;
		// Request queue
		TSglQue<TRadioMessageRequestData> iRequestQue;
    	// Outstanding request
    	TInt iRequest;
    	TInt iError;

		// Tuner data
    	TBool iTunerOn;
    	TFMRadioFrequencyRange iRange;
		TInt iFrequency;
		TInt iSigStrength;
		TFMRadioAudioMode iMode;
		TBool iSquelch;

		// RDS data
    	TBool iNotifyOn;
		TBool iRdsSignal;
		TUint32 iRdsData;
		TBool iAutoSwitch;
		RArray<TInt> iFreqList;
		RArray<TRdsPSName> iPsList;
		TInt iPi;
		TRdsProgrammeType iPty;
		TRdsPSName iPs;
		TRdsRadioText iRt;
		RArray<TRdsRTplusTag> iRTplusTags;
		TDateTime iCt;

	private:    // Friend classes
		friend class CAdaptation;
		friend class CRdsGenerator;
	};

/**
*  Class CRdsGenerator
*  @lib FMRadioTunerControlStub.lib
*  @since Series 60 3.2
*/
class CRdsGenerator : public CTimer
    {
	public:
		static CRdsGenerator* NewL(CTuner& aTuner)
			{
			CRdsGenerator* self = new( ELeave ) CRdsGenerator(aTuner);
			CleanupStack::PushL( self );
		    self->ConstructL();
		    CleanupStack::Pop();
		    return self;
			};

		virtual ~CRdsGenerator()
			{
			if ( IsActive() )
				{
				Cancel();
				}
			};

		void Start()
			{
			if ( IsActive() )
				{
				Cancel();
				}
			After(1000000);	// 1 sec
			};

    private:
		void DoCancel()
			{};

		void RunL()
			{
			iTuner->SendRdsData();
			};

    private:
        CRdsGenerator( CTuner& aTuner )
        	:	CTimer(EPriorityStandard),
        		iTuner(&aTuner)
        	{};

        void ConstructL()
        	{
        	CTimer::ConstructL();
			CActiveScheduler::Add(this);
			};

    private:    // Data
    	// Tuner
    	CTuner* iTuner;
	};

/**
*  Class CAfSwitcher
*  @lib FMRadioTunerControlStub.lib
*  @since Series 60 3.2
*/
class CAfSwitcher : public CTimer
    {
	public:
		static CAfSwitcher* NewL(CTuner& aTuner)
			{
			CAfSwitcher* self = new( ELeave ) CAfSwitcher(aTuner);
			CleanupStack::PushL( self );
		    self->ConstructL();
		    CleanupStack::Pop();
		    return self;
			};

		virtual ~CAfSwitcher()
			{
			if ( IsActive() )
				{
				Cancel();
				}
			};

		void Start()
			{
			if ( IsActive() )
				{
				Cancel();
				}
			iBegin = ETrue;
			After(3000000);	// 3 sec
			};

    private:
		void DoCancel()
			{};

		void RunL()
			{
			iTuner->SendRdsAf(iBegin);
			if (iBegin)
				{
				iBegin = EFalse;
				After(2000000);	// 2 sec
				}
			};

    private:
        CAfSwitcher( CTuner& aTuner )
        	:	CTimer(EPriorityStandard),
        		iTuner(&aTuner),
        		iBegin(EFalse)
        	{};

        void ConstructL()
        	{
        	CTimer::ConstructL();
			CActiveScheduler::Add(this);
			};

    private:    // Data
    	// Tuner
    	CTuner* iTuner;
    	TBool iBegin;
	};

EXPORT_C CFMRadioTunerControl* CFMRadioTunerControl::NewL(
	MFMRadioTunerControlObserver& aObserver )
	{
	return CAdaptation::NewL(aObserver);
	}

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CAdaptation::CAdaptation
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CAdaptation::CAdaptation()
	:	iTuner(NULL),
		iRds(NULL),
		iRdsAf(NULL)
    {
    }

// -----------------------------------------------------------------------------
// CAdaptation::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CAdaptation::ConstructL(
	MFMRadioTunerControlObserver& aObserver )
    {
	iTuner = CTuner::NewL(aObserver);
	iRds = CRdsGenerator::NewL(*iTuner);
	iRdsAf = CAfSwitcher::NewL(*iTuner);
    }

// -----------------------------------------------------------------------------
// CAdaptation::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CAdaptation* CAdaptation::NewL(
	MFMRadioTunerControlObserver& aObserver )
    {
    CAdaptation* self = new( ELeave ) CAdaptation();
    CleanupStack::PushL( self );
    self->ConstructL(aObserver);
    CleanupStack::Pop();
    return self;
    }

// Destructor
CAdaptation::~CAdaptation()
    {
	delete iRdsAf;
	delete iRds;
	delete iTuner;
    }

// -----------------------------------------------------------------------------
//
void CAdaptation::TunerOn(
	TFMRadioFrequencyRange aRange,
	TInt aFrequency )
	{
	if ( ValidFrequency(aRange, aFrequency) )
		{
		iTuner->iTunerOn = ETrue;
		iTuner->iRange = aRange;
		SetFrequencySettings(aFrequency);
		iTuner->CompleteRequest(KReqTunerOn, KErrNone);
		}
	else
		{
		iTuner->CompleteRequest(KReqTunerOn, KFMRadioErrFrequencyOutOfBandRange);
		}
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelTunerOn()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::TunerOff()
	{
	iTuner->Reset();
	iTuner->CompleteRequest(KReqTunerOff, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelTunerOff()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::SetFrequencyRange(
	TFMRadioFrequencyRange aRange )
	{
	iTuner->iRange = aRange;
	iTuner->CompleteRequest(KReqSetFrequencyRange, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelSetFrequencyRange()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::SetFrequency(
	TInt aFrequency )
	{
	if ( !iTuner->iTunerOn )
		{
		iTuner->CompleteRequest(KReqSetFrequency, KFMRadioErrNotPermittedInCurrentState);
		return;
		}

	if ( ValidFrequency(iTuner->iRange, aFrequency) )
		{
		SetFrequencySettings(aFrequency);
		iTuner->CompleteRequest(KReqSetFrequency, KErrNone);
		}
	else
		{
		iTuner->CompleteRequest(KReqTunerOn, KFMRadioErrFrequencyNotValid);
		}
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelSetFrequency()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::StationSeek(
	TFMRadioSeekDirection aDirection )
	{
	if ( !iTuner->iTunerOn )
		{
		iTuner->CompleteRequest(KReqStationSeek, KFMRadioErrNotPermittedInCurrentState);
		return;
		}

	if ( aDirection == EFMRadioSeekUp )
		{
		if ( iTuner->iFrequency < KFrequency1 )
			{
			SetFrequencySettings(KFrequency1);
			}
		else if ( (iTuner->iFrequency >= KFrequency1) && (iTuner->iFrequency < KFrequency2) )
			{
			SetFrequencySettings(KFrequency2);
			}
		else if ( (iTuner->iFrequency >= KFrequency2) && (iTuner->iFrequency < KFrequency3) )
			{
			SetFrequencySettings(KFrequency3);
			}
		else if ( (iTuner->iFrequency >= KFrequency3) && (iTuner->iFrequency < KFrequency4) )
			{
			SetFrequencySettings(KFrequency4);
			}
		else	// iTuner->iFrequency >= KFrequency4
			{
			SetFrequencySettings(KFrequency1);
			}
		}
	else
		{
		if ( iTuner->iFrequency > KFrequency4 )
			{
			SetFrequencySettings(KFrequency4);
			}
		else if ( (iTuner->iFrequency <= KFrequency4) && (iTuner->iFrequency > KFrequency3) )
			{
			SetFrequencySettings(KFrequency3);
			}
		else if ( (iTuner->iFrequency <= KFrequency3) && (iTuner->iFrequency > KFrequency2) )
			{
			SetFrequencySettings(KFrequency2);
			}
		else if ( (iTuner->iFrequency <= KFrequency2) && (iTuner->iFrequency > KFrequency1) )
			{
			SetFrequencySettings(KFrequency1);
			}
		else	// iTuner->iFrequency < KFrequency1
			{
			SetFrequencySettings(KFrequency4);
			}
		}

	iTuner->CompleteRequest(KReqStationSeek, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelStationSeek()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::AudioMode()
	{
	iTuner->CompleteRequest(KReqAudioMode, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelAudioMode()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::SetAudioMode(
	TFMRadioAudioMode aMode )
	{
	iTuner->iMode = aMode;
	iTuner->CompleteRequest(KReqSetAudioMode, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelSetAudioMode()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::MaxSignalStrength()
	{
	iTuner->CompleteRequest(KReqMaxSignalStrength, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelMaxSignalStrength()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::SignalStrength()
	{
	iTuner->CompleteRequest(KReqSignalStrength, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelSignalStrength()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::Squelch()
	{
	iTuner->CompleteRequest(KReqSquelch, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelSquelch()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::SetSquelch(
	TBool aEnabled )
	{
	iTuner->iSquelch = aEnabled;
	iTuner->CompleteRequest(KReqSetSquelch, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelSetSquelch()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::BufferToBeFilled(
	TDes8& /*aBuffer*/ )
	{
	// Not used.
	}

// -----------------------------------------------------------------------------
//
TFMRadioCapabilities CAdaptation::Capabilities()
	{
	TFMRadioCapabilities cap;
	cap.iSampleRate = EFMRadioSampleRate48000Hz;
	cap.iChannels = EFMRadioAudioMono | EFMRadioAudioStereo;
	//const TUint32 KMMFFourCCCodePCM16 = 0x36315020;		//(' ', 'P', '1', '6')
	cap.iEncoding = 0x36315020;

	cap.iTunerBands = EFMRadioFrequencyEuroAmerica | EFMRadioFrequencyJapan;
	//cap.iTunerFunctions = ETunerAvailableInOfflineMode | ETunerRdsSupport;
	cap.iTunerFunctions = 0x01 | 0x02;
	return cap;
	}

/*********************************************************************
*  RD_FM_RADIO_ENHANCEMENTS
**********************************************************************/
#ifdef RD_FM_RADIO_ENHANCEMENTS

// -----------------------------------------------------------------------------
//
MRdsControl* CAdaptation::RdsControl(
	MRdsControlObserver& aObserver )
	{
	iTuner->iRdsObserver = &aObserver;
	return this;
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetFrequencyRange(
	TFMRadioFrequencyRange& aRange,
	TInt& aMinFreq,
	TInt& aMaxFreq ) const
	{
	aRange = iTuner->iRange;
	if ( aRange == EFMRadioFrequencyEuroAmerica )
		{
		aMinFreq = KEuroAmericaMin;
		aMaxFreq = KEuroAmericaMax;
		}
	else	// aRange = EFMRadioFrequencyJapan;
		{
		aMinFreq = KJapanMin;
		aMaxFreq = KJapanMax;
		}
	return KErrNone;
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetCapabilities(
	TRdsCapabilities& aCaps ) const
	{
	aCaps.iRdsFunctions = ERdsProgrammeIdentification | ERdsProgrammeType | ERdsProgrammeService
			| ERdsRadioText | ERdsClockTime | ERdsAlternateFrequency;
	return KErrNone;
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetRdsSignalStatus(
	TBool& aRdsSignal ) const
	{
    aRdsSignal = iTuner->iRdsSignal;
    return KErrNone;
    }

// -----------------------------------------------------------------------------
//
TInt CAdaptation::NotifyRdsDataChange(
	TRdsData aRdsData )
	{
	iTuner->iRdsData = aRdsData.iRdsFunctions;
	// We'll only generate events for ERdsProgrammeIdentification, ERdsProgrammeType,
	// ERdsProgrammeService for now.
	if ( (aRdsData.iRdsFunctions & ERdsProgrammeIdentification) ||
		 (aRdsData.iRdsFunctions & ERdsProgrammeType) ||
		 (aRdsData.iRdsFunctions & ERdsProgrammeService) )
		{
		iTuner->iNotifyOn = ETrue;
		}

	if ( (iTuner->iRdsSignal) && (iTuner->iNotifyOn) )
		{
		iRds->Start();
		}
     return KErrNone;
     }

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelNotifyRdsDataChange()
	{
	iTuner->iRdsData = 0;
	iTuner->iNotifyOn = EFalse;
	iRds->Cancel();
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::SetAutomaticSwitching(
	TBool aAuto )
	{
	iTuner->iAutoSwitch = aAuto;
	if ( aAuto && (iTuner->iFrequency == KFrequency1) )
		{
		iRdsAf->Start();
		}
	return KErrNone;
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetAutomaticSwitching(
	TBool& aAuto )
	{
	aAuto = iTuner->iAutoSwitch;
	return KErrNone;
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelAFSearch()
	{
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::SetAutomaticTrafficAnnouncement(
	TBool /*aAuto*/ )
	{
	return KErrNotSupported;
	};

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetAutomaticTrafficAnnouncement(
	TBool& aAuto )
	{
	aAuto = EFalse;
	return KErrNone;
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::StationSeekByPTY(
	TRdsProgrammeType aPty,
	TBool aSeekUp )
	{
	if ( !iTuner->iTunerOn )
		{
		iTuner->CompleteRequest(KReqStationSeekByPTY, KRdsErrNotPermittedInCurrentState);
		return;
		}

	TInt err = KErrNone;
	switch ( aPty )
		{
		case KRdsPtyReligion:
			if ( aSeekUp )
				{
				if ( iTuner->iFrequency < KFrequency1 )
					{
					SetFrequencySettings(KFrequency1);
					}
				else if ( (iTuner->iFrequency >= KFrequency1) && (iTuner->iFrequency < KFrequency2) )
					{
					SetFrequencySettings(KFrequency2);
					}
				else	// iTuner->iFrequency >= KFrequency2
					{
					SetFrequencySettings(KFrequency1);
					}
				}
			else
				{
				if ( iTuner->iFrequency > KFrequency2 )
					{
					SetFrequencySettings(KFrequency2);
					}
				else if ( (iTuner->iFrequency <= KFrequency2) && (iTuner->iFrequency > KFrequency1) )
					{
					SetFrequencySettings(KFrequency1);
					}
				else	// iTuner->iFrequency < KFrequency1
					{
					SetFrequencySettings(KFrequency2);
					}
				}
			break;
		case KRdsPtyLightClassical:
			SetFrequencySettings(KFrequency3);
			break;
		case KRdsPtyRockMusic:
			SetFrequencySettings(KFrequency4);
			break;
		default:
			err = KErrNotFound;
			break;
		}
	iTuner->CompleteRequest(KReqStationSeekByPTY, err);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::StationSeekByTA(
	TBool aSeekUp )
	{
	if ( !iTuner->iTunerOn )
		{
		iTuner->CompleteRequest(KReqStationSeekByTA, KRdsErrNotPermittedInCurrentState);
		return;
		}

	// Just for testing purpose, if aSeekUp return KFrequency1, else return KErrNotFound.
	if ( aSeekUp )
		{
		SetFrequencySettings(KFrequency1);
		iTuner->CompleteRequest(KReqStationSeekByTA, KErrNone);
		}
	else
		{
		iTuner->CompleteRequest(KReqStationSeekByTA, KErrNotFound);
		}
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::StationSeekByTP(
	TBool aSeekUp )
	{
	if ( !iTuner->iTunerOn )
		{
		iTuner->CompleteRequest(KReqStationSeekByTP, KRdsErrNotPermittedInCurrentState);
		return;
		}

	if ( aSeekUp )
		{
		if ( iTuner->iFrequency < KFrequency1 )
			{
			SetFrequencySettings(KFrequency1);
			}
		else if ( (iTuner->iFrequency >= KFrequency1) && (iTuner->iFrequency < KFrequency4) )
			{
			SetFrequencySettings(KFrequency4);
			}
		else	// iTuner->iFrequency >= KFrequency4
			{
			SetFrequencySettings(KFrequency1);
			}
		}
	else
		{
		if ( iTuner->iFrequency > KFrequency4 )
			{
			SetFrequencySettings(KFrequency4);
			}
		else if ( (iTuner->iFrequency <= KFrequency4) && (iTuner->iFrequency > KFrequency1) )
			{
			SetFrequencySettings(KFrequency1);
			}
		else	// iTuner->iFrequency < KFrequency1
			{
			SetFrequencySettings(KFrequency4);
			}
		}

	iTuner->CompleteRequest(KReqStationSeekByTP, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelRdsStationSeek()
     {
     }

// -----------------------------------------------------------------------------
//
void CAdaptation::GetFreqByPTY(
	TRdsProgrammeType aPty )
	{
	if ( !iTuner->iTunerOn )
		{
		iTuner->CompleteRequest(KReqGetFreqByPTY, KRdsErrNotPermittedInCurrentState);
		return;
		}

	TInt err = KErrNone;
	iTuner->iFreqList.Reset();
	switch ( aPty )
		{
		case KRdsPtyReligion:
			iTuner->iFreqList.Append(KFrequency1);
			iTuner->iFreqList.Append(KFrequency2);
			break;
		case KRdsPtyLightClassical:
			iTuner->iFreqList.Append(KFrequency3);
			break;
		case KRdsPtyRockMusic:
			iTuner->iFreqList.Append(KFrequency4);
			break;
		default:
			err = KErrNotFound;
			break;
		}
	iTuner->CompleteRequest(KReqGetFreqByPTY, err);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelGetFreqByPTY()
    {
    }

// -----------------------------------------------------------------------------
//
void CAdaptation::GetFreqByTA()
	{
	if ( !iTuner->iTunerOn )
		{
		iTuner->CompleteRequest(KReqGetFreqByTA, KRdsErrNotPermittedInCurrentState);
		return;
		}

	iTuner->iFreqList.Reset();
	iTuner->iFreqList.Append(KFrequency1);
	iTuner->CompleteRequest(KReqGetFreqByTA, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelGetFreqByTA()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::GetPSByPTY
	( TRdsProgrammeType aPty )
	{
	if ( !iTuner->iTunerOn )
		{
		iTuner->CompleteRequest(KReqGetPSByPTY, KRdsErrNotPermittedInCurrentState);
		return;
		}

	TInt err = KErrNone;
	iTuner->iPsList.Reset();
	switch ( aPty )
		{
		case KRdsPtyReligion:
			iTuner->iPsList.Append(_L8("KCBI"));
			iTuner->iPsList.Append(_L8("KLTY"));
			break;
		case KRdsPtyLightClassical:
			iTuner->iPsList.Append(_L8("KVIL"));
			break;
		case KRdsPtyRockMusic:
			iTuner->iPsList.Append(_L8("KHKS"));
			break;
		default:
			err = KErrNotFound;
			break;
		}
	iTuner->CompleteRequest(KReqGetPSByPTY, err);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelGetPSByPTY()
	{
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::GetPSByTA()
	{
	if ( !iTuner->iTunerOn )
		{
		iTuner->CompleteRequest(KReqGetPSByTA, KRdsErrNotPermittedInCurrentState);
		return;
		}

	iTuner->iPsList.Reset();
	iTuner->iPsList.Append(_L8("KCBI"));
	iTuner->CompleteRequest(KReqGetPSByTA, KErrNone);
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::CancelGetPSByTA()
	{
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetProgrammeIdentification(
	TInt& aPi )
	{
	if ( !iTuner->iTunerOn )
		{
		return KRdsErrNotPermittedInCurrentState;
		}

	if ( iTuner->iRdsSignal )
		{
		aPi = iTuner->iPi;
		return KErrNone;
		}
	else
		{
		return KErrNotFound;
		}
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetProgrammeType(
	TRdsProgrammeType& aPty )
	{
	if ( !iTuner->iTunerOn )
		{
		return KRdsErrNotPermittedInCurrentState;
		}

	if ( iTuner->iRdsSignal )
		{
		aPty = iTuner->iPty;
		return KErrNone;
		}
	else
		{
		return KErrNotFound;
		}
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetProgrammeService(
	TRdsPSName& aPs )
	{
	if ( !iTuner->iTunerOn )
		{
		return KRdsErrNotPermittedInCurrentState;
		}

	if ( iTuner->iRdsSignal )
		{
		aPs = iTuner->iPs;
		return KErrNone;
		}
	else
		{
		return KErrNotFound;
		}
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetRadioText(
	TRdsRadioText& aRt,
	RArray<TRdsRTplusTag>& /*aRTplusTags*/ )
	{
	if ( !iTuner->iTunerOn )
		{
		return KRdsErrNotPermittedInCurrentState;
		}

	if ( iTuner->iRdsSignal )
		{
		aRt = iTuner->iRt;
		return KErrNone;
		}
	else
		{
		return KErrNotFound;
		}
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetClockTime(
	TDateTime& aCt )
	{
	if ( !iTuner->iTunerOn )
		{
		return KRdsErrNotPermittedInCurrentState;
		}

	if ( iTuner->iRdsSignal )
		{
		// Hardcoded to 2007/11/10 7:35
		aCt.Set(2007,ENovember,10,7,35,0,0);
		return KErrNone;
		}
	else
		{
		return KErrNotFound;
		}
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetTrafficAnnouncementStatus(
	TBool& aTaStatus )
	{
	if ( !iTuner->iTunerOn )
		{
		return KRdsErrNotPermittedInCurrentState;
		}

	if ( iTuner->iFrequency == KFrequency1 )
		{
		aTaStatus = ETrue;
		return KErrNone;
		}
	else if ( iTuner->iFrequency == KFrequency4 )
		{
		aTaStatus = EFalse;
		return KErrNone;
		}
	else
		{
		return KErrNotFound;

		}
	}

// -----------------------------------------------------------------------------
//
TInt CAdaptation::GetTrafficProgrammeStatus(
	TBool& aTpStatus )
	{
	if ( !iTuner->iTunerOn )
		{
		return KRdsErrNotPermittedInCurrentState;
		}

	if ( (iTuner->iFrequency == KFrequency1) || (iTuner->iFrequency == KFrequency4) )
		{
		aTpStatus = ETrue;
		return KErrNone;
		}
	else if ( (iTuner->iFrequency == KFrequency2) || (iTuner->iFrequency == KFrequency3) )
		{
		aTpStatus = EFalse;
		return KErrNone;
		}
	else
		{
		return KErrNotFound;
		}
     }

#endif //#ifdef RD_FM_RADIO_ENHANCEMENTS

// -----------------------------------------------------------------------------
//
TBool CAdaptation::ValidFrequency(
	TFMRadioFrequencyRange aRange,
	TInt aFrequency )
	{
	if ( aRange == EFMRadioFrequencyEuroAmerica )
		{
		if ( (aFrequency < KEuroAmericaMin) || (aFrequency > KEuroAmericaMax) )
			{
			return EFalse;
			}
		}
	else	// aRange = EFMRadioFrequencyJapan;
		{
		if ( (aFrequency < KJapanMin) || (aFrequency > KJapanMax) )
			{
			return EFalse;
			}
		}
	return ETrue;
	}

// -----------------------------------------------------------------------------
//
void CAdaptation::SetFrequencySettings(
	TInt aFrequency )
	{
	iTuner->iFrequency = aFrequency;
	switch ( aFrequency )
		{
		case KFrequency1:
			iTuner->iSigStrength = 1;
			iTuner->iRdsSignal = ETrue;
			iTuner->iPi = 1000;
			iTuner->iPty = KRdsPtyReligion;
			iTuner->iPs = _L8("KCBI");
			iTuner->iRt = _L8("Hail to the King");
			if ( iTuner->iAutoSwitch )
				{
				iRdsAf->Start();
				}
			break;
		case KFrequency2:
			iTuner->iSigStrength = 2;
			iTuner->iRdsSignal = ETrue;
			iTuner->iPi = 1000;
			iTuner->iPty = KRdsPtyReligion;
			iTuner->iPs = _L8("KLTY");
			iTuner->iRt = _L8("He Knows My Name");
			break;
		case KFrequency3:
			iTuner->iSigStrength = 3;
			iTuner->iRdsSignal = ETrue;
			iTuner->iPi = 2000;
			iTuner->iPty = KRdsPtyLightClassical;
			iTuner->iPs = _L8("KVIL");
			iTuner->iRt = _L8("Power of Love");
			break;
		case KFrequency4:
			iTuner->iSigStrength = 4;
			iTuner->iRdsSignal = ETrue;
			iTuner->iPi = 3000;
			iTuner->iPty = KRdsPtyRockMusic;
			iTuner->iPs = _L8("KHKS");
			iTuner->iRt = _L8("The Reason");
			break;
		default:
			iTuner->iSigStrength = 5;
			iTuner->iRdsSignal = EFalse;
			break;
		}
	if ( (iTuner->iRdsSignal) && (iTuner->iNotifyOn) )
		{
		iRds->Start();
		}
	}

// ========================== OTHER EXPORTED FUNCTIONS =========================


//  End of File

