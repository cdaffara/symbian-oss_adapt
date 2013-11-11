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
* HeAac Decoder Wrapper.The member functions are pure virtual functions which
* are to be implemented by the derived class.
*
*/

#ifndef ARIHEAACDECWRAPPER_H
#define ARIHEAACDECWRAPPER_H

#include <e32def.h>
#include <e32base.h>


// Class declaration
class CAriHeAacDecWrapper:public CBase
    {

public:
    /**
     * Two-phased constructor.
     * @return pointer to an instance of CAriHeAacDecWrapper
     */
    IMPORT_C static CAriHeAacDecWrapper* NewL();

    /**> Destructor */
	virtual ~CAriHeAacDecWrapper();

	/**
     * The function returns the Format type of the input stream.
     * @param aData
     *    Pointer to buffer containing the first chunk of input stream data
     * @return Format type of input stream
     */
	virtual TInt FormatType( TUint8* aData ) = 0;

	/**
     * The function Gets frame information for the given source.
     * @param aHeaderBuffer
     *    Pointer to buffer containing the header of input stream data
     * @param aHeaderSize
     *    size of header in bytes
     * @param aFrameInfo
     *    pointer to FrameInfo structure
     * @param aFormat
     *    Format type of input stream
     * @param aDataBuffer
     *    Pointer to buffer containing input stream data
     * @param aDataSize
     *    size of input data in bytes
     * @return return status
     */
	virtual TInt GetFrameInfo( TUint8* aHeaderBuffer, TInt &aHeaderSize,
                            TAny* aFrameInfo, TInt aFormat,
                            TUint8* aDataBuffer, TInt aDataSize ) = 0;

	/**
     * The function decodes the input stream.
     * @param aSrcBuf
     *    The source buffer containing data to decode.
     * @param aSrcUsed
     *    Size of source buffer in bytes
     * @param aDstBuf
     *    The destination buffer to hold the data after decoding.
     * @param aOutSize
     *    Size of destination buffer in bytes
     * @return return status
     */
 	virtual TInt Decode( TUint8 *aSrcBuf, TInt &aSrcUsed, TUint8 *aDstBuf,
                                                TInt &aOutSize ) = 0;

 	/**
     * The function resets the codec.
     * @param aFrameInfo
     *    pointer to FrameInfo structure
     * @return return status
     */
	virtual TInt Reset( TAny* aFrameInfo ) = 0;
    };


#endif /*   ARIHEAACDECWRAPPER_H   */

