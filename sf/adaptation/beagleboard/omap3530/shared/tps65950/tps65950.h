// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#ifndef TPS65950_H
#define TPS65950_H

#include <e32cmn.h>
#include <nklib.h>
#include <assp/omap3530_assp/omap3530_irqmap.h>
#include <assp/omap3530_shared/tps65950_registers.h>

// Trace - 191 = 0x00000000 0x00000000 0x00000000 0x00000000 0x0000000 0x0000000 0x80000000 0x00000000 0x00000000
#define KTPS65950		191



namespace TPS65950
{

/** Structure used for issuing an asynchronous request
 * You must set iDfc to point to a TDfc object which will
 * be queued on completion
 */
struct TReq
	{
	enum TAction
		{
		ERead,
		EWrite,
		EClearSet,
		EDisableProtect,
		ERestoreProtect
		};


	TUint16	iRegister;		///< Register to be accessed
	TAction	iAction : 8;	///< type of request, read, write or clearset
	TUint8	iReadValue;		///< Returned value from a read, original value for a ClearSet, unused for write
	union
		{
		TUint8	iWriteValue;	///< Value to write into register for a write
		TUint8	iSetMask;		///< Bits to set in a ClearSet
		};
	TUint8	iClearMask;		///< Bits to clear in a ClearSet
	TDfc*	iCompletionDfc;	///< Pointer to DFC to be called on completion (for multi-phase only the last TReq causes a completion)
	TInt	iResult;		///< KErrNone on success, else error code
	TReq*	iNextPhase;		///< set to NULL if this is a single request, for a multi-phase request set this to point to next TReq
	SDblQueLink	iLink;		///< Used internally to maintain linked list of requests
	};

struct TRtcTime
	{
	TUint8	iSecond;
	TUint8	iMinute;
	TUint8	iHour;
	TUint8	iDay;
	TUint8	iMonth;
	TUint8	iYear;
	};

/** Test whether this driver has been initialized
 * Use this in code which is expected to run early during boot
 * to prevent it trying to access this driver before it is ready
 *
 * @return	ETrue if initialized, EFalse if not
 */
IMPORT_C TBool Initialized();

/** Execute a request asynchronously
 *
 * @param	aRequest	Request object to executed, must stay valid until request completes
 */
IMPORT_C void ExecAsync( TReq& aRequest );

/** Execute a write synchronously
 * @param	aRegister	Register to write to - this must be one of the 
 *					register enumerations from tps65950_register.h
 *					or the value of a register Addr property
 *
 * @param	aValue	Value to write to register
 * @return	KErrNone on success, else standard error code
 */
IMPORT_C TInt WriteSync( TUint16 aRegister, TUint8 aValue );

/** Execute a read synchronously
 * @param	aRegister	Register to write to - this must be one of the 
 *					register enumerations from tps65950_register.h
 *					or the value of a register Addr property
 *
 * @param	aValue	Value read will be written to here
 * @return	KErrNone on success, else standard error code
 */
IMPORT_C TInt ReadSync( TUint16 aRegister, TUint8& aValue );

/** Execute a bit clear/set synchronously
 * @param	aRegister	Register to write to - this must be one of the 
 *					register enumerations from tps65950_register.h
 *					or the value of a register Addr property
 *
 * @param	aClearMask	Each '1' clear the corresponding bit in the register
 * @param	aSetMask	Each '1' sets the corresponding bit in the register
 * @return	KErrNone on success, else standard error code
 */
IMPORT_C TInt ClearSetSync( TUint16 aRegister, TUint8 aClearMask, TUint8 aSetMask );

/** Disable protection of voltage control registers
 * Call RestoreProtect() to re-enable protection
 *
 * Note - calls to DisableProtect and RestoreProtect() are
 * reference-counted, so you must call RestoreProtect() the same
 * number of times you called DisableProtect(). This is to allow
 * multiple clients to disable and restore protection so that
 * protection will only be re-enabled when the last client has
 * restored it.
 */
IMPORT_C TInt DisableProtect();

/** Restore protection after a DisableProtect().
 * If other clients have called DisableProtect(), or this client
 * has other DisableProtect() calls still not balanced by a 
 * RestoreProtect() then the protection will remain disabled
 */
IMPORT_C TInt RestoreProtect();

/** Read the current RTC time */
IMPORT_C TInt GetRtcData( TRtcTime& aTime );

/** Set the RTC time */
IMPORT_C TInt SetRtcData( const TRtcTime& aTime );

enum TPanic
	{
	EBadAction,			///< illegal value in TReq::iAction
	ENoDfc,				///< iCompletionDFC is NULL
	EBadGroup			///< Group component of iRegister is invalid
	};


enum TInterruptId 
	{
	KTPS65950IrqFirst= (EIrqRangeBasePsu << KIrqRangeIndexShift),
	
	ETPS65950_IRQ_PWR_SC_DETECT = KTPS65950IrqFirst,
	ETPS65950_IRQ_PWR_MBCHG,
	ETPS65950_IRQ_PWR_PWROK_TIMEOUT,
	ETPS65950_IRQ_PWR_HOT_DIE,	
	ETPS65950_IRQ_PWR_RTC_IT,
	ETPS65950_IRQ_PWR_USB_PRES,
	ETPS65950_IRQ_PWR_CHG_PRES,
	ETPS65950_IRQ_PWR_CHG_PWRONS,
	
	ETPS65950_IRQ_MADC_USB_ISR1,
	ETPS65950_IRQ_MADC_SW2_ISR1,
	ETPS65950_IRQ_MADC_SW1_ISR1,
	ETPS65950_IRQ_MADC_RT_ISR1,
	
	ETPS65950_IRQ_GPIO_0ISR1,
	ETPS65950_IRQ_GPIO_1ISR1,	
	ETPS65950_IRQ_GPIO_2ISR1,
	ETPS65950_IRQ_GPIO_3ISR1,
	ETPS65950_IRQ_GPIO_4ISR1,
	ETPS65950_IRQ_GPIO_5ISR1,
	ETPS65950_IRQ_GPIO_6ISR1,
	ETPS65950_IRQ_GPIO_7ISR2,
	
	ETPS65950_IRQ_GPIO_8ISR2,
	ETPS65950_IRQ_GPIO_9ISR2,
	ETPS65950_IRQ_GPIO_10ISR2,
	ETPS65950_IRQ_GPIO_11ISR2,
	ETPS65950_IRQ_GPIO_12ISR2,
	ETPS65950_IRQ_GPIO_13ISR2,
	ETPS65950_IRQ_GPIO_14ISR2,
	ETPS65950_IRQ_GPIO_15ISR2,
	
	ETPS65950_IRQ_GPIO16ISR3,
	ETPS65950_IRQ_GPIO17ISR3,
	
	ETPS65950_IRQ_BCI_BATSTS_ISR1,
	ETPS65950_IRQ_BCI_TBATOR1_ISR1,
	ETPS65950_IRQ_BCI_TBATOR2_ISR1,
	ETPS65950_IRQ_BCI_ICHGEOC_ISR1,
	ETPS65950_IRQ_BCI_ICHGLOW_ISR1ASTO,
	ETPS65950_IRQ_BCI_IICHGHIGH_ISR1,
	ETPS65950_IRQ_BCI_TMOVF_ISR1,
	ETPS65950_IRQ_BCI_WOVF_ISR1,
	
	ETPS65950_IRQ_BCI_ACCHGOV_ISR1,
	ETPS65950_IRQ_BCI_VBUSOV_ISR1,
	ETPS65950_IRQ_BCI_VBATOV_ISR1,
	ETPS65950_IRQ_BCI_VBATLVL_ISR1,
	
	ETPS65950_IRQ_KEYP_ITMISR1,
	ETPS65950_IRQ_KEYP_ITTOISR1,
	ETPS65950_IRQ_KEYP_ITLKISR1,
	ETPS65950_IRQ_KEYP_ITKPISR1,
	
	ETPS65950_IRQ_USB_INTSTS_IDGND, 
	ETPS65950_IRQ_USB_INTSTS_SESSEND,
	ETPS65950_IRQ_USB_INTSTS_SESSVALID,
	ETPS65950_IRQ_USB_INTSTS_VBUSVALID,	
	ETPS65950_IRQ_USB_INTSTS_HOSTDISCONNECT,
	ETPS65950_IRQ_USB_CARKIT_CARDP,
	ETPS65950_IRQ_USB_CARKIT_CARINTDET,
	ETPS65950_IRQ_USB_CARKIT_IDFLOAT,
	ETPS65950_IRQ_USB_OTHER_INT_VB_SESS_VLD,
	ETPS65950_IRQ_USB_OTHER_INT_DM_HI,
	ETPS65950_IRQ_USB_OTHER_INT_DP_HI,
	ETPS65950_IRQ_USB_OTHER_INT_MANU,
	ETPS65950_IRQ_USB_OTHER_INT_ABNORMAL_STRESS,
	ETPS65950_IRQ_USB_ID_INT_ID_RES_FLOAT,
	ETPS65950_IRQ_USB_ID_INT_ID_RES_440K,
	ETPS65950_IRQ_USB_ID_INT_ID_RES_200K,
	ETPS65950_IRQ_USB_ID_INT_ID_RES_102K,
	ETPS65950_IRQ_USB_CARKIT_SM_1_PSM_ERROR,	
	ETPS65950_IRQ_USB_CARKIT_SM_1_PH_ACC,
	ETPS65950_IRQ_USB_CARKIT_SM_1_CHARGER,
	ETPS65950_IRQ_USB_CARKIT_SM_1_USB_HOST,
	ETPS65950_IRQ_USB_CARKIT_SM_1_USB_OTG_B,
	ETPS65950_IRQ_USB_CARKIT_SM_1_CARKIT,
	ETPS65950_IRQ_USB_CARKIT_SM_1_DISCONNECTED,
	ETPS65950_IRQ_USB_CARKIT_SM_2_STOP_PLS_MISS,
	ETPS65950_IRQ_USB_CARKIT_SM_2_STEREO_TO_MONO,
	ETPS65950_IRQ_USB_CARKIT_SM_2_PHONE_UART,
	ETPS65950_IRQ_USB_CARKIT_SM_2_PH_NO_ACK,
	
	KTPS65950IrqLast,
	};

const TInt KNumTPSInts = (KTPS65950IrqLast - KTPS65950IrqFirst);

} // namespace TPS65950


#endif //tps65950
