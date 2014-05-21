/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// HARDWARE DEPENDENT LAYER
#include "hal.h"


#ifdef TARGET_PC_SIMULATOR


// NEWSIZE IS IN 32-BIT WORDS!
WORDPTR *hal_growmem(WORDPTR *base, BINT newsize)
{
    if(!base) return  malloc(MAX_RAM);
    else {
        if((newsize<<2)>MAX_RAM) return 0;
        return base;
    }
}




#endif


#ifdef TARGET_50G

// NEWSIZE IS IN 32-BIT WORDS!
WORDPTR *hal_growmem(WORDPTR *base,BINT newsize)
{
// TODO: ALLOCATE RAM PAGES TO THIS REGION

}



#endif
