// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//



/**
	@file
	@internalComponent
*/

#ifndef OMXILMICSOURCECONST_H_
#define OMXILMICSOURCECONST_H_

#include <d32soundsc.h>

_LIT8(KSymbianOmxILMicSourceName, "OMX.NOKIA.AUDIO.CAPTURER.PCM");
_LIT8(KSymbianOmxILMicSourceRole, "audio_capturer.pcm");

_LIT(KSndLddFileName,"ESOUNDSC.LDD");
_LIT(KSndPddFileName,"SOUNDSC.PDD");
_LIT(KSndPddWildcardExtension,".*");

const TUint8 KMicSourceComponentVersionMajor = 1;
const TUint8 KMicSourceComponentVersionMinor = 0;
const TUint8 KMicSourceComponentVersionRevision = 0;
const TUint8 KMicSourceComponentVersionStep = 0;

const OMX_U32 KMICSOURCE_APB0PORT_INDEX = 0;
const OMX_U32 KMICSOURCE_OPB0PORT_INDEX = 1;

const OMX_U32 KMICSOURCE_VOLUME_MIN = 0;
const OMX_U32 KMICSOURCE_VOLUME_MAX = KSoundMaxVolume;

const OMX_U32 KMICSOURCE_PCMPORT_BUFFERCOUNT_MIN = 4;
const OMX_U32 KMICSOURCE_PCMPORT_BUFFERSIZE_MIN = 15360; // TODO: Decide the actual size of it, currently just copied from PCM render. 
const OMX_U32 KMICSOURCE_CLOCKPORT_BUFFERCOUNT_MIN = 3;

#ifndef MICSOURCE_DYNAMIC_SETTINGS
//Default current sound settings 
const TUint32 			KMICSOURCE_VOLUME_DEFAULT 			= 127;
const TInt 				KMICSOURCE_CHANNELS_DEFAULT 		= 2;
const TSoundRate 		KMICSOURCE_SOUNDRATE_DEFAULT 		= ESoundRate48000Hz;
const TSoundEncoding 	KMICSOURCE_SOUNDENCODING_DEFAULT 	= ESoundEncoding16BitPCM;
const TSoundDataFormat  KMICSOURCE_SOUNDDATAFORMAT_DEFAULT 	= ESoundDataFormatInterleaved;

//Default supported sound settings
const TUint32 KMICSOURCE_CHANNELS_SUPPORT = KSoundMonoChannel | KSoundStereoChannel;
const TUint32 KMICSOURCE_SAMPLERATES_SUPPORT = KSoundRate7350Hz | KSoundRate8000Hz | KSoundRate8820Hz |
						KSoundRate9600Hz | KSoundRate11025Hz | KSoundRate12000Hz | KSoundRate14700Hz |
						KSoundRate16000Hz | KSoundRate22050Hz | KSoundRate24000Hz | KSoundRate29400Hz |
						KSoundRate32000Hz | KSoundRate44100Hz | KSoundRate48000Hz;

const TUint32 KMICSOURCE_ENCODING_SUPPORT = KSoundEncoding8BitPCM | KSoundEncoding16BitPCM;
const TUint32 KMICSOURCE_DATAFORMATS_SUPPORT = KSoundDataFormatInterleaved;
const TSoundDirection KMICSOURCE_SOUNDDIRECTION = ESoundDirRecord;
const TBool KMICSOURCE_HWCONFIGNOTIFICATION_SUPPORT = EFalse;
const TInt KMICSOURCE_REQUESTMINSIZE_SUPPORT = 0;
const TInt KMICSOURCE_REQUESTALIGNMENT_SUPPORT = 0;
#endif //MICSOURCE_DYNAMIC_SETTINGS

#endif /*OMXILMICSOURCECONST_H_*/
