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
* ne1_tb\inc\variant.h
* NE1_TBVariant Variant Header
*
*/



#ifndef __VA_STD_H__
#define __VA_STD_H__
#include "naviengine_priv.h"
#include "iolines.h"

NONSHARABLE_CLASS(NE1_TBVariant) : public NaviEngineAssp
	{
public:
	NE1_TBVariant();
public:
	/**
	 * These are the mandatory Asic class functions which need to be implemented here. The other mandatory
	 * functions are implemented in NaviEngineAssp
	 */

	/**
	 * initialisation
	 */
	virtual void Init1();
#ifdef __SMP__
	virtual void Init2AP();
#endif
	virtual void Init3();

	/**
 	 * power management
 	 * Device specific Idle: prepares the CPU to go into Idle and sets out the conditions to come out of it
 	 */
 	virtual void Idle();

	/**
	 * HAL
	 * @param aFunction A TVariantHalFunction enumerated value
	 * @param a1 Optional input/output parameter
	 * @param a2 Optional input/output parameter
	 * @return System wide error code. 
	 * @see TVariantHalFunction
	 */
	virtual TInt VariantHal(TInt aFunction, TAny* a1, TAny* a2);

	/**
	 * Machine configuration
	 * @return Pointer to a device configuration information
	 * @see TTemplateMachineConfig
	 */
	virtual TPtr8 MachineConfiguration();

public:
	/**
	 * external interrupt handling
	 * These are used to model second-level interrupt controllers at Variant level
	 * @param anId An interrupt identification number (TTemplateInterruptId enumerated value)
	 * @param an Isr Address of an Interrupt Service Routine
	 * @param aPtr Extra parameter to be passed to ISR function
	 * @return System wide error code
	 * @see TTemplateInterruptId
	 */

	virtual TInt InterruptBind(TInt anId, TIsr anIsr, TAny* aPtr);
	virtual TInt InterruptUnbind(TInt anId);
	virtual TInt InterruptEnable(TInt anId);
	virtual TInt InterruptDisable(TInt anId);
	virtual TInt InterruptClear(TInt anId);

	/**
	 * USB client controller - Some example functions for the case that USB cable detection and
	 * UDC connect/disconnect functionality are part of the Variant.
	 * These virtual functions are called by the USB PSL (pa_usbc.cpp).
	 * If this functionality is part of the ASSP then these functions can be removed as calls to them
	 * in the PSL will have been replaced by the appropriate internal operations.
	 */
	virtual TBool UsbClientConnectorDetectable();
	virtual TBool UsbClientConnectorInserted();
	virtual TInt RegisterUsbClientConnectorCallback(TInt (*aCallback)(TAny*), TAny* aPtr);
	virtual void UnregisterUsbClientConnectorCallback();
	virtual TBool UsbSoftwareConnectable();
	virtual TInt UsbConnect();
	virtual TInt UsbDisconnect();

	/**
	 * miscellaneous if Video buffer is allocated in the main System memory during ASSP/Variant initialisation
	 * this will return the required size @return Size (in bytes) of required RAM for Video buffer
	 */
	virtual TInt VideoRamSize();

	/**
	 * RAM zone callback functions that will be invoked by the kernel when a RAM zone
	 * operation should be performed.
	 */
	static TInt RamZoneCallback(TRamZoneOp aOp, TAny* aId, const TAny* aMasks);
	TInt DoRamZoneCallback(TRamZoneOp aOp, TUint aId, const TUint* aMasks);

    IMPORT_C static TUint16 SetSerialNumber( TUint32 aSerialNum );
    IMPORT_C static TUint32 GetSerialNumber( );

private: // or public:
	static void InitInterrupts();
	static void Spurious(TAny* aId);
public:
	// TLinAddr iIdleFunction;		// may be used to point to a Bootstrap routine which prepares the CPU to Sleep 
	static TUint32 HandlerData[3];
	static SInterruptHandler Handlers[ENumXInts];
	TUint32 iSerialNumber;

private:
	static void UsbClientConnectorIsr(TAny *);

private:
	TInt (*iUsbClientConnectorCallback)(TAny*);
	TAny* iUsbClientConnectorCallbackArg;
	};

GLREF_D NE1_TBVariant TheVariant;

#endif
