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
* Description:  ocrengine definitions
*
*/


#ifndef OCRENGINE_H
#define OCRENGINE_H

// INCLUDES
#include "language.h"
#include <ocrsrv.h>

/** Activate language selection */
enum TLanguageSelected
    {
    /** One language selected to be active */
    ESignleLanguageSelected = 1,
    
    /** Two languages selected to be active */
    ETwoLanguageSelected = 2
    };

// FORWARD DECLARATIONS
class CEngineAdaptor;
class COCREngineImplBase;

// CLASS DECLARATION

/**
 *  Ocr engine wrapper implementation base
 *
 *  Abstruct for ocr engines
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class COCREngineImplBase : public MOCREngineBase,
                           public CBase
    {
public: 

    /**
    * Symbian Two-phased constructor.
    */
    static COCREngineImplBase* NewL(MOCREngineObserver& aObserver, const TOcrEngineEnv aEngineEnv);
    
    /**
    * Default C++ Destructor
    */
    virtual ~COCREngineImplBase();

public: // From MOCREngineInterface

    /**
    * Cancel recognition operation
    * @since S60 v3.1
    * @return None
    */
    virtual void CancelOperation();

    /** 
    * Get the version of the ocrsrv component
    * @since S60 v3.1
    * @return The version of the module
    */
    virtual TVersion GetVersion() const;

    /**
    * Refresh installed language databases
    * @since S60 v3.1
    * @return None
    */
    virtual void RefreshInstalledLanguageL();
    
    /**
    * Set the language for recognition
    * @since S60 v3.1
    * @param aActiveLanguage A languages to be activated
    * @return None
    */
    virtual void SetActiveLanguageL(const RArray<TLanguage>& aActiveLanguage);

    /** 
    * Get the installed file directory
    * @since S60 v3.1
    * @param aLanguages Get installed languages
    * @return None
    */
    virtual void GetInstalledLanguage(RArray<TLanguage>& aLanguages) const;

    /**
    * Test if the language is supported
    * @since S60 v3.1
    * @param aLanguage A language to be tested
    * @return ETrue if it's supported
    */
    virtual TBool IsLanguageSupported(const TLanguage aLanguage);

    /**
    * Release the engine instance
    * @since S60 v3.1 
    * @return None
    */
    void Release();
    
    /**
    * Get adaptor object
    * @since S60 v3.1
    * @return Pointer to the adaptor object
    */
    inline CEngineAdaptor* GetAdaptor() const; 
    
    /**
    * Test if the engine is active or not
    * @since S60 v3.1
    * @return None
    */
    virtual TBool IsEngineActive() const;       

protected:

    /**
    * Default Symbian 2nd phase constructor
    */
    void ConstructL(MOCREngineObserver& aObserver, const TOcrEngineEnv aEngineEnv);

private:

    /**
    * Reset adaptor when active language(s) is changed
    * @since S60 v3.1
    * @param aActiveLanguage New language to be activated
    * @return None
    */
    void ResetAdaptorL(const RArray<TLanguage>& aActiveLanguage);

private:

    /** 
    * Pointer to the adaptor instance (Own)
    */
    CEngineAdaptor* iAdaptor;
    
    /**
    * Pointer to the engine observer (Not Own)
    */
    MOCREngineObserver* iObserver;
    
    /**
    * Pointer to the current engine (Own)
    */
    COCREngineList* iLanguageEngine;
    
    /**
    * File name of the current loaded Dll
    */
    TFileName iCurrentDll;
    
    /**
    * Settings of the engine working aera
    */
    TOcrEngineEnv iEngineEnv;     
    };

/**
 *  Ocr engine wrapper for recognition with layout analysis
 *
 *  Abstruct for ocr engines with layout analysis
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class COCREngineLayoutRecognize : public MOCREngineLayoutRecognize,
                                  public CBase
    {
    friend class OCREngineFactory;

public: 
    /**
    * Symbian Two-phased constructor.
    */
    static COCREngineLayoutRecognize* NewL(MOCREngineObserver& aObserver, 
                                           const TOcrEngineEnv aEngineEnv);

public: // From MOCREngineWithLayout

    /**
    * Layout analysis method
    * @since S60 v3.1
    * @param aBitmapHandle An image to analyzed
    * @param aSettings Setting for layout analysis
    * @return None
    */
    virtual void LayoutAnalysisL(const TInt aBitmapHandle, const TOCRLayoutSetting aSettings);

    /**
    * Text recognition method
    * @since S60 v3.1
    * @param aSetting Setting for recognition
    * @param aBlock Block Ids to be recognized
    * @return None
    */
    virtual void RecognizeL(const TOCRRecognizeSetting aSettings, const RArray<TInt>& aBlock);
   
    /**
    * Provide base functions
    * @since S60 v3.1
    * @return Pointer to the base engine instance
    */
    MOCREngineBase* Base() const;

private:

    /**
    * Default Symbian 2nd phase constructor
    */
    void ConstructL(MOCREngineObserver& aObserver, const TOcrEngineEnv aEngineEnv);
    
protected:

    /**
    * Default C++ Destructor
    */
    virtual ~COCREngineLayoutRecognize();

private:

    /** 
    * Pointer to the engine base instance (Own)
    */
    COCREngineImplBase* iBase;
    
    /**
    * Pointer to the adaptor instance (Not Own)
    */
    CEngineAdaptor* iAdaptor;
    };

/**
 *  Ocr engine wrapper for recognition with block recognition
 *
 *  Abstruct for ocr engines with block recognition
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class COCREngineRecognizeBlock : public MOCREngineRecognizeBlock,
                                 public CBase
    {
    friend class OCREngineFactory;

public: 
    /**
    * Description: Two-phased constructor.
    */
    static COCREngineRecognizeBlock* NewL(MOCREngineObserver& aObserver, 
                                          const TOcrEngineEnv aEngineEnv);

public: // From MOCREngineWithoutLayout

    /**
    * Recognize block without layout analysis
    * @since S60 v3.1
    * @param aBitmapHandle An image to analyzed
    * @param aLayoutInfo Layout information
    * @return None
    */
    virtual void RecognizeBlockL(const TInt aBitmapHandle,
                                 const TOCRLayoutBlockInfo aLayoutInfo);
   
    /**
    * Recognize special region
    * @since S60 v3.1
    * @param aBitmapHandle An image to analyzed
    * @param aRegionInfo Region information
    * @return None
    */
    virtual void RecognizeSpecialRegionL(const TInt aBitmapHandle,
                                         const TRegionInfo aRegionInfo);

    /**
    * Provide base functions
    * @since S60 v3.1
    * @return Pointer to the base engine instance
    */
    MOCREngineBase* Base() const;

private:

    /**
    * Default Symbian 2nd phase constructor
    */
    void ConstructL(MOCREngineObserver& aObserver, const TOcrEngineEnv aEngineEnv);
    
    /**
    * Check the validity of the given rect
    * If the range is invalid, a leave will be raised with KErrArgument
    * @since S60 v3.1
    * @param aBitmapHandle A bitmap handle
    * @param aRange A given range
    * @return None
    */
    void CheckRangeL(const TInt aBitmapHandle, const TRect aRange);

protected:

    /**
    * Default C++ Destructor
    */
    virtual ~COCREngineRecognizeBlock();

private:

    /** 
    * Pointer to the engine base instance (Own)
    */
    COCREngineImplBase* iBase;
    
    /**
    * Pointer to the adaptor instance (Not Own)
    */    
    CEngineAdaptor* iAdaptor;
    };

#include "ocrengine.inl"

#endif // OCRENGINE_H   
            
// End of File
