/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  
* \hwip_nec_naviengine\naviengine_assp\lcdgce\lcdgce.cpp
* Implementation of an LCD driver with GCE support.
* This file is part of the NE1_TBVariant Base port
* N.B. This sample code assumes that the display supports setting the backlight on or off,
* as well as adjusting the contrast and the brightness.
*
*/



/**
 @file Implementation of an LCD driver with GCE support.
 @internalTechnology
 @prototype
*/

#include <videodriver.h>
#include "platform.h"
#include <nkern.h>
#include <kernel/kernel.h>
#include <kernel/kern_priv.h>
#include <kernel/kpower.h>
#include <naviengine_priv.h>
#include "lcdgce.h"
#include <naviengine_lcd.h>
#include "powerresources.h"
#include <resourcecontrol.h>

// define the characteristics of the LCD display
// This is only example code... you need to modify it for your hardware
const TBool	KConfigLcdIsMono				= EFalse;
const TBool	KConfigLcdPixelOrderLandscape	= ETrue;
const TBool	KConfigLcdPixelOrderRGB			= ETrue;
const TInt	KConfigLcdMaxDisplayColors		= 65536;
const TUint32 KShiftBitsPerByte				= 3;
const TInt	KConfigBitsPerPixel16			= 16;
const TInt	KConfigBitsPerPixel32			= 32;

#define RESOLUTION_AND_CYCLE		DISPLAY_RESOLUTION_AND_CYCLE(Lcd_Mode_Config[iInitialMode].iConfigLcdHeight, Lcd_Mode_Config[iInitialMode].iConfigLcdWidth, Lcd_Mode_Config[iInitialMode].iLinesInFrame, Lcd_Mode_Config[iInitialMode].iPixelsInLine)

#define SCREEN_UNIT					0
#define SCREEN_UNIT_COUNT			1

struct SLcdConfig
	{
	TInt    iOffsetToFirstVideoBuffer;
	TInt    iConfigLcdWidth;    // The width of the physical display
	TInt    iConfigLcdHeight;   // The height of the physical display
	TBool   iIsPalettized;
	TInt    iBitsPerPixel;
	TUint64 iPulseWidth;
	TUint   iLinesInFrame;      // This appears to be a magic number that has no resemblence to the height
	TUint   iPixelsInLine;      // This appears to be a magic number that has no resemblence to the width
	TInt    iReportedLcdWidth;  // The width reported to the higher levels of software
	TInt    iReportedLcdHeight; // The height reported to the higher levels of software
	};

static const SLcdConfig Lcd_Mode_Config[]=
	{
	// 0: DISPLAY_MODE_ANALOG_VGA: No LCD, Landscape VGA 640x480
		{ 0, 640, 480, EFalse, KConfigBitsPerPixel32, KPulseWidthVga,    525, 800,  640, 480, },
	// 1: DISPLAY_MODE_HITACHI_VGA: Hitachi LCD, Portrait VGA 480x640 (for this mode, SW4-1 should be turned OFF on the LCD panel)
		{ 0, 480, 640, EFalse, KConfigBitsPerPixel32, KPulseWidthVgaLCD, 650, 650,  480, 640, },
	// 2: DISPLAY_MODE_ANALOG_QVGA_LANDSCAPE_OLD: No LCD, Landscape QVGA 320x240
		{ 0, 320, 240, EFalse, KConfigBitsPerPixel32, KPulseWidthVga,    525, 800,  320, 240, },
	// 3: DISPLAY_MODE_HITACHI_QVGA: Hitachi LCD, Portrait QVGA 240x320
		{ 0, 240, 320, EFalse, KConfigBitsPerPixel32, KPulseWidth,       525, 800,  240, 320, },
	// 4: DISPLAY_MODE_NEC_WVGA: NEC LCD, Landscape WVGA
		{ 0, 800, 480, EFalse, KConfigBitsPerPixel32, KPulseWidthWvga,   525, 1024, 800, 480, },
	// 5: DISPLAY_MODE_ANALOG_QVGA_PORTRAIT: No LCD, Portrait QVGA 240x320
	// Note! this screen mode reports different dimensions than the real panel size
		{ 0, 640, 480, EFalse, KConfigBitsPerPixel16, KPulseWidthVga,    525, 800,  240, 320, },
	// 6: DISPLAY_MODE_ANALOG_QVGA_LANDSCAPE: No LCD, Landscape QVGA 320x240
		{ 0, 640, 480, EFalse, KConfigBitsPerPixel16, KPulseWidthVga,    525, 800,  320, 240, },
	};

// Hack function to convert pixels to TWIPS
// Twips are screen-independent units to ensure that the proportion of screen elements
// are the same on all display systems. A twip is defined as being 1/20th of a point.
inline TInt PixelsToTwips(TInt aPixels)
	{
	return (119*aPixels)/10;
	}

DLcdPowerHandler	*  DLcdPowerHandler::pLcd			= NULL;


const TInt   KDisplay0ThreadPriority					= 26;

const TInt   KVSyncDfcPriority							= 7 ;   //priority of DFC within the queue (0 to 7, where 7 is highest)


_LIT(KLitLcd,"LCD");
_LIT(KDisplay0DfcThread,"Display0DfcThread-");

TInt vsync_irq_callback(TUint a, TAny* b);

#ifdef __SMP__
static TSpinLock callbackLock = TSpinLock(TSpinLock::EOrderGenericIrqHigh0);
#endif


/**********************************************
*    Class DDisplayPddNaviEng
***********************************************/

void DDisplayPddNaviEng::VSyncDfcFn(TAny* aChannel)
	{
	DDisplayPddNaviEng * channel =(DDisplayPddNaviEng*)aChannel;

	if (channel->iPostFlag)
		{
		 channel->iPostFlag = EFalse;

					
		if (channel->iActiveBuffer)
			{					 		 	 		 		
	 		//When a User buffer is registered its iFree member becomes EFalse and Deregister sets it 
	 		//back to ETrue. Composition and Legacy buffers are not free when they are in the pending or 
	 		//active state. 		
			if (channel->iActiveBuffer->iType == EBufferTypeUser)
				{
				//If a subsequent PostUserRequest has occured,so there is a pending user buffer which has 
				//cancelled the previous post( the one that  queued the current active buffer), calling 
				//RequestComplete would mistakenly complete the latest PostUserRequest. 
				
				if (!(channel->iPendingBuffer && channel->iPendingBuffer->iType == EBufferTypeUser) )
					{	
					channel->RequestComplete(RDisplayChannel::EReqPostUserBuffer, KErrNone);	
					}
				
				}
			else
				{
				channel->iActiveBuffer->iFree	= ETrue;
				}
			
			channel->iActiveBuffer->iState		= EBufferFree;
							
			
			if (channel->iActiveBuffer->iType == EBufferTypeComposition)
				{
				//If no buffer was available during a call to GetCompositionBuffer the active buffer has  
				//been returned as the next available one, so we must set the buffer to the proper state before we 
				//send the notification.
				TInt pendingIndex = channel->iLdd->iPendingIndex[RDisplayChannel::EReqGetCompositionBuffer];
				if( channel->iLdd->iPendingReq[RDisplayChannel::EReqGetCompositionBuffer][pendingIndex].iTClientReq)
					{
					if(channel->iLdd->iPendingReq[RDisplayChannel::EReqGetCompositionBuffer][pendingIndex].iTClientReq->IsReady() )
						{
						channel->iActiveBuffer->iState	= EBufferCompose;
						channel->RequestComplete(RDisplayChannel::EReqGetCompositionBuffer, KErrNone);
						}
					}			
				}
													              
			channel->iActiveBuffer				= NULL;            							
			}					
		
		if (channel->iPendingBuffer)
			{
			__GCE_DEBUG_PRINT2("DDisplayPddNaviEng::VSyncDfcFn moving pending buffer at address %08x to the active state\n", channel->iPendingBuffer->iAddress);		
			channel->iActiveBuffer			= channel->iPendingBuffer;
			channel->iActiveBuffer->iState	= EBufferActive;
			channel->iPendingBuffer			= NULL; 

			channel->RequestComplete(RDisplayChannel::EReqWaitForPost,  KErrNone);
			}
		}
		
	}



/**
	Constructor
*/
DDisplayPddNaviEng::DDisplayPddNaviEng():
	iPendingBuffer(NULL),
	iActiveBuffer(NULL),
	iChunk(NULL),
	iLcdCallback(NULL),
	iVSyncDfc(&VSyncDfcFn, this, KVSyncDfcPriority)
	{
	__GCE_DEBUG_PRINT("DDisplayPddNaviEng::DDisplayPddNaviEng\n");

	iPostFlag = EFalse;
	}

/**
	Destructor
*/
DDisplayPddNaviEng::~DDisplayPddNaviEng()
	{
	__GCE_DEBUG_PRINT("DDisplayPddNaviEng::~DDisplayPddNaviEng()  \n");

	if(iLcdCallback)
		{
		DLcdPowerHandler::pLcd->DeRegisterCallback(iLcdCallback) ;
		delete iLcdCallback;
		iLcdCallback = NULL;
		}

	//The DFC Queue is owned by DLcdPowerHandler so we shouldn't call Destroy() at this point.
	if (iDfcQ)
		{
		iDfcQ=NULL;
		}

	DChunk* chunk = (DChunk*) __e32_atomic_swp_ord_ptr(&iChunk, 0);

	if(chunk)
		{
		Kern::ChunkClose(chunk);
		}

	}


/**
    Called by the LDD's DoCreate function to handle the device specific part of opening the channel.
    (DoCreate is called by RDisplayChannel::Open)

	@param aUnit	The screen unit

    @return KErrNone if successful; or one of the other system wide error codes.
*/
TInt DDisplayPddNaviEng::CreateChannelSetup(TInt aUnit)
	{
	__GCE_DEBUG_PRINT("DDisplayPddNaviEng::CreateChannelSetup\n");

	iScreenInfo = DLcdPowerHandler::pLcd->iVideoInfo;
	iLdd->iUnit = aUnit;

	iLdd->iDisplayInfo.iAvailableRotations			= RDisplayChannel::ERotationNormal;
	iLdd->iDisplayInfo.iNormal.iOffsetBetweenLines	= iScreenInfo.iOffsetBetweenLines;
	iLdd->iDisplayInfo.iNormal.iHeight				= iScreenInfo.iSizeInPixels.iHeight;
	iLdd->iDisplayInfo.iNormal.iWidth				= iScreenInfo.iSizeInPixels.iWidth;
	iLdd->iDisplayInfo.iNumCompositionBuffers		= KDisplayCBMax;
	iLdd->iDisplayInfo.iBitsPerPixel				= iScreenInfo.iBitsPerPixel;
    iLdd->iDisplayInfo.iRefreshRateHz = 60;


	switch (iScreenInfo.iBitsPerPixel)
		{
		case 16:
			iLdd->iDisplayInfo.iPixelFormat = EUidPixelFormatRGB_565;
			break;
		case 24:
			iLdd->iDisplayInfo.iPixelFormat = EUidPixelFormatRGB_888;
			break;
		case 32:
			iLdd->iDisplayInfo.iPixelFormat = EUidPixelFormatXRGB_8888;
			break;
		default:
			iLdd->iDisplayInfo.iPixelFormat = EUidPixelFormatUnknown;
			break;
		}

	iLdd->iCurrentRotation = RDisplayChannel::ERotationNormal;

	// Open shared chunk to the composition framebuffer

	DChunk* chunk = 0;
	TLinAddr chunkKernelAddr  = 0;
	TUint32 chunkMapAttr = 0;

	// round to twice the page size
	TUint round  =  Kern::RoundToPageSize(2*DLcdPowerHandler::pLcd->iSize);

	__GCE_DEBUG_PRINT2("DDisplayPddNaviEng::CreateChannelSetup DLcdPowerHandler::pLcd->iSize  = %d\n", DLcdPowerHandler::pLcd->iSize );

	TChunkCreateInfo info;
	info.iType					 = TChunkCreateInfo::ESharedKernelMultiple;
	info.iMaxSize				 = round;
	info.iMapAttr				 = EMapAttrFullyBlocking;
	info.iOwnsMemory			 = EFalse;
	info.iDestroyedDfc			 = 0;

	TInt r = Kern::ChunkCreate(info, chunk, chunkKernelAddr, chunkMapAttr);

	__GCE_DEBUG_PRINT2("CreateChannelSetup:ChunkCreate called for composition chunk. Set iChunkKernelAddr  = %08x\n", chunkKernelAddr );

	if( r == KErrNone)
		{
		// map our chunk
		r = Kern::ChunkCommitPhysical(chunk, 0,round , DLcdPowerHandler::pLcd->iCompositionPhysical);
		if(r != KErrNone)
			{
			Kern::ChunkClose(chunk);
			}
		}

	if ( r!= KErrNone)
		{
		return r;
		}

	iChunk	= chunk;

	// init CB 0
	iLdd->iCompositionBuffer[0].iType			= EBufferTypeComposition;
	iLdd->iCompositionBuffer[0].iBufferId		= 0;
	iLdd->iCompositionBuffer[0].iFree			= ETrue;
	iLdd->iCompositionBuffer[0].iState			= EBufferFree;
	iLdd->iCompositionBuffer[0].iAddress		= chunkKernelAddr;
	iLdd->iCompositionBuffer[0].iChunk			= chunk;
	iLdd->iCompositionBuffer[0].iHandle			= 0;
	iLdd->iCompositionBuffer[0].iOffset			= 0;
	iLdd->iCompositionBuffer[0].iSize			= DLcdPowerHandler::pLcd->iSize;
	iLdd->iCompositionBuffer[0].iPendingRequest = 0;

	// init CB 1
	iLdd->iCompositionBuffer[1].iType			= EBufferTypeComposition;
	iLdd->iCompositionBuffer[1].iBufferId		= 1;
	iLdd->iCompositionBuffer[1].iFree			= ETrue;
	iLdd->iCompositionBuffer[1].iState			= EBufferFree;
	iLdd->iCompositionBuffer[1].iAddress		= chunkKernelAddr + DLcdPowerHandler::pLcd->iSize;
	iLdd->iCompositionBuffer[1].iChunk			= chunk;
	iLdd->iCompositionBuffer[1].iHandle			= 0;
	iLdd->iCompositionBuffer[1].iOffset			= DLcdPowerHandler::pLcd->iSize;
	iLdd->iCompositionBuffer[1].iSize			= DLcdPowerHandler::pLcd->iSize;
	iLdd->iCompositionBuffer[1].iPendingRequest = 0;

	iLdd->iCompositionBuffIdx					= 0;

	TUint64 reg64 = AsspRegister::Read64(KHwDisplayMemoryFrameAddress);

	//set up Start address for frames 0 and 1 (The actual Physical addresses must be passed)
	AsspRegister::Write64(KHwDisplayMemoryFrameAddress , ( (TUint64)DLcdPowerHandler::pLcd->iCompositionPhysical<<32  | reg64 ) );

	//set up layer 2
    TUint64 layer2Physical =  DLcdPowerHandler::pLcd->iCompositionPhysical+ DLcdPowerHandler::pLcd->iSize ;
    AsspRegister::Write64(KHwDisplayMemoryFrameAddress + KHex8,layer2Physical ) ;

	//Use the same DFC queue created by the DLcdPowerHandler so all hardware accesses are executed under the same DFC thread.
	iDfcQ= DLcdPowerHandler::pLcd->iDfcQ;

	// Set the Post DFC.
	iVSyncDfc.SetDfcQ(iDfcQ);

	//Register callback function. The function will be called when V Sync is triggered
	iLcdCallback = new TLcdUserCallBack(vsync_irq_callback, (TAny*)this);

	r = KErrNoMemory;
	if (iLcdCallback)
		{
		r = DLcdPowerHandler::pLcd->RegisterCallback(iLcdCallback) ;
		}

	if( r!= KErrNone)
		{
		delete iLcdCallback ;
		iLcdCallback = NULL ;
		return r;
		}

	return KErrNone;
	}


/**
	Called by the Vsync callback method and queues the corresponding DFC.
 */
void DDisplayPddNaviEng::VSyncIsr()
	{
	iVSyncDfc.Add();
	}


/**
	Return the DFC queue to be used for this device.
 */
TDfcQue * DDisplayPddNaviEng:: DfcQ(TInt aUnit)
	{
	return iDfcQ;
	}


/**
    Handles device specific operations when a close message has been sent to the Logical Channel.

*/
TInt DDisplayPddNaviEng::CloseMsg()
	{
	__GCE_DEBUG_PRINT("DDisplayPddNaviEng::CloseMsg()\n");

	iPendingBuffer  = NULL;
	iActiveBuffer	= NULL;

	iVSyncDfc.Cancel();
    return KErrNone;
	}


/**
    Set the GCE mode by posting a composition buffer.

*/
TInt DDisplayPddNaviEng::SetGceMode()
	{
	__GCE_DEBUG_PRINT("DDisplayPddNaviEng::SetGceMode()\n");

    PostCompositionBuffer(&iLdd->iCompositionBuffer[0]);
    return KErrNone;
	}


/**
    Set the Legacy Mode by setting the appropriate Frame control value.

*/
TInt DDisplayPddNaviEng::SetLegacyMode()
	{
	__GCE_DEBUG_PRINT("DDisplayPddNaviEng::SetLegacyMode()\n");

	//Set the default frame 0 which corresponds to the Legacy Buffer.
	AsspRegister::Write64(KHwDisplayDisplayFrameControl, KDisplayFrameControlValue);
    return KErrNone;
	}


/**
	If the specified rotation is supported set it as the current rotation.  The NaviEngine
	version supports only the RDisplayChannel::ERotationNormal rotation.

	@param  aDegOfRot The requested rotation to be set.

	@return KErrNone if the rotation is supported else KErrArgument.
*/
TInt DDisplayPddNaviEng::SetRotation(RDisplayChannel::TDisplayRotation aDegOfRot)
	{
	TInt r;

	switch (aDegOfRot)
		{
		case RDisplayChannel::ERotationNormal:
			r = KErrNone;
			break;
		default:
			r = KErrArgument;
		}

	return r;
	}


/**
    Remove any previous post operations, set the appropriate layer as the next layer to be displayed( This value is updated in synchronization
    with V Sync so it will take affect in the next V Sync after that) and also set the buffer provided as the buffer to
    be posted next. Layer 3 is associated with user buffers.

	@param	aNode  Pointer to the User buffer to post.
*/
TInt DDisplayPddNaviEng::PostUserBuffer(TBufferNode* aNode)
	{

	__GCE_DEBUG_PRINT2("PostUserBuffer :  aNode->iAddress = %08x\n", aNode->iAddress);

	if(iPendingBuffer)
		{
		iPendingBuffer->iState = EBufferFree;
		if (!(iPendingBuffer->iType == EBufferTypeUser) )
			{
			iPendingBuffer->iFree  = ETrue;
			}
		}

	//Set the Physical address for layer3 and then set layer3 as the frame to be displayed.
	AsspRegister::Write64(KHwDisplayMemoryFrameAddress + KHex8 ,(TUint64)aNode->iPhysicalAddress<<32 );
	AsspRegister::Write64(KHwDisplayDisplayFrameControl, KDisplayFrame3ControlValue);

	aNode->iState   = EBufferPending;
	iPendingBuffer	= aNode;
	iPostFlag		= ETrue;

	TUint64 reg64 = AsspRegister::Read64(KHwDisplayInterruptEnable);
	reg64 |= KVSyncEnable;
	AsspRegister::Write64(KHwDisplayInterruptEnable , reg64 ); //Enable Vsync

	return KErrNone;
	}


/**
    Remove any previous post operations, set the appropriate layer as the next layer to be displayed( This value is updated in synchronization
    with V Sync so it will take affect in the next V Sync after that) and also set the buffer provided as the buffer to
    be posted next. Layer 1 and 2 are associated with composition buffers 0 and 1 respectively.

	@param	aNode  Pointer to the Composition buffer to post.
*/
TInt DDisplayPddNaviEng::PostCompositionBuffer(TBufferNode* aNode)
	{

	__GCE_DEBUG_PRINT2("PostCompositionBuffer :  aNode->iAddress = %08x\n", aNode->iAddress);

	if(iPendingBuffer)
		{
		iPendingBuffer->iState = EBufferFree;
		if (iPendingBuffer->iType == EBufferTypeUser)
			{
			RequestComplete(RDisplayChannel::EReqPostUserBuffer, KErrCancel);
			}
		else
			{
			iPendingBuffer->iFree  = ETrue;
			}
		}

	if ( aNode->iBufferId == 0)
		{
		// Display frame control registers	(Layer 1)
		AsspRegister::Write64(KHwDisplayDisplayFrameControl , KDisplayFrame1ControlValue );
		}
	else if	( aNode->iBufferId == 1)
		{
		// Display frame control registers	(Layer 2)
		AsspRegister::Write64(KHwDisplayDisplayFrameControl , KDisplayFrame2ControlValue);

		//Reset Layer2 ( The value might have been overwriten by a PostUserBuffer operation).
		AsspRegister::Write64(KHwDisplayMemoryFrameAddress + KHex8, DLcdPowerHandler::pLcd->iCompositionPhysical + DLcdPowerHandler::pLcd->iSize);
		}

	aNode->iState	= EBufferPending;
	aNode->iFree	= EFalse;
	iPendingBuffer	= aNode;
	iPostFlag		= ETrue;

	TUint64 reg64 = AsspRegister::Read64(KHwDisplayInterruptEnable);
	reg64 |= KVSyncEnable;
	AsspRegister::Write64(KHwDisplayInterruptEnable , reg64 ); //Enable Vsync

	return KErrNone;
	}


/**
    Remove any previous post operations, set the appropriate layer as the next layer to be displayed( This value is updated in synchronization
    with V Sync so it will take affect in the next V Sync after that) and also set the Legacy Buffer as the buffer to
    be posted next.Layer 0 is associated with legacy buffer.

	@param	aNode  Pointer to the Composition buffer to post.
*/
TInt DDisplayPddNaviEng::PostLegacyBuffer()
	{
	__GCE_DEBUG_PRINT("PostLegacyBuffer() \n");

	if(iPendingBuffer)
		{
		iPendingBuffer->iState = EBufferFree;
		if (iPendingBuffer->iType == EBufferTypeUser)
			{

			RequestComplete(RDisplayChannel::EReqPostUserBuffer, KErrCancel);
			}
		else
			{
			iPendingBuffer->iFree  = ETrue;
			}
		}


	AsspRegister::Write64(KHwDisplayDisplayFrameControl, KDisplayFrameControlValue);

	iLdd->iLegacyBuffer[0].iState		= EBufferPending;
	iLdd->iLegacyBuffer[0].iFree		= EFalse;
	iPendingBuffer						= &iLdd->iLegacyBuffer[0];
	iPostFlag		= ETrue;

	TUint64 reg64 = AsspRegister::Read64(KHwDisplayInterruptEnable);
	reg64 |= KVSyncEnable;
	AsspRegister::Write64(KHwDisplayInterruptEnable , reg64 ); //Enable Vsync

	return KErrNone;
	}

/**
Detect whether a post operation is pending
*/
TBool DDisplayPddNaviEng::PostPending()
	{
	return (iPendingBuffer != NULL);
	}

/**
VSync Callback function
 */
TInt vsync_irq_callback(TUint a, TAny* ch)
	{
	// get channel
	if(ch)
		{
		DDisplayPddNaviEng * channel=(DDisplayPddNaviEng*)ch;
		channel->VSyncIsr();
		}
	return KErrNone;
	}


//*****************************************************************
//DDisplayPddFactory
//*****************************************************************/


/**
	Constructor
*/
DDisplayPddFactory::DDisplayPddFactory()
	{
	__GCE_DEBUG_PRINT("DDisplayPddFactory::DDisplayPddFactory()\n");

	iVersion		= TVersion(KDisplayChMajorVersionNumber,
                      KDisplayChMinorVersionNumber,
                      KDisplayChBuildVersionNumber);
	}

/**
	PDD factory function. Creates a PDD object.

	@param  aChannel  A pointer to an PDD channel object which will be initialised on return.

	@return KErrNone  if object successfully allocated, KErrNoMemory if not.
*/
TInt DDisplayPddFactory::Create(DBase*& aChannel, TInt aUnit, const TDesC8* aInfo, const TVersion& aVer)
	{
	DDisplayPddNaviEng *device= new DDisplayPddNaviEng() ;
	aChannel=device;
	if (!device)
		{
		return KErrNoMemory;
		}
	return KErrNone;
	}


/**
    Set the Pdd name and return error code
*/
TInt DDisplayPddFactory::Install()
	{
	__GCE_DEBUG_PRINT("DDisplayPddFactory::Install() \n");

	TBuf<32> name(RDisplayChannel::Name());
	_LIT(KPddExtension,".pdd");
	name.Append(KPddExtension);
	return SetName(&name);
	}


void DDisplayPddFactory::GetCaps(TDes8& /*aDes*/) const
	{
	//Not supported
	}


/**
    Validate version and number of units.
*/
TInt DDisplayPddFactory::Validate(TInt aUnit, const TDesC8* /*anInfo*/, const TVersion& aVer)
	{
	if (!Kern::QueryVersionSupported(iVersion,aVer))
		{
		return KErrNotSupported;
		}

	if (aUnit != 0)
		{
		return KErrNotSupported;
		}

	return KErrNone;
	}

DECLARE_EXTENSION_PDD()
/**
	"Standard PDD" entrypoint.Creates PDD factory when Kern::InstallPhysicalDevice is called

	@return pointer to the PDD factory object.
*/
	{
	__GCE_DEBUG_PRINT("DECLARE_EXTENSION_PDD()\n");
	return new DDisplayPddFactory ;
	}


/**
HAL handler function

@param	aPtr a pointer to an instance of DLcdPowerHandler
@param	aFunction the function number
@param	a1 an arbitrary parameter
@param	a2 an arbitrary parameter
*/
LOCAL_C TInt halFunction(TAny* aPtr, TInt aFunction, TAny* a1, TAny* a2)
	{
	DLcdPowerHandler* pH=(DLcdPowerHandler*)aPtr;
	return pH->HalFunction(aFunction,a1,a2);
	}

/**
DFC for receiving messages from the power handler
@param	aPtr a pointer to an instance of DLcdPowerHandler
*/
void rxMsg(TAny* aPtr)
	{
	DLcdPowerHandler& h=*(DLcdPowerHandler*)aPtr;
	TMessageBase* pM=h.iMsgQ.iMessage;
	if (pM)
		{
		h.HandleMsg(pM);
		}
	}

/**
DFC for powering up the device

@param aPtr	aPtr a pointer to an instance of DLcdPowerHandler
*/
void power_up_dfc(TAny* aPtr)
	{
	((DLcdPowerHandler*)aPtr)->PowerUpDfc();
	}

/**
DFC for powering down the device

@param aPtr	aPtr a pointer to an instance of DLcdPowerHandler
*/
void power_down_dfc(TAny* aPtr)
	{
	((DLcdPowerHandler*)aPtr)->PowerDownDfc();
	}


/**
Default constructor
*/
DLcdPowerHandler::DLcdPowerHandler() :
		DPowerHandler(KLitLcd),
		iPowerUpDfc(&power_up_dfc,this,6),
		iPowerDownDfc(&power_down_dfc,this,7),
		iBacklightOn(EFalse),
		iContrast(KConfigInitialDisplayContrast),
		iBrightness(KConfigInitialDisplayBrightness),
		iMsgQ(rxMsg,this,NULL,1)
	{
	}


TInt DLcdPowerHandler::InitialiseController()
	{
	// Call Power Resource Manager to set clk value, immediately after it initialises the resource
	DPowerResourceController::PostBootLevel(ENE1_TBDisplayDclkResource,	EDisplayDclk24937KHz);

	//Set up layers 0-3 needed by the GCE.
	//Layer 0 is associated with legacy buffer.
	//Layer 1 and 2 are associated with composition buffers 0 and 1 respectively.
	//Layer 3 is associated with user buffers.

	// Memory Frame control registers
	AsspRegister::Write64(KHwDisplayEndianConversion,	KEndianConversionValue);

	// Default to 32bpp
	TUint64 PixelFormatValue = KPixelFormatValue32bpp;
	if (iVideoInfo.iBitsPerPixel == 32)
		{
		PixelFormatValue = KPixelFormatValue32bpp;
		}
	else if (iVideoInfo.iBitsPerPixel == 16)
		{
		PixelFormatValue = KPixelFormatValue16bpp;
		}

	AsspRegister::Write64(KHwDisplayPixelFormat, PixelFormatValue<<48 | PixelFormatValue<<32 | PixelFormatValue<<16 | PixelFormatValue);

	//Set up the the memory frame 0 start address to be the one of the Legacy Buffer
	AsspRegister::Write64(KHwDisplayMemoryFrameAddress, ivRamPhys);

	//Memory frame 0-3 size H
	TInt HValue		= Lcd_Mode_Config[iInitialMode].iConfigLcdWidth-1 ;
	AsspRegister::Write64(KHwDisplayMemoryFrameSizeH,  (TUint64) HValue<<48  | (TUint64) HValue<<32 | HValue<<16 | HValue ) ;

	//Memory frame 0-3 size V
	TInt VValue		= Lcd_Mode_Config[iInitialMode].iConfigLcdHeight-1 ;
	AsspRegister::Write64(KHwDisplayMemoryFrameSizeV,  (TUint64) VValue<<48 | (TUint64) VValue<<32 | VValue<<16 | VValue ) ;

	//0 for all layers
    AsspRegister::Write64(KHwDisplayMemoryFrameStartPointX, 0);
	AsspRegister::Write64(KHwDisplayMemoryFrameStartPointY, 0);

	//Memory frame 0-3 display frame size H
	HValue			= Lcd_Mode_Config[iInitialMode].iConfigLcdWidth-1 ;
	AsspRegister::Write64(KHwDisplayMemoryFrameDisplayFrameSizeH, (TUint64) HValue<<48 | (TUint64) HValue<<32 | HValue<<16 | HValue );

	//Memory frame 1-3 display frame size V
	VValue			= Lcd_Mode_Config[iInitialMode].iConfigLcdHeight-1 ;
	AsspRegister::Write64(KHwDisplayMemoryFrameDisplayFrameSizeV,  (TUint64) VValue<<48 |(TUint64) VValue<<32 | VValue<<16 | VValue);


	// Display frame control registers ( Frame 0 is associated to the Legacy buffer)
	AsspRegister::Write64(KHwDisplayDisplayFrameControl, KDisplayFrameControlValue);

	//0 for all layers
	AsspRegister::Write64(KHwDisplayDisplayFrameStartPointX, 1);
	AsspRegister::Write64(KHwDisplayDisplayFrameStartPointY, 0);

	// Display frames 1-3 size H - Sets the horizontal size to be displayed in pixel units
	TInt width		= Lcd_Mode_Config[iInitialMode].iConfigLcdWidth-1;
	AsspRegister::Write64(KHwDisplayDisplayFrameSizeH,  (TUint64) width<<48 | (TUint64) width<<32 | width<<16 | width   );


	 // Display frames 0-3 size V - Sets the vertical  size to be displayed in pixel units
	TInt height       = Lcd_Mode_Config[iInitialMode].iConfigLcdHeight-1;
	AsspRegister::Write64(KHwDisplayDisplayFrameSizeV,  (TUint64)height<<48 |(TUint64) height<<32 | height<<16 | height   );


	// Display frames    0-3 Constant color R - G - B
	AsspRegister::Write64(KHwDisplayDisplayFrameConstantColour,         (TUint64) 0x404<<32 | 0x404  );
	AsspRegister::Write64(KHwDisplayDisplayFrameConstantColour + KHex8, (TUint64) 0x404<<32 | 0x404  );
	AsspRegister::Write64(KHwDisplayDisplayFrameBackgroundColour,	    0x404);

	// Display control registers
	AsspRegister::Write64(KHwDisplayResolutionAndCycle,  RESOLUTION_AND_CYCLE);
	AsspRegister::Write64(KHwDisplayPulseWidth,          Lcd_Mode_Config[iInitialMode].iPulseWidth);

	AsspRegister::Write64(KHwDisplaySettings,	   KDisplaySettingsValue);
	AsspRegister::Write64(KHwDisplayBrightness,   KDisplayBrightnessVal);

	// Interface control registers

	AsspRegister::Write64(KHwDisplayInterfaceControl, KInterfaceControlValue); //sends signals out

	// Set DCLK frequency:
	AsspRegister::Write32(KHwSystemCtrlBase+KHoSCUDisplayDCLKCtrl, 11);

	return KErrNone;
	}

/**
Second-phase constructor

Called by factory function at ordinal 0
*/
TInt DLcdPowerHandler::Create()
	{
	pLcd			= this;

	// map the video RAM
	TInt vSize		= ((NaviEngineAssp*)Arch::TheAsic())->VideoRamSize();
	ivRamPhys		= TNaviEngine::VideoRamPhys();
	TInt r			= DPlatChunkHw::New(iChunk,ivRamPhys,vSize,EMapAttrUserRw|EMapAttrBufferedC);
	if (r != KErrNone)
		return r;

	//create "secure" screen immediately after normal one
	iSecurevRamPhys = ivRamPhys + vSize;
	TInt r2 = DPlatChunkHw::New(iSecureChunk,iSecurevRamPhys,vSize,EMapAttrUserRw|EMapAttrBufferedC);
	if (r2 != KErrNone)
		return r2;

	TUint* pV		= (TUint*)iChunk->LinearAddress();

	__KTRACE_OPT(KEXTENSION,Kern::Printf("DLcdPowerHandler::Create: VideoRamSize=%x, VideoRamPhys=%08x, VideoRamLin=%08x",vSize,ivRamPhys,pV));

	TUint* pV2		= (TUint*)iSecureChunk->LinearAddress();

	__KTRACE_OPT(KEXTENSION,Kern::Printf("DLcdPowerHandler::Create: Secure display VideoRamSize=%x, VideoRamPhys=%08x, VideoRamLin=%08x",vSize,iSecurevRamPhys,pV2));

	// Read display mode set with DIP switches 7 & 8
	// to get the requested LCD display mode
	iInitialMode = ReadDipSwitchDisplayMode();

	// setup the video info structure, this'll be used to remember the video settings
	iVideoInfo.iDisplayMode				= SCREEN_UNIT;
	iVideoInfo.iOffsetToFirstPixel		= Lcd_Mode_Config[iInitialMode].iOffsetToFirstVideoBuffer;
	iVideoInfo.iIsPalettized			= Lcd_Mode_Config[iInitialMode].iIsPalettized;
	iVideoInfo.iOffsetBetweenLines		= Lcd_Mode_Config[iInitialMode].iConfigLcdWidth * (Lcd_Mode_Config[iInitialMode].iBitsPerPixel >> KShiftBitsPerByte);
	iVideoInfo.iBitsPerPixel			= Lcd_Mode_Config[iInitialMode].iBitsPerPixel;
	iVideoInfo.iSizeInPixels.iWidth		= Lcd_Mode_Config[iInitialMode].iReportedLcdWidth;
	iVideoInfo.iSizeInPixels.iHeight	= Lcd_Mode_Config[iInitialMode].iReportedLcdHeight;
	iVideoInfo.iSizeInTwips.iWidth		= PixelsToTwips(iVideoInfo.iSizeInPixels.iWidth);
	iVideoInfo.iSizeInTwips.iHeight		= PixelsToTwips(iVideoInfo.iSizeInPixels.iHeight);
	iVideoInfo.iIsMono					= KConfigLcdIsMono;
	iVideoInfo.iVideoAddress			= (TInt)pV;
	iVideoInfo.iIsPixelOrderLandscape	= KConfigLcdPixelOrderLandscape;
	iVideoInfo.iIsPixelOrderRGB			= KConfigLcdPixelOrderRGB;

	iSecureVideoInfo					= iVideoInfo;
	iSecureVideoInfo.iVideoAddress		= (TInt)pV2;

	iDisplayOn							= EFalse;
	iSecureDisplay						= EFalse;

	// install the HAL function
	r=Kern::AddHalEntry(EHalGroupDisplay, halFunction, this);
	if (r!=KErrNone)
		return r;

	iPowerUpDfc.SetDfcQ(iDfcQ);
	iPowerDownDfc.SetDfcQ(iDfcQ);
	iMsgQ.SetDfcQ(iDfcQ);
	iMsgQ.Receive();

	// Alloc Physical RAM for the Composition Buffers used by the GCE
	iSize = FRAME_BUFFER_SIZE(Lcd_Mode_Config[iInitialMode].iBitsPerPixel, Lcd_Mode_Config[iInitialMode].iConfigLcdWidth, Lcd_Mode_Config[iInitialMode].iConfigLcdHeight);
	// double and round the page size
	TUint round = Kern::RoundToPageSize(2*iSize);

	r=Epoc::AllocPhysicalRam(round , iCompositionPhysical);
	if(r!=KErrNone)
		{
		return r;
		}

	// clear interrupts
	AsspRegister::Write64(KHwDisplayInterruptClear, 0x7f);

	//Set up V Sync interrupt.
	//Bind Interrupt
	TInt interruptId= Interrupt::Bind(EIntDisp0,Service,this);

	if (interruptId<0)
		{
		return interruptId;
		}

	Interrupt::Enable(interruptId);

	//In case more display related interrupts are enabled KHwDisplayInterruptEnableSelection
	// should be changed appropriately

	AsspRegister::Write64(KHwDisplayInterruptEnableSelection , KVSyncSelectToChannel1 );

	// install the power handler
	// power up the screen
	Add();
	DisplayOn();

	InitialiseController();

	__KTRACE_OPT(KEXTENSION, Kern::Printf(
				"Lcd_Mode_Config: mode = %d, iInitialMode %d, physical %dx%d, reporting %dx%d, bpp %d, obl %d",
				iVideoInfo.iDisplayMode, iInitialMode, Lcd_Mode_Config[iInitialMode].iConfigLcdWidth, Lcd_Mode_Config[iInitialMode].iConfigLcdHeight,
				iVideoInfo.iSizeInPixels.iWidth, iVideoInfo.iSizeInPixels.iHeight, iVideoInfo.iBitsPerPixel, iVideoInfo.iOffsetBetweenLines)
	);

	SplashScreen();

	return KErrNone;
	}



/**
 * Dispatch interrupts received by the display subsystem
 * @param aPtr Argument passed to the ISR
 */
void DLcdPowerHandler::Service(TAny* aPtr)
	{
	DLcdPowerHandler& display = *(DLcdPowerHandler*)aPtr;

	TInt irq=__SPIN_LOCK_IRQSAVE(callbackLock);

	TUint64 dispStatus = AsspRegister::Read64(KHwDisplayInterruptStatus);
	TUint64 dispEnable = AsspRegister::Read64(KHwDisplayInterruptEnable);

	TUint64 reg64 =  dispEnable;

	//disable VSYNC
	reg64 &= KVSyncDisable;
	AsspRegister::Write64(KHwDisplayInterruptEnable , reg64 );

	//V sync interrupt signal has been received
	if(  ((dispStatus & KVSyncStatus) == KVSyncStatus ) && ( ( dispEnable & KVSyncEnable ) == KVSyncEnable  )    )
		{
		// Call the GCE call back function in case of VSync
		if ((display.iAppCallBk[0] != NULL) && (display.iAppCallBk[0]->iCbFn != NULL))
			{
			(*(display.iAppCallBk[0]->iCbFn))(0,display.iAppCallBk[0]->iDataPtr);
			}
		if((display.iAppCallBk[1] != NULL) && (display.iAppCallBk[1]->iCbFn != NULL))
			{
			(*(display.iAppCallBk[1]->iCbFn))(1,display.iAppCallBk[1]->iDataPtr);
			}
		}

	__SPIN_UNLOCK_IRQRESTORE(callbackLock,irq);

	reg64 = AsspRegister::Read64(KHwDisplayInterruptClear);
	AsspRegister::Write64(KHwDisplayInterruptClear, reg64 | KVSyncClear);   //CLear Vsync interrupt bit

	}


/**
 * Register the call back function.
 * Components interested in receiving notification of the Vsync interrupt should register a callback function.
 */
EXPORT_C TInt DLcdPowerHandler::RegisterCallback(TLcdUserCallBack* aCbPtr)
{
	__KTRACE_OPT(KEXTENSION ,Kern::Printf("DLcdPowerHandler::RegisterCallBack %08x\n",aCbPtr->iCbFn) );

	TInt irq=__SPIN_LOCK_IRQSAVE(callbackLock);

	if(aCbPtr != NULL)
		{
		if ( pLcd->iAppCallBk[0] == NULL  )
			{
			pLcd->iAppCallBk[0] = aCbPtr;
			}
		else
			{
			if((pLcd->iAppCallBk[1] == NULL) && (pLcd->iAppCallBk[0]->iCbFn != aCbPtr->iCbFn))
				{
				pLcd->iAppCallBk[1] = aCbPtr;
				}
			else
				{
				__SPIN_UNLOCK_IRQRESTORE(callbackLock,irq);
				return KErrInUse;
				}
			}

		__SPIN_UNLOCK_IRQRESTORE(callbackLock,irq);
		return KErrNone;
		}
	else
		{
		__SPIN_UNLOCK_IRQRESTORE(callbackLock,irq);
		__KTRACE_OPT(KEXTENSION, Kern::Printf("Error: The supplied listener's callback is NULL"));
		return KErrArgument;
		}
}


/**
 *DeRegister the call back function
 */
EXPORT_C void DLcdPowerHandler::DeRegisterCallback(TLcdUserCallBack* aCbPtr)
{
	__KTRACE_OPT(KEXTENSION ,Kern::Printf("DLcdPowerHandler::DeRegisterCallBack %08x\n ",aCbPtr->iCbFn)  );

	TInt irq=__SPIN_LOCK_IRQSAVE(callbackLock);
	if(aCbPtr != NULL)
		{
	    if( pLcd->iAppCallBk[0] != NULL)
			{
			if ( (pLcd->iAppCallBk[0]->iDataPtr == aCbPtr->iDataPtr) && (pLcd->iAppCallBk[0]->iCbFn == aCbPtr->iCbFn) )
				{
				pLcd->iAppCallBk[0] = NULL;
				}
			}

		if( pLcd->iAppCallBk[1] != NULL)
			{
			if ( (pLcd->iAppCallBk[1]->iDataPtr == aCbPtr->iDataPtr) && (pLcd->iAppCallBk[1]->iCbFn == aCbPtr->iCbFn) )
				{
				pLcd->iAppCallBk[1] = NULL;
				}
			}
		}
	__SPIN_UNLOCK_IRQRESTORE(callbackLock,irq);
}


/**
Turn the display on
May be called as a result of a power transition or from the HAL
If called from HAL, then the display may be already be on (iDisplayOn == ETrue)
*/
void DLcdPowerHandler::DisplayOn()
	{
	__KTRACE_OPT(KPOWER, Kern::Printf("DisplayOn %d", iDisplayOn));
	if (!iDisplayOn)				// may have been powered up already
		{
		iDisplayOn = ETrue;
		PowerUpLcd(iSecureDisplay);
		SetContrast(iContrast);
		SetBrightness(iBrightness);
		}
	}

/**
Turn the display off
May be called as a result of a power transition or from the HAL
If called from Power Manager, then the display may be already be off (iDisplayOn == EFalse)
if the platform is in silent running mode
*/
void DLcdPowerHandler::DisplayOff()
	{
	__KTRACE_OPT(KPOWER, Kern::Printf("DisplayOff %d", iDisplayOn));
	if (iDisplayOn)
		{
		iDisplayOn = EFalse;
		PowerDownLcd();
		}
	}

/**
Switch between secure and non-secure displays

@param aSecure ETrue if switching to secure display
*/
void DLcdPowerHandler::SwitchDisplay(TBool aSecure)
	{
	if (aSecure)
		{
		if (!iSecureDisplay)
			{
			TThreadMessage& m=Kern::Message();
			m.iValue = EDisplayHalSetSecure;
			m.SendReceive(&iMsgQ);		// send a message and block Client thread until secure display has been enabled.
			}
		}
	else
		{
		if (iSecureDisplay)
			{
			TThreadMessage& m=Kern::Message();
			m.iValue = -EDisplayHalSetSecure;
			m.SendReceive(&iMsgQ);		// send a message and block Client thread until secure display has been disabled.
			}
		}
	}


/**
Switch to secure display

*/
void DLcdPowerHandler::SwitchToSecureDisplay()
	{
	DisplayOff();
	iSecureDisplay = ETrue;
	DisplayOn();
	}


/**
Switch from secure display

*/
void DLcdPowerHandler::SwitchFromSecureDisplay()
	{
	DisplayOff();
	iSecureDisplay = EFalse;
	DisplayOn();
	}


/**
DFC to power up the display
*/
void DLcdPowerHandler::PowerUpDfc()
	{
	__KTRACE_OPT(KPOWER, Kern::Printf("PowerUpDfc"));
	DisplayOn();

	PowerUpDone();				// must be called from a different thread than PowerUp()
	}

/**
DFC to power down the display
*/
void DLcdPowerHandler::PowerDownDfc()
	{
	__KTRACE_OPT(KPOWER, Kern::Printf("PowerDownDfc"));
	DisplayOff();
	PowerDownDone();			// must be called from a different thread than PowerUp()
	}

/**
Schedule the power-down DFC
*/
void DLcdPowerHandler::PowerDown(TPowerState)
	{
	iPowerDownDfc.Enque();		// schedules DFC to execute on this driver's thread
	}

/**
Schedule the power-up DFC
*/
void DLcdPowerHandler::PowerUp()
	{
	iPowerUpDfc.Enque();		// schedules DFC to execute on this driver's thread
	}

/**
Power up the display

@param aSecure ETrue if powering up the secure display
*/
void DLcdPowerHandler::PowerUpLcd(TBool aSecure)
	{
	AsspRegister::Write16(KHwLCDDispBase,0x140); //power-up board and backlight
	}

/**
Power down the display and the backlight
*/
void DLcdPowerHandler::PowerDownLcd()
	{
	SetBacklightState(EFalse);
	AsspRegister::Write16(KHwLCDDispBase, 0x143); //power-down board and backlight
	}

/**
Set the Lcd contrast

@param aValue the contrast setting
*/
TInt DLcdPowerHandler::SetContrast(TInt aValue)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetContrast(%d)", aValue));

	if (aValue >= KConfigLcdMinDisplayContrast && aValue <= KConfigLcdMaxDisplayContrast)
		{
		iContrast=aValue;
		return KErrNone;
		}

	return KErrArgument;
	}

/**
Queue a message to set the Lcd Contrast

@param aValue the contrast setting
*/
TInt DLcdPowerHandler::HalSetContrast(TInt aValue)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("HalSetContrast(%d)", aValue));

	TThreadMessage& m=Kern::Message();
	m.iValue = EDisplayHalSetDisplayContrast;
	m.iArg[0] = (TAny *) aValue;

	return ( m.SendReceive(&iMsgQ) );		// send a message and block Client thread until contrast has been set.
	}



/**
Set the Lcd brightness

@param aValue the brightness setting
*/
TInt DLcdPowerHandler::SetBrightness(TInt aValue)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetBrightness(%d)", aValue));

	if (aValue >= KConfigLcdMinDisplayBrightness && aValue <= KConfigLcdMaxDisplayBrightness)
		{
		iBrightness=aValue;

		return KErrNone;
		}
	return KErrArgument;
	}

/**
Queue a message to set the Lcd brightness

@param aValue the brightness setting
*/
TInt DLcdPowerHandler::HalSetBrightness(TInt aValue)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("HalSetBrightness(%d)", aValue));

	TThreadMessage& m=Kern::Message();
	m.iValue = EDisplayHalSetDisplayBrightness;
	m.iArg[0]= (TAny *) aValue;

	return (m.SendReceive(&iMsgQ));		// send a message and block Client thread until brightness has been set.
	}


/**
Turn the backlight on
*/
void DLcdPowerHandler::BacklightOn()
	{
	// turn the backlight on
	AsspRegister::Write16(KHwLCDDispBase, 0x140);
	}

/**
Turn the backlight off
*/
void DLcdPowerHandler::BacklightOff()
	{
	// turn the backlight off
	AsspRegister::Write16(KHwLCDDispBase, 0x142);
	}

/**
Set the state of the backlight

@param aState ETrue if setting the backlight on
*/
void DLcdPowerHandler::SetBacklightState(TBool aState)
	{
	iBacklightOn=aState;
	if (iBacklightOn)
		BacklightOn();
	else
		BacklightOff();
	}

void DLcdPowerHandler::ScreenInfo(TScreenInfoV01& anInfo)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("DLcdPowerHandler::ScreenInfo"));

	anInfo.iWindowHandleValid	 = EFalse;
	anInfo.iWindowHandle		 = NULL;
	anInfo.iScreenAddressValid	 = ETrue;
	anInfo.iScreenAddress		 = (TAny *)(iChunk->LinearAddress());
	anInfo.iScreenSize.iWidth	 = Lcd_Mode_Config[iInitialMode].iReportedLcdWidth;
	anInfo.iScreenSize.iHeight	 = Lcd_Mode_Config[iInitialMode].iReportedLcdHeight;
	}

/**
Handle a message from the power handler
*/
void DLcdPowerHandler::HandleMsg(TMessageBase* aMsg)
{
	TInt r = KErrNone;
	TThreadMessage& m=*(TThreadMessage*)aMsg;
	switch(m.iValue)
		{
		case EDisplayHalWsSwitchOnScreen:
			DisplayOn();
			break;
		case (-EDisplayHalWsSwitchOnScreen):
			DisplayOff();
			break;
		case EDisplayHalSetSecure:
			SwitchToSecureDisplay();
			break;
		case (-EDisplayHalSetSecure):
			SwitchFromSecureDisplay();
			break;
		case EDisplayHalSetDisplayContrast:
			{
			r = SetContrast(m.Int0());
			break;
			}
		case EDisplayHalSetDisplayBrightness:
			{
			r = SetBrightness(m.Int0());
			break;
		default:
			r = KErrNotSupported;
			__KTRACE_OPT(KHARDWARE,Kern::Printf("DLcdPowerHandler::HalFunction %d defaulted", m.iValue));
			break;
			}
		}

	m.Complete(r, ETrue);
	}

/**
Send a message to the power-handler message queue to turn the display on
*/
void DLcdPowerHandler::WsSwitchOnScreen()
	{
	TThreadMessage& m=Kern::Message();
	m.iValue = EDisplayHalWsSwitchOnScreen;
	m.SendReceive(&iMsgQ);		// send a message and block Client thread until keyboard has been powered up
	}

/**
Send a message to the power-handler message queue to turn the display off
*/
void DLcdPowerHandler::WsSwitchOffScreen()
	{
	TThreadMessage& m=Kern::Message();
	m.iValue = -EDisplayHalWsSwitchOnScreen;
	m.SendReceive(&iMsgQ);		// send a message and block Client thread until keyboard has been powered down
	}

/**
Return information about the current display mode

@param	aInfo a structure supplied by the caller to be filled by this function.
@param	aSecure ETrue if requesting information about the secure display
@return	KErrNone if successful
*/
TInt DLcdPowerHandler::GetCurrentDisplayModeInfo(TVideoInfoV01& aInfo, TBool aSecure)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("GetCurrentDisplayModeInfo"));
	NKern::FMWait(&iLock);
	if (aSecure)
		aInfo = iSecureVideoInfo;
	else
		aInfo = iVideoInfo;
	NKern::FMSignal(&iLock);
	return KErrNone;
	}

/**
Return information about the specified display mode

@param	aScreenNumber the screen number to query
@param	aInfo a structure supplied by the caller to be filled by this function.
@return	KErrNone if successful
*/
TInt DLcdPowerHandler::GetSpecifiedDisplayModeInfo(TInt aScreenNumber, TVideoInfoV01& aInfo)
	{
	__KTRACE_OPT(KEXTENSION, Kern::Printf("GetSpecifiedDisplayModeInfo screen unit is %d",aScreenNumber));

	if (aScreenNumber < 0 || aScreenNumber >= SCREEN_UNIT_COUNT)
		return KErrArgument;

	NKern::FMWait(&iLock);
	aInfo = iVideoInfo;
	NKern::FMSignal(&iLock);

	return KErrNone;
	}

/**
Set the display mode

@param	aScreenNumber the screen number to set
*/
TInt DLcdPowerHandler::SetDisplayMode(TInt aScreenNumber)
	{
	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetDisplayMode ( screen unit) = %d", aScreenNumber));

	if (aScreenNumber < 0 || aScreenNumber >= SCREEN_UNIT_COUNT)
		return KErrArgument;

	NKern::FMWait(&iLock);

	// store the current mode
	iVideoInfo.iDisplayMode					= SCREEN_UNIT;
	iVideoInfo.iOffsetToFirstPixel			= Lcd_Mode_Config[iInitialMode].iOffsetToFirstVideoBuffer;
	iVideoInfo.iIsPalettized				= Lcd_Mode_Config[iInitialMode].iIsPalettized;
	iVideoInfo.iOffsetBetweenLines			= Lcd_Mode_Config[iInitialMode].iConfigLcdWidth * (Lcd_Mode_Config[iInitialMode].iBitsPerPixel >> KShiftBitsPerByte);
	iVideoInfo.iBitsPerPixel				= Lcd_Mode_Config[iInitialMode].iBitsPerPixel;

	// store the current mode for secure screen
	iSecureVideoInfo.iDisplayMode			= SCREEN_UNIT;
	iSecureVideoInfo.iOffsetToFirstPixel	= Lcd_Mode_Config[iInitialMode].iOffsetToFirstVideoBuffer;
	iSecureVideoInfo.iIsPalettized			= Lcd_Mode_Config[iInitialMode].iIsPalettized;
	iSecureVideoInfo.iOffsetBetweenLines	= Lcd_Mode_Config[iInitialMode].iConfigLcdWidth * (Lcd_Mode_Config[iInitialMode].iBitsPerPixel >> KShiftBitsPerByte);
	iSecureVideoInfo.iBitsPerPixel			= Lcd_Mode_Config[iInitialMode].iBitsPerPixel;

	NKern::FMSignal(&iLock);

	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetDisplayMode screenNumber = %d, otfp = %d, palettized = %d, bpp = %d, obl = %d",
		aScreenNumber, iVideoInfo.iOffsetToFirstPixel, iVideoInfo.iIsPalettized, iVideoInfo.iBitsPerPixel, iVideoInfo.iOffsetBetweenLines));

	return KErrNone;
	}

/**
Fill the video memory with an initial pattern or image
This will be displayed on boot-up
*/
const TInt KGranularity = 4;

void DLcdPowerHandler::SplashScreen()
	{
	//initialise the video ram to be a splash screen

	__KTRACE_OPT(KEXTENSION,Kern::Printf("SplashScreen"));

	TInt xres, yres, bpp, bpl, ofp;
	TLinAddr addr;
	addr = (TLinAddr)iVideoInfo.iVideoAddress;
	xres = iVideoInfo.iSizeInPixels.iWidth;
	yres = iVideoInfo.iSizeInPixels.iHeight;
	bpp  = iVideoInfo.iBitsPerPixel;
	bpl  = iVideoInfo.iOffsetBetweenLines;
	ofp  = iVideoInfo.iOffsetToFirstPixel;

	TInt xb, yb;

	for (yb=0; yb<yres/KGranularity; ++yb)
		{
		for (xb=0; xb<xres/KGranularity; ++xb)
			{
			TUint c=((xb*xb+yb*yb)>>1)%251;
			c^=0xff;
			TUint r=c&7;
			TUint g=(c>>3)&7;
			TUint b=(c>>6);
			TUint c16=(b<<14)|(g<<8)|(r<<2);
			c16 |= (c16<<16);
			TUint c8=c|(c<<8);
			c8 |= (c8<<16);
			TUint c32=(b<<22)|(g<<13)|(r<<5);
			TInt baddr=addr+ofp+yb*KGranularity*bpl+xb*KGranularity*bpp/8;
			TInt l;
			for (l=0; l<KGranularity; ++l, baddr+=bpl)
				{
				TUint32* p=(TUint32*)baddr;
				if (bpp==8)
					*p++=c8;
				else if (bpp==16)
					*p++=c16, *p++=c16;
				else
					{
					*p++=c32+0x0;
					*p++=c32+0x4;
					*p++=c32+0x8;
					*p++=c32+0xc;
					c32+=0x100;
					}
				}
			}
		}

	// Secure screen
	TUint8* linePtr = (TUint8*)iSecureVideoInfo.iVideoAddress+
					   iSecureVideoInfo.iOffsetToFirstPixel;
	TInt pixelSize  = iSecureVideoInfo.iBitsPerPixel;
	if(pixelSize>8)
		pixelSize = (pixelSize+7)&~7; // Round up to whole number of bytes
	TInt bytesPerLine = (pixelSize*iSecureVideoInfo.iSizeInPixels.iWidth) >> 3;
	for(TInt y=0; y<iSecureVideoInfo.iSizeInPixels.iHeight; y++)
		{
		for(TInt x=0; x<bytesPerLine; x++)
			linePtr[x] = x+y;
		linePtr += iSecureVideoInfo.iOffsetBetweenLines;
		}
	}



/**
Get the size of the pallete

@return	the number of pallete entries
*/
TInt DLcdPowerHandler::NumberOfPaletteEntries()
	{
	TInt num = iVideoInfo.iIsPalettized ? 1<<iVideoInfo.iBitsPerPixel : 0;

	__KTRACE_OPT(KEXTENSION,Kern::Printf("NumberOfPaletteEntries = %d", num));

	return num;
	}


/**
Retrieve the palette entry at a particular offset

@param	aEntry the palette index
@param	aColor a caller-supplied pointer to a location where the returned RGB color is to be stored
@return	KErrNone if successful
		KErrNotSupported if the current vide mode does not support a palette
		KErrArgument if aEntry is out of range
*/
TInt DLcdPowerHandler::GetPaletteEntry(TInt aEntry, TInt* aColor)
	{
	NKern::FMWait(&iLock);
	if (!iVideoInfo.iIsPalettized)
		{
		NKern::FMSignal(&iLock);
		return KErrNotSupported;
		}

	if ((aEntry < 0) || (aEntry >= NumberOfPaletteEntries()))
		{
		NKern::FMSignal(&iLock);
		return KErrArgument;
		}

	NKern::FMSignal(&iLock);

	__KTRACE_OPT(KEXTENSION,Kern::Printf("GetPaletteEntry %d color 0x%x", aEntry, aColor));

	return KErrNone;
	}

/**
Set the palette entry at a particular offset

@param	aEntry the palette index
@param	aColor the RGB color to store
@return	KErrNone if successful
		KErrNotSupported if the current vide mode does not support a palette
		KErrArgument if aEntry is out of range
*/
TInt DLcdPowerHandler::SetPaletteEntry(TInt aEntry, TInt aColor)
	{

	NKern::FMWait(&iLock);
	if (!iVideoInfo.iIsPalettized)
		{
		NKern::FMSignal(&iLock);
		return KErrNotSupported;
		}

	if ((aEntry < 0) || (aEntry >= NumberOfPaletteEntries()))	//check entry in range
		{
		NKern::FMSignal(&iLock);
		return KErrArgument;
		}

	NKern::FMSignal(&iLock);
	__KTRACE_OPT(KEXTENSION,Kern::Printf("SetPaletteEntry %d to 0x%x", aEntry, aColor ));

	return KErrNone;
	}

/**
a HAL entry handling function for HAL group attribute EHalGroupDisplay.
The HalFunction is called in the context of the user thread which is
requesting the particular HAL display function.

@param	a1 an arbitrary argument
@param	a2 an arbitrary argument
@return	KErrNone if successful
*/
TInt DLcdPowerHandler::HalFunction(TInt aFunction, TAny* a1, TAny* a2)
	{
	TInt r=KErrNone;
	switch(aFunction)
		{
		case EDisplayHalScreenInfo:
			{
			TPckgBuf<TScreenInfoV01> vPckg;
			ScreenInfo(vPckg());
			Kern::InfoCopy(*(TDes8*)a1,vPckg);
			break;
			}

		case EDisplayHalWsRegisterSwitchOnScreenHandling:
			iWsSwitchOnScreen=(TBool)a1;
			break;

		case EDisplayHalWsSwitchOnScreen:
			WsSwitchOnScreen();
			break;

		case EDisplayHalMaxDisplayContrast:
			{
			TInt mc=KConfigLcdMaxDisplayContrast;
			kumemput32(a1,&mc,sizeof(mc));
			break;
			}
		case EDisplayHalSetDisplayContrast:
			if(!Kern::CurrentThreadHasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetDisplayContrast")))
				return KErrPermissionDenied;
			r=HalSetContrast(TInt(a1));
			break;

		case EDisplayHalDisplayContrast:
			kumemput32(a1,&iContrast,sizeof(iContrast));
			break;

		case EDisplayHalMaxDisplayBrightness:
			{
			TInt mc=KConfigLcdMaxDisplayBrightness;
			kumemput32(a1,&mc,sizeof(mc));
			break;
			}

		case EDisplayHalSetDisplayBrightness:
			if(!Kern::CurrentThreadHasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetDisplayBrightness")))
				return KErrPermissionDenied;
			r=HalSetBrightness(TInt(a1));
			break;

		case EDisplayHalDisplayBrightness:
			kumemput32(a1,&iBrightness,sizeof(iBrightness));
			break;

		case EDisplayHalSetBacklightOn:
			if(!Kern::CurrentThreadHasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetBacklightOn")))
				return KErrPermissionDenied;
			if (Kern::MachinePowerStatus()<ELow)
				r=KErrBadPower;
			else
				SetBacklightState(TBool(a1));
			break;

		case EDisplayHalBacklightOn:
			kumemput32(a1,&iBacklightOn,sizeof(TInt));
			break;

		case EDisplayHalModeCount:
			{
			TInt ndm = SCREEN_UNIT_COUNT;
			kumemput32(a1, &ndm, sizeof(ndm));
			break;
			}

		case EDisplayHalSetMode:
			if(!Kern::CurrentThreadHasCapability(ECapabilityMultimediaDD,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetMode")))
				return KErrPermissionDenied;
			r = SetDisplayMode((TInt)a1);
			break;

		case EDisplayHalMode:
			kumemput32(a1, &iVideoInfo.iDisplayMode, sizeof(iVideoInfo.iDisplayMode));
			break;

		case EDisplayHalSetPaletteEntry:
			if(!Kern::CurrentThreadHasCapability(ECapabilityMultimediaDD,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetPaletteEntry")))
				return KErrPermissionDenied;
			r = SetPaletteEntry((TInt)a1, (TInt)a2);
			break;

		case EDisplayHalPaletteEntry:
			{
			TInt entry;
			kumemget32(&entry, a1, sizeof(TInt));
			TInt x;
			r = GetPaletteEntry(entry, &x);
			if (r == KErrNone)
				kumemput32(a2, &x, sizeof(x));
			break;
			}

		case EDisplayHalSetState:
			{
			if(!Kern::CurrentThreadHasCapability(ECapabilityPowerMgmt,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetState")))
				return KErrPermissionDenied;
			if ((TBool)a1)
				{
				WsSwitchOnScreen();
				}
			else
				{
				WsSwitchOffScreen();
				}
			break;
			}

		case EDisplayHalState:
			kumemput32(a1, &iDisplayOn, sizeof(TBool));
			break;

		case EDisplayHalColors:
			{
			TInt mdc = KConfigLcdMaxDisplayColors;
			kumemput32(a1, &mdc, sizeof(mdc));
			break;
			}

		case EDisplayHalCurrentModeInfo:
			{
			TPckgBuf<TVideoInfoV01> vPckg;
			r = GetCurrentDisplayModeInfo(vPckg(), (TBool)a2);
			if (KErrNone == r)
				Kern::InfoCopy(*(TDes8*)a1,vPckg);
			}
			break;

		case EDisplayHalSpecifiedModeInfo:
			{
			TPckgBuf<TVideoInfoV01> vPckg;
			TInt screenNumber;
			kumemget32(&screenNumber, a1, sizeof(screenNumber));
			r = GetSpecifiedDisplayModeInfo(screenNumber, vPckg());
			if (KErrNone == r)
				Kern::InfoCopy(*(TDes8*)a2,vPckg);
			}
			break;

		case EDisplayHalSecure:
			kumemput32(a1, &iSecureDisplay, sizeof(TBool));
			break;

		case EDisplayHalSetSecure:
			{
			if(!Kern::CurrentThreadHasCapability(ECapabilityMultimediaDD,__PLATSEC_DIAGNOSTIC_STRING("Checked by Hal function EDisplayHalSetSecure")))
				return KErrPermissionDenied;
			SwitchDisplay((TBool)a1);
			}
			break;

		default:
			r=KErrNotSupported;
			break;
		}
	return r;
	}


DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KPOWER,Kern::Printf("Starting LCD power manager"));

	// create LCD power handler
	TInt r=KErrNoMemory;
	DLcdPowerHandler* pH=new DLcdPowerHandler;
	if (pH)
		{
		//The same DFC queue will be used by both the LCD extension and GCE PDD
		r= Kern::DfcQCreate( pH->iDfcQ, KDisplay0ThreadPriority, &KDisplay0DfcThread );
		if ( r == KErrNone)
			{
			 r =pH->Create();
			__KTRACE_OPT(KPOWER,Kern::Printf("Returns %d",r));

			if ( r == KErrNone)
				{
				DDisplayPddFactory * device = new DDisplayPddFactory;

				if (device==NULL)
					{
					r=KErrNoMemory;
					}
				else
					{
					r=Kern::InstallPhysicalDevice(device);
					}

				#ifdef CPU_AFFINITY_ANY
                NKern::ThreadSetCpuAffinity((NThread*) pH->iDfcQ->iThread, KCpuAffinityAny);
				#endif

				__KTRACE_OPT(KEXTENSION,Kern::Printf("Installing the display device from the kernel extension returned with error code %d",r));

				}
			}

		}
	return r;
	}

