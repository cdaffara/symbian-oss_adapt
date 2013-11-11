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



#ifndef __SWI_LOG_H__
#define __SWI_LOG_H__

#include <e32debug.h>

namespace DSD
{

#ifdef _DEBUG

#ifdef _OMXIL_MICSOURCE_DEBUG_TRACING_ON

#define DEBUG_PRINTF(a) {DSD::DebugPrintf(__LINE__, __FILE__, a);}
#define DEBUG_PRINTF2(a, b) {DSD::DebugPrintf(__LINE__, __FILE__, a, b);}
#define DEBUG_PRINTF3(a, b, c) {DSD::DebugPrintf(__LINE__, __FILE__, a, b, c);}
#define DEBUG_PRINTF4(a, b, c, d) {DSD::DebugPrintf(__LINE__, __FILE__, a, b, c, d);}
#define DEBUG_PRINTF5(a, b, c, d, e) {DSD::DebugPrintf(__LINE__, __FILE__, a, b, c, d, e);}

#define DEBUG_CODE_SECTION(a) TRAP_IGNORE({ a; }) 

class TTruncateOverflowHandler16 : public TDes16Overflow
	{
	public:
		virtual void Overflow( TDes16& aDes );
	};
	
inline void TTruncateOverflowHandler16::Overflow( TDes16& aDes)
	{
	_LIT(KErrOverflowMsg,"Descriptor Overflow, hence value truncated");
	if( aDes.MaxLength() >= KErrOverflowMsg().Length() + aDes.Length() )
     	aDes.Append(KErrOverflowMsg);
	}
	
class TTruncateOverflowHandler8 : public TDes8Overflow
	{
	public:
		virtual void Overflow( TDes8& aDes );
	};
	
inline void TTruncateOverflowHandler8::Overflow( TDes8& aDes)
	{
    _LIT(KErrOverflowMsg,"Descriptor Overflow, hence value truncated");
	if( aDes.MaxLength() >= KErrOverflowMsg().Length() + aDes.Length() )
     	aDes.Append(KErrOverflowMsg);
	}

// UTF-8 overload of the DebufPrintf method. Should be used by default,
// since it's cheaper both in CPU cycles and stack space.

inline void DebugPrintf(TInt aLine, char* aFile, TRefByValue<const TDesC8> aFormat, ...)
	{
	TTruncateOverflowHandler8 overflowHandler8;
	VA_LIST list;
	VA_START(list, aFormat);
	
	TTime now;
	now.HomeTime();
	
	TBuf8<1024> buffer;
	_LIT8(KSwiLogPrefix, "[pcmcapturer] ");
	_LIT8(KSwiLineFileFormat, "TID[%d] : [%s:%d] -- ");
	buffer.Append(KSwiLogPrefix);
	RThread thread;
	TUint threadId = thread.Id();
	thread.Close();
	RProcess proc;
	TFileName fName = proc.FileName();
	proc.Close();
	buffer.AppendFormat(KSwiLineFileFormat, threadId, aFile, aLine);
	buffer.AppendFormatList(aFormat, list ,&overflowHandler8 );
	buffer.Append(_L8("\r\n"));
	
	RDebug::RawPrint(buffer);
	
	VA_END(list);
	}
	
// Unicode DebufPrintf overload

inline void DebugPrintf(TInt aLine, char* aFile, TRefByValue<const TDesC16> aFormat, ...)
	{
	TTruncateOverflowHandler16 overflowHandler16;
	VA_LIST list;
	VA_START(list, aFormat);
	
	TTime now;
	now.HomeTime();
	
	TBuf8<256> header;
	_LIT8(KSwiLogPrefix, "[pcmcapturer] ");
	_LIT8(KSwiLineFileFormat, "%Ld Line: % 5d, File: %s -- ");
	header.Append(KSwiLogPrefix);
	header.AppendFormat(KSwiLineFileFormat, now.Int64(), aLine, aFile);
	
	TBuf<1024> buffer;
	buffer.Copy(header);
	buffer.AppendFormatList(aFormat, list ,&overflowHandler16);
	buffer.Append(_L("\r\n"));
	
	RDebug::RawPrint(buffer);
	
	VA_END(list);
	}
#else

#define DEBUG_PRINTF(a)
#define DEBUG_PRINTF2(a, b)
#define DEBUG_PRINTF3(a, b, c)
#define DEBUG_PRINTF4(a, b, c, d)
#define DEBUG_PRINTF5(a, b, c, d, e)

#define DEBUG_CODE_SECTION(a)

#endif

#else

#define DEBUG_PRINTF(a)
#define DEBUG_PRINTF2(a, b)
#define DEBUG_PRINTF3(a, b, c)
#define DEBUG_PRINTF4(a, b, c, d)
#define DEBUG_PRINTF5(a, b, c, d, e)

#define DEBUG_CODE_SECTION(a)

#endif //_OMXIL_MICSOURCE_DEBUG_TRACING_ON


} // namespace DSD

#endif // __SWI_LOG_H__

