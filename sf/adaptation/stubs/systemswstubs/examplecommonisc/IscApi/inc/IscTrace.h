/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  ?Description
*
*/


#ifndef ISCTRACE_H
#define ISCTRACE_H

// INCLUDES
// CONSTANTS

// MACROS

#ifdef TRACE_MODE_KERNEL

	// INCLUDES
    #include <kernel.h>

	#define _T(a) a
#else 

	// INCLUDES
	#include <e32svr.h>	// RDebug
    #include <e32std.h>

	#define _T(a) _L(a)

#endif // TRACE_MODE_KERNEL

#ifdef _DEBUG

    #ifdef TRACE_MODE_KERNEL
    	#define __TRACE_PRINTF(a) Kern::Printf a
    #else
	    #define __TRACE_PRINTF(a) RDebug::Print a
    #endif

#else
    #define __TRACE_PRINTF(a)
#endif // _DEBUG


#ifdef _DEBUG

    void BuildTraceFunc( const TDesC& aPrefix );
    void DataDumpTraceFunc( const TDesC& aPrefix, const TDesC8& aData );

    #define BUILD_TRACE( a ) BuildTraceFunc( a )
	#define DATA_DUMP_TRACE( a, b ) DataDumpTraceFunc( a, b )

    #ifdef TRACE_MODE_KERNEL

		// define assertions
        #define TRACE_ASSERT(a) if (!(a)) __TRACE_PRINTF(("Assertion failed: file=" __FILE__ ", line=%d, compiled=" __DATE__" "__TIME__, __LINE__ ))
        
    #else 

		// define assertions
        #ifdef __WINS__
            #define TRACE_ASSERT(a) if (!(a)) __TRACE_PRINTF((_L("Assertion failed: file= __FILE__ , line=%d, compiled=__DATE__ __TIME__ "),__LINE__))
        #else
            #define TRACE_ASSERT(a) if (!(a)) __TRACE_PRINTF((_L("Assertion failed: file=" __FILE__ ", line=%d, compiled="__DATE__ __TIME__""),__LINE__ ))
        #endif  //__WINS__

    #endif // TRACE_MODE_KERNEL


    #if defined COMPONENT_TRACE_FLAG
        #define C_TRACE(a) __TRACE_PRINTF(a)
    #else
        #define C_TRACE(a) 
    #endif // COMPONENT_TRACE_FLAG

    #if defined API_TRACE_FLAG
        #define A_TRACE(a) __TRACE_PRINTF(a)
    #else
        #define A_TRACE(a)
    #endif // API_TRACE_FLAG

    #if defined EXTENDED_TRACE_FLAG
        #define E_TRACE(a) __TRACE_PRINTF(a)
	#else
		#define E_TRACE(a)
    #endif // EXTENDED_TRACE_FLAG

#else
    #define TRACE_ASSERT(a)
    #define C_TRACE(a)
    #define A_TRACE(a)
    #define E_TRACE(a)
	#define BUILD_TRACE(a)
    #define DATA_DUMP_TRACE(a, b)
#endif // _DEBUG


// Resets are made both in UDEB and UREL (traces only in udeb)
#ifdef TRACE_MODE_KERNEL
    #define ASSERT_RESET_ALWAYS(a,b,c) if(!(a)) { __TRACE_PRINTF(("Assertion failed: file=" __FILE__ ", line=%d, compiled="__DATE__" "__TIME__, __LINE__)); Kern::Fault(b,c); }
	#define TRACE_ASSERT_ALWAYS __TRACE_PRINTF( ("Assertion failed: file=" __FILE__ ", line=%d, compiled="__DATE__" "__TIME__, __LINE__) )
#else
    #ifdef __WINS__
		#define ASSERT_RESET_ALWAYS(a,b,c) error 
        #define TRACE_ASSERT_ALWAYS __TRACE_PRINTF( (_L("Assertion failed: file= __FILE__ , line=%d, compiled=__DATE__ __TIME__ "),__LINE__) )
    #else
		#define ASSERT_RESET_ALWAYS(a,b,c) error 
        #define TRACE_ASSERT_ALWAYS __TRACE_PRINTF( (_L("Assertion failed: file=%s, line=%d, compiled=%s %s "),__FILE__, __LINE__, __DATE__, __TIME__) )
    #endif
#endif // kernel


// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

#endif // ISCTRACE_H

//  End of File
