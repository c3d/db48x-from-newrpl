#ifndef CMDCODES_H
#define CMDCODES_H


#define COMMANDS_ONLY_PASS

#include "romlibs.h"

#define INCLUDELIB(a,b) b
#define LIBRARY_LIST ROM_LIST

// ITERATE OVER ALL LIBRARIES
#include "include-all.h"

#undef LIBRARY_LIST
#undef INCLUDELIB
#undef COMMANDS_ONLY_PASS

#endif // CMDCODES_H

