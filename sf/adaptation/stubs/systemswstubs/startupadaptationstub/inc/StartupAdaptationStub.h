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
* Description:  CStartupAdaptationStub class declaration.
*
*/



#ifndef __STARTUPADAPTATIONSTUB_H__
#define __STARTUPADAPTATIONSTUB_H__

//  INCLUDES
#include "CommandProcessingObserver.h"
#include "sastubeventsender.h"
#include <startupadaptation.h>

// FORWARD DECLARATIONS
class CCommandProcessingEngine;
class CStartupAdaptationStubModel;
class CSAStubEventListener;

// CLASS DECLARATION

/**
*  A stub implementation of CStartupAdaptation interface.
*
*  @lib StartupAdaptationStub.lib
*  @since Series 60 3.0
*/
class CStartupAdaptationStub :
    public CStartupAdaptation,
    public MCommandProcessingObserver,
    public MSAStubEventSender
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aConstructionParameters Construction parameters from the
        * client.
        */
        static CStartupAdaptationStub* NewL( TAny* aConstructionParameters );

        /**
        * Destructor.
        */
        virtual ~CStartupAdaptationStub();

    public: // Functions from base classes

        /**
        * From CStartupAdaptation.
        * Issue a command to startup adaptation.
        *
        * @param aCommandId A command identifier.
        * @param aData      Data associated with the command.
        */
        virtual void CommandL(
            const StartupAdaptation::TCommand aCommandId,
            TDesC8& aData );

        /**
        * From CStartupAdaptation.
        * Cancel a command.
        *
        * @param aCommandId A command identifier.
        */
        virtual void CancelCommandL(
            const StartupAdaptation::TCommand aCommandId );

        /**
        * From CStartupAdaptation.
        * Get plugin implementation version. Major and minor version numbers
        * are relevant.
        *
        * @return The plugin implementation version.
        */
        virtual TVersion Version() const;

        /**
        * From MCommandProcessingObserver.
        * A command has been processed.
        * @param aCommandId A command identifier.
        */
        virtual void CommandProcessedL(
            const TInt aCommandId );

        /**
         * From MSAStubEventSender.
         * Send an event.
         *
         * @since S60 4.0
         * @param aEvent Identifies the event to send.
         * @param aArg Optional paraneter for the event.
         */
        virtual void SendEvent( const TInt aEvent, const TInt aArg );

    private:

        /**
        * C++ default constructor.
        * @param anObserver Observer for command responses and events.
        */
        CStartupAdaptationStub( MStartupAdaptationObserver* anObserver );

        /**
        * Performs the second phase construction of a CStartupAdaptationStub
        * object.
        */
        void ConstructL();

    private:    // Data

        // Observer for command responses and events. Not owned.
        MStartupAdaptationObserver* iObserver;

        // The engine class that handles the 'processing' of requests.
        CCommandProcessingEngine* iEngine;

        // Model containg the response values for the commands.
        CStartupAdaptationStubModel* iModel;

        // Observer for indications to send events.
        CSAStubEventListener* iEventListener;
    };

// CLASS DECLARATION

#endif // __STARTUPADAPTATIONSTUB_H__

// End of File
