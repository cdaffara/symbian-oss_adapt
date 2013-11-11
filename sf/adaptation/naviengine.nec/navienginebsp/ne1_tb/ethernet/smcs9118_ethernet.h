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
* ne1_tb\ethernet\smcs9118_ethernet.h
* SMCS9118 Ethernet driver header
*
*/




#ifndef __SMCS9118_ETHERNET_H__
#define __SMCS9118_ETHERNET_H__

#include <naviengine.h>
#include "shared_ethernet.h"
#include <drivers/gpio.h>
#include <nkern.h>

/**
 * @addtogroup shared_ethernet
 * @{
 */

//
// Driver Constants
//

_LIT(KEthernetPddName,"Ethernet.Navi9118");

//
// SMCS9118 FIFO Registers (see datasheet Figure 5.1)
//
const TUint32 SMCS9118_RX_DATA_FIFO		= (TUint32)(KHwBaseEthernet + 0x00);
const TUint32 SMCS9118_TX_DATA_FIFO		= (TUint32)(KHwBaseEthernet + 0x20);
const TUint32 SMCS9118_RX_STATUS		= (TUint32)(KHwBaseEthernet + 0x40);
const TUint32 SMCS9118_RX_STATUS_PEEK	= (TUint32)(KHwBaseEthernet + 0x44);
const TUint32 SMCS9118_TX_STATUS		= (TUint32)(KHwBaseEthernet + 0x48);
const TUint32 SMCS9118_TX_STATUS_PEEK	= (TUint32)(KHwBaseEthernet + 0x4c);

//
// SMSC9118 System Control and Status Registers - see datasheet section 5.3
//
const TUint32 SMCS9118_ID_REV		= (TUint32)(KHwBaseEthernet + 0x50);
const TUint32 SMCS9118_IRQ_CFG		= (TUint32)(KHwBaseEthernet + 0x54);
const TUint32 SMCS9118_INT_STS		= (TUint32)(KHwBaseEthernet + 0x58);
const TUint32 SMCS9118_INT_EN		= (TUint32)(KHwBaseEthernet + 0x5c);
const TUint32 SMCS9118_BYTE_TEST	= (TUint32)(KHwBaseEthernet + 0x64);
const TUint32 SMCS9118_FIFO_INT		= (TUint32)(KHwBaseEthernet + 0x68);
const TUint32 SMCS9118_RX_CFG		= (TUint32)(KHwBaseEthernet + 0x6c);
const TUint32 SMCS9118_TX_CFG		= (TUint32)(KHwBaseEthernet + 0x70);
const TUint32 SMCS9118_HW_CFG		= (TUint32)(KHwBaseEthernet + 0x74);
const TUint32 SMCS9118_RX_DP_CTL	= (TUint32)(KHwBaseEthernet + 0x78);
const TUint32 SMCS9118_RX_FIFO_INF	= (TUint32)(KHwBaseEthernet + 0x7c);
const TUint32 SMCS9118_TX_FIFO_INF	= (TUint32)(KHwBaseEthernet + 0x80);
const TUint32 SMCS9118_PMT_CTRL		= (TUint32)(KHwBaseEthernet + 0x84);
const TUint32 SMCS9118_GPIO_CFG		= (TUint32)(KHwBaseEthernet + 0x88);
const TUint32 SMCS9118_GPT_CFG		= (TUint32)(KHwBaseEthernet + 0x8c);
const TUint32 SMCS9118_GPT_CNT		= (TUint32)(KHwBaseEthernet + 0x90);
const TUint32 SMCS9118_WORDSWAP		= (TUint32)(KHwBaseEthernet + 0x98);
const TUint32 SMCS9118_FREE_RUN		= (TUint32)(KHwBaseEthernet + 0x9c);
const TUint32 SMCS9118_RX_DROP		= (TUint32)(KHwBaseEthernet + 0xa0);
const TUint32 SMCS9118_MAC_CSR_CMD	= (TUint32)(KHwBaseEthernet + 0xa4);
const TUint32 SMCS9118_MAC_CSR_DATA = (TUint32)(KHwBaseEthernet + 0xa8);
const TUint32 SMCS9118_AFC_CFG		= (TUint32)(KHwBaseEthernet + 0xac);
const TUint32 SMCS9118_E2P_CMD		= (TUint32)(KHwBaseEthernet + 0xb0);
const TUint32 SMCS9118_E2P_DATA		= (TUint32)(KHwBaseEthernet + 0xb0);

//
// SMCS9118 MAC CSR register map - see datasheet section 5.4
//
const TUint32 SMCS9118_MAC_CR		= 0x01;
const TUint32 SMCS9118_MAC_ADDRH	= 0x02;
const TUint32 SMCS9118_MAC_ADDRL	= 0x03;
const TUint32 SMCS9118_MAC_MII_ACC	= 0x06;
const TUint32 SMCS9118_MAC_MII_DATA = 0x07;
const TUint32 SMCS9118_MAC_FLOW		= 0x08;
const TUint32 SMCS9118_MAC_VLAN1	= 0x09;
const TUint32 SMCS9118_MAC_VLAN2	= 0x0a;
const TUint32 SMCS9118_MAC_WUFF		= 0x0b;
const TUint32 SMCS9118_MAC_WUCSR	= 0x0c;

//
// SMCS9118 PHY control and status registers - see datasheet section 5.5
//
const TUint32 SMCS9118_PHY_BCR			= 0x00;
const TUint32 SMCS9118_PHY_BSR			= 0x01;
const TUint32 SMCS9118_PHY_ID1			= 0x02;
const TUint32 SMCS9118_PHY_ID2			= 0x03;
const TUint32 SMCS9118_PHY_AUTONEG_AD	= 0x04;
const TUint32 SMCS9118_PHY_AUTONEG_LPAR	= 0x05;
const TUint32 SMCS9118_PHY_AUTONEG_ER	= 0x06;
const TUint32 SMCS9118_PHY_MCSR			= 0x11;
const TUint32 SMCS9118_PHY_SMR			= 0x12;
const TUint32 SMCS9118_PHY_SCSI			= 0x1b;
const TUint32 SMCS9118_PHY_ISR			= 0x1d;
const TUint32 SMCS9118_PHY_IMR			= 0x1e;
const TUint32 SMCS9118_PHY_SCSR			= 0x1f;

//
// SMCS9118 system register values - see datasheet section 3.12.2
//
const TUint32 SMCS9118_TX_FIRSTSEG	= 0x00002000; // 
const TUint32 SMCS9118_TX_LASTSEG	= 0x00001000; //

const TUint32 SMCS9118_RX_ES		= 0x00008000; // RX Error Status - see datasheet section 3.13.3

const TUint32 SMCS9118_ID_VAL	= 0x01180000; // Chip ID = 0x0118
const TUint32 SMCS9118_ID_MASK	= 0xffff0000; // chip id is top 16bits
const TUint32 SMCS9118_REV_MASK	= 0x0000ffff; // chip revision is bottom 16bits

// IRQ_CFG values - see datasheet section 5.3.2
const TUint32 SMCS9118_IRQ_CFG_DEAS = 0x16000000; // Interupt deassert interval
const TUint32 SMCS9118_IRQ_CFG_TYPE = 0x00000111; // IRQ_TYPE active low, push-pull 

// INT_STS values - see datasheet section 5.3.3
const TUint32 SMCS9118_INT_STS_RSFL = 0x00000008; // RX Status FIFO Level Int
const TUint32 SMCS9118_RXSTOP_INT	= 0x00100000; // RX Status FIFO Level Int
const TUint32 SMCS9118_INT_STS_TXE	= 0x00002000; // TX error
const TUint32 SMCS9118_INT_STS_TX	= 0x02212f80; //

// INT_EN values - see datasheet section 5.3.4
const TUint32 SMCS9118_INT_EN_RSFL	= 0x00000008; // RX Status FIFO Level Int

// RX_CFG - see datasheet section 5.3.7
const TUint32 SMCS9118_RX_DUMP		= 0x00008000; // RX dump whole FIFO

// TX_CFG - see datasheet section 5.3.8
const TUint32 SMCS9118_TX_CFG_TXSAO = 0x00000004; // TX Status allow overrun
const TUint32 SMCS9118_TX_CFG_TX_ON = 0x00000002; // TX ON

// HW_CFG - see datasheet section 5.3.9
const TUint32 SMCS9118_HW_CFG_SRST	= 0x00000001; // Software Reset Time-out
const TUint32 SMCS9118_HW_CFG_SF	= 0x00100000; // SF - Store and forward

// TX FIFO Allocations - see datasheet section 5.3.9.1
const TUint32 SMCS9118_TX_FIFO_SZ	= 0x03<<16; // bits 16:19 determine FIFO size
const TUint32 SMCS9118_TX_PKT_TAG	= 0xabcd0000;

// RX FIFO - see datasheet section 5.3.10/5.3.11
const TUint32 SMCS9118_RX_DP_FFWD	= 0x80000000; // RX Data FIFO Fast Forward
const TUint32 SMCS9118_RX_LEN_MASK	= 0x0000ffff;
const TUint32 SMCS9118_RX_LEN_SHIFT	= 0x10;

// TX FIFO - see datasheet section 5.3.12
const TUint32 SMCS9118_TX_SPACE_MASK = 0x0000ffff;
const TUint32 SMCS9118_TX_USED_MASK	= 0x00ff0000;

// PMT_CTRL - see datasheet section 5.3.13
const TUint32 SMCS9118_PMT_PHY_RST	= 0x00000400; // Physical reset
const TUint32 SMCS9118_PMT_READY	= 0x00000001; // Device ready
const TUint32 SMCS9118_PM_MODE_D1	= 0x00001000; // D1 Sleep
const TUint32 SMCS9118_PM_MODE_D2	= 0x00002000; // D2 Sleep

// GPIO_CFG - - see datasheet section 5.3.14
const TUint32 SMCS9118_GPIO_LED_EN	= 0x70000000; // LED on
const TUint32 SMCS9118_GPIO_GPIOBUF = 0x00070000; // GPIO Buffer Type

// MAC_CSR_CMD - see datasheet section 5.3.20
const TUint32 SMCS9118_MAC_CSR_BUSY = 0x80000000;
const TUint32 SMCS9118_MAC_CSR_READ = 0x40000000;

// Auto Flow Control Config Register - see datasheet section 5.3.22
// BACK_DUR = 4
// AFC_LO = 0x37
// AFC_HI = 0x6e
const TUint32 SMCS9118_AFC_CFG_VAL	= 0x006e3740;

// EEPROM Command Register - see datasheet section 5.3.23
const TUint32 SMCS9118_E2P_CMD_BUSY = 0x80000000;

// SMCS9118 MAC control register values - see datasheet section 5.4.1
const TUint32 SMCS9118_MAC_RXALL	= 0x80000000;
const TUint32 SMCS9118_MAC_TXEN		= 0x00000008;
const TUint32 SMCS9118_MAC_RXEN		= 0x00000004;

// SMCS9118 MII access register values - see datasheet section 5.4.6
const TUint32 SMCS9118_PHY_ADDR		= 0x00000800;
const TUint32 SMCS9118_MII_WRITE	= 0x00000002;
const TUint32 SMCS9118_MII_BUSY		= 0x00000001;


// SMCS9118 PHY Basic Control register values - see datasheet section 5.5.1
const TUint32 SMCS9118_PHY_ANEG_EN		= 0x00001000; // Auto negotiate enable
const TUint32 SMCS9118_PHY_ANEG_RESTART = 0x00000200; // Auto negotiate restart

// SMCS9118 PHY Basic Status register values - see datasheet section 5.5.2
const TUint32 SMCS9118_PHY_ANEG_CMPL = 0x00000020; // Auto negotiate complete

// SMCS9118 PHY auto negotiate advertisement values - see datasheet section 5.5.5
const TUint32 SMCS9118_PHY_10BT		= 0x00000020;
const TUint32 SMCS9118_PHY_10BTFD	= 0x00000040;
const TUint32 SMCS9118_PHY_100BTX	= 0x00000080;
const TUint32 SMCS9118_PHY_100BTXFD	= 0x00000100;
const TUint32 SMCS9118_PHY_PAUSE	= 0x00000C00;
const TUint32 SMCS9118_PHY_DEF_ANEG	= SMCS9118_PHY_10BT |
									  SMCS9118_PHY_10BTFD |
									  SMCS9118_PHY_100BTX |
									  SMCS9118_PHY_100BTXFD |
									  SMCS9118_PHY_PAUSE;

const TUint32 KEthernetInterruptId	= KGpio_Ethernet_Int_Pin;

const TUint32 SMCS9118_LOCK_ORDER	= 0x03u;

//

class DEthernetPddFactory : public DPhysicalDevice
/**
Ethernet PDD factory class
*/
	{
public:
	DEthernetPddFactory();
	
	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DBase*& aChannel, TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);
	virtual TInt Validate(TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);
	};	//DEthernetPddFactory




class DEthernetSMCS9118Pdd : public DEthernetPdd
/**
Ethernet PDD class.
*/
	{
public:
	DEthernetSMCS9118Pdd();
	~DEthernetSMCS9118Pdd();

	static void Isr(TAny* aPtr);

	/**
	 * Stop receiving frames
	 * @param aMode The stop mode
	 */
	void Stop(TStopMode aMode);

	/**
	 * Configure the device
	 * Reconfigure the device using the new configuration supplied.
	 * This should not change the MAC address.
	 * @param aConfig The new configuration
	 * @see ValidateConfig()
	 * @see MacConfigure()
	 */
	TInt Configure(TEthernetConfigV01 &aConfig) ;
	/**
	 * Change the MAC address
	 * Attempt to change the MAC address of the device
	 * @param aConfig A Configuration containing the new MAC
	 * @see Configure()
	 */
	void MacConfigure(TEthernetConfigV01 &aConfig) ;

	/**
	 * Transmit data
	 * @param aBuffer referance to the data to be sent
	 * @return KErrNone if the data has been sent
	 */
	TInt Send(TBuf8<KMaxEthernetPacket+32> &aBuffer) ;
	/**
	 * Retrieve data from the device
	 * Pull the received data out of the device and into the supplied buffer. 
	 * Need to be told if the buffer is OK to use as if it not we could dump 
	 * the waiting frame in order to clear the interrupt if necessory.
	 * @param aBuffer Referance to the buffer to be used to store the data in
	 * @param okToUse Bool to indicate if the buffer is usable
	 * @return KErrNone if the buffer has been filled.
	 */
	TInt ReceiveFrame(TBuf8<KMaxEthernetPacket+32> &aBuffer, 
								TBool okToUse) ;

	TInt DoCreate();

	/**
	 * Put the card to sleep
	 */

    void	Sleep();
	/**
	 * Wake the card up
	 */
    TInt	Wakeup();

protected:
	/**
	 * Discard a frame 
	 */
	TInt DiscardFrame();

	static void ServiceRxDfc(TAny *aPtr);

	/**
	 * Does the soft reset of the lan card 
	 */
	TInt CardSoftReset();

	inline TInt32 IsReady();
	/**
	 * see data sheet section 6.1, Host Interface Timing
	 * "dummy" reads of the BYTE_TEST register will
	 * guarantee the minimum write-to-read timing restrictions
	 * as listed in Table 6.1
	 */
	inline void ByteTestDelay(TUint32 aCount);

	/**
	 * Read/Write the MAC registers
	 */
	TInt32 ReadMac(TUint32 aReg, TUint32 &aVal);
	TInt32 WriteMac(TUint32 aReg, TUint32 aVal);

	/**
	 * Read/Write the PHY registers
	 */
	TInt32 ReadPhy(TUint32 aReg, TUint32 &aVal);
	TInt32 WritePhy(TUint32 aReg, TUint32 aVal);

	/**
	 * Read a 32bit register
	 */
	inline TUint32 Read32(TUint32 aReg);

	/**
	 * Write a 32bit register
	 */
	inline void Write32(TUint32 aReg, TUint32 aVal);

	/**
	 * Interrupt handling
	 */
	inline void ClearInterrupt(TInt aId); 
	inline void UnbindInterrupt(TInt aId); 
	inline TInt BindInterrupt(TInt aId, TGpioIsr aIsr, TAny *aPtr); 
	inline TInt EnableInterrupt(TInt aId); 
	inline TInt DisableInterrupt(TInt aId);

	/**
	 * lock handling
	 */
	inline TInt DriverLock(); 
	inline void DriverUnlock(TInt);

protected:
	TDfc iRxDfc;
#ifdef __SMP__
	TSpinLock	*iDriverLock;
#endif
	};

#include "smcs9118_ethernet.inl"

/** @} */

#endif //__SMCS9118_ETHERNET_H__
