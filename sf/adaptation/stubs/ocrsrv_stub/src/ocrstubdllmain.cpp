/*
* Copyright (c) 2200 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dll main entry
*
*/


// INCLUDE FILES
#include <e32base.h>

#if !defined(EKA2)

// requirement for E32 DLLs
EXPORT_C TInt E32Dll(TDllReason)
    {
    return KErrNone;
    }
    
#endif // EKA2

//  End of File
