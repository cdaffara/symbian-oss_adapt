/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Used as an reference implementation.
*
*/


#ifndef CASYREFERENCECMDHANDLERBASE_H
#define CASYREFERENCECMDHANDLERBASE_H

// INCLUDES
#include <AsyCommandHandlerBase.h>
#include "ASYAdaptation.h"
#include "ASYmessage.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
//class CSACSession;
class CASYReferenceMainServiceBase;

// CLASS DECLARATION

/**
*  CSACAccessoryCommandHandler
*  This class declarates the CSACAccessoryCommandHandler
*/

class CASYReferenceCmdHandlerBase : public CASYCommandHandlerBase, public MASYAdaptation
    {
    public: // constructor and destructor

        /**
        * Constructor
        */
        static CASYReferenceCmdHandlerBase* NewL();

        /**
        * Destructor.
        */
        virtual ~CASYReferenceCmdHandlerBase();

    private: // constuctors

        /**
        * C++ constructor.
        */
        CASYReferenceCmdHandlerBase();

        /**
        * 2nd phase constructor.
        */
        void ConstructL();

         /**
        * Maps ASY command parameter to internal accessory feature
        * @param aCommandParamRecord    Accessory Server parameter
        * @param aAccessoryFeature      Internal accessory feature
        */
        void MapCommandToAccessoryFeatureL( const TASYCommandParamRecord& aCommandParamRecord, 
                                            AccessoryFeature::TAccessoryFeature& aAccessoryFeature );

    private: // functions from base classes
    
        /**
        * From CASYCommandHandlerBase
        * Processes command from Accessory FW
        * @param aCommand               Command
        * @param aCmdParams             Parameters
        * @return none
        */
        void ProcessCommandL( const TProcessCmdId aCommand, const TASYCmdParams& aCmdParams );
        
    private: // functions from base classes

        /**
        * From MASYAdaptation
        * Indicates ASY about received response message
        * @param aMessage              Reference to a received message
        * @return                      none
        */
        void ProcessResponseL( TASYMessage& aMessage );

        /**
        * From MASYAdaptation
        * Indicates ASY about received error response message
        * @param aErrorCode             Error code for current message
        * @param aMessage               Reference to a received message
        * @return none
        */
        void ProcessErrorResponseL( TInt aErrorCode, TASYMessage& aMessage );
        
        /**
        * From MASYAdaptation
        * Indicates ASY about received accessory event message
        * @param aMessage               Reference to a received message
        * @return none
        */
        void RaiseEventL( TASYMessage& aMessage );

    private: // data
                
    };

#endif //CASYREFERENCECMDHANDLERBASE_H
            
// End of File

