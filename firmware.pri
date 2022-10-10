#******************************************************************************
# firmwware.pri                                                   DB48X project
#******************************************************************************
#
#  File Description:
#
#   Configuration for firmware builds
#
#
#
#
#
#
#
#
#******************************************************************************
#  (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
#  (C) 2022 Claudio Lapilli and the newRPL team
#  This software is licensed under the terms described in LICENSE.txt
#******************************************************************************

isEmpty(HOST):HOST = hardware

newrpl_color:NEWRPL_CONFIG += newrpl_color

# Strip the configuratoin to the strictest minimim
CONFIG(debug, debug|release) {
    CONFIG = debug static ordered depend_includepath
} else {
    CONFIG = static ordered depend_includepath
}

CONFIG += newrpl_firmware $$NEWRPL_CONFIG

TEMPLATE = app

DEFINES += TARGET_FIRMWARE
DEFINES += RECORDER_STANDALONE RECORDER_STANDALONE_PRINTF RECORDER_NO_ATOMICS

# Compile in thumb mode to reduce memory usage and improve battery life
THUMB_MODE=-mthumb

# Uncomment below to generate detailed assembly output of each file
#DEVEL_OPTIONS=-Wa,-adhln=$@.s

include(newrpl.pri)

# ARM-specific optimizations
SOURCES += \
    newrpl/mul_real_arm.c \

INCLUDEPATH += firmware/include newrpl

LIBS += -lc -lgcc

DISTFILES += firmware/platform/$$PLATFORM/ld.script

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc
QMAKE_CFLAGS =--specs=nosys.specs

QMAKE_CFLAGS_COMMON = \
	$${DEVEL_OPTIONS} \
	-pipe \
	-mlittle-endian \
	-msoft-float \
	-mtune=$$MACHINE_TUNE \
	-mcpu=$$MACHINE_CPU \
	-march=$$MACHINE_ARCH \
	-mthumb-interwork \
	-fomit-frame-pointer \
	-fdata-sections \
	-ffunction-sections \
	-Wno-packed-bitfield-compat \

QMAKE_CFLAGS_DEBATABLE = \
	-nostdinc \
	-fno-jump-tables \
	-fno-toplevel-reorder \
	-fno-tree-loop-distribute-patterns \
	-fno-partial-inlining

QMAKE_CFLAGS_DEBUG   = -g -Og $$QMAKE_CFLAGS_COMMON
QMAKE_CFLAGS_RELEASE = -O3 $$QMAKE_CFLAGS_COMMON

QMAKE_LFLAGS = \
	--specs=nosys.specs \
	-g \
	-T$$PWD/firmware/platform/$$PLATFORM/ld.script \
	-L$$GCC_LIBDIR \
	-nostdlib \
	-nodefaultlibs \
	-Wl,--gc-sections \
	-Wl,-Map,build/$(TARGET).map

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom $(TARGET)
