/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

// IMPORT A FEW FUNCTIONS FROM THE CPU MODULE
int cpu_getPCLK();

void tmr_event_reschedule();

// TIMERS
volatile long long systmr SYSTEM_GLOBAL;
volatile long long evtmr SYSTEM_GLOBAL;
unsigned int sysfreq SYSTEM_GLOBAL;
volatile int tmr_lock SYSTEM_GLOBAL;
timed_event tmr_events[NUM_EVENTS] SYSTEM_GLOBAL;

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
void tmr_newirqeventsvc()
{
    if(cpu_get_lock(2, &tmr_lock))
        return; // GET A LOCK ON THE TIMERS, ABORT IF SOMEBODY ELSE HAS IT
    tmr_lock = 1;

    // GET THE CURRENT SYSTEM TICKS
    tmr_t current_ticks = tmr_ticks();
    int f;

    if(evtmr && (current_ticks >= evtmr)) {
        // PROCESS THE NEXT EVENT
        for(f = 0; f < NUM_EVENTS; ++f) {
            if(tmr_events[f].status & 1) {
                if(tmr_events[f].ticks <= evtmr) {
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

    tmr_lock = 0;

    tmr_event_reschedule();

}

void tmr_irqservice()
{
    systmr += 0x10000;

    if(evtmr && (evtmr - systmr < 0x10000)
            && !(*TCON & 0x100))
        tmr_newirqeventsvc();

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
    tmr_lock = 0;
    systmr = 0;

    unsigned int pclk = cpu_getPCLK();

    unsigned int divider, prescaler;

    prescaler = (pclk << 3) / SYSTIMER_FREQ;
    divider = 1;

    while(prescaler > (1 << (11 + divider))) {
        divider++;
    }

    prescaler += (1 << (2 + divider));
    prescaler >>= divider + 3;

//if(divider>4) PCLK TOO HIGH TO ACHIEVE TIMER FREQUENCY, USE HIGHER MULTIPLE
    if(divider > 4)
        divider = 4;

// CALCULATE SYSTEM CLOCK FREQUENCY
    sysfreq =
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

    evtmr = 0;

    irq_add_hook(10, (tmr_event_fn) & tmr_irqservice);
    irq_add_hook(11, (tmr_event_fn) & tmr_newirqeventsvc);

// UNMASK INTERRUPTS FOR TIMERS 0 AND 1
    *INTMSK1 &= ~0xc00;

// SET MANUAL UPDATE BIT
    *TCON |= 2;
// START TIMER0
    *TCON = (*TCON & (~0x1f)) | 0x9;

}

// FIX TIMERS SPEED WHEN CPU CLOCK IS CHANGED, CALLED FROM cpu_setspeed
void tmr_fix()
{
// MASK ALL TIMER INTERRUPTS
//*HWREG(INT_REGS,0x8)|=0x7c00;

// STOP ALL RUNNING TIMERS
//*HWREG(TMR_REGS,8)&=0xFF8000e0;

// CLEAN ALL PENDING INTERRUPTS
//*HWREG(INT_REGS,0x10)=0x7c00;
//*HWREG(INT_REGS,0)=0x7c00;

// START TIMER0 AS 64-BIT RUNNING TIMER AT 100 KHz

    unsigned int pclk = cpu_getPCLK();

    unsigned int divider, prescaler;

    prescaler = (pclk << 3) / SYSTIMER_FREQ;
    divider = 1;

    while(prescaler > (1 << (11 + divider))) {
        divider++;
    }

    prescaler += (1 << (2 + divider));
    prescaler >>= divider + 3;

//if(divider>4) PCLK TOO HIGH TO ACHIEVE TIMER FREQUENCY, USE HIGHER MULTIPLE
    if(divider > 4)
        divider = 4;

// CALCULATE SYSTEM CLOCK FREQUENCY
    sysfreq =
            (((pclk << 3) / prescaler) + (1 << (divider + 2))) >> (divider + 3);

// SET PRESCALER VALUES FOR TIMERS 0 AND 1
    *TCFG0 = (*TCFG0 & (~0xFF)) | (prescaler - 1);
    *TCFG1 =
            (*TCFG1 & (~0xf000ff)) | (divider - 1) | ((divider - 1) << 4);


}

tmr_t tmr_getsysfreq()
{
    return sysfreq;
}

tmr_t tmr_ticks()
{
    unsigned int *timer0 = (unsigned int *)TCNTO0;

    unsigned int before;
    unsigned long long ticks1, ticks2;

    do {
        ticks1 = systmr;
        before = *timer0;
        ticks2 = systmr;
    }
    while(ticks1 != ticks2);

    return ticks1 + 0x10000 - before;
}

uintptr_t recorder_tick()
{
    return tmr_ticks();
}

// RETURN DELAY IN MILLISECONDS
int tmr_ticks2ms(tmr_t before, tmr_t after)
{
    return ((after - before) * 1000) / sysfreq;
}

// RETURN DELAY IN MICROSECONDS
int tmr_ticks2us(tmr_t before, tmr_t after)
{
    return ((after - before) * 1000000) / sysfreq;
}

// ADD/SUBTRACT AN INTERVAL IN MILLISECONDS TO THE GIVEN TIME IN TICKS
tmr_t tmr_addms(tmr_t time, int ms)
{
    return time + ((ms * sysfreq) / 1000);
}

// ADD/SUBTRACT AN INTERVAL IN MICROSECONDS TO THE GIVEN TIME IN TICKS
tmr_t tmr_addus(tmr_t time, int us)
{
    return time + ((us * sysfreq) / 1000000);
}

void tmr_delayms(int milliseconds)
{
    if(milliseconds <= 0)
        return;
    tmr_t start = tmr_ticks();

// CALCULATE ENDING TICKS
    tmr_t end = start + ((milliseconds * sysfreq) / 1000);

// AND WAIT
    while(end > tmr_ticks());

}

void tmr_delayus(int microseconds)
{
    if(microseconds <= 0)
        return;

    tmr_t start = tmr_ticks();

// CALCULATE ENDING TICKS
    tmr_t end = start + ((microseconds * tmr_getsysfreq()) / 1000000);

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
HEVENT tmr_eventcreate(tmr_event_fn handler, unsigned int ms, int autorepeat)
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

            tmr_event_reschedule();
            return f;
        }
    }
    return -1;
}

void tmr_event_reschedule()
{
    if(cpu_get_lock(2, &tmr_lock))
        return; // GET A LOCK ON THE TIMERS, ABORT IF SOMEBODY ELSE HAS IT

    tmr_t current_ticks;
    tmr_t next;
    int f;

    *TCON = (*TCON & (~0xf00)); // STOP TIMER1

    tmr_lock = 1;

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
    evtmr = next;

    if(next) {

        current_ticks = tmr_ticks();
        if(next - current_ticks >= 0x10000) {
            // NO NEED TO DO ANYTHING
        }
        else {
            // SET THE TIMER1 TO WAKE US UP EXACTLY AT THE RIGHT TIME
            if(next <= current_ticks) {
                *TCNTB1 = 50;    // THIS EVENT IS LATE! RUN IN HALF A MILLISECOND TO CATCH UP
                evtmr = current_ticks + 50;
            }
            else
                *TCNTB1 = next - current_ticks;
            *TCON = (*TCON & (~0xf00)) | 0x200; // MANUAL UPDATE
            *TCON = (*TCON & (~0xf00)) | 0x100; // RESTART TIMER, SINGLE SHOT

        }
    }

    while(tmr_lock == 2) {
        // SOMEBODY TRIED TO ACQUIRE THE LOCK WHILE WE WERE USING IT
        // MUST REDO THE TIMING IN CASE THEY ADDED/REMOVED EVENTS
        tmr_lock = 1;
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

        if(evtmr != next)
            goto restart;
    }

    tmr_lock = 0;

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
    tmr_event_reschedule();
}

void tmr_eventkill(HEVENT event)
{
    if(event < 0 || event >= NUM_EVENTS)
        return;
    tmr_events[event].status = 0;       // KILL EVENT, TIMER WILL STOP AUTOMATICALLY

}


// INDEPENDENT TIMING FUNCTIONS THAT DON'T DEPEND ON INTERRUPTS OR THE TIMER MODULE TO BE INITIALIZED TO WORK
// USED MAINLY FOR HARDWARE SETUP THAT NEEDS ACCURATE TIMING

// Use RTC tick counter for delays in LCD chip communications
#define LLTIMER_FREQ 32768        // 32.768 kHz tick

// Do a single delay 100 usec
void tmr_delay100us()
{
    unsigned int start,end;
        start = *TICKCNT;
        end = *TICKCNT + (LLTIMER_FREQ * 100) / 1000000;

        if(end<start) while(*TICKCNT>=start);         // Wait for the counter to wrap

        while(*TICKCNT<end);                          // And wait for the timer count to reach the end
}

// Do a single delay 10 msec
void tmr_delay10ms()
{
    unsigned int start,end;
        start = *TICKCNT;
        end = *TICKCNT + (LLTIMER_FREQ * 10) / 1000;

        if(end<start) while(*TICKCNT>=start);         // Wait for the counter to wrap

        while(*TICKCNT<end);                          // And wait for the timer count to reach the end
}

// Do a single delay 100 usec
void tmr_delay20ms()
{
    unsigned int start,end;
        start = *TICKCNT;
        end = *TICKCNT + (LLTIMER_FREQ * 20) / 1000;

        if(end<start) while(*TICKCNT>=start);         // Wait for the counter to wrap

        while(*TICKCNT<end);                          // And wait for the timer count to reach the end
}

// Prepare for an open loop timeout
void tmr_setuptimeoutms(int delayms,unsigned int *start,unsigned int *end)
{
    *start = *TICKCNT;
    *end = *TICKCNT + (LLTIMER_FREQ * delayms) / 1000;
}

// Check if clock timed out or not
int tmr_timedout(unsigned int start,unsigned int end)
{
    if(end<start) if(*TICKCNT>=start) return 0;         // Wait for the counter to wrap

    if(*TICKCNT>=end) return 1;                         // And wait for the timer count to reach the end
    return 0;
}
