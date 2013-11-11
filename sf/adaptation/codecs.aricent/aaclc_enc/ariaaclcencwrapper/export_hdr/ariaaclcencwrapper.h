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
* AacLC encoder Wrapper.The member functions are pure virtual functions which
* are to be implemented by the derived class.
*
*/

#ifndef ARIAACLCENCWRAPPER_H
#define ARIAACLCENCWRAPPER_H

#include<e32base.h>

// Class declaration
class CAriAacLCEncWrapper:public CBase
    {

public:
    /**
     * Two-phased constructor.
     * @return pointer to an instance of CAriAacLCEncWrapper
     */
    IMPORT_C static CAriAacLCEncWrapper* NewL();

    /**> Destructor */
    ~CAriAacLCEncWrapper();

	/**
     * The function resets the encoder.
     * @param aParam
     *    pointer to encoding parameter structure
     * @return return status
     */
    virtual TInt Reset( TAny* aParam ) = 0;

    /**
     * The function creates the header for the AAC stream with the
     * parameters specified.
     * @param aDstBuf
     *    The destination buffer to hold the header information
     * @param aDstLen
     *    Length of the header in bytes
     * @return return status
     */
    virtual TInt GetHeader( TUint8* aDstBuf, TInt &aDstLen ) = 0;

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
    virtual TInt Encode( TInt16* aSrcBuf, TInt &aSrcLen,
                                    TUint8* aDstBuf, TInt &aDstLen ) = 0;

    };


#endif /* ARIAACLCENCWRAPPER_H  */
