/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LOW-LEVEL FLASH DRIVER FOR SST 36VF1601 CHIPSET
unsigned int __attribute__((section (".text"))) flash_code_start[]={ 0 };
// SEND THE EXIT SEQUENCE COMMAND
void flash_exit()
{
    asm volatile ("push {r0,r1,r2,r3}");

    asm volatile ("mov r0,#0xAA");
    asm volatile ("orr r0,r0,#0xAA00"); // ADDRESS 0x5555h <<1
    asm volatile ("add r1,r0,r0");
    asm volatile ("bic r1,r1,#0x10000"); // ADDRESS 0x2AAAh <<1
    asm volatile ("mov r3,#0x55");
    asm volatile ("mov r2,#0xf0");

    // SEND THE COMMANDS

    asm volatile ("strh r0,[r0]"); //0x5555h=0xaa
    asm volatile ("strh r3,[r1]"); //0x2aaah=0x55
    asm volatile ("strh r2,[r0]"); //0x5555h=0xf0

    asm volatile ("pop {r0,r1,r2,r3}");
}

// ENTER QUERY MODEflash_
void flash_CFIQuery()
{
    asm volatile ("push {r0,r1,r2,r3}");

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
    asm volatile ("pop {r0,r1,r2,r3}");

}

unsigned int __attribute__((section (".text"))) flash_code_end[]={ 0 };




// COPIES CFI INFORMATION FROM ADDRESS 10H TO 34H
// INTO BUFFER AT ptr

void flash_CFIRead(unsigned short *ptr)
{
    unsigned int buffer[400],*copy;
    void (*funcquery)();
    void (*funcexit)();
    unsigned short *read=0;
    unsigned short *end=(unsigned short *)0x6a;
    int k;

    copy=flash_code_start+1;

    funcexit=(void (*)())buffer;
    funcquery=(void (*)())(buffer-1+((unsigned int *)&flash_CFIQuery-(unsigned int *)&flash_code_start));

    // COPY ROUTINES TO RAM
    for(k=0;k<flash_code_end-flash_code_start;++k) buffer[k]=copy[k];

    // DISABLE INTERRUPTS
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");

    cpu_flushwritebuffers();






    //flash_CFIQuery();
    funcquery();


    for(k=0;k<0x10000;++k) {
        asm volatile ("mov r0,r0");
    }

    for(k=0x10;k<0x34;++k) {
        *ptr=read[k];
        ++ptr;
    }

    //flash_exit();
    funcexit();

    for(k=0;k<0x10000;++k) {
        asm volatile ("mov r0,r0");
    }


    // RE-ENABLE INTERRUPTS
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");

    cpu_flushwritebuffers();


}

void flash_CFIRead_end()
{
    return;
}


