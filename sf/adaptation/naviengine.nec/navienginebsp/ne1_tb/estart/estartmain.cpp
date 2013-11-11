/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* estart.mmp
*
*/



#include <e32std.h>
#include <e32std_private.h>
#include "estart.h"

class TNaviEngineFSStartup : public TFSStartup
	{
public:
	virtual TPtrC LocalDriveMappingFileName();
	};
	

_LIT(KLocalDriveMappingFile,"Z:\\SYS\\DATA\\ESTART.TXT");

/** Return the filename of the drive mapping file.
@return	A non-modifiable ptr descriptor containing the path and filename of the mapping file.
*/ 
TPtrC TNaviEngineFSStartup::LocalDriveMappingFileName()
	{
	
	DEBUGPRINT("LocalDriveMappingFileName");

	InitCompositeFileSystem();

	return(KLocalDriveMappingFile());
	}	

GLDEF_C TInt E32Main()
	{
	
	TNaviEngineFSStartup fsStart;
	fsStart.Init();
	
	fsStart.Run();

	fsStart.StartSystem();
				
	fsStart.Close();
	return(0);
	}
	


		
