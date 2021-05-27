#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------


TARGET = primeg1_newrpl.elf
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
    firmware/sys/Font18.c \
    firmware/sys/FontNotification.c \
    firmware/sys/target_prime1/preamble_prime1.c \
    firmware/sys/target_prime1/boot_prime1.c \
    firmware/sys/target_prime1/nand.c \
    firmware/sys/target_prime1/battery.c \
    firmware/sys/target_prime1/cpu.c \
    firmware/sys/target_prime1/exception.c \
    firmware/sys/target_prime1/irq.c \
    firmware/sys/target_prime1/keyboard.c \
    firmware/sys/keybcommon.c \
    firmware/sys/target_prime1/lcd.c \
    firmware/sys/target_prime1/stdlib.c \
    firmware/sys/target_prime1/timer.c \
    firmware/sys/target_prime1/mem.c \
    firmware/sys/target_prime1/rtc.c \
    firmware/sys/target_prime1/touch.c \
    firmware/sys/target_prime1/usbdriver.c \
    firmware/sys/usbcommon.c \
    firmware/sys/target_prime1/fwupdate.c \
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
    firmware/ggl/ggl/ggl_fltreplace.c \
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
    firmware/sys/Font10A.c \
    firmware/sys/Font24.c \
    firmware/sys/target_prime1/sddriver.c \
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
    firmware/hal_cpu.c \
    firmware/hal_globals.c \
    firmware/hal_battery_primeg1.c \
    firmware/hal_screen_primeg1.c \
    firmware/ui_cmdline.c \
    firmware/ui_softmenu.c \
    firmware/hal_clock.c \
    firmware/hal_keyboard_primeg1.c \
    firmware/hal_alarm.c \
    firmware/ui_render.c \
    firmware/ui_forms.c \
    newrpl/decimal.c \
    newrpl/lighttranscend.c \
    newrpl/atan_ltables.c \
    newrpl/ln_ltables.c \
    newrpl/sysvars.c \
    newrpl/compiler.c \
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
    newrpl/lib-zero-messages.c \
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
    newrpl/lib-76-ui.c \
    newrpl/lib-77-libdata.c \
    newrpl/lib-78-fonts.c \
    newrpl/lib-80-bitmaps.c \
    newrpl/lib-88-plot.c \
    newrpl/lib-96-composites.c \
    newrpl/lib-98-statistics.c \
    newrpl/lib-100-usb.c \
    newrpl/lib-102-libptr.c \
    newrpl/fastmath.c \
    newrpl/render.c \
    newrpl/mul_real_arm.c \
    newrpl/rng.c \
    newrpl/solvers.c \
    newrpl/lib-104-solvers.c \
    newrpl/lib-55-constants.c \
    newrpl/lib-4081-tags.c \
    newrpl/lib-112-asm.c



HEADERS  += \
    firmware/include/ggl.h \
    firmware/include/target_prime1.h \
    firmware/include/ui.h \
    firmware/include/hal_api.h \
    firmware/include/usb.h \
    newrpl/libraries.h \
    newrpl/newrpl.h \
    newrpl/newrpl_types.h \
    newrpl/sysvars.h \
    newrpl/decimal.h \
    newrpl/arithmetic.h \
    newrpl/cmdcodes.h \
    newrpl/common-macros.h \
    newrpl/lib-header.h \
    newrpl/include-all.h \
    newrpl/romlibs.h \
    firmware/sys/rtc.h \
    firmware/sys/sddriver.h \
    firmware/sys/fsystem/fsyspriv.h \
    firmware/include/fsystem.h \
    newrpl/fastmath.h \
    newrpl/render.h \
    firmware/include/firmware.h

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
                newrpl/rpl-objects/lib-62.nrpl \
                newrpl/rpl-objects/lib-64.nrpl \
                newrpl/rpl-objects/lib-65.nrpl \
                newrpl/rpl-objects/lib-66.nrpl \
                newrpl/rpl-objects/lib-68.nrpl \
                newrpl/rpl-objects/lib-70.nrpl \
                newrpl/rpl-objects/lib-72.nrpl \
                newrpl/rpl-objects/lib-74.nrpl \
                newrpl/rpl-objects/lib-76.nrpl \
                newrpl/rpl-objects/lib-0.nrpl \
                newrpl/rpl-objects/lib-8.nrpl \
                newrpl/rpl-objects/lib-52.nrpl \
                newrpl/rpl-objects/lib-55.nrpl \
                newrpl/rpl-objects/lib-56.nrpl \
                newrpl/rpl-objects/lib-77.nrpl \
                newrpl/rpl-objects/lib-78.nrpl \
                newrpl/rpl-objects/lib-80.nrpl \
                newrpl/rpl-objects/lib-88.nrpl \
                newrpl/rpl-objects/lib-96.nrpl \
                newrpl/rpl-objects/lib-98.nrpl \
                newrpl/rpl-objects/lib-100.nrpl \
                newrpl/rpl-objects/lib-102.nrpl \
                newrpl/rpl-objects/lib-104.nrpl \
                newrpl/rpl-objects/lib-4081.nrpl \
                newrpl/rpl-objects/version.nrpl


# Cross compiler dependent

GCC_LIBDIR = $$system(arm-none-eabi-gcc -print-file-name=)

INCLUDEPATH += $$GCC_LIBDIR/include
QMAKE_LIBDIR += $$GCC_LIBDIR




INCLUDEPATH += firmware/include newrpl /usr/local/include /usr/include

LIBS += -lgcc

DISTFILES += \
    firmware/sys/target_prime1/ld_newrpl.script

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc

QMAKE_CFLAGS_DEBUG = -g $${DEVEL_OPTIONS} -mtune=arm926ej-s -mcpu=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -Og -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns -fno-isolate-erroneous-paths-dereference
QMAKE_CFLAGS_RELEASE = $${DEVEL_OPTIONS} -mtune=arm926ej-s -mcpu=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -O2 -fno-partial-inlining -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns -fno-isolate-erroneous-paths-dereference
QMAKE_CFLAGS_SHLIB =
QMAKE_CFLAGS_MT =
QMAKE_CFLAGS_MT_DBG =
QMAKE_CFLAGS_THREAD =
QMAKE_CFLAGS_APP =

QMAKE_LFLAGS_DEBUG =
QMAKE_LFLAGS_SHAPP =
QMAKE_LFLAGS_THREAD =
QMAKE_LFLAGS = -g -T$$PWD/firmware/sys/target_prime1/ld_newrpl.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom -outNEWRPL.ROM $(TARGET)


## Additional RPL compiler, make sure it's in the PATH
rpl_compiler.output = auto_${QMAKE_FILE_BASE}.c
rpl_compiler.commands = $$PWD/tools-bin/newrpl-comp -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
rpl_compiler.input = RPL_OBJECTS
rpl_compiler.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += rpl_compiler
