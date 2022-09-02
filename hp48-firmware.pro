#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------
include(hp50-firmware.pro)

TARGET = hp48-firmware.elf

DEFINES -= TARGET_50G
DEFINES += TARGET_48GII

HEADERS += \
    firmware/include/target_48gii.h

OBJECTS_DIR = build/hp48-firmware
