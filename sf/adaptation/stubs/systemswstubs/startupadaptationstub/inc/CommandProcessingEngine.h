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
* Description:  CCommandProcessingEngine class declaration.
*
*/



#ifndef __COMMANDPROCESSINGENGINE_H__
#define __COMMANDPROCESSINGENGINE_H__

//  INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class MCommandProcessingObserver;

// CLASS DECLARATION

/**
*  ?one_line_short_description. CCommandProcessingEngine
*  ?other_description_lines
*
*  @lib ?library
*  @since Series ?XX ?SeriesXX_version
*/
class CCommandProcessingEngine : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aObserver Command processing completion observer.
        */
        static CCommandProcessingEngine* NewL(
            MCommandProcessingObserver& aObserver );

        /**
        * Destructor.
        */
        virtual ~CCommandProcessingEngine();

    public: // New functions

        /**
        * Append command to queue.
        *
        * @param aCommandId Command ID from the client.
        * @param aDuration Timeout value in milliseconds.
        */
        void ExecuteCommandL( const TInt aCommandId, const TInt aDuration );

    public: // Functions from base classes

        /**
        * From CActive.
        * Handles an active object’s request completion event.
        */
        void RunL();

        /**
        * From CActive.
        * Handles a leave occurring in the request completion event handler
        * RunL().
        * @param aError The leave code.
        */
        TInt RunError( TInt aError );

        /**
        * From CActive.
        * Implements cancellation of an outstanding request.
        */
        void DoCancel();

    private:

        /**
        * C++ default constructor.
        * @param aObserver Command processing completion observer.
        */
        CCommandProcessingEngine( MCommandProcessingObserver& aObserver );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Set the timer and activate.
        *
        * @param aDuration Timeout value in milliseconds.
        */
        void ActivateWithTimeout( const TInt aDuration );

    private:    // Data

        struct TCommandInfo
            {
            TInt iCommandId;
            TInt iDuration;
            };

        typedef RArray<TCommandInfo> RCommandArray;

        RCommandArray iCommands; // List of commands to process.

        RTimer iTimer; // A timer for emulating command processing time.

        // Command processing completion observer.
        MCommandProcessingObserver& iObserver;
    };

#endif // __COMMANDPROCESSINGENGINE_H__

// End of File
