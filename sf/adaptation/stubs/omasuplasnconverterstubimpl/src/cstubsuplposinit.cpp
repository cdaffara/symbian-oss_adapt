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
 * Description: Supl PosInit
 *
 */

#include <e32base.h>
#include <e32def.h> 
#include <e32cmn.h> 
#include "epos_comasuplasnbase.h"
#include "epos_comasuplasnmessagebase.h"
#include "epos_comasuplposinit.h"
#include "cstubsuplposinit.h"
#include "cstubsuplstart.h"
#include "lbs/epos_comasuplreqasstdata.h"
#include "lbs/epos_comasuplposition.h"
#include "lbs/epos_comasuplvelocity.h"
#include "lbs/epos_comasuplsetcapabilities.h"
#include "lbs/epos_comasuplpospayload.h"
#include "epos_comasuplpos.h"
#include "epos_comasuplsessionid.h"
#include "epos_comasupllocationid.h"

// -----------------------------------------------------------------------------
// CStubSuplPosInit::NewL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplPosInit* CStubSuplPosInit::NewL()
    {
    CStubSuplPosInit* self = new (ELeave) CStubSuplPosInit;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CStubSuplPosInit::~CStubSuplPosInit
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
EXPORT_C CStubSuplPosInit::~CStubSuplPosInit()
    {

    }
// -----------------------------------------------------------------------------
// CStubSuplPosInit::ConstructL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplPosInit::ConstructL()
    {
    COMASuplPosInit::ConstructL();
    //Allocate memory for internal usage....
    }
// -----------------------------------------------------------------------------
// CStubSuplPosInit::EncodeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
HBufC8* CStubSuplPosInit::EncodeL(TInt& aErrorCode)
    {
    aErrorCode = 0;
    HBufC8* encodedBuffer = NULL;
    InitializeL();
    // Do actual encoding here and return encoded buffer and put appropriate error code in aErrorCode
    return encodedBuffer;
    }
// -----------------------------------------------------------------------------
// CStubSuplPosInit::CStubSuplPosInit
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
CStubSuplPosInit::CStubSuplPosInit()
    {
    }
// -----------------------------------------------------------------------------
// CStubSuplPosInit::InitializeL
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplPosInit::InitializeL()
    {
    if (iReqAsstData)  //This parameter is optional in asn
        {
    InitializeRequestData();
        }
    if (iPosition)  //This parameter is optional in asn
        {
    InitializePosition();
        }
    if (iPosPayload)  //This parameter is optional in asn
        {
    InitializePosPayLoad();
        }
    if (iSETCapabilities)  //This parameter is mandatory in asn
        {
    InitializeSetCapabilities();
        }
    if (iLocationId)  //This parameter is mandatory in asn
        {
    InitializeLocationId();
        }
    if (ieCID)  //This parameter is optional in asn
        {
    InitializeECellId();
        }
    }
void CStubSuplPosInit::InitializeRequestData()
    { 
    TBool almanac;
    TBool utc;
    TBool ionos;
    TBool dgps;
    TBool ref;
    TBool refTime;
    TBool acq;
    TBool realTime; 
    iReqAsstData->GetReqAsstData(almanac,utc,ionos,dgps,ref,refTime,acq,realTime);
    //Initialize structure here...
    }
void CStubSuplPosInit::InitializePosition()
    {
    // Retrieve mandatory parametrs of Position
    // Retrieve mandatory parametrs of Position
    TOMASuplUtcTime utcTime;
    TOMASuplPositionEstimate posEstimate;

    iPosition->GetPosition(utcTime,posEstimate );

    TInt error = 0;

    //Initialize Time
    InitializeTime(utcTime);
    //InitializePositionEstimate
    
    InitializePositionEstimate(posEstimate);
    
    if(error)
        {
        return;
        }
    else
        {
        COMASuplVelocity* velocity = NULL;
        error = iPosition->GetVelocity(velocity);
        if(error)
            {
            
            }
        else
            {

            TOMASuplVelocityType velocityType = velocity->VelType();
            COMASuplHorizVelocity* horizVelocity = velocity->Velocity();
            if(horizVelocity)
                {
                switch(velocityType)
                    {
                    case EHorizVelocity:
                        {
                        //Initialize Horizontal velocity
                        IntializeHorizVelocity(horizVelocity);
                        break;      
                        }
                    case EHorizAndVertVelocity:
                        {
                        //Initialize Horizontal and Vertical velocity
                        IntializeHorizAndVertVelocity(horizVelocity);
                        break;
                        }
                    case EHorizUncertVelocity:
                        {
                        //Initialize Horizontal Uncert velocity
                        IntializeHorizUncertVelocity(horizVelocity);
                        break;
                        }
                    case EHorizAndVertUncertVelocity:
                        {
                        //Initialize Horizontal Vertical Uncert velocity
                        IntializeHorizVertUncertVelocity(horizVelocity);
                        break;  
                        }
                    default:
                        {
                        //error
                        return;         
                        }
                    }   
                }
            else
                {
                //error
                return; 
                }
            }
        }
    }

void CStubSuplPosInit::IntializeHorizVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;

    aVelocity->GetHorizVel(bearing,horSpeed);

    //Initialize bearing,horSpeed
    }

void CStubSuplPosInit::IntializeHorizAndVertVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;
    TUint8 verDirect;
    TUint8 verSpeed;
    
    COMASuplHorizAndVertVelocity* horizVertVel = (COMASuplHorizAndVertVelocity*)aVelocity;

    horizVertVel->GetHorizAndVertVel(bearing,horSpeed,verDirect,verSpeed);

    //Initialize bearing,horSpeed,verDirect,verSpeed
    }

void CStubSuplPosInit::IntializeHorizUncertVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;
    TUint8 uncertSpeed;

    COMASuplHorizUncertVelocity* horizUncertVel = (COMASuplHorizUncertVelocity*)aVelocity;

    horizUncertVel->GetHorizUncertVel(bearing,horSpeed,uncertSpeed);
    //Initialize bearing,horSpeed,uncertSpeed
    }

void CStubSuplPosInit::IntializeHorizVertUncertVelocity(const COMASuplHorizVelocity* aVelocity)
    {
    TUint16 bearing;
    TUint16 horSpeed;
    TUint8  verDirect;
    TUint8  verSpeed;
    TUint8  horizUncertSpeed;
    TUint8  vertUncertSpeed;

    COMASuplHorizAndVertUncertVelocity* horizVertUncertVel = (COMASuplHorizAndVertUncertVelocity*)aVelocity;

    horizVertUncertVel->GetHorizVertUncertVel(bearing,horSpeed,verDirect,verSpeed,
            horizUncertSpeed,vertUncertSpeed);
    //Initialize bearing,horSpeed,verDirect,verSpeed,horizUncertSpeed,vertUncertSpeed
    }

void CStubSuplPosInit::InitializePosPayLoad()
    {   
    // Retrieve mandatory parametrs of PosMessage
    COMASuplPosPayload::TOMASuplPosPayloadType payloadType;
    HBufC8* payloadData = NULL;

    iPosPayload->GetPosPayload(payloadData,payloadType);

    if(!payloadData)  
        {
        //error
        return; 
        }

    TPtr8 ptr = payloadData->Des();

    TInt len = payloadData->Length();

    const char* lTemp = (const char *)ptr.Ptr();


    switch(payloadType)
        {
        case COMASuplPosPayload::ETIA801:
            {
            //initialize pospayload for TIA   
            break;
            }   
        case COMASuplPosPayload::ERRC:
            {
            //initialize pospayload for RRC
            break;                      
            }
        case COMASuplPosPayload::ERRLP:
            {
            //initialize pospayload for RRLP
            break;                      
            }
        default:
            {
            //error
            return;
            }           
        }

    }
void CStubSuplPosInit::InitializeSetCapabilities()
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
// CStubSuplPosInit::InitializeLocationId
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplPosInit::InitializeLocationId()
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
                }
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
                    }
                }
            else
                {
                //error    
                }

            }
        }
    //Initialize structure accordingly from here...
    }

// -----------------------------------------------------------------------------
// CStubSuplPosInit::InitializeECellId
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplPosInit::InitializeECellId()
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
// CStubSuplPosInit::InitializeGSMECellId
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplPosInit::InitializeGSMECellId()
    {
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
                //error
                return;
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
            return;
            }   

        }
    }
// -----------------------------------------------------------------------------
// CStubSuplPosInit::InitializeWCDMAECellId
// Other Items were commented in a header
// -----------------------------------------------------------------------------
//
void CStubSuplPosInit::InitializeWCDMAECellId()
    {
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

TInt CStubSuplPosInit::FillFrequencyInfo(TOMASuplFrequencyInfo& aOMASuplFrequencyInfo)
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

    return 0;
    }

void CStubSuplPosInit::FillFrequencyInfoULInfo(TInt aFddUL,TInt& aRetErrorCode)
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

void CStubSuplPosInit::FillFrequencyInfoDT(TInt aTddDt,TInt& aRetErrorCode)
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

TInt CStubSuplPosInit::FillMeasuredResultsList(TOMASuplNetworkMeasureReport aNwkMeasureReport[])
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
    }

TInt CStubSuplPosInit::FillCellMeasuredResult(TOMASuplCellMeasuredResult aCellMeasuredResult[])
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

TInt CStubSuplPosInit::FillModeSpecificInfo(TOMASuplCellMeasuredResult&  aCellMeasuredResult)
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
    }

void CStubSuplPosInit::InitializePositionEstimate(TOMASuplPositionEstimate& aPosEstimate)
    {
    TInt error;

    // Retrieve mandatory parametrs of Position Estimate
    TOMASuplPositionEstimate::TOMASuplLatitudeSign  latSign;
    TInt               latitude;
    TInt               longitude;
    aPosEstimate.GetPositionEstimate(latSign,latitude,longitude);   

    // Set Position Estimate
    if(KLatitudeMin <= latitude && latitude <= KLatitudeMax)
        {
        //Initialize
        }
    else
        {
        //error
        }

    if(KLongitudeMin <= longitude && longitude <= KLongitudeMax)
        {
        //Initialize
        }
    else
        {
        //error
        }

    //Initialize

    // Retrieve optional parametrs of Position Estimate
    // Retrieve Uncertainity
    TOMASuplUncertainty uncertainty;
    error = aPosEstimate.GetUncertainty(uncertainty);
    if(!error)
        {    
        TInt uncertSemiMajor;
        TInt uncertSemiMinor;
        TInt orientMajorAxis;
        uncertainty.GetUncertainty(uncertSemiMajor,uncertSemiMinor,orientMajorAxis);
        if((KUncertSemiMajorMin <= uncertSemiMajor && uncertSemiMajor<= KUncertSemiMajorMax)
                && (KUncertSemiMinorMin <= uncertSemiMinor && uncertSemiMinor<= KUncertSemiMinorMax)
                && (KOrientMajorAxisMin <= orientMajorAxis && orientMajorAxis<= KOrientMajorAxisMax))
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
        //return error
        }

    // Retrieve Confidence
    TInt confidence;
    error = aPosEstimate.GetConfidence(confidence);
    if(!error)
        {

        if(KConfidenceMin <= confidence && confidence <= KConfidenceMax)
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
        //return error
        }

    // Retrieve AltitudeInfo
    TOMASuplAltitudeInfo altitudeInfo;
    error = aPosEstimate.GetAltitudeInfo(altitudeInfo);
    if(!error)
        {

        TOMASuplAltitudeInfo::TOMASuplAltitudeDirection altDirection;
        TInt altitude;
        TInt altUncert;
        altitudeInfo.GetAltitudeInfo(altDirection,altitude,altUncert);
        if((KAltitudeMin <= altitude && altitude<= KAltitudeMax) && (KAltitudeUncertMin <= altUncert && altUncert<= KAltitudeUncertMax))
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
        //return error
        }
    }

void CStubSuplPosInit::InitializeTime(const TOMASuplUtcTime aUtcTime)
    {
    
    TDateTime lDateTime;
    TInt      zoneCode;
    TInt      zone;
    
    aUtcTime.GetUtcTime(lDateTime,zoneCode,zone);
    
    TInt   year     = lDateTime.Year();
    TMonth month    = lDateTime.Month(); 
    TInt   day      = lDateTime.Day();
    TInt   hour     = lDateTime.Hour();
    TInt   minute   = lDateTime.Minute();
    TInt   second   = lDateTime.Second();
    TInt   microsec = lDateTime.MicroSecond();
    
    if(0 == zoneCode || 1 == zoneCode)
        {
        //Initialize 
        }
    else
        {
        //error
        }
    
    
    }
