/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#include <e32test.h>
#include <e32ver.h>
#include <e32cmn.h>
#include <e32def.h>
#include <e32def_private.h>
#include "d_csi.h"

_LIT(testName,"t_csi");
_LIT(KLddFileName, "d_csi.ldd"); // Kernel-side proxy LDD acting as a client of the IIC
_LIT(KLddFileNameRoot, "d_csi");

// global data..
GLDEF_D RTest csiTest(testName);
GLDEF_D RBusCsiTestClient gCsiLdd; // Channel to kernel-side proxy
GLDEF_D TConfigSpiBufV01 SpiHeader;
GLDEF_D TUint32 SpiConfig;

LOCAL_C void SetDefaultHeader()
	{
	const TConfigSpiV01 DefaultHeader =
		{
		ESpiWordWidth_8, //iWordWidth
		260000, //iClkSpeed
		ESpiPolarityLowRisingEdge, //iClkMode
		500, // iTimeoutPeriod
		EBigEndian, // iEndianness
		EMsbFirst, //iBitOrder
		0, //iTransactionWaitCycles
		ESpiCSPinActiveLow //iCsPinActiveMode
		};

	SpiHeader = DefaultHeader;
	}

LOCAL_C void SetDefaultConfig()
	{
	//Set the default settings
	SpiConfig = 0;
	SET_CHAN_NUM(SpiConfig,KSpiChannel0); //0 or
	SET_BUS_TYPE(SpiConfig,ESpi);
	SET_SLAVE_ADDR(SpiConfig,1); // Any Number between 1 to 32  */
	SetDefaultHeader();
	}

// ==================
// MASTER tests
// clock values supported by the navi-engine CSI interface
const TUint32 TSpiClkSpeedValues[] =
	{130000, 260000, 521000, 1040000, 2080000, 4170000, 16670000};

LOCAL_C void TestClockSpeed()
	{
	TInt r = KErrNone;

	// try to set each of the supported clk speeds. (queuing a transaction using
	// header with one of them set)..
	for(TUint i = 0; i < sizeof(TSpiClkSpeedValues) / sizeof(TUint32); ++i)
		{
		SpiHeader().iClkSpeedHz = TSpiClkSpeedValues[i];
		r = gCsiLdd.TestConfigParams(&SpiHeader, SpiConfig);
		csiTest(r == KErrNone);
		}

	// now try some of known - as not supported:
	SpiHeader().iClkSpeedHz = 300000;
	r = gCsiLdd.TestConfigParams(&SpiHeader, SpiConfig);
	if(r != KErrNotSupported)
		{
		csiTest.Printf(_L("Error setting clk %d, r = %d \n"), SpiHeader().iClkSpeedHz, r);
		csiTest(EFalse);
		}
	// Set it to the default header after testing
	SetDefaultHeader();
	}


LOCAL_C void TestWordWidth()
	{
	TInt r = KErrNone;
	for (TInt i = 0; i <= ESpiWordWidth_16; i++)
		{
		SpiHeader().iWordWidth = static_cast<TSpiWordWidth> (i);
		r = gCsiLdd.TestConfigParams(&SpiHeader, SpiConfig);
		switch (i)
			{
			case ESpiWordWidth_8:
			case ESpiWordWidth_16:

				csiTest(r == KErrNone);
				break;
			default:
				csiTest(r == KErrNotSupported);
				break;
			}//Switch
		} //For loop
	//Set it to the default header after testing
	SetDefaultHeader();
	}

LOCAL_C void TestAsynchTransactions()
	{
	// Queue 3 asynchronous Transaction and expect them to nogified in FIFO order.
	const TInt KNumberOfTransactions = 3;
	TRequestStatus status[KNumberOfTransactions];

	TInt r = KErrNone;

	for(TInt i = 0; i < KNumberOfTransactions; ++i)
		{
		status[i] = KErrNotReady;
		gCsiLdd.TestAsynTransaction(status[i], &SpiHeader, SpiConfig);
		}

	// And Expect them to be notified in the same order.
	for(TInt i = 0; i < KNumberOfTransactions; ++i)
		{
		User::WaitForAnyRequest();

		// next should be finished..
		if(status[i] != KErrNone)
			{
			r = KErrGeneral;
			continue;
			}

		if(i == 0) // only first should be ready..
			{
			if((status[1] == KErrNone) ||
			   (status[2] == KErrNone))
				{
				r = KErrGeneral;
				continue;
				}
			}

		if(i == 1) // only first two should be ready..
			{
			if(status[2] == KErrNone)
				{
				r = KErrGeneral;
				}
			}
		}

	csiTest(r == KErrNone);
	}

LOCAL_C void TestMaster()
	{
	TInt r = KErrNone;

	// Instigate AudioCodecBeep
	csiTest.Printf(_L("Testing Audio Codec Beep\n"));
	r = gCsiLdd.TestAudioCodecBeep();	
	if(r == KErrNotSupported)
	    {
	    csiTest.Printf(_L("Master mode is not supported by the client, skipping master test.. \n"));
	    return;
	    }
	csiTest(r == KErrNone);

	// Instigate TransferTimeout
	csiTest.Printf(_L("Testing Transfer Timeout\n"));
	r = gCsiLdd.TestTransferTimeout();
	csiTest(r == KErrNone);

	// Test TestBulkTransfer
	csiTest.Printf(_L("Testing TestBulkTransfer\n"));
	r = gCsiLdd.TestBulkTransfer();
	csiTest(r == KErrNone);

	// Test Half Duplex
	csiTest.Printf(_L("Testing Half Duplex operations \n"));
	r = gCsiLdd.TestHalfDuplex();
	csiTest(r == KErrNone);

	// TestDuplexTransaction
	csiTest.Printf(_L("Testing DuplexTransaction\n"));
	r = gCsiLdd.TestDuplexTransaction();
	csiTest(r == KErrNone);

	// Test all available  ClockSpeeds
	csiTest.Printf(_L("Testing Different  ClockSpeeds\n"));
	TestClockSpeed();

	// Test WordWidth
	csiTest.Printf(_L("Testing WordWidths\n"));
	TestWordWidth();

	csiTest.Printf(_L("Testing  Asynchronous Transactions\n"));
	TestAsynchTransactions();
	}

// ==================
// SLAVE tests
LOCAL_C TInt SlaveCaptureChannel(TInt aBufferSize)
	{
	csiTest.Printf(_L("SlaveCaptureChannel(), buff %d\n"), aBufferSize);
	// Open Slave channel (use buffers declared in the d_csi)
	TInt r = gCsiLdd.CaptureSlaveChannel(aBufferSize);
	if(r != KErrNone)
		{
		csiTest.Printf(_L("couln't Capture Slave Channel, r= %d\n"), r);
		}
	return r;
	}

LOCAL_C TInt SlaveAsyncCaptureChannel(TInt aBufferSize)
	{
	// Open Slave channel (use buffers declared in the d_csi)
	TRequestStatus status;
	TInt r = KErrNone;

	gCsiLdd.CaptureSlaveChannel(aBufferSize, status);

	User::WaitForRequest(status);
	r = status.Int();
	if (r != KErrCompletion)
		{
		csiTest.Printf(_L("SlaveAsyncCaptureChannel request returned, r= %d\n"), r);
		}
	return r;
	}

LOCAL_C TInt SlaveReleaseChannel()
	{
	TInt r = gCsiLdd.ReleaseSlaveChannel();
	if (r != KErrNone)
		{
		csiTest.Printf(_L("couln't Release Slave Channel, r= %d\n"), r);
		}
	return r;
	}

#ifdef EXTERNAL_HW_LOOPBACK_USED
LOCAL_C void StartSlaveRequest(TRequestStatus &aStatus)
	{
	csiTest.Printf(_L("StartSlaveRequest\n"));

	enum TTriggers
		{
		ERxAllBytes = 0x01,
		ERxUnderrun = 0x02,
		ERxOverrun = 0x04,
		ETxAllBytes = 0x08,
		ETxUnderrun = 0x10,
		ETxOverrun = 0x20,
		EGeneralBusError = 0x40
		};

	// register for all possible notifications..
	TInt trigger = ERxAllBytes | ERxOverrun | ERxUnderrun | ETxAllBytes | ETxOverrun | ETxUnderrun;

	// Set notification trigger - this starts asynchronous Slave operation..
	csiTest.Printf(_L("Set Notification trigger\n"));
	gCsiLdd.SetSlaveNotificationTrigger(aStatus, trigger);
	}

LOCAL_C TInt WaitForSlaveReqest(TRequestStatus &aStatus)
	{
	csiTest.Printf(_L("Waiting for the slave to complete..\n"));
	User::WaitForRequest(aStatus);
	TInt r = aStatus.Int();
	if(r != KErrNone)
		{
		csiTest.Printf(_L("request returned, r= %d\n"), r);
		}
	return r;
	}

// Requirements: local-HW loopback between CSI channel 0 (master) and 1 (Slave)
// in this test scenario we will:
// 1. Initiate Asynchronous, full duplex Slave operation with buffers set 64 Bytes
// 2. Queue one full duplex Master transfer with 64 bytes
// 3. Wait and check the SlaveRequest competition
LOCAL_C TInt TestSlaveNormalTransfer()
	{
	csiTest.Printf(_L("TestSlaveNormalTransfer()\n"));
	// capture the channel
	TInt r = SlaveCaptureChannel(64);
	csiTest(r == KErrNone);

	// instigate the request..
	TRequestStatus status;
	StartSlaveRequest(status);

	// queue One FullDuplex transaction..
	// (it can be either synchronous or asynchronous)
	// specify both MASTER_MODE and SLAVE_MODE to run the test
	csiTest.Printf(_L("QueueOneDuplexTransaction\n"));
	r = gCsiLdd.QueueOneDuplexTransaction(64);

	if (r == KErrNone)
		{
		csiTest.Printf(_L("WaitForSlaveRequest\n"));
		r = WaitForSlaveReqest(status);
		}
	else if (r == KErrNotSupported)
		{
		return r;
		}
	else
		{
		csiTest.Printf(_L("Error queuing duplex trans?\n"));
		}

	// release the channel..
	SlaveReleaseChannel();

	return r;
	}
#endif

LOCAL_C void TestSlave()
	{
	csiTest.Printf(_L("TestSlave operations\n"));
	TInt r = KErrNone;

	csiTest.Printf(_L("Capture channel\n"));
	r = SlaveCaptureChannel(64);
	if(r == KErrNotSupported)
	    {
	    csiTest.Printf(_L("Slave mode is not supported by the client, skipping slave tests.. \n"));
	    return;
	    }
	csiTest(r == KErrNone);

	csiTest.Printf(_L("Capture already captured channel\n"));
	r = SlaveCaptureChannel(64);
	csiTest(r == KErrInUse);

	csiTest.Printf(_L("Release channel\n"));
	r = SlaveReleaseChannel();

	csiTest.Printf(_L("Asynchronously capture channel\n"));
	r = SlaveAsyncCaptureChannel(64);
	csiTest(r == KErrCompletion);

	csiTest.Printf(_L("Release channel\n"));
	r = SlaveReleaseChannel();

#ifdef EXTERNAL_HW_LOOPBACK_USED
	// this test needs additional HW loopback (channel 0 and 1) attached to the board
	r = TestSlaveNormalTransfer();
	if(r == KErrNotSupported)
		{
		csiTest.Printf(_L("TestSlaveNormalTransfer() is not supported for slave only mode \n"));
		return;
		}
	csiTest(r == KErrNone);
#endif
	}


LOCAL_C void LoadTestDriver()
	{
	//	csiTest.Printf(_L("Loading the proxy-device driver\n"));
	TInt r = User::LoadLogicalDevice(KLddFileName);
	if(r != KErrNone && r != KErrAlreadyExists)
		{
		csiTest.Printf(_L("Failed to load the proxy-device  driver, r= %d\n"), r);
		csiTest.End();
		}
	}

LOCAL_C void OpenTestDriver()
	{
	// Open a Master SPI channel to the kernel side proxy
	TBufC<6> proxyName(KLddFileNameRoot);
	//	csiTest.Printf(_L("opening the proxy-device driver\n"));
	TInt r = gCsiLdd.Open(proxyName);
	if(r != KErrNone)
		{
		csiTest.Printf(_L("Failed to open the proxy-device the driver, r= %d\n"), r);
		csiTest(r == KErrNone);
		}
	//Have  the default config
	SetDefaultConfig();
	}

LOCAL_C void UnloadTestDriver()
    {
    TInt r = User::FreeLogicalDevice(KLddFileName);
    if(r != KErrNone)
        {
        csiTest.Printf(_L("Failed to unload the proxy-device  driver, r= %d\n"), r);
        }
    }

/*************************************************
 *********Main************************************
 ************************************************/
EXPORT_C TInt E32Main()
	{   
	csiTest.Title();
	csiTest.Start(_L("Test CSI Master \n"));

	//Load the Test Driver
	LoadTestDriver();
	__KHEAP_MARK;
	
	OpenTestDriver();

	// Run tests for the Slave
	TestMaster();

	// Run tests for the Slave
	TestSlave();
	
	//Close the driver
	csiTest.Printf(_L("Tests completed OK, about to close channel\n"));

	gCsiLdd.Close();

	__KHEAP_MARKEND;

	UnloadTestDriver();
	csiTest.End();	

	return KErrNone;
	}

