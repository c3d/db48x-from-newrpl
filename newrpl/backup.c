/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// BACKUP TEMPOB AND DIRECTORIES (NO STACK) TO EXTERNAL DEVICE

void rplBackup(void (*writefunc)(unsigned int))
{
    BINT offset;
    BINT k;
    // COMPACT TEMPOB AS MUCH AS POSSIBLE
    rplGCollect();

    // DUMP SYSTEM VARIABLES TO THE FILE


    // GENERIC SECTIONS
    struct {
        WORDPTR start;
        BINT nitems;
        BINT offwords;
    } sections[10];

    // FILL SECTIONS

    offset=1024;        // SAVE 1024 WORD ON THE FILE TO STORE SYSTEM VARIABLES
                        // AND SECTION INFORMATION


    // TEMPBLOCKS
    sections[k].start= (WORDPTR)TempBlocks;
    sections[k].nitems= TempBlocksEnd-TempBlocks;
    sections[k].offwords=offset;

    offset+=sections[k].nitems;
    ++k;

    // TEMPOB
    sections[k].start=(WORDPTR)TempOb;
    sections[k].nitems=TempObSize-TempOb;        // INCLUDE SPACE AFTER TEMPOBEND IN CASE THE BACKUP IS DONE DURING COMPILE/DECOMPILE
    sections[k].offwords=offset;

    offset+=sections[k].nitems;
    ++k;

    // DIRECTORIES
    sections[k].start=(WORDPTR)Directories;
    sections[k].nitems=DirsTop-Directories;
    sections[k].offwords=offset;

    offset+=sections[k].nitems;
    ++k;


    // FILL ALL OTHER SECTIONS
    for(;k<10;++k) {
        sections[k].start=0;
        sections[k].nitems=0;
        sections[k].offwords=offset;
    }


    BINT writeoff=0;

    // FIRST, WRITE SIGNATURE TO THE FILE
    writefunc( ((int)'N') | ((int)'R'<<8) | ((int)'P'<<16) | ((int)'L'<<24));
    writeoff++;
    // WRITE ALL 10 SECTIONS START OFFSET
    // END OFFSET IS THE START OF THE SECTION IMMEDIATELY AFTER

    for(k=0;k<10;++k) {
        write(sections[k].offwords);
        ++writeoff;
    }

    // TODO: WRITE OTHER SYSTEM VARIABLES HERE

    // FILL THE HEADER SECTION
    while(writeoff<1024) { write(0) ++writeoff; }

    // HERE WE ARE AT OFFSET 4096 (1024 WORDS)

    // DUMP TEMPBLOCKS TO THE FILE
    for(k=0;k<sections[0].nitems;++k) {
        write((BINT)(TempBlocks[k]-TempOb)+sections[1].offwords);        // WRITE BLOCKS AS OFFSET RELATIVE TO THE FILE INSTEAD OF POINTER
        ++writeoff;
    }

    // DUMP TEMPOB TO THE FILE
    for(k=0;k<sections[1].nitems;++k) {
        write(TempOb[k]);        // WRITE TEMPOB AS-IS, NO POINTERS THERE
        ++writeoff;
    }

    // DUMP DIRECTORIES TO THE FILE
    for(k=0;k<sections[2].nitems;++k) {

        write(TempOb[k]);        // WRITE TEMPOB AS-IS, NO POINTERS THERE
        ++writeoff;
    }



    // TODO: JUST WRITE A WORD FOR NOW
    writefunc(0X12345678);

    return;

}


// FULLY RESTORE TEMPOB AND DIRECTORIES FROM BACKUP

void rplRestoreBackup(void (*readfunc)(unsigned int))
{

}
