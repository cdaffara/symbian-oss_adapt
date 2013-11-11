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
* AmrWb Decoder Wrapper.The member functions are pure virtual functions which     
* are to be implemented by the derived class.
*
*/

#ifndef ARIAMRWBDECWRAPPER_H	
#define ARIAMRWBDECWRAPPER_H

#include<e32def.h>
#include<e32const.h>
#include<e32base.h>
#include<e32debug.h>


class CAriAmrWbDecWrapper:public CBase
	{

public:
	/**
	 * Two-phased constructor.
	 * @return pointer to an instance of CAriAmrWbDecWrapper
	 */
     IMPORT_C static CAriAmrWbDecWrapper* NewL();
     /**> Destructor */
	 ~CAriAmrWbDecWrapper();
	 
	 /**
	 * The function resets the codec.
	 * @param  
	 * @return return status           
	 */
	 virtual TInt Reset() = 0;

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
	 virtual TInt Decode(TUint8* aSrcBuf,TInt32 &aSrcLen,TInt32* aDstBuf, 
														 TInt32 &aDstLen) = 0;
	};


#endif /* ARIAMRWBDECWRAPPER_H  */
