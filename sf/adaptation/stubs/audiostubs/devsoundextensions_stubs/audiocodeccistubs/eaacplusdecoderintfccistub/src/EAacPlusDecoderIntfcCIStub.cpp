/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  Custom Interface stub implementation for eAAC+ decoder.
*
*/


// INCLUDE FILES
#include <e32debug.h>
#include "EAacPlusDecoderIntfcCI.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

/**
 * CEAacPlusDecoderIntfcCI::CEAacPlusDecoderIntfcCI
 * C++ default constructor can NOT contain any code, that might leave.
 */
CEAacPlusDecoderIntfcCI::CEAacPlusDecoderIntfcCI()
    {
    }

/**
 * CEAacPlusDecoderIntfcCI::ConstructL
 * Symbian 2nd phase constructor can leave.
 */
void CEAacPlusDecoderIntfcCI::ConstructL()
    {
    }

/**
 * CEAacPlusDecoderIntfcCI::NewL
 * Two-phased constructor.
 */
EXPORT_C CEAacPlusDecoderIntfcCI* CEAacPlusDecoderIntfcCI::NewL()
    {
	CEAacPlusDecoderIntfcCI* self = new (ELeave)CEAacPlusDecoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

/**
 * Destructor
 */
EXPORT_C CEAacPlusDecoderIntfcCI::~CEAacPlusDecoderIntfcCI()
    {
    }


// From CEAacPlusDecoderIntfc

/**
 * Configures decoder's profile defined by TAudioObjectType.
 * (other items defined in the header)
 */
EXPORT_C void CEAacPlusDecoderIntfcCI::SetAudioObjectType(
                                       TAudioObjectType aAudioObjectType)
    {
    iAudioObjectType = aAudioObjectType;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::SetAudioObjectType - object type[%d]"),
                  iAudioObjectType);
#endif //_DEBUG
    }

/**
 * Configures decoder's input sampling frequency.
 * (other items defined in the header)
 */
EXPORT_C void CEAacPlusDecoderIntfcCI::SetInputSamplingFrequency(
                                       TUint aInputSamplingFrequency)
    {
    iInputSamplingFrequency = aInputSamplingFrequency;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::SetInputSamplingFrequency [%d]"),
                  iInputSamplingFrequency);
#endif //_DEBUG
    }

/**
 * Configures decoder's channel settings.
 * (other items defined in the header)
 */
EXPORT_C void CEAacPlusDecoderIntfcCI::SetNumOfChannels(TUint aNumChannels)
    {
    iNumOfChannels = aNumChannels;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::SetNumOfChannels [%d]"),
                  iNumOfChannels);
#endif //_DEBUG
    }

/**
 * Configures decoder's SBR settings.
 * (other items defined in the header)
 */
EXPORT_C void CEAacPlusDecoderIntfcCI::SetSbr(TBool aSbrEnabled)
    {
    iSbrEnabled = aSbrEnabled;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::SetSbr [%d]"), iSbrEnabled);
#endif //_DEBUG
    }

/**
 * Configures decoder's DSM settings.
 * (other items defined in the header)
 */
EXPORT_C void CEAacPlusDecoderIntfcCI::SetDownSampledMode(TBool aDsmEnabled)
    {
    iDsmEnabled = aDsmEnabled;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::SetDownSampledMode [%d]"),
                  iDsmEnabled);
#endif //_DEBUG
    }

/**
 * Returns decoder's AAC profile settings.
 * (other items defined in the header)
 */
EXPORT_C TInt CEAacPlusDecoderIntfcCI::GetAudioObjectType(
                                       TAudioObjectType& aAudioObjectType)
	{
	aAudioObjectType = iCurrentAudioObjectType;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::GetAudioObjectType [%d]"),
                  iAudioObjectType);
#endif //_DEBUG

	return KErrNone;
	}

/**
 * Returns decoder's input sampling frequency settings.
 * (other items defined in the header)
 */
EXPORT_C TInt CEAacPlusDecoderIntfcCI::GetInputSamplingFrequency(
                                       TUint& aInputSamplingFrequency)
    {
	aInputSamplingFrequency = iCurrentInputSamplingFrequency;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::GetInputSamplingFrequency [%d]"),
                  iInputSamplingFrequency);
#endif //_DEBUG

	return KErrNone;
    }

/**
 * Returns decoder's channel settings.
 * (other items defined in the header)
 */
EXPORT_C TInt CEAacPlusDecoderIntfcCI::GetNumOfChannels(TUint& aNumOfChannels)
    {
	aNumOfChannels = iCurrentNumOfChannels;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::GetNumOfChannels [%d]"),
                  iCurrentNumOfChannels);
#endif //_DEBUG

	return KErrNone;
    }

/**
 * Returns decoder's SBR settings.
 * (other items defined in the header)
 */
EXPORT_C TInt CEAacPlusDecoderIntfcCI::GetSbr(TBool& aSbrEnabled)
    {
	aSbrEnabled = iCurrentSbrEnabled;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::GetSbr [%d]"),
                  iCurrentSbrEnabled);
#endif //_DEBUG

	return KErrNone;
    }

/**
 * Returns decoder's DSM settings.
 * (other items defined in the header)
 */
EXPORT_C TInt CEAacPlusDecoderIntfcCI::GetDownSampledMode(TBool& aDsmEnabled)
    {
	aDsmEnabled = iCurrentDsmEnabled;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::GetDownSampledMode [%d]"),
                  iCurrentDsmEnabled);
#endif //_DEBUG

	return KErrNone;
    }

/**
 * Applies configuration settings to the decoder.
 * (other items defined in the header)
 */
EXPORT_C TInt CEAacPlusDecoderIntfcCI::ApplyConfig()
    {
	iCurrentAudioObjectType        = iAudioObjectType;
	iCurrentInputSamplingFrequency = iInputSamplingFrequency;
	iCurrentNumOfChannels          = iNumOfChannels;
	iCurrentSbrEnabled             = iSbrEnabled;
	iCurrentDsmEnabled             = iDsmEnabled;

#ifdef _DEBUG
    RDebug::Print(_L("CEAacPlusDecoderIntfcCI::ApplyConfig - eAAC+ Config Applied"));
#endif //_DEBUG

	return KErrNone;
    }

// End of file
