// Copyright (c) 1994-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/beagle_drivers/wb/cyashalbeagleboard_spi.cpp
//

#include <kern_priv.h>
#include <beagle/beagle_gpio.h>
#include <beagle/variant.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_irqmap.h> // GPIO interrupts
#include <assp/omap3530_assp/omap3530_gpio.h>

#include <assp.h> // Required for definition of TIsr

#include <cyasregs.h> // Astoria register definitions
#include <cyashalbeagleboard_spi.h>
#include <cyashalbeagleboard.h>

int gConfigured = 0 ;

/***********************************************************************/
/******************************* ISR  **********************************/
/***********************************************************************/

int
CyAsHalBeagleBoard__SetupISR(void* handler, void* ptr)
{

	//Set up the button to proivde a panic button invoking Fault()
	if(KErrNone != GPIO::SetPinDirection(KGPIO_INT, GPIO::EInput))
		return KErrArgument;

	GPIO::SetPinMode(KGPIO_INT, GPIO::EEnabled);

	if(KErrNone !=GPIO::BindInterrupt(KGPIO_INT, (TGpioIsr)handler, (TAny*)ptr))
		return KErrArgument;

	/*EEdgeFalling*/
	if(KErrNone !=GPIO::SetInterruptTrigger(KGPIO_INT, GPIO::ELevelLow))
		return KErrArgument;

	if(KErrNone !=GPIO::EnableInterrupt(KGPIO_INT))
	{
		GPIO::UnbindInterrupt(KGPIO_INT);
		return KErrInUse;
	}
	return 0 ;
}

static void
ManualTriggerISRTest(void)
{
	GPIO::TGpioState intState = GPIO::EHigh ;

	GPIO::GetInputState(KGPIO_INT, intState);
	Kern::Printf("KGPIO_INT before INT is %d", intState);

	//Set INT to LOW state
	CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(CY_AS_MEM_P0_VM_SET, 0x0545) ;
	GPIO::GetInputState(KGPIO_INT, intState);
	Kern::Printf("KGPIO_INT after INT is %d", intState);

	//return INT to HIGH state
	CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(CY_AS_MEM_P0_VM_SET, 0x0745) ;
	GPIO::GetInputState(KGPIO_INT, intState);
	Kern::Printf("KGPIO_INT after INT is %d", intState);
}

TUint32 gISRCnt = 1 ;
TDfc* gpMyDfc;

static void myTestDFCFn(
    TAny *aPtr)
{

	TUint32* gpISRCnt = (TUint32*)aPtr ;
	Kern::Printf("myTestDFCFn called (%d)",*gpISRCnt);

	*gpISRCnt += 1;

	Kern::Printf("Enable interrupt");
	GPIO::EnableInterrupt(KGPIO_INT) ;
}

static void myISR(TAny* aPtr)
{
	Kern::Printf("AstoriaISR");
	gpMyDfc->Add();

	Kern::Printf("Disable interrupt");
	/* Disable Interrupt Here, it will be re-enabled by DFCs
	GPIO::DisableInterrupt(KGPIO_INT) ;*/
}

/***********************************************************************/
/**************************** SPI Driver *******************************/
/***********************************************************************/

extern void
McSPI4Ch0_SetXFERLEVEL(TUint16 wcnt, TUint8 afl, TUint8 ael)
{
	TUint32 r = wcnt ;

	r <<= 16 ;
	r |= (afl & 0x3f) << 8 ;
	r |= (ael & 0x3f) ;

	AsspRegister::Write32(KMcSPI4_XFERLEVEL, r);
}

inline void McSPI4Ch0_Enable(void)
{
	/* Enable channel 0 */
	AsspRegister::Write32(KMcSPI4_CH0CTRL, 0x1);
}

inline void McSPI4Ch0_Disable(void)
{
	/* Disable channel 0 */
	AsspRegister::Write32(KMcSPI4_CH0CTRL, 0x0);
}

inline void McSPI4Ch0_TransmitMode(void)
{
	AsspRegister::Modify32(KMcSPI4_CH0CONF, 0x0, 0x1<<13);
}

inline void McSPI4Ch0_TransmitAndReceiveMode(void)
{
	AsspRegister::Modify32(KMcSPI4_CH0CONF, 0x1<<13, 0x0);
}

extern
void McSPI4Ch0_SyncSlave(void)
{
	Kern::Printf("Sync With SPI slave not implemented\n");
}

inline void CheckForTxEmpty(void)
{
	TUint32 r ;
	for(;;)
	{
		r = AsspRegister::Read32(KMcSPI4_CH0STAT);
		if ( r & 0x2)
		{
			break;
		}
		Kern::Printf("Waiting for TX empty.");
	}
}

inline void CheckForRxFull(void)
{
	TUint32 r ;
	for(;;)
	{
		r = AsspRegister::Read32(KMcSPI4_CH0STAT);
		if ( r & 0x1)
		{
			break;
		}
		Kern::Printf("Waiting for RX full.");
	}
}

inline void CheckForTxEmptyRxFull(void)
{
	TUint32 r ;
	for(;;)
	{
		r = AsspRegister::Read32(KMcSPI4_CH0STAT);
		if ( r & 0x3)
		{
			break;
		}
		Kern::Printf("Waiting for TX empty and RX full.");
	}
}

extern void
McSPI4Ch0_DumpAstoriaRegs(int allReg)
{
	int i ;
	TUint16 regVal = 0 ;

	for ( i = 0x80 ; i < 0xfb ; i++ )
	{
		if ( (i == 0x84) ||
		     ((i >= 0x87) && (i <= 0x8f)) ||
		     (i == 0x93) ||
		     ((i >= 0x96) && (i <= 0x97)) ||
		     (i == 0x99) ||
		     ((i >= 0x9b) && (i <= 0x9f)) ||
		     ((i >= 0xb0) && (i <= 0xbf)) ||
		     ((i >= 0xc6) && (i <= 0xd8)) ||
		     ((i >= 0xe3) && (i <= 0xef)) ||
		     ((i >= 0xf4) && (i <= 0xf7)) )
		{
			/*skip*/
			continue ;
		}

		regVal = 0 ;
		if ( allReg )
			CyAsHalBeagleBoardMcSPI4Ch0_ReadReg(i, &regVal) ;
		else
			CyAsHalBeagleBoardMcSPI4Ch0_ReadReg(CY_AS_MEM_CM_WB_CFG_ID, &regVal) ;
		Kern::Printf("REG[%02x] = 0x%04x", i, regVal);
	}
}

static int
AstoriaRegTest(int type)
{
	int errCnt = 0 ;
	int i ;
	for ( i= 0 ; i < 100 ; i++ )
	{
		TUint16 regVal0 = 0, regVal1 = 0, regVal2 = 0, regVal3 = 0 ;
		TUint16 expVal = 0 ;

		if ( type == 1 )
		{
			Kern::Printf("Write/Read Register with sequential value...");
			CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf8, (i+1)) ;
			CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf9, (i+2)) ;
			CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfa, (i+3)) ;
			CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfb, (i+4)) ;
		}
		else if ( type == 2 )
		{
			Kern::Printf("Write/Read 0xffff and 0x0 ...");
			if ( i % 2 == 0 )
			{
				expVal = 0xffff ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf8, expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf9, ~expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfa, expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfb, ~expVal) ;

			}
			else
			{
				expVal = 0x0 ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf8, expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf9, ~expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfa, expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfb, ~expVal) ;
			}
		}
		else if ( type == 3 )
		{
			Kern::Printf("Write/Read 0x5555 and 0xaaaa ...");
			if ( i % 2 == 0 )
			{
				expVal = 0x5555 ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf8, expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf9, ~expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfa, expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfb, ~expVal) ;
			}
			else
			{
				expVal = 0xaaaa ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf8, expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xf9, ~expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfa, expVal) ;
				CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(0xfb, ~expVal) ;
			}
		}

		CyAsHalBeagleBoardMcSPI4Ch0_ReadReg(0xf8, &regVal0) ;
		CyAsHalBeagleBoardMcSPI4Ch0_ReadReg(0xf9, &regVal1) ;
		CyAsHalBeagleBoardMcSPI4Ch0_ReadReg(0xfa, &regVal2) ;
		CyAsHalBeagleBoardMcSPI4Ch0_ReadReg(0xfb, &regVal3) ;

		if ( (type == 1) &&
			 (   (regVal0 != (i+1)) ||
				 (regVal1 != (i+2)) ||
				 (regVal2 != (i+3)) ||
				 (regVal3 != (i+4)) ))
		{
			Kern::Printf("ERROR: Write register failed (%d)\n",i);
			Kern::Printf("ERROR: Exp[0]: 0x%04x\n",i+1);
			Kern::Printf("ERROR: Act[0]: 0x%04x\n",regVal0);
			Kern::Printf("ERROR: Exp[1]: 0x%04x\n",i+2);
			Kern::Printf("ERROR: Act[1]: 0x%04x\n",regVal1);
			Kern::Printf("ERROR: Exp[2]: 0x%04x\n",i+3);
			Kern::Printf("ERROR: Act[2]: 0x%04x\n",regVal2);
			Kern::Printf("ERROR: Exp[3]: 0x%04x\n",i+4);
			Kern::Printf("ERROR: Act[3]: 0x%04x\n",regVal3);

			errCnt++ ;
			if ( errCnt > 10 )
				return 1;
		}
		else if ( (type != 1 ) &&
				  ((regVal0 != expVal) ||
				   (regVal1 != ((~expVal)&0xffff)) ||
				   (regVal2 != expVal) ||
				   (regVal3 != ((~expVal)&0xffff)) ))
		{
			Kern::Printf("ERROR: Write register failed (%d)\n",i);
			Kern::Printf("ERROR: Exp[0]: 0x%04x\n",expVal);
			Kern::Printf("ERROR: Act[0]: 0x%04x\n",regVal0);
			Kern::Printf("ERROR: Exp[1]: 0x%04x\n",(~expVal)&0xffff);
			Kern::Printf("ERROR: Act[1]: 0x%04x\n",regVal1);
			Kern::Printf("ERROR: Exp[2]: 0x%04x\n",expVal);
			Kern::Printf("ERROR: Act[2]: 0x%04x\n",regVal2);
			Kern::Printf("ERROR: Exp[3]: 0x%04x\n",(~expVal)&0xffff);
			Kern::Printf("ERROR: Act[3]: 0x%04x\n",regVal3);

			errCnt++ ;
			if ( errCnt > 10 )
				return 1 ;
		}

	}
	Kern::Printf("Register test PASSED!!\n");
	return 0 ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

static void
DumpMcSPI4Reg(void)
{
	TUint32 r ;
	TUint32 i;

	for ( i = KMcSPI4_SYSCONFIG ; i <= KMcSPI4_RX0 ; i+=0x4 )
	{
		r = AsspRegister::Read32(i);
		switch (i)
		{
			case KMcSPI4_SYSCONFIG:
				Kern::Printf("KMcSPI4_SYSCONFIG = (0x%08x)", r);
				break;
			case KMcSPI4_SYSSTATUS:
				Kern::Printf("KMcSPI4_SYSSTATUS = (0x%08x)", r);
				break;
			case KMcSPI4_IRQSTATUS:
				Kern::Printf("KMcSPI4_IRQSTATUS = (0x%08x)", r);
				break;
			case KMcSPI4_IRQENABLE:
				Kern::Printf("KMcSPI4_IRQENABLE = (0x%08x)", r);
				break;
			case KMcSPI4_WAKEUPEN:
				Kern::Printf("KMcSPI4_WAKEUPEN = (0x%08x)", r);
				break;
			case KMcSPI4_SYST:
				Kern::Printf("KMcSPI4_SYST = (0x%08x)", r);
				break;
			case KMcSPI4_MODULCTRL:
				Kern::Printf("KMcSPI4_MODULCTRL = (0x%08x)", r);
				break;
			case KMcSPI4_CH0CONF:
				Kern::Printf("KMcSPI4_CH0CONF = (0x%08x)", r);
				break;
			case KMcSPI4_CH0STAT:
				Kern::Printf("KMcSPI4_CH0STAT = (0x%08x)", r);
				break;
			case KMcSPI4_CH0CTRL:
				Kern::Printf("KMcSPI4_CH0CTRL = (0x%08x)", r);
				break;
			case KMcSPI4_TX0:
				Kern::Printf("KMcSPI4_TX0 = (0x%08x)", r);
				break;
			case KMcSPI4_RX0:
				Kern::Printf("KMcSPI4_RX0 = (0x%08x)", r);
				break;
			case KMcSPI4_XFERLEVEL:
				Kern::Printf("KMcSPI4_XFERLEVEL = (0x%08x)", r);
				break;
			default:
				Kern::Printf("ERROR: unknow register value", r);
		}
	}

	r = AsspRegister::Read32(KMcSPI4_XFERLEVEL);
	Kern::Printf("KMcSPI4_XFERLEVEL = (0x%08x)\n", r);
}

inline TUint32 constructSPIConfigReg(
		TUint8 PHA,
		TUint8 POL,
		TUint8 CLKD,
		TUint8 EPOL,
		TUint8 WL,
		TUint8 TRM,
		TUint8 DMAW,
		TUint8 DMAR,
		TUint8 DPE0,
		TUint8 DPE1,
		TUint8 IS,
		TUint8 TURBO,
		TUint8 FORCE,
		TUint8 SPIENSLV,
		TUint8 SBE,
		TUint8 SBPOL,
		TUint8 TCS,
		TUint8 FFEW,
		TUint8 FFER,
		TUint8 CLKG)
{
	TUint32 r = 0 ;

	r = ( CLKG & 0x1 ) << 29;
	r |= ( FFER & 0x1 ) << 28;
	r |= ( FFEW & 0x1 ) << 27;
	r |= ( TCS & 0x3 ) << 25;
	r |= ( SBPOL & 0x1 ) << 24;
	r |= ( SBE & 0x1 ) << 23;
	r |= ( SPIENSLV & 0x3 ) << 21;
	r |= ( FORCE & 0x1 ) << 20;
	r |= ( TURBO & 0x1 ) << 19;
	r |= ( IS & 0x1 ) << 18;
	r |= ( DPE1 & 0x1 ) << 17;
	r |= ( DPE0 & 0x1 ) << 16;
	r |= ( DMAR & 0x1 ) << 15;
	r |= ( DMAW & 0x1 ) << 14;
	r |= ( TRM & 0x3 ) << 12;
	r |= ( WL & 0x1f ) << 7;
	r |= ( EPOL & 0x1 ) << 6;
	r |= ( CLKD & 0xf ) << 2;
	r |= ( POL & 0x1 ) << 1;
	r |= PHA & 0x1;
	return r ;
}

static void ConfigMcSPI4Ch0(void)
{
	TUint32 r ;
	Kern::Printf("Configure McSPI4 CH0...\n");

	/* Enable McSPI4 functional clock */
	r = AsspRegister::Read32(KCM_FCLKEN1_CORE);
	AsspRegister::Write32(KCM_FCLKEN1_CORE, (r | 0x200000));

	/* Enable McSPI4 interface clock */
	r = AsspRegister::Read32(KCM_ICLKEN1_CORE);
	AsspRegister::Write32(KCM_ICLKEN1_CORE, (r | 0x200000));

	/* Software reset McSPI4 */
	AsspRegister::Write32(KMcSPI4_SYSCONFIG, 0x1);

	/* Wait until RESETDONE is set in SYSSTATUS */
	do
	{
		r = AsspRegister::Read32(KMcSPI4_SYSSTATUS);
		Kern::Printf("KMcSPI4_SYSSTATUS = (0x%08x)\n", r);
	}while((r & 0x1) != 1);

	/* Disable channel 0 */
	McSPI4Ch0_Disable();

	/* Disable all interrupts */
	AsspRegister::Write32(KMcSPI4_IRQENABLE, 0x0);

	/* Clear all interrupts */
	AsspRegister::Write32(KMcSPI4_IRQSTATUS, 0x1777f);

	/* Set Master mode */
#ifdef SINGLE_CHANNEL_MASTER_MODE
	AsspRegister::Write32(KMcSPI4_MODULCTRL, 0x1);
#else
	AsspRegister::Write32(KMcSPI4_MODULCTRL, 0x0);
#endif


	/* Config clock, wl */
	r = constructSPIConfigReg(
			0x0, /*PHA*/
			0x0, /*POL*/
			0x2, /*CLKD 	(divider 2 = 12MHz)3 = 6MHz*/
			0x1, /*EPOL 	(active low)*/
			0x7, /*WL		(8bit)*/
			0x0, /*TRM 		(Transmit and receive mode)*/
			0x0, /*DMAW 	(DMAW disable) TODO*/
			0x0, /*DMAR 	(DMAR disable) TODO*/
			0x1, /*DPE0 	(spim_somi no transmission)*/
			0x0, /*DPE1 	(spim_simo transmission)*/
			0x0, /*IS		(spim_somi reception)*/
			0x0, /*TURBO 	(mutli spi-word transfer)*/
			0x0, /*FORCE 	TODO*/
			0x0, /*SPIENSLV (not used)*/
			0x0, /*SBE		(no start bit) TODO*/
			0x0, /*SBPOL	(start bit polarity) */
			0x0, /*TCS		(cs time control) TODO*/
			0x0, /*FFEW		(FIFO enabled) TODO*/
			0x0, /*FFER		(FIFO enabled) TODO*/
			0x0  /*CLKG		(One clock cycle granularity)*/
			);

	AsspRegister::Write32(KMcSPI4_CH0CONF, r);

}

static void
SetPADConfigRegister(void)
{
	TUint32 r ;
	Kern::Printf("Set PAD configuration registers...\n");

	/* MUXMODE = 1, INPUTENABLE for SOMI */
	AsspRegister::Write32(KPADCONFIG_McSPI4_SIMO_P12_0, 0x1010001);

	r = AsspRegister::Read32(KPADCONFIG_McSPI4_CS0_P16_16);
	AsspRegister::Write32(KPADCONFIG_McSPI4_CS0_P16_16, ((r & 0xffff) | 0x10000));

	/* NOTE:
	   we must set SPI clock INPUTENABLE bit otherwise the RX register will not
	   latch any data */
	AsspRegister::Write32(KPADCONFIG_McSPI4_CLK_P20_0, 0x40001 | 0x0100);

	/* MUXMODE = 4 */
	r = AsspRegister::Read32(KPADCONFIG_GPIO183_P23_0);
	//AsspRegister::Write32(KPADCONFIG_GPIO183_P23_0, ((r & 0xffff0000) | 0x4 | 0x8 | 0x0100));
	AsspRegister::Write32(KPADCONFIG_GPIO183_P23_0, ((r & 0xffff0000) | 0x4 ));

	r = AsspRegister::Read32(KPADCONFIG_GPIO168_P24_16);
	AsspRegister::Write32(KPADCONFIG_GPIO168_P24_16, ((r & 0xffff) | 0x40000 | 0x01000000));

}

static void
ReadPADConfigRegister(void)
{
	TUint32 r ;

	Kern::Printf("Reading PAD configuration registers...");
	r = AsspRegister::Read32(KPADCONFIG_McSPI4_SIMO_P12_0);
	Kern::Printf("KPADCONFIG_McSPI4_SIMO_P12_0 = (0x%08x)", r);

	r = AsspRegister::Read32(KPADCONFIG_McSPI4_SIMO_P18_16);
	Kern::Printf("KPADCONFIG_McSPI4_SIMO_P18_16 = (0x%08x)", r);

	r = AsspRegister::Read32(KPADCONFIG_McSPI4_CS0_P16_16);
	Kern::Printf("KPADCONFIG_McSPI4_CS0_P16_16 = (0x%08x)", r);

	r = AsspRegister::Read32(KPADCONFIG_McSPI4_CLK_P20_0);
	Kern::Printf("KPADCONFIG_McSPI4_CLK_P20_0 = (0x%08x)", r);

	r = AsspRegister::Read32(KPADCONFIG_GPIO157_P22_16);
	Kern::Printf("KPADCONFIG_GPIO157_P22_16 = (0x%08x)", r);

	r = AsspRegister::Read32(KPADCONFIG_GPIO183_P23_0);
	Kern::Printf("KPADCONFIG_GPIO183_P23_0 = (0x%08x)", r);

	r = AsspRegister::Read32(KPADCONFIG_GPIO168_P24_16);
	Kern::Printf("KPADCONFIG_GPIO168_P24_16 = (0x%08x)", r);

}

/***********************************************************************/
/****************************   Public   *******************************/
/***********************************************************************/

void
CyAsHalBeagleBoardMcSPI4Ch0_WriteEP(TUint32 addr, TUint8* buff, TUint16 size)
{
	TUint8 size_l = size ;
	TUint8 size_h = (size >> 8)&0x7f ;
	TUint16 size16 = size / 2 ;
	TUint8* p = buff ;

	TUint32 r ;

	r = 0x00800000;
	r |= (addr & 0xff) << 8 ;
	r |= size_l << 24 ;
	r |= size_h << 16 ;
#ifdef SPI_DEBUG_LOG
	Kern::Printf("WB SPI write packet header = (0x%08x)", r);
#endif
	/* Set McSPI4Ch0 to Transmit only mode for write */
	McSPI4Ch0_TransmitMode();

	McSPI4Ch0_Enable();

	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, r);
	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, r>>8);
	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, r>>16);
	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, r>>24);

	while(size16 != 0)
	{
		CheckForTxEmpty();
		AsspRegister::Write32(KMcSPI4_TX0, *(p+1));
		CheckForTxEmpty();
		AsspRegister::Write32(KMcSPI4_TX0, *p);

/*#ifdef SPI_DEBUG_LOG
		Kern::Printf("0x%02x", *(p+1));
		Kern::Printf("0x%02x", *p);
#endif*/
		p+=2 ;
		size16-- ;
	}

	CheckForTxEmpty();

	McSPI4Ch0_Disable();

	/*
	 * Change back to Transmit and Receive mode for RegRead
	 */
	McSPI4Ch0_TransmitAndReceiveMode();

}

void
CyAsHalBeagleBoardMcSPI4Ch0_ReadEP(TUint32 addr, TUint8* buff, TUint16 size)
{
	TUint8 size_l = size ;
	TUint8 size_h = (size >> 8)&0x7f ;
	TUint16 size16 = size / 2 ;
	TUint32 r ;

	r = 0x00000000;
	r |= (addr & 0xff) << 8 ;
	r |= size_l << 24 ;
	r |= size_h << 16 ;
#ifdef SPI_DEBUG_LOG
	Kern::Printf("WB SPI read packet header = (0x%08x)", r);
#endif

	McSPI4Ch0_Enable();

	{
		CheckForTxEmpty();
		AsspRegister::Write32(KMcSPI4_TX0, r);
		CheckForTxEmptyRxFull();
		AsspRegister::Read32(KMcSPI4_RX0);

		AsspRegister::Write32(KMcSPI4_TX0, r>>8);
		CheckForTxEmptyRxFull();
		AsspRegister::Read32(KMcSPI4_RX0);


		AsspRegister::Write32(KMcSPI4_TX0, r>>16);
		CheckForTxEmptyRxFull();
		AsspRegister::Read32(KMcSPI4_RX0);

		AsspRegister::Write32(KMcSPI4_TX0, r>>24);
		CheckForTxEmptyRxFull();
		AsspRegister::Read32(KMcSPI4_RX0);

		while(size16 != 0)
		{
			AsspRegister::Write32(KMcSPI4_TX0, 0);
			CheckForTxEmptyRxFull();
			*(buff+1) = AsspRegister::Read32(KMcSPI4_RX0);

			AsspRegister::Write32(KMcSPI4_TX0, 0);
			CheckForTxEmptyRxFull();
			*buff = AsspRegister::Read32(KMcSPI4_RX0);

			buff+=2;
			size16--;

		}

	}

	McSPI4Ch0_Disable();

}

void
CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(TUint32 addr, TUint16 value)
{
	TUint32 r ;

	r = 0x02800000;
	r |= (addr & 0xff) << 8 ;
#ifdef SPI_DEBUG_LOG
	Kern::Printf("WB SPI write packet header = (0x%08x), value = (0x%04x)", r, value);
#endif

	/* Set McSPI4Ch0 to Transmit only mode for write */
	McSPI4Ch0_TransmitMode();

	McSPI4Ch0_Enable();

	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, r);
	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, r>>8);
	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, r>>16);
	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, r>>24);
	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, value>>8);
	CheckForTxEmpty();
	AsspRegister::Write32(KMcSPI4_TX0, value & 0xff );
	CheckForTxEmpty();

	McSPI4Ch0_Disable();

	/*
	 * Change back to Transmit and Receive mode for RegRead
	 */
	McSPI4Ch0_TransmitAndReceiveMode();
}

void
CyAsHalBeagleBoardMcSPI4Ch0_ReadReg(TUint32 addr, TUint16* value)
{
	TUint32 r ;

	r = 0x02000000;
	r |= (addr & 0xff) << 8 ;
#ifdef SPI_DEBUG_LOG
	Kern::Printf("WB SPI read packet header = (0x%08x)", r);
#endif

	McSPI4Ch0_Enable();

	{
		TUint16 v ;

		CheckForTxEmpty();
		AsspRegister::Write32(KMcSPI4_TX0, r);
		CheckForTxEmptyRxFull();
		AsspRegister::Read32(KMcSPI4_RX0);

		AsspRegister::Write32(KMcSPI4_TX0, r>>8);
		CheckForTxEmptyRxFull();
		AsspRegister::Read32(KMcSPI4_RX0);


		AsspRegister::Write32(KMcSPI4_TX0, r>>16);
		CheckForTxEmptyRxFull();
		AsspRegister::Read32(KMcSPI4_RX0);

		AsspRegister::Write32(KMcSPI4_TX0, r>>24);
		CheckForTxEmptyRxFull();
		AsspRegister::Read32(KMcSPI4_RX0);

		AsspRegister::Write32(KMcSPI4_TX0, 0);
		CheckForTxEmptyRxFull();
		v = AsspRegister::Read32(KMcSPI4_RX0);
		*value = v << 8;

		AsspRegister::Write32(KMcSPI4_TX0, 0);
		CheckForTxEmptyRxFull();
		v = AsspRegister::Read32(KMcSPI4_RX0);
		*value |= v & 0xff;

	}

	McSPI4Ch0_Disable();

}

int
CyAsHalBeagleBoard__ConfigureSPI(void)
{
	/*Configure PAD*/
	ReadPADConfigRegister();
	SetPADConfigRegister();
	ReadPADConfigRegister();

	/*Configure McSPI4*/
	ConfigMcSPI4Ch0();
	DumpMcSPI4Reg();

	McSPI4Ch0_Enable();

	if(KErrNone != GPIO::SetPinMode(KGPIO_183, GPIO::EEnabled))
		Kern::Printf("ERROR: SetPinMode failed!");

	if(KErrNone != GPIO::SetPinDirection(KGPIO_183, GPIO::EOutput))
		Kern::Printf("ERROR: SetPinDirection failed!");

	if(KErrNone != GPIO::SetOutputState(KGPIO_183, GPIO::EHigh))
		Kern::Printf("ERROR: SetOutputState failed!");

	gConfigured = 1 ;
	return 0 ;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

extern int
CyAsHalBeagleBoard__Test(void)
{
	int i ;
	int errCnt = 0 ;

	if ( !gConfigured )
	{
		CyAsHalBeagleBoard__ConfigureSPI();
	}

	{
		TUint16 regVal = 0 ;

		Kern::Printf("Read Astoria ID register\n");

		CyAsHalBeagleBoardMcSPI4Ch0_ReadReg(0x80, &regVal) ;

		if ( regVal != 0xA200 )
		{
			Kern::Printf("ERROR: Wrong ID register value\n");
			Kern::Printf("ERROR: Exp: 0x%04x\n",0xa200);
			Kern::Printf("ERROR: Act: 0x%04x\n",regVal);
			errCnt++;
			return errCnt ;
		}
		else
		{
			Kern::Printf("ID register read successful 0x%04x\n",regVal);
		}
	}

	{
		Kern::Printf("Register test...\n");
		for ( i = 1 ; i < 4 ; i++ )
		{
			if ( AstoriaRegTest(i) != 0 )
			{
				errCnt++ ;
				return errCnt ;
			}
		}

	}

	{

		Kern::Printf("Interrupt test...\n");

		/* Setup DFC */
		gpMyDfc = new TDfc( myTestDFCFn, (void*)(&gISRCnt), Kern::DfcQue0(), 1 ) ;
		CyAsHalBeagleBoard__SetupISR((void*)myISR,(void*)(0));

		for ( i = 0 ; i < 5 ; i++ )
		{
			Kern::Printf("Manually trigger interrupt (%d)\n",i);

			ManualTriggerISRTest();


			if (gISRCnt != i+1)
			{
				Kern::Printf("ISR called (%d)\n",gISRCnt);
			}
			else
			{
				Kern::Printf("ISR didn't call (%d)\n",gISRCnt);
				Kern::Printf("Interrupt test failed\n");
				errCnt++ ;
				return errCnt ;
			}
		}
	}

	Kern::Printf("CyAsHalBeagleBoard__Test Successful!\n");

	/*if ( gIndex == 1 )
	{
		GPIO::TGpioState aState ;

		GPIO::GetOutputState(KGPIO_183, aState);

		if ( aState == GPIO::EHigh )
		{
			if(KErrNone != GPIO::SetOutputState(KGPIO_183, GPIO::ELow))
				Kern::Printf("ERROR: SetOutputState failed!");
		}
		else
		{
			if(KErrNone != GPIO::SetOutputState(KGPIO_183, GPIO::EHigh))
				Kern::Printf("ERROR: SetOutputState failed!");
		}
		gIndex = 0 ;
	}*/

	/*
	if ( gIndex == 2 )
	{
		McSPI4Ch0_DumpAstoriaRegs(0);
	}

	if ( gIndex == 3 )
	{
		McSPI4Ch0_DumpAstoriaRegs(1);
		gIndex = 0 ;
	}
	*/
	return 0 ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/








