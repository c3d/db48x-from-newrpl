#ifndef XGL_H
#define XGL_H

// Select the proper library according to the target (4-bit grayscale or full color)

#ifdef TARGET_PC
#include "ggl.h"
#endif
#ifdef TARGET_PC_PRIMEG1
#include "cgl.h"
#endif

#ifdef TARGET_PRIME1
//#include "cgl.h"
#include "ggl.h"
#endif

#ifdef TARGET_50G
#include "ggl.h"
#endif

#ifdef TARGET_39GS
#include "ggl.h"
#endif

#ifdef TARGET_40GS
#include "ggl.h"
#endif

#ifdef TARGET_48GII
#include "ggl.h"
#endif

#endif // XGL_H
