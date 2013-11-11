/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Engine factory implementation
*
*/


// INCLUDE FILES
#include "ocrsharedata.h"
#include "ocradaptor.h"
#include "ocrengine.h"
#include "language.h"
#include <ocrsrv.h>

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// OCREngineFactory::ReleaseOCREngine
// Description : Release engine
// ----------------------------------------------------------
//
EXPORT_C void OCREngineFactory::ReleaseOCREngine(MOCREngineInterface* /*aEngine*/)
    {
    }

// ----------------------------------------------------------
// OCREngineFactory::CreateOCREngineL
// Description : Create engine
// ----------------------------------------------------------
//
EXPORT_C MOCREngineInterface* OCREngineFactory::CreateOCREngineL(MOCREngineObserver& /*aObserver*/, 
                                                                 const TOcrEngineEnv /*aEngineEnv*/, 
                                                                 TEngineType /*aEngineType*/)
    {
    User::Leave(KErrNotSupported); 
    return NULL;       
    }
    
OCREngineFactory::OCREngineFactory()
    {
    }

// ---------------------------------------------------------------
// COCREngine::COCREngine
// Description : self-construct
// ---------------------------------------------------------------
//
COCREngine::COCREngine()
    {
    }

// ---------------------------------------------------------------
// COCREngine::~COCREngine
// Description : Destruct
// ---------------------------------------------------------------
//
COCREngine::~COCREngine()
    {
    }
    
// -----------------------------------------------------------------------------
// CShareData::CShareData
// Description : C++ default constructor can NOT contain any code, that
//               might leave.
// -----------------------------------------------------------------------------
//
CShareData::CShareData()
    {
    }

// -----------------------------------------------------------------------------
// CShareData::~CShareData
// Description : destructor.
// -----------------------------------------------------------------------------
//
CShareData::~CShareData()
    {
    }    

// -----------------------------------------------------------------------------
// CEngineAdaptor::CEngineAdaptor
// Description : C++ default constructor can NOT contain any code, that
//               might leave.
// -----------------------------------------------------------------------------
//
CEngineAdaptor::CEngineAdaptor(MOCREngineObserver& aObserver)
                    : CActive(EPriorityNormal), iObserver(aObserver)
    {
    }

// -----------------------------------------------------------------------------
// CEngineAdaptor::~CEngineAdaptor
// Description : Destructor
// -----------------------------------------------------------------------------
//
CEngineAdaptor::~CEngineAdaptor()
    {
    }

// ---------------------------------------------------------
// CEngineAdaptor::RunL()
// Description :  callback function for request complete,
//                either layout analysis complete,
//                or recognize complete
//                or recognize process information is given 
// ---------------------------------------------------------
//
void CEngineAdaptor::RunL()
    {
    }

// ---------------------------------------------------------
// CEngineAdaptor::RunError(...)
// Description : Deal with errors from RunL()
// ---------------------------------------------------------
//
TInt CEngineAdaptor::RunError(TInt /*aError*/)
    {   
    return KErrNone;
    }

// ----------------------------------------------------------
// CEngineAdaptor::DoCancel
// Description : from CActive
// ----------------------------------------------------------
//
void CEngineAdaptor::DoCancel()
    {
    }

// ---------------------------------------------------------------
// COCREngineList::COCREngineList
// Description : self-construct
// ---------------------------------------------------------------
//
COCREngineList::COCREngineList()
    {
    }   

// ---------------------------------------------------------------
// COCREngineList::~COCREngineList
// Description : destruct
// ---------------------------------------------------------------
//
COCREngineList::~COCREngineList()
    {
    }

// ---------------------------------------------------------------
// COCREngineImplBase::~COCREngineImplBase
// Description : Destructor.
// ---------------------------------------------------------------
//
COCREngineImplBase::~COCREngineImplBase()
    {
    }

// ---------------------------------------------------------------
// COCREngineImplBase::CancelRecognition()
// Description : Cancel recognition operation
// -------------------------------------------------------------
//
TBool COCREngineImplBase::IsEngineActive() const
    {
    return EFalse;
    }

// ---------------------------------------------------------------
// COCREngineImplBase::IsLanguageSupported
// Description : IsLanguageSupported
// -------------------------------------------------------------
//
TBool COCREngineImplBase::IsLanguageSupported(const TLanguage /*aLanguage*/)
    {
    return EFalse;
    }

// ---------------------------------------------------------------
// COCREngineImplBase::CancelRecognition()
// Description : Cancel recognition operation
// -------------------------------------------------------------
//
void COCREngineImplBase::CancelOperation()
    {   
    }

// -------------------------------------------------------------
// COCREngineImplBase::GetInstalledLanguageL
// Description : Get installed language information
// -------------------------------------------------------------
//
void COCREngineImplBase::GetInstalledLanguage(RArray<TLanguage>& /*aLanguages*/) const
    {
    }

// ------------------------------------------------------------- 
// COCREngineImplBase::SetActiveLanguageL
// Description : Set active language
// -------------------------------------------------------------
//
void COCREngineImplBase::SetActiveLanguageL(const RArray<TLanguage>& /*aActiveLanguage*/)
    {   
    }

// ---------------------------------------------------------------
// COCREngineImplBase::RefreshInstalledLanguageL
// Description : Refresh installed language
// -------------------------------------------------------------
//
void COCREngineImplBase::RefreshInstalledLanguageL()
    {
    }

// ---------------------------------------------------------------
// COCREngineImplBase::CancelRecognition()
// Description : Cancel recognition operation
// -------------------------------------------------------------
//    
TVersion COCREngineImplBase::GetVersion() const
    {
    return (TVersion(1, 0, 0));
    } 

// ---------------------------------------------------------------
// COCREngineRecognizeBlock::~COCREngineRecognizeBlock
// Description : Destructor
// ---------------------------------------------------------------
//
COCREngineRecognizeBlock::~COCREngineRecognizeBlock()
    {
    }

// -------------------------------------------------------------
// COCREngineRecognizeBlock::RecognizeBlockL
// Description : Recognize a specified block
// -------------------------------------------------------------
//
void COCREngineRecognizeBlock::RecognizeBlockL(const TInt /*aBitmapHandle*/,
                                               const TOCRLayoutBlockInfo /*aLayoutInfo*/)
    {
    }

// ---------------------------------------------------------------
// COCREngineRecognizeBlock::Base
// Description : Provide base functions to be called
// ---------------------------------------------------------------
//
MOCREngineBase* COCREngineRecognizeBlock::Base() const
    {
    return NULL; 
    }

// -------------------------------------------------------------
// COCREngineRecognizeBlock::RecognizeSpecialregionL
// Description : Recognize a special region
// -------------------------------------------------------------
//
void COCREngineRecognizeBlock::RecognizeSpecialRegionL(const TInt /*aBitmapHandle*/,
                                                       const TRegionInfo /*aRegionInfo*/)
    {
    }
    
// ---------------------------------------------------------------
// COCREngineLayoutRecognize::~COCREngineLayoutRecognize
// Description : Destructor
// ---------------------------------------------------------------
//
COCREngineLayoutRecognize::~COCREngineLayoutRecognize()
    {
    }

// ---------------------------------------------------------------
// COCREngineLayoutRecognize::LayoutAnalysisL
// Description : Layout analysis 
// -------------------------------------------------------------
//
void COCREngineLayoutRecognize::LayoutAnalysisL(const TInt /*aBitmapHandle*/,
                                                const TOCRLayoutSetting /*aSettings*/)
    {
    }    

// -------------------------------------------------------------
// COCREngineLayoutRecognize::RecognizeL
// Description : Recognize text
// -------------------------------------------------------------
//
void COCREngineLayoutRecognize::RecognizeL(const TOCRRecognizeSetting /*aSettings*/,
                                           const RArray<TInt>& /*aBlock*/)
    {
    }
    
// ---------------------------------------------------------------
// COCREngineLayoutRecognize::Base
// Description : Provide base functions to be called
// ---------------------------------------------------------------
//
MOCREngineBase* COCREngineLayoutRecognize::Base() const
    {
    return NULL; 
    }    

// End of file
