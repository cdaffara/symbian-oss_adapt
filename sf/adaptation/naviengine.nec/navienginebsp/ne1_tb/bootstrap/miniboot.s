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
; \ne1_tb\bootstrap\miniboot.s
; Code to retrieve the core loader image from the NAND flash and
; run it. Expects that the system is in a state where NAND can be
; read and SDRAM can be written.
; See variant_bootstrap.inc for the variant specfic constants 
; Where in RAM should the core loader be copied to (LINKBASE of coreldr)
; There is a companion to this value in the coreloader makefile, they must
; match!
;

	INCLUDE e32rom.inc
	INCLUDE naviengine.inc

KMinibootStackAddr              EQU     0x820003FC
KRAMBase						EQU		0x80000000		; base of ram

KNandPageSize	EQU	2048
KPageByteShift	EQU	11

;Command Set Reg Bits
KNandBusyBit	 EQU 0x0100
KNandPageReadBM  EQU 0x0101
KNandResetBM	 EQU 0x0106

;ECC Reg Bits
KNandECCDRC		 EQU 0x0080

; NAND Register Addresses
KNandDataWndw		EQU 0x18019000;
KNandPgeAdr1Reg 	EQU 0x18019840;
KNandPgeAdr2Reg 	EQU 0x18019842;
KNandPgeAdrMskReg0 	EQU 0x18019844;
KNandPgeAdrMskReg1 	EQU 0x18019846;
KNandPgeAdrMskReg2 	EQU 0x18019848;
KNandPgeAdrMskReg3 	EQU 0x1801984A;
KNandCmdSetReg	  	EQU 0x1801984E;
KNandDataAreaECCReg EQU 0x18019850;
KNandROMTypeSetup1	EQU 0x18019852;
KNandROMTypeSetup2	EQU 0x18019854;
KNandChipSelect	  	EQU 0x18019856;


; Which page in NAND holds the Coreldr image
KSymbianCoreldrPage			EQU		1 	;
KSymbianCoreldrSizePages	EQU		64	; 64 * 2048B = 128KB

; Export the miniboot's only entry point
	EXPORT	GetCoreldr

;
; Area definition for ARMLD linker
;
	AREA |Boot$$Code|, CODE, READONLY, ALIGN=6

;******************************************************************************
; Boot Entry point for bootloader's miniboot code
;
;	Always runs
;		after the bootloader has initialised the memory subsystem
;		with the SRAM/SDRAM devices available. (RAM >0x8000.0000)
;
;	Tasks
;		Confirm NAND device is as expected
;		read TOC structure at the start of the NAND (in page 0)
;		search TOC array for coreloader image entry
;		load coreloader sized image from NAND offset (from TOC)
;		jump to coreloader entry point
;
;******************************************************************************

GetCoreldr
	; create a stack so that we can use ordinary function calls
	ldr		r13, =KMinibootStackAddr
	
	; RESET_NAND
	LDR		r0, =KNandResetBM 	; CMD_RESET | XROMC_BUSY_BIT_MASK
	LDR		r4, =KNandCmdSetReg	; XROMC
	STRH	r0, [r4]
	
	; POLLED_WAIT_FOR_RDY
1
	LDR		r4, =KNandCmdSetReg  	
	LDRH	r0, [r4]				; Get contents of Command setup register
	TST		r0, #KNandBusyBit
	BNE		%B1	

	; Start from a clean, ready system state
	; INIT_PNL_DEVICE
	LDR		r0, =0xF000	;Unmask address for XROMM0-3
	LDR		r4, =KNandPgeAdrMskReg0 
	STRH	r0,	[r4]
	
	LDR		r4, =KNandPgeAdrMskReg1 
	STRH	r0,	[r4]
	
	LDR		r4, =KNandPgeAdrMskReg2 
	STRH	r0,	[r4]
	
	LDR		r4, =KNandPgeAdrMskReg3 
	STRH	r0,	[r4]
		
	;Configure the NAND Type
	;CS1 NAND Config: 8bit, 2K
	;CS2 NAND Config: 8bit, 2K
	LDR		r0,	=0x0101
	LDR		r4,	=KNandROMTypeSetup1
	STRH	r0,	[r4]
	;CS3 NAND Config: 8bit, 2K
	LDR		r0,	=0x01
	LDR		r4,	=KNandROMTypeSetup2
	STRH	r0,	[r4]
	
	; Select chipset 0 to be used (navi supports upto 4 NAND chips)
	LDR		r0, =0x0000	;Select ChipSet 0
	LDR		r4, =KNandChipSelect 
	STRH	r0,	[r4]

	LDR 	r4,=KNandDataAreaECCReg
	LDRH	r0,	[r4] ; get contents of register
	BIC		r0, r0, #0x1
	STRH	r0, [r4] 


	; Normally Read the device ID and confirm nand device is as expected
	; NAND Controller on NaviEngine doesn't allow the Device id to be read

	bl		readhdr16						; read header of 16bit device
	; Readhdr16 -> parse_header -> find/load Symbian coreldr -> run coreldr
	

;------------------------------------------------------------------------
;
; strcmp - bytewise compare of two strings
;	r0->string A
;	r1->string B
;	r3->size of string A
;
;	if string A == string B return 0 in r0
;	else returns non zero in r0
;
;------------------------------------------------------------------------
strcmp	ROUT
	stmfd	r13!, {r4-r6,lr}

	mov		r4, r0
	mov		r5, r1
	mov		r6, r3
	
strloop
	ldrb	r2, [r4], #1
	ldrb	r3, [r5], #1
	
	subs	r0, r2, r3
	cmp		r2, #0
	beq		strreturn
	subs	r6, r6, #1
	bne		strloop
strreturn
	ldmfd	r13!, {r4-r6,pc}


;---------------------------------------------------------------------------------
; readpage16
;
;	read 1 page (2048KB) of NAND device,
;	starting at page (r0)
;	put data into address (r1)
;
;---------------------------------------------------------------------------------
readpage16	ROUT
	stmfd	r13!, {r4-r7,lr}

	; set the page start addresss [r0]
	; need multiple by page size to get correct address
	MOV		r6, #KNandPageSize ; 2048
	
	;Set Address Reg1 (lower range)
	MUL		r7, r0, r6
	LDR		r4, =KNandPgeAdr1Reg 
	STRH	r7,	[r4]
	
	;Set Address Reg2 (upper range)
	;MUL		r7, r0, r6
	LSR		r7, r7, #0x10;
	LDR		r4, =KNandPgeAdr2Reg 
	STRH	r7,	[r4]

	; Issue Read Command
	LDR		r5, =KNandPageReadBM 	; CMD_READ | XROMC_BUSY_BIT_MASK
	LDR		r4, =KNandCmdSetReg	; XROMC
	STRH	r5, [r4]

	; Poll Busy Bit
1  	
	LDRH	r5, [r4]				; Get contents of Command setup register
	TST		r5, #KNandBusyBit
	BNE		%B1	
	
	; Poll status DRC(Data Read Complete)
2
	LDR		r4, =KNandDataAreaECCReg  	
	LDRH	r5, [r4]				; Get contents of ECC register
	TST		r5, #KNandECCDRC
	BNE		%B2
	
	;Copy page (2048) into address [r1]
	LDR		r4, =KNandDataWndw
	MOV		r6, #KNandPageSize ; 2048
read
	LDRH	r5, [r4]
	STRH	r5, [r1], #2	; save at target
	ADD		r4, r4, #2		; move data wndw pointer
	SUBS	r6, r6, #2		; end of page?
	BNE		read			; no => copy some more ;)
	
	ldmfd	r13!, {r4-r7,pc}				; yes, return

;-----------------------------------------------------------------------------------
; readpages16
;
; read <r2> pages of NAND,
; starting at page <r0>
; placing data starting at <r1>
;
;-----------------------------------------------------------------------------------
readpages16	ROUT
	stmfd	r13!, {r4-r6,lr}
	mov		r4, r0
	mov		r5, r1
	mov		r6, r2
loop16
	bl		readpage16
	subs	r6, r6, #1
	ldmeqfd	r13!, {r4-r6,pc}
	add		r0, r0, #1
	b		loop16

;-----------------------------------------------------------------------------------
;
; readhdr16
;
; Read the NAND header from flash into ram so it may be examined for a Symbian
; signature.
;
;-----------------------------------------------------------------------------------
readhdr16
	; read the header of a 16b device
	;Print page
	
	ldr		r0, =0x0		; header pag
	ldr		r1, =KCoreLoaderAddress			;
	bl		readpage16
	
	b		parse_header

;---------------------------------------------------------------------------
;
; At this point the header buffer contains the first NAND page
; Determine whether the NAND device has been formatted according to the
; new style Symbian FBR layout
;
;---------------------------------------------------------------------------

parse_header
	bl		find_symbian_coreldr
	cmp		r0, #0
	beq		load_symbian_coreldr

	; Use a label rather than a SUB/PC as it makes a symbol for a debugger.
SymbianCoreldrNotFound

	b		SymbianCoreldrNotFound			; spin forever

	; NOTREACHED

;---------------------------------------------------------------------------------
;
; find_symbian_coreldr
;
; searches for the Symbian1 signature in the FBR then assumes
; that the coreldr will be stored in the 30 pages (15k) starting
; at page 4.
;
;---------------------------------------------------------------------------------

Symbian_Signature		DCB		"1naibmyS"
Symbian_Signature_size  EQU		8


find_symbian_coreldr	ROUT
		stmfd	r13!, {r4,lr}
		ldr		r0, =KCoreLoaderAddress
		add		r0, r0, #4						; signature at byte 4 in page 0
		ADR		r1, Symbian_Signature 
		ldr		r3, =Symbian_Signature_size
		bl		strcmp
		ldmfd	r13!, {r4,pc}

;---------------------------------------------------------------------------------
;
; load_symbian_coreldr
;
; Assumses Coreloader starts at page 1 and can be as big as 64 pages (128KB)
;
;---------------------------------------------------------------------------------
load_symbian_coreldr	ROUT

		mov		r0, #KSymbianCoreldrPage
		ldr		r1, =KCoreLoaderAddress
		mov		r2, #KSymbianCoreldrSizePages
		bl		readpages16

		; restart CPUs, by calling restart vertor.
		mov		r0, #KtRestartReasonCoreLdr 
		add		pc, r12,  #TRomHeader_iRestartVector

		; NOTREACHED
	END
