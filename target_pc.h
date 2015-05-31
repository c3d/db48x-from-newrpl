#ifndef TARGET_PC_H
#define TARGET_PC_H


extern char PhysicalScreen[8192];
extern char ExceptionScreen[8192];

#undef MEM_PHYS_SCREEN
#define MEM_PHYS_SCREEN PhysicalScreen

#undef MEM_PHYS_EXSCREEN
#define MEM_PHYS_EXSCREEN ExceptionScreen


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

#endif // TARGET_PC_H
