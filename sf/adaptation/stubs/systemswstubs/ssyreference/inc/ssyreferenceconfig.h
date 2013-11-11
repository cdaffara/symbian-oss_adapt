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
* Description:  Contains SSY Reference's channel configuration data
*
*/




#ifndef SSYREFERENCECONFIG_H
#define SSYREFERENCECONFIG_H

//  INCLUDES
#include <e32base.h>
#include <f32file.h>
#include <sensrvtypes.h>
#include <sensrvchannelinfo.h>
#include <gmxmlparser.h>
#include <sensrvchanneldatatypes.h>
#include <sensrvproximitysensor.h>
#include <sensrvilluminationsensor.h>

// CONSTANTS
// Data item sizes
const TInt KSsyRefAxisDataItemSize = sizeof( TSensrvAccelerometerAxisData );
const TInt KSsyRefTappingDataItemSize = sizeof( TSensrvTappingData );
const TInt KSsyRefProximityDataItemSize = sizeof( TSensrvProximityData );
const TInt KSsyRefAmbientLightDataItemSize = sizeof( TSensrvAmbientLightData );
const TInt KSsyRefMagneticAxisDataItemSize = sizeof( TSensrvMagnetometerAxisData );

// ENUMS
enum TSsyReferenceFunctions
    {
    ESsyReferenceOpenChannel,
    ESsyReferenceOpenChannelResp,
    ESsyReferenceDataItemReceived,
    ESsyReferenceCloseChannel,
    ESsyReferenceCloseChannelResp,
    ESsyReferenceStartChannelData,
    ESsyReferenceStopChannelData
    };

/**
 * Base class for Channel data types. Contains iChannelDataType which is set
 * by derived classes on construction.
 */
class TSsyRefChannelDataBase
    {
    public:
    /**
     * Enumeration for possible channel data types
     */
    enum TSsyRefChannelDataType
        {
        ESsyRefChannelTypeTapping = 5000,
        ESsyRefChannelTypeAxis,
        ESsyRefChannelTypeProximity,
        ESsyRefChannelTypeAmbientLight,
        ESsyRefChannelTypeMagneticAxis
        };
   
    TInt  ChannelDataType() const { return iChannelDataType; }
    TInt  Interval() const { return iInterval; }
    
    void SetTimestamp( const TTime aTimestamp ) { iTimestamp = aTimestamp; }
    TTime Timestamp() const { return iTimestamp; }

    TInt  Size() const { return iSize; }

    protected:

    /**
     * Protected constructor as this class is not supposed to be instantiate
     * directly.
     */
    TSsyRefChannelDataBase() {}

    protected: // data

    /**
     * Identifies the type of data type class derived from this base class
     */
    TInt iChannelDataType;
    
    /**
     * Interval. Indicates time in ms from previous item until next item is produced
     */
    TInt iInterval;

    /**
     * Timestamp. Time when this data item is generated.
     */
    TTime iTimestamp;

    /**
     * Size of one data item. This is filled by derived class
     */
    TInt iSize;

    /**
     * Axis data item values. 
     * Accessible from TSsyRefChannelDataAxis/TSsyRefChannelDataMagneticAxis classes
     */
    TInt iXAxis;
    TInt iYAxis;
    TInt iZAxis;
    /**
     * Axis data calibrated values. 
     * Accessible from TSsyRefChannelDataMagneticAxis classes
     */
    TInt iXAxisCalib;
    TInt iYAxisCalib;
    TInt iZAxisCalib;

    
    /**
     * Proximity data item values.
     * Accessible only from TSsyRefChannelProximity class
     */    
    TInt iProximityState;

    /**
     * AmbientLight data item values.
     * Accessible only from TSsyRefChannelAmbientLight class
     */    
    TInt iAmbientLightState;

    /**
     * Tapping data item values. Difrection of the tapping.
     * Accessible only from TSsyRefChannelDataTapping class
     */
    TInt iDirection;
    };

// Type definition array for Data item base class
typedef RArray<TSsyRefChannelDataBase> TSsyRefDataItemArray;

/**
 * Message item for SSY <--> Sensor communications
 * Contains Channel ID for which the message belongs to,
 * Function ID that identifies the command and
 * error for error cases. 
 */
class TSsyReferenceMsg
    {
    public:

    /**
    * Constructor of the TSsyReferenceMsg
    *
    * @since S60 5.0
    * @param[in] aChannelId Channel identifier
    * @param[in] aFunction See TSsyReferenceFunctions
    */  
    TSsyReferenceMsg( TInt aChannelId, TInt aFunction ) : 
        iChannelId( aChannelId ), 
        iFunction( aFunction ),
        iError( KErrNone )
        {}

    /**
     * Copy constructor of the TSsyReferenceMsg
     *
     * @since S60 5.0
     * @param[in] aMsg Object to be copied to constructed object
     */    
    TSsyReferenceMsg( const TSsyReferenceMsg& aMsg ) :
        iChannelId( aMsg.iChannelId ),
        iFunction( aMsg.iFunction ),
        iError( aMsg.iError )
        {}

    TInt ChannelId() { return iChannelId; }

    void SetFunction( TInt aFunction ) { iFunction = aFunction; }
    TInt Function()  { return iFunction; }

    void SetError( TInt aError ) { iError = aError; }
    TInt Error()     { return iError; }

    void SetDataItem( TSsyRefChannelDataBase* aDataItem ) { iDataItem = aDataItem; }
    TSsyRefChannelDataBase* DataItem() const { return iDataItem; }

    private: // data
    
    TInt iChannelId; // Identifies the channel
    TInt iFunction;  // Identifies the command
    TInt iError;     // Error is passed to response handler
    
    // Data item for received data. This is casted to correct data item 
    // class implementation according to ChannelDataType
    TSsyRefChannelDataBase* iDataItem;
    };

/**
 * Tapping data type class implementation.
 */
class TSsyRefChannelDataTapping : public TSsyRefChannelDataBase
    {
    public:

    TSsyRefChannelDataTapping( TInt aDirection, TInt aInterval  )
        { 
        iChannelDataType = ESsyRefChannelTypeTapping;
        iSize = KSsyRefTappingDataItemSize;
        iInterval = aInterval;
        iDirection = aDirection;
        }
    
    TInt Direction() const { return iDirection; }
    };

/**
 * XYZ Axis data type class implementation.
 */
class TSsyRefChannelDataAxis : public TSsyRefChannelDataBase
    {
    public:

    TSsyRefChannelDataAxis( TInt aXAxis, TInt aYAxis, TInt aZAxis, 
                            TInt aInterval )
        { 
        iChannelDataType = ESsyRefChannelTypeAxis;
        iSize = KSsyRefAxisDataItemSize;
        iInterval = aInterval; 
        iXAxis = aXAxis;
        iYAxis = aYAxis;
        iZAxis = aZAxis;
        }
    
    TInt XAxis() const { return iXAxis; }
    TInt YAxis() const { return iYAxis; }
    TInt ZAxis() const { return iZAxis; }
    };


/**
 * Proximity data type class implementation.
 */
class TSsyRefChannelDataProximity : public TSsyRefChannelDataBase
    {
    public:

    TSsyRefChannelDataProximity( TInt aProximityState, TInt aInterval )
        { 
        iChannelDataType = ESsyRefChannelTypeProximity;
        iSize = KSsyRefProximityDataItemSize;
        iInterval = aInterval; 
        iProximityState = aProximityState;
        }
    
    TInt ProximityState() const { return iProximityState; }
    };

/**
 * AmbientLight data type class implementation.
 */
class TSsyRefChannelDataAmbientLight : public TSsyRefChannelDataBase
    {
    public:

    TSsyRefChannelDataAmbientLight( TInt aAmbientLightState, TInt aInterval )
        { 
        iChannelDataType = ESsyRefChannelTypeAmbientLight;
        iSize = KSsyRefAmbientLightDataItemSize;
        iInterval = aInterval; 
        iAmbientLightState = aAmbientLightState;
        }
    
    TInt AmbientLightState() const { return iAmbientLightState; }
    };

/**
 * XYZ Axis data type class implementation.
 */
class TSsyRefChannelDataMagneticAxis : public TSsyRefChannelDataBase
    {
    public:

    TSsyRefChannelDataMagneticAxis( TInt aXAxis, TInt aYAxis, TInt aZAxis, 
        TInt aXAxisCalib, TInt aYAxisCalib, TInt aZAxisCalib, TInt aInterval )
        { 
        iChannelDataType = ESsyRefChannelTypeMagneticAxis;
        iSize = KSsyRefMagneticAxisDataItemSize;
        iInterval = aInterval; 
        iXAxis = aXAxis;
        iYAxis = aYAxis;
        iZAxis = aZAxis;
        iXAxisCalib = aXAxisCalib;
        iYAxisCalib = aYAxisCalib;
        iZAxisCalib = aZAxisCalib;
        }
    
    TInt XAxis() const { return iXAxis; }
    TInt YAxis() const { return iYAxis; }
    TInt ZAxis() const { return iZAxis; }
    TInt XAxisCalib() const { return iXAxisCalib; }
    TInt YAxisCalib() const { return iYAxisCalib; }
    TInt ZAxisCalib() const { return iZAxisCalib; }
    };

/**
 * Channel ID pair class for pairing config file channel id and
 * Sensor Server generated channel ID.
 *
 */
class TSsyRefChannelIdPair
    {
    public:
    /**
    * Constructor of the TSsyRefChannelIdPair
    *
    * @since S60 5.0
    * @param[in] aConfigChannelId Channel identifier from config file
    */  
    TSsyRefChannelIdPair( TInt aConfigChannelId ) : 
        iConfigChannelId( aConfigChannelId )
        {}

    TInt ConfigId() const { return iConfigChannelId; }
    TInt ServerId() const { return iSrvChannelId; }

    void SetServerId( const TInt aSrvId ) { iSrvChannelId = aSrvId; }

    private: // data
    
    TInt iConfigChannelId; // Config file ID of the channel
    TInt iSrvChannelId;    // Sensor server assigned ID of the channel
    };

typedef RArray<TSsyRefChannelIdPair> TSsyRefChannelIdArray;


// CONSTANTS

/**
 *  Configuration class for SSY reference plugin. Generates configured SSY channel information.
 *  This keeps reference SSY implementation independent from 'sensor' it uses. This class is fully
 *  modifiable regarding to the needs of this SSY. It may be for example accelerometer sensor 
 *  without any changes in the SSY reference implementation. Only this class is modified.
 *
 *  @lib ssyreferenceplugin.lib
 *  @since S60 5.0
 */
class CSsyReferenceConfig : public CActive, public MMDXMLParserObserver
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 5.0
     * @return CSsyReferenceConfig* Pointer to created CSsyReferenceControl object
     */
    static CSsyReferenceConfig* NewL();

    /**
     * Virtual destructor
     *
     * @since S60 5.0
     */
    virtual ~CSsyReferenceConfig();

    /**
     * From CActive
     */
    void RunL();

    /**
     * From CActive
     */
    void DoCancel();

    /**
     * From CActive
     */
    TInt RunError( TInt aError );

    /**
     * From MMDXMLParserObserver
     *
	 * Call back function used to inform a client of the Parser when a parsing operation completes.
     * @since S60 5.0
	 */
	void ParseFileCompleteL();

    /**
	 * Starts parsing config file. This function blocks until file is parsed
     * @since S60 5.0
	 */
	void InitConfigL();

    /**
     * Total number of channels this SSY provides
     *
     * @since S60 5.0
     * @return TInt Count of channels this SSY is configured to provide
     */
    TInt ChannelCount();

    /**
     * Generates channels this SSY is configured to provide
     *
     * @since S60 5.0
     * @param[in,out] aChannnelList Filled with generated channels by this configurator
     */
    void GenerateChannels( RSensrvChannelInfoList& aChannelList );

    /**
     * Updates Sensor server's generated channel Ids
     *
     * @since S60 5.0
     * @param[in] aChannnelList Same list as GenerateChannels produces but 
     *                this contains channel identifiers
     */
    void UpdateChannelIds( RSensrvChannelInfoList aChannelList );

    /**
     * Reads all channel data information from config file and fills
     * parameters with information
     *
     * @since S60 5.0
     * @param[in]     aSrvChannelId Sensor server generated channel id of the target channel
     * @param[in,out] aDataItemList Contains data item objects defined in config file. Each
     *                Data item is presented as Data Item class derived from TSsyRefChannelDataBase.
     *                List can contain only one type of derived channel data items
     * @param[in,out] aStartInterval Contains start interval to start producing data items
     */
    void GetChannelDataInformationL( const TInt aSrvChannelId, 
                                     TSsyRefDataItemArray& aDataItemList, 
                                     TInt& aStartInterval );

    /**
     * Reads sensor properties from config file and adds them to list
     *
     * @since S60 5.0
     * @param[out] aPropertyList List where to append properties
     */
    void GetSensorPropertiesL( RSensrvPropertyList& aPropertyList );

    /**
     * Reads channel properties from config file and adds them to list
     *
     * @since S60 5.0
     * @param[in] aSenSrvChannelId Sensor server generated channel id
     * @param[out] aPropertyList List where to append properties
     */
    void GetChannelPropertiesL( const TInt aSrvChannelId, 
                                RSensrvPropertyList& aPropertyList );

private:

    /**
     * C++ constructor.
     */
    CSsyReferenceConfig();

    /**
     * Symbian 2nd phase constructor.
     */
    void ConstructL();

    /**
     * Makes synchronous calls asynchronous
     */
    void IssueRequest( TInt aError = KErrNone );

    /**
     * Reads attribute value from element and casts it into TInt value
     * 
     * @since S60 5.0
     * @param[in] aElement Element from where to read attribute
     * @param[in] aAttrib Attribute name which to read
     * @return TInt Integer value of read value
     */
    TInt GetAttributeIntValue( CMDXMLElement& aElement, const TDesC& aAttrib );

    /**
     * Reads attribute value from element and casts it into literal value
     * 
     * @since S60 5.0
     * @param[in] aElement Element from where to read attribute
     * @param[in] aAttrib Attribute name which to read
     * @param[in/out] aTarget Target descriptor where to copy read literal
     */
    void GetAttributeStrValue( CMDXMLElement& aElement, const TDesC& aAttrib, TDes8& aTarget );

    /**
     * Reads attribute value from element and casts it into TReal value
     * 
     * @since S60 5.0
     * @param[in] aElement Element from where to read attribute
     * @param[in] aAttrib Attribute name which to read
     * @return TReal value of the attribute
     */
    TReal GetAttributeRealValue( CMDXMLElement& aElement, const TDesC& aAttrib );

    /**
     * Reads Hexadesimal attribute value from element and casts it into Integer value
     * 
     * @since S60 5.0
     * @param[in] aElement Element from where to read attribute
     * @param[in] aAttrib Attribute name which to read
     * @return TUint Unsigned integer value of read Hexadesimal value
     */
    TUint GetAttributeHexValue( CMDXMLElement& aElement, const TDesC& aAttrib );

    /**
     * Compares Sensor server generated channel IDs and return corresponding 
     * ConfigFile channel id
     * 
     * @since S60 5.0
     * @param[in] aSrvChannelId SenServer generated channel ID for which pair is needed
     * @return TInt ConfigFile channel ID that is paired with aSrvChannelId
     */
    TInt ConfigChannelId( const TInt aSrvChannelId ) const;

    /**
     * Searches channel element for given SensorServer generated channel ID
     * 
     * @since S60 5.0
     * @param[in] aSrvChannelId SenServer generated channel ID identifying wanted channel element
     * @return CMDXMLElement Pointer to found channel element or NULL if not found
     */
    CMDXMLElement* ChannelElement( const TInt aSrvChannelId );

    /**
     * Reads properties from given element and adds them to list. 
     * Element can be either 'SsyGenealInformation' or 'ChannelItem'
     *
     * @since S60 5.0
     * @param[in]  aElement Element from where to read properties
     * @param[out] aPropertyList List where to append properties
     */
    void GetElementPropertiesL( CMDXMLElement& aElement, RSensrvPropertyList& aPropertyList );


private: // data
    
    /**
     * Pointer of the config xml-file parser
     */
    CMDXMLParser* iConfigParser;

    /**
     * Contains Ssy general information element and all of its childs
     */
    CMDXMLElement* iGenralInfoElement;

    /**
     * Contains Ssy Channel information group element and all of its childs
     * including each channel information and channel data for testing purpose
     */
    CMDXMLElement* iChannelGroupElement;

    /**
     * Number of channels defined in config file
     */
    TInt iChannelCount;

    /**
     * Active scheduler wait for blocking construction until config file is parsed
     */
    CActiveSchedulerWait iSchedulerWait;

    /**
     * Indicates is config file parsed
     */
    TBool iConfigFileParsed;

    /**
     * Channel ID pair array
     */
    TSsyRefChannelIdArray iChannelPairArray;

    /**
     * Elements of the config file. These needs to be stored
     */
    CMDXMLDocument* iConfigFile;
    CMDXMLElement*  iSsyReferenceConfig; // Root of the config

    };

#endif //SSYREFERENCECONFIG_H

// End of file
