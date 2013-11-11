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
* Description:  Declaration of MASYAdaptation class. MASYAdaptation class
*                defines an example interface to receive events from lower layer and also command sending.
*
*/



#ifndef MASYADAPTATION_H
#define MASYADAPTATION_H

// INCLUDES
#include <e32base.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class TASYMessage;

// CLASS DECLARATION

/**
*  MASYAdaptation
*/
class MASYAdaptation
    {

    public: // New functions
        
        /**
        * Indicates ASY about received response message
        * @return none
        */
        virtual void ProcessResponseL( TASYMessage& aMessage ) = 0;

        /**
        * Indicates ASY about received error response message
        * @return none
        */
        virtual void ProcessErrorResponseL( TInt aErrorCode, TASYMessage& aMessage ) = 0;

        /**
        * Indicates ASY about received event message
        * @return none
        */
        virtual void RaiseEventL( TASYMessage& aMessage ) = 0;
    };

#endif      // MASYADAPTATION_H   
            
// End of File