/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

INTERRUPT_TYPE cpu_state;
INTERRUPT_TYPE saveint;
volatile unsigned int cpu_idle;

enum
{
    CPU_INTMASKED = 1,

};

void cpu_intoff()
{
    // MASK ALL INTERRUPTS
    if(!saveint)
        saveint = cpu_state;
    cpu_state |= CPU_INTMASKED;
}

INTERRUPT_TYPE cpu_intoff_nosave()
{
    //ARM ints off
    INTERRUPT_TYPE previous = cpu_state;
    cpu_state |= CPU_INTMASKED;
    return previous;
}

void cpu_inton()
{
    if(saveint)
        cpu_state = saveint;
}

// LOW-LEVEL VERSION USED BY THE EXCEPTION HANDLERS
// RESTORES A PREVIOUSLY SAVED INTERRUPT STATE
void cpu_inton_nosave(INTERRUPT_TYPE state)
{
    cpu_state = state;
}

void tmr_fix();

int cpu_getspeed()
{
    return 75000000;    // DUMMY VALUE ON A PC
}

// DUMMY
int cpu_setspeed(int mhz)
{
    UNUSED(mhz);
    return 75000000;
}

void thread_yield();

// PUT THE CPU IN "DOZE" MODE
void cpu_wait_for_interrupt()
{
// TODO: IMPLEMENT THIS ONE IN QT
// BLOCK THREAD UNTIL AN INTERRUPT HAS OCCURRED

// ON THE PC, JUST YIELD FOR 1 MSECOND

// BLOCK SO OTHER THREAD CAN DO WORK ON RPL
    while(cpu_idle == 2)
        thread_yield();

    cpu_idle = 1;
    thread_yield();
    if(cpu_idle == 1)
        cpu_idle = 0;
}

// ACQUIRE A LOCK AND RETURN PREVIOUS VALUE
// IF PREVIOUS VALUE IS ZERO, LOCK WAS ACQUIRED SUCCESSFULLY
// IF NON-ZERO, LOCKING FAILED (RESOURCE WAS ALREADY LOCKED)
int __attribute__((noinline)) cpu_get_lock(int lockvar, volatile int *lock_ptr)
{
    int tmp = *lock_ptr;
    if(!tmp)
        *lock_ptr = lockvar;
    return tmp;
// TODO: IMPLEMENT THIS ONE IN QT
}

void cpu_flushwritebuffers(void)
{
// DUMMY
}

void cpu_flushTLB(void)
{
// DUMMY
}

void cpu_off()
{
    // DUMMY
}
