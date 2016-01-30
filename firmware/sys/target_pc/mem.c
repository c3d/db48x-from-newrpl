/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>

// PC IMPLEMENTATION
#define DSTK_SIZE 32768     // 1MB FOR DATA STACK MAXIMUM
#define RSTK_SIZE 32768     // 1MB FOR RETURN STACK
#define DIR_SIZE  32768     // 1MB FOR DIRECTORIES
#define LAM_SIZE  32768     // 1MB FOR LAMS
#define TEMPOB_SIZE 32768 // 1MB FOR TEMPOB
#define TEMPBLK_SIZE  32768 // 1MB FOR TEMP BLOCKS

// ALL MEMORY IS ALLOCATED STATICALLY FOR THE SIMULATOR
WORDPTR __dstk_memory[DSTK_SIZE];
WORDPTR __rstk_memory[RSTK_SIZE];
WORD __dir_memory[DIR_SIZE];
WORD __lam_memory[LAM_SIZE];
WORD __tempob_memory[TEMPOB_SIZE];
WORDPTR __tempblk_memory[TEMPBLK_SIZE];
BINT __dstk_used,__rstk_used,__dir_used,__lam_used,__tempob_used,__tempblk_used;
BINT __memmap_intact=0;


int halGetFreePages()
{
    return ((DSTK_SIZE>>10)-__dstk_used
           +(RSTK_SIZE>>10)-__rstk_used
            +(DIR_SIZE>>10)-__dir_used
            +(LAM_SIZE>>10)-__lam_used
            +(TEMPOB_SIZE>>10)-__tempob_used
            +(TEMPBLK_SIZE>>10)-__tempblk_used);
}
int halGetTotalPages()
{
    return  ((DSTK_SIZE>>10)
             +(RSTK_SIZE>>10)
              +(DIR_SIZE>>10)
              +(LAM_SIZE>>10)
              +(TEMPOB_SIZE>>10)
              +(TEMPBLK_SIZE>>10));
}


// GROW AREA OF MEMORY AT base, TO AT LEAST newsize WORDS
// RETURN SAME base OR A NEW LOCATION OF base
// RETURN NULL IF NOT ENOUGH MEMORY

WORDPTR *halGrowMemory(BINT zone, WORDPTR *base, BINT newsize)
{
    int maxpages;
    BINT *current;
    switch(zone) {

            case MEM_AREA_RSTK:
                 base=(WORDPTR *)__rstk_memory;
                 current=&__rstk_used;
                 maxpages=RSTK_SIZE>>10;
            break;
            case MEM_AREA_DSTK:
                 base=(WORDPTR *)__dstk_memory;
                 current=&__dstk_used;
                 maxpages=DSTK_SIZE>>10;
            break;
            case MEM_AREA_DIR:
                base=(WORDPTR *)__dir_memory;
                current=&__dir_used;
                maxpages=DIR_SIZE>>10;
            break;
            case MEM_AREA_LAM:
                base=(WORDPTR *)__lam_memory;
                current=&__lam_used;
                maxpages=LAM_SIZE>>10;
            break;
            case MEM_AREA_TEMPOB:
                base=(WORDPTR *)__tempob_memory;
                current=&__tempob_used;
                maxpages=TEMPOB_SIZE>>10;
            break;
            case MEM_AREA_TEMPBLOCKS:
                base=(WORDPTR *)__tempblk_memory;
                current=&__tempblk_used;
                maxpages=TEMPBLK_SIZE>>10;
            break;
            default:
                return 0;
    }

    newsize+=1023;
    newsize>>=10;    // REQUIRED NUMBER OF PAGES

    // CAN'T EXCEED MAXIMUM AREA SIZE
    if(newsize>maxpages) return 0;


    if(newsize>*current) {
        // FIND FREE PAGES AND ADD THEM
        int needed=newsize-*current;
        int free=halGetFreePages();

        if(free<needed) return 0;


        *current+=needed;

        return base;
    }

    if(newsize<*current) {
        // RELEASE PAGES TO THE SYSTEM
        int release=*current-newsize;

        *current-=release;

        return base;

    }

    // newsize==current, NOTHING TO DO
    return base;
}


int __last_used_byte;

// INITIALIZE THE MEMORY AFTER A TOTAL RESET
void halInitMemoryMap()
{
__dstk_used=__rstk_used=__dir_used=__lam_used=__tempob_used=__tempblk_used=0;
}


// RETURN TRUE IF MEMORY MAPS ARE INTACT, ZERO IF THEY ARE BAD OR INEXISTENT
int halCheckMemoryMap()
{
return __memmap_intact;
}


// RETURN THE NUMBER OF ALLOCATED PAGES FOR A SPECIFIC MEMORY AREA
int halCountUsedPages(int zone)
{
    switch(zone) {

            case MEM_AREA_RSTK:
                return __rstk_used;
            case MEM_AREA_DSTK:
                return __dstk_used;
            case MEM_AREA_DIR:
                return __dir_used;
            case MEM_AREA_LAM:
                return __lam_used;
            case MEM_AREA_TEMPOB:
                return __tempob_used;
            case MEM_AREA_TEMPBLOCKS:
                return __tempblk_used;
    }
    return 0;

}


// CHECK THE MEMORY MAP AGAINST RPL SYSTEM VARIABLES
// CHECK CONSISTENCY OF RPL CORE MEMORY

void halCheckRplMemory()
{
    if(TempObEnd>=TempObSize)  throw_dbgexception("TempObEnd>=TempObSize",__EX_CONT);

    if((int)(TempObSize-TempOb)!=halCountUsedPages(MEM_AREA_TEMPOB)<<10) throw_dbgexception("TempObSize!=MMU size",__EX_CONT);
    if(TempBlocksSize!=halCountUsedPages(MEM_AREA_TEMPBLOCKS)<<10) throw_dbgexception("TempBlocksSize!=MMU size",__EX_CONT);
    if(RStkSize!=halCountUsedPages(MEM_AREA_RSTK)<<10) throw_dbgexception("RStkSize!=MMU size",__EX_CONT);
    if(DStkSize!=halCountUsedPages(MEM_AREA_DSTK)<<10) throw_dbgexception("DStkSize!=MMU size",__EX_CONT);
    if(LAMSize!=halCountUsedPages(MEM_AREA_LAM)<<10) throw_dbgexception("LAMSize!=MMU size",__EX_CONT);
    if(DirSize!=halCountUsedPages(MEM_AREA_DIR)<<10) throw_dbgexception("DirSize!=MMU size",__EX_CONT);

}

