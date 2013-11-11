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
* Description:  ocr engine thread definitions
*
*/


#ifndef OCRTHREAD_H
#define OCRTHREAD_H

// INCLUDES FILES
#include "ocrsharedata.h"

class CFbsBitmap;

/** Global function type definitions */
typedef TBool (*IsCanceled)( void );
typedef TBool (*RecogProgress)(TUint aPercentage);

/** Internal error code definition */
const TInt KErrEngineOK             ( 0 );          //no error
const TInt KErrEngineBadImage       ( -1001 );      //bad image or unsupported format
const TInt KErrEngineBadLanguage    ( -1002 );      //unsupported language
const TInt KErrEngineBadRgn         ( -1003 );      //bad layout region
const TInt KErrEngineNoMemory       ( -1004 );      //no enough memory
const TInt KErrEngineBadParam       ( -1005 );      //wrong parameter
const TInt KErrEngineNotSetLang     ( -1006 );      //not set any language
const TInt KErrEngineBadDicFile     ( -1007 );      //bad dictionary files
const TInt KErrEngineGeneral        ( -1100 );      //the other errors

/** Number of interfaces defined in the engine DLLs */
const TInt KNumOfInterfaces ( 14 );

/** Implementation interfaces definitions */
enum TImplementationIds
    {
    EExitEngine = 1,
    EFreeBinImage,
    EFreeBlocks,
    EGetInstalledLanguages,
    EGetVersion,
    EInitEngine,
    ELayoutAnalysis,
    ERecognizeWithLayout,
    ERecognizeBlock,
    ERefresh,
    ESetActiveLanguage,
    ERecognizeSpecialRegion,
    EIsLanguageSupported,
    EFreeTextRgns
    };

/**
 *  Version of 3rd party ocr engines
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class TEngineVersion
    {
public:
    
    /** 
    * Major version 
    */
    TInt8 iMajor;
    
    /**
    * Minor version
    */
    TInt8 iMinor;
    
    /**
    * Build number
    */
    TInt16 iBuild;
    };
    
/** type definition of the interfaces */
typedef TInt            (*TLibFuncInitEngine)(IsCanceled aFuncIsCanceled, TAny* aParam);
typedef void            (*TLibFuncExitEngine)( void );

typedef TInt            (*TLibFuncLayout)(const CFbsBitmap* aImage, const TOCRLayoutSetting aSettings, HBufC8** aBinImage, TOCRBlockInfo** aBlock, TInt* aBlockCount);
typedef TInt            (*TLibFuncRecognize)(HBufC8* aBinImage, const TOCRRecognizeSetting aSettings, TOCRBlockInfo* aBlock, const TInt aBlockCount, TOCRTextRgnInfo** aTextRgn, RecogProgress aFuncRecogProgress);

typedef void            (*TLibFuncFreeBinImage)(HBufC8* aBinImage);
typedef void            (*TLibFuncFreeBlocks)(TOCRBlockInfo* aBlock);
typedef void            (*TLibFuncFreeTextRgns)(TOCRTextRgnInfo *aTextRgn,const TInt aBlockCount);

typedef TEngineVersion  (*TLibFuncGetVersion)( void );
typedef TInt            (*TLibFuncSetActiveLanguage)(RArray<TInt> iLanguages);
typedef TInt            (*TLibFuncGetInstalledLanguages)(RArray<TInt>& aLanguages);
typedef TInt            (*TLibFuncRefresh)( void );
typedef TBool           (*TLibFuncIsLanguageSupported)(const TLanguage aLanguage);

typedef TInt            (*TLibFuncRecognizeBlock)(const CFbsBitmap* aImage, TOCRLayoutBlockInfo aLayoutInfo, TOCRTextRgnInfo** aTextRgn, RecogProgress aFuncRecogProgress);
typedef TInt            (*TLibFuncRecognizeSpecialRegion)(const CFbsBitmap* aImage, TRegionInfo aRegionInfo, TOCRTextRgnInfo** aTextRgn, RecogProgress aFuncRecogProgress);

// CLASS DECLARATION

/**
 *  Thread for Ocr process
 *
 *  Definition of the child thread of the engine
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class EngineThread 
    {
public:

    /**
    * Thread entry function
    * @since S60 v3.1
    * @param aData Thread parameter
    * @return KErrNone if no error
    */
    static TInt ThreadFunction(TAny* aData);
    
private:

    /**
    * Request Processor
    * @since S60 v3.1
    * @param aShareData Pointer to the shared data
    * @return None
    */
    static void ProcessRequestL(CShareData* aShareData);

    /**
    * Inform main thread that request is completed
    * @since S60 v3.1
    * @param aShareData Pointer to the shared data
    * @param aCmd A specified command
    * @param aErrCode An error code
    * @return None
    */
    static void CompleteRequest(CShareData* aShareData, TEngineCmd aCmd, TInt aErrCode);

    /**
    * Reset the vendor engine and release all memory allocated
    * @since S60 v3.1
    * @param aShareData Pointer to the shared data
    * @param aLibrary A reference to the loaded library
    * @return None
    */
    static void ResetVendorEngine(CShareData* aShareData, const RLibrary& aLibrary);

    /**
    * Get the cancel flag
    * @since S60 v3.1
    * @return ETrue if cancal flag is on; otherwise EFalse
    */
    static TBool GetCancelFlag();

    /**
    * Get the progress information from the engine
    * @since S60 v3.1
    * @param aPercentage A percentage of recognition progress
    * @return KErrNone if no error
    */
    static TInt RecognizeProgress(TUint aPercentage);

    /**
    * Check and convert the engine error to system level error msg
    * @since S60 v3.1
    * @param aError Vendor error code
    * @return None
    */
    static void CheckError(TInt& aError);
    
    /**
    * Handle engine command
    * @since S60 v3.1
    * @param aLibrary A handle to the engine library
    * @param aCommand A command to be handled
    * @param aData Pointer to the shared data
    * @return None
    */        
    static TBool HandleCommandL(const RLibrary& aLibrary, TEngineCmd aCommand, CShareData* aData);
    
    /**
    * Handle engine command - Layout Analysis
    * @since S60 v3.1
    * @param aLibrary A handle to the engine library
    * @param aCommand A command to be handled
    * @param aData Pointer to the shared data
    * @return None
    */    
    static void HandleLayoutAnalysis(const RLibrary& aLibrary, CShareData* aData);
    
    /**
    * Handle engine command - Recognition
    * @since S60 v3.1
    * @param aLibrary A handle to the engine library
    * @param aCommand A command to be handled
    * @param aData Pointer to the shared data
    * @return None
    */    
    static void HandleRecognition(const RLibrary& aLibrary, CShareData* aData);

    /**
    * Handle engine command - Block Recognition
    * @since S60 v3.1
    * @param aLibrary A handle to the engine library
    * @param aCommand A command to be handled
    * @param aData Pointer to the shared data
    * @return None
    */    
    static void HandleBlockRecognition(const RLibrary& aLibrary, CShareData* aData);
    
    /**
    * Handle engine command - Special Region Recognition
    * @since S60 v3.1
    * @param aLibrary A handle to the engine library
    * @param aCommand A command to be handled
    * @param aData Pointer to the shared data
    * @return None
    */    
    static void HandleSpecialRegionRecognition(const RLibrary& aLibrary, CShareData* aData);
    
    /**
    * Handle engine command - Getting Language
    * @since S60 v3.1
    * @param aLibrary A handle to the engine library
    * @param aCommand A command to be handled
    * @param aData Pointer to the shared data
    * @return None
    */    
    static void HandleGettingLanguage(const RLibrary& aLibrary, CShareData* aData);
    
    /**
    * Handle engine command - Setting Language
    * @since S60 v3.1
    * @param aLibrary A handle to the engine library
    * @param aCommand A command to be handled
    * @param aData Pointer to the shared data
    * @return None
    */    
    static void HandleSettingLanguage(const RLibrary& aLibrary, CShareData* aData);
    
    /**
    * Handle engine command - Refresh Database
    * @since S60 v3.1
    * @param aLibrary A handle to the engine library
    * @param aCommand A command to be handled
    * @param aData Pointer to the shared data
    * @return None
    */    
    static void HandleRefreshDatabase(const RLibrary& aLibrary, CShareData* aData);
    };

#endif      // OCRTHREAD_H   
            
// End of File
