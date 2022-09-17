/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>

// PC IMPLEMENTATION
#define DSTK_SIZE 32768 //  DATA STACK MAXIMUM
#define RSTK_SIZE 32768 //  RETURN STACK
#define DIR_SIZE  32768 //  DIRECTORIES
#define LAM_SIZE  32768 //  LAMS
#define TEMPOB_SIZE 2097152      //   TEMPOB
#define TEMPBLK_SIZE  32768     //  TEMP BLOCKS

// ALL MEMORY IS ALLOCATED STATICALLY FOR THE SIMULATOR
word_p dstk_memory[DSTK_SIZE];
word_p rstk_memory[RSTK_SIZE];
WORD dir_memory[DIR_SIZE];
WORD lam_memory[LAM_SIZE];
WORD tempob_memory[TEMPOB_SIZE];
word_p tempblk_memory[TEMPBLK_SIZE];
int32_t dstk_used, rstk_used, dir_used, lam_used, tempob_used,
        tempblk_used;
int32_t memmap_intact = 0;

int halGetFreePages()
{
    return ((DSTK_SIZE >> 10) - dstk_used
            + (RSTK_SIZE >> 10) - rstk_used
            + (DIR_SIZE >> 10) - dir_used
            + (LAM_SIZE >> 10) - lam_used
            + (TEMPOB_SIZE >> 10) - tempob_used
            + (TEMPBLK_SIZE >> 10) - tempblk_used);
}

int halGetTotalPages()
{
    return ((DSTK_SIZE >> 10)
            + (RSTK_SIZE >> 10)
            + (DIR_SIZE >> 10)
            + (LAM_SIZE >> 10)
            + (TEMPOB_SIZE >> 10)
            + (TEMPBLK_SIZE >> 10));
}

// GROW AREA OF MEMORY AT base, TO AT LEAST newsize WORDS
// RETURN SAME base OR A NEW LOCATION OF base
// RETURN NULL IF NOT ENOUGH MEMORY

word_p *halGrowMemory(int32_t zone, word_p * base, int32_t newsize)
{
    int maxpages;
    int32_t *current;
    switch (zone) {

    case MEM_AREA_RSTK:
        base = (word_p *) rstk_memory;
        current = &rstk_used;
        maxpages = RSTK_SIZE >> 10;
        break;
    case MEM_AREA_DSTK:
        base = (word_p *) dstk_memory;
        current = &dstk_used;
        maxpages = DSTK_SIZE >> 10;
        break;
    case MEM_AREA_DIR:
        base = (word_p *) dir_memory;
        current = &dir_used;
        maxpages = DIR_SIZE >> 10;
        break;
    case MEM_AREA_LAM:
        base = (word_p *) lam_memory;
        current = &lam_used;
        maxpages = LAM_SIZE >> 10;
        break;
    case MEM_AREA_TEMPOB:
        base = (word_p *) tempob_memory;
        current = &tempob_used;
        maxpages = TEMPOB_SIZE >> 10;
        break;
    case MEM_AREA_TEMPBLOCKS:
        base = (word_p *) tempblk_memory;
        current = &tempblk_used;
        maxpages = TEMPBLK_SIZE >> 10;
        break;
    default:
        return 0;
    }

    newsize += 1023;
    newsize >>= 10;     // REQUIRED NUMBER OF PAGES

    // CAN'T EXCEED MAXIMUM AREA SIZE
    if(newsize > maxpages)
        return 0;

    if(newsize > *current) {
        // FIND FREE PAGES AND ADD THEM
        int needed = newsize - *current;
        int free = halGetFreePages();

        if(free < needed)
            return 0;

        *current += needed;

        return base;
    }

    if(newsize < *current) {
        // RELEASE PAGES TO THE SYSTEM
        int release = *current - newsize;

        *current -= release;

        return base;

    }

    // newsize==current, NOTHING TO DO
    return base;
}

int last_used_byte;

// INITIALIZE THE MEMORY AFTER A TOTAL RESET
void halInitMemoryMap()
{
    dstk_used = rstk_used = dir_used = lam_used = tempob_used =
            tempblk_used = 0;
// MAKE DSTK MEMORY DIRTY WITH KNOWN STATE
    memsetw(dstk_memory, 0xbaadf00d,
            DSTK_SIZE * sizeof(word_p) / sizeof(WORD));
    memsetw(tempob_memory, 0xbaadf00d, TEMPOB_SIZE);

}

// RETURN TRUE IF MEMORY MAPS ARE INTACT, ZERO IF THEY ARE BAD OR INEXISTENT
int halCheckMemoryMap()
{
    return memmap_intact;
}

// RETURN THE NUMBER OF ALLOCATED PAGES FOR A SPECIFIC MEMORY AREA
int halCountUsedPages(int zone)
{
    switch (zone) {

    case MEM_AREA_RSTK:
        return rstk_used;
    case MEM_AREA_DSTK:
        return dstk_used;
    case MEM_AREA_DIR:
        return dir_used;
    case MEM_AREA_LAM:
        return lam_used;
    case MEM_AREA_TEMPOB:
        return tempob_used;
    case MEM_AREA_TEMPBLOCKS:
        return tempblk_used;
    }
    return 0;

}

// CHECK THE MEMORY MAP AGAINST RPL SYSTEM VARIABLES
// CHECK CONSISTENCY OF RPL CORE MEMORY
int halCheckRplMemory()
{
    // VERIFY MAIN RPL POINTERS
    if(TempObEnd >= TempObSize)
        return 0;
    if((int)(TempObSize - TempOb) != halCountUsedPages(MEM_AREA_TEMPOB) << 10)
        return 0;

    if(TempBlocksSize != halCountUsedPages(MEM_AREA_TEMPBLOCKS) << 10)
        return 0;
    if(TempBlocksEnd >= TempBlocks + TempBlocksSize)
        return 0;

    if(RStkSize != halCountUsedPages(MEM_AREA_RSTK) << 10)
        return 0;
    if(RSTop >= RStk + RStkSize)
        return 0;

    if(DStkSize != halCountUsedPages(MEM_AREA_DSTK) << 10)
        return 0;
    if(DSTop >= DStk + DStkSize)
        return 0;

    if(LAMSize != halCountUsedPages(MEM_AREA_LAM) << 10)
        return 0;
    if(LAMTop >= LAMs + LAMSize)
        return 0;

    if(DirSize != halCountUsedPages(MEM_AREA_DIR) << 10)
        return 0;
    if(DirsTop >= Directories + DirSize)
        return 0;

    // ALL MEMORY POINTERS SEEM TO BE VALID
    return 1;
}
