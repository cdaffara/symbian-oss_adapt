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
* Description:  Shared data definitions
*
*/


#ifndef OCRSHAREDATA_H
#define OCRSHAREDATA_H

// INCLUDES FILES
#include <ocrcommon.h>

// DATA TYPES

/** Request definitions between main thread and child thread */
enum TEngineCmd
    {
    ECmdInvalid = 0,                    // Invalid  
    ECmdLayout,                         // Layout
    ECmdRecognize,                      // Recognize
    ECmdRecognizeBlock,                 // Recognize block
    ECmdRecognizeSpecialRegion,         // Recognize special region
    ECmdSetLanguage,                    // Set language 
    ECmdGetLanguage,                    // Get language 
    ECmdRefresh,                        // Refresh
    ECmdExit,                           // Exit
    ECmdLayoutComplete,                 // Layout complete
    ECmdRecognizeComplete,              // Recognize complete
    ECmdRecognizeBlockComplete,         // Recognize block complete
    ECmdRecognizeSpecialRegionComplete, // Recognize special region complete
    ECmdRecogProcess,                   // Recognize progress
    ECmdIsLanguageSupported             // Just if supporting the language
    };

// FORWARD DECLARATIONS
class CEngineAdaptor;
class EngineThread;

// CLASS DECLARATION

/**
 *  Shared data between threads
 *
 *  Wrapper class for share data between main thread and child thread
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class CShareData : public CBase
    {
    friend class CEngineAdaptor;
    friend class EngineThread;

public:  // Constructors and destructor

    /**
    * Symbian Two-phased constructor.
    */
    static CShareData* NewL();
    
    /**
    * C++ Default Destructor.
    */
   virtual ~CShareData();

public: // New functions

    /**
    * Enter Share Section
    * @since S60 v3.1
    * @return None
    */
    inline void EnterShareSection();

    /**
    * Leave Share Section
    * @since S60 v3.1
    * @return None
    */
    inline void LeaveShareSection();
    
private:

    /**
    * C++ default constructor
    */
    CShareData();

    /**
    * Default Symbian 2nd phase constructor
    */
    void ConstructL();

private:    // Data

    /**
    * Share data mutex
    */
    RMutex iShareMutex;
    
    /**
    * Notifier for the shared data
    */
    RSemaphore iNotifier;
    
    /**
    * Save active language information
    */
    RArray<TInt> iActiveLanguage;
    
    /**
    * Save installed language information
    */
    RArray<TInt> iInstalledLanguage;

    /**
    * Command sent to child thread
    */
    TEngineCmd iCmd;
    
    /**
    * Command sent from child thread to main thread
    */
    TEngineCmd iCmdRet;

    /** 
    * Recognized first block's pointer (Own)
    */
    TOCRBlockInfo* iFirstBlock;
    
    /**
    * Information of selected blocks
    */
    TOCRBlockInfo* iSelectedBlocks;
    
    /**
    * Information of text region
    */
    TOCRTextRgnInfo* iTextRgnInfo;                                       
                     
    /**
    * Number of blocks recognized
    */                   
    TInt iBlockCount;

    /**
    * OCR binary image (Own)
    */
    HBufC8* iBinaryImage;
    
    /**
    * Handle to the image
    */
    TInt iImageHandle;

    /**
    * Layout setting
    */
    TOCRLayoutSetting iLayoutSettings;
    
    /**
    * Recognzie setting
    */
    TOCRRecognizeSetting iRecognizeSettings;
    
    /**
    * Layout block information
    */
    TOCRLayoutBlockInfo iLayoutInfo;
    
    /**
    * Region information
    */
    TRegionInfo iRegionInfo;

    /**
    * Cancel flag
    */
    TBool iCanceled;

    /**
    * Main thread id
    */
    TThreadId iMainThreadId;

    /** 
    * Percent of the progress
    */
    TUint iPercent;

    /**
    * Error code from the engine
    */
    TInt iError;

    /**
    * Pointer to Active object's status variable (Not Own)
    */
    TRequestStatus* iActiveStatus;
    
    /**
    * Pointer to normal status variable (Not Own)
    */
    TRequestStatus* iRequestStatus;
    
    /**
    * Name of the target dll
    */
    TFileName iTargetDll;
    };

// -----------------------------------------------------------------------------
// CShareData::EnterShareSection
// Description : Enter share section
// -----------------------------------------------------------------------------
//
inline void CShareData::EnterShareSection()
    {
    iShareMutex.Wait();
    }

// -----------------------------------------------------------------------------
// CShareData::LeaveShareSection
// Description : Leave share section
// -----------------------------------------------------------------------------
//
inline void CShareData::LeaveShareSection()
    {
    iShareMutex.Signal();
    }
    
#endif      // OCRSHAREDATA_H   
            
// End of File
