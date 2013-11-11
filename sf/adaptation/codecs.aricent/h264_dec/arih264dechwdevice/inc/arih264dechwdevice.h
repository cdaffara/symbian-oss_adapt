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
* Interface of the arih264dechwdevice.
*
*/

#ifndef ARIH264DECHWDEVICE_H
#define ARIH264DECHWDEVICE_H

//System includes
#include <videoplayhwdevice.h>
#include <ImplementationProxy.h>
#include <buffermanagementci.h>

//User includes
#include "aribaseengine.h"
#include "arih264dechwdeviceuid.hrh"

// CONSTANTS
const TInt KMaxAllowPicLoss		= 75;
const TInt KMaxInputBufferSize  = ( 248 * 2048 );
const TInt KMaxOutputBuffers    = 2;
const TInt KMaxInputBuffers		= 2;
const TInt KMinInputBuffers		= 2;
// Implementatin id for the hwdevice
const TUid KUidH264DecoderHwDevice = {KUidH264DecoderHwDeviceImplUid};

const TUint KMaxBitRate = 15*1024*1024;
const TInt KDecoderInfoCount = 14;

//const TInt KMaxFrameWidth			= 1280;
//const TInt KMaxFrameHeight		= 720;
const TInt KPicRate					= 30;

// FORWARD DECLARTIONS
class CAriH264decWrapper;
class CStateMachine;
//CLASS DECLARATIONS

/**
 * This class is part of Aricent's H264 decoder HwDevice plugin used for
 * decoding H264 content to yuv420.
 * Provides implementation for standard MDF HwDevice plugin APIs as well as private functions
 * used internal to this class for .This class also implements callback APIs from
 * MMmfVideoBufferManagementObserver and MProcessEngineObserver which are called from CBaseEngine.
 */
class CAriH264decHwDevice: public CMMFVideoDecodeHwDevice,
						   public MProcessEngineObserver,
						   public MMmfVideoBufferManagementObserver
{
public:	//Constructors and destructor

	/**
	 * Two-phased constructor.
	 * @return pointer to an instance of CMMFVideoDecodeHwDevice
	 */
	static CMMFVideoDecodeHwDevice* NewL();

	/**
	 * Function to destroy resources allocated by this object
	 */
	virtual ~CAriH264decHwDevice();

public:	//CMMFVideoHwDevice Functions

   /**
	* Retrieves a custom interface to the specified hardware device

	* @param	aInteface	Interface UID, defined with the custom interface
	* @return	Pointer to the interface implementation, or NULL if the
	*           device does not implement the interface requested.
	*       	The return value must be cast to the correct type by the user.
	*/
	virtual TAny* CustomInterface( TUid aInterface );

public:	//CMMFVideoDecodeHwDevice Functions

    /**
	 * Retrieves decoder information about this hardware device.
	 * The device creates a CVideoDecoderInfo structure,
	 * fills it with correct data, pushes it to cleanup stack and returns it.
	 * The client will delete the object when it is no longer needed

	 * @return	Decoder information as a CVideoDecoderInfo object.
	 * The object is pushed to the cleanup stack, and must be deallocated by
	 * the caller.

	 * @leave	The method will leave if an error occurs.
	 */
	virtual CVideoDecoderInfo* VideoDecoderInfoLC();

   /**
	* Reads header information from a coded data unit.

	* @param aDataUnitType   The type of the coded data unit that is contained
	*                        in aDataUnit.
	*						 If the data is a simple piece of bitstream, use
	*						 EDuArbitraryStreamSection.
	* @param aEncapsulation  The encapsulation type used for the coded data.
	*                        If the data is a simple piece of bitstream, use
	*                        EDuElementaryStream.
	* @param aDataUnit	     The coded data unit, contained in a
	*                        TVideoInputBuffer.

	* @return				 Header information for the data unit, or NULL if
	*                        the coded data unit did not contain enough data
	*                        to parse the header.The header data must be
	* 						 returned to device using ReturnHeader() before
	*                        Initialize() is called or decoder is destroyed.
	* 						 The data remains valid until it is returned.

	* @leave				 This method may leave with one of the standard
	*                        error codes.

	* @pre					 This method can only be called before hwdevice
	*                        has been initialized with Initialize()
	*/
	virtual TVideoPictureHeader* GetHeaderInformationL(
			                       TVideoDataUnitType aDataUnitType,
		                           TVideoDataUnitEncapsulation aEncapsulation,
		                           TVideoInputBuffer* aDataUnit );

   /**
	* Returns a header from GetHeaderInformationL() back to the decoder so
	* that the memory can be freed.

	* @param aHeader	The header to return

	* @pre				This method can only be called before the hwdevice
	*                    has been initialized with Initialize()
	*/
	virtual void ReturnHeader( TVideoPictureHeader* aHeader );

   /**
	* Sets the device input format to a compressed video format.

	* @param aFormat		  The input format to use
	* @param aDataUnitType	  The encapsulation type used for the coded data
	* @param aEncapsulation   The encapsulation type used for the coded data
	* @param aDataInOrder	  True if the input data is written in correct
	*                         decoding order, false if will be written in
	*                         arbitrary order.

	* @leave				  This method may leave with one of the standard
	*                         error codes.

	* @pre					  This method can only be called before hwdevice
	*                          has been initialized with Initialize()
	*/
	virtual void SetInputFormatL( const CCompressedVideoFormat& aFormat,
								   TVideoDataUnitType aDataUnitType,
								   TVideoDataUnitEncapsulation aEncapsulation,
								   TBool aDataInOrder );

   /**
	* Sets whether decoding should be synchronzed to the current clock source,
	* if any, or if pictures should instead be decoded as soon as possible.
	* If decoding is synchronized, decoding timestamps are used if available,
	* presentation timestamps are used if not. When decoding is not
	* synchronized, pictures are decoded as soon as source data is available
	* for them, and the decoder has a free output buffer. If a clock source is
	* not available, decoding will not be synchronized.

	* @param aSynchronize	True if decoding should be synchronized to a clock
	*                       source.

	* @pre					This method can only be called before the hwdevice
	*                       has been initialized with Initialize()
	*/
	virtual void SynchronizeDecoding( TBool aSynchronize );

   /**
	* Sets decoder buffering options

	* @param aOptions   Buffering options

	* @leave			This method may leave with one of the standard error
	*                   codes.

	* @pre				This method can only be called before the hwdevice has
	*                   been initialized with Initialize()
	*/
	virtual void SetBufferOptionsL(
			               const CMMFDevVideoPlay::TBufferOptions& aOptions );

   /**
	* Gets the video decoder buffer options actually in use. This can be used
	* before calling SetBufferOptions() to determine the default options, or
	* afterwards to check the values actually in use ( if some default values
	* were used ).

	* @param aOptions  Buffering options structure to fill.

	* @pre			   This method can only be called before the hwdevice has
	*                  been initialized with Initialize()
	*/
	virtual void GetBufferOptions(
			                     CMMFDevVideoPlay::TBufferOptions& aOptions );

   /**
	* Indicates which HRD/VBV specification is fulfilled in the input stream
	* and any related parameters.

	* @param aHrdVbvSpec	The HRD/VBV specification fulfilled
	* @param aHrdVbvParams	HRD/VBV parameters. The data format depends on the
	*                       parameters chosen. For 3GPP TS 26.234. parameters
	*                       aHrdVbvSpec=EHrdVbv3GPP, data in the descriptor
	*                       is a package of type TPckC<T3gppHrdVbvParams>
	* 						If no HRD/VBV parameters are used, the descriptor
	*                       is empty.

	* @pre					This method can only be called before the hwdevice
	*                       has been initialized with Initialize()
	*/
	virtual void SetHrdVbvSpec( THrdVbvSpecification aHrdVbvSpec,
			                   const TDesC8& aHrdVbvParams );

   /**
	* Sets the output post-procesor device to use. If an output device is set,
	* all decoded pictures are delivered to that device, and not drawn on
	* screen or returned to the client. Pictures are written using
	* CMMDVideoPostProcDevice::WritePictureL() or a custom interface after
	* they have been decoded. The post-processor must then synchronize
	* rendering to the clock source if necessary.
	*
	* @param aDevice	 The output post-processor device to use.

	* @pre				 This method can only be called before the hwdevice
	*                     has been initialized with Initialize()
	*/
	virtual void SetOutputDevice( CMMFVideoPostProcHwDevice* aDevice );

   /**
	* Returns the current decoding position, i.e. the timestamp for the most
	* recently decoded picture.

	* @return		Current decoding position.

	* @pre			This method can only be called before the hwdevice has
	*                been initialized with Initialize()
	*/
	virtual TTimeIntervalMicroSeconds DecodingPosition();

   /**
	* Returns the current pre-decoder buffer size

	* @return		The number of bytes of data in the pre-decoder buffer.

	* @pre			This method can only be called after the hwdevice has been
	*               initialized with Initialize().
	*/
	virtual TUint PreDecoderBufferBytes();

   /**
	* Reads various counters related to the received input bitstream and coded
	* data units.

	* @param aCounters     The counter structure to fill.

	* @pre				   This method can only be called after the hwdevice
	*                      has been initialized with Initialize().
	*/
	virtual void GetBitstreamCounters(
			                CMMFDevVideoPlay::TBitstreamCounters& aCounters );

   /**
	* Retrieves the number of free input buffers the decoder has available

	* @return	Number of free input buffers the decoder has available.

	* @leave	This method may leave with one of the standard error codes.

	* @pre		This method can only be called after the hwdevice has been
	*            initialized with Initialize().
	*/
	virtual TUint NumFreeBuffers();

   /**
	* Retrieves an empty video input buffer from the decoder. After input data
	* has been written to the buffer, it can be written to the decoder using
	* WriteCodedDataL(). The number of buffers the decoder must be able
	* to provide before expecting any back, and the maximum size for each
	* buffer, are specified in the buffer options.The decoder maintains
	* ownership of the buffers even while they have been retrieved by client,
	* and will take care of deallocating them.

	* @param aBufferSize	Required buffer size, in bytes. The resulting
	*                        buffer can be larger than this, but not smaller

	* @return				A new input data buffer. The buffer is at least
	*                        as large as requested, but it may be larger.
	* 						If no free buffers are available, the return value
	*                       is NULL.

	* @leave				This method may leave with one of the standard
	*                        error codes.

	* @pre					This method can only be called after the hwdevice
	*                        has been initialized with Initialize().
	*/
	virtual TVideoInputBuffer* GetBufferL( TUint aBufferSize );

   /**
	* Writes a piece of coded video data to the decoder. The data buffer must
	* be retrieved from the decoder with GetBufferL().

	* @param aBuffer	The coded data unit to write.

	* @leave				This method may leave with one of the standard
	*                        error codes.

	* @pre				This method can only be called after the hwdevice has
	*                    been initialized with Initialize().
	*/
	virtual void WriteCodedDataL( TVideoInputBuffer* aBuffer );

#ifdef SYMBIAN_MDF_API_V2
   /**
	* Configures the Decoder using header information known by the client.
	* @param	aVideoPictureHeader	Header information to configure the
	*                                    decoder with
	* @leave	The method will leave if an error occurs. Running out of data
	*            is not considered an error,
	* 		as described above.
	* @pre	This method can only be called before the hwdevice has been
	*        initialized with Initialize().
	*/
    virtual void ConfigureDecoderL(
    		                 const TVideoPictureHeader& aVideoPictureHeader );

#endif

public:	//Inherited from CMMFVideoPlayHwDevice

   /**
	* Retrieves post-processing information about this hardware device. The
	* device creates a CPostProcessorInfo structure ( defined in [3] ), fills
	* it with correct data, pushes it to the cleanup stack and returns it. The
	* client will delete the object when it is no longer needed.

	* @return		Post-processor information as a CPostProcessorInfo object.
	* 				The object is pushed to the cleanup stack, and must be
	*               deallocated by the caller.

	* @leave		This method may leave with one of the standard error codes
	*/
	virtual CPostProcessorInfo* PostProcessorInfoLC();

   /**
	* Retrieves the list of the output formats that the device supports. The
	* list is ordered in plug-in preference order, with the preferred formats
	* at the beginning of the list. The list can depend on the device source
	* format, and therefore SetSourceFormatL() must be called before calling
	* this method.

	* @param aFormats		An array for the result format list. The array
	*                       must be created and destroyed by the caller.

	* @leave				This method may leave with one of the standard
	*                       error codes.

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void GetOutputFormatListL(
			                     RArray<TUncompressedVideoFormat>& aFormats );

   /**
	* Sets the device output format.

	* @param aDataUnit	    The format to use.

	* @leave				This method may leave with one of the standard
	*                       error codes.

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void SetOutputFormatL( const TUncompressedVideoFormat &aFormat );

   /**
	* Sets the clock source to use for video timing. See [3] for a discussion
	* about audio/video synchronization. If no clock source is set. video
	* playback will not be synchronized, but will proceed as fast as possible,
	* depending on input data and output buffer availability.

	* @param aDataUnitType  The clock source to use.

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void SetClockSource( MMMFClockSource* aClock );

   /**
	* Sets the device video output destination. The destination can be the
	* screen ( using direct screen access ) or memory buffers. By default
	* memory buffers are used. If data is written to another device, this
	* method is ignored, and suitable memory buffers are always used.

	* @param aScreen  True if video output destination is the screen, false
	*                 if memory buffers.

	* @leave		  This method may leave with one of the standard error
	*                 codes.

	* @pre			  This method can only be called after the hwdevice has
	*                 been initialized with Initialize().
	*/
	virtual void SetVideoDestScreenL( TBool aScreen );

   /**
	* Sets the post-processing types to be used.

	* @param aPostProcCombination   The post-processing steps to perform, a
	*                               bitwise or of values from TPostProcessType
	* @leave						This method may leave with one of the
	*                               standard error codes.

	* @pre							This method can be called either before
	*                               or after the hwdevice has been initialized
	*								with Initialize(). If called after
	*								with initialization, the change will only
	*								with be committed once CommitL() is called
	*/
	virtual void SetPostProcessTypesL( TUint32 aPostProcCombination );

   /**
	* Sets post-processing options for input ( pan-scan ) cropping.

	* @param aRect  The type of the coded data unit that is contained in
	*               aDataUnit.

	* @leave		This method may leave with one of the standard error codes

	* @pre			This method can be called either before or after the
	*               hwdevice has been initialized with Initialize(). If called
	*               after initialization, the change will only be committed
	*               once CommitL() is called.
	*/
	virtual void SetInputCropOptionsL( const TRect& aRect );

   /**
	* Sets post-processing options for YUV to RGB color space conversion. The
	* first variant specifies the input YUV and output RGB formats to use
	* explicitly, while the second variant uses the device input and output
	* formats. For decoder devices the default YUV format used is the format
	* specified in the input bitstream. SetSourceFormatL(), SetOutputFormatL()
	* and SetPostProcessTypesL() must be called before this method is used

	* @param aOptions	 The conversion options to use
	* @param aYuvFormat	 Conversion source YUV format, if specified.
	* @param aRgbFormat	 Conversion target RGB format, if specified..

	* @leave			 This method may leave with one of the standard error
	*                    codes.

	* @pre				 This method can be called either before or after the
	*                    hwdevice has been initialized with Initialize(). If
	*                    called after initialization, the change will only be
	*					 committed once CommitL() is called.
	*/
	virtual void SetYuvToRgbOptionsL( const TYuvToRgbOptions& aOptions,
									 const TYuvFormat& aYuvFormat,
									 TRgbFormat aRgbFormat );
   /**
	* Sets post-processing options for YUV to RGB color space conversion. The
	* first variant specifies the input YUV and output RGB formats to use
	* explicitly, while the second variant uses the device input and output
	* formats. For decoder devices the default YUV format used is the format
	* specified in the input bitstream. SetSourceFormatL(), SetOutputFormatL()
	* and SetPostProcessTypesL() must be called before this method is used

	* @param aOptions	 The conversion options to use

	* @leave			 This method may leave with one of the standard error
	*                    codes.

	* @pre				 This method can be called either before or after the
	*                    hwdevice has been initialized
	*					 with Initialize(). If called after initialization,
	*					 the change will only be committed once CommitL() is
	*					 called
	*/
	virtual void SetYuvToRgbOptionsL( const TYuvToRgbOptions& aOptions );

   /**
	* Sets post-processing options for YUV to RGB color space conversion. The
	* first variant specifies the input YUV and output RGB formats to use
	* explicitly, while the second variant uses the device input and output
	* formats. For decoder devices the default YUV format used is the format
	* specified in the input bitstream. SetSourceFormatL(), SetOutputFormatL()
	* and SetPostProcessTypesL() must be called before this method is used

	* @param aRotationType The rotation to perform.

	* @leave			  This method may leave with one of the standard
	*                     error codes.

	* @pre				  This method can be called either before or after
	*                     the hwdevice has been initialized with Initialize().
	*                     If called after initialization, the change will only
	*                     be committed once CommitL() is called.
	*/
	virtual void SetRotateOptionsL( TRotationType aRotationType );

   /**
	* @param aRotationType The rotation to perform.

	* @leave			  This method may leave with one of the standard
	*                     error codes.

	* @pre				  This method can be called either before or after
	*                     the hwdevice has been initialized with Initialize().
	*                     If called after initialization, the change will
	*                     only be committed once CommitL() is called.
	*/
	virtual void SetScaleOptionsL( const TSize& aTargetSize,
			                      TBool aAntiAliasFiltering );

   /**
	* Sets post-processing options for output cropping. SetPostProcessTypesL()
	* must be called before this method is used.

	* @param aRect Output cropping area

	* @leave	   This method may leave with one of the standard error codes.

	* @pre		   This method can be called either before or after the
	*              hwdevice has been initialized with Initialize(). If called
	*              after initialization, the change will only be committed
	*              once CommitL() is called.
	*/
	virtual void SetOutputCropOptionsL( const TRect& aRect );

   /**
	* Sets post-processing plug-in specific options. SetPostProcessTypesL()
	* must be called before this method is used.

	* @param aOptions  The format is plug-in specific

	* @leave		   This method may leave with one of the standard error
	*                  codes.

	* @pre		       This method can be called either before or after the
	*                  hwdevice has been initialized with Initialize(). If
	*                  called after initialization, the change will only be
	* 			       committed once CommitL() is called.
	*/
	virtual void SetPostProcSpecificOptionsL( const TDesC8& aOptions );

   /**
	* Initializes the device. This method is asynchronous, the device will
	* call MMFVideoPlayProxy::MdvppInitializeComplete() after initialization
	* has completed. After this method has successfully completed, further
	* configuration changes are not possible except where separately noted

	* @leave	   This method may leave with one of the standard error codes.
	*/
	virtual void Initialize();

   /**
	* Commit all configuration changes since the last CommitL(), Revert() or
	* Initialize(). This only applies to methods that can be called both
	* before AND after the hwdevice has been initialized.

	@leave		   This method may leave with one of the standard error codes.
	*/
	virtual void CommitL();

   /**
	* Revert any configuration changes that have not yet been committed using
	* CommitL(). This only applies to methods that can be called both before
	* AND after the hwdevice has been initialized.

	* @leave	This method may leave with one of the standard error codes.

  	* @pre		This method can only be called after the hwdevice has been
  	*           initialized with Initialize().
	*/
	virtual void Revert();

   /**
	* Starts writing output directly to the display frame buffer using Direct
	* Screen Access.

	* @param aVideoRect		The video output rectangle on screen.
	* @param aScreenDevice	The screen device to use. The screen device object
	*                       must be valid in the current thread.
	* @param aClipRegion    Initial clipping region to use.

	* @leave				This method may leave with one of the standard
	*                       error codes.

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void StartDirectScreenAccessL( const TRect& aVideoRect,
											CFbsScreenDevice& aScreenDevice,
											const TRegion& aClipRegion );

   /**
	* Sets a new clipping region for Direct Screen Access. After the method
	* returns, no video will be drawn outside of the region. If clipping is
	* not supported, or the clipping region is too complex, either playback
	* will pause or will resume without video display, depending on the
	* current setting of SetPauseOnClipFail(), and the result can be verified
	* with IsPlaying(). Clipping can be disabled by setting a new clipping
	* region that includes the whole video window.

	* @param aRegion		The new clipping region. After the method returns,
	*                       no video will be drawn outside the region.

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void SetScreenClipRegion( const TRegion& aRegion );


   /**
	* Sets whether the system should pause playback when it gets a clipping
	* region it cannot handle, or Direct Screen Access is aborted completely.
	* If not, processing will proceed normally, but no video will be drawn.
	* By default, playback is paused.

	* @param aPause		True if playback should be paused when clipping fails,
	*                   false if not.
	*					If playback is not paused, it will be continued
	*					If without video display
    *
	* @pre				This method can only be called after the hwdevice has
	*                   been initialized with Initialize().
	*/
	virtual void SetPauseOnClipFail( TBool aPause );

   /**
	* Aborts Direct Screen Access completely, to be called from
	* MAbortDirectScreenAccess::AbortNow() and similar methods. DSA can be
	* resumed by calling StartDirectScreenAccessL().

	* @pre		This method can only be called after the hwdevice has been
	* initialized with Initialize().
	*/
	virtual void AbortDirectScreenAccess();

   /**
	* Indicates Whether playback is proceeding
	* @return		ETrue if video is still being played ( even if not
	*               necessarily displayed )

	* @pre		    This method can only be called after the hwdevice has been
	*               initialized with Initialize().
	*/
	virtual TBool IsPlaying();

   /**
	* Re-draws the latest video picture.Only available when DSA is being used.
	* If DSA is aborted or a non-supported clipping region has been set, the
	* request may be ignored.

	* @pre		This method can only be called after the hwdevice has been
	*           initialized with Initialize().
	*/
	virtual void Redraw();

   /**
	* Starts video playback, including decoding, post-processing, and
	* rendering. Playback will proceed until it has been stopped or paused, or
	* the end of the bitstream is reached.

	* @pre		This method can only be called after the hwdevice has been
	*           initialized with Initialize().
	*/
	virtual void Start();

   /**
	* Stops video playback. No new pictures will be decoded, post-processed,
	* or rendered.

	* @pre		This method can only be called after the hwdevice has been
	*           initialized with Initialize().
	*/
	virtual void Stop();

   /**
	* Pauses video playback, including decoding, post-processing, and
	* rendering. No pictures will be decoded, post-processed, or rendered
	* until playback has been resumed.

	* @pre		This method can only be called after the hwdevice has been
	*           initialized with Initialize().
	*/
	virtual void Pause();

   /**
	* Resumes video playback after a pause.

	* @pre		This method can only be called after the hwdevice has been
	*           initialized with Initialize().
	*/
	virtual void Resume();

   /**
	* Changes to a new decoding and playback position, used for randomly
	* accessing ( seeking ) the input stream. The position change flushes all
	* input and output buffers, pre-decoder and post-decoder buffering are
	* handled as if a new bitstream was. If the device still has buffered
	* pictures that precede the new playback position, they will be discarded.
	* If playback is synchronized to a clock source, the client is responsible
	* for setting the clock source to the new position.

	* @param aPlaybackPositio	The new playback position in the video stream.

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void SetPosition(
			             const TTimeIntervalMicroSeconds& aPlaybackPosition );

   /**
	* Freezes a picture on the screen. After the picture has been frozen, no
	* new pictures are displayed until the freeze is released with
	* ReleaseFreeze(). If the device output is being written to memory buffers
	* or to another plug-in, instead of the screen, no decoded pictures will
	* be delivered while the freeze is active, and they are simply discarded.

	* @param aTimestamp		The presentation timestamp of the picture to
	*                       freeze. The frozen picture will be the first
	*                       picture with a timestamp greater than or equal to
	*                       this parameter

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void FreezePicture( const TTimeIntervalMicroSeconds& aTimestamp );

   /**
	* Releases a picture frozen with FreezePicture()

	* @param aTimestamp		The presentation timestamp of the picture to
	*                       release. The first picture displayed after the
	*                       release will be the first picture with a timestamp
	*						greater than or equal to this parameter. To
	*						greater release the freeze immediately,	set the
	*						greater timestamp to zero.

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void ReleaseFreeze( const TTimeIntervalMicroSeconds& aTimestamp );

   /**
	* Returns the current playback position, i.e. the timestamp for the most
	* recently displayed or virtually displayed picture. If the device output
	* is written to another device, the most recent output picture is used.

	* @return	Current playback position

	* @pre		This method can only be called after the hwdevice has been
	*           initialized with Initialize().
	*/
	virtual TTimeIntervalMicroSeconds PlaybackPosition();

   /**
	* Returns the total amount of memory allocated for uncompressed pictures.
	* This figure only includes the pictures actually allocated by the plug-in
	* itself, so that the total number of bytes allocated in the system can be
	* calculated by taking the sum of the values from all plug-ins.

	* @return		Total number of bytes of memory allocated for uncompressed
	*               pictures.

	* @pre		    This method can only be called after the hwdevice has been
	*               initialized with Initialize().
	*/
	virtual TUint PictureBufferBytes();

   /**
	* Reads various counters related to decoded pictures. See the definition
	* of TPictureCounters for a description of the counters. The counters are
	* reset when Initialize() or this method is called, and thus they only
	* include pictures processed since the last call.

	* @param aCounters	The counter structure to fill
	* @pre				This method can only be called after the hwdevice has
	*                   been initialized with Initialize().
	*/
	virtual void GetPictureCounters(
			                  CMMFDevVideoPlay::TPictureCounters& aCounters );

   /**
	* Sets the computational complexity level to use. If separate complexity
	* levels are not available, the method call is ignored. If the level
	* specified is not available, the results are undefined. Typically the
	* device will either ignore the request or use the nearest suitable level.

	* @param aLevel	The computational complexity level to use. Level zero (0)
	*               is the most complex one, with the highest quality. Higher
	*               level numbers require less processing and may have lower
	*               quality

	* @pre			This method can only be called after the hwdevice has been
	*               initialized with Initialize().
	*/
	virtual void SetComplexityLevel( TUint aLevel );

   /**
	* Gets the number of complexity levels available.

	* @return	The number of complexity control levels available, or zero if
	*           the information is not available yet. The information may not
	*           be available if the number of levels depends on the input
	*           data, and enough input data has not	been read yet. In that
	*           case, using level zero is safe.

	* @pre		This method can only be called after the hwdevice has been
	*           initialized with Initialize().
	*/
	virtual TUint NumComplexityLevels();

   /**
	* Gets information about a computational complexity level. This method can
	* be called after NumComplexityLevels() has returned a non-zero value - at
	* that point the information is guaranteed to be available. Some hardware
	* device implementations may not be able to provide all values, in that
	* case the values will be approximated.

	* @param aLevel		The computational complexity level to query. The level
	*                   numbers range from zero ( the most complex ) to
	*                   NumComplexityLevels()-1.
	* @param aInfo		The information structure to fill

	* @pre				This method can only be called after the hwdevice has
	*                   been initialized with Initialize().
	*/
	virtual void GetComplexityLevelInfo( TUint aLevel,
			                  CMMFDevVideoPlay::TComplexityLevelInfo& aInfo );

   /**
	* Returns a picture back to the device. This method is called by
	* CDevVideoPlay to return pictures from the client ( after they have been
	* written with NewPicture() ), or by the output device when it has
	* finished using a picture.

	* @param aPicture	The picture to return. The device can re-use the
	*                   memory for the picture.

	* @pre				This method can only be called after the hwdevice has
	*                   been initialized with Initialize().
	*/
	virtual void ReturnPicture( TVideoPicture* aPicture );

   /**
	* Gets a copy of the latest picture sent to output

	* @param aPictureData	  Target picture. The memory for the picture
	*                         must be allocated by the caller, and
	*                         initialized properly. The data formats must
	*                         match the snapshot format requested.
	* @param aFormat		  The picture format to use for the snapshot.

	* @return				  ETrue if the snapshot was taken, EFalse if a
	*                         picture is not available. The picture may not
	*                         be available if decoding has not progressed
	*                         far enough yet.

	* @leave				  The method will leave if an error occurs.

	* @pre					  This method can only be called after the
	*                         hwdevice has been initialized with Initialize().
	*/
	virtual TBool GetSnapshotL( TPictureData& aPictureData,
			                    const TUncompressedVideoFormat& aFormat );

   /**
	* Gets a copy of a specified picture.When the snapshot is available, it
	* will be returned to the client using the TimedSnapshotComplete()
	* callback. To cancel a timed snapshot request, use CancelTimedSnapshot().
	* Only one timed snapshot request can be active at a time.

	* @param aPictureData			Target picture. The memory for the picture
	*                               must be allocated by the caller, and
	*                               initialized properly. The data formats
	*                               must match the snapshot format requested.
	*                               The picture must remain valid until
	*								the snapshot has been taken or until the
	*								the request has been cancelled with
	*								the CancelTimedSnapshot().
	* @param aFormat				The picture format to use for the snapshot
	* @param aPresentationTimestamp	Presentation timestamp for the picture to
	* copy. The timestamp
	*								must match the timestamp in the picture
	*								must exactly, so the same clock frequency
	*								must should be used. Used for the first
	*								must method variant

	* @leave						The method will leave if an error occurs.

	* @pre							This method can only be called after the
	*                               hwdevice has been initialized with
	*                               Initialize().
	*/
	virtual void GetTimedSnapshotL( TPictureData* aPictureData,
			        const TUncompressedVideoFormat& aFormat,
					const TTimeIntervalMicroSeconds& aPresentationTimestamp );

   /**
	* Gets a copy of a specified picture.When the snapshot is available, it
	* will be returned to the client using the TimedSnapshotComplete()
	* callback. To cancel a timed snapshot request, use CancelTimedSnapshot().
	* Only one timed snapshot request can be active at a time.

	* @param aPictureData	Target picture. The memory for the picture must be
	*                       allocated by the caller, and initialized properly.
	*                       The data formats must match	the snapshot format
	*                       requested. The picture must remain valid until
							the snapshot has been taken or until the request
							has been cancelled with CancelTimedSnapshot().
	* @param aFormat		The picture format to use for the snapshot.
	* @param aPictureId  	Picture identifier for the picture to copy. Used
	*                       for the second method variant

	* @leave				The method will leave if an error occurs.

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void GetTimedSnapshotL( TPictureData* aPictureData,
			                        const TUncompressedVideoFormat& aFormat,
									const TPictureId& aPictureId );
	/**
	* Cancels a timed snapshot request

	* @pre		This method can only be called after the hwdevice has been
	*           initialized with Initialize().
	*/
	virtual void CancelTimedSnapshot();

   /**
	* Gets a list of the supported snapshot picture formats.

	* @param aFormats		An array for the result format list. The array
	*                       must be created and destroyed by the caller

	* @leave				The method will leave if an error occurs.

	* @pre					This method can only be called after the hwdevice
	*                       has been initialized with Initialize().
	*/
	virtual void GetSupportedSnapshotFormatsL(
			                     RArray<TUncompressedVideoFormat>& aFormats );

   /**
	* Notifies the hardware device that the end of input data has been reached
	* and no more input data will be written. The hardware device can use this
	* signal to ensure that the remaining data gets processed, without waiting
	* for new data. After the remaining data has been processed, the hardware
	* device must notify call the proxy MdvppStreamEnd() callback.This method
	* is mainly useful for file-to-file conversions and other non-realtime
	* processing. For real-time playback all video pictures are processed or
	* discarded according to their timestamps.

	* @pre		This method can only be called after the hwdevice has been
	*           initialized with Initialize().
	*/
	virtual void InputEnd();

public:	// MProcessEngineObserver Function

   /**
    * Callback from Engine to represent buffer has been consumed
    * @param  :  aInp and aError
    * @return :  TInt: KErrNone or KErrCancel
    */
	virtual TInt InputBufferConsumed ( TAny* aInp, TInt aError );

   /**
    * Callback to reprsent that output buffer is ready
    * @param  :  aInp and aError
    * @return :  TInt: KErrNone or KErrCancel
    */
	virtual TInt OutputBufferReady ( TAny* aOup, TInt aError );

   /**
    * CallBack to handle error
    * @return :  None
    */
	virtual void FatalErrorFromProcessEngine ( TInt aError );
    /**
	 * added as a dummy function, to make compatibility with the
	 * encoder for the utility-processengine
	 */
	// Callback to indicate the completion of callback
	void CommandProcessed ( TInt aCmd, TAny* aCmdData, TInt aError );

public : // MMmfVideoBufferManagementObserver Functions

   /**
    * Notifies the observer that one or more new input buffers are available.
	* The client can then use MmvbmGetBufferL() to get a buffer.
    */
	virtual void MmvbmoNewBuffers() ;

   /**
    * Notifies the observer all outstanding input buffers must be
    * released immediately
    */
	virtual void MmvbmoReleaseBuffers();

protected:

   /**
	* Set the proxy implementation to be used. Called just after the object is
	* constructed.

	* @param	aProxy	The proxy to use.
	*/
	virtual void SetProxy( MMMFDevVideoPlayProxy& aProxy );

private:
	/**
     * C++ default constructor.
     */
	CAriH264decHwDevice();

	/**
     *  Symbian 2nd phase constructor .
     */
	void ConstructL();

	/**
	 *Creates o/p buffers based on the o/p formats supported
	 *@param  : None
	 */
	void CreateOutputBuffersL();

	/**
	 * Gives call backs to clinet regarding slice and picture loss
	 * @param  : None
	 */

	void SliceAndPictureLoss();

	/**
	 * Allcotion and reallocation of input buffers
	 * @param  : None
	 */

	void CreateInputBufferL( TUint aBufferSize, TBool aReallocate );

	/**
	 * HandleInputEndInStopping
	 * @param  : None
	 */
	void HandleInputEndInStopping();

private : //Data

	// Reference to Input Free Buffer Queue
	RArray<TVideoInputBuffer*>		iInputFreeBufferQueue;

	// Free Input Buffers
	TVideoInputBuffer*				iInputFreeBuffers;

	// Reference to Output Device
	CMMFVideoPostProcHwDevice*		iOutputDevice;

	// Reference to Proxy Object
	MMMFDevVideoPlayProxy*			iMMFDevVideoPlayProxy;

	// Represents the state of the Decoder Hw Device
	CStateMachine					*iState;

	// Reference to Video Picture Header
	TVideoPictureHeader*			iVideoPictureHeader;

	// Current Decoding Position
	TTimeIntervalMicroSeconds		iDecodingPosition;

	//	Handle to the Compressed video format
	CCompressedVideoFormat*			iInputFormat;

	//	Uncompressed Output format
	TUncompressedVideoFormat		iOutputFormat;

	// Buffer Options Set by the Client
	CMMFDevVideoPlay::TBufferOptions		iBufferOptions;

	// Current Picture Counter value
	CMMFDevVideoPlay::TPictureCounters		iPictureCounters;

	// Current Bit Stream Counter value
	CMMFDevVideoPlay::TBitstreamCounters	iBitstreamCounters;

	// Codec Reference
	CAriH264decWrapper*						iCodec;

	//	Base Process Engine Reference
	CBaseEngine*				iEngine;

	// To Check whether inputend is called or not
	TBool						iInputEndCalled;

	// OutPut  Buffers
	TVideoPicture*				iOutputFreeBuffer;

	// Free OutPut Buffer Queue
	RArray<TVideoPicture*>		iOutputFreeBufferQueue;

	// counter to keep track of the no of buffers added to the engine
	TInt						iFilledBufferCounter;

	// data unit type set by the client
	TVideoDataUnitType			iDataUnitType;

	// Encapulation set by the client
	TVideoDataUnitEncapsulation iEncapsulation;

	// represents the o/p buffer size
	TInt						iOutputBufferSize;

	// flag to whether o/p buffers created or not
	TBool						iOutputBuffersCreated;

	// Picture Number
	TUint						iPictureNumber;

	// represents the number of inputbuffers created so avoids reallocation
	TUint						iNumberOfInputBuffersAllocated;

	// Reference to the custom buffer interface
	MMmfVideoBufferManagement*	iCustomBufferHandle;

	// buffer options for the custom interface
	MMmfVideoBufferManagement::TBufferOptions *iCustomBufferOptions;

	// represents whether the o/p buffers should be added to engine or not
	TBool 			iBufferAdded;

    // flag to indciate that ConfigureDecoderL is called
	TBool			iConfigureDecoderCalled;

	// flag to indciate that decoder has been configured
	TBool			iDecoderConfigured;

	// the width of the source file
	TInt			iWidthSource;

	// the height of the source file
	TInt			iHeightSource;

	// type of the input stream
	TInt			iStreamType;

	// length of the NAL hdr
	TUint iLenOfNalLenFld, iLenOfHdrLen;

	// array which holds the list of supported formats
	RArray<CCompressedVideoFormat*> iSupportedFormats;

	// array which holds the max value of the picture rates
	RArray<TPictureRateAndSize> iMaxPictureRates;

};

#endif //ARIH264DECHWDEVICE_H
