/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>
#include <newrpl.h>


int halGetFreePages()
{
    int *mmutable=(int *)MEM_REVERSEMMU;
    int count=0;
    while(mmutable<(int *)(MEM_REVERSEMMU+0x200)) { if(*mmutable==0) ++count; ++mmutable; }
    return count;
}



// GROW AREA OF MEMORY AT base, TO AT LEAST newsize WORDS
// RETURN SAME base OR A NEW LOCATION OF base
// RETURN NULL IF NOT ENOUGH MEMORY

WORDPTR *halGrowMemory(BINT zone, WORDPTR *base, BINT newsize)
{
    int *mmubase;
    int maxpages;
    switch(zone) {

            case MEM_AREA_RSTK:
                 mmubase=(int *)MEM_RSTKMMU;
                 base=(WORDPTR *)MEM_RSTK;
                 maxpages=256;
            break;
            case MEM_AREA_DSTK:
                 mmubase=(int *)MEM_DSTKMMU;
                 base=(WORDPTR *)MEM_DSTK;
                 maxpages=256;
            break;
            case MEM_AREA_DIR:
                mmubase=(int *)MEM_DIRMMU;
                base=(WORDPTR *)MEM_DIRS;
                maxpages=256;
            break;
            case MEM_AREA_LAM:
                mmubase=(int *)MEM_LAMMMU;
                base=(WORDPTR *)MEM_LAM;
                maxpages=256;
            break;
            case MEM_AREA_TEMPOB:
                mmubase=(int *)MEM_TEMPOBMMU;
                base=(WORDPTR *)MEM_TEMPOB;
                maxpages=1024;
            break;
            case MEM_AREA_TEMPBLOCKS:
                mmubase=(int *)MEM_TEMPBLKMMU;
                base=(WORDPTR *)MEM_TEMPBLOCKS;
                maxpages=256;
            break;
            default:
                return 0;
    }

    newsize+=1023;
    newsize>>=10;    // REQUIRED NUMBER OF PAGES

    // CAN'T EXCEED MAXIMUM AREA SIZE
    if(newsize>maxpages) return 0;

    int current=0;
    int *ptr=mmubase;

    while( (*ptr!=0) && ( (ptr-mmubase)<maxpages)) { ++current; ++ptr; } // NUMBER OF PAGES CURRENTLY ALLOCATED

    if(newsize>current) {
        // FIND FREE PAGES AND ADD THEM
        int needed=newsize-current;
        int free=halGetFreePages();

        if(free<needed) return 0;

        int *mmutable=(int *)MEM_REVERSEMMU;
        while(needed && (mmutable<(int *)(MEM_REVERSEMMU+0x200))) {
            if(*mmutable==0) {
                // THIS SHOULD BE ATOMIC
                // FIRST MARK PAGE AS USED
                *mmutable=((int)base)+(current<<12);
                // STORE THE NEW PAGE IN THE PROPER MMU TABLE
                mmubase[current]=(RAM_BASE_PHYSICAL + (((int)mmutable-MEM_REVERSEMMU)<<10)) | 0xffe;
                ++current;
                --needed;
            }
            ++mmutable;
        }

        if(newsize!=halCountUsedPages(zone)) throw_dbgexception("Allocation error",__EX_CONT);

        return base;
    }
    if(newsize<current) {
        // RELEASE PAGES TO THE SYSTEM
        int release=current-newsize;

        cpu_flushwritebuffers();

        int *mmutable=(int *)MEM_REVERSEMMU;
        while(release) {
            int index=(mmubase[current-1]-RAM_BASE_PHYSICAL)>>12;

            if( (index<0) || ( (index<<12)>(RAM_END_PHYSICAL-RAM_BASE_PHYSICAL))) {
                throw_dbgexception("Corrupted MMU",__EX_WARM | __EX_RESET);
            }
                // REMOVE PAGE FROM MEMORY AREA
                mmubase[current-1]=0;
                // MARK AS FREE IN REVERSE MMU TABLE
                mmutable[index]=0;
                --current;
                --release;
            }

        cpu_flushTLB();

        if(newsize!=halCountUsedPages(zone)) throw_dbgexception("Release error",__EX_CONT);

        return base;

    }

    // newsize==current, NOTHING TO DO
    return base;
}


extern int __last_used_byte;

// INITIALIZE THE MEMORY AFTER A TOTAL RESET
void halInitMemoryMap()
{
// MARK PAGES USED IN MEMORY MAP
    unsigned int end=(((unsigned int)&__last_used_byte)+0xfff)&~0xfff;
 int *mmutable=(int *)MEM_REVERSEMMU;
 int phys=0x02000000;

 // MARK USED PAGES AS USED
 while(phys<end) {
     *mmutable=phys;
     phys+=0x1000;
     ++mmutable;
 }

 // AND ALL OTHERS AS FREE (1 kbytes RESERVED FOR THE TABLE, ONLY 512 BYTES USED FOR NOW)
 while(mmutable<(int *)(MEM_REVERSEMMU+0x400)) *mmutable++=0;

 // SINCE ALL PAGES ARE FREE, CLEAR ALL MMU TABLES

 memsetw((void *)MEM_RSTKMMU,0,256);
 memsetw((void *)MEM_DSTKMMU,0,256);
 memsetw((void *)MEM_LAMMMU,0,256);
 memsetw((void *)MEM_DIRMMU,0,256);
 memsetw((void *)MEM_TEMPBLKMMU,0,256);
 memsetw((void *)MEM_TEMPOBMMU,0,1024);



}


// RETURN TRUE IF MEMORY MAPS ARE INTACT, ZERO IF THEY ARE BAD OR INEXISTENT
int halCheckMemoryMap()
{
    unsigned int end=(((unsigned int)&__last_used_byte)+0xfff)&~0xfff;

    int *mmutable=(int *)MEM_REVERSEMMU;
    int virt=0x02000000;

    // CHECK INITIAL USED PAGES TO VERIFY IF VALID MMU MAP EXISTS
    while(virt<end) {
        if(mmutable[(virt-0x02000000)>>12]!=virt) {
            throw_dbgexception("bad initial pages",__EX_CONT);
            return 0;   // MMU TABLE WAS CORRUPTED
        }
        virt+=0x1000;
    }

    // CHECK RSTK MEMORY MAP
    int *mmu=(int *)MEM_RSTKMMU;
    int page;

    while( (*mmu!=0) && ((mmu-(int *)MEM_RSTKMMU)<0x100)) {
        page=*mmu;
        if( (page<RAM_BASE_PHYSICAL)|| (page>RAM_END_PHYSICAL)) {
            throw_dbgexception("bad RSTK MMU",__EX_CONT);

            return 0;   // CORRUPTED MMU TABLE!
        }
        page=(page-RAM_BASE_PHYSICAL)>>12;
        if( mmutable[page]!=MEM_RSTK+(((int)(mmu-(int *)MEM_RSTKMMU))<<12)) {
            throw_dbgexception("bad reverse-RSTK MMU",__EX_CONT);
            return 0;
        }
        ++mmu;
    }

    // CHECK DSTK MEMORY MAP
    mmu=(int *)MEM_DSTKMMU;

    while( (*mmu!=0) && ((mmu-(int *)MEM_DSTKMMU)<0x100)) {
        page=*mmu;
        if( (page<RAM_BASE_PHYSICAL)|| (page>RAM_END_PHYSICAL)) {
            throw_dbgexception("bad DSTK MMU",__EX_CONT);

            return 0;   // CORRUPTED MMU TABLE!
        }
        page=(page-RAM_BASE_PHYSICAL)>>12;
        if( (mmutable[page]&~0xfff)!=MEM_DSTK+(((int)(mmu-(int *)MEM_DSTKMMU))<<12)) {
            throw_dbgexception("bad reverse-DSTK MMU",__EX_CONT);
            return 0;
        }
        ++mmu;
    }

    // CHECK DIR MEMORY MAP
    mmu=(int *)MEM_DIRMMU;

    while( (*mmu!=0) && ((mmu-(int *)MEM_DIRMMU)<0x100)) {
        page=*mmu;
        if( (page<RAM_BASE_PHYSICAL)|| (page>RAM_END_PHYSICAL)) {
            throw_dbgexception("bad DIR MMU",__EX_CONT);

            return 0;   // CORRUPTED MMU TABLE!
        }
        page=(page-RAM_BASE_PHYSICAL)>>12;
        if( (mmutable[page]&~0xfff)!=MEM_DIRS+(((int)(mmu-(int *)MEM_DIRMMU))<<12)) {
            throw_dbgexception("bad reverse-DIRS MMU",__EX_CONT);

            return 0;
        }
        ++mmu;
    }

    // CHECK LAM MEMORY MAP
    mmu=(int *)MEM_LAMMMU;

    while( (*mmu!=0) && ((mmu-(int *)MEM_LAMMMU)<0x100)) {
        page=*mmu;
        if( (page<RAM_BASE_PHYSICAL)|| (page>RAM_END_PHYSICAL)) {
            throw_dbgexception("bad LAM MMU",__EX_CONT);

            return 0;   // CORRUPTED MMU TABLE!
        }
        page=(page-RAM_BASE_PHYSICAL)>>12;
        if( (mmutable[page]&~0xfff)!=MEM_LAM+(((int)(mmu-(int *)MEM_LAMMMU))<<12)) {
            throw_dbgexception("bad reverse-LAM MMU",__EX_CONT);

            return 0;
        }
        ++mmu;
    }

    // CHECK TEMPBLOCKS MEMORY MAP
    mmu=(int *)MEM_TEMPBLKMMU;

    while( (*mmu!=0) && ((mmu-(int *)MEM_TEMPBLKMMU)<0x100)) {
        page=*mmu;
        if( (page<RAM_BASE_PHYSICAL)|| (page>RAM_END_PHYSICAL)) {
            throw_dbgexception("bad TEMPBLK MMU",__EX_CONT);

            return 0;   // CORRUPTED MMU TABLE!
        }
        page=(page-RAM_BASE_PHYSICAL)>>12;
        if( (mmutable[page]&~0xfff)!=MEM_TEMPBLOCKS+(((int)(mmu-(int *)MEM_TEMPBLKMMU))<<12)) {
            throw_dbgexception("bad reverse-TEMPBLK MMU",__EX_CONT);

            return 0;
        }
        ++mmu;
    }

    // CHECK TEMPOB MEMORY MAP
    mmu=(int *)MEM_TEMPOBMMU;

    while( (*mmu!=0) && ((mmu-(int *)MEM_TEMPOBMMU)<0x400)) {
        page=*mmu;
        if( (page<RAM_BASE_PHYSICAL)|| (page>RAM_END_PHYSICAL)) {
            throw_dbgexception("bad TEMPOB MMU",__EX_CONT);

            return 0;   // CORRUPTED MMU TABLE!
        }
        page=(page-RAM_BASE_PHYSICAL)>>12;
        if( (mmutable[page]&~0xfff)!=MEM_TEMPOB+(((int)(mmu-(int *)MEM_TEMPOBMMU))<<12)) {
            throw_dbgexception("bad reverse-TEMPOB MMU",__EX_CONT);

            return 0;
        }
        ++mmu;
    }

    // SO FAR, ALL ALLOCATED PAGES HAVE A VALID REVERSE LOOKUP

    // TODO: REVERSE LOOKUP TABLE MIGHT HAVE ORPHAN ENTRIES THAT COULD BE RELEASED?

    return 1;
}


// RETURN THE NUMBER OF ALLOCATED PAGES FOR A SPECIFIC MEMORY AREA
int halCountUsedPages(int zone)
{
    int *mmubase;
    int maxpages;

    switch(zone) {

            case MEM_AREA_RSTK:
                 mmubase=(int *)MEM_RSTKMMU;
                 maxpages=256;
            break;
            case MEM_AREA_DSTK:
                 mmubase=(int *)MEM_DSTKMMU;
                 maxpages=256;
            break;
            case MEM_AREA_DIR:
                mmubase=(int *)MEM_DIRMMU;
                maxpages=256;
            break;
            case MEM_AREA_LAM:
                mmubase=(int *)MEM_LAMMMU;
                maxpages=256;
            break;
            case MEM_AREA_TEMPOB:
                mmubase=(int *)MEM_TEMPOBMMU;
                maxpages=1024;
            break;
            case MEM_AREA_TEMPBLOCKS:
                mmubase=(int *)MEM_TEMPBLKMMU;
                maxpages=256;
            break;
            default:
                return 0;
    }

    int current=0;
    int *ptr=mmubase;

    while( (*ptr!=0) && ( (ptr-mmubase)<maxpages)) { ++current; ++ptr; } // NUMBER OF PAGES CURRENTLY ALLOCATED
    return current;
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

