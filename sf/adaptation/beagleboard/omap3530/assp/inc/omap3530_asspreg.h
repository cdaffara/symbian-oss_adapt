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

#if 0 // not needed for Symbian^3

#ifndef __ASSPREG_H__
#define __ASSPREG_H__

/**
@publishedPartner
@prototype

A class that exports ASSP register access functionality to 
device drivers and other kernel-side code. 

Although Symbian OS defines this class, it does not implement all 
the functions within it. An implementation for each of the register 
modification functions defined by this class must be provided by 
the baseport.
*/	
class AsspRegister
	{
public:

	/**
	Return the contents of an 8-bit register.

	@param aAddr        The address of the register to be read.
	@return             The contents of the register.
	@pre                Can be called in any context.
	*/
	static inline TUint8 Read8(TLinAddr aAddr)
		{ return *(volatile TUint8*)aAddr; }

	/**
	Store a new value in an 8-bit register. This will change
	the entire contents of the register concerned.

	@param aAddr        The address of the register to be written.
	@param aValue       The new value to be written to the register.
	@pre                Can be called in any context.
	*/
	static inline void Write8(TLinAddr aAddr, TUint8 aValue)
		{ *(volatile TUint8*)aAddr = aValue; }

	/**
	Modify the contents of an 8-bit register.

	@param aAddr        The address of the register to be modified.
	@param aClearMask   A mask of the bits to be cleared in the register.
	@param aSetMask     A mask of the bits to be set in the register after the clear.
	@pre                Can be called in any context.
	*/
	IMPORT_C static void Modify8(TLinAddr aAddr, TUint8 aClearMask, TUint8 aSetMask);

	/**
	Return the contents of an 16-bit register.

	@param aAddr        The address of the register to be read.
	@return             The contents of the register.
	@pre                Can be called in any context.
	*/
	static inline TUint16 Read16(TLinAddr aAddr)
		{ return *(volatile TUint16*)aAddr; }

	/**
	Store a new value in a 16-bit register. This will change
	the entire contents of the register concerned.

	@param aAddr        The address of the register to be written.
	@param aValue       The new value to be written to the register.
	@pre                Can be called in any context.
	*/
	static inline void Write16(TLinAddr aAddr, TUint16 aValue)
		{ *(volatile TUint16*)aAddr = aValue; }

	/**
	Modify the contents of a 16-bit register.

	@param aAddr        The address of the register to be modified.
	@param aClearMask   A mask of the bits to be cleared in the register.
	@param aSetMask     A mask of the bits to be set in the register after the clear.
	@pre                Can be called in any context.
	*/
	IMPORT_C static void Modify16(TLinAddr aAddr, TUint16 aClearMask, TUint16 aSetMask);

	/**
	Return the contents of a 32-bit register.

	@param aAddr        The address of the register to be read.
	@return             The contents of the register.
	@pre                Can be called in any context.
	*/
	static inline TUint32 Read32(TLinAddr aAddr)
		{ return *(volatile TUint32*)aAddr; }

	/**
	Store a new value in a 32-bit register. This will change
	the entire contents of the register concerned.

	@param aAddr        The address of the register to be written.
	@param aValue       The new value to be written to the register.
	@pre                Can be called in any context.
	*/
	static inline void Write32(TLinAddr aAddr, TUint32 aValue)
		{ *(volatile TUint32*)aAddr = aValue; }

	/**
	Modify the contents of a 32-bit register.

	@param aAddr        The address of the register to be modified.
	@param aClearMask   A mask of the bits to be cleared in the register.
	@param aSetMask     A mask of the bits to be set in the register after the clear.
	@pre                Can be called in any context.
	*/
	IMPORT_C static void Modify32(TLinAddr aAddr, TUint32 aClearMask, TUint32 aSetMask);

	/**
	Return the contents of a 64-bit register.

	@param aAddr        The address of the register to be read.
	@return             The contents of the register.
	@pre                Can be called in any context.
	*/
	IMPORT_C static TUint64 Read64(TLinAddr aAddr);

	/**
	Store a new value in a 64-bit register. This will change
	the entire contents of the register concerned.

	@param aAddr        The address of the register to be written.
	@param aValue       The new value to be written to the register.
	@pre                Can be called in any context.
	*/
	IMPORT_C static void Write64(TLinAddr aAddr, TUint64 aValue);

	/**
	Modify the contents of a 64-bit register.

	@param aAddr        The address of the register to be modified.
	@param aClearMask   A mask of the bits to be cleared in the register.
	@param aSetMask     A mask of the bits to be set in the register after the clear.
	@pre                Can be called in any context.
	*/
	IMPORT_C static void Modify64(TLinAddr aAddr, TUint64 aClearMask, TUint64 aSetMask);
	};

#endif
#endif
