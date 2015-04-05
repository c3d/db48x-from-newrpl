#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------


TARGET = newrpl-fw
TEMPLATE = lib
CONFIG = static ordered

DEFINES += TARGET_50G

SOURCES +=\
    firmware/sys/target_50g/preamble.c \
    firmware/sys/target_50g/boot.c \
    firmware/sys/target_50g/battery.c \
    firmware/sys/target_50g/cpu.c \
    firmware/sys/target_50g/exception.c \
    firmware/sys/target_50g/irq.c \
    firmware/sys/target_50g/keyboard.c \
    firmware/sys/target_50g/lcd.c \
    firmware/sys/target_50g/stdlib.c \
    firmware/sys/target_50g/timer.c \
    firmware/sys/target_50g/mem.c \
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
    firmware/sys/font5.c \
    firmware/sys/font6.c \
    firmware/sys/font7.c \
    firmware/sys/font8.c \
    firmware/sys/graphics.c \
    firmware/sys/icons.c \
    firmware/sys/minifont.c \
    firmware/hal_globals.c \
    newrpl/atan_1_comp.c \
    newrpl/atan_2_comp.c \
    newrpl/atan_5_comp.c \
    newrpl/atanh_1_comp.c \
    newrpl/atanh_2_comp.c \
    newrpl/atanh_5_comp.c \
    newrpl/compiler.c \
    newrpl/cordic_K_comp.c \
    newrpl/cordic_Kh_comp.c \
    newrpl/datastack.c \
    newrpl/directory.c \
    newrpl/errors.c \
    newrpl/gc.c \
    newrpl/io_substring.c \
    newrpl/lam.c \
    newrpl/lib-24-string.c \
    newrpl/lib-28-dirs.c \
    newrpl/lib-30-complex.c \
    newrpl/lib-32-symbolic.c \
    newrpl/lib-50-lists.c \
    newrpl/lib-64-precision.c \
    newrpl/lib-65-development.c \
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
    newrpl/lib-twenty-lam.c \
    newrpl/lib-two-ident.c \
    newrpl/lists.c \
    newrpl/returnstack.c \
    newrpl/romlibs.c \
    newrpl/runstream.c \
    newrpl/symbolic.c \
    newrpl/sysvars.c \
    newrpl/tempob.c \
    newrpl/transcendentals.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/basearith.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/memory.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/mpdecimal.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/context.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/mpsignal.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/fnt.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/crt.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/constants.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/convolute.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/sixstep.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/difradix2.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/fourstep.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/transpose.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/io.c \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/numbertheory.c \
    firmware/ui_cmdline.c \
    newrpl/lib-48-matrix.c

HEADERS  += \
    firmware/include/ggl.h \
    firmware/include/ui.h \
    firmware/include/hal_api.h \
    newrpl/hal.h \
    newrpl/libraries.h \
    newrpl/newrpl.h \
    newrpl/sysvars.h \
    newrpl/contrib/mpdecimal-2.4.0/libmpdec/mpdecimal.h

# This might need to be adapted to each cross-compiler installation
INCLUDEPATH += /usr/local/lib/gcc/arm-none-eabi/4.9.1/include
QMAKE_LIBDIR += /usr/local/lib/gcc/arm-none-eabi/4.9.1
# End of cross-compiler dependent


INCLUDEPATH += firmware/include newrpl newrpl/contrib/mpdecimal-2.4.0/libmpdec /usr/local/include /usr/include

LIBS +=

FORMS    +=

DISTFILES += \
    firmware/ld.script

RESOURCES +=

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
#QMAKE_AR_CMD = arm-none-eabi-ar -cqs $(TARGET) $(OBJECTS)
QMAKE_AR_CMD = arm-none-eabi-ld --verbose -T$$PWD/firmware/ld.script -nodefaultlibs -nostdlib -L$$QMAKE_LIBDIR $(OBJECTS) -lgcc -o $(TARGET).elf

QMAKE_CFLAGS = -mtune=arm920t -mcpu=arm920t -mlittle-endian -fomit-frame-pointer -msoft-float -Os -pipe -mthumb-interwork -nostdinc
QMAKE_LFLAGS = -nodefaultlibs -nostdlib


