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

#ifndef OMXILFILESINKPROCESSINGFUNCTION_H
#define OMXILFILESINKPROCESSINGFUNCTION_H

#include <openmax/il/common/omxilprocessingfunction.h>
#include <openmax/il/khronos/v1_x/OMX_Component.h>
#include <e32msgqueue.h>
#include <f32file.h>

class COmxILFileSinkProcessingFunction : public COmxILProcessingFunction
	{
public:
	static COmxILFileSinkProcessingFunction* NewL(MOmxILCallbackNotificationIf& aCallbacks);
	~COmxILFileSinkProcessingFunction();

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
	COmxILFileSinkProcessingFunction(MOmxILCallbackNotificationIf& aCallbacks);
	void ConstructL();

	void InitFileAndUriL();
	OMX_ERRORTYPE SetFileName(const OMX_PARAM_CONTENTURITYPE* aContentUriType);
			
private:
	RPointerArray<OMX_BUFFERHEADERTYPE> iBuffersToEmpty;
	OMX_STATETYPE iState;
	HBufC8* ipUri;
	HBufC* ipFileName;

private:
	class CPFHelper;
	class CFileSinkAccess : public CActive
		{
	friend class CPFHelper;
	public:
		static CFileSinkAccess* NewL(COmxILFileSinkProcessingFunction& aParent);
		~CFileSinkAccess();

		// from CActive
		void RunL();
		void DoCancel();

		TInt Execute();
		void Pause();
		void Stop();
		void Idle();
		
		TInt ProcessNextBuffer();

	private:
		CFileSinkAccess(COmxILFileSinkProcessingFunction& aParent);
		void ConstructL();
		
	private:
		COmxILFileSinkProcessingFunction& iParent;
		OMX_BUFFERHEADERTYPE* iCurrentBuffer;
		
		RFs iFs;
		RFile iFileHandle;
		TInt iBufferOffset;
		TPtr8 iWriteBuffer;
		
		} *ipFileSinkAccess;

	class CPFHelper : public CActive
		{
	public:
		static CPFHelper* NewL(COmxILFileSinkProcessingFunction& aParent, CFileSinkAccess& aFileSinkAccess);
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
		TInt BufferIndication(OMX_BUFFERHEADERTYPE* apBufferHeader);

		enum TMessageType 
			{
			EExecuteCommand,
			EStopCommand,
			EPauseCommand,
			ECancelCommand,
			EBufferIndication,
			EIdleCommand
			};

		class TProcMessage
			{
		public:
			TMessageType iType;
			TAny* iPtr;
			};
			
		RMsgQueue<TProcMessage> iMsgQueue;
	
	private:
		CPFHelper(COmxILFileSinkProcessingFunction& aParent, CFileSinkAccess& aFileSinkAccess);
		void ConstructL();
		
		TInt ProcessQueue();

	private:
		COmxILFileSinkProcessingFunction& iParent;
		CFileSinkAccess& iFileSinkAccess;
		} *ipPFHelper;			
	};

#endif // OMXILFILESINKPROCESSINGFUNCTION_H

