#******************************************************************************
# Makefile<bd48x>                                                 BD48X project
#******************************************************************************
#
#  File Description:
#
##    Makefile to build the various possible targets of the BD48X project
##      Avaiable targets include:
##       % make sim:             Build all simulators
##       % make fw:              Build all firmwares
##       % make all:             Build simulators and firmwares
##       % make <name>-sim:      Build one simulator
##       % make <name>-fw:       Build one firmware
##       % make debug-<target>   Build with debugging enabled
##       % make release-<target> Build release, debugging disabled
##       % make help:            Show this message
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

SIMULATORS=hp50 prime
FIRMWARES=hp39 hp40 hp48 hp50 primeg1 primeg1-multiload
CONFIG=debug

sim simulators: $(SIMULATORS:%=%-simulator)
fw firmwares:  $(FIRMWARES:%=%-firmware)
all: simulators firmwares

help:
	@sed -E -e '/^##/s/^##\s?//g;t' -e d $(MAKEFILE_LIST)
	@echo "    Available simulators: $(SIMULATORS)"
	@echo "    Available firmwares:  $(FIRMWARES)"

compiler: compiler-$(CONFIG).mak recorder
	$(MAKE) -f $< install
elf2rom: tools-bin/elf2rom
tools-bin/elf2rom: tools/elf2rom/elf2rom.mak
	cd tools/elf2rom && $(MAKE) -f elf2rom.mak install
tools/elf2rom/elf2rom.mak: tools/elf2rom/elf2rom.pro
	cd tools/elf2rom && qmake $(<F) -o $(@F)

%-sim %-simulator: %-simulator-$(CONFIG).mak compiler recorder .ALWAYS
	$(MAKE) -f $<
%-fw %-firmware: %-firmware-$(CONFIG).mak compiler elf2rom .ALWAYS
	$(MAKE) -f $<

%-$(CONFIG).mak: %.pro
	qmake $< CONFIG+=$(CONFIG) -o $@

recorder: recorder/config.h
recorder/config.h: recorder/Makefile
	cd recorder && $(MAKE)

debug-% %-debug:
	$(MAKE) CONFIG=debug $*
release-% %-release:
	$(MAKE) CONFIG=release $*
debug: debug-sim
release: release-all

.PRECIOUS: %.mak
.ALWAYS:
