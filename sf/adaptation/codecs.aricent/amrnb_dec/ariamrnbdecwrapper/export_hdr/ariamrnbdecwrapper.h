/*
* Copyright (c) 2009 Aricent and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Aricent - initial contribution.
*
* Contributors:
*
* Description: 
* Export header file for Wrapper APIs.Interface class for 
* AmrNb Decoder Wrapper.The member functions are pure virtual functions which     
* are to be implemented by the derived class.
*
*/

#ifndef ARIAMRNBDECWRAPPER_H 
#define ARIAMRNBDECWRAPPER_H

// system include files
#include<e32def.h>
#include<e32const.h>
#include<e32base.h>
#include<e32debug.h>

// Class declaration
class TDecParam
	{
public:
	//mediatype takes 0 for RTP and 1 for 3GPP
	TInt iMediatype;	
	};
	
// Class declaration
class CAriAmrNbDecWrapper:public CBase
	{
public:
	/**
	 * Two-phased constructor.
	 * @return pointer to an instance of CAriAmrNbDecWrapper
	 */
	IMPORT_C static CAriAmrNbDecWrapper* NewL();
	
	/**> Destructor */
	~CAriAmrNbDecWrapper();
	
	/**
	 * The function resets the codec.
	 * @param aParam 
	 *    TDecParam in which it takes 0 for RTP and 1 for RTP
	 * @return return status           
	 */
	virtual TInt Reset( TDecParam aParam ) = 0;
	
	/**
	 * The function decodes the input stream.
	 * @param aSrcBuf
	 *    The source buffer containing data to decode.
	 * @param aSrcLen
	 *    Size of source buffer in bytes
	 * @param aDstBuf
	 *    The destination buffer to hold the data after decoding.  
	 * @param aDstLen
	 *    Size of destination buffer in bytes
	 * @return return status           
	 */
	virtual TInt Decode( TUint8* aSrcBuf, TInt &aSrcLen, TInt* aDstBuf, 
														TInt &aDstLen ) = 0;	
	};


#endif /* ARIAMRNBDECWRAPPER_H  */
