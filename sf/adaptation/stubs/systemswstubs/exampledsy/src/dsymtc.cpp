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
* Description: 
*    It implements the class CDosMtcExample.
*
*/


#include "DsyMtc.h"
#include "DsyDebug.h"
#include <f32file.h>

_LIT( KExampleDSYMTCStateFlagFile, "KExampleDSYMTCStateFlagFile.dat");


//
// ---------------------------------------------------------
// CDosMtcExample::NewL
// ---------------------------------------------------------
//  
CDosMtcExample* CDosMtcExample::NewL()
{
	FLOG(_L("CDosMtcExample::NewL()"));

	CDosMtcExample* result = new (ELeave) CDosMtcExample;

	CleanupStack::PushL(result);
	result->ConstructL();
	CleanupStack::Pop();

	return result;
}


//
// ---------------------------------------------------------
// CDosMtcExample::ConstructL
// ---------------------------------------------------------
//  
void CDosMtcExample::ConstructL()
{
	FLOG(_L("CDosMtcExample::ConstructL()"));
}

//
// ---------------------------------------------------------
// CDosMtcExample::PowerOnL
// ---------------------------------------------------------
//  
TInt CDosMtcExample::PowerOnL()
{
	FLOG(_L("CDosMtcExample::PowerOnL()"));

	return KErrNone;
}


//
// ---------------------------------------------------------
// CDosMtcExample::PowerOffL
// ---------------------------------------------------------
//  
TInt CDosMtcExample::PowerOffL()
{
	FLOG(_L("CDosMtcExample::PowerOffL()"));

	return KErrNone;
}


//
// ---------------------------------------------------------
// CDosMtcExample::DosSyncL
// ---------------------------------------------------------
//  
TInt CDosMtcExample::DosSyncL()
{
	FLOG(_L("CDosMtcExample::DosSyncL()"));

	return KErrNone;
}


//
// ---------------------------------------------------------
// CDosMtcExample::ResetGenerateL
// ---------------------------------------------------------
//  
TInt CDosMtcExample::ResetGenerateL()
{
	FLOG(_L("CDosMtcExample::ResetGenerateL()"));

	return KErrNone;	
}

//
// ---------------------------------------------------------
// CDosMtcExample::SetStateL
// ---------------------------------------------------------
//  
TInt CDosMtcExample::SetStateL(const TInt /*aStateType*/)
{
	FLOG(_L("CDosMtcExample::SetStateL()"));

	return KErrNone;
}

//
// ---------------------------------------------------------
// CDosMtcExample::SetStateFlagL
// ---------------------------------------------------------
//  

TInt CDosMtcExample::SetStateFlagL(const TDosStateFlag aFlag)
{
	FLOG(_L("CDosMtcExample::SetStateFlagL() - Write state to disk"));

	RFs fsSession;
	TInt err = fsSession.Connect();

	if ( err == KErrNone )
	{
		RFile file;
		err = file.Open( fsSession, KExampleDSYMTCStateFlagFile, EFileWrite);

		if ( err == KErrNotFound) // file does not exist - create it
	    {
			err = file.Create( fsSession, KExampleDSYMTCStateFlagFile, EFileWrite);
			FLOG(_L("CDosMtcExample::SetStateFlagL() - file doesn't exist, create file"));
	    }

		if ( err == KErrNone )
		{
			// write stateflag to disk
			TBuf8<1> buf;
			buf.Append((TInt8)aFlag);
			err = file.Write( buf, 1);
		}

		//close file
		file.Close();
	}

	if ( err != KErrNone) // file does not exist or read fails - aFlag is EDosOfflineStateFlagFalse and return
    {
		FLOG(_L("CDosMtcExample::SetStateFlagL() - flag value write fails"));
    }

	//close session
	fsSession.Close();

	return KErrNone;
}


//
// ---------------------------------------------------------
// CDosMtcExample::GetStateFlagL
//
// ---------------------------------------------------------
//  


TInt CDosMtcExample::GetStateFlagL(TDosStateFlag& aFlag)
{
	FLOG(_L("CDosMtcExample::GetStateFlagL() - Read state from disk"));

	// set default value
	aFlag = EDosOfflineStateFlagFalse;
	
	RFs fsSession;
	TInt err = fsSession.Connect();

	if ( err == KErrNone )
	{
		RFile file;
		err = file.Open( fsSession, KExampleDSYMTCStateFlagFile, EFileRead);

		if ( err == KErrNone )
		{
			// read stateflag status from disk
			TBuf8<1> buf(1);
			err = file.Read( buf, 1);
			if ( err == KErrNone && buf.Size() > 0)
			{
				if ( buf[0] == (TInt8)EDosOfflineStateFlagTrue )
				{
					aFlag = EDosOfflineStateFlagTrue;
				}
			}
		}
	
		//close file
		file.Close();
	}


	if ( err != KErrNone) // file does not exist or read fails - aFlag is EDosOfflineStateFlagFalse and return
    {
		FLOG(_L("CDosMtcExample::GetStateFlagL() - file read fails, default value EDosOfflineStateFlagFalse returned"));
    }


	// close session
	fsSession.Close();

	return KErrNone;
}


