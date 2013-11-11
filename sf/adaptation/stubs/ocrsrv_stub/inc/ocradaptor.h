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
* Description:  ocradapter definitions
*
*/


#ifndef OCRADAPTOR_H
#define OCRADAPTOR_H

// INCLUDES
#include <ocrsrv.h>

// FORWARD DECLARATIONS
class CShareData;
enum TEngineCmd;

// CLASS DECLARATION

/**
 *  Adaptor for engines
 *
 *  Implement concrete operation with child thread either send request 
 *  to child thread or receive operation result from child thread
 *
 *  @lib ocrsrv.lib
 *  @since S60 v3.1
 */
class CEngineAdaptor : public CActive
    {
    public:
        
        /**
        * Symbian Two-phased constructor
        */
        static CEngineAdaptor* NewL(MOCREngineObserver& aObserver, 
                                    const TFileName& aTargetDll, 
                                    const TOcrEngineEnv aEngineEnv);
        
        /**
        * C++ Default Destructor
        */
        virtual ~CEngineAdaptor();
        
    public:
    
        /** 
        * Issue request to child thread
        * @since S60 v3.1
        * @param aCmd A command that is issued
        * @return KErrNone if no error
        */
        TInt IssueRequest(const TEngineCmd aCmd);

        /** 
        * Set image handle to be processed
        * @since S60 v3.1
        * @param aHandle Handle to the image
        * @return None
        */
        void SetImage(const TInt aHandle);

        /**
        * Set the setting for layout
        * @since S60 v3.1
        * @param aSettings Layout settings
        * @return None
        */
        void SetLayoutSetting(const TOCRLayoutSetting& aSettings);

        /**
        * Set Layout block information
        * @since S60 v3.1
        * @param aSettings Layout block information
        * @return None
        */
        void SetLayoutBlockInfo(const TOCRLayoutBlockInfo& aLayoutInfo);

        /**
        * Set special region information
        * @since S60 v3.1
        * @param aSettings Special region information
        * @return None
        */
        void SetSpecialRegionInfo(const TRegionInfo& aRegionInfo);

        /**
        * Set recognition setting 
        * @since S60 v3.1
        * @param aSettings Recognize settings
        * @return None
        */
        void SetRecognizeSetting(const TOCRRecognizeSetting& aSettings);

        /**
        * Set blocks to be recognized
        * @since S60 v3.1
        * @param aRecognizedBlock Blocks to be recognized
        * @return None
        */
        void SetRecognizeBlocksL(const RArray<TInt>& aRecognizedBlock);
        
        /**
        * Set active languages
        * @since S60 v3.1
        * @param aActiveLanguage Languages to be recognized
        * @return None
        */
        void SetActiveLangL(const RArray<TLanguage>& aActiveLanguage);

        /**
        * Cancel the operation
        * @since S60 v3.1
        * @return None
        */
        void CancelOperation();
        
        /**
        * Test if the engine is active or not
        * @since S60 v3.1
        * @return None
        */
        TBool IsEngineActive() const;

    protected: // From CActive

        /**
        * AO callback function
        * @since S60 v3.1
        * @return None
        */
        void RunL();

        /**
        * AO callback function
        * @since S60 v3.1
        * @return None
        */
        TInt RunError(TInt aError);

        /**
        * AO callback function
        * @since S60 v3.1
        * @return None
        */
        void DoCancel();
    
    protected:

        /**
        * Send the command to exit and kill the thread
        * @since S60 v3.1
        * @return None
        */
        void SendExitCmd();
        
        /**
        * Get the current CPU tick of the thread
        * @since S60 v3.1
        * @param aThreadName Name of the thread
        * @return None
        */        
        static void GetThreadName(TDes& aThreadName);
        
        /**
        * Test if the current thread is alive
        * @since S60 v3.1
        * @return ETrue if the thread is alive
        */                
        TBool IsThreadAlive() const;

    private:
    
        /**
        * C++ default constructor
        */
        CEngineAdaptor(MOCREngineObserver& aObserver);

        /**
        * Default Symbian 2nd phase constructor
        */
        void ConstructL(const TFileName& aTargetDll, const TOcrEngineEnv aEngineEnv);

    private:    // Data
    
        /**
        * Handle of the child thread
        */
        RThread iThread;
        
        /**
        * Engine observer
        */
        MOCREngineObserver& iObserver;
        
        /**
        * Pointer to the shared data (Own)
        */
        CShareData* iShareData;
        
        /**
        * Request Status
        */
        TRequestStatus iRequestStatus;
    };

#endif      // OCRADAPTOR_H   
            
// End of File
