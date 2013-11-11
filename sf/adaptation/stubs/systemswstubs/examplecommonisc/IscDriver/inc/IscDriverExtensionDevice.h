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
* Description:  Reference implementation for ISC Driver extension
*
*/




#ifndef ISCDRIVER_EXTENSION_DEVICE_H
#define ISCDRIVER_EXTENSION_DEVICE_H

//  INCLUDES
#include <kernel.h>
#include <IscDevice.h> // Base class

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Isc driver extension ldd factory object
*
*  @lib IscDriver.LIB
*  @since 2.8
*/
class DIscDriverExtensionDevice : public DIscDevice
    {
    public:  // Constructors and destructor
        
        /**
        * C++ default constructor.
        */
        DIscDriverExtensionDevice();
       
        /**
        * Destructor.
        */
        virtual ~DIscDriverExtensionDevice();

    public: // Functions from base classes

        /**
        * Create a logical channel
		* @since 2.8
        * @param DLogicalChannelBase*& aChannel logical channel to be set
        * @return virtual TInt KErrNone if succesful, otherwise KErrNoMemory
        */
        virtual TInt Create( DLogicalChannelBase*& aChannel );

        /**
        * Complete the installation of driver
		* @since 2.8
        * @return TInt KErrNone if successful 
        */
        TInt Install();


    private:
    
        DIscDevice* iIscDriverDevice;
    };

#endif      // ISCDRIVER_EXTENSION_DEVICE_H   
            
// End of File
