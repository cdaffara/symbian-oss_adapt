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
* Description: Audio Stubs -  Implementation of the adaptation stub for
*              : MessageHandlerFactory.
*
*/

#include "MessageHandlerFactory.h"
#include "AudioInputMessageHandler.h"
#include "AudioOutputMessageHandler.h"
#include "AudioInputMessageTypes.h"
#include "AudioOutputMessageTypes.h"

#include "MessageHandlerFactory.h"
#include "AudioEqualizerMessageHandler.h"
#include "BassBoostMessageHandler.h"
#include "DistanceAttenuationMessageHandler.h"
#include "EnvironmentalReverbMessageHandler.h"
#include "ListenerDopplerMessageHandler.h"
#include "ListenerLocationMessageHandler.h"
#include "ListenerOrientationMessageHandler.h"
#include "LoudnessMessageHandler.h"
#include "RoomLevelMessageHandler.h"
#include "SourceDopplerMessageHandler.h"
#include "SourceLocationMessageHandler.h"
#include "SourceOrientationMessageHandler.h"
#include "StereoWideningMessageHandler.h"
#include "AddedDevSoundControlMsgHdlr.h"
#include "RestrictedAudioOutputMessageHandler.h"
#include "AudioOutputMessageHandler.h"

#include "SpeechEncoderConfigMsgHdlr.h"
#include "ErrorConcealmentIntfcMsgHdlr.h"
#include "G711DecoderIntfcMsgHdlr.h"
#include "G729DecoderIntfcMsgHdlr.h"
#include "IlbcDecoderIntfcMsgHdlr.h"
#include "G711EncoderIntfcMsgHdlr.h"
#include "G729EncoderIntfcMsgHdlr.h"
#include "IlbcEncoderIntfcMsgHdlr.h"
#include "AacDecoderConfigMsgHdlr.h"
//#include "AudioVibraControlMsgHdlr.h" // Audio Vibra Control API is deprecated

#include "SpeechEncoderConfig.h"
#include "AacDecoderConfig.h"
#include "ErrorConcealmentIntfc.h"
#include "G711DecoderIntfc.h"
#include "G729DecoderIntfc.h"
#include "IlbcDecoderIntfc.h"
#include "G711EncoderIntfc.h"
#include "G729EncoderIntfc.h"
#include "IlbcEncoderIntfc.h"
//#include "AudioVibraControl.h" // Audio Vibra Control API is deprecated

EXPORT_C CMMFObject* CMessageHandlerFactory::Create(TUid aInterfaceUid,
                                                    TAny* aCustomInterface,
                                                    CMMFObjectContainer& aContainer)
	{
	CMMFObject* messageHandler = NULL;

	if (aInterfaceUid == KUidAudioEqualizerEffect)
		{
		messageHandler = CAudioEqualizerMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidBassBoostEffect)
		{
		messageHandler = CBassBoostMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidDistanceAttenuationEffect)
		{
		messageHandler = CDistanceAttenuationMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidEnvironmentalReverbEffect)
		{
		messageHandler = CEnvironmentalReverbMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidListenerDopplerEffect)
		{
		messageHandler = CListenerDopplerMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidListenerLocationEffect)
		{
		messageHandler = CListenerLocationMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidListenerOrientationEffect)
		{
		messageHandler = CListenerOrientationMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidLoudnessEffect)
		{
		messageHandler = CLoudnessMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidRoomLevelEffect)
		{
		messageHandler = CRoomLevelMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidSourceDopplerEffect)
		{
		messageHandler = CSourceDopplerMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidSourceLocationEffect)
		{
		messageHandler = CSourceLocationMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidSourceOrientationEffect)
		{
		messageHandler = CSourceOrientationMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidStereoWideningEffect)
		{
		messageHandler = CStereoWideningMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidRestrictedAudioOutput)
		{
		messageHandler = CRestrictedAudioOutputMessageHandler::NewL(aCustomInterface);
		}
	else if (aInterfaceUid == KUidAudioOutput)
		{
		messageHandler = CAudioOutputMessageHandler::NewL(aCustomInterface, aContainer);
		}
	else if (aInterfaceUid == KUidAudioInput)
		{
		messageHandler = CAudioInputMessageHandler::NewL(aCustomInterface, aContainer);
		}
	else if (aInterfaceUid == KUidAudioOutput)
		{
		messageHandler = CAudioOutputMessageHandler::NewL(aCustomInterface, aContainer);
		}
   else if (aInterfaceUid == KUidSpeechEncoderConfig)
       {
       messageHandler = CSpeechEncoderConfigMsgHdlr::NewL(aCustomInterface);
       }
   else if (aInterfaceUid == KUidErrorConcealmentIntfc)
       {
       messageHandler = CErrorConcealmentIntfcMsgHdlr::NewL(aCustomInterface);
       }
   else if (aInterfaceUid == KUidG711DecoderIntfc)
       {
       messageHandler = CG711DecoderIntfcMsgHdlr::NewL(aCustomInterface);
       }
   else if (aInterfaceUid == KUidG729DecoderIntfc)
       {
       messageHandler = CG729DecoderIntfcMsgHdlr::NewL(aCustomInterface);
       }
   else if (aInterfaceUid == KUidIlbcDecoderIntfc)
       {
       messageHandler = CIlbcDecoderIntfcMsgHdlr::NewL(aCustomInterface);
       }
   else if (aInterfaceUid == KUidG711EncoderIntfc)
       {
       messageHandler = CG711EncoderIntfcMsgHdlr::NewL(aCustomInterface);
       }
   else if (aInterfaceUid == KUidG729EncoderIntfc)
       {
       messageHandler = CG729EncoderIntfcMsgHdlr::NewL(aCustomInterface);
       }
   else if (aInterfaceUid == KUidIlbcEncoderIntfc)
       {
       messageHandler = CIlbcEncoderIntfcMsgHdlr::NewL(aCustomInterface);
       }/*
    else if (aInterfaceUid == KUidAddedDevSoundControlInterface)
       {
       messageHandler = CAddedDevSoundControlMsgHdlr::NewL(aCustomInterface);
       }
	else if (aInterfaceUid == KUidAudioResource)
		{
		messageHandler = CAudioResourceMessageHandler::NewL(aCustomInterface, aContainer);
		}
	else if (aInterfaceUid == KUidAudioEqualizerEffect)
		{
		messageHandler = CAudioEqualizerMessageHandler::NewL(aCustomInterface);
		}
   else if (aInterfaceUid == KUidAudioVibraControl)
       {
       messageHandler = CAudioVibraControlMsgHdlr::NewL(aCustomInterface);
       }
   else if (aInterfaceUid == KUidAacDecoderConfig)
       {
       messageHandler = CAacDecoderConfigMsgHdlr::NewL(aCustomInterface);
       }*/

	return messageHandler;
	}


// End of File
