/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Haptics test (adaptation) plugin header file.
*
*/


#ifndef __HWRMHAPTICSSTUBPLUGIN_H_
#define __HWRMHAPTICSSTUBPLUGIN_H_

#include <hwrmhapticspluginservice.h>
#include "hwrmhapticsstubpluginidleresponder.h"

/**
 * Haptics stub plugin.
 * Stub implementation of Haptics adaptation ECom plugin for clean build(s).
 *
 * @since Series S60 5.1
 */
class CHWRMHapticsStubPlugin : public CHWRMHapticsPluginService
    {
public:
    /**
     * Static method for instantiating stub plugin objects used by
     * ECom framework.
     *
     * @aCallback Haptics plugin callback instance.
     * @return Pointer to a new CHWRMHapticsStubPlugin instance.
     */
    static CHWRMHapticsStubPlugin* NewL( 
            MHWRMHapticsPluginCallback* aCallback );
    
    /**
     * Destructor
     */
    virtual ~CHWRMHapticsStubPlugin();

    /**
     * Method to invoke a particular command in the plugin.
     * Dummy implementation of the CHWRMHapticsPluginService::ProcessCommandL 
     * abstract interface method. 
     *
     * @see CHWRMHapticsPluginService
     */
    virtual void ProcessCommandL( TInt aCommandId,
                                  TUint8 aTransId,
                                  TDesC8& aData );

    /**
     * Method to cancel a particular command.
     * Dummy implementation the CHWRMHapticsPluginService::CancelCommandL
     * abstract interface method. 
     *
     * @see CHWRMHapticsPluginService
     */
    virtual void CancelCommandL( TUint8 aTransId, 
                                 TInt aCommandId );
          

    /**
     * The actual response generation method (called from idle responder 
     * object)
     *
     * @param aTransId Transaction Id of the command for which the response 
     *                 generation is requested.
     * @param aData    Pointer to the data received in command (based on
     *                 which the command-specific response is generated).
     */
    void GenerateResponseL( TUint8 aTransId,
                            TUint8* aData );

    /**
     * Static method that CIdle calls as callback to remove "spent" responder
     * objects from their pointer array
     *
     * @param aObjectPtr Pointer to the CHWRMHapticsStubPlugin object that
     *                   started the CIdle runner whose RunL calls this
     *                   method as callback.
     * @return TInt      Always returns KErrNone. Required by 
     *                   CIdle( TCallBack ).
     */
    static TInt CollectGarbageIdle( TAny* aObjectPtr );
    
    /**
     * Method that does the actual garbage collection of spent responder
     * objects
     */
    void CollectGarbage();
     
private:

    /**
     * Constructor.
     * 
     * @aCallback Haptics plugin callback instance.
     */
    CHWRMHapticsStubPlugin( MHWRMHapticsPluginCallback* aCallback );
    
    /**
     * Two-phase construction ConstructL
     */
    void ConstructL();
    
    /**
     * Method for removing a specific responder object from internal
     * RPointerArray
     *
     * @param aTransId Transaction Id of the responder that is to be removed.
     */
    void RemoveResponder( TUint8 aTransId );
    
private: // data
    
    /**
     * Array of pointers to running idle responder objects
     */
    RPointerArray<CHWRMHapticsStubPluginIdleResponder> iResponders;

    /**
     * CIdle object through which the responders are cleaned (kind of garbage
     * collection)
     */
    CIdle *iIdle;
    };

#endif // __HWRMHAPTICSSTUBPLUGIN_H_
