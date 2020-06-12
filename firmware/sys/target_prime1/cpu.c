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

// Set CPU Clock Speed in Hz
// Peripherals clock will be modified accordingly

// Valid clocks for this target:
// 400000000 = 400MHz:
// MSysCLK = 800 MHz ; ARMCLK = 400 MHz, HCLK = 133 MHz, PCLK = 66 MHz, DDRCLK=2*HCLK=266 MHz
// 200000000 = 200MHz:
// MSysCLK = 800 MHz ; ARMCLK = 200 MHz, HCLK = 100 MHz, PCLK = 50 MHz, DDRCLK=2*HCLK=200 MHz


// Given a PLL configuration, set the clock and adjust all other hardware clocks to comply with specs
// HCLK target is 100 MHz
// PCLK target is 50 MHz
// ARMCLK target has to be an integer multiple of HCLK


// Auxiliary function that fixes timer frequency every time the cpu clock is adjusted
extern void __tmr_fix();

int __cpu_setspeed(unsigned int mode)
{
switch(mode)
{



case 400:
    // MSysCLK = 800 MHz ; ARMCLK = 400 MHz, HCLK = 133 MHz, PCLK = 66 MHz, DDRCLK=2*HCLK=266 MHz
    // Max. performance MsysCLK = 800 MHz
    //          MDIV        PDIV    SDIV
    *MPLLCON = (400<<14) | (3<<5) | (1);

    //         ARMDIV   PREDIV   PCLKDIV  HCLKDIV
    *CLKDIV0 = (1<<9) | (2<<4) | (1<<2) | (1) ;
    break;
case 200:
    // MSysCLK = 400 MHz ; ARMCLK = 200 MHz, HCLK = 100 MHz, PCLK = 50 MHz, DDRCLK=2*HCLK=200 MHz
    //         MDIV        PDIV    SDIV
    *MPLLCON= (400<<14) | (3<<5) | (2);

    //         ARMDIV   PREDIV   PCLKDIV  HCLKDIV
    *CLKDIV0 = (1<<9) | (1<<4) | (1<<2) | (1) ;
    break;
default:
case 100:
    // MSysCLK = 400 MHz ; ARMCLK = 100 MHz, HCLK = 100 MHz, PCLK = 50 MHz, DDRCLK=2*HCLK=200 MHz
    //         MDIV        PDIV    SDIV
    *MPLLCON= (400<<14) | (3<<5) | (2);

    //         ARMDIV   PREDIV   PCLKDIV  HCLKDIV
    *CLKDIV0 = (3<<9) | (1<<4) | (1<<2) | (1) ;
    break;

}

// TODO: Also set the EPLL clocks although this clock doesn't change
//*EPLLCON= 0x200102; // 96 MHz regardless of CPU speed
//*CLKSRC=0x118;

__tmr_fix();

}


// Support only a few speeds:
// Slow mode at 100 MHz
// Full speed mode is at 400 MHz

int cpu_setspeed(int freq)
{
    if(freq >= 400000000)
        return __cpu_setspeed(400);
    if(freq >= 200000000)
        return __cpu_setspeed(200);
    return __cpu_setspeed(100);
}



int __cpu_getFCLK()
{
        int PLLCON = *MPLLCON;

        if(PLLCON&0x1000000) return 12000000;

        int m = (PLLCON >> 14)&0x3ff, p = ((PLLCON >> 5) & 0x3f) , s =
                PLLCON & 7;

        return ((long long)m * 12000000LL) / (p * (1 << s));

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



__ARM_MODE__ void cpu_flushwritebuffers(void)
{
    register unsigned int value;

    value = 0;

    // Test, Clean and invalidate DCache
    // Uses special behavior of R15 as per ARM 926EJ-S Reference Manual
    asm volatile("flush_loop:\n"
                 "mrc p15, 0, r15, c7, c14, 3\n"
                 "bne flush_loop");


    // Drain write buffers to make sure everything is written before returning
    asm volatile ("mcr p15, 0, %0, c7, c10, 4"::"r" (value));

    // Make sure the prefetched operations that are read into the pipeline before the cache is flushed don't read any data
    asm volatile ("nop");
    asm volatile ("nop");

}


__ARM_MODE__ void cpu_flushicache(void)
{
    register unsigned int value;

    // Invalidate ICache
    value = 0;
    asm volatile ("mcr p15, 0, %0, c7, c5, 0"::"r" (value));

    // Make sure the prefetched operations that are read into the pipeline before the cache is flushed are well-known
    asm volatile ("nop");
    asm volatile ("nop");

}

__ARM_MODE__ void cpu_flushTLB(void)
{
    register unsigned int value;

    value = 0;
    asm volatile ("mcr p15, 0, %0, c8, c7, 0"::"r" (value));

    // Make sure all instructions past the return of this function will be read using a full lookup through the MMU table.
    asm volatile ("nop");
    asm volatile ("nop");
}
