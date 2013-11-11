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
* Description:  Used as an reference implementation
*
*/


#ifndef ASYMESSAGEDEFINITIONS_H
#define ASYMESSAGEDEFINITIONS_H

//  INCLUDES
#include <bldvariant.hrh>
#include <AsyCmdTypes.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

const TUint32 KASYMessageGroupAccessoryMessage  = 0x00010001; /* Message group for Accessory messages */
const TUint32 KASYMessageGroupAccessoryEvent    = 0x00010002; /* Message group for Accessory events */


typedef TBuf8< 15 > TASYAccessoryFeatureData;   // Data

namespace AccessoryConnection
    {
    enum TConnectionState
        {
        EStateNotConnected = 1,
        EStateConnected
        };

    NONSHARABLE_STRUCT( TConnectionStateInfoStruct )
        {
        TInt32              iAccessoryId;       // specific accessory ID
        TConnectionState    iState;             // Accessory connection state
        };
        
    NONSHARABLE_STRUCT( TConnectionStateInfoRetStruct )
        {
        TConnectionStateInfoStruct  iStateInfo;
        TInt                        iErrorCode;
        };  
    }

namespace AccessoryFeature
    {
    enum TAccessoryFeatureType
        {
        ETypeNotSupported = 0,
        ETypeBoolean,
        ETypeData
        };

    enum TAccessoryFeature
        {
        ENotSupported = 0,
        EHeadphonesConnected,
        EExternalPowerSourceConnected,
        };

    // Data type for getter (boolean and data)
    NONSHARABLE_STRUCT( TFeatureStateGetStruct )
        {
        TInt32                              iAccessoryId;       // specific accessory ID
        TAccessoryFeature                   iFeature;           // Feature
        };
    
    // Data type for boolean feature
    NONSHARABLE_STRUCT( TFeatureStateBooleanStruct )
        {
        TInt32                              iAccessoryId;       // specific accessory ID
        TAccessoryFeature                   iFeature;           // Feature
        TBool                               iState;             // Feature state (boolean)
        };   

    NONSHARABLE_STRUCT( TFeatureStateBooleanRetStruct )
        {
        TFeatureStateBooleanStruct iFeatureState;
        TInt                                iErrorCode;
        };

    }

// Example messages
// -----------------------------------------------------------------------------
    const TUint32 KASYMsgAccessoryConnectionHandlerInitCmd         = 0x00000001;

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    const TUint32 KASYMsgAccessoryConnectionHandlerInitRet         = 0x00000002;
    typedef TPckgBuf< AccessoryConnection::TConnectionStateInfoRetStruct > TASYMsgAccessoryConnectionHandlerInitRetBuf;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    const TUint32 KASYMsgAccessoryGetValueBooleanCmd               = 0x00000003;

    typedef TPckgBuf< AccessoryFeature::TFeatureStateGetStruct > TASYMsgAccessoryGetValueBooleanCmdBuf;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    const TUint32 KASYMsgAccessoryGetValueBooleanRet               = 0x00000004;

    typedef TPckgBuf< AccessoryFeature::TFeatureStateBooleanRetStruct > TASYMsgAccessoryGetValueBooleanRetBuf;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    const TUint32 KASYMsgAccessorySetValueBooleanCmd               = 0x00000005;

    typedef TPckgBuf< AccessoryFeature::TFeatureStateBooleanStruct > TASYMsgAccessorySetValueBooleanCmdBuf;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    const TUint32 KASYMsgAccessorySetValueBooleanRet               = 0x00000006;

    typedef TPckgBuf< AccessoryFeature::TFeatureStateBooleanRetStruct > TASYMsgAccessorySetValueBooleanRetBuf;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    const TUint32 KASYMsgAccessoryConnectionStateChangedEvent      = 0x00000007;

    typedef TPckgBuf< AccessoryConnection::TConnectionStateInfoStruct > TASYMsgAccessoryConnectionStateChangedEventBuf;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    const TUint32 KASYMsgAccessoryFeatureStateChangedEvent         = 0x00000008;

    typedef TPckgBuf< AccessoryFeature::TFeatureStateBooleanStruct > TASYMsgAccessoryFeatureStateChangedEventBuf;
// -----------------------------------------------------------------------------

#endif      // ASYMESSAGEDEFINITIONS_H
            
// End of File
