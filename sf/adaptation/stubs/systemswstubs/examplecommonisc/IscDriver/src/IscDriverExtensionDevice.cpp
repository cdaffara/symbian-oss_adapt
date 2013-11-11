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



// INCLUDE FILES

#include "IscDriverExtensionDevice.h"
#include "IscChannel.h"
#include "IscTrace.h"
#include "kern_priv.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS
_LIT( KIscDriverExtensionName,"IscDriverExtension" );
_LIT( KIscDriverName,"IscDriver" );

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// DIscDriverExtensionDevice::DIscDriverExtensionDevice
// C++ default constructor
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
DIscDriverExtensionDevice::DIscDriverExtensionDevice()
    : iIscDriverDevice( NULL )
    {
    C_TRACE( ( _T( "DIscDriverExtensionDevice::DIscDriverExtensionDevice()" ) ) );
    iParseMask |= KDeviceAllowUnit;
    iParseMask |= KDeviceAllowInfo;
    }

// Destructor
DIscDriverExtensionDevice::~DIscDriverExtensionDevice()
    {
    
    }


// -----------------------------------------------------------------------------
// DIscDriverExtensionDevice::Install
// Complete the installation of driver
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscDriverExtensionDevice::Install()
    {
    C_TRACE( ( _T( "DIscDriverExtensionDevice::Install()" ) ) );
  
    // Find pointer to ISC Driver.
    TInt err( KErrNone );
    DObjectCon* lDevices = Kern::Containers()[ELogicalDevice];
    TKName driverName;

    ASSERT_RESET_ALWAYS( lDevices, "IscDriverExtension", EIscLogicalDevicesNotFound );

    //TInt driverHandle( KErrNone );  // API change in SOS9.2 WK08
	TFindHandle driverHandle;
    err = lDevices->FindByName( driverHandle, KIscDriverName, driverName );
    if( KErrNone != err )
        {
        C_TRACE( ( _T( "DIscDriverExtensionDevice::Install() Pointer to IscDriver not found!" ) ) );
        ASSERT_RESET_ALWAYS( 0, "IscDriverExtension", EIscDriverNotFound );
        }
    iIscDriverDevice = static_cast<DIscDevice*>( lDevices->At( driverHandle ) );
    ASSERT_RESET_ALWAYS( iIscDriverDevice, "IscDriverExtension", EIscDriverNotFound )
    return ( SetName( &KIscDriverExtensionName ) );
    
    }

// -----------------------------------------------------------------------------
// DIscDevice::Create
// Create a logical channel
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
TInt DIscDriverExtensionDevice::Create( 
    DLogicalChannelBase*& aChannel )
    {
    aChannel=new DIscChannel( iIscDriverDevice );
    return aChannel?KErrNone:KErrNoMemory;
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================
// -----------------------------------------------------------------------------
// E32Dll
// Epoc Kernel Architecture 2 style entry point
// ( other items were commented in a header ).
// -----------------------------------------------------------------------------
//
DECLARE_STANDARD_LDD()
    {
    DLogicalDevice* device = new DIscDriverExtensionDevice;
    ASSERT_RESET_ALWAYS( device,"IscDriverExtension",EIscPanicCreateLogicalDevice );
    return device;
    
    }

//  End of File  
