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
* Description: Audio Stubs -  Implementation of class Audio Visualiztion.
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
CVisualization::CVisualization()
    {
    }


// Destructor
CVisualization::~CVisualization()
    {
    delete iBody;
    }

// -----------------------------------------------------------------------------
// CVisualization::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CVisualization::ConstructL(MVisualizationObserver& aObserver)
     {
#ifdef _DEBUG
    RDebug::Print(_L("CVisualization::ConstructL"));
#endif
     iBody = CVisualization::CBody::NewL(aObserver);
	}

// -----------------------------------------------------------------------------
// CVisualization::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CVisualization* CVisualization::NewL(
    MVisualizationObserver& aObserver )
    {
#ifdef _DEBUG
    RDebug::Print(_L("CVisualization::NewL"));
#endif
    CVisualization* self = new (ELeave) CVisualization();
    CleanupStack::PushL(self);
    self->ConstructL(aObserver);
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CVisualization::DataRate
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CVisualization::DataRate() const
    {
	return iBody->DataRate();
    }

// -----------------------------------------------------------------------------
// CVisualization::DataRateRange
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::DataRateRange(
	TUint32& aMin,
	TUint32& aMax )
    {
	iBody->DataRateRange(aMin, aMax);
    }

// -----------------------------------------------------------------------------
// CVisualization::EnableFrequencyData
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::EnableFrequencyData(
	TBool aIndicator )
    {
	iBody->EnableFrequencyData(aIndicator);
    }

// -----------------------------------------------------------------------------
// CVisualization::EnableWaveformData
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::EnableWaveformData(
	TBool aIndicator )
    {
	iBody->EnableWaveformData(aIndicator);
    }

// -----------------------------------------------------------------------------
// CVisualization::FreqencyBandCount
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CVisualization::FreqencyBandCount() const
    {
	return iBody->FreqencyBandCount();
    }

// -----------------------------------------------------------------------------
// CVisualization::FrequencyBandCountRange
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::FrequencyBandCountRange(
	TUint32& aMin,
	TUint32& aMax )
    {
	iBody->FrequencyBandCountRange(aMin, aMax);
    }

// -----------------------------------------------------------------------------
// CVisualization::IsFrequencyDataEnabled
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CVisualization::IsFrequencyDataEnabled() const
    {
	return iBody->IsFrequencyDataEnabled();
    }

// -----------------------------------------------------------------------------
// CVisualization::IsWaveformDataEnabled
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CVisualization::IsWaveformDataEnabled() const
    {
	return iBody->IsWaveformDataEnabled();
    }

// -----------------------------------------------------------------------------
// CVisualization::SamplingRate
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CVisualization::SamplingRate() const
    {
	return iBody->SamplingRate();
    }

// -----------------------------------------------------------------------------
// CVisualization::SetDataRateL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::SetDataRateL(
	TUint32 aDataRate )
	{
	iBody->SetDataRateL(aDataRate);
	}

// -----------------------------------------------------------------------------
// CVisualization::SetFrequencyBandCount
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::SetFrequencyBandCountL(
	TUint32 aFrequencyBandCount )
	{
    iBody->SetFrequencyBandCountL(aFrequencyBandCount);
	}

// -----------------------------------------------------------------------------
// CVisualization::SetWaveformDataLength
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::SetWaveformDataLengthL(
	TUint32 aWaveformDataLength )
	{
    iBody->SetWaveformDataLengthL(aWaveformDataLength );
	}

// -----------------------------------------------------------------------------
// CVisualization::StartL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::StartL()
    {
	User::Leave(KErrNotSupported);
    }

// -----------------------------------------------------------------------------
// CVisualization::StopL
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::StopL()
    {
	User::Leave(KErrNotSupported);
    }

// -----------------------------------------------------------------------------
// CVisualization::WaveformDataLength
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CVisualization::WaveformDataLength() const
    {
	return iBody->WaveformDataLength();
    }

// -----------------------------------------------------------------------------
// CVisualization::WaveformDataLengthRange
// -----------------------------------------------------------------------------
//
EXPORT_C void CVisualization::WaveformDataLengthRange(
	TUint32& aMin,
	TUint32& aMax )
    {
	iBody->WaveformDataLengthRange(aMin,aMax);
    }

