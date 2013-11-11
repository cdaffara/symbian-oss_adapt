/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  CTvOutBehaviourImpl class declaration.
*
*/


#ifndef __TVOUTBEHAVIOURIMPL_H__
#define __TVOUTBEHAVIOURIMPL_H__

// INCLUDE FILES
#include <e32base.h>
#include "tvoutbehaviour.h"

// CLASS DECLARATION

/**
 * TV Out Behaviour Stub implementation.
 *
 * @lib tvoutbehaviour.lib
 * @since S60 TB9.2
 */
NONSHARABLE_CLASS(CTvOutBehaviourImpl) : public CTvOutBehaviour
    {
public:

    /**
     * Symbian two phased constructors.
     *
     * @since S60 TB9.2
     * @param None.
     * @return CTvOutBehaviourImpl
     */
    static CTvOutBehaviourImpl* NewL();

    /**
     * C++ destructor.
     */
    virtual ~CTvOutBehaviourImpl();

protected: // From CTvOutBehaviour

    virtual TInt SetTvOutSettings(const TTvOutSettings& aParams);
    virtual TInt DeActivateSettings();
    virtual TUint GetTvOutScreenDevices();
    virtual TInt GetTvOutDisplayNumber();
    virtual TInt GetTvOutSettings(TTvOutSettings& aParams);
    virtual TInt SettingsListener(TRequestStatus& aStatus);
    virtual TInt CancelSettingsListener();
    virtual TInt GetTvOutDefaultSettings(TTvOutSettings& aParams);

private:

    CTvOutBehaviourImpl();
    };

#endif //__TVOUTBEHAVIOURIMPL_H__

// End of File

