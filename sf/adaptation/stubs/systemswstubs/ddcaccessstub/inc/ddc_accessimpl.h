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
 * Description:
 * CDdcPortAccessImpl class declaration.
 *
 */

#ifndef DDCACCESSIMPL_H
#define DDCACCESSIMPL_H

// SYSTEM INCLUDE FILES
#include <ddc_access.h>
#include <f32file.h>

// USER INCLUDE FILES

// FORWARD DECLARATIONS

// DATA TYPES

// CLASS DECLARATION

/**
 * DDC Access Stub implementation.
 *
 * @lib ddc_access.lib
 * @since S60 TB9.2
 */
NONSHARABLE_CLASS( CDdcPortAccessImpl ): public CDdcPortAccess
    {
public:

    /**
     * Symbian two phased constructors.
     *
     * @since S60 TB9.2
     * @param None.
     * @return CDDCAccessImpl
     */
    static CDdcPortAccessImpl* NewL();
    static CDdcPortAccessImpl* NewLC();

    /**
     * C++ destructor.
     */
    virtual ~CDdcPortAccessImpl();

protected: // From base classes

    // @see CDdcPortAccess
    virtual TInt Read( TDdcPort aDdcPort,
        TUint aBlockNumber,
        TDataBlock& aDataBlock,
        TRequestStatus& aCompletedWhenRead );

    // @see CDdcPortAccess
    virtual void CancelAll();

    // @see CDdcPortAccess
    virtual void ConstructL();
    
private:

    CDdcPortAccessImpl();
    };

#endif // DDCACCESSIMPL_H
