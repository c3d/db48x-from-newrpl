/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

// IRQ AND CPU LOW LEVEL
unsigned int irq_table[32] SYSTEM_GLOBAL;

ARM_MODE void irq_dummy(void)
{
    return;
}

void irq_service() __attribute__((naked));
ARM_MODE void irq_service()
{
    asm volatile ("stmfd sp!, {r0-r12,lr}");
    asm volatile ("mov r0,sp");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0x1f");
    asm volatile ("msr cpsr_all,r1");   // SWITCH TO SYSTEM MODE
    asm volatile ("stmfd r0!,{sp,lr}"); // SAVE REGISTERS THAT WERE BANKED
    asm volatile ("stmfd sp!,{ r0 }");  // SAVE IRQ STACK PTR
    *HWREG(INT_REGS, 0x0) = *HWREG(INT_REGS, 0x10);     // CLEAR SRCPENDING EARLY TO AVOID MISSING ANY OTHER INTERRUPTS
    (*((tmr_event_fn) (irq_table[*HWREG(INT_REGS, 0x14)]))) ();
    // CLEAR INTERRUPT PENDING FLAG
    register unsigned int a = 1 << (*HWREG(INT_REGS, 0x14));
    *HWREG(INT_REGS, 0x10) = a;

    asm volatile ("ldmia sp!, { r0 }"); // GET IRQ STACK PTR
    asm volatile ("ldmia r0!, { sp, lr }");     // RESTORE USER STACK AND LR
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xd");
    asm volatile ("msr cpsr_all,r1");   // SWITCH BACK TO IRQ MODE
    asm volatile ("ldmia sp!, {r0-r12,lr}");    // RESTORE ALL OTHER REGISTERS BACK
    asm volatile ("subs pc,lr,#4");
}

void irq_install()
{
    int f;

// MASK ALL INTERRUPTS, CLEAR ALL PENDING REQUESTS
    *HWREG(INT_REGS, 0x8) = 0xffffffff;
    *HWREG(INT_REGS, 0) = 0xffffffff;
    *HWREG(INT_REGS, 0x10) = 0xffffffff;

// SET ALL IRQ SERVICES TO DUMMY
    for(f = 0; f < 32; ++f) {
        irq_table[f] = (unsigned int)&irq_dummy;
    }

// HOOK INTERRUPT SERVICE ROUTINE
// ORIGINAL SAVED W/EXCEPTION HANDLERS
    *((unsigned int *)0x08000018) = (unsigned int)&irq_service;

}

void irq_add_hook(int service_number, tmr_event_fn serv_routine)
{
    if(service_number < 0 || service_number > 31)
        return;
    irq_table[service_number] = (uintptr_t) serv_routine;

}

void irq_releasehook(int service_number)
{
    if(service_number < 0 || service_number > 31)
        return;
    irq_table[service_number] = (unsigned int)&irq_dummy;
}

void irq_mask(int service_number)
{
    *HWREG(INT_REGS, 0x8) |= 1 << service_number;
}

void irq_unmask(int service_number)
{
    *HWREG(INT_REGS, 0x8) &= ~(1 << service_number);
}

void usb_mutex_lock(void)
{
    // FIXME disable irq
}

void usb_mutex_unlock(void)
{
    // FIXME enable irq
}
