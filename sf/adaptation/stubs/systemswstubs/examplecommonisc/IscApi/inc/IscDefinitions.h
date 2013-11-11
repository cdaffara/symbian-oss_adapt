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
* Description:  ?Description
*
*/



#ifndef ISCDEFINITIONS_H
#define ISCDEFINITIONS_H

//  INCLUDES
#ifndef __KERNEL_MODE__
#include <e32std.h>
#else
#include <kernel.h>
#endif // __KERNEL_MODE__

// CONSTANTS
const TInt KIscNumberOfUnits = 64;

// uncomment definition to support channel sharing
//#define ISC_CHANNEL_SHARING_IN_USE

#ifdef ISC_CHANNEL_SHARING_IN_USE
// amount of how many users one channel can have
const TInt KIscMaxNumberOfChannelSharers = 3;
#else
const TInt KIscMaxNumberOfChannelSharers = 1;
#endif // ISC_CHANNEL_SHARING_IN_USE

const TInt KIscIniLineLength = 256;

// constant used by DIscMultiplexerBase::GetFrameInfo, 
// TIscFrameInfo.DataMode
enum TIscFrameType
    {
    EIscNonDataFrame,
    EIscDataFrame
    };

// frame concatenation enumeration
enum TIscFrameConcatenation
    {
    EIscNoConcatenation,
    EIscConcatenationDataStart,
    EIscConcatenationData,
    EIscConcatenationDataEnd
    };

/** Connection status enumeration. 
@see RIscApi::NotifyConnectionStatus
@see RIscApi::ConnectionStatus
@publishedPartner
@released
*/
enum TIscConnectionStatus
    {
    /**
    The connection is functional.
    */
    EIscConnectionOk=0,
    /**
    The connection to baseband modem is not functional.
    */
    EIscConnectionNotOk
    };

/** Flow control status enumeration. 
@see RIscApi::FlowControlStatus
@see RIscApi::NotifyFlowControl
@publishedPartner
@released
*/
enum TIscFlowControlStatus
    {
    /**
    Flow control is not active; i.e. the client can operate normally.
    */
    EIscFlowControlOff=0,
    /**
    Flow control is invoked, meaning that the user may no longer send data until the status is back to EIscFlowControlOff.
    */
    EIscFlowControlOn,
    /**
    Indicates that other end has finished sending data or the data connection has been removed.
    */
    EIscTransmissionEnd
    };

const TUint16 KIscControlChannel = 0x00;
const TUint16 KIscFirstChannel = 0x01;
const TUint16 KIscLastChannel = KIscNumberOfUnits-1;   // Maximun number of channels in ISC driver framework

const TUint16 KIscMaxChannelsInLdd = 0x20; // 32

const TUint16 KIscFrameReceiverNotFound = 0x500;

const TUint16 KIscAllChannels = 0x100;


// MACROS

// DATA TYPES

// enumerations for asynchronous requests in ISC
enum TIscAsyncRequest
    {
    EIscAsyncInitializeModemInterface,
    EIscAsyncOpen,
    EIscAsyncLastKernelServerContext, // last kernel server context call
    EIscAsyncSend,
    EIscAsyncReceive,
    EIscAsyncDataSend,
    EIscAsyncDataReceive,
    EIscAsyncNotifyConnectionStatus,
    EIscAsyncNotifyFlowControlStatus,
    EIscAsyncCustomOperation1,
    EIscAsyncCustomOperation2,
    EIscAsyncCustomOperation3,
    EIscAsyncCustomOperation4,
    EIscAsyncCustomOperation5,
    EIscAsyncClose,
    EIscAsyncLast
    }; 

// enumerations for synchronous requests in ISC
enum TIscSyncRequest
    {
    EIscSyncClose = EIscAsyncLast,  // 15
    EIscSyncLastKernelServerContext,// last kernel server context call
    EIscSyncSend,
    EIscSyncDataSend,
    EIscSyncGetConnectionStatus,
    EIscSyncGetFlowControlStatus,
    EIscSyncGetChannelInfo,
    EIscSyncGetMaximunDataSize,
    EIscSyncCustomOperation1,
    EIscSyncCustomOperation2,
    EIscSyncCustomOperation3,
    EIscSyncCustomOperation4,
    EIscSyncCustomOperation5,
    EIscSyncResetBuffers,
    EIscSyncLast
    };

// enumerations for canceling asynchronous requests in ISC
enum TIscCancelRequest
    {
    EIscCancelAsyncInitialize = EIscSyncLast,
    EIscCancelAsyncOpen,
    EIscCancelAsyncSend,
    EIscCancelAsyncDataSend,
    EIscCancelAsyncReceive,
    EIscCancelAsyncDataReceive,
    EIscCancelAsyncNotifyConnection,
    EIscCancelAsyncNotifyFlowControl,
    EIscCancelAsyncCustomOperation1,
    EIscCancelAsyncCustomOperation2,
    EIscCancelAsyncCustomOperation3,
    EIscCancelAsyncCustomOperation4,
    EIscCancelAsyncCustomOperation5,
    EIscCancelAsyncClose,
    EIscCancelLast
    };

// Panic values used in ASSERT_RESET macro
// Can be seen as "FAULT [component] [TIscIfFault]" in trace or crash debugger
// e.g "FAULT IscDriver: 0x1"
enum TIscIfFault
    {
    EIscPanicSetChannel=1,              // 1
    EIscPanicCreateLogicalDevice,       // 2
    EIscMainRcvBufferOverflow,          // 3
    EIscNotAllowedCallToDoRequest,      // 4
    EIscNotAllowedCallToDoCancel,       // 5
    EIscUnknownCommand,                 // 6
    EIscNullIscDriverPointer,           // 7
    EIscMemoryAllocationFailure,        // 8
    EIscRequestAlreadyActive,           // 9    
    EIscDriverNotFound,                 // 10
    EIscLogicalDevicesNotFound,         // 11
    EIscMultiplexerNotFound,            // 12
    EIscDataTransmissionDriverNotFound, // 13
    EIscMainRcvBufferInitialize,        // 14
    EIscBufferAllocationFailure,        // 15
    EIscControlBufferOverflow,          // 16
    EIscInvalidChannelPtr               // 17 only in debug mode
    }; 

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

#endif      // ISCDEFINITIONS_H
            
// End of File
