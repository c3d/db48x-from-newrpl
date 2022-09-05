#******************************************************************************
# primeg1-multiload-firmware.pro                                  BD48X project
#******************************************************************************
#
#  File Description:
#
#
#
#
#
#
#
#
#
#
#******************************************************************************
#  (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
#  (C) 2022 Claudio Lapilli and the newRPL team
#  This software is licensed under the terms described in LICENSE.txt
#******************************************************************************

SOURCES += \
    firmware/sys/target_prime1/preamble_multiload.c \
    firmware/sys/target_prime1/boot_multiload.c \

include(primeg1-firmware.pro)

SOURCES -= \
        firmware/sys/target_prime1/preamble_prime1.c \
        firmware/sys/target_prime1/boot_prime1.c \

TARGET = primeg1_multiload.elf

DISTFILES += firmware/sys/target_prime1/ld_multiload.script

QMAKE_LFLAGS = -g -T$$PWD/firmware/sys/target_prime1/ld_multiload.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom -pad1024k -outPRIME_OS.ROM $(TARGET)
