/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Audio Stubs -  Implementation of the DevSound utilities.
*
*/



// INCLUDE FILES
#include <e32std.h>
#include <barsc.h>
#include <barsread.h>
#include <mmfbase.h>
#include <mmfplugininterfaceuids.hrh>
#include <mmf/common/mmfcontroller.h> //needed for CleanupResetAndDestroyPushL()
#include <fixedsequence.rsg>
#include "DevSoundUtility.h"

_LIT(KFixedSequenceResourceFile, "Z:\\Resource\\DevSound\\FixedSequence.rsc");

// CONSTANTS
const TUint K4ByteSeq = 4;
const TInt KFourCCStringLength = 9;

inline TMMFRawPackage::TMMFRawPackage(TInt aDerivedSize)
#pragma warning( disable : 4355 )
// 'this' : used in base member initializer list
: iThis((TUint8*)this,aDerivedSize,aDerivedSize)
#pragma warning( default : 4355 )
    {
    }

// -----------------------------------------------------------------------------
// TMMFRawPackage::Package
// Returns a reference to descriptor pointer.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
inline TPtr8& TMMFRawPackage::Package()
    {
    ((TMMFRawPackage*)this)->iThis.Set((TUint8*)this,
                                       iThis.Length(),
                                       iThis.MaxLength());
    return iThis;
    }

// -----------------------------------------------------------------------------
// TMMFRawPackage::Package
// Returns a reference to descriptor pointer.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
inline const TPtr8& TMMFRawPackage::Package() const
    {
    ((TMMFRawPackage*)this)->iThis.Set((TUint8*)this,
                                       iThis.Length(),
                                       iThis.MaxLength());
    return iThis;
    }

// -----------------------------------------------------------------------------
// TMMFRawPackage::SetSize
// Sets the size of the raw package
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
inline void TMMFRawPackage::SetSize(TInt aDerivedSize)
    {
    iThis.Set((TUint8*)this,aDerivedSize,aDerivedSize);
    }

// -----------------------------------------------------------------------------
// TMMFToneFixedSequenceNames::TMMFToneFixedSequenceNames
// Default constructor
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
inline TMMFToneFixedSequenceNames::TMMFToneFixedSequenceNames()
    : TMMFRawPackage(sizeof(TMMFToneFixedSequenceNames))
    {
    }

// -----------------------------------------------------------------------------
// TNameBuf class definition
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
#ifdef _UNICODE
class TNameBuf : public TBufCBase16
#else
class TNameBuf : public TBufCBase8
#endif
    {
    friend class HMMFToneFixedSequenceNames;
    };

// -----------------------------------------------------------------------------
// HMMFToneFixedSequenceNames::HMMFToneFixedSequenceNames
// Default constructor
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
HMMFToneFixedSequenceNames::HMMFToneFixedSequenceNames()
    {
    iCount = 0;
    }

// -----------------------------------------------------------------------------
// HMMFToneFixedSequenceNames::AddNameL
// Append a copy of the supplied descriptor to the end of the
// current heap cell. This will involve a realloc that will normally
// result in the object moving
// -----------------------------------------------------------------------------
//
HMMFToneFixedSequenceNames*
HMMFToneFixedSequenceNames::AddNameL(const TDesC& aName)
    {
    TInt size = Package().Length();
    TInt desSize = aName.Size() + sizeof(TInt);
    if (desSize&3)
        {
        // Must round up to word boundary to keep aligned
        desSize = ((desSize+4)&(~3));
        }

    HMMFToneFixedSequenceNames* self =
        REINTERPRET_CAST(HMMFToneFixedSequenceNames*,
                         User::ReAllocL(STATIC_CAST(TAny*,this),
                                        size + desSize));
    TUint8* newDesPtr = REINTERPRET_CAST(TUint8*,self) + size;
    Mem::FillZ(newDesPtr,desSize);
    TNameBuf* newDes = REINTERPRET_CAST(TNameBuf*,newDesPtr);
    newDes->Copy(aName,aName.Length());
    self->SetSize(size+desSize);
    self->iCount++;
    return self;
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::CMMFDevSoundUtility
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CMMFDevSoundUtility::CMMFDevSoundUtility()
    {
    // No default implementation
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CMMFDevSoundUtility::ConstructL()
    {
    iFixedSequenceNames = new (ELeave) HMMFToneFixedSequenceNames;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CMMFDevSoundUtility* CMMFDevSoundUtility::NewL()
    {
    CMMFDevSoundUtility* self = NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CMMFDevSoundUtility* CMMFDevSoundUtility::NewLC()
    {
    CMMFDevSoundUtility* self = new(ELeave) CMMFDevSoundUtility();
    CleanupStack::PushL(self);
    self->ConstructL();
    // Leave it on Cleanupstack
    return self;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::~CMMFDevSoundUtility
// Destructor
// -----------------------------------------------------------------------------
//
CMMFDevSoundUtility::~CMMFDevSoundUtility()
    {
    delete iInfo;
    delete iFixedSequenceNames;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::SeekUsingFourCCLC
// Finds the ECom plugins based on FourCC.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundUtility::SeekUsingFourCCLC(
    TUid aInterfaceUid,
    RImplInfoPtrArray& aPlugInArray,
    const TFourCC& aSrcDataType,
    const TFourCC& aDstDataType,
    const TDesC& aPreferredSupplier)
    {
    // Create a match string using the two FourCC codes.
    _LIT8(KEmptyFourCCString, "    ,    ");
    TBufC8<9> fourCCString(KEmptyFourCCString);
    TPtr8 fourCCPtr = fourCCString.Des();
    TPtr8 fourCCPtr1(&fourCCPtr[0], 4);
    TPtr8 fourCCPtr2(&fourCCPtr[5], 4 );
    aSrcDataType.FourCC(&fourCCPtr1);
    aDstDataType.FourCC(&fourCCPtr2);

    // Create a TEcomResolverParams structure.
    TEComResolverParams resolverParams;
    resolverParams.SetDataType(fourCCPtr);
    resolverParams.SetWildcardMatch(EFalse);

    // ListImplementationsL leaves if it cannot find anything so trap the error
    // and ignore it.
    TRAPD(err, REComSession::ListImplementationsL(aInterfaceUid,
                                                  resolverParams,
                                                  aPlugInArray));

    // The error above may not be KErrNotFound eg could be KErrNoMemory in which
    // case leave
    User::LeaveIfError(err);

    // If there are no plugins, indicate failure
    if (aPlugInArray.Count() == 0)
        {
        User::Leave(KErrNotFound) ;
        }

    // If more than one match, narrow the search by preferred supplier
    if ((aPlugInArray.Count() > 1) && aPreferredSupplier.Length())
        {
        SelectByPreference( aPlugInArray, aPreferredSupplier ) ;
        }
    }


// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::SelectByPreference
// local function to disable items which do not match the preferred supplier.
// Note that at least one enabled item is returned (if there was an enabled item
// to begin with) which may not match the preferred supplier.
// -----------------------------------------------------------------------------
//
void CMMFDevSoundUtility::SelectByPreference(RImplInfoPtrArray& aPlugInArray,
                                             const TDesC& aPreferredSupplier)
    {

    // Use the Disabled flag to eliminated all currently enabled matches that
    // do not match the preferred supplier.
    TInt firstEnabled = -1 ; // to ensure that we return something valid
    TInt matchCount = 0 ;

    for ( TInt ii = 0 ; ii < aPlugInArray.Count() ; ii++ )
        {
        if (!(aPlugInArray[ii]->Disabled()))
            {
            if (firstEnabled == -1)
                {
                firstEnabled = ii;
                }

            if (aPlugInArray[ii]->DisplayName().FindF(aPreferredSupplier) ==
                KErrNotFound)
                {
                aPlugInArray[ii]->SetDisabled(ETrue) ;
                }
            else
                {
                matchCount++ ;
                }
            }
        }

    // If there are no matches then re-enable the first enabled
    if (matchCount == 0 )
        {
        aPlugInArray[firstEnabled]->SetDisabled(EFalse);
        }
    else if (matchCount > 1)
        {
        // find the latest version from more than one match
        TInt highestVersionIndex = -1;

        for (TInt ii = 0; ii < aPlugInArray.Count(); ii++)
            {
            // only interested in enabled elements
            if (!(aPlugInArray[ii]->Disabled()))
                {
                if (highestVersionIndex == -1)
                    {
                    // first match. Store this. Keep it enabled
                    highestVersionIndex = ii;
                    }
                else if (aPlugInArray[ii]->Version() >
                         aPlugInArray[highestVersionIndex]->Version())
                    {
                    // A new leader.  Disable the previous leader.
                    // Keep this one.
                    aPlugInArray[highestVersionIndex]->SetDisabled(ETrue);
                    highestVersionIndex = ii;
                    }
                else  // we already have a higher version.
                    aPlugInArray[ii]->SetDisabled(ETrue);
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::SeekHwDevicePluginsL
// This method looks for hwDevicePlugins that support the state given in aState
// which must be either EMMFStatePlaying or EMMFStateRecording.
// For each HwDevice plugin found the datatype as indicated by its fourCC code
// from the default_data field in the resource file is added to the array of
// aSupportedDataTypes
// -----------------------------------------------------------------------------
//
void CMMFDevSoundUtility::SeekHwDevicePluginsL(
    RArray<TFourCC>& aSupportedDataTypes,
    TMMFState aState)
    {
    //check argument precondition for aState
    if ((aState != EMMFStatePlaying) && (aState != EMMFStateRecording))
        {
        User::Leave(KErrArgument);
        }

    //clear any existing data in aSupportedDataTypes array
    aSupportedDataTypes.Reset();

    // Array to return hw device plugin resource info(place on cleanupstack
    // _after_ ListImplementationsL() )
    RImplInfoPtrArray plugInArray;
    TUid KUidMmfHWPluginInterfaceCodec = {KMmfUidPluginInterfaceHwDevice};

    // ListImplementationsL leaves if it cannot find anything so trap the error
    TRAPD(err, REComSession::ListImplementationsL(KUidMmfHWPluginInterfaceCodec,
                                                  plugInArray));
    CleanupResetAndDestroyPushL(plugInArray);

    TUint numberOfHwDevicePlugins = plugInArray.Count();

    // if no errors and have hwdevice plugin resource entries then scan entries
    // matching on a datatype of pcm16 as the destination datatype for play and
    // the source datatype for record. If a match is found and isn't already in
    // the list of supported data types, then add it to the list
    if ((err == KErrNone) && (numberOfHwDevicePlugins))
        {
        CImplementationInformation* hwDeviceResourceEntry = NULL;
        _LIT8(KPCM16FourCCString, " P16");
        TBufC8<KFOURCCLENGTH> fourCCStringPCM16(KPCM16FourCCString);
        TPtr8 fourCCPtrPCM16 = fourCCStringPCM16.Des();
        TUint entryNumber = 0;

        // check each resource entry for dst 4CC = P16 for play and
        // src 4CC = P16 for record
        for (TUint hwDeviceEntry = 0;
             hwDeviceEntry < numberOfHwDevicePlugins;
             hwDeviceEntry++)
            {
            hwDeviceResourceEntry = plugInArray[hwDeviceEntry];

            if (IsDataTypeMatch(hwDeviceResourceEntry, fourCCPtrPCM16, aState))
                {
                // resource entry data field has dest/src datatype ' P16'
                // i.e. pcm16 for play/record
                TPtrC8 fourCCPtr(0,0);

                if (aState == EMMFStatePlaying)
                    {
                    // datatype supported 4CC is left 4 chars
                    fourCCPtr.Set(
                    hwDeviceResourceEntry->DataType().Left(KFOURCCLENGTH));
                    }
                else if (aState == EMMFStateRecording)
                    {
                    // datatype supported 4CC is right 4 chars
                    fourCCPtr.Set(
                    hwDeviceResourceEntry->DataType().Right(KFOURCCLENGTH));
                    }

                TFourCC fourCCEntry(fourCCPtr);
                //need to check if entry already exists to prevent
                // duplicate entries
                TBool alreadyExists = EFalse;

                for (TUint fourCCEntryNumber = 0;
                     fourCCEntryNumber < entryNumber;
                     fourCCEntryNumber++)
                    {
                    if (aSupportedDataTypes[fourCCEntryNumber]==fourCCEntry)
                        {
                        // we already have this 4CC in the supported data types
                        alreadyExists = ETrue;
                        break;
                        }
                    }
                if (!alreadyExists)
                    {
                    err = aSupportedDataTypes.Append(fourCCEntry);
                    if (err)
                        {
                        // note: we don't destroy array because we don't own it
                        // but we do reset it as it is incomplete
                        aSupportedDataTypes.Reset();
                        User::Leave(err);
                        }
                    }
                }
            }
        }
    else
        {
        // if an error occured and not KErrNotFound then must be a 'real' error
        // e.g. KErrNoMemory
        if ((err != KErrNotFound) && (err != KErrNone))
            {
            User::Leave(err);
            }
        }

    CleanupStack::PopAndDestroy(&plugInArray);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::IsDataTypeMatch
// This method takes a given resource entry from a hardware device and
// determines whether the hwdevice plugin is a data type match for playing or
// recording depending on the setting of aState
// The method matchs the default_data field from the hw device resource entry
// matching it with the aHwMatchFourCC code.
// -----------------------------------------------------------------------------
//
TBool CMMFDevSoundUtility::IsDataTypeMatch(
    CImplementationInformation* aHwDeviceResourceEntry,
    const TDesC8& aHwMatchFourCC,
    TMMFState aState)
    {
    TBool match = EFalse;
	// extra length safety check to remove adapter plugins and incorrect ones
	if (aHwDeviceResourceEntry->DataType().Length()>=KFourCCStringLength)
		{
        if (aState == EMMFStatePlaying)
            {
            //play need to match with the right four characters
            match =
            (!(aHwMatchFourCC.Match(
                aHwDeviceResourceEntry->DataType().Right(KFOURCCLENGTH)) ==
                KErrNotFound));
            }
        else if (aState == EMMFStateRecording)
            {
            //record need to match with the left four characters
            match =
              (!(aHwMatchFourCC.Match(
                aHwDeviceResourceEntry->DataType().Left(KFOURCCLENGTH)) ==
                KErrNotFound));
            }
        }
    return match;
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::InitializeFixedSequenceL
// Populate fixed sequences
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CMMFDevSoundUtility::InitializeFixedSequenceL(
    CPtrC8Array** aFixedSequences)
    {

    RFs fsSession;
    User::LeaveIfError(fsSession.Connect());
    CleanupClosePushL(fsSession);

    // Open the resource file
    RResourceFile resourceFile;
    resourceFile.OpenL(fsSession, KFixedSequenceResourceFile);
    CleanupClosePushL(resourceFile);

    // Allocate buffer to hold resource data in binary format
    iInfo = resourceFile.AllocReadL(FIXED_TONE_SEQUENCE);

    TResourceReader reader;
    reader.SetBuffer(iInfo);

    // Create array to hold fixed sequences data
    CPtrC8Array* tempSequences = new(ELeave) CPtrC8Array(8); //  granularity
    CleanupStack::PushL(tempSequences);

    // First word gives number of entries
    TInt numberOfEntries = reader.ReadUint16();
    ASSERT(!(numberOfEntries&1)); // Should have atleast one entry

    // There must be an even number entries as each sequence structure is made
    // of a name string and a data string (SEQUENCE_NAME and SEQUENCE_DATA)

    HMMFToneFixedSequenceNames* names = new (ELeave) HMMFToneFixedSequenceNames;
    CleanupStack::PushL(names);

    for (TInt i = 0; i < numberOfEntries; i += 2)
        {
        // Copy name from resource array to returnable array
        HMMFToneFixedSequenceNames* newNames =
            names->AddNameL(reader.ReadTPtrC());

        if (names != newNames)
            {
            // May have moved so fixup cleanupstack reference
            CleanupStack::Pop(names);
            names = newNames;
            CleanupStack::PushL(names);
            }

        TInt len = reader.ReadUint16();
        TPtrC8 tempTPtrC8(REINTERPRET_CAST(const TUint8*,reader.Ptr()),len<<1);
        tempSequences->AppendL(tempTPtrC8);
        reader.Advance(len<<1);
        }

    CleanupStack::Pop(names);

    // Delete the old fixed sequence names
    delete iFixedSequenceNames;
    iFixedSequenceNames = NULL;
    iFixedSequenceNames = names;

    *aFixedSequences = tempSequences;
    CleanupStack::Pop(tempSequences);
    CleanupStack::PopAndDestroy(&resourceFile);
    CleanupStack::PopAndDestroy(&fsSession);
    }

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::RecognizeSequence
// Recognizes tone sequence.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TBool CMMFDevSoundUtility::RecognizeSequence(const TDesC8& aData)
	{
	TBool ret = EFalse;
	if (aData.Length() > K4ByteSeq)
		{
		//Check for Smart Message (OTA) formats
		if ((aData[0x000] == 0x02) &&
            (aData[0x001] == 0x4a) &&
            (aData[0x002] == 0x3a))
            {
			ret = ETrue;
            }
		else if ((aData[0x000] == 0x03) &&
                 (aData[0x001] == 0x4a) &&
                 (aData[0x002] == 0x44) &&
                 (aData[0x003] == 0x3a))
            {
			ret = ETrue;
            }
		else if ((aData[0] == 0x00) &&
                 (aData[1] == 0x11)) // Check for Nokia Ring Tone format
            {
		    ret = ETrue;
            }
		}
	return ret;
	}

// -----------------------------------------------------------------------------
// CMMFDevSoundUtility::FixedSequenceName
// Returns a descriptor reference containing name of the fixed sequence.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
const TDesC& CMMFDevSoundUtility::FixedSequenceName(TInt aSequenceNumber)
    {
    ASSERT(iFixedSequenceNames); // Defect if not true when previous was true
    ASSERT((aSequenceNumber>=0) &&
           (aSequenceNumber<iFixedSequenceNames->iCount));

    // Ptr to first descriptor
    TUint8* ptr = REINTERPRET_CAST(TUint8*,
                                   &(iFixedSequenceNames->iCount))+sizeof(TInt);
    TDesC* desPtr = REINTERPRET_CAST(TDesC*,ptr); // First des

    while (aSequenceNumber--)
        {
        TInt size = desPtr->Size();
        if (size&3)
            {
            size = ((size+4)&(~3));
            }

        ptr += sizeof(TInt) + size;
        desPtr = REINTERPRET_CAST(TDesC*,ptr); // Next des
        }

    return *desPtr;
    }

//  End of File
