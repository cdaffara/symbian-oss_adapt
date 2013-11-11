/*
* Copyright (c) 2004-2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  CStartupAdaptationStubModel class declaration.
*
*/


#ifndef __STARTUPADAPTATIONSTUBMODEL_H__
#define __STARTUPADAPTATIONSTUBMODEL_H__

#include <e32base.h>
#include <startupadaptationcommands.h>

const TInt KNumResponseLists = 16;

/**
*  A model containing data used by the StartupAdaptationStub.
*
*  @lib StartupAdaptationStub.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CStartupAdaptationStubModel ) : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CStartupAdaptationStubModel* NewL();

        /**
        * Destructor.
        */
        virtual ~CStartupAdaptationStubModel();

    public: // New functions

        TInt GetGlobalStateChangeResponse();
        void GetSecurityStateChangeResponse(
            StartupAdaptation::TSecurityStateResponse& aResponse );
        void GetStartupModeResponse(
            StartupAdaptation::TGlobalStartupModeResponse& aResponse );
        TInt GetSelftestResponse();
        void GetLanguageListResponse(
            StartupAdaptation::RLanguageListResponse& aResponse );
        TInt GetRTCTimeValidityResponse();
        void GetSimChangedResponse(
            StartupAdaptation::TBooleanResponse& aResponse );
        void GetSimOwnedResponse(
            StartupAdaptation::TBooleanResponse& aResponse );
        TInt GetSetAlarmResponse();
        TInt GetCancelAlarmResponse();
        TInt GetResetResponse();
        TInt GetShutdownResponse();
        TInt GetRFSResponse();
        TInt GetActivateRfResponse();
        TInt GetDeactivateRfResponse();
        void GetHiddenResetResponse(
            StartupAdaptation::TBooleanResponse& aResponse );

        TInt GetDurationForNextCall( const TInt aCommandId );

    private:

        typedef CArrayFixFlat<TInt> CStructuredList;

        /**
        * C++ default constructor.
        */
        CStartupAdaptationStubModel();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Read settings from config file.
        */
        void ReadConfigFileL();

        /**
        * Initialize to default values (if config file does not exist or is
        * corrupted).
        */
        void InitToDefaultL();

        TInt GetSimpleResponse( const TInt aIndex );
        TInt CalculateCurrentBaseIndex( const TInt aIndex );
        void UpdateCounter( const TInt aIndex );

        static void ReadStructuredListL(
            const TInt aNumParts,
            TLex& aLexer,
            CStructuredList& aList );

        static TInt ReadDurationL( TLex& aLexer );

    private:    // Data

        struct TResponseData
            {
            TInt iParts;
            TInt iCounter;
            CStructuredList* iList; // Not owned
            };

        TResponseData iResponses[KNumResponseLists];

    };

// CLASS DECLARATION

#endif // __STARTUPADAPTATIONSTUBMODEL_H__

// End of File
