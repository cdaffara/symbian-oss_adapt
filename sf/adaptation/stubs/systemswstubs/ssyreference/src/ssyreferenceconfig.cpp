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
* Description:  Configuration implementation of this SSY
*
*/


#include <sensrvsensorchannels.h>
#include <sensrvchanneldatatypes.h>
#include <sensrvtypes.h>
#include <f32file.h>
#include <gmxmldocument.h>
#include <gmxmlelement.h>

#include "ssyreferencecontrol.h"
#include "ssyreferenceconfig.h"
#include "ssyreferencetrace.h"


// ======== CONSTANTS =======
_LIT( KSsyReferenceCfgFileOrig, "Z:\\private\\1020507E\\reference\\SsyReferenceConfig.xml" );
_LIT( KSsyReferenceCfgFileExt, "C:\\ssyreference\\SsyReferenceConfig.xml" );

// Config file definitions

// Maximum attribute lenghth
const TInt KSsyRefMaxAttribLength = 20;

// TAG DEFINITIONS
_LIT( KSsyRefRootTag, "SsyReferenceConfig" );                   // Ssy general information tag
_LIT( KSsyRefGeneralInfoTag, "SsyGeneralInformation" );         // Ssy general information tag
_LIT( KSsyRefChannelInfoGroupTag, "ChannelInformationGroup" );  // Channel information group tag
_LIT( KSsyRefChannelItemTag, "ChannelItem" );                   // Channel item tag
_LIT( KSsyRefChannelDataTag, "ChannelData" );                   // Channel data tag
_LIT( KSsyRefChannelDataItemTag, "ChannelDataItem" );           // Channel data item tag

_LIT( KSsyRefProperties, "Properties" );                        // Properties tag
_LIT( KSsyRefPropertyItem, "PropertyItem" );                    // PropertyItem tag

// Data item definitions
_LIT( KSsyRefAxisDataItemTag, "SsyRefChannelDataAxis" );        // SsyRefChannelDataAxis data item tag
_LIT( KSsyRefXAxis, "XAxis" );                                  // XAxis from SsyRefChannelDataAxis
_LIT( KSsyRefYAxis, "YAxis" );                                  // YAxis from SsyRefChannelDataAxis
_LIT( KSsyRefZAxis, "ZAxis" );                                  // ZAxis from SsyRefChannelDataAxis

_LIT( KSsyRefTappingDataItemTag, "SsyRefChannelDataTapping" );  // SsyRefChannelDataTapping data item tag
_LIT( KSsyRefDirection, "Direction" );                          // Direction from SsyRefChannelDataTapping

_LIT( KSsyRefProximityDataItemTag, "SsyRefChannelDataProximity" );  // SsyRefChannelDataProximity data item tag
_LIT( KSsyRefProximityState, "ProximityState" );                                   // ProximityStatus from SsyRefChannelDataProximity

_LIT( KSsyRefAmbientLightDataItemTag, "SsyRefChannelDataAmbientLight" );  // SsyRefChannelDataAmbientLight data item tag
_LIT( KSsyRefAmbientLightState, "AmbientLightState" );                                   // AmbientLightStatus from SsyRefChannelDataAmbientLight

_LIT( KSsyRefMagneticAxisDataItemTag, "SsyRefChannelDataMagneticAxis" );        // SsyRefChannelDataMagneticAxis data item tag
_LIT( KSsyRefXAxisCalib, "XAxisCalib" );                        // XAxis from SsyRefChannelDataMagneticAxis
_LIT( KSsyRefYAxisCalib, "YAxisCalib" );                        // YAxis from SsyRefChannelDataMagneticAxis
_LIT( KSsyRefZAxisCalib, "ZAxisCalib" );                        // ZAxis from SsyRefChannelDataMagneticAxis

// ATTRIBUTE DEFINITIONS
_LIT( KSsyRefChannelCount, "ChannelCount" );    // Channel count from ChannelInformationGroup
_LIT( KSsyRefChannelId, "ChannelId" );          // Channel ID from ChannelItem
_LIT( KSsyRefContextType, "ContextType" );      // Context type from ChannelItem
_LIT( KSsyRefQuantity, "Quantity" );            // Quantity from ChannelItem
_LIT( KSsyRefChannelType, "ChannelType" );      // ChannelType from ChannelItem
_LIT( KSsyRefLocation, "Location" );            // Location from ChannelItem
_LIT( KSsyRefVendorId, "Vendor" );              // Vendor from ChannelItem


// Channel data item specific attribute definitions
_LIT( KSsyRefStartInterval, "StartIntervalMs" );              // StartInterval from ChannelData
_LIT( KSsyRefDataItemCount, "count" );                        // count from ChannelDataItem
_LIT( KSsyRefDataTypeID, "DataTypeId" );                      // DataTypeId from ChannelDataItem
_LIT( KSsyRefInterval, "IntervalMs" );                        // IntervalMs from ChannelDataItem

// Property spesific attributes
_LIT( KSsyRefPropertyId, "PropertyId" );              // PropertyId from PropertyItem
_LIT( KSsyRefArrayIndex, "ArrayIndex" );              // ArrayIndex from PropertyItem
_LIT( KSsyRefItemIndex, "ItemIndex" );                // ItemIndex from PropertyItem
_LIT( KSsyRefPropertyValue, "PropertyValue" );        // PropertyValue from PorpertyItem
_LIT( KSsyRefPropertyType, "PropertyType" );          // PropertyType from PropertyItem
_LIT( KSsyRefMaxValue, "MaxValue" );                  // MaxValue from PropertyItem
_LIT( KSsyRefMinValue, "MinValue" );                  // MinValue from PorpertyItem
_LIT( KSsyRefReadOnly, "ReadOnly" );                  // ReadOnly from PropertyItem


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSsyReferenceConfig C++ constructor
// ---------------------------------------------------------------------------
//
CSsyReferenceConfig::CSsyReferenceConfig() :
    CActive( EPriorityMuchLess )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::CSsyReferenceConfig()" ) ) );
    CActiveScheduler::Add( this );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::CSsyReferenceConfig() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// Symbian 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::ConstructL()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ConstructL()" ) ) );
    // Create config file parser
    iConfigParser = CMDXMLParser::NewL( this );

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ConstructL() - return" ) ) );
    }


// ---------------------------------------------------------------------------
// CSsyReferenceConfig::NewL
// ---------------------------------------------------------------------------
//
CSsyReferenceConfig* CSsyReferenceConfig::NewL()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::NewL()" ) ) );
    CSsyReferenceConfig* self = new ( ELeave ) CSsyReferenceConfig();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::NewL() - return" ) ) );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CSsyReferenceConfig::~CSsyReferenceConfig()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::~CSsyReferenceConfig()" ) ) );
    
    if ( iConfigParser )
        {
        delete iConfigParser;
        iConfigParser = NULL;
        }

    if ( iGenralInfoElement )
        {
        delete iGenralInfoElement;
        iGenralInfoElement = NULL;
        }

    if ( iChannelGroupElement )
        {
        delete iChannelGroupElement;
        iChannelGroupElement = NULL;
        }

    if ( iSsyReferenceConfig )
        {
        delete iSsyReferenceConfig;
        iSsyReferenceConfig = NULL;
        }

    if ( iConfigFile )
        {
        delete iConfigFile;
        iConfigFile = NULL;
        }

    iChannelPairArray.Reset();
    
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::~CSsyReferenceConfig() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::InitConfig
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::InitConfigL()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::InitConfig()" ) ) );
    
    // Open config file
    RFs fileSession;
    User::LeaveIfError( fileSession.Connect() );

    // Locate extrenal file... 
    RFile file;
    TInt err( file.Open( fileSession, KSsyReferenceCfgFileExt, EFileRead ) );
    file.Close();

    // Check is external file found
    if ( KErrNone == err )
        {
        // Use SSY with external configuration
        iConfigParser->ParseFile( fileSession, KSsyReferenceCfgFileExt );
        }
    else
        {
        // Use SSY with original configuration

        // Start parsing file and wait notification to ParseFileCompleteL
        // XML Parser takes ownership of the RFs and closes it when file is parsed
        iConfigParser->ParseFile( fileSession, KSsyReferenceCfgFileOrig );
        }

    iConfigFileParsed = EFalse;

    // This active object has very low priority since XML parser uses Active objects also, 
    // so it is mandatory to let XML parser to complete sooner than this active object
    IssueRequest();
    iSchedulerWait.Start(); // Blocks until file is parsed

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::InitConfig() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::IssueRequest
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::IssueRequest( TInt aError )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::IssueRequest()" ) ) );
    // Provides synchronous function calls to be handled as asynchronous
    if ( !IsActive() )
        {
        SetActive();
        TRequestStatus *s = &iStatus;
	    User::RequestComplete( s, aError );
        }
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::IssueRequest() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::RunL
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::RunL()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::RunL() - %i" ), iStatus.Int() ) );

    if ( iConfigFileParsed )
        {
        // Stop blocking
        iSchedulerWait.AsyncStop();
        }
    else
        {
        // Continue RunL loop
        IssueRequest();
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::RunL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::DoCancel
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::DoCancel()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::DoCancel()" ) ) );

    // Stop blocking
    iSchedulerWait.AsyncStop();
    
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::DoCancel() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::RunError
// ---------------------------------------------------------------------------
//
TInt CSsyReferenceConfig::RunError( TInt /*aError*/ )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::RunError()" ) ) );

    // Handle possible errors here and return KErrNone to prevent SSY from panic

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::RunError() - return" ) ) );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::ParseFileCompleteL
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::ParseFileCompleteL()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ParseFileCompleteL()" ) ) );
    
    ERROR_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ParseFileCompleteL(): parse error=%d", iConfigParser->Error() ) ) );
    
    // if the parser fails with KErrNoMemory, increase the default heap size for SSY's (key 3) in the repository 1020507E.txt exported by this component
    __ASSERT_DEBUG( iConfigParser->ErrorSeverity() != EXMLFatal, User::Invariant() ); //  OK to continue if not a fatal error
    
    // First get document
    iConfigFile = iConfigParser->DetachXMLDoc();
    // Then get document element
    CMDXMLElement*  documentElement = iConfigFile->DocumentElement();
    // Get root element, 'SsyReferenceConfig'
    iSsyReferenceConfig = documentElement->FirstChildOfType( KSsyRefRootTag );
    // Get gereral information element 
    iGenralInfoElement = iSsyReferenceConfig->FirstChildOfType( KSsyRefGeneralInfoTag );
    // Get channel information group element
    iChannelGroupElement = iSsyReferenceConfig->FirstChildOfType( KSsyRefChannelInfoGroupTag );
    // Get channel count
    iChannelCount = GetAttributeIntValue( *iChannelGroupElement, KSsyRefChannelCount );

    // No need to delete documentElement, it is owned by iConfigFile.
    documentElement = NULL;
    iConfigFileParsed = ETrue;

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ParseFileCompleteL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::GetAttributeIntValue
// ---------------------------------------------------------------------------
//
TInt CSsyReferenceConfig::GetAttributeIntValue( CMDXMLElement& aElement, const TDesC& aAttrib )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetAttributeIntValue()" ) ) );
    COMPONENT_TRACE( ( _L( "  Element: %s", aElement.NodeName() ) ) );
    COMPONENT_TRACE( ( _L( "  Attribute: %s", aAttrib ) ) );

    TInt intValue( 0 );

    // Check availability
    if ( aElement.IsAttributeSpecified( aAttrib ) )
        {
        // Buffer to where to read value
        TBufC<KSsyRefMaxAttribLength> buffer( KNullDesC );
        TPtrC ptr( buffer );

        // Read attribute value
        aElement.GetAttribute( aAttrib, ptr );

        // Cast literal value into TInt
        TLex lexValue( ptr );
        lexValue.Val( intValue );
        }

    COMPONENT_TRACE( ( _L( "  IntValue: %i", intValue ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetAttributeIntValue() - return" ) ) );
    return intValue;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::GetAttributeStrValue
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::GetAttributeStrValue( CMDXMLElement& aElement, const TDesC& aAttrib, TDes8& aTarget )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetAttributeStrValue()" ) ) );
    COMPONENT_TRACE( ( _L( "  Element: %s", aElement.NodeName() ) ) );
    COMPONENT_TRACE( ( _L( "  Attribute: %s", aAttrib ) ) );

    // Check availability
    if ( aElement.IsAttributeSpecified( aAttrib ) )
        {
        // Buffer to where to read value
        TBufC<KSsyRefMaxAttribLength> buffer( KNullDesC );
        TPtrC ptr( buffer );

        // Read attribute value
        aElement.GetAttribute( aAttrib, ptr );

        // Copy string from 16-bit descriptor to 8-bit descriptor
        aTarget.Copy( ptr );
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetAttributeStrValue() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::GetAttributeHexValue
// ---------------------------------------------------------------------------
//
TUint CSsyReferenceConfig::GetAttributeHexValue( CMDXMLElement& aElement, const TDesC& aAttrib )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetAttributeHexValue()" ) ) );
    COMPONENT_TRACE( ( _L( "  Element: %s", aElement.NodeName() ) ) );
    COMPONENT_TRACE( ( _L( "  Attribute: %s", aAttrib ) ) );

    TUint32 hexValue( 0 );

    // Check availability
    if ( aElement.IsAttributeSpecified( aAttrib ) )
        {
        // Buffer to where to read value
        TBufC<KSsyRefMaxAttribLength> buffer( KNullDesC );
        TPtrC ptr( buffer );

        // Read attribute value
        aElement.GetAttribute( aAttrib, ptr );

        // Get bounded value and cast it into TUint32 (hex)
        TRadix radix( EHex );
        TUint limit( 0xFFFFFFFF );

        // Append string into Lex and skip first two characters, 0x
        TLex lexValue( ptr );
        lexValue.Inc( 2 );

        // Read value
        lexValue.BoundedVal( hexValue, radix, limit );
        }

    COMPONENT_TRACE( ( _L( "  HexValue: %x", hexValue ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetAttributeHexValue() - return" ) ) );
    return hexValue;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::GetAttributeRealValue
// ---------------------------------------------------------------------------
//
TReal CSsyReferenceConfig::GetAttributeRealValue( CMDXMLElement& aElement, const TDesC& aAttrib )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetAttributeRealValue()" ) ) );
    COMPONENT_TRACE( ( _L( "  Element: %s", aElement.NodeName() ) ) );
    COMPONENT_TRACE( ( _L( "  Attribute: %s", aAttrib ) ) );

    TReal realValue( 0 );

    // Check availability
    if ( aElement.IsAttributeSpecified( aAttrib ) )
        {

        // Buffer to where to read value
        TBufC<KSsyRefMaxAttribLength> buffer( KNullDesC );
        TPtrC ptr( buffer );

        // Read attribute value
        aElement.GetAttribute( aAttrib, ptr );

        // Cast literal value into TReal
        TLex lexValue( ptr );
        lexValue.Val( realValue );
        }

    COMPONENT_TRACE( ( _L( "  IntValue: %i", realValue ) ) );
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetAttributeRealValue() - return" ) ) );
    return realValue;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::ChannelCount
// ---------------------------------------------------------------------------
//
TInt CSsyReferenceConfig::ChannelCount()
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ChannelCount() - %i" ), iChannelCount ) );
    return iChannelCount;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::GenerateChannels
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::GenerateChannels( RSensrvChannelInfoList& aChannelList )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GenerateChannels()" ) ) );

    // Initialize channel pair array
    TSsyRefChannelIdArray tempArray( iChannelCount );
    iChannelPairArray = tempArray;

    // Go through Channel group element and get all information
    TSensrvChannelInfo channelInfo;
    CMDXMLElement* channelElement = iChannelGroupElement->FirstChildOfType( KSsyRefChannelItemTag );

    while( channelElement )
        {
        // check is element correct type of node
        if ( channelElement->NodeType() == CMDXMLNode::EElementNode )
            {
            // read channel identifier
            iChannelPairArray.Append( TSsyRefChannelIdPair( GetAttributeIntValue( *channelElement, KSsyRefChannelId ) ) );

            // Read attributes
            channelInfo.iContextType = ( TSensrvContextType ) GetAttributeIntValue( *channelElement, KSsyRefContextType );
            channelInfo.iQuantity = ( TSensrvQuantity ) GetAttributeIntValue( *channelElement, KSsyRefQuantity );
            channelInfo.iChannelType = ( TSensrvChannelTypeId ) GetAttributeHexValue( *channelElement, KSsyRefChannelType );
            GetAttributeStrValue( *channelElement, KSsyRefLocation, channelInfo.iLocation );
            GetAttributeStrValue( *channelElement, KSsyRefVendorId, channelInfo.iVendorId );
            channelInfo.iChannelDataTypeId = ( TSensrvChannelDataTypeId ) GetAttributeHexValue( *channelElement, KSsyRefDataTypeID );
            
            // Calculate data item size based on channel type
            switch ( channelInfo.iChannelType )
                {
                case KSensrvChannelTypeIdAccelerometerXYZAxisData:
                    {
                    channelInfo.iDataItemSize = KSsyRefAxisDataItemSize;
                    break;
                    }
                case KSensrvChannelTypeIdProximityMonitor:
                    {
                    channelInfo.iDataItemSize = KSsyRefProximityDataItemSize;
                    break;
                    }
                case KSensrvChannelTypeIdAmbientLightData:
                    {
                    channelInfo.iDataItemSize = KSsyRefAmbientLightDataItemSize;
                    break;
                    }
                case KSensrvChannelTypeIdAccelerometerWakeupData:
                case KSensrvChannelTypeIdAccelerometerDoubleTappingData:
                    {
                    channelInfo.iDataItemSize = KSsyRefTappingDataItemSize;
                    break;
                    }
                case KSensrvChannelTypeIdMagnetometerXYZAxisData:
                    {
                    channelInfo.iDataItemSize = KSsyRefMagneticAxisDataItemSize;
                    break;
                    }
                default:
                    {
                    channelInfo.iDataItemSize = 0;
                    break;
                    }
                }

            // Append channel info to list
            aChannelList.Append( channelInfo );
            }
        channelElement = static_cast<CMDXMLElement*>( channelElement->NextSibling() );
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GenerateChannels() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::GetChannelDataInformation
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::GetChannelDataInformationL( 
    const TInt aSrvChannelId,
    TSsyRefDataItemArray& aDataItemList, 
    TInt& aStartInterval )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetChannelDataInformation()" ) ) );

    // First, get correct config channel element corresponding to aSrvChannelId
    CMDXMLElement* channelElement = ChannelElement( aSrvChannelId );

    if ( channelElement )
        {
        // Channel element found, get channel data group element
        CMDXMLElement* groupElement = channelElement->FirstChildOfType( KSsyRefChannelDataTag );
        
        if ( groupElement )
            {
            // Get start interval
            aStartInterval = GetAttributeIntValue( *groupElement, KSsyRefStartInterval );

            // First, loop channel data items to get total count
            CMDXMLElement* dataItemElement = groupElement->FirstChildOfType( KSsyRefChannelDataItemTag );

            // Take channel data item type at this point. One channel can produce only one type of
            // channel data item
            TUint channelType( GetAttributeHexValue( *dataItemElement, KSsyRefDataTypeID ) );

            TInt channelItemCount( 0 ); // Total number of data items
            TInt definitionCount( 0 );  // Total number of different definitions

            // Go through elements and get counters
            while ( dataItemElement )
                {
                definitionCount++;
                channelItemCount = channelItemCount + GetAttributeIntValue( *dataItemElement, KSsyRefDataItemCount );
                // This will return NULL if no next sibling found
                dataItemElement = static_cast<CMDXMLElement*>( dataItemElement->NextSibling() );
                }

            // Now, start all over to get item information
            dataItemElement = groupElement->FirstChildOfType( KSsyRefChannelDataItemTag );

            // Create temp array now that we know the data item count
            TSsyRefDataItemArray tempArray( channelItemCount );

            for ( TInt i = 0; i < definitionCount; i++ )
                {
                // Check element type
                if ( dataItemElement->NodeType() == CMDXMLNode::EElementNode )
                    {
                    // First we get interval and count from channel item
                    TInt interval( GetAttributeIntValue( *dataItemElement, KSsyRefInterval ) );
                    TInt countOfType( GetAttributeIntValue( *dataItemElement, KSsyRefDataItemCount ) );

                    // Read next child values to corresponding data type class
                    switch ( channelType )
                        {
                        case TSensrvAccelerometerAxisData::KDataTypeId:
                            {
                            CMDXMLElement* axisDataElement = dataItemElement->FirstChildOfType( KSsyRefAxisDataItemTag );
                            TInt axisX( GetAttributeIntValue( *axisDataElement, KSsyRefXAxis ) );
                            TInt axisY( GetAttributeIntValue( *axisDataElement, KSsyRefYAxis ) );
                            TInt axisZ( GetAttributeIntValue( *axisDataElement, KSsyRefZAxis ) );

                            // Create channel data type item
                            TSsyRefChannelDataAxis channelData( axisX, axisY, axisZ, interval );
                            // add items into array
                            for ( TInt k = 0; k < countOfType; k++ )
                                {
                                tempArray.Append( channelData );
                                }
                            break;
                            }
                        case TSensrvTappingData::KDataTypeId:
                            {
                            CMDXMLElement* tappingDataElement = dataItemElement->FirstChildOfType( KSsyRefTappingDataItemTag );
                            TInt direction( GetAttributeHexValue( *tappingDataElement, KSsyRefDirection ) );
                            
                            // Create channel data type item
                            TSsyRefChannelDataTapping channelData( direction, interval );
                            // add items into array
                            for ( TInt k = 0; k < countOfType; k++ )
                                {
                                tempArray.Append( channelData );
                                }
                            break;
                            }
                        case TSensrvProximityData::KDataTypeId:
                            {
                            CMDXMLElement* proximityDataElement = dataItemElement->FirstChildOfType( KSsyRefProximityDataItemTag );
                            TInt state( GetAttributeIntValue( *proximityDataElement, KSsyRefProximityState ) );
                            
                            // Create channel data type item
                            TSsyRefChannelDataProximity channelData( state, interval );
                            // add items into array
                            for ( TInt k = 0; k < countOfType; k++ )
                                {
                                tempArray.Append( channelData );
                                }
                            break;
                            }
                        case TSensrvAmbientLightData::KDataTypeId:
                            {
                            CMDXMLElement* ambientLightDataElement = dataItemElement->FirstChildOfType( KSsyRefAmbientLightDataItemTag );
                            TInt state( GetAttributeIntValue( *ambientLightDataElement, KSsyRefAmbientLightState ) );
                            
                            // Create channel data type item
                            TSsyRefChannelDataAmbientLight channelData( state, interval );
                            // add items into array
                            for ( TInt k = 0; k < countOfType; k++ )
                                {
                                tempArray.Append( channelData );
                                }
                            break;
                            }
                        case TSensrvMagnetometerAxisData::KDataTypeId:
                            {
                            CMDXMLElement* axisDataElement = dataItemElement->FirstChildOfType( KSsyRefMagneticAxisDataItemTag );
                            TInt axisX( GetAttributeIntValue( *axisDataElement, KSsyRefXAxis ) );
                            TInt axisY( GetAttributeIntValue( *axisDataElement, KSsyRefYAxis ) );
                            TInt axisZ( GetAttributeIntValue( *axisDataElement, KSsyRefZAxis ) );
                            TInt axisXCalib( GetAttributeIntValue( *axisDataElement, KSsyRefXAxisCalib ) );
                            TInt axisYCalib( GetAttributeIntValue( *axisDataElement, KSsyRefYAxisCalib ) );
                            TInt axisZCalib( GetAttributeIntValue( *axisDataElement, KSsyRefZAxisCalib ) );

                            // Create channel data type item
                            TSsyRefChannelDataMagneticAxis channelData( axisX, axisY, axisZ, 
                                axisXCalib, axisYCalib, axisZCalib, interval );
                            // add items into array
                            for ( TInt k = 0; k < countOfType; k++ )
                                {
                                tempArray.Append( channelData );
                                }
                            break;
                            }
                        default:
                            {
                            // Other data items are not supported
                            User::Leave( KErrGeneral );
                            }
                        }
                    }

                // Get next channel data item element
                dataItemElement = static_cast<CMDXMLElement*>( dataItemElement->NextSibling() );
                }

            // Compress temp array in case there were comments
            tempArray.Compress();

            // copy information to param array
            aDataItemList = tempArray;
            }
        else
            {
            User::Leave( KErrNotFound );
            }
        }
    else
        {
        User::Leave( KErrNotFound );
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetChannelDataInformation() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::GetElementPropertiesL
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::GetElementPropertiesL( 
    CMDXMLElement& aElement, 
    RSensrvPropertyList& aPropertyList )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetElementPropertiesL()" ) ) );

    // First we need 'Properties' element
    CMDXMLElement* properties = aElement.FirstChildOfType( KSsyRefProperties );

    if ( !properties )
        {
        // Wrong element... or properties are not defined
        User::Leave( KErrNotFound );
        }

    // Get first property get started
    CMDXMLElement* propertyItem = properties->FirstChildOfType( KSsyRefPropertyItem );
    TInt propertyCount( 0 );

    // Loop properties to get count of properties
    while ( propertyItem )
        {
        propertyCount++;
        propertyItem = static_cast<CMDXMLElement*>( propertyItem->NextSibling() ); // returns NULL if next not found
        }
    
    if ( !propertyCount )
        {
        // Check that there are properties
        User::Leave( KErrNotFound );
        }

    // Temporary property list now that we know the property count
    RSensrvPropertyList tempList( propertyCount );

    // Start loop again from the start and read each property
    propertyItem = properties->FirstChildOfType( KSsyRefPropertyItem );
    for ( TInt i = 0; i < propertyCount; i++ )
        {
        // Check element type
        if ( propertyItem->NodeType() == CMDXMLNode::EElementNode )
            {
            // Read property values
            const TSensrvPropertyId propertyId( ( TSensrvPropertyId )GetAttributeHexValue( *propertyItem, KSsyRefPropertyId ) );
            const TInt itemIndex( GetAttributeIntValue( *propertyItem, KSsyRefItemIndex ) );
            const TBool readOnly( ( TBool )GetAttributeIntValue( *propertyItem, KSsyRefReadOnly ) );
            const TSensrvPropertyType propertyType( ( TSensrvPropertyType ) GetAttributeIntValue( *propertyItem, KSsyRefPropertyType ) );

            // Array index must be handled in different way as it is not mandatory. Little modification is needed as it may not exist in XML file
            TInt arrayIndex( ESensrvSingleProperty );

            // Extra check is needed, otherwise this value is always '0' when it should be 'ESensrvSingleProperty' by default
            if ( propertyItem->IsAttributeSpecified( KSsyRefArrayIndex ) )
                {
                // Attribute exists, now we can read the value
                arrayIndex = GetAttributeIntValue( *propertyItem, KSsyRefArrayIndex );
                }

            // Resolve type, get correct type value and append property into list
            switch ( propertyType )
                {
                case ESensrvIntProperty:
                    {
                    const TInt intValue( GetAttributeIntValue( *propertyItem, KSsyRefPropertyValue ) );
                    const TInt maxValue( GetAttributeIntValue( *propertyItem, KSsyRefMaxValue ) );
                    const TInt minValue( GetAttributeIntValue( *propertyItem, KSsyRefMinValue ) );
                    TSensrvProperty property( propertyId, itemIndex, intValue, maxValue, minValue, readOnly, propertyType );
                    property.SetArrayIndex( arrayIndex );
                    tempList.Append( property );
                    break;
                    }
                 case ESensrvRealProperty:
                    {
                    const TReal intValue( GetAttributeRealValue( *propertyItem, KSsyRefPropertyValue ) );
                    const TReal maxValue( GetAttributeRealValue( *propertyItem, KSsyRefMaxValue ) );
                    const TReal minValue( GetAttributeRealValue( *propertyItem, KSsyRefMinValue ) );
                    TSensrvProperty property( propertyId, itemIndex, intValue, maxValue, minValue, readOnly, propertyType );
                    property.SetArrayIndex( arrayIndex );
                    tempList.Append( property );
                    break;
                    }
                 case ESensrvBufferProperty:
                    {
                    TBuf8<KSensrvPropertyTextBufferSize> desValue;
                    GetAttributeStrValue( *propertyItem, KSsyRefPropertyValue, desValue );
                    TSensrvProperty property( propertyId, itemIndex, desValue, readOnly, propertyType );
                    property.SetArrayIndex( arrayIndex );
                    tempList.Append( property );
                    break;
                    }
                default:
                    {
                    // Unknown property type -> leave
                    User::Leave( KErrArgument );    
                    }
                }
            }

        // Next property
        propertyItem = static_cast<CMDXMLElement*>( propertyItem->NextSibling() ); // returns NULL if next not found
        }

    // Compress temp list in case there were comment nodes
    tempList.Compress();

    // copy temp list to parameter list
    aPropertyList = tempList;

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetElementPropertiesL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::ChannelElement
// ---------------------------------------------------------------------------
//
CMDXMLElement* CSsyReferenceConfig::ChannelElement( const TInt aSrvChannelId )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ChannelElement()" ) ) );

    const TInt configId( ConfigChannelId( aSrvChannelId ) );
    TBool channelFound( EFalse );
    
    // Loop channel group and match configId for the channel ID in element
    CMDXMLElement* channelItemElement = iChannelGroupElement->FirstChildOfType( KSsyRefChannelItemTag );
    
    for ( TInt i = 0; i < iChannelCount || !channelFound; i++ )
        {
        TInt channelId( GetAttributeIntValue( *channelItemElement, KSsyRefChannelId ) );
        if ( configId == channelId  )
            {
            // Channel found, no  need to loop
            channelFound = ETrue;
            }
        else
            {
            // Take next channel
            channelItemElement = static_cast<CMDXMLElement*>( channelItemElement->NextSibling() );
            }
        }

    // If not found, return NULL
    if ( !channelFound )
        {
        channelItemElement = NULL;
        }

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ChannelElement() - return" ) ) );
    return channelItemElement;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::UpdateChannelIds
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::UpdateChannelIds( RSensrvChannelInfoList aChannelList )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::UpdateChannelIds()" ) ) );
    

    if ( ChannelCount() == aChannelList.Count() )
        {
        for ( TInt i = 0; i < aChannelList.Count(); i++ )
            {
            iChannelPairArray[i].SetServerId( aChannelList[i].iChannelId );
            }
        }
    
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::UpdateChannelIds() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::ConfigChannelId
// ---------------------------------------------------------------------------
//
TInt CSsyReferenceConfig::ConfigChannelId( const TInt aSrvChannelId ) const
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ConfigChannelId()" ) ) );    
    TInt returnValue( 0 );
    
    for ( TInt i = 0; i < iChannelPairArray.Count(); i++ )
        {
        if ( iChannelPairArray[i].ServerId() == aSrvChannelId )
            {
            returnValue = iChannelPairArray[i].ConfigId();
            }
        }
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::ConfigChannelId() - return" ) ) );
    return returnValue;
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::GetSensorPropertiesL
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::GetSensorPropertiesL( RSensrvPropertyList& aPropertyList )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetSensorPropertiesL()" ) ) );

    // We already have SsyGeneralInformation element, read properties from that
    GetElementPropertiesL( *iGenralInfoElement, aPropertyList );

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetSensorPropertiesL() - return" ) ) );
    }

// ---------------------------------------------------------------------------
// CSsyReferenceConfig::GetChannelPropertiesL
// ---------------------------------------------------------------------------
//
void CSsyReferenceConfig::GetChannelPropertiesL( 
    const TInt aSrvChannelId, 
    RSensrvPropertyList& aPropertyList )
    {
    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetChannelPropertiesL()" ) ) );

    // Get channel element first
    CMDXMLElement* channelElement = ChannelElement( aSrvChannelId );

    if ( !channelElement )
        {
        // Leave, channel element is not found
        User::Leave( KErrNotFound );
        }

    // Get properties of this channel element
    GetElementPropertiesL( *channelElement, aPropertyList );

    COMPONENT_TRACE( ( _L( "SSY Reference Plugin - CSsyReferenceConfig::GetChannelPropertiesL() - return" ) ) );
    }

// End of file
