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

#ifndef OMAP3530_HARDWARE_BASE_INL__
#	define OMAP3530_HARDWARE_BASE_INL__


// **** TEST CODE ****
//#	define HEADER_OMAP3530_HARDWARE_BASE_H_DO_COMPILE_TIME_CHECK_TESTS 1
#	ifdef HEADER_OMAP3530_HARDWARE_BASE_H_DO_COMPILE_TIME_CHECK_TESTS
#		warning HEADER_OMAP3530_HARDWARE_BASE_H_DO_COMPILE_TIME_CHECK_TESTS defined in __FILE__
namespace OMAP3530
	{
			
	namespace CompileTimeTest
		{

		inline void CompileTimeChecks(void)
			{
			__ASSERT_COMPILE((OMAP3530::TBit<0>::KValue == 0x00000001)) ;
			__ASSERT_COMPILE((OMAP3530::TBit<1>::KValue == 0x00000002)) ;
			__ASSERT_COMPILE((OMAP3530::TBit<30>::KValue == 0x40000000)) ;
			__ASSERT_COMPILE((OMAP3530::TBit<31>::KValue == 0x80000000u)) ;
			
			__ASSERT_COMPILE((0 == OMAP3530::TBitFieldBase<0, 1>::KShift)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<0, 1>::KValueMask)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<0, 1>::KFieldMask)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<0, 1>::KValueMax)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<0, 1>::KFieldMax)) ;
			
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<1, 1>::KShift)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<1, 1>::KValueMask)) ;
			__ASSERT_COMPILE((0x00000002 == OMAP3530::TBitFieldBase<1, 1>::KFieldMask)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<1, 1>::KValueMax)) ;
			__ASSERT_COMPILE((0x00000002 == OMAP3530::TBitFieldBase<1, 1>::KFieldMax)) ;
			
			__ASSERT_COMPILE((16 == OMAP3530::TBitFieldBase<16, 1>::KShift)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<16, 1>::KValueMask)) ;
			__ASSERT_COMPILE((0x00010000 == OMAP3530::TBitFieldBase<16, 1>::KFieldMask)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<16, 1>::KValueMax)) ;
			__ASSERT_COMPILE((0x00010000 == OMAP3530::TBitFieldBase<16, 1>::KFieldMax)) ;
			
			__ASSERT_COMPILE((30 == OMAP3530::TBitFieldBase<30, 1>::KShift)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<30, 1>::KValueMask)) ;
			__ASSERT_COMPILE((0x40000000 == OMAP3530::TBitFieldBase<30, 1>::KFieldMask)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<30, 1>::KValueMax)) ;
			__ASSERT_COMPILE((0x40000000 == OMAP3530::TBitFieldBase<30, 1>::KFieldMax)) ;
			
			__ASSERT_COMPILE((31 == OMAP3530::TBitFieldBase<31, 1>::KShift)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<31, 1>::KValueMask)) ;
			__ASSERT_COMPILE((0x80000000 == OMAP3530::TBitFieldBase<31, 1>::KFieldMask)) ;
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<31, 1>::KValueMax)) ;
			__ASSERT_COMPILE((0x80000000 == OMAP3530::TBitFieldBase<31, 1>::KFieldMax)) ;
			
			__ASSERT_COMPILE((0 == OMAP3530::TBitFieldBase<0, 3>::KShift)) ;
			__ASSERT_COMPILE((7 == OMAP3530::TBitFieldBase<0, 3>::KValueMask)) ;
			__ASSERT_COMPILE((7 == OMAP3530::TBitFieldBase<0, 3>::KFieldMask)) ;
			__ASSERT_COMPILE((7 == OMAP3530::TBitFieldBase<0, 3>::KValueMax)) ;
			__ASSERT_COMPILE((7 == OMAP3530::TBitFieldBase<0, 3>::KFieldMax)) ;
			
			__ASSERT_COMPILE((1 == OMAP3530::TBitFieldBase<1, 2>::KShift)) ;
			__ASSERT_COMPILE((3 == OMAP3530::TBitFieldBase<1, 2>::KValueMask)) ;
			__ASSERT_COMPILE((0x00000006 == OMAP3530::TBitFieldBase<1, 2>::KFieldMask)) ;
			__ASSERT_COMPILE((3 == OMAP3530::TBitFieldBase<1, 2>::KValueMax)) ;
			__ASSERT_COMPILE((0x00000006 == OMAP3530::TBitFieldBase<1, 2>::KFieldMax)) ;
			
			__ASSERT_COMPILE((29 == OMAP3530::TBitFieldBase<29, 3>::KShift)) ;
			__ASSERT_COMPILE((7 == OMAP3530::TBitFieldBase<29, 3>::KValueMask)) ;
			__ASSERT_COMPILE((0xE0000000 == OMAP3530::TBitFieldBase<29, 3>::KFieldMask)) ;
			__ASSERT_COMPILE((7 == OMAP3530::TBitFieldBase<29, 3>::KValueMax)) ;
			__ASSERT_COMPILE((0xE0000000 == OMAP3530::TBitFieldBase<29, 3>::KFieldMax)) ;
			
			__ASSERT_COMPILE((0x00000000 == OMAP3530::TBitFieldValue<15, 3, 0>::KValue)) ;
			__ASSERT_COMPILE((0x00008000 == OMAP3530::TBitFieldValue<15, 3, 1>::KValue)) ;
			__ASSERT_COMPILE((0x00038000 == OMAP3530::TBitFieldValue<15, 3, 7>::KValue)) ;
			
			typedef	OMAP3530::TBitField<8, 2>	TBitField89 ;
			__ASSERT_COMPILE((0 == TBitField89::TConstVal<0>::KValue)) ;
			__ASSERT_COMPILE((0x00000100 == TBitField89::TConstVal<1>::KValue)) ;
			__ASSERT_COMPILE((0x00000300 == TBitField89::TConstVal<3>::KValue)) ;
			
			typedef OMAP3530::TSingleBitField<0>	TBit0 ;
			__ASSERT_COMPILE((0 == TBit0::KOff)) ;
			__ASSERT_COMPILE((1 == TBit0::KOn)) ;
			typedef OMAP3530::TSingleBitField<1>	TBit1 ;
			__ASSERT_COMPILE((0 == TBit1::KOff)) ;
			__ASSERT_COMPILE((2 == TBit1::KOn)) ;
			typedef OMAP3530::TSingleBitField<16>	TBit16 ;
			__ASSERT_COMPILE((0 == TBit16::KOff)) ;
			__ASSERT_COMPILE((0x00010000 == TBit16::KOn)) ;
			typedef OMAP3530::TSingleBitField<31>	TBit31 ;
			__ASSERT_COMPILE((0 == TBit31::KOff)) ;
			__ASSERT_COMPILE((0x80000000 == TBit31::KOn)) ;
			
			__ASSERT_COMPILE((Omap3530HwBase::TVirtual<0x48318000>::KIsL4Core)) ;
			__ASSERT_COMPILE((OMAP3530::TVirtual<0x48318000>::KIsL4WakeUp)) ;
			__ASSERT_COMPILE((!OMAP3530::TVirtual<0x48318000>::KIsL4Emu)) ;
			__ASSERT_COMPILE((!OMAP3530::TVirtual<0x0000FFFF>::KIsConvertable)) ;
			__ASSERT_COMPILE((OMAP3530::TLinearCheck< OMAP3530::TVirtual<0x48318000>::Value >::KIsMapped)) ;
			__ASSERT_COMPILE((!OMAP3530::TLinearCheck< OMAP3530::TVirtual<0x0000FFFF>::Value >::KIsMapped)) ;
			const TLinAddr	mapped(OMAP3530::TVirtual<0x48318000>::Value) ;
			const TLinAddr	unmapped(OMAP3530::TVirtual<0x0000FFFF>::Value) ;
			__ASSERT_COMPILE((OMAP3530::TLinearCheck< mapped >::KIsMapped)) ;
			__ASSERT_COMPILE((!OMAP3530::TLinearCheck< unmapped >::KIsMapped)) ;
			__ASSERT_COMPILE((0)) ;		// Prove that testing is happening
			}
			
		} ;	// namespace CompileTimeTest
		
	} ;	// namespace OMAP3530
#	endif

#endif // !OMAP3530_HARDWARE_BASE_INL__
