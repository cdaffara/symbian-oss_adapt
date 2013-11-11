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


#ifndef ISCDEVICE_H
#define ISCDEVICE_H

//  INCLUDES
#include <kernel.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class DIscDataTransmissionBase;
class DIscMultiplexerBase;
class DIscChannel;
class DIscMainRcvBuffer;
class DIscQueue;
class DIscSendQueue;
class TIscSendFrameInfo;

// CLASS DECLARATION

/**
*  Isc driver ldd factory object
*
*  @lib IscDriver.LIB
*  @since 2.8
*/
class DIscDevice : public DLogicalDevice
    {
    public:  // Constructors and destructor
         /**
        * C++ default constructor.
        */
        IMPORT_C DIscDevice();
       
        /**
        * Destructor.
        */
        IMPORT_C virtual ~DIscDevice();

        
    public: // Functions from base classes

        /**
        * From DLogicalDevice. 
        * Create a logical channel
        * @since 2.8
        * @param DLogicalChannelBase*& aChannel logical channel to be set
        * @return TInt KErrNone if succesful, otherwise KErrNoMemory
        */
        virtual TInt Create( DLogicalChannelBase*& aChannel );

        /**
        * Complete the installation of driver
        * @since 2.8
        * @return TInt KErrNone if successful 
        */
        virtual TInt Install();

         /**
        * From DLogicalDevice.
        * Get capabilities of this logical device
        * @since 2.8
        * @param TDes8& aDes a reference which contains return  
        * information describing the capabilities of this device.
        * @return void
        */
        IMPORT_C virtual void GetCaps( TDes8& aDes ) const;

    public: // New functions
    
        /**
        * Complete the initialization of driver
        * @since 2.8
        * @return void 
        */
        void Initialize();
       
        /**
        * Get message block from buffers allocated in IscDataTransmissionBase.dll
        * @since 2.8
        * @param TDes8*& aPtr pointer where message block is set
        * @param TUint16 aSize size of needed memory block
        * @return void 
        */
        IMPORT_C void ReserveMemoryBlock( TDes8*& aPtr, TUint16 aSize );

        /**
        * Release memory block allocated with ReserveMemoryBlock
        * @since 2.8
        * @param TDes8* aData
        * @return void 
        */
        IMPORT_C void ReleaseMemoryBlock( TDes8* aData );

        /**
        * Connection status change function. Can be called from any context.
        * @since 2.8
        * @param const TInt aStatus status of driver
        * @return void 
        */
        EXPORT_C void NotifyConnectionStatus( const TInt aStatus );

        /**
        * Frames coming from Domestic OS
        * @since 2.8
        * @param TDesC8* aData pointer to data
        * @return void 
        */
        IMPORT_C void Receive( TDesC8* aData ) const;

        /**
        * Queue frames that will be sent to Domestic OS
        * @since 2.8
        * @param aChannelId number of sender channel
        * @param const TDesC8* aFrame pointer to send frame
        * @param aChannelPtr DIscChannel pointer
        * @param aMultiplexerDfc dfc to run by data transmission driver after frame has delivered
        * @param aMultiplexerErrorCode status how frame transmission succeeded
        * @return TInt KErrNone if frame added to send queue succesfully
        */
        IMPORT_C TInt QueueFrame( const TUint16 aChannelId, 
                                  const TDesC8* aFrame,
                                  const TAny* aChannelPtr,
                                  TAny* aFrameInfo );

        /**
        * Cancels sending of frames to Domestic OS of certain channel and deletes queued frames
        * @since 2.8
        * @param aChannelId channel number to which canceling is done
        * @param aChannelPtr DIscChannel pointer
        * @return TInt KErrNone when canceling is done
        */
        IMPORT_C TInt CancelSending( const TUint16 aChannelId,
                                     const TAny* aChannelPtr );

        /**
        * Adds Dfc to empty queues. Can be called from any context.
        * @since 2.8
        * @return void 
        */
        IMPORT_C void FlushQueues();

        /**
        * Function to complete user side asynchronous request. Uses DIscChannel pointer
        * @since 2.8
        * @param aOperation operation to complete
        * @param aCompleteStatus status of operation
        * @param aChannelPtr pointer to a DIscChannel
        * @return void 
        */
        IMPORT_C void CompleteRequest( TUint16 aOperation, 
                                       TInt aCompleteStatus, 
                                       const TAny* aChannelPtr );
        
        /**
        * Function to notify client about uplink flow control status
        * @since 2.8
        * @param  aULFlowStatus EIscFlowControlOn / EIscFlowControlOff
        * @param aChannelId Channel to be affected by the flow control
        * @param aChannelPtr pointer to DIscChannel
        * @return void 
        */
        IMPORT_C void ULFlowControl( const TInt aULFlowStatus, 
                                     const TUint16 aChannelId,
                                     const TAny* aChannelPtr );

        /**
        * Function to check if asycnhronous request is active
        * @since 2.8
        * @param  const TUint16 aReqNumber request number
        * @param const TUint16 aChannelId channel number
        * @return TInt KRequestPending if request active, otherwise KErrNone
        */
        IMPORT_C TInt IsPending( const TUint16 aReqNumber, 
                                 const TAny* aChannelPtr );

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
                                          const TInt aOffset );

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
                                        const TInt aOffset );
                                        
        /**
        * Returns a user side thread of a given channel. 
        * @since 3.0
        * @param const TAny* aChannelPtr, channel pointer (DIscChannel).
        * @return DThread*, user side clients thread. Ownership is not given.
        */
        IMPORT_C DThread* GetThreadPtr( const TAny* aChannelPtr );                                        
          
        /**
        * Notify multiplexer about down link flow control
        * @since 2.8
        * @param  const TInt aDLFlowStatus EIscFlowControlOn / EIscFlowControlOff
        * @param const TUint16 aChannel channel number
        * @return void 
        */
        void DLFlowControlNotify( const TInt aDLFlowStatus, 
                                  const TUint16 aChannel,
                                  const TAny* aChannelPtr );

        /**
        * Function to tell current status of connection to Domestic OS
        * @since 2.8
        * @return TInt EIscConnectionOk / EIscConnectionNotOk
        */
        TInt ConnectionStatus();

    protected:

        /**
        * Function to disable interrupts
        * @since 2.8
        * @return TInt KErrNone if succesful
        */
        static TInt DisableIrqs();

        /**
        * Function to restore interrupts
        * @since 2.8
        * @param TInt aLevel level where interrupts should be restored
        * @return void 
        */
        static void RestoreIrqs( TInt aLevel );


    private:

        /**
        * Function to connect to DataTransmission and Multiplexer ldds
        * @since 2.8
        * @return TInt KErrNone if succesful
        */
        TInt InitializeLdd2LddInterface();

        /**
        * Dfc to empty control channel and other send queues
        * @since 2.8
        * @param TAny* aPtr pointer to DIscDevice
        * @return static void
        */
        static void Flush( TAny* aPtr );

        /**
        * Notifies connection changes.
        * @since 2.8
        * @param TAny* aPtr
        * @return static void
        */
        static void NotifyConnection( TAny* aPtr );


    public:     // Data
    
        static DIscMultiplexerBase* iIscMultiplexerInterface;
        static DIscDataTransmissionBase* iIscDataTransmissionInterface;
    

    private:    // Data
    
        TUint32**               iSend;
        TUint32**               iTempSend;
        TUint32**               iControlSend;        
        
        TIscSendFrameInfo**     iSendFrameParameters;
        TIscSendFrameInfo**     iTempFrameParameters;
        TIscSendFrameInfo**     iControlFrameParameters;
        
        static DIscSendQueue*   iSendQueue;
        static DIscSendQueue*   iTempQueue;
        static DIscSendQueue*   iControlSendQueue;

        static TDfc*            iSendDfc;
        static TDfc*            iNotifyDfc;
        
        DIscMainRcvBuffer*      iIscMainRcvBuffer;

        static TInt             iConnectionStatus;

    };

#endif      // ISCDEVICE_H
            
// End of File
