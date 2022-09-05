/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

// IRQ AND CPU LOW LEVEL
unsigned int irq_table[40] __SYSTEM_GLOBAL__;


void __irq_dummy(void)
{
	return;
}

void __irq_service() __attribute__ ((naked));
void __irq_service()
{
	asm volatile ("stmfd sp!, {r0-r12,lr}");
    asm volatile ("mov r0,sp");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0x1f");
    asm volatile ("msr cpsr_all,r1");   // SWITCH TO SYSTEM MODE
    asm volatile ("stmfd r0!,{sp,lr}"); // SAVE REGISTERS THAT WERE BANKED
    asm volatile ("stmfd sp!,{ r0 }");  // SAVE IRQ STACK PTR
    *SRCPND1=*INTPND1; // CLEAR SRCPENDING EARLY TO AVOID MISSING ANY OTHER INTERRUPTS
    *SRCPND2=*INTPND2;
    if(*INTPND1) {
        (*( (__interrupt__) (irq_table[*INTOFFSET1]))) ();
        *INTPND1=*INTPND1;
    }
    else if(*INTPND2) {
        (*( (__interrupt__) (irq_table[32+ *INTOFFSET2]))) ();
        *INTPND2=*INTPND2;
    }
	
    asm volatile ("ldmia sp!, { r0 }"); // GET IRQ STACK PTR
    asm volatile ("ldmia r0!, { sp, lr }"); // RESTORE USER STACK AND LR
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xd");
    asm volatile ("msr cpsr_all,r1");   // SWITCH BACK TO IRQ MODE
    asm volatile ("ldmia sp!, {r0-r12,lr}");    // RESTORE ALL OTHER REGISTERS BACK
	asm volatile ("subs pc,lr,#4");
}


void __irq_install()
{
int f;

// MASK ALL INTERRUPTS, CLEAR ALL PENDING REQUESTS
*INTMSK1=0xffffffff;
*INTMSK2=0xffffffff;

*SRCPND1=*SRCPND1;
*INTPND1=*INTPND1;
*SRCPND2=*SRCPND2;
*INTPND2=*INTPND2;

*INTMOD1=0;
*INTMOD2=0;

// SET ALL IRQ SERVICES TO DUMMY
for(f=0;f<40;++f)
{
    irq_table[f]=(unsigned int)&__irq_dummy;
}
	
// HOOK INTERRUPT SERVICE ROUTINE
// ORIGINAL SAVED W/EXCEPTION HANDLERS
*( (unsigned int *) 0x31ffff18)=(unsigned int)&__irq_service;
	
}

void __irq_addhook(int service_number,__interrupt__ serv_routine)
{
    if(service_number<0 || service_number>39) return;
    irq_table[service_number]=(unsigned int)serv_routine;

}

void __irq_releasehook(int service_number)
{
    if(service_number<0 || service_number>39) return;
    irq_table[service_number]=(unsigned int)&__irq_dummy;
}

void __irq_mask(int service_number)
{
    if(service_number>31) *INTMSK2|=1<<(service_number-32);
    else *INTMSK1|=1<<service_number;
}

void __irq_unmask(int service_number)
{
    if(service_number>31) *INTMSK2&=~(1<<(service_number-32));
    else *INTMSK1&=~(1<<service_number);
}

void __irq_clrpending(int service_number)
{
    if(service_number>31) {
        *SRCPND2=(1<<(service_number-32));
        *INTPND2=(1<<(service_number-32));
    }
    else {
        *SRCPND1=(1<<service_number);
        *INTPND1=(1<<service_number);
    }
}



void usb_mutex_lock(void)
{
    // FIXME disable irq
}

void usb_mutex_unlock(void)
{
    // FIXME enable irq
}
