#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = newrpl-ui
TEMPLATE = app

DEFINES += TARGET_PC

SOURCES += main.cpp\
        mainwindow.cpp \
    qemuscreen.cpp \
    firmware/ggl/ggl/ggl_bitblt.c \
    firmware/ggl/ggl/ggl_bitbltoper.c \
    firmware/ggl/ggl/ggl_filter.c \
    firmware/ggl/ggl/ggl_fltdarken.c \
    firmware/ggl/ggl/ggl_fltlighten.c \
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
    firmware/hal_battery.c \
    firmware/hal_keyboard.c \
    firmware/hal_screen.c \
    firmware/sys/graphics.c \
    firmware/sys/icons.c \
    firmware/sys/target_pc/battery.c \
    firmware/sys/target_pc/cpu.c \
    firmware/sys/target_pc/exception.c \
    firmware/sys/target_pc/irq.c \
    firmware/sys/target_pc/keyboard.c \
    firmware/sys/target_pc/lcd.c \
    firmware/sys/target_pc/stdlib.c \
    firmware/sys/target_pc/timer.c \
    firmware/sys/target_pc/sddriver.c \
    firmware/sys/target_pc/fsystem/fatconvert.c \
    firmware/sys/target_pc/fsystem/fsallocator.c \
    firmware/sys/target_pc/fsystem/fsattr.c \
    firmware/sys/target_pc/fsystem/fscalcfreespace.c \
    firmware/sys/target_pc/fsystem/fschattr.c \
    firmware/sys/target_pc/fsystem/fschdir.c \
    firmware/sys/target_pc/fsystem/fschmode.c \
    firmware/sys/target_pc/fsystem/fsclose.c \
    firmware/sys/target_pc/fsystem/fscloseanddelete.c \
    firmware/sys/target_pc/fsystem/fsconvert2shortentry.c \
    firmware/sys/target_pc/fsystem/fscreate.c \
    firmware/sys/target_pc/fsystem/fsdelete.c \
    firmware/sys/target_pc/fsystem/fsdeletedirentry.c \
    firmware/sys/target_pc/fsystem/fseof.c \
    firmware/sys/target_pc/fsystem/fsexpandchain.c \
    firmware/sys/target_pc/fsystem/fsfileisopen.c \
    firmware/sys/target_pc/fsystem/fsfileisreferenced.c \
    firmware/sys/target_pc/fsystem/fsfilelength.c \
    firmware/sys/target_pc/fsystem/fsfindchar.c \
    firmware/sys/target_pc/fsystem/fsfindentry.c \
    firmware/sys/target_pc/fsystem/fsfindfile.c \
    firmware/sys/target_pc/fsystem/fsfindforcreation.c \
    firmware/sys/target_pc/fsystem/fsflushbuffers.c \
    firmware/sys/target_pc/fsystem/fsflushfatcache.c \
    firmware/sys/target_pc/fsystem/fsfreechain.c \
    firmware/sys/target_pc/fsystem/fsfreefile.c \
    firmware/sys/target_pc/fsystem/fsgetaccessdate.c \
    firmware/sys/target_pc/fsystem/fsgetchain.c \
    firmware/sys/target_pc/fsystem/fsgetchainsize.c \
    firmware/sys/target_pc/fsystem/fsgetcreattime.c \
    firmware/sys/target_pc/fsystem/fsgetcurrentvolume.c \
    firmware/sys/target_pc/fsystem/fsgetcwd.c \
    firmware/sys/target_pc/fsystem/fsgetdatetime.c \
    firmware/sys/target_pc/fsystem/fsgeterrormsg.c \
    firmware/sys/target_pc/fsystem/fsgetfilename.c \
    firmware/sys/target_pc/fsystem/fsgethandle.c \
    firmware/sys/target_pc/fsystem/fsgetnametype.c \
    firmware/sys/target_pc/fsystem/fsgetnextentry.c \
    firmware/sys/target_pc/fsystem/fsgetvolumefree.c \
    firmware/sys/target_pc/fsystem/fsgetvolumesize.c \
    firmware/sys/target_pc/fsystem/fsgetwritetime.c \
    firmware/sys/target_pc/fsystem/fsinit.c \
    firmware/sys/target_pc/fsystem/fsmkdir.c \
    firmware/sys/target_pc/fsystem/fsmountvolume.c \
    firmware/sys/target_pc/fsystem/fsmovedopenfiles.c \
    firmware/sys/target_pc/fsystem/fsnamecompare.c \
    firmware/sys/target_pc/fsystem/fsopen.c \
    firmware/sys/target_pc/fsystem/fsopendir.c \
    firmware/sys/target_pc/fsystem/fspackdir.c \
    firmware/sys/target_pc/fsystem/fspackname.c \
    firmware/sys/target_pc/fsystem/fspatchfatblock.c \
    firmware/sys/target_pc/fsystem/fsread.c \
    firmware/sys/target_pc/fsystem/fsreadll.c \
    firmware/sys/target_pc/fsystem/fsreleaseentry.c \
    firmware/sys/target_pc/fsystem/fsrename.c \
    firmware/sys/target_pc/fsystem/fsrestart.c \
    firmware/sys/target_pc/fsystem/fsrmdir.c \
    firmware/sys/target_pc/fsystem/fsseek.c \
    firmware/sys/target_pc/fsystem/fssetcasemode.c \
    firmware/sys/target_pc/fsystem/fssetcurrentvolume.c \
    firmware/sys/target_pc/fsystem/fsshutdown.c \
    firmware/sys/target_pc/fsystem/fssleep.c \
    firmware/sys/target_pc/fsystem/fsstripsemi.c \
    firmware/sys/target_pc/fsystem/fstell.c \
    firmware/sys/target_pc/fsystem/fstruncatechain.c \
    firmware/sys/target_pc/fsystem/fsupdatedirentry.c \
    firmware/sys/target_pc/fsystem/fsvolumeinserted.c \
    firmware/sys/target_pc/fsystem/fsvolumemounted.c \
    firmware/sys/target_pc/fsystem/fsvolumepresent.c \
    firmware/sys/target_pc/fsystem/fswrite.c \
    firmware/sys/target_pc/fsystem/fswritefatentry.c \
    firmware/sys/target_pc/fsystem/fswritell.c \
    firmware/sys/target_pc/fsystem/fsystem.c \
    firmware/sys/target_pc/fsystem/misalign.c \
    firmware/hal_globals.c \
    newrpl/compiler.c \
    newrpl/datastack.c \
    newrpl/directory.c \
    newrpl/errors.c \
    newrpl/gc.c \
    newrpl/lam.c \
    newrpl/lib-24-string.c \
    newrpl/lib-28-dirs.c \
    newrpl/lib-30-complex.c \
    newrpl/lib-64-arithmetic.c \
    newrpl/lib-66-transcendentals.c \
    newrpl/lib-68-flags.c \
    newrpl/lib-70-binary.c \
    newrpl/lib-72-stack.c \
    newrpl/lib-4080-localenv.c \
    newrpl/lib-4090-overloaded.c \
    newrpl/lib-common.c \
    newrpl/lib-eight-docol.c \
    newrpl/lib-nine-docol2.c \
    newrpl/lib-ten-reals.c \
    newrpl/lib-twelve-bint.c \
    newrpl/lib-two-ident.c \
    newrpl/lists.c \
    newrpl/returnstack.c \
    newrpl/romlibs.c \
    newrpl/runstream.c \
    newrpl/symbolic.c \
    newrpl/sysvars.c \
    newrpl/tempob.c \
    firmware/sys/target_pc/mem.c \
    firmware/sys/target_pc/boot.c \
    rplthread.cpp \
    firmware/ui_cmdline.c \
    newrpl/utf8lib.c \
    newrpl/utf8data.c \
    firmware/sys/Font5C.c \
    firmware/sys/Font6A.c \
    firmware/sys/keybcommon.c \
    firmware/sys/Font7A.c \
    newrpl/matrix.c \
    firmware/sys/Font8C.c \
    firmware/sys/Font8D.c \
    newrpl/atan_1_8_comp.c \
    newrpl/atan_2_8_comp.c \
    newrpl/atan_5_8_comp.c \
    newrpl/atanh_1_8_comp.c \
    newrpl/atanh_2_8_comp.c \
    newrpl/atanh_5_8_comp.c \
    newrpl/cordic_K_8_comp.c \
    newrpl/cordic_Kh_8_comp.c \
    newrpl/decimal.c \
    newrpl/dectranscen.c \
    newrpl/backup.c \
    newrpl/sanity.c \
    newrpl/lib-32-lam.c \
    newrpl/lib-65-system.c \
    firmware/hal_msgenglish.c \
    newrpl/units.c \
    newrpl/lib-62-lists.c \
    newrpl/lib-56-symbolic.c \
    newrpl/lib-52-matrix.c \
    newrpl/lib-54-units.c \
    newrpl/autocomplete.c \
    newrpl/arithmetic.c \
    newrpl/lib-20-comments.c \
    firmware/sys/target_pc/flash.c \
    firmware/ui_softmenu.c \
    firmware/ggl/ggl/ggl_fltinvert.c \
    newrpl/lib-48-angles.c \
    newrpl/lib-74-sdcard.c

HEADERS  += mainwindow.h \
    qemuscreen.h \
    firmware/include/ggl.h \
    target_pc.h \
    firmware/include/ui.h \
    firmware/include/hal_api.h \
    newrpl/hal.h \
    newrpl/libraries.h \
    newrpl/newrpl.h \
    newrpl/sysvars.h \
    rplthread.h \
    newrpl/utf8lib.h \
    newrpl/decimal.h \
    newrpl/errorcodes.h \
    newrpl/arithmetic.h \
    newrpl/cmdcodes.h \
    newrpl/common-macros.h \
    newrpl/lib-header.h \
    newrpl/include-all.h \
    newrpl/romlibs.h \
    firmware/sys/target_pc/fsystem.h \
    firmware/sys/target_pc/sddriver.h \
    firmware/sys/target_pc/fsystem/fsyspriv.h

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


INCLUDEPATH += firmware/include newrpl /usr/local/include /usr/include

LIBS += -L/usr/local/lib

FORMS    += mainwindow.ui

RESOURCES += \
    annunciators.qrc

QMAKE_CFLAGS += -Wno-duplicate-decl-specifier



# Additional RPL compiler, make sure it's in the PATH
rpl_compiler.output = auto_${QMAKE_FILE_BASE}.c
rpl_compiler.commands = $$PWD/tools-bin/newrpl-comp -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
rpl_compiler.input = RPL_OBJECTS
rpl_compiler.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += rpl_compiler

DISTFILES += \








