#include <ui.h>

extern unsigned int __saveint;

void cpu_intoff()
{
    //ARM ints off
    unsigned int volatile * INTMSK = (unsigned int*) (INT_REGS+0x8);

    if(!__saveint) {

    __saveint=*INTMSK;
    *INTMSK = 0xffffffff; //mask all interrupts

    }
}


// LOW-LEVEL VERSION, RETURN PREVIOUS STATE
// USED BY THE EXCEPTION HANDLERS
unsigned int __cpu_intoff()
{
    //ARM ints off
    unsigned int volatile * INTMSK = (unsigned int*) (INT_REGS+0x8);
    unsigned int previous=*INTMSK;
    *INTMSK = 0xffffffff; //mask all interrupts
    return previous;
}

void cpu_inton()
{
    //ARM ints on

    unsigned int volatile * INTMSK = (unsigned int*) (INT_REGS+0x8);

    if(__saveint) *INTMSK = __saveint;
}


// LOW-LEVEL VERSION USED BY THE EXCEPTION HANDLERS
// RESTORES A PREVIOUSLY SAVED INTERRUPT STATE
void __cpu_inton(unsigned int state)
{
    //ARM ints on

    unsigned int volatile * INTMSK = (unsigned int*) (INT_REGS+0x8);

    *INTMSK=state;
}

EXTERN void __tmr_fix();



#define HWREG(base,off) ( (volatile unsigned int *) (((int)base+(int)off)))

int __cpu_getFCLK()
{
int CLKSLOW=*HWREG(CLK_REGS,0x10);
int FCLK=12000000;

if(CLKSLOW&0x10) {	// slow mode
    if(CLKSLOW&7) FCLK/= (CLKSLOW&7)<<1;
    }
else {				// fast mode
    int PLLCON=*HWREG(CLK_REGS,0x4);
    int m=((PLLCON>>12)&0xff)+8,p=((PLLCON>>4)&0x1f)+2,s=PLLCON&3;

    FCLK=((unsigned)m*12000000)/(p*(1<<s));
}
return FCLK;
}


int __cpu_getHCLK()
{
int FCLK=__cpu_getFCLK();
int CLKDIVN=*HWREG(CLK_REGS,0x14);
if(CLKDIVN&2) FCLK>>=1;
return FCLK;
}

int __cpu_getPCLK()
{
int FCLK=__cpu_getFCLK();
int CLKDIVN=*HWREG(CLK_REGS,0x14);
if(CLKDIVN&2) FCLK>>=1;
if(CLKDIVN&1) FCLK>>=1;
return FCLK;
}

int cpu_getspeed()
{
    return __cpu_getFCLK();
}


int __cpu_setspeed(int PLLCON)
{

    lcd_sync();

volatile unsigned int *LCDPTR=(unsigned int *)LCD_REGS;
// TURN LCD OFF
LCDPTR[0]&=0xFFFFFFFE;

    if(PLLCON>0xffffff) {

    // SWITCH TO SLOW MODE
    *HWREG(CLK_REGS,0x10)=(*HWREG(CLK_REGS,0x10)& (~0x37)) | (PLLCON>>24);
    *HWREG(CLK_REGS,0x14)=*HWREG(CLK_REGS,0x14)& (~3);	// SET HCLK=FCLK; PCLK=HCLK=FCLK;

    // SWITCH MEMORY TIMINGS TO NORMAL
    *HWREG(MEM_REGS,0x4)= (*HWREG(MEM_REGS,0x4)&(~0x700)) | 0x300;
    *HWREG(MEM_REGS,0x8)=(*HWREG(MEM_REGS,0x8)&(~0x700)) | 0x300;

    // FIX TIMERS OPERATION
    __tmr_fix();

    // FIX LCD OPERATION
    __lcd_fix();


    return ((PLLCON>>24)&7)? 6000000/((PLLCON>>24)&7) : 12000000;
    }

    // SWITCH TO FAST MODE
    int m=((PLLCON>>12)&0xff)+8,p=((PLLCON>>4)&0x1f)+2,s=PLLCON&3;

    int FCLK=((unsigned)m*12000000)/(p*(1<<s));

    // SET MEMORY TIMINGS FOR VERY FAST MODES
    if(FCLK>75000000) {
    *HWREG(MEM_REGS,0x4)= (*HWREG(MEM_REGS,0x4)&(~0x700)) | 0x700;
    *HWREG(MEM_REGS,0x8)=(*HWREG(MEM_REGS,0x8)&(~0x700)) | 0x700;
    }

    // TURN PLL ON, ENTER SLOW MODE
    //*HWREG(CLK_REGS,0x10)=(*HWREG(CLK_REGS,0x10)& (~0x37)) | 0x10;

    // CHANGE SPEED
    *HWREG(CLK_REGS,0x4)=PLLCON;

    // KEEP HCLK AROUND 37.5 MHZ
    *HWREG(CLK_REGS,0x14)|=3;	// SET HCLK=FCLK/2; PCLK=HCLK/2;

    // SET TIMING FOR SLOWER MODES
    if(FCLK<=75000000) {
    *HWREG(MEM_REGS,0x4)= (*HWREG(MEM_REGS,0x4)&(~0x700)) | 0x300;
    *HWREG(MEM_REGS,0x8)=(*HWREG(MEM_REGS,0x8)&(~0x700)) | 0x300;
    }

    // DO SOMETHING TO WASTE PLL LOCK TIME

    // ENTER FAST MODE AFTER PLL LOCK TIME
    *HWREG(CLK_REGS,0x10)=(*HWREG(CLK_REGS,0x10)& (~0x37));



    // FIX LCD TIMING
    __lcd_fix();

    // FIX TIMERS
    __tmr_fix();

return FCLK;
}



// USER-FRIENDLY VERSION, ENSURE PROPER PLLCON IS USED
int cpu_setspeed(int mhz)
{
if(mhz>=192000000) return __cpu_setspeed(CLK_192MHZ);
if(mhz>=152000000) return __cpu_setspeed(CLK_152MHZ);
if(mhz>=120000000) return __cpu_setspeed(CLK_120MHZ);
if(mhz>= 75000000) return __cpu_setspeed(CLK_75MHZ);
if(mhz>= 48000000) return __cpu_setspeed(CLK_48MHZ);
if(mhz>= 12000000) return __cpu_setspeed(CLK_12MHZ);
//if(mhz>=  6000000) return __cpu_setspeed(CLK_6MHZ);
return __cpu_setspeed(CLK_6MHZ);
}

// PUT THE CPU IN "DOZE" MODE
void cpu_waitforinterrupt()
{
    asm volatile ("mov r0,#0");
    asm volatile ("mcr p15,0,r0,c7,c0,4");
}

// ACQUIRE A LOCK AND RETURN PREVIOUS VALUE
// IF PREVIOUS VALUE IS ZERO, LOCK WAS ACQUIRED SUCCESSFULLY
// IF NON-ZERO, LOCKING FAILED (RESOURCE WAS ALREADY LOCKED)
int __attribute__ ((noinline)) cpu_getlock(int lockvar,volatile int *lock_ptr)
{
    asm volatile ("swp %1,%1,[%2];" :"=r" (lockvar) : "r" (lockvar) ,"r" (lock_ptr) );

    return lockvar;
}

void cpu_flushwritebuffers(void)
{
    register unsigned int counter asm("r2");
    register unsigned int cacheaddr asm("r3");

    counter=0;
    while(counter<512) {
    cacheaddr=((counter>>1)&0xe0) | ((counter&63)<<26);
    // CLEAN AND INVALIDATE ENTRY USING INDEX
    // TODO: ONLY INVALIDATE ENTRIES USING MVA
    asm volatile ("mcr p15, 0, %0, c7, c14, 2" : : "r" (cacheaddr));

    ++counter;
    }

}

void cpu_flushTLB(void)
{
    register unsigned int value;

    value=0;
    asm volatile ("mcr p15, 0, %0, c8, c7, 0" : : "r" (value));

}

void cpu_off_prepare()
{
    // TODO: CHECK FOR SERIAL TRANSMISSIONS, SD CARD WRITE OPERATIONS, ETC BEFORE GOING DOWN


    // MASK ALL INTERRUPTS
    __cpu_intoff();

    lcd_off();

    // SET ALL GPIO LINES TO INPUT
    *HWREG(IO_REGS,0x60)=2; // ALL KEY LINES INPUT
    *HWREG(IO_REGS,0x68)=0;    // PULLUP ENABLED

    //*HWREG(IO_REGS,0x80)|=0x3000;   // USB PADS IN SUSPEND MODE AND ENABLE PULL-UPS FOR DATA BUS

    // CLEAR BITS ON GSTATUS
    *HWREG(IO_REGS,0xB4)=7;   // CLEAR ALL BITS ON GSTATUS2


    // SAVE SOMETHING IN RSTATUS3 AND 4
    *HWREG(IO_REGS,0xB8)=0x11223344;
    *HWREG(IO_REGS,0xBC)=0x55667788;


    // SETUP ON KEY TO WAKE UP
    *HWREG(IO_REGS,0x58)=0;   // PULLUP ENABLED
    *HWREG(IO_REGS,0x88)=6;   // TRY HIGH LEVEL - RISING EDGE TRIGGERS EINT0
    *HWREG(IO_REGS,0x50)=2;   // ONLY ENABLE EINT0, ALL OTHERS INPUT

    *HWREG(INT_REGS,0x8)=0xfffffffe;    // UNMASK ONLY THE ON INTERRUPT


    cpu_flushwritebuffers();

    // TODO: SETUP ALARM TO WAKE UP
    // FLUSH ALL BUFFERS
}


// WARNING: CALL THIS FUNCTION AFTER DISABLING MMU

#define PHYS_CLK_REGS 0x4c000000

void cpu_off_die()
{
    // GO OFF!
    *HWREG(PHYS_CLK_REGS,0)|=0x8;    // POWER OFF
    // DOES NOT RETURN

    while(1);
}
