#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------

CONFIG += color

include(hp50-simulator.pro)

TARGET = prime-simulator
TEMPLATE = app

DEFINES += TARGET_PC TARGET_PC_PRIMEG1 TARGET_PRIME1  "NEWRPL_BUILDNUM=$$system(git rev-list --count HEAD)"

OBJECTS_DIR = build/prime-simulator

SOURCES += \
        firmware/ggl/cgl/cgl_bitblt.c \
        firmware/ggl/cgl/cgl_bitbltoper.c \
        firmware/ggl/cgl/cgl_filter.c \
        firmware/ggl/cgl/cgl_fltdarken.c \
        firmware/ggl/cgl/cgl_fltinvert.c \
        firmware/ggl/cgl/cgl_fltlighten.c \
        firmware/ggl/cgl/cgl_fltreplace.c \
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
        firmware/sys/FontNotification.c \


HEADERS  += \
        firmware/include/cgl.h \

RESOURCES -= annunciators.qrc
RESOURCES += annunciators-prime.qrc

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-Prime

ANDROID_EXTRA_LIBS = $$PWD/external/libusb-1.0.22/android/libs/x86/libusb1.0.so $$PWD/external/libusb-1.0.22/android/libs/armeabi-v7a/libusb1.0.so
