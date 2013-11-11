/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Reference implementation of SSY Channel Data Provider interface
*
*/


#include "ssyreferencechanneldataprovider.h"
#include "ssyreferencetrace.h"
#include "ssyreferencechannel.h"
#include "ssyreferencecontrol.h"
#include "ssyreferencecmdhandler.h"
#include "ssycallback.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSsyReferenceChannelDataProvider C++ constructor
// ---------------------------------------------------------------------------
//
CSsyReferenceChannelDataProvider::CSsyReferenceChannelDataProvider( CSsyReferenceChannel& aChannel ) :
    iChannel( aChannel )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::CSsyReferenceChannelDataProvider()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::CSsyReferenceChannelDataProvider() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// Symbian 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannelDataProvider::ConstructL()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::ConstructL()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::ConstructL() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// CSsyReferenceChannelDataProvider::NewL
// ---------------------------------------------------------------------------
//
CSsyReferenceChannelDataProvider* CSsyReferenceChannelDataProvider::NewL( CSsyReferenceChannel& aChannel )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::NewL()" ) ) );
    CSsyReferenceChannelDataProvider* self = new ( ELeave ) CSsyReferenceChannelDataProvider( aChannel );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::NewL() - return" ) ) );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CSsyReferenceChannelDataProvider::~CSsyReferenceChannelDataProvider()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::~CSsyReferenceChannelDataProvider()" ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::~CSsyReferenceChannelDataProvider() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannelDataProvider::StartChannelDataL
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannelDataProvider::StartChannelDataL( 
    const TSensrvChannelId aChannelId, 
    TUint8* aBuffer, 
    TInt aCount )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::StartChannelDataL()" ) ) );

    if ( iChannel.ChannelId() != aChannelId )
        {
        User::Leave( KErrNotFound );
        }

    // Store buffer pointer
    iDataBuffer = aBuffer;
    iMaxCount = aCount;
    iDataCount = 0;

    // Udpate channel state
    iChannel.UpdateState( CSsyReferenceChannel::ESsyReferenceChannelReceiving );

    // Start receiving
    iChannel.CommandHandler().ProcessCommand( TSsyReferenceMsg( aChannelId, ESsyReferenceStartChannelData ) );

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::StartChannelDataL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannelDataProvider::StopChannelDataL
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannelDataProvider::StopChannelDataL( const TSensrvChannelId aChannelId )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::StopChannelDataL()" ) ) );

    // Leave if wrong channel
    if ( iChannel.ChannelId() != aChannelId )
        {
        User::Leave( KErrNotFound );
        }

    // Udpate channel state
    iChannel.UpdateState( CSsyReferenceChannel::ESsyReferenceChannelOpen );

    // Stop receiving
    iChannel.CommandHandler().ProcessCommand( TSsyReferenceMsg( aChannelId, ESsyReferenceStopChannelData ) );

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::StopChannelDataL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannelDataProvider::ForceBufferFilledL
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannelDataProvider::ForceBufferFilledL( const TSensrvChannelId aChannelId )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::ForceBufferFilledL()" ) ) );

    // Leave if wrong channel
    if ( iChannel.ChannelId() != aChannelId )
        {
        User::Leave( KErrNotFound );
        }

    // Send current buffer. Channel keeps receiveing
    SendBufferFilled();

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::ForceBufferFilledL() - return" ) ) );
    }

// -----------------------------------------------------------------------------
// CSensrvTestCases::GetChannelDataProviderInterfaceL
// -----------------------------------------------------------------------------
//
void CSsyReferenceChannelDataProvider::GetChannelDataProviderInterfaceL( TUid aInterfaceUid, 
	                                        TAny*& aInterface )
    {
    aInterface = NULL;
    
	if ( aInterfaceUid.iUid == KSsyChannelDataProviderInterface1.iUid )
		{
		aInterface = reinterpret_cast<TAny*>(
			static_cast<MSsyChannelDataProvider*>( this ) );
		}
    }
    
// ---------------------------------------------------------------------------
// CSsyReferenceChannelDataProvider::ChannelDataReceived
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannelDataProvider::ChannelDataReceivedL( TSsyReferenceMsg* aMessage )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::ChannelDataReceived()" ) ) );
    
    // Get base class from message
    TSsyRefChannelDataBase* dataItemBase = aMessage->DataItem();

    if ( !dataItemBase )
        {
        User::Leave( KErrArgument );
        }

    // get size of the object
    TInt size( dataItemBase->Size() );

    // First, resolve data item type
    switch ( dataItemBase->ChannelDataType() )
        {
        case TSsyRefChannelDataBase::ESsyRefChannelTypeTapping:
            {
            // Cast data item base to tapping data item
            TSsyRefChannelDataTapping* tappingData = static_cast<TSsyRefChannelDataTapping*>( dataItemBase );
            TSensrvTappingData senSrvTapping;
            senSrvTapping.iTimeStamp = tappingData->Timestamp();
            senSrvTapping.iDirection = tappingData->Direction();

            // Add mapped data item into buffer
            AddDataToBuffer( reinterpret_cast<TUint8*>( &senSrvTapping ), size );
            break;
            }
        case TSsyRefChannelDataBase::ESsyRefChannelTypeAxis:
            {
            // Cast data item base to Axis data item
            TSsyRefChannelDataAxis* axisData = static_cast<TSsyRefChannelDataAxis*>( dataItemBase );
            TSensrvAccelerometerAxisData senSrvAxis;
            senSrvAxis.iTimeStamp = axisData->Timestamp();
            senSrvAxis.iAxisX = axisData->XAxis();
            senSrvAxis.iAxisY = axisData->YAxis();
            senSrvAxis.iAxisZ = axisData->ZAxis();
            
            // Add data to buffer
            AddDataToBuffer( reinterpret_cast<TUint8*>( &senSrvAxis ), size );
            break;
            }
        case TSsyRefChannelDataBase::ESsyRefChannelTypeProximity:
            {
            // Cast data item base to tapping data item
            TSsyRefChannelDataProximity* proximityData = static_cast<TSsyRefChannelDataProximity*>( dataItemBase );
            TSensrvProximityData senSrvProximity;
            senSrvProximity.iProximityState = ( TSensrvProximityData::TProximityState ) proximityData->ProximityState();

            // Add mapped data item into buffer
            AddDataToBuffer( reinterpret_cast<TUint8*>( &senSrvProximity ), size );
            break;
            }
        case TSsyRefChannelDataBase::ESsyRefChannelTypeAmbientLight:
            {
            // Cast data item base to tapping data item
            TSsyRefChannelDataAmbientLight* ambientLightData = static_cast<TSsyRefChannelDataAmbientLight*>( dataItemBase );
            TSensrvAmbientLightData senSrvAmbientLight;
            senSrvAmbientLight.iAmbientLight = ambientLightData->AmbientLightState();

            // Add mapped data item into buffer
            AddDataToBuffer( reinterpret_cast<TUint8*>( &senSrvAmbientLight ), size );
            break;
            }
        case TSsyRefChannelDataBase::ESsyRefChannelTypeMagneticAxis:
            {
            // Cast data item base to Axis data item
            TSsyRefChannelDataMagneticAxis* axisData = static_cast<TSsyRefChannelDataMagneticAxis*>( dataItemBase );
            TSensrvMagnetometerAxisData senSrvAxis;
            senSrvAxis.iTimeStamp = axisData->Timestamp();
            senSrvAxis.iAxisXRaw = axisData->XAxis();
            senSrvAxis.iAxisYRaw = axisData->YAxis();
            senSrvAxis.iAxisZRaw = axisData->ZAxis();
            senSrvAxis.iAxisXCalibrated = axisData->XAxisCalib();
            senSrvAxis.iAxisYCalibrated = axisData->YAxisCalib();
            senSrvAxis.iAxisZCalibrated = axisData->ZAxisCalib();
            
            // Add data to buffer
            AddDataToBuffer( reinterpret_cast<TUint8*>( &senSrvAxis ), size );
            break;
            }
        default:
            {
            // Unknown data item -> Leave
            User::Leave( KErrUnknown );
            }
        }
    
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::ChannelDataReceived() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannelDataProvider::AddDataToBuffer
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannelDataProvider::AddDataToBuffer( TUint8* aData, const TInt aSize )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::AddDataToBuffer()" ) ) );
    // Write data to buffer. If buffer is full, send notification to SensorServer

    // Write data bytes one by one to buffer pointer. The actual buffer is in Sensor Server    
    for ( TInt i = 0; i < aSize; i++ )
        {
        *iDataBuffer++ = *aData++;
        }
    
    // Increase number of items count
    iDataCount++;

    // Check is maximum data count received
    if ( iDataCount == iMaxCount )
        {
        // Send BufferFilled notification to Sensor server
        SendBufferFilled();
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::AddDataToBuffer() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceChannelDataProvider::SendBufferFilled
// ---------------------------------------------------------------------------
//
void CSsyReferenceChannelDataProvider::SendBufferFilled()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::SendBufferFilled()" ) ) );
    // Send BufferFilled notification to Sensor server
    iChannel.SsyControl().SsyCallback().BufferFilled( iChannel.ChannelId(), iDataCount, iDataBuffer, iMaxCount );
    iDataCount = 0;
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceChannelDataProvider::SendBufferFilled() - return" ) ) );
    }

// End of file
