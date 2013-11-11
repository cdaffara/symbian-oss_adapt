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
// omap3530/omap3530_drivers/usbcc/omap3530_usbc.h
// Platform-dependent USB client controller layer (USB PSL).
//


#ifndef __OMAP3530_USBC_H__
#define __OMAP3530_USBC_H__

#include <e32cmn.h>
#include <drivers/usbc.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>

// This is the header file for the implementation of the USB driver PSL layer for an imaginary USB client
// (device) controller.
// For simplicity's sake we assume the following endpoint layout of the controller.
// We have 5 endpoints in total - two Bulk endpoints (IN and OUT), two Isochronous endpoint (IN and OUT),
// one Interrupt endpoint (IN), and of course endpoint zero (Ep0).
//
// This is the mapping of "Hardware Endpoint Numbers" to "Real Endpoints" (and thus is also
// used as the array index for our local TTemplateAsspUsbcc::iEndpoints[]):
//
//	0 -	 0 (Ep0 OUT)
//	0 -	 1 (Ep0 IN)
//	1 -	 3 (Bulk  IN, Address 0x11, -> EpAddr2Idx(0x11) =  3)
//	2 -	 4 (Bulk OUT, Address 0x02, -> EpAddr2Idx(0x02) =  4)
//	3 -	 7 (Iso   IN, Address 0x13, -> EpAddr2Idx(0x13) =  7)
//	4 -	 8 (Iso  OUT, Address 0x04, -> EpAddr2Idx(0x04) =  8)
//	5 - 11 (Int   IN, Address 0x15, -> EpAddr2Idx(0x15) = 11)
//
// For the reason why this is so (or rather for the perhaps not so obvious system behind it),
// see the comment at the beginning of \e32\drivers\usbcc\ps_usbc.cpp and also the structure
// DeviceEndpoints[] at the top of pa_usbc.cpp.

// The total number of endpoints in our local endpoint array:
static const TInt KUsbTotalEndpoints = 16; //32; // Disabled due to limited FIFO space

// The numbers used in the following macros are 'aRealEndpoint's (i.e. array indices):
#define IS_VALID_ENDPOINT(x)	((x) > 0 && (x) < KUsbTotalEndpoints)
#define IS_OUT_ENDPOINT(x)		IS_VALID_ENDPOINT(x) && ((x) == 0 || (x) == 2 || (x) == 4 || (x) == 6 || (x) == 8 || (x) == 10 || (x) == 12 || (x) == 14 || (x) == 16 || (x) == 18 || (x) == 20 || (x) == 22 ||(x) == 24 || (x) == 26 ||(x) == 28) 
#define IS_IN_ENDPOINT(x)		IS_VALID_ENDPOINT(x) && ((x) == 1 || (x) == 3 || (x) == 5 || (x) == 7 || (x) == 9 || (x) == 11 || (x) == 13 || (x) == 15 || (x) == 17 || (x) == 19 || (x) == 21 || (x) == 23 ||(x) == 25 || (x) == 27 ||(x) == 29)
#define IS_BULK_IN_ENDPOINT(x)	IS_VALID_ENDPOINT(x) && ((x) == 1 || (x) == 3 || (x) == 5 || (x) == 7 || (x) == 9 || (x) == 11 || (x) == 13 || (x) == 15 || (x) == 17 || (x) == 19 || (x) == 21 || (x) == 23 ||(x) == 25 || (x) == 27)
#define IS_BULK_OUT_ENDPOINT(x)		IS_VALID_ENDPOINT(x) &&((x) == 2 || (x) == 4 || (x) == 6 || (x) == 8 || (x) == 10 || (x) == 12 || (x) == 14 || (x) == 16 || (x) == 18 || (x) == 20 || (x) == 22 ||(x) == 24 || (x) == 26 ||(x) == 28) 
#define IS_BULK_ENDPOINT(x)		(IS_BULK_IN_ENDPOINT(x) || IS_BULK_OUT_ENDPOINT(x))
#define IS_ISO_IN_ENDPOINT(x)	EFalse
#define IS_ISO_OUT_ENDPOINT(x)	EFalse
#define IS_ISO_ENDPOINT(x)		(IS_ISO_IN_ENDPOINT(x) || IS_ISO_OUT_ENDPOINT(x))
#define IS_INT_IN_ENDPOINT(x)	IS_VALID_ENDPOINT(x) && ((x) == 29)

// This takes as an index the TTemplateAsspUsbcc::iEndpoints index (== aRealEndpoint) 0..11
// and returns the hardware endpoint number 0..5 (note that not all input indices are valid;
// these will return -1):
/*static const TInt TBeagleAsspEndpoints[KUsbTotalEndpoints] = 
{0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};*/
static const TInt TBeagleAsspEndpoints[KUsbTotalEndpoints] = 
{0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

// And here is a function to use the above array:
static inline TInt ArrayIdx2TemplateEp(TInt aRealEndpoint)
	{
	if (IS_VALID_ENDPOINT(aRealEndpoint)) return TBeagleAsspEndpoints[aRealEndpoint];
	else return -1;
	}
	
static inline TInt TemplateEp2ArrayIdx(TInt aRealEndpoint)
	{
	for(TInt x=0; x<KUsbTotalEndpoints; x++)
		{
		if(TBeagleAsspEndpoints[x]==aRealEndpoint)
			return x;
		}
	return -1;
	}

// Access to clocks is reference counted
static TInt iSICLKEnabled;

// Endpoint max packet sizes
static const TInt KEp0MaxPktSz = 64;						// Control
static const TInt KIntMaxPktSz = 64;							// Interrupt
static const TInt KBlkMaxPktSz = 512;						// Bulk
static const TInt KIsoMaxPktSz = 256;						// Isochronous
static const TInt KEp0MaxPktSzMask = KUsbEpSize64;			// Control
static const TInt KIntMaxPktSzMask = KUsbEpSize64;			// Interrupt
static const TInt KBlkMaxPktSzMask = /*KUsbEpSize64 | */KUsbEpSize512;			// Bulk
static const TInt KIsoMaxPktSzMask = KUsbEpSize256;			// Isochronous

// 1 ms (i.e. the shortest delay possible with the sort of timer used) seems to give
// the best results, both for Bulk and Iso, and also (in the USBRFLCT test program)
// both for loop tests as well as unidirectional transfers.
static const TInt KRxTimerTimeout = 5;						// milliseconds

// Used in descriptors
static const TUint16 KUsbVendorId	= KUsbVendorId_Symbian;	// Symbian
static const TUint16 KUsbProductId	= 0x0666;				// bogus...
static const TUint16 KUsbDevRelease = 0x0100;				// bogus... (BCD!)
static const TUint16 KUsbLangId		= 0x0409;				// English (US) Language ID

// String descriptor default values
static const wchar_t KStringManufacturer[] = L"Symbian Software Ltd.";
static const wchar_t KStringProduct[]	   = L"BeagleBoard";
static const wchar_t KStringSerialNo[]	   = L"0123456789";
static const wchar_t KStringConfig[]	   = L"First and Last and Always";


// We use our own Ep0 state enum:
enum TEp0State
	{
	EP0_IDLE = 0,											// These identifiers don't conform to
	EP0_OUT_DATA_PHASE = 1,									// Symbian's coding standard... ;)
	EP0_IN_DATA_PHASE = 2,
	EP0_END_XFER = 3,
	};



class DOmap3530Usbcc;
// The lowest level endpoint abstraction
struct TEndpoint
	{
	TEndpoint();
	static void RxTimerCallback(TAny* aPtr);
	// data
	DOmap3530Usbcc* iController;						// pointer to controller object
	union
		{
		TUint8* iRxBuf;										// where to store /
		const TUint8* iTxBuf;								// from where to send
		};
	union
		{
		TInt iReceived;										// bytes already rx'ed /
		TInt iTransmitted;									// bytes already tx'ed
		};
	TInt iLength;											// number of bytes to be transferred
	TBool iZlpReqd;											// ZeroLengthPacketRequired
	TBool iNoBuffer;										// no data buffer was available when it was needed
	TBool iDisabled;										// dto but stronger
	TInt iPackets;											// number of packets rx'ed or tx'ed
	TInt iLastError;										//
	TUsbcRequestCallback* iRequest;							//
	NTimer iRxTimer;										//
	TBool iRxTimerSet;										// true if iRxTimer is running
	TBool iRxMoreDataRcvd;									// true if after setting timer data have arrived
	TUsbcPacketArray* iPacketIndex;							// actually TUsbcPacketArray (*)[]
	TUsbcPacketArray* iPacketSize;							// actually TUsbcPacketArray (*)[]
	};


// The hardware driver object proper
class Omap3530BoardAssp;
class MOmap3530UsbPhy;

NONSHARABLE_CLASS( DOmap3530Usbcc ) : public DUsbClientController
	{
friend void TEndpoint::RxTimerCallback(TAny*);

public:
	enum TPHYMode
		{
		ENormal,
		EPowerUp,
		EPeripheralChirp,
		EUART
		};

public:
	DOmap3530Usbcc();
	TInt Construct();
	virtual ~DOmap3530Usbcc();
	virtual void DumpRegisters();

private:
	virtual TInt SetDeviceAddress(TInt aAddress);
	virtual TInt ConfigureEndpoint(TInt aRealEndpoint, const TUsbcEndpointInfo& aEndpointInfo);
	virtual TInt DeConfigureEndpoint(TInt aRealEndpoint);
	virtual TInt AllocateEndpointResource(TInt aRealEndpoint, TUsbcEndpointResource aResource);
	virtual TInt DeAllocateEndpointResource(TInt aRealEndpoint, TUsbcEndpointResource aResource);
	virtual TBool QueryEndpointResource(TInt aRealEndpoint, TUsbcEndpointResource aResource) const;
	virtual TInt OpenDmaChannel(TInt aRealEndpoint);
	virtual void CloseDmaChannel(TInt aRealEndpoint);
	virtual TInt SetupEndpointRead(TInt aRealEndpoint, TUsbcRequestCallback& aCallback);
	virtual TInt SetupEndpointWrite(TInt aRealEndpoint, TUsbcRequestCallback& aCallback);
	virtual TInt CancelEndpointRead(TInt aRealEndpoint);
	virtual TInt CancelEndpointWrite(TInt aRealEndpoint);
	virtual TInt SetupEndpointZeroRead();
	virtual TInt SetupEndpointZeroWrite(const TUint8* aBuffer, TInt aLength, TBool aZlpReqd = EFalse);
	virtual TInt SendEp0ZeroByteStatusPacket();
	virtual TInt StallEndpoint(TInt aRealEndpoint);
	virtual TInt ClearStallEndpoint(TInt aRealEndpoint);
	virtual TInt EndpointStallStatus(TInt aRealEndpoint) const;
	virtual TInt EndpointErrorStatus(TInt aRealEndpoint) const;
	virtual TInt ResetDataToggle(TInt aRealEndpoint);
	virtual TInt SynchFrameNumber() const;
	virtual void SetSynchFrameNumber(TInt aFrameNumber);
	virtual TInt StartUdc();
	virtual TInt StopUdc();
	virtual TInt UdcConnect();
	virtual TInt UdcDisconnect();
	virtual TBool UsbConnectionStatus() const;
	virtual TBool UsbPowerStatus() const;
	virtual TBool DeviceSelfPowered() const;
	virtual const TUsbcEndpointCaps* DeviceEndpointCaps() const;
	virtual TInt DeviceTotalEndpoints() const;
	virtual TBool SoftConnectCaps() const;
	virtual TBool DeviceStateChangeCaps() const;
	virtual void Suspend();
	virtual void Resume();
	virtual void Reset();
	virtual TInt SignalRemoteWakeup();
	virtual void Ep0ReadSetupPktProceed();
	virtual void Ep0ReceiveProceed();
	virtual TDfcQue* DfcQ(TInt aUnit);
	virtual TBool CurrentlyUsingHighSpeed();

private:
	// general
	void EnableEndpointInterrupt(TInt aEndpoint);
	void DisableEndpointInterrupt(TInt aEndpoint);
	void ClearEndpointInterrupt(TInt aEndpoint);
	void InitialiseUdcRegisters();
	void UdcEnable();
	void UdcDisable();
	TInt SetupUdcInterrupt();
	void ReleaseUdcInterrupt();
	void UdcInterruptService();
	void EndpointIntService(TInt aEndpoint);
	TInt ResetIntService();
	void SuspendIntService();
	void ResumeIntService();
	void SofIntService();
	static void UdcIsr(TAny* aPtr);
	static TInt UsbClientConnectorCallback(TAny* aPtr);
	// endpoint zero
	void Ep0IntService();
	void Ep0ReadSetupPkt();
	void Ep0Receive();
	void Ep0Transmit();
	void Ep0EndXfer();
	void Ep0Cancel();
	void Ep0PrematureStatusOut();
	void Ep0StatusIn();
	void Ep0NextState(TEp0State aNextState);
	// endpoint n with n != 0
	void BulkTransmit(TInt aEndpoint);
	void BulkReceive(TInt aEndpoint);
	void BulkReadRxFifo(TInt aEndpoint);
	void IsoTransmit(TInt aEndpoint);
	void IsoReceive(TInt aEndpoint);
	void IsoReadRxFifo(TInt aEndpoint);
	void IntTransmit(TInt aEndpoint);
	void RxComplete(TEndpoint* aEndpoint);
	void StopRxTimer(TEndpoint* aEndpoint);
	
private:
	void EnableSICLK();
	void DisableSICLK();
	
	// Dfc functions
	static void SuspendDfcFn(TAny *aPtr);
	static void ResumeDfcFn(TAny *aPtr);
	static void ResetDfcFn(TAny *aPtr);
	
public:
	TBool DeviceHighSpeedCaps() const;

private:
	// general
	TBool iSoftwareConnectable;
	TBool iCableDetectable;
	TBool iCableConnected;
	TBool iBusIsPowered;
	TBool iInitialized;
	TInt (*iUsbClientConnectorCallback)(TAny *);
	Omap3530Assp* iAssp;
	// endpoint zero
	TBool iEp0Configured;
	TEp0State iEp0State;
	// endpoints n
	TEndpoint iEndpoints[KUsbTotalEndpoints];				// for how this is indexed, see top of pa_usbc.cpp
		
	// Dfc's for configuring the Tranceiver when we get a Suspend/Resume/Reset interrupt.
	TDfcQue* iDfcQueue;
	
	TDfc iSuspendDfc;
	TDfc iResumeDfc;
	TDfc iResetDfc;

	MOmap3530UsbPhy*	iPhy;
	TUint	iPrmClientId;
	};


class MOmap3530UsbPhy
	{
public:
	IMPORT_C static MOmap3530UsbPhy* New();

	virtual void StartPHY() = 0;
	virtual void SetPHYMode( DOmap3530Usbcc::TPHYMode aMode ) = 0;
	virtual void EnablePHY() = 0;
	virtual void DisablePHY() = 0;
	};


#endif // __PA_USBC_H__
