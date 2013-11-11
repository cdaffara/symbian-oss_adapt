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
// \omap3530\omap3530_assp\shared\tps65950\tps65950_int.cpp
// Interrupt dispatcher for TPS65950
// This file is part of the Beagle Base port
//

#include <e32cmn.h>
#include <nk_priv.h>
#include <kernel.h>
//#include <e32atomics.h>
#include <omap3530_i2c.h>
#include <omap3530_i2creg.h>
#include "tps65950.h"
#include <assp/omap3530_assp/locks.h>

// Use a dedicated thread for processing interrupts to prevent deadlocking
// the main TSP65960 thread and to avoid needing a big state machine to
// process interrupts in multiple DFCs
TDfcQue*	TheInterruptDfcQue;
_LIT( KInterruptDfcQueName, "TPS65950INT" );
const TInt KInterruptDfcQuePriority = 28;
const TInt KDfcPriority	= 2;

#define FULL_RISING_EDGEMASK  0xAA
#define FULL_FALLING_EDGEMASK 0x55


namespace TPS65950
{

struct TInterruptBank
	{
	TInt	iBit[8]; 
	};

struct TSubInterruptBank
	{
	TUint8	iLen;
	TUint16	iRegs[6];		
	};

enum TMaskPolarity
	{
	EClearToEnable,
	ESetToEnable
	};

struct  TControl
	{
	TUint16			iSetReg;
	TUint16			iClrReg;
	TUint16			iStatReg;
	TUint8			iBitMask;
	TMaskPolarity	iPolarity : 8;
	};

NONSHARABLE_CLASS( TPS65950Int ) : public MInterruptDispatcher
	{
	public:
		TPS65950Int();
		TInt Init();

		virtual TInt Bind(TInt aId, TIsr aIsr, TAny* aPtr);
		virtual TInt Unbind(TInt aId);
		virtual TInt Enable(TInt aId);
		virtual TInt Disable(TInt aId);
		virtual TInt Clear(TInt aId);
		virtual TInt SetPriority(TInt aId, TInt aPriority);

	private:
		static void Spurious( TAny* aParam );
		static void Dispatch( TAny* aParam );
		static void Dfc( TAny* aParam );

		TInt InitialiseTPS65950IntController();

	private:
		TDfc	iDfc;
	};

static SInterruptHandler TheHandlers[ TPS65950::KNumTPSInts ];

static const TControl KControl[ TPS65950::KNumTPSInts ] =
	{
//iimr iReg      group      bitoffset					/*	
/*0*/{	Register::PWR_IMR1,	Register::PWR_IMR1,	Register::PWR_ISR1,	PWR_IMR1::PWR_SC_DETECT,	ESetToEnable}, 		
/*1*/{	Register::PWR_IMR1,	Register::PWR_IMR1,	Register::PWR_ISR1,	PWR_IMR1::PWR_MBCHG,		ESetToEnable},
/*2*/{	Register::PWR_IMR1,	Register::PWR_IMR1,	Register::PWR_ISR1,	PWR_IMR1::PWR_PWROK_TIMEOUT, ESetToEnable},
/*3*/{	Register::PWR_IMR1,	Register::PWR_IMR1,	Register::PWR_ISR1,	PWR_IMR1::PWR_HOT_DIE,		ESetToEnable},	
/*4*/{	Register::PWR_IMR1,	Register::PWR_IMR1,	Register::PWR_ISR1,	PWR_IMR1::PWR_RTC_IT,		ESetToEnable},
/*5*/{	Register::PWR_IMR1,	Register::PWR_IMR1,	Register::PWR_ISR1,	PWR_IMR1::PWR_USB_PRES,		ESetToEnable},
/*6*/{	Register::PWR_IMR1,	Register::PWR_IMR1,	Register::PWR_ISR1,	PWR_IMR1::PWR_CHG_PRES,		ESetToEnable},
/*7*/{	Register::PWR_IMR1,	Register::PWR_IMR1,	Register::PWR_ISR1,	PWR_IMR1::PWR_CHG_PWRONS,	ESetToEnable},
			
/*8*/{	Register::MADC_IMR1,	Register::MADC_IMR1,	Register::MADC_ISR1,	MADC_IMR1::MADC_USB_ISR1,	EClearToEnable},
/*9*/{	Register::MADC_IMR1,	Register::MADC_IMR1,	Register::MADC_ISR1,	MADC_IMR1::MADC_SW2_ISR1,	EClearToEnable},
/*10*/{	Register::MADC_IMR1,	Register::MADC_IMR1,	Register::MADC_ISR1,	MADC_IMR1::MADC_SW1_ISR1,	EClearToEnable},
/*11*/{	Register::MADC_IMR1,	Register::MADC_IMR1,	Register::MADC_ISR1,	MADC_IMR1::MADC_RT_ISR1,	EClearToEnable},
			
/*12*/{	Register::GPIO_IMR1A,			Register::GPIO_IMR1A, Register::GPIO_ISR1A,	GPIO_IMR1A::GPIO7ISR1,	EClearToEnable},
/*13*/{	Register::GPIO_IMR1A,			Register::GPIO_IMR1A, Register::GPIO_ISR1A,	GPIO_IMR1A::GPIO6ISR1,	EClearToEnable},
/*14*/{	Register::GPIO_IMR1A,			Register::GPIO_IMR1A, Register::GPIO_ISR1A,	GPIO_IMR1A::GPIO5ISR1,	EClearToEnable},	
/*15*/{	Register::GPIO_IMR1A,			Register::GPIO_IMR1A, Register::GPIO_ISR1A,	GPIO_IMR1A::GPIO4ISR1,	EClearToEnable},
/*16*/{	Register::GPIO_IMR1A,			Register::GPIO_IMR1A, Register::GPIO_ISR1A,	GPIO_IMR1A::GPIO3ISR1,	EClearToEnable},
/*17*/{	Register::GPIO_IMR1A,			Register::GPIO_IMR1A, Register::GPIO_ISR1A,	GPIO_IMR1A::GPIO2ISR1,	EClearToEnable},
/*18*/{	Register::GPIO_IMR1A,			Register::GPIO_IMR1A, Register::GPIO_ISR1A,	GPIO_IMR1A::GPIO1ISR1,	EClearToEnable},
/*19*/{	Register::GPIO_IMR1A,			Register::GPIO_IMR1A, Register::GPIO_ISR1A,	GPIO_IMR1A::GPIO0ISR1,	EClearToEnable},
			
/*20*/	{	Register::GPIO_IMR2A,			Register::GPIO_IMR2A, Register::GPIO_ISR2A,	GPIO_IMR2A::GPIO15ISR2,	EClearToEnable},
/*22*/{	Register::GPIO_IMR2A,			Register::GPIO_IMR2A, Register::GPIO_ISR2A,	GPIO_IMR2A::GPIO14ISR2,	EClearToEnable},
/*23*/{	Register::GPIO_IMR2A,			Register::GPIO_IMR2A, Register::GPIO_ISR2A,	GPIO_IMR2A::GPIO13ISR2,	EClearToEnable},
/*24*/{	Register::GPIO_IMR2A,			Register::GPIO_IMR2A, Register::GPIO_ISR2A,	GPIO_IMR2A::GPIO12ISR2,	EClearToEnable},
/*25*/{	Register::GPIO_IMR2A,			Register::GPIO_IMR2A, Register::GPIO_ISR2A,	GPIO_IMR2A::GPIO11ISR2,	EClearToEnable},
/*26*/{	Register::GPIO_IMR2A,			Register::GPIO_IMR2A, Register::GPIO_ISR2A,	GPIO_IMR2A::GPIO10ISR2,	EClearToEnable},
/*27*/{	Register::GPIO_IMR2A,			Register::GPIO_IMR2A, Register::GPIO_ISR2A,	GPIO_IMR2A::GPIO9ISR2,	EClearToEnable},
/*28*/{	Register::GPIO_IMR2A,			Register::GPIO_IMR2A, Register::GPIO_ISR2A,	GPIO_IMR2A::GPIO8ISR2,	EClearToEnable},

/*29*/{	Register::GPIO_IMR3A,			Register::GPIO_IMR3A, Register::GPIO_ISR3A,	GPIO_IMR3A::GPIO17ISR3,	EClearToEnable},
/*30*/{	Register::GPIO_IMR3A,			Register::GPIO_IMR3A, Register::GPIO_ISR3A,	GPIO_IMR3A::GPIO16ISR3,	EClearToEnable},
			
/*31*/{	Register::BCIIMR1A,				Register::BCIIMR1A,		Register::BCIISR1A,	BCIIMR1A::BCI_BATSTS_ISR1,	EClearToEnable},
/*32*/{	Register::BCIIMR1A,				Register::BCIIMR1A,		Register::BCIISR1A,	BCIIMR1A::BCI_TBATOR1_ISR1,	EClearToEnable},
/*33*/{	Register::BCIIMR1A,				Register::BCIIMR1A,		Register::BCIISR1A,	BCIIMR1A::BCI_TBATOR2_ISR1,	EClearToEnable},
/*34*/{	Register::BCIIMR1A,				Register::BCIIMR1A,		Register::BCIISR1A,	BCIIMR1A::BCI_ICHGEOC_ISR1,	EClearToEnable},
/*35*/{	Register::BCIIMR1A,				Register::BCIIMR1A,		Register::BCIISR1A,	BCIIMR1A::BCI_ICHGLOW_ISR1ASTO,	EClearToEnable},
/*36*/{	Register::BCIIMR1A,				Register::BCIIMR1A,		Register::BCIISR1A,	BCIIMR1A::BCI_IICHGHIGH_ISR1,	EClearToEnable},
/*37*/{	Register::BCIIMR1A,				Register::BCIIMR1A,		Register::BCIISR1A,	BCIIMR1A::BCI_TMOVF_ISR1,	EClearToEnable},
/*38*/{	Register::BCIIMR1A,				Register::BCIIMR1A,		Register::BCIISR1A,	BCIIMR1A::BCI_WOVF_ISR1,	EClearToEnable},
	
/*39*/{	Register::BCIIMR2A,				Register::BCIIMR2A,		Register::BCIISR2A,	BCIIMR2A::BCI_ACCHGOV_ISR1,	EClearToEnable},
/*40*/{	Register::BCIIMR2A,				Register::BCIIMR2A,		Register::BCIISR2A,	BCIIMR2A::BCI_VBUSOV_ISR1,	EClearToEnable},
/*41*/{	Register::BCIIMR2A,				Register::BCIIMR2A,		Register::BCIISR2A,	BCIIMR2A::BCI_VBATOV_ISR1,	EClearToEnable},
/*42*/{	Register::BCIIMR2A,				Register::BCIIMR2A,		Register::BCIISR2A,	BCIIMR2A::BCI_VBATLVL_ISR1,	EClearToEnable},
			
/*43*/{	Register::KEYP_IMR1,			Register::KEYP_IMR1,	Register::KEYP_ISR1,	KEYP_IMR1::KEYP_ITMISR1,	EClearToEnable},
/*44*/{	Register::KEYP_IMR1,			Register::KEYP_IMR1,	Register::KEYP_ISR1,	KEYP_IMR1::KEYP_ITTOISR1,	EClearToEnable},
/*45*/{	Register::KEYP_IMR1,			Register::KEYP_IMR1,	Register::KEYP_ISR1,	KEYP_IMR1::KEYP_ITLKISR1,	EClearToEnable},
/*46*/{	Register::KEYP_IMR1,			Register::KEYP_IMR1,	Register::KEYP_ISR1,	KEYP_IMR1::KEYP_ITKPISR1,	EClearToEnable},
			
/*46*/{	Register::USB_INT_EN_RISE_SET,	Register::USB_INT_EN_RISE_CLR,	Register::USB_INT_STS, USB_INT_STS::USB_INTSTS_IDGND,		ESetToEnable }, 
/*47*/{	Register::USB_INT_EN_RISE_SET,	Register::USB_INT_EN_RISE_CLR,	Register::USB_INT_STS, USB_INT_STS::USB_INTSTS_SESSEND,		ESetToEnable },
/*48*/{	Register::USB_INT_EN_RISE_SET,	Register::USB_INT_EN_RISE_CLR,	Register::USB_INT_STS, USB_INT_STS::USB_INTSTS_SESSVALID,	ESetToEnable },
/*49*/{	Register::USB_INT_EN_RISE_SET,	Register::USB_INT_EN_RISE_CLR,	Register::USB_INT_STS, USB_INT_STS::USB_INTSTS_VBUSVALID,	ESetToEnable },	
/*50*/{	Register::USB_INT_EN_RISE_SET,	Register::USB_INT_EN_RISE_CLR,	Register::USB_INT_STS, USB_INT_STS::USB_INTSTS_HOSTDISCONNECT, ESetToEnable },
			
/*51*/{	Register::CARKIT_INT_EN_SET,	Register::CARKIT_INT_EN_CLR,	Register::CARKIT_INT_STS,	CARKIT_INT_STS::CARKIT_CARDP,	ESetToEnable },
/*52*/{	Register::CARKIT_INT_EN_SET,	Register::CARKIT_INT_EN_CLR,	Register::CARKIT_INT_STS,	CARKIT_INT_STS::CARKIT_CARINTDET,	ESetToEnable },
/*53*/{	Register::CARKIT_INT_EN_SET,	Register::CARKIT_INT_EN_CLR,	Register::CARKIT_INT_STS,	CARKIT_INT_STS::CARKIT_IDFLOAT,	ESetToEnable },
	
/*54*/{	Register::OTHER_INT_EN_RISE_SET, Register::OTHER_INT_EN_RISE_CLR, Register::OTHER_INT_STS,	OTHER_INT_STS::OTHER_INT_VB_SESS_VLD,	ESetToEnable },
/*55*/{	Register::OTHER_INT_EN_RISE_SET, Register::OTHER_INT_EN_RISE_CLR, Register::OTHER_INT_STS,	OTHER_INT_STS::OTHER_INT_DM_HI,	ESetToEnable },
/*56*/{	Register::OTHER_INT_EN_RISE_SET, Register::OTHER_INT_EN_RISE_CLR, Register::OTHER_INT_STS,	OTHER_INT_STS::OTHER_INT_DP_HI,	ESetToEnable },
/*57*/{	Register::OTHER_INT_EN_RISE_SET, Register::OTHER_INT_EN_RISE_CLR, Register::OTHER_INT_STS,	OTHER_INT_STS::OTHER_INT_MANU,	ESetToEnable },
/*58*/{	Register::OTHER_INT_EN_RISE_SET, Register::OTHER_INT_EN_RISE_CLR, Register::OTHER_INT_STS,	OTHER_INT_STS::OTHER_INT_ABNORMAL_STRESS,	ESetToEnable },
			
/*59*/{	Register::ID_INT_EN_RISE_SET,	Register::ID_INT_EN_RISE_CLR,	Register::ID_INT_STS,		ID_INT_STS::ID_INTID_RES_FLOAT,	ESetToEnable },
/*60*/{	Register::ID_INT_EN_RISE_SET,	Register::ID_INT_EN_RISE_CLR,	Register::ID_INT_STS,		ID_INT_STS::ID_INTID_RES_440K,	ESetToEnable },
/*61*/{	Register::ID_INT_EN_RISE_SET,	Register::ID_INT_EN_RISE_CLR,	Register::ID_INT_STS,		ID_INT_STS::ID_INTID_RES_200K,	ESetToEnable },
/*62*/{	Register::ID_INT_EN_RISE_SET,	Register::ID_INT_EN_RISE_CLR,	Register::ID_INT_STS,		ID_INT_STS::ID_INTID_RES_102K,	ESetToEnable },
				
/*63*/	{	Register::CARKIT_SM_1_INT_EN_SET, Register::CARKIT_SM_1_INT_EN_CLR, Register::CARKIT_SM_1_INT_STS, CARKIT_SM_1_INT_STS::CARKIT_SM_1_PSM_ERROR,	ESetToEnable },	
/*64*/	{	Register::CARKIT_SM_1_INT_EN_SET, Register::CARKIT_SM_1_INT_EN_CLR, Register::CARKIT_SM_1_INT_STS, CARKIT_SM_1_INT_STS::CARKIT_SM_1_PH_ACC,	ESetToEnable },
/*65*/	{	Register::CARKIT_SM_1_INT_EN_SET, Register::CARKIT_SM_1_INT_EN_CLR, Register::CARKIT_SM_1_INT_STS, CARKIT_SM_1_INT_STS::CARKIT_SM_1_CHARGER,	ESetToEnable },
/*66*/	{	Register::CARKIT_SM_1_INT_EN_SET, Register::CARKIT_SM_1_INT_EN_CLR, Register::CARKIT_SM_1_INT_STS, CARKIT_SM_1_INT_STS::CARKIT_SM_1_USB_HOST,	ESetToEnable },
/*67*/	{	Register::CARKIT_SM_1_INT_EN_SET, Register::CARKIT_SM_1_INT_EN_CLR, Register::CARKIT_SM_1_INT_STS, CARKIT_SM_1_INT_STS::CARKIT_SM_1_USB_OTG_B,	ESetToEnable },
/*68*/	{	Register::CARKIT_SM_1_INT_EN_SET, Register::CARKIT_SM_1_INT_EN_CLR, Register::CARKIT_SM_1_INT_STS, CARKIT_SM_1_INT_STS::CARKIT_SM_1_CARKIT,	ESetToEnable },
/*69*/	{	Register::CARKIT_SM_1_INT_EN_SET, Register::CARKIT_SM_1_INT_EN_CLR, Register::CARKIT_SM_1_INT_STS, CARKIT_SM_1_INT_STS::CARKIT_SM_1_DISCONNECTED,	ESetToEnable },
			
/*70*/	{	Register::CARKIT_SM_2_INT_EN_SET, Register::CARKIT_SM_2_INT_EN_CLR, Register::CARKIT_SM_2_INT_STS, CARKIT_SM_2_INT_STS::CARKIT_SM_2_STOP_PLS_MISS,	ESetToEnable },
/*71*/	{	Register::CARKIT_SM_2_INT_EN_SET, Register::CARKIT_SM_2_INT_EN_CLR, Register::CARKIT_SM_2_INT_STS, CARKIT_SM_2_INT_STS::CARKIT_SM_2_STEREO_TO_MONO,	ESetToEnable },
/*72*/	{	Register::CARKIT_SM_2_INT_EN_SET, Register::CARKIT_SM_2_INT_EN_CLR, Register::CARKIT_SM_2_INT_STS, CARKIT_SM_2_INT_STS::CARKIT_SM_2_PHONE_UART,	ESetToEnable },
/*73*/	{	Register::CARKIT_SM_2_INT_EN_SET, Register::CARKIT_SM_2_INT_EN_CLR, Register::CARKIT_SM_2_INT_STS, CARKIT_SM_2_INT_STS::CARKIT_SM_2_PH_NO_ACK,	ESetToEnable }
	};


const static TInterruptBank pwrBank = {
		ETPS65950_IRQ_PWR_CHG_PWRONS,
		ETPS65950_IRQ_PWR_CHG_PRES,
		ETPS65950_IRQ_PWR_USB_PRES,
		ETPS65950_IRQ_PWR_RTC_IT,
		ETPS65950_IRQ_PWR_HOT_DIE,
		ETPS65950_IRQ_PWR_PWROK_TIMEOUT,
		ETPS65950_IRQ_PWR_MBCHG,
		ETPS65950_IRQ_PWR_SC_DETECT,
};

const static TInterruptBank madcBank = {
		ETPS65950_IRQ_MADC_RT_ISR1,
		ETPS65950_IRQ_MADC_SW1_ISR1,
		ETPS65950_IRQ_MADC_SW2_ISR1,
		ETPS65950_IRQ_MADC_USB_ISR1,
};

const static TInterruptBank gpioBank0 = {
		ETPS65950_IRQ_GPIO_0ISR1,
		ETPS65950_IRQ_GPIO_1ISR1,	
		ETPS65950_IRQ_GPIO_2ISR1,
		ETPS65950_IRQ_GPIO_3ISR1,
		ETPS65950_IRQ_GPIO_4ISR1,
		ETPS65950_IRQ_GPIO_5ISR1,
		ETPS65950_IRQ_GPIO_6ISR1,
		ETPS65950_IRQ_GPIO_7ISR2	
};

const static TInterruptBank gpioBank1 = {
		ETPS65950_IRQ_GPIO_8ISR2,
		ETPS65950_IRQ_GPIO_9ISR2,
		ETPS65950_IRQ_GPIO_10ISR2,
		ETPS65950_IRQ_GPIO_11ISR2,
		ETPS65950_IRQ_GPIO_12ISR2,
		ETPS65950_IRQ_GPIO_13ISR2,
		ETPS65950_IRQ_GPIO_14ISR2,
		ETPS65950_IRQ_GPIO_15ISR2
};

const static TInterruptBank gpioBank2 = {
		ETPS65950_IRQ_GPIO16ISR3,
		ETPS65950_IRQ_GPIO17ISR3
};

const static TInterruptBank bciBank0 = {
		ETPS65950_IRQ_BCI_WOVF_ISR1,
		ETPS65950_IRQ_BCI_TMOVF_ISR1,
		ETPS65950_IRQ_BCI_IICHGHIGH_ISR1,
		ETPS65950_IRQ_BCI_ICHGLOW_ISR1ASTO,
		ETPS65950_IRQ_BCI_ICHGEOC_ISR1,
		ETPS65950_IRQ_BCI_TBATOR2_ISR1,
		ETPS65950_IRQ_BCI_TBATOR1_ISR1,
		ETPS65950_IRQ_BCI_BATSTS_ISR1
};

const static TInterruptBank bciBank1 = {
		ETPS65950_IRQ_BCI_VBATLVL_ISR1,
		ETPS65950_IRQ_BCI_VBATOV_ISR1,
		ETPS65950_IRQ_BCI_VBUSOV_ISR1,
		ETPS65950_IRQ_BCI_ACCHGOV_ISR1
};

const static TInterruptBank keypBank = {
		ETPS65950_IRQ_KEYP_ITKPISR1,
		ETPS65950_IRQ_KEYP_ITLKISR1,
		ETPS65950_IRQ_KEYP_ITTOISR1,
		ETPS65950_IRQ_KEYP_ITMISR1,
};

const static TInterruptBank usbINTSTSBank = {
		ETPS65950_IRQ_USB_INTSTS_IDGND, 
		ETPS65950_IRQ_USB_INTSTS_SESSEND,
		ETPS65950_IRQ_USB_INTSTS_SESSVALID,
		ETPS65950_IRQ_USB_INTSTS_VBUSVALID,	
		ETPS65950_IRQ_USB_INTSTS_HOSTDISCONNECT
};

const static TInterruptBank usbCARKITBank = {
		ETPS65950_IRQ_USB_CARKIT_CARDP,
		ETPS65950_IRQ_USB_CARKIT_CARINTDET,
		ETPS65950_IRQ_USB_CARKIT_IDFLOAT
};

const static TInterruptBank usbOTHERBank = {
		ETPS65950_IRQ_USB_OTHER_INT_VB_SESS_VLD,
		ETPS65950_IRQ_USB_OTHER_INT_DM_HI,
		ETPS65950_IRQ_USB_OTHER_INT_DP_HI,
		ETPS65950_IRQ_USB_OTHER_INT_MANU,
		ETPS65950_IRQ_USB_OTHER_INT_ABNORMAL_STRESS
};

const static TInterruptBank usbIDINTBank = {
		ETPS65950_IRQ_USB_ID_INT_ID_RES_FLOAT,
		ETPS65950_IRQ_USB_ID_INT_ID_RES_440K,
		ETPS65950_IRQ_USB_ID_INT_ID_RES_200K,
		ETPS65950_IRQ_USB_ID_INT_ID_RES_102K
};

const static TInterruptBank usbSM1Bank = {
		ETPS65950_IRQ_USB_CARKIT_SM_1_PSM_ERROR,	
		ETPS65950_IRQ_USB_CARKIT_SM_1_PH_ACC,
		ETPS65950_IRQ_USB_CARKIT_SM_1_CHARGER,
		ETPS65950_IRQ_USB_CARKIT_SM_1_USB_HOST,
		ETPS65950_IRQ_USB_CARKIT_SM_1_USB_OTG_B,
		ETPS65950_IRQ_USB_CARKIT_SM_1_CARKIT
};

const static TInterruptBank usbSM2Bank = {
		ETPS65950_IRQ_USB_CARKIT_SM_2_STOP_PLS_MISS,
		ETPS65950_IRQ_USB_CARKIT_SM_2_STEREO_TO_MONO,
		ETPS65950_IRQ_USB_CARKIT_SM_2_PHONE_UART,
		ETPS65950_IRQ_USB_CARKIT_SM_2_PH_NO_ACK
};

const static TInterruptBank* TheMapTable [6][6]  = {
 //maps against PIH_ISR bits		
 //reg banks	sub modules
				{&gpioBank0,		&gpioBank1,		&gpioBank2,		NULL,			NULL,			NULL},
				{&keypBank,			NULL,			NULL,			NULL,			NULL,			NULL},
				{&bciBank0,			&bciBank1,		NULL,			NULL,			NULL,			NULL},
				{&madcBank,			NULL,			NULL,			NULL,			NULL,			NULL},
				{&usbINTSTSBank,	&usbCARKITBank,	&usbOTHERBank,	&usbIDINTBank,	&usbSM1Bank,	&usbSM2Bank},
				{&pwrBank,			NULL,			NULL,			NULL,			NULL,			NULL}							
};

const static TSubInterruptBank subBank[6] = { 
	/*gpio*/{3,{Register::GPIO_ISR1A,Register::GPIO_ISR2A,Register::GPIO_ISR3A,NULL,NULL,NULL}},
		/*keyp*/{1,{Register::KEYP_ISR1,NULL,NULL,NULL,NULL,NULL}},
		/*bci*/	{2,{Register::BCIISR1A,Register::BCIISR2A,NULL,NULL,NULL,NULL}},
		/*madc*/{1,{Register::MADC_ISR1,NULL,NULL,NULL,NULL,NULL}},
		/*usb*/ {6,{Register::USB_INT_STS,Register::CARKIT_INT_STS,Register::OTHER_INT_STS,Register::ID_INT_STS, Register::CARKIT_SM_1_INT_STS, Register::CARKIT_SM_2_INT_STS}},
		/*pwr*/	{1,{Register::PWR_ISR1,NULL,NULL,NULL,NULL,NULL}},
};


TPS65950Int::TPS65950Int()
	: iDfc( Dfc, this, KDfcPriority )
	{
	for( TInt i = 0; i < TPS65950::KNumTPSInts; ++i )
		{
		TheHandlers[ i ].iIsr = Spurious;
		TheHandlers[ i ].iPtr = (TAny*)( KTPS65950IrqFirst + i );
		}
	}

TInt TPS65950Int::Init()
	{
	iDfc.SetDfcQ( TheInterruptDfcQue );

	TInt r = InitialiseTPS65950IntController();

	if( KErrNone == r )
		{
		TInt r = Interrupt::Bind( EOmap3530_IRQ7_SYS_NIRQ, Dispatch, this );
		if( KErrNone == r )
			{
			r = Interrupt::Enable( EOmap3530_IRQ7_SYS_NIRQ );
			}
		}

	if( KErrNone == r )
		{
		Register( EIrqRangeBasePsu );
		}
	return r;
	}


TInt TPS65950Int::InitialiseTPS65950IntController()
	{
	__KTRACE_OPT(KTPS65950,Kern::Printf("+TPS65950Int:InitIntController"));	

	struct TInitRegList
		{
		TUint16	iReg;
		TUint8	iValue;
		};

	static const TInitRegList KInitList[] =
		{
		{ GPIO_SIH_CTRL::Addr,	GPIO_SIH_CTRL::SIH_PENDDIS | GPIO_SIH_CTRL::SIH_COR | GPIO_SIH_CTRL::SIH_EXCLEN },
		// { Register::GPIO_CTRL,	0x00 },
		{ Register::GPIO_IMR1A ,	0xff },
		{ Register::GPIO_IMR2A ,	0xff },
		{ Register::GPIO_IMR3A ,	0xff },
		{ Register::GPIO_IMR1B ,	0xff },
		{ Register::GPIO_IMR2B ,	0xff },
		{ Register::GPIO_IMR3B ,	0xff },
		{ Register::GPIO_EDR1,	0x00 },
		{ Register::GPIO_EDR2,	0x00 },
		{ Register::GPIO_EDR3,	0x00 },
		{ Register::GPIO_EDR4,	0x00 },
		{ Register::GPIO_EDR5,	0x00 },
		{ Register::USB_INT_EN_RISE_CLR,	0x1f },
		{ Register::USB_INT_EN_FALL_CLR,	0x1f },
		{ Register::CARKIT_INT_EN_CLR,	0x1f },
		{ Register::OTHER_INT_EN_RISE_CLR,0xe3 },
		{ Register::OTHER_INT_EN_FALL_CLR,0xe3 },
		{ Register::ID_INT_EN_RISE_CLR,	0x0f },
		{ Register::ID_INT_EN_FALL_CLR,	0x0f },
		{ Register::CARKIT_SM_1_INT_EN_CLR,0xff },
		{ Register::CARKIT_SM_2_INT_EN_CLR,0xff },		
		{ KEYP_SIH_CTRL::Addr,	KEYP_SIH_CTRL::SIH_PENDDIS |  KEYP_SIH_CTRL::SIH_COR | KEYP_SIH_CTRL::SIH_EXCLEN },
		{ Register::KEYP_IMR1,		0x0f },
		{ Register::KEYP_IMR2,		0x0f },
		{ Register::KEYP_EDR,		FULL_RISING_EDGEMASK },
		{ BCISIHCTRL::Addr,	BCISIHCTRL::SIH_PENDDIS |  BCISIHCTRL::SIH_COR | BCISIHCTRL::SIH_EXCLEN },
		{ Register::BCIIMR1A,	0xff },
		{ Register::BCIIMR2A,	0xff },
		{ Register::BCIIMR1B,	0xff },
		{ Register::BCIIMR2B,	0xff },
		{ Register::BCIEDR1,	FULL_RISING_EDGEMASK },
		{ Register::BCIEDR2,	FULL_RISING_EDGEMASK },
		{ Register::BCIEDR3,	FULL_RISING_EDGEMASK },
		{ MADC_SIH_CTRL::Addr,	MADC_SIH_CTRL::SIH_PENDDIS |  MADC_SIH_CTRL::SIH_COR | MADC_SIH_CTRL::SIH_EXCLEN },
		{ Register::MADC_IMR1,	0x0f },
		{ Register::MADC_IMR2,	0x0f },
		{ Register::MADC_EDR,	FULL_RISING_EDGEMASK },
		{ PWR_SIH_CTRL::Addr,	PWR_SIH_CTRL::SIH_PENDDIS |  PWR_SIH_CTRL::SIH_COR | PWR_SIH_CTRL::SIH_EXCLEN },
		{ Register::PWR_IMR1,		0xff },
		{ Register::PWR_IMR2,		0xff },
		{ Register::PWR_EDR1,		FULL_FALLING_EDGEMASK },
		{ Register::PWR_EDR2,		FULL_FALLING_EDGEMASK }
		};

	const TInt KInitListCount = (sizeof( KInitList ) / sizeof( KInitList[0] )  );

	static const TUint16 KClearList[] =
		{
		Register::CARKIT_INT_LATCH,
		Register::USB_INT_LATCH,
		Register::OTHER_INT_LATCH,
		Register::ID_INT_LATCH,
		Register::CARKIT_SM_1_INT_LATCH,
		Register::CARKIT_SM_2_INT_LATCH,
		Register::GPIO_ISR1A,
		Register::GPIO_ISR2A,
		Register::GPIO_ISR3A,
		Register::KEYP_ISR1,
		Register::BCIISR1A,
		Register::BCIISR2A,
		Register::MADC_ISR1,
		Register::PWR_ISR1
		};

	const TInt KClearListCount = (sizeof( KClearList ) / sizeof( KClearList[0] ) );


	TInt r = KErrNone;

	// Disable all interrupts
	for( TInt i = 0; (i < KInitListCount) && (KErrNone == r); ++i )
		{
		r = WriteSync( KInitList[i].iReg, KInitList[i].iValue );
		}

	// Clear all interrupts
	for( TInt i = 0; (i < KClearListCount) && (KErrNone == r); ++i )
		{
		TUint8 dummy;
		r = ReadSync( KClearList[i], dummy );
		}

	__KTRACE_OPT(KTPS65950,Kern::Printf("-TPS65950Int:InitIntController:%d", r));	

	return r;
	}


void TPS65950Int::Spurious( TAny* aParam )
	{
	Kern::Fault("TPS65950SpurioustInt", (TInt)aParam );
	}

TInt TPS65950Int::Bind(TInt aId, TIsr aIsr, TAny* aPtr)
	{
	__KTRACE_OPT(KTPS65950,Kern::Printf("+tps65950:Bind:%x->%x", aId, aIsr ));

	TInt r = KErrNone;

	if( (TUint)aId < KTPS65950IrqLast )
		{
		TUint tblOffset = aId - KTPS65950IrqFirst;

		TInt irq=__SPIN_LOCK_IRQSAVE_W(BeagleExtIVTLock);
		TheHandlers[tblOffset ].iIsr = aIsr;
		TheHandlers[tblOffset].iPtr = aPtr;
		__SPIN_UNLOCK_IRQRESTORE_W(BeagleExtIVTLock,irq);

		}
	else
		{
		r = KErrArgument;
		}

	__KTRACE_OPT(KTPS65950,Kern::Printf("-tps65950:Bind:%x:%d", aId, r ));
	return r;
	}

TInt TPS65950Int::Unbind(TInt aId)
	{
	__KTRACE_OPT(KTPS65950,Kern::Printf("+tps65950:Unbind:%x", aId ));

	TInt r = KErrNone;

	if( (TUint)aId < KTPS65950IrqLast )
		{
		TUint tblOffset = aId - KTPS65950IrqFirst;
		TInt irq=__SPIN_LOCK_IRQSAVE_W(BeagleExtIVTLock);
		TheHandlers[tblOffset ].iIsr = Spurious;
		TheHandlers[tblOffset ].iPtr = NULL;
		__SPIN_UNLOCK_IRQRESTORE_W(BeagleExtIVTLock,irq);
		}
	else
		{
		r = KErrArgument;
		}

	__KTRACE_OPT(KTPS65950,Kern::Printf("-tps65950:Unbind:%x:%d", aId, r ));
	return r;
	}

TInt TPS65950Int::Enable(TInt aId)
	{
	__KTRACE_OPT(KTPS65950,Kern::Printf("+tps65950:Enable:%x", aId ));

	TInt r = KErrNone;

	if( (TUint)aId < KTPS65950IrqLast )
		{
		CHECK_PRECONDITIONS(MASK_NOT_ISR | MASK_NOT_IDFC,"tps65950::InterruptEnable Cant enable a slow src in ISR Context");

		TUint tblOffset = aId - KTPS65950IrqFirst;
		
		TInt irq=__SPIN_LOCK_IRQSAVE_R(BeagleExtIVTLock);
		if( TheHandlers[ tblOffset ].iIsr == Spurious )
			{
			r = KErrNotReady;
			}
		__SPIN_UNLOCK_IRQRESTORE_R(BeagleExtIVTLock,irq);
				
		if( r != KErrNone )
			{
			__KTRACE_OPT(KTPS65950,Kern::Printf("=tps65950:Enable:%d NOT BOUND", aId ));	
			}
		else
			{
			const TControl& control = KControl[ tblOffset ];
		
			TUint8 val;
			ReadSync( control.iSetReg, val );
			if( EClearToEnable == control.iPolarity )
				{
				ClearSetSync( control.iSetReg, control.iBitMask, KSetNone );
				}
			else
				{
				ClearSetSync( control.iSetReg, KClearNone, control.iBitMask );
				}
			ReadSync( control.iSetReg, val );
			}
		}
	else
		{
		r = KErrArgument;
		}

	__KTRACE_OPT(KTPS65950,Kern::Printf("-tps65950:Enable:%x:%d", aId, r ));
	return r;
	}

TInt TPS65950Int::Disable(TInt aId)
	{
	__KTRACE_OPT(KTPS65950,Kern::Printf("+tps65950:Disable:%x", aId ));

	TInt r = KErrNone;

	if( (TUint)aId < KTPS65950IrqLast )
		{
		CHECK_PRECONDITIONS(MASK_NOT_ISR | MASK_NOT_IDFC,"tps65950::InterruptDisable Cant disable a slow src in ISR Context");

		TUint tblOffset = aId - KTPS65950IrqFirst;
		
		TInt irq=__SPIN_LOCK_IRQSAVE_R(BeagleExtIVTLock);
		if( TheHandlers[ tblOffset ].iIsr == Spurious )
			{
			r = KErrNotReady;
			}
		__SPIN_UNLOCK_IRQRESTORE_R(BeagleExtIVTLock,irq);
				
		if( r != KErrNone )
			{
			__KTRACE_OPT(KTPS65950,Kern::Printf("=tps65950:Disable:%d NOT BOUND", aId ));	
			}
		else
			{
			const TControl& control = KControl[ tblOffset ];
		
			if( EClearToEnable == control.iPolarity )
				{
				ClearSetSync( control.iClrReg, KClearNone, control.iBitMask );
				}
			else
				{
				ClearSetSync( control.iSetReg, control.iBitMask, KSetNone );
				}
			}
		}
	else
		{
		r = KErrArgument;
		}

	__KTRACE_OPT(KTPS65950,Kern::Printf("-tps65950:Disable:%x:%d", aId, r ));
	return r;
	}

TInt TPS65950Int::Clear(TInt aId)
	{
	__KTRACE_OPT(KTPS65950,Kern::Printf("+tps65950:Clear:%x", aId ));

	TInt r = KErrNone;

	if( (TUint)aId < KTPS65950IrqLast )
		{
		CHECK_PRECONDITIONS(MASK_NOT_ISR,"tps65950::InterruptClear Cant clear a slow src in ISR Context");
		
		TUint tblOffset = aId - KTPS65950IrqFirst;
		TUint8 value;
		//clear on read !  //we may lose some of the other ints if many enabled
		ReadSync( KControl[ tblOffset ].iStatReg, value );
		}
	else
		{
		r = KErrArgument;
		}

	__KTRACE_OPT(KTPS65950,Kern::Printf("-tps65950:Clear:%x:%d", aId, r ));
	return r;
	}

TInt TPS65950Int::SetPriority(TInt aId, TInt aPriority)
	{
	__KTRACE_OPT(KTPS65950,Kern::Printf("tps65950:SetPriority:%x", aId ));
	return KErrNotSupported;
	}


void TPS65950Int::Dispatch(TAny * aParam )
	{	
	Interrupt::Disable(EOmap3530_IRQ7_SYS_NIRQ);
	reinterpret_cast<TPS65950Int*>(aParam)->iDfc.Add();	
	}

void TPS65950Int::Dfc( TAny* aParam )
	{	
	__KTRACE_OPT(KTPS65950,Kern::Printf("+tps65950Int:Dfc" ));

	TUint8 highVectors=0;
	TUint8 subVector=0;
	
	ReadSync( PIH_ISR_P1::Addr, highVectors );
	__ASSERT_DEBUG( highVectors != 0,Kern::Fault("tps65950 int signalled but no vector ",highVectors));
	
	for(TInt i=0; i<=5;i++,highVectors >>=1)
		{
		if(highVectors & 0x1)			
			{
			for(TInt8 j=0;j<subBank[i].iLen;j++)
				{
				ReadSync( subBank[i].iRegs[j], subVector );
				for(TInt k=0;k < 8;k++)
					{
					if(subVector & 0x1)
						{	
						TInt tblOffset =  TheMapTable[i][j]->iBit[k] - KTPS65950IrqFirst;

						__KTRACE_OPT(KTPS65950,Kern::Printf("=tps65950:Dfc:BIT_%d HIGH on REG %x VECTOR is %x ISR %x",
													k,subBank[i].iRegs[j], tblOffset, TheHandlers[tblOffset].iIsr));
						
						(TheHandlers[tblOffset].iIsr)(TheHandlers[tblOffset].iPtr);
						}	
					subVector >>= 1;
					}
				}		
			}
		}		
	Interrupt::Enable(EOmap3530_IRQ7_SYS_NIRQ);	

	__KTRACE_OPT(KTPS65950,Kern::Printf("-tps65950:Dfc" ));
	}


} // namespace TPS65950


GLDEF_C TInt InitInterrupts()
	{
	TInt r = Kern::DfcQCreate( TheInterruptDfcQue, KInterruptDfcQuePriority, &KInterruptDfcQueName );
	if( KErrNone == r )
		{
		r = KErrNoMemory;
		TPS65950::TPS65950Int* dispatcher = new TPS65950::TPS65950Int;
		if( dispatcher )
			{
			r = dispatcher->Init();
			}
		}
	return r;
	}


