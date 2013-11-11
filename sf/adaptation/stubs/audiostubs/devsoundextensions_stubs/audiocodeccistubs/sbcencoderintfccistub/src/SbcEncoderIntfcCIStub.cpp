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
* Description: Audio Stubs -  Custom Interface stub object for configuring the SBC encoder.
*
*/


// INCLUDE FILES
#include <e32debug.h>
#include "SbcEncoderIntfcCI.h"

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
 * CSbcEncoderIntfcCI::CSbcEncoderIntfcCI
 * C++ default constructor.
 */
CSbcEncoderIntfcCI::CSbcEncoderIntfcCI()
    {
    }

/**
 * CSbcEncoderIntfcCI::ConstructL
 * Symbian 2nd phase constructor.
 */
void CSbcEncoderIntfcCI::ConstructL()
    {
    }

/**
 * CSbcEncoderIntfcCI::NewL
 * Two-phased constructor.
 */
EXPORT_C CSbcEncoderIntfcCI* CSbcEncoderIntfcCI::NewL()
    {
	CSbcEncoderIntfcCI* self = new (ELeave)CSbcEncoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

/**
 * Destructor
 */
EXPORT_C CSbcEncoderIntfcCI::~CSbcEncoderIntfcCI()
    {
    }

/**
 * CSbcEncoderIntfcCI::GetSupportedSamplingFrequencies
 * Returns and array of supported sampling frequencies.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetSupportedSamplingFrequencies(
                                  RArray<TUint>& aSupportedSamplingFrequencies)
    {
    aSupportedSamplingFrequencies.Reset();
    aSupportedSamplingFrequencies.Append(16000);
    aSupportedSamplingFrequencies.Append(32000);
    aSupportedSamplingFrequencies.Append(44100);
    aSupportedSamplingFrequencies.Append(48000);

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetSupportedSamplingFrequencies - Num of supported Fs[%d]"),
                  aSupportedSamplingFrequencies.Count());
#endif
	return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::GetSupportedChannelModes
 * Returns and array of supported channel modes.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetSupportedChannelModes(
                                  RArray<TSbcChannelMode>&
                                  aSupportedChannelModes)
    {
    aSupportedChannelModes.Reset();
    aSupportedChannelModes.Append(ESbcChannelMono);
    aSupportedChannelModes.Append(ESbcChannelDual);
    aSupportedChannelModes.Append(ESbcChannelStereo);
    aSupportedChannelModes.Append(ESbcChannelJointStereo);

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetSupportedChannelModes - Num of supported ch modes[%d]"),
                  aSupportedChannelModes.Count());
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::GetSupportedNumOfBlocks
 * Returns an array of supported block numbers.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetSupportedNumOfBlocks(
		                          RArray<TUint>& aSupportedNumOfBlocks)
    {
    aSupportedNumOfBlocks.Reset();
    aSupportedNumOfBlocks.Append(4);
    aSupportedNumOfBlocks.Append(8);
    aSupportedNumOfBlocks.Append(12);
    aSupportedNumOfBlocks.Append(16);

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetSupportedNumOfBlocks - Num of supported blocks[%d]"),
                  aSupportedNumOfBlocks.Count());
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::GetSupportedNumOfSubbands
 * Returns an array of supported subbands.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetSupportedNumOfSubbands(
                                  RArray<TUint>& aSupportedNumOfSubbands)
    {
    aSupportedNumOfSubbands.Reset();
    aSupportedNumOfSubbands.Append(4);
    aSupportedNumOfSubbands.Append(8);

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetSupportedNumOfSubbands - Num of supported subbands[%d]"),
                  aSupportedNumOfSubbands.Count());
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::GetSupportedAllocationMethods
 * Returns an array of supported allocation methods.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetSupportedAllocationMethods(
                                  RArray<TSbcAllocationMethod>&
                                  aSupportedAllocationMethods)
    {
    aSupportedAllocationMethods.Reset();
    aSupportedAllocationMethods.Append(ESbcAllocationSNR);
    aSupportedAllocationMethods.Append(ESbcAllocationLoudness);

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetSupportedAllocationMethods - Num of supported alloc methods[%d]"),
                  aSupportedAllocationMethods.Count());
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::GetSupportedBitpoolRange
 * Returns supported bitpool range.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetSupportedBitpoolRange(
        		                  TUint& aMinSupportedBitpoolSize,
        		                  TUint& aMaxSupportedBitpoolSize)
    {
    aMinSupportedBitpoolSize = 2;
    aMaxSupportedBitpoolSize = 250;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetSupportedBitpoolRange - Min[%d] - Max [%d]"),
                  aMinSupportedBitpoolSize,
                  aMaxSupportedBitpoolSize);
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::SetSamplingFrequency
 * Sets new sampling frequency.
 * (other items were commented in a header).
 */
EXPORT_C void CSbcEncoderIntfcCI::SetSamplingFrequency(TUint aSamplingFrequency)
    {
    iSamplingFrequency = aSamplingFrequency;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::SetSamplingFrequency [%d]"),
                  iSamplingFrequency);
#endif
    }

/*
 * CSbcEncoderIntfcCI::GetSamplingFrequency
 * Returns current sampling frequency.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetSamplingFrequency(
                                  TUint& aSamplingFrequency)
    {
    aSamplingFrequency = iCurrentSamplingFrequency;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetSamplingFrequency [%d]"),
                  iCurrentSamplingFrequency);
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::SetChannelMode
 * Sets new channel mode.
 * (other items were commented in a header).
 */
EXPORT_C void CSbcEncoderIntfcCI::SetChannelMode(TSbcChannelMode aChannelMode)
    {
    iChannelMode = aChannelMode;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::SetChannelMode [%d]"), iChannelMode);
#endif
    }

/*
 * CSbcEncoderIntfcCI::GetChannelMode
 * Returns current channel mode.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetChannelMode(TSbcChannelMode& aChannelMode)
    {
    aChannelMode = iCurrentChannelMode;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetChannelMode [%d]"),
                  iCurrentChannelMode);
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::SetNumOfSubbands
 * Sets new number of subbands.
 * (other items were commented in a header).
 */
EXPORT_C void CSbcEncoderIntfcCI::SetNumOfSubbands(TUint aNumOfSubbands)
    {
    iNumOfSubbands = aNumOfSubbands;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::SetNumOfSubbands [%d]"),
                  iNumOfSubbands);
#endif
    }

/*
 * CSbcEncoderIntfcCI::GetNumOfSubbands
 * Returns current number of subbands.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetNumOfSubbands(TUint& aNumOfSubbands)
    {
    aNumOfSubbands = iCurrentNumOfSubbands;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetNumOfSubbands [%d]"),
                  iCurrentNumOfSubbands);
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::SetNumOfBlocks
 * Sets new number of blocks.
 * (other items were commented in a header).
 */
EXPORT_C void CSbcEncoderIntfcCI::SetNumOfBlocks(TUint aNumOfBlocks)
    {
    iNumOfBlocks = aNumOfBlocks;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::SetNumOfBlocks [%d]"), iNumOfBlocks);
#endif
    }

/*
 * CSbcEncoderIntfcCI::GetNumOfBlocks
 * Returns current number of blocks.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetNumOfBlocks(TUint& aNumOfBlocks)
    {
    aNumOfBlocks = iCurrentNumOfBlocks;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetNumOfBlocks [%d]"),
                  iCurrentNumOfBlocks);
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::SetAllocationMethod
 * Sets new allocation method.
 * (other items were commented in a header).
 */
EXPORT_C void CSbcEncoderIntfcCI::SetAllocationMethod(
                                  TSbcAllocationMethod aAllocationMethod)
    {
    iAllocationMethod = aAllocationMethod;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::SetAllocationMethod [%d]"),
                  iAllocationMethod);
#endif
    }

/*
 * CSbcEncoderIntfcCI::GetAllocationMethod
 * Returns current allocation method.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetAllocationMethod(
                                  TSbcAllocationMethod& aAllocationMethod)
    {
    aAllocationMethod = iCurrentAllocationMethod;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetAllocationMethod [%d]"),
                  iCurrentAllocationMethod);
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::SetBitpoolSize
 * Sets new bitpool size.
 * (other items were commented in a header).
 */
EXPORT_C void CSbcEncoderIntfcCI::SetBitpoolSize(TUint aBitpoolSize)
    {
    iBitpoolSize = aBitpoolSize;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::SetBitpoolSize [%d]"), iBitpoolSize);
#endif
    }

/*
 * CSbcEncoderIntfcCI::GetBitpoolSize
 * Returns current bitpool size.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::GetBitpoolSize(TUint& aBitpoolSize)
    {
    aBitpoolSize = iCurrentBitpoolSize;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::GetBitpoolSize [%d]"),
                  iCurrentBitpoolSize);
#endif
    return KErrNone;
    }

/*
 * CSbcEncoderIntfcCI::ApplyConfig
 * Commits new configutaion settings.
 * (other items were commented in a header).
 */
EXPORT_C TInt CSbcEncoderIntfcCI::ApplyConfig()
    {
	iCurrentSamplingFrequency = iSamplingFrequency;
	iCurrentChannelMode       = iChannelMode;
	iCurrentNumOfSubbands     = iNumOfSubbands;
	iCurrentNumOfBlocks       = iNumOfBlocks;
	iCurrentAllocationMethod  = iAllocationMethod;
	iCurrentBitpoolSize       = iBitpoolSize;

#ifdef _DEBUG
    RDebug::Print(_L("CSbcEncoderIntfcCI::ApplyConfig - SBC config applied"));
#endif
	return KErrNone;
    }

// End of File
