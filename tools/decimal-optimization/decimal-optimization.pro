TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += NDEBUG

SOURCES += main.c \
    decimal.c \
    sysvars.c \
    utf8data.c \
    utf8lib.c \
    atan_ltables.c \
    ln_ltables.c \
    lighttranscend.c \
    mul_real_arm.c

DISTFILES +=

HEADERS += \
    decimal.h \
    newrpl.h \
    sysvars.h \
    ui.h \
    utf8lib.h \
    bindecimal.h



# This might need to be adapted to each cross-compiler installation
GCC_LIBDIR = /usr/lib/gcc/arm-none-eabi/4.9.3

INCLUDEPATH += $$GCC_LIBDIR/include
QMAKE_LIBDIR += $$GCC_LIBDIR

# End of cross-compiler dependent


INCLUDEPATH += firmware/include newrpl /usr/local/include /usr/include

LIBS += -lgcc


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
QMAKE_CFLAGS = -mtune=arm920t -mcpu=arm920t -mlittle-endian -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -O2 -pipe -mthumb-interwork -nostdinc
QMAKE_CFLAGS_APP =

QMAKE_LFLAGS_DEBUG =
QMAKE_LFLAGS_SHAPP =
QMAKE_LFLAGS_THREAD =
QMAKE_LFLAGS = -T$$PWD/ld.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

#QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom $(TARGET)


## Additional RPL compiler, make sure it's in the PATH
rpl_compiler.output = auto_${QMAKE_FILE_BASE}.c
rpl_compiler.commands = $$PWD/tools-bin/newrpl-comp -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
rpl_compiler.input = RPL_OBJECTS
rpl_compiler.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += rpl_compiler
