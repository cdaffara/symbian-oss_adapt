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
*/

#ifndef TSU_OMXIL_COMPONENT_CONSTANTS_H
#define TSU_OMXIL_COMPONENT_CONSTANTS_H

_LIT(KAacDecoderTestFile, "c:\\omxil\\testfiles\\probando123.hev2.aac");
_LIT(KAacDecoderOutputTestFile, "c:\\omxil\\testfiles\\omxilaacdecoderoutput.wav");

const TInt KTestHeapSize = 0x400000;	// 4 Mb;

// AudioSpecificConfig length (bytes) for probando123.hev2.aac
const TInt		KAudioSpecificConfigLength = 7;

// Wav header params... based on  probando123.hev2.aac...
const TInt		KTestWavFormatPCMChunkHeaderSize = 36;
const TInt		KTestWavFormatPCMSubchunk1Size	 = 16;
const TInt		KTestAudioFormatPCM				 = 1;
const TInt		KTestSampleRate					 = 44100;
const TInt		KTestNumChannels				 = 2;
const TInt		KTestBitsPerSample				 = 16;
const TInt		KTestBitRate					 = 1411000;

// These are two numbers that identify some input/output buffers in the middle
// of the processing of a test file
const TInt		KSomeInputBufferCount  = 15;
const TInt		KSomeOutputBufferCount = 25;

// Some time interval that can be used for timeouts...
const TInt      KTwoPointFiveSeconds = 2500000;

// Some other time interval that can be used for timeouts...
const TInt      KPointFiveSeconds = 500000;

#endif // TSU_OMXIL_COMPONENT_CONSTANTS_H
