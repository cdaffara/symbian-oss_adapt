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
* Description:  The implementation of TAdaptationMessage example clas.s
*
*/



// INCLUDE FILES
#include "AdaptationMessage.h"

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
TAdaptationMessage::TAdaptationMessage( const TUint32 aGroupId, const TUint32 aMessageId, TDesC8& aData )
    : iGroupId( aGroupId ),
    iMessageId( aMessageId ),    
    iData( aData )
    {
    }

// -----------------------------------------------------------------------------
// TSASMessage::GroupId()
// -----------------------------------------------------------------------------
//
TUint32 TAdaptationMessage::GroupId()
    {

    return iGroupId;

    }

// -----------------------------------------------------------------------------
// TSASMessage::MessageId()
// -----------------------------------------------------------------------------
//
TUint32 TAdaptationMessage::MessageId()
    {

    return iMessageId;

    }

// -----------------------------------------------------------------------------
// TSASMessage::Data()
// -----------------------------------------------------------------------------
//
TDesC8& TAdaptationMessage::Data()
    {

    return iData;

    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

//  End of File
