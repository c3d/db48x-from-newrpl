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
    // COMPACT TEMPOB AS MUCH AS POSSIBLE
    rplGCollect();

    // DUMP SYSTEM VARIABLES TO THE FILE
    // DUMP TEMPBLOCKS TO THE FILE
    // DUMP TEMPOB TO THE FILE
    // DUMP DIRECTORIES TO THE FILE

    // TODO: JUST WRITE A WORD FOR NOW
    writefunc(0X12345678);

    return;

}


// FULLY RESTORE TEMPOB AND DIRECTORIES FROM BACKUP

void rplRestoreBackup(void (*readfunc)(unsigned int))
{

}
