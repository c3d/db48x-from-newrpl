/*
 * Copyright (c) 2014-2020 Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef TARGET_PRIME_H
#define TARGET_PRIME_H

#include <stdint.h>

// General hardware register macro
#define HWREG(base, off) \
    ((volatile uint32_t *) (((uintptr_t) base + (uintptr_t) off)))
#define ARM_MODE           __attribute__((target("arm"))) \
                           __attribute__((noinline))
#define ARM_MODE_INLINE    __attribute__((target("arm")))


#define DRAMC_BASE         0x48000000
#define USBH_BASE          0x49000000
#define USB2_BASE          0x49800000
#define INT_BASE           0x4a000000
#define HSMMC1_BASE        0x4a800000
#define HSMMC0_BASE        0x4ac00000
#define DMA_BASE           0x4b000000
#define SYS_BASE           0x4c000000
#define LCD_BASE           0x4c800000
#define TWOD_BASE          0x4d408000
#define NAND_BASE          0x4e000000
#define MATRIX_BASE        0x4e800000
#define SMC_BASE           0x4f000000
#define UART_BASE          0x50000000
#define PWM_BASE           0x51000000
#define SPI_BASE           0x52000000
#define WDT_BASE           0x53000000
#define IIC_BASE           0x54000000
#define IIS_BASE           0x55000000
#define IO_BASE            0x56000000
#define RTC_BASE           0x57000000
#define TSADC_BASE         0x58000000
#define AC_BASE            0x5b000000
#define PCM_BASE           0x5c000000

#define LOCKCON0           HWREG(SYS_BASE, 0x00)
#define LOCKCON1           HWREG(SYS_BASE, 0x04)
#define OSCSET             HWREG(SYS_BASE, 0x08)
#define MPLLCON            HWREG(SYS_BASE, 0x10)
#define EPLLCON            HWREG(SYS_BASE, 0x18)
#define EPLLCON_K          HWREG(SYS_BASE, 0x1c)
#define CLKSRC             HWREG(SYS_BASE, 0x20)
#define CLKDIV0            HWREG(SYS_BASE, 0x24)
#define CLKDIV1            HWREG(SYS_BASE, 0x28)
#define CLKDIV2            HWREG(SYS_BASE, 0x2c)
#define HCLKCON            HWREG(SYS_BASE, 0x30)
#define PCLKCON            HWREG(SYS_BASE, 0x34)
#define SCLKCON            HWREG(SYS_BASE, 0x38)
#define PWRMODE            HWREG(SYS_BASE, 0x40)
#define SWRST              HWREG(SYS_BASE, 0x44)
#define BUSPRI0            HWREG(SYS_BASE, 0x50)
#define PWRCFG             HWREG(SYS_BASE, 0x60)
#define RSTCON             HWREG(SYS_BASE, 0x64)
#define RSTSTAT            HWREG(SYS_BASE, 0x68)
#define WKUPSTAT           HWREG(SYS_BASE, 0x6c)
#define INFORM0            HWREG(SYS_BASE, 0x70)
#define INFORM1            HWREG(SYS_BASE, 0x74)
#define INFORM2            HWREG(SYS_BASE, 0x78)
#define INFORM3            HWREG(SYS_BASE, 0x7c)
#define PHYCTRL            HWREG(SYS_BASE, 0x80)
#define PHYPWR             HWREG(SYS_BASE, 0x84)
#define URSTCON            HWREG(SYS_BASE, 0x88)
#define UCLKCON            HWREG(SYS_BASE, 0x8c)

#define BPRIORITY0         HWREG(MATRIX_BASE, 0x00)
#define BPRIORITY1         HWREG(MATRIX_BASE, 0x04)
#define EBICON             HWREG(MATRIX_BASE, 0x08)

#define SMBIDCYR0          HWREG(SMC_BASE, 0x00)
#define SMBWSTRDR0         HWREG(SMC_BASE, 0x04)
#define SMBWSTWRR0         HWREG(SMC_BASE, 0x08)
#define SMBWSTOENR0        HWREG(SMC_BASE, 0x0c)
#define SMBWSTWENR0        HWREG(SMC_BASE, 0x10)
#define SMBCR0             HWREG(SMC_BASE, 0x14)
#define SMBIDCYR1          HWREG(SMC_BASE, 0x20)
#define SMBWSTRDR1         HWREG(SMC_BASE, 0x24)
#define SMBWSTWRR1         HWREG(SMC_BASE, 0x28)
#define SMBWSTOENR1        HWREG(SMC_BASE, 0x2c)
#define SMBWSTWENR1        HWREG(SMC_BASE, 0x30)
#define SMBCR1             HWREG(SMC_BASE, 0x34)
#define SMBIDCYR2          HWREG(SMC_BASE, 0x40)
#define SMBWSTRDR2         HWREG(SMC_BASE, 0x44)
#define SMBWSTWRR2         HWREG(SMC_BASE, 0x48)
#define SMBWSTOENR2        HWREG(SMC_BASE, 0x4c)
#define SMBWSTWENR2        HWREG(SMC_BASE, 0x50)
#define SMBCR2             HWREG(SMC_BASE, 0x54)
#define SMBIDCYR3          HWREG(SMC_BASE, 0x60)
#define SMBWSTRDR3         HWREG(SMC_BASE, 0x64)
#define SMBWSTWRR3         HWREG(SMC_BASE, 0x68)
#define SMBWSTOENR3        HWREG(SMC_BASE, 0x6c)
#define SMBWSTWENR3        HWREG(SMC_BASE, 0x70)
#define SMBCR3             HWREG(SMC_BASE, 0x74)
#define SMBIDCYR4          HWREG(SMC_BASE, 0x80)
#define SMBWSTRDR4         HWREG(SMC_BASE, 0x84)
#define SMBWSTWRR4         HWREG(SMC_BASE, 0x88)
#define SMBWSTOENR4        HWREG(SMC_BASE, 0x8c)
#define SMBWSTWENR4        HWREG(SMC_BASE, 0x90)
#define SMBCR4             HWREG(SMC_BASE, 0x94)
#define SMBIDCYR5          HWREG(SMC_BASE, 0xa0)
#define SMBWSTRDR5         HWREG(SMC_BASE, 0xa4)
#define SMBWSTWRR5         HWREG(SMC_BASE, 0xa8)
#define SMBWSTOENR5        HWREG(SMC_BASE, 0xac)
#define SMBWSTWENR5        HWREG(SMC_BASE, 0xb0)
#define SMBCR5             HWREG(SMC_BASE, 0xb4)
#define SMBONETYPER        HWREG(SMC_BASE, 0x100)
#define SMBCSR             HWREG(SMC_BASE, 0x200)
#define SMBCCR             HWREG(SMC_BASE, 0x204)

#define BANKCFG            HWREG(DRAMC_BASE, 0x00)
#define BANKCON1           HWREG(DRAMC_BASE, 0x04)
#define BANKCON2           HWREG(DRAMC_BASE, 0x08)
#define BANKCON3           HWREG(DRAMC_BASE, 0x0c)
#define REFRESH            HWREG(DRAMC_BASE, 0x10)
#define TIMEOUT            HWREG(DRAMC_BASE, 0x14)

#define NFCONF             HWREG(NAND_BASE, 0x00)
#define NFCONT             HWREG(NAND_BASE, 0x04)
#define NFCMMD             HWREG(NAND_BASE, 0x08)
#define NFADDR             HWREG(NAND_BASE, 0x0c)
#define NFDATA             HWREG(NAND_BASE, 0x10)
#define NFMECCD0           HWREG(NAND_BASE, 0x14)
#define NFMECCD1           HWREG(NAND_BASE, 0x18)
#define NFSECCD            HWREG(NAND_BASE, 0x1c)
#define NFSBLK             HWREG(NAND_BASE, 0x20)
#define NFEBLK             HWREG(NAND_BASE, 0x24)
#define NFSTAT             HWREG(NAND_BASE, 0x28)
#define NFECCERR0          HWREG(NAND_BASE, 0x2c)
#define NFECCERR1          HWREG(NAND_BASE, 0x30)
#define NFMECC0            HWREG(NAND_BASE, 0x34)
#define NFMECC1            HWREG(NAND_BASE, 0x38)
#define NFSECC             HWREG(NAND_BASE, 0x3c)
#define NFMLCBITPT         HWREG(NAND_BASE, 0x40)
#define NF8ECCERR0         HWREG(NAND_BASE, 0x44)
#define NF8ECCERR1         HWREG(NAND_BASE, 0x48)
#define NF8ECCERR2         HWREG(NAND_BASE, 0x4c)
#define NFM8ECC0           HWREG(NAND_BASE, 0x50)
#define NFM8ECC1           HWREG(NAND_BASE, 0x54)
#define NFM8ECC2           HWREG(NAND_BASE, 0x58)
#define NFM8ECC3           HWREG(NAND_BASE, 0x5c)
#define NFMLC8BITPT0       HWREG(NAND_BASE, 0x60)
#define NFMLC8BITPT1       HWREG(NAND_BASE, 0x64)

#define DISRC0             HWREG(DMA_BASE, 0x000)
#define DISRCC0            HWREG(DMA_BASE, 0x004)
#define DIDST0             HWREG(DMA_BASE, 0x008)
#define DIDSTC0            HWREG(DMA_BASE, 0x00c)
#define DCON0              HWREG(DMA_BASE, 0x010)
#define DSTAT0             HWREG(DMA_BASE, 0x014)
#define DCSRC0             HWREG(DMA_BASE, 0x018)
#define DCDST0             HWREG(DMA_BASE, 0x01c)
#define DMASKTRIG0         HWREG(DMA_BASE, 0x020)
#define DMAREQSEL0         HWREG(DMA_BASE, 0x024)
#define DISRC1             HWREG(DMA_BASE, 0x100)
#define DISRCC1            HWREG(DMA_BASE, 0x104)
#define DIDST1             HWREG(DMA_BASE, 0x108)
#define DIDSTC1            HWREG(DMA_BASE, 0x10c)
#define DCON1              HWREG(DMA_BASE, 0x110)
#define DSTAT1             HWREG(DMA_BASE, 0x114)
#define DCSRC1             HWREG(DMA_BASE, 0x118)
#define DCDST1             HWREG(DMA_BASE, 0x11c)
#define DMASKTRIG1         HWREG(DMA_BASE, 0x120)
#define DMAREQSEL1         HWREG(DMA_BASE, 0x124)
#define DISRC2             HWREG(DMA_BASE, 0x200)
#define DISRCC2            HWREG(DMA_BASE, 0x204)
#define DIDST2             HWREG(DMA_BASE, 0x208)
#define DIDSTC2            HWREG(DMA_BASE, 0x20c)
#define DCON2              HWREG(DMA_BASE, 0x210)
#define DSTAT2             HWREG(DMA_BASE, 0x214)
#define DCSRC2             HWREG(DMA_BASE, 0x218)
#define DCDST2             HWREG(DMA_BASE, 0x21c)
#define DMASKTRIG2         HWREG(DMA_BASE, 0x220)
#define DMAREQSEL2         HWREG(DMA_BASE, 0x224)
#define DISRC3             HWREG(DMA_BASE, 0x300)
#define DISRCC3            HWREG(DMA_BASE, 0x304)
#define DIDST3             HWREG(DMA_BASE, 0x308)
#define DIDSTC3            HWREG(DMA_BASE, 0x30c)
#define DCON3              HWREG(DMA_BASE, 0x310)
#define DSTAT3             HWREG(DMA_BASE, 0x314)
#define DCSRC3             HWREG(DMA_BASE, 0x318)
#define DCDST3             HWREG(DMA_BASE, 0x31c)
#define DMASKTRIG3         HWREG(DMA_BASE, 0x320)
#define DMAREQSEL3         HWREG(DMA_BASE, 0x324)
#define DISRC4             HWREG(DMA_BASE, 0x400)
#define DISRCC4            HWREG(DMA_BASE, 0x404)
#define DIDST4             HWREG(DMA_BASE, 0x408)
#define DIDSTC4            HWREG(DMA_BASE, 0x40c)
#define DCON4              HWREG(DMA_BASE, 0x410)
#define DSTAT4             HWREG(DMA_BASE, 0x414)
#define DCSRC4             HWREG(DMA_BASE, 0x418)
#define DCDST4             HWREG(DMA_BASE, 0x41c)
#define DMASKTRIG4         HWREG(DMA_BASE, 0x420)
#define DMAREQSEL4         HWREG(DMA_BASE, 0x424)
#define DISRC5             HWREG(DMA_BASE, 0x500)
#define DISRCC5            HWREG(DMA_BASE, 0x504)
#define DIDST5             HWREG(DMA_BASE, 0x508)
#define DIDSTC5            HWREG(DMA_BASE, 0x50c)
#define DCON5              HWREG(DMA_BASE, 0x510)
#define DSTAT5             HWREG(DMA_BASE, 0x514)
#define DCSRC5             HWREG(DMA_BASE, 0x518)
#define DCDST5             HWREG(DMA_BASE, 0x51c)
#define DMASKTRIG5         HWREG(DMA_BASE, 0x520)
#define DMAREQSEL5         HWREG(DMA_BASE, 0x524)

#define SRCPND1            HWREG(INT_BASE, 0x00)
#define INTMOD1            HWREG(INT_BASE, 0x04)
#define INTMSK1            HWREG(INT_BASE, 0x08)
#define INTPND1            HWREG(INT_BASE, 0x10)
#define INTOFFSET1         HWREG(INT_BASE, 0x14)
#define SUBSRCPND          HWREG(INT_BASE, 0x18)
#define INTSUBMSK          HWREG(INT_BASE, 0x1c)
#define PRIORITY_MODE1     HWREG(INT_BASE, 0x30)
#define PRIORITY_UPDATE1   HWREG(INT_BASE, 0x34)
#define SRCPND2            HWREG(INT_BASE, 0x40)
#define INTMOD2            HWREG(INT_BASE, 0x44)
#define INTMSK2            HWREG(INT_BASE, 0x48)
#define INTPND2            HWREG(INT_BASE, 0x50)
#define INTOFFSET2         HWREG(INT_BASE, 0x54)
#define PRIORITY_MODE2     HWREG(INT_BASE, 0x70)
#define PRIORITY_UPDATE2   HWREG(INT_BASE, 0x74)

#define GPACON             HWREG(IO_BASE, 0x000)
#define GPADAT             HWREG(IO_BASE, 0x004)
#define GPBCON             HWREG(IO_BASE, 0x010)
#define GPBDAT             HWREG(IO_BASE, 0x014)
#define GPBUDP             HWREG(IO_BASE, 0x018)
#define GPBSEL             HWREG(IO_BASE, 0x01c)
#define GPCCON             HWREG(IO_BASE, 0x020)
#define GPCDAT             HWREG(IO_BASE, 0x024)
#define GPCUDP             HWREG(IO_BASE, 0x028)
#define GPDCON             HWREG(IO_BASE, 0x030)
#define GPDDAT             HWREG(IO_BASE, 0x034)
#define GPDUDP             HWREG(IO_BASE, 0x038)
#define GPECON             HWREG(IO_BASE, 0x040)
#define GPEDAT             HWREG(IO_BASE, 0x044)
#define GPEUDP             HWREG(IO_BASE, 0x048)
#define GPESEL             HWREG(IO_BASE, 0x04c)
#define GPFCON             HWREG(IO_BASE, 0x050)
#define GPFDAT             HWREG(IO_BASE, 0x054)
#define GPFUDP             HWREG(IO_BASE, 0x058)
#define GPGCON             HWREG(IO_BASE, 0x060)
#define GPGDAT             HWREG(IO_BASE, 0x064)
#define GPGUDP             HWREG(IO_BASE, 0x068)
#define GPHCON             HWREG(IO_BASE, 0x070)
#define GPHDAT             HWREG(IO_BASE, 0x074)
#define GPHUDP             HWREG(IO_BASE, 0x078)
#define MISCCR             HWREG(IO_BASE, 0x080)
#define DCLKCON            HWREG(IO_BASE, 0x084)
#define EXTINT0            HWREG(IO_BASE, 0x088)
#define EXTINT1            HWREG(IO_BASE, 0x08c)
#define EINTMASK           HWREG(IO_BASE, 0x0a4)
#define EINTPEND           HWREG(IO_BASE, 0x0a8)
#define GSTATUS0           HWREG(IO_BASE, 0x0ac)
#define GSTATUS1           HWREG(IO_BASE, 0x0b0)
#define DSC0               HWREG(IO_BASE, 0x0c0)
#define DSC1               HWREG(IO_BASE, 0x0c4)
#define DSC2               HWREG(IO_BASE, 0x0c8)
#define GPKCON             HWREG(IO_BASE, 0x0e0)
#define GPKDAT             HWREG(IO_BASE, 0x0e4)
#define GPKUDP             HWREG(IO_BASE, 0x0e8)
#define GPLCON             HWREG(IO_BASE, 0x0f0)
#define GPLDAT             HWREG(IO_BASE, 0x0f4)
#define GPLUDP             HWREG(IO_BASE, 0x0f8)
#define GPMCON             HWREG(IO_BASE, 0x100)
#define GPMDAT             HWREG(IO_BASE, 0x104)
#define GPMUDP             HWREG(IO_BASE, 0x108)
#define DSC3               HWREG(IO_BASE, 0x110)
#define PDDMCON            HWREG(IO_BASE, 0x114)
#define PDSMCON            HWREG(IO_BASE, 0x118)

#define WDTCON             HWREG(WDT_BASE, 0x00)
#define WTDAT              HWREG(WDT_BASE, 0x04)
#define WTCNT              HWREG(WDT_BASE, 0x08)

#define TCFG0              HWREG(PWM_BASE, 0x00)
#define TCFG1              HWREG(PWM_BASE, 0x04)
#define TCON               HWREG(PWM_BASE, 0x08)
#define TCNTB0             HWREG(PWM_BASE, 0x0c)
#define TCMPB0             HWREG(PWM_BASE, 0x10)
#define TCNTO0             HWREG(PWM_BASE, 0x14)
#define TCNTB1             HWREG(PWM_BASE, 0x18)
#define TCMPB1             HWREG(PWM_BASE, 0x1c)
#define TCNTO1             HWREG(PWM_BASE, 0x20)
#define TCNTB2             HWREG(PWM_BASE, 0x24)
#define TCMPB2             HWREG(PWM_BASE, 0x28)
#define TCNTO2             HWREG(PWM_BASE, 0x2c)
#define TCNTB3             HWREG(PWM_BASE, 0x30)
#define TCMPB3             HWREG(PWM_BASE, 0x34)
#define TCNTO3             HWREG(PWM_BASE, 0x38)
#define TCNTB4             HWREG(PWM_BASE, 0x3c)
#define TCNTO4             HWREG(PWM_BASE, 0x40)

#define RTCCON             HWREG(RTC_BASE, 0x40)
#define TICNT0             HWREG(RTC_BASE, 0x44)
#define TICNT2             HWREG(RTC_BASE, 0x48)
#define TICNT1             HWREG(RTC_BASE, 0x4c)
#define RTCALM             HWREG(RTC_BASE, 0x50)
#define ALMSEC             HWREG(RTC_BASE, 0x54)
#define ALMMIN             HWREG(RTC_BASE, 0x58)
#define ALMHOUR            HWREG(RTC_BASE, 0x5c)
#define ALMDATE            HWREG(RTC_BASE, 0x60)
#define ALMMON             HWREG(RTC_BASE, 0x64)
#define ALMYEAR            HWREG(RTC_BASE, 0x68)
#define RTCRST             HWREG(RTC_BASE, 0x6c)
#define BCDSEC             HWREG(RTC_BASE, 0x70)
#define BCDMIN             HWREG(RTC_BASE, 0x74)
#define BCDHOUR            HWREG(RTC_BASE, 0x78)
#define BCDDATE            HWREG(RTC_BASE, 0x7c)
#define BCDDAY             HWREG(RTC_BASE, 0x80)
#define BCDMON             HWREG(RTC_BASE, 0x84)
#define BCDYEAR            HWREG(RTC_BASE, 0x88)
#define TICKCNT            HWREG(RTC_BASE, 0x90)

#define ULCON0             HWREG(UART_BASE, 0x0000)
#define UCON0              HWREG(UART_BASE, 0x0004)
#define UFCON0             HWREG(UART_BASE, 0x0008)
#define UMCON0             HWREG(UART_BASE, 0x000c)
#define UTRSTAT0           HWREG(UART_BASE, 0x0010)
#define UERSTAT0           HWREG(UART_BASE, 0x0014)
#define UFSTAT0            HWREG(UART_BASE, 0x0018)
#define UMSTAT0            HWREG(UART_BASE, 0x001c)
#define UTXH0              HWREG(UART_BASE, 0x0020)
#define URXH0              HWREG(UART_BASE, 0x0024)
#define UBRDIV0            HWREG(UART_BASE, 0x0028)
#define UDIVSLOT0          HWREG(UART_BASE, 0x002c)
#define ULCON1             HWREG(UART_BASE, 0x4000)
#define UCON1              HWREG(UART_BASE, 0x4004)
#define UFCON1             HWREG(UART_BASE, 0x4008)
#define UMCON1             HWREG(UART_BASE, 0x400c)
#define UTRSTAT1           HWREG(UART_BASE, 0x4010)
#define UERSTAT1           HWREG(UART_BASE, 0x4014)
#define UFSTAT1            HWREG(UART_BASE, 0x4018)
#define UMSTAT1            HWREG(UART_BASE, 0x401c)
#define UTXH1              HWREG(UART_BASE, 0x4020)
#define URXH1              HWREG(UART_BASE, 0x4024)
#define UBRDIV1            HWREG(UART_BASE, 0x4028)
#define UDIVSLOT1          HWREG(UART_BASE, 0x402c)
#define ULCON2             HWREG(UART_BASE, 0x8000)
#define UCON2              HWREG(UART_BASE, 0x8004)
#define UFCON2             HWREG(UART_BASE, 0x8008)
#define UMCON2             HWREG(UART_BASE, 0x800c)
#define UTRSTAT2           HWREG(UART_BASE, 0x8010)
#define UERSTAT2           HWREG(UART_BASE, 0x8014)
#define UFSTAT2            HWREG(UART_BASE, 0x8018)
#define UMSTAT2            HWREG(UART_BASE, 0x801c)
#define UTXH2              HWREG(UART_BASE, 0x8020)
#define URXH2              HWREG(UART_BASE, 0x8024)
#define UBRDIV2            HWREG(UART_BASE, 0x8028)
#define UDIVSLOT2          HWREG(UART_BASE, 0x802c)
#define ULCON3             HWREG(UART_BASE, 0xc000)
#define UCON3              HWREG(UART_BASE, 0xc004)
#define UFCON3             HWREG(UART_BASE, 0xc008)
#define UTRSTAT3           HWREG(UART_BASE, 0xc010)
#define UERSTAT3           HWREG(UART_BASE, 0xc014)
#define UFSTAT3            HWREG(UART_BASE, 0xc018)
#define UTXH3              HWREG(UART_BASE, 0xc020)
#define URXH3              HWREG(UART_BASE, 0xc024)
#define UBRDIV3            HWREG(UART_BASE, 0xc028)
#define UDIVSLOT3          HWREG(UART_BASE, 0xc02c)

#define HcRevision         HWREG(USBH_BASE, 0x00)
#define HcControl          HWREG(USBH_BASE, 0x04)
#define HcCommonStatus     HWREG(USBH_BASE, 0x08)
#define HcInterruptStatus  HWREG(USBH_BASE, 0x0c)
#define HcInterruptEnable  HWREG(USBH_BASE, 0x10)
#define HcInterruptDisable HWREG(USBH_BASE, 0x14)
#define HcHCCA             HWREG(USBH_BASE, 0x18)
#define HcPeriodCuttentED  HWREG(USBH_BASE, 0x1c)
#define HcControlHeadED    HWREG(USBH_BASE, 0x20)
#define HcControlCurrentED HWREG(USBH_BASE, 0x24)
#define HcBulkHeadED       HWREG(USBH_BASE, 0x28)
#define HcBulkCurrentED    HWREG(USBH_BASE, 0x2c)
#define HcDoneHead         HWREG(USBH_BASE, 0x30)
#define HcRmInterval       HWREG(USBH_BASE, 0x34)
#define HcFmRemaining      HWREG(USBH_BASE, 0x38)
#define HcFmNumber         HWREG(USBH_BASE, 0x3c)
#define HcPeriodicStart    HWREG(USBH_BASE, 0x40)
#define HcLSThreshold      HWREG(USBH_BASE, 0x44)
#define HcRhDescriptorA    HWREG(USBH_BASE, 0x48)
#define HcRhDescriptorB    HWREG(USBH_BASE, 0x4c)
#define HcRhStatus         HWREG(USBH_BASE, 0x50)
#define HcRhPortStatus1    HWREG(USBH_BASE, 0x54)
#define HcRhPortStatus2    HWREG(USBH_BASE, 0x58)

#define IR                 HWREG(USB2_BASE, 0x000)
#define EIR                HWREG(USB2_BASE, 0x004)
#define EIER               HWREG(USB2_BASE, 0x008)
#define FAR                HWREG(USB2_BASE, 0x00c)
#define EDR                HWREG(USB2_BASE, 0x014)
#define TR                 HWREG(USB2_BASE, 0x018)
#define SSR                HWREG(USB2_BASE, 0x01c)
#define SCR                HWREG(USB2_BASE, 0x020)
#define EP0SR              HWREG(USB2_BASE, 0x024)
#define EP0CR              HWREG(USB2_BASE, 0x028)
#define ESR                HWREG(USB2_BASE, 0x02c)
#define ECR                HWREG(USB2_BASE, 0x030)

// COUNT IN uint16_tS, STATUS REGISTER LWO INDICATES ODD COUNT
// CAN'T BE USED AS COUNTER BECAUSE IT COUNTS DOWN ON READ BUT VALUE JUMPS
// WHEN IT SHOULD BE 0
#define BRCR               HWREG(USB2_BASE, 0x034)

#define BWCR               HWREG(USB2_BASE, 0x038) // COUNT IN BYTES TO INDICATE ODD COUNT
#define MPR                HWREG(USB2_BASE, 0x03c)
#define DCR                HWREG(USB2_BASE, 0x040)
#define DTCR               HWREG(USB2_BASE, 0x044)
#define DFCR               HWREG(USB2_BASE, 0x048)
#define DTTCR1             HWREG(USB2_BASE, 0x04c)
#define DTTCR2             HWREG(USB2_BASE, 0x050)
#define EP0BR \
    HWREG(USB2_BASE, 0x060) // INGOING: 1ST BYTE IN UPPER POS; OUTGOING: 1ST
                            // BYTE IN LOWER POS
#define EP1BR               HWREG(USB2_BASE, 0x064)
#define EP2BR               HWREG(USB2_BASE, 0x068)
#define EP3BR               HWREG(USB2_BASE, 0x06c)
#define EP4BR               HWREG(USB2_BASE, 0x070)
#define EP5BR               HWREG(USB2_BASE, 0x074)
#define EP6BR               HWREG(USB2_BASE, 0x078)
#define EP7BR               HWREG(USB2_BASE, 0x07c)
#define EP8BR               HWREG(USB2_BASE, 0x080)
#define DICR                HWREG(USB2_BASE, 0x084)
#define MBAR                HWREG(USB2_BASE, 0x088)
#define MCAR                HWREG(USB2_BASE, 0x08c)
#define FCON                HWREG(USB2_BASE, 0x100)
#define FSTAT               HWREG(USB2_BASE, 0x104)

#define IICCON0             HWREG(IIC_BASE, 0x00)
#define IICSTAT0            HWREG(IIC_BASE, 0x04)
#define IICADD0             HWREG(IIC_BASE, 0x08)
#define IICDS0              HWREG(IIC_BASE, 0x0c)
#define IICLC0              HWREG(IIC_BASE, 0x10)

#define CONTROL_REG         HWREG(TWOD_BASE, 0x000)
#define INTEN_REG           HWREG(TWOD_BASE, 0x004)
#define FIFO_INTC_REG       HWREG(TWOD_BASE, 0x008)
#define INTC_PEND_REG       HWREG(TWOD_BASE, 0x00c)
#define FIFO_STAT_REG       HWREG(TWOD_BASE, 0x010)
#define CMD0_REG            HWREG(TWOD_BASE, 0x100)
#define CMD1_REG            HWREG(TWOD_BASE, 0x104)
#define CMD2_REG            HWREG(TWOD_BASE, 0x108)
#define CMD3_REG            HWREG(TWOD_BASE, 0x10c)
#define CMD4_REG            HWREG(TWOD_BASE, 0x110)
#define CMD5_REG            HWREG(TWOD_BASE, 0x114)
#define CMD6_REG            HWREG(TWOD_BASE, 0x118)
#define CMD7_REG            HWREG(TWOD_BASE, 0x11c)
#define SRC_REG_REG         HWREG(TWOD_BASE, 0x200)
#define SRC_HORI_RES_REG    HWREG(TWOD_BASE, 0x204)
#define SRC_VERT_RES_REG    HWREG(TWOD_BASE, 0x208)
#define SC_RES_REG          HWREG(TWOD_BASE, 0x210)
#define SC_HORI_RES_REG     HWREG(TWOD_BASE, 0x214)
#define SC_VERT_RES_REG     HWREG(TWOD_BASE, 0x218)
#define CW_LT_REG           HWREG(TWOD_BASE, 0x220)
#define CW_LT_X_REG         HWREG(TWOD_BASE, 0x224)
#define CW_LT_Y_REG         HWREG(TWOD_BASE, 0x228)
#define CW_RB_REG           HWREG(TWOD_BASE, 0x230)
#define CW_RB_X_REG         HWREG(TWOD_BASE, 0x234)
#define CW_RB_Y_REG         HWREG(TWOD_BASE, 0x238)
#define COORD0_REG          HWREG(TWOD_BASE, 0x300)
#define COORD0_X_REG        HWREG(TWOD_BASE, 0x304)
#define COORD0_Y_REG        HWREG(TWOD_BASE, 0x308)
#define COORD1_REG          HWREG(TWOD_BASE, 0x310)
#define COORD1_X_REG        HWREG(TWOD_BASE, 0x314)
#define COORD1_Y_REG        HWREG(TWOD_BASE, 0x318)
#define COORD2_REG          HWREG(TWOD_BASE, 0x320)
#define COORD2_X_REG        HWREG(TWOD_BASE, 0x324)
#define COORD2_Y_REG        HWREG(TWOD_BASE, 0x428)
#define COORD3_REG          HWREG(TWOD_BASE, 0x330)
#define COORD3_X_REG        HWREG(TWOD_BASE, 0x334)
#define COORD3_Y_REG        HWREG(TWOD_BASE, 0x338)
#define ROT_OC_REG          HWREG(TWOD_BASE, 0x340)
#define ROT_OC_X_REG        HWREG(TWOD_BASE, 0x344)
#define ROT_OC_Y_REG        HWREG(TWOD_BASE, 0x348)
#define ROTATE_REG          HWREG(TWOD_BASE, 0x34c)
#define X_INCR_REG          HWREG(TWOD_BASE, 0x400)
#define Y_INCR_REG          HWREG(TWOD_BASE, 0x404)
#define ROP_REG             HWREG(TWOD_BASE, 0x410)
#define ALPHA_REG           HWREG(TWOD_BASE, 0x420)
#define FG_COLOR_REG        HWREG(TWOD_BASE, 0x500)
#define BG_COLOR_REG        HWREG(TWOD_BASE, 0x504)
#define BS_COLOR_REG        HWREG(TWOD_BASE, 0x508)
#define SRC_COLOR_MODE_REG  HWREG(TWOD_BASE, 0x510)
#define DEST_COLOR_MODE_REG HWREG(TWOD_BASE, 0x514)
#define PATTERN0_REG        HWREG(TWOD_BASE, 0x600) // 0-31
#define PATOFF_REG          HWREG(TWOD_BASE, 0x700)
#define PATOFF_X_REG        HWREG(TWOD_BASE, 0x704)
#define PATOFF_Y_REG        HWREG(TWOD_BASE, 0x708)
#define STENCIL_CNTL_REG    HWREG(TWOD_BASE, 0x720)
#define STENCIL_DR_MIN_REG  HWREG(TWOD_BASE, 0x724)
#define STENCIL_DR_MAX_REG  HWREG(TWOD_BASE, 0x728)
#define SRC_BASE_ADDR_REG   HWREG(TWOD_BASE, 0x730)
#define DEST_BASE_ADDR_REG  HWREG(TWOD_BASE, 0x734)

#define CH_CFG              HWREG(SPI_BASE, 0x00)
#define Clk_CFG             HWREG(SPI_BASE, 0x04)
#define MODE_CFG            HWREG(SPI_BASE, 0x08)
#define Slave_slection_reg  HWREG(SPI_BASE, 0x0c)
#define HS_SPI_INT_EN       HWREG(SPI_BASE, 0x10)
#define HS_SPI_STATUS       HWREG(SPI_BASE, 0x14)
#define HS_SPI_TX_DATA      HWREG(SPI_BASE, 0x18)
#define HS_SPI_RX_DATA      HWREG(SPI_BASE, 0x1c)
#define Packet_Count_reg    HWREG(SPI_BASE, 0x20)
#define Pending_clr_reg     HWREG(SPI_BASE, 0x24)
#define SWAP_CFG            HWREG(SPI_BASE, 0x28)
#define FB_Clk_sel          HWREG(SPI_BASE, 0x2c)

#define SYSAD0              HWREG(HSMMC0_BASE, 0x00)
#define BLKSIZE0            HWREG(HSMMC0_BASE, 0x04)
#define BLKCNT0             HWREG(HSMMC0_BASE, 0x06)
#define ARGUMENT0           HWREG(HSMMC0_BASE, 0x08)
#define TRNMOD0             HWREG(HSMMC0_BASE, 0x0c)
#define CMDREG0             HWREG(HSMMC0_BASE, 0x0e)
#define RSPREG0_0           HWREG(HSMMC0_BASE, 0x10)
#define RSPREG1_0           HWREG(HSMMC0_BASE, 0x14)
#define RSPREG2_0           HWREG(HSMMC0_BASE, 0x18)
#define RSPREG3_0           HWREG(HSMMC0_BASE, 0x1c)
#define BDATA0              HWREG(HSMMC0_BASE, 0x20)
#define PRNSTS0             HWREG(HSMMC0_BASE, 0x24)
#define HOSTCTL0            HWREG(HSMMC0_BASE, 0x28)
#define PWRCON0             HWREG(HSMMC0_BASE, 0x29)
#define BLKGAP0             HWREG(HSMMC0_BASE, 0x2a)
#define WAKCON0             HWREG(HSMMC0_BASE, 0x2b)
#define CLKCON0             HWREG(HSMMC0_BASE, 0x2c)
#define TIMEOUTCON0         HWREG(HSMMC0_BASE, 0x2e)
#define SWRST0              HWREG(HSMMC0_BASE, 0x2f)
#define NORINTSTS0          HWREG(HSMMC0_BASE, 0x30)
#define ERRINTSTS0          HWREG(HSMMC0_BASE, 0x32)
#define NORINTSTSEN0        HWREG(HSMMC0_BASE, 0x34)
#define ERRINTSTSEN0        HWREG(HSMMC0_BASE, 0x36)
#define NORINTSIGEN0        HWREG(HSMMC0_BASE, 0x38)
#define ERRINTSIGEN0        HWREG(HSMMC0_BASE, 0x3a)
#define ACMD12ERRSTS0       HWREG(HSMMC0_BASE, 0x3c)
#define CAPAREG0            HWREG(HSMMC0_BASE, 0x40)
#define MAXCURR0            HWREG(HSMMC0_BASE, 0x48)
#define FEAER0              HWREG(HSMMC0_BASE, 0x50)
#define FEERR0              HWREG(HSMMC0_BASE, 0x52)
#define ADMAERR0            HWREG(HSMMC0_BASE, 0x54)
#define ADMASYSADDR0        HWREG(HSMMC0_BASE, 0x58)
#define CONTROL2_0          HWREG(HSMMC0_BASE, 0x80)
#define CONTROL3_0          HWREG(HSMMC0_BASE, 0x84)
#define DEBUG_0             HWREG(HSMMC0_BASE, 0x88)
#define CONTROL4_0          HWREG(HSMMC0_BASE, 0x8c)
#define HCVER0              HWREG(HSMMC0_BASE, 0xFE)

#define SYSAD1              HWREG(HSMMC1_BASE, 0x00)
#define BLKSIZE1            HWREG(HSMMC1_BASE, 0x04)
#define BLKCNT1             HWREG(HSMMC1_BASE, 0x06)
#define ARGUMENT1           HWREG(HSMMC1_BASE, 0x08)
#define TRNMOD1             HWREG(HSMMC1_BASE, 0x0c)
#define CMDREG1             HWREG(HSMMC1_BASE, 0x0e)
#define RSPREG0_1           HWREG(HSMMC1_BASE, 0x10)
#define RSPREG1_1           HWREG(HSMMC1_BASE, 0x14)
#define RSPREG2_1           HWREG(HSMMC1_BASE, 0x18)
#define RSPREG3_1           HWREG(HSMMC1_BASE, 0x1c)
#define BDATA1              HWREG(HSMMC1_BASE, 0x20)
#define PRNSTS1             HWREG(HSMMC1_BASE, 0x24)
#define HOSTCTL1            HWREG(HSMMC1_BASE, 0x28)
#define PWRCON1             HWREG(HSMMC1_BASE, 0x29)
#define BLKGAP1             HWREG(HSMMC1_BASE, 0x2a)
#define WAKCON1             HWREG(HSMMC1_BASE, 0x2b)
#define CLKCON1             HWREG(HSMMC1_BASE, 0x2c)
#define TIMEOUTCON1         HWREG(HSMMC1_BASE, 0x2e)
#define SWRST1              HWREG(HSMMC1_BASE, 0x2f)
#define NORINTSTS1          HWREG(HSMMC1_BASE, 0x30)
#define ERRINTSTS1          HWREG(HSMMC1_BASE, 0x32)
#define NORINTSTSEN1        HWREG(HSMMC1_BASE, 0x34)
#define ERRINTSTSEN1        HWREG(HSMMC1_BASE, 0x36)
#define NORINTSIGEN1        HWREG(HSMMC1_BASE, 0x38)
#define ERRINTSIGEN1        HWREG(HSMMC1_BASE, 0x3a)
#define ACMD12ERRSTS1       HWREG(HSMMC1_BASE, 0x3c)
#define CAPAREG1            HWREG(HSMMC1_BASE, 0x40)
#define MAXCURR1            HWREG(HSMMC1_BASE, 0x48)
#define FEAER1              HWREG(HSMMC1_BASE, 0x50)
#define FEERR1              HWREG(HSMMC1_BASE, 0x52)
#define ADMAERR1            HWREG(HSMMC1_BASE, 0x54)
#define ADMASYSADDR1        HWREG(HSMMC1_BASE, 0x58)
#define CONTROL2_1          HWREG(HSMMC1_BASE, 0x80)
#define CONTROL3_1          HWREG(HSMMC1_BASE, 0x84)
#define DEBUG_1             HWREG(HSMMC1_BASE, 0x88)
#define CONTROL4_1          HWREG(HSMMC1_BASE, 0x8c)
#define HCVER1              HWREG(HSMMC1_BASE, 0xFE)

#define VIDCON0             HWREG(LCD_BASE, 0x000)
#define VIDCON1             HWREG(LCD_BASE, 0x004)
#define VIDTCON0            HWREG(LCD_BASE, 0x008)
#define VIDTCON1            HWREG(LCD_BASE, 0x00c)
#define VIDTCON2            HWREG(LCD_BASE, 0x010)
#define WINCON0             HWREG(LCD_BASE, 0x014)
#define WINCON1             HWREG(LCD_BASE, 0x018)
#define VIDOSD0A            HWREG(LCD_BASE, 0x028)
#define VIDOSD0B            HWREG(LCD_BASE, 0x02c)
#define VIDOSD1A            HWREG(LCD_BASE, 0x034)
#define VIDOSD1B            HWREG(LCD_BASE, 0x038)
#define VIDOSD1C            HWREG(LCD_BASE, 0x03c)
#define VIDW00ADD0B0        HWREG(LCD_BASE, 0x064)
#define VIDW00ADD0B1        HWREG(LCD_BASE, 0x068)
#define VIDW01ADD0          HWREG(LCD_BASE, 0x06c)
#define VIDW00ADD1B0        HWREG(LCD_BASE, 0x07c)
#define VIDW00ADD1B1        HWREG(LCD_BASE, 0x080)
#define VIDW01ADD1          HWREG(LCD_BASE, 0x084)
#define VIDW00ADD2B0        HWREG(LCD_BASE, 0x094)
#define VIDW00ADD2B1        HWREG(LCD_BASE, 0x098)
#define VIDW01ADD2          HWREG(LCD_BASE, 0x09c)
#define VIDINTCON           HWREG(LCD_BASE, 0x0ac)
#define W1KEYCON0           HWREG(LCD_BASE, 0x0b0)
#define W1KEYCON1           HWREG(LCD_BASE, 0x0b4)
#define W2KEYCON0           HWREG(LCD_BASE, 0x0b8)
#define W2KEYCON1           HWREG(LCD_BASE, 0x0bc)
#define W3KEYCON0           HWREG(LCD_BASE, 0x0c0)
#define W3KEYCON1           HWREG(LCD_BASE, 0x0c4)
#define W4KEYCON0           HWREG(LCD_BASE, 0x0c8)
#define W4KEYCON1           HWREG(LCD_BASE, 0x0cc)
#define WIN0MAP             HWREG(LCD_BASE, 0x0d0)
#define WIN1MAP             HWREG(LCD_BASE, 0x0d4)
#define WPALCON             HWREG(LCD_BASE, 0x0e4)
#define SYSIFCON0           HWREG(LCD_BASE, 0x130)
#define SYSIFCON1           HWREG(LCD_BASE, 0x134)
#define DITHMODE            HWREG(LCD_BASE, 0x138)
#define SIFCCON0            HWREG(LCD_BASE, 0x13c)
#define SIFCCON1            HWREG(LCD_BASE, 0x140)
#define SIFCCON2            HWREG(LCD_BASE, 0x144)
#define CPUTRIGCON2         HWREG(LCD_BASE, 0x160)
#define WIN0Palette         HWREG(LCD_BASE, 0x400) // 0-255
#define WIN1Palette         HWREG(LCD_BASE, 0x800) // 0-255

#define ADCCON              HWREG(TSADC_BASE, 0x00)
#define ADCTSC              HWREG(TSADC_BASE, 0x04)
#define ADCDLY              HWREG(TSADC_BASE, 0x08)
#define ADCDAT0             HWREG(TSADC_BASE, 0x0c)
#define ADCDAT1             HWREG(TSADC_BASE, 0x10)
#define ADCUPDN             HWREG(TSADC_BASE, 0x14)
#define ADCMUX              HWREG(TSADC_BASE, 0x18)

#define IISCON              HWREG(IIS_BASE, 0x00)
#define IISMOD              HWREG(IIS_BASE, 0x04)
#define IISFIC              HWREG(IIS_BASE, 0x08)
#define IISPSR              HWREG(IIS_BASE, 0x0c)
#define IISTXD              HWREG(IIS_BASE, 0x10)
#define IISRXD              HWREG(IIS_BASE, 0x14)

#define AC_GLBCTRL          HWREG(AC_BASE, 0x00)
#define AC_GLBSTAT          HWREG(AC_BASE, 0x04)
#define AC_CODEC_CMD        HWREG(AC_BASE, 0x08)
#define AC_CODEC_STAT       HWREG(AC_BASE, 0x0c)
#define AC_PCMADDR          HWREG(AC_BASE, 0x10)
#define AC_MICADDR          HWREG(AC_BASE, 0x14)
#define AC_PCMDATA          HWREG(AC_BASE, 0x18)
#define AC_MICDATA          HWREG(AC_BASE, 0x1c)

#define PCM_CTL             HWREG(PCM_BASE, 0x00)
#define PCM_CLKCTL          HWREG(PCM_BASE, 0x04)
#define PCM_TXFIFO          HWREG(PCM_BASE, 0x08)
#define PCM_RXFIFO          HWREG(PCM_BASE, 0x0c)
#define PCM_IRQ_CTL         HWREG(PCM_BASE, 0x10)
#define PCM_IRQ_STAT        HWREG(PCM_BASE, 0x14)
#define PCM_FIFO_STAT       HWREG(PCM_BASE, 0x18)
#define PCM_CLRINT          HWREG(PCM_BASE, 0x20)

// ARM CPU MODE CONSTANTS
#define USER_MODE           0x10
#define FIQ_MODE            0x11
#define IRQ_MODE            0x12
#define SVC_MODE            0x13
#define ABT_MODE            0x17
#define UND_MODE            0x1b
#define SYS_MODE            0x1f


#define BPPMODE_1BPP        0x0 // uses palette
#define BPPMODE_2BPP        0x1 // uses palette
#define BPPMODE_4BPP        0x2 // uses palette
#define BPPMODE_8BPP        0x3 // uses palette
#define BPPMODE_16BPP565    0x5
#define BPPMODE_16BPP555    0x7
#define BPPMODE_18BPP       0x8
#define BPPMODE_24BPP       0xb // rgb

// Screen size
#define LCD_W               320
#define LCD_H               240
#define LCD_SCANLINE        320
#define LCD_H               240
#define SCREEN_BUFFERS      2

// Soft menu tab size
#define MENU_TAB_SPACE      1
#define MENU_TAB_INSET      2
#define MENU_TAB_WIDTH      ((LCD_W - 5 * MENU_TAB_SPACE) / 6)
#define MENU_TAB_HEIGHT     (FONT_HEIGHT(FONT_MENU) + 2 * MENU_TAB_INSET)

// CONSTANTS THAT CHANGE WITH DIFFERENT TARGETS
// Physical memory partition:

// 0x30000000 - 0x30100000 - Approx. 1 MB newRPL code AND data, including all
// persistent and scratch areas 0x30100000 - 0x31efffff - Free pages to be
// allocated to the different regions (30 MB approx. of free pages) 0x31f00000 -
// 0x31fbffff - Screen area (768 kbytes - 2 screens at full RGB color)
// 0x31fc0000 - 0x31ff3fff - MMU tables for all the different regions - Main
// table at 0x31ff0000

// 0x31ff4000 - 0x31ffff00  - Stacks for exception handlers
// 0x31ffff00 - 0x31ffff20  - Relocated exception handler pointers

// Note: Regular CPU Stack should not be here
#define RAM_BASE_PHYSICAL   0x30000000
#define RAM_END_PHYSICAL    0x31f00000
#define MEM_PHYS_SCREEN     0x31f00000
#define MEM_VIRT_SCREEN     MEM_PHYS_SCREEN
#define MEM_DSTKMMU         0x31fe8000
#define MEM_RSTKMMU         0x31fe0000
#define MEM_LAMMMU          0x31fd8000
#define MEM_DIRMMU          0x31fd0000
#define MEM_TEMPBLKMMU      0x31fc8000
#define MEM_TEMPOBMMU       0x31fc0000
#define MEM_REVERSEMMU      0x31fb8000

// newRPL for HP Prime G1 VIRTUAL memory map:
// 0x00010000 - 64 kbytes SRAM for exception handlers and CPU stack
// 0x02000000 - Data Stack
// 0x04000000 - Return Stack
// 0x06000000 - LAMs
// 0x08000000 - Directories
// 0x0a000000 - TempBlocks
// 0x10000000 - TempOb
// 0x30000000 - Physical memory for persistent and temporary variables (32 MB)

#define MEM_PHYSTACK        0x40010ffc // C program stack top
#define MEM_PHYSTACK_BASE   0x40000000 // C program stack limit
#define MEM_DSTK            0x02000000 // Data stack (up to 32 MB)
#define MEM_RSTK            0x04000000 // Return stack (up to 32 MB)
#define MEM_LAM             0x06000000 // Local variables (up to 32 MB)
#define MEM_DIRS            0x08000000 // Global directories (up to 32 MB)
#define MEM_TEMPBLOCKS      0x0a000000 // Block index for tempob (UP TO 32 MB)
#define MEM_TEMPOB          0x10000000 // Global object allocation (up to 32 MB)
#define MEM_SYSTEM          0x30000000 // System RAM (fixed amount)
#define MEM_SRAM            0x00000000 // On-chip SRAM
#define MEM_HARDWARE        0x48000000 // Hardware and peripherals


// USB RELATED DEFINITIONS

// MACRO TO READ GPIO INDICATING IF CABLE IS CONNECTED OR NOT
#define CABLE_IS_CONNECTED (*GPFDAT & 0x8)

// FIFO SIZE LIMITS CONFIGURED IN MPR
#define EP0_FIFO_SIZE      64
#define EP1_FIFO_SIZE      RAWHID_TX_SIZE // 512 POSSIBLE
#define EP2_FIFO_SIZE      RAWHID_RX_SIZE // 512 POSSIBLE


// BIT MASKS FOR VARIOUS HARDWARE REGISTERS

#define URSTCON_FUNC_RESET 4
#define URSTCON_PHY_RESET  1

#define EP0SR_RSR          1    // RSR = RX SUCCESSFULLY RECEIVED BIT
#define EP0SR_TST          2    // TST = TX SUCCESSFULLY RECEIVED BIT
#define EP0SR_SHT          0x10 // SHT = STALL HANDHAKE TRANSMITTED BIT
#define EP0SR_LWO          0x40 // LWO = LAST WORD ODD BIT; READONLY

#define EP0CR_TZLS         1 // TZLS = TX ZERO LENGTH SET BIT
#define EP0CR_ESS          2 // ESS = ENDPOINT STALL SET BIT

#define ESR_RPS            1    // RPS = RX PACKET SUCCESS BIT
#define ESR_TPS            2    // TPS = TX PACKET SUCCESS BIT
#define ESR_LWO            0x10 // LWO = LAST WORD ODD BIT; READONLY
#define ESR_FSC            0x20 // FSC = FUNCTION STALL CONDITION BIT
#define ESR_FFS            0x40 // FFS = FIFO FLUSHED BIT

#define ECR_ESS            2    // ESS = ENDPOINT STALL SET BIT
#define ECR_FLUSH          0x40 // FLUSH = FIFO FLUSH BIT

#define SSR_HFRES          1    // HOST FORCED RESET
#define SSR_HFSUSP         2    // HOST FORCED SUSPEND
#define SSR_HFRM           4    // HOST FORCED RESUME
#define SSR_TBM            0x80 // TOGGLE BIT MISMATCH

#define USB_DIRECTION      0x80
#define USB_DEV_TO_HOST    0x80

// Address of the serial number in this hardware
extern char SERIAL_NUMBER_ADDRESS[11];

// Override the device string on the USB bus
#undef STR_PRODUCT
#define STR_PRODUCT                                                          \
    {                                                                        \
        'n', 0, 'e', 0, 'w', 0, 'R', 0, 'P', 0, 'L', 0, ' ', 0, 'P', 0, 'r', \
            0, 'G', 0, '1', 0                                                \
    }
#undef STR_PRODLENGTH
#define STR_PRODLENGTH 22 + 2

// Override USB buffer size
#undef LONG_BUFFER_SIZE
#define LONG_BUFFER_SIZE 6 * 32 * RAWHID_RX_SIZE


// SPECIAL CONSTANTS FOR PROPER COMPILATION OF FIRMWARE IMAGE
#define SYSTEM_GLOBAL    __attribute__((section(".system_globals")))
#define DATAORDER1       __attribute__((section(".data_1")))
#define DATAORDER2       __attribute__((section(".data_2")))
#define DATAORDER3       __attribute__((section(".data_3")))
#define DATAORDERLAST    __attribute__((section(".data_last")))
#define SCRATCH_MEMORY   __attribute__((section(".scratch_memory")))
#define ROMOBJECTS       __attribute__((section(".romobjects")))
#define ROMLINK          __attribute__((section(".romlink")))

#ifndef TARGET_PC
typedef struct
{
    uint32_t mask1;
    uint32_t mask2;
} INTERRUPT_TYPE;
#endif

// This preamble preceeds PRIME_OS.ROM
struct Preamble
{
    uint32_t entrypoint;
    uint32_t unused1;
    uint32_t copy_size;
    uint32_t load_addr;
    uint32_t load_size;
    uint32_t cpu_arch;
    uint32_t cpuid;
    uint32_t unused2;
} __attribute__((packed));


/*

KEYBOARD BIT MAP
----------------
This is the bit number in the 64-bit keymatrix.
Bit set means key is pressed.

        AP]+  SY]+                   HL]+  ES]+
        |36|  |20|                   |61|  |52|
        +--+  +--+                   +--+  +--+

        HM]+  PL]+        UP]+       VW]+  CA]+
        |28|  |12|        |37|       |29|  |13|
        +--+  +--+  LF]+  +--+  RT]+ +--+  +--+
                    |57|  DN]+  |15|
              NM]+  +--+  |44|  +--+ ME]+
              |04|        +--+       |21|
              +--+                   +--+

        A]--+  B]--+  C]--+  D]--+  E]--+  BKS]+
        | 42|  | 58|  | 18|  | 10|  | 34|  | 02|
        +---+  +---+  +---+  +---+  +---+  +---+

        F]--+  G]--+  H]--+  I]--+  J]--+  K]--+
        | 59|  | 50|  | 43|  | 35|  | 27|  | 19|
        +---+  +---+  +---+  +---+  +---+  +---+

        L]--+  M]--+  N]--+  O]--+  ENTER]-----+
        | 11|  | 03|  | 60|  | 06|  |    07    |
        +---+  +---+  +---+  +---+  +----------+

        P]--+  7]---+  8]---+  9]---+  /]--+
        | 01|  | 22 |  | 14 |  | 05 |  | 17|
        +---+  +----+  +----+  +----+  +---+

        AL]-+  4]---+  5]---+  6]---+  *]--+
        | 26|  | 46 |  | 38 |  | 30 |  | 25|
        +---+  +----+  +----+  +----+  +---+

        RS]-+  1]---+  2]---+  3]---+  -]--+
        | 51|  | 45 |  | 62 |  | 54 |  | 33|
        +---+  +----+  +----+  +----+  +---+

        ON]-+  0]---+  .]---+  SP]--+  +]--+
        | 63|  | 09 |  | 53 |  | 49 |  | 41|
        +---+  +----+  +----+  +----+  +---+

*/

#define KB_ALPHA             26         //! Alpha
#define KB_ON                63         //! ON
#define KB_ESC               52         //! ESC key (prime only)
#define KB_DOT               53         //! Dot
#define KB_SPC               49         //! Space
#define KB_QUESTION          KB_HELP    //! Question -> Help
#define KB_SHIFT             51         //! Shift
#define KB_LSHIFT            51         //! Shift -> Left Shift
#define KB_RSHIFT            51         //! Shift -> Right Shift

#define KB_ADD               41         //! +
#define KB_SUB               33         //! -
#define KB_MUL               25         //! *
#define KB_DIV               17         //! /

#define KB_ENT                7         //! ENTER
#define KB_BKS                2         //! backspace
#define KB_UP                37         //! up arrow
#define KB_DN                44         //! down arrow
#define KB_LF                57         //! left arrow
#define KB_RT                15         //! right arrow

#define KB_F1                20         //! Function key 1 = SYMB
#define KB_F2                12         //! Function key 2 = PLOT
#define KB_F3                04         //! Function key 3 = NUM
#define KB_F4                61         //! Function key 4 = HELP
#define KB_F5                29         //! Function key 5 = VIEW
#define KB_F6                21         //! Function key 6 = MENU

#define KB_0                  9         //! 0
#define KB_1                 45         //! 1
#define KB_2                 62         //! 2
#define KB_3                 54         //! 3
#define KB_4                 46         //! 4
#define KB_5                 38         //! 5
#define KB_6                 30         //! 6
#define KB_7                 22         //! 7
#define KB_8                 14         //! 8
#define KB_9                  5         //! 9

#define KB_A                 42         //! A / Vars
#define KB_B                 58         //! B / Mem
#define KB_C                 18         //! C / Math / Units
#define KB_D                 10         //! D / Define
#define KB_E                 34         //! E / a b/c
#define KB_F                 59         //! F x^y
#define KB_G                 50         //! G SIN
#define KB_H                 43         //! H COS
#define KB_I                 35         //! I TAN
#define KB_J                 27         //! J LN
#define KB_K                 19         //! K LOG
#define KB_L                 11         //! L X^2
#define KB_M                  3         //! M +/=
#define KB_N                 60         //! N ()
#define KB_O                  6         //! O ,
#define KB_P                  1         //! P / EEX
#define KB_Q                 KB_7       //! Q / 7
#define KB_R                 KB_8       //! R / 8
#define KB_S                 KB_9       //! S / 9
#define KB_T                 KB_DIV     //! T / DIV
#define KB_U                 KB_4       //! U / 4
#define KB_V                 KB_5       //! V / 5
#define KB_W                 KB_6       //! W / 6
#define KB_X                 KB_MUL     //! X / MUL
#define KB_Y                 45         //! Y / 1
#define KB_Z                 62         //! Z / 2

// Prime-specific keys (using their names)
#define KB_APPS              36         //! APPS key (prime only)
#define KB_SYMB              20         //! SYMB key (prime only)
#define KB_HELP              61         //! HELP key (prime only)
#define KB_HOME              28         //! HOME key (prime only)
#define KB_PLOT              12         //! PLOT key (prime only)
#define KB_VIEW              29         //! VIEW key (prime only)
#define KB_CAS               13         //! CAS key (prime only)
#define KB_NUM                4         //! NUM key (prime only)
#define KB_MENU              21         //! MENU key (prime only)
#define KB_TOOL              58         //! TOOL key (prime only)

// Touchscreen related functions
extern void ts_init();

// HAL Required definitions

// Screen Update Notification

#define halScreenUpdated()  ((void) 0)

// DEFAULT CLOCK SPEEDS
#define HAL_SLOWCLOCK       100000000
#define HAL_USBCLOCK        400000000
#define HAL_FASTCLOCK       400000000

#define DEFAULT_AUTOOFFTIME 3

// DEFAULT COLOR MODE OF THE SYSTEM
#define BITS_PER_PIXEL      16
#define DEFAULT_BITMAP_MODE 3 // SAME AS BITMAP_RGB64K

#define PIXELS_PER_WORD     2


// LOW LEVEL TIMER FUNCTIONS FOR HARDWARE SETUP

// Do a single delay 100 usec
void tmr_delay100us();
// Do a single delay 10 msec
void tmr_delay10ms();
// Do a single delay 20 msec
void tmr_delay20ms();
// Prepare for an open loop timeout
void tmr_setuptimeoutms(int delayms,unsigned int *start,unsigned int *end);
// Check if clock timed out or not
int tmr_timedout(unsigned int start,unsigned int end);


// OTHER ADDITIONAL HARDWARE FUNCTIONS
// Reset all GPIO to a known state (low power for sleep mode)
void reset_gpio();

// We use only 2 fingers, no need for more than that...
#define TS_FINGERS      2
#define TS_STAT_INIT    1
#define TS_STAT_DATAMISSED 2

#define ENABLE_ARM_ASSEMBLY 1

// Magic word "NRPL"
#define NEWRPL_MAGIC   0x4c50524e

void uart_init(void);
void debug_print(const char *string);
void debug_print_hex(const char *key, uint32_t value);

#include <host.h>

#endif // TARGET_PRIME_H
