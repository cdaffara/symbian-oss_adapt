/*
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Accenture
*
* Contributors:
*
* Description: Minimalistic serial driver
*
*/

#ifndef _BEAGLE_SERIAL_H
#define _BEAGLE_SERIAL_H

#include <comm.h>
#include <e32hal.h>
#include <omap3530_prcm.h>
#include <omap3530_uart.h>

const TInt KMinimumLddMajorVersion=1;
const TInt KMinimumLddMinorVersion=1;
const TInt KMinimumLddBuild=122;

class DDriverComm : public DPhysicalDevice
	{
public:
	DDriverComm();
	virtual TInt Install();
	virtual void GetCaps(TDes8 &aDes) const;
	virtual TInt Create(DBase*& aChannel, TInt aUnit, const TDesC8* anInfo, const TVersion &aVer);
	virtual TInt Validate(TInt aUnit, const TDesC8* anInfo, const TVersion &aVer);
	};

class DCommBeagle : public DComm
	{
public:
	DCommBeagle();
	~DCommBeagle();
	TInt DoCreate(TInt aUnit, const TDesC8* anInfo);
public:
	virtual TInt Start();
	virtual void Stop(TStopMode aMode);
	virtual void Break(TBool aState);
	virtual void EnableTransmit();
	virtual TUint Signals() const;
	virtual void SetSignals(TUint aSetMask,TUint aClearMask);
	virtual TInt ValidateConfig(const TCommConfigV01 &aConfig) const;
	virtual void Configure(TCommConfigV01 &aConfig);
	virtual void Caps(TDes8 &aCaps) const;
	virtual TInt DisableIrqs();
	virtual void RestoreIrqs(TInt aIrq);
	virtual TDfcQue* DfcQ(TInt aUnit);
	virtual void CheckConfig(TCommConfigV01& aConfig);
public:
	static void Isr(TAny* aPtr);
public:
	TInt iInterruptId;
	TInt iUnit;
	TLinAddr iPortAddr;
	TInt iInInterrupt;
	TUint iSignals;
	TDynamicDfcQue*	iDfcQ;
	Omap3530Uart::TUart*	iUart;
	TInt gData;
	};

#endif
