/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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


#ifndef ISCMULTIPLEXERBASE_H
#define ISCMULTIPLEXERBASE_H

//  INCLUDES
#include <kernel.h>
#include <IscDefinitions.h>

// CONSTANTS
_LIT( KIscMultiplexerName,"IscMultiplexer" );

const TUint16 KIscBufferAmount = 10;
// MACROS

// DATA TYPES

typedef struct
{
   TIscFrameType frameType;
   TIscFrameConcatenation concatenation;
   TUint16 totalLength;
   TUint16 writeStartIndex;
   TUint16 writeLength;
} TIscFrameInfo;

typedef struct
{
   TUint16 size;
   TUint16 count;
} TIscBufferEntry;

typedef struct
{
   TUint16 mainRcvQueueSize;
   TUint16 channelRcvQueueSize;
   TUint16 channelDataRcvQueueSize;
   TUint16 channelSendQueueSize;
   TIscBufferEntry bufferConfig[KIscBufferAmount];
} TIscConfiguration;

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class DIscDevice;

// CLASS DECLARATION

/**
*  DIscMultiplexerBase defines the ISC Multiplexer Plug-In API.
*  This class defines the base class for Multiplexer( s ) used in 
*  conjunction with ISC Driver
*
*  @lib IscMultiplexerBase.LIB
*  @since 2.8
*/
class DIscMultiplexerBase : public DLogicalDevice
    {
    public:  // Constructors and destructor

        /**
        * C++ default constructor.
        */        
        IMPORT_C DIscMultiplexerBase();
        
        /**
        * Destructor
        */
        IMPORT_C virtual ~DIscMultiplexerBase();

    public: // Functions from base class
        
        /**
        * Implementation of pure-virtual Install from DLogicalDevice. This 
        * function calls Install2ndPhase.
        * @since 2.8
        * @return TInt KErrNone
        */
        IMPORT_C TInt Install();

    public: // New functions
        /**
        * Second phase initialization, called during ISC Multiplexer loading
        * ISC Multiplexer must do all initialization here that are normally 
        * done in Install()
        * @since 2.8
        * @return TInt KErrNone if successful
        */
        virtual TInt Install2ndPhase() = 0;
        
        /**
        * Multiplexers "pre" intialization before Data transmission driver 
        * initialization has been done ( e.g. if buffer configurations are given from isc_config.ini )
        * @since 2.8
        * @param aParameters Additional parameters passed to multiplexer
        * @return void 
        */
        IMPORT_C virtual void SetInitializationParameters( const TDesC8& aParameters );

        /**
        * Multiplexer intialization
        * @since 2.8
        * @param aOperation The asynchronous request to complete after
        * multiplexer intialization is complete
        * @param aParameters Additional parameters passed to multiplexer
        * @param aChannelPtr DIscChannel pointer
        * @return void 
        */
        virtual void InitializeMultiplexer( const TUint16 aOperation, 
                                            const TDesC8& aParameters,
                                            const TAny* aChannelPtr ) = 0;

        /**
        * Open data link connection with the modem
        * @since 2.8
        * @param aDLCI The channel id to open
        * @param aOpenParams Additional open parameters given by the ISC user,
        * NULL if omitted
        * @param aChannelPtr DIscChannel pointer
        * @return void 
        */
        virtual void OpenDLC( const TUint16 aDLCI, 
                              const TDesC8* aOpenParams,
                              const TAny* aChannelPtr ) = 0;

        /**
        * Close data link connection
        * @since 2.8
        * @param aDLCI The channel id to close
        * @param aChannelPtr DIscChannel pointer
        * @return void 
        */
        virtual void CloseDLC( const TUint16 aDLCI,
                               const TAny* aChannelPtr ) = 0;

        /**
        * Send data to Domestic OS
        * @since 2.8
        * @param aOperation Used to distinguish whether it is a synchronous 
        * or asynchronous Send request ( EIscSyncSend / EIscAsyncSend )
        * @param aSenderChannel Sender channel id
        * @param aData Data to be sent
        * @param aChannelPtr DIscChannel pointer
        * @return KErrNone if successful 
        */
        virtual TInt Send( const TUint16 aOperation,
                           const TUint16 aSenderChannel, 
                           const TDesC8& aData,
                           const TAny* aChannelPtr ) = 0;
        /**
        * Send data to Domestic OS
        * @since 2.8
        * @param aOperation Used to distinguish whether it is a synchronous 
        * or asynchronous Send request ( EIscSyncDataSend / EIscAsyncDataSend )
        * @param aSenderChannel Sender channel id
        * @param aData Data to be sent
        * @param aChannelPtr DIscChannel pointer
        * @return KErrNone if successful 
        */
        virtual TInt DataSend( const TUint16 aOperation,
                               const TUint16 aSenderChannel,
                               const TDesC8& aData,
                               const TAny* aChannelPtr ) = 0;

        
        /**
        * Retrieve information on the incoming frame
        * @since 2.8
        * @param aFrame Frame which info to retrieve
        * @param aFrameInfo Structure that the multiplexer must fill
        * @return void 
        */
        virtual void GetFrameInfo( const TDesC8& aFrame, 
                                   TIscFrameInfo& aFrameInfo ) const = 0;

        /**
        * Retrieve the channel this frame belongs to
        * @since 2.8
        * @param aFrame Frame to check
        * @param aRcvChannelId Receiver channel id has to written into this
        * variable. Value should be between 0 and 63. KIscFrameReceiverNotFound
        * should be returned if multiplexer could not determine correct receiver 
        * or multiplexer just wants to delete the frame. For multiplexer's purposes 
        * channel ids higher than 63 are also possible
        * @param aRcvChannelPtr Receiver channel pointer. If routing to control channel
        * NULL should be returned an 0 to aRcvChannelId
        * @return void 
        */
        virtual void GetRcvChannelId( const TDesC8& aFrame, 
                                      TUint16& aRcvChannelId,
                                      TAny*& aRcvChannelPtr ) = 0;

        /**
        * Set flow control to specified state
        * @since 2.8
        * @param aFlowStatus EIscFlowControlOn / EIscFlowControlOff
        * @param aChannelId Channel who requested to DL flow control change
        * @param aChannelPtr DIscChannel pointer for corresponding channel
        * @return void 
        */
        virtual void DLFlowControl( const TInt aFlowStatus,
                                    const TUint16 aChannelId,
                                    const TAny* aChannelPtr ) = 0;

        /**
        * Handle frames going to control channel ( channel zero )
        * @since 2.8
        * @param aFrame Control frame to be handled
        * @return void 
        */
        virtual void HandleControlFrame( const TDesC8& aFrame ) = 0;
        
        /**
        * Notify multiplexer that some asynchronous operation has been
        * cancelled
        * @since 2.8
        * @param aChannelId Channel id which has cancelled the operation
        * @param aOperation The operation that has been cancelled
        * @param aChannelPtr Channel pointer which has cancelled the operation
        * @return void 
        */
        virtual void CancelNotify( const TUint16 aChannelId,
                                   const TUint16 aOperation,
                                   const TAny* aChannelPtr ) = 0;

        /**
        * API extension function
        * @since 2.8
        * @param aChannelId Channel id that is executing the operation
        * @param aOperation Which custom operation to execute
        * @param aParameters Additional parameters, NULL if omitted
        * @param aChannelPtr Pointer to DIscChannel
        * @return TInt KErrNone if successful. KErrNotSupported if the
        * operation that was requested is not supported by the multiplexer
        */
        IMPORT_C virtual TInt CustomFunction( const TUint16 aChannelId,
                                              const TUint16 aOperation, 
                                              TAny* aParameters,
                                              const TAny* aChannelPtr );

        /**
        * Get information on specified channel
        * @since 2.8
        * @param aChannelNumber The channel, which information is required
        * @param aInfo Buffer in client address space where to write the data
        * @return TInt KErrNone if successful. KErrNotSupported if not supported 
        * by the multiplexer
        */
        IMPORT_C virtual TInt GetChannelInfo( const TUint16 aChannelNumber,
                                              TDes8& aInfo );

        /**
        * Get maximun data size ( for sending and receiving )
        * @since 2.8
        * @param aChannelId Channel which needed maximum data size is queried
        * @return TInt Maximun data size in bytes 
        */
        virtual TInt MaximumDataSize( const TUint16 aChannelId ) = 0; 

        /**
        * Inform multiplexer on connection status changes
        * @since 2.8
        * @param aConnectionStatus EIscConnectionOk / EIscConnectionNotOk
        * @return void 
        */
        virtual void NotifyConnectionStatus( const TInt aConnectionStatus ) = 0;

        /**
        * Connect ISC Driver to ISC Multiplexer
        * @since 2.8
        * @param aDevice Pointer to DIscDevice class of ISC Driver
        * @return void 
        */
        IMPORT_C virtual void Connect( DIscDevice* aDevice );

        /**
        * Get buffer configurations from ISC Multiplexer
        * @since 2.8
        * @param aConfig Configuration data structure that the multiplexer must fill
        * @return  void 
        */
        virtual void GetConfiguration( TIscConfiguration& aConfig ) = 0;


    protected:  // New functions
        
        /**
        * Complete specified asynchronous request
        * @since 2.8
        * @param aReqNumber Request to complete ( possible values in 
        * IscDefinitions.h )
        * @param aStatus Error code, KErrNone if the operation in question
        * was successful
        * @param aChannelPtr The channel, which request to complete
        * @return void 
        */
        IMPORT_C void CompleteRequest( TUint16 aReqNumber, 
                                       TInt aStatus, 
                                       const TAny* aChannelPtr );
     
        /**
        * Check if specified request is pending in ISC Driver
        * @since 2.8
        * @param aReqNumber Request to check if pending
        * @param aChannelId The channel, which to check for the request
        * @return TInt KRequestPending if the request is pending. KErrNone if 
        * request is not pending
        */
        IMPORT_C TInt IsPending( const TUint16 aReqNumber, 
                                 const TAny* aChannelId );

        /**
        * Queue frame for sending
        * @since 2.8
        * @param aChannelId On which channels queue to add the frame. Value
        * must be between 0 and 63. 0( zero ) refers to the control channel.
        * @param aFrame Frame to send
        * @param aMultiplexerDfc Dfc to run when frame transmission to modem has been done
        * @param aMultiplexerErrorCode error if data transmsission was succesful or not
        * @return TInt KErrNone if successful. KErrNotFound if the channel 
        * in question was not open 
        */        
        IMPORT_C TInt QueueFrame( const TUint16 aChannelId,
                                  const TDesC8* aFrame,
                                  const TAny* aChannelPtr,
                                  TAny* aFrameInfo = NULL );

        /**
        * Start sending the frames
        * @since 2.8
        * @return void 
        */
        IMPORT_C void FlushQueues();

        /**
        * Notify flow control changes to ISC Driver
        * @since 2.8
        * @param aULFlowStatus Uplink flow control status, 
        * EIscFlowControlOff/EIscFlowControlOn
        * @param aChannelId Channel to be affected by the flow control
        * @param aChannel Channel to be affected by the flow control, NULL if wanted to have 
        * effect on all channels within this id. By default it affects all channels
        * @return void 
        */
        IMPORT_C void ULFlowControl( const TInt aULFlowStatus,
                                     const TUint16 aChannelId,
                                     TAny* aChannelPtr = NULL );

        /**
        * Reserve memory block
        * @since 2.8
        * @param aPtr Pointer to reserved memory block, NULL if not successful
        * @param aSize Required minimun size of the block
        * @return void 
        */
        IMPORT_C void ReserveMemoryBlock( TDes8*& aPtr, 
                                          const TUint16 aSize );

        /**
        * Release memory block allocated with ReserveMemoryBlock
        * @since 2.8
        * @param aPtr Pointer to memory block to be released
        * @return void 
        */
        IMPORT_C void ReleaseMemoryBlock( TDes8* aPtr );

        
        /**
        * Copy data from user buffer to kernel side buffer by multiplexer.
        * This method will be deprecated in the future.
        * It is preffered to use GetThreadPtr instead.          
        * @since 2.8
        * @param aUserBuffer pointer to source descriptor
        * @param aKernelBuffer pointer to destination descriptor
        * @param aChannelPtr DIscChannel pointer
        * @param aOffset Offset of the source buffer
        * @return KErrNone if successful
        */
        IMPORT_C TInt CopyFromUserBuffer( const TDesC8& aUserBuffer,
                                          TDes8& aKernelBuffer,
                                          const TAny* aChannelPtr,
                                          const TInt aOffset = 0 );

        /**
        * Copy data to user side buffer from kernel side by multiplexer.
        * This method will be deprecated in the future.
        * It is preffered to use GetThreadPtr instead.        
        * @since 2.8
        * @param aUserBuffer pointer to destination descriptor
        * @param aKernelBuffer pointer to source descriptor
        * @param aChannelPtr DIscChannel pointer
        * @param aOffset Offset of the destination buffer
        * @return KErrNone if succesful
        */
        IMPORT_C TInt CopyToUserBuffer( TAny* aUserBuffer, 
                                        const TDesC8& aKernelBuffer, 
                                        const TAny* aChannelPtr,
                                        const TInt aOffset = 0 );


        /**
        * Description_here
        * @since 2.8
        * @param aChannelId Channel whose queued send frames should be deleted
        * @param aChannelPtr Corresponding DIscChannel pointer
        * @return TInt KErrNone if succesful
        */
        IMPORT_C TInt CancelSending( const TUint16 aChannelId, const TAny* aChannelPtr );


        /**
        * Returns a user side thread of a given channel. Returns NULL if channelptr
        * is not valid or if the channel is not found.
        * @since 3.0
        * @param const TAny* aChannelPtr, channel pointer (DIscChannel).
        * @return DThread&, user side clients thread. Ownership is not given.
        */
        IMPORT_C DThread* GetThreadPtr( const TAny* aChannelPtr );
    
    
    protected:  // Data
        
        // pointer to DIscDevice class from ISC Driver
        DIscDevice* iDevice; // No ownership

    };

#endif      // ISCMULTIPLEXERBASE_H   
            
// End of File
