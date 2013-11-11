// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/omap3530_drivers/uart/uart.cpp
// This file is part of the Beagle Base port
//

#include <e32cmn.h>
#include <assp/omap3530_assp/omap3530_uart.h>

#ifdef _DEBUG
LOCAL_C void ClientPanic( TInt aLine )
	{
	_LIT( KString, "uart.cpp" );
	Kern::PanicCurrentThread( KString, aLine );
	}
#endif


namespace Omap3530Uart
{

struct THwInfo
	{
	TUint32				iBaseAddress;
	TInt				iInterruptId;
	Prcm::TClock		iInterfaceClock : 16;
	Prcm::TClock		iFunctionClock : 16;
//	Omap3530Prm::TPrmId	iPrmInterfaceClock : 16;
//	Omap3530Prm::TPrmId	iPrmFunctionClock : 16;
	};

static const THwInfo KHwInfo[3] =
	{
		{ // EUart0
		TUartTraits< EUart0 >::KBaseAddress,
		TUartTraits< EUart0 >::KInterruptId,
		TUartTraits< EUart0 >::KInterfaceClock,
		TUartTraits< EUart0 >::KFunctionClock,
//		TUartTraits< EUart0 >::KPrmInterfaceClock,
//		TUartTraits< EUart0 >::KPrmFunctionClock
		},
		{ // EUart1
		TUartTraits< EUart1 >::KBaseAddress,
		TUartTraits< EUart1 >::KInterruptId,
		TUartTraits< EUart1 >::KInterfaceClock,
		TUartTraits< EUart1 >::KFunctionClock,
//		TUartTraits< EUart1 >::KPrmInterfaceClock,
//		TUartTraits< EUart1 >::KPrmFunctionClock
		},
		{ // EUart2
		TUartTraits< EUart2 >::KBaseAddress,
		TUartTraits< EUart2 >::KInterruptId,
		TUartTraits< EUart2 >::KInterfaceClock,
		TUartTraits< EUart2 >::KFunctionClock,
//		TUartTraits< EUart2 >::KPrmInterfaceClock,
//		TUartTraits< EUart2 >::KPrmFunctionClock
		},
	};

// Baud lookup table, indexed by [ TBaud, TUartMode ].
// Values obtained from 3530 datasheet

struct TBaudInfo
	{
	TUint8		iDlh;
	TUint8		iDll;
	TUint8		iMultiplier;	// Multiplier selection for UART16x/13x mode
	};

static const TBaudInfo KBaudControl[ TUart::KSupportedBaudCount ][ TUart::KSupportedUartModes ] =
	{
	//				EUart				EUartAutoBaud		ESIR				EMIR				EFIR				ECIR
	/*1200*/	{	{0x09, 0xC4, 16	},	{0x09, 0xC4, 16	},	{0x09, 0xC4, 16	},	{0x00, 0x02, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*2400*/	{	{0x04, 0xE2, 16	},	{0x04, 0xE2, 16	},	{0x04, 0xE2, 16	},	{0x00, 0x02, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*4800*/	{	{0x02, 0x71, 16	},	{0x02, 0x71, 16	},	{0x02, 0x71, 16	},	{0x00, 0x02, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*9600*/	{	{0x01, 0x38, 16	},	{0x01, 0x38, 16	},	{0x01, 0x38, 16	},	{0x00, 0x02, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*14400*/	{	{0x00, 0xD0, 16	},	{0x00, 0xD0, 16	},	{0x00, 0xD0, 16	},	{0x00, 0x02, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*19200*/	{	{0x00, 0x9C, 16	},	{0x00, 0x9C, 16	},	{0x00, 0x9C, 16	},	{0x00, 0x02, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*28800*/	{	{0x00, 0x68, 16	},	{0x00, 0x68, 16	},	{0x00, 0x68, 16	},	{0x00, 0x02, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*38400*/	{	{0x00, 0x4E, 16	},	{0x00, 0x4E, 16	},	{0x00, 0x4E, 16	},	{0x00, 0x02, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*57600*/	{	{0x00, 0x34, 16	},	{0x00, 0x34, 16	},	{0x00, 0x34, 16	},	{0x00, 0x02, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*115200*/	{	{0x00, 0x1A, 16	},	{0x00, 0x1A, 16	},	{0x00, 0x1A, 16	},	{0x00, 0x01, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*230400*/	{	{0x00, 0x0D, 16	},	{0x09, 0xC4, 16	},	{0x00, 0x1A, 16	},	{0x00, 0x01, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 16	} },
	/*460800*/	{	{0x00, 0x08, 13	},	{0x09, 0xC4, 13	},	{0x00, 0x1A, 16	},	{0x00, 0x01, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 13	} },
	/*921600*/	{	{0x00, 0x04, 13	},	{0x09, 0xC4, 13	},	{0x00, 0x1A, 16	},	{0x00, 0x01, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 13	} },
	/*1843000*/	{	{0x00, 0x02, 13	},	{0x09, 0xC4, 13	},	{0x00, 0x1A, 16	},	{0x00, 0x01, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 13	} },
	/*3688400*/	{	{0x00, 0x01, 13	},	{0x09, 0xC4, 13	},	{0x00, 0x1A, 16	},	{0x00, 0x01, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 13	} },
	/*4000000*/	{	{0x00, 0x01, 13	},	{0x09, 0xC4, 13	},	{0x00, 0x1A, 16	},	{0x00, 0x01, 1	},	{0x00, 0x0, 1	},	{0x09, 0xC4, 13	} }
	};

/** Default mode to target mode conversion */
static const MDR1::MODE_SELECT::TMode	KDefaultTargetMode[ TUart::KSupportedUartModes ] =
	{
	MDR1::MODE_SELECT::EUart16x,			// EUart
	MDR1::MODE_SELECT::EUart16xAutoBaud,	// EUartAutoBaud
	MDR1::MODE_SELECT::ESIR,				// ESIR
	MDR1::MODE_SELECT::EMIR,				// EMIR
	MDR1::MODE_SELECT::EFIR,				// EFIR
	MDR1::MODE_SELECT::ECIR,				// ECIR
	};

/** Conversion table from parity mode to LCR bit settings */
static const TUint8 KParitySelectTable[ 5 ] =
	{
	LCR::PARITY_TYPE2::KOff		bitor	LCR::PARITY_TYPE1::KOff		bitor	LCR::PARITY_EN::KOff,	//	ENone,
	LCR::PARITY_TYPE2::KOff		bitor	LCR::PARITY_TYPE1::KOff		bitor	LCR::PARITY_EN::KOn,	//	EOdd,
	LCR::PARITY_TYPE2::KOff		bitor	LCR::PARITY_TYPE1::KOn		bitor	LCR::PARITY_EN::KOn,	//	EEven,
	LCR::PARITY_TYPE2::KOn		bitor	LCR::PARITY_TYPE1::KOff		bitor	LCR::PARITY_EN::KOn,	//	EMark,
	LCR::PARITY_TYPE2::KOn		bitor	LCR::PARITY_TYPE1::KOn		bitor	LCR::PARITY_EN::KOn,	//	ESpace
	};

static const TUint8 KRxFifoTrigTable[] =
	{
	FCR::RX_FIFO_TRIG::K8Char,	// EFifoTrigger8,
	FCR::RX_FIFO_TRIG::K16Char,	// EFifoTrigger16,
	FCR::RX_FIFO_TRIG::K16Char,	// EFifoTrigger32,
	FCR::RX_FIFO_TRIG::K56Char,	// EFifoTrigger56,
	FCR::RX_FIFO_TRIG::K60Char,	// EFifoTrigger60
	};

static const TUint8 KTxFifoTrigTable[] =
	{
	FCR::TX_FIFO_TRIG::K8Char,	// EFifoTrigger8,
	FCR::TX_FIFO_TRIG::K16Char,	// EFifoTrigger16,
	FCR::TX_FIFO_TRIG::K32Char,	// EFifoTrigger32,
	FCR::TX_FIFO_TRIG::K56Char,	// EFifoTrigger56,
	FCR::TX_FIFO_TRIG::K56Char,	// EFifoTrigger60
	};

// RAII for entering and leaving mode B with enhanced enabled
class TRaiiEnhancedModeB
	{
	public:
		TRaiiEnhancedModeB( TUart& aUart );
		~TRaiiEnhancedModeB();

	private:
		TRaiiEnhancedModeB();

	private:
		TUart&	iUart;
		TUint8	iOldLcr;
		TUint8	iOldEnhanced;
	};

TRaiiEnhancedModeB::TRaiiEnhancedModeB( TUart& aUart )
	: iUart( aUart )
	{
	iOldLcr = LCR::iMem.Read( aUart );
	LCR::iMem.Write( aUart, LCR::KConfigModeB );
	iOldEnhanced = EFR::iMem.Read( aUart ) bitand EFR::ENHANCED_EN::KMask;
	EFR::iMem.Modify( aUart, KClearNone, EFR::ENHANCED_EN::KOn );
	}

TRaiiEnhancedModeB::~TRaiiEnhancedModeB()
	{
	LCR::iMem.Write( iUart, LCR::KConfigModeB );
	EFR::iMem.Modify( iUart, EFR::ENHANCED_EN::KMask, iOldEnhanced );
	LCR::iMem.Write( iUart, iOldLcr );
	}



EXPORT_C void TUart::Init()
	{
	// Perfom a UART soft reset
	SYSC::iMem.Write( *this, SYSC::SOFTRESET::KOn );
	while( 0 == (SYSS::iMem.Read( *this ) bitand SYSS::RESETDONE::KMask) );
	LCR::iMem.Write( *this, LCR::KConfigModeB );
	EFR::iMem.Modify( *this, KClearNone, EFR::ENHANCED_EN::KOn );
	LCR::iMem.Write( *this, LCR::KConfigModeA );
	MCR::iMem.Modify( *this, KClearNone, MCR::TCR_TLR::KOn );
	FCR::iMem.Write( *this,			FCR::FIFO_EN::KOn
													bitor	FCR::RX_FIFO_CLEAR::KOn
													bitor	FCR::TX_FIFO_CLEAR::KOn );
	LCR::iMem.Write( *this, LCR::KConfigModeB );
	EFR::iMem.Modify( *this, EFR::ENHANCED_EN::KOn, KSetNone );
	LCR::iMem.Write( *this, LCR::KConfigModeA );
	MCR::iMem.Modify( *this, MCR::TCR_TLR::KOn, KSetNone );
	LCR::iMem.Write( *this, LCR::KConfigModeOperational );
	}

EXPORT_C TInt TUart::InterruptId() const
	{
	return KHwInfo[ iUartNumber ].iInterruptId;
	}

EXPORT_C Prcm::TClock TUart::PrcmInterfaceClk() const
	{
	return KHwInfo[ iUartNumber ].iInterfaceClock;
	}
		
EXPORT_C Prcm::TClock TUart::PrcmFunctionClk() const
	{
	return KHwInfo[ iUartNumber ].iFunctionClock;
	}

/*PORT_C TInt TUart::PrmInterfaceClk() const
	{
	return KHwInfo[ iUartNumber ].iPrmInterfaceClock;
	}
		
EXPORT_C TInt TUart::PrmFunctionClk() const
	{
	return KHwInfo[ iUartNumber ].iPrmFunctionClock;
	}
*/
EXPORT_C void TUart::DefineMode( const TUartMode aMode )
	{
	__ASSERT_DEBUG( (TUint)aMode <= KSupportedUartModes, ClientPanic( __LINE__ ) );
	iMode = aMode;
	iTargetMode = KDefaultTargetMode[ aMode ];
	}

EXPORT_C void TUart::Enable()
	{
	// UART won't be enabled if a read-write cycle is done to MDR1
	// So just write the mode into MDR1 and clear anythis already in register
//	MDR1::iMem.Modify( *this, MDR1::MODE_SELECT::KFieldMask, iTargetMode );
	MDR1::iMem.Write( *this, iTargetMode );
	}

EXPORT_C void TUart::Disable()
	{
	MDR1::iMem.Modify( *this, MDR1::MODE_SELECT::KFieldMask, MDR1::MODE_SELECT::EDisable );
	}

EXPORT_C void TUart::SetBaud( const TBaud aBaud )
	{
	__ASSERT_DEBUG( (TUint)aBaud < KSupportedBaudCount, ClientPanic( __LINE__ ) );

	const TUint dlh = KBaudControl[ aBaud ][ iMode ].iDlh;
	const TUint dll = KBaudControl[ aBaud ][ iMode ].iDll;

		{
		TRaiiEnhancedModeB	enhanced_mode_b_in_current_scope( *this );

		LCR::iMem.Write( *this, LCR::KConfigModeOperational );
		const TUint8 ier = IER::iMem.Read( *this );
		IER::iMem.Write( *this, 0 );
		LCR::iMem.Write( *this, LCR::KConfigModeB );

		DLL::iMem.Write( *this, dll );
		DLH::iMem.Write( *this, dlh );

		LCR::iMem.Write( *this, LCR::KConfigModeOperational );
		IER::iMem.Write( *this, ier );
		LCR::iMem.Write( *this, LCR::KConfigModeB );
		}

	// Update target mode if a multipler change is required
	if( EUart == iMode )
		{
		const TUint m = KBaudControl[ aBaud ][ iMode ].iMultiplier;
		if( 13 == m )
			{
			iTargetMode = MDR1::MODE_SELECT::EUart13x;
			}
		else
			{
			iTargetMode = MDR1::MODE_SELECT::EUart16x;
			}
		}
	}

EXPORT_C void TUart::SetDataFormat( const TDataBits aDataBits, const TStopBits aStopBits, const TParity aParity )
	{
	__ASSERT_DEBUG( (TUint)aDataBits <= E8Data, ClientPanic( __LINE__ ) );
	__ASSERT_DEBUG( (TUint)aStopBits <= E2Stop, ClientPanic( __LINE__ ) );
	__ASSERT_DEBUG( (TUint)aParity <= ESpace, ClientPanic( __LINE__ ) );

	const TRegValue8 lcrSet = aDataBits bitor aStopBits bitor KParitySelectTable[ aParity ];
	const TRegValue8 KClearMask =			LCR::PARITY_TYPE2::KFieldMask
									bitor	LCR::PARITY_TYPE1::KFieldMask
									bitor	LCR::PARITY_EN::KFieldMask
									bitor	LCR::NB_STOP::KFieldMask
									bitor	LCR::CHAR_LENGTH::KFieldMask;
	
	LCR::iMem.Modify( *this, KClearMask, lcrSet );

	}

EXPORT_C void TUart::EnableFifo( const TEnableState aState, const TFifoTrigger aRxTrigger, const TFifoTrigger aTxTrigger )
	{
	TRaiiEnhancedModeB	enhanced_mode_b_in_current_scope( *this );

	const TUint8 dll = DLL::iMem.Read( *this );
	const TUint8 dlh = DLH::iMem.Read( *this );

	DLL::iMem.Write( *this, 0 );
	DLH::iMem.Write( *this, 0 );

	const TUint8 rx_trig = ((TUint)aRxTrigger >= ETriggerUnchanged)
							? 0
							: KRxFifoTrigTable[ aRxTrigger ];
	const TUint8 tx_trig = ((TUint)aTxTrigger >= ETriggerUnchanged)
							? 0
							: KTxFifoTrigTable[ aTxTrigger ];
	const TUint8 KClearMask = TUint8(Omap3530Uart::FCR::RX_FIFO_TRIG::KFieldMask bitor Omap3530Uart::FCR::TX_FIFO_TRIG::KFieldMask);

	if( EEnabled == aState )
		{
		FCR::iMem.Modify( *this,
							KClearMask,
							rx_trig bitor tx_trig bitor FCR::FIFO_EN::KOn );
		}
	else
		{
		FCR::iMem.Modify( *this, FCR::FIFO_EN::KOn,	KSetNone );
		}

	DLL::iMem.Write( *this, dll );
	DLH::iMem.Write( *this, dlh );
	}


LOCAL_C void ModifyIER( TUart& aUart, TUint8 aClearMask, TUint8 aSetMask )
	{
	TRaiiEnhancedModeB	enhanced_mode_b_in_current_scope( aUart );

	LCR::iMem.Write( aUart, Omap3530Uart::LCR::KConfigModeOperational );
	IER::iMem.Modify( aUart, aClearMask, aSetMask );
	LCR::iMem.Write( aUart, LCR::KConfigModeB );
	}

inline void EnableDisableInterrupt( TUart& aUart, TBool aEnable, TUart::TInterrupt aWhich )
	{
	ModifyIER( aUart, 
					(aEnable ? KClearNone : (TUint8)1 << aWhich),
					(aEnable ? (TUint8)1 << aWhich : KSetNone) );
	}

EXPORT_C void TUart::EnableInterrupt( const TInterrupt aWhich )
	{
	EnableDisableInterrupt( *this, ETrue, aWhich );
	}

EXPORT_C void TUart::DisableInterrupt( const TInterrupt aWhich )
	{
	EnableDisableInterrupt( *this, EFalse, aWhich );
	}


EXPORT_C void TUart::DisableAllInterrupts()
	{
	ModifyIER( *this, (TUint8)KClearAll, KSetNone );
	}


} // namespace Omap3530Uart


DECLARE_STANDARD_EXTENSION()
	{
	return KErrNone;
	}

