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

#ifndef TPS65950_REGISTERS_H
#define TPS65950_REGISTERS_H

#include <e32cmn.h>

namespace TPS65950
{

namespace Register
	{
	const TUint KGroupShift = 8;
	const TUint KRegisterMask = 0xFF;
	const TUint KGroupMask = 0xFF00;

	enum TGroup
		{
		EGroup12 = (0 << KGroupShift),
		EGroup48 = (1 << KGroupShift),
		EGroup49 = (2 << KGroupShift),
		EGroup4a = (3 << KGroupShift),
		EGroup4b = (4 << KGroupShift)
		};

	enum TGroup12Registers
		{
		VDD1_SR_CONTROL = EGroup12,
		VDD2_SR_CONTROL
		};


	enum TGroup48Registers
		{
		VENDOR_ID_LO =  EGroup48,
		VENDOR_ID_HI, 
		PRODUCT_ID_LO,
		PRODUCT_ID_HI,
		FUNC_CTRL,
		FUNC_CTRL_SET,
		FUNC_CTRL_CLR,
		IFC_CTRL,
		IFC_CTRL_SET, 
		IFC_CTRL_CLR, 
		OTG_CTRL,
		OTG_CTRL_SET,
		OTG_CTRL_CLR,
		USB_INT_EN_RISE, 
		USB_INT_EN_RISE_SET,
		USB_INT_EN_RISE_CLR,
		USB_INT_EN_FALL, 
		USB_INT_EN_FALL_SET, 
		USB_INT_EN_FALL_CLR,
		USB_INT_STS, 
		USB_INT_LATCH,
		DEBUG, 
		SCRATCH_REG, 
		SCRATCH_REG_SET,
		SCRATCH_REG_CLR,
		CARKIT_CTRL, 
		CARKIT_CTRL_SET, 
		CARKIT_CTRL_CLR, 
		CARKIT_INT_DELAY,
		CARKIT_INT_EN, 
		CARKIT_INT_EN_SET,
		CARKIT_INT_EN_CLR,
		CARKIT_INT_STS, 
		CARKIT_INT_LATCH, 
		CARKIT_PLS_CTRL, 
		CARKIT_PLS_CTRL_SET, 
		CARKIT_PLS_CTRL_CLR,
		TRANS_POS_WIDTH, 
		TRANS_NEG_WIDTH,
		RCV_PLTY_RECOVERY,
		MCPC_CTRL = 0x30, 
		MCPC_CTRL_SET,
		MCPC_CTRL_CLR,
		MCPC_IO_CTRL,
		MCPC_IO_CTRL_SET, 
		MCPC_IO_CTRL_CLR,
		MCPC_CTRL2, 
		MCPC_CTRL2_SET, 
		MCPC_CTRL2_CLR, 
		OTHER_FUNC_CTRL = EGroup48 + 0x80, 
		OTHER_FUNC_CTRL_SET, 
		OTHER_FUNC_CTRL_CLR, 
		OTHER_IFC_CTRL,
		OTHER_IFC_CTRL_SET,
		OTHER_IFC_CTRL_CLR,
		OTHER_INT_EN_RISE,
		OTHER_INT_EN_RISE_SET,
		OTHER_INT_EN_RISE_CLR,
		OTHER_INT_EN_FALL,
		OTHER_INT_EN_FALL_SET,
		OTHER_INT_EN_FALL_CLR,
		OTHER_INT_STS,
		OTHER_INT_LATCH,
		ID_INT_EN_RISE,
		ID_INT_EN_RISE_SET,
		ID_INT_EN_RISE_CLR,
		ID_INT_EN_FALL,
		ID_INT_EN_FALL_SET,
		ID_INT_EN_FALL_CLR,
		ID_INT_STS,
		ID_INT_LATCH,
		ID_STATUS,
		CARKIT_SM_1_INT_EN,
		CARKIT_SM_1_INT_EN_SET,
		CARKIT_SM_1_INT_EN_CLR,
		CARKIT_SM_1_INT_STS,
		CARKIT_SM_1_INT_LATCH,
		CARKIT_SM_2_INT_EN,
		CARKIT_SM_2_INT_EN_SET,
		CARKIT_SM_2_INT_EN_CLR,
		CARKIT_SM_2_INT_STS,
		CARKIT_SM_2_INT_LATCH,
		CARKIT_SM_CTRL,
		CARKIT_SM_CTRL_SET,
		CARKIT_SM_CTRL_CLR,
		CARKIT_SM_CMD,
		CARKIT_SM_CMD_SET,
		CARKIT_SM_CMD_CLR,
		CARKIT_SM_CMD_STS,
		CARKIT_SM_STATUS,
		CARKIT_SM_NEXT_STATUS,
		CARKIT_SM_ERR_STATUS,
		CARKIT_SM_CTRL_STATE,
		POWER_CTRL,
		POWER_CTRL_SET,
		POWER_CTRL_CLR,
		OTHER_IFC_CTRL2,
		OTHER_IFC_CTRL2_SET,
		OTHER_IFC_CTRL2_CLR,
		REG_CTRL_EN,
		REG_CTRL_EN_SET,
		REG_CTRL_EN_CLR,
		REG_CTRL_ERROR,
		OTHER_FUNC_CTRL2,
		OTHER_FUNC_CTRL2_SET,
		OTHER_FUNC_CTRL2_CLR,
		CARKIT_ANA_CTRL,
		CARKIT_ANA_CTRL_SET,
		CARKIT_ANA_CTRL_CLR,
		VBUS_DEBOUNCE = EGroup48 + 0xC0,
		ID_DEBOUNCE,
		TPH_DP_CON_MIN,
		TPH_DP_CON_MAX,
		TCR_DP_CON_MIN,
		TCR_DP_CON_MAX,
		TPH_DP_PD_SHORT,
		TPH_CMD_DLY,
		TPH_DET_RST,
		TPH_AUD_BIAS,
		TCR_UART_DET_MIN,
		TCR_UART_DET_MAX,
		TPH_ID_INT_PW = EGroup48 + 0xCC,
		TACC_ID_INT_WAIT,
		TACC_ID_INT_PW,
		TPH_CMD_WAIT = EGroup48 + 0xD0,
		TPH_ACK_WAIT,
		TPH_DP_DISC_DET,
		VBAT_TIMER,
		CARKIT_4W_DEBUG = EGroup48 + 0xE0,
		CARKIT_5W_DEBUG,
		TEST_CTRL_CLR = EGroup48 + 0xEB,
		TEST_CARKIT_SET,
		TEST_CARKIT_CLR,
		TEST_POWER_SET,
		TEST_POWER_CLR,
		TEST_ULPI,
		TXVR_EN_TEST_SET,
		TXVR_EN_TEST_CLR,
		VBUS_EN_TEST,
		ID_EN_TEST,
		PSM_EN_TEST_SET,
		PSM_EN_TEST_CLR,
		PHY_TRIM_CTRL = EGroup48 + 0xFC,
		PHY_PWR_CTRL,
		PHY_CLK_CTRL,
		PHY_CLK_CTRL_STS // 0x000000ff
		};


	enum TGroup49Registers
		{
		CODEC_MODE = EGroup49 + 1,
		OPTION , 
		MICBIAS_CTL = EGroup49 + 0x04,
		ANAMICL,
		ANAMICR,
		AVADC_CTL,
		ADCMICSEL,
		DIGMIXING,
		ATXL1PGA,
		ATXR1PGA,
		AVTXL2PGA,
		AVTXR2PGA,
		AUDIO_IF,
		VOICE_IF,
		ARXR1PGA,
		ARXL1PGA,
		ARXR2PGA,
		ARXL2PGA,
		VRXPGA,
		VSTPGA,
		VRX2ARXPGA,
		AVDAC_CTL,
		ARX2VTXPGA,
		ARXL1_APGA_CTL,
		ARXR1_APGA_CTL,
		ARXL2_APGA_CTL,
		ARXR2_APGA_CTL,
		ATX2ARXPGA,
		BT_IF,
		BTPGA,
		BTSTPGA,
		EAR_CTL,
		HS_SEL,
		HS_GAIN_SET,
		HS_POPN_SET,
		PREDL_CTL,
		PREDR_CTL,
		PRECKL_CTL,
		PRECKR_CTL,
		HFL_CTL,
		HFR_CTL, 
		ALC_CTL, 
		ALC_SET1,
		ALC_SET2,
		BOOST_CTL,
		SOFTVOL_CTL,
		DTMF_FREQSEL,
		DTMF_TONEXT1H,
		DTMF_TONEXT1L,
		DTMF_TONEXT2H,
		DTMF_TONEXT2L,
		DTMF_TONOFF,
		DTMF_WANONOFF,//  8 0x0000 0036
		
		I2S_RX_SCRAMBLE_H, 
		I2S_RX_SCRAMBLE_M, 
		I2S_RX_SCRAMBLE_L,
		APLL_CTL,
		DTMF_CTL,
		DTMF_PGA_CTL2,
		DTMF_PGA_CTL1,
		MISC_SET_1,
		PCMBTMUX,
		RX_PATH_SEL,
		VDL_APGA_CTL,
		VIBRA_CTL,
		VIBRA_SET,
		ANAMIC_GAIN,
		MISC_SET_2,// RW 8 0x0000 0049
		
		AUDIO_TEST_CTL = EGroup49 + 0x0000004C,
		INT_TEST_CTL,
		DAC_ADC_TEST_CTL,
		RXTX_TRIM_IB,
		CLD_CONTROL,
		CLD_MODE_TIMING,
		CLD_TRIM_RAMP,
		CLD_TESTV_CTL,
		APLL_TEST_CTL,
		APLL_TEST_DIV,
		APLL_TEST_CTL2,
		APLL_TEST_CUR,
		DIGMIC_BIAS1_CTL,
		DIGMIC_BIAS2_CTL,
		RX_OFFSET_VOICE,
		RX_OFFSET_AL1,
		RX_OFFSET_AR1,
		RX_OFFSET_AL2,
		RX_OFFSET_AR2,
		OFFSET1,
		OFFSET2,
		
		
		GPIODATAIN1  = EGroup49 + 0x00000098,
		GPIODATAIN2, 
		GPIODATAIN3, 
		GPIODATADIR1, 
		GPIODATADIR2, 
		GPIODATADIR3, 
		GPIODATAOUT1,
		GPIODATAOUT2,
		GPIODATAOUT3,
		CLEARGPIODATAOUT1,
		CLEARGPIODATAOUT2,
		CLEARGPIODATAOUT3,
		SETGPIODATAOUT1,
		SETGPIODATAOUT2,
		SETGPIODATAOUT3,
		GPIO_DEBEN1,
		GPIO_DEBEN3,
		GPIO_CTRL ,
		GPIOPUPDCTR1,
		GPIOPUPDCTR2,
		GPIOPUPDCTR3,
		GPIOPUPDCTR4,
		GPIOPUPDCTR5,
		GPIO_TEST,
		GPIO_ISR1A = EGroup49 + 0xb1,
		GPIO_ISR2A,
		GPIO_ISR3A,
		GPIO_IMR1A, // 
		GPIO_IMR2A,
		GPIO_IMR3A,
		GPIO_ISR1B,
		GPIO_ISR2B,
		GPIO_ISR3B,
		GPIO_IMR1B,
		GPIO_IMR2B,
		GPIO_IMR3B,
		GPIO_SIR1,
		GPIO_SIR2,
		GPIO_SIR3,
		GPIO_EDR1,
		GPIO_EDR2,
		GPIO_EDR3,
		GPIO_EDR4,
		GPIO_EDR5,
		GPIO_SIH_CTRL,
		
		PIH_ISR_P1 = EGroup49 + 0x00000081,
		PIH_ISR_P2,
		PIH_SIR ,
		IDCODE_7_0 = EGroup49 + 0x00000085,
		IDCODE_15_8,
		IDCODE_23_16,
		IDCODE_31_24,
		DIEID_7_0,
		DIEID_15_8,
		DIEID_23_16,
		DIEID_31_24,
		DIEID_39_32,
		DIEID_47_40,
		DIEID_55_48,
		DIEID_63_56,
		GPBR1,
		PMBR1,
		PMBR2,
		GPPUPDCTR1,
		GPPUPDCTR2,
		GPPUPDCTR3,
		UNLOCK_TEST_REG,
		};



	/*
	Note: Access to the following registers is protected:
	� IDCODE_7_0
	� IDCODE_15_8
	� IDCODE_23_16
	� IDCODE_31_24
	� DIEID_7_0
	� DIEID_15_8
	� DIEID_23_16
	� DIEID_31_24
	� DIEID_39_32
	� DIEID_47_40
	� DIEID_55_48
	� DIEID_63_56
	To read these registers, the UNLOCK_TEST_REG register must first be written with 0x49.
	Table 2-29. GPPUPDCTR1
	Address Offset 0x0F
	Physical Address 0x0000 0094 Instance INT_SCINTBR
	*/

	enum TGroup4aRegisters
		{
		CTRL1 = EGroup4a + 0,
		CTRL2,
		RTSELECT_LSB,
		RTSELECT_MSB,
		RTAVERAGE_LSB,
		RTAVERAGE_MSB,
		SW1SELECT_LSB,
		SW1SELECT_MSB,
		SW1AVERAGE_LSB,
		SW1AVERAGE_MSB,
		SW2SELECT_LSB,
		SW2SELECT_MSB,
		SW2AVERAGE_LSB,
		SW2AVERAGE_MSB,
		BCI_USBAVERAGE,
		ACQUISITION,
		USBREF_LSB,
		USBREF_MSB,
		CTRL_SW1,
		CTRL_SW2,
		MADC_TEST,
		GP_MADC_TEST1,
		GP_MADC_TEST2,
		RTCH0_LSB, 
		RTCH0_MSB, 
		RTCH1_LSB, 
		RTCH1_MSB, 
		RTCH2_LSB, 
		RTCH2_MSB, 
		RTCH3_LSB, 
		RTCH3_MSB,
		RTCH4_LSB,
		RTCH4_MSB,
		RTCH5_LSB,
		RTCH5_MSB,
		RTCH6_LSB,
		RTCH6_MSB,
		RTCH7_LSB,
		RTCH7_MSB,
		RTCH8_LSB,
		RTCH8_MSB,
		RTCH9_LSB,
		RTCH9_MSB,
		RTCH10_LSB,
		RTCH10_MSB,
		RTCH11_LSB,
		RTCH11_MSB,
		RTCH12_LSB,
		RTCH12_MSB,
		RTCH13_LSB,
		RTCH13_MSB,
		RTCH14_LSB,
		RTCH14_MSB,
		RTCH15_LSB,
		RTCH15_MSB,
		GPCH0_LSB,
		GPCH0_MSB,
		GPCH1_LSB,
		GPCH1_MSB,
		GPCH2_LSB,
		GPCH2_MSB, 
		GPCH3_LSB,
		GPCH3_MSB,
		GPCH4_LSB,
		GPCH4_MSB,
		GPCH5_LSB,
		GPCH5_MSB,
		GPCH6_LSB,
		GPCH6_MSB,
		GPCH7_LSB,
		GPCH7_MSB,
		GPCH8_LSB,
		GPCH8_MSB,
		GPCH9_LSB,
		GPCH9_MSB,
		GPCH10_LSB,
		GPCH10_MSB,
		GPCH11_LSB,
		GPCH11_MSB,
		GPCH12_LSB,
		GPCH12_MSB,
		GPCH13_LSB,
		GPCH13_MSB,
		GPCH14_LSB,
		GPCH14_MSB,
		GPCH15_LSB,
		GPCH15_MSB,
		BCICH0_LSB,
		BCICH0_MSB,
		BCICH1_LSB,
		BCICH1_MSB,
		BCICH2_LSB,
		BCICH2_MSB,
		BCICH3_LSB,
		BCICH3_MSB,
		BCICH4_LSB,
		BCICH4_MSB,
		MADC_ISR1,
		MADC_IMR1,
		MADC_ISR2,
		MADC_IMR2,
		MADC_SIR,
		MADC_EDR,
		MADC_SIH_CTRL,
		BCIMDEN,
		BCIMDKEY,
		BCIMSTATEC,
		BCIMSTATEP,
		BCIVBAT1,
		BCIVBAT2,
		BCITBAT1,
		BCITBAT2,
		BCIICHG1,
		BCIICHG2,
		BCIVAC1,
		BCIVAC2,
		BCIVBUS1,
		BCIVBUS2,
		BCIMFSTS2,
		BCIMFSTS3,
		BCIMFSTS4,
		BCIMFKEY,
		BCIMFEN1,
		BCIMFEN2,
		BCIMFEN3,
		BCIMFEN4,
		BCIMFTH1,
		BCIMFTH2,
		BCIMFTH3,
		BCIMFTH4,
		BCIMFTH5,
		BCIMFTH6,
		BCIMFTH7,
		BCIMFTH8,	
		BCIMFTH9, 
		BCITIMER1,
		BCITIMER2,
		BCIWDKEY,
		BCIWD,
		BCICTL1,
		BCICTL2,
		BCIVREF1,
		BCIVREF2,
		BCIIREF1,
		BCIIREF2,
		BCIPWM2,
		BCIPWM1,
		BCITRIM1,
		BCITRIM2,
		BCITRIM3,
		BCITRIM4,
		BCIVREFCOMB1,
		BCIVREFCOMB2,
		BCIIREFCOMB1,
		BCIIREFCOMB2,

		BCIISR1A  = EGroup4a + 0x000000B9,
		BCIISR2A, 
		BCIIMR1A,
		BCIIMR2A, 
		BCIISR1B, 
		BCIISR2B,
		BCIIMR1B, 
		BCIIMR2B, //0x000000c0 
		 
		BCIEDR1  = EGroup4a + 0x000000c3, 
		BCIEDR2, 
		BCIEDR3, 
		BCISIHCTRL, // c6
		
		KEYP_CTRL_REG = EGroup4a + 0x000000D2,
		KEY_DEB_REG,
		LONG_KEY_REG1,
		LK_PTV_REG,
		TIME_OUT_REG1,
		TIME_OUT_REG2,
		KBC_REG,
		KBR_REG,
		KEYP_SMS,
		FULL_CODE_7_0,
		FULL_CODE_15_8,
		FULL_CODE_23_16,	
		FULL_CODE_31_24,
		FULL_CODE_39_32,
		FULL_CODE_47_40,
		FULL_CODE_55_48,
		FULL_CODE_63_56,
		KEYP_ISR1,
		KEYP_IMR1,
		KEYP_ISR2,
		KEYP_IMR2,
		KEYP_SIR,
		KEYP_EDR,
		KEYP_SIH_CTRL,
		
		LEDEN = EGroup4a + 0x000000EE,
		PWMAON,
		PWMAOFF,
		PWMBON, 
		PWMBOFF,
		
		PWM1ON= EGroup4a + 0x000000FB,
		PWM1OFF,
		PWM0ON = EGroup4a + 0x000000F8,
		PWM0OFF,
		};

	enum TGroup4bRegisters
		{
		SECURED_REG_A = EGroup4b + 0,
		SECURED_REG_B,
		SECURED_REG_C,
		SECURED_REG_D,
		SECURED_REG_E,
		SECURED_REG_F,
		SECURED_REG_G,
		SECURED_REG_H, 
		SECURED_REG_I, 
		SECURED_REG_J,
		SECURED_REG_K,
		SECURED_REG_L,
		SECURED_REG_M,
		SECURED_REG_N,
		SECURED_REG_O,
		SECURED_REG_P,
		SECURED_REG_Q,
		SECURED_REG_R,
		SECURED_REG_S,
		SECURED_REG_U,
		BACKUP_REG_A,
		BACKUP_REG_B, 
		BACKUP_REG_C,
		BACKUP_REG_D, 
		BACKUP_REG_E, 
		BACKUP_REG_F,
		BACKUP_REG_G,
		BACKUP_REG_H,
		PWR_ISR1 = EGroup4b + 0x2e,
		PWR_IMR1,
		PWR_ISR2,
		PWR_IMR2,
		PWR_SIR, 
		PWR_EDR1,
		PWR_EDR2,
		PWR_SIH_CTRL,
		CFG_P1_TRANSITION,
		CFG_P2_TRANSITION,
		CFG_P3_TRANSITION,
		CFG_P123_TRANSITION,
		STS_BOOT,
		CFG_BOOT, 
		SHUNDAN,
		BOOT_BCI,
		CFG_PWRANA1,
		CFG_PWRANA2,
		BGAP_TRIM,
		BACKUP_MISC_STS,
		BACKUP_MISC_CFG,
		BACKUP_MISC_TST,
		PROTECT_KEY, 
		STS_HW_CONDITIONS, 
		P1_SW_EVENTS, 
		P2_SW_EVENTS, 
		P3_SW_EVENTS, 
		STS_P123_STATE,
		PB_CFG, 
		PB_WORD_MSB,
		PB_WORD_LSB,
		RESERVED_A,
		RESERVED_B,
		RESERVED_C,
		RESERVED_D,
		RESERVED_E,
		SEQ_ADD_W2P,
		SEQ_ADD_P2A,
		SEQ_ADD_A2W,
		SEQ_ADD_A2S,
		SEQ_ADD_S2A12,
		SEQ_ADD_S2A3,
		SEQ_ADD_WARM,
		MEMORY_ADDRESS,
		MEMORY_DATA,
		SC_CONFIG, 
		SC_DETECT1,
		SC_DETECT2,
		WATCHDOG_CFG,
		IT_CHECK_CFG,
		VIBRATOR_CFG,
		DCDC_GLOBAL_CFG,
		VDD1_TRIM1,
		VDD1_TRIM2,
		VDD2_TRIM1,
		VDD2_TRIM2,
		VIO_TRIM1,
		VIO_TRIM2,
		MISC_CFG,
		LS_TST_A,
		LS_TST_B,
		LS_TST_C,
		LS_TST_D,
		BB_CFG,
		MISC_TST,
		TRIM1,
		TRIM2,
		DCDC_TIMEOUT,
		VAUX1_DEV_GRP,
		VAUX1_TYPE,
		VAUX1_REMAP,
		VAUX1_DEDICATED,
		VAUX2_DEV_GRP, 
		VAUX2_TYPE, 
		VAUX2_REMAP,
		VAUX2_DEDICATED, 
		VAUX3_DEV_GRP,
		VAUX3_TYPE, 
		VAUX3_REMAP, 
		VAUX3_DEDICATED,
		VAUX4_DEV_GRP,
		VAUX4_TYPE,
		VAUX4_REMAP,
		VAUX4_DEDICATED, 
		VMMC1_DEV_GRP,
		VMMC1_TYPE,
		VMMC1_REMAP,
		VMMC1_DEDICATED,
		VMMC2_DEV_GRP,
		VMMC2_TYPE,
		VMMC2_REMAP,
		VMMC2_DEDICATED,
		VPLL1_DEV_GRP,
		VPLL1_TYPE,
		VPLL1_REMAP,
		VPLL1_DEDICATED,
		VPLL2_DEV_GRP,
		VPLL2_TYPE,
		VPLL2_REMAP,
		VPLL2_DEDICATED,
		VSIM_DEV_GRP,
		VSIM_TYPE,
		VSIM_REMAP,
		VSIM_DEDICATED,
		VDAC_DEV_GRP,
		VDAC_TYPE,
		VDAC_REMAP,
		VDAC_DEDICATED,
		VINTANA1_DEV_GRP,
		VINTANA1_TYPE,
		VINTANA1_REMAP,
		VINTANA1_DEDICATED,
		VINTANA2_DEV_GRP,
		VINTANA2_TYPE,
		VINTANA2_REMAP,
		VINTANA2_DEDICATED,
		VINTDIG_DEV_GRP,
		VINTDIG_TYPE,
		VINTDIG_REMAP,
		VINTDIG_DEDICATED,
		VIO_DEV_GRP,
		VIO_TYPE,
		VIO_REMAP,
		VIO_CFG,
		VIO_MISC_CFG,
		VIO_TEST1,
		VIO_TEST2,
		VIO_OSC,
		VIO_RESERVED,
		VIO_VSEL,
		VDD1_DEV_GRP,
		VDD1_TYPE,
		VDD1_REMAP,
		VDD1_CFG,
		VDD1_MISC_CFG,
		VDD1_TEST1,
		VDD1_TEST2,
		VDD1_OSC,
		VDD1_RESERVED,
		VDD1_VSEL,
		VDD1_VMODE_CFG,
		VDD1_VFLOOR,
		VDD1_VROOF,
		VDD1_STEP,
		VDD2_DEV_GRP,
		VDD2_TYPE,
		VDD2_REMAP,
		VDD2_CFG,
		VDD2_MISC_CFG,
		VDD2_TEST1,
		VDD2_TEST2,
		VDD2_OSC,
		VDD2_RESERVED,
		VDD2_VSEL,
		VDD2_VMODE_CFG,
		VDD2_VFLOOR,
		VDD2_VROOF,
		VDD2_STEP,
		VUSB1V5_DEV_GRP,
		VUSB1V5_TYPE,
		VUSB1V5_REMAP,
		VUSB1V8_DEV_GRP,
		VUSB1V8_TYPE,
		VUSB1V8_REMAP,
		VUSB3V1_DEV_GRP,
		VUSB3V1_TYPE,
		VUSB3V1_REMAP,
		VUSBCP_DEV_GRP,
		VUSBCP_TYPE,
		VUSBCP_REMAP,
		VUSB_DEDICATED1,
		VUSB_DEDICATED2,
		REGEN_DEV_GRP,
		REGEN_TYPE,
		REGEN_REMAP,
		NRESPWRON_DEV_GRP,
		NRESPWRON_TYPE,
		NRESPWRON_REMAP,
		CLKEN_DEV_GRP,
		CLKEN_TYPE,
		CLKEN_REMAP,
		SYSEN_DEV_GRP,
		SYSEN_TYPE,
		SYSEN_REMAP,
		HFCLKOUT_DEV_GRP,
		HFCLKOUT_TYPE,
		HFCLKOUT_REMAP,
		E32KCLKOUT_DEV_GRP,
		E32KCLKOUT_TYPE,
		E32KCLKOUT_REMAP,
		TRITON_RESET_DEV_GRP,
		TRITON_RESET_TYPE,
		TRITON_RESET_REMAP,
		MAINREF_DEV_GRP,
		MAINREF_TYPE,
		MAINREF_REMAP,
		SECONDS_REG,
		MINUTES_REG,
		HOURS_REG,
		DAYS_REG,
		MONTHS_REG,
		YEARS_REG,
		WEEKS_REG,
		ALARM_SECONDS_REG,
		ALARM_MINUTES_REG,
		ALARM_HOURS_REG,
		ALARM_DAYS_REG,
		ALARM_MONTHS_REG,
		ALARM_YEARS_REG,
		RTC_CTRL_REG,
		RTC_STATUS_REG,
		RTC_INTERRUPTS_REG,
		RTC_COMP_LSB_REG,
		RTC_COMP_MSB_REG, //2d
		};
	} // namespace Register

	namespace DCDC_GLOBAL_CFG
		{
		const TUint16		Addr = Register::DCDC_GLOBAL_CFG;

		const TUint8	CARD_DETECT_2_LEVEL = KBit7;
		const TUint8	CARD_DETECT_1_LEVEL = KBit6;
		const TUint8	REGEN_PU_DISABLE = KBit5;
		const TUint8	SYSEN_PU_DISABLE = KBit4;
		const TUint8	SMARTREFLEX_ENABLE = KBit3;
		const TUint8	CARD_DETECT_CFG = KBit2;
		const TUint8	CLK_32K_DEGATE = KBit1;
		const TUint8	CLK_HF_DEGATE = KBit0;
		};


	namespace _VMODE_CFG_
		{
		const TUint8	STS_BUSY	= KBit5;
		const TUint8	STS_ROOF	= KBit4;
		const TUint8	STS_FLOOR	= KBit3;
		const TUint8	DCDC_SLP	= KBit2;
		const TUint8	READ_REG	= KBit1;
		const TUint8	ENABLE_VMODE	= KBit0;
		}

	namespace VDD1_VMODE_CFG
		{
		const TUint16		Addr = Register::VDD1_VMODE_CFG;
		using namespace _VMODE_CFG_;
		};

	namespace VDD2_VMODE_CFG
		{
		const TUint16		Addr = Register::VDD2_VMODE_CFG;
		using namespace _VMODE_CFG_;
		};

	namespace _VDDx_VSEL_
		{
		namespace Mask
			{
			const TUint8	VSEL	= 0x7F;
			}

		namespace Shift
			{
			const TUint		VSEL = 0;
			}
		}

	namespace VDD1_VSEL
		{
		const TUint16		Addr = Register::VDD1_VSEL;
		using namespace _VDDx_VSEL_;
		};

	namespace VDD2_VSEL
		{
		const TUint16	Addr = Register::VDD2_VSEL;
		using namespace _VDDx_VSEL_;
		};

	namespace _PWR_I_1_
		{
		const TUint8 PWR_SC_DETECT 		= KBit7;
		const TUint8 PWR_MBCHG	   		= KBit6;
		const TUint8 PWR_PWROK_TIMEOUT	= KBit5;
		const TUint8 PWR_HOT_DIE		= KBit4;
		const TUint8 PWR_RTC_IT			= KBit3;
		const TUint8 PWR_USB_PRES		= KBit2;
		const TUint8 PWR_CHG_PRES		= KBit1;
		const TUint8 PWR_CHG_PWRONS		= KBit0;
		}

	namespace PWR_IMR1
		{
		const TUint16	Addr = Register::PWR_IMR1;
		using namespace _PWR_I_1_;
		}

	namespace PWR_ISR1
		{
		const TUint16	Addr = Register::PWR_ISR1;
		using namespace _PWR_I_1_;
		}

	namespace _MADC_I_1_
		{
		const TUint8 MADC_USB_ISR1		= KBit3;
		const TUint8 MADC_SW2_ISR1		= KBit2;
		const TUint8 MADC_SW1_ISR1		= KBit1;
		const TUint8 MADC_RT_ISR1		= KBit0;
		}

	namespace MADC_IMR1
		{
		const TUint16 Addr = Register::MADC_IMR1;
		using namespace _MADC_I_1_;
		}

	namespace MADC_ISR1
		{
		const TUint16 Addr = Register::MADC_ISR1;
		using namespace _MADC_I_1_;
		}

	namespace _GPIO_I_1A_
		{
		const TUint8 GPIO7ISR1 			= KBit7;
		const TUint8 GPIO6ISR1 			= KBit6;
		const TUint8 GPIO5ISR1 			= KBit5;
		const TUint8 GPIO4ISR1 			= KBit4;
		const TUint8 GPIO3ISR1 			= KBit3;
		const TUint8 GPIO2ISR1 			= KBit2;
		const TUint8 GPIO1ISR1 			= KBit1;
		const TUint8 GPIO0ISR1 			= KBit0;
		}

	namespace _GPIO_I_2A_
		{
		const TUint8 GPIO15ISR2 		= KBit7;
		const TUint8 GPIO14ISR2 		= KBit6;
		const TUint8 GPIO13ISR2 		= KBit5;
		const TUint8 GPIO12ISR2 		= KBit4;
		const TUint8 GPIO11ISR2 		= KBit3;
		const TUint8 GPIO10ISR2 		= KBit2;
		const TUint8 GPIO9ISR2 			= KBit1;
		const TUint8 GPIO8ISR2 			= KBit0;
		}

	namespace _GPIO_I_3A_
		{
		const TUint8 GPIO17ISR3 		= KBit1;
		const TUint8 GPIO16ISR3 		= KBit0;
		}

	namespace GPIO_IMR1A
		{
		const TUint16 Addr = Register::GPIO_IMR1A;
		using namespace _GPIO_I_1A_;
		}

	namespace GPIO_ISR1A
		{
		const TUint16 Addr = Register::GPIO_ISR1A;
		using namespace _GPIO_I_1A_;
		}

	namespace GPIO_IMR2A
		{
		const TUint16 Addr = Register::GPIO_IMR2A;
		using namespace _GPIO_I_2A_;
		}

	namespace GPIO_ISR2A
		{
		const TUint16 Addr = Register::GPIO_ISR2A;
		using namespace _GPIO_I_2A_;
		}

	namespace GPIO_IMR3A
		{
		const TUint16 Addr = Register::GPIO_IMR3A;
		using namespace _GPIO_I_3A_;
		}

	namespace GPIO_ISR3A
		{
		const TUint16 Addr = Register::GPIO_ISR3A;
		using namespace _GPIO_I_3A_;
		}

	namespace _BCI_I_1_
		{
		const TUint8 BCI_BATSTS_ISR1 		= KBit7;
		const TUint8 BCI_TBATOR1_ISR1 		= KBit6;
		const TUint8 BCI_TBATOR2_ISR1 		= KBit5;
		const TUint8 BCI_ICHGEOC_ISR1 		= KBit4;
		const TUint8 BCI_ICHGLOW_ISR1ASTO 	= KBit3;
		const TUint8 BCI_IICHGHIGH_ISR1 	= KBit2;
		const TUint8 BCI_TMOVF_ISR1 		= KBit1;
		const TUint8 BCI_WOVF_ISR1 			= KBit0;
		}

	namespace _BCI_I_2_
		{
		const TUint8 BCI_ACCHGOV_ISR1 		= KBit3;
		const TUint8 BCI_VBUSOV_ISR1 		= KBit2;
		const TUint8 BCI_VBATOV_ISR1 		= KBit1;
		const TUint8 BCI_VBATLVL_ISR1 		= KBit0;
		}

	namespace BCIIMR1A
		{
		const TUint16 Addr = Register::BCIIMR1A;
		using namespace _BCI_I_1_;
		}

	namespace BCIISR1A
		{
		const TUint16 Addr = Register::BCIISR1A;
		using namespace _BCI_I_1_;
		}

	namespace BCIIMR2A
		{
		const TUint16 Addr = Register::BCIIMR2A;
		using namespace _BCI_I_2_;
		}

	namespace BCIISR2A
		{
		const TUint16 Addr = Register::BCIISR2A;
		using namespace _BCI_I_2_;
		}

	namespace _KEYP_I_
		{
		const TUint8 KEYP_ITMISR1 			= KBit3;
		const TUint8 KEYP_ITTOISR1 			= KBit2;
		const TUint8 KEYP_ITLKISR1  		= KBit1;
		const TUint8 KEYP_ITKPISR1 			= KBit0;
		}

	namespace KEYP_IMR1
		{
		const TUint16 Addr   = Register::KEYP_IMR1;
		using namespace _KEYP_I_;
		}

	namespace KEYP_ISR1
		{
		const TUint16 Addr   = Register::KEYP_ISR1;
		using namespace _KEYP_I_;
		}

	namespace _USB_INT_EN_
		{
		const TUint8 USB_INTSTS_IDGND 				= KBit4;
		const TUint8 USB_INTSTS_SESSEND				= KBit3;
		const TUint8 USB_INTSTS_SESSVALID			= KBit2;
		const TUint8 USB_INTSTS_VBUSVALID			= KBit1;
		const TUint8 USB_INTSTS_HOSTDISCONNECT		= KBit0;
		}

	namespace USB_INT_EN_RISE_SET
		{
		const TUint16 Addr = Register::USB_INT_EN_RISE_SET;
		using namespace _USB_INT_EN_;
		}

	namespace USB_INT_EN_RISE_CLR
		{
		const TUint16 Addr = Register::USB_INT_EN_RISE_CLR;
		using namespace _USB_INT_EN_;
		}

	namespace USB_INT_STS
		{
		const TUint16 Addr = Register::USB_INT_STS;
		using namespace _USB_INT_EN_;
		}

	namespace _OTHER_INT_
		{
		const TUint8 OTHER_INT_VB_SESS_VLD		= KBit7;
		const TUint8 OTHER_INT_DM_HI			= KBit6;
		const TUint8 OTHER_INT_DP_HI			= KBit5;
		const TUint8 OTHER_INT_MANU				= KBit1;
		const TUint8 OTHER_INT_ABNORMAL_STRESS	= KBit0;
		}

	namespace OTHER_INT_EN_RISE_SET
		{
		const TUint16 Addr = Register::OTHER_INT_EN_RISE_SET;
		using namespace _OTHER_INT_;
		}

	namespace OTHER_INT_EN_RISE_CLR
		{
		const TUint16 Addr = Register::OTHER_INT_EN_RISE_CLR;
		using namespace _OTHER_INT_;
		}

	namespace OTHER_INT_STS
		{
		const TUint16 Addr = Register::OTHER_INT_STS;
		using namespace _OTHER_INT_;
		}

	namespace _CARKIT_INT_
		{
		const TUint8 CARKIT_CARDP				= KBit2;
		const TUint8 CARKIT_CARINTDET			= KBit1;
		const TUint8 CARKIT_IDFLOAT				= KBit0;
		}
	
	namespace CARKIT_INT_EN_SET
		{
		const TUint16 Addr = Register::CARKIT_INT_EN_SET;
		using namespace _CARKIT_INT_;
		}

	namespace CARKIT_INT_EN_CLR
		{
		const TUint16 Addr = Register::CARKIT_INT_EN_CLR;
		using namespace _CARKIT_INT_;
		}

	namespace CARKIT_INT_STS
		{
		const TUint16 Addr = Register::CARKIT_INT_STS;
		using namespace _CARKIT_INT_;
		}

	namespace _ID_INT_
		{
		const TUint8 ID_INTID_RES_FLOAT 		= KBit3;
		const TUint8 ID_INTID_RES_440K 		= KBit2;
		const TUint8 ID_INTID_RES_200K 		= KBit1;
		const TUint8 ID_INTID_RES_102K			= KBit0;
		}

	namespace ID_INT_EN_RISE_SET
		{
		const TUint16 Addr = Register::ID_INT_EN_RISE_SET;
		using namespace _ID_INT_;
		}

	namespace ID_INT_EN_RISE_CLR
		{
		const TUint16 Addr = Register::ID_INT_EN_RISE_CLR;
		using namespace _ID_INT_;
		}

	namespace ID_INT_STS
		{
		const TUint16 Addr = Register::ID_INT_STS;
		using namespace _ID_INT_;
		}

	namespace _CARKIT_SM_1_INT_
		{
		const TUint8 CARKIT_SM_1_PSM_ERROR			= KBit6;
		const TUint8 CARKIT_SM_1_PH_ACC 			= KBit5;
		const TUint8 CARKIT_SM_1_CHARGER			= KBit4;
		const TUint8 CARKIT_SM_1_USB_HOST			= KBit3;
		const TUint8 CARKIT_SM_1_USB_OTG_B			= KBit2;
		const TUint8 CARKIT_SM_1_CARKIT				= KBit1;
		const TUint8 CARKIT_SM_1_DISCONNECTED		= KBit0;
		}

	namespace _CARKIT_SM_2_INT_
		{
		const TUint8 CARKIT_SM_2_STOP_PLS_MISS		= KBit7;
		const TUint8 CARKIT_SM_2_STEREO_TO_MONO		= KBit3;
		const TUint8 CARKIT_SM_2_PHONE_UART			= KBit1;
		const TUint8 CARKIT_SM_2_PH_NO_ACK 			= KBit0;
		}

	namespace CARKIT_SM_1_INT_EN_SET
		{
		const TUint16 Addr = Register::CARKIT_SM_1_INT_EN_SET;
		using namespace _CARKIT_SM_1_INT_;
		}

	namespace CARKIT_SM_1_INT_EN_CLR
		{
		const TUint16 Addr = Register::CARKIT_SM_1_INT_EN_CLR;
		using namespace _CARKIT_SM_1_INT_;
		}

	namespace CARKIT_SM_1_INT_STS
		{
		const TUint16 Addr = Register::CARKIT_SM_1_INT_STS;
		using namespace _CARKIT_SM_1_INT_;
		}

	namespace CARKIT_SM_2_INT_EN_SET
		{
		const TUint16 Addr = Register::CARKIT_SM_2_INT_EN_SET;
		using namespace _CARKIT_SM_2_INT_;
		}

	namespace CARKIT_SM_2_INT_EN_CLR
		{
		const TUint16 Addr = Register::CARKIT_SM_2_INT_EN_CLR;
		using namespace _CARKIT_SM_2_INT_;
		}

	namespace CARKIT_SM_2_INT_STS
		{
		const TUint16 Addr = Register::CARKIT_SM_2_INT_STS;
		using namespace _CARKIT_SM_2_INT_;
		}

	namespace _PIH_
		{
		const TUint8 PIH_PWR_INT	= KBit5;
		const TUint8 PIH_USB_INT	= KBit4;
		const TUint8 PIH_MADC_INT	= KBit3;
		const TUint8 PIH_BCI_INT	= KBit2;
		const TUint8 PIH_KEYP_INT	= KBit1;
		const TUint8 PIH_GPIO_INT	= KBit0;
		}

	namespace PIH_ISR_P1
		{
		const TUint Addr = Register::PIH_ISR_P1;
		
		const TUint8	PIH_ISR7	= KBit7;
		const TUint8	PIH_ISR6	= KBit6;
		const TUint8	PIH_ISR5	= KBit5;
		const TUint8	PIH_ISR4	= KBit4;
		const TUint8	PIH_ISR3	= KBit3;
		const TUint8	PIH_ISR2	= KBit2;
		const TUint8	PIH_ISR1	= KBit1;
		const TUint8	PIH_ISR0	= KBit0;
		}

	namespace _SIH_CTRL_
		{
		const TUint8 SIH_EXCLEN		= KBit0;
		const TUint8 SIH_PENDDIS	= KBit1;
		const TUint8 SIH_COR		= KBit2;
		}

	namespace GPIO_SIH_CTRL
		{
		const TUint16 Addr = Register::GPIO_SIH_CTRL;
		using namespace _SIH_CTRL_;
		}

	namespace KEYP_SIH_CTRL
		{
		const TUint16 Addr = Register::KEYP_SIH_CTRL;
		using namespace _SIH_CTRL_;
		}

	namespace BCISIHCTRL
		{
		const TUint16 Addr = Register::BCISIHCTRL;
		using namespace _SIH_CTRL_;
		}

	namespace MADC_SIH_CTRL
		{
		const TUint16 Addr = Register::MADC_SIH_CTRL;
		using namespace _SIH_CTRL_;
		}

	namespace PWR_SIH_CTRL
		{
		const TUint16 Addr = Register::PWR_SIH_CTRL;
		using namespace _SIH_CTRL_;
		}

	namespace PROTECT_KEY
		{
		const TUint16 Addr = Register::PROTECT_KEY;

		const TUint8 KEY_TEST = KBit0;
		const TUint8 KEY_CFG = KBit1;
		}

	namespace RTC_CTRL_REG
		{
		const TUint16 Addr = Register::RTC_CTRL_REG;

		const TUint8 STOP_RTC = KBit0;
		const TUint8 ROUND_30S = KBit1;
		const TUint8 AUTO_COMP = KBit2;
		const TUint8 MODE_12_24 = KBit3;
		const TUint8 TEST_MODE = KBit4;
		const TUint8 SET_32_COUNTER = KBit5;
		const TUint8 GET_TIME = KBit6;
		}

} // namespace TPS65950

#endif // define TPS65950_REGISTERS_H
