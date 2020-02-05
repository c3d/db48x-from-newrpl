#ifndef FIRMWARE_H
#define FIRMWARE_H

// INCLUDE ALL USB EARLY TO ALLOW OVERRIDING SOME CONSTANTS IN target_xxx HEADERS
#include <usb.h>

// REDEFINE SOME CONSTANTS FOR THE VARIOUS TARGETS
#ifdef TARGET_PC
#include <target_pc.h>
#else
#ifdef TARGET_39GS
#include <target_39gs.h>
#else
#ifdef TARGET_40GS
#include <target_40gs.h>
#else
#include <target_50g.h>
#endif
#endif
#endif

#endif // FIRMWARE_H
