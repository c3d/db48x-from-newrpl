/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#ifndef TARGET_PC_H
#define TARGET_PC_H


extern char PhysicalScreen[8192];
extern char ExceptionScreen[8192];

#undef MEM_PHYS_SCREEN
#define MEM_PHYS_SCREEN PhysicalScreen

#undef MEM_PHYS_EXSCREEN
#define MEM_PHYS_EXSCREEN ExceptionScreen

#undef DEFAULT_AUTOOFFTIME
#define DEFAULT_AUTOOFFTIME 0   // NO AUTO OFF ON A PC!

#undef __ENABLE_ARM_ASSEMBLY__ // THIS TARGET IS NOT ARM

#undef __SYSTEM_GLOBAL__
#define __SYSTEM_GLOBAL__

#undef __DATAORDER1__
#define __DATAORDER1__

#undef __DATAORDER2__
#define __DATAORDER2__

#undef __DATAORDER3__
#define __DATAORDER3__

#undef __DATAORDERLAST__
#define __DATAORDERLAST__

#undef __SCRATCH_MEMORY__
#define __SCRATCH_MEMORY__

#undef __ROMOBJECT__
#define __ROMOBJECT__

#undef __ROMLINK__
#define __ROMLINK__



#endif // TARGET_PC_H
