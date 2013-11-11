/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
 * Description:Supl Start
 *
 */

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplstart.h"
#include "cstubsuplstart.h"
#include "lbs/epos_comasuplsetcapabilities.h"
#include "epos_comasupllocationid.h"

// -----------------------------------------------------------------------------
// CStubSuplStart::NewL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplStart* CStubSuplStart::NewL()
    {
    CStubSuplStart* self = new (ELeave) CStubSuplStart;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::~CStubSuplStart
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplStart::~CStubSuplStart()
    {

    }
// -----------------------------------------------------------------------------
// CStubSuplStart::EncodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
HBufC8* CStubSuplStart::EncodeL(TInt& aErrorCode)
    {
    aErrorCode = 0;
    HBufC8* encodedBuffer = NULL;
    InitializeL();
    // Do actual encoding here and return encoded buffer and put appropriate error code in aErrorCode
    return encodedBuffer;
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::CStubSuplStart
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
CStubSuplStart::CStubSuplStart()
    {
//delete all member variables here...
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::ConstructL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplStart::ConstructL()
    {
    COMASuplStart::ConstructL();
    //Allocate memmory for internal usage....
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::InitializeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//   
void CStubSuplStart::InitializeL()
    {
	
    if(iSETCapabilities) //This parameter is mandatory in asn
        {
        InitializeSetCapabilities();
        }
    if(iLocationId)   //This parameter is mandatory in asn
        {
        InitializeLocationId();
        }
    if (iQopSet)   //This parameter is optional in asn
        {
        InitializeQop();
        }
    if(ieCID)   //This parameter is optional in asn
        {
        InitializeECellId();
        }
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::InitializeSetCapabilities
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplStart::InitializeSetCapabilities()
    {
    TOMASuplPosTechnology lPosTechnology;
    COMASuplSETCapabilities::TOMASuplPrefMethod    lPrefMethod;
    TOMASuplPosProtocol   lPosProtocol;

    iSETCapabilities->GetSETCapabilities(lPosTechnology,lPrefMethod,lPosProtocol);

    TBool lSETAssist;
    TBool lGpsSETBased;
    TBool lAuto;
    TBool lFLT;
    TBool leOTD;
    TBool loTDOA;

    lPosTechnology.GetPosTechnology(lSETAssist,lGpsSETBased,lAuto,
            lFLT,leOTD,loTDOA);

    //Initialize structure accordingly from here...
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::InitializeLocationId
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplStart::InitializeLocationId()
    {
    COMASuplLocationId::TOMASuplCellInfoType lChoice = iLocationId->SuplCellInfoType();

    switch(lChoice)
        {
        case COMASuplLocationId::EGSM :
            {
            COMASuplGSMCellInfo* lCellInfo = NULL;
            COMASuplLocationId::TOMASuplStatus lStatus;
            iLocationId->SuplLocationId(lCellInfo,lStatus);
            if(lCellInfo)
                {
                TInt lRefMNC;
                TInt lRefMCC;
                TInt lRefLAC;
                TInt lRefCI;
                lCellInfo->SuplGSMCellInfo(lRefMNC,lRefMCC,lRefCI,lRefLAC);
                
                //Use lRefMNC,lRefMCC,lRefCI,lRefLAC for filling asn structs for encoding.
                }
                break;
            }
        case COMASuplLocationId::EWCDMA:
            {
            COMASuplCellInfo* lCellInfo = NULL;
            COMASuplLocationId::TOMASuplStatus lStatus;
            iLocationId->SuplLocationId(lCellInfo,lStatus);
            if(lCellInfo)
                {
                TInt lRefMNC;
                TInt lRefMCC;
                TInt lRefUC;
                lCellInfo->SuplCellInfo(lRefMNC,lRefMCC,lRefUC);
                if((KMncMin<=lRefMNC && lRefMNC<=KMncMax) && (KMccMin<=lRefMCC && lRefMCC<=KMccMax) &&
                        (KRefUcMin<=lRefUC && lRefUC<=KRefUcMax))
                    {
                    //Initialize  
                     //Use RefMNC,lRefMCC,lRefUC for filling asn structs for encoding.
                    }
                }
            else
                {
                //error    
                }
								break;
            }
        }
    //Initialize structure accordingly from here...
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::InitializeQop
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplStart::InitializeQop()
    {
    TInt horizAccuracy = iSuplQop.SuplQop();
    if(KHorizAccuracyMin <= horizAccuracy && horizAccuracy <= KHorizAccuracyMax)
        {
        //initialize here
        }
    else
        {
        //error...return with error 
        }

    TInt vertAccuracy;  
    TInt error = iSuplQop.VerticalAccuracy(vertAccuracy);
    if(!error)
        {
        if(KVertAccuracyMin <= vertAccuracy && vertAccuracy <= KVertAccuracyMax)
            {
            //initialize
            }
        else
            {
            //return error
            
            }
        }
    else
        {
        //initialize
        }

    TInt maxLocAge; 
    error = iSuplQop.MaxLocationAge(maxLocAge);
    if(!error)
        {
        //initialize
        if(KMaxLocationAgeMin <= maxLocAge && maxLocAge <= KMaxLocationAgeMax)
            {
            //initialize
            }
        else
            {
            //error
            return; 
            }
        }
    else
        {
        //initialize
        }

    TInt delay; 
    error = iSuplQop.Delay(delay);
    if(!error)
        {
        if(KDelayMin <= delay && delay <= KDelayMax)
            {
            //initialize
            }
        else
            {
            //error
            return; 
            }
        }
    else
        {
        //initialize
        }
    //Initialize structure accordingly...
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::InitializeECellId
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplStart::InitializeECellId()
    {
    TOMASuplNwMode nwMode = iMmCellInfo.NetworkMode();

    if (nwMode == EOMASuplGSM)
        {
        InitializeGSMECellId();
        }
    else
        {
        InitializeWCDMAECellId();
        }
    //Initialize structure accordingly...
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::InitializeGSMECellId
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplStart::InitializeGSMECellId()
    {
    	// In E-Cell id structs, if any of the data field contains -1 as a value, it means that no data is available for that field.
    TOMASuplGSMCellinfo  omaSuplGSMCellinfo;
    iMmCellInfo.GSMCellInfo(omaSuplGSMCellinfo);
    if(KOMASuplOptionalParamNotSet != omaSuplGSMCellinfo.iTA)
        {
        if(KTAMin <= omaSuplGSMCellinfo.iTA && omaSuplGSMCellinfo.iTA <= KTAMax)
            {
            //Initialize
            }
        else
            {
            //error
            }
        }       
    else
        {
        }

    TOMASuplNmr Nmr;
    TInt ARFCN;
    TInt BSIC;
    TInt RxLEV;
    
    for(TInt i = 0; i < KOMASUPLMaxNmrAmount; i++)
        {
        Nmr = omaSuplGSMCellinfo.iNmr[i];
        ARFCN = Nmr.iARFCN; // 0 to 1023  
        BSIC = Nmr.iBSIC;   // 0 to 63    
        RxLEV = Nmr.iRxLEV; // 0 to 63   
         
        if(ARFCN == KOMASuplOptionalParamNotSet && BSIC == KOMASuplOptionalParamNotSet &&  RxLEV ==  KOMASuplOptionalParamNotSet ) 
            {
            //initialize
            }

        if(KARFCNMin <= ARFCN && ARFCN <= KARFCNMax && 
                KBSICMin  <= BSIC  && BSIC  <= KBSICMax  &&
                KRxLEVMin <= RxLEV && RxLEV <= KRxLEVMax)
            {
            //initialize
            }
        else
            {
            //error
            }   

        }
    }
// -----------------------------------------------------------------------------
// CStubSuplStart::InitializeWCDMAECellId
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplStart::InitializeWCDMAECellId()
    {
    	// In E-Cell id structs, if any of the data field contains -1 as a value, it means that no data is available for that field.
    TOMASuplWCDMACellinfo  omaSuplWCDMACellinfo;
    iMmCellInfo.WCDMACellInfo(omaSuplWCDMACellinfo);
    if(KOMASuplOptionalParamNotSet != omaSuplWCDMACellinfo.iPrimaryScrambilingCode) // means PrimaryScrambilingCode is filled
        {
        if(KPrimaryScrambilingCodeMin <= omaSuplWCDMACellinfo.iPrimaryScrambilingCode && omaSuplWCDMACellinfo.iPrimaryScrambilingCode <= KPrimaryScrambilingCodeMax)
            {
            //Initialize
            }
        else
            {
            //error
            }
        }       
    else
        {
        //error
        }
    
    FillFrequencyInfo(omaSuplWCDMACellinfo.iFrequencyInfo);
            
    FillMeasuredResultsList(omaSuplWCDMACellinfo.iNwkMeasureReport);   
    }

TInt CStubSuplStart::FillFrequencyInfo(TOMASuplFrequencyInfo& aOMASuplFrequencyInfo)
    {
    TInt    FddUL = aOMASuplFrequencyInfo.iFddUL;       // 0..16383     In case of fdd( iFddUL, iFddDL),
    TInt    FddDL = aOMASuplFrequencyInfo.iFddDL;       // 0..16383     iFddUL is optional while iFddDL is mandatory. 
    TInt    TddDt = aOMASuplFrequencyInfo.iTddNt;       // 0..16383     

    TInt retError = KErrNone;   

    if(FddDL != KOMASuplOptionalParamNotSet ) //means FddDL is provided...
    {
    if(KFddDLMin <= FddDL && FddDL <= KFddDLMax)
        {
        //Initialize here..
        }
    else
        {
        //error
        }
    }
    else
        {
            //error
        }

    FillFrequencyInfoULInfo(FddUL,retError);

    FillFrequencyInfoDT(TddDt,retError);

    return retError;
    }

void CStubSuplStart::FillFrequencyInfoULInfo(TInt aFddUL,TInt& aRetErrorCode)
    {
    if(aFddUL != KOMASuplOptionalParamNotSet && aRetErrorCode == KErrNone )
        {
        if(KFddULMin <= aFddUL && aFddUL <= KFddULMax)
            {
            //set to message
              aRetErrorCode = KErrNone;
            }
        else
            {     
            //error   
            }
        }                                                                       
    }

void CStubSuplStart::FillFrequencyInfoDT(TInt aTddDt,TInt& aRetErrorCode)
    {
    if(aTddDt != KOMASuplOptionalParamNotSet ) //means TddDt is provided...
        {
        if(KTddDtMin <= aTddDt && aTddDt <= KTddDtMax)
            {
            //Initialize
            }
        else
            {
            //error
            }
        }
    else
        {
         //error
        }
    aRetErrorCode = KErrNone;
    }

TInt CStubSuplStart::FillMeasuredResultsList(TOMASuplNetworkMeasureReport aNwkMeasureReport[])
    {
    
    
    for(TInt i = 0; i < KOMASUPLMaxNetworkMeasureReports; i++ )
        {
        TOMASuplNetworkMeasureReport NwkMeasureReport = aNwkMeasureReport[i];
        TInt err = FillFrequencyInfo(NwkMeasureReport.iFrequencyInfo);                

        if(KOMASuplOptionalParamNotSet != NwkMeasureReport.iCarrierRSSI) // means iCarrierRSSI is filled
            {
            if(KCarrierRSSIMin <= NwkMeasureReport.iCarrierRSSI && NwkMeasureReport.iCarrierRSSI <= KCarrierRSSIMax)
                {  
                //initialize
                }
            else
                {
                //error
                }
            }       

        err = FillCellMeasuredResult(NwkMeasureReport.iCellMeasuredResult);               
        }
    return 0;
    //Own error code can be set...
    }

TInt CStubSuplStart::FillCellMeasuredResult(TOMASuplCellMeasuredResult aCellMeasuredResult[])
    {

    TInt eleCount = 0;
    for(TInt i = 0; i < KOMASUPLMaxCellMeasuredResults; i++ )
        {
        TOMASuplCellMeasuredResult  CellMeasuredResult = aCellMeasuredResult[i];
        TInt err = FillModeSpecificInfo(CellMeasuredResult);
        if( err == KErrNone)
            {
            //Initialize

            if(CellMeasuredResult.iCID != KOMASuplOptionalParamNotSet) // CID is present
                {
                if(KCIDMin <= CellMeasuredResult.iCID && CellMeasuredResult.iCID <= KCIDMax)
                    {
                    //Set to message                                
                    }
                else
                    {
                       //error
                    }
                }
            else
                {
                    //error
                }
            eleCount++;
            }           
        }


    if(eleCount != 0 )
        return KErrNone;
    else
        return KErrArgument; 
    }

TInt CStubSuplStart::FillModeSpecificInfo(TOMASuplCellMeasuredResult&  aCellMeasuredResult)
    {

    TOMASuplFddInfo     FddInfo = aCellMeasuredResult.iFddInfo;
    TOMASuplTddInfo     TddInfo = aCellMeasuredResult.iTddInfo;

    if(FddInfo.iPrimaryCPICH != KOMASuplOptionalParamNotSet) // iPrimaryCPICH is present
        {

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////              
        if(KPrimaryCPICHMin <= FddInfo.iPrimaryCPICH && FddInfo.iPrimaryCPICH <= KPrimaryCPICHMax)
            {
            //Set to message    
            //initialize
            }
        else
            {
               //error
            }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                  

        if(FddInfo.iCpichEcN0 != KOMASuplOptionalParamNotSet) // iCpichEcN0 is present
            {
            if(KCpichEcN0Min <= FddInfo.iCpichEcN0 && FddInfo.iCpichEcN0 <= KCpichEcN0Max)
                {
                //Set to message                                
                }
            else
                {
                    //error
                }
            }       
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                              

        if(FddInfo.iCichRscp != KOMASuplOptionalParamNotSet) // iCichRscp is present
            {
            if(KCichRscpMin <= FddInfo.iCichRscp && FddInfo.iCichRscp <= KCichRscpMax)
                {
                //Set to message                                
                }
            else
                {
                    //error
                }
            }       
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                  

        if(FddInfo.iPathloss != KOMASuplOptionalParamNotSet) // iPathloss is present
            {
            if(KPathlossMin <= FddInfo.iPathloss && FddInfo.iPathloss <= KPathlossMax)
                {
                //Set to message                                
                }
            else
                {
                    //error
                }
            }       
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                  
        return KErrNone;            
        }
    else
        {
            //error
        }

    if(TddInfo.iCellParamID != KOMASuplOptionalParamNotSet) // iCellParamID is present
        {
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                 
        if(KCellParamIDMin <= TddInfo.iCellParamID && TddInfo.iCellParamID <= KCellParamIDMax)
            {
            //Set to message
            }
        else
            {
                //error
            }
           
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                                         

        if(TddInfo.iProposedTGSN != KOMASuplOptionalParamNotSet) // iProposedTGSN is present
            {
            if(KProposedTGSNMin <= TddInfo.iProposedTGSN && TddInfo.iProposedTGSN <= KProposedTGSNMax)
                {
                //Set to message    
                }
            else
                {
                    //error
                }
            }       

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                                         

        if(TddInfo.iPrimaryCcpchRscp != KOMASuplOptionalParamNotSet) // iPrimaryCcpchRscp is present
            {
            if(KPrimaryCcpchRscpMin <= TddInfo.iPrimaryCcpchRscp && TddInfo.iPrimaryCcpchRscp <= KPrimaryCcpchRscpMax)
                {
                //Set to message           
                }
            else
                {
                    //error
                }
            }       

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                                         

        if(TddInfo.iPathloss != KOMASuplOptionalParamNotSet) // iPathloss is present
            {
            if(KPathlossMin <= TddInfo.iPathloss && TddInfo.iPathloss <= KPathlossMax)
                {
                //Set to message                                
                }
            else
                {
                   //error
                }
            }       
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                  
        TInt eleCount = 0; 
        for(TInt i = 0; i < KOMASUPLMaxTimeSlotIscpAmount; i++)
            {
            if(TddInfo.iTimeslotISCP[i] != KOMASuplOptionalParamNotSet && KTimeslotISCPMin <= TddInfo.iTimeslotISCP[i] && TddInfo.iTimeslotISCP[i] <= KTimeslotISCPMax)
                {
                //Set to message                                
                //Set to message that its not optional
                eleCount++;
                }
            else
                {
                    //error
                }

            }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                  
        //          
        return KErrNone;
        }
    else
        {
            //error
        }
    return 0;
    //Own error code can be set...
    }
