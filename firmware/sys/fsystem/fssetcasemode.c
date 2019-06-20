/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


#ifndef CONFIG_NO_FSYSTEM


void FSSetCaseMode(int newmode)
{
if( (newmode<0) || (newmode>3)) return;
FSystem.CaseMode=newmode;
return;
}

#endif


