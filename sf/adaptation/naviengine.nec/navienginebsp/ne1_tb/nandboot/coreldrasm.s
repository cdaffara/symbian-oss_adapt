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
; ne1_tb\nandboot\coreldrasm.s
;

	INCLUDE armcpudefs.inc			; Constants used by the MMU code
	INCLUDE nand_plat.inc			; NAND device specifics
	INCLUDE naviengine.inc

	EXPORT	BootEntry
	EXPORT	GetRomTargetAddress
	EXPORT	GetSMRIBTargetAddress
	EXPORT	GetSMRTargetStartAddress
	EXPORT  GetSMRTargetMaxAddress
	EXPORT	clearRestartTimer
	EXPORT 	RestartAuxiliaryCores
	IF USE_MMU
		IMPORT	setupMMU
		EXPORT	GetPageTableBaseAddress
		EXPORT	GetMemoryBankArrayBase
		EXPORT	GetNumberOfMemoryBankAddresses
	ELSE ; USE_MMU
		; NOT using MMU
		IMPORT	loadmain
		EXPORT	bootos
	ENDIF	; USE_MMU
	
	EXPORT	charout
	EXPORT	WriteW

;******************************************************************************
; Constants

KHwSDramAddr				EQU	0x80000000
KSDramSizeDdr				EQU	0x10000000  ; 256 MBytes


KSerial0PhysBase				EQU	0x18034000 ; // must be same number as in ne1_tb.s
;Serial Port Register Offsets
Serial_DLL			EQU		0
Serial_DLH			EQU		4
Serial_LCR			EQU		0xc
Serial_THR			EQU		0
Serial_LSR			EQU		0x14

; KRomTargetAddress is the memory location where the core image will be
; placed in RAM.
KRomTargetAddress				EQU	KHwSDramAddr	                    ; Base of RAM
KSDramTopAddress                EQU KHwSDramAddr + KSDramSizeDdr        ; 0x90000000
KSRTargetSize                   EQU 0x02000000                          ; 32 Mb
KSRIBTargetAddress				EQU KSDramTopAddress - KSRTargetSize    ;0x8E000000 (Top of Ram - 0x02000000 (32 Mb))
KSRIBTargetSize                 EQU 0x00001000                          ; 4 Kb
KSRTargetStartAddress			EQU KSRIBTargetAddress + KSRIBTargetSize ; 0x8E001000 (+4Kb)
KSRTargetMaxAddress             EQU KSDramTopAddress 

; define linker area for placement of .text section (LINKBASE)
; also use PRESERVE8 to indicate (to linker) stack 8B alignment

	PRESERVE8
	AREA	|Boot$$Code|, CODE, READONLY, ALIGN=6

;******************************************************************************
; Entry point for the Core Loader
; Note that this MUST be at the start of the image!!!
;******************************************************************************
BootEntry

	LDR		r13, =KCoreldrStackAddr		; C code needs a stack

	bl PauseAuxiliaryCores

	; This hardware reference platorm, NaviEngine, bootstraps the coreloader image from NAND and
	; has just started executing. Most of the hardware setup, such as configuring RAM, has 
	; already been done by the bootloader. For systems that boot directly from the coreloader 
	; image most of the hardware setup will need to be done here as it won't have been done 
	; in bootoader code.

	BL		InitUart
		
	IF USE_MMU
		
		b		setupMMU

; Store the page tables after the coreloader on the next 128K boundary
KCoreLdrPDEAddress			EQU ( ( _LINKBASE_ +0x00020000)) :AND: 0xfff20000

; Format of table is, for each mapping region
; virtual address, physical address, length of region (each these must be
; megabyte aligned) the PDEs created will be for 1MB sections and so will cover
; 1MB above the VA+length. 
;
; A table with more elements would look like this
; memoryTable DCD virtual_address1, physical_address1, length1
;             DCD virtual_address2, physical_address2, length2
;             DCD virtual_address3, physical_address3, length3
;
memoryTable DCD KHwSDramAddr, KHwSDramAddr, KSDramSizeDdr; Map all SDRAM

GetPageTableBaseAddress
		LDR		r0,=KCoreLdrPDEAddress
		__JUMP	lr

GetMemoryBankArrayBase
		ADRL	r0,memoryTable
		__JUMP	lr

GetNumberOfMemoryBankAddresses
		MOV		r0,#1			; 1 row only
		__JUMP	lr

	ELSE	; USE_MMU
	; NOT using MMU
				
		b		loadmain		; jump directly into the NAND core loader without setting up the MMU

; bootos symbol needs defining if NOT using the MMU code
bootos

	BL RestartAuxiliaryCores
	MOV		pc, r0				; jump off to OS

	ENDIF	; USE_MMU


GetRomTargetAddress
	ldr		r0, =KRomTargetAddress
	__JUMP	lr
	
GetSMRIBTargetAddress
	ldr		r0, =KSRIBTargetAddress
	__JUMP	lr
	
GetSMRTargetStartAddress
	ldr		r0, =KSRTargetStartAddress
	__JUMP	lr
	
GetSMRTargetMaxAddress
    ldr     r0, =KSRTargetMaxAddress
	__JUMP	lr    
	
;******************************************************************************
;	writes character in r0 to the debug port
;******************************************************************************
charout
		STMFD	sp!, {r1,r2,lr}
		LDR		r1, =KSerial0PhysBase

		; wait for debug port to be ready for data
1
		LDR		r2, [r1, #Serial_LSR]
		TST		r2, #0x20
		BEQ		%BT1
		
		; output character to debug port
		STR		r0, [r1, #Serial_THR]

		LDMFD	sp!, {r1,r2,pc}
	
;******************************************************************************
; Printf("%0x", r0) a word to the serial port (stack required)
;
; Enter with
;		r0 = word
;		debug port initialised
;
; Leave with
;		no registers changed
;******************************************************************************
WriteW
	STMFD	sp!, {r0-r4, lr}
	MOV		r4, #28
	MOV		r1, r0

1	MOV		r0, r1, LSR r4
	AND		r0, r0, #0x0000000F
	CMP		r0, #9
	ADDLE	r0, r0, #48
	ADDGT	r0, r0, #55
	BL		charout
	SUBS	r4, r4, #4
	BGE		%BT1

	MOV		r0, #' '
	BL		charout

	LDMFD	sp!, {r0-r4, pc}
	
;******************************************************************************
;	Prepares and starts a timer
;	Preserves all registers
; NOT SUPPORTED ON NAVIENGINE - SUPPLIED TO ALLOW COMPILE
;******************************************************************************
clearRestartTimer
	MOV     r0, lr
	NOP
	MOV		pc, r0

;******************************************************************************
; Initialise the serial port (stack required)
; This is derived from the NaviEngine bootstrap's debug uart initialisation code.
; Enter with :
;		none
; Leave with :
;		no registers changed
;******************************************************************************
InitUart
		MOV     r0, lr
		LDR		r1, =KSerial0PhysBase

		; set up debug port
		MOV		r2, #0x83
		STR		r2, [r1, #Serial_LCR]
		MOV		r2, #KBaudRateDiv_default
		STR		r2, [r1, #Serial_DLL]
		MOV		r2, #0
		STR		r2, [r1, #Serial_DLH]
		MOV		r2, #0x03
		STR		r2, [r1, #Serial_LCR]

		MOV		pc, r0
		
	
	


;***************************************************************************************
;	SMP code - PauseAuxiliaryCores
;	Expects lr and sp
;***************************************************************************************
PauseAuxiliaryCores	
	
	
	;	IF SMP

		; Is this the boot processor ?
		MRC		p15, 0, r0, c0, c0, 5
		ANDS	r0, r0, #0x0f			; r0 = CPU number 0-3
		BEQ		IsBootProcessor			; Branch out if CPU 0 (= boot processor)
		mov 	r10, r0
		; No - this is an AP

		mov		r5, r13
		add		r13, r13, r10, lsl #2  ; Move to stack var for this core
		mov 	r3, #10

1
		ldr		r4, [r5]	; load message
		teq		r3,r4		; is message r3?
		streq	r3, [r13]	; 
		addeq	r3, r3, #1
		teq		r3, #13
		beq		Signaled

		nop
		nop
		nop
		nop

		B		%BA1

Signaled

		ldr		pc, =KRomTargetAddress

IsBootProcessor
	add r13, r13, #16	; reserve space on stack for signaling
	mov	pc, lr



;***************************************************************************************
;	SMP code - RestartAuxiliaryCores
;	Expects lr, r0 - address to start cores at.
;   Corrupts r1-3
;***************************************************************************************


RestartAuxiliaryCores

;KCoreldrStackAddr			EQU 0x8C0003FC


		; This code wakes the other cores, causing them to run the image

		LDR		r3, =KCoreldrStackAddr ; Find our origanal stack frame
		mov		r2 , #10
1
		str		r2, [r3]				; send r2

		ldr	r1,	[r3, #4]				; read from core 1
		teq r1, r2							; has it recieved?
		bne	%BA1

		ldr	r1,	[r3, #8]				; read from core 2
		teq r1, r2							; has it recieved?
		bne	%BA1

		ldr	r1,	[r3, #12]				; read from core 3
		teq r1, r2							; has it recieved?
		bne	%BA1

		add	r2,r2, #1					; add 1 to our massage, and try agian
		teq	r2, #13						; we repeat a few times, to make sure
		bne %BA1

		mov pc, lr

	END
