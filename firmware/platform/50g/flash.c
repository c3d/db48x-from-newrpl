/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */
#include <ui.h>
// LOW-LEVEL FLASH DRIVER FOR SST 36VF1601 CHIPSET

// ENTER QUERY MODE AND COPY INFORMATION
ARM_MODE void flash_CFIQuery(unsigned short *ptr)
{
    asm volatile ("push {r0,r1,r2,r3,r4,r5}");

    // DISABLE INTERRUPTS
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");

    asm volatile ("mov r4,r0"); // SAVE THE POINTER
    asm volatile ("mov r0,#0xAA");
    asm volatile ("orr r0,r0,#0xAA00"); // ADDRESS 0x5555h <<1
    asm volatile ("add r1,r0,r0");
    asm volatile ("bic r1,r1,#0x10000");        // ADDRESS 0x2AAAh <<1
    asm volatile ("mov r3,#0x55");
    asm volatile ("mov r2,#0x98");

    // SEND THE COMMANDS

    asm volatile ("strh r0,[r0]");      //0x5555h=0xaa
    asm volatile ("strh r3,[r1]");      //0x2aaah=0x55
    asm volatile ("strh r2,[r0]");      //0x5555h=0x98

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
    asm volatile ("cmp r2,#0x68");
    asm volatile ("bne loop");

    asm volatile ("mov r2,#0xf0");

    // SEND THE COMMANDS

    asm volatile ("strh r0,[r0]");      //0x5555h=0xaa
    asm volatile ("strh r3,[r1]");      //0x2aaah=0x55
    asm volatile ("strh r2,[r0]");      //0x5555h=0xf0

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

// ENTER QUERY MODE AND COPY INFORMATION
ARM_MODE void flash_ProgramWord(unsigned short *ptr, unsigned int data)
{
    asm volatile ("push {r0,r1,r2,r3,r4,r5}");

    // DISABLE INTERRUPTS
    asm volatile ("mrs r2,cpsr_all");
    asm volatile ("orr r2,r2,#0xc0");
    asm volatile ("msr cpsr_all,r2");

    asm volatile ("mov r5,r1"); // SAVE THE DATA
    asm volatile ("mov r4,r0"); // SAVE THE POINTER
    asm volatile ("mov r0,#0xAA");
    asm volatile ("orr r0,r0,#0xAA00"); // ADDRESS 0x5555h <<1
    asm volatile ("add r1,r0,r0");
    asm volatile ("bic r1,r1,#0x10000");        // ADDRESS 0x2AAAh <<1
    asm volatile ("mov r3,#0x55");
    asm volatile ("mov r2,#0xA0");

    // SEND THE COMMANDS

    asm volatile ("strh r0,[r0]");      //0x5555h=0xaa
    asm volatile ("strh r3,[r1]");      //0x2aaah=0x55
    asm volatile ("strh r2,[r0]");      //0x5555h=0xA0
    asm volatile ("strh r5,[r4]");      // PROGRAM THE WORD

    // USE TOGGLE BIT STRATEGY TO DETECT END OF PROGRAMMING

    asm volatile ("waitloop3:");
    asm volatile ("ldrh r0,[r4]");
    asm volatile ("ldrh r1,[r4]");
    asm volatile ("cmp r0,r1"); // IF WORD IS THE SAME, THERE'S NO MORE BIT TOGGLING
    asm volatile ("bne waitloop3");

    // RE-ENABLE INTERRUPTS
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");

    // AND CLEANUP
    asm volatile ("pop {r0,r1,r2,r3,r4,r5}");

}

unsigned int __attribute__((section(".text"))) flash_code_end[] = { 0 };

// COPIES CFI INFORMATION FROM ADDRESS 10H TO 34H
// INTO BUFFER AT ptr

ARM_MODE void flash_CFIRead(unsigned short *ptr)
{

    flash_prepareforwriting();

    unsigned int buffer[400], *copy;
    void (*funcquery)(unsigned short *);
    unsigned short *read = 0;
    unsigned short *end = (unsigned short *)0x6a;
    int k;

    copy = (unsigned int *)&flash_CFIQuery;

    funcquery = (void (*)(unsigned short *))buffer;

    // COPY ROUTINES TO RAM
    for(k = 0; k < flash_code_end - (unsigned int *)&flash_CFIQuery; ++k)
        buffer[k] = copy[k];

    // DISABLE INTERRUPTS
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");

    // ENSURE THE COPIED ROUTINE WILL BE EXECUTED
    cpu_flushwritebuffers();
    // AND MAKE SURE WE DON'T EXECUTE AN OLD COPY LEFT IN THE CACHE
    cpu_flushicache();

    funcquery(ptr);

    cpu_flushwritebuffers();

    // INTERRUPTS ARE RE-ENABLED BEFORE
    flash_donewriting();

}

ARM_MODE void flash_Write(unsigned short *ptr, unsigned int data)
{
    unsigned int buffer[400], *copy;
    void (*funcquery)(unsigned short *, unsigned int);
    unsigned short *read = 0;
    unsigned short *end = (unsigned short *)0x6a;
    int k;

    copy = (unsigned int *)&flash_ProgramWord;

    funcquery = (void (*)(unsigned short *, unsigned int))buffer;

    // COPY ROUTINES TO RAM
    for(k = 0; k < flash_code_end - (unsigned int *)&flash_ProgramWord; ++k)
        buffer[k] = copy[k];

    // DISABLE INTERRUPTS
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");

    // ENSURE THE COPIED ROUTINE WILL BE EXECUTED
    cpu_flushwritebuffers();
    // AND MAKE SURE WE DON'T EXECUTE AN OLD COPY LEFT IN THE CACHE
    cpu_flushicache();

    funcquery(ptr, data);

    cpu_flushwritebuffers();

    // INTERRUPTS ARE RE-ENABLED BEFORE

}

#define MMU_SECTION_RAM(a) (((a)&0xfff00000)| 0xc1e)
#define MMU_SECTION_ROM(a) (((a)&0xfff00000)| 0x01e)
#define MMU_SECTION_DEV(a) (((a)&0xfff00000)| 0xc12)

#define MMU_PAGES_RAM(a) (((a)&0xfffffc00)|0x011)

#define MMU_PAGE(a) (((a)&0xfffff000)|0xffe)

#define MMU_LEVEL1_INDEX(virt) (((virt)>>20)&0xfff)
#define MMU_LEVEL2_INDEX(virt) (((virt)>>12)&0xff)

#define MMU_MAP_SECTION_ROM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_ROM(phys))
#define MMU_MAP_SECTION_RAM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_RAM(phys))
#define MMU_MAP_SECTION_DEV(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_DEV(phys))
#define MMU_MAP_COARSE_RAM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_PAGES_RAM(phys))

#define MMU_MAP_PAGE(phys,virt) ( ( (unsigned int *)(mmu_base[MMU_LEVEL1_INDEX(virt)]&0xfffffc00))[MMU_LEVEL2_INDEX(virt)]=MMU_PAGE(phys))

// SETUP FLASH MEMORY AS UNCACHED, UNBUFFERED SO THE CACHES DON'T INTERFERE WITH PROGRAMMING
ARM_MODE void flash_prepareforwriting()
{
    unsigned int *mmu_base = (unsigned int *)0x08008000;

    //MEM_ROM         0x00000000  // VIRTUAL (AND PHYSICAL) ROM LOCATION (UP TO 4 MBytes)
    MMU_MAP_SECTION_DEV(0x00000000, 0x00000000);        // MAP 1ST MEGABYTE SECTION AS UNCACHED/UNBUFFERED
    MMU_MAP_SECTION_DEV(0x00100000, 0x00100000);        // MAP TOTAL 2 MBYTES OF ROM AS UNCACHED/UNBUFFERED

    // SHOW SOME VISUALS
    unsigned int *scrptr = (unsigned int *)MEM_PHYS_SCREEN;
    *scrptr = 0xf0f0f0f0;       // FLUSH THE TLB TO FORCE THE MMU TO READ THE UPDATED SECTION MARKER
    cpu_flushTLB();
    *scrptr = 0xffff0000;       // FLUSH THE TLB TO FORCE THE MMU TO READ THE UPDATED SECTION MARKER
    // INVALIDATE THE DATA CACHES TO MAKE SURE THERE'S NO CACHE HIT ON THE ROM AREA WE ARE PROGRAMMING
    cpu_flushwritebuffers();
    *scrptr = 0xf00f00f0;       // FLUSH THE TLB TO FORCE THE MMU TO READ THE UPDATED SECTION MARKER
    cpu_flushicache();
    *scrptr = 0xffff8888;       // FLUSH THE TLB TO FORCE THE MMU TO READ THE UPDATED SECTION MARKER
}

// ENABLE CACHING AND BUFFERS AGAIN ON THE ROM
ARM_MODE void flash_donewriting()
{
    unsigned int *mmu_base = (unsigned int *)0x08008000;

    //MEM_ROM         0x00000000  // VIRTUAL (AND PHYSICAL) ROM LOCATION (UP TO 4 MBytes)
    MMU_MAP_SECTION_ROM(0x00000000, 0x00000000);        // MAP 1ST MEGABYTE SECTION AS UNCACHED/UNBUFFERED
    MMU_MAP_SECTION_ROM(0x00100000, 0x00100000);        // MAP TOTAL 2 MBYTES OF ROM AS UNCACHED/UNBUFFERED

    // FLUSH THE TLB TO FORCE THE MMU TO READ THE UPDATED SECTION MARKER
    cpu_flushTLB();

    // FROM NOW ON, THE CAHCE WILL START FILLING UP AGAIN
    unsigned int *scrptr = (unsigned int *)MEM_PHYS_SCREEN;
    *scrptr = 0x44444444;       // FLUSH THE TLB TO FORCE THE MMU TO READ THE UPDATED SECTION MARKER
}
