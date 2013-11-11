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
* ne1_tb\inc\cs42l51.h
*
*/



#ifndef CS42L51_H_
#define CS42L51_H_
#include <drivers/iic_channel.h>


const TUint KMaxAttempts 		 = 2000;  // Number of maximum attempts in operations
const TInt8 KCodecWriteCommand 	=  0x94;   // value of the first byte to be sent in each write request on CSI bus.

const TInt KCodecResetPin = 25; // GPIO pin 25 works as AUDIO_RESET
const TInt KCodecCSPin = 12; // GPIO pin 12 works as CS.

const TUint KHwCS42L51ChipID     = 0x01;  // Chip I.D. and Revision Register (Address 01h) (Read Only)
const TUint KHwCS42L51PwrCtrl = 0x02;  // Power Control 1
const TUint KHtCS42L51PwrCtrl_PDN_ALL  =  0x7f;  // Power Down ALL (bit 7-reserved)
const TUint KHtCS42L51PwrCtrl_PDN_DACB  = 1<<6;  // Power Down DACB
const TUint KHtCS42L51PwrCtrl_PDN_DACA  = 1<<5;  // Power Down DACA
const TUint KHtCS42L51PwrCtrl_PDN_PGAB  = 1<<4;  // Power Down PGAB
const TUint KHtCS42L51PwrCtrl_PDN_PGAA  = 1<<3;  // Power Down PGAA
const TUint KHtCS42L51PwrCtrl_PDN_ADCB  = 1<<2;  // Power Down ADCB
const TUint KHtCS42L51PwrCtrl_PDN_ADCA  = 1<<1;  // Power Down ADCA
const TUint KHtCS42L51PwrCtrl_PDN  	   = 1<<0;  // Power Down

const TUint KHwCS42L51MicPwrSpeed = 0x03;  // MIC Power Control & Speed Control
const TUint KHtCS42L51MicPwrSpeed_AUTO     		= 1<<7;  // Auto-Detect Speed Mode
const TUint KHtCS42L51MicPwrSpeed_3ST_SP    	= 1<<4;  // Tri-State Serial Port Interface
const TUint KHtCS42L51MicPwrSpeed_PDN_MICB  	= 1<<3;  // Power Down MICB
const TUint KHtCS42L51MicPwrSpeed_PDN_MICA  	= 1<<2;  // Power Down MICA
const TUint KHtCS42L51MicPwrSpeed_PDN_MICBIAS 	= 1<<1;  // MCLK Divide By 2
const TUint KHtCS42L51MicPwrSpeed_MCLKDIV2 	  	= 1<<0;  // MCLK Divide By 2

const TUint KHsCS42L51MicPwrSpeed    = 5; // Speed shift (bits[6:5])
const TUint KHCS42L51MicPwrSpeed_SpeedQSM = 3; // Quarter-Speed Mode (QSM) - 4 to 12.5 kHz sample rates
const TUint KHCS42L51MicPwrSpeed_SpeedHSM = 2; // Half-Speed Mode (HSM) - 12.5 to 25 kHz sample rates
const TUint KHCS42L51MicPwrSpeed_SpeedSSM = 1; // Single-Speed Mode (SSM) - 4 to 50 kHz sample rates
const TUint KHCS42L51MicPwrSpeed_SpeedDSM = 0; // Double-Speed Mode (DSM) - 50 to 100 kHz sample rates

const TUint KHwCS42L51Ctrl = 0x04;  // Interface Control
const TUint KHtCS42L51Ctrl_SDOUT_SDIN = 1<<7;  // SDOUT to SDIN Loopback
const TUint KHtCS42L51Ctrl_MS     	 = 1<<6;  // Master/Slave Mode
const TUint KHtCS42L51Ctrl_ADC_I2S    = 1<<2;  // ADC I2S or Left-Justified
const TUint KHtCS42L51Ctrl_DIGMIX     = 1<<1;  // Digital Mix
const TUint KHtCS42L51Ctrl_MICMIX     = 1<<0;  // Mic Mix

const TUint KHsCS42L51CtrlFormat = 3; // DAC Digital Interface Format shift(DAC_DIF[5:3])
const TUint KHCS42L51CtrlLeftJustifiedUpto24bit  = 0;
const TUint KHCS42L51CtrlI2sUpto24bit 			 = 1;
const TUint KHCS42L51CtrlRightJustifiedUpto24bit = 2;
const TUint KHCS42L51CtrlRightJustifiedUpto20bit = 3;
const TUint KHCS42L51CtrlRightJustifiedUpto18bit = 4;
const TUint KHCS42L51CtrlRightJustifiedUpto16bit = 5;

const TUint KHwCS42L51MicCtrl = 0x05;  // MIC Control
const TUint KHtCS42L51MicCtrl_ADC_SNGVOL  = 1<<7; // ADC Single Volume Control
const TUint KHtCS42L51MicCtrl_ADCB_DBOOST = 1<<6; // ADCB 20 dB Digital Boost
const TUint KHtCS42L51MicCtrl_ADCA_DBOOST = 1<<5; // ADCA 20 dB Digital Boost
const TUint KHtCS42L51MicCtrl_MICBIAS_SEL = 1<<4; // MIC Bias Select
const TUint KHtCS42L51MicCtrl_MICB_BOOST  = 1<<1; // MIC B Preamplifier Boost
const TUint KHtCS42L51MicCtrl_MICA_BOOST  = 1<<0; // MIC A Preamplifier Boost
const TUint KHsCS42L51MicCtrl_MICBIAS	  = 2; // MIC Bias Level shift (0-3) [3:2]

const TUint KHwCS42L51ADCCtrl = 0x06;  // ADC Control
const TUint KHtCS42L51ADCCtrl_ADCB_HPFEN  = 1<<7; // ADCB High-Pass Filter Enable
const TUint KHtCS42L51ADCCtrl_ADCB_HPFRZ  = 1<<6; // ADCB High-Pass Filter Freeze
const TUint KHtCS42L51ADCCtrl_ADCA_HPFEN  = 1<<5; // ADCA High-Pass Filter Enable
const TUint KHtCS42L51ADCCtrl_ADCA_HPFRZ  = 1<<4; // ADCA High-Pass Filter Freeze
const TUint KHtCS42L51ADCCtrl_SOFTB 	  = 1<<3; // Soft Ramp CHB Control
const TUint KHtCS42L51ADCCtrl_ZCROSSB 	  = 1<<2; // Zero Cross CHB Control
const TUint KHtCS42L51ADCCtrl_SOFTA 	  = 1<<1; // Soft Ramp CHA Control
const TUint KHtCS42L51ADCCtrl_ZCROSSA 	  = 1<<0; // Zero Cross CHA Control

const TUint KHwCS42L51ADCInputMute = 0x07;  // ADCx Input Select, Invert & Mute
const TUint KHsCS42L51ADCInputMute_AINB_MUX  = 6; // ADCB Input Select Bits [7:6]
const TUint KHsCS42L51ADCInputMute_AINA_MUX  = 4; // ADCA Input Select Bits [5:4]
/*
PDN_PGAx AINx_MUX[1:0] Selected Path to ADC
	0 	  00 		AIN1x-->PGAx
	0	  01 		AIN2x-->PGAx
	0 	  10 		AIN3x/MICINx-->PGAx
	0 	  11 		AIN3x/MICINx-->Pre-Amp(+16/+32 dB Gain)-->PGAx
	1 	  00 		AIN1x
	1 	  01 		AIN2x
	1 	  10 		AIN3x/MICINx
	1 	  11 		Reserved */
const TUint KHwCS42L51ADCInputMute_INV_ADCB   = 1<<3; // ADCB Invert Signal Polarity
const TUint KHwCS42L51ADCInputMute_INV_ADCA   = 1<<2; // ADCA Invert Signal Polarity
const TUint KHwCS42L51ADCInputMute_ADCB_MUTE  = 1<<1; // ADCB Channel Mute
const TUint KHwCS42L51ADCInputMute_ADCA_MUTE  = 1<<0; // ADCA Channel Mute

const TUint KHwCS42L51DACOutputControl = 0x08;  // DAC Output Control
const TUint KHsCS42L51DACOutputControl_HP_GAIN = 5; // Headphone Analog Gain (HP_GAIN[7:5])
const TUint KHtCS42L51DACOutputControl_DAC_SNGVOL = 1<<4; // DAC Single Volume Control
const TUint KHtCS42L51DACOutputControl_INV_PCMB   = 1<<3; // PCMB Invert Signal Polarity
const TUint KHtCS42L51DACOutputControl_INV_PCMA   = 1<<2; // PCMA Invert Signal Polarity
const TUint KHtCS42L51DACOutputControl_DACB_MUTE  = 1<<1; // DACB Channel Mute
const TUint KHtCS42L51DACOutputControl_DACA_MUTE  = 1<<0; // DACA Channel Mute
const TUint KHtCS42L51DACOutputControl_DACAB_MUTE  = 3<<0; // DACA and DACB Channel Mute


const TUint KHwCS42L51DACControl = 0x09;  // DAC Control
const TUint KHsCS42L51DACControl_DATA_SEL = 6; // DAC Data Selection (DATA_SEL[7:6])
/*00 - PCM Serial Port to DAC
  01 - Signal Processing Engine to DAC
  10 - ADC Serial Port to DAC
  11 - Reserved */
const TUint KHtCS42L51DACControl_FREEZE = 1<<5; // Freeze Controls
const TUint KHtCS42L51DACControl_DEEMPH = 1<<3; // DAC De-Emphasis Control
const TUint KHtCS42L51DACControl_AMUTE = 1<<2; // Analog Output Auto MUTE
const TUint KHsCS42L51DACControl_DAC_SZC = 0; // DAC Soft Ramp and Zero Cross Control (DAC_SZC[1:0])

const TUint KHwCS42L51ALC_PGA_A_Control = 0x0A;  // ALCA & PGAA Control
const TUint KHtCS42L51ALC_PGA_A_Control_ALC_SRDIS = 1<<7; // ALCA Soft Ramp Disable
const TUint KHtCS42L51ALC_PGA_A_Control_ALC_ZCDIS = 1<<6; // ALCA Zero Cross Disable
const TUint KHsCS42L51ALC_PGA_A_Control_PGA_VOL = 0; // PGA A Gain Control

const TUint KHwCS42L51ALC_PGA_B_Control = 0x0B;  // ALCB & PGAB Control
const TUint KHtCS42L51ALC_PGA_B_Control_ALC_SRDIS = 1<<7; // ALCB Soft Ramp Disable
const TUint KHtCS42L51ALC_PGA_B_Control_ALC_ZCDIS = 1<<6; // ALCB Zero Cross Disable
const TUint KHsCS42L51ALC_PGA_B_Control_PGA_VOL = 0; // PGA B Gain Control

const TUint KHwCS42L51ALC_ADC_A_Attenuator = 0x0C;  // ADCA Attenuator
const TUint KHwCS42L51ALC_ADC_B_Attenuator = 0x0D;  // ADCB Attenuator

const TUint KHwCS42L51ALC_ADC_A_MixVolume = 0x0E;  // ADCA Mixer Volume Control
const TUint KHwCS42L51ALC_ADC_B_MixVolume = 0x0F;  // ADCB Mixer Volume Control

const TUint KHwCS42L51ALC_PCM_A_MixVolume = 0x10;  // PCMA Mixer Volume Control
const TUint KHwCS42L51ALC_PCM_B_MixVolume = 0x11;  // PCMB Mixer Volume Control

// this applies to ADCx_MixVolume and PCMx_MixVolume registers..
const TUint KHtCS42L51ALC_MixVolume_ChannelMute = 1<<7; // Channel Mute
const TUint KHmCS42L51ALC_MixVolume_VolumeMask 	= 0x7F;  // Volume Control Mask

const TUint KHwCS42L51ALC_Beep_FQ_Time = 0x12;  // Beep Frequency & Timing Configuration
const TUint KHsCS42L51ALC_Beep_FQ 	 	 = 4;   // Beep Frequency (FREQ[7:4])
const TUint KHmCS42L51ALC_Beep_FQ_Mask 	 = 0xF0; // Beep frequency mask
const TUint KHsCS42L51ALC_Beep_Time 	 = 0; // Beep on duration (ONTIME[3:0])
const TUint KHmCS42L51ALC_Beep_Time_Mask = 0x0F; // Time on duration mask

const TUint KHwCS42L51ALC_Beep_Off_Volume = 0x13;  // Beep Off Time & Volume
const TUint KHsCS42L51ALC_Beep_Off		   = 5; // Beep off time mask
const TUint KHmCS42L51ALC_Beep_Off_Mask    = 0xE0; // Beep off time mask
const TUint KHmCS42L51ALC_Beep_Volume_Mask = 0x1F; // Beep volume mask

const TUint KHwCS42L51ALC_Beep_Conf_Tone = 0x14;  // Beep Configuration & Tone Configuration
const TUint KHtCS42L51ALC_Beep_Conf_Tone_REPEAT  = 1<<7; // Repeat Beep
const TUint KHtCS42L51ALC_Beep_Conf_Tone_BEEP 	 = 1<<6; // Beep
const TUint KHtCS42L51ALC_Beep_Conf_Tone_TC_EN 	 = 1<<0; // Tone Control Enable
const TUint KHsCS42L51ALC_Beep_Conf_Tone_TREB_CF = 3; // Treble Corner Frequency (TREB_CF[4:3])
const TUint KHsCS42L51ALC_Beep_Conf_Tone_BASS_CF = 1; // Bass Corner Frequency (BASS_CF[2:1])

const TUint KHwCS42L51ALC_ToneCtrl = 0x15; // Tone Control
const TUint KHsCS42L51ALC_ToneCtrl_TREB 	 = 4; 	 // Treble Gain Level (TREB[7:4])
const TUint KHmCS42L51ALC_ToneCtrl_TREB_Mask = 0xF0; // Treble Gain Level mask
const TUint KHsCS42L51ALC_ToneCtrl_BASS 	 = 0; 	 // Bass Gain Level (TREB[7:4])
const TUint KHmCS42L51ALC_ToneCtrl_BASS_Mask = 0x0F; // Bass Gain Level mask

const TUint KHwCS42L51ALC_Out_A_Volume = 0x16;  // AOUTA Volume Control
const TUint KHwCS42L51ALC_Out_B_Volume = 0x17;  // AOUTB Volume Control
const TUint KHbCS42L51ALC_Volume_Min = 25;  	// Value for ALC_Out_x for Minimum Volume


const TUint KHwCS42L51ALC_PCM_Mix 	 	    = 0x18;  // PCM Channel Mixer
const TUint KHwCS42L51ALC_Limiter_SZCD 	    = 0x19;  // Limiter Threshold SZC Disable
const TUint KHwCS42L51ALC_Limiter_Release   = 0x1A;  // Limiter Release Rate Register
const TUint KHwCS42L51ALC_Limiter_Attack    = 0x1B;  // Limiter Attack Rate Register
const TUint KHwCS42L51ALC_ALCE_Attack_Rate  = 0x1C;  // ALC Enable & Attack Rate
const TUint KHwCS42L51ALC_ALC_Release_Rate  = 0x1D;  // ALC Release Rate
const TUint KHwCS42L51ALC_ALC_Threshold  	= 0x1E;  // ALC Threshold
const TUint KHwCS42L51ALC_NoiseGate_Misc  	= 0x1F;  // Noise Gate Configuration & Misc
const TUint KHwCS42L51ALC_Status  			= 0x20;  // Noise Gate Configuration & Misc
const TUint KHwCS42L51ALC_Charge_Pump_Fq 	= 0x21;  // Charge Pump Frequency

/**
Definiton of audio codec singleton class...
*/
class RCS42AudioCodec
	{
public:

	struct TCodecConfigData
		{
		TInt8 iAddress;
		TInt8 iRegister;
		TInt8 iData;
		};
public:
	static TInt Open(RCS42AudioCodec* &aSelf); // open the reference codec
	static void Close(RCS42AudioCodec* &aSelf); // close the reference to the codec
	TInt SetPlayVolume(TInt aVolume); // set the Playback volume
	TInt SetRecordVolume(TInt aVolume); // set the Record volume

private:
	RCS42AudioCodec();
	static TInt Create(); // create an instance of the codec
	static void Destroy(); // delete an instance of the codec
	TInt DoWrite(TUint16 aRegAddr, TUint16 aData);
	void StartWrite();
	TInt StopWrite();
	void Write(TUint16 aRegAddr, TUint16 aData); // access codec's registers
	TInt Init();
	void PowerDown();
	static RCS42AudioCodec* iSelf;
	static TInt iRefCnt;
	TInt iTransferStatus;
	TUint8 iInterfaceCtrlVal;
	TInt iResult;

#ifdef _DEBUG
	TBool iStartWriteCalled;
#endif
	TConfigSpiBufV01 iHeaderBuff;
	TPckgBuf <TCodecConfigData> iTransBuff;
	TUint32 iCsiBusConfig;
	};


#endif /*CS42L51_H_*/
