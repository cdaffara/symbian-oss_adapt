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
* Description:  Haptics test (adaptation) plugin idle responder header file.
*
*/


#ifndef __HWRMHAPTICSSTUBPLUGINIDLERESPONDER_H_
#define __HWRMHAPTICSSTUBPLUGINIDLERESPONDER_H_

#include <e32base.h>

class CHWRMHapticsStubPlugin;

/**
 * Haptics stub plugin idle responder.
 * Class that initiates response generation after embedded 
 * CIdle AO gets runtime.
 *
 * @since Series S60 5.1
 */
class CHWRMHapticsStubPluginIdleResponder : public CBase
    {
public:
    /**
     * Static method for instantiating stub plugin objects used by
     * ECom framework.
     *
     * @param aPlugin     Pointer to the stub plugin object that calls 
     *                    this instantiation method.
     * @param aTransId    Transaction Id of the command (that is to be
     *                    responded when the embedded CIdle object is
     *                    scheduled to run).
     * @param aDataPacket Pointer to the received command data.
     * @return            Pointer to a new instance of idle responder.
     */
    static CHWRMHapticsStubPluginIdleResponder* NewL(
        CHWRMHapticsStubPlugin* aPlugin,
        TUint8 aTransId,
        TUint8* aDataPacket );
    
    /**
     * Destructor
     */
    virtual ~CHWRMHapticsStubPluginIdleResponder();

    /**
     * Response generation method (called as callback from CIdle)
     *
     * @param aSelf TAny pointer to the object that started the CIdle
     *              whose callback method this is.
     * @return TInt Always returns KErrNone. Required by CIdle(TCallBack).
     */
    static TInt GenerateResponse( TAny* aSelf );

    /**
     * The actual response generation method.
     */
    void GenerateResponseL();

    /**
     * Getter for transId
     * 
     * @return TUint8 The transaction Id.
     */
    TUint8 TransId() const; 

    /**
     * Getter for 'active' state
     *
     * @return TBool True iff the embedded CIdle AO is currently active.
     */
    TBool Active() const; 

private:

    /**
     * Constructor
     *
     * @param aPlugin     See the above NewL method.
     * @param aTransId    See the above NewL method.
     * @param aDataPacket See the above NewL method.
     */
    CHWRMHapticsStubPluginIdleResponder(
        CHWRMHapticsStubPlugin* aPlugin,
        TUint8 aTransId,
        TUint8* aDataPacket );

    /**
     * Two-phase construction ConstructL
     */
     void ConstructL();
    
private: // data
    
    /**
     * CIdle active object (through which response generation is done)
     */
    CIdle* iIdle;
    
    /**
     * Pointer to the stub plugin object that created this idle responder
     * and through which the actual response to stub client (i.e., to 
     * HapticsPluginManager) is sent.
     */
    CHWRMHapticsStubPlugin* iPlugin;
    
    /**
     * Transaction Id (of the received command)
     */
    TUint8 iTransId;
    
    /**
     * Pointer to raw command data (not owned)
     */
    TUint8* iDataPacket;
    };

#endif // __HWRMHAPTICSSTUBPLUGINIDLERESPONDER_H_
