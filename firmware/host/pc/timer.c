/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

void stop_singleshot();
void timer_singleshot(int ms);

void tmr_eventreschedule();

// TIMERS
volatile tmr_t systmr SYSTEM_GLOBAL;
volatile tmr_t evtmr SYSTEM_GLOBAL;

volatile unsigned long long pcsystmr; // MOVING COUNTER FOR SYSTEM PC TIMER
long long pctmr1;     // TIMER1 COUNTDOWN
unsigned int sysfreq SYSTEM_GLOBAL;
volatile int tmr_lock SYSTEM_GLOBAL;
timed_event tmr_events[NUM_EVENTS] SYSTEM_GLOBAL;

// SAVE TIMERS CONFIGURATION - 13 WORDS REQUIRED
void tmr_save(unsigned int *tmrbuffer)
{
    UNUSED(tmrbuffer);

}

// RESTORE TIMERS

void tmr_restore(unsigned int *tmrbuffer)
{
    UNUSED(tmrbuffer);

}

// THIS EVENT IS CALLED TO CHECK IF WE HAVE AN EVENT
void tmr_newirqeventsvc()
{
    if(cpu_getlock(2, &tmr_lock))
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

    tmr_eventreschedule();

}

void tmr_irqservice()
{
    systmr += 0x10000;

    if(evtmr && (evtmr - systmr < 0x10000))
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

// STOP ALL RUNNING TIMERS

// CLEAN ALL PENDING INTERRUPTS

// START TIMER0 AS 64-BIT RUNNING TIMER AT 100 KHz
    tmr_lock = 0;
    systmr = 0;

// CALCULATE SYSTEM CLOCK FREQUENCY
    sysfreq = SYSTIMER_FREQ;

// PREPARE TIMED EVENTS HANDLER
    int k;
    for(k = 0; k < NUM_EVENTS; ++k)
        tmr_events[k].status = 0;

    evtmr = 0;

//irq_addhook(10,(__interrupt__) &tmr_irqservice);
//irq_addhook(11,(__interrupt__) &tmr_newirqeventsvc);

// UNMASK INTERRUPTS FOR TIMERS 0 AND 1

// SET MANUAL UPDATE BIT
// START TIMER0

}

// FIX TIMERS SPEED WHEN CPU CLOCK IS CHANGED, CALLED FROM cpu_setspeed
void tmr_fix()
{
    sysfreq = SYSTIMER_FREQ;

}

tmr_t tmr_getsysfreq()
{
    return sysfreq;
}

tmr_t tmr_ticks()
{
// RETURN THE SYSTEM MOVING TIMER
    return pcsystmr;

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

    unsigned long long start = tmr_ticks();

// CALCULATE ENDING TICKS
    unsigned long long end =
            start + ((microseconds * tmr_getsysfreq()) / 1000000);

// AND WAIT
    while(end > (unsigned long long)tmr_ticks());

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

            tmr_eventreschedule();
            return f;
        }
    }
    return -1;
}

void tmr_eventreschedule()
{
    if(cpu_getlock(2, &tmr_lock))
        return; // GET A LOCK ON THE TIMERS, ABORT IF SOMEBODY ELSE HAS IT

    tmr_t current_ticks;
    tmr_t next;
    int f;

    // STOP TIMER1
    stop_singleshot();
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
                pctmr1 = 50;  // THIS EVENT IS LATE! RUN IN HALF A MILLISECOND TO CATCH UP
                evtmr = current_ticks + 50;
            }
            else
                pctmr1 = next - current_ticks;
            // MANUAL UPDATE
            // RESTART TIMER, SINGLE SHOT
            timer_singleshot((int)(pctmr1 * 1000.0 / (double)SYSTIMER_FREQ));

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
    tmr_eventreschedule();
}

void tmr_eventkill(HEVENT event)
{
    if(event < 0 || event >= NUM_EVENTS)
        return;
    tmr_events[event].status = 0;       // KILL EVENT, TIMER WILL STOP AUTOMATICALLY

}
