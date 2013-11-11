/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


/**
 @file
 @internalComponent
*/


#ifndef OMXILPCMRENDERERCONST_H_
#define OMXILPCMRENDERERCONST_H_

#include "mdasoundadapter.h"

_LIT8(KSymbianOmxILPcmRendererName, "OMX.NOKIA.AUDIO.RENDERER.PCM");
_LIT8(KSymbianOmxILPcmRendererRole, "audio_renderer.pcm");
_LIT8(KMimeTypeAudioPcm, "audio/pcm");

const OMX_U32 KPCMRENDERER_APB0PORT_INDEX = 0;
const OMX_U32 KPCMRENDERER_OPB0PORT_INDEX = 1;

const TInt KBufferSize = 16384;   // we need 16k to hold a pcm packet
const TInt KDefaultSampleRate = 48000; // check out ./boardsupport/omap3variants/tiomap3bsp/variant/common/soundsc/variant_sound.cpp to find out the supporting rates
const TInt KDefaultNumberChannels = 2;
const TInt KDefaultBitPerSample = 16;
const TInt KMaxVolume = 255;
const TInt KMedianVolume = 226;
const TInt KDefaultVolume = KMedianVolume;
const TInt KMinBufferMilliseconds = 100; // Minimum number of milliseconds allowed in a buffer

// Timeout to call RSoundSc::TimePlayed() every second
// Under normal operation, the timer will return exactly one sec.
// It is particular useful to compare the value from TimePlayed()
static const TInt KPcmRendererTimePlayedDelay = 998113;

const TBool KDefaultMuted = EFalse;

const RMdaDevSound::TMdaSoundEncoding KDefaultEncoding = RMdaDevSound::EMdaSoundEncoding16BitPCM;

/**
 * Custom index used by Symbian's OpenMAX IL PCM renderer to select the
 * structure for setting/getting a Volume Ramp configuration data item
 */
#define OMX_SYMBIANINDEXCONFIGAUDIOPCMVOLUMERAMP 0x7F000002

/**
 * Custom index used by Symbian's OpenMAX IL PCM renderer to select the
 * structure for getting a Data Amount configuration data item
 */
#define OMX_SYMBIANINDEXCONFIGAUDIODATAAMOUNT 0x7F000003

#endif /*OMXILPCMRENDERERCONST_H_*/
