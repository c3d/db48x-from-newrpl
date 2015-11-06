/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LOW-LEVEL FLASH DRIVER FOR SST 36VF1601 CHIPSET


// ENTER QUERY MODE AND COPY INFORMATION
void flash_CFIQuery(unsigned short *ptr)
{
    asm volatile ("push {r0,r1,r2,r3,r4,r5}");

    // DISABLE INTERRUPTS
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");



    asm volatile ("mov r4,r0");         // SAVE THE POINTER
    asm volatile ("mov r0,#0xAA");
    asm volatile ("orr r0,r0,#0xAA00"); // ADDRESS 0x5555h <<1
    asm volatile ("add r1,r0,r0");
    asm volatile ("bic r1,r1,#0x10000"); // ADDRESS 0x2AAAh <<1
    asm volatile ("mov r3,#0x55");
    asm volatile ("mov r2,#0x98");

    // SEND THE COMMANDS

    asm volatile ("strh r0,[r0]"); //0x5555h=0xaa
    asm volatile ("strh r3,[r1]"); //0x2aaah=0x55
    asm volatile ("strh r2,[r0]"); //0x5555h=0x98

    // WAIT FOR 125nsec, ABOUT 750 CYCLES AT 6 MHz
    // LETS DO SOME MORE JUST IN CASE

    asm volatile ("mov r2,#0x100");
    asm volatile ("waitloop1:");
    asm volatile ("subs r2,r2,#1");
    asm volatile ("mov r2,r2");
    asm volatile ("bne waitloop1");



    asm volatile ("mov r2,#0x20");
    asm volatile ("loop:");
    asm volatile ("ldrh r5,[r2],#2");
    asm volatile ("strh r5,[r4],#2");
    asm volatile ("cmp r2,#0x34");
    asm volatile ("bne loop");

    asm volatile ("mov r2,#0xf0");

    // SEND THE COMMANDS

    asm volatile ("strh r0,[r0]"); //0x5555h=0xaa
    asm volatile ("strh r3,[r1]"); //0x2aaah=0x55
    asm volatile ("strh r2,[r0]"); //0x5555h=0xf0

    // WAIT FOR 125nsec, ABOUT 750 CYCLES AT 6 MHz
    // LETS DO SOME MORE JUST IN CASE

    asm volatile ("mov r2,#0x100");
    asm volatile ("waitloop2:");
    asm volatile ("subs r2,r2,#1");
    asm volatile ("mov r2,r2");
    asm volatile ("bne waitloop2");


    // RE-ENABLE INTERRUPTS
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");

    // AND CLEANUP
    asm volatile ("pop {r0,r1,r2,r3,r4,r5}");

}


unsigned int __attribute__((section (".text"))) flash_code_end[]={ 0 };




// COPIES CFI INFORMATION FROM ADDRESS 10H TO 34H
// INTO BUFFER AT ptr

void flash_CFIRead(unsigned short *ptr)
{
    unsigned int buffer[400],*copy;
    void (*funcquery)(unsigned short *);
    unsigned short *read=0;
    unsigned short *end=(unsigned short *)0x6a;
    int k;

    copy=(unsigned int *)&flash_CFIQuery;

    funcquery=(void (*)(unsigned short *))buffer;

    // COPY ROUTINES TO RAM
    for(k=0;k<flash_code_end-(unsigned int *)&flash_CFIQuery;++k) buffer[k]=copy[k];

    // DISABLE INTERRUPTS
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");

    // ENSURE THE COPIED ROUTINE WILL BE EXECUTED
    cpu_flushwritebuffers();

    funcquery(ptr);

    cpu_flushwritebuffers();


    // INTERRUPTS ARE RE-ENABLED BEFORE


}


