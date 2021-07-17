#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------


TARGET = primeg1_multiload.elf
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
    firmware/sys/target_prime1/preamble_multiload.c \
    firmware/sys/target_prime1/boot_multiload.c \
    firmware/sys/target_prime1/cpu.c \
    firmware/sys/target_prime1/exception.c \
    firmware/sys/target_prime1/irq.c \
    firmware/sys/target_prime1/lcd.c \
    firmware/sys/target_prime1/nand.c \
    firmware/sys/target_prime1/rtc.c \
    firmware/sys/target_prime1/sddriver.c \
    firmware/sys/target_prime1/stdlib.c \
    firmware/sys/target_prime1/timer.c \
    firmware/sys/keybcommon.c \
    firmware/sys/target_prime1/keyboard.c \
    firmware/sys/fsystem/fatconvert.c \
    firmware/sys/fsystem/fsattr.c \
    firmware/sys/fsystem/fscalcfreespace.c \
    firmware/sys/fsystem/fschattr.c \
    firmware/sys/fsystem/fschdir.c \
    firmware/sys/fsystem/fschmode.c \
    firmware/sys/fsystem/fsclose.c \
    firmware/sys/fsystem/fscloseanddelete.c \
    firmware/sys/fsystem/fsconvert2shortentry.c \
    firmware/sys/fsystem/fscreate.c \
    firmware/sys/fsystem/fsdelete.c \
    firmware/sys/fsystem/fsdeletedirentry.c \
    firmware/sys/fsystem/fseof.c \
    firmware/sys/fsystem/fsexpandchain.c \
    firmware/sys/fsystem/fsfileisopen.c \
    firmware/sys/fsystem/fsfileisreferenced.c \
    firmware/sys/fsystem/fsfilelength.c \
    firmware/sys/fsystem/fsfindchar.c \
    firmware/sys/fsystem/fsfindentry.c \
    firmware/sys/fsystem/fsfindfile.c \
    firmware/sys/fsystem/fsfindforcreation.c \
    firmware/sys/fsystem/fsflushbuffers.c \
    firmware/sys/fsystem/fsflushfatcache.c \
    firmware/sys/fsystem/fsfreechain.c \
    firmware/sys/fsystem/fsfreefile.c \
    firmware/sys/fsystem/fsgetaccessdate.c \
    firmware/sys/fsystem/fsgetchain.c \
    firmware/sys/fsystem/fsgetchainsize.c \
    firmware/sys/fsystem/fsgetcreattime.c \
    firmware/sys/fsystem/fsgetcurrentvolume.c \
    firmware/sys/fsystem/fsgetcwd.c \
    firmware/sys/fsystem/fsgetdatetime.c \
    firmware/sys/fsystem/fsgeterrormsg.c \
    firmware/sys/fsystem/fsgetfilename.c \
    firmware/sys/fsystem/fsgethandle.c \
    firmware/sys/fsystem/fsgetnametype.c \
    firmware/sys/fsystem/fsgetnextentry.c \
    firmware/sys/fsystem/fsgetvolumefree.c \
    firmware/sys/fsystem/fsgetvolumesize.c \
    firmware/sys/fsystem/fsgetwritetime.c \
    firmware/sys/fsystem/fsinit.c \
    firmware/sys/fsystem/fsmkdir.c \
    firmware/sys/fsystem/fsmountvolume.c \
    firmware/sys/fsystem/fsmovedopenfiles.c \
    firmware/sys/fsystem/fsnamecompare.c \
    firmware/sys/fsystem/fsopen.c \
    firmware/sys/fsystem/fsopendir.c \
    firmware/sys/fsystem/fspackdir.c \
    firmware/sys/fsystem/fspackname.c \
    firmware/sys/fsystem/fspatchfatblock.c \
    firmware/sys/fsystem/fsread.c \
    firmware/sys/fsystem/fsreadll.c \
    firmware/sys/fsystem/fsreleaseentry.c \
    firmware/sys/fsystem/fsrename.c \
    firmware/sys/fsystem/fsrestart.c \
    firmware/sys/fsystem/fsrmdir.c \
    firmware/sys/fsystem/fsseek.c \
    firmware/sys/fsystem/fssetcasemode.c \
    firmware/sys/fsystem/fssetcurrentvolume.c \
    firmware/sys/fsystem/fsshutdown.c \
    firmware/sys/fsystem/fssleep.c \
    firmware/sys/fsystem/fsstripsemi.c \
    firmware/sys/fsystem/fstell.c \
    firmware/sys/fsystem/fstruncatechain.c \
    firmware/sys/fsystem/fsupdatedirentry.c \
    firmware/sys/fsystem/fsvolumeinserted.c \
    firmware/sys/fsystem/fsvolumemounted.c \
    firmware/sys/fsystem/fsvolumepresent.c \
    firmware/sys/fsystem/fswrite.c \
    firmware/sys/fsystem/fswritefatentry.c \
    firmware/sys/fsystem/fswritell.c \
    firmware/sys/fsystem/fsystem.c \
    firmware/sys/fsystem/misalign.c \
    firmware/sys/fsystem/fsallocator.c \
    firmware/ggl/cgl/cgl_bitblt.c \
    firmware/ggl/cgl/cgl_bitbltoper.c \
    firmware/ggl/cgl/cgl_filter.c \
    firmware/ggl/cgl/cgl_fltdarken.c \
    firmware/ggl/cgl/cgl_fltlighten.c \
    firmware/ggl/cgl/cgl_fltinvert.c \
    firmware/ggl/cgl/cgl_getnib.c \
    firmware/ggl/cgl/cgl_hblt.c \
    firmware/ggl/cgl/cgl_hbltfilter.c \
    firmware/ggl/cgl/cgl_hbltoper.c \
    firmware/ggl/cgl/cgl_hline.c \
    firmware/ggl/cgl/cgl_initscr.c \
    firmware/ggl/cgl/cgl_mkcolor32.c \
    firmware/ggl/cgl/cgl_opmask.c \
    firmware/ggl/cgl/cgl_optransp.c \
    firmware/ggl/cgl/cgl_ovlblt.c \
    firmware/ggl/cgl/cgl_pltnib.c \
    firmware/ggl/cgl/cgl_rect.c \
    firmware/ggl/cgl/cgl_rectp.c \
    firmware/ggl/cgl/cgl_revblt.c \
    firmware/ggl/cgl/cgl_scrolldn.c \
    firmware/ggl/cgl/cgl_scrolllf.c \
    firmware/ggl/cgl/cgl_scrollrt.c \
    firmware/ggl/cgl/cgl_scrollup.c \
    firmware/ggl/cgl/cgl_vline.c \
    firmware/ggl/cgl/cgl_fltreplace.c \
    firmware/sys/Font10A.c \
    firmware/sys/graphics.c \
    newrpl/decimal.c \
    newrpl/mul_real_arm.c \
    newrpl/sysvars.c \
    newrpl/utf8data.c \
    newrpl/utf8lib.c

HEADERS  += \
    firmware/sys/target_prime1/nand.h \
    newrpl/arithmetic.h \
    newrpl/decimal.h \
    newrpl/fastmath.h \
    newrpl/libraries.h \
    newrpl/newrpl.h \
    newrpl/newrpl_types.h \
    newrpl/sysvars.h \
    newrpl/utf8lib.h \
    firmware/sys/fsystem/fsyspriv.h \
    firmware/include/firmware.h \
    firmware/include/fsystem.h \
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
    firmware/sys/target_prime1/ld_multiload.script

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc

QMAKE_CFLAGS_DEBUG = -g $${DEVEL_OPTIONS} -mtune=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -Og -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns
QMAKE_CFLAGS_RELEASE = $${DEVEL_OPTIONS} -mtune=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -O2 -fno-partial-inlining -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns
QMAKE_CFLAGS_SHLIB =
QMAKE_CFLAGS_MT =
QMAKE_CFLAGS_MT_DBG =
QMAKE_CFLAGS_THREAD =
QMAKE_CFLAGS_APP =

QMAKE_LFLAGS_DEBUG =
QMAKE_LFLAGS_SHAPP =
QMAKE_LFLAGS_THREAD =
QMAKE_LFLAGS = -g -T$$PWD/firmware/sys/target_prime1/ld_multiload.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom -pad1024k -outPRIME_OS.ROM $(TARGET)
