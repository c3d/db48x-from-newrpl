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
