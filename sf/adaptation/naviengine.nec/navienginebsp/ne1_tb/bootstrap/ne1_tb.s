;
; Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
; All rights reserved.
; This component and the accompanying materials are made available
; under the terms of "Eclipse Public License v1.0"
; which accompanies this distribution, and is available
; at the URL "http://www.eclipse.org/legal/epl-v10.html".
;
; Initial Contributors:
; Nokia Corporation - initial contribution.
;
; Contributors:
;
; Description:  
; ne1_tb/bootstrap/ne1_tb.s
; NE1_TBVariant for platform specific boot code
;

		GBLL	__VARIANT_S__		; indicates that this is platform-specific code
		GBLL	__NE1_TB_S__		; indicates which source file this is

		INCLUDE	bootcpu.inc
		INCLUDE	arm_types.inc
		INCLUDE naviengine.inc

	IF	CFG_DebugBootRom
	GBLL					CFG_InitDebugPort
	ENDIF
	INIT_LOGICAL_SYMBOL		CFG_InitDebugPort

	IF :DEF: CFG_AlternateEntryPoint; NOTE: Bootloader is defined by this macro
	IMPORT	GetCoreldr
	ENDIF
	
;*******************************************************************************
;
; Platform specific constant definitions

	IF :DEF: CFG_AlternateEntryPoint :LOR: CFG_MMDirect
RamBank0MaxSize		EQU		0x08000000 ; for bootloader or direct, the upper half is reserverd for the image
	ELSE
RamBank0MaxSize		EQU		0x10000000
	ENDIF

PrimaryRomBase		EQU		0x00000000
PrimaryRomSize		EQU		0x04000000
ExtensionRomBase	EQU		0x08000000
ExtensionRomSize	EQU		0x00000000


;*******************************************************************************

; ASSP Specific constants

	IF :LNOT: CFG_MMDirect

;Serial Ports Linear Addresses:

Serial0LinBase		EQU		KPrimaryIOBase
Serial1LinBase		EQU		KPrimaryIOBase + (KHwUART1Phys - KHwUART0Phys)

	ENDIF

;Serial Port Register Offsets
Serial_DLL			EQU		0
Serial_DLH			EQU		4
Serial_LCR			EQU		0xc
Serial_THR			EQU		0
Serial_LSR			EQU		0x14


	IF :LNOT: CFG_MMDirect

SysCtrlUnitLinBase  		EQU		KPrimaryIOBase + (0x1000*3)+0x1c00
												; ... as the value in ne1_tb_bootloader\inc\bootloader_variantconfig.h
	ENDIF												

;*******************************************************************************
; There are 4 red LEDs that can be controlled by a 16 bit register in the FPGA.
; Each LED has a bit in the register, where 0==off and 1==on
; We use each register to show the CPU status, where:
;    off is "not started"
;    on  is "started"
; once the system is running, the OS may use the LEDs for other purposes
;*******************************************************************************
KHwFpgaLedsPhyicalBase      EQU   KHwFPGAPhys                + KHoFpgaLeds;

	IF :LNOT: CFG_MMDirect
KHwFpgaLedsLinBase          EQU   KPrimaryIOBase   + 0xB000  + KHoFpgaLeds;
	ENDIF												


;*******************************************************************************
; struct SSmrIF
; interface between Core Loader and Bootstrap
; See: bootstrap_smrif.h
;*******************************************************************************
SSmrIF_iNumOfShwPart                    EQU 0x00000000
SSmrIF_iSmribSize                       EQU 0x00000004
SSmrIF_iSmrBanks                        EQU 0x00000008
SSmrIF_sz                               EQU 0x0000000c



;*******************************************************************************
;

        AREA |Boot$$Code|, CODE, READONLY, ALIGN=6

;
;*******************************************************************************
;


;***************************************************************************************
; Determine variant required when running on NaviEngine NE1-TB
; Enter with:
;	R12 points to TRomHeader
;	NO STACK
;	R14 = return address (as usual)
;
; Return with:
;	R10 = Super page address
;***************************************************************************************
;
; The SuperPage is placed at the start of the free RAM.
	EXPORT	CalculateSuperPageAddress
CalculateSuperPageAddress
		MOV		r7, lr
		ANDS	r10, pc, #KHwDDR2RamBasePhys	; running from RAM? if not, r10=0
		MOVEQ	r10, #KHwDDR2RamBasePhys		; if running from flash, super page is at base of RAM bank 0
		BLNE	SetupSuperPageRunningFromRAM	; if running from RAM, super page goes after ROM image(s)
		MOV		lr, r7
		MOV		pc, lr

;*******************************************************************************
; Initialise Hardware
;	Initialise CPU registers
;	Determine the hardware configuration
;	Determine the reset reason. If it is wakeup from a low power mode, perform
;		whatever reentry sequence is required and jump back to the kernel.
;	Set up the memory controller so that at least some RAM is available
;	Set R10 to point to the super page or to a temporary version of the super page
;		with at least the following fields valid:
;		iBootTable, iCodeBase, iActiveVariant, iCpuId
;	and optionally:
;			iSmrData
;	In debug builds initialise the debug serial port
;
; Enter with:
;		R2	= value as at entry to ResetEntry, preserved unmodified
;		R3	= value of cpsr on entry to ResetEntry
;		R12 = points to TRomHeader
;		NO STACK
;		R14 = return address (as usual)
;
; Leave with :
;		R10 = physical address of super page 
;
; All registers may be modified by this call
;
;*******************************************************************************
	EXPORT	InitialiseHardware
InitialiseHardware	ROUT
		MOV		r13, lr					; save return address

		MOV		r8, r2					; Preserve r2, r3 (cpsr) till later
		MOV		r9, r3					; S/P initialisation by boot processor

	IF :DEF: CFG_AlternateEntryPoint; NOTE: Bootloader is defined by this macro

		LDR		r1, =KHwSYSCTRLPhys + 0x00C	; Reset status
		LDR		r1,	[r1]
		ANDS	r1, r1, #0x00030000		; soft-reset or hot-reset?
		BNE		changed_wtop_mode

		LDR		r1, =KHwSYSCTRLPhys + 0x11C	; WTOP
		LDR		r0,	[r1]
		ANDS	r0, r0, #0xFFFFFFFE		; clear bit 0b to be normal mode
		STR		r0, [r1]

		LDR		r1, =KHwSYSCTRLPhys + 0x00C	; Reset status
		MOV		r0, #0x00000001
		STR		r0, [r1]			; soft reset is executed

changed_wtop_mode
		; Fix the WFE JTAG problem:  WFE operations disconnected the JTAG debugger
		; 0x18037d08 = 0;  // SCU CPU status = 0
		LDR		r0, =KHwSYSCTRLPhys
		MOV		r1, #0
		STR		r1, [r0, #0x108]
		
		ARM_DSB
		
		; 0xC0000008 = 0;  // SCU CPU status = 0
		LDR		r0, =KHwBaseMPcorePrivatePhys
		MOV		r1, #0
		STR		r1, [r0, #0x08]

		ARM_DSB

		; Check boot reason in the SCU Memo register
		; Using ADRL as when CFG_DebugBootRom defined, SysCtrlUnitPhysicalAdr 
		; beyond range for an ADR instruction.
		ADRL 	r1, SysCtrlUnitPhysicalAdr
		LDR		r1, [r1]
		LDR		r0, [r1]

		; Check if miniboot has run, and we're ready for the coreldr
		TST		r0, #KtRestartReasonCoreLdr
		MOVNE	r5, #KCoreLoaderAddress		; Jump into coreldr, NO RETURN!
		BNE		all_cores_run_r5

		TST		r0,	#KtRestartReasonBootRestart
		BEQ		check_cpu_id
		MOV		r5, #KRamTargetAddr
	
all_cores_run_r5
		; There is something for all cores to run (coreldr or new image)
		; at the address in r5.
		; But, before we start, we need to get CPU0 to initialise the ASSP
		; to make RAM accessible, and reset the Memo register.
		; Otherwise, if the board reboots again (eg. if the user presses the reset switch)
		; then the BootLoader will attempt to boot the RAM image again.  And after the
		; reset switch, RAM is not longer valid.

		; Is this the boot processor ?
		MRC		p15, 0, r0, c0, c0, 5
		ANDS	r0, r0, #0x0f			; r0 = CPU number 0-3
		BNE		skip_init_step			; Branch out if CPU != 0 (!= boot processor)

		; Initialise RAM controller etc. Only if NOT running from RAM
		CMP		PC, #KHwDDR2RamBasePhys
		BLLS	AsspInitialiseHardware

		; Set SCU Memo register using CPU0
		ADR 	r1, SysCtrlUnitPhysicalAdr
		LDR		r1, [r1]
		MOV		r0, #0
		STR		r0, [r1]				; clear restart reason to 0

skip_init_step
		LDR		r0, =KHwSYSCTRLPhys + 0x014 ; Peripheral reset control
		LDR		r1, [r0]
		CMP		r1, #0
		BNE		skip_init_step			; wait for AsspInitialiseHardware to complete

		MOV		pc, r5					; Jump into loaded code, NO RETURN!

	ENDIF

check_cpu_id
		; Is this the boot processor ?
		MRC		p15, 0, r0, c0, c0, 5
		ANDS	r0, r0, #0x0f			; r0 = CPU number 0-3
		BEQ		IsBootProcessor			; Branch out if CPU 0 (= boot processor)

		; No - this is an AP
	IF	SMP
		LDR		r11, =KHwBaseMPcorePrivatePhys	; r11 points to SCU
		ADD		r8, r11, #0x1000		; Physical address of GIC Distributor
		ADD		r9, r11, #0x100			; Physical address of GIC CPU Interface
		B		APResetEntry			; NO RETURN!
	ELSE
1
		mov		r0, #0
		mcr		p15, 0, r0, c7, c0, 4
		B		%BA1					; NO RETURN!
	ENDIF

		; This is the boot processor
IsBootProcessor
		; Initialise RAM controller etc. Only if NOT running from RAM
		CMP		PC, #KHwDDR2RamBasePhys
		BLLS	AsspInitialiseHardware		

	IF :DEF: CFG_AlternateEntryPoint; NOTE: Bootloader is defined by this macro		
		; Check for Alternate boot reasons (other than KtRestartReasonBootRestart)
		ADR		r1, SysCtrlUnitPhysicalAdr
		LDR		r1,	[r1]
		LDR		r0,	[r1]
			
		TST		r0, #KtRestartReasonNANDImage	; Check for specific nand boot restart reason
		BNE		GetCoreldr						; try booting from NAND flash, NO RETURN!
	ENDIF	
	
		; Turn on LED for CPU 0 and turn off LEDs for CPU 1, 2, 3
		GET_ADDRESS	r0, KHwFpgaLedsPhyicalBase, KHwFpgaLedsLinBase
											; r0 is the address of the 16bit LED register in FPGA, aka FLED
		MOV		r1, #KHtFpgaLed0			; Turn on LED for CPU0 and turn off the other LEDs
		STRH	r1, [r0]					; write r1 back to the FLED register

	IF :DEF: CFG_USE_SHARED_MEMORY
		; Switch on Snoop Control Unit.
		; The whole procedure for switching to SMP is:
		;	- Set SCU on. 			: Done here
		;	- Disable INTs			: Alreday disabled
		;	- Flush DCache			: See InitCpu
		;	- Set SMP bit in AUX reg		: See InitCpu
		;	- Enable INTs			: Later on
		MOV		r0, #KHwBaseMPcorePrivatePhys
		MVN		r1, #0
		STR		r1, [r0, #0x0C]			; invalidate all SCU ways
		LDR		r1, [r0]
		ORR		r1, r1, #1				; SCU Enable
		ORR		r1, r1, #0xFE			; Enable all aliases of everything
		ORR		r1, r1, #0x1F
		STR		r1, [r0]
	ENDIF
		
		ADRL	r1, ParameterTable		; pass address of parameter table
		BL		InitCpu					; initialise CPU/MMU registers, r0..r7 modified

	IF	CFG_InitDebugPort
		BL		InitDebugPort			; r0..r2 modified  
	ENDIF


;;;;;;;;;;;;
; Put super page at end of SDRAM for now and set up the required super page values
;;;;;;;;;;;

;        LDR     r10, =(KHwSdramBaseAddr+(KHwRamSizeMb<<20)-0x2000)
		BL		CalculateSuperPageAddress


		LDR		r7, =CFG_HWVD				        ; variant number from config.inc
		STR		r7, [r10, #SSuperPageBase_iActiveVariant]

		MOV		r1, #0
		STR		r1, [r10, #SSuperPageBase_iHwStartupReason]	; reset reason = 0

		ADD		r1, r10, #CpuPageOffset
		STR		r1, [r10, #SSuperPageBase_iMachineData]

		ADRL	r0, BootTable
		STR		r0, [r10, #SSuperPageBase_iBootTable]		; set the boot function table

		STR		r12, [r10, #SSuperPageBase_iCodeBase]		; set the base address of bootstrap code

		MRC		p15, 0, r0, c0, c0, 0						; read CPU ID from CP15
		STR		r0, [r10, #SSuperPageBase_iCpuId]
		
		
		; Process SMRIB from pre-OS Loader and copy to CPU Page.
		;
		; r8 = r2 from ResetEntry = Address of block: <SMRIB size><SMRIB entries><...> 
		; r9 = r3 the CPSR from entry of ResetEntry
		;
		; SMRIB address in r2 only valid when cpsr (r3) shows ResetEntry entered
		; with System CPU mode set. Such a scenario can only be supported from 
		; local media ROM boots. i.e. boot from NAND
		;
  
		AND		r0, r9, #EMaskMode				 	; Check for System CPU mode
		CMP		r0, #ESystemMode
		MVNNE	r0, #0 								; Set iSmrData to KSuperPageAddressFieldUndefined when CPU
		STRNE	r0, [r10, #SSuperPageBase_iSmrData]	; not in system mode before
		BNE		NoSMRIB								; i.e. no SMRIB present/defined
		
		; Proceed to copy SMRIB to Cpu Page at SP+CpuSmrTableOffset
		
		ADD		r1, r10, #CpuSmrTableOffset			; Set the iSmrData member to the SMRIB address 
		STR		r1, [r10, #SSuperPageBase_iSmrData] ; in the CpuPage, see bootdefs.h, space 
													; for 8 SSmrBank records, max. Sets r1 for call to WordMove

        ; preparing r0, r2 for call to WordMove                                                    
        LDR     r0, [r8, #SSmrIF_iSmrBanks]         ; Load SMR Banks starting address from SSmrIF::iSmrBanks
                                                    ; see kernboot.h 
        
        LDR     r2, [r8, #SSmrIF_iSmribSize]        ; Load SMRIB size from SSmrIF::iSmribSize and validate, while

		DWORD	r0, "Variant Bootstrap found SMRIB at"
		DWORD	r2, "With the size of the SMRIB being"
		DWORD	r1, "Will copy to                    "

		CMP		r2, #SSmrBank_sz
		FAULT	LT									; Fault if SMRIB size < 16
		CMP		r2, #CpuSmrTableTop-CpuSmrTableOffset-16	; -16 to allow space for null entry, 7 entries allowed
		FAULT	GT									; Fault if SMRIB size > 112 	

		BL		WordMove							; r0=src addr, r1=dest addr, r2=bytes, modifies r0..r3
													; No need to copy or add zero entry, Cpu page zerod already		  
NoSMRIB


		MOV		pc, r13
		; END of InitialiseHardware()                



;*******************************************************************************
; Initialise Assp H/W (memory controllers)
;
; Enter with :
;		R12 points to ROM header
;		There is no valid stack
;
; Leave with :
;		R0-R2 modified
;		Other registers unmodified
;*******************************************************************************
	EXPORT	AsspInitialiseHardware
AsspInitialiseHardware	ROUT

		ADR		r0,Init_data
1
		LDR		r1,[r0],#4
		LDR		r2,[r0],#4
		CMP		r1, #0
		BEQ		%FT2
		STR		r2,[r1]
		B		%BT1
2
		MOV		pc, r14	; return

Init_data
;*DDR2 Init
	DCD	0x18021044,0x30022123
	DCD 0x18021058,0x00000001
	DCD 0x18021008,0x00000020

;*delay(Reset Status Register dummy write)
    DCD 0x18037C0C,0x00000000
    DCD 0x18021008,0x10000004
    DCD 0x18021008,0x00010002
    DCD 0x18021008,0x00018002
    DCD 0x18021008,0x00008002
    DCD 0x18021008,0X1D480002
    DCD 0x18021008,0x10000004
    DCD 0x18021008,0x00000001
    DCD 0x18021008,0x00000001

;*delay(Reset Status Register dummy write)
    DCD 0x18037C0C,0x00000000
    DCD 0x18037C0C,0x00000000
    DCD 0x18037C0C,0x00000000
    DCD 0x18021008,0x19480002
    DCD 0x18021008,0x01308002
    DCD 0x18021008,0x00000100
    DCD 0x18021040,0x1485A912
    DCD 0x18021034,0x00000121

;*SysCon Init
;*  .word 0x18037C80,0x007F0103

    DCD 0x18037C80,0x00000000

;*ExBus Init
    DCD 0x1801A000,0x0000004A
    DCD 0x1801A004,0x08000049
    DCD 0x1801A008,0x0600004E
    DCD 0x1801A00C,0x0400004B
    DCD 0x1801A010,0x1000004A
    DCD 0x1801A014,0x1400000A
    DCD 0x1801A020,0x10388E7F
    DCD 0x1801A024,0x10388E7E
    DCD 0x1801A028,0x10388E7E
    DCD 0x1801A02C,0x10388E7F
    DCD 0x1801A030,0x10388E7E
    DCD 0x1801A034,0x10388E7E

;*ExBus PCS5 UART-EX Init
	DCD 0x14020003,0x00
	DCD 0x14020001,0x00
	DCD 0x14020002,0x07
	DCD 0x14020003,0x80
	DCD 0x14020000,0x1E
	DCD 0x14020001,0x00
	DCD 0x14020003,0x03
	DCD 0x14020004,0x03
	
;*ExBus PCS5 CharLED
	DCD 0x14000000,0x59
	DCD 0x14000001,0x45
	DCD 0x14000002,0x53
	DCD 0x14000003,0x21
	DCD 0x14000004,0x21
	DCD 0x14000005,0x20
	DCD 0x14000006,0x20
	DCD 0x14000007,0x20
	
;*ExBus PCS4 LED
	DCD 0x10000030,0x00AA

    DCD 0x18037C14,0x00000000; reset release for all peripheral units
							 ; other cores are waiting for this, must be last
		
;*End
    DCD 0x0,		  0x0


;*******************************************************************************
; Notify an unrecoverable error during the boot process
;
; Enter with:
;	R14 = address at which fault detected
;
; Don't return
;*******************************************************************************
	EXPORT	Fault
Fault	ROUT
		B		BasicFaultHandler	; generic handler dumps registers via debug
									; serial port





;*******************************************************************************
; Reboot the system
; This function assumes that CPU#0 is running !!!
;
; Enter with:
;		R0 = reboot reason code
;
; Don't return (of course)
;*******************************************************************************
	ALIGN	32, 0
	EXPORT	RestartEntry
RestartEntry	ROUT

		; Save R0 parameter in HW dependent register which is preserved over reset
		GET_ADDRESS	r1, KHwSYSCTRLPhys, SysCtrlUnitLinBase
		STR		r0, [r1]

		; Set SFTRSTP to reset all peripherals and all cores
		MOV		r0, #1
		STR		r0, [r1, #0xC]
		SUB		pc, pc, #8

SysCtrlUnitPhysicalAdr
		DCD KHwSYSCTRLPhys
LedPhysicalAdr
		DCD 0x4010a06

;*******************************************************************************
; Get a pointer to the list of RAM banks
;
; The pointer returned should point to a list of {BASE; MAXSIZE;} pairs, where
; BASE is the physical base address of the bank and MAXSIZE is the maximum
; amount of RAM which may be present in that bank. MAXSIZE should be a power of
; 2 and BASE should be a multiple of MAXSIZE. The generic code will examine the
; specified range of addresses and determine the actual amount of RAM if any
; present in the bank. The list is terminated by an entry with zero size.
;
; The pointer returned will usually be to constant data, but could equally well
; point to RAM if dynamic determination of the list is required.
;
; Enter with :
;		R10 points to super page
;		R12 points to ROM header
;		R13 points to valid stack
;
; Leave with :
;		R0 = pointer
;		Nothing else modified
;*******************************************************************************
GetRamBanks	ROUT
		ADR		r0, %FT1
		MOV		pc, lr
1
		DCD		KHwDDR2RamBasePhys | RAM_VERBATIM, RamBank0MaxSize
		DCD		0,0				; terminator





;*******************************************************************************
; Get a pointer to the list of ROM banks
;
; The pointer returned should point to a list of entries of SRomBank structures,
; usually declared with the ROM_BANK macro.
; The list is terminated by a zero size entry (four zero words)
;
; ROM_BANK	PB, SIZE, LB, W, T, RS, SS
; PB = physical base address of bank
; SIZE = size of bank
; LB = linear base if override required - usually set this to 0
; W = bus width (ROM_WIDTH_8, ROM_WIDTH_16, ROM_WIDTH_32)
; T = type (see TRomType enum in kernboot.h)
; RS = random speed
; SS = sequential speed
;
; Only PB, SIZE, LB are used by the rest of the bootstrap.
; The information given here can be modified by the SetupRomBank call, if
; dynamic detection and sizing of ROMs is required.
;
; Enter with :
;		R10 points to super page
;		R12 points to ROM header
;		R13 points to valid stack
;
; Leave with :
;		R0 = pointer
;		Nothing else modified
;*******************************************************************************
GetRomBanks	ROUT
		ADR		r0, %FT1
		MOV		pc, lr
1
	IF	CFG_MMDirect
		ROM_BANK	KRamTargetAddr,		0x08000000,			0, ROM_WIDTH_32, ERomTypeXIPFlash, 0, 0	; image in RAM
	ENDIF
;		ROM_BANK	PrimaryRomBase,		PrimaryRomSize,		0, ROM_WIDTH_32, ERomTypeXIPFlash, 0, 0
;		ROM_BANK	ExtensionRomBase,	ExtensionRomSize,	0, ROM_WIDTH_32, ERomTypeXIPFlash, 0, 0
		DCD		0,0,0,0			; terminator





;*******************************************************************************
; Get a pointer to the list of hardware banks
;
; The pointer returned should point to a list of hardware banks declared with
; the HW_MAPPING and/or HW_MAPPING_EXT macros. A zero word terminates the list.
; For the direct memory model, all hardware on the system should be mapped here
; and the mapping will set linear address = physical address.
; For the moving or multiple model, only the hardware required to boot the kernel
; and do debug tracing needs to be mapped here. The linear addresses used will
; start at KPrimaryIOBase and step up as required with the order of banks in
; the list being maintained in the linear addresses used.
;
; HW_MAPPING PB, SIZE, MULT
;	This declares a block of I/O with physical base PB and address range SIZE
;	blocks each of which has a size determined by MULT. The page size used for
;	the mapping is determined by MULT. The linear address base of the mapping
;	will be the next free linear address rounded up to the size specified by
;	MULT.
;	The permissions used for the mapping are the standard I/O permissions (BTP_Hw).
;
; HW_MAPPING_EXT PB, SIZE, MULT
;	This declares a block of I/O with physical base PB and address range SIZE
;	blocks each of which has a size determined by MULT. The page size used for
;	the mapping is determined by MULT. The linear address base of the mapping
;	will be the next free linear address rounded up to the size specified by
;	MULT.
;	The permissions used for the mapping are determined by a BTP_ENTRY macro
;	immediately following this macro in the HW bank list or by a DCD directive
;	specifying a different standard permission type.
;
; HW_MAPPING_EXT2 PB, SIZE, MULT, LIN
;	This declares a block of I/O with physical base PB and address range SIZE
;	blocks each of which has a size determined by MULT. The page size used for
;	the mapping is determined by MULT. The linear address base of the mapping
;	is specified by the LIN parameter.
;	The permissions used for the mapping are the standard I/O permissions (BTP_Hw).
;
; HW_MAPPING_EXT3 PB, SIZE, MULT, LIN
;	This declares a block of I/O with physical base PB and address range SIZE
;	blocks each of which has a size determined by MULT. The page size used for
;	the mapping is determined by MULT. The linear address base of the mapping
;	is specified by the LIN parameter.
;	The permissions used for the mapping are determined by a BTP_ENTRY macro
;	immediately following this macro in the HW bank list or by a DCD directive
;	specifying a different standard permission type.
;
; Configurations without an MMU need not implement this function.
;
; Enter with :
;		R10 points to super page
;		R12 points to ROM header
;		R13 points to valid stack
;
; Leave with :
;		R0 = pointer
;		Nothing else modified
;*******************************************************************************
GetHwBanks	ROUT
		ADR		r0, %FT1
		MOV		pc, lr

1
	IF	CFG_MMDirect
		HW_MAPPING		KHwLANPhys				,1,	HW_MULT_1M		; LAN, FPGA
		HW_MAPPING		0x18000000				,1,	HW_MULT_1M		; I/O registers @ 18000000
		HW_MAPPING		KHwBaseMPcorePrivatePhys,1,	HW_MULT_1M		; MPCORE private range
	ELSE
		HW_MAPPING		KHwUART0Phys			,1,	HW_MULT_4K		; Mapped at KPrimaryIOBase + 0
		HW_MAPPING		KHwBaseMPcorePrivatePhys,2,	HW_MULT_4K		; Mapped at KPrimaryIOBase + 1000h
		HW_MAPPING		KHwTimer0Phys			,2,	HW_MULT_4K		; Mapped at KPrimaryIOBase + 3000h
		HW_MAPPING		KHwDispPhys				,4,	HW_MULT_4K		; Mapped at KPrimaryIOBase + 5000h
		HW_MAPPING		KHwI2CPhys				,2, HW_MULT_4K		; Mapped at KPrimaryIOBase + 9000h
		HW_MAPPING		KHwFPGAPhys				,1, HW_MULT_4K		; Mapped at KPrimaryIOBase + B000h
		HW_MAPPING		KHwSPDIFPhys			,1, HW_MULT_4K		; Mapped at KPrimaryIOBase + C000h
		HW_MAPPING		KHwSDPhys				,1, HW_MULT_4K		; Mapped at KPrimaryIOBase + D000h
		HW_MAPPING		KHwAHBEXDMACPhys		,6, HW_MULT_4K		; Mapped at KPrimaryIOBase + E000h
		HW_MAPPING		KHwLANPhys				,1, HW_MULT_4K		; Mapped at KPrimaryIOBase + 14000h
		HW_MAPPING		KHwGPIOPhys				,1, HW_MULT_4K		; Mapped at KPrimaryIOBase + 15000h
		HW_MAPPING		KHwAHB32PCI_ExtPhys		,2, HW_MULT_4K		; Mapped at KPrimaryIOBase + 16000h
		HW_MAPPING		KHwUSBHPhys				,2, HW_MULT_4K		; Mapped at KPrimaryIOBase + 18000h
		HW_MAPPING		KHwAXI64DMACPhys		,1,	HW_MULT_4K		; Mapped at KPrimaryIOBase + 1A000h
																	; Next to be mapped at KPrimaryIOBase + 1B000h
	ENDIF
		DCD			0												; Terminator

;*******************************************************************************
; Set up RAM bank
;
; Do any additional RAM controller initialisation for each RAM bank which wasn't
; done by InitialiseHardware.
; Called twice for each RAM bank :-
;	First with R3 = 0xFFFFFFFF before bank has been probed
;	Then, if RAM is present, with R3 indicating validity of each byte lane, ie
;	R3 bit 0=1 if D0-7 are valid, bit1=1 if D8-15 are valid etc.
; For each call R1 specifies the bank physical base address.
;
; Enter with :
;		R10 points to super page
;		R12 points to ROM header
;		R13 points to stack
;		R1 = physical base address of bank
;		R3 = width (bottom 4 bits indicate validity of byte lanes)
;			 0xffffffff = preliminary initialise
;
; Leave with :
;		No registers modified
;*******************************************************************************
SetupRamBank	ROUT
		MOV		pc, lr





;*******************************************************************************
; Set up ROM bank
;
; Do any required autodetection and autosizing of ROMs and any additional memory
; controller initialisation for each ROM bank which wasn't done by
; InitialiseHardware.
;
; The first time this function is called R11=0 and R0 points to the list of
; ROM banks returned by the BTF_RomBanks call. This allows any preliminary setup
; before autodetection begins.
;
; This function is subsequently called once for each ROM bank with R11 pointing
; to the current information held about that ROM bank (SRomBank structure).
; The structure pointed to by R11 should be updated with the size and width
; determined. The size should be set to zero if there is no ROM present in the
; bank.
;
; Enter with :
;		R10 points to super page
;		R12 points to ROM header
;		R13 points to stack
;		R11 points to SRomBank info for this bank
;		R11 = 0 for preliminary initialise (all banks)
;
; Leave with :
;		Update SRomBank info with detected size/width
;		Set the size field to 0 if the ROM bank is absent
;		Can modify R0-R4 but not other registers
;
;*******************************************************************************
SetupRomBank	ROUT
		MOV		pc, lr





;*******************************************************************************
; Reserve physical memory
;
; Reserve any physical RAM needed for platform-specific purposes before the
; bootstrap begins allocating RAM for page tables/kernel data etc.
;
; There are two methods for this:
;	1.	The function ExciseRamArea may be used. This will remove a contiguous
;		region of physical RAM from the RAM bank list. That region will never
;		again be identified as RAM.
;	2.	A list of excluded physical address ranges may be written at [R11].
;		This should be a list of (base,size) pairs terminated by a (0,0) entry.
;		This RAM will still be identified as RAM by the kernel but will not
;		be allocated by the bootstrap and will subsequently be marked as
;		allocated by the kernel immediately after boot.
;
; Enter with :
;		R10 points to super page
;		R11 indicates where preallocated RAM list should be written.
;		R12 points to ROM header
;		R13 points to stack
;
; Leave with :
;		R0-R3 may be modified. Other registers should be preserved.
;*******************************************************************************
ReservePhysicalMemory	ROUT
;	IF :DEF: CFG_AlternateEntryPoint
;	IF 0
;		STMFD	sp!, {r9,r11,lr}
;		LDR		r0, =KRamTargetAddr		; reserve the first 64MB RAM for the image to download into.
;		MOV		r1, #0x4000000			; 64MB
;		MOV		r2, #0
;		MOV		r11, #0
;		LDR		r9, [r10, #SSuperPageBase_iRamBootData]
;		BL		ExciseRamArea							; remove RAM
;		LDMFD	sp!, {r9,r11,pc}
;	ENDIF
		MOV		pc, lr

	IF	CFG_MMDirect
	INIT_NUMERIC_CONSTANT	KTTBRExtraBits, 0x02
	ELSE
	IF SMP
	INIT_NUMERIC_CONSTANT	KTTBRExtraBits, 0x02
	ELSE
	INIT_NUMERIC_CONSTANT	KTTBRExtraBits, 0x08
	ENDIF
	ENDIF


;*******************************************************************************
; Return parameter specified by R0 (see TBootParam enum)
;
; Enter with :
;		R0 = parameter number
;
; Leave with :
;		If parameter value is supplied, R0 = value and N flag clear
;		If parameter value is not supplied, N flag set. In this case the
;		parameter may be defaulted or the system may fault.
;		R0,R1,R2 modified. No other registers modified.
;
;*******************************************************************************
GetParameters ROUT
		ADR		r1, ParameterTable
		B		FindParameter
ParameterTable
		; Include any parameters specified in TBootParam enum here
		; if you want to override them.
		DCD		BPR_TTBRExtraBits,	KTTBRExtraBits
	IF	CFG_MMDirect
		DCD		BPR_UncachedLin,	0x7F000000	; parameter number, parameter value
	IF	SMP
		DCD		BPR_APBootLin,		0x7F001000	; parameter number, parameter value
	ENDIF
;	IF	CFG_BootLoader
;		DCD		BPR_BootLdrImgAddr,	KRamImageAddr
;	ENDIF
	ENDIF
		DCD		-1								; terminator

;*******************************************************************************
; Do final platform-specific initialisation before booting the kernel
;
; Typical uses for this call would be:
;	1.	Mapping cache flushing areas
;	2.	Setting up pointers to routines in the bootstrap which are used by
;		the variant or drivers (eg idle code).
;
; Enter with :
;		R10 points to super page
;		R11 points to TRomImageHeader for the kernel
;		R12 points to ROM header
;		R13 points to stack
;
; Leave with :
;		R0-R9 may be modified. Other registers should be preserved.
;
;*******************************************************************************
FinalInitialise ROUT
		STMFD	sp!, {lr}

	IF	SMP
; Handshake with APs

	IF	CFG_MMDirect
		LDR		r7, =KHwBaseMPcorePrivatePhys		; R7 points to SCU (physical address for direct memory model)
	ELSE
		LDR		r7, =KPrimaryIOBase + 0x1000	; R7 points to SCU (virtual address for other memory models)
	ENDIF
		ADD		r8, r7, #0x1000					; Virtual address of GIC Distributor
		ADD		r9, r7, #0x100					; Virtual address of GIC CPU Interface
		LDR		r5, [r7, #4]					; SCU configuration register
		DWORD	r5, "SCU Config"
		AND		r5, r5, #3
		ADD		r5, r5, #1						; r5 = number of CPUs
		DWORD	r5, "NCPUs"
		MOV		r6, #0							; CPU number
		B		%FA2
1
		DWORD	r6, "CPU"
		BL		HandshakeAP						; handshake with this AP
2
		; Turn on LED for this CPU (CPU0==LED0, etc...)
		GET_ADDRESS	r0, KHwFpgaLedsPhyicalBase, KHwFpgaLedsLinBase
												; r0 is the address of the 16 bit LED register in FPGA, aka FLED
		LDRH	r1, [r0]						; read the contents of FLED into r1
		MOV		r2, #KHtFpgaLed0
		MOV		r2, r2, LSL r6					; r2 is the LED value we want to OR in (ie, 1<<CPU number)
		ORR		r1, r1, r2						; r1 = r1 | r2
		STRH	r1, [r0]						; write r1 back to the FLED register

		ADD		r6, r6, #1						; r6 = CPU number of next AP
		CMP		r6, r5							; if equal to number of CPUs, finished
		BLO		%BA1							; else do next AP
	ENDIF

		LDMFD	sp!, {pc}

;*******************************************************************************
; Output a character to the debug port
;
; Enter with :
;		R0 = character to output
;		R13 points to valid stack
;
; Leave with :
;		nothing modified
;*******************************************************************************
DoWriteC	ROUT
	IF	CFG_DebugBootRom
		STMFD	sp!, {r1,r2,lr}
		BL		GetDebugPortBase	; r1 = base address of debug port

		; wait for debug port to be ready for data
1
		LDR		r2, [r1, #Serial_LSR]
		TST		r2, #0x20
		BEQ		%BT1
		
		; output character to debug port
		STR		r0, [r1, #Serial_THR]

		LDMFD	sp!, {r1,r2,pc}
	ELSE
		MOV		pc, lr
	ENDIF


	IF	CFG_InitDebugPort
;*******************************************************************************
; Initialise the debug port
;
; Enter with :
;		R12 points to ROM header
;		There is no valid stack
;
; Leave with :
;		R0-R2 modified
;		Other registers unmodified
;*******************************************************************************
InitDebugPort	ROUT
		MOV     r0, lr
		BL		GetDebugPortBase			; r1 = base address of debug port

		LDR		r2, [r12, #TRomHeader_iDebugPort]
		MOVS	r2, r2, LSL #24				; C=1 if high speed, C=0 low speed

		; set up debug port
		MOV		r2, #0x83
		STR		r2, [r1, #Serial_LCR]
		MOVCS	r2, #KBaudRateDiv_230400
		MOVCC	r2, #KBaudRateDiv_default
		STR		r2, [r1, #Serial_DLL]
		MOV		r2, #0
		STR		r2, [r1, #Serial_DLH]
		MOV		r2, #0x03
		STR		r2, [r1, #Serial_LCR]

		MOV		pc, r0

;*******************************************************************************
; Get the base address of the debug UART
;	It is uart0 (for TRomHeader::iDebugPort ==0) or uart1 otherwise
;	Returns physical or linear address, depending on the state of MMU.
;
; Enter with :
;		R12 points to ROM header
;		There may be no stack
;
; Leave with :
;		R1 = base address of port
;		No other registers modified
;*******************************************************************************
GetDebugPortBase	ROUT
		LDR		r1, [r12, #TRomHeader_iDebugPort]
		CMP		r1, #0x100
		BEQ		%FA2				; port 0 at 230400
		CMP		r1, #0
		BNE		%FA1				; skip if not port 0
2
		GET_ADDRESS	r1, KHwUART0Phys, Serial0LinBase
		MOV		pc, lr
1
		GET_ADDRESS	r1, KHwUART1Phys, Serial1LinBase
		MOV		pc, lr

	ENDIF	; CFG_InitDebugPort

;*******************************************************************************
; BOOT FUNCTION TABLE
;*******************************************************************************

BootTable
		DCD	DoWriteC				; output a debug character
		DCD	GetRamBanks				; get list of RAM banks
		DCD	SetupRamBank			; set up a RAM bank
		DCD	GetRomBanks				; get list of ROM banks
		DCD	SetupRomBank			; set up a ROM bank
		DCD	GetHwBanks				; get list of HW banks
		DCD	ReservePhysicalMemory	; reserve physical RAM if required
		DCD	GetParameters			; get platform dependent parameters
		DCD	FinalInitialise			; Final initialisation before booting the kernel
		DCD HandleAllocRequest		; allocate memory		
		DCD	GetPdeValue				; usually in generic code
		DCD	GetPteValue				; usually in generic code
		DCD	PageTableUpdate			; usually in generic code
		DCD	EnableMmu				; Enable the MMU (usually in generic code)

	IF :DEF: CFG_USE_SHARED_MEMORY
SharedMemory	EQU		1
	ELSE
SharedMemory	EQU		0
	ENDIF

		BTP_ENTRY   CLIENT_DOMAIN, PERM_RORO, MEMORY_FULLY_CACHED,       	1,  1,  0,  SharedMemory	; ROM
		BTP_ENTRY   CLIENT_DOMAIN, PERM_RWNO, MEMORY_FULLY_CACHED,       	0,  1,  0,  SharedMemory	; kernel data/stack/heap
		BTP_ENTRY   CLIENT_DOMAIN, PERM_RWNO, MEMORY_FULLY_CACHED,       	0,  1,  0,  SharedMemory	; super page/CPU page
	IF	SMP
	    BTP_ENTRY   CLIENT_DOMAIN, PERM_RWNO, MEMORY_UNCACHED,				0,  1,  0,  SharedMemory	; page directory/tables
	ELSE
		BTP_ENTRY   CLIENT_DOMAIN, PERM_RWNO, MEMORY_FULLY_CACHED,			0,  1,  0,  SharedMemory	; page directory/tables
	ENDIF
		BTP_ENTRY   CLIENT_DOMAIN, PERM_RONO, MEMORY_FULLY_CACHED,       	1,  1,  0,  SharedMemory	; exception vectors
		BTP_ENTRY   CLIENT_DOMAIN, PERM_RWNO, MEMORY_STRONGLY_ORDERED,      0,  1,  0,  SharedMemory	; hardware registers
		DCD         0                                                       			 				; unused (minicache flush)
		DCD         0                                                         							; unused (maincache flush)
		BTP_ENTRY   CLIENT_DOMAIN, PERM_RWNO, MEMORY_FULLY_CACHED,       	0,  1,  0,  SharedMemory	; page table info
		BTP_ENTRY   CLIENT_DOMAIN, PERM_RWRW, MEMORY_FULLY_CACHED,       	1,  1,  0,  SharedMemory	; user RAM
		BTP_ENTRY   CLIENT_DOMAIN, PERM_RONO, MEMORY_STRONGLY_ORDERED,      1,  1,  0,  SharedMemory	; temporary identity mapping
		BTP_ENTRY   CLIENT_DOMAIN, UNC_PERM,  MEMORY_STRONGLY_ORDERED,      0,  1,  0,  SharedMemory	; uncached


		END


