#******************************************************************************
# primeg1-multiload-firmware.pro                                  DB48X project
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

isEmpty(PLATFORM):PLATFORM=prime_multiload

include(prime-firmware.pro)

TARGET = primeg1_multiload.elf

INCLUDEPATH += firmware/platform/prime

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom -pad1024k -outPRIME_OS.ROM $(TARGET)
