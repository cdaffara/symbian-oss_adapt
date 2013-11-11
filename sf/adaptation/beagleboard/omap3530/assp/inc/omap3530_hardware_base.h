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
// omap3530/assp/inc/omap3530_hardware_base.h
// Linear base addresses for hardware peripherals on the beagle board.
// This file is part of the Beagle Base port
//

#ifndef OMAP3530_HARDWARE_BASE_H__
#	define OMAP3530_HARDWARE_BASE_H__

#include <assp.h> // for TPhysAddr, AsspRegister
#include <assp/omap3530_assp/omap3530_asspreg.h>

namespace TexasInstruments
	{

	namespace Omap3530
		{

		/**
		Define constants for the various physical address blocks used on the OMAP3530
		*/
		enum TPhysicalAddresses
			{
			KKiloByte				=	1024,
			KMegaByte				=	(1024 * KKiloByte),
			KL4_Core_PhysicalBase	=	0x48000000,
			KL4_Core_PhysicalSize	=	(4 * KMegaByte),
			KL4_Core_PhysicalEnd	=	(KL4_Core_PhysicalBase + KL4_Core_PhysicalSize),

			KL4_WakeUp_PhysicalBase	=	0x48300000,
			KL4_WakeUp_PhysicalSize	=	(256 * KKiloByte ),
			KL4_WakeUp_PhysicalEnd	=	(KL4_WakeUp_PhysicalBase + KL4_WakeUp_PhysicalSize),

			KL4_Per_PhysicalBase	=	0x49000000,
			KL4_Per_PhysicalSize	=	(1 * KMegaByte),
			KL4_Per_PhysicalEnd		=	(KL4_Per_PhysicalBase + KL4_Per_PhysicalSize),

			KL4_Sgx_PhysicalBase	=	0x50000000,
			KL4_Sgx_PhysicalSize	=	(64 * KKiloByte),
			KL4_Sgx_PhysicalEnd		=	(KL4_Sgx_PhysicalBase + KL4_Sgx_PhysicalSize),

			KL4_Emu_PhysicalBase	=	0x54000000,
			KL4_Emu_PhysicalSize	=	(8 * KMegaByte),
			KL4_Emu_PhysicalEnd		=	(KL4_Emu_PhysicalBase + KL4_Emu_PhysicalSize),

			KL3_Control_PhysicalBase	=	0x68000000,
			KL3_Control_PhysicalSize	=	(1 * KMegaByte),
			KL3_Control_PhysicalEnd		=	(KL3_Control_PhysicalBase + KL3_Control_PhysicalSize),

			KL3_Gpmc_PhysicalBase		=	0x6e000000,
			KL3_Gpmc_PhysicalSize		=	(1 * KMegaByte),
			KL3_Gpmc_PhysicalEnd		=	(KL3_Gpmc_PhysicalBase + KL3_Gpmc_PhysicalSize)
			} ;

		/**
		Define constants for the virtual address mappings used on the OMAP3530
		*/
		enum TLinearAddresses
			{
			KL4_Core_LinearBase		=	0xC6000000,
			KL4_Core_LinearSize		=	KL4_Core_PhysicalSize,
			KL4_Core_LinearEnd		=	(KL4_Core_LinearBase + KL4_Core_LinearSize),

			KL4_WakeUp_LinearBase	=	(KL4_Core_LinearBase + (KL4_WakeUp_PhysicalBase - KL4_Core_PhysicalBase)),
			KL4_WakeUp_LinearSize	=	KL4_WakeUp_PhysicalSize,
			KL4_WakeUp_LinearEnd	=	(KL4_WakeUp_LinearBase + KL4_WakeUp_LinearSize),

			KL4_Per_LinearBase		=	KL4_Core_LinearEnd,
			KL4_Per_LinearSize		=	KL4_Per_PhysicalSize,
			KL4_Per_LinearEnd		=	(KL4_Per_LinearBase + KL4_Per_LinearSize),

			KL4_Sgx_LinearBase		=	KL4_Per_LinearEnd,
			KL4_Sgx_LinearSize		=	KL4_Sgx_PhysicalSize,
			KL4_Sgx_LinearEnd		=	(KL4_Sgx_LinearBase + KL4_Sgx_LinearSize),

			KL4_Emu_LinearBase		=	KL4_Sgx_LinearBase + KMegaByte,
			KL4_Emu_LinearSize		=	KL4_Emu_PhysicalSize,
			KL4_Emu_LinearEnd		=	(KL4_Emu_LinearBase + KL4_Emu_LinearSize),

			KL3_Control_LinearBase	=	KL4_Emu_LinearEnd,
			KL3_Control_LinearSize	=	KL3_Control_PhysicalSize,
			KL3_Control_LinearEnd	=	(KL3_Control_LinearBase + KL3_Control_LinearSize),

			KL3_Gpmc_LinearBase		=	KL3_Control_LinearEnd,
			KL3_Gpmc_LinearSize		=	KL3_Gpmc_PhysicalSize,
			KL3_Gpmc_LinearEnd		=	(KL3_Gpmc_LinearBase + KL3_Gpmc_LinearSize)
			} ;

		/**
		A template to provide the virtual address of a given physical address.
		@example
		@code
			enum TTimerBaseAddress
				{
				KGPTIMER1_Base			= Omap3530HwBase::TVirtual<0x48318000>::Value,
				} ;
		*/
		template<const TPhysAddr aDdReSs>
		struct TVirtual
			{
			enum TConstants
				{
				KIsL4Core		=	((aDdReSs >= KL4_Core_PhysicalBase) && (aDdReSs < KL4_Core_PhysicalEnd)),
				KIsL4WakeUp		=	((aDdReSs >= KL4_WakeUp_PhysicalBase) && (aDdReSs < KL4_WakeUp_PhysicalEnd)),	// Subset of L4Core
				KIsL4Per		=	((aDdReSs >= KL4_Per_PhysicalBase) && (aDdReSs < KL4_Per_PhysicalEnd)),
				KIsL4Sgx		=	((aDdReSs >= KL4_Sgx_PhysicalBase) && (aDdReSs < KL4_Sgx_PhysicalEnd)),
				KIsL4Emu		=	((aDdReSs >= KL4_Emu_PhysicalBase) && (aDdReSs < KL4_Emu_PhysicalEnd)),
				KIsL3Control	=	((aDdReSs >= KL3_Control_PhysicalBase) && (aDdReSs < KL3_Control_PhysicalEnd)),
				KIsL3Gpmc		=	((aDdReSs >= KL3_Gpmc_PhysicalBase) && (aDdReSs < KL3_Gpmc_PhysicalEnd)),
				KIsConvertable	=	(KIsL4Core || KIsL4Per || KIsL4Sgx || KIsL4Emu || KIsL3Control || KIsL3Gpmc),
				KIsMapped		=	(KIsL4Core || KIsL4Per || KIsL4Sgx || KIsL4Emu || KIsL3Control || KIsL3Gpmc),
				KOffset			= 	((KIsL4Core) ? (aDdReSs - KL4_Core_PhysicalBase)
										: ((KIsL4Per) ? (aDdReSs - KL4_Per_PhysicalBase)
											: ((KIsL4Sgx) ? (aDdReSs - KL4_Sgx_PhysicalBase)
												: ((KIsL4Emu) ? (aDdReSs - KL4_Emu_PhysicalBase)
													: ((KIsL3Control) ? (aDdReSs - KL3_Control_PhysicalBase)
														: ((KIsL3Gpmc) ? (aDdReSs - KL3_Gpmc_PhysicalBase)
															: (0))))))),
				// TODO: Change to give compile time error if address not mapped
				KLinearBase		=	((KIsL4Core) ? (KL4_Core_LinearBase)
										: ((KIsL4Per) ? (KL4_Per_LinearBase)
											: ((KIsL4Sgx) ? (KL4_Sgx_LinearBase)
												: ((KIsL4Emu) ? (KL4_Emu_LinearBase)
													: ((KIsL3Control) ? (KL3_Control_LinearBase)
														: ((KIsL3Gpmc) ? (KL3_Gpmc_LinearBase)
															: (0))))))),
				/**
				Returns the Linear address mapping  for a specific Physical address
				*/
				Value			=	(KLinearBase + KOffset)
				} ;
			} ;

		template<const TLinAddr aDdReSs>
		struct TLinearCheck
			{
			enum TConstants
				{
				KIsL4Core		=	((aDdReSs >= KL4_Core_LinearBase) && (aDdReSs < KL4_Core_LinearEnd)),
				KIsL4Per		=	((aDdReSs >= KL4_Per_LinearBase) && (aDdReSs < KL4_Per_LinearEnd)),
				KIsL4Sgx		=	((aDdReSs >= KL4_Sgx_LinearBase) && (aDdReSs < KL4_Sgx_LinearEnd)),
				KIsL4Emu		=	((aDdReSs >= KL4_Emu_LinearBase) && (aDdReSs < KL4_Emu_LinearEnd)),
				KIsL3Control	=	((aDdReSs >= KL3_Control_LinearBase) && (aDdReSs < KL3_Control_LinearBase)),
				KIsL3Gpmc		=	((aDdReSs >= KL3_Gpmc_LinearBase) && (aDdReSs < KL3_Gpmc_LinearBase)),
				KIsMapped		=	(KIsL4Core || KIsL4Per || KIsL4Sgx || KIsL4Emu || KIsL3Control || KIsL3Gpmc)
				} ;
			} ;

#	ifdef __MEMMODEL_MULTIPLE__
		const TUint KL4_Core	= KL4_Core_LinearBase; // KPrimaryIOBase
		const TUint KL4_Per		= KL4_Per_LinearBase;
		const TUint KSgx		= KL4_Sgx_LinearBase;
		const TUint KL4_Emu		= KL4_Emu_LinearBase;
		const TUint KL3_Control	= KL3_Control_LinearBase;
		const TUint KL3_Gpmc	= KL3_Gpmc_LinearBase;
//const TUint KIva2_2Ss = KL4_Core + 0x01910000;
//const TUint KL3ControlRegisters = KL4_Core + 0x04910000;
//const TUint KSmsRegisters = KL4_Core + 0x05910000;
//const TUint KSdrcRegisters = KL4_Core + 0x06910000;
//const TUint KGpmcRegisters = KL4_Core + 0x07910000;

//#elif __MEMMODEL_FLEXIBLE__
// define flexible memery model hw base addresses

#	else // unknown memery model
#		error hardware_base.h: Constants may need changing
#	endif // memory model

// Register Access types.

		typedef	TUint32	TRegValue;
		typedef	TUint32	TRegValue32;
		typedef	TUint16	TRegValue16;
		typedef	TUint8	TRegValue8;

		/**
		An interface template for read-only registers.
		*/
		template <TLinAddr aDdReSs>
		class TReg32_R
			{
		  public :
			static inline TRegValue Read()
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				return AsspRegister::Read32(aDdReSs) ;
				}
			} ;

		template <TLinAddr aDdReSs>
		class TReg16_R
			{
		  public :
			static inline TRegValue16 Read()
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				return AsspRegister::Read16(aDdReSs) ;
				}
			} ;

		template <TLinAddr aDdReSs>
		class TReg8_R
			{
		  public :
			static inline TRegValue8 Read()
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				return AsspRegister::Read8(aDdReSs) ;
				}
			} ;

		/**
		An interface template for read-write registers.
		*/
		template <TLinAddr aDdReSs>
		class TReg32_RW : public TReg32_R<aDdReSs>
			{
		  public :
			static inline void Write(const TRegValue aValue)
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				AsspRegister::Write32(aDdReSs, aValue) ;
				}
			static inline void Modify(const TRegValue aClearMask, const TRegValue aSetMask)
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				AsspRegister::Modify32(aDdReSs, aClearMask, aSetMask) ;
				}
			} ;

		template <TLinAddr aDdReSs>
		class TReg16_RW : public TReg16_R<aDdReSs>
			{
		  public :
			static inline void Write(const TRegValue16 aValue)
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				AsspRegister::Write16(aDdReSs, aValue) ;
				}
			static inline void Modify(const TRegValue16 aClearMask, const TRegValue16 aSetMask)
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				AsspRegister::Modify16(aDdReSs, aClearMask, aSetMask) ;
				}
			} ;

		template <TLinAddr aDdReSs>
		class TReg8_RW : public TReg8_R<aDdReSs>
			{
		  public :
			static inline void Write(const TRegValue8 aValue)
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				AsspRegister::Write8(aDdReSs, aValue) ;
				}
			static inline void Modify(const TRegValue8 aClearMask, const TRegValue8 aSetMask)
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				AsspRegister::Modify8(aDdReSs, aClearMask, aSetMask) ;
				}
			} ;

		/**
		An interface template for write-only registers.
		*/
		template <TLinAddr aDdReSs>
		class TReg32_W
			{
		  public :
			static inline void Write(const TRegValue aValue)
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				AsspRegister::Write32(aDdReSs, aValue) ;
				}
			} ;

		template <TLinAddr aDdReSs>
		class TReg16_W
			{
		  public :
			static inline void Write(const TRegValue16 aValue)
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				AsspRegister::Write16(aDdReSs, aValue) ;
				}
			} ;

		template <TLinAddr aDdReSs>
		class TReg8_W
			{
		  public :
			static inline void Write(const TRegValue8 aValue)
				{
				__ASSERT_COMPILE((TLinearCheck<aDdReSs>::KIsMapped)) ;
				AsspRegister::Write8(aDdReSs, aValue) ;
				}
			} ;

		/** Class for registers that have dynamic base address */
		template <class T, TUint OfFsEt>
		class TDynReg8_R
			{
		  public :
			static inline TRegValue8 Read( const T& aOwner )
				{
				return AsspRegister::Read8( aOwner.Base() + OfFsEt ) ;
				}
			} ;

		template <class T, TUint OfFsEt>
		class TDynReg16_R
			{
		  public :
			static inline TRegValue16 Read( const T& aOwner )
				{
				return AsspRegister::Read16( aOwner.Base() + OfFsEt ) ;
				}
			} ;

		template <class T, TUint OfFsEt>
		class TDynReg32_R
			{
		  public :
			static inline TRegValue32 Read( const T& aOwner )
				{
				return AsspRegister::Read32( aOwner.Base() + OfFsEt ) ;
				}
			} ;


		template <class T, TUint OfFsEt>
		class TDynReg8_RW : public TDynReg8_R<T, OfFsEt>
			{
		  public :
			static inline void Write( T& aOwner, const TRegValue8 aValue)
				{
				AsspRegister::Write8( aOwner.Base() + OfFsEt, aValue) ;
				}
			static inline void Modify( T& aOwner, const TRegValue8 aClearMask, const TRegValue8 aSetMask)
				{
				AsspRegister::Modify8( aOwner.Base() + OfFsEt, aClearMask, aSetMask) ;
				}
			} ;

		template <class T, TUint OfFsEt>
		class TDynReg16_RW : public TDynReg16_R<T, OfFsEt>
			{
		  public :
			static inline void Write( T& aOwner, const TRegValue16 aValue)
				{
				AsspRegister::Write16( aOwner.Base() + OfFsEt, aValue) ;
				}
			static inline void Modify( T& aOwner, const TRegValue16 aClearMask, const TRegValue16 aSetMask)
				{
				AsspRegister::Modify16( aOwner.Base() + OfFsEt, aClearMask, aSetMask) ;
				}
			} ;
		
		template <class T, TUint OfFsEt>
		class TDynReg32_RW : public TDynReg32_R<T, OfFsEt>
			{
		  public :
			static inline void Write( T& aOwner, const TRegValue32 aValue)
				{
				AsspRegister::Write32( aOwner.Base() + OfFsEt, aValue) ;
				}
			static inline void Modify( T& aOwner, const TRegValue32 aClearMask, const TRegValue32 aSetMask)
				{
				AsspRegister::Modify32( aOwner.Base() + OfFsEt, aClearMask, aSetMask) ;
				}
			} ;

		template <class T, TUint OfFsEt>
		class TDynReg8_W
			{
		  public :
			static inline void Write( T& aOwner, const TRegValue8 aValue)
				{
				AsspRegister::Write8( aOwner.Base() + OfFsEt, aValue) ;
				}
			static inline void Modify( T& aOwner, const TRegValue8 aClearMask, const TRegValue8 aSetMask)
				{
				AsspRegister::Modify8( aOwner.Base() + OfFsEt, aClearMask, aSetMask) ;
				}
			} ;

		template <class T, TUint OfFsEt>
		class TDynReg16_W
			{
		  public :
			static inline void Write( T& aOwner, const TRegValue16 aValue)
				{
				AsspRegister::Write16( aOwner.Base() + OfFsEt, aValue) ;
				}
			static inline void Modify( T& aOwner, const TRegValue16 aClearMask, const TRegValue16 aSetMask)
				{
				AsspRegister::Modify16( aOwner.Base() + OfFsEt, aClearMask, aSetMask) ;
				}
			} ;
		
		template <class T, TUint OfFsEt>
		class TDynReg32_W
			{
		  public :
			static inline void Write( T& aOwner, const TRegValue32 aValue)
				{
				AsspRegister::Write32( aOwner.Base() + OfFsEt, aValue) ;
				}
			static inline void Modify( T& aOwner, const TRegValue32 aClearMask, const TRegValue32 aSetMask)
				{
				AsspRegister::Modify32( aOwner.Base() + OfFsEt, aClearMask, aSetMask) ;
				}
			} ;

			/**
		An Null class for when no register access is required.
		*/
		class TNull_Reg
			{
		  public :
			static inline TRegValue Read()
				{
				return 0 ;
				}
			static inline void Write(const TRegValue)
				{
				}
			static inline void Modify(const TRegValue, const TRegValue)
				{
				}
			} ;

		template <int aBiTpOsItIoN>
		class TBit
			{
		  public :
			enum	TConstants
				{
				KValue	= (1 << aBiTpOsItIoN)
				} ;
			} ;

		template <int aBiTpOsItIoN, int aBiTwIdTh>
		class TBitFieldBase
			{
		  public :
			enum	TConstants
				{
				KShift		= aBiTpOsItIoN,
				KValueMask	= (TBit<aBiTwIdTh>::KValue - 1),
				KFieldMask	= (KValueMask << KShift),
				KValueMax	= KValueMask
				} ;
			} ;

		template <int aBiTpOsItIoN, int aBiTwIdTh, int aVaLuE>
		class TBitFieldValue : public TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>
			{
		  public :
			using TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>::KShift ;
			using TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>::KValueMask ;
			using TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>::KFieldMask ;
			using TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>::KValueMax ;

			enum	TValues
				{
				KValue	= ((KValueMask & aVaLuE) << KShift)
				} ;
			} ;

		template <int aBiTpOsItIoN, int aBiTwIdTh>
		class TBitField : public TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>
			{
		  public :
			using TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>::KShift ;
			using TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>::KValueMask ;
			using TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>::KFieldMask ;
			using TBitFieldBase<aBiTpOsItIoN, aBiTwIdTh>::KValueMax ;

			template <int aVaLuE>
			class	TConstVal : public TBitFieldValue<aBiTpOsItIoN, aBiTwIdTh, aVaLuE>
				{
			  public :
				using TBitFieldValue<aBiTpOsItIoN, aBiTwIdTh, aVaLuE>::KValue ;
				} ;

			inline TBitField(const TRegValue aValue)
				  : iValue((KValueMask & aValue) << KShift) {}

			inline TBitField(const TRegValue * aValuePtr)
				  : iValue(KFieldMask & *aValuePtr) {}

			template <TLinAddr aDdReSs>
			inline TBitField(const TReg32_R<aDdReSs>& aReg)
				  : iValue(KFieldMask & aReg.Read()) {}

			inline TRegValue Value() const {return (KValueMask & (iValue >> KShift)) ;}

			inline TRegValue RegField() const {return (iValue) ;}

		  private :
			TRegValue			iValue ;
			} ;

		template <int aBiTpOsItIoN>
		class TSingleBitField : public TBitField<aBiTpOsItIoN, 1>
			{
		  public :
			enum TConstants
				{
				KOff	= 0,
				KOn		= (1 << aBiTpOsItIoN),
				KClear	= KOff,
				KSet	= KOn,
				KMask	= KOn,
				} ;
			} ;

		} ;	// namespace Omap3530

	} ;	// namespace TexasInstruments

	
namespace TI = TexasInstruments ;

namespace OMAP3530 = TexasInstruments::Omap3530 ;

namespace Omap3530HwBase = TexasInstruments::Omap3530 ;

// **** TEST CODE ****
//#	define HEADER_OMAP3530_HARDWARE_BASE_H_DO_COMPILE_TIME_CHECK_TESTS 1
#	ifdef HEADER_OMAP3530_HARDWARE_BASE_H_DO_COMPILE_TIME_CHECK_TESTS
		inline void CompileTimeChecks(void)
			{
			__ASSERT_COMPILE((Omap3530HwBase::TVirtual<0x48318000>::KIsL4Core)) ;
			__ASSERT_COMPILE((TI::Omap3530::TVirtual<0x48318000>::KIsL4WakeUp)) ;
			__ASSERT_COMPILE((!Omap3530HwBase::TVirtual<0x48318000>::KIsL4Emu)) ;
			__ASSERT_COMPILE((!Omap3530HwBase::TVirtual<0x0000FFFF>::KIsConvertable)) ;
			__ASSERT_COMPILE((Omap3530HwBase::TLinearCheck< Omap3530HwBase::TVirtual<0x48318000>::Value >::KIsMapped)) ;
			__ASSERT_COMPILE((!Omap3530HwBase::TLinearCheck< Omap3530HwBase::TVirtual<0x0000FFFF>::Value >::KIsMapped)) ;
			const TLinAddr	mapped(Omap3530HwBase::TVirtual<0x48318000>::Value) ;
			const TLinAddr	unmapped(Omap3530HwBase::TVirtual<0x0000FFFF>::Value) ;
			__ASSERT_COMPILE((Omap3530HwBase::TLinearCheck< mapped >::KIsMapped)) ;
			__ASSERT_COMPILE((!Omap3530HwBase::TLinearCheck< unmapped >::KIsMapped)) ;
			__ASSERT_COMPILE((0)) ;		// Prove that testing is happening
			}
#	endif

const TUint KSetNone = 0;
const TUint KSetAll = 0xffffffff;
const TUint KClearNone = 0;
const TUint KClearAll = 0xffffffff;
const TUint KHOmapClkULPD48Mhz = 48000000;
		
#endif // !OMAP3530_HARDWARE_BASE_H__


		
