/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

INTERRUPT_TYPE __saveint;

void cpu_intoff()
{
	if (!__saveint.mask1 && !__saveint.mask2) {
		__saveint.mask1 = *INTMSK1;
		__saveint.mask2 = *INTMSK2;
		*INTMSK1 = 0xffffffff;
		*INTMSK2 = 0xffffffff;
	}
}

INTERRUPT_TYPE __cpu_intoff()
{
	INTERRUPT_TYPE previous;
	previous.mask1 = *INTMSK1;
	previous.mask2 = *INTMSK2;

	*INTMSK1 = 0xffffffff;
	*INTMSK2 = 0xffffffff;

	return previous;	
}

void cpu_inton()
{
	if (__saveint.mask1 || __saveint.mask2) {
		*INTMSK1 = __saveint.mask1;
		*INTMSK2 = __saveint.mask2;
	}
}

void __cpu_inton(INTERRUPT_TYPE state)
{
	*INTMSK1 = state.mask1;
	*INTMSK2 = state.mask2;
}


// HP PRIME BOOTLOADER BOOT PROCEDURE
//; CLKDIV0=0x22D HCLKDIV=1, PCLKDIV=1, HALFHCLK=1 PREDIV=2 ARMDIV=1
//; CLKDIV1=0
//; MPLLCON=0x640061 SDIV=1 PDIV=6 MDIV=0X190 ONOFF=ON  (Fin=12MHz, SETS Fout=400MHz)
//; CLKSRC=0X118 (SELESRC=XTAL, SELMPLL=1 SELEXTCLK=1)
//; HCLKCON=0xFFFFFFFF (FEED THE CLOCK TO ALL PERIPHERALS)
//; PCLK= ~0x40 (ALL ENABLED EXCEPT SPI_HS0)
//; SCLKCON= ~0x6000 (ALL ENABLED EXCEPT HSMMCCLK_EXT AND SPICLK_0)
//; RSTCON=0X1fe80 (PWRSETCNT=0X80 RSTCNT=0XEF Clear PWROFF_SLP BY WRITING 1)
//; read INFORM2 register
//; IS INFORM2==0X55AA?
//; WakeupFromSleep ; YES, WE ARE WAKING UP, RESTORE STATE AND RESUME


int __cpu_getFCLK()
{
        int PLLCON = *MPLLCON;

        if(PLLCON&0x1000000) return 12000000;

        int m = (PLLCON >> 14)&0xff, p = ((PLLCON >> 5) & 0x3f) + 2, s =
                PLLCON & 7;

        return ((unsigned)m * 12000000) / (p * (1 << s));

}

int __cpu_getARMCLK()
{
    int FCLK = __cpu_getFCLK();
    int ARMDIV = (*CLKDIV0>>9) & 7;
    return FCLK / (ARMDIV+1);
}

int __cpu_getHCLK()
{
    int FCLK = __cpu_getFCLK();
    int HCLKDIV = *CLKDIV0 & 3;
    int PREDIV = (*CLKDIV0>>4) & 3;
    return FCLK / ( (PREDIV+1)*(HCLKDIV + 1) );
}

int __cpu_getPCLK()
{
    int HCLK = __cpu_getHCLK();
    int PCLKDIV = (*CLKDIV0>>2) & 1;
    if(PCLKDIV) HCLK >>= 1;
    return HCLK;
}

// ACQUIRE A LOCK AND RETURN PREVIOUS VALUE
// IF PREVIOUS VALUE IS ZERO, LOCK WAS ACQUIRED SUCCESSFULLY
// IF NON-ZERO, LOCKING FAILED (RESOURCE WAS ALREADY LOCKED)
__ARM_MODE__ int cpu_getlock(int lockvar, volatile int *lock_ptr)
{
    asm volatile ("swp %1,%1,[%2];":"=r" (lockvar):"r"(lockvar), "r"(lock_ptr));

    return lockvar;
}
