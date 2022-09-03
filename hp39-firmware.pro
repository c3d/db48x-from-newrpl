#******************************************************************************
# hp39-firmware.pro                                               DB48X project
#******************************************************************************
#
#  File Description:
#
#    Build the firmware for the HP39
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
#  This software is licensed under the GNU General Public License v3
#******************************************************************************
#  This file is part of BD48X.
#
#  BD48X is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  BD48X is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with BD48X.  If not, see <https://www.gnu.org/licenses/>.
#******************************************************************************

# UNCOMMENT BELOW TO COMPILE IN THUMB MODE
THUMB_MODE=-mthumb

include(hp50-firmware.pro)

TARGET = hp39-firmware.elf

DEFINES -= TARGET_50G
DEFINES += TARGET_39GS

HEADERS += \
    firmware/include/target_39gs.h

OBJECTS_DIR = build/hp39-firmware
