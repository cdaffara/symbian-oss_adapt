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
// \omap3530\omap3530_assp\shared\tps65950\tps65950.cpp
// Access driver for TPS65950
// This file is part of the Beagle Base port
//

#include <e32cmn.h>
#include <kernel.h>
#include <nk_priv.h>
//#include <e32atomics.h>
#include <omap3530_i2c.h>
#include <omap3530_i2creg.h>
#include "tps65950.h"


GLREF_C TInt InitInterrupts();


const TUint KGroupCount = 5;

// One handle per group on TPS65950
static I2c::THandle	I2cHandle[ KGroupCount ];

// One DCB per group
static I2c::TConfigPb TheDcb[ KGroupCount ];

// I2C transfer object
enum TPhase
	{
	EAddressPb,
	EDataPb
	};
static I2c::TTransferPb TheTransferPb[2];

// Group index to Group number
static const TUint8 KGroupIndexToGroupNumber[ KGroupCount ] =
	{ 0x12, 0x48, 0x49, 0x4a, 0x4b };

// Queue of requests
static SDblQue TheQueue;

// Current state
enum TState
	{
	EIdle,
	EPending,
	EReading,
	EWriting,
	EUnprotectPhase0
	};
static TState CurrentState;
TPS65950::TReq* CurrentPhaseReq;
TPS65950::TReq* PreviousPhaseReq;

LOCAL_D TUint8 IsInitialized;	// auto-cleared to EFalse

const TUint8	KUnprotectPhase0Data	= 0xCE;
const TUint8	KUnprotectPhase1Data	= 0xEC;

static const TUint8 KUnprotectData[4] =
	{
	TPS65950::Register::PROTECT_KEY bitand TPS65950::Register::KRegisterMask, KUnprotectPhase0Data,
	TPS65950::Register::PROTECT_KEY bitand TPS65950::Register::KRegisterMask, KUnprotectPhase1Data,
	};

static const TUint8 KProtectData[2] =
	{
	TPS65950::Register::PROTECT_KEY bitand TPS65950::Register::KRegisterMask, 0
	};

static TUint8 TempWriteBuf[2];

// Spinlock to protect queue when adding or removing items
//static TSpinLock QueueLock(TSpinLock::EOrderGenericIrqLow1+1);
//static TSpinLock QueueLock();

GLDEF_D TDfcQue*	TheDfcQue;

const TInt KDfcQuePriority	= 27;
_LIT( KDriverNameDes, "tps65950" );

TInt TheProtectionUsageCount = 0;



LOCAL_C void InternalPanic( TInt aLine )
	{
	Kern::Fault( "tps65950", aLine );
	}
#ifdef _DEBUG
LOCAL_C void PanicClient( TPS65950::TPanic aPanic )
	{
	Kern::PanicCurrentThread( KDriverNameDes, aPanic );
	}
#endif
namespace TPS65950
{
void CompletionDfcFunction( TAny* aParam );
void SyncDfcFunction( TAny* aParam );
void DummyDfcFunction( TAny* aParam );
static TDfc CompletionDfc( CompletionDfcFunction, NULL, 1 );
static TDfc DummyDfc( CompletionDfcFunction, NULL, 1 );

FORCE_INLINE TReq& ReqFromLink( SDblQueLink* aLink )
	{
	return *_LOFF( aLink, TReq, iLink );
	}

inline TBool AtomicSetPendingWasIdle()
	{
	// atomic if (CurrentState == idleState) {CurrentState=EPending, returns TRUE} else {idleState=CurrentState, CurrentState unchanged, return FALSE}
	TState idleState = EIdle;	// required for atomic comparison
	//TBool wasIdle = __e32_atomic_cas_ord32( (TUint32*)&CurrentState, (TUint32*)&idleState, EPending );
	//return wasIdle;
	
	if (CurrentState == idleState)
		{
		CurrentState = EPending;
		return ETrue;
		}
	else
		{
		idleState = CurrentState;
		return EFalse;
		}
	
	}


void StartRead( TReq& aReq )
	{
	__KTRACE_OPT( KTPS65950, Kern::Printf( "+TPS65950:StartRead(@%x) [%x:%x]",
												&aReq,
												KGroupIndexToGroupNumber[ aReq.iRegister >> Register::KGroupShift ],
												aReq.iRegister bitand Register::KRegisterMask ) );

	//__e32_atomic_store_ord32( &CurrentState, EReading );
	CurrentState = EReading;
	
	TheTransferPb[ EAddressPb ].iData = (TUint8*)&aReq.iRegister;	// low byte is register address
	TheTransferPb[ EDataPb ].iType = I2c::TTransferPb::ERead;
	TheTransferPb[ EDataPb ].iData = &aReq.iReadValue;
	__DEBUG_ONLY( aReq.iReadValue = 0xEE );
	TheTransferPb[ EDataPb ].iLength = 1;
	TheTransferPb[ EAddressPb ].iResult = KErrNone;
	TheTransferPb[ EDataPb ].iResult = KErrNone;

	TUint groupIndex = aReq.iRegister >> Register::KGroupShift;
	I2c::TransferA( I2cHandle[ groupIndex ], TheTransferPb[ EAddressPb ] );

	__KTRACE_OPT( KTPS65950, Kern::Printf( "-TPS65950:StartRead(@%x)", &aReq ) );
	}

void StartWrite( TReq& aReq )
	{
	__KTRACE_OPT( KTPS65950, Kern::Printf( "+TPS65950:StartWrite(@%x) [%x:%x]<-%x",
												&aReq,
												KGroupIndexToGroupNumber[ aReq.iRegister >> Register::KGroupShift ],
												aReq.iRegister bitand Register::KRegisterMask,
												aReq.iWriteValue ) );

	//__e32_atomic_store_ord32( &CurrentState, EWriting );
	CurrentState = EWriting;
	
	TempWriteBuf[0] = aReq.iRegister bitand Register::KRegisterMask;
	TempWriteBuf[1] = aReq.iWriteValue;
	TheTransferPb[ EDataPb ].iType = I2c::TTransferPb::EWrite;
	TheTransferPb[ EDataPb ].iData = &TempWriteBuf[0];
	TheTransferPb[ EDataPb ].iLength = 2;
	TheTransferPb[ EDataPb ].iResult = KErrNone;

	TUint groupIndex = aReq.iRegister >> Register::KGroupShift;
	I2c::TransferA( I2cHandle[ groupIndex ], TheTransferPb[ EDataPb ] );

	__KTRACE_OPT( KTPS65950, Kern::Printf( "-TPS65950:StartWrite(@%x)", &aReq ) );
	}

void StartUnprotectPhase0( TReq& aReq )
	{
	__KTRACE_OPT( KTPS65950, Kern::Printf( "+TPS65950:StartUnprotectPhase0(@%x)", &aReq ) );

	//__e32_atomic_store_ord32( &CurrentState, EUnprotectPhase0 );
	CurrentState = EUnprotectPhase0;
	
	TheTransferPb[ EDataPb ].iType = I2c::TTransferPb::EWrite;
	TheTransferPb[ EDataPb ].iData = &KUnprotectData[0];
	TheTransferPb[ EDataPb ].iLength = 2;
	TheTransferPb[ EDataPb ].iResult = KErrNone;

	const TUint groupIndex = Register::PROTECT_KEY >> Register::KGroupShift;
	I2c::TransferA( I2cHandle[ groupIndex ], TheTransferPb[ EDataPb ] );

	__KTRACE_OPT( KTPS65950, Kern::Printf( "-TPS65950:StartUnprotectPhase0(@%x)", &aReq ) );
	}

void StartUnprotectPhase1( TReq& aReq )
	{
	__KTRACE_OPT( KTPS65950, Kern::Printf( "+TPS65950:StartUnprotectPhase1(@%x)", &aReq ) );

	// Set state to writing so that it will complete as a normal write
	//__e32_atomic_store_ord32( &CurrentState, EWriting );
	CurrentState = EWriting;
	
	TheTransferPb[ EDataPb ].iData = &KUnprotectData[2];
	TheTransferPb[ EDataPb ].iResult = KErrNone;

	const TUint groupIndex = Register::PROTECT_KEY >> Register::KGroupShift;
	I2c::TransferA( I2cHandle[ groupIndex ], TheTransferPb[ EDataPb ] );

	__KTRACE_OPT( KTPS65950, Kern::Printf( "-TPS65950:StartUnprotectPhase1(@%x)", &aReq ) );
	}

void StartProtect( TReq& aReq )
	{
	__KTRACE_OPT( KTPS65950, Kern::Printf( "+TPS65950:StartProtect(@%x)", &aReq ) );

	//__e32_atomic_store_ord32( &CurrentState, EWriting );
	CurrentState = EWriting;
	
	TheTransferPb[ EDataPb ].iType = I2c::TTransferPb::EWrite;
	TheTransferPb[ EDataPb ].iData = &KProtectData[0];
	TheTransferPb[ EDataPb ].iLength = 2;
	TheTransferPb[ EDataPb ].iResult = KErrNone;

	const TUint groupIndex = Register::PROTECT_KEY >> Register::KGroupShift;
	I2c::TransferA( I2cHandle[ groupIndex ], TheTransferPb[ EDataPb ] );

	__KTRACE_OPT( KTPS65950, Kern::Printf( "-TPS65950:StartWrite(@%x)", &aReq ) );
	}


void StartRequest()
	{
	__KTRACE_OPT( KTPS65950, Kern::Printf( "+TPS65950:StartRequest(%d)", CurrentState ) );
	__ASSERT_DEBUG( EPending == CurrentState, InternalPanic( __LINE__ ) );

	// We don't need to take lock here because we're currently idle so it's not possible
	// for there to be a change in which item is queue head. The link pointers of the
	// head item could change if another thread is queueing a new request, but that doesn't
	// affect any of the fields we care about here
	__ASSERT_DEBUG( !TheQueue.IsEmpty(), InternalPanic( __LINE__ ) );
	
	if( !CurrentPhaseReq )
		{
		PreviousPhaseReq = NULL;
		CurrentPhaseReq = &ReqFromLink( TheQueue.First() );
		}

	FOREVER
		{
		if( !CurrentPhaseReq )
			{
			__ASSERT_DEBUG( PreviousPhaseReq, InternalPanic( __LINE__ ) );

			// we didn't find any phases to execute, so complete request
			// by faking a write completion on the previous phase
			CurrentPhaseReq = PreviousPhaseReq;
			//__e32_atomic_store_ord32( &CurrentState, EWriting );
			CurrentState = EWriting;
			
			// Queue DFC instead of calling directly to avoid recursion if multiple items on queue
			// complete without any action
			CompletionDfc.Enque();
			break;
			}
		else
			{
			TReq::TAction action = CurrentPhaseReq->iAction;

			__KTRACE_OPT( KTPS65950, Kern::Printf( "=TPS65950:StartRequest:req@%x:a=%x", CurrentPhaseReq, action ) );

			if( (TReq::ERead == action) || (TReq::EClearSet == action) )
				{
				StartRead( *CurrentPhaseReq );
				break;
				}
			else if( TReq::EWrite == action )
				{
				StartWrite( *CurrentPhaseReq );
				break;
				}
			else if( TReq::EDisableProtect == action )
				{
				if( ++TheProtectionUsageCount == 1 )
					{
					// Currently protected, start an unprotect sequence
					StartUnprotectPhase0( *CurrentPhaseReq );
					break;
					}
				else 
					{
					goto move_to_next_phase;
					}
				}
			else if( TReq::ERestoreProtect == action )
				{
				if( --TheProtectionUsageCount == 0 )
					{
					StartProtect( *CurrentPhaseReq );
					break;
					}
				else 
					{
	move_to_next_phase:
					// already unprotected, skip to next phase
					CurrentPhaseReq->iResult = KErrNone;
					PreviousPhaseReq = CurrentPhaseReq;
					CurrentPhaseReq = CurrentPhaseReq->iNextPhase;
					}
				}
			else
				{
				InternalPanic( __LINE__ );
				}
			}
		}


	__KTRACE_OPT( KTPS65950, Kern::Printf( "-TPS65950:StartRequest(%d)", CurrentState ) );
	}

void DummyDfcFunction( TAny* /*aParam*/ )
	{
	__KTRACE_OPT( KTPS65950, Kern::Printf( "TPS65950:DummyDFC(%d)", CurrentState ) );
	}

void CompletionDfcFunction( TAny* /*aParam*/ )
	{
	__KTRACE_OPT( KTPS65950, Kern::Printf( "+TPS65950:DFC(%d)", CurrentState ) );
	__ASSERT_DEBUG( EIdle != CurrentState, InternalPanic( __LINE__ ) );
	__ASSERT_DEBUG( CurrentPhaseReq, InternalPanic( __LINE__ ) );

	TInt result = TheTransferPb[ EDataPb ].iResult;
	if( KErrNone != TheTransferPb[ EAddressPb ].iResult )
		{
		result = TheTransferPb[ EAddressPb ].iResult;
		}

	TReq& req = *CurrentPhaseReq;
	TBool completed = ETrue;

	if( KErrNone == result)
		{
		if( EReading == CurrentState )
			{
			__KTRACE_OPT( KTPS65950, Kern::Printf( "=TPS65950:DFC:Read [%x:%x]=%x",
													KGroupIndexToGroupNumber[ req.iRegister >> Register::KGroupShift ],
													req.iRegister bitand Register::KRegisterMask,
													req.iReadValue ) );

			if( TReq::EClearSet == req.iAction)
				{
				// Start write phase of a ClearSet
				req.iWriteValue = (req.iReadValue bitand ~req.iClearMask) bitor req.iSetMask;
				StartWrite( req );
				completed = EFalse;
				}
			}
		else if( EUnprotectPhase0 == CurrentState )
			{
			StartUnprotectPhase1( req );
			completed = EFalse;
			}
		}

	if( completed || (KErrNone != result) )
		{
		// Read or write, protect has completed, or final write stage of a ClearSet or unprotect, or error
		PreviousPhaseReq = CurrentPhaseReq;
		CurrentPhaseReq = req.iNextPhase;

		if( CurrentPhaseReq )
			{
			// start next phase
			//__e32_atomic_store_ord32( &CurrentState, EPending );
			CurrentState = EPending;
			StartRequest();
			}
		else
			{
			//__e32_atomic_store_ord32( &CurrentState, EIdle );
			CurrentState = EIdle;
			// From now a concurrent ExecAsync() can start a new request if it adds an item to the queue

			// remove item from queue and complete
			TUint irq = __SPIN_LOCK_IRQSAVE( QueueLock );
			ReqFromLink( TheQueue.First() ).iLink.Deque();
			TBool queueEmpty = TheQueue.IsEmpty();
			__SPIN_UNLOCK_IRQRESTORE( QueueLock, irq );

			// If queue was empty inside spinlock but an ExecAsync() adds an item before the if statement below,
			// the ExecAsync() will start the new request
			if( !queueEmpty )
				{
				if( AtomicSetPendingWasIdle() )
					{
					// ExecAsync didn't start a request
					StartRequest();
					}
				}

			// Notify client of completion
			req.iResult = result;
			if( req.iCompletionDfc )
				{
				req.iCompletionDfc->Enque();
				}
			}
		}

	__KTRACE_OPT( KTPS65950, Kern::Printf( "-TPS65950:DFC(%d)", CurrentState ) );
	}

// Used to complete synchronous operations
void SyncDfcFunction( TAny* aParam )
	{
	NKern::FSSignal( reinterpret_cast<NFastSemaphore*>( aParam ) );
	}


EXPORT_C void ExecAsync( TReq& aRequest )
	{
	__KTRACE_OPT( KTPS65950, Kern::Printf( "+TPS65950:ExecAsync(@%x)", &aRequest ) );

	__ASSERT_DEBUG( (TUint)aRequest.iAction <= TReq::ERestoreProtect, PanicClient( EBadAction ) );
	__ASSERT_DEBUG( (TReq::EDisableProtect == aRequest.iAction) 
					|| (TReq::ERestoreProtect == aRequest.iAction)
					|| (((TUint)aRequest.iRegister >> Register::KGroupShift) < KGroupCount), PanicClient( EBadGroup ) );

	TUint irq = __SPIN_LOCK_IRQSAVE( QueueLock );
	TheQueue.Add( &aRequest.iLink );
	__SPIN_UNLOCK_IRQRESTORE( QueueLock, irq );

	if( AtomicSetPendingWasIdle() )
		{
		StartRequest();
		}

	__KTRACE_OPT( KTPS65950, Kern::Printf( "-TPS65950:ExecAsync" ) );
	}

EXPORT_C TInt WriteSync( TUint16 aRegister, TUint8 aValue )
	{
	__ASSERT_NO_FAST_MUTEX;

	NFastSemaphore sem;
	TDfc dfc( SyncDfcFunction, &sem, TheDfcQue, 2 );
	TReq req;
	req.iRegister = aRegister;
	req.iAction = TReq::EWrite;
	req.iCompletionDfc = &dfc;
	req.iWriteValue = aValue;
	req.iNextPhase = NULL;

	NKern::FSSetOwner( &sem, NULL );
	ExecAsync( req );
	NKern::FSWait( &sem );

	return req.iResult;
	}

EXPORT_C TInt ReadSync( TUint16 aRegister, TUint8& aValue )
	{
	__ASSERT_NO_FAST_MUTEX;

	NFastSemaphore sem;
	TDfc dfc( SyncDfcFunction, &sem, TheDfcQue, 2 );
	TReq req;
	req.iRegister = aRegister;
	req.iAction = TReq::ERead;
	req.iCompletionDfc = &dfc;
	req.iNextPhase = NULL;

	NKern::FSSetOwner( &sem, NULL );
	ExecAsync( req );
	NKern::FSWait( &sem );

	aValue = req.iReadValue;
	return req.iResult;
	}

EXPORT_C TInt ClearSetSync( TUint16 aRegister, TUint8 aClearMask, TUint8 aSetMask )
	{
	__ASSERT_NO_FAST_MUTEX;

	NFastSemaphore sem;
	TDfc dfc( SyncDfcFunction, &sem, TheDfcQue, 2 );
	TReq req;
	req.iRegister = aRegister;
	req.iAction = TReq::EClearSet;
	req.iCompletionDfc = &dfc;
	req.iClearMask = aClearMask;
	req.iSetMask = aSetMask;
	req.iNextPhase = NULL;

	NKern::FSSetOwner( &sem, NULL );
	ExecAsync( req );
	NKern::FSWait( &sem );

	return req.iResult;
	}

EXPORT_C TInt DisableProtect()
	{
	__ASSERT_NO_FAST_MUTEX;

	NFastSemaphore sem;
	TDfc dfc( SyncDfcFunction, &sem, TheDfcQue, 2 );
	TReq req;
	req.iAction = TReq::EDisableProtect;
	req.iCompletionDfc = &dfc;
	req.iNextPhase = NULL;

	NKern::FSSetOwner( &sem, NULL );
	ExecAsync( req );
	NKern::FSWait( &sem );

	return req.iResult;
	}

EXPORT_C TInt RestoreProtect()
	{
	__ASSERT_NO_FAST_MUTEX;

	NFastSemaphore sem;
	TDfc dfc( SyncDfcFunction, &sem, TheDfcQue, 2 );
	TReq req;
	req.iAction = TReq::ERestoreProtect;
	req.iCompletionDfc = &dfc;
	req.iNextPhase = NULL;

	NKern::FSSetOwner( &sem, NULL );
	ExecAsync( req );
	NKern::FSWait( &sem );

	return req.iResult;
	}


TInt Init()
	{
	// Create DFC queue
	TInt r = Kern::DfcQCreate( TheDfcQue, KDfcQuePriority, &KDriverNameDes );
	if( KErrNone != r )
		{
		return r;
		}

	TPS65950::CompletionDfc.SetDfcQ( TheDfcQue );
	TPS65950::DummyDfc.SetDfcQ( TheDfcQue );

	// Open I2c handles
	for( TInt i = 0; i < KGroupCount; ++i )
		{
		TheDcb[i].iUnit = I2c::E1; // Master / slave
		TheDcb[i].iRole = I2c::EMaster;
		TheDcb[i].iMode = I2c::E7Bit;
		TheDcb[i].iExclusiveClient = NULL;
		TheDcb[i].iRate = I2c::E400K;
		TheDcb[i].iOwnAddress = 0x01;
		TheDcb[i].iDfcQueue = TheDfcQue;
		TheDcb[i].iDeviceAddress = KGroupIndexToGroupNumber[i];

		I2cHandle[i] = I2c::Open( TheDcb[i] );
		if( I2cHandle[i] < 0 )
			{
			return I2cHandle[i];
			}
		}

	// Setup transfer linked list
	TheTransferPb[ EAddressPb ].iType = I2c::TTransferPb::EWrite;	// address write
	TheTransferPb[ EAddressPb ].iLength = 1;
	TheTransferPb[ EAddressPb ].iCompletionDfc = &TPS65950::DummyDfc;
	TheTransferPb[ EAddressPb ].iNextPhase = &TheTransferPb[ EDataPb ];
	TheTransferPb[ EDataPb ].iCompletionDfc = &TPS65950::CompletionDfc;
	TheTransferPb[ EDataPb ].iNextPhase = NULL;

	return r;
	}

inline TInt BcdToDecimal( TUint8 aBcd )
	{
	return ( aBcd bitand 0xF ) + ( (aBcd >> 4) * 10);
	}

inline TUint8 DecimalToBcd( TInt aDecimal )
	{
	TUint tens = (aDecimal / 10);
	return ( tens << 4 ) + ( aDecimal - tens );
	}

EXPORT_C TInt GetRtcData( TRtcTime& aTime )
	{
	__ASSERT_NO_FAST_MUTEX;

	NFastSemaphore sem;
	TDfc dfc( SyncDfcFunction, &sem, TheDfcQue, 2 );
	TReq req[8];	// 9 stages to the operation
	req[0].iRegister = RTC_CTRL_REG::Addr;
	req[0].iAction = TReq::EClearSet;
	req[0].iSetMask = RTC_CTRL_REG::GET_TIME;
	req[0].iClearMask = 0;
	req[0].iCompletionDfc = NULL;
	req[0].iNextPhase = &req[1];

	req[1].iRegister = Register::SECONDS_REG;
	req[1].iAction = TReq::ERead;
	req[1].iCompletionDfc = NULL;
	req[1].iNextPhase = &req[2];

	req[2].iRegister = Register::MINUTES_REG;
	req[2].iAction = TReq::ERead;
	req[2].iCompletionDfc = NULL;
	req[2].iNextPhase = &req[3];
	
	req[3].iRegister = Register::HOURS_REG;
	req[3].iAction = TReq::ERead;
	req[3].iCompletionDfc = NULL;
	req[3].iNextPhase = &req[4];

	req[4].iRegister = Register::DAYS_REG;
	req[4].iAction = TReq::ERead;
	req[4].iCompletionDfc = NULL;
	req[4].iNextPhase = &req[5];

	req[5].iRegister = Register::MONTHS_REG;
	req[5].iAction = TReq::ERead;
	req[5].iCompletionDfc = NULL;
	req[5].iNextPhase = &req[6];

	req[6].iRegister = Register::YEARS_REG;
	req[6].iAction = TReq::ERead;
	req[6].iCompletionDfc = NULL;
	req[6].iNextPhase = &req[7];

	req[7].iRegister = RTC_CTRL_REG::Addr;
	req[7].iAction = TReq::EClearSet;
	req[7].iSetMask = 0;
	req[7].iClearMask = RTC_CTRL_REG::GET_TIME;
	req[7].iCompletionDfc = &dfc;
	req[7].iNextPhase = NULL;

	NKern::FSSetOwner( &sem, NULL );
	ExecAsync( req[0] );
	NKern::FSWait( &sem );

	aTime.iSecond = BcdToDecimal( req[1].iReadValue );
	aTime.iMinute = BcdToDecimal( req[2].iReadValue );
	aTime.iHour = BcdToDecimal( req[3].iReadValue );
	aTime.iDay = BcdToDecimal( req[4].iReadValue );
	aTime.iMonth = BcdToDecimal( req[5].iReadValue );
	aTime.iYear = BcdToDecimal( req[6].iReadValue );

	return KErrNone;
	}

#define BCD0(a) ((a)%10)
#define BCD1(a) (((a)/10)<<4)
#define TOBCD(i) (BCD1(i)|BCD0(i))

EXPORT_C TInt SetRtcData( const TRtcTime& aTime )
	{	
	__ASSERT_NO_FAST_MUTEX;

	NFastSemaphore sem;
	TDfc dfc( SyncDfcFunction, &sem, TheDfcQue, 2 );
	TReq req[8];	// 9 stages to the operation
	req[0].iRegister = RTC_CTRL_REG::Addr;
	req[0].iAction = TReq::EClearSet;
	req[0].iSetMask = 0;
	req[0].iClearMask = RTC_CTRL_REG::STOP_RTC;
	req[0].iCompletionDfc = NULL;
	req[0].iNextPhase = &req[1];

	req[1].iRegister = Register::SECONDS_REG;
	req[1].iAction = TReq::EWrite;
	req[1].iWriteValue = DecimalToBcd( aTime.iSecond );
	req[1].iCompletionDfc = NULL;
	req[1].iNextPhase = &req[2];

	req[2].iRegister = Register::MINUTES_REG;
	req[2].iAction = TReq::EWrite;
	req[2].iWriteValue = DecimalToBcd( aTime.iMinute );
	req[2].iCompletionDfc = NULL;
	req[2].iNextPhase = &req[3];
	
	req[3].iRegister = Register::HOURS_REG;
	req[3].iAction = TReq::EWrite;
	req[3].iWriteValue = DecimalToBcd( aTime.iHour );
	req[3].iCompletionDfc = NULL;
	req[3].iNextPhase = &req[4];

	req[4].iRegister = Register::DAYS_REG;
	req[4].iAction = TReq::EWrite;
	req[4].iWriteValue = DecimalToBcd( aTime.iDay );
	req[4].iCompletionDfc = NULL;
	req[4].iNextPhase = &req[5];

	req[5].iRegister = Register::MONTHS_REG;
	req[5].iAction = TReq::EWrite;
	req[5].iWriteValue = DecimalToBcd( aTime.iMonth );
	req[5].iCompletionDfc = NULL;
	req[5].iNextPhase = &req[6];

	req[6].iRegister = Register::YEARS_REG;
	req[6].iAction = TReq::EWrite;
	req[6].iWriteValue = DecimalToBcd( aTime.iYear );
	req[6].iCompletionDfc = NULL;
	req[6].iNextPhase = &req[7];

	req[7].iRegister = RTC_CTRL_REG::Addr;
	req[7].iAction = TReq::EClearSet;
	req[7].iSetMask = RTC_CTRL_REG::STOP_RTC;
	req[7].iClearMask = 0;
	req[7].iCompletionDfc = &dfc;
	req[7].iNextPhase = NULL;

	NKern::FSSetOwner( &sem, NULL );
	ExecAsync( req[0] );
	NKern::FSWait( &sem );

	return KErrNone;
	}

EXPORT_C TBool Initialized()
	{
	return IsInitialized;
	}

} // namespace TPS65950




DECLARE_STANDARD_EXTENSION()
	{
	TInt r = TPS65950::Init();
	if( KErrNone == r )
		{
		r = InitInterrupts();
		}

	IsInitialized = ( KErrNone == r );

	return r;
	}

