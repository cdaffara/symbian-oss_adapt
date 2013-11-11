/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies). 
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


#ifndef DISCCHANNEL_H
#define DISCCHANNEL_H

//  INCLUDES
#include <kernel.h>
#include <IscMultiplexerBase.h> // TIscFrameInfo

// CONSTANTS
const TInt KMajorVersionNumber = 1;
const TInt KMinorVersionNumber = 0;
const TInt KBuildVersionNumber = 1;

const TUint8 KBehaviormask = 15; //bits 00001111

// MACROS

const TInt KIscEmptyMyBuffersDfcPriority( 4 );
// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION
class DIscDevice;
class DIscQueue;
class IscChannelContainer;

/**
*  Handles all interraction with the user-side
*
*  @lib IscDriver.LIB
*  @since 2.8
*/
class DIscChannel : public DLogicalChannel
    {

    public:  // Constructors and destructor
       
        /**
        * C++ default constructor.
        * @param pointer to owner device
        */
        IMPORT_C DIscChannel( DLogicalDevice* aDevice );
       
        /**
        * Destructor.
        */
        ~DIscChannel();

    public: // Functions from base classes
        
        /**
        * Secondary initialization of channel.
        * @since 2.8
        * @param  TInt aUnit Channel number
        * @param const TDesC8* anInfo pointer to a descriptor containing extra 
        *                             information for the device. 
        * @param const TVersion& aVer reference containing the version being 
        *                             requested by the thread which is opening this channel 
        * @return virtual TInt KErrNone if succesful. KErrAlreadyExist if channel already open
        */
        virtual TInt DoCreate( TInt aUnit, const TDesC8* anInfo, const TVersion& aVer );

        /**
        * Message handling ( kernel server context ).
        * @since 2.8
        * @param aMsg Thread message to get parameters from user side
        * @return void 
        */
        virtual void HandleMsg( TMessageBase* aMsg );

        /**
        * Message handling ( user thread context ).
        * @since 2.8
        * @param aReqNo request number called from IscApi
        * @param a1 pointer array to get parameters from user side
        * @param a2 not used in ISC
        * @return TInt KErrNone if succesful
        */
        TInt Request( TInt aReqNo, TAny* a1, TAny* a2 );

    public: // New functions
        /**
        * Function to complete clients pending asynchronous request
        * @since 2.8
        * @param  TInt aOperation asynchronous operation to complete
        * @param TInt aCompleteStatus status of request to be completed
        * @return void 
        */
        void CompleteRequest( TInt aOperation, TInt aCompleteStatus );

        /**
        * Stores the incoming frame to channels receive queue
        * @since 2.8
        * @param TDesC8* aData pointer to storable frame
        * @return void 
        */
        void StoreFrame( TDesC8* aData );

        /**
        * Goes through channel's queue and delivers possible frame to client
        * @since 2.8
        * @return void 
        */
        void EmptyBuffers();
        
        /**
        * Notify user side client that uplink flow control is on/off
        * @since 2.8
        * @param const TInt aFlowControlStatus EIscFlowControlOn / EIscFlowControlOff
        * @return void 
        */
        void NotifyFlowControl( const TInt aFlowControlStatus );

        /**
        * Notify user side client that status of connection between Symbian side and the Domestic OS has changed
        * @since 2.8
        * @param const TInt aConnectionStatus  EIscConnectionOk / EIscConnectionNotOk
        * @return void 
        */
        void NotifyConnectionStatus( const TInt aConnectionStatus );
    
        /**
        * Check if asynchronous request is active
        * @since 2.8
        * @param const TUint16 aReqNumber Number of request
        * @return TInt KRequestPending if request active. Otherwise KErrNone
        */
        TInt IsPending( const TUint16 aReqNumber );

        /**
        * Copy data from user buffer to kernel side buffer by multiplexer
        * @since 2.8
        * @param aUserBuffer pointer to source descriptor
        * @param aKernelBuffer pointer to destination descriptor
        * @param aOffset Offset of the source buffer
        * @return KErrNone if successful
        */
        TInt CopyFromUserBuffer( const TDesC8& aUserBuffer,
                                 TDes8& aKernelBuffer,
                                 TInt aOffset );

        /**
        * Writes data/frames to clients buffer
        * @since 2.8
        * @param aDest pointer to destination descriptor
        * @param aSrc pointer to source descriptor
        * @param aOffset Offset on where to start writing to target descriptor
        * @return TInt 
        */
        TInt ThreadWrite( TAny* aDest, const TDesC8* aSrc, TInt aOffset );

        /**
        * Delete send frames / receive queue for channel in question
        * @since 2.8
        * @return void 
        */
        void ResetBuffers();

        /**
        * Returns the channel number.
        * @since 3.0
        * @return TUint16, the number of the channel.
        */
        inline TUint16 GetChannelNumber(){return iChannelNumber;};

        /**
        * Returns a reference to user side thread of this channel.
        * @since 3.0
        * @param None
        * @return DThread*, user side thread ownership is not given.
        */
        inline DThread* GetDThread()
        	{
        	return iThread;	
        	};

    protected: 
      
		// None

    private:

        /**
        * Call from user side comes to this function
        * @since 2.8
        * @param  TInt aFunction request number
        * @param TAny* a1 pointer to array including info about request
        * @param TAny* a2 not used
        * @return TInt KErrNone if succesful
        */
        TInt HandleRequest( TInt aFunction, TAny* a1, TAny* a2 );
        
        /**
        * Handles channel destruction.
        * @since 2.8
        * @param void
        * @return void 
        */        
        void ChannelDestruction();
        
        /**
        * Handles asynchronous client requests
        * @since 2.8
        * @param  TInt aFunction request number
        * @param TAny* a1 pointer to array including info about request
        * @return void 
        */
        void HandleAsyncRequest( TInt aFunction, TAny* a1 );

        /**
        * Handles synchronous client requests
        * @since 2.8
        * @param  TInt aFunction
        * @param TAny* a1 pointer to array including info about request
        * @return TInt 
        */
        TInt HandleSyncRequest( TInt aFunction, TAny* a1 );

        /**
        * Cancels active request
        * @since 2.8
        * @param  TInt aFunction request number
        * @param TAny* a1 not used
        * @return TInt 
        */
        TInt HandleCancelRequest( TInt aFunction, TAny* a1 );
        
        /**
        * Copies several data frames to clients buffer if needed before compliting receive request
        * @since 2.8
        * @param  TDes8* aPtr pointer to frame
        * @param TIscFrameInfo& aInfo info needed in frame combining
        * @return void 
        */
        void HandleConcatenatedDataFrame( TDes8* aPtr, TIscFrameInfo& aInfo );

        /**
        * Copies several frames to clients buffer if needed before compliting receive request
        * @since 2.8
        * @param  TDes8* aPtr pointer to frame
        * @param TIscFrameInfo& aInfo info needed in frame combining
        * @return void 
        */
        void HandleConcatenatedFrame( TDes8* aPtr, TIscFrameInfo& aInfo );

        /**
        * Initialization complete dfc
        * @since 2.8
        * @param TAny* aPtr pointer to channel
        * @return static void.
        */
        static void InitializeComplete( TAny* aPtr );
        
        /**
        * Completes the multiplexer initialization
        * @since 2.8
        * @return void 
        */
        void DoMultiplexerInitialize();

    private:    // Data

        TDfc*       iInitializeDfc;
        TPtr8*      iDataTransmissionIniData;
        TPtr8*      iMultiplexerIniData;
        TUint8*     iMultiplexerBuffer;
        TUint8*     iDataTransmissionBuffer;
        DIscDevice* iIscDevice;
        TPtr8*      iIscConnectionStatusPtr;
        TPtr8*      iIscFlowControlStatusPtr;
        //Store of Isc requests
        TRequestStatus* iIscRequests[ EIscAsyncLast ];
        // Pointers to client message buffers
        TPtr8*      iReceiveBufPtr;
        TPtr8*      iDataReceiveBufPtr;
        TPtr8*      iNeededBufLen;
        TPtr8*      iNeededDataBufLen;
        TUint16     iChannelNumber;
        TBool       iChannelOpen;
        // Queue for incoming frames
        TUint32**   iFrameRx;
        DIscQueue*  iFrameRxQueue;

        // Queue for incoming data frames
        TUint32**   iDataFrameRx;
        DIscQueue*  iDataFrameRxQueue;
          
        TInt        iULFlowControlStatus;
        TInt        iDLFlowControlStatus;
        // Stores the last notified uplink flowcontrol status.
        TInt        iLastNotifiedULFlowstatus;

        TUint16     iIscChannelHighWaterMark;
        TUint16     iIscChannelLowWaterMark;
        TBool       iOverFlow;
        TBool       iClientPanic;
        TInt        iDataTransmissionErrorCode;

        DThread*    iThread;

    };

#endif      // DISCCHANNEL_H   
            
// End of File
