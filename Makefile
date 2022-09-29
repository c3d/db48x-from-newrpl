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

SIMULATORS=hp50 prime dm42
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
tools-bin/elf2rom: tools/elf2rom/elf2rom.mak | compiler
	cd tools/elf2rom && $(MAKE) -f elf2rom.mak install
tools/elf2rom/elf2rom.mak: tools/elf2rom/elf2rom.pro
	cd tools/elf2rom && qmake $(<F) -o $(@F)

bmp2font: tools-bin/bmp2font
tools-bin/bmp2font: tools/fonts/bmp2font/bmp2font.mak tools/fonts/bmp2font/main.c
	cd tools/fonts/bmp2font && $(MAKE) -f bmp2font.mak install
tools/fonts/bmp2font/bmp2font.mak: tools/fonts/bmp2font/bmp2font.pro
	cd tools/fonts/bmp2font && qmake $(<F) -o $(@F)

ttf2font: tools-bin/ttf2font
tools-bin/ttf2font: tools/fonts/ttf2font/ttf2font.mak tools/fonts/ttf2font/ttf2font.c
	cd tools/fonts/ttf2font && $(MAKE) -f ttf2font.mak install
tools/fonts/ttf2font/ttf2font.mak: tools/fonts/ttf2font/ttf2font.pro
	cd tools/fonts/ttf2font && qmake $(<F) -o $(@F)

%-sim %-simulator: %-simulator-$(TAG).mak compiler recorder .ALWAYS
	$(MAKE) -f $<
%-fw %-firmware: %-firmware-$(TAG).mak compiler elf2rom .ALWAYS
	$(MAKE) -f $<

%-firmware-$(TAG).mak: %-firmware.pro | compiler elf2rom
	qmake $< CONFIG+=$(CONFIG) -o $@ -spec devices/linux-generic-g++
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

FONTLIST=firmware/include/fontlist.h
fonts: bmp2font ttf2font
	cd bitmap/fonts && ./doallfonts.sh
	mv bitmap/fonts/*.c firmware/sys/
	mv bitmap/fonts/fontlist.h firmware/include/
	tools-bin/ttf2font -s18 FONT_18 fonts/C43StandardFont.ttf firmware/sys/Font18.c
	tools-bin/ttf2font -s24 FONT_24 fonts/C43StandardFont.ttf firmware/sys/Font24.c
	tools-bin/ttf2font -s32 FONT_32 fonts/C43SNumericFont.ttf firmware/sys/Font32.c
	sed -e 's/_24/_32/g;t' -e d < $(FONTLIST) >> $(FONTLIST)

FILES=$(shell git ls-files '*.c' '*.h' '*.cpp')
reformat clang-format: $(FILES:%=%.clang-format)
%.clang-format:
	clang-format -i $*

.PRECIOUS: %-firmware-$(TAG).mak %-$(TAG).mak
.ALWAYS:
