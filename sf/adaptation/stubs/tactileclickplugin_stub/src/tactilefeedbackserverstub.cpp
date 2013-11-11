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
* Description:  Implementation of the method for accessing 
*                MTactileFeedbackServer -instance
* Part of:      Tactile Feedback.
*
*/


#include "tactileclickpluginstub.h"
#include "tactilefeedbackserver.h"




EXPORT_C MTactileFeedbackServer* MTactileFeedbackServer::Instance()
    {   
    CTactileClickPlugin* clickPlugin = NULL;
    
    TAny* tmpPtr = Dll::Tls();
    
    if ( tmpPtr )
        {
        clickPlugin = static_cast<CTactileClickPlugin*>( tmpPtr );
        }
    
    return clickPlugin;
    }


