#ifndef TARGET_PC_H
#define TARGET_PC_H


extern char PhysicalScreen[8192];
extern char ExceptionScreen[8192];

#undef MEM_PHYS_SCREEN
#define MEM_PHYS_SCREEN PhysicalScreen

#undef MEM_PHYS_EXSCREEN
#define MEM_PHYS_EXSCREEN ExceptionScreen


#define __SYSTEM_GLOBAL__





#endif // TARGET_PC_H
