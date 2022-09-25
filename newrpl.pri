#******************************************************************************
# newrpl.pri                                                      DB48X project
#******************************************************************************
#
#  File Description:
#
#    This is the main project description for newRPL
#    It is altered by various CONFIG settings, and included by the
#    top-level .pro files
#
#    This file is configured with the variables HOST and PLATFORM.
#    - PLATFORM can be one of: 50g, 39gs, 40gs, 48gii or prime
#    - HOST can be one of: hardware, pc, compiler (future: android, ios)
#
#    When HOST is hardware, we are building native firwmare for PLATFORM
#
#
#******************************************************************************
#  (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
#  (C) 2022 Claudio Lapilli and the newRPL team
#  This software is licensed under the terms described in LICENSE.txt
#******************************************************************************

isEmpty(PLATFORM): error(Cannot build newRPL without selecting a platform)
isEmpty(HOST):     error(Cannot build newRPL without selecting a host)

CONFIG += newrpl_$$PLATFORM newrpl_$$HOST

# Record build number
DEFINES += "NEWRPL_BUILDNUM=$$system(git rev-list --count HEAD)"
DEFINES += PLATFORM_$$PLATFORM
DEFINES += HOST_$$HOST

# Include paths for the firmware and new RPL core
INCLUDEPATH += newrpl
INCLUDEPATH += firmware/host/$$HOST
INCLUDEPATH += firmware/platform/$$PLATFORM
INCLUDEPATH += firmware/include firmware

# Build directory
CONFIG(debug, debug|release) {
    NEWRPL_BUILD=debug
    DEFINES += DEBUG
    DEFINES -= NDEBUG
} else {
    NEWRPL_BUILD=release
    DEFINES -= DEBUG
    DEFINES += NDEBUG
}
OS_NAME = $$system(uname -s)

# Put objects in separate directories for parallel builds
OBJECTS_DIR = build/$$OS_NAME/$$NEWRPL_BUILD/$$TARGET

# Preamble and boot code needs to be placed first
SOURCES += \
        firmware/sys/preamble.c \
        firmware/sys/boot.c \
        firmware/sys/battery.c \
        firmware/sys/cpu.c \
        firmware/sys/exception.c \
        firmware/sys/irq.c \
        firmware/sys/keyboard.c \
        firmware/sys/lcd.c \
        firmware/sys/stdlib.c \
        firmware/sys/timer.c \
        firmware/sys/mem.c \
        firmware/sys/flash.c \
        firmware/sys/rtc.c \
        firmware/sys/usbdriver.c \
        firmware/sys/fwupdate.c \
        firmware/sys/sddriver.c \

# Core of newRPL
SOURCES += \
        newrpl/arithmetic.c \
        newrpl/atan_ltables.c \
        newrpl/autocomplete.c \
        newrpl/backup.c \
        newrpl/compiler.c \
        newrpl/datastack.c \
        newrpl/decimal.c \
        newrpl/directory.c \
        newrpl/errors.c \
        newrpl/fastmath.c \
        newrpl/gc.c \
        newrpl/lam.c \
        newrpl/lib-100-usb.c \
        newrpl/lib-102-libptr.c \
        newrpl/lib-104-solvers.c \
        newrpl/lib-112-asm.c \
        newrpl/lib-20-comments.c \
        newrpl/lib-24-string.c \
        newrpl/lib-28-dirs.c \
        newrpl/lib-30-complex.c \
        newrpl/lib-32-lam.c \
        newrpl/lib-4080-localenv.c \
        newrpl/lib-4081-tags.c \
        newrpl/lib-4090-overloaded.c \
        newrpl/lib-48-angles.c \
        newrpl/lib-52-matrix.c \
        newrpl/lib-54-units.c \
        newrpl/lib-55-constants.c \
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
        newrpl/lib-common.c \
        newrpl/lib-eight-docol.c \
        newrpl/lib-nine-docol2.c \
        newrpl/lib-ten-reals.c \
        newrpl/lib-twelve-bint.c \
        newrpl/lib-two-ident.c \
        newrpl/lib-zero-messages.c \
        newrpl/lighttranscend.c \
        newrpl/lists.c \
        newrpl/ln_ltables.c \
        newrpl/matrix.c \
        newrpl/render.c \
        newrpl/returnstack.c \
        newrpl/rng.c \
        newrpl/romlibs.c \
        newrpl/runstream.c \
        newrpl/sanity.c \
        newrpl/solvers.c \
        newrpl/symbolic.c \
        newrpl/sysvars.c \
        newrpl/tempob.c \
        newrpl/units.c \
        newrpl/utf8data.c \
        newrpl/utf8lib.c \

# Hardware abstraction layer (HAL)
SOURCES += \
        firmware/hal_alarm.c \
        firmware/hal_clock.c \
        firmware/hal_cpu.c \
        firmware/hal_globals.c \
        firmware/hal_keyboard.c \
        firmware/hal_screen.c \
        firmware/hal_battery.c \

# Graphics
newrpl_color:DEFINES += NEWRPL_COLOR

SOURCES += \
        firmware/ggl/ggl_initscr.c \

# System
SOURCES += \
        firmware/sys/Font10A.c \
        firmware/sys/Font18.c \
        firmware/sys/Font24.c \
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
        firmware/sys/FontNotification.c \
        firmware/sys/fsystem/fatconvert.c \
        firmware/sys/fsystem/fsallocator.c \
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
        firmware/sys/graphics.c \
        firmware/sys/icons.c \
        firmware/sys/keybcommon.c \
        firmware/sys/usbcommon.c \
        firmware/ui_cmdline.c \
        firmware/ui_forms.c \
        firmware/ui_render.c \
        firmware/ui_softmenu.c \

# Flight recorder
SOURCES += recorder/recorder.c recorder/recorder_ring.c
HEADERS += recorder/recorder.h
INCLUDEPATH += recorder

# Headers for the newRPL core
HEADERS += \
        newrpl/arithmetic.h \
        newrpl/cmdcodes.h \
        newrpl/common-macros.h \
        newrpl/decimal.h \
        newrpl/fastmath.h \
        newrpl/include-all.h \
        newrpl/lib-header.h \
        newrpl/libraries.h \
        newrpl/newrpl.h \
        newrpl/newrpl_types.h \
        newrpl/render.h \
        newrpl/romlibs.h \
        newrpl/sysvars.h \
        newrpl/utf8lib.h \


# Headers for the firmware
HEADERS  += \
        firmware/include/firmware.h \
        firmware/include/fsystem.h \
        firmware/include/hal_api.h \
        firmware/include/ui.h \
        firmware/include/usb.h \
        firmware/include/ggl.h \
        firmware/sys/fsystem/fsyspriv.h \
        firmware/sys/sddriver.h \

# Platform-dependent header:
HEADERS += \
	firmware/platform/$$PLATFORM/target.h \


#  List of RPL objects, shared by all variants
!newrpl_compiler:RPL_OBJECTS = \
        newrpl/rpl-objects/lib-0.nrpl \
        newrpl/rpl-objects/lib-10.nrpl \
        newrpl/rpl-objects/lib-100.nrpl \
        newrpl/rpl-objects/lib-102.nrpl \
        newrpl/rpl-objects/lib-104.nrpl \
        newrpl/rpl-objects/lib-12.nrpl \
        newrpl/rpl-objects/lib-20.nrpl \
        newrpl/rpl-objects/lib-24.nrpl \
        newrpl/rpl-objects/lib-28.nrpl \
        newrpl/rpl-objects/lib-30.nrpl \
        newrpl/rpl-objects/lib-32.nrpl \
        newrpl/rpl-objects/lib-4081.nrpl \
        newrpl/rpl-objects/lib-48.nrpl \
        newrpl/rpl-objects/lib-52.nrpl \
        newrpl/rpl-objects/lib-54.nrpl \
        newrpl/rpl-objects/lib-55.nrpl \
        newrpl/rpl-objects/lib-56.nrpl \
        newrpl/rpl-objects/lib-62.nrpl \
        newrpl/rpl-objects/lib-64.nrpl \
        newrpl/rpl-objects/lib-65.nrpl \
        newrpl/rpl-objects/lib-66.nrpl \
        newrpl/rpl-objects/lib-68.nrpl \
        newrpl/rpl-objects/lib-70.nrpl \
        newrpl/rpl-objects/lib-72.nrpl \
        newrpl/rpl-objects/lib-74.nrpl \
        newrpl/rpl-objects/lib-76.nrpl \
        newrpl/rpl-objects/lib-77.nrpl \
        newrpl/rpl-objects/lib-78.nrpl \
        newrpl/rpl-objects/lib-8.nrpl \
        newrpl/rpl-objects/lib-80.nrpl \
        newrpl/rpl-objects/lib-88.nrpl \
        newrpl/rpl-objects/lib-9.nrpl \
        newrpl/rpl-objects/lib-96.nrpl \
        newrpl/rpl-objects/lib-98.nrpl \
        newrpl/rpl-objects/version.nrpl \


# Additional RPL compiler, make sure it's in the PATH
rpl_compiler.output = auto_${QMAKE_FILE_BASE}.c
rpl_compiler.commands = $$PWD/tools-bin/newrpl-comp -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
rpl_compiler.input = RPL_OBJECTS
rpl_compiler.variable_out = SOURCES

!newrpl_compiler:QMAKE_EXTRA_COMPILERS += rpl_compiler



# Compiler tweaaks
# gcc and Clang don't like double const specifiers,
# but they are needed for firmware: disable the warning
QMAKE_CFLAGS += -Wno-duplicate-decl-specifier -Wno-implicit-fallthrough
