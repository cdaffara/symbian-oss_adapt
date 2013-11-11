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
* Description:  The implementation of TASYMessage example class
*
*/



// INCLUDE FILES
#include "ASYmessage.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TSASMessage::TSASMessage
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
TASYMessage::TASYMessage( const TUint32 aGroupId, const TUint32 aMessageId, TDesC8& aData )
    : iGroupId( aGroupId ),
    iMessageId( aMessageId ),    
    iData( aData )
    {
    }

// -----------------------------------------------------------------------------
// TSASMessage::GroupId()
// This function returns the group id of the SAS message
// -----------------------------------------------------------------------------
//
TUint32 TASYMessage::GroupId()
    {

    return iGroupId;

    }

// -----------------------------------------------------------------------------
// TSASMessage::MessageId()
// This function returns the message id of the SAS message
// -----------------------------------------------------------------------------
//
TUint32 TASYMessage::MessageId()
    {

    return iMessageId;

    }

// -----------------------------------------------------------------------------
// TSASMessage::Data()
// This function returns the transaction id of the SAS message
// -----------------------------------------------------------------------------
//
TDesC8& TASYMessage::Data()
    {

    return iData;

    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

//  End of File
