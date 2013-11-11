/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/



#ifndef __D_CSI_H__
#define __D_CSI_H__

#ifdef __KERNEL_MODE__
#include <drivers/iic.h>
#include <drivers/iic_channel.h>
#endif /* __KERNEL_MODE__ */

const TInt KIicClientThreadPriority = 24;
const TInt KIicClientMajorVersionNumber = 1;
const TInt KIicClientMinorVersionNumber = 0;
const TInt KIicClientBuildVersionNumber = KE32BuildVersionNumber;
const TUint8 KSpiChannel0 = 0;
const TUint8 KSpiChannel1 = 1;
const TUint8 KCodecChannelNumber = KSpiChannel0;
const TInt KMaxSlaveTestBufferSize = 256;

#ifndef __KERNEL_MODE__

/**
 **********************************************************************************************************
 * The Following typdef are copied from iic.h. This will be replaced with a cutshort version of iic.h,
 * which will have  only the required information which has to be shared across user test application
 * and the driver.
 **********************************************************************************************************

 */
enum TEndianness
	{
	ELittleEndian,
	EBigEndian
	};


enum TSpiWordWidth
	{
	ESpiWordWidth_8,
	ESpiWordWidth_10,
	ESpiWordWidth_12,
	ESpiWordWidth_16
	};

/**
 @publishedPartner
 @prototype 9.6

 The set of clock mode values for use with TConfigSpiV01

 @see TConfigSpiV01

 */
enum TSpiClkMode
	{
	ESpiPolarityLowRisingEdge, // Active high, odd edges
	ESpiPolarityLowFallingEdge, // Active high, even edges
	ESpiPolarityHighFallingEdge, // Active low,  odd edges
	ESpiPolarityHighRisingEdge
	// Active low,  even edges
	};

enum TSpiSsPinMode
    {
    ESpiCSPinActiveLow,      // Active low
    ESpiCSPinActiveHigh     // Active high
    };

enum TBusType
	{
	EI2c = 0,
	ESpi = 0x01,
	EMicrowire = 0x02,
	ECci = 0x03,
	ESccb = 0x04
	};

/**
 @publishedPartner
 @prototype 9.6

 Class to represent the configuration data for a SPI bus channel registered with IIC

 */

enum TBitOrder
	{
	ELsbFirst,
	EMsbFirst
	};


class TConfigSpiV01
	{
public:
	TSpiWordWidth iWordWidth;
	TInt32        iClkSpeedHz;
	TSpiClkMode   iClkMode;
	TInt32        iTimeoutPeriod;
	TEndianness   iEndianness;
	TBitOrder     iBitOrder;
	TUint         iTransactionWaitCycles;
	TSpiSsPinMode iSSPinActiveMode;
	};

typedef TPckgBuf <TConfigSpiV01> TConfigSpiBufV01;


// Bus realisation configuration
//
// 31 30 29 28 | 27 26 25 24 | 23 22 21 20 | 19 18 17 16 | 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0
//
// 31:29 - HS Master address (I2C only)
// 28    - HS address valid bit
// 27:23 - Reserved
// 22:20 - Bus type
// 19:15 - Channel number
// 14:10 - Transaction speed
//  9:0  - Slave address
#define HS_MASTER_ADDR_SHIFT 29
#define HS_MASTER_ADDR_MASK 0x7
#define HS_ADDR_VALID_SHIFT 28
#define HS_ADDR_VALID_MASK 0x1
#define BUS_TYPE_SHIFT 20
#define BUS_TYPE_MASK 0x7
#define CHANNEL_NO_SHIFT 15
#define CHANNEL_NO_MASK 0x1F
#define TRANS_SPEED_SHIFT 10
#define TRANS_SPEED_MASK 0x1F
#define SLAVE_ADDR_SHIFT 0
#define SLAVE_ADDR_MASK 0x3FF
//
// Macros to access fields within Bus Realisation Configuration data, used on a per-transaction basis with IIC
#define SET_CONFIG_FIELD(aBusId,aField,aMask,aShift) ((aBusId)=((aBusId) &~ ((aMask)<<(aShift))) | (((aField)&(aMask)) << (aShift)))
#define GET_CONFIG_FIELD(aBusId,aMask,aShift) (((aBusId)>>(aShift))&(aMask))


#define GET_HS_MASTER_ADDR(aBusId) GET_CONFIG_FIELD(aBusId,HS_MASTER_ADDR_MASK,HS_MASTER_ADDR_SHIFT)
#define SET_HS_MASTER_ADDR(aBusId,aHsMasterAddr) SET_CONFIG_FIELD(aBusId,aHsMasterAddr,HS_MASTER_ADDR_MASK,HS_MASTER_ADDR_SHIFT)
#define GET_HS_VALID(aBusId) GET_CONFIG_FIELD(aBusId,HS_ADDR_VALID_MASK,HS_ADDR_VALID_SHIFT)
#define SET_HS_VALID(aBusId,aHsValid) SET_CONFIG_FIELD(aBusId,aHsValid,HS_ADDR_VALID_MASK,HS_ADDR_VALID_SHIFT)
#define GET_BUS_TYPE(aBusId) GET_CONFIG_FIELD(aBusId,BUS_TYPE_MASK,BUS_TYPE_SHIFT)
#define SET_BUS_TYPE(aBusId,aBusType) SET_CONFIG_FIELD(aBusId,aBusType,BUS_TYPE_MASK,BUS_TYPE_SHIFT)
#define GET_CHAN_NUM(aBusId) GET_CONFIG_FIELD(aBusId,CHANNEL_NO_MASK,CHANNEL_NO_SHIFT)
#define SET_CHAN_NUM(aBusId,aChanNum) SET_CONFIG_FIELD(aBusId,aChanNum,CHANNEL_NO_MASK,CHANNEL_NO_SHIFT)
#define SET_TRANS_SPEED(aBusId,aTransSpeed) SET_CONFIG_FIELD(aBusId,aTransSpeed,TRANS_SPEED_MASK,TRANS_SPEED_SHIFT)
#define GET_TRANS_SPEED(aBusId) GET_CONFIG_FIELD(aBusId,TRANS_SPEED_MASK,TRANS_SPEED_SHIFT)
#define SET_SLAVE_ADDR(aBusId,aSlaveAddr) SET_CONFIG_FIELD(aBusId,aSlaveAddr,SLAVE_ADDR_MASK,SLAVE_ADDR_SHIFT)
#define GET_SLAVE_ADDR(aBusId) GET_CONFIG_FIELD(aBusId,SLAVE_ADDR_MASK,SLAVE_ADDR_SHIFT)


// Forward declarations
class TIicBusTransaction;
class DIicBusChannelMaster;

/**
 @publishedPartner
 @prototype 9.6

 Class to represent and provide access to configuration data used on a per-transaction basis with IIC

 @see TIicBusTransaction

 */
class TIicBusTransfer
	{
public:
	enum TReqType
		{
		EMasterRead,
		EMasterWrite
		};
	// the client interface for creating and linking simple requests
	inline TIicBusTransfer(TReqType aType, TInt8 aGranularity, TDes8* aBuffer) :
		iType((TInt8) aType), iNext(NULL)
		{
		iBufGranularity = aGranularity;
		iBuffer = aBuffer;
		}
	inline void LinkAfter(TIicBusTransfer* aPrev)
		{
		iNext = aPrev;
		}
	inline TInt8 WordWidth()
		{
		return iBufGranularity;
		}
	inline TReqType Direction()
		{
		return (TReqType) iType;
		}
	inline TInt Length()
		{
		TInt8 granularityInBytes = (TInt8)(((iBufGranularity - 1) >> 3) + 1);
		return (iBuffer->Size() / granularityInBytes);
		}
	inline const TIicBusTransfer* Next()
		{
		return iNext;
		}
private:
	TInt8 iType; // as one of TReqType
	TInt8 iBufGranularity; // width of a transfer word in bits
	TInt8 iSpare1;
	TInt8 iSpare2;
	const TDes8* iBuffer; // the data for this transfer (packed into 8-bit words with padding)
	TIicBusTransfer* iNext;

	friend class DIicBusChannelMaster;
	};

#endif //End of Temporarily inclusion

struct TUsideTracnDesc
	{
	TDes8* iHeader;
	TIicBusTransfer* iHalfDuplexTrans;
	TIicBusTransfer* iFullDuplexTrans;
	TUint8 iFlags; // used to indicate if it supports a preamble
	TAny* iPreambleArg; // used for preamble argument
	TAny* iMultiTranscArg; // used for multi transc argument
	};

class RBusCsiTestClient: public RBusLogicalChannel
	{
public:
	enum TControl
		{
		ETestAudioCodecBeep,
		ETestTransferTimeout,
		ETestBulkTransfer,
		ETestTestHalfDuplex,
		ETestDuplexTransaction,
		ETestConfigParams,
		ECaptureSlaveChannel,
		EReleaseSlaveChannel,
		EQueueOneDuplexTransaction
		};

	enum TRequest
		{
		ECaptureSlaveChannelAsync,
		ESetSlaveNotificationTrigger,
		ETestAsynTransaction
		};

#ifndef __KERNEL_MODE__

public:
	TInt Open(TDesC& aProxyName)
		{return (DoCreate(aProxyName,TVersion(KIicClientMajorVersionNumber,KIicClientMinorVersionNumber,KIicClientBuildVersionNumber),-1,NULL,NULL,EOwnerThread));}

	// test-cases
	TInt TestAudioCodecBeep()
		{return DoControl(ETestAudioCodecBeep);}

	TInt TestTransferTimeout()
		{return DoControl(ETestTransferTimeout);}

	TInt TestHalfDuplex()
		{return DoControl(ETestTestHalfDuplex);}

	TInt TestBulkTransfer()
		{return DoControl(ETestBulkTransfer);}

	TInt TestDuplexTransaction()
		{return DoControl(ETestDuplexTransaction);}

	TInt TestConfigParams(TConfigSpiBufV01 *aSpiHeader, TUint32 aBusId)
		{return DoControl(ETestConfigParams, (TAny*)aSpiHeader, (TAny*)aBusId);}

	void CaptureSlaveChannel(TInt aBufSize, TRequestStatus &aStatus)
		{DoRequest(ECaptureSlaveChannelAsync, aStatus, (TAny*)aBufSize);}

	TInt CaptureSlaveChannel(TInt aBufSize)
		{return DoControl(ECaptureSlaveChannel, (TAny*)aBufSize);}

	TInt ReleaseSlaveChannel()
		{return(DoControl(EReleaseSlaveChannel));}

	void SetSlaveNotificationTrigger(TRequestStatus &aStatus, TUint32 aTrigger)
		{DoRequest(ESetSlaveNotificationTrigger, aStatus, (TAny*) aTrigger);}

	TInt QueueOneDuplexTransaction(TInt aSize)
		{return DoControl(EQueueOneDuplexTransaction, (TAny*)aSize);}

	void TestAsynTransaction(TRequestStatus &aStatus, TConfigSpiBufV01* aSpiHeader, TUint32 aBusId)
		{DoRequest(ETestAsynTransaction, aStatus, (TAny*)aSpiHeader, (TAny*)aBusId);}

#endif
	};

#ifdef __KERNEL_MODE__
struct TCapsIicClient
	{
	TVersion version;
	};

class DDeviceIicClient: public DLogicalDevice
	{
public:
	DDeviceIicClient();
	~DDeviceIicClient();
	virtual TInt Install();
	virtual void GetCaps(TDes8 &aDes) const;
	virtual TInt Create(DLogicalChannelBase*& aChannel);
	};

// declaration for the client channel
class DChannelIicClient: public DLogicalChannel
	{
public:
	DChannelIicClient();
	~DChannelIicClient();
	virtual TInt DoCreate(TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);

protected:
	virtual void HandleMsg(TMessageBase* aMsg); // Note: this is a pure virtual in DLogicalChannel

	void DoCancel(TInt aMask);
	TInt DoControl(TInt aId, TAny* a1, TAny* a2);
	TInt DoRequest(TInt aId, TRequestStatus* aStatus, TAny* a1, TAny* a2);

private:

#ifdef SLAVE_MODE
	TInt RegisterRxBuffer(TInt aChannelId, TPtr8 aRxBuffer, TInt8 aBufGranularity, TInt8 aNumWords, TInt8 aOffset);
	TInt RegisterTxBuffer(TInt aChannelId, TPtr8 aRxBuffer, TInt8 aBufGranularity, TInt8 aNumWords, TInt8 aOffset);
	TInt CaptureChannel(TInt aBusId, TDes8* aConfigHdr, TIicBusSlaveCallback* aCallback, TInt& aChannelId, TBool aAsynch);
	TInt ReleaseChannel(TInt aChannelId);
	TInt SetNotificationTrigger(TInt aChannelId, TInt aTrigger);
#endif
	
#ifdef MASTER_MODE
    TInt QueueTransaction(TInt aBusId, TIicBusTransaction* aTransaction, TIicBusCallback *aCallback=NULL);
	inline TInt QueueRegisterWriteTransaction(TInt16 aRegister, TInt16 aValue, TIicBusTransaction* aTrans, TInt aConfig);
	TInt TestAudioCodecBeep(TUint8 aFreq, TUint8 aTime, TUint8 aOffTime,
	                        TUint8 aVolume, TBool aRepeat = 0);
	TInt TestBulkTransfer();
	TInt TestHalfDuplexTransaction();
	TInt TestTransferTimeout();
	TInt TestDuplexTransaction();
	TInt TestConfigParams(TInt aBusId);

	static void AsyncTransCallbackFunc(TIicBusTransaction* aTrans, TInt aBusId, TInt aResult, TAny* aParam);
	TInt TestAsynTransaction(TRequestStatus* aStatus,TConfigSpiBufV01* aSpiHeader, TInt aBusId);
#endif
	
	TDynamicDfcQue* iDfcQue; // Kernel thread for the test driver

	// Header which is common to all transactions
	TConfigSpiBufV01* iSpiHeader;
	// member shared in all transfers
	TUint32 iCsiBusId;

#ifdef MASTER_MODE
	// codec test related member - to not pass it on every call to QueueRegisterWriteTransaction()
	TPckgBuf<RCS42AudioCodec::TCodecConfigData> iTransBuff;
#endif

#ifdef SLAVE_MODE
	// Slave test..
	// Slave callback function:
	static void SlaveCallbackFunc(TInt aChannelId, TInt aReturn, TInt aTrigger, TInt16 aRxWords, TInt16 aTxWords, TAny* aParam);
	
	TInt RegisterSlaveBuffers();
	TInt CaptureSlaveChannel(TInt aBufSize, TBool aAsync = EFalse);
	TInt ReleaseSlaveChannel();
	TInt SetSlaveNotificationTrigger(TUint32 aTrigger);

	// The object is created on the heap  and buffers are needed for the whole
	// time the object exists on the heap, so these buffers can be a part of the object(as they are used in all tests)
	// buffers for the Slave..
	TBuf8<KMaxSlaveTestBufferSize> iTxBuf; // buffer..
	TBuf8<KMaxSlaveTestBufferSize> iRxBuf; // buffer..
	TInt iSlaveBufSize;
	
	TInt iChannelId; // cookie for SlaveChannel - assigned when capturing the channel..
	TRequestStatus* iSlaveReqStatus;
#endif
	
	// Master callback function (used when queuing an asynch master transaction)
	static void MasterCallbackFunc(TIicBusTransaction* aTransction, TInt aBusId, TInt aResult, TAny* aParam);
	TInt QueueOneDuplexTransaction(TInt aSize);
	// transaction for the Master's asynchronous transaction..
	TIicBusTransaction iMasterTransaction;
	TIicBusSlaveCallback *iSlaveCallback;
	TIicBusCallback *iMasterCallback;
#ifdef STANDALONE_CHANNEL
#ifdef MASTER_MODE
	DCsiChannelMaster* iMasterChannel;
#endif/*Master mode*/

#ifdef SLAVE_MODE
	DCsiChannelSlave* iSlaveChannel;
#endif/*Slave mode*/
#endif /*STANDALONE_CHANNEL*/
	DThread* iClient;
	};


//Class to be passed along with Asynchronous Transaction.
#ifdef MASTER_MODE
template <int dim>
class TCallBckRequest
	{
public:
	TCallBckRequest(DThread* aClient, TRequestStatus* aReqStatus, TConfigSpiBufV01* aHeader) :
	  iClient(aClient),
	  iReqStatus(aReqStatus),
	  iHeader(aHeader),
	  iCallback(NULL)
	  {}

	TCallBckRequest() : iClient(NULL), iReqStatus(NULL), iHeader(NULL), iCallback(NULL)
		{
		for(TUint i = 0; i<dim; ++i)
			{
			iBuffers[i]   = NULL;
			iTransfers[i] = NULL;
			}
		}

	inline TIicBusTransfer* GetTransfer(TInt index)
		{
		// __ASSERT_DEBUG(index >= 0 && index < dim, )
		return iTransfers[index];
		}

	inline TIicBusTransfer* GetBuffer(TInt index)
		{
		// __ASSERT_DEBUG(index >= 0 && index < dim, )
		return iBuffers[index];
		}

	// destructor.
	inline ~TCallBckRequest()
		{
		delete iHeader;
		delete iCallback;
		for(TUint i = 0; i<dim; ++i)
			{
			delete iBuffers[i];
			delete iTransfers[i];
			}
		}

	DThread* iClient;
	TRequestStatus* iReqStatus;

	// all following need to be freed
	TConfigSpiBufV01* iHeader;
	TIicBusCallback *iCallback;

	// to store buffers..
	HBuf8* iBuffers[dim];

	// to store transfers..
	TIicBusTransfer* iTransfers[dim];

	};
#endif /*MASTER_MODE*/

#endif /* __KERNEL_MODE__ */

#endif /* __D_CSI_H__ */
