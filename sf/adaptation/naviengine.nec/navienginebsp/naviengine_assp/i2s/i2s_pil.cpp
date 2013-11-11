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
* bsp\hwip_nec_navienegine\navienegine_assp\i2s\i2s_pil.cpp
*
*/



#include <kernel/kernel.h>
#include "navi_i2s.h"

#define CHANNEL_ID_FROM_INTERFACE_ID(aId)	(TUint16)aId		// channel ID on bottom 16 bits (as returned by Configuration Repository)

TInt TI2sManager::iChannelsNum = 0;
DI2sChannelBase** TI2sManager::iChannels = 0;

TInt TI2sManager::DoCreate()
	{
	TInt err=KErrNone;
	//		(TBD) Must call into ConfRep to obtain the number of channels present and store it in iChannelsNum and create a list of all interfaces supported.
	//		Then iterate through the list of interfaces and create channels.
	//		This way, each channel implementation can be written independently from the others, and brroght
	//		together at the end at the cost of a virtaul pointer dereferencing. (Incidentaly that is totally
	//		irrelevant for the NaviEngine implementation where all channels are implemented in a single file
	//		but we present this code as an example of how it could be done in a situation of need)
	
	iChannelsNum=4;		// TBD: for now, in future need to call into ConfRep

	TInt* chanArray=(TInt*)Kern::Alloc(iChannelsNum*sizeof(TInt));	// allocate a temporary array of interface Ids to populate with ConfRep data
	if(!chanArray)
		return KErrNoMemory;

	DI2sChannelBase** channels=(DI2sChannelBase**)Kern::Alloc(iChannelsNum*sizeof(DI2sChannelBase*));	// allocate an array of pointers to channels (stored in iChannels)
	if(!channels)
		return KErrNoMemory;

	for(TInt i=0; i<iChannelsNum;i++)	// TBD: for now, in future need to call into ConfRep to populate the array of interface Ids
		chanArray[i]=i;	// without ConfRep, interface Ids are just the channel numbers
	
	DI2sChannelBase* chan=NULL;
	for(TInt j=0;j<iChannelsNum;j++)
		{
		err=DI2sChannelBase::CreateChannels(chan,chanArray[j]);	// this creates the channel corresponding to the interface Id passed and return it in chan
		if(chan)
			channels[j]=chan;		// j is the channel number
		else
			{
			err=KErrNotFound;
			break;
			}
		}
	if(err==KErrNone)
		iChannels=channels;
	Kern::Free(chanArray);	// no longer need this
	return err;
	}

/**,
 Configures the interface.
 
 @param aInterfaceId	The interface Id.
 @param aConfig		A pointer to the configuration as one of TI2sConfigBufV01 or greater.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid or aConfig is NULL;
 KErrNotSupported, if the configuration is not supported by this interface;
 KErrInUse, if interface is not quiescient (a transfer is under way).
 */
EXPORT_C TInt I2s::ConfigureInterface(TInt aInterfaceId, TDes8* aConfig)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if((chanId >= TI2sManager::iChannelsNum) || !aConfig)
		return KErrArgument;
	 
	return TI2sManager::iChannels[chanId]->ConfigureInterface(aConfig);
	}

/**
 Reads the current configuration.
 
 @param aInterfaceId	The interface Id.
 @param aConfig		On return, the buffer passed is filled with the current configuration.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid.
 */
EXPORT_C TInt I2s::GetInterfaceConfiguration(TInt aInterfaceId, TDes8& aConfig)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->GetInterfaceConfiguration(aConfig);
	}

/**
 Sets the sampling rate.
 
 @param aInterfaceId	 The interface Id.
 @param aSamplingRate One of TI2sSamplingRate.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the sampling rate is not supported by this interface;
 KErrInUse, if interface is not quiescient (a transfer is under way).
 */
EXPORT_C TInt I2s::SetSamplingRate(TInt aInterfaceId, TI2sSamplingRate aSamplingRate)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->SetSamplingRate(aSamplingRate);
	}

/**
 Reads the sampling rate.
 
 @param aInterfaceId	 The interface Id.
 @param aSamplingRate On return, contains one of TI2sSamplingRate.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid.
 */
EXPORT_C TInt I2s::GetSamplingRate(TInt aInterfaceId, TInt& aSamplingRate)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->GetSamplingRate(aSamplingRate);
	}

/**
 Sets the frame length and format.

 @param aInterfaceId		  The interface Id.
 @param aFrameLength		  One of TI2sFrameLength.
 @param aLeftFramePhaseLength The length of the left frame phase (in number of data bits).

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the frame length or format are not supported by this interface;
 KErrInUse, if interface is not quiescient (a transfer is under way).
		   
 The implementation calculates the Right frame phase length as (FrameLength - LeftFramePhaseLength)
 */
EXPORT_C TInt I2s::SetFrameLengthAndFormat(TInt aInterfaceId, TI2sFrameLength aFrameLength, TInt aLeftFramePhaseLength)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->SetFrameLengthAndFormat(aFrameLength, aLeftFramePhaseLength);
	}

/**
 Reads the frame format.

 @param aInterfaceId			 The interface Id.
 @param aLeftFramePhaseLength  On return, contains the length of the left frame phase.
 @param aRightFramePhaseLength On return, contains the length of the right frame phase.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid.
 */
EXPORT_C TInt I2s::GetFrameFormat(TInt aInterfaceId, TInt& aLeftFramePhaseLength, TInt& aRightFramePhaseLength)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->GetFrameFormat(aLeftFramePhaseLength, aRightFramePhaseLength);
	}

/**
 Sets the sample length for a frame phase (left or right).
 
 @param aInterfaceId	 The interface Id.
 @param aFramePhase	 One of TI2sFramePhase.
 @param aSampleLength One of TI2sSampleLength.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the sample length for the frame phase selected is not supported by this interface;
 KErrInUse, if interface is not quiescient (a transfer is under way).
 */
EXPORT_C TInt I2s::SetSampleLength(TInt aInterfaceId, TI2sFramePhase aFramePhase, TI2sSampleLength aSampleLength)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->SetSampleLength(aFramePhase, aSampleLength);
	}

/**
 Reads the sample length for a frame phase (left or right).
 
 @param aInterfaceId	 The interface Id.
 @param aFramePhase	 One of TI2sFramePhase.
 @param aSampleLength On return, contains the sample length for the frame phase indicated by aFramePhase.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid.
 */
EXPORT_C TInt I2s::GetSampleLength(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt& aSampleLength)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->GetSampleLength(aFramePhase, aSampleLength);
	}

/**
 Sets the number of delay cycles for a frame phase (left or right).
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aDelayCycles The number of delay cycles to be introduced for the frame phase indicated by aFramePhase.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the number of delay cycles for the frame phase selected is not supported by this interface;
 KErrInUse, if interface is not quiescient (a transfer is under way).
 
 Each delay cycle has a duration of a bit clock cycle. Delay cycles are inserted between the start of the frame and the start of data.
 */
EXPORT_C TInt I2s::SetDelayCycles(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt aDelayCycles)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->SetDelayCycles(aFramePhase, aDelayCycles);
	}

/**
 Reads the number of delay cycles for a frame phase (left or right).
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aDelayCycles On return, contains the number of delay cycles for the frame phase indicated by aFramePhase.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid.
 */
EXPORT_C TInt I2s::GetDelayCycles(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt& aDelayCycles)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->GetDelayCycles(aFramePhase, aDelayCycles);
	}

/**
 Reads the receive data register for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aData		On return, contains the receive data register contents.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if reading the receive data register is not supported (e.g. when if DMA is enabled)
 KErrNotReady, if the interface is not ready.

 If the implementation has a combined receive/transmit register - half duplex operation only - this API is used to read from it.
 If the implementation only supports a single receive register for both frame phases, the aFramePhase argument shall be ignored and the 
 API shall return the contents of the single register. The user of the API shall use the ReadRegisterModeStatus() API to determine
 which frame phase the data corresponds to.
 */
EXPORT_C TInt I2s::ReadReceiveRegister(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt& aData)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->ReadReceiveRegister(aFramePhase, aData);
	}

/**
 Writes to the transmit data register for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aData		The data to be written.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if writing to the receive data register is not supported (e.g. when if DMA is enabled)
 KErrNotReady, if the interface is not ready.

 If the implementation has a combined receive/transmit register - half duplex operation only - this API is used to write to it.
 If the implementation only supports a single transmit register for both frame phases, the aFramePhase argument shall be ignored and the 
 API shall write to the single register. The user of the API shall use the ReadRegisterModeStatus() API to determine under which frame 
 phase the data corresponds will be transmitted.
 */
EXPORT_C TInt I2s::WriteTransmitRegister(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt aData)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->WriteTransmitRegister(aFramePhase, aData);
	}

/**
 Reads the transmit data register for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aData		On return, contains the transmit data register contents.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if reading the transmit data register is not supported;
 KErrNotReady, if the interface is not ready.

 If the implementation has a combined receive/transmit register this API is used to read from it (equivalent to ReadReceiveRegister()).
 If the implementation only supports a single transmit register for both frame phases, the aFramePhase argument shall be ignored and the 
 API shall return the contents of the single register. The user of the API shall use the ReadRegisterModeStatus() API to determine
 which frame phase the data corresponds to.
 */
EXPORT_C TInt I2s::ReadTransmitRegister(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt& aData)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->ReadTransmitRegister(aFramePhase, aData);
	}

/**
 Reads the Register PIO access mode status flags for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aFlags		On return, contains a bitmask with the status flags for the frame phase selected (see TI2sFlags).
 A bit set to "1" indicates the condition described by the corresponding flag is occurring.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if reading the status flags for Register PIO mode is not supported by this implementation.
 
 The client driver may use one of IS_I2s_<CONDITION> macros to determine the status of individual conditions.
 */
EXPORT_C TInt I2s::ReadRegisterModeStatus(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt& aFlags)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->ReadRegisterModeStatus(aFramePhase, aFlags);
	}

/**
 Enables Register PIO access mode related interrupts for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aInterrupt	A bitmask containing the relevant interrupt flags (see TI2sFlags).
 Bits set to "1" enable the corresponding interrupts.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if one of the selected interrupt conditions cannot be generated by this implementation.

 If the implementation only supports single transmit and receive registers for both frame phases, the aFramePhase argument is 
 ignored.
 */
EXPORT_C TInt I2s::EnableRegisterInterrupts(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt aInterrupt)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->EnableRegisterInterrupts(aFramePhase, aInterrupt);
	}

/**
 Disables Register PIO access mode related interrupts for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aInterrupt	A bitmask containing the relevant interrupt flags (see TI2sFlags).
 Bits set to "1" disable the corresponding interrupts.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if one of the selected interrupt conditions cannot be generated by this implementation.

 If the implementation only supports single transmit and receive registers for both frame phases, the aFramePhase argument is 
 ignored.
 */
EXPORT_C TInt I2s::DisableRegisterInterrupts(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt aInterrupt)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->DisableRegisterInterrupts(aFramePhase, aInterrupt);
	}

/**
 Reads the Register PIO access mode interrupt mask for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aEnabled		On return, contains a bitmask with the interrupts which are enabled for the frame phase selected (see TI2sFlags).
 A bit set to "1" indicates the corresponding interrupt is enabled.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if one of the selected interrupt conditions cannot be generated by this implementation.

 If the implementation only supports single transmit and receive registers for both frame phases, the aFramePhase argument is 
 ignored.
 */
EXPORT_C TInt I2s::IsRegisterInterruptEnabled(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt& aEnabled)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->IsRegisterInterruptEnabled(aFramePhase, aEnabled);
	}

/**
 Enables receive and/or transmit FIFO on a per frame phase basis.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aFifoMask	A bitmask specifying which FIFO direction(s) - receive and/or transmit - are to be enabled for the frame 
 phase selected (see TI2sDirection).
 Bits set to "1" enable the corresponding FIFO.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the implementation does no support FIFOs.
 
 If the implementation has a combined receive/transmit FIFO - half duplex operation only - then aFifoMask is ignored.
 If the implementation only supports a single FIFO for both frame phases then aFramePhase is ignored.
 */
EXPORT_C TInt I2s::EnableFIFO(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt aFifoMask)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->EnableFIFO(aFramePhase, aFifoMask);
	}

/**
 Disables receive and/or transmit FIFO on a per frame phase basis.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aFifoMask	A bitmask specifying which FIFO direction(s) - receive and/or transmit - are to be disabled for the frame 
 phase selected (see TI2sDirection).
 Bits set to "1" disable the corresponding FIFO.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the implementation does no support FIFOs.

 If the implementation has a combined receive/transmit FIFO - half duplex operation only - then aFifoMask is ignored.
 If the implementation only supports a single FIFO for both frame phases then aFramePhase is ignored.
 */
EXPORT_C TInt I2s::DisableFIFO(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt aFifoMask)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->DisableFIFO(aFramePhase, aFifoMask);
	}

/**
 Reads the enabled state of a frame phase's FIFOs.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aEnabled		On return, contains a bitmask indicating which FIFOs which are enabled for the frame phase selected (see TI2sDirection).
 A bit set to "1" indicates the corresponding FIFO is enabled.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the implementation does no support FIFOs.

 If the implementation has a combined receive/transmit FIFO - half duplex operation only - then aEnabled will have 
 both Rx and Tx bits set when the FIFO is enabled.
 If the implementation only supports a single FIFO for both frame phases then aFramePhase is ignored.
 */
EXPORT_C TInt I2s::IsFIFOEnabled(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt& aEnabled)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->IsFIFOEnabled(aFramePhase, aEnabled);
	}

/**
 Sets the receive or transmit FIFO threshold on a per frame phase basis.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aDirection	One of TDirection.
 @param aThreshold	A threshold level at which a receive FIFO is considered full or a transmit FIFO is considered empty.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the implementation does no support FIFOs;
 KErrOverflow if the threshold level requested exceeds the FIFO length (or the admissible highest level allowed)
 KErrUnderflow if the threshold level requested is less than the minimum threshold allowed.
 
 If the implementation has a combined receive/transmit FIFO - half duplex operation only - then aDirection is ignored.
 If the implementation only supports a single FIFO for both frame phases then aFramePhase is ignored.
 */
EXPORT_C TInt I2s::SetFIFOThreshold(TInt aInterfaceId, TI2sFramePhase aFramePhase, TI2sDirection aDirection, TInt aThreshold)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->SetFIFOThreshold(aFramePhase, aDirection, aThreshold);
	}

/**
 Reads the FIFO PIO access mode status flags for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aFlags		On return, contains a bitmask with the status flags for the frame phase selected (see TI2sFlags).
 A bit set to "1" indicates the condition described by the corresponding flag is occurring.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if reading the status flags for FIFO PIO mode is not supported by this implementation.
 
 The client driver may use one of IS_I2s_<CONDITION> macros to determine the status of individual conditions.
 If the implementation has a combined receive/transmit FIFO - half duplex operation only - then aFlags will be set according
 to which operation (receive/transmit) is undergoing.
 If the implementation only supports a single FIFO for both frame phases then aFramePhase is ignored.
 */
EXPORT_C TInt I2s::ReadFIFOModeStatus(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt& aFlags)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->ReadFIFOModeStatus(aFramePhase, aFlags);
	}

/**
 Enables FIFO related interrupts for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aInterrupt	A bitmask containing the relevant interrupt flags (see TI2sFlags).
 Bits set to "1" enable the corresponding interrupts.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if one of the selected interrupt conditions cannot be generated by this implementation.

 If the implementation only supports single transmit and receive FIFO for both frame phases, the aFramePhase argument is 
 ignored.
 */
EXPORT_C TInt I2s::EnableFIFOInterrupts(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt aInterrupt)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->EnableFIFOInterrupts(aFramePhase, aInterrupt);
	}

/**
 Disables FIFO related interrupts for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aInterrupt	A bitmask containing the relevant interrupt flags (see TI2sFlags).
 Bits set to "1" disable the corresponding interrupts.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if one of the selected interrupt conditions cannot be generated by this implementation.

 If the implementation only supports single transmit and receive FIFO for both frame phases, the aFramePhase argument is 
 ignored.
 */
EXPORT_C TInt I2s::DisableFIFOInterrupts(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt aInterrupt)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->DisableFIFOInterrupts(aFramePhase, aInterrupt);
	}

/**
 Reads the FIFO interrupt masks for a frame phase.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aEnabled		On return, contains a bitmask with the interrupts which are enabled for the frame phase selected (see TI2sFlags).
 A bit set to "1" indicates the corresponding interrupt is enabled.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if one of the selected interrupt conditions cannot be generated by this implementation.
 */
EXPORT_C TInt I2s::IsFIFOInterruptEnabled(TInt aInterfaceId, TI2sFramePhase aFramePhase, TInt& aEnabled)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->IsFIFOInterruptEnabled(aFramePhase, aEnabled);
	}

/**
 Reads the receive or transmit FIFO current level on a per frame phase basis.
 
 @param aInterfaceId	The interface Id.
 @param aFramePhase	One of TI2sFramePhase.
 @param aDirection	One of TDirection.
 @param aLevel		On return, contains the current level for the FIFO described by the (aFramePhase,aDirection) pair.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the implementation does no support FIFOs.
 
 If the implementation has a combined receive/transmit FIFO - half duplex operation only - then aDirection is ignored.
 If the implementation only supports a single FIFO for both frame phases then aFramePhase is ignored.
 */
EXPORT_C TInt I2s::ReadFIFOLevel(TInt aInterfaceId, TI2sFramePhase aFramePhase, TI2sDirection aDirection, TInt& aLevel)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->ReadFIFOLevel(aFramePhase, aDirection, aLevel);
	}

/**
 Enables receive and/or transmit DMA.
 
 @param aInterfaceId	The interface Id.
 @param aFifoMask	A bitmask specifying which directions - receive and/or transmit - is DMA to be enabled (see TI2sDirection).
 Bits set to "1" enable DMA.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the implementation does no support DMA.
 
 If the implementation has a combined receive/transmit FIFO - half duplex operation only - then aFifoMask is ignored.
 */
EXPORT_C TInt I2s::EnableDMA(TInt aInterfaceId, TInt aFifoMask)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->EnableDMA(aFifoMask);
	}

/**
 Disables receive and/or transmit DMA.
 
 @param aInterfaceId	The interface Id.
 @param aFifoMask	A bitmask specifying which directions - receive and/or transmit - is DMA to be disabled (see TI2sDirection).
 Bits set to "1" disable DMA.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the implementation does no support DMA.

 If the implementation has a combined receive/transmit FIFO - half duplex operation only - then aFifoMask is ignored.
 */
EXPORT_C TInt I2s::DisableDMA(TInt aInterfaceId, TInt aFifoMask)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->DisableDMA(aFifoMask);
	}

/**
 Reads the enabled state of DMA.
 
 @param aInterfaceId	The interface Id.
 @param aEnabled		On return, contains a bitmask indicating if DMA enabled for the corresponding directions (see TI2sDirection).
 A bit set to "1" indicates DMA is enabled for the corresponding direction.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid;
 KErrNotSupported, if the implementation does no support FIFOs.

 If the implementation has a combined receive/transmit FIFO - half duplex operation only - then aEnabled will have 
 both Rx and Tx bits set when the DMA is enabled.
 */
EXPORT_C TInt I2s::IsDMAEnabled(TInt aInterfaceId, TInt& aEnabled)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->IsDMAEnabled(aEnabled);
	}

/**
 Starts data transmission and/or data reception unless interface is a Controller;
 if device is also a Master, starts generation of data synchronisation signals.
 
 @param aInterfaceId	The interface Id.
 @param aDirection	A bitmask made of TI2sDirection values. The value is ignored if interface is a Controller.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid or if aDirection  is invalid (i.e. negative, 0 or greater than 3)
 KErrNotSupported, if one of the transfer directions selected is not supported on this interface;
 KErrInUse, if interface has a bidirectional data port and an access in the opposite direction is underway;
 KErrNotReady, if interface is not ready (e.g. incomplete configuration).
 
 Start() is idempotent, attempting to start an already started interface has no effect (returns KErrNone).
 */
EXPORT_C TInt I2s::Start(TInt aInterfaceId, TInt aDirection)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->Start(aDirection);
	}

/**
 Stops data transmission and/or data reception;
 if device is also a Master, stops generation of data synchronisation signals.
 
 @param aInterfaceId	The interface Id.
 @param aDirection	A bitmask made of TI2sDirection values.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid or if aDirection  is invalid (i.e. negative, 0 or greater than 3)
 KErrNotSupported, if one of the transfer directions selected is not supported on this interface.

 Stop() is idempotent, attempting to stop an already started interface has no effect (returns KErrNone).
 */
EXPORT_C TInt I2s::Stop(TInt aInterfaceId, TInt aDirection)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->Stop(aDirection);
	}

/**
 Checks if a transmission or a reception is underway.
 
 @param aInterfaceId	The interface Id.
 @param aDirection	One of TI2sDirection.
 @param aStarted 	On return, contains ETrue if the the access is underway, EFalse otherwise.

 @return 	KErrNone, if successful; 
 KErrArgument, if aInterfaceId is invalid or if aDirection  is invalid (i.e. negative, 0 or greater than 3)
 KErrNotSupported, if one of the transfer directions selected is not supported on this interface.

 If the interface is a Controller and a bus operation is underway, ETrue is returned regardless of aDirection.
 */
EXPORT_C TInt I2s::IsStarted(TInt aInterfaceId, TI2sDirection aDirection, TBool& aStarted)
	{
	TUint16 chanId=CHANNEL_ID_FROM_INTERFACE_ID(aInterfaceId);
	if(chanId >= TI2sManager::iChannelsNum)
		return KErrArgument;

	return TI2sManager::iChannels[chanId]->IsStarted(aDirection, aStarted);
	}

// dll entry point..
DECLARE_STANDARD_EXTENSION()
	{
	__KTRACE_OPT(KBOOT, Kern::Printf("Starting I2S Extension"));
	// coverity[alloc_fn]
	TI2sManager* pD = new TI2sManager;
	TInt r=KErrNoMemory;
	if (pD)
		r=pD->DoCreate();
	return r;
	}

