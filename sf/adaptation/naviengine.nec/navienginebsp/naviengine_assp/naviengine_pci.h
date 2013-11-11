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
* naviengine_assp\naviengine_pci.h
*
*/



#ifndef __NAVIENGINE_PCI_H__ 
#define __NAVIENGINE_PCI_H__ 
//----------------------------------------------------------------------------
//PCI Definitions
//----------------------------------------------------------------------------

//Register Offsets
const TUint KHoPciVid				= 0x0;		//Vendor Id
const TUint KHoPciDid				= 0x2;		//Device Id
const TUint KHoPciCmd				= 0x04;		//PCI Command Register
const TUint KHoPciStatus			= 0x06;		//PCI Status

const TInt KNeBridgeNumberOfBars	=12;
const TUint KHoBar[KNeBridgeNumberOfBars] = //Defines base address of a window to access AHB space from PCI space
	{
	0x10, //0
	0x14, //1
   	0x18, //2
   	0x40, //3
	0x48, //4
   	0x50, //5
   	0x58, //6
   	0x60, //7
	0x68, //8
	0x80, //9
	0x90, //10
	0x98  //11
	};
//Each ACR controls size and address conversion for its associated BAR
const TUint KHoAcr[KNeBridgeNumberOfBars] =
	{
	0xA0, //0
	0xA4, //1
	0xA8, //2
	0x44, //3
	0x4C, //4
	0x54, //5
	0x5C, //6
	0x64, //7
	0x6C, //8
	0x84, //9
	0x94, //10
	0x9C  //11
	};

const TUint KHoError1		= 0xC0;		//Error Register 1
const TUint KHoPciCtrlH		= 0xE4;		//PCI Control Register Hi
const TUint KHoBarEnable	= 0xEC;		// PCIBAREn 
const TUint KHoCnfig_addr 	= 0xF8; 	//PCI Config Address
const TUint KHoCnfig_data 	= 0xFC; 	//PCI Config Data

//Bit masks

//KPcicmd
const TUint16 KHtPcicmd_Memen		=  KBit1; //Specifies whether the PCI interface is to acknowkedge a memory access as a PCI target.
const TUint16 KHtPcicmd_Bmasen		=  KBit2; //Specifies whether the PCI interface is to operate as the PCI master. 
const TUint16 KHtPcicmd_Peren		=  KBit6; // Controls the operation of the device in case of a parity error. 1:React, 0:Ignore
const TUint16 KHtPcicmd_Seren		=  KBit8; // Controls the operation of the device in case of a system error. 1:React, 0:Ignore

//KPciStatus
const TUint16 KHtPciStatus_ParityError		=  KBit15;
const TUint16 KHtPciStatus_SystemError		=  KBit14;
const TUint16 KHtPciStatus_MasterAbrtRcvd 	=  KBit13;
const TUint16 KHtPciStatus_TargetAbrtRcvd 	=  KBit12;
const TUint16 KHtPciStatus_STA				=  KBit11;
const TUint16 KHmPciStatus_DevSe			=  KBit10|KBit9;
const TUint16 KHtPciStatus_DPErrorAsserted	=  KBit8;
const TUint16 KHtPciStatus_FBBC				=  KBit7;
const TUint16 KHtPciStatus_UDF 				=  KBit6;


//KAcr(N)
const TUint32 KHmAcr_BarMask				=  KBit8|KBit7|KBit6|KBit5|KBit4;
const TUint32 KHtAcr_P2Ace					=  KBit0;	//Decides whether or not PCI address should be translated before AHB access.

//KError1
const TUint32 KHtError1_PEEn				=  KBit10;	// If system error occurs on PCI bus report on the AHB64PCI_ERR pin
const TUint32 KHtError1_AMEn				=  KBit9;	//Assert AHB64PCI_ERR if AHB master receives error response.
const TUint32 KHtError1_SystemError			=  KBit6;	//System error has occurred
const TUint32 KHtError1_AMEr				=  KBit5;	//AHB Master has recieved error response

//KPciCtrlH
const TUint32 KHtPciCtrlH_CnfigDone 		=  KBit28;
const TUint32 KHtPciCtrlH_Aper 				=  KBit21; // Asserts if an address parity error is detected.
const TUint32 KHtPciCtrlH_Dtep 				=  KBit20; // Asserts if the time of the discard timer is out
const TUint32 KHtPciCtrlH_Dper 				=  KBit19; // Asserts if a data parity error occurs
const TUint32 KHtPciCtrlH_Rlex 				=  KBit18; // Asserts if the retry limit is exceed
const TUint32 KHtPciCtrlH_Mabo 				=  KBit17; // Asserts if the PCI interface generates a master abort signal as the PCI master
const TUint32 KHtPciCtrlH_Tabo 				=  KBit16; // Asserts if the PCI interface detects a target abort signal as the PCI master
const TUint32 KHtPciCtrlH_Aerse 			=  KBit13;
const TUint32 KHtPciCtrlH_Dtimse 			=  KBit12;
const TUint32 KHtPciCtrlH_Perse 			=  KBit11;
const TUint32 KHtPciCtrlH_Rtyse 			=  KBit10;
const TUint32 KHtPciCtrlH_Mase 				=  KBit9;
const TUint32 KHtPciCtrlH_Tase 				=  KBit8;

namespace Initiator
	{
	const TUint	KHoReg1	= 0xF0;		// Pci Initiator 1 - configure how to map AHB-->PCI
	const TUint KHoReg2	= 0xF4; 	// Pci Initiator 2 - configure how to map AHB-->PCI

	//! The upper bits of inbound AHB address are converted to this value
	const TUint32 KHmA2PCA					= 0xFFFFFC00;
	const TUint32 KHmA2PCAMask				= 0x1F0;
	const TUint32 KHmType					= 0xE;
	const TUint32 KHtConvEnable				= KBit0;

	const TUint32 KHsA2PCA					= 9;
	const TUint32 KHsA2PCAMask				= 4;
	const TUint32 KHsType					= 1;
	const TUint32 KHsConvEnable				= 0;
	}

namespace ConfigAddress
	{
	const TUint32 KHtCnfigEnable		= KBit31;	// Must be set for bridge to allow access to KHoCnfig_data register.

	const TUint32 KHmBus				= 0xFF0000;	// Bits 23:16
	const TUint32 KHmDevice				= 0xF800;	// Bits 15:11
	const TUint32 KHmFunction			= 0x700;	// Bits 10:8
	const TUint32 KHmOffset				= 0xFC;		// Bits 7:2

	const TUint32 KHsBus				= 16; 
	const TUint32 KHsDevice				= 11;
	const TUint32 KHsFunction           = 8;
	const TUint32 KHsOffset				= 2;
	}

const TUint32 KHwUSBHInternalPciWindowSize = 0x2000;  // Size of the system to PCI window at KHwUSBHPhys
#endif // __NAVIENGINE_PCI_H__ 
