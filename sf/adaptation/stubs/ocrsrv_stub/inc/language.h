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
* Description:  language manager definitions
*
*/


#ifndef OCRLANGUAGE_H
#define OCRLANGUAGE_H

// INCLUDES FILES
#include "languageconvertor.h"
#include "ocrthread.h"
#include <ocrsrv.h>
#include <f32file.h>

// CONSTANTS
const TInt KFileNameLength = 128;

/** Pathes where the engine marks stores */
_LIT(KDataPathZ, "z:\\resource\\ocrsrv\\*.dat");
_LIT(KDataPathC, "c:\\resource\\ocrsrv\\*.dat");

_LIT( KExtend, "dll" );

/** Language Definitions */
const TInt KEngineLangInvalid     ( 0 );
const TInt KEngineLangChineseSimp ( 1 );
const TInt KEngineLangChineseTrad ( 2 );
const TInt KEngineLangJapanese    ( 3 );
const TInt KEngineLangEnglish     ( 101 );
const TInt KEngineLangFinish      ( 102 );

/**
 *  Internal Language Structure
 *
 *  Map one language instance with an engine id
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class TOCRLanguage
    {
public:

    /**
    * Eninge id
    */
    TInt       iEngineId;
    
    /**
    * Language
    */
    TLanguage  iLanguage;
    };

/**
 *  Internal Language Structure
 *
 *  Map two combined languages with an engine ID
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class TOCRCombinedLanguage
    {
public:

    /**
    * Eninge id
    */ 
    TInt       iEngineId;
    
    /**
    * Eastern language that the engine supports
    */
    TLanguage  iLanguageEast;
    
    /**
    * Western language that the engine supports
    */
    TLanguage  iLanguageWest;
    };
 
/**
 *  OCR Engine Wrapper class
 *
 *  OCR Engine wrapper which offers the abstruct of all
 *  OCR engines
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class COCREngine : public CBase
    {
public:

    /**
    * Default C++ Destructor
    */
    virtual ~COCREngine();

    /**
    * Symbian Two-phased constructor
    */
    static COCREngine* NewL(const TDesC& aEnginePathAndName,const TInt aEngineId);

    /**
    * Initialize the analyze engine
    * @since S60 v3.1
    * @return None
    */
    void AnalyzeEngineL();

    /**
    * Empty data of current engine
    * @since S60 v3.1
    * @return none
    */
    void EmptyData();

    /**
    * Get engine Id
    * @since S60 v3.1
    * @return Engine id
    */
    inline const TInt GetEngineId() const ;

    /**
    * Get unique engine name
    * @since S60 v3.1
    * @param aEngineName A new engine name
    * @return None
    */
    void GetEngineName(TDes& aEngineName) const ;

    /**
    * Get languages the engine support
    * @since S60 v3.1
    * @return Supported language list
    */
    inline const RArray<TOCRLanguage>& GetSupportLanguage() const ;

    /**
    * Get language combinations the engine support
    * @since S60 v3.1
    * @return Supported language combination list
    */
    inline const RArray<TOCRCombinedLanguage>& GetSupportCombinedLanguage() const;

private:

    /**
    * By default Symbian 2nd phase constructor is private
    * @since S60 v3.1
    * @param aEngineName Engine name to be constructed
    * @param aEngineId Engine Id to be constructed
    * @return None
    */
    void ConstructL(const TDesC& aEngineName,const TInt aEngineId);

    /**
    * Justify if given language is an eastern language
    * @since S60 v3.1
    * @param aLanguage A language to be tested
    * @return ETrue if it's an eastern language
    */
    inline TBool IsEast(const TLanguage aLanguage) const;

    /**
    * Combine eastern and western languages
    * @since S60 v3.1
    * @param aEast eastern language to be combined
    * @param aWest western language to be combined
    * @param aDestination An array which holds the result
    * @return None
    */
    void CombineLanguage(const RArray<TLanguage>& aEast, 
                         const RArray<TLanguage>& aWest, 
                         RArray<TOCRCombinedLanguage>& aDestination);

    /**
    * C++ default constructor
    */
    COCREngine();

private:

    /**
    * Engine id
    */
    TInt iEngineId;
    
    /**
    * Enigne name
    */
    TBuf<KFileNameLength> iEngineName;
    
    /**
    * Languages that the engine support
    */
    RArray<TOCRLanguage>         iLanguageList;
    
    /**
    * Language combinations that the engine support
    */
    RArray<TOCRCombinedLanguage> iCombinedLanguageList;
    };

/**
 *  OCR Engine Wrapper list
 *
 *  List of all loaded engines
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class COCREngineList : public CBase
    {
public:

    /**
    * Symbian Two-phased constructor
    */
    static COCREngineList* NewL();

    /**
    * Default C++ destructor
    */
    virtual ~COCREngineList();

    /**
    * Justify if the language is supported
    * @since S60 v3.1
    * @param aLanguage A language to be tested
    * @return ETrue if supported
    */
    TBool IsSupportLanguage(const TLanguage aLanguage);

    /**
    * Get all supported languages
    * @since S60 v3.1
    * @param aLanguages A list which holds all supported languages
    * @return None
    */
    void GetALLSupportLanguage(RArray<TLanguage> &aLanguages);

    /**
    * Refresh language databases, this method shall be always called
    * whenever databases changed
    * @since S60 v3.1
    * @return None
    */
    void RefreshL();

    /**
    * Get supported engine numbers
    * @since S60 v3.1
    * @return Num of engine detected
    */
    inline TInt GetEngineCount() const;

    /**
    * Get engine name by a given language
    * @since S60 v3.1
    * @param aLanguage A language supported by the engine
    * @param aEngineName The name of the engine
    * @return KErrNone if no error, otherwise system level error raised
    */
    TInt GetEngineName(const TLanguage aLanguage, TDes& aEngineName);

    /**
    * Get engine name by language combination
    * @since S60 v3.1
    * @param aLanguageFirst A language supported by the engine
    * @param aLanguageSecond Another language supported by the engine
    * @param aEngineName The name of the engine
    * @return KErrNone if no error, otherwise system level error raised
    */        
    TInt GetEngineName(const TLanguage aLanguageFirst,
                       const TLanguage aLanguageSecond, 
                       TDes& aEngineName);

private:

    /**
    * Empty data of the engine
    * @since S60 v3.1
    * @return None
    */
    void EmptyData();

    /**
    * Analyze engines information
    * @since S60 v3.1
    * @return None
    */
    void DoAnalyzeEnginesInfoL();

    /**
    * Set engines information
    * @since S60 v3.1
    * @param aDir A directory information array
    * @param aCount A number of dirs of the array
    * @return None
    */
    void SetEngineInfoL(const CDir* aDir, const TInt aCount);

    /**
    * Default Symbian 2nd phase constructor
    * @since S60 v3.1
    * @return None
    */
    void ConstructL();

    /**
    * Add language array
    * @since S60 v3.1
    * @param aSource A src language array
    * @param aDestination A tar language array
    * @return None
    */
    void AddLanguageArray(const RArray<TOCRLanguage>& aSource, RArray<TOCRLanguage>& aDestination);

    /**
    * Add language combination array
    * @since S60 v3.1
    * @param aSource A src language array
    * @param aDestination A tar language array
    * @return None
    */
    void AddCombinedLanguageArray(const RArray<TOCRCombinedLanguage>& aSource, 
                                  RArray<TOCRCombinedLanguage>& aDestination);

    /**
    * Test if the language is within one array
    * @since S60 v3.1
    * @param aLanguage A language to be tested
    * @param aArray An array to be tested
    * @return ETrue if the language is in language list
    */
    TBool IsHaveLanguage(const TLanguage aLanguage, const RArray<TOCRLanguage>& aArray) const;

    /**
    * Test if the language is within one combined array
    * @since S60 v3.1
    * @param aLanguageFirst A language to be tested
    * @param aLanguageSecond Another language to be tested
    * @param aArray An array to be tested
    * @return ETrue if the language is in language list
    */
    TBool IsHaveCombinedLanguage(const TLanguage aLanguageFirst, 
                                 const TLanguage aLanguageSecond, 
                                 const RArray<TOCRCombinedLanguage>& aArray) const;

    /**
    * C++ default constructor
    */
    COCREngineList();

private:
    
    /**
    * Pathes for searching the ocr libraries
    */
    RPointerArray<HBufC> iPathes;
    
    /**
    * List of all supported lanugages
    */
    RArray<TOCRLanguage> iLanguageList;
    
    /**
    * List of all combined languages, this generated from iLanguageList
    * e.g. if we support English and Japanese, then the iCombinedLanguageList
    * would have English, Japanese and EnglishJapanese languages. Note that
    * only one western language and one eastern language would be combined
    */
    RArray<TOCRCombinedLanguage> iCombinedLanguageList;
    
    /**
    * List of all loaded OCR engine instances
    */
    RPointerArray<COCREngine> iOCREngineList;
    
    /**
    * File session
    */
    RFs iFs;
    };

#include "language.inl"

#endif // OCRLANGUAGE_H

// End Of File
