#ifndef _CYASHALBEAGLEBOARD_SPI_H_
#define _CYASHALBEAGLEBOARD_SPI_H_

#include <kern_priv.h>
#include <beagle/beagle_gpio.h>
#include <beagle/variant.h>
#include <assp/omap3530_assp/omap3530_assp_priv.h>
#include <assp/omap3530_assp/omap3530_irqmap.h> // GPIO interrupts
#include <assp/omap3530_assp/omap3530_gpio.h>

#include <assp.h> // Required for definition of TIsr

const TUint KGPIO_183 = 183;
const TUint KGPIO_INT = 168;

//#define SINGLE_CHANNEL_MASTER_MODE

/* PRCM registers */
const TUint KCM_FCLKEN1_CORE = Omap3530HwBase::TVirtual<0x48004A00>::Value;
const TUint KCM_ICLKEN1_CORE = Omap3530HwBase::TVirtual<0x48004A10>::Value;

const TUint32 KCONTROL_SEC_CTRL  = Omap3530HwBase::TVirtual<0x480022B0>::Value;

/* Pad configuration registers */
const TUint32 KPADCONFIG_McSPI4_SIMO_P12_0  = Omap3530HwBase::TVirtual<0x48002190>::Value;
const TUint32 KPADCONFIG_McSPI4_SIMO_P18_16 = Omap3530HwBase::TVirtual<0x48002190>::Value;
const TUint32 KPADCONFIG_McSPI4_CS0_P16_16  = Omap3530HwBase::TVirtual<0x48002194>::Value;
const TUint32 KPADCONFIG_McSPI4_CLK_P20_0   = Omap3530HwBase::TVirtual<0x4800218c>::Value;
const TUint32 KPADCONFIG_GPIO157_P22_16		= Omap3530HwBase::TVirtual<0x4800218c>::Value;
const TUint32 KPADCONFIG_GPIO183_P23_0		= Omap3530HwBase::TVirtual<0x480021c0>::Value;
const TUint32 KPADCONFIG_GPIO168_P24_16		= Omap3530HwBase::TVirtual<0x480021bc>::Value;

/* SPI configuration registers */
const TUint32 KMcSPI4_SYSCONFIG = Omap3530HwBase::TVirtual<0x480BA010>::Value;
const TUint32 KMcSPI4_SYSSTATUS = Omap3530HwBase::TVirtual<0x480BA014>::Value;
const TUint32 KMcSPI4_IRQSTATUS = Omap3530HwBase::TVirtual<0x480BA018>::Value;
const TUint32 KMcSPI4_IRQENABLE = Omap3530HwBase::TVirtual<0x480BA01c>::Value;
const TUint32 KMcSPI4_WAKEUPEN  = Omap3530HwBase::TVirtual<0x480BA020>::Value;
const TUint32 KMcSPI4_SYST	    = Omap3530HwBase::TVirtual<0x480BA024>::Value;
const TUint32 KMcSPI4_MODULCTRL = Omap3530HwBase::TVirtual<0x480BA028>::Value;
const TUint32 KMcSPI4_CH0CONF	= Omap3530HwBase::TVirtual<0x480BA02c>::Value;
const TUint32 KMcSPI4_CH0STAT	= Omap3530HwBase::TVirtual<0x480BA030>::Value;
const TUint32 KMcSPI4_CH0CTRL 	= Omap3530HwBase::TVirtual<0x480BA034>::Value;
const TUint32 KMcSPI4_TX0 		= Omap3530HwBase::TVirtual<0x480BA038>::Value;
const TUint32 KMcSPI4_RX0 		= Omap3530HwBase::TVirtual<0x480BA03c>::Value;
const TUint32 KMcSPI4_XFERLEVEL = Omap3530HwBase::TVirtual<0x480BA07c>::Value;
/**/

extern int
CyAsHalBeagleBoard__ConfigureSPI(void);

extern int
CyAsHalBeagleBoard__SetupISR(void* handler, void* ptr);

extern void
CyAsHalBeagleBoardMcSPI4Ch0_ReadReg(TUint32 addr, TUint16* value);

extern void
CyAsHalBeagleBoardMcSPI4Ch0_WriteReg(TUint32 addr, TUint16 value);

extern void
CyAsHalBeagleBoardMcSPI4Ch0_ReadEP(TUint32 addr, TUint8* buff, TUint16 size);

extern void
CyAsHalBeagleBoardMcSPI4Ch0_WriteEP(TUint32 addr, TUint8* buff, TUint16 size);

#endif
