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
* Description: Audio Stubs -
*
*/



// INCLUDE FILES
#include "WmaDecoderIntfcCI.h"
#include <e32debug.h>

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

// -----------------------------------------------------------------------------
// CWmaDecoderIntfcCI::CWmaDecoderIntfcCI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWmaDecoderIntfcCI::CWmaDecoderIntfcCI()
    {

    }

// -----------------------------------------------------------------------------
// CWmaDecoderIntfcCI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWmaDecoderIntfcCI::ConstructL()
    {
	iCurrentFormat = EWma;
    iCurrentBitsPerSample = 8;
    iCurrentNumChannelsIn = 2;
    iCurrentSamplesPerSec = 4600;
    iCurrentAvgBytesPerSec = 3600;
    iCurrentBlockAlign = 4945;
    iCurrentEncodeOptions = 5;
    iCurrentEncodeOptions1 = 5;
    iCurrentEncodeOptions2 = 5;

    iCurrentChannelMask = 2;
    iCurrentSupportedMaxSampleRate = 8900;
    iCurrentSupportedMaxChannelsIn = 5678;
    iCurrentSupportedMaxBitrate = 4356;
    iCurrentEnable = CWmaDecoderIntfc::EToolOutput32Bit;
    iCurrentDisable = CWmaDecoderIntfc::EDownMixToStereo;
/*
    iSupportedFormats.Append(EWma);
    iSupportedFormats.Append(EWmaPro);

    iSupportedTools.Append(EDownMixToStereo);
    iSupportedTools.Append(ELostDataConcealment);
    iSupportedTools.Append(EToolOutput32Bit);

    iControllableTools.Append(EToolOutput32Bit);
    iControllableTools.Append(EDownMixToStereo);
    iControllableTools.Append(EToolOutput32Bit);
    iControllableTools.Append(ELostDataConcealment);
 */

    iCurrentSupportedFormats.Append(EWma);
    iCurrentSupportedFormats.Append(EWmaPro);

    iCurrentSupportedTools.Append(EDownMixToStereo);
    iCurrentSupportedTools.Append(ELostDataConcealment);
   // iCurrentSupportedTools.Append(EToolOutput32Bit);

    iCurrentControllableTools.Append(EToolOutput32Bit);
   // iCurrentControllableTools.Append(EDownMixToStereo);
   //iCurrentControllableTools.Append(EToolOutput32Bit);
   // iCurrentControllableTools.Append(ELostDataConcealment);

       // iCurrentControllableTools;


    }

// -----------------------------------------------------------------------------
// CWmaDecoderIntfcCI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CWmaDecoderIntfcCI* CWmaDecoderIntfcCI::NewL()
    {
	CWmaDecoderIntfcCI* self = new (ELeave)CWmaDecoderIntfcCI;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

// Destructor
EXPORT_C CWmaDecoderIntfcCI::~CWmaDecoderIntfcCI()
    {
       iCurrentSupportedFormats.Close();
       iCurrentSupportedTools.Close();
       iCurrentControllableTools.Close();
/*
       iSupportedFormats.Close();
        iSupportedTools.Close();
         iControllableTools.Close();
  */
    }

// -----------------------------------------------------------------------------
// CWmaDecoderIntfcCI::SetAudioConfig
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
// From CWmaDecoderIntfc
/**
 * Configures format.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetFormat(TFormat aFormat)
    {
    iCurrentFormat = aFormat;
    }

/**
 * Configures decoder's bits per sample.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetBitsPerSampleIn(TUint aBitsPerSample)
    {
    iCurrentBitsPerSample = aBitsPerSample;
    }

/**
 * Configures decoder's number of channels.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetNumChannelsIn (TUint aNumChannelsIn)
    {
    iCurrentNumChannelsIn = aNumChannelsIn;
    }

/**
 * Configures decoder's sample rate.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetSamplesPerSec (TUint aSamplesPerSec)
    {
    iCurrentSamplesPerSec = aSamplesPerSec;
    }

/**
 * Configures decoder's average bytes per second.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetAvgBytesPerSec (TUint aAvgBytesPerSec)
    {
    iCurrentAvgBytesPerSec = aAvgBytesPerSec;
    }

/**
 * Configures decoder's block align.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetBlockAlign (TUint aBlockAlign)
    {
    iCurrentBlockAlign = aBlockAlign;
    }

/**
 * Configures decoder's Encode Options.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetEncodeOptions (TUint aEncodeOptions)
    {
    iCurrentEncodeOptions = aEncodeOptions;
    }

/**
 * Configures decoder's Advance Encode Options.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetEncodeOptions1 (TUint aEncodeOptions)
    {
    iCurrentEncodeOptions1 = aEncodeOptions;
    }

/**
 * Configures decoder's Advance Encode Options.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetEncodeOptions2 (TUint aEncodeOptions)
    {
    iCurrentEncodeOptions2 = aEncodeOptions;
    }

/**
 * Configures decoder's Channel Mask.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::SetChannelMaskIn (TUint aChannelMask)
    {
    iCurrentChannelMask = aChannelMask;
    }

/**
 * Returns decoder's current configured format.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetFormat (TFormat& aFormat)
    {
    TInt status = KErrNone;
    aFormat = iCurrentFormat;
/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
*/
    return status;
    }

/**
 * Returns configured bits per sample .
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetBitsPerSampleIn(TUint& aBitsPerSample)
    {
    TInt status = KErrNone;
   	aBitsPerSample = iCurrentBitsPerSample;
/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
  */
    return status;
    }

/**
 * Returns configured number of channels.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetNumOfChannelsIn (TUint& aNumOfChannels)
    {
    TInt status = KErrNone;
    aNumOfChannels = iCurrentNumChannelsIn;

/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
  */
    return status;
    }

/**
 * Returns configured sample rate.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetSamplesPerSec (TUint& aSamplesPerSec)
    {
    TInt status = KErrNone;
   	aSamplesPerSec = iCurrentSamplesPerSec;
/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
  */
    return status;
    }

/**
 * Returns configured average bytes per second.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetAvgBytesPerSec (TUint& aAvgBytesPerSec)
    {
    TInt status = KErrNone;
   	aAvgBytesPerSec = iCurrentAvgBytesPerSec;
/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
  */

    return status;
    }

/**
 * Returns configured block align.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetBlockAlign (TUint& aBlockAlign)
    {
    TInt status = KErrNone;
    aBlockAlign = iCurrentBlockAlign;
/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
*/
    return status;
    }

/**
 * Returns configured encode options.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetEncodeOptions (TUint& aEncodeOpts)
    {
    TInt status = KErrNone;
   	aEncodeOpts = iCurrentEncodeOptions;
/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
  */
    return status;
    }

/**
 * Returns configured advanced encode options.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetEncodeOptions1 (TUint& aEncodeOpts1)
    {
    TInt status = KErrNone;
   	aEncodeOpts1 = iCurrentEncodeOptions1;

/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound ||
             iApplyConfStatus == KErrNotSupported)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
*/

    return status;
    }

 /**
 * Returns configured advanced encode options.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetEncodeOptions2 (TUint& aEncodeOpts2)
    {
    TInt status = KErrNone;
   	aEncodeOpts2 = iCurrentEncodeOptions2;

/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound ||
             iApplyConfStatus == KErrNotSupported)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
*/

    return status;
    }

/**
 * Returns configured channel mask.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetChannelMaskIn (TUint& aChannelMask)
    {
    TInt status = KErrNone;
   	aChannelMask = iCurrentChannelMask;
/*
    else if (iApplyConfStatus == KErrNone  ||  iApplyConfStatus == KErrCompletion ||
             iApplyConfStatus == KErrUnknown ||  iApplyConfStatus == KErrArgument ||
             iApplyConfStatus == KErrNotReady || iApplyConfStatus == KErrNotFound ||
             iApplyConfStatus == KErrNotSupported)
        {
        status = iApplyConfStatus;
        }
    else
    	{
    	status = KErrGeneral;
    	}
*/
    return status;
    }

/**
 * Returns supported formats.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetSupportedFormats(RArray<TFormat>& aSupportedFormats)
    {
    TInt status = KErrNone;
    TInt num = iCurrentSupportedFormats.Count();
    aSupportedFormats.Reset();
    for (int i = 0; i < num; ++i)
    {
       	aSupportedFormats.Append(iCurrentSupportedFormats[i]);

    }
    return status;
    }


/**
 * Returns supported tools.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetSupportedTools(RArray<TTool>& aSupportedTools)
    {
    TInt status = KErrNone;
   TInt num = iCurrentSupportedTools.Count();
   aSupportedTools.Reset();
    for (int i = 0; i < num; ++i)
    {
        aSupportedTools.Append(iCurrentSupportedTools[i]);

    }
    return status;
    }

/**
 * Returns supported max number of channels.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetSupportedMaxChannelsIn(TUint& aSupportedMaxChannelsIn)
    {
    TInt status = KErrNone;
   	aSupportedMaxChannelsIn = iCurrentSupportedMaxChannelsIn;
    return status;
    }

/**
 * Returns supported max bit rate.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetSupportedMaxBitrate(TUint& aSupportedMaxBitrate)
    {
    TInt status = KErrNone;
   	aSupportedMaxBitrate = iCurrentSupportedMaxBitrate;
    return status;
    }

/**
 * Returns supported max sample rate.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetSupportedMaxSampleRate(TUint& aSupportedMaxSampleRate)
    {
    TInt status = KErrNone;
    aSupportedMaxSampleRate = iCurrentSupportedMaxSampleRate;
    return status;
    }

/**
 * Returns controllable tools.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetControllableTools(RArray<TTool>& aControllableTools)
    {
    TInt status = KErrNone;
    TInt num = iCurrentControllableTools.Count();
    aControllableTools.Reset();
    for (int i = 0; i < num; ++i)
    {
        aControllableTools.Append(iCurrentControllableTools[i]);

    }
    return status;
    }

/**
 * Enable tool.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::EnableTool(TTool aTool)
    {
	switch (aTool)
		{
    	case EToolOutput32Bit:
    	     iCurrentToolOutPut32Bit = ETrue;
    	     break;
    	case EDownMixToStereo:
             iCurrentToolDownMixToStereo = ETrue;
             break;
    	case ELostDataConcealment:
    	     iCurrentToolLostDataConcealment = ETrue;
    	     break;
    	default:
    	     break;
    }
    }
/**
 * disable tool.
 *
 */
EXPORT_C void CWmaDecoderIntfcCI::DisableTool(TTool aTool)
    {
	switch (aTool)
		{
    	case EToolOutput32Bit:
    	     iCurrentToolOutPut32Bit = EFalse;
    	     break;
    	case EDownMixToStereo:
             iCurrentToolDownMixToStereo = EFalse;
             break;
    	case ELostDataConcealment:
    	     iCurrentToolLostDataConcealment = EFalse;
    	     break;
    	default:
    	     break;
		}
    }


/**
 * Returns tool state.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::GetTool(TTool aTool, TBool& aEnabled)
    {

    TInt status = KErrNone;
	switch (aTool)
		{
    	case EToolOutput32Bit:
    	     if (iCurrentToolOutPut32Bit)
    	     	{
    	     	aEnabled = ETrue;
    	     	}
    	     else
    	     	{
     	     	aEnabled = EFalse;
    	     	}
    	     break;

    	case EDownMixToStereo:
    	     if (iCurrentToolDownMixToStereo)
    	     	{
    	     	aEnabled = ETrue;
    	     	}
    	     else
    	     	{
     	     	aEnabled = EFalse;
    	     	}
			 break;

    	case ELostDataConcealment:
    	     if (iCurrentToolLostDataConcealment)
    	     	{
    	     	aEnabled = ETrue;
    	     	}
    	     else
    	     	{
     	     	aEnabled = EFalse;
    	     	}
    	     break;
    	default:
    	     break;
		}

return status;
}

/**
 * Applies configuration settings to the decoder.
 *
 */
EXPORT_C TInt CWmaDecoderIntfcCI::ApplyConfig()
    {
	iCurrentFormat = iFormat;
    iCurrentBitsPerSample = iBitsPerSample;
    iCurrentNumChannelsIn = iNumChannelsIn;
    iCurrentSamplesPerSec = iSamplesPerSec;
    iCurrentAvgBytesPerSec = iAvgBytesPerSec;
    iCurrentBlockAlign = iBlockAlign;
    iCurrentEncodeOptions = iEncodeOptions;

	iCurrentEncodeOptions1 = iEncodeOptions1;
	iCurrentEncodeOptions2 = iEncodeOptions2;

    iCurrentChannelMask = iChannelMask;
    iCurrentSupportedMaxSampleRate = iSupportedMaxSampleRate;
    iCurrentSupportedMaxChannelsIn = iSupportedMaxChannelsIn;
    iCurrentSupportedMaxBitrate = iSupportedMaxBitrate;
    iCurrentEnable = iEnable;
    iCurrentDisable = iDisable;

            iCurrentToolOutPut32Bit = iToolOutPut32Bit;
            iCurrentToolDownMixToStereo = iToolDownMixToStereo;
            iCurrentToolLostDataConcealment = iToolLostDataConcealment;


	return KErrNone;
    }


/*
 	iCurrentFormat = EWma;
    iCurrentBitsPerSample = 8;
    iCurrentNumChannelsIn = 2;
    iCurrentSamplesPerSec = 4600;
    iCurrentAvgBytesPerSec = 3600;
    iCurrentBlockAlign = 4945;
    iCurrentEncodeOptions = 5;
    iCurrentEncodeOptions1 = 5;
    iCurrentEncodeOptions2 = 5;

    iCurrentChannelMask = 2;
    iCurrentSupportedMaxSampleRate = 8900;
    iCurrentSupportedMaxChannelsIn = 5678;
    iCurrentSupportedMaxBitrate = 4356;
    iCurrentEnable = CWmaDecoderIntfc::EToolOutput32Bit;
    iCurrentDisable = CWmaDecoderIntfc::EDownMixToStereo;

     iCurrentSupportedFormats[0] = EWma;
    iCurrentSupportedFormats[1] = EWmaPro;

    iCurrentSupportedTools[0] = EToolOutput32Bit;
    iCurrentSupportedTools[1] = EToolOutput32Bit;
    iCurrentSupportedTools[2] = EToolOutput32Bit;
*/
