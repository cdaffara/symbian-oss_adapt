/*
* Copyright (c) 2009 Aricent and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Aricent - initial contribution.
*
* Contributors:
*
* Description:
* Declares plugin class for Mpeg4SP/H263 encoder HwDevice. This class inherits 
* from CMMFVideoEncodeHwDevice and implements the pure virtual functions.The 
* class also contains implementation specific private methods.
*
*/

#ifndef ARIMP4SPENCHWDEVICEIMPL_H
#define ARIMP4SPENCHWDEVICEIMPL_H

//System Includes
#include <Implementationproxy.h>
#include <f32file.h>
#include <e32const.h>
#include <e32base.h>
#include <Videorecordhwdevice.h>

//User Includes
#include "arivideoenccommon.h"
#include "aristatemachine.h"
#include "aribaseengine.h"
#include "arimp4spencwrapper.h"
//#include "arimp4spenchwdevice.h"
#include "arimp4spenchwdeviceuids.hrh"
#include "ariprint.h"

// Forward declarations

/**
 * This class is part of Aricent's Mpeg4-sp/H263 encoder HwDevice plugin used
 * for encoding yuv420 input to Mpeg4-sp/H263 content.
 * Provides implementation for standard MDF HwDevice plugin APIs as well as
 * private functions used internal to this class for .This class also
 * implements callback APIs from MProcessEngineObserver which are called
 * from CBaseEngine.
 */
class CAriMp4spencHwDeviceImpl: public CMMFVideoEncodeHwDevice,
							    public MProcessEngineObserver
{

public:
	/**
	 * Two-phased constructor.
	 * @return  pointer to an instance of CAriMp4spencHwDeviceImpl
	 * @leave	"The method will leave if an error occurs"
	 */
	static CAriMp4spencHwDeviceImpl* NewL();

	/**> Destructor */
	~CAriMp4spencHwDeviceImpl();

public:
	/**
	 * From CMMFVideoHwDevice
	 * The function retrieves a custom interface to the HwDevice.
	 * @param aInterface
	 *    UID of the interface to be retrieved. It is defined with the custom
	 *    interface
	 * @return returns pointer to the interface
     */
    TAny* CustomInterface( TUid aInterface );

public:
	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function retrieves information about the video encoder
	 * @return "returns pointer to the object with encoder information"
	 * @leave  "The method will leave if an error occurs"
     */
	CVideoEncoderInfo* VideoEncoderInfoLC();

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the encoder output format.
	 * @param aFormat
	 *    UID of the interface to be retrieved. It is defined with the custom
	 *    interface
	 * @param aDataUnitType
	 *	  The type of output coded data units
	 * @param aDataEncapsulation
	 *	  Data encapsulation type for output encoded data units
	 * @param aSegmentationAllowed
	 *     Indicates if segmentation is allowed or not
	 * @leave	"The method will leave if an error occurs"
     */
	void SetOutputFormatL( const CCompressedVideoFormat& aFormat,
						TVideoDataUnitType aDataUnitType,
						TVideoDataUnitEncapsulation aDataEncapsulation,
						TBool aSegmentationAllowed=EFalse );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the pre-processor hwdevice that will write data to
	 * this encoder.
	 * @param aDevice
	 *      Pre-processor device that will write to this encoder
     */
	void SetInputDevice( CMMFVideoPreProcHwDevice* aDevice );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the number of bit-rate scalability layers to use.
	 * @param aNumLayers
	 *    The number of bit-rate scalability layers to use
	 * @leave "The method will leave if an error occurs"
     */
	void SetNumBitrateLayersL( TUint aNumLayers );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the scalability type for a bit-rate scalability layer
	 * @param aLayer
	 *    The layer number
	 * @param aScalabilityType
	 *    Layer scalability type
	 * @leave "The method will leave if an error occurs"
     */
	void SetScalabilityLayerTypeL( TUint aLayer,
									TScalabilityType aScalabilityType);

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the reference picture options to be used for all
	 * scalability layers
	 * @param aMaxReferencePictures
	 *    Maximum number of reference pictures to be used
	 * @param aMaxPictureOrderDelay
	 *    The maximum picture order delay, in number of pictures.
     */
	void SetGlobalReferenceOptions( TUint aMaxReferencePictures,
									TUint aMaxPictureOrderDelay );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the reference picture options to be used for a
	 * particular scalability layer
	 * @param aLayer
	 *    The layer number
	 * @param aMaxReferencePictures
	 *    Maximum number of reference pictures to be used for this layer
	 * @param aMaxPictureOrderDelay
	 *    The maximum picture order delay for this layer, in number of
	 *    pictures.
	 */
    void SetLayerReferenceOptions( TUint aLayer,TUint aMaxReferencePictures,
									TUint aMaxPictureOrderDelay );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the encoder buffering options
	 * @param aOptions
	 *    Buffering options to be used
	 * @leave "The method will leave if an error occurs"
     */
	void SetBufferOptionsL( const TEncoderBufferOptions& aOptions );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the encoder output rectangle
	 * @param aRect
	 *    Output rectangle to be used
	 * @leave "The method will leave if an error occurs"
	 */
    void SetOutputRectL( const TRect& aRect );

    /**
	 * From CMMFVideoEncodeHwDevice
	 * The function notifies the encoder whether bit errors or packets losses
	 * can be expected in the video transmission
	 * @param aBitErrors
	 *    Boolean to indicate if bit errors can be expected
	 * @param aPacketLosses
	 *    Boolean to indicate if packet losses can be expected
	 */
    void SetErrorsExpected( TBool aBitErrors, TBool aPacketLosses );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the minimum random access rate to be used
	 * @param aRate
	 *    The minimum random access rate
     */
	void SetMinRandomAccessRate( TReal aRate );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the coding standard specific options to be used.
	 * @param aOptions
	 *    Coding standard specific options to be used
	 * @leave "The method will leave if an error occurs"
	 */
    void SetCodingStandardSpecificOptionsL( const TDesC8& aOptions );

     /**
	  * From CMMFVideoEncodeHwDevice
	  * The function sets  the implementation specific options to be used
	  * @param aOptions
	  *    Implementation specific options to be used.
	  * @leave "The method will leave if an error occurs"
	  */
    void SetImplementationSpecificEncoderOptionsL( const TDesC8& aOptions );

    /**
	 * From CMMFVideoEncodeHwDevice
	 * The function returns coding-standard specific initialization output
	 * from the encoder
	 * @return Returns the pointer to buffer holding coding-standard specific
	 * initialization output
	 * @leave "The method will leave if an error occurs"
	 */
	HBufC8* CodingStandardSpecificInitOutputLC();

	/**
	* From CMMFVideoEncodeHwDevice
	* The function gets the implementation specific initialization output from
	* the encoder
	* @return Returns the pointer to buffer holding implementation specific
	* initialization output
	* @leave "The method will leave if an error occurs"
	*/
	HBufC8* ImplementationSpecificInitOutputLC();

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function the number of unequal error protection levels.
	 * @param aNumLevels
	 *    The number of unequal error protection levels to be used
	 * @param aSeparateBuffers
	 *    Boolean indicating whether each unequal error protection level of a
	 *    coded data unit shall be encapsulated in its own output buffer
	 * @leave "The method will leave if an error occurs"
     */
    void SetErrorProtectionLevelsL( TUint aNumLevels,
									TBool aSeparateBuffers );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function the number of unequal error protection levels.
	 * @param aNumLevels
	 *    The number of unequal error protection levels to be used
	 * @param aSeparateBuffers
	 *    Boolean indicating whether each unequal error protection level of a
	 *    coded data unit shall be encapsulated in its own output buffer
	 * @param aStrength
	 *    Forward error control strength for this error protection level
	 * @leave "The method will leave if an error occurs"
     */
    void SetErrorProtectionLevelL( TUint aLevel, TUint aBitrate,
												TUint aStrength );

	 /**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the expected or prevailing channel conditions for an
	 * unequal error protection
	 * level in terms of expected packet loss rate.
	 * @param aLevel
	 *    Error protection level number
	 * @param aLossRate
	 *    Packet loss rate, in number of packets lost per second.
	 * @param aLossBurstLength
	 *    Expected average packet loss burst length
     */
    void SetChannelPacketLossRate( TUint aLevel, TReal aLossRate,
							TTimeIntervalMicroSeconds32 aLossBurstLength );


	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the expected or prevailing channel conditions for an
	 * unequal error
	 * protection level, in terms of expected bit error rate
	 * @param aLevel
	 *    Error protection level number
	 * @param aErrorRate
	 *    Expected bit error rate
	 * @param aStdDeviation
	 *    Expected bit error rate standard deviation
     */
    void SetChannelBitErrorRate( TUint aLevel, TReal aErrorRate,
								TReal aStdDeviation );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the segment size. To be used only for packet mode
	 * @param aLayer
	 *    Layer number
	 * @param aSizeBytes
	 *    Segment size in bytes
	 * @param aSizeMacroblocks
	 *    Size of the macro blocks
     */
    void SetSegmentTargetSize( TUint aLayer, TUint aSizeBytes,
								TUint aSizeMacroblocks );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the bit-rate control options for a layer
	 * @param aLayer
	 *    Bit-rate scalability layer number
	 * @param aOptions
	 *    Bit-rate control options to be used
     */
	void SetRateControlOptions( TUint aLayer,
								const TRateControlOptions& aOptions );


	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the in-layer scalability options for a layer
	 * @param aLayer
	 *    Bit-rate scalability layer number
	 * @param aNumSteps
	 *    The number of in-layer scalability steps to use
	 * @param aScalabilityType
	 *    Bit-rate share for each scalability step
	 * @param aBitrateShare
	 *    Picture rate share for each scalability step
	 * @param aPictureShare
	 *    The scalability type to use
	 * @leave "The method will leave if an error occurs"
     */
    void SetInLayerScalabilityL( TUint aLayer,TUint aNumSteps,
								TInLayerScalabilityType aScalabilityType,
								const TArray<TUint>& aBitrateShare,
								const TArray<TUint>& aPictureShare );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the segment size. To be used only for packet mode
	 * @param aLayer
	 *    Layer number
	 * @param aPeriod
	 *    The number of in-layer scalability steps to use
     */
    void SetLayerPromotionPointPeriod( TUint aLayer, TUint aPeriod );


	/**
	* From CMMFVideoEncodeHwDevice
	* The function gets the coding-standard specific settings output from the
	* encoder
	* @return Returns the pointer to buffer holding coding-standard specific
	* settings output
	* @leave "The method will leave if an error occurs"
	*/
    HBufC8* CodingStandardSpecificSettingsOutputLC();

    /**
	* From CMMFVideoEncodeHwDevice
	* The function gets the Implementation specific settings output from the
	* encoder
	* @return Returns the pointer to buffer holding Implementation specific
	* settings output
	* @leave "The method will leave if an error occurs"
	*/
    HBufC8* ImplementationSpecificSettingsOutputLC();

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function requests the encoder to send supplemental information in
	 * the bit stream
	 * @param aData
	 *    Supplemental information data to send
	 * @leave "The method will leave if an error occurs"
     */
	void SendSupplementalInfoL( const TDesC8& aData );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function requests the encoder to send supplemental information in
	 * the bit stream
	 * @param aData
	 *    Supplemental information data to send
	 * @param aTimestamp
	 *    Timestamp for the picture in which the supplemental information
	 *    should be included
	 * @leave "The method will leave if an error occurs"
     */
	void SendSupplementalInfoL( const TDesC8& aData,
								const TTimeIntervalMicroSeconds& aTimestamp );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function requests the encoder to cancel the current supplemental
	 * information send request
     */
	void CancelSupplementalInfo();

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function gets the current output buffer status
	 * @param aNumFreeBuffers
	 *    Target for the number of free output buffers
	 * @param aTotalFreeBytes
	 *    Target for the total free buffer size in bytes
     */
	void GetOutputBufferStatus( TUint& aNumFreeBuffers,
								TUint& aTotalFreeBytes );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function returns a used output buffer back to the encoder
	 * @param aBuffer
	 *    The buffer to be returned
     */
    void ReturnBuffer( TVideoOutputBuffer* aBuffer );

    /**
	 * From CMMFVideoEncodeHwDevice
	 * The function notifies the encoder that a picture loss has occurred
	 * without specifying the lost picture.
     */
    void PictureLoss();


    /**
	 * From CMMFVideoEncodeHwDevice
	 * The function notifies the encoder that a picture loss has occurred
	 * @param aPictures
	 *    Picture identifiers of lost pictures
     */
    void PictureLoss( const TArray<TPictureId>& aPictures );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function notify the encoder that slice loss has occurred, giving
	 * the details of the macroblocks and the picture to which they belong
	 * @param aFirstMacroblock
	 *    The first lost macroblock.
	 * @param aNumMacroblocks
	 *    The number of macroblocks in the lost slice
	 * @param aPicture
	 *    The picture identified for the picture where the slice was lost
     */
    void SliceLoss( TUint aFirstMacroblock, TUint aNumMacroblocks,
					const TPictureId& aPicture );

    /**
	 * From CMMFVideoEncodeHwDevice
	 * The function requests the encoder to use reference picture selection
	 * @param aSelectionData
	 *    The reference picture selection request message
     */
    void ReferencePictureSelection( const TDesC8& aSelectionData );


public:

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function initializes the encoder HwDevice with the configuration
	 * settings and reserve
	 * the hardware resources required
     */
	void Initialize();

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function commits all changes since the last CommitL(), Revert() or
	 *  Initialize() to the hardware device
	 * @leave "The method will leave if an error occurs"
	 */
	void CommitL();

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function reverts all changes since the last CommitL(), Revert() or
	 * Initialize()
	 * back to their previous settings.
	 */
	void Revert();

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function writes an uncompressed input picture
	 * @param aPicture
	 *    The picture to write
	 * @leave "The method will leave if an error occurs"
     */
	void WritePictureL( TVideoPicture* aPicture );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function notifies the hardware device that the end of input data
	 * has been reached.
     */
	void InputEnd();

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function starts recording video.
     */
	void Start();

	static TInt TimerCallBack( TAny* aPtr );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function stops recording video
     */
	void Stop();

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function pauses video recording.
     */
	void Pause();


	/**
	 * From CMMFVideoRecordHwDevice
	 * The function resumes video recording after a pause.
     */
	void Resume();


	/**
	 * From CMMFVideoRecordHwDevice
	 * The function freezes the input picture
     */
	void Freeze();


	/**
	 * From CMMFVideoRecordHwDevice
	 * The function releases a frozen input picture.
     */
	void ReleaseFreeze();


	/**
	 * From CMMFVideoRecordHwDevice
	 * The function returns the current recording position
	 * @return Returns the current recording position
     */
	TTimeIntervalMicroSeconds RecordingPosition();


	/**
	 * From CMMFVideoRecordHwDevice
	 * The function reads various counters related to processed video pictures
	 * @param aCounters
	 *    The counter structure to fill
     */
	void GetPictureCounters( CMMFDevVideoRecord::
								TPictureCounters& aCounters );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function reads the frame stabilisation output picture position
	 * @param aRect
	 *	  Output rect which holds frame stabilisation output
     */
    void GetFrameStabilisationOutput( TRect& aRect );

    /**
	 * From CMMFVideoRecordHwDevice
	 * The function retrieves the number of complexity control levels
	 * available for this hardware device
	 * @return Returns the number of complexity level
     */
    TUint NumComplexityLevels();


    /**
	 * From CMMFVideoRecordHwDevice
	 * The function sets the complexity level to use for video processing in a
	 * hardware device
	 * @param aLevel
	 *    The computational complexity level to use.
     */
    void SetComplexityLevel( TUint aLevel );

	/**
	* From CMMFVideoRecordHwDevice
	* The function gets the information about the pre-processing capabilities
	* of the encoder hwdevice
	* @return Returns the pointer to object holding preprocessor info
	* @leave "The method will leave if an error occurs"
	*/
    CPreProcessorInfo* PreProcessorInfoLC();

    /**
	 * From CMMFVideoRecordHwDevice
	 * The function sets the hardware device input format
	 * @param aFormat
	 *    The input format to use
	 * @param aPictureSize
	 *   The input picture size in pixels
	 * @leave "The method will leave if an error occurs"
	 */
	void SetInputFormatL( const TUncompressedVideoFormat& aFormat,
							const TSize& aPictureSize );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets the data source to be a camera
	 * @param aCameraHandle
	 *    Camera handle to be used
	 * @param aPictureRate
	 *    Video capture frame rate
	 * @leave "The method will leave if an error occurs"
     */
    void SetSourceCameraL( TInt aCameraHandle, TReal aPictureRate );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets the data source to be memory buffers
	 * @param aMaxPictureRate
	 *    The maximum picture rate for input pictures.
	 * @param aConstantPictureRate
	 *    Flag indicating if picture rate is constant or not
	 * @param aProcessRealtime
	 *    Flag indicating real time processing should be done or not
	 * @leave "The method will leave if an error occurs"
     */
    void SetSourceMemoryL( TReal aMaxPictureRate, TBool aConstantPictureRate,
							TBool aProcessRealtime );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function enables synchronized encoding and set the clock source to
	 * use for synchronization.
	 * @param aClock
	 *    Clock source to be used.
     */
    void SetClockSource( MMMFClockSource* aClock );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets pre-processing options for RGB to YUV color space
	 * conversion
	 * @param aRange
	 *    Input RGB data range
	 * @param aOutputFormat
	 *    Conversion output YUV format
	 * @leave "The method will leave if an error occurs"
     */
	void SetRgbToYuvOptionsL( TRgbRange aRange,
								const TYuvFormat& aOutputFormat);

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets pre-processing options for YUV to YUV data format
	 * conversion
	 * @param aRange
	 *    Conversion input format
	 * @param aOutputFormat
	 *    Conversion output YUV format
	 * @leave "The method will leave if an error occurs"
     */
	void SetYuvToYuvOptionsL( const TYuvFormat& aInputFormat,
								const TYuvFormat& aOutputFormat );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets the pre-processing types to be used
	 * @param aPreProcessTypes
	 *    The pre-processing steps to perform, a bitwise OR of values from
	 *    TPrePostProcessType
	 * @leave "The method will leave if an error occurs"
     */
	void SetPreProcessTypesL( TUint32 aPreProcessTypes );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets pre-processing options for rotation
	 * @param aRotationType
	 *    The rotation to perform
	 * @leave "The method will leave if an error occurs"
     */
	void SetRotateOptionsL( TRotationType aRotationType );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets pre-processing options for scaling.
	 * @param aTargetSize
	 *    Target picture size.
	 * @param aAntiAliasFiltering
	 *   True if anti-aliasing filtering should be used
	 * @leave "The method will leave if an error occurs"
     */
	void SetScaleOptionsL( const TSize& aTargetSize,
							TBool aAntiAliasFiltering );


	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets pre-processing options for input cropping.
	 * @param aRect
	 *    The input cropping rectangle specifying the area of the picture to
	 *    use
	 * @leave "The method will leave if an error occurs"
     */
	void SetInputCropOptionsL( const TRect& aRect );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets pre-processing options for output cropping.
	 * @param aRect
	 *    The output cropping rectangle specifying the area of the picture to
	 *     use
	 * @leave "The method will leave if an error occurs"
     */
	void SetOutputCropOptionsL( const TRect& aRect );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets pre-processing options for output padding
	 * @param aOutputSize
	 *    The padded output picture size.
	 * @param aPicturePos
	 *   The position for the original picture in the new padded picture.
	 * @leave "The method will leave if an error occurs"
     */
	void SetOutputPadOptionsL( const TSize& aOutputSize,
								const TPoint& aPicturePos );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets color enhancement pre-processing options.
	 * @param aOptions
	 *    Color enchancement options
	 * @leave "The method will leave if an error occurs"
     */
	void SetColorEnhancementOptionsL(
									const TColorEnhancementOptions& aOptions);

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets frame stabilisation options
	 * @param aOutputSize
	 *    Output picture size.
	 * @param aFrameStabilisation
	 *   True if frame stabilisation should be used
	 * @leave "The method will leave if an error occurs"
     */
	void SetFrameStabilisationOptionsL( const TSize& aOutputSize,
										TBool aFrameStabilisation );

	/**
	 * From CMMFVideoRecordHwDevice
	 * The function sets custom implementation-specific pre-processing options
	 * @param aOptions
	 *   Post-processing options
	 * @leave "The method will leave if an error occurs"
     */
	void SetCustomPreProcessOptionsL( const TDesC8& aOptions );


    /**
	 * From CMMFVideoEncodeHwDevice
	 * The function sets the proxy implementation to be used
	 * @param aProxy
	 *    The proxy to use
     */
	void SetProxy( MMMFDevVideoRecordProxy& aProxy );



public:

	/**
	 * From MProcessEngineObserver
	 * The function is a callback to indicate the input buffer is consumed
	 * @param aInp
	 *    Pointer to the input picture that has been processed
	 * @param aError
	 *   Error code returned by process engine
	 * @return error value
	 */
	TInt InputBufferConsumed( TAny* aInp, TInt aError);

	/**
	 * From MProcessEngineObserver
	 * The function is a callback to indicate the output buffer is ready
	 * @param aOup
	 *    Pointer to the output picture that has been processed
	 * @param aError
	 *   Error code returned by process engine
	 * @return error value
	 */
	TInt OutputBufferReady( TAny* aOup, TInt aError  );

	/**
	 * From MProcessEngineObserver
	 * The function indicates to hwdevice that process engine has finished
	 * with the processing of command requested by hwdevice
	 * @param aCmd
	 *    Command that has been processed by process engine
	 * @param aCmdData
	 *   Pointer to command data that has been processed by process engine
	 * @param aError
	 *   Error code corresponding to the command
	 */
	void CommandProcessed( TInt aCmd, TAny* aCmdData, TInt aError );

	/**
	 * From CMMFVideoEncodeHwDevice
	 * The function indicates to hwdevice that process engine has met with an
	 * unrecoverable error during its processing
	 * @param aError
	 *    The fatal error code
     */
	void FatalErrorFromProcessEngine( TInt aError );

private:

	/**
	 * The function is the default constructor for class
	 * CAriMp4spencHwDeviceImpl
     */
	CAriMp4spencHwDeviceImpl();

	/**
	 * Symbian 2 phase constructor
	 */
	void ConstructL();

	/**
	 * Updates the time periodically i.e  gets the clock source values from
	 * client
	 * @return returns system wide error code on error
	 */
	TInt UpdateTime();

	/**
	 * Creates coded Output buffers for the hw device.
	 * @param aSize
	 *    Size of buffers to be created
	 */
	void CreateCodedOutputBuffersL( TUint aSize );


	/**
	 * Creates Internal buffers in segment mode, to hold the complete
	 * encoded frame returned from the codec
	 * @param aSize
	 *    Size of buffers to be created
	 */
	void CreateInternalOutputBuffersL( TUint aBufferSize );

	/**
	 * Creates packet offset info buffers
	 * @param aSize
	 *    Size of buffers to be created
	 */
	void CreatePacketOffsetLengthInfoBuffersL( TUint aSize );

	/**
	 * Decides whether this input picture can be encoded or not
	 * @param aPicture
	 *    Pointer to the input picture
	 */
	TBool CanEncode( TVideoPicture *aPicture );

	/**
	 * Input picture is not processed, returned back to client
	 * @param aPicture
	 *    Pointer to the input picture
	 */
	void SkipInputPicture( TVideoPicture *aPicture );


	/**
	 * Checks whether this input picture is coded as i-frame or not
	 * @param aPicture
	 *    Pointer to the input picture
	 */
	TBool IsForcedIFrameRequired( TVideoPicture* aPicture );

	/**
	 * Fills one segment data from the internal buffer to output buffer
	 * @param aOutBuf
	 *    Destination buffer to hold the packet
	 * @param aSrcBuf
	 *    Buffer which contains encoder output frame
	 */
	void FillVideoSegment( TVideoOutputBuffer* aOutBuf,
							TVideoOutputBuffer* aSrcBuf );


    /**
	 * Initializes the members of the output coded buffers created
	 * @param aOutputBuffer
	 *   Output buffer for which members are to be initialized
	 */
	void InitializeOuputCodedBuffer( TVideoOutputBuffer& aOutputBuffer );

	/**
	 * Checks if the specified input format is supported or not
	 * @param aFormat
	 *    Input format to be checked
	 * @return Returns True if input format is supported else false
	 */
	TBool CheckInputFormat( const TUncompressedVideoFormat& aFormat );

	/**
	 * Notifies the client that the hw device has encountered with error
	 * @param aError
	 *    Error code indicating type of fatal error
	 */
	void ClientFatalError ( TInt aError );

private:

	/**
	 * Handle to proxy i.e observer
	 */
	MMMFDevVideoRecordProxy*	iMMFDevVideoRecordProxy;

	/**
	 * Handle to preprocessor hw device
	 */
    CMMFVideoPreProcHwDevice*	iInputDevice;

	/**
	 * Reference to clock source if client
	 */
	MMMFClockSource*			iClockSource;

	/**
	 * Flag to indicate whether the buffer is returned to inputdevice
	 */
	TBool						iInputBufReturnToPreProc;

	/**
	 * Hold the parameters that are required after initialize
	 */
	TMpeg4H263EncoderInitParams  iCurSetMpeg4H263HWParams;

	/**
	 * Hold all the parameters before  initialize
	 */
	TMpeg4H263EncoderInitParams   iSetMpeg4H263HWParams;

	/**
	 * pointer to state machine object
	 */
	CStateMachine*				  iEncStateMac;

	/**
	 * All levels information is stored
	 */
	RArray<TInt>				  iLevels;

	/**
	 * Supported Input format information
	 */
	RArray<TUncompressedVideoFormat>  iSupportedInputFormats;

	/**
	 * Supported output formats information
	 */
	RArray<CCompressedVideoFormat*>	  iSupportedOutputFormats;

	/**
	 * All supported data unit types
	 */
	TUint32							  iSupportedDataUnitTypes;

	/**
	 * All supported data unit encapsulation
	 */
	TUint32							  iSupportedDataUnitEncapsulations;

	/**
	 * Timer for updates
	 */
	CPeriodic*						  iPeriodicTimer;

	/**
	 *  Stores tha value when clock is paused
	 */
	TInt64							  iClockTimeWhenPaused;

	/**
	 * Polling interval to be used
	 */
	TTimeIntervalMicroSeconds32		  iPollingInterval;

	/**
	 * Codec Reference
	 */
	CAriMp4spencWrapper*			  iCodec;

	/**
	 * Pointer to base process engine object
	 */
	CBaseEngine*					  iEngine;

	/**
	 * Output Free buffers can be segment or coded buffers
	 */
	TVideoOutputBuffer*				  iOutputBuffers;

	/**
	 * Output buffer queue
	 */
	RPointerArray<TVideoOutputBuffer> iOutputFreeBufferQueue;

	/**
	 * Output buffer size of the coded picture
	 */
	TInt							  iOutputBufferSize;


	/**
	 * Pause offset i.e offset between pause and resume
	 */
	TInt64							  iPauseOffset;

	/**
	 * Total time deviation from client clock source
	 */
	TInt64							  iTotalTime;

	/**
	 * Last encoded picture time stamp
	 */
	TInt64							  iLastEncodedPictureTimestamp;

	/**
	 * Indicates picture loss
	 */
	TBool							  iPictureLoss;

	/**
	 * Indicates input end called
	 */
	TBool							  iInputEndCalled;

	/**
	 * Picture Counters
	 */
	CMMFDevVideoRecord::TPictureCounters	iPictureCounters;

	/**
	 * Indicates whether HwDevice is frozen by calling Freeze ()
	 */
	TBool 							  iFrozen;

	/**
	 * Current postion in the coded frame in packet mode
	 */
	TUint							  iTotalLengthFilledSoFarInPacketMode;

	/**
	 * Total Output buffer Length of the coded picture in packet mode
	 */
	TUint						      iTotalOutputBufferLengthInPacketMode;

	/**
	 * Pointer to current offset position within a frame - used in packet mode
	 * only
	 */
	TUint*							  iPacketOffSetCurrentPosition;

	/**
	 * Holds offsets and length information array
	 */
	TUint**							  iPacketOffSetAndLengthInfoBuffers;

	/**
	 * Free buffer queue for packet offset information
	 */
	RPointerArray<TUint>			  iFreeBufferQueueForPacketOffsetInfo;

	/**
	 * Filled buffer queue for packet offset information
	 */
	RPointerArray<TUint>			  iFilledBufferQueueForPacketOffsetInfo;

	/**
	 * Temporary output buffers
	 */
	TVideoOutputBuffer*				  iInternalOutputBuffers;

	/**
	 * Holds temporary o/p buffers
	 */
	RPointerArray<TVideoOutputBuffer> iInternalOutputBufferQueue;

	/**
	 * Indicates if all the packets in a frame are sent to above layer or not
	 */
	TBool							  iPacketsPending;

	/**
	 * Stores config data
	 */
	HBufC8* 						  iConfigData;

	/**
	 * Stores value of max no of buffers
	 */
	TUint                             iMaxNumOfPackets;

	/**
	 * Buffer to store packet length info
	 */
	TPtr8*                            iPacketOffsetBuf;
};

#endif //ARIMP4SPENCHWDEVICEIMPL_H
