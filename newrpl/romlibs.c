/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl_types.h"
#include "cmdcodes.h"

#define INCLUDELIB(a,b) lib##a##_handler
// THIS IS THE LIST OF ALL PRE-INSTALLED LIBRARIES IN THIS ROM
const LIBHANDLER const ROMLibs[] = { ROM_LIST, 0 };

#undef INCLUDELIB
