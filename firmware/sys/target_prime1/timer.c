/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

// IMPORT A FEW FUNCTIONS FROM THE CPU MODULE
int __cpu_getPCLK();

void __tmr_eventreschedule();

// TIMERS
volatile long long __systmr __SYSTEM_GLOBAL__;
volatile long long __evtmr __SYSTEM_GLOBAL__;
unsigned int __sysfreq __SYSTEM_GLOBAL__;
volatile int __tmr_lock __SYSTEM_GLOBAL__;
timed_event tmr_events[NUM_EVENTS] __SYSTEM_GLOBAL__;

// SAVE TIMERS CONFIGURATION - 13 WORDS REQUIRED
void tmr_save(unsigned int *tmrbuffer)
{

    tmrbuffer[0] = *TCFG0;
    tmrbuffer[1] = *TCFG1;
    tmrbuffer[2] = *TCON;
    tmrbuffer[3] = *TCNTB0;
    tmrbuffer[4] = *TCMPB0;

    tmrbuffer[5] = *TCNTB1;
    tmrbuffer[6] = *TCMPB1;
    tmrbuffer[7] = *TCNTB2;
    tmrbuffer[8] = *TCMPB2;
    tmrbuffer[9] = *TCNTB3;
    tmrbuffer[10] = *TCMPB3;
    tmrbuffer[11] = *TCNTB4;
    tmrbuffer[12] = *INTMSK1;      // GET TIMERS INTERRUPT MASK
}

// RESTORE TIMERS

void tmr_restore(unsigned int *tmrbuffer)
{
// MASK ALL TIMER INTERRUPTS
    *INTMSK1 |= 0x7c00;

// STOP ALL RUNNING TIMERS
    *TCON &= 0xFF8000e0;

// CLEAN ALL PENDING INTERRUPTS
    *INTPND1 = 0x7c00;
    *SRCPND1 = 0x7c00;

// RESTORE REGISTERS
    *TCFG0 = tmrbuffer[0];
    *TCFG1 = tmrbuffer[1];
    *TCNTB0 = tmrbuffer[3];
    *TCMPB0 = tmrbuffer[4];

    *TCNTB1 = tmrbuffer[5];
    *TCMPB1 = tmrbuffer[6];
    *TCNTB2 = tmrbuffer[7];
    *TCMPB2 = tmrbuffer[8];
    *TCNTB3 = tmrbuffer[9];
    *TCMPB3 = tmrbuffer[10];
    *TCNTB4 = tmrbuffer[11];

    *INTMSK1 =
            (*INTMSK1 & (~0x7c00)) | (tmrbuffer[12] & 0x7c00);

// FINALLY RESTART ALL TIMERS
    *TCON = tmrbuffer[2];
}

// THIS EVENT IS CALLED TO CHECK IF WE HAVE AN EVENT
void __tmr_newirqeventsvc()
{
    if(cpu_getlock(2, &__tmr_lock))
        return; // GET A LOCK ON THE TIMERS, ABORT IF SOMEBODY ELSE HAS IT
    __tmr_lock = 1;

    // GET THE CURRENT SYSTEM TICKS
    tmr_t current_ticks = tmr_ticks();
    int f;

    if(__evtmr && (current_ticks >= __evtmr)) {
        // PROCESS THE NEXT EVENT
        for(f = 0; f < NUM_EVENTS; ++f) {
            if(tmr_events[f].status & 1) {
                if(tmr_events[f].ticks <= __evtmr) {
                    // EXECUTE EVENT
                    (*(tmr_events[f].eventhandler)) ();
                    if((tmr_events[f].status & 3) == 3) {
                        // AUTORELOAD
                        tmr_events[f].ticks += tmr_events[f].delay;
                    }
                    else {
                        tmr_events[f].status = 0;
                    }
                }
            }
        }
    }

    __tmr_lock = 0;

    __tmr_eventreschedule();

}

void __tmr_irqservice()
{
    __systmr += 0x10000;

    if(__evtmr && (__evtmr - __systmr < 0x10000)
            && !(*TCON & 0x100))
        __tmr_newirqeventsvc();

}

// SYSTEM TIMER FREQUENCY = 100 KHz
// ALTERNATIVELY, SWITCH TO 125 KHz IF 100 KHz CANNOT BE CORRECTLY REPRESENTED
#define SYSTIMER_FREQ 100000
#define ALTSYSTIMER_FREQ 125000

// PREPARE TIMERS
void tmr_setup()
{

// MASK ALL TIMER INTERRUPTS
    *INTMSK1 |= 0x7c00;

// STOP ALL RUNNING TIMERS
    *TCON &= 0xFF8000e0;

// CLEAN ALL PENDING INTERRUPTS
    *INTPND1 = 0x7c00;
    *SRCPND1 = 0x7c00;

// START TIMER0 AS 64-BIT RUNNING TIMER AT 100 KHz
    __tmr_lock = 0;
    __systmr = 0;

    unsigned int pclk = __cpu_getPCLK();

    unsigned int divider, prescaler;

    prescaler = (pclk << 3) / SYSTIMER_FREQ;
    divider = 1;

    if(prescaler & 0xf) {
// WARNING: REQUESTED FREQUENCY CANNOT BE REPRESENTED ACCURATELY
// THIS ONLY HAPPENS AT 75 MHz, ALL OTHER SPEEDS ARE OK
        prescaler = (pclk << 3) / ALTSYSTIMER_FREQ;
    }

    while(prescaler > (1 << (11 + divider))) {
        divider++;
    }

    prescaler += (1 << (2 + divider));
    prescaler >>= divider + 3;

//if(divider>4) PCLK TOO HIGH TO ACHIEVE TIMER FREQUENCY, USE HIGHER MULTIPLE
    if(divider > 4)
        divider = 4;

// CALCULATE SYSTEM CLOCK FREQUENCY
    __sysfreq =
            (((pclk << 3) / prescaler) + (1 << (divider + 2))) >> (divider + 3);

// SET PRESCALER VALUES FOR TIMERS 0 AND 1
    *TCFG0 = (*TCFG0 & (~0xFF)) | (prescaler - 1);
    *TCFG1 =
            (*TCFG1 & (~0xf000ff)) | (divider - 1) | ((divider - 1) << 4);

// SET COUNT VALUES TO MAXIMUM
    *TCNTB0 = 0xffff;
    *TCMPB0 = 0;
    *TCNTB1 = 0xffff;
    *TCMPB1 = 0;

// PREPARE TIMED EVENTS HANDLER
    int k;
    for(k = 0; k < NUM_EVENTS; ++k)
        tmr_events[k].status = 0;

    __evtmr = 0;

    __irq_addhook(10, (__interrupt__) & __tmr_irqservice);
    __irq_addhook(11, (__interrupt__) & __tmr_newirqeventsvc);

// UNMASK INTERRUPTS FOR TIMERS 0 AND 1
    *INTMSK1 &= ~0xc00;

// SET MANUAL UPDATE BIT
    *TCON |= 2;
// START TIMER0
    *TCON = (*TCON & (~0x1f)) | 0x9;

}

// FIX TIMERS SPEED WHEN CPU CLOCK IS CHANGED, CALLED FROM cpu_setspeed
void __tmr_fix()
{
// MASK ALL TIMER INTERRUPTS
//*HWREG(INT_REGS,0x8)|=0x7c00;

// STOP ALL RUNNING TIMERS
//*HWREG(TMR_REGS,8)&=0xFF8000e0;

// CLEAN ALL PENDING INTERRUPTS
//*HWREG(INT_REGS,0x10)=0x7c00;
//*HWREG(INT_REGS,0)=0x7c00;

// START TIMER0 AS 64-BIT RUNNING TIMER AT 100 KHz

    unsigned int pclk = __cpu_getPCLK();

    unsigned int divider, prescaler;

    prescaler = (pclk << 3) / SYSTIMER_FREQ;
    divider = 1;

    if(prescaler & 0xf) {
// WARNING: REQUESTED FREQUENCY CANNOT BE REPRESENTED ACCURATELY
// THIS ONLY HAPPENS AT 75 MHz, ALL OTHER SPEEDS ARE OK
        prescaler = (pclk << 3) / ALTSYSTIMER_FREQ;
    }

    while(prescaler > (1 << (11 + divider))) {
        divider++;
    }

    prescaler += (1 << (2 + divider));
    prescaler >>= divider + 3;

//if(divider>4) PCLK TOO HIGH TO ACHIEVE TIMER FREQUENCY, USE HIGHER MULTIPLE
    if(divider > 4)
        divider = 4;

// CALCULATE SYSTEM CLOCK FREQUENCY
    __sysfreq =
            (((pclk << 3) / prescaler) + (1 << (divider + 2))) >> (divider + 3);

// SET PRESCALER VALUES FOR TIMERS 0 AND 1
    *TCFG0 = (*TCFG0 & (~0xFF)) | (prescaler - 1);
    *TCFG1 = (*TCFG1 & (~0xf000ff)) | (divider - 1) | ((divider - 1) << 4);

}

tmr_t tmr_getsysfreq()
{
    return __sysfreq;
}

tmr_t tmr_ticks()
{
    unsigned int *timer0 = (unsigned int *)TCNTO0;

    unsigned int before;
    unsigned long long ticks1, ticks2;

    do {
        ticks1 = __systmr;
        before = *timer0;
        ticks2 = __systmr;
    }
    while(ticks1 != ticks2);

    return ticks1 + 0x10000 - before;
}

// RETURN DELAY IN MILLISECONDS
int tmr_ticks2ms(tmr_t before, tmr_t after)
{
    return ((after - before) * 1000) / __sysfreq;
}

// RETURN DELAY IN MICROSECONDS
int tmr_ticks2us(tmr_t before, tmr_t after)
{
    return ((after - before) * 1000000) / __sysfreq;
}

// ADD/SUBTRACT AN INTERVAL IN MILLISECONDS TO THE GIVEN TIME IN TICKS
tmr_t tmr_addms(tmr_t time, int ms)
{
    return time + ((ms * __sysfreq) / 1000);
}

// ADD/SUBTRACT AN INTERVAL IN MICROSECONDS TO THE GIVEN TIME IN TICKS
tmr_t tmr_addus(tmr_t time, int us)
{
    return time + ((us * __sysfreq) / 1000000);
}

void tmr_delayms(int milliseconds)
{
    if(milliseconds <= 0)
        return;
    tmr_t start = tmr_ticks();

// CALCULATE ENDING TICKS
    tmr_t end = start + ((milliseconds * __sysfreq) / 1000);

// AND WAIT
    while(end > tmr_ticks());

}

void tmr_delayus(int microseconds)
{
    if(microseconds <= 0)
        return;

    unsigned long long start = tmr_ticks();

// CALCULATE ENDING TICKS
    unsigned long long end =
            start + ((microseconds * tmr_getsysfreq()) / 1000000);

// AND WAIT
    while(end > tmr_ticks());

}

// WAIT UNTIL THE TIME REACHES A CERTAIN VALUE
void tmr_waituntil(tmr_t time)
{
    while(tmr_ticks() < time);
    return;
}

// RETURN AN EVENT HANDLER
HEVENT tmr_eventcreate(__interrupt__ handler, unsigned int ms, int autorepeat)
{
    int f;

// NOTE: EVENT 0 IS RESERVED FOR THE SYSTEM (KEYBOARD DRIVER)

    for(f = 1; f < NUM_EVENTS; ++f) {
        if(!tmr_events[f].status) {
            // FOUND AVAILABLE EVENT
            tmr_t ticks = tmr_ticks();

            tmr_events[f].eventhandler = handler;
            tmr_events[f].delay = (ms * tmr_getsysfreq()) / 1000;
            tmr_events[f].ticks = ticks + tmr_events[f].delay;
            tmr_events[f].status = ((autorepeat) ? 2 : 0) | 1;

            __tmr_eventreschedule();
            return f;
        }
    }
    return -1;
}

void __tmr_eventreschedule()
{
    if(cpu_getlock(2, &__tmr_lock))
        return; // GET A LOCK ON THE TIMERS, ABORT IF SOMEBODY ELSE HAS IT

    tmr_t current_ticks;
    tmr_t next;
    int f;

    *TCON = (*TCON & (~0xf00)); // STOP TIMER1

    __tmr_lock = 1;

    next = 0;
    for(f = 0; f < NUM_EVENTS; ++f) {
        if(tmr_events[f].status & 1) {
            // GET THE NEXT EVENT READY
            if(next) {
                if(tmr_events[f].ticks < next)
                    next = tmr_events[f].ticks;
            }
            else
                next = tmr_events[f].ticks;
        }
    }

  restart:

    // HERE next HAS THE NEXT SCHEDULED EVENT
    __evtmr = next;

    if(next) {

        current_ticks = tmr_ticks();
        if(next - current_ticks >= 0x10000) {
            // NO NEED TO DO ANYTHING
        }
        else {
            // SET THE TIMER1 TO WAKE US UP EXACTLY AT THE RIGHT TIME
            if(next <= current_ticks) {
                *TCNTB1 = 50;    // THIS EVENT IS LATE! RUN IN HALF A MILLISECOND TO CATCH UP
                __evtmr = current_ticks + 50;
            }
            else
                *TCNTB1 = next - current_ticks;
            *TCON = (*TCON & (~0xf00)) | 0x200; // MANUAL UPDATE
            *TCON = (*TCON & (~0xf00)) | 0x100; // RESTART TIMER, SINGLE SHOT

        }
    }

    while(__tmr_lock == 2) {
        // SOMEBODY TRIED TO ACQUIRE THE LOCK WHILE WE WERE USING IT
        // MUST REDO THE TIMING IN CASE THEY ADDED/REMOVED EVENTS
        __tmr_lock = 1;
        next = 0;
        int f;
        for(f = 0; f < NUM_EVENTS; ++f) {
            if(tmr_events[f].status & 1) {
                // GET THE NEXT EVENT READY
                if(next) {
                    if(tmr_events[f].ticks < next)
                        next = tmr_events[f].ticks;
                }
                else
                    next = tmr_events[f].ticks;
            }
        }

        if(__evtmr != next)
            goto restart;
    }

    __tmr_lock = 0;

}

void tmr_eventpause(HEVENT event)
{
    if(event < 0 || event >= NUM_EVENTS)
        return;
    tmr_events[event].status |= 4;      // PREVENT EVENT FROM BEING REUSED
    tmr_events[event].status &= ~1;     // DISABLE EVENT

}

void tmr_eventresume(HEVENT event)
{
    if(event < 0 || event >= NUM_EVENTS)
        return;
    tmr_events[event].ticks = tmr_ticks() + tmr_events[event].delay;
    tmr_events[event].status |= 1;      // TURN EVENT ON
    tmr_events[event].status &= ~4;     // CLEAR THE DISABLE BIT
    __tmr_eventreschedule();
}

void tmr_eventkill(HEVENT event)
{
    if(event < 0 || event >= NUM_EVENTS)
        return;
    tmr_events[event].status = 0;       // KILL EVENT, TIMER WILL STOP AUTOMATICALLY

}
