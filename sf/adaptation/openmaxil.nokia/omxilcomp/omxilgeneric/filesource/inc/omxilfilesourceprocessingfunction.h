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
 * @file
 * @internalTechnology
 */

#ifndef OMXILFILESOURCEPROCESSINGFUNCTION_H
#define OMXILFILESOURCEPROCESSINGFUNCTION_H

#include <openmax/il/common/omxilprocessingfunction.h>
#include <e32base.h>
#include <e32msgqueue.h>
#include <f32file.h>
#include <openmax/il/khronos/v1_x/OMX_Component.h>

class COmxILFileSourceProcessingFunction : public COmxILProcessingFunction
	{
public:
	static COmxILFileSourceProcessingFunction* NewL(MOmxILCallbackNotificationIf& aCallbacks);
	~COmxILFileSourceProcessingFunction();

	OMX_ERRORTYPE StateTransitionIndication(TStateIndex aNewState);
	OMX_ERRORTYPE BufferFlushingIndication(TUint32 aPortIndex, OMX_DIRTYPE aDirection);
	OMX_ERRORTYPE ParamIndication(OMX_INDEXTYPE aParamIndex, const TAny* apComponentParameterStructure);
	OMX_ERRORTYPE ConfigIndication(OMX_INDEXTYPE aConfigIndex, const TAny* apComponentConfigStructure);
	OMX_ERRORTYPE BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE aDirection);
	OMX_BOOL BufferRemovalIndication(OMX_BUFFERHEADERTYPE* apBufferHeader, OMX_DIRTYPE aDirection);
	
	MOmxILCallbackNotificationIf& GetCallbacks();

	/** can return NULL if parameter has not been set. */
	const HBufC* FileName() const;
	const HBufC8* Uri() const;

private:
	COmxILFileSourceProcessingFunction(MOmxILCallbackNotificationIf& aCallbacks);
	void ConstructL();
	
	void InitFileAndUriL();
	OMX_ERRORTYPE SetFileName(const OMX_PARAM_CONTENTURITYPE* aContentUriType);
			
private:
	RPointerArray<OMX_BUFFERHEADERTYPE> iBuffersToFill;
	OMX_STATETYPE iState;
	HBufC8* ipUri;
	HBufC* ipFileName;

private:
	class CPFHelper;
	class CBufferArrivalHandler;
	
	class CFileSourceAccess : public CActive
		{
	friend class CPFHelper;
	public:
		static CFileSourceAccess* NewL(COmxILFileSourceProcessingFunction& aParent);
		~CFileSourceAccess();

		// from CActive
		void RunL();
		void DoCancel();

		TInt Execute();
		void Pause();
		void Stop();
		void Idle();
		
		void CancelDataTransfer();
		
		TInt ProcessNextBuffer();

	private:
		CFileSourceAccess(COmxILFileSourceProcessingFunction& aParent);
		void ConstructL();
		
	private:
		COmxILFileSourceProcessingFunction& iParent;
		OMX_BUFFERHEADERTYPE* iCurrentBuffer;
		
		RFs iFs;
		RFile iFileHandle;
		TInt iBufferOffset;
		TPtr8 iReadBuffer;
		TBool iEOSFlag; // Notifies that end of file has been reached.
		TInt iEOSBufferNotificationCount; // counts the number of responses to buffer notification when URI is unspecified
		
		} *ipFileSourceAccess;

	class CPFHelper : public CActive
		{
	public:
		static CPFHelper* NewL(COmxILFileSourceProcessingFunction& aParent, CFileSourceAccess& aFileSourceAccess, CBufferArrivalHandler& aBufferArrivalHandler);
		~CPFHelper();
		
		static OMX_ERRORTYPE ConvertSymbianErrorType(TInt aError);

		// from CActive
		void RunL();
		void DoCancel();

		TInt ExecuteAsync();
		TInt StopAsync();
		TInt PauseAsync();
		void StopSync();
		TInt IdleAsync();
		TInt FlushIndication();
		OMX_BOOL RemoveBufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader);

		enum TMessageType 
			{
			EExecuteCommand,
			EStopCommand,
			EPauseCommand,
			ECancelCommand,
			EIdleCommand,
			EFlushCommand,
			ERemoveBufferCommand
			};

		class TProcMessage
			{
		public:
			TMessageType iType;
			TAny* iPtr;
			};
		
		class TBufferRemovalArgs
		    {
		public:
		    OMX_BUFFERHEADERTYPE* ipHeader; // Input parameter. Not owned
		    OMX_BOOL* iHdrRemoved; // Return parameter. Not owned
		    };
				
		RMsgQueue<TProcMessage> iMsgQueue;
	
	private:
		CPFHelper(COmxILFileSourceProcessingFunction& aParent, CFileSourceAccess& aFileSourceAccess,CBufferArrivalHandler& aBufferArrivalHandler);
		void ConstructL();
		
		TInt ProcessQueue();
		void PerformFlush();
		OMX_BOOL PerformBufferRemoval(OMX_BUFFERHEADERTYPE* apBufferHeader);

	private:
		COmxILFileSourceProcessingFunction& iParent;
		CFileSourceAccess& iFileSourceAccess;
		CBufferArrivalHandler& iBufferArrivalHandler;
	    RThread iRunLContext;
	    RSemaphore iWaitSemaphore;
		} *ipPFHelper;
		
    class CBufferArrivalHandler : public CActive
        {
    public:
        static CBufferArrivalHandler* NewL(COmxILFileSourceProcessingFunction& aParent, CFileSourceAccess& aFileSourceAccess);
        ~CBufferArrivalHandler();        
        
        TInt BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader);
        void FlushIndication();
        OMX_BOOL RemoveBufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader);
    
        // CActive
        void RunL();
        void DoCancel();
        
    private:
        CBufferArrivalHandler(COmxILFileSourceProcessingFunction& aParent, CFileSourceAccess& aFileSourceAccess);
        void ConstructL();
        
    private:    
		class TBufferIndicationMessage
            {
        public:
            OMX_BUFFERHEADERTYPE* iBufferHdr; // Not owned
            };
		
        RMsgQueue<TBufferIndicationMessage> iMsgQueue; 
        COmxILFileSourceProcessingFunction& iParent;
        CFileSourceAccess& iFileSourceAccess;
        } *ipBufferArrivalHandler;
	};

#endif // OMXILFILESOURCEPROCESSINGFUNCTION_H

