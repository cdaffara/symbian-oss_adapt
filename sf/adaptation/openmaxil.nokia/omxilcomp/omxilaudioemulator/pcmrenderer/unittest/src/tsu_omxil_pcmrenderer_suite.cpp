// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
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

/**
 @file
 @internalComponent
*/

#include <e32base.h>

#include "log.h"
#include "tsu_omxil_pcmrenderer_suite.h"
#include "tsu_omxil_pcmrenderer_step.h"
#include "omxilpcmrenderer.hrh"

/**
 *
 * NewTestSuiteL
 *	NewTestSuite is exported at ordinal 1
 *	this provides the interface to allow schedule test
 *	to create instances of this test suite
 * @result CTestSuiteOmxILPcmRenderer*
 *
 */
EXPORT_C CTestSuiteOmxILPcmRenderer* NewTestSuiteL()
	{
	CTestSuiteOmxILPcmRenderer* result = new (ELeave) CTestSuiteOmxILPcmRenderer;
	CleanupStack::PushL(result);
	result->ConstructL();
	CleanupStack::Pop(); // result
	return result;
	}

/**
 *
 * CTestSuiteOmxILPcmRenderer
 *
 */
CTestSuiteOmxILPcmRenderer::CTestSuiteOmxILPcmRenderer()
	{
    DEBUG_PRINTF(_L8("CTestSuiteOmxILPcmRenderer::CTestSuiteOmxILPcmRenderer"));

	}


/**
 *
 * ~CTestSuiteOmxILPcmRenderer
 *
 */
CTestSuiteOmxILPcmRenderer::~CTestSuiteOmxILPcmRenderer()
	{
    DEBUG_PRINTF(_L8("CTestSuiteOmxILPcmRenderer::~CTestSuiteOmxILPcmRenderer"));
	}
/**
 *
 * GetVersion
 *	Get Test Suite version
 * @result TPtrC
 *
 */
TPtrC CTestSuiteOmxILPcmRenderer::GetVersion( void ) const
	{
	_LIT(KTxtVersion,"1.00");
	return KTxtVersion();
	}



/**
 *
 * InitialiseL
 *	Constructor for test suite
 *	this creates all the test steps and
 *	stores them inside CTestSuiteOmxILPcmRenderer
 *
 */
void CTestSuiteOmxILPcmRenderer::InitialiseL(void)
	{
	// store the name of this test suite
	iSuiteName = _L("TSU_OMXIL_PcmRenderer");

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0020(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0020-HP"),
			KUidSymbianOmxILPcmRenderer));

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0021(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0021-HP"),
			KUidSymbianOmxILPcmRenderer));

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0022(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0022-HP"),
			KUidSymbianOmxILPcmRenderer));

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0001(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0001-HP"),
			KUidSymbianOmxILPcmRenderer));

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0002(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0002-HP"),
			KUidSymbianOmxILPcmRenderer));

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0003(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0003-HP"),
			KUidSymbianOmxILPcmRenderer));

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0004(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0004-HP"),
			KUidSymbianOmxILPcmRenderer));

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0005(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0005-HP"),
			KUidSymbianOmxILPcmRenderer));

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0006(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0006-HP"),
			KUidSymbianOmxILPcmRenderer));

	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0007(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0007-HP"),
			KUidSymbianOmxILPcmRenderer));
	
	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0008(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0008-HP"),
			KUidSymbianOmxILPcmRenderer));
	
	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0009(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0009-HP"),
			KUidSymbianOmxILPcmRenderer));
	
	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0010(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0010-HP"),
			KUidSymbianOmxILPcmRenderer));
	
	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0030(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0030-HP"),
			KUidSymbianOmxILPcmRenderer));
	
	AddTestStepL(
		new(ELeave) RAsyncTestStepOmxILPcmRenderer0031(
			_L("MM-OMXIL-OMXILPcmRenderer-U-0031-HP"),
			KUidSymbianOmxILPcmRenderer));

	}
