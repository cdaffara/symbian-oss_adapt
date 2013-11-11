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
// omap3530/omap3530_drivers/uart/omap3530_uart.h
// This file is part of the Beagle Base port
//

#ifndef __OMAP3530_UART_H__
#define __OMAP3530_UART_H__

#include <assp/omap3530_assp/omap3530_hardware_base.h>
#include <assp/omap3530_assp/omap3530_prcm.h>
//#include "assp/omap3530_assp/omap3530_prm.h"
#include <assp/omap3530_assp/omap3530_irqmap.h>

//#include "omap3530_prm.h"


namespace Omap3530Uart
{
using namespace TexasInstruments::Omap3530;

enum TUartNumber
	{
	EUartNone = -1,
	EUart0 = 0,
	EUart1,
	EUart2
	};

template< const TUartNumber aUartNumber >
struct TUartTraits
	{
	};

template<>
struct TUartTraits< EUart0 >
	{
	static const TUint32 KBaseAddress = Omap3530HwBase::KL4_Core + 0x0006A000;
	static const TInt KInterruptId = EOmap3530_IRQ72_UART1_IRQ;
	static const Prcm::TClock	KInterfaceClock = Prcm::EClkUart1_I;
	static const Prcm::TClock	KFunctionClock = Prcm::EClkUart1_F;
//	static const Omap3530Prm::TPrmId	KPrmInterfaceClock = Omap3530Prm::EPrmClkUart1_I;
//	static const Omap3530Prm::TPrmId	KPrmFunctionClock = Omap3530Prm::EPrmClkUart1_F;
	};

template<>
struct TUartTraits< EUart1 >
	{
	static const TUint32 KBaseAddress = Omap3530HwBase::KL4_Core + 0x0006C000;
	static const TInt KInterruptId = EOmap3530_IRQ73_UART2_IRQ;
	static const Prcm::TClock	KInterfaceClock = Prcm::EClkUart2_I;
	static const Prcm::TClock	KFunctionClock = Prcm::EClkUart2_F;
//	static const Omap3530Prm::TPrmId	KPrmInterfaceClock = Omap3530Prm::EPrmClkUart2_I;
//	static const Omap3530Prm::TPrmId	KPrmFunctionClock = Omap3530Prm::EPrmClkUart2_F;
	};

template<>
struct TUartTraits< EUart2 >
	{
	static const TUint32 KBaseAddress = Omap3530HwBase::KL4_Per + 0x00020000;
	static const TInt KInterruptId = EOmap3530_IRQ74_UART3_IRQ;
	static const Prcm::TClock	KInterfaceClock = Prcm::EClkUart3_I;
	static const Prcm::TClock	KFunctionClock = Prcm::EClkUart3_F;
//	static const Omap3530Prm::TPrmId	KPrmInterfaceClock = Omap3530Prm::EPrmClkUart3_I;
//	static const Omap3530Prm::TPrmId	KPrmFunctionClock = Omap3530Prm::EPrmClkUart3_F;
	};

// Forward declaration
class TUart;


/** Representation of general UART register set */
struct DLL
	{
	static const TInt KOffset	= 0x00;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	CLOCK_LSB;
	};

struct RHR
	{
	static const TInt KOffset	= 0x00;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};

struct THR
	{
	static const TInt KOffset	= 0x00;
	static TDynReg8_W< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};

struct IER
	{
	static const TInt KOffset	= 0x04;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<7>	CTS_IT;
	typedef TSingleBitField<6>	RTS_IT;
	typedef TSingleBitField<5>	XOFF_IT;
	typedef TSingleBitField<4>	SLEEP_MODE;
	typedef TSingleBitField<3>	MODEM_STS_IT;
	typedef TSingleBitField<2>	LINE_STS_IT;
	typedef TSingleBitField<1>	THR_IT;
	typedef TSingleBitField<0>	RHR_IT;
	};

struct IER_IRDA
	{
	static const TInt KOffset	= 0x04;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<7>	EOF_IT;
	typedef TSingleBitField<6>	LINE_STS_IT;
	typedef TSingleBitField<5>	TX_STATUS_IT;
	typedef TSingleBitField<4>	STS_FIFO_TRIG_IT;
	typedef TSingleBitField<3>	RX_OVERRUN_IT;
	typedef TSingleBitField<2>	LAST_RX_BYTE_IT;
	typedef TSingleBitField<1>	THR_IT;
	typedef TSingleBitField<0>	RHR_IT;
	};

struct DLH
	{
	static const TInt KOffset	= 0x04;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<0,6>	CLOCK_MSB;
	};

struct FCR
	{
	static const TInt KOffset	= 0x08;
	static TDynReg8_W< TUart, KOffset >	iMem;
	typedef TSingleBitField<0>	FIFO_EN;
	typedef TSingleBitField<1>	RX_FIFO_CLEAR;
	typedef	TSingleBitField<2>	TX_FIFO_CLEAR;
	typedef TSingleBitField<3>	DMA_MODE;
	struct TX_FIFO_TRIG : public TBitField<4,2>
		{
		enum TConstants
			{
			K8Char	= 0 << KShift,
			K16Char	= 1 << KShift,
			K32Char	= 2 << KShift,
			K56Char	= 3 << KShift
			};
		};
	struct RX_FIFO_TRIG : public TBitField<6,2>
		{
		static const TUint8	K8Char	= 0 << KShift;
		static const TUint8	K16Char	= 1 << KShift;
		static const TUint8	K56Char	= 2 << KShift;
		static const TUint8	K60Char	= 3 << KShift;
		};
	};

struct IIR
	{
	static const TInt KOffset	= 0x08;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TBitField<6,2>	FCR_MIRROR;
	struct IT_TYPE : public TBitField<1,5>
		{
		enum TConstants
			{
			EModem	= 0 << KShift,
			ETHR	= 1 << KShift,
			ERHR	= 2 << KShift,
			ERxLineStatus	= 3 << KShift,
			ERxTimeout		= 6 << KShift,
			EXoff			= 8 << KShift,
			ECtsRts			= 16  << KShift
			};
		};
	typedef TSingleBitField<0>	IT_PENDING;
	};

struct EFR
	{
	static const TInt KOffset	= 0x08;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<7>	AUTO_CTS_EN;
	typedef TSingleBitField<6>	AUTO_RTS_EN;
	typedef TSingleBitField<5>	SPEC_CHAR;
	typedef TSingleBitField<4>	ENHANCED_EN;
	struct SW_FLOW_CONTROL : public TBitField<0,4>
		{
		enum TFlowControl
			{
			ENone		= 0,
			EXonXoff1	= 8,
			EXonXoff2	= 4,
			EXonXoffBoth = 8 + 4,
			};
		};
	};

struct LCR
	{
	static const TInt KOffset	= 0x0c;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<7>	DIV_EN;
	typedef TSingleBitField<6>	BREAK_EN;
	typedef TSingleBitField<5>	PARITY_TYPE2;
	typedef TSingleBitField<4>	PARITY_TYPE1;
	typedef TSingleBitField<3>	PARITY_EN;
	struct NB_STOP : public TSingleBitField<2>
		{
		enum TConstants
			{
			E1Stop		= 0 << KShift,
			E1_5Stop	= 1 << KShift,
			E2Stop		= 1 << KShift
			};
		};
	struct CHAR_LENGTH : public TBitField<0,2>
		{
		enum TConstants
			{
			E5Bits		= 0,
			E6Bits		= 1,
			E7Bits		= 2,
			E8Bits		= 3
			};
		};

	/** Special magic number to enter MODEA */
	static const TUint8 KConfigModeA = 0x80;

	/** Special magic number to enter MODEB */
	static const TUint8 KConfigModeB = 0xBF;

	/** Special magic number to enter operational mode */
	static const TUint8 KConfigModeOperational = 0x00;
	};

struct MCR
	{
	static const TInt KOffset	= 0x10;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<6>	TCR_TLR;
	typedef TSingleBitField<5>	XON_EN;
	typedef TSingleBitField<4>	LOOPBACK_EN;
	typedef TSingleBitField<3>	CD_STS_CH;
	typedef TSingleBitField<2>	RI_STS_CH;
	typedef TSingleBitField<1>	RTS;
	typedef TSingleBitField<0>	DTR;
	};

struct XON1_ADDR1
	{
	static const TInt KOffset	= 0x10;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};

struct LSR
	{
	static const TInt KOffset	= 0x14;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TSingleBitField<7>	RX_FIFO_STS;
	typedef TSingleBitField<6>	TX_SR_E;
	typedef TSingleBitField<5>	TX_FIFO_E;
	typedef TSingleBitField<4>	RX_BI;
	typedef TSingleBitField<3>	RX_FE;
	typedef TSingleBitField<2>	RX_PE;
	typedef TSingleBitField<1>	RX_OE;
	typedef TSingleBitField<0>	RX_FIFO_E;
	};

struct XON2_ADDR2
	{
	static const TInt KOffset	= 0x14;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};

struct XOFF1
	{
	static const TInt KOffset	= 0x18;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};

struct TCR
	{
	static const TInt KOffset	= 0x18;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<4,4>	RX_FIFO_TRIG_START;
	typedef TBitField<0,4>	RX_FIFO_TRIG_HALT;
	};

struct MSR
	{
	static const TInt KOffset	= 0x18;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TSingleBitField<7>	NCD_STS;
	typedef TSingleBitField<6>	NRI_STS;
	typedef TSingleBitField<5>	NDSR_STS;
	typedef TSingleBitField<4>	NCTS_STS;
	typedef TSingleBitField<3>	DCD_STS;
	typedef TSingleBitField<2>	RI_STS;
	typedef TSingleBitField<1>	DSR_STS;
	typedef TSingleBitField<0>	CTS_STS;
	};

struct SPR
	{
	static const TInt KOffset	= 0x1c;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	SPR_WORD;
	};

struct XOFF2
	{
	static const TInt KOffset	= 0x1c;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};

struct TLR
	{
	static const TInt KOffset	= 0x1c;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<4,4>	RX_FIFO_TRIG_DMA;
	typedef TBitField<0,4>	TX_FIFO_TRIG_DMA;
	};

struct MDR1
	{
	static const TInt KOffset	= 0x20;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<7>	FRAME_END_MODE;
	typedef TSingleBitField<6>	SIP_MODE;
	typedef TSingleBitField<5>	SCT;
	typedef TSingleBitField<4>	SET_TXIR;
	typedef TSingleBitField<3>	IR_SLEEP;
	struct MODE_SELECT : public TBitField<0,3>
		{
		enum TMode
			{
			EUart16x		= 0,
			ESIR			= 1,
			EUart16xAutoBaud = 2,
			EUart13x		= 3,
			EMIR			= 4,
			EFIR			= 5,
			ECIR			= 6,
			EDisable		= 7
			};
		};
	};

struct MDR2
	{
	static const TInt KOffset	= 0x24;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<6>	IRRXINVERT;
	struct CIR_PULSE_MODE : public TBitField<4,2>
		{
		enum TConstants
			{
			EPw3	= 0 << KShift,
			EPw4	= 1 << KShift,
			EPw5	= 2 << KShift,
			EPw6	= 3 << KShift
			};
		};
	typedef TSingleBitField<3>	UART_PULSE;
	struct STS_FIFO_TRIG : public TBitField<1,2>
		{
		enum TConstants
			{
			E1Entry		= 0 << KShift,
			E4Entry		= 1 << KShift,
			E7Entry		= 2 << KShift,
			E8Entry		= 3 << KShift
			};
		};
	typedef TSingleBitField<0>	IRTX_UNDERRUN;
	};

struct TXFLL
	{
	static const TInt KOffset	= 0x28;
	static TDynReg8_W< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	TX_FLL;
	};

struct SFLSR
	{
	static const TInt KOffset	= 0x28;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TSingleBitField<4>	OE_ERROR;
	typedef TSingleBitField<3>	FTL_ERROR;
	typedef TSingleBitField<2>	ABORT_DETECT;
	typedef TSingleBitField<1>	CRC_ERROR;
	};

struct RESUME
	{
	static const TInt KOffset	= 0x2c;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};

struct TXFLH
	{
	static const TInt KOffset	= 0x2c;
	static TDynReg8_W< TUart, KOffset >	iMem;
	typedef TBitField<0,5>	TX_FLH;
	};

struct RXFLL
	{
	static const TInt KOffset	= 0x30;
	static TDynReg8_W< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	RX_FLL;
	};

struct SFREGL
	{
	static const TInt KOffset	= 0x30;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};

struct SFREGH
	{
	static const TInt KOffset	= 0x34;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TBitField<0,4>	Value;
	};

struct RXFLH
	{
	static const TInt KOffset	= 0x34;
	static TDynReg8_W< TUart, KOffset >	iMem;
	typedef TBitField<0,4>	RX_FLH;
	};

struct BLR
	{
	typedef TSingleBitField<7>	STS_FIFO_RESET;
	typedef TSingleBitField<6>	XBOF_TYPE;
	};

struct UASR
	{
	static const TInt KOffset	= 0x38;
	static TDynReg8_R< TUart, KOffset >	iMem;
	struct PARITY_TYPE : public TBitField<6,2>
		{
		enum TConstants
			{
			ENone	= 0 << KShift,
			ESpace	= 1 << KShift,
			EEven	= 2 << KShift,
			EOdd	= 3 << KShift
			};
		};
	struct BIT_BY_CHAR : public TSingleBitField<5>
		{
		enum TConstants
			{
			E7Bit	= 0 << KShift,
			E8Bit	= 1 << KShift
			};
		};
	struct SPEED : public TBitField<0,5>
		{
		enum TBaud
			{
			EUnknown	= 0,
			E115200		= 1,
			E57600		= 2,
			E38400		= 3,
			E28800		= 4,
			E19200		= 5,
			E14400		= 6,
			E9600		= 7,
			E4800		= 8,
			E4800_2		= 9,
			E1200		= 10
			};
		};
	};

struct ACREG
	{
	static const TInt KOffset	= 0x3c;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<7>	PULSE_TYPE;
	typedef TSingleBitField<6>	SID_MOD;
	typedef TSingleBitField<5>	DIS_IR_RX;
	typedef TSingleBitField<4>	DIS_TX_UNDERRUN;
	typedef TSingleBitField<3>	SEND_SIP;
	typedef TSingleBitField<2>	SCTX_EN;
	typedef TSingleBitField<1>	ABORT_EN;
	typedef TSingleBitField<0>	EOT_EN;
	};

struct SCR
	{
	static const TInt KOffset	= 0x40;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<7>	RX_TRIG_GRANU1;
	typedef TSingleBitField<6>	TX_TRIG_GRANU1;
	typedef TSingleBitField<4>	RX_CTS_WU_EN;
	typedef TSingleBitField<3>	TX_EMPTY_CTL_IT;
	typedef TBitField<1,2>		DMA_MODE2;
	typedef TSingleBitField<0>	DMA_MODE_CTL;
	};

struct SSR
	{
	static const TInt KOffset	= 0x44;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TSingleBitField<1>	RX_CTS_WU_STS;
	typedef TSingleBitField<0>	TX_FIFO_FULL;
	};

struct EBLR
	{
	static const TInt KOffset	= 0x48;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};

struct SYSC
	{
	static const TInt KOffset	= 0x54;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	struct IDLE_MODE : public TBitField<3,2>
		{
		enum TMode
			{
			EForceIdle	= 0 << KShift,
			ENoIdle		= 1 << KShift,
			ESmartIdle	= 2 << KShift
			};
		};
	typedef TSingleBitField<2>	ENAWAKEUP;
	typedef TSingleBitField<1>	SOFTRESET;
	typedef TSingleBitField<0>	AUTOIDLE;
	};

struct SYSS
	{
	static const TInt KOffset	= 0x58;
	static TDynReg8_R< TUart, KOffset >	iMem;
	typedef TSingleBitField<0>	RESETDONE;
	};

struct WER
	{
	static const TInt KOffset	= 0x5c;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TSingleBitField<6>	EVENT_6_RLS_INTERRUPT;
	typedef TSingleBitField<5>	EVENT_5_RHR_INTERRUPT;
	typedef TSingleBitField<4>	EVENT_4_RX_ACTIVITY;
	typedef TSingleBitField<2>	EVENT_2_RI_ACTIVITY;
	typedef TSingleBitField<0>	EVENT_0_CTS_ACTIVITY;
	};

struct CFPS
	{
	static const TInt KOffset	= 0x60;
	static TDynReg8_RW< TUart, KOffset >	iMem;
	typedef TBitField<0,8>	Value;
	};


class TUart
	{
	public:
		enum TBaud
			{
			E1200,
			E2400,
			E4800,
			E9600,
			E14400,
			E19200,
			E28800,
			E38400,
			E57600,
			E115200,
			E230400,
			E460800,
			E921600,
			E1843000,
			E3688400,
			E4000000,		// FIR

			KSupportedBaudCount
			};

		enum TParity
			{
			ENone,
			EOdd,
			EEven,
			EMark,
			ESpace
			};

		enum TDataBits
			{
			E5Data = ::Omap3530Uart::LCR::CHAR_LENGTH::E5Bits,
			E6Data = ::Omap3530Uart::LCR::CHAR_LENGTH::E6Bits,
			E7Data = ::Omap3530Uart::LCR::CHAR_LENGTH::E7Bits,
			E8Data = ::Omap3530Uart::LCR::CHAR_LENGTH::E8Bits,
			};

		enum TStopBits
			{
			E1Stop = ::Omap3530Uart::LCR::NB_STOP::E1Stop,
			E1_5Stop = ::Omap3530Uart::LCR::NB_STOP::E1_5Stop,
			E2Stop = ::Omap3530Uart::LCR::NB_STOP::E2Stop,
			};

		enum TUartMode
			{
			EUart,
			EUartAutoBaud,
			ESIR,
			EMIR,
			EFIR,
			ECIR,

			KSupportedUartModes
			};

		enum TFifoTrigger
			{
			ETrigger8,
			ETrigger16,
			ETrigger32,
			ETrigger56,
			ETrigger60,
			ETriggerUnchanged
			};

		enum TEnableState
			{
			EDisabled,
			EEnabled
			};

		enum TInterrupt
			{
			EIntRhr = 0,
			EIntThr = 1,
			EIntLineStatus = 2,
			EIntModemStatus = 3,
			EIntXoff = 5,
			EIntRts = 6,
			EIntCts = 7
			};

	public:
		inline TUart( const TUartNumber aUartNumber )
			: iBase(	(aUartNumber == EUart0 )	?	TUartTraits<EUart0>::KBaseAddress
							:	(aUartNumber == EUart1 )	?	TUartTraits<EUart1>::KBaseAddress
							:	(aUartNumber == EUart2 )	?	TUartTraits<EUart2>::KBaseAddress
							:	0 ),
				iUartNumber( aUartNumber )
			{}

		FORCE_INLINE TLinAddr Base() const
			{ return iBase; }

		IMPORT_C TInt InterruptId() const;

		IMPORT_C Prcm::TClock PrcmInterfaceClk() const;
		
		IMPORT_C Prcm::TClock PrcmFunctionClk() const;

//		IMPORT_C TInt PrmInterfaceClk() const;
		
//		IMPORT_C TInt PrmFunctionClk() const;

		/** Reset and initialize the UART 
		 * On return the UART will be in disable mode */
		IMPORT_C void Init();

		/** Defines which mode the UART will run in when enabled, but does not configure that mode
		 * You must call this before calling SetBaud to ensure that correct baud rate multiplier is used */
		IMPORT_C void DefineMode( const TUartMode aMode );

		/** Enabled the UART in the defined mode
		 * You must call DefineMode() and SetBaud() before calling Enable()
		 */
		IMPORT_C void Enable();

		/** Disables the UART */
		IMPORT_C void Disable();

		/** Set the baud rate 
		 * Do not call this while the UART is enabled
		 * You must have previously called DefineMode()
		 */
		IMPORT_C void SetBaud( const TBaud aBaud );

		/** Set the data length, parity and stop bits */
		IMPORT_C void SetDataFormat( const TDataBits aDataBits, const TStopBits aStopBits, const TParity aParity );

		/** Setup the FIFO configuration */
		IMPORT_C void EnableFifo( const TEnableState aState, const TFifoTrigger aRxTrigger = ETriggerUnchanged, const TFifoTrigger aTxTrigger = ETriggerUnchanged );

		/** Enable a particular interrupt source */
		IMPORT_C void EnableInterrupt( const TInterrupt aWhich );

		/** Disable a particular interrupt source */
		IMPORT_C void DisableInterrupt( const TInterrupt aWhich );

		/** Disable all interrupts */
		IMPORT_C void DisableAllInterrupts();

		inline TBool TxFifoFull()
			{ return SSR::iMem.Read(*this) bitand SSR::TX_FIFO_FULL::KMask; }

		inline TBool TxFifoEmpty()
			{ return LSR::iMem.Read(*this) bitand LSR::TX_FIFO_E::KMask; }

		inline TBool RxFifoEmpty()
			{ return !(LSR::iMem.Read(*this) bitand LSR::RX_FIFO_E::KMask); }

		inline void Write( TUint8 aByte )
			{ THR::iMem.Write( *this, aByte ); }

		inline TUint8 Read()
			{ return RHR::iMem.Read( *this ); }

	private:
		TUart();

	public:
		const TLinAddr	iBase;
		const TUartNumber	iUartNumber : 8;
		TUartMode		iMode : 8;
		::Omap3530Uart::MDR1::MODE_SELECT::TMode	iTargetMode : 8;
	};


} // Omap3530Uart

#endif // ndef __OMAP3530_UART_H__

