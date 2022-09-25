TARGET = newrpl-base
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += TARGET_PC "NEWRPL_BUILDNUM=$$system(git rev-list --count HEAD)"

SOURCES += \
    ../firmware/ggl/ggl/ggl_hblt.c \
    ../firmware/ggl/ggl/ggl_hbltfilter.c \
    ../firmware/ggl/ggl/ggl_initscr.c \
        ../firmware/hal_battery.c \
    ../firmware/hal_keyboard.c \
    ../firmware/hal_screen.c \
    ../firmware/sys/Font18.c \
    ../firmware/sys/graphics.c \
    ../firmware/sys/icons.c \
    ../firmware/sys/target_pc/non-gui-stubs.c \
    ../firmware/sys/target_pc/battery.c \
    ../firmware/sys/target_pc/cpu.c \
    ../firmware/sys/target_pc/exception.c \
    ../firmware/sys/target_pc/irq.c \
    ../firmware/sys/target_pc/keyboard.c \
    ../firmware/sys/target_pc/lcd.c \
    ../firmware/sys/target_pc/stdlib.c \
    ../firmware/sys/target_pc/timer.c \
    ../firmware/sys/target_pc/sddriver.c \
    ../firmware/sys/fsystem/fatconvert.c \
    ../firmware/sys/fsystem/fsallocator.c \
    ../firmware/sys/fsystem/fsattr.c \
    ../firmware/sys/fsystem/fscalcfreespace.c \
    ../firmware/sys/fsystem/fschattr.c \
    ../firmware/sys/fsystem/fschdir.c \
    ../firmware/sys/fsystem/fschmode.c \
    ../firmware/sys/fsystem/fsclose.c \
    ../firmware/sys/fsystem/fscloseanddelete.c \
    ../firmware/sys/fsystem/fsconvert2shortentry.c \
    ../firmware/sys/fsystem/fscreate.c \
    ../firmware/sys/fsystem/fsdelete.c \
    ../firmware/sys/fsystem/fsdeletedirentry.c \
    ../firmware/sys/fsystem/fseof.c \
    ../firmware/sys/fsystem/fsexpandchain.c \
    ../firmware/sys/fsystem/fsfileisopen.c \
    ../firmware/sys/fsystem/fsfileisreferenced.c \
    ../firmware/sys/fsystem/fsfilelength.c \
    ../firmware/sys/fsystem/fsfindchar.c \
    ../firmware/sys/fsystem/fsfindentry.c \
    ../firmware/sys/fsystem/fsfindfile.c \
    ../firmware/sys/fsystem/fsfindforcreation.c \
    ../firmware/sys/fsystem/fsflushbuffers.c \
    ../firmware/sys/fsystem/fsflushfatcache.c \
    ../firmware/sys/fsystem/fsfreechain.c \
    ../firmware/sys/fsystem/fsfreefile.c \
    ../firmware/sys/fsystem/fsgetaccessdate.c \
    ../firmware/sys/fsystem/fsgetchain.c \
    ../firmware/sys/fsystem/fsgetchainsize.c \
    ../firmware/sys/fsystem/fsgetcreattime.c \
    ../firmware/sys/fsystem/fsgetcurrentvolume.c \
    ../firmware/sys/fsystem/fsgetcwd.c \
    ../firmware/sys/fsystem/fsgetdatetime.c \
    ../firmware/sys/fsystem/fsgeterrormsg.c \
    ../firmware/sys/fsystem/fsgetfilename.c \
    ../firmware/sys/fsystem/fsgethandle.c \
    ../firmware/sys/fsystem/fsgetnametype.c \
    ../firmware/sys/fsystem/fsgetnextentry.c \
    ../firmware/sys/fsystem/fsgetvolumefree.c \
    ../firmware/sys/fsystem/fsgetvolumesize.c \
    ../firmware/sys/fsystem/fsgetwritetime.c \
    ../firmware/sys/fsystem/fsinit.c \
    ../firmware/sys/fsystem/fsmkdir.c \
    ../firmware/sys/fsystem/fsmountvolume.c \
    ../firmware/sys/fsystem/fsmovedopenfiles.c \
    ../firmware/sys/fsystem/fsnamecompare.c \
    ../firmware/sys/fsystem/fsopen.c \
    ../firmware/sys/fsystem/fsopendir.c \
    ../firmware/sys/fsystem/fspackdir.c \
    ../firmware/sys/fsystem/fspackname.c \
    ../firmware/sys/fsystem/fspatchfatblock.c \
    ../firmware/sys/fsystem/fsread.c \
    ../firmware/sys/fsystem/fsreadll.c \
    ../firmware/sys/fsystem/fsreleaseentry.c \
    ../firmware/sys/fsystem/fsrename.c \
    ../firmware/sys/fsystem/fsrestart.c \
    ../firmware/sys/fsystem/fsrmdir.c \
    ../firmware/sys/fsystem/fsseek.c \
    ../firmware/sys/fsystem/fssetcasemode.c \
    ../firmware/sys/fsystem/fssetcurrentvolume.c \
    ../firmware/sys/fsystem/fsshutdown.c \
    ../firmware/sys/fsystem/fssleep.c \
    ../firmware/sys/fsystem/fsstripsemi.c \
    ../firmware/sys/fsystem/fstell.c \
    ../firmware/sys/fsystem/fstruncatechain.c \
    ../firmware/sys/fsystem/fsupdatedirentry.c \
    ../firmware/sys/fsystem/fsvolumeinserted.c \
    ../firmware/sys/fsystem/fsvolumemounted.c \
    ../firmware/sys/fsystem/fsvolumepresent.c \
    ../firmware/sys/fsystem/fswrite.c \
    ../firmware/sys/fsystem/fswritefatentry.c \
    ../firmware/sys/fsystem/fswritell.c \
    ../firmware/sys/fsystem/fsystem.c \
    ../firmware/sys/fsystem/misalign.c \
    ../firmware/hal_globals.c \
    compiler.c \
    datastack.c \
    directory.c \
    errors.c \
    gc.c \
    lam.c \
    lib-24-string.c \
    lib-28-dirs.c \
    lib-30-complex.c \
    lib-64-arithmetic.c \
    lib-66-transcendentals.c \
    lib-68-flags.c \
    lib-70-binary.c \
    lib-72-stack.c \
    lib-4080-localenv.c \
    lib-4090-overloaded.c \
    lib-common.c \
    lib-eight-docol.c \
    lib-nine-docol2.c \
    lib-ten-reals.c \
    lib-twelve-bint.c \
    lib-two-ident.c \
    lists.c \
    returnstack.c \
    romlibs.c \
    runstream.c \
    symbolic.c \
    sysvars.c \
    tempob.c \
    ../firmware/sys/target_pc/mem.c \
    ../firmware/sys/target_pc/boot.c \
    ../firmware/ui_cmdline.c \
    utf8lib.c \
    utf8data.c \
    ../firmware/sys/Font5C.c \
    ../firmware/sys/Font6A.c \
    ../firmware/sys/keybcommon.c \
    ../firmware/sys/Font7A.c \
    matrix.c \
    ../firmware/sys/Font8C.c \
    ../firmware/sys/Font8D.c \
    decimal.c \
    backup.c \
    sanity.c \
    lib-32-lam.c \
    lib-65-system.c \
    units.c \
    lib-62-lists.c \
    lib-56-symbolic.c \
    lib-52-matrix.c \
    lib-54-units.c \
    autocomplete.c \
    arithmetic.c \
    lib-20-comments.c \
    ../firmware/sys/target_pc/flash.c \
    ../firmware/ui_softmenu.c \
    lib-4079-rpl2c.c \
    lib-48-angles.c \
    lib-74-sdcard.c \
    ../firmware/sys/Font8B.c \
    ../firmware/sys/Font8A.c \
    ../firmware/sys/Font6m.c \
    ../firmware/sys/Font5B.c \
    ../firmware/sys/Font5A.c \
    ../firmware/sys/Font10A.c \
    ../firmware/sys/target_pc/rtc.c \
    ../firmware/hal_clock.c \
    ../firmware/hal_alarm.c \
    lib-76-ui.c \
    lib-77-libdata.c \
    lib-zero-messages.c \
    lib-78-fonts.c \
    lib-80-bitmaps.c \
    ../firmware/ui_forms.c \
    ../firmware/ui_render.c \
    lib-88-plot.c \
    fastmath.c \
    render.c \
    lib-96-composites.c \
    lib-98-statistics.c \
    atan_ltables.c \
    lighttranscend.c \
    ln_ltables.c \
    solvers.c \
    rng.c \
    lib-100-usb.c \
    lib-102-libptr.c \
    lib-104-solvers.c \
    ../firmware/sys/target_pc/usbdriver.c \
    ../firmware/sys/usbcommon.c \
    ../firmware/hal_cpu.c \
    lib-55-constants.c \
    lib-4081-tags.c \
    lib-112-asm.c \
    ../firmware/sys/target_pc/fwupdate.c \
    ../firmware/sys/Font24.c \
    main.cpp

HEADERS  += \
    ../firmware/include/ggl.h \
    ../firmware/include/target_pc.h \
    ../firmware/include/ui.h \
    ../firmware/include/hal_api.h \
    ../firmware/include/usb.h \
    libraries.h \
    newrpl.h \
    newrpl_types.h \
    sysvars.h \
    utf8lib.h \
    decimal.h \
    arithmetic.h \
    cmdcodes.h \
    common-macros.h \
    lib-header.h \
    include-all.h \
    romlibs.h \
    ../firmware/sys/rtc.h \
    ../firmware/sys/sddriver.h \
    ../firmware/sys/fsystem/fsyspriv.h \
    ../firmware/include/fsystem.h \
    fastmath.h \
    render.h \
    ../firmware/include/firmware.h

RPL_OBJECTS = \
    rpl-objects/lib-54.nrpl \
    rpl-objects/lib-9.nrpl \
    rpl-objects/lib-10.nrpl \
    rpl-objects/lib-12.nrpl \
    rpl-objects/lib-20.nrpl \
    rpl-objects/lib-24.nrpl \
    rpl-objects/lib-28.nrpl \
    rpl-objects/lib-30.nrpl \
    rpl-objects/lib-32.nrpl \
    rpl-objects/lib-48.nrpl \
    rpl-objects/lib-62.nrpl \
    rpl-objects/lib-64.nrpl \
    rpl-objects/lib-65.nrpl \
    rpl-objects/lib-66.nrpl \
    rpl-objects/lib-68.nrpl \
    rpl-objects/lib-70.nrpl \
    rpl-objects/lib-72.nrpl \
    rpl-objects/lib-74.nrpl \
    rpl-objects/lib-76.nrpl \
    rpl-objects/lib-0.nrpl \
    rpl-objects/lib-8.nrpl \
    rpl-objects/lib-52.nrpl \
    rpl-objects/lib-55.nrpl \
    rpl-objects/lib-56.nrpl \
    rpl-objects/lib-77.nrpl \
    rpl-objects/lib-78.nrpl \
    rpl-objects/lib-80.nrpl \
    rpl-objects/lib-88.nrpl \
    rpl-objects/lib-96.nrpl \
    rpl-objects/lib-98.nrpl \
    rpl-objects/lib-100.nrpl \
    rpl-objects/lib-102.nrpl \
    rpl-objects/lib-104.nrpl \
    rpl-objects/lib-4081.nrpl \
    rpl-objects/version.nrpl

INCLUDEPATH += ../firmware/include

LIBS += -L/usr/local/lib

DISTFILES +=

# Clang doesn't like double const specifiers, but are needed for firmware: disable the warning
QMAKE_CFLAGS += -Wno-duplicate-decl-specifier -Wno-implicit-fallthrough


install_bin.path = $$PWD/tools-bin
install_bin.files = $$OUT_PWD/newrpl-base
INSTALLS += install_bin


# Additional external library HIDAPI linked statically into the code


INCLUDEPATH +=  ../external/hidapi/hidapi

HEADERS += ../external/hidapi/hidapi/hidapi.h

win32: SOURCES += ../external/hidapi/windows/hid.c
win32: LIBS += -lsetupapi

freebsd: SOURCES += ../external/hidapi/libusb/hid.c
freebsd: LIBS += -lusb -lthr -liconv

unix:!macx:!freebsd: SOURCES += ../external/hidapi/linux/hid.c
unix:!macx:!freebsd: LIBS += -ludev

macx: SOURCES += ../external/hidapi/mac/hid.c
macx: LIBS += -framework CoreFoundation -framework IOKit

## Additional RPL compiler, make sure it's in the PATH
rpl_compiler.output = auto_${QMAKE_FILE_BASE}.c
rpl_compiler.commands = $$PWD/../tools-bin/newrpl-comp -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
rpl_compiler.input = RPL_OBJECTS
rpl_compiler.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += rpl_compiler

# End of HIDAPI
