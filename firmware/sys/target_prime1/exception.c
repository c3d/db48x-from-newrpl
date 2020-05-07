/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>

__ARM_MODE__ void throw_exception(char *message, unsigned int options)
{
	static char buffer[9];
    tohex(options, buffer);
    printline(message, buffer);
    while(1);
}

__ARM_MODE__ void throw_dbgexception(char *message, unsigned int options)
{
    throw_exception(message, options);
}
