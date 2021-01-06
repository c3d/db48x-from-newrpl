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
// MSysCLK = 400 MHz ; ARMCLK = 400 MHz, HCLK = 100 MHz, PCLK = 50 MHz, DDRCLK=2*HCLK=200 MHz
// 200000000 = 200MHz:
// MSysCLK = 400 MHz ; ARMCLK = 200 MHz, HCLK = 100 MHz, PCLK = 50 MHz, DDRCLK=2*HCLK=200 MHz


// Given a PLL configuration, set the clock and adjust all other hardware clocks to comply with specs
// HCLK target is 100 MHz
// PCLK target is 50 MHz
// ARMCLK target has to be an integer multiple of HCLK


// Auxiliary function that fixes timer frequency every time the cpu clock is adjusted
extern void __tmr_fix();

int __cpu_setspeed(unsigned int mode)
{

    // Check if LCD is already on, then adjust cpu speed only at end of frame
    // and fix the LCD frequency
    if(*VIDCON0&3) {
        *VIDCON0 = (*VIDCON0&~3)|0x2;     // Request LCD signals off at end of current frame
        while(*VIDCON0&1) ; // And wait for it to happen
    }


switch(mode)
{
case 400:
    // MSysCLK = 400 MHz ; ARMCLK = 400 MHz, HCLK = 100 MHz, PCLK = 50 MHz, DDRCLK=2*HCLK=200 MHz
    // Max. performance MsysCLK = 800 MHz
    //          MDIV        PDIV    SDIV
    *MPLLCON= (400<<14) | (3<<5) | (2);

    //         ARMDIV   PREDIV   PCLKDIV  HCLKDIV
    *CLKDIV0 = (0<<9) | (1<<4) | (1<<2) | (1) ;
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

if(*VIDCON0&3) {
    __lcd_fix();        // Recalculate frequency and fix it
    *VIDCON0 |= 3;      // Enable LCD again
}


}


// Support only a few speeds:
// Slow mode at 100 MHz
// Full speed mode is at 400 MHz

int cpu_setspeed(int freq)
{

// Disable this, we need specialized procedure to change clocks
// while running from DRAM.

// TODO: Ideally, code needs to run from SRAM and wait until the DRAM clock stabilizes to return
// or do NOT change HCLK, ever, to avoid messing with the DDR timings
// DDR clock is locked to twice the HCLK, so HCLK must be changed very carefully


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


// PUT THE CPU IN "DOZE" MODE
__ARM_MODE__ void cpu_waitforinterrupt()
{
    register unsigned int var = 0;
    asm volatile ("mcr p15,0,%0,c7,c0,4"::"r" (var));
}



__ARM_MODE__ void cpu_off_prepare()
{
    // TODO: CHECK FOR SERIAL TRANSMISSIONS, SD CARD WRITE OPERATIONS, ETC BEFORE GOING DOWN

    // MASK ALL INTERRUPTS

    *INTSUBMSK = 0xffffffff; // INTSUBMSK ALL MASKED
    *INTMSK1 = 0xffffffff; // INTMSK ALL MASKED
    *INTMSK2 = 0xffffffff; // INTMSK ALL MASKED
    *EINTMASK = 0xfff0;   // EINTMSK ALL MASKED

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    lcd_off();

    // SET GPB9 and GPF4 TO ZERO TO POWER DOWN THE LCD CHIP
    *GPBDAT = (*GPBDAT & ~0x200);                 // GPB9 POWER UP THE LCD DRIVER CHIP
    *GPFDAT = (*GPFDAT & ~0x10);                  // GPF4 POWER UP THE LCD DRIVER CHIP

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // PREPARE ALL GPIO BLOCKS FOR POWEROFF

    // TIMERS
    *TCON = 0;  // STOP ALL RUNNING TIMERS

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER
/*
    // GPIO-A

    *GPADAT = 0;
    *GPACON = 0X7FFFFF;      //ALL FUNCTIONAL PINS

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-B

    *GPBDAT = 0;
    *GPBCON = 0x155555;   // ALL OUTPUTS AND ALL AT LOW STATE
    *GPBUDP = 0;      // ALL PULLUPS/PULLDOWN DISABLED
    *GPBSEL = 0;

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-C

    *GPCDAT = 0;
    *GPCCON = 0x55555555; // ALL OUTPUTS AND ALL AT LOW STATE
    *GPCUDP = 0;     // ALL PULLUPS DISABLED

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-D

    *GPDDAT = 0;
    *GPDCON = 0x55555555; // ALL OUTPUTS AND ALL AT LOW STATE
    *GPDUDP = 0;     // ALL PULLUPS DISABLED

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-E

    *GPEDAT = 0;
    *GPECON = 0x55555555; // ALL OUTPUTS AND ALL AT LOW STATE
    *GPEUDP = 0;     // ALL PULLUPS DISABLED
    *GPESEL = 0;

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-F

    *GPFDAT = 0;
    *GPFCON = 0x5555;     // ALL OUTPUTS AND ALL AT LOW STATE
    *GPFUDP = 0;       // ALL PULLUPS DISABLED

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-G

    *GPGDAT = 0;
    *GPGCON = 0x5555; // ALL OUTPUTS AND ALL AT LOW STATE
    *GPGUDP = 0;     // ALL PULLUPS DISABLED

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-H

    *GPHDAT = 0;
    *GPHCON = 0x15555555;   // ALL OUTPUTS AND ALL AT LOW STATE
    *GPHUDP = 0;      // ALL PULLUPS DISABLED

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-K

    *GPKDAT = 0;
    *GPKCON = 0x55555555;   // ALL OUTPUTS AND ALL AT LOW STATE
    *GPKUDP = 0;      // ALL PULLUPS DISABLED

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-L

    *GPLDAT = 0;
    *GPLCON = 0x4050055;   // ALL OUTPUTS AND ALL AT LOW STATE
    *GPLUDP = 0;      // ALL PULLUPS DISABLED

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // GPIO-M

    *GPMDAT = 0;
    *GPMCON = 0xA;   // SELECT FUNCTION (Flash/OneNAND related)
    *GPMUDP = 0;      // ALL PULLUPS DISABLED
*/

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    //*DSC0 = 0x2aaaaaaa; // Reset values
    //*DSC1 = 0x0aaaaaaa; // Reset values
    //*DSC2 = 0x0aa8aaaa; // Reset values
    //*DSC3 = 0x000002aa; // Reset values

    //*PDDMCON = 0x00aaaaaa;  // All Hi-Z - We probably shouldn't be touching these
    //*PDSMCON = 0x00aaaaaa;  // All Hi-Z - We probably shouldn't be touching these


     *MISCCR |= 0x1000;          // USB PORT INTO SUSPEND MODE

    // SIGNAL THAT IT'S US GOING INTO POWEROFF MODE
    *INFORM2 = 0x4c50524e;  // 'NRPL' SIGNAL THE BOOT PROCEDURE THAT WE WANT TO STAY IN NEWRPL

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER




    // SETUP ON KEY TO WAKE UP
    // 01 01 01 01 01 01 01 10 =0x5556
    //  7  6  5  4  3  2  1  0
    *GPGCON = 0x5556;     // ONLY ENABLE EINT8, ALL OTHERS OUTPUT
    *GPGUDP = 1;          // ENABLE PULLDOWN ON GPG0
    *EXTINT1 = 6;         // BOTH EDGES TRIGGER EINT8

    asm volatile ("mov r0,r0"); // USE NOPS AS BARRIER TO FORCE COMPILER TO RESPECT THE ORDER

    // CLEAR SRCPEND, INTPEND BITS

    *SRCPND1 = *SRCPND1;
    *SRCPND2 = *SRCPND2;
    *SUBSRCPND = *SUBSRCPND;    // SUB-SRCPND
    *INTPND1 = *INTPND1;
    *INTPND2 = *INTPND2;
    *EINTPEND = *EINTPEND;

    *INTMSK1 = 0xffffffdf; // UNMASK ONLY THE ON KEY INTERRUPT
    *EINTMASK = 0xfef0;    // UNMASK THE ON KEY INTERRUPT
    // TODO: SETUP ALARM TO WAKE UP
    // FLUSH ALL BUFFERS

    cpu_flushwritebuffers();

}

// WARNING: CALL THIS FUNCTION AFTER DISABLING MMU
struct PRIME_BL_WAKEUP {
    unsigned int pc_resume;
    unsigned int cpu_control_c1;
    unsigned int cpu_mmu_base;
    unsigned int cpu_domaccess;
} __cpu_wakeup_struct __SCRATCH_MEMORY__;


// Startup function is supplied by boot module
extern void startup(void);


__ARM_MODE__ void cpu_off_die()
{
    register unsigned int data;


    // SAVE CPU STATE TO DATA STRUCTURE REQUIRED BY BOOTLOADER
    __cpu_wakeup_struct.pc_resume=(unsigned int)&startup;

    asm volatile ("mrc p15,0,%[reg],c1,c0,0" : [ reg ] "=r" (data) );
    __cpu_wakeup_struct.cpu_control_c1=data;

    asm volatile ("mrc p15,0,%[reg],c2,c0,0" : [ reg ] "=r" (data) );
    __cpu_wakeup_struct.cpu_mmu_base=data;

    asm volatile ("mrc p15,0,%[reg],c3,c0,0" : [ reg ] "=r" (data) );
    __cpu_wakeup_struct.cpu_domaccess=data;

    *INFORM3=(unsigned int)&__cpu_wakeup_struct;
    *INFORM2=0x55aa;

    *PWRCFG = 0x28088;

    *PWRMODE = 0x2bed;  // POWER OFF
    // DOES NOT RETURN
    *GPCCON= (*GPCCON&~0x3000) | 0x1000; // GPC6 TO OUTPUT
    *GPCDAT|= 1<<6 ;

    while(1);

}
