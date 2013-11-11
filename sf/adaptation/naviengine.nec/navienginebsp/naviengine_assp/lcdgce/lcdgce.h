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
* naviengine_assp\lcdgce\lcdgce.h
*
*/



/**
 @file Definitions for the ne1_tb GCE PDD
 @internalTechnology
 @prototype
*/

#ifndef __LCDGCE_H__
#define __LCDGCE_H__

#include <display.h>


const TUint   KVSyncEnable 					= 1<<1;
const TUint64 KVSyncDisable 				= ~(TUint64)(KVSyncEnable) ;
const TUint   KVSyncClear 					= 1<<1;
const TUint   KVSyncStatus 					= 1<<1;
const TUint   KVSyncSelectToChannel1		= 1<<8;

const TUint   KHex8 						= 0x8;


/* The available display modes.
	These modes can be changed by software, but the initial mode is specified by
	DSW1 DIP switch settings (7 & 8) as described in the table below:

DSW1 | 7 8 |
-----------------------------------------------------------------------------------------------------
     | 0 0 | 640 x 480 | D-SUB       | landscape |  VGA on analog 15 pin DSUB connector
     | 0 1 | 800 x 480 | NEC LCD     | landscape | WVGA
     | 1 0 | 480 x 640 | Hitachi LCD | portrait  |  VGA (LCD switches: SW4=0111, SW5=0, SW6=0, SW7=0)
     | 1 1 | 320 x 240 | D-SUB       | landscape | QVGA on analog 15 pin DSUB connector
*/
enum TVideoMode
	{
	DISPLAY_MODE_ANALOG_VGA   = 0,
	DISPLAY_MODE_HITACHI_VGA,
	DISPLAY_MODE_ANALOG_QVGA_LANDSCAPE_OLD,
	DISPLAY_MODE_HITACHI_QVGA,
	DISPLAY_MODE_NEC_WVGA,
	DISPLAY_MODE_ANALOG_QVGA_PORTRAIT,
	DISPLAY_MODE_ANALOG_QVGA_LANDSCAPE,

	DISPLAY_NUM_MODES
	};

inline TInt ReadDipSwitchDisplayMode()
	{
	// Check the IDMODE register for DIP switch settings
	// to set the requested display mode
	// Read bits 12 and 13 from IDMODE register to get the LCD mode
	TUint switches = (AsspRegister::Read32(KHwIDMODE) & KHmLcdSwitches) >> KHsLcdSwitches;

	TInt mode = 0;
	switch (switches)
		{
		default:
		case 0:
			mode = DISPLAY_MODE_ANALOG_VGA;
			break;
		case 1:
			mode = DISPLAY_MODE_HITACHI_VGA;
			break;
		case 2:
			mode = DISPLAY_MODE_NEC_WVGA;
			break;
		case 3:
			mode = DISPLAY_MODE_ANALOG_QVGA_LANDSCAPE;
			break;
			// An alternative case 3 line for QVGA Portrait
			//		case 3: iInitialMode = DISPLAY_MODE_ANALOG_QVGA_PORTRAIT;   break;
		}
	return mode;
	}

/********************************************************************/
/* Class Definition                                                 */
/********************************************************************/
/**
 * This class defines a callback mechanism that is used by a resource user to specify its callback. It contains a
 * function pointer and data pointer. The function pointer specifies the user callback function to be invoked by the
 * resource while the data pointer specifies the data to be passed to the callback function.
 */
class TLcdUserCallBack
    {
public:
    // The constructor for the callback mechanism.
    TLcdUserCallBack(TInt (*aFunction)(TUint aResID, TAny* aPtr), TAny* aPtr)

        {
        iCbFn = aFunction;
        iDataPtr = aPtr;
        }

public:
    // The callback function pointer.
    TInt (*iCbFn)(TUint aResID, TAny* aPtr);

    // Pointer to the data structure to be passed to the callback function.
    TAny *iDataPtr;
    };


/**
 PDD Factory class
 */

class DDisplayPddFactory : public DPhysicalDevice
	{
public:
	DDisplayPddFactory();

	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DBase*& aChannel, TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);
	virtual TInt Validate(TInt aDeviceType, const TDesC8* anInfo, const TVersion& aVer);
	};



class DDisplayPddNaviEng : public DDisplayPdd
	{

	public:
	DDisplayPddNaviEng();
	~DDisplayPddNaviEng();


   	//Inherited from DDisplayPdd
   	TInt  		SetLegacyMode();
    TInt  		SetGceMode();
    TInt  		SetRotation(RDisplayChannel::TDisplayRotation aRotation);

	TInt  		PostUserBuffer(TBufferNode* aNode);
	TInt  		PostCompositionBuffer(TBufferNode* aNode);
    TInt  		PostLegacyBuffer();

    TBool 		PostPending();

    TInt  		CloseMsg();
    TInt  	 	CreateChannelSetup(TInt aUnit);
    TDfcQue*	DfcQ(TInt  aUnit);

public:
	void  VSyncIsr();
	static void VSyncDfcFn(TAny* aChannel);



private:
	TDfcQue* 		iDfcQ;

    //generic  display info
	TVideoInfoV01    	iScreenInfo;

    //Pointer to a buffer in the Pending state
    TBufferNode*     	iPendingBuffer;

     //Pointer to a buffer in the Active state
    TBufferNode*     	iActiveBuffer;

    DChunk * 		 	iChunk;
    TLcdUserCallBack*   iLcdCallback;

 public:
	TDfc 		     	iVSyncDfc;

	};


//
// Add any private functions and data you require
//
NONSHARABLE_CLASS(DLcdPowerHandler) : public DPowerHandler
	{
public:
	DLcdPowerHandler();

	// from DPowerHandler
	void PowerDown(TPowerState);
	void PowerUp();

	void PowerUpDfc();
	void PowerDownDfc();

	TInt Create();
	void DisplayOn();
	void DisplayOff();
	TInt HalFunction(TInt aFunction, TAny* a1, TAny* a2);

	void PowerUpLcd(TBool aSecure);
	void PowerDownLcd();

	void ScreenInfo(TScreenInfoV01& aInfo);
	void WsSwitchOnScreen();
	void WsSwitchOffScreen();
	void HandleMsg(TMessageBase* aMsg);
	void SwitchDisplay(TBool aSecure);

	void SetBacklightState(TBool aState);
	void BacklightOn();
	void BacklightOff();
	TInt SetContrast(TInt aContrast);
	TInt HalSetContrast(TInt aContrast);
	TInt SetBrightness(TInt aBrightness);
	TInt HalSetBrightness(TInt aBrightness);
	void SwitchToSecureDisplay();
	void SwitchFromSecureDisplay();

private:
	TInt SetPaletteEntry(TInt aEntry, TInt aColor);
	TInt GetPaletteEntry(TInt aEntry, TInt* aColor);
	TInt NumberOfPaletteEntries();
	TInt GetCurrentDisplayModeInfo(TVideoInfoV01& aInfo, TBool aSecure);
	TInt GetSpecifiedDisplayModeInfo(TInt aMode, TVideoInfoV01& aInfo);
	TInt SetDisplayMode(TInt aMode);
	void SplashScreen();
	TInt GetDisplayColors(TInt* aColors);

public:
	TInt InitialiseController();
	static void Service(TAny* aPtr);

    IMPORT_C static    TInt    RegisterCallback(TLcdUserCallBack* aCbPtr);
    IMPORT_C static    void    DeRegisterCallback(TLcdUserCallBack* aCbPtr);

private:
	TBool 			iIsPalettized;
	TBool 			iDisplayOn;				// to prevent a race condition with WServer trying to power up/down at the same time
	DPlatChunkHw* 	iChunk;
	DPlatChunkHw* 	iSecureChunk;
	TBool 			iWsSwitchOnScreen;
 	TBool 			iSecureDisplay;

	TDfc 			iPowerUpDfc;
	TDfc 			iPowerDownDfc;

	TVideoInfoV01 	iSecureVideoInfo;
	NFastMutex 		iLock;				// protects against being preempted whilst manipulating iVideoInfo/iSecureVideoInfo
	TPhysAddr 		ivRamPhys;
	TPhysAddr 		iSecurevRamPhys;

	TBool 			iBacklightOn;
	TInt 			iContrast;
	TInt 			iBrightness;
	TUint 			iInitialMode;


	TLcdUserCallBack * iAppCallBk[2];

public:
	TMessageQue 	iMsgQ;
	TDfcQue* 		iDfcQ;
	TVideoInfoV01 	iVideoInfo;
	TInt			iSize;
	TPhysAddr		iCompositionPhysical;

    static   DLcdPowerHandler 	* pLcd;

	};


//#define _GCE_NAVIDISPLAY_DEBUG

#ifdef _GCE_NAVIDISPLAY_DEBUG

#define  __GCE_DEBUG_PRINT(a) 		Kern::Printf(a)
#define  __GCE_DEBUG_PRINT2(a,b) 	Kern::Printf(a,b)

#else

#define  __GCE_DEBUG_PRINT(a)
#define  __GCE_DEBUG_PRINT2(a,b)

#endif


#endif //__LCDGCE_H__
