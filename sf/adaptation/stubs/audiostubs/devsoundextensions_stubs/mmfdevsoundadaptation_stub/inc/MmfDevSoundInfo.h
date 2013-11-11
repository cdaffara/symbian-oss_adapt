/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -
*
*/


#ifndef MMF_DEVSOUNDINFO_H
#define MMF_DEVSOUNDINFO_H

#include <e32base.h>
#include <MmfBase.h>


/**
*  Class representing DevSound information.
*
*  @lib MmfDevSoundAdaptation_Stub
*  @since S60 3.0
*/
class TMMFDevSoundInfo
    {
public:
    TInt iDevSoundId;
    // More to be added for OMAP server
    };

typedef TPckgBuf<TMMFDevSoundInfo> TMMFDevSoundInfoPckg;

#endif

// End of file
