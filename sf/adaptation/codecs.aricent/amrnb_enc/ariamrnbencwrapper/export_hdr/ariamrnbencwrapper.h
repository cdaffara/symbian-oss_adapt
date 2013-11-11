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
* AmrNb Encoder Wrapper.The member functions are pure virtual functions which
* are to be implemented by the derived class.
*
*/

#ifndef ARIAMRNBENCWRAPPER_H
#define ARIAMRNBENCWRAPPER_H

#include <e32base.h>

// Class declaration
class CAriAmrNbEncWrapper:public CBase
    {
public:
    /**
    * Two-phased constructor.
    * @return pointer to an instance of CAriAmrNbEncWrapper
    */
    IMPORT_C static CAriAmrNbEncWrapper* NewL();

    /**> Destructor */
    virtual ~CAriAmrNbEncWrapper();

    /**
    * The function resets the encoder.
    * @param aParam
    *    pointer to FrameInfo structure
    * @return return status
    */
    virtual TInt Reset( TAny* aParam ) = 0;

    /**
    * The function encodes the input stream.
    * @param aSrcBuf
    *    The source buffer containing data to encode.
    * @param aSrcLen
    *    Size of source buffer in bytes
    * @param aDstBuf
    *    The destination buffer to hold the data after encoding.
    * @param aDstLen
    *    Size of destination buffer in bytes
    * @return return status
    */
    virtual TInt Encode( TInt* aSrcBuf, TInt &aSrcLen, TUint8* aDstBuf,
                                                      TInt &aDstLen ) = 0;

    };


#endif /* ARIAMRNBENCWRAPPER_H  */
