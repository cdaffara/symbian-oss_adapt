/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Inline functions for language mgr
*
*/


// -----------------------------------------------------------------------------
// Get the id of the engine
// -----------------------------------------------------------------------------
//
inline const TInt COCREngine::GetEngineId() const
    {
    return iEngineId;
    }

// -----------------------------------------------------------------------------
// Get supported languages
// -----------------------------------------------------------------------------
//
inline const RArray<TOCRLanguage>& COCREngine::GetSupportLanguage() const  
    {
    return iLanguageList;
    } 

// -----------------------------------------------------------------------------
// Get supported language list
// -----------------------------------------------------------------------------
//
inline const RArray<TOCRCombinedLanguage>& COCREngine::GetSupportCombinedLanguage() const 
    {
    return iCombinedLanguageList;
    } 

// -----------------------------------------------------------------------------
// Test if the language is a eastern language
// -----------------------------------------------------------------------------
//
inline TBool COCREngine::IsEast(const TLanguage aLanguage) const
    {
	return ((aLanguage == ELangPrcChinese) ||
		(aLanguage == ELangJapanese) ||
		(aLanguage == ELangHongKongChinese) ||
		(aLanguage == ELangTaiwanChinese));
    }

// -----------------------------------------------------------------------------
// ?implementation_description
// -----------------------------------------------------------------------------
//
inline TInt COCREngineList::GetEngineCount() const
    {
    return iOCREngineList.Count();
    }

// End Of File
