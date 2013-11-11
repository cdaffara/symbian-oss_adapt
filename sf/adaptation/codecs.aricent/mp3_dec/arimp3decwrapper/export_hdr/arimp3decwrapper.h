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
* Mp3 Decoder Wrapper.The member functions are pure virtual functions which     
* are to be implemented by the derived class.
*
*/

#ifndef ARIMP3DECWRAPPER_H	
#define ARIMP3DECWRAPPER_H

// System includes
#include <E32Base.h>

// Class declaration
class TMp3FrameInfo
    {
		
public:  
    TInt iSamplingFrequency;
	TInt iNumberOfChannels;
	TInt iBufferSize;
	TInt iBitRate;
	
    };

// Class declaration
class CAriMp3DecWrapper : public CBase
    {

public:
    /**
     * Two-phased constructor.
     * @return pointer to an instance of CAriMp3DecWrapper
     */
	IMPORT_C static CAriMp3DecWrapper* NewL();
	/**> Destructor */
	virtual ~CAriMp3DecWrapper();
	/**
	 * The function resets the decoder.
	 * @param   
	 * @return return status         
	 */
    virtual TInt Reset() = 0 ;
    /**
	 * The function Gets frame information for the given source.
	 * @param aBuf
	 *    	Pointer to buffer containing the header of input stream data
	 * @param aBufLen
	 * 		Size of Buffer in bytes
	 * @param aFrameLen 
	 * 		Length of the frame
	 * @param aInfo
	 * 		Reference to the structure TMp3FrameInfo 
	 * @return return status         
	 */  
    virtual TInt GetFrameInfo( const TUint8* aBuf, TInt aBufLen,
								TInt &aFrameLen, TMp3FrameInfo& aInfo ) = 0 ;
    /**
	 * The function Gets maximum out put frame length with the information 
	 * @param aFrameLen 
	 * 		Length of the maximum out put frame
	 * @param aInfo
	 * 		Reference to the structure TMp3FrameInfo 
	 * @return return status         
	 */ 
    virtual TInt GetMaxFrameInfo( TInt &aFrameLen, 
									TMp3FrameInfo& aInfo ) = 0 ;
    /**
	 * The function gives the offset value of the buffer from where frame 
	 * starts. 
	 * @param aBuf 
	 * 		Pointer to the input buffer 
	 * @param aBufLen
	 * 		Length of the input buffer
	 * @param aSyncPos
	 * 		Offset value of the buffer from where frame starts	
	 * @return return status         
	 */ 
    virtual TInt SeekSync( const TUint8* aBuf, TInt aBufLen, 
														TInt &aSyncPos ) = 0 ;
    /**
	 * The function decodes the aSrc buffer. 
	 * @param aSrc 
	 * 		Pointer to the source buffer 
	 * @param aSrcUsed
	 * 		Size of the source buffer
	 * @param aDst
	 * 		Pointer to the destination buffer
	 * @param aDstLen
	 * 		Size of the destination buffer	
	 * @return return status         
	 */ 
    virtual TInt Decode( TUint8* aSrc, TInt& aSrcUsed, TUint8* aDst, 
														TInt& aDstLen ) = 0 ;    
   
    };

#endif //ARIMP3DECWRAPPER_H

//-----------------------------------------------------------------------------
//  End of File
//-----------------------------------------------------------------------------
