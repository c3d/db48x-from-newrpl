#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------


TARGET = newrplp1.elf
TEMPLATE = app

CONFIG(release, debug|release) {
CONFIG = static ordered
}
CONFIG(debug, debug|release) {
CONFIG = debug static ordered

}

DEFINES += TARGET_PRIME1 NDEBUG "NEWRPL_BUILDNUM=$$system(git rev-list --count HEAD)"

# DO NOT ALTER THE ORDER OF THESE MODULES


SOURCES +=\
    firmware/sys/target_prime1/boot.c \
    firmware/sys/target_prime1/preamble.c

HEADERS  += \
    newrpl/newrpl.h \
    newrpl/newrpl_types.h


# Cross compiler dependent

GCC_LIBDIR = $$system(arm-none-eabi-gcc -print-file-name=)

INCLUDEPATH += $$GCC_LIBDIR/include
QMAKE_LIBDIR += $$GCC_LIBDIR




INCLUDEPATH += firmware/include newrpl /usr/local/include /usr/include

LIBS += -lgcc

DISTFILES += \
    firmware/sys/target_prime1/ld.script

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc
#QMAKE_AR_CMD = arm-none-eabi-ar -cqs $(TARGET) $(OBJECTS)
#QMAKE_AR_CMD = arm-none-eabi-ld --verbose -T$$PWD/firmware/sys/target_prime1/ld.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR $(OBJECTS) -lgcc -o $(TARGET).elf


QMAKE_CFLAGS_DEBUG = -g $${DEVEL_OPTIONS} -mtune=arm926ej-s -mcpu=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -Og -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc
QMAKE_CFLAGS_RELEASE = $${DEVEL_OPTIONS} -mtune=arm926ej-s -mcpu=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -O2 -fno-partial-inlining -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc
QMAKE_CFLAGS_SHLIB =
QMAKE_CFLAGS_MT =
QMAKE_CFLAGS_MT_DBG =
QMAKE_CFLAGS_THREAD =
QMAKE_CFLAGS_APP =

QMAKE_LFLAGS_DEBUG =
QMAKE_LFLAGS_SHAPP =
QMAKE_LFLAGS_THREAD =
QMAKE_LFLAGS = -g -T$$PWD/firmware/sys/target_prime1/ld.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom $(TARGET)
