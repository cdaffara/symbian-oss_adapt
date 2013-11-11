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
* Description:  Declaration of MAdaptation class. MAdaptation class
*                defines an example interface to receive events from Domestic OS.
*
*/



#ifndef MADAPTATION_H
#define MADAPTATION_H

// INCLUDES
#include <e32base.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class TAdaptationMessage;

// CLASS DECLARATION

/**
*  MASYAdaptation
*/
class MAdaptation
    {

    public: // New functions
        
        /**
        * Indicates bearer about received response message
        * @return none
        */
        virtual void ProcessResponseL( TAdaptationMessage& aMessage ) = 0;

        /**
        * Indicates bearer about received error response message
        * @return none
        */
        virtual void ProcessErrorResponseL( TInt aErrorCode, TAdaptationMessage& aMessage ) = 0;

        /**
        * Indicates bearer about received key event message.
        * @return none
        */
        virtual void RaiseEventL( TAdaptationMessage& aMessage ) = 0;
    };

#endif      // MADAPTATION_H   
            
// End of File