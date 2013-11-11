/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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



#ifndef __CSI_PSL_H__
#define __CSI_PSL_H__

const TUint KInterruptsAll = 0xf111;

extern TInt32 csiTimeoutValue;

// table of pointers to the channels - to be filled with pointers to channels
// and used to register these channels with the Bus Controller
// (declaration-to be included by master/slave)
extern DIicBusChannel** ChannelPtrArray;

// helper function - to dump the supplied configuration
#ifdef _DEBUG
void DumpConfiguration(const TConfigSpiBufV01& spiHeader, TInt aCsPin);
#endif

// a bit-field to store the current mode of operation
struct TCsiOperationType
    {
    enum TOperation
        {
        ENop             = 0x00,
        ETransmitOnly    = 0x01,
        EReceiveOnly     = 0x02,
        ETransmitReceive = 0x03
        };

    struct TOp
        {
        TUint8 iIsTransmitting : 1;
        TUint8 iIsReceiving    : 1;
        TUint8 iRest           : 6;
        };

    union
        {
        TOp iOp;
        TUint8 iValue;
        };
    };

#endif /*__CSI_PSL_H__*/
