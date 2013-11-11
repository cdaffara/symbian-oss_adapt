// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// omap3530/beagle_drivers/usb/usbv.cpp
//

#include <kernel.h>
#include <assp/omap3530_assp/omap3530_usbc.h>
// Platform-dependent USB client controller layer (USB PSL).
#include <assp/omap3530_assp/omap3530_i2c.h>
#include <assp/omap3530_assp/omap3530_i2creg.h>
#include <assp/omap3530_shared/tps65950.h>


// I2C Bit definitions
// PHY_CLK_CTRL
const TUint KCLK32K_EN = KBit1;
const TUint KREQ_PHY_DPLL_CLK = KBit0;
// PHY_CLK_STS
const TUint KPHY_DPLL_CLK = KBit0;
// MCPC_CTRL2
const TUint KMPC_CLK_EN = KBit0;
// FUNC_CTRL
const TUint KXCVRSELECT_HS = 0x0;
const TUint KXCVRSELECT_FS = KBit0;
const TUint KXCVRSELECT_MASK = 0x3;
const TUint KTERMSELECT = KBit2;
const TUint KOPMODE_DISABLED = KBit4;
const TUint KOPMODE_MASK = KBit3 | KBit4;
// MCPC_IO_CTRL
const TUint KRXD_PU = KBit3;
// OTG_CTRL
const TUint KDPPULLDOWN = KBit1;
const TUint KDMPULLDOWN = KBit2;
// POWER_CTRL
const TUint KOTG_EN = KBit5;
// VUSB???_DEV_GRP
const TUint KDEV_GRP_P1 = KBit5;
// CFG_BOOT
const TUint KHFCLK_FREQ_26Mhz = KBit1;


NONSHARABLE_CLASS( TBeagleUsbPhy ) : public MOmap3530UsbPhy
	{
public:
	TBeagleUsbPhy();
	TInt Construct();

	virtual void StartPHY();
	virtual void SetPHYMode( DOmap3530Usbcc::TPHYMode aMode );
	virtual void EnablePHY();
	virtual void DisablePHY();

private:
	TInt iPHYEnabled;
	};



TBeagleUsbPhy::TBeagleUsbPhy()
	{
	}

TInt TBeagleUsbPhy::Construct()
	{
	return KErrNone;
	}


void TBeagleUsbPhy::StartPHY()
	{
	// PHY clock must be enabled before this point (can't enable it in this function as it is called from an ISR context)
	TPS65950::DisableProtect();
	
	// Enable the USB LDO's (low-dropout regulators)
	TPS65950::ClearSetSync(TPS65950::Register::VUSB1V5_DEV_GRP,0x00,KDEV_GRP_P1);
	TPS65950::ClearSetSync(TPS65950::Register::VUSB1V8_DEV_GRP,0x00,KDEV_GRP_P1);
	TPS65950::ClearSetSync(TPS65950::Register::VUSB3V1_DEV_GRP,0x00,KDEV_GRP_P1);

	TPS65950::ClearSetSync(TPS65950::Register::CFG_BOOT,0x00, KHFCLK_FREQ_26Mhz);	
	
	TPS65950::RestoreProtect();
	}

void TBeagleUsbPhy::SetPHYMode( DOmap3530Usbcc::TPHYMode aMode )
	{
	EnablePHY();
	switch(aMode)
		{
		// Configure trancever (see swcu05b.pdf table 15-21 D+/D- Termination settings) 
		case DOmap3530Usbcc::ENormal:
			TPS65950::WriteSync(TPS65950::Register::MCPC_CTRL2_CLR, KMPC_CLK_EN); // Not UART Mode
			TPS65950::WriteSync(TPS65950::Register::FUNC_CTRL_CLR,(KXCVRSELECT_MASK | KTERMSELECT | KOPMODE_DISABLED ));	 // XCVRSELECT high speed mode (HS), TERM SELECT=0, OPMODE=0 (normal operation)	
			TPS65950::WriteSync(TPS65950::Register::MCPC_IO_CTRL_CLR, KRXD_PU);
			TPS65950::WriteSync(TPS65950::Register::OTG_CTRL_CLR, KDPPULLDOWN | KDMPULLDOWN); // Disable DP pulldown 
			TPS65950::WriteSync(TPS65950::Register::POWER_CTRL_SET, KOTG_EN);
			break;
		case DOmap3530Usbcc::EPowerUp:
			// Power up or VBUS<VSESSEND			
			TPS65950::WriteSync(TPS65950::Register::MCPC_CTRL2_CLR, KMPC_CLK_EN); // Not UART Mode
			TPS65950::WriteSync(TPS65950::Register::FUNC_CTRL, KXCVRSELECT_FS);		 // XXcvr Select 01, Term select 0, opmode 0,
			TPS65950::WriteSync(TPS65950::Register::OTG_CTRL_SET, KDPPULLDOWN | KDMPULLDOWN); // Disable DP pulldown 
			TPS65950::WriteSync(TPS65950::Register::POWER_CTRL_CLR, KOTG_EN); // Power down OTG
			break;
		case DOmap3530Usbcc::EPeripheralChirp:
			// OTG device Peripheral chirp			
			TPS65950::WriteSync(TPS65950::Register::MCPC_CTRL2_CLR, KMPC_CLK_EN); // Not UART Mode
			TPS65950::WriteSync(TPS65950::Register::FUNC_CTRL,(KXCVRSELECT_HS | KTERMSELECT | KOPMODE_MASK )); //Term select 1, opmode 10,  Xcvr Select 00,
			TPS65950::WriteSync(TPS65950::Register::MCPC_IO_CTRL_CLR, KRXD_PU);
			TPS65950::WriteSync(TPS65950::Register::OTG_CTRL_CLR, KDPPULLDOWN | KDMPULLDOWN); // Disable DP pulldown 
			TPS65950::WriteSync(TPS65950::Register::POWER_CTRL_SET, KOTG_EN);
			
			break;
		case DOmap3530Usbcc::EUART:
			// UART Mode
			TPS65950::WriteSync(TPS65950::Register::MCPC_CTRL2_SET, KMPC_CLK_EN); // Not UART Mode
			TPS65950::WriteSync(TPS65950::Register::MCPC_IO_CTRL_SET, KRXD_PU);
			break;
		default:
			// Don't change mode
			break;
		}
	DisablePHY();
	}

// The PHY 60Mhz clock must be enabled before Register accesses are attempted.
void TBeagleUsbPhy::EnablePHY()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("TBeagleUsbPhy::EnablePHY"));	
	if(iPHYEnabled==0)
		{		
		TPS65950::WriteSync(TPS65950::Register::PHY_CLK_CTRL, KREQ_PHY_DPLL_CLK | KCLK32K_EN);
		TUint8 val=0;
		TInt retries =0;
		do
			{
			TPS65950::ReadSync(TPS65950::Register::PHY_CLK_CTRL_STS, val);
			NKern::Sleep( NKern::TimerTicks( 1 ) );
			//Kern::NanoWait(50000); // wait 1/2 millis to prevent soak
			retries++;
			}
		while(! (val & KPHY_DPLL_CLK) && (retries < 1000) );
		
		__ASSERT_ALWAYS(retries < 1000,Kern::Fault("TBeagleUsbPhy::EnablePHY Cant enable in 5s ",__LINE__));		
		__KTRACE_OPT(KUSB, Kern::Printf("TBeagleUsbPhy: PHY Enabled"));
		}
	iPHYEnabled++;
	}
	
void TBeagleUsbPhy::DisablePHY()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("TBeagleUsbPhy::DisablePHY"));
	if(iPHYEnabled==1)
		{
		TPS65950::WriteSync(TPS65950::Register::PHY_CLK_CTRL, 0x0);			
		}
	if(iPHYEnabled>0)
		{
		iPHYEnabled--;
		}
	}


EXPORT_C MOmap3530UsbPhy* MOmap3530UsbPhy::New()
	{
	__KTRACE_OPT(KUSB, Kern::Printf(" > Initializing USB PHY"));

	TBeagleUsbPhy* const phy = new TBeagleUsbPhy;
	if (!phy)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for TBeagleUsbPhy failed"));
		return NULL;
		}

	TInt r = phy->Construct();
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Construction of TBeagleUsbPhy failed (%d)", r));
		delete phy;
		return NULL;
		}

	return phy;
	}


DECLARE_STANDARD_EXTENSION()
	{
	return KErrNone;
	}


