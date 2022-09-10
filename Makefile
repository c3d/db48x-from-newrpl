#******************************************************************************
# Makefile                                                        DB48X project
#******************************************************************************
#
#  File Description:
#
##    Makefile to build the various possible targets of the DB48X project
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
#******************************************************************************
#  (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
#  (C) 2022 Claudio Lapilli and the newRPL team
#  This software is licensed under the terms described in LICENSE.txt
#******************************************************************************

SIMULATORS=hp50 prime
FIRMWARES=hp39 hp40 hp48 hp50 prime prime-multiload
CONFIG=debug
OS=$(shell uname -s)
TAG=$(OS)-$(CONFIG)

sim simulators: $(SIMULATORS:%=%-simulator)
fw firmwares:  $(FIRMWARES:%=%-firmware)
all: simulators firmwares

help:
	@sed -E -e '/^##/s/^##\s?//g;t' -e d $(MAKEFILE_LIST)
	@echo "    Available simulators: $(SIMULATORS)"
	@echo "    Available firmwares:  $(FIRMWARES)"

compiler: compiler-$(TAG).mak recorder
	$(MAKE) -f $< install
elf2rom: tools-bin/elf2rom
tools-bin/elf2rom: tools/elf2rom/elf2rom.mak
	cd tools/elf2rom && $(MAKE) -f elf2rom.mak install
tools/elf2rom/elf2rom.mak: tools/elf2rom/elf2rom.pro
	cd tools/elf2rom && qmake $(<F) -o $(@F)
bmp2font: tools-bin/bmp2font
tools-bin/bmp2font: tools/fonts/bmp2font/bmp2font.mak tools/fonts/bmp2font/main.c
	cd tools/fonts/bmp2font && $(MAKE) -f bmp2font.mak install
tools/fonts/bmp2font/bmp2font.mak: tools/fonts/bmp2font/bmp2font.pro
	cd tools/fonts/bmp2font && qmake $(<F) -o $(@F)

%-sim %-simulator: %-simulator-$(TAG).mak compiler recorder .ALWAYS
	$(MAKE) -f $<
%-fw %-firmware: %-firmware-$(TAG).mak compiler elf2rom .ALWAYS
	$(MAKE) -f $<

%-firmware-$(TAG).mak: %-firmware.pro
	qmake $< -spec devices/linux-generic-g++ CONFIG+=$(CONFIG) -o $@
%-$(TAG).mak: %.pro
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

fonts: bmp2font
	cd bitmap/fonts && ./doallfonts.sh
	mv bitmap/fonts/*.c firmware/sys/
	mv bitmap/fonts/fontlist.h firmware/include/

FILES=$(shell git ls-files '*.c' '*.h' '*.cpp')
reformat clang-format: $(FILES:%=%.clang-format)
%.clang-format:
	clang-format -i $*

.PRECIOUS: %.mak %-$(TAG).mak
.ALWAYS:
