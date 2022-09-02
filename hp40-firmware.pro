#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------
include(hp50-firmware.pro)

TARGET = hp40-firmware.elf

DEFINES -= TARGET_50G
DEFINES += TARGET_40GS

HEADERS += \
    firmware/include/target_40gs.h

OBJECTS_DIR = build/hp40-firmware
