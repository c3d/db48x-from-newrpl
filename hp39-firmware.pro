#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------

# UNCOMMENT BELOW TO COMPILE IN THUMB MODE
THUMB_MODE=-mthumb

include(hp50-firmware.pro)

TARGET = hp39-firmware.elf

DEFINES -= TARGET_50G
DEFINES += TARGET_39GS

HEADERS += \
    firmware/include/target_39gs.h

OBJECTS_DIR = build/hp39-firmware
