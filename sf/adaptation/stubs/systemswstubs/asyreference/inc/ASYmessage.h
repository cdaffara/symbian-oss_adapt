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
* Description:  Used as an reference implementation
*
*/



#ifndef ASYMESSAGE_H
#define ASYMESSAGE_H

//  INCLUDES
#include <e32std.h>
#include "ASYmessagedefinitions.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION
NONSHARABLE_CLASS( TASYMessage );

/**
*  TASYMessage
*  This class declarates the TASYMessage, which is a generic message class example.
*  message class.
*
*/
class TASYMessage
    {
    public:  // Constructors and destructor

        /**
        * Constructor.
        * @param aGroup         Message group.
        * @param aID            Message ID.
        * @param aData          Pointer to data.        
        */
        TASYMessage( const TUint32 aGroupId, const TUint32 aMessageId, 
                              TDesC8& aData );         
    
    public: // New functions
        
        /**
        * Returns message group
        * @return               Message group
        */
        TUint32 GroupId(); 

        /**
        * Returns message id
        * @return               Message id
        */
        TUint32 MessageId();

        /**
        * Returns message data
        * @return               Message data
        */
        TDesC8& Data();


    private:    // Data
        // Group id
        TUint32 iGroupId;

        // Message id
        TUint32 iMessageId;
       
        // Message data
        TDesC8& iData;

    };

#endif      // ASYMESSAGE_H

// End of File
