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




#ifndef ISCCHANNELCONTAINER_H
#define ISCCHANNELCONTAINER_H

//  INCLUDES
#include <kernel.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class DIscChannel;

// CLASS DECLARATION

/**
*  Static class to hold channel objects
*
*  @lib IscDriver.LIB
*  @since 2.8
*/
class IscChannelContainer
    {
    public:  // Constructors and destructor
  

    public: // New functions
        
        /**
        * Set pointer to channel
        * @since 2.8
        * @param IscChannel* aChannel pointer to channel
        * @param const TUint16 aUnit channelnumber
        * @return void
        */
        IMPORT_C static TInt SetChannel( DIscChannel* aChannel,
        								 const TUint16 aUnit );

        /**
        * Return pointer to channel. TUint16 aChannelSharingIndex is needed
        * for channel sharing
        * @since 2.8
        * @param const TUint16 aChannel channel number
        * @param const TUint16 aChannelSharingIndex, index in channel table. 
        * @return static DIscChannel* pointer to channel instance
        */
        static DIscChannel* Channel( const TUint16 aChannel,
        							 const TUint16 aChannelSharingIndex );

        /**
        * Remove channel pointer from channel table
        * @since 2.8
        * @param DIscChannel* aChannel pointer to DIscChannel
        * @return static TInt 
        */
        static TInt RemoveChannel( DIscChannel* aChannel );
    
        /**
        * Goes through channels and open ones are emptied
        * @since 2.8
        * @param TAny* not used
        * @return static void 
        */
        static void ChannelComplition( TAny* /*a*/ );

        /**
        * Allocates Dfc function dynamically and formats data tables.
        * @since 2.8
        * @return static void 
        */
        static void Initialize();

        
        /**
        * Deallocates Dfc function
        * @since 2.8
        * @return static void 
        */
        static void DeActivate();

        /**
        * Adds ChannelComplition Dfc.
        * Can be called from any context.
        * @since 2.8
        * @return static void 
        */
        static void AddDfc();

        /**
        * Make sure that pointer is valid DIscChannel pointer
        * @since 2.8
        * @param aChannelPtr DIscChannel pointer
        * @return void 
        */
        static TInt ValidateChannel( DIscChannel* aChannelPtr );

    private:    // Data
    
        static DIscChannel* iIscChannels[KIscNumberOfUnits][KIscMaxNumberOfChannelSharers];         
    
        static TDfc* iDfc;
      
    };

#endif      // ISCCHANNELCONTAINER_H   
            
// End of File
