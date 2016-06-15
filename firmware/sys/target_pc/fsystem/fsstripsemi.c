/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


// STRIP TRAILING SEMICOLONS ON NAME
void FSStripSemi(char *name)
{
char *ptr=name;

while(*ptr) ++ptr;
--ptr;
while(*ptr==';' && ptr>=name) --ptr;
ptr[1]=0;
}
