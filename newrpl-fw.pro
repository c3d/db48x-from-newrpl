#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------


TARGET = newrplfw.elf
TEMPLATE = app
CONFIG = static ordered

DEFINES += TARGET_50G NDEBUG


# DO NOT ALTER THE ORDER OF THESE MODULES


SOURCES +=\
    firmware/sys/target_50g/preamble.c \
    firmware/sys/target_50g/boot.c \
    firmware/sys/target_50g/battery.c \
    firmware/sys/target_50g/cpu.c \
    firmware/sys/target_50g/exception.c \
    firmware/sys/target_50g/irq.c \
    firmware/sys/target_50g/keyboard.c \
    firmware/sys/keybcommon.c \
    firmware/sys/target_50g/lcd.c \
    firmware/sys/target_50g/stdlib.c \
    firmware/sys/target_50g/timer.c \
    firmware/sys/target_50g/mem.c \
    firmware/sys/target_50g/flash.c \
    newrpl/decimal.c \
    newrpl/sysvars.c \
    newrpl/dectranscen.c \
    newrpl/atan_1_8_comp.c \
    newrpl/atan_2_8_comp.c \
    newrpl/atan_5_8_comp.c \
    newrpl/atanh_1_8_comp.c \
    newrpl/atanh_2_8_comp.c \
    newrpl/atanh_5_8_comp.c \
    newrpl/compiler.c \
    newrpl/cordic_K_8_comp.c \
    newrpl/cordic_Kh_8_comp.c \
    newrpl/datastack.c \
    newrpl/directory.c \
    newrpl/errors.c \
    newrpl/gc.c \
    newrpl/lam.c \
    newrpl/lists.c \
    newrpl/matrix.c \
    newrpl/units.c \
    newrpl/returnstack.c \
    newrpl/romlibs.c \
    newrpl/runstream.c \
    newrpl/symbolic.c \
    newrpl/tempob.c \
    newrpl/backup.c \
    newrpl/sanity.c \
    newrpl/utf8lib.c \
    newrpl/utf8data.c \
    newrpl/autocomplete.c \
    newrpl/arithmetic.c \
    firmware/hal_globals.c \
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
    firmware/ui_cmdline.c \
    firmware/ui_softmenu.c \
    firmware/hal_battery.c \
    firmware/hal_keyboard.c \
    firmware/hal_screen.c \
    firmware/sys/graphics.c \
    firmware/sys/icons.c \
    firmware/sys/Font5A.c \
    firmware/sys/Font5B.c \
    firmware/sys/Font5C.c \
    firmware/sys/Font6A.c \
    firmware/sys/Font6m.c \
    firmware/sys/Font7A.c \
    firmware/sys/Font8A.c \
    firmware/sys/Font8B.c \
    firmware/sys/Font8C.c \
    firmware/sys/Font8D.c \
    firmware/hal_msgenglish.c \
    firmware/sys/target_50g/sddriver.c \
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
    newrpl/lib-4080-localenv.c \
    newrpl/lib-4090-overloaded.c \
    newrpl/lib-common.c \
    newrpl/lib-two-ident.c \
    newrpl/lib-eight-docol.c \
    newrpl/lib-nine-docol2.c \
    newrpl/lib-ten-reals.c \
    newrpl/lib-twelve-bint.c \
    newrpl/lib-20-comments.c \
    newrpl/lib-24-string.c \
    newrpl/lib-28-dirs.c \
    newrpl/lib-30-complex.c \
    newrpl/lib-32-lam.c \
    newrpl/lib-48-angles.c \
    newrpl/lib-52-matrix.c \
    newrpl/lib-54-units.c \
    newrpl/lib-56-symbolic.c \
    newrpl/lib-62-lists.c \
    newrpl/lib-64-arithmetic.c \
    newrpl/lib-65-system.c \
    newrpl/lib-66-transcendentals.c \
    newrpl/lib-68-flags.c \
    newrpl/lib-70-binary.c \
    newrpl/lib-72-stack.c \
    newrpl/lib-74-sdcard.c \
    newrpl/lib-80-fonts.c \
    firmware/sys/target_50g/rtc.c

HEADERS  += \
    firmware/include/ggl.h \
    firmware/include/ui.h \
    firmware/include/hal_api.h \
    newrpl/hal.h \
    newrpl/libraries.h \
    newrpl/newrpl.h \
    newrpl/sysvars.h \
    newrpl/decimal.h \
    newrpl/errorcodes.h \
    newrpl/arithmetic.h \
    newrpl/cmdcodes.h \
    newrpl/common-macros.h \
    newrpl/lib-header.h \
    newrpl/include-all.h \
    newrpl/romlibs.h \
    firmware/sys/sddriver.h \
    firmware/sys/fsystem/fsyspriv.h \
    firmware/include/fsystem.h


RPL_OBJECTS =   newrpl/rpl-objects/lib-54.nrpl \
                newrpl/rpl-objects/lib-9.nrpl \
                newrpl/rpl-objects/lib-10.nrpl \
                newrpl/rpl-objects/lib-12.nrpl \
                newrpl/rpl-objects/lib-20.nrpl \
                newrpl/rpl-objects/lib-24.nrpl \
                newrpl/rpl-objects/lib-28.nrpl \
                newrpl/rpl-objects/lib-30.nrpl \
                newrpl/rpl-objects/lib-32.nrpl \
                newrpl/rpl-objects/lib-48.nrpl \
                newrpl/rpl-objects/lib-64.nrpl \
                newrpl/rpl-objects/lib-74.nrpl






# This might need to be adapted to each cross-compiler installation
GCC_LIBDIR = /usr/lib/gcc/arm-none-eabi/4.9.3

INCLUDEPATH += $$GCC_LIBDIR/include
QMAKE_LIBDIR += $$GCC_LIBDIR

# End of cross-compiler dependent


INCLUDEPATH += firmware/include newrpl /usr/local/include /usr/include

LIBS += -lgcc

FORMS    +=

DISTFILES += \
    firmware/ld.script

RESOURCES +=

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc
#QMAKE_AR_CMD = arm-none-eabi-ar -cqs $(TARGET) $(OBJECTS)
#QMAKE_AR_CMD = arm-none-eabi-ld --verbose -T$$PWD/firmware/ld.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR $(OBJECTS) -lgcc -o $(TARGET).elf
QMAKE_CFLAGS_DEBUG =
QMAKE_CFLAGS_RELEASE =
QMAKE_CFLAGS_SHLIB =
QMAKE_CFLAGS_MT =
QMAKE_CFLAGS_MT_DBG =
QMAKE_CFLAGS_THREAD =
QMAKE_CFLAGS = -mtune=arm920t -mcpu=arm920t -mlittle-endian -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -Os -pipe -mthumb-interwork -nostdinc
QMAKE_CFLAGS_APP =

QMAKE_LFLAGS_DEBUG =
QMAKE_LFLAGS_SHAPP =
QMAKE_LFLAGS_THREAD =
QMAKE_LFLAGS = -T$$PWD/firmware/ld.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom $(TARGET)


## Additional RPL compiler, make sure it's in the PATH
rpl_compiler.output = auto_${QMAKE_FILE_BASE}.c
rpl_compiler.commands = $$PWD/tools-bin/newrpl-comp -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
rpl_compiler.input = RPL_OBJECTS
rpl_compiler.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += rpl_compiler
