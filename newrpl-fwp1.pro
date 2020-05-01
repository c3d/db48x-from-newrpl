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
    firmware/sys/target_prime1/preamble.c \
    firmware/sys/target_prime1/lcd.c \
    firmware/ggl/ggl/ggl_bitblt.c \
    firmware/ggl/ggl/ggl_bitbltoper.c \
    firmware/ggl/ggl/ggl_filter.c \
    firmware/ggl/ggl/ggl_fltdarken.c \
    firmware/ggl/ggl/ggl_fltlighten.c \
    firmware/ggl/ggl/ggl_fltinvert.c \
    firmware/ggl/ggl/ggl_getnib.c \
    firmware/ggl/ggl/ggl_hblt.c \
    firmware/ggl/ggl/ggl_hbltfilter.c \
    firmware/ggl/ggl/ggl_hbltoper.c \
    firmware/ggl/ggl/ggl_hline.c \
    firmware/ggl/ggl/ggl_initscr.c \
    firmware/ggl/ggl/ggl_mkcolor.c \
    firmware/ggl/ggl/ggl_mkcolor32.c \
    firmware/ggl/ggl/ggl_opmask.c \
    firmware/ggl/ggl/ggl_optransp.c \
    firmware/ggl/ggl/ggl_ovlblt.c \
    firmware/ggl/ggl/ggl_pltnib.c \
    firmware/ggl/ggl/ggl_rect.c \
    firmware/ggl/ggl/ggl_rectp.c \
    firmware/ggl/ggl/ggl_revblt.c \
    firmware/ggl/ggl/ggl_scrolldn.c \
    firmware/ggl/ggl/ggl_scrolllf.c \
    firmware/ggl/ggl/ggl_scrollrt.c \
    firmware/ggl/ggl/ggl_scrollup.c \
    firmware/ggl/ggl/ggl_vline.c \
    firmware/ggl/ggl/ggl_fltreplace.c

HEADERS  += \
    newrpl/arithmetic.h \
    newrpl/decimal.h \
    newrpl/fastmath.h \
    newrpl/libraries.h \
    newrpl/newrpl.h \
    newrpl/newrpl_types.h \
    newrpl/sysvars.h \
    newrpl/utf8lib.h \
    firmware/include/firmware.h \
    firmware/include/ggl.h \
    firmware/include/hal_api.h \
    firmware/include/target_prime1.h \
    firmware/include/ui.h \
    firmware/include/usb.h


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
