/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Language convertor definitions
*
*/


#ifndef OCRLANGUAGECONVERTOR_H
#define OCRLANGUAGECONVERTOR_H

// INCLUDE FILES
#include <e32base.h>

/**
 *  Language utils
 *
 *  Offering language utilities
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class ConvertLanguage
    {
    public:

        /**
        * Convert vendor language defination to Symbian's language definitions
        * @since S60 v3.1
        * @param aLanguage A language to be converted
        * @return The language converted
        */
        static TLanguage ConvertToSymbian(const TInt aLanguage);

        /**
        * Convert Symbian language defination to Vendor's language definitions
        * @since S60 v3.1
        * @param aLanguage A language to be converted
        * @return The language converted
        */
        static TInt ConvertToVendor(const TLanguage aLanguage);
    
        /**
        * Test if the language refer to multiple symbian's language definitions
        * @since S60 v3.1
        * @param aLanguage A language to be tested
        * @return ETrue if it is
        */
        static TBool IsForMutiLanguage(const TInt aLanguage);

        /**
        * Get symbian's full language list for the language 
        * @since S60 v3.1
        * @param aLanguage A language to be specified
        * @param aArray Symbian's full language list
        * @return None
        */
        static void GetMutiLanguage(const TInt aLanguage, RArray<TLanguage>& aArray);
    };

#endif // OCRLANGUAGECONVERTOR_H

// End of file
