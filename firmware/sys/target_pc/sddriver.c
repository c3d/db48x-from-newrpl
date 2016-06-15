
/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <newrpl.h>
#include <ui.h>

// THIS IS A STUB DRIVER TO ALLOW COMPILATION ON A PC
// COMMANDS WILL ACT AS IF THERE WAS NO CARD INSERTED


#include "../fsystem/fsyspriv.h"
// SD MODULE



int SDCardInserted()
{
return FALSE;
}


// FULLY INITIALIZE THE SDCARD INTERFACE
// RETURNS TRUE IF THERE IS A CARD
// FALSE IF THERE'S NO CARD

int SDInit(SD_CARD *card)
{
    UNUSED_ARGUMENT(card);
return TRUE;

}

int SDIOSetup(SD_CARD *card,int shutdown)
{
    UNUSED_ARGUMENT(card);
    if(!shutdown) return TRUE;
    return FALSE;
}

void SDPowerDown()
{
}

void SDPowerUp()
{
}


int SDSelect(int RCA)
{
    UNUSED_ARGUMENT(RCA);
    return 0;
}



// READS WORDS DIRECTLY INTO BUFFER
// AT THE CURRENT BLOCK LENGTH
// CARD MUST BE SELECTED
int SDDRead(int SDAddr,int NumBytes,char *buffer, SD_CARD *card)
{
    UNUSED_ARGUMENT(card);
    UNUSED_ARGUMENT(SDAddr);
    UNUSED_ARGUMENT(NumBytes);
    UNUSED_ARGUMENT(buffer);

    return FALSE;
}



int SDCardInit(SD_CARD * card)
{
    UNUSED_ARGUMENT(card);

    return FALSE;
}

// WRITE BYTES AT SPECIFIC ADDRESS
// AT THE CURRENT BLOCK LENGTH
// CARD MUST BE SELECTED
int SDDWrite(int SDAddr,int NumBytes,char *buffer, SD_CARD *card)
{
    UNUSED_ARGUMENT(card);
    UNUSED_ARGUMENT(SDAddr);
    UNUSED_ARGUMENT(NumBytes);
    UNUSED_ARGUMENT(buffer);


    return FALSE;

}
