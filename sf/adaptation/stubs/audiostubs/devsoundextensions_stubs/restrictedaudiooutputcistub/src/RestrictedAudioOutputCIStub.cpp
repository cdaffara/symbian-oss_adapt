/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  Project definition file for project ?myapp
*
*/


#ifdef _DEBUG
#include <e32svr.h> // Needed for RDebug Prints
#endif
#include <e32def.h>
#include "RestrictedAudioOutputCIStub.h"
#include <RestrictedAudioOutputProxy.h>

// -----------------------------------------------------------------------------
// CRestrictedAudioOutput::NewL
// Static function for creating an instance of the CRestrictedAudioOutputCI object.
// -----------------------------------------------------------------------------
//

EXPORT_C CRestrictedAudioOutputCI* CRestrictedAudioOutputCI::NewL()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CRestrictedAudioOutputCI::NewL"));
#endif
	CRestrictedAudioOutputCI* self = new(ELeave) CRestrictedAudioOutputCI();
	CleanupStack::PushL(self);
	self->ConstructL(); // will call base class ConstructL
	CleanupStack::Pop(self);
	return self;
	}


CRestrictedAudioOutputCI::CRestrictedAudioOutputCI()
	{

	}

// Destructor

CRestrictedAudioOutputCI::~CRestrictedAudioOutputCI()
	{
#ifdef _DEBUG
    RDebug::Print(_L("CRestrictedAudioOutputCI::~CRestrictedAudioOutputCI"));
#endif

	}

// -----------------------------------------------------------------------------
// CRestrictedAudioOutputCI::Commit
//
// Sets Observer for DevSound.
// -----------------------------------------------------------------------------

TInt CRestrictedAudioOutputCI::Commit()
    {
#ifdef _DEBUG
    RDebug::Print(_L("CRestrictedAudioOutputCI::Commit"));
#endif
	return KErrNone;
    }


// ========================== OTHER EXPORTED FUNCTIONS =========================



// End of File
