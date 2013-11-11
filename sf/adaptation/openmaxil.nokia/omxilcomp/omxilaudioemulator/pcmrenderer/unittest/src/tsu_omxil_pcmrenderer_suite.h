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

#ifndef TSU_OMXIL_PCMRENDERER_SUITE_H
#define TSU_OMXIL_PCMRENDERER_SUITE_H

#include <testframework.h>

/**
 *
 * CTestSuiteOmxILPcmRenderer defines the test suite for OMX IL PCM Renderer tests
 * 
 *
 *
 */
class  CTestSuiteOmxILPcmRenderer : public CTestSuite
	{

public:
CTestSuiteOmxILPcmRenderer();
	void InitialiseL(void);
	virtual ~CTestSuiteOmxILPcmRenderer();
	virtual TPtrC GetVersion( void ) const;
	};

#endif // TSU_OMXIL_PCMRENDERER_SUITE_H
