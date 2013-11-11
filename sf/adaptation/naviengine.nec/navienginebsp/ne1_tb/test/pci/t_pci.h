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
* Description:  This is the header file for the PCI driver test. 
*
*/


#ifndef __TPCI_TEST_H
#define __TPCI_TEST_H

#ifndef __KERNEL_MODE__
#define __E32TEST_EXTENSION__
#include <e32def_private.h>

// Following contents are carbon copy of \os\kernelhwsrv\kerneltest\e32test\misc\test_thread.h
#include <e32test.h>
#include <e32svr.h>
#include <e32des8.h>
#include <e32des8_private.h>
#include <e32cmn.h>
#include <e32cmn_private.h>
#include <e32std.h>
#include <e32std_private.h>
    

_LIT(KPciPanicCat, "test_thread.h");

static const TInt KPciHeapSize=0x2000;

enum TPciPanicCode
	{
	EThreadCreateFailed
	};

/**
A utility class for running functions in other threads/processes
*/
class TTestRemote
	{
public:
	virtual TInt WaitForExitL() = 0;
	virtual ~TTestRemote()
		{}

	virtual void Rendezvous(TRequestStatus& aStatus) = 0;

protected:
	TTestRemote()
		{}

	static TInt RunFunctor(TAny* aFunctor)
		{
		TFunctor& functor = *(TFunctor*)aFunctor;
		functor();
		return KErrNone;
		}

	TRequestStatus iLogonStatus;
	static TInt iCount;
	};
TInt TTestRemote::iCount=0;

class TTestThread : public TTestRemote
	{
public:
	TTestThread(const TDesC& aName, TThreadFunction aFn, TAny* aData, TBool aAutoResume=ETrue)
		{
		Init(aName, aFn, aData, aAutoResume);
		}

	/**
	Run aFunctor in another thread
	*/
	TTestThread(const TDesC& aName, TFunctor& aFunctor, TBool aAutoResume=ETrue)
		{
		Init(aName, RunFunctor, &aFunctor, aAutoResume);
		}

	~TTestThread()
		{
		//RTest::CloseHandleAndWaitForDestruction(iThread);
		iThread.Close();
		}

	void Resume()
		{
		iThread.Resume();
		}

	/**
	If thread exited normally, return its return code
	Otherwise, leave with exit reason
	*/
	virtual TInt WaitForExitL()
		{
		User::WaitForRequest(iLogonStatus);
		const TInt exitType = iThread.ExitType();
		const TInt exitReason = iThread.ExitReason();

		__ASSERT_ALWAYS(exitType != EExitPending, User::Panic(_L("TTestThread"),0));

		if(exitType != EExitKill)
			User::Leave(exitReason);

		return exitReason;
		}

	virtual void Rendezvous(TRequestStatus& aStatus)
		{
		iThread.Rendezvous(aStatus);
		}

private:
	void Init(const TDesC& aName, TThreadFunction aFn, TAny* aData, TBool aAutoResume)
		{
		TKName name(aName);
		name.AppendFormat(_L("-%d"), iCount++);	
		TInt r=iThread.Create(name, aFn, KDefaultStackSize, KPciHeapSize, KPciHeapSize, aData);
		if(r!=KErrNone)
			{
			RDebug::Printf("RThread::Create failed, code=%d", r);
			User::Panic(KPciPanicCat, EThreadCreateFailed);
			}
		
		iThread.Logon(iLogonStatus);
		__ASSERT_ALWAYS(iLogonStatus == KRequestPending, User::Panic(_L("TTestThread"),0));

		if(aAutoResume)
			iThread.Resume();
		}

	RThread iThread;
	};

class CTest : public CBase, public TFunctor
	{
public:
	~CTest()
		{
		iName.Close();
		}

	virtual void operator()()
		{
		RTest test(iName);
		test.Start(iName);
		for(TInt i=0; i<iIterations; i++)
			{
			test.Next(iName);
			RunTest();
			}
		test.End();
		}

	virtual void RunTest() = 0; 

	virtual CTest* Clone() const = 0;

	const TDesC& Name() const
		{
		return iName;
		}

protected:
	CTest(const TDesC& aName, TInt aIterations)
		:iIterations(aIterations)
		{
		iName.CreateL(aName);
		}


	
	CTest(const CTest& aOther)
		:iIterations(aOther.iIterations)
		{
		iName.CreateL(aOther.iName);
		}

	//It would be useful to have an RTest member, but this can't be
	//initialised untill the new thread is running as it will refer to
	//the creating thread
	RBuf iName;
	const TInt iIterations; 
	};

/**
Make aNumberOfThreads copies of aTest and run
each in its own thread

@param test Reference to test object
@param aTest Referance
*/
void MultipleTestRun(RTest& test, const CTest& aTest, TInt aNumberOfThreads)
	{
	RPointerArray<CTest> testArray;
	RPointerArray<TTestThread> threadArray;

	for(TInt i=0; i<aNumberOfThreads; i++)
		{		
		test.Printf(_L("Create test thread"));
		CTest* newTest = aTest.Clone();
		test_NotNull(newTest);

		TTestThread* thread = new TTestThread(aTest.Name(), *newTest);
		test_NotNull(thread);

		threadArray.AppendL(thread);
		testArray.AppendL(newTest);
		}

	const TInt count = threadArray.Count();
	for(TInt j=0; j<count; j++)
		{
		TTestThread* thread = threadArray[j];
		
		TInt r = KErrNone;
		TRAPD(leaveCode, r = thread->WaitForExitL());
		if(leaveCode != KErrNone)
			{
			test.Printf(_L("Thread %d: Panic code:%d\n"), j, leaveCode);
			test_KErrNone(leaveCode);
			}

		if(r!=KErrNone)
			{
			test.Printf(_L("Thread Number %d\n"), j);
			test_KErrNone(r);
			}
		}
	
	threadArray.ResetAndDestroy();
	threadArray.Close();

	testArray.ResetAndDestroy();
	testArray.Close();
	}
// end of \os\kernelhwsrv\kerneltest\e32test\misc\test_thread.h

#endif // __KERNEL_MODE__

_LIT(KPciLdd, "d_pci.ldd");
_LIT(KPciLddFactory, "PCI_test_factory");
_LIT(KPciTest, "PCI Test LDD");

/**
Test driver op-codes
*/
enum TPciTestCmd
	{
	EGetTestInfo,
	EAccessConfigSpace,
	EAccessMemorySpace,
	EOpenPciDChunk,
	EOpenPciPlatHwChunk,
	EOpenPciMappedChunk,
	EOpenPciWindowChunk,
	ERunUnitTests
	};

/**
Identifies a PCI Function (device) on the system
*/
struct TPciDevice
	{
	TPciDevice()
		:iVendorId(0xFFFFFFFF), iDeviceId(0xFFFFFFFF), iInstance(0) {}

	TPciDevice(TUint aVendorId, TUint aDeviceId, TInt aInstance=0)
		:iVendorId(aVendorId), iDeviceId(aDeviceId), iInstance(aInstance) {}

	TUint iVendorId;
	TUint iDeviceId;
	TInt iInstance; //< Unit to open (there could be multiple devices on system)
	};

/**
Used to send chunk size and recieve
PCI address
*/
struct TPciChunkCreateInfo
	{
	TPciChunkCreateInfo()
		:iSize(0), iPciAddress(NULL)
		{
		}

	TPciChunkCreateInfo(TInt aSize, TUint& aPciAddress, TRequestStatus* aStatus=NULL)
		:iSize(aSize), iPciAddress(&aPciAddress), iStatus(aStatus)
		{
		}
	TInt iSize;
	TUint* iPciAddress;
	TRequestStatus* iStatus;
	};	

/**
Information about the PSL required by the
user side test
*/
struct TPciTestInfo
	{
	TPciDevice iDevice; //< Probe for this

	/**
	Supplies the necessary information to test Read, Write, and
	Modify for a word of PCI memory or configuration space
	*/
	struct TAddrSpaceTest
		{
		TAddrSpaceTest()
			:iOffset(0), iExpectedValue(0), iReadOnlyMask(0)
			{}

		TAddrSpaceTest(TUint aOffset, TUint aExpectedValue, TUint aReadOnlyMask)
			:iOffset(aOffset), iExpectedValue(aExpectedValue), iReadOnlyMask(aReadOnlyMask)
			{}

		/**
		Returns a specified sub byte, or word from the whole dword
		*/
		inline TUint Expected(TInt aBitWidth, TInt aExtraOffset) const
			{
			//the right shift required to get field to bit 0
			const TInt shift = 8 *((aExtraOffset + iOffset) % 4);
			
			const TUint mask = 0xFFFFFFFF >> (32-aBitWidth);
			return (iExpectedValue >> shift) & mask;
			}

		const TUint iOffset;
		const TUint iExpectedValue; //< The initial value of word
		const TUint iReadOnlyMask; //< Mask of unwritable bits
		//Future work, memory spaces should state a bar index
		};


	TAddrSpaceTest iCfgSpaceRead;
	TAddrSpaceTest iCfgSpaceWrite;

	TUint iMemSpaceIndex; //< Memory space to select
	TAddrSpaceTest iMemSpaceRead;
	TAddrSpaceTest iMemSpaceWrite;

	TInt iNumberOfBars; //< Number of simultaneous mappings into PCI space
	};

class RPci;
class TAddrSpace;
/**
This class encapsulates all the various read/write/and modify commands
that can be carried out on a PCI memory space. The command is stored user
side, and then executed on kernel side when KRun() is called.
*/
class TUserPciSpace
	{
public:
	TUserPciSpace()
		:iPci(NULL), iOperation(EInvalid), iBitWidth(0), iOffset(0),
		iWriteValue(0), iClearMask(0), iSetMask(0)
	{}
	TUserPciSpace(RPci& aPci);
	
	/**
	Perform the encapsulated read/write/or modify
	@note Only run on kernel side
	*/
	TUint KRun(TAddrSpace& aAddrSpace);
	
	/**
	Clone method is required so that multiple threads may
	have their own copy of a TUserPciSpace (without knowing
	its runtime type)
	*/
	virtual TUserPciSpace* Clone() const = 0;

	TUint Read(TInt aBitWidth, TUint aOffset)
		{
		iOffset = aOffset;
		iOperation = ERead;
		iBitWidth = aBitWidth;

		return Call();
		}

	void Write(TInt aBitWidth, TUint aOffset, TUint aValue)
		{
		iOffset = aOffset;
		iOperation = EWrite;
		iBitWidth = aBitWidth;
		
		iWriteValue = aValue;
		Call();
		}

	void Modify(TInt aBitWidth, TUint aOffset, TUint aClearMask, TUint aSetMask)
		{
		iOffset = aOffset;
		iOperation = EModify;
		iBitWidth = aBitWidth;

		iClearMask = aClearMask;
		iSetMask = aSetMask;
		Call();
		}

protected:
	/**
	Makes a request to iPci and passes a copy of this object to
	the kernel side.
	*/
	virtual TUint Call() =0;

	enum TOperation {EInvalid, ERead, EWrite, EModify};

	/**
	Pointer to a PCI device handle
	*/
	RPci* iPci;

	TOperation iOperation; //!< Type of access to perform
	TInt iBitWidth;
	
	TUint iOffset;
	TUint32 iWriteValue;
	TUint32 iClearMask;
	TUint32 iSetMask;
	};

/**
Grants access to a PCI device's (identified
by aPci) config space from user side
*/
class TUserConfigSpace : public TUserPciSpace
	{
public:
	TUserConfigSpace()
		:TUserPciSpace()
		{}
	TUserConfigSpace(RPci& aPci);

	virtual TUserPciSpace* Clone() const;
private:
	TUint Call();
	};

/**
Grants access to some region of a PCI
device's memory space. A PCI device(or function)
may have up to 8 distinct memory spaces
*/
class TUserMemorySpace : public TUserPciSpace
	{
public:
	TUserMemorySpace()
		:TUserPciSpace(), iBarIndex(-1)
		{}

	TUserMemorySpace(RPci& aPci, TInt aBarIndex);	

	virtual TUserPciSpace* Clone() const;
	
	inline TInt BarIndex() {return iBarIndex;}

private:
	TUint Call();

	TInt iBarIndex; //< Each PCI function may have up to 8 memory spaces
	};

#endif //__TPCI_TEST_H

