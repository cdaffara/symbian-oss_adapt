/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Haptics test (adaptation) plugin implementation.
*
*/


#include <s32mem.h>
#include <hwrmhapticscommands.h>
#include "hwrmhapticsstubplugin.h"

// ---------------------------------------------------------------------------
// Static instantiation method.
// ---------------------------------------------------------------------------
//
CHWRMHapticsStubPlugin* CHWRMHapticsStubPlugin::NewL( 
                                    MHWRMHapticsPluginCallback* aCallback )
    {
    CHWRMHapticsStubPlugin* self = 
        new ( ELeave ) CHWRMHapticsStubPlugin( aCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CHWRMHapticsStubPlugin::~CHWRMHapticsStubPlugin()
    {
    iResponders.ResetAndDestroy();

    if ( iIdle )
        {
        iIdle->Cancel();
        delete iIdle;
        }
    }

// ---------------------------------------------------------------------------
// Implementation of ProcessCommandL. Creates an idle responder and appends it
// to the array of idle responders. (Later then, when the responder gets 
// runtime, it will initiate the response generation).
// ---------------------------------------------------------------------------
//
void CHWRMHapticsStubPlugin::ProcessCommandL( TInt aCommandId,
                                              TUint8 aTransId,
                                              TDesC8& aData )
    {
    if ( aCommandId != HWRMHapticsCommand::EHapticsCmdId ||
         aData.Size() < 2 ) // minimum length check
        {
        User::Leave( KErrBadDescriptor );
        }
        
    TUint8* dataPtr = const_cast<TUint8*>( aData.Ptr() );
    
    if ( !dataPtr )
        {
        User::Leave( KErrBadDescriptor );
        }
    
    CHWRMHapticsStubPluginIdleResponder* responder = 
        CHWRMHapticsStubPluginIdleResponder::NewL( this, aTransId, dataPtr );
    CleanupStack::PushL( responder );
    iResponders.AppendL( responder );
    CleanupStack::Pop ( responder );
    }

// ---------------------------------------------------------------------------
// Implementation of CancelCommandL. Just removes the corresponding idle 
// responder.
// ---------------------------------------------------------------------------
//
void CHWRMHapticsStubPlugin::CancelCommandL( TUint8 aTransId, 
                                             TInt /* aCommandId */ )
    {
    RemoveResponder( aTransId );
    }


// ---------------------------------------------------------------------------
// Method that does the actual response generation towards the issuer of 
// command i.e., the HapticsPluginManager
// ---------------------------------------------------------------------------
//
void CHWRMHapticsStubPlugin::GenerateResponseL( TUint8 aTransId,
                                                TUint8* aData )
    {
    // Hardcoded responses generated based on the command code (first 2 bytes
    // of the aData data packet, but since at the moment the 2nd byte is 
    // always 0x00, we just use the 1st one)
    TUint8 command = aData[0];

    TBuf8<KHWRMHapticsRespMaxSize> binRespData;
    binRespData.SetLength( KHWRMHapticsRespMaxSize );
    
    TInt i( 0 );
    // For each command the response data's first two bytes are copy of
    // the received command code (first 2 bytes of aData).
    
    binRespData[i++] = aData[0]; // command code bits 0..7
    binRespData[i++] = aData[1]; // command code bits 8..15
    
    switch ( command )
        {
        case 0x00: // API version query
            {
            // Response to API version query contains:
            //      1 byte major version number
            //      1 byte minor version number
            //      2 bytes for build version
            binRespData[i++] = 0x02; // major version
            binRespData[i++] = 0x00; // minor version
            binRespData[i++] = 0x53; // build version bits 0..7
            binRespData[i++] = 0x04; // build version bits 8..15
            break;    
            }

        case 0x01: // Initialize
        case 0x02: // Terminate
        case 0x06: // Modify basis effect
        case 0x07: // Stop effect
        case 0x08: // Stop all effects
        case 0x0B: // Reset debug buffer
        case 0x0C: // Stop designed bridge
        case 0x13: // Close device
        case 0x14: // Start designed bridge
        case 0x16: // Play streaming sample
        case 0x18: // Destroy streaming effect
        case 0x19: // Pause playing effect
        case 0x1A: // Resume playing effect
        case 0x1E: // Modify magsweep effect
        case 0x1F: // Modify periodic effect
            {
            // For these commands the response only contains status byte 
            // (set to 0x00 ("Success") in this stub).
            binRespData[i++] = 0x00; // status
            break;
            }
 
        case 0x03: // Play basis effect
        case 0x04: // Play IVT effect (with IVT data)
        case 0x05: // Play IVT effect (without IVT data)
        case 0x17: // Create streaming effect
        case 0x1C: // Play magsweep effect
        case 0x1D: // Play periodic effect
            {
            // The response consists of status byte (0x00) and 4-byte long
            // effectHandle (here hardcoded as 0x01000000)
            binRespData[i++] = 0x00; // status
            binRespData[i++] = 0x01; // effectHandle bits 0..7
            binRespData[i++] = 0x00; // effectHandle bits 8..15
            binRespData[i++] = 0x00; // effectHandle bits 16..23
            binRespData[i++] = 0x00; // effectHandle bits 24..31
            break;    
            }
            
        case 0x09: // Get device capabilities
            {
            // The response consists of status byte (0x00), 4-byte long
            // capability type (copied from the received command (bytes 
            // 7..10)), 1-byte capability  value type (copied from the 
            // received command (byte 2), 1-byte size byte and "size" bytes
            // long value
            binRespData[i++] = 0x00;      // status
            binRespData[i++] = aData[7];  // capability type bits 0..7
            binRespData[i++] = aData[8];  // capability type bits 8..15
            binRespData[i++] = aData[9];  // capability type bits 16..23
            binRespData[i++] = aData[10]; // capability type bits 24..31
            binRespData[i++] = aData[2];  // capability value type
            // The size and value depend on what exactly is being queried
            // Note: Currently only the lowest (0..7) bytes of capability
            //       type have meaning, thus the switch case below is 
            //       simplified..
            // Note: Since all values, except device name, are actually 32-bit
            //       integers, their size is always 4.
            TUint8 capabilityType = aData[7];
            switch ( capabilityType )
                {
                case 0: // device category
                    {
                    binRespData[i++] = 0x04; // size
                    binRespData[i++] = 0x02;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00; // value - virtual device
                    break;    
                    }
                case 1: // max nested repeats
                case 2: // num of actuators
                case 4: // num of effect slots
                case 6: // min period
                    {
                    binRespData[i++] = 0x04; // size
                    binRespData[i++] = 0x01;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00; // value - 0x1
                    break;  
                    }
                case 3: // actuator type
                    {
                    binRespData[i++] = 0x04; // size
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00; // value - ERM actuator
                    break;    
                    }
                case 5: // supported styles
                    {
                    binRespData[i++] = 0x04; // size
                    binRespData[i++] = 0x07;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00; // value - all styles   
                    break;
                    }
                case 7:  // max period 
                case 8:  // max effect duration
                case 11: // max envelope time
                case 13: // max IVT file size (tethered)
                case 14: // max IVT file size
                    {
                    binRespData[i++] = 0x04; // size
                    binRespData[i++] = 0xFF;
                    binRespData[i++] = 0xFF;
                    binRespData[i++] = 0x00; // 
                    binRespData[i++] = 0x00; // value => 0xFFFF.
                    break;    
                    }
                case 9: // supported effects
                    {
                    binRespData[i++] = 0x04; // size
                    binRespData[i++] = 0x07;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00; // value - mag|per|timeline   
                    break;
                    }
                case 10: // device name
                    {
                    binRespData[i++] = 0x04;  // size
                    binRespData[i++] = 0x52;  // value - byte 1, ascii "S"
                    binRespData[i++] = 0x53;  // value - byte 2, ascii "T"
                    binRespData[i++] = 0x54;  // value - byte 3, ascii "U"
                    binRespData[i++] = 0x42;  // value - byte 4, ascoo "B"
                    break;   
                    }
                case 12: // API version number
                    {
                    binRespData[i++] = 0x04; // size
                    binRespData[i++] = 0x02;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00; // value - API version 
                                             // Note: same as major version in
                                             // specific API version query
                                             // 0x2
                    break;
                    }
                default:
                    {
                    // Here status byte changed to -6 (0xFA)
                    // ("incompatible capability type")
                    binRespData[2]   = 0xFA; // status (re-assigned)
                    binRespData[i++] = 0x00; // value size set to zero
                    break;
                    }
                } // inner switch ends
            break;    
            }

        case 0x0A: // Get debug buffer
            {
            // The response consists of status byte (0x00) and 2 bytes
            // indicating the buffer size followed by the buffer itself.
            // Here buffer size zero is used => thus this only consists of
            // 3 bytes
            // FFS: This may have to be changed if the zero-length is not ok.
            binRespData[i++] = 0x00; // status
            binRespData[i++] = 0x00; // buffer size bits 0..7
            binRespData[i++] = 0x00; // buffer size bits 8..15
            break;    
            }

        case 0x0D: // Get device state
            {
            // The response consists of status byte (0x00) and 4 bytes that
            // indicate the device state (the bytes used here mean 
            // "device attached to the system")
            binRespData[i++] = 0x00; // status
            binRespData[i++] = 0x01; // device state bits 0..7
            binRespData[i++] = 0x00; // device state bits 8..15
            binRespData[i++] = 0x00; // device state bits 16..23
            binRespData[i++] = 0x00; // device state bits 24..31
            break;    
            }

        case 0x0E: // Set kernel param
            {
            // The response consists of status byte (0x00) and 2 bytes that
            // are the kernel param Id's copied from the received command 
            // (bytes 6 and 7)
            binRespData[i++] = 0x00;     // status
            binRespData[i++] = aData[6]; // kernel param Id bits 0..7
            binRespData[i++] = aData[7]; // kernel param Id bits 8..15
            break;    
            }

        case 0x0F: // Get kernel param
            {
            // The response consists of status byte (0x00), 2 bytes that are
            // the kernel param Id's copied from the received command 
            // (bytes 6&7) and two bytes for (here dummy 0x00) values
            binRespData[i++] = 0x00;     // status
            binRespData[i++] = aData[6]; // kernel param Id bits 0..7
            binRespData[i++] = aData[7]; // kernel param Id bits 8..15
            binRespData[i++] = 0x00;     // kernel param value bits 0..7
            binRespData[i++] = 0x00;     // kernel param value bits 8..15
            break;    
            }

        case 0x10: // Set device property
            {
            // The response consists of status byte (0x00), 4 bytes indicating
            // the property type (copied from the received command's bytes 
            // 6..9) and 1 byte indicating the property value type (copied
            // from the received command's byte 10).
            binRespData[i++] = 0x00;      // status
            binRespData[i++] = aData[6];  // property type bits 0..7
            binRespData[i++] = aData[7];  // property type bits 8..15
            binRespData[i++] = aData[8];  // property type bits 16..23
            binRespData[i++] = aData[9];  // property type bits 24..31
            binRespData[i++] = aData[10]; // property value type
            break;    
            }

        case 0x11: // Get device property
            {
            // The response consists of 1-byte status (0x00), 4-byte long
            // property type (copied from the received command bytes 7..10),
            // 1-byte property value type (copied from the received command
            // byte 2), 1-byte size byte and "size" bytes long value part
            binRespData[i++] = 0x00;      // status
            binRespData[i++] = aData[7];  // property type bits 0..7
            binRespData[i++] = aData[8];  // property type bits 8..15
            binRespData[i++] = aData[9];  // property type bits 16..23
            binRespData[i++] = aData[10]; // property type bits 24..31
            binRespData[i++] = aData[2];  // property value type
            // The size and value depend on what exactly is being queried
            // Note: Currently only the lowest (0..7) bytes of capability type
            //       have meaning, thus the switch case below is simplified
            TUint8 propertyType = aData[7];
            switch ( propertyType )
                {
                case 0: 
                    // license key.. not feasible as GET device property type
                    {
                    // re-assign the status
                    binRespData[2]   = 0xF9; // status - incompatible property
                                             //          type
                    binRespData[i++] = 0x00; // value size == 0 (=> no value)                    
                    break;
                    }
                case 1: // priority
                    {
                    binRespData[i++] = 0x04; // size (32-bit integer takes 4
                                             // bytes)
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00; // value - 0x0 is lowest priority
                    break;
                    }
                case 2: // disable effects
                    {
                    binRespData[i++] = 0x04; // size 
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00; // value - 0x0 means not disabled
                    break;
                    }
                case 3: // strength
                case 4: // master strength
                    {
                    binRespData[i++] = 0x04; // size 
                    binRespData[i++] = 0xFF;
                    binRespData[i++] = 0xFF;
                    binRespData[i++] = 0x00;
                    binRespData[i++] = 0x00; // => value 0xFFFF
                    break;
                    }
                default:
                    {
                    // Pehaps it's best to change the status byte to -7 (0xF9)
                    // ("incompatible property type")
                    binRespData[2]   = 0xF9; // status (re-assigned)
                    binRespData[i++] = 0x00; // value size, set to zero
                    break;    
                    }
                } // inner switch ends
            break;    
            }

        case 0x12: // Open device
            {
            // The response consists of status byte (0x00) and 4-byte long
            // deviceHandle (here hardcoded as 0x01000000)
            binRespData[i++] = 0x00; // status
            binRespData[i++] = 0x01; // deviceHandle bits 0..7
            binRespData[i++] = 0x00; // deviceHandle bits 8..15
            binRespData[i++] = 0x00; // deviceHandle bits 16..23
            binRespData[i++] = 0x00; // deviceHandle bits 24..31
            break;    
            }

        case 0x15: // Device count
            {
            // The response only contains the device count (here value == 1)
            binRespData[i++] = 0x01;
            break;    
            }

        case 0x1B: // Get effect state
            {
            // The response consists of status byte (0x00) and one byte effect
            // state (0x00 used here meaning "Not playing")
            binRespData[i++] = 0x00; // status
            binRespData[i++] = 0x00; // effect state 
            break;
            }
            
        case 0x20: // Load IVT data
            {
            // The response consists of status byte (0x00) and 4-byte long
            // fileHandle (here hardcoded as 0x01000000)
            binRespData[i++] = 0x00; // status
            binRespData[i++] = 0x01; // fileHandle bits 0..7
            binRespData[i++] = 0x00; // fileHandle bits 8..15
            binRespData[i++] = 0x00; // fileHandle bits 16..23
            binRespData[i++] = 0x00; // fileHandle bits 24..31
            break;    
            }

        case 0x30: // Get license key
            {
            binRespData[i++] = 0x00; // status
            binRespData[i++] = 0x20; // size
            for( TInt n=0; n<0x20; )
                {
                binRespData[i++] = 0x53; // S
                n++;
                binRespData[i++] = 0x54; // T
                n++;
                binRespData[i++] = 0x55; // U
                n++;
                binRespData[i++] = 0x42; // B
                n++;
                }
            break;
            }
            
        case 0xFF: // Protocol version query
            {
            // Response to Protocol version query contains:
            //      1 byte minor version number
            //      1 byte major version number
            binRespData[i++] = 0x00; // minor version 
            binRespData[i++] = 0x03; // major version
            break;    
            }
    
        default:
            {
            // Unknown command.. 
            break;    
            }
        } // switch ends 
        
    // set the lenght of the raw response data
    binRespData.SetLength( i );

    // create Haptics response data. This is done by externalizing 
    // (with RDesWriteStream whose sink is the Haptics response data)
    TInt32 respDataErr( KErrNone );
    CHWRMHapticsRespData* respData = 
        CHWRMHapticsRespData::NewLC( respDataErr, binRespData );
    HWRMHapticsCommand::RHWRMHapticsRespData resp;
    resp.CreateL( KHWRMHapticsRespMaxSize );
    CleanupClosePushL( resp );
    RDesWriteStream streamWriter( resp );
    CleanupClosePushL( streamWriter );
    streamWriter << *respData;
    // This commits to stream's sink (i.e., to 'resp')
    CleanupStack::PopAndDestroy( &streamWriter );
    
    // Call the ProcessResponseL of the HapticsPluginManager
    // object that created this stub plugin instance.
    iResponseCallback->ProcessResponseL
        ( HWRMHapticsCommand::EHapticsCmdId, aTransId, resp );    
    
    CleanupStack::PopAndDestroy( &resp );
    CleanupStack::PopAndDestroy( respData );
    // Order garbage collection of "spent" responder(s), if not already
    // waiting for scheduling
    if ( !iIdle->IsActive() )
        {
        iIdle->Start( TCallBack ( CollectGarbageIdle, this ) );    
        }         
    }

// ---------------------------------------------------------------------------
// Static method called as CIdle TCallBack in order to remove obsolete 
// responder from the array of responder pointers
// ---------------------------------------------------------------------------
//
TInt CHWRMHapticsStubPlugin::CollectGarbageIdle( TAny* aObjectPtr )
    {
    CHWRMHapticsStubPlugin* self = 
        reinterpret_cast<CHWRMHapticsStubPlugin*>( aObjectPtr );
    if ( self )
        {
        self->CollectGarbage();
        }
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Removes all responders that have finished their work from iResponders array
// ---------------------------------------------------------------------------
// 
void CHWRMHapticsStubPlugin::CollectGarbage()
    {
    for( TInt i( 0 ); i < iResponders.Count(); )
        {
        if ( iResponders[i]->Active() )
            {
            ++i; // skip this, it's still active.. 
            }
        else     
            {
            delete iResponders[i];
            iResponders.Remove( i );
            // note: array index i is not incremented as in the next round
            //       it already is the position of next item..
            //       obviously the iResponders.Count() will be then one less
            //       than in this round.
            }
        }
    }

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CHWRMHapticsStubPlugin::CHWRMHapticsStubPlugin( 
                                MHWRMHapticsPluginCallback* aCallback )     
    {
    // set callback to baseclass' member variable
    iResponseCallback = aCallback;
    }

// ---------------------------------------------------------------------------
// Two-phase construction ConstructL
// ---------------------------------------------------------------------------
//
void CHWRMHapticsStubPlugin::ConstructL()     
    {
    iIdle = CIdle::NewL( CActive::EPriorityIdle );

    // inform haptics of the plugin state
    iResponseCallback->PluginEnabled( EHWRMLogicalActuatorAny, ETrue );
    iResponseCallback->PluginEnabled( EHWRMLogicalActuatorDevice, ETrue );
    }

// ---------------------------------------------------------------------------
// Removes a specific responder (based on transId) from iResponders array
// ---------------------------------------------------------------------------
//
void CHWRMHapticsStubPlugin::RemoveResponder( TUint8 aTransId )
    {
    TInt count( iResponders.Count() );
    
    for( TInt i( 0 ); i < count; ++i )
        {
        if ( iResponders[i]->TransId() == aTransId )
            {
            delete iResponders[i];
            iResponders.Remove( i );
            break;
            }
        }
    }

// end of file

