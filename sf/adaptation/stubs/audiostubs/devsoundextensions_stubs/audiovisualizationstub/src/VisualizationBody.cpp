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
* Description: Audio Stubs -  Implementation of class Audio VisualiztionBody.
*
*/


// INCLUDE FILES

#ifdef _DEBUG
#include <e32svr.h>
#endif
#include <Visualization.h>
#include <MVisualizationObserver.h>
#include "VisualizationBody.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CVisualization::CVisualization
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CVisualization::CBody::CBody()
    : 	iFrequencyDataEnabled(EFalse),
    	iWaveFormDataEnabled(EFalse)
    {
    }


// Destructor
CVisualization::CBody::~CBody()
    {
	iFrequencyData.Close();
	iWaveFormData.Close();
    }

// -----------------------------------------------------------------------------
// CVisualization::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CVisualization::CBody::ConstructL(
	MVisualizationObserver& aObserver )
     {
#ifdef _DEBUG
    RDebug::Print(_L("CVisualization::Body::ConstructL"));
#endif
	iObserver = &aObserver;
	}

// -----------------------------------------------------------------------------
// CVisualization::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CVisualization::CBody* CVisualization::CBody::NewL(
    MVisualizationObserver& aObserver )
    {
#ifdef _DEBUG
    RDebug::Print(_L("CVisualization::Cbody::NewL"));
#endif
    CVisualization::CBody* self = new (ELeave) CVisualization::CBody();
    CleanupStack::PushL(self);
    self->ConstructL(aObserver);
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CVisualization::DataRate
// -----------------------------------------------------------------------------
//
 TUint32 CVisualization::CBody::DataRate() const
    {
	return iDataRate;
    }

// -----------------------------------------------------------------------------
// CVisualization::DataRateRange
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::DataRateRange(
	TUint32& aMin,
	TUint32& aMax )
    {
	aMin = iDataRateMin;
	aMax = iDataRateMax;
    }

// -----------------------------------------------------------------------------
// CVisualization::EnableFrequencyData
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::EnableFrequencyData(
	TBool aIndicator )
    {
	iFrequencyDataEnabled = aIndicator;
    }

// -----------------------------------------------------------------------------
// CVisualization::EnableWaveformData
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::EnableWaveformData(
	TBool aIndicator )
    {
	iWaveFormDataEnabled = aIndicator;
    }

// -----------------------------------------------------------------------------
// CVisualization::FreqencyBandCount
// -----------------------------------------------------------------------------
//
 TUint32 CVisualization::CBody::FreqencyBandCount() const
    {
	return iFreqencyBandCount;
    }

// -----------------------------------------------------------------------------
// CVisualization::FrequencyBandCountRange
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::FrequencyBandCountRange(
	TUint32& aMin,
	TUint32& aMax )
    {
	aMin = iFreqencyBandCountMin;
	aMax = iFreqencyBandCountMax;
    }

// -----------------------------------------------------------------------------
// CVisualization::IsFrequencyDataEnabled
// -----------------------------------------------------------------------------
//
 TBool CVisualization::CBody::IsFrequencyDataEnabled() const
    {
	return iFrequencyDataEnabled;
    }

// -----------------------------------------------------------------------------
// CVisualization::IsWaveformDataEnabled
// -----------------------------------------------------------------------------
//
 TBool CVisualization::CBody::IsWaveformDataEnabled() const
    {
	return iWaveFormDataEnabled;
    }

// -----------------------------------------------------------------------------
// CVisualization::SamplingRate
// -----------------------------------------------------------------------------
//
 TUint32 CVisualization::CBody::SamplingRate() const
    {
	return iSamplingRate;
    }

// -----------------------------------------------------------------------------
// CVisualization::SetDataRateL
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::SetDataRateL(
	TUint32 aDataRate )
	{
	if ( (aDataRate >= iDataRateMin) && (aDataRate <= iDataRateMax) )
		{
		iDataRate = aDataRate;
		}
	else
		{
		User::Leave(KErrArgument);
		}
	}

// -----------------------------------------------------------------------------
// CVisualization::SetFrequencyBandCount
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::SetFrequencyBandCountL(
	TUint32 aFrequencyBandCount )
	{
	if ( (aFrequencyBandCount >= iFreqencyBandCountMin) && (aFrequencyBandCount <= iFreqencyBandCountMax) )
		{
		iFreqencyBandCount = aFrequencyBandCount;
		}
	else
		{
		User::Leave(KErrArgument);
		}
	}

// -----------------------------------------------------------------------------
// CVisualization::SetWaveformDataLength
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::SetWaveformDataLengthL(
	TUint32 aWaveformDataLength )
	{
	if ( (aWaveformDataLength >= iWaveformDataLengthMin) && (aWaveformDataLength <= iWaveformDataLengthMax) )
		{
		iWaveformDataLength = aWaveformDataLength;
		}
	else
		{
		User::Leave(KErrArgument);
		}
	}

// -----------------------------------------------------------------------------
// CVisualization::StartL
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::StartL()
    {
	User::Leave(KErrNotSupported);
    }

// -----------------------------------------------------------------------------
// CVisualization::StopL
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::StopL()
    {
	User::Leave(KErrNotSupported);
    }

// -----------------------------------------------------------------------------
// CVisualization::WaveformDataLength
// -----------------------------------------------------------------------------
//
 TUint32 CVisualization::CBody::WaveformDataLength() const
    {
	return iWaveformDataLength;
    }

// -----------------------------------------------------------------------------
// CVisualization::WaveformDataLengthRange
// -----------------------------------------------------------------------------
//
 void CVisualization::CBody::WaveformDataLengthRange(
	TUint32& aMin,
	TUint32& aMax )
    {
	aMin = iWaveformDataLengthMin;
	aMax = iWaveformDataLengthMax;
    }


