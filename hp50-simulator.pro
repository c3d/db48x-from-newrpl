#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------

QT       += core gui quick widgets quickcontrols2 quickwidgets

TARGET = hp50-simulator
TEMPLATE = app

DEFINES += TARGET_PC "NEWRPL_BUILDNUM=$$system(git rev-list --count HEAD)"

OBJECTS_DIR = build/hp50-simulator

!color:SOURCES += \
        firmware/ggl/ggl/ggl_bitblt.c \
        firmware/ggl/ggl/ggl_bitbltoper.c \
        firmware/ggl/ggl/ggl_filter.c \
        firmware/ggl/ggl/ggl_fltdarken.c \
        firmware/ggl/ggl/ggl_fltinvert.c \
        firmware/ggl/ggl/ggl_fltlighten.c \
        firmware/ggl/ggl/ggl_fltreplace.c \
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

SOURCES += \
        firmware/hal_alarm.c \
        firmware/hal_battery.c \
        firmware/hal_clock.c \
        firmware/hal_cpu.c \
        firmware/hal_globals.c \
        firmware/hal_keyboard.c \
        firmware/hal_screen.c \
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
        firmware/sys/target_pc/battery.c \
        firmware/sys/target_pc/boot.c \
        firmware/sys/target_pc/cpu.c \
        firmware/sys/target_pc/exception.c \
        firmware/sys/target_pc/flash.c \
        firmware/sys/target_pc/fwupdate.c \
        firmware/sys/target_pc/irq.c \
        firmware/sys/target_pc/keyboard.c \
        firmware/sys/target_pc/lcd.c \
        firmware/sys/target_pc/mem.c \
        firmware/sys/target_pc/rtc.c \
        firmware/sys/target_pc/sddriver.c \
        firmware/sys/target_pc/stdlib.c \
        firmware/sys/target_pc/timer.c \
        firmware/sys/target_pc/usbdriver.c \
        firmware/sys/usbcommon.c \
        firmware/ui_cmdline.c \
        firmware/ui_forms.c \
        firmware/ui_render.c \
        firmware/ui_softmenu.c \
        interaction.cpp \
        interaction_rpl.c \
        main.cpp \
        mainwindow.cpp \
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
        qemuscreen.cpp \
        qpaletteeditor.cpp \
        rplthread.cpp \
        usbselector.cpp \


HEADERS  += \
        firmware/include/cgl.h \
        firmware/include/firmware.h \
        firmware/include/fsystem.h \
        firmware/include/ggl.h \
        firmware/include/hal_api.h \
        firmware/include/target_pc.h \
        firmware/include/ui.h \
        firmware/include/usb.h \
        firmware/include/xgl.h \
        firmware/sys/fsystem/fsyspriv.h \
        firmware/sys/sddriver.h \
        mainwindow.h \
        menuwidget.h \
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
        qemuscreen.h \
        qpaletteeditor.h \
        rplthread.h \
        usbselector.h \

include(newrpl-libs.pri)

INCLUDEPATH += firmware/include newrpl

LIBS += -L/usr/local/lib

FORMS    += mainwindow.ui \
    qpaletteeditor.ui \
    usbselector.ui

RESOURCES += \
    annunciators.qrc

# Set application icon for the windows applications - this is target-specific
win32: RC_ICONS = bitmap/newRPL.ico


# gcc and Clang don't like double const specifiers, but are needed for firmware: disable the warning
QMAKE_CFLAGS += -Wno-duplicate-decl-specifier -Wno-implicit-fallthrough



# Additional RPL compiler, make sure it's in the PATH
rpl_compiler.output = auto_${QMAKE_FILE_BASE}.c
rpl_compiler.commands = $$PWD/tools-bin/newrpl-comp -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
rpl_compiler.input = RPL_OBJECTS
rpl_compiler.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += rpl_compiler


# Additional external library HIDAPI linked statically into the code


INCLUDEPATH += external/hidapi/hidapi

HEADERS += external/hidapi/hidapi/hidapi.h

win32: SOURCES += external/hidapi/windows/hid.c
win32: LIBS += -lsetupapi

android: SOURCES += external/hidapi/libusb/hid.c
android: INCLUDEPATH += external/libusb-1.0.22/libusb/
android: LIBS += -L$$PWD/external/libusb-1.0.22/android/libs/armeabi-v7a -L$$PWD/external/libusb-1.0.22/android/libs/x86 -lusb1.0

freebsd: SOURCES += external/hidapi/libusb/hid.c
freebsd: LIBS += -lusb -lthr -liconv

unix:!macx:!freebsd:!android: SOURCES += external/hidapi/linux/hid.c
unix:!macx:!freebsd:!android: LIBS += -ludev

macx: SOURCES += external/hidapi/mac/hid.c
macx: LIBS += -framework CoreFoundation -framework IOKit

# End of HIDAPI

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

ANDROID_EXTRA_LIBS = $$PWD/external/libusb-1.0.22/android/libs/x86/libusb1.0.so $$PWD/external/libusb-1.0.22/android/libs/armeabi-v7a/libusb1.0.so
