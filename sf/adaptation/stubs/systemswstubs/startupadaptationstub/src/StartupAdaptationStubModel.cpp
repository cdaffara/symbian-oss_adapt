/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  CStartupAdaptationStubModel class definition.
*
*/



// INCLUDE FILES
#include "StartupAdaptationStubModel.h"
#include "StartupAdaptationStubDebug.h"
#include <f32file.h>

// CONSTANTS

_LIT( KConfigFile, "C:\\private\\2000D766\\StartupAdaptationStubConf.txt" );
const TInt KDefaultDuration( 1 ); // Default command duration in ms.

const TInt KGlobalStateChangeIndex( 0 );
const TInt KSecurityStateChangeIndex( 1 );
const TInt KGlobalStartupModeIndex( 2 );
const TInt KSelftestIndex( 3 );
const TInt KLanguageListIndex( 4 );
const TInt KRTCTimeIndex( 5 );
const TInt KSimChangedIndex( 6 );
const TInt KSimOwnedIndex( 7 );
const TInt KSetAlarmIndex( 8 );
const TInt KCancelAlarmIndex( 9 );
const TInt KResetIndex( 10 );
const TInt KShutdownIndex( 11 );
const TInt KRFSIndex( 12 );
const TInt KActivateRfIndex( 13 );
const TInt KDeactivateRfIndex( 14 );
const TInt KHiddenResetIndex( 15 );

using namespace StartupAdaptation;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CStartupAdaptationStubModel* CStartupAdaptationStubModel::NewL()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::NewL." ) );

    CStartupAdaptationStubModel* self =
        new( ELeave ) CStartupAdaptationStubModel();

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    RDEBUG( _L( "CStartupAdaptationStubModel::NewL finished." ) );

    return self;
    }


// Destructor
CStartupAdaptationStubModel::~CStartupAdaptationStubModel()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::~CStartupAdaptationStubModel." ) );

    for ( TInt i = 0; i < KNumResponseLists; i++ )
        {
        delete iResponses[i].iList;
        }

    RDEBUG( _L( "CStartupAdaptationStubModel::~CStartupAdaptationStubModel finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetGlobalStateChangeResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetGlobalStateChangeResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetGlobalStateChangeResponse." ) );

    TInt response = GetSimpleResponse( KGlobalStateChangeIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetGlobalStateChangeResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetSecurityStateChangeResponse
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::GetSecurityStateChangeResponse(
    StartupAdaptation::TSecurityStateResponse& aResponse )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetSecurityStateChangeResponse." ) );

    const TInt index = KSecurityStateChangeIndex;
    __ASSERT_DEBUG( iResponses[index].iList != NULL, User::Invariant() );

    CStructuredList& list = *( ( iResponses[index] ).iList );
    const TInt currentIndex = CalculateCurrentBaseIndex( index );;

    __ASSERT_DEBUG( list.Count() > 0, User::Invariant() );
    __ASSERT_DEBUG( currentIndex < list.Count() - 2, User::Invariant() );

    aResponse.iErrorCode = list[currentIndex + 1 ];
    aResponse.iValue =
        static_cast<TSecurityStateInfo>( list[currentIndex + 2 ] );

    UpdateCounter( index );

    RDEBUG( _L( "CStartupAdaptationStubModel::GetSecurityStateChangeResponse finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetStartupModeResponse
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::GetStartupModeResponse(
    StartupAdaptation::TGlobalStartupModeResponse& aResponse )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetStartupModeResponse." ) );

    const TInt index = KGlobalStartupModeIndex;
    __ASSERT_DEBUG( iResponses[index].iList != NULL, User::Invariant() );

    CStructuredList& list = *( ( iResponses[index] ).iList );
    const TInt currentIndex = CalculateCurrentBaseIndex( index );;

    __ASSERT_DEBUG( list.Count() > 0, User::Invariant() );
    __ASSERT_DEBUG( currentIndex < list.Count() - 2, User::Invariant() );

    aResponse.iErrorCode = list[currentIndex + 1 ];
    aResponse.iValue =
        static_cast<TGlobalStartupMode>( list[currentIndex + 2 ] );

    UpdateCounter( index );

    RDEBUG( _L( "CStartupAdaptationStubModel::GetStartupModeResponse finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetSelftestResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetSelftestResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetSelftestResponse." ) );

    TInt response = GetSimpleResponse( KSelftestIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetSelftestResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetLanguageListResponse
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::GetLanguageListResponse(
    StartupAdaptation::RLanguageListResponse& aResponse )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetLanguageListResponse." ) );

    const TInt index = KLanguageListIndex;
    __ASSERT_DEBUG( iResponses[index].iList != NULL, User::Invariant() );

    CStructuredList& list = *( ( iResponses[index] ).iList );
    const TInt currentIndex = CalculateCurrentBaseIndex( index );;

    __ASSERT_DEBUG( list.Count() > 0, User::Invariant() );
    __ASSERT_DEBUG( currentIndex < list.Count() - iResponses[ index ].iParts,
                    User::Invariant() );

    aResponse.iErrorCode = list[ currentIndex + 1 ];
    for ( TInt i = 0; i < list[ currentIndex + 2 ]; i++ )
        {
        aResponse.iLanguages.Append(
            static_cast<TPreferredLanguage>( list[ currentIndex + i + 3 ] ) );
        }

    UpdateCounter( index );

    RDEBUG( _L( "CStartupAdaptationStubModel::GetLanguageListResponse finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetRTCTimeValidityResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetRTCTimeValidityResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetRTCTimeValidityResponse." ) );

    TInt response = GetSimpleResponse( KRTCTimeIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetRTCTimeValidityResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetSimChangedResponse
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::GetSimChangedResponse(
    StartupAdaptation::TBooleanResponse& aResponse )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetSimChangedResponse." ) );

    const TInt index = KSimChangedIndex;
    __ASSERT_DEBUG( iResponses[index].iList != NULL, User::Invariant() );

    CStructuredList& list = *( ( iResponses[index] ).iList );
    const TInt currentIndex = CalculateCurrentBaseIndex( index );;

    __ASSERT_DEBUG( list.Count() > 0, User::Invariant() );
    __ASSERT_DEBUG( currentIndex < list.Count() - 2, User::Invariant() );

    aResponse.iErrorCode = list[currentIndex + 1];
    aResponse.iValue = static_cast<TBool>( list[currentIndex + 2 ] );

    UpdateCounter( index );

    RDEBUG( _L( "CStartupAdaptationStubModel::GetSimChangedResponse finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetSimOwnedResponse
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::GetSimOwnedResponse(
    StartupAdaptation::TBooleanResponse& aResponse )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetSimOwnedResponse." ) );

    const TInt index = KSimOwnedIndex;
    __ASSERT_DEBUG( iResponses[index].iList != NULL, User::Invariant() );

    CStructuredList& list = *( ( iResponses[index] ).iList );
    const TInt currentIndex = CalculateCurrentBaseIndex( index );;

    __ASSERT_DEBUG( list.Count() > 0, User::Invariant() );
    __ASSERT_DEBUG( currentIndex < list.Count() - 2, User::Invariant() );

    aResponse.iErrorCode = list[currentIndex + 1 ];
    aResponse.iValue = static_cast<TBool>( list[currentIndex + 2 ] );

    UpdateCounter( index );

    RDEBUG( _L( "CStartupAdaptationStubModel::GetSimOwnedResponse finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetSetAlarmResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetSetAlarmResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetSetAlarmResponse." ) );

    TInt response = GetSimpleResponse( KSetAlarmIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetSetAlarmResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetCancelAlarmResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetCancelAlarmResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetCancelAlarmResponse." ) );

    TInt response = GetSimpleResponse( KCancelAlarmIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetCancelAlarmResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetResetResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetResetResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetResetResponse." ) );

    TInt response = GetSimpleResponse( KResetIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetResetResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetShutdownResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetShutdownResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetShutdownResponse." ) );

    TInt response = GetSimpleResponse( KShutdownIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetShutdownResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetRFSResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetRFSResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetRFSResponse." ) );

    TInt response = GetSimpleResponse( KRFSIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetRFSResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetActivateRfResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetActivateRfResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetActivateRfResponse." ) );

    TInt response = GetSimpleResponse( KActivateRfIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetActivateRfResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetDeactivateRfResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetDeactivateRfResponse()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetDeactivateRfResponse." ) );

    TInt response = GetSimpleResponse( KDeactivateRfIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetDeactivateRfResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetHiddenResetResponse
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::GetHiddenResetResponse(
    StartupAdaptation::TBooleanResponse& aResponse )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetHiddenResetResponse." ) );

    const TInt index = KHiddenResetIndex;
    __ASSERT_DEBUG( iResponses[index].iList != NULL, User::Invariant() );

    CStructuredList& list = *( ( iResponses[index] ).iList );
    const TInt currentIndex = CalculateCurrentBaseIndex( index );;

    __ASSERT_DEBUG( list.Count() > 0, User::Invariant() );
    __ASSERT_DEBUG( currentIndex < list.Count() - 2, User::Invariant() );

    aResponse.iErrorCode = list[currentIndex + 1 ];
    aResponse.iValue = static_cast<TBool>( list[currentIndex + 2 ] );

    UpdateCounter( index );

    RDEBUG( _L( "CStartupAdaptationStubModel::GetHiddenResetResponse finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetDurationForNextCall
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetDurationForNextCall( const TInt aCommandId )
    {
    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetDurationForNextCall( %d )" ), aCommandId );

    TInt index = aCommandId - EGlobalStateChange;
    if ( aCommandId >= EValidateRTCTime )
        {
        // Skip the missing two commands.
        index -= ( EValidateRTCTime - EGetSIMLanguages );
        }

    __ASSERT_DEBUG( iResponses[index].iList != NULL, User::Invariant() );

    CStructuredList& list = *( ( iResponses[index] ).iList );
    const TInt currentIndex = CalculateCurrentBaseIndex( index );;

    __ASSERT_DEBUG( list.Count() > 0, User::Invariant() );
    __ASSERT_DEBUG( currentIndex < list.Count(), User::Invariant() );

    const TInt duration = list[currentIndex];

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetDurationForNextCall finished with %d." ), duration );
    return duration;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::CStartupAdaptationStubModel
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CStartupAdaptationStubModel::CStartupAdaptationStubModel()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::CStartupAdaptationStubModel." ) );

    for ( TInt i = 0; i < KNumResponseLists; i++ )
        {
        iResponses[i].iCounter = 0;
        }

    iResponses[ KGlobalStateChangeIndex ].iParts = 1;
    iResponses[ KSecurityStateChangeIndex ].iParts = 2;
    iResponses[ KGlobalStartupModeIndex ].iParts = 2;
    iResponses[ KSelftestIndex ].iParts = 1;
    iResponses[ KLanguageListIndex ].iParts = 16;
    iResponses[ KRTCTimeIndex ].iParts = 1;
    iResponses[ KSimChangedIndex ].iParts = 2;
    iResponses[ KSimOwnedIndex ].iParts = 2;
    iResponses[ KSetAlarmIndex ].iParts = 1;
    iResponses[ KCancelAlarmIndex ].iParts = 1;
    iResponses[ KResetIndex ].iParts = 1;
    iResponses[ KShutdownIndex ].iParts = 1;
    iResponses[ KRFSIndex ].iParts = 1;
    iResponses[ KActivateRfIndex ].iParts = 1;
    iResponses[ KDeactivateRfIndex ].iParts = 1;
    iResponses[ KHiddenResetIndex ].iParts = 2;

    RDEBUG( _L( "CStartupAdaptationStubModel::CStartupAdaptationStubModel finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::ConstructL()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::ConstructL." ) );

    for ( TInt i = 0; i < KNumResponseLists; i++ )
        {
        iResponses[i].iList =
            new ( ELeave ) CStructuredList( iResponses[i].iParts + 1 );
        }

    TRAPD( errorCode, ReadConfigFileL() );
    if ( errorCode != KErrNone )
        {
        RDEBUG_1( _L( "Configuration file does not exist or is corrupt (error code %d). Initializing to default configuration." ), errorCode );

        InitToDefaultL();
        }

    RDEBUG( _L( "CStartupAdaptationStubModel::ConstructL finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::ReadConfigFileL
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::ReadConfigFileL()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::ReadConfigFileL." ) );

    RFs fs;
    User::LeaveIfError( fs.Connect() );
    CleanupClosePushL( fs );

    RFile file;
    User::LeaveIfError( file.Open( fs, KConfigFile, EFileShareReadersOnly ) );
    CleanupClosePushL( file );

    TFileText reader;
    reader.Set( file );

    TBuf<256> buf;
    TLex lex( buf );

    for ( TInt i = 0; i < KNumResponseLists; i++ )
        {
        User::LeaveIfError( reader.Read( buf ) );

        RDEBUG_1( _L( "CStartupAdaptationStubModel: Config line: %S." ), &buf );

        lex.Assign( buf );
        ReadStructuredListL(
            iResponses[ i ].iParts, lex, *( iResponses[ i ].iList ) );
        }

    CleanupStack::PopAndDestroy( 2 ); // file, fs

    RDEBUG( _L( "CStartupAdaptationStubModel::ReadConfigFileL finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::InitToDefaultL
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::InitToDefaultL()
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::InitToDefaultL." ) );

    for ( TInt i = 0; i < KNumResponseLists; i++ )
        {
        iResponses[i].iList->Reset();
        }

    iResponses[KGlobalStateChangeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KGlobalStateChangeIndex].iList->AppendL( KErrNone );

    iResponses[KSecurityStateChangeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KErrNone );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( EYes );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KErrNone );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( ENoValue );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KErrNone );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( ENo );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KErrNone );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( ENo );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KErrNone );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( ENo );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KErrNone );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( ENoValue );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( KErrNone );
    iResponses[KSecurityStateChangeIndex].iList->AppendL( ESimLockOk );

    iResponses[KGlobalStartupModeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KGlobalStartupModeIndex].iList->AppendL( KErrNone );
    iResponses[KGlobalStartupModeIndex].iList->AppendL( ENormal );

    iResponses[KSelftestIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSelftestIndex].iList->AppendL( KErrNone );

    iResponses[KLanguageListIndex].iList->AppendL( KDefaultDuration );
    iResponses[KLanguageListIndex].iList->AppendL( KErrNotFound );
    for ( TInt i = 0; i < 15; i++ )
        {
        iResponses[KLanguageListIndex].iList->AppendL( 0 );
        }
    iResponses[KLanguageListIndex].iList->AppendL( KDefaultDuration );
    iResponses[KLanguageListIndex].iList->AppendL( KErrNotFound );
    for ( TInt i = 0; i < 15; i++ )
        {
        iResponses[KLanguageListIndex].iList->AppendL( 0 );
        }
    iResponses[KLanguageListIndex].iList->AppendL( KDefaultDuration );
    iResponses[KLanguageListIndex].iList->AppendL( KErrNotFound );
    for ( TInt i = 0; i < 15; i++ )
        {
        iResponses[KLanguageListIndex].iList->AppendL( 0 );
        }

    iResponses[KRTCTimeIndex].iList->AppendL( KDefaultDuration );
    iResponses[KRTCTimeIndex].iList->AppendL( 0 );

    iResponses[KSimChangedIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSimChangedIndex].iList->AppendL( KErrNotFound );
    iResponses[KSimChangedIndex].iList->AppendL( EFalse );

    iResponses[KSimOwnedIndex].iList->AppendL( KDefaultDuration );
    iResponses[KSimOwnedIndex].iList->AppendL( KErrNotFound );
    iResponses[KSimOwnedIndex].iList->AppendL( EFalse );

    for ( TInt i = 8; i < KNumResponseLists - 1; i++ )
        {
        iResponses[i].iList->AppendL( KDefaultDuration );
        iResponses[i].iList->AppendL( 0 );
        }

    iResponses[KHiddenResetIndex].iList->AppendL( KDefaultDuration );
    iResponses[KHiddenResetIndex].iList->AppendL( KErrNotSupported );
    iResponses[KHiddenResetIndex].iList->AppendL( EFalse );

    RDEBUG( _L( "CStartupAdaptationStubModel::InitToDefaultL finished." ) );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::GetSimpleResponse
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::GetSimpleResponse( const TInt aIndex )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::GetSimpleResponse." ) );
    __ASSERT_DEBUG( iResponses[aIndex].iList != NULL, User::Invariant() );

    CStructuredList& list = *( ( iResponses[aIndex] ).iList );
    const TInt currentIndex = CalculateCurrentBaseIndex( aIndex );;

    RDEBUG_1( _L( "CStartupAdaptationStubModel: count = %d" ), list.Count() );

    __ASSERT_DEBUG( list.Count() > 0, User::Invariant() );
    __ASSERT_DEBUG( currentIndex < list.Count() + 1, User::Invariant() );

    TInt response = list[currentIndex + 1];

    UpdateCounter( aIndex );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::GetSimpleResponse finished with %d." ), response );
    return response;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::CalculateCurrentBaseIndex
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::CalculateCurrentBaseIndex( const TInt aIndex )
    {
    RDEBUG_1( _L( "CStartupAdaptationStubModel::CalculateCurrentBaseIndex( %d )" ), aIndex );

    const TInt counter = iResponses[aIndex].iCounter;
    const TInt parts = iResponses[aIndex].iParts;
    const TInt currentBaseIndex = counter * ( parts + 1 );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::CalculateCurrentBaseIndex finished with %d." ), currentBaseIndex );
    return currentBaseIndex;
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::UpdateCounter
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::UpdateCounter( const TInt aIndex )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::UpdateCounter." ) );
    __ASSERT_DEBUG( iResponses[aIndex].iList != NULL, User::Invariant() );

    CStructuredList& list = *( ( iResponses[aIndex] ).iList );
    TInt counter = iResponses[aIndex].iCounter;
    const TInt parts = iResponses[aIndex].iParts;

    __ASSERT_DEBUG( list.Count() > 0, User::Invariant() );
    __ASSERT_DEBUG( counter * ( parts + 1 ) < list.Count(), User::Invariant() );

    counter++;

    if ( counter == ( list.Count() / ( parts + 1 ) ) )
        {
        iResponses[aIndex].iCounter = 0;
        }
    else
        {
        iResponses[aIndex].iCounter = counter;
        }

    RDEBUG_1( _L( "CStartupAdaptationStubModel::UpdateCounter finished. New counter is %d" ), iResponses[aIndex].iCounter );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::ReadStructuredListL
//
// -----------------------------------------------------------------------------
//
void CStartupAdaptationStubModel::ReadStructuredListL(
    const TInt aNumParts,
    TLex& aLexer,
    CStructuredList& aList )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::ReadStructuredListL." ) );

    while ( !aLexer.Eos() )
        {
        aList.AppendL( ReadDurationL( aLexer ) );

        TUint val( 0 );
        User::LeaveIfError( aLexer.Val( val, EHex ) );
        aList.AppendL( val );

        for ( TInt i = 0; i < aNumParts - 1; i++ )
            {
            if ( aLexer.Get() != ',' ) User::Leave( KErrCorrupt );

            User::LeaveIfError( aLexer.Val( val, EHex ) );
            aList.AppendL( val );
            }

        if ( !aLexer.Eos() && aLexer.Get() != ';' ) User::Leave( KErrCorrupt );
        }

    if ( aList.Count() == 0 ) User::Leave( KErrCorrupt );

    RDEBUG_1( _L( "CStartupAdaptationStubModel::ReadStructuredListL finished. List length: %d" ), aList.Count() );
    }


// -----------------------------------------------------------------------------
// CStartupAdaptationStubModel::ReadDurationL
//
// -----------------------------------------------------------------------------
//
TInt CStartupAdaptationStubModel::ReadDurationL( TLex& aLexer )
    {
    RDEBUG( _L( "CStartupAdaptationStubModel::ReadDurationL." ) );

    TInt val = KDefaultDuration;
    if ( aLexer.Peek() == '[' ) // Duration value is written to file
        {
        aLexer.Inc();
        User::LeaveIfError( aLexer.Val( val ) );
        if ( aLexer.Get() != ']' )
            {
            User::Leave( KErrCorrupt );
            }
        }

    RDEBUG_1( _L( "CStartupAdaptationStubModel::ReadDurationL finished with %d." ), val );
    return val;
    }
