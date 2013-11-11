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
* Description:  A reference implementation for ISC data transmission base.
*
*/



#ifndef ISCDATATRANSMISSIONBASE_H
#define ISCDATATRANSMISSIONBASE_H

//  INCLUDES
#include <iscmultiplexerbase.h>
#include <kernel.h>

// CONSTANTS
_LIT(KIscDataTransmissionDriverName,"IscDataTransmissionDriver");

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class DIscDevice;
class DIscBufferAllocator;

// CLASS DECLARATION

/**
*  DIscDataTransmissionBase defines the ISC Data Transmission Plug-In API
*
*  @lib IscDataTransmissionBase.LIB
*  @since 2.8
*/
class DIscDataTransmissionBase : public DLogicalDevice
    {

    public:
        /**
        * C++ default constructor.
        */
        IMPORT_C DIscDataTransmissionBase();
       
        /**
        * Destructor.
        */
        IMPORT_C virtual ~DIscDataTransmissionBase();
    
    public:  // Functions from base class
        
        /**
        * Implementation of pure-virtual Install from DLogicalDevice. This 
        * function calls Install2ndPhase.
		* @since 2.8
        * @return TInt KErrNone
        */
        IMPORT_C TInt Install();

        /**
        * This method makes the buffer allocations according to the 
        * configuration specified by the ISC Multiplexer
		* @since 2.8
        * @param TIscBufferEntry* aBufferConfig pointer to buffer configuration table
        * @return void 
        */
        IMPORT_C virtual void AllocBuffers( TIscBufferEntry* aBufferConfig );

    public: // New functions
     
        /**
        * Second phase initialization, called during ISC Multiplexer loading. 
        * ISC Data Transmission Driver must do all initialization here that
        * are normally done in Install()
		* @since ?Series60_version
        * @return TInt KErrNone if successful
        */
        virtual TInt Install2ndPhase() = 0;

        /**
        * ISC Data Transmission driver initialization
		* @since 2.8
        * @param aParameters Initialization parameters passed from 
        * ISC initialization file
        * @param aDfc Dfc that must be runned after the initialization 
        * has been done
        * @return void 
        */
        virtual void InitializeDataTransmission( TDesC8& aParameters, 
                                                 TDfc* aDfc,
                                                 TInt& aErrorCode /* KErrNone */) = 0;
         
        /**
        * In asynchronous data transmission checkin if transmission is not yet finished
		* Access to this method is necessary since transmission can be asynchronous.
		* @since 2.8
        * @return TBool EFalse / ETrue
        */
        virtual TBool IsWritePending() { return EFalse; };

        /**
        * Send frame to Domestic OS
		* @since 2.8
        * @param aFrame Frame to be sent
        * @param aDfc Dfc function that must be runned after the frame is sent. 
        * Or the dfc can be runned immediately if the driver internally buffers
        * the outgoing frames
        * @param aMultiplexerDfc Dfc function in multiplexer to handle send complition
        * @param aMultiplexerErrorCode error code for frame sending to modem
        * @return void 
        */
        virtual void SendFrame( TDes8& aFrame, TDfc* aDfc, TAny* aFrameInfo ) = 0;
        
        /**
        * Get the connection status
		* @since 2.8
        * @return TInt EIscConnectionOk / EIscConnectionNotOk
        */
        virtual TInt ConnectionStatus() = 0;

        /**
        * Connects ISC Driver to ISC Data Transmission Driver
		* @since 2.8
        * @param aDevice Pointer to DIscDevice class in ISC Driver
        * @return void 
        */
        IMPORT_C virtual void Connect(DIscDevice* aDevice);

        /**
        * Reserve memory block 
		* @since 2.8
        * @param aPtr Pointer to reserved memory block, NULL if not successful
        * @param aSize Required minimun size of the block
        * @return void 
        */
        IMPORT_C virtual void ReserveMemoryBlock( TDes8*& aPtr, TUint16 aSize);

        /**
        * Release the reserved memory block
		* @since 2.8
        * @param aPtr Pointer to memory block to be released
        * @return void 
        */
        IMPORT_C virtual void ReleaseMemoryBlock( TDes8* aPtr );

    protected:  // New functions

        /**
        * Transfer incoming frame to ISC Driver
		* @since 2.8
        * @param aFrame Frame that was received
        * @return void 
        */
        IMPORT_C void ReceiveFrame(TDesC8& aFrame);
        
        /**
        * Notifies any connection status changes to ISC Driver
		* @since 2.8
        * @param aStatus EIscConnectionOk / EIscConnectionNotOk
        * @return void 
        */
        IMPORT_C void NotifyConnectionStatus( TInt aStatus );

    private:    // Data
        
        // pointer to DIscDevice class from ISC Driver
        DIscDevice* iDevice; // No ownership
        
        // pointer to pre-allocated buffers
        DIscBufferAllocator* iIscBufferAllocator; // Ownership

    };

#endif      // ISCDATATRANSMISSIONBASE_H   
            
// End of File
